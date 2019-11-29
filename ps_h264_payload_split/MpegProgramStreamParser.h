#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>


//-------------------------------------
//mpeg2
//ISO_IEC13818-1_2000.pdf    Page34
//Table 2-18 – Stream_id assignments
#define SEQUENCE_END_CODE                            0x000001b7
#define ISO_11172_END_CODE                           0x000001b9
#define PROGRAM_STREAM_PACKET_START_CODE             0x000001ba
#define PROGRAM_STREAM_SYSTEM_HEADER_START_CODE      0x000001bb
#define PROGRAM_STREAM_MAP                           0x000001bc
#define PRIVATE_STREAM_1                             0x000001bd
#define PADDING_STREAM                               0x000001be
#define PRIVATE_STREAM_2                             0x000001bf
// audio stream number    110x xxxx    [0x000001c0 - 0x000001df]
// video stream number    1110 xxxx    [0x000001e0 - 0x000001ef]
#define ECM_STREAM                                   0x000001f0
#define EMM_STREAM                                   0x000001f1
#define H2220_DSMCC_STREAM                           0x000001f2
#define ISO_IEC_13522_STREAM                         0x000001f3
#define H2221_A_STREAM                               0x000001f4
#define H2221_B_STREAM                               0x000001f5
#define H2221_C_STREAM                               0x000001f6
#define H2221_D_STREAM                               0x000001f7
#define H2221_E_STREAM                               0x000001f8
#define ANCILLARY_STREAM                             0x000001f9
#define ISO_IEC_14496_1_SL_PACKETIZED_STREAM         0x000001fa
#define ISO_IEC_14496_1_FLEXMUX_STREAM               0x000001fb
// reserved data stream    [0x000001fc - 0x000001fe]
#define PROGRAM_STREAM_DIRECTORY                     0x000001ff

#define AUDIO_ID 0xc0
#define VIDEO_ID 0xe0
#define H264_ID  0xe2
#define AC3_ID   0x80
#define DTS_ID   0x88
#define LPCM_ID  0xa0
#define SUB_ID   0x20

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b // ISO/IEC 13818-1: 2007(C)    Page48    Table 2-34 – Stream type assignments
#define STREAM_TYPE_VIDEO_H265      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_SVAC      0x80
#define STREAM_TYPE_AUDIO_G711      0x90
#define STREAM_TYPE_AUDIO_G7221     0x92
#define STREAM_TYPE_AUDIO_G7231     0x93
#define STREAM_TYPE_AUDIO_G729      0x99
#define STREAM_TYPE_AUDIO_SVAC      0x9b

#define STREAM_TYPE_AUDIO_AC3       0x81


//-----------------------------------------
//ISO_IEC13818-1_2000.pdf    Page55
//Table 2-33 – Program Stream pack header
typedef struct _PS_PACKET_HEADER_
{
    int pack_start_code[4]; //32bits    00 00 01 BA
    int magic_01; //2bits    (01)b
    int system_clock_reference_base_32_30[3]; //3bits
    int marker_bit_1; //1bit
    int system_clock_reference_base_29_15[15]; //15bits
    int marker_bit_2; //1bit
    int system_clock_reference_base_14_0[15]; //15bits
    int marker_bit_3; //1bit
    int system_clock_reference_extension; //9bits
    int marker_bit_4; //1bit
    int program_mux_rate; //22bits
    int marker_bit_5; //1bit
    int marker_bit_6; //1bit
    int reserved; //5bits
    int pack_stuffing_length; //3bits
    int ps_packet_payload_length;
    unsigned char * ps_packet_payload_data;

public:
    _PS_PACKET_HEADER_()
    {
        memset(this, 0, sizeof(_PS_PACKET_HEADER_));
    }

    ~_PS_PACKET_HEADER_()
    {
    }

    int printInfo()
    {
        printf("\n----------PS_PACKET_HEADER------------\n");
        printf("pack_start_code: 0x%02x 0x%02x 0x%02x 0x%02x\n", pack_start_code[0], pack_start_code[1], pack_start_code[2], pack_start_code[3]);
        printf("magic_01: 0x%02x\n", magic_01);

        printf("system_clock_reference_base_32_30[3]:");
        for (int i = 0; i < 3; ++i)
        {
            printf(" %d", system_clock_reference_base_32_30[i]);
        }
        printf("\n");
        printf("marker_bit_1: %d\n", marker_bit_1);

        printf("system_clock_reference_base_29_15[15]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", system_clock_reference_base_29_15[i]);
        }
        printf("\n");
        printf("marker_bit_2: %d\n", marker_bit_2);

        printf("system_clock_reference_base_14_0[15]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", system_clock_reference_base_14_0[i]);
        }
        printf("\n");
        printf("marker_bit_3: %d\n", marker_bit_3);

        printf("system_clock_reference_extension: %d(0x%x)\n", system_clock_reference_extension, system_clock_reference_extension);
        printf("marker_bit_4: %d\n", marker_bit_4);
        printf("program_mux_rate: %d\n", program_mux_rate);
        printf("marker_bit_5: %d\n", marker_bit_5);
        printf("marker_bit_6: %d\n", marker_bit_6);
        printf("reserved: %d\n", reserved);
        printf("pack_stuffing_length: %d(0x%x)\n", pack_stuffing_length, pack_stuffing_length);
        printf("ps_packet_payload_length: %d(0x%x)\n", ps_packet_payload_length, ps_packet_payload_length);
        printf("ps_packet_payload_data: %p\n", ps_packet_payload_data);

        return 0;
    }
}PS_PACKET_HEADER;


typedef struct _PS_P_STD_BUFFER_INFO_
{
    int stream_id; //8bits
    int magic_11; //2bits
    int P_STD_buffer_bound_scale; //1bit
    int P_STD_buffer_size_bound; //13bits

public:
    _PS_P_STD_BUFFER_INFO_()
    {
        memset(this, 0, sizeof(_PS_P_STD_BUFFER_INFO_));
    }

    ~_PS_P_STD_BUFFER_INFO_()
    {
    }

    int printInfo()
    {
        printf("----------PS_P_STD_BUFFER_INFO------------\n");
        printf("stream_id: %d(0x%02x)\n", stream_id, stream_id);
        printf("magic_11: 0x%x\n", magic_11);
        printf("P_STD_buffer_bound_scale: %d(0x%x)\n", P_STD_buffer_bound_scale, P_STD_buffer_bound_scale);
        printf("P_STD_buffer_size_bound: %d(0x%x)\n", P_STD_buffer_size_bound, P_STD_buffer_size_bound);

        return 0;
    }
}PS_P_STD_BUFFER_INFO;


//ISO_IEC13818-1_2000.pdf    Page56
//Table 2-34 – Program Stream system header
typedef struct _PS_SYSTEM_HEADER_
{
    int system_header_start_code[4]; //32bits    00 00 01 BB
    int header_length; //16bits
    int marker_bit_1; //1bit
    int rate_bound; //22bits
    int marker_bit_2; //1bit
    int audio_bound; //6bits
    int fixed_flag; //1bit
    int CSPS_flag; //1bit
    int system_audio_lock_flag; //1bit
    int system_video_lock_flag; //1bit
    int marker_bit_3; //1bit
    int video_bound; //5bits
    int packet_rate_restriction_flag; //1bit
    int reserved_bits; //7bit
    std::vector<PS_P_STD_BUFFER_INFO> P_STD_buffer_infos;

public:
    _PS_SYSTEM_HEADER_()
    {
        memset(this, 0, sizeof(_PS_SYSTEM_HEADER_));
    }

    ~_PS_SYSTEM_HEADER_()
    {
    }

    int printInfo()
    {
        printf("----------PS_SYSTEM_HEADER------------\n");
        printf("system_header_start_code: 0x%02x 0x%02x 0x%02x 0x%02x\n", system_header_start_code[0], system_header_start_code[1], system_header_start_code[2], system_header_start_code[3]);
        printf("header_length: %d(0x%x)\n", header_length, header_length);

        printf("marker_bit_1: %d\n", marker_bit_1);
        printf("rate_bound: %d\n", rate_bound);
        printf("marker_bit_2: %d\n", marker_bit_2);
        printf("audio_bound: %d\n", audio_bound);
        printf("fixed_flag: %d\n", fixed_flag);
        printf("CSPS_flag: %d\n", CSPS_flag);
        printf("system_audio_lock_flag: %d\n", system_audio_lock_flag);
        printf("system_video_lock_flag: %d\n", system_video_lock_flag);
        printf("marker_bit_3: %d\n", marker_bit_3);
        printf("video_bound: %d\n", video_bound);
        printf("packet_rate_restriction_flag: %d\n", packet_rate_restriction_flag);
        printf("reserved_bits: %d\n", reserved_bits);

        for (int i = 0; i < P_STD_buffer_infos.size(); ++i)
        {
            printf("%d/%d:\n", i, P_STD_buffer_infos.size());
            P_STD_buffer_infos[i].printInfo();
        }

        return 0;
    }
}PS_SYSTEM_HEADER;


typedef struct _PS_ELEMENTARY_STREAM_MAP_INFO_
{
    int stream_type; //8bits
    int elementary_stream_id; //8bits
    int elementary_stream_info_length; //16bits

public:
    _PS_ELEMENTARY_STREAM_MAP_INFO_()
    {
        memset(this, 0, sizeof(_PS_ELEMENTARY_STREAM_MAP_INFO_));
    }

    ~_PS_ELEMENTARY_STREAM_MAP_INFO_()
    {
    }

    int printInfo()
    {
        printf("----------PS_ELEMENTARY_STREAM_MAP_INFO------------\n");
        printf("stream_type: %d(0x%02x)\n", stream_type, stream_type);
        printf("elementary_stream_id: %d(0x%x)\n", elementary_stream_id, elementary_stream_id);
        printf("elementary_stream_info_length: %d(0x%x)\n", elementary_stream_info_length, elementary_stream_info_length);

        return 0;
    }
}PS_ELEMENTARY_STREAM_MAP_INFO;


//ISO_IEC13818-1_2000.pdf    Page59
//Table 2-35 – Program Stream map
typedef struct _PS_MAP_HEADER_
{
    int packet_start_code_prefix; //24bits    00 00 01
    int map_stream_id; //8bits    BC
    int program_stream_map_length; //16bits
    int current_next_indicator; //1bit
    int reserved_1; //2bits
    int program_stream_map_version; //5bits
    int reserved_2; //7bits
    int marker_bit_1; //1bit
    int program_stream_info_length; //16bits
    int elementary_stream_map_length; //16bits
    std::vector<PS_ELEMENTARY_STREAM_MAP_INFO> ps_elementary_stream_map_infos;
    int CRC_32; //32bits

public:
    _PS_MAP_HEADER_()
    {
        packet_start_code_prefix = 0;
        map_stream_id = 0;
        program_stream_map_length = 0;
        current_next_indicator = 0;
        reserved_1 = 0;
        program_stream_map_version = 0;
        reserved_2 = 0;
        marker_bit_1 = 0;
        program_stream_info_length = 0;
        elementary_stream_map_length = 0;
        CRC_32 = 0;
    }

    ~_PS_MAP_HEADER_()
    {
    }

    int printInfo()
    {
        printf("\n----------PS_MAP_HEADER------------\n");
        printf("packet_start_code_prefix: 0x%06x\n", packet_start_code_prefix);
        printf("map_stream_id: %d(0x%02x)\n", map_stream_id, map_stream_id);
        printf("program_stream_map_length: %d(0x%x)\n", program_stream_map_length, program_stream_map_length);
        printf("current_next_indicator: %d\n", current_next_indicator);
        printf("reserved_1: %d\n", reserved_1);
        printf("program_stream_map_version: %d\n", program_stream_map_version);
        printf("reserved_2: %d\n", reserved_2);
        printf("marker_bit_1: %d\n", marker_bit_1);
        printf("program_stream_info_length: %d(0x%x)\n", program_stream_info_length, program_stream_info_length);
        printf("elementary_stream_map_length: %d(0x%x)\n", elementary_stream_map_length, elementary_stream_map_length);
        printf("CRC_32: 0x%08x\n", CRC_32);

        for (int i = 0; i < ps_elementary_stream_map_infos.size(); ++i)
        {
            printf("%d/%d:\n", i, ps_elementary_stream_map_infos.size());
            ps_elementary_stream_map_infos[i].printInfo();
        }

        return 0;
    }
}PS_MAP_HEADER;


typedef struct _PS_ACCESS_UNITS_
{
    int packet_stream_id; //8bits
    int PES_header_position_offset_sign; //1bit
    int PES_header_position_offset_43_30[14]; //14bits
    int marker_bit_1; //1bit
    int PES_header_position_offset_29_15[15]; //15bits
    int marker_bit_2; //1bit
    int PES_header_position_offset_14_0[15]; //15bits
    int marker_bit_3; //1bit
    int reference_offset; //16bits
    int marker_bit_4; //1bit
    int reserved_1; //3bits
    int PTS_32_30[3]; //3bits
    int marker_bit_5; //1bit
    int PTS_29_15[15]; //15bits
    int marker_bit_6; //1bit
    int PTS_14_0[15]; //15bits
    int marker_bit_7; //1bit
    int bytes_to_read_22_8[15]; //15bits
    int marker_bit_8; //1bit
    int bytes_to_read_7_0[8]; //8bits
    int marker_bit_9; //1bit
    int intra_coded_indicator; //1bit
    int coding_parameters_indicator; //2bits
    int reserved_2; //4bits

public:
    _PS_ACCESS_UNITS_()
    {
        memset(this, 0, sizeof(_PS_ACCESS_UNITS_));
    }

    ~_PS_ACCESS_UNITS_()
    {
    }

    int printInfo()
    {
        printf("----------PS_ELEMENTARY_STREAM_MAP_INFO------------\n");
        printf("packet_stream_id: %d(0x%x)\n", packet_stream_id, packet_stream_id);
        printf("PES_header_position_offset_sign: %d\n", PES_header_position_offset_sign);

        printf("PES_header_position_offset_43_30[14]:");
        for (int i = 0; i < 14; ++i)
        {
            printf(" %d", PES_header_position_offset_43_30[i]);
        }
        printf("\n");
        printf("marker_bit_1: %d\n", marker_bit_1);

        printf("PES_header_position_offset_29_15[15]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", PES_header_position_offset_29_15[i]);
        }
        printf("\n");
        printf("marker_bit_2: %d\n", marker_bit_2);

        printf("PES_header_position_offset_14_0[14]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", PES_header_position_offset_14_0[i]);
        }
        printf("\n");
        printf("marker_bit_3: %d\n", marker_bit_3);

        printf("reference_offset: %d\n", reference_offset);
        printf("marker_bit_4: %d\n", marker_bit_4);
        printf("reserved_1: %d\n", reserved_1);

        //---------------------------
        printf("PTS_32_30[3]:");
        for (int i = 0; i < 3; ++i)
        {
            printf(" %d", PTS_32_30[i]);
        }
        printf("\n");
        printf("marker_bit_5: %d\n", marker_bit_5);

        printf("PTS_29_15[15]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", PTS_29_15[i]);
        }
        printf("\n");
        printf("marker_bit_6: %d\n", marker_bit_6);

        printf("PTS_14_0[15]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", PTS_14_0[i]);
        }
        printf("\n");
        printf("marker_bit_7: %d\n", marker_bit_7);

        //---------------------------
        printf("bytes_to_read_22_8[15]:");
        for (int i = 0; i < 15; ++i)
        {
            printf(" %d", bytes_to_read_22_8[i]);
        }
        printf("\n");
        printf("marker_bit_8: %d\n", marker_bit_8);

        printf("bytes_to_read_7_0[8]:");
        for (int i = 0; i < 8; ++i)
        {
            printf(" %d", bytes_to_read_7_0[i]);
        }
        printf("\n");
        printf("marker_bit_9: %d\n", marker_bit_9);

        printf("intra_coded_indicator: %d\n", intra_coded_indicator);
        printf("coding_parameters_indicator: %d\n", coding_parameters_indicator);
        printf("reserved_2: %d\n", reserved_2);

        return 0;
    }
}PS_ACCESS_UNITS;


//ISO_IEC13818-1_2000.pdf    Page61
//Table 2-36 – Program Stream directory packet
typedef struct _PS_DIRECTORY_PES_PACKET_HEADER_
{
    int packet_start_code_prefix; //24bits    00 00 01
    int stream_id; //8bits    FF
    int PES_packet_length; //16bits
    int number_of_access_units; //15bits
    int marker_bit_1; //1bit
    int prev_directory_offset_44_30[15]; //15bits
    int marker_bit_2; //1bit
    int prev_directory_offset_29_15[15]; //15bits
    int marker_bit_3; //1bit
    int prev_directory_offset_14_0[15]; //15bits
    int marker_bit_4; //1bit
    int next_directory_offset_44_30[15]; //15bits
    int marker_bit_5; //1bit
    int next_directory_offset_29_15[15]; //15bits
    int marker_bit_6; //1bit
    int next_directory_offset_14_0[15]; //15bits
    int marker_bit_7; //1bit
    std::vector<PS_ACCESS_UNITS> ps_access_units;

public:
    _PS_DIRECTORY_PES_PACKET_HEADER_()
    {
        packet_start_code_prefix = 0;
        stream_id = 0;
        PES_packet_length = 0;
        number_of_access_units = 0;
        marker_bit_1 = 0;
        memset(prev_directory_offset_44_30, 0, 15);
        marker_bit_2 = 0;
        memset(prev_directory_offset_29_15, 0, 15);
        marker_bit_3 = 0;
        memset(prev_directory_offset_14_0, 0, 15);
        marker_bit_4 = 0;
        memset(next_directory_offset_44_30, 0, 15);
        marker_bit_5 = 0;
        memset(next_directory_offset_29_15, 0, 15);
        marker_bit_6 = 0;
        memset(next_directory_offset_14_0, 0, 15);
        marker_bit_7 = 0;
    }

    ~_PS_DIRECTORY_PES_PACKET_HEADER_()
    {
    }

    int printInfo()
    {
        return 0;
    }
}PS_DIRECTORY_PES_PACKET_HEADER;


//ISO_IEC13818-1_2000.pdf    Page31
//Table 2-17 – PES packet
typedef struct _PS_PES_PACKET_HEADER_
{
    int packet_start_code_prefix; //24bits    00 00 01
    int pes_stream_id; //8bits    
    int pes_packet_length; //16bits
    int ps_pes_packet_payload_length;
    unsigned char * ps_pes_packet_payload_data;

public:
    _PS_PES_PACKET_HEADER_()
    {
        memset(this, 0, sizeof(_PS_PES_PACKET_HEADER_));
    }

    ~_PS_PES_PACKET_HEADER_()
    {
    }

    int printInfo()
    {
        printf("----------PS_PES_PACKET_HEADER------------\n");
        printf("packet_start_code_prefix: %d(0x%06x)\n", packet_start_code_prefix, packet_start_code_prefix);
        printf("pes_stream_id: %d(0x%x)\n", pes_stream_id, pes_stream_id);
        printf("pes_packet_length: %d(0x%x)\n", pes_packet_length, pes_packet_length);
        printf("ps_pes_packet_payload_length: %d(0x%x)\n", ps_pes_packet_payload_length, ps_pes_packet_payload_length);
        printf("ps_pes_packet_payload_data: %p\n", ps_pes_packet_payload_data);

        return 0;
    }
}PS_PES_PACKET_HEADER;


//-------------------------------------
typedef struct _BITS_STREAM_
{
    unsigned char * start;
    unsigned char * p;
    unsigned char * end;
    int bits_left;

public:
    _BITS_STREAM_(unsigned char *buffer, int bufferSize)
    {
        start = buffer;
        p = buffer;
        end = buffer + bufferSize;
        bits_left = 8;
    }
    ~_BITS_STREAM_(){}

    bool isEnd()
    {
        return p == end;
    }

    unsigned int readOneBit()
    {
        unsigned int ret = 0;

        bits_left--;

        if (!isEnd())
        {
            ret = ((*(p)) >> bits_left) & 0x01;
        }

        if (bits_left == 0 && p < end)
        {
            p++;
            bits_left = 8;
        }

        return ret;
    }

    int skipOneBit()
    {
        int ret = 0;

        bits_left--;
        if (bits_left == 0 && p < end)
        {
            p++;
            bits_left = 8;
        }

        return ret;
    }

    unsigned int readBits(int n)
    {
        unsigned int ret = 0;

        for (int i = 0; i < n; ++i)
        {
            ret |= (readOneBit() << (n - i - 1));
        }
        return ret;
    }

    unsigned int getBits(int n)
    {
        unsigned int ret = 0;

        unsigned char * p2 = p;
        int bits_left2 = bits_left;

        ret = readBits(n);

        p = p2;
        bits_left = bits_left2;

        return ret;
    }

    int skipBits(int n)
    {
        int ret = 0;

        for (int i = 0; i < n; ++i)
        {
            ret = skipOneBit();
        }
        return ret;
    }
}BITS_STREAM;


//-----------------------------------------
class CMpegProgramStreamParser
{
public:
    bool m_isFixPsFile; //是否修复有问题的原始PS封装文件

public:
    CMpegProgramStreamParser();
    ~CMpegProgramStreamParser();

    int getNextStartCode(unsigned char *data, int len, unsigned char codeType, unsigned char *&pos);
    int getNextStartCodeRange(unsigned char *data, int len, unsigned char codeTypeMin, unsigned char codeTypeMax, unsigned char *&pos);
    int splitPsByPacketHeader(const std::string inputFilename, const std::string outDir);

    int readOnePsPacketHeader(unsigned char *buffer, int bufferSize, PS_PACKET_HEADER &psPacketHeader, unsigned char *&newPos, unsigned char *bufferBase);
    int readOnePsSystemHeader(unsigned char *buffer, int bufferSize, PS_SYSTEM_HEADER &psSystemHeader, unsigned char *&newPos, unsigned char *bufferBase);
    int readOnePsMapHeader(unsigned char *buffer, int bufferSize, PS_MAP_HEADER &psMapHeader, unsigned char *&newPos, unsigned char *bufferBase);
    int readOnePesPacketHeader(unsigned char *buffer, int bufferSize, PS_PES_PACKET_HEADER &psPesHeader, unsigned char *&newPos, unsigned char *bufferBase, FILE * fp);
};

