#ifndef AACUTIL_H
#define AACUTIL_H
#include <stdio.h>
#include <stdint.h>


typedef struct {
    unsigned int syncword;
    unsigned int id;
    unsigned int layer;
    unsigned int protection_absent;
    unsigned int profile;
    unsigned int sampling_frequency_index;
    unsigned int private_bit;
    unsigned int channel_configuration;
    unsigned int original_copy;
    unsigned int home;

    unsigned int copyright_id;
    unsigned int copyright_id_start;
    unsigned int aac_frame_length;
    unsigned int adts_buffer_fullness;
    unsigned int num_raw_data_blocks;

}aac_header_t;



typedef struct {
    aac_header_t header;
    uint8_t adts_buf[9];
    int adts_len;
    uint8_t frame_buf[8192];
    int frame_len;
}aac_frame_t;


extern int aac_freq[];

// 打开文件
FILE *aac_open_bitstream_file (char *filename);
// 根据传入的adts header buffer 解析出来对应的参数， show=1的时候打印解析结果，=0就不打印了
int aac_parse_header(uint8_t* in, aac_header_t* res, uint8_t show);
// 读取一帧完整的AAC帧（adts header + data），不会去查找sync word
int aac_get_one_frame (aac_frame_t *aac_frame, FILE *bits);
void aac_rtp_create_sdp(uint8_t *file, uint8_t *ip, uint16_t port,
                        uint16_t profile, uint16_t chn,
                        uint16_t freq, uint16_t type);

#endif // AACUTIL_H
