#ifndef H264UTIL_H
#define H264UTIL_H

#include <stdio.h>
#include <stdint.h>

typedef struct _nalu_t {
   int startcodeprefix_len;
   unsigned len;
   unsigned max_size;
   int forbidden_bit;
   int nal_reference_idc;
   int nal_uint_type;
   char *buf;
   unsigned short lost_packets;
}nalu_t;


FILE *open_bitstream_file(char *filename);
nalu_t *alloc_nalu(int buffersize);
void free_nalu(nalu_t *n);

void h264_sdp_create(uint8_t *file,uint8_t *ip,uint16_t port,const uint8_t *sps,const int sps_len,const uint8_t *pps,const int ppe_len,int payload_type,int time_base,int bitrate);




#endif // H264UTIL_H
