#include "MpegProgramStreamParser.h"
#include "CommonFunction.h"


#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define RETURN_IF_FAILED(condition, ret)                                                      \
    do                                                                                        \
    {                                                                                         \
        if (condition)                                                                        \
        {                                                                                     \
            printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);    \
            return ret;                                                                       \
        }                                                                                     \
    } while (0)


CMpegProgramStreamParser::CMpegProgramStreamParser()
{
    m_isFixPsFile = false;
}


CMpegProgramStreamParser::~CMpegProgramStreamParser()
{

}


int CMpegProgramStreamParser::getNextStartCode(unsigned char *data, int len, unsigned char codeType, unsigned char *&pos)
{
    for (int i = 0; i < len - 4; ++i)
    {
        if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x01 && data[i + 3] == codeType)
        {
            pos = data + i;
            return 0;
        }
    }
    return -1;
}


int CMpegProgramStreamParser::getNextStartCodeRange(unsigned char *data, int len, unsigned char codeTypeMin, unsigned char codeTypeMax, unsigned char *&pos)
{
    for (int i = 0; i < len - 4; ++i)
    {
        if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x01 && (data[i + 3] >= codeTypeMin && data[i + 3] <= codeTypeMax))
        {
            pos = data + i;
            return 0;
        }
    }
    return -1;
}


int CMpegProgramStreamParser::splitPsByPacketHeader(const std::string inputFilename, const std::string outDir)
{
    int ret = 0;

    //-----------------------------------
    printf("%s(%d): %s: inputFilename=%s;\n", __FILE__, __LINE__, __FUNCTION__, inputFilename.c_str());

    FILE * fp = fopen(inputFilename.c_str(), "rb");
    RETURN_IF_FAILED(fp == NULL, -2);

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * fileSize); //暂时将整个文件读入内存
    RETURN_IF_FAILED(buffer == NULL, -3);

    size_t readSize = fread(buffer, fileSize, 1, fp);
    if (readSize != 1)
    {
        printf("%s(%d): Error: read_size=%d != 1\n", __FUNCTION__, __LINE__, readSize);
        fclose(fp);
        free(buffer);
    }

    fclose(fp);

    RETURN_IF_FAILED(fileSize < 24, -3);

    //-----------------------------------
    std::string dirName;
    std::string baseName;
    std::string extensionName;

    ret = getFileDirnameAndBasenameAndExtname(inputFilename.c_str(), dirName, baseName, extensionName);
    RETURN_IF_FAILED(ret != 0, -1);
    
    ret = createNestedDir(outDir.c_str());
    RETURN_IF_FAILED(ret != 0, -1);

    std::string outputFilename = outDir + "/" + baseName + ".h264";
    printf("%s(%d): %s: outputFilename=%s;\n", __FILE__, __LINE__, __FUNCTION__, outputFilename.c_str());

    FILE * fp2 = fopen(outputFilename.c_str(), "wb");
    RETURN_IF_FAILED(fp2 == NULL, -2);

    //-----------------------------------
    unsigned char *p = buffer;
    unsigned char *p1 = p;
    unsigned char *p2 = p;
    unsigned char *p3 = buffer + fileSize - 1;
    size_t writeBytes = 0;
    int cnt = 0;

    while (p1 + 4 <= p3)
    {
        ret = getNextStartCode(p1, p3 - p1 + 1, 0xBA, p1); // 00 00 01 BA    PROGRAM_STREAM_PACKET_START_CODE
        if (ret != 0)
        {
            printf("%s(%d): %s: getNextStartCode() failed!\n", __FILE__, __LINE__, __FUNCTION__);
            break;
        }

        if (cnt > 0)
        {
            int psPacketLength = p1 - p2;
            std::string outputFilename = outDir + "/" + std::to_string(cnt) + ".ps";
            printf("outputFilename = %s; file_offset=%d(0x%x); psPacketLength=%d(0x%x);\n", outputFilename.c_str(), p1 - p, p1 - p, psPacketLength, psPacketLength);
/*
            FILE * fp3 = fopen(outputFilename.c_str(), "wb");
            RETURN_IF_FAILED(fp3 == NULL, -2);

            writeBytes = fwrite(p2, psPacketLength, 1, fp3);

            fclose(fp3);
*/
            //--------------------
            unsigned char *p11 = p2;
            unsigned char *p22 = p11;
            unsigned char *p33 = p11 + psPacketLength - 1;

            //--------------------
            PS_PACKET_HEADER psPacketHeader;

            ret = readOnePsPacketHeader(p11, p33 - p11 + 1, psPacketHeader, p11, p);
            RETURN_IF_FAILED(ret != 0, -3);
            ret = psPacketHeader.printInfo();

            //--------------------
            ret = getNextStartCode(p11, p33 - p11 + 1, 0xBB, p11); // 00 00 01 BB    PROGRAM_STREAM_SYSTEM_HEADER_START_CODE
            if (ret == 0)
            {
                PS_SYSTEM_HEADER psSystemHeader;

                ret = readOnePsSystemHeader(p11, p33 - p11 + 1 - (p11 - p22 + 1), psSystemHeader, p11, p);
                RETURN_IF_FAILED(ret != 0, -3);
                ret = psSystemHeader.printInfo();
            }

            p22 = p11;
            ret = getNextStartCode(p11, p33 - p11 + 1, 0xBC, p11); // 00 00 01 BC    PROGRAM_STREAM_MAP
            if (ret == 0)
            {
                PS_MAP_HEADER psMapHeader;

                ret = readOnePsMapHeader(p11, p33 - p11 + 1, psMapHeader, p11, p);
                RETURN_IF_FAILED(ret != 0, -3);
                ret = psMapHeader.printInfo();
            }

            //-------------------------
            int cnt2 = 0;

            while (1)
            {
                // audio stream number    110x xxxx    [0x000001c0 - 0x000001df]
                // video stream number    1110 xxxx    [0x000001e0 - 0x000001ef]
                ret = getNextStartCodeRange(p11, p33 - p11 + 1, 0xC0, 0xEF, p11); // 00 00 01 E0    VIDEO_ID
                if (ret == 0 || cnt2 > 0)
                {
                    if (p11 - p == 0x42773)
                    {
                        int a = 1;
                    }

                    if (ret != 0 && cnt2 > 0)
                    {
                        p11 = p33;
                    }

                    printf("%s(%d): %s: file_pos=%d(0x%x);\n", __FILE__, __LINE__, __FUNCTION__, p11 - p, p11 - p);

                    if (cnt2 > 0)
                    {
                        int psPesPacketLength = p11 - p22 + 1;

                        PS_PES_PACKET_HEADER psPesHeader;

                        ret = readOnePesPacketHeader(p22, psPesPacketLength, psPesHeader, p22, p, fp2);
                        if (ret == 0)
                        {
                            ret = psPesHeader.printInfo();
                        }
                        else
                        {
                            break;
                        }
                    }
                    p22 = p11;
                    cnt2++;
                    p11 += 4;
                }
                else
                {
                    break;
                }
            }
        }

        p2 = p1;
        cnt++;
        p1 += 4;
    }

    //------------------
    if (m_isFixPsFile)
    {
        std::string outputFilenameFixed = outDir + "/" + baseName + ".fixed.ps";
        printf("%s(%d): %s: outputFilenameFixed=%s;\n", __FILE__, __LINE__, __FUNCTION__, outputFilenameFixed.c_str());

        FILE * fp3 = fopen(outputFilenameFixed.c_str(), "wb");
        RETURN_IF_FAILED(fp3 == NULL, -2);

        writeBytes = fwrite(buffer, fileSize, 1, fp3);

        fclose(fp3);
    }

    //-----------------
    if (buffer)
    {
        free(buffer);
        buffer = NULL;
    }

    fclose(fp2);

    return ret;
}


int CMpegProgramStreamParser::readOnePsPacketHeader(unsigned char *buffer, int bufferSize, PS_PACKET_HEADER &psPacketHeader, unsigned char *&newPos, unsigned char *bufferBase)
{
    int ret = 0;

    RETURN_IF_FAILED(bufferSize < 14, -1);

    //-------------------------
    unsigned char *p = buffer;
    unsigned char *p1 = p;
    unsigned char *p2 = p;

    if (*(unsigned int *)p1 != 0xBA010000)
    {
        RETURN_IF_FAILED(-1, -1);
    }

    BITS_STREAM bs(p1, bufferSize);

    psPacketHeader.pack_start_code[0] = bs.readBits(8);
    psPacketHeader.pack_start_code[1] = bs.readBits(8);
    psPacketHeader.pack_start_code[2] = bs.readBits(8);
    psPacketHeader.pack_start_code[3] = bs.readBits(8);

    //-----------------------------------------------
    psPacketHeader.magic_01 = bs.readBits(2);
    if (psPacketHeader.magic_01 != 1)
    {
        RETURN_IF_FAILED(-1, -1);
    }

    //-----------------------------------------------
    for (int i = 0; i < 3; ++i)
    {
        psPacketHeader.system_clock_reference_base_32_30[i] = bs.readBits(1);
    }
    psPacketHeader.marker_bit_1 = bs.readBits(1);

    for (int i = 0; i < 15; ++i)
    {
        psPacketHeader.system_clock_reference_base_29_15[i] = bs.readBits(1);
    }
    psPacketHeader.marker_bit_2 = bs.readBits(1);

    for (int i = 0; i < 15; ++i)
    {
        psPacketHeader.system_clock_reference_base_14_0[i] = bs.readBits(1);
    }
    psPacketHeader.marker_bit_3 = bs.readBits(1);

    //-----------------------------------------------
    psPacketHeader.system_clock_reference_extension = bs.readBits(9);
    psPacketHeader.marker_bit_4 = bs.readBits(1);

    //-----------------------------------------------
    psPacketHeader.program_mux_rate = bs.readBits(22);
    psPacketHeader.marker_bit_5 = bs.readBits(1);
    psPacketHeader.marker_bit_6 = bs.readBits(1);

    psPacketHeader.reserved = bs.readBits(5);
    psPacketHeader.pack_stuffing_length = bs.readBits(3);

    RETURN_IF_FAILED(bs.p - p != 14, -1);

    //-----------------------------------------------
    for (int i = 0; i < psPacketHeader.pack_stuffing_length; ++i)
    {
        int stuffing_byte = bs.readBits(8);
    }

    newPos = bs.p;

    return ret;
}


int CMpegProgramStreamParser::readOnePsSystemHeader(unsigned char *buffer, int bufferSize, PS_SYSTEM_HEADER &psSystemHeader, unsigned char *&newPos, unsigned char *bufferBase)
{
    int ret = 0;

    RETURN_IF_FAILED(bufferSize < 12, -1);

    //-------------------------
    unsigned char *p = buffer;
    unsigned char *p1 = p;
    unsigned char *p2 = p;

    if (*(unsigned int *)p1 != 0xBB010000)
    {
        RETURN_IF_FAILED(-1, -1);
    }

    BITS_STREAM bs(p1, bufferSize);

    psSystemHeader.system_header_start_code[0] = bs.readBits(8);
    psSystemHeader.system_header_start_code[1] = bs.readBits(8);
    psSystemHeader.system_header_start_code[2] = bs.readBits(8);
    psSystemHeader.system_header_start_code[3] = bs.readBits(8);

    //-----------------------------------------------
    psSystemHeader.header_length = bs.readBits(16);
    psSystemHeader.marker_bit_1 = bs.readBits(1);

    psSystemHeader.rate_bound = bs.readBits(22);
    psSystemHeader.marker_bit_1 = bs.readBits(1);

    psSystemHeader.audio_bound = bs.readBits(6);
    psSystemHeader.fixed_flag = bs.readBits(1);
    psSystemHeader.CSPS_flag = bs.readBits(1);
    psSystemHeader.system_audio_lock_flag = bs.readBits(1);
    psSystemHeader.system_video_lock_flag = bs.readBits(1);
    psSystemHeader.marker_bit_3 = bs.readBits(1);
    psSystemHeader.video_bound = bs.readBits(5);
    psSystemHeader.packet_rate_restriction_flag = bs.readBits(1);
    psSystemHeader.reserved_bits = bs.readBits(7);

    while (bs.getBits(1) == 0x01)
    {
        PS_P_STD_BUFFER_INFO ps_p_std_buffer_info;

        ps_p_std_buffer_info.stream_id = bs.readBits(8);
        ps_p_std_buffer_info.magic_11 = bs.readBits(2);
        ps_p_std_buffer_info.P_STD_buffer_bound_scale = bs.readBits(1);
        ps_p_std_buffer_info.P_STD_buffer_size_bound = bs.readBits(13);

        psSystemHeader.P_STD_buffer_infos.push_back(ps_p_std_buffer_info);
    }

    newPos = bs.p;

    return ret;
}


int CMpegProgramStreamParser::readOnePsMapHeader(unsigned char *buffer, int bufferSize, PS_MAP_HEADER &psMapHeader, unsigned char *&newPos, unsigned char *bufferBase)
{
    int ret = 0;

    RETURN_IF_FAILED(bufferSize < 16, -1);

    //-------------------------
    unsigned char *p = buffer;
    unsigned char *p1 = p;
    unsigned char *p2 = p;

    if (*(unsigned int *)p1 != 0xBC010000)
    {
        RETURN_IF_FAILED(-1, -1);
    }

    BITS_STREAM bs(p1, bufferSize);

    psMapHeader.packet_start_code_prefix = bs.readBits(24);
    psMapHeader.map_stream_id = bs.readBits(8);

    //-----------------------------------------------
    psMapHeader.program_stream_map_length = bs.readBits(16);
    psMapHeader.current_next_indicator = bs.readBits(1);
    psMapHeader.reserved_1 = bs.readBits(2);
    psMapHeader.program_stream_map_version = bs.readBits(5);
    psMapHeader.reserved_2 = bs.readBits(7);
    psMapHeader.marker_bit_1 = bs.readBits(1);
    psMapHeader.program_stream_info_length = bs.readBits(16);

    //-----------------------------------------------
    for (int i = 0; i < psMapHeader.program_stream_info_length; ++i)
    {
        //descriptor();
        bs.skipBits(8);
    }

    //-----------------------------------------------
    psMapHeader.elementary_stream_map_length = bs.readBits(16);
    int lenTemp = psMapHeader.elementary_stream_map_length; //有的PS流，此值不可信
    
    if (lenTemp >= (psMapHeader.program_stream_map_length - (2 + 2 + 2 + 4)))
    {
        printf("%s(%d): %s: Warn: elementary_stream_map_length(%d) >= program_stream_map_length(%d) - 10; just fix it!\n", __FILE__, __LINE__, __FUNCTION__, lenTemp, psMapHeader.program_stream_map_length);

        //---------修复有问题的原始PS封装文件-----------------
        m_isFixPsFile = true;
        int elementary_stream_map_length_fixed = psMapHeader.elementary_stream_map_length - 4;
        buffer[10 + psMapHeader.program_stream_info_length] = (unsigned char)((elementary_stream_map_length_fixed & 0xFF00) >> 8);
        buffer[11 + psMapHeader.program_stream_info_length] = (unsigned char)(elementary_stream_map_length_fixed & 0x00FF);

        lenTemp -= 4; //HIK的GB PS流就出现这样的问题
    }

    while (lenTemp > 0)
    {
        PS_ELEMENTARY_STREAM_MAP_INFO ps_elementary_stream_map_info;

        ps_elementary_stream_map_info.stream_type = bs.readBits(8);
        ps_elementary_stream_map_info.elementary_stream_id = bs.readBits(8);
        ps_elementary_stream_map_info.elementary_stream_info_length = bs.readBits(16);
        for (int j = 0; j < ps_elementary_stream_map_info.elementary_stream_info_length; ++j)
        {
            //descriptor();
            bs.skipBits(8);
        }
        psMapHeader.ps_elementary_stream_map_infos.push_back(ps_elementary_stream_map_info);
        lenTemp -= (1 + 1 + 2 + ps_elementary_stream_map_info.elementary_stream_info_length);
    }

    psMapHeader.CRC_32 = bs.readBits(32);

//    newPos = bs.p;
    newPos = buffer + 4 + 2 + psMapHeader.program_stream_map_length;

    return ret;
}


int CMpegProgramStreamParser::readOnePesPacketHeader(unsigned char *buffer, int bufferSize, PS_PES_PACKET_HEADER &psPesHeader, unsigned char *&newPos, unsigned char *bufferBase, FILE * fp)
{
    int ret = 0;

    RETURN_IF_FAILED(bufferSize < 12, -1);

    //-------------------------
    unsigned char *p = buffer;
    unsigned char *p1 = p;
    unsigned char *p2 = p;
    unsigned char *p3 = p;

    if (!(p1[0] == 0x00 && p1[1] == 0x00 && p1[2] == 0x01))
    {
        RETURN_IF_FAILED(-1, -1);
    }

    BITS_STREAM bs(p1, bufferSize);

    psPesHeader.packet_start_code_prefix = bs.readBits(24);
    psPesHeader.pes_stream_id = bs.readBits(8);
    psPesHeader.pes_packet_length = bs.readBits(16); //网络丢包的情况下，请不要相信此值
    psPesHeader.ps_pes_packet_payload_length = psPesHeader.pes_packet_length;

    p3 = bs.p;

    unsigned int stream_id = (psPesHeader.packet_start_code_prefix << 8) | psPesHeader.pes_stream_id;
    int PES_packet_length = psPesHeader.pes_packet_length;

    //---------修复有问题的原始PS封装文件-----------------
    if (psPesHeader.pes_packet_length + 6 > bufferSize)
    {
        printf("%s(%d): %s: Warn: psPesHeader.pes_packet_length(%d) + 6 > bufferSize(%d); just fix it!\n", __FILE__, __LINE__, __FUNCTION__, psPesHeader.pes_packet_length, bufferSize);

        m_isFixPsFile = true;
        int PES_packet_length_fixed = bufferSize - 6;
        PES_packet_length = PES_packet_length_fixed;
        buffer[4] = (unsigned char)((PES_packet_length_fixed & 0xFF00) >> 8);
        buffer[5] = (unsigned char)(PES_packet_length_fixed & 0x00FF);
    }

    //-------------------------------------
    if (stream_id != PROGRAM_STREAM_MAP
        && stream_id != PADDING_STREAM
        && stream_id != PRIVATE_STREAM_2
        && stream_id != ECM_STREAM
        && stream_id != EMM_STREAM
        && stream_id != PROGRAM_STREAM_DIRECTORY
        && stream_id != H2220_DSMCC_STREAM
        && stream_id != H2221_E_STREAM)
    {
        int magic_10 = bs.readBits(2); // '10' 2 bslbf
        int PES_scrambling_control = bs.readBits(2); // 2 bslbf
        int PES_priority = bs.readBits(1); // 1 bslbf
        int data_alignment_indicator = bs.readBits(1); // 1 bslbf
        int copyright = bs.readBits(1); // 1 bslbf
        int original_or_copy = bs.readBits(1); // 1 bslbf
        int PTS_DTS_flags = bs.readBits(2); // 2 bslbf
        int ESCR_flag = bs.readBits(1); // 1 bslbf
        int ES_rate_flag = bs.readBits(1); // 1 bslbf
        int DSM_trick_mode_flag = bs.readBits(1); // 1 bslbf
        int additional_copy_info_flag = bs.readBits(1); // 1 bslbf
        int PES_CRC_flag = bs.readBits(1); // 1 bslbf
        int PES_extension_flag = bs.readBits(1); // 1 bslbf
        int PES_header_data_length = bs.readBits(8); // 8 uimsbf
        
        p2 = bs.p;
        if (PTS_DTS_flags == 0x02) // '10'b
        {
            int magic_0010 = bs.readBits(4); // '0010' 4 bslbf
            int PTS_32_30 = bs.readBits(3); // PTS[32..30] 3 bslbf
            int marker_bit_1 = bs.readBits(1); // 1 bslbf
            int PTS_29_15 = bs.readBits(15); // PTS[29..15] 15 bslbf
            int marker_bit_2 = bs.readBits(1); // 1 bslbf
            int PTS_14_0 = bs.readBits(15); // PTS[14..0] 15 bslbf
            int marker_bit_3 = bs.readBits(1); // 1 bslbf
        }

        if (PTS_DTS_flags == 0x03) // '11'b
        {
            int magic_0011 = bs.readBits(4); // '0011' 4 bslbf
            int PTS_32_30 = bs.readBits(3); // PTS[32..30] 3 bslbf
            int marker_bit_1 = bs.readBits(1); // 1 bslbf
            int PTS_29_15 = bs.readBits(15); // PTS[29..15] 15 bslbf
            int marker_bit_2 = bs.readBits(1); // 1 bslbf
            int PTS_14_0 = bs.readBits(15); // PTS[14..0] 15 bslbf
            int marker_bit_3 = bs.readBits(1); // 1 bslbf

            int magic_0001 = bs.readBits(4); // '0011' 4 bslbf
            int DTS_32_30 = bs.readBits(3); // DTS[32..30] 3 bslbf
            int marker_bit_4 = bs.readBits(1); // 1 bslbf
            int DTS_29_15 = bs.readBits(15); // DTS[29..15] 15 bslbf
            int marker_bit_5 = bs.readBits(1); // 1 bslbf
            int DTS_14_0 = bs.readBits(15); // DTS[14..0] 15 bslbf
            int marker_bit_6 = bs.readBits(1); // 1 bslbf
        }

        if (ESCR_flag == 0x01) //'1'b
        {
            int reserved = bs.readBits(2); // 2 bslbf
            int ESCR_base_32_30 = bs.readBits(3); // ESCR_base[32..30] 3 bslbf
            int marker_bit_1 = bs.readBits(1); // 1 bslbf
            int ESCR_base_29_15 = bs.readBits(15); // ESCR_base[29..15] 15 bslbf
            int marker_bit_2 = bs.readBits(1); // 1 bslbf
            int ESCR_base_14_0 = bs.readBits(15); // ESCR_base[14..0] 15 bslbf
            int marker_bit_3 = bs.readBits(1); // 1 bslbf

            int ESCR_extension = bs.readBits(9); // 9 uimsbf
            int marker_bit_4 = bs.readBits(1); // 1 bslbf
        }

        if (ES_rate_flag == 0x01) //'1'b
        {
            int marker_bit_1 = bs.readBits(1); // 1 bslbf
            int ES_rate = bs.readBits(22); // 22 bslbf
            int marker_bit_2 = bs.readBits(1); // 1 bslbf
        }

        if (DSM_trick_mode_flag == 0x01) //'1'b
        {
            int trick_mode_control = bs.readBits(1); // 3 uimsbf

            //ISO_IEC13818-1_2000.pdf    Page37
            //Table 2-20 – Trick mode control value
            if (trick_mode_control == 0x00) // fast_forward = (000)b
            {
                int field_id = bs.readBits(2); // 2 bslbf
                int intra_slice_refresh = bs.readBits(1); // 1 bslbf
                int frequency_truncation = bs.readBits(2); // 2 bslbf
            }
            else if (trick_mode_control == 0x01) // slow_motion = (001)b
            {
                int rep_cntrl = bs.readBits(5); // 5 uimsbf
            }
            else if (trick_mode_control == 0x02) // freeze_frame = (010)b
            {
                int field_id = bs.readBits(2); // 2 uimsbf
                int reserved = bs.readBits(3); // 3 bslbf
            }
            else if (trick_mode_control == 0x03) // fast_reverse = (011)b
            {
                int field_id = bs.readBits(2); // 2 bslbf
                int intra_slice_refresh = bs.readBits(1); // 1 bslbf
                int frequency_truncation = bs.readBits(2); // 2 bslbf
            }
            else if (trick_mode_control == 0x04) // slow_reverse = (100)b
            {
                int rep_cntrl = bs.readBits(5); // 5 uimsbf
            }
            else
            {
                int reserved = bs.readBits(5); // 5 bslbf
            }
        }

        if (additional_copy_info_flag == 0x01) // '1'
        {
            int marker_bit_1 = bs.readBits(1); // 1 bslbf
            int additional_copy_info = bs.readBits(7); // 7 bslbf
        }

        if (PES_CRC_flag == 0x01) // '1'
        {
            int previous_PES_packet_CRC = bs.readBits(16); // 16 bslbf
        }

        if (PES_extension_flag == 0x01) // '1'
        {
            int PES_private_data_flag = bs.readBits(1); // 1 bslbf
            int pack_header_field_flag = bs.readBits(1); // 1 bslbf
            int program_packet_sequence_counter_flag = bs.readBits(1); // 1 bslbf
            int P_STD_buffer_flag = bs.readBits(1); // 1 bslbf
            int reserved = bs.readBits(3); // 3 bslbf
            int PES_extension_flag_2 = bs.readBits(1); // 1 bslbf

            if (PES_private_data_flag == 0x01) // '1'
            {
                unsigned long long PES_private_data_1 = bs.readBits(64); // PES_private_data 128 bslbf
                unsigned long long PES_private_data_2 = bs.readBits(64); // PES_private_data 128 bslbf
            }

            if (pack_header_field_flag == 0x01) // '1'
            {
                int pack_field_length = bs.readBits(8); // 8 uimsbf
                //pack_header();
            }

            if (program_packet_sequence_counter_flag == 0x01) // '1'
            {
                int marker_bit_1 = bs.readBits(1); // 1 bslbf
                int program_packet_sequence_counter = bs.readBits(1); // 7 uimsbf
                int marker_bit_2 = bs.readBits(1); // 1 bslbf
                int MPEG1_MPEG2_identifier = bs.readBits(1); // 1 bslbf
                int original_stuff_length = bs.readBits(1); // 6 uimsbf
            }

            if (P_STD_buffer_flag == 0x01) // '1'
            {
                int magic_01 = bs.readBits(2); // '01' 2 bslbf
                int P_STD_buffer_scale = bs.readBits(1); // 1 bslbf
                int P_STD_buffer_size = bs.readBits(13); // 13 uimsbf
            }

            if (PES_extension_flag_2 == 0x01) // '1'
            {
                int marker_bit_1 = bs.readBits(1); // 1 bslbf
                int PES_extension_field_length = bs.readBits(7); // 7 uimsbf
                for (int i = 0; i < PES_extension_field_length; i++)
                {
                    int reserved = bs.readBits(8); // 8 bslbf
                }
            }
        }

        //-----------------------
        int skipBytes = PES_header_data_length - (bs.p - p2);
        if (skipBytes < 0)
        {
            printf("%s(%d): %s: Error: skipBytes = %d = PES_header_data_length(%d) - (bs.p - p2) (%d);\n", __FILE__, __LINE__, __FUNCTION__, skipBytes, PES_header_data_length, bs.p - p2);
            return -1;
        }

        RETURN_IF_FAILED(bs.bits_left != 8, -1);

        bs.skipBits(skipBytes * 8);

        //---------discarded stuffing byte-----------
        int stuffing_byte_count = 0;
        while (1)
        {
            int stuffing_byte = bs.getBits(8); // 8 bslbf
            if (stuffing_byte == 0xff)
            {
                bs.skipBits(8); // 8 bslbf
            }
            else
            {
                break;
            }
            stuffing_byte_count++;
        }

        //---------PES packet data (H264 payload)-----------
        int pes_data_length = PES_packet_length - 3 - PES_header_data_length - stuffing_byte_count;
//        for (int i = 0; i < pes_data_length; i++)
//        {
//            int PES_packet_data_byte = bs.readBits(8); // 8 bslbf
//        }

        pes_data_length = MIN(pes_data_length, bufferSize);

        //--------------------------
        if (stream_id >= 0x000001c0 && stream_id <= 0x000001df) //AUDIO_ID
        {
            printf("%s(%d): %s: Warn: stream_id(0x%08x) [0x000001c0, 0x000001df] = AUDIO_ID; pes_data_length=%d(0x%x)\n", __FILE__, __LINE__, __FUNCTION__, stream_id, pes_data_length, pes_data_length);
        }
        else if (stream_id >= 0x000001e0 && stream_id <= 0x000001ef) //VIDEO_ID
        {
            printf("%s(%d): %s: INFO: stream_id(0x%08x) [0x000001e0, 0x000001ef] = VIDEO_ID; pes_data_length=%d(0x%x)\n", __FILE__, __LINE__, __FUNCTION__, stream_id, pes_data_length, pes_data_length);
            if (fp)
            {
                fwrite(bs.p, pes_data_length, 1, fp);
            }
        }
        else
        {
            //TODO
        }
    }
    else if (stream_id == PROGRAM_STREAM_MAP
        || stream_id == PRIVATE_STREAM_2
        || stream_id == ECM_STREAM
        || stream_id == EMM_STREAM
        || stream_id == PROGRAM_STREAM_DIRECTORY
        || stream_id == H2220_DSMCC_STREAM
        || stream_id == H2221_E_STREAM)
    {
        for (int i = 0; i < PES_packet_length; i++)
        {
            int PES_packet_data_byte = bs.readBits(8); // 8 bslbf
        }
    }
    else if (stream_id == PADDING_STREAM)
    {
        for (int i = 0; i < PES_packet_length; i++)
        {
            int padding_byte = bs.readBits(8); // 8 bslbf
        }
    }

    newPos = p3 + PES_packet_length;

    return ret;
}
