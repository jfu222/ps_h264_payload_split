// main.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include "MpegProgramStreamParser.h"
#include "version.h"


int printHelp(int argc, char *argv[])
{
    printf("====== MPEG-2 Program Stream Parser Version: %s ======\n", VERSION_STR3(VERSION_STR));
    printf("====== Author: jfu2 ======\n");
    printf("====== Email: 386520874@qq.com ======\n");
    printf("====== Date: 2019.11.29 ======\n\n");

    printf("Usage:\n");
    printf("  %s <in|./test.ps> <outDir|./data/test/>\n", argv[0]);
    printf("For Example:\n");
    printf("  %s ./test.ps ./data/test/\n", argv[0]);

    return 0;
}


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printHelp(argc, argv);
        return -1;
    }

    std::string inputFilename = argv[1];
    std::string outDir = argv[2];

    int ret = 0;
    CMpegProgramStreamParser mpsp;

    ret = mpsp.splitPsByPacketHeader(inputFilename, outDir);
    if (ret != 0)
    {
        printf("Error: mpsp.splitPsByPacketHeader() failed! ret=%d;\n", ret);
    }

    printf("All thing is Over!\n");

    return 0;
}

