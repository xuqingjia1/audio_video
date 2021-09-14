#ifndef RTPPACKET_H
#define RTPPACKET_H


#include "rtp-header.h"


#define RTP_FIXED_HEADER 12     // RTP固定 header的长度


struct rtp_packet_t{
    rtp_header_t rtp;
    uint32_t csrc[16];
    const void *extension;
    uint16_t extlen;
    uint16_t reserved;
    const void *payload;
    int payloadlen;
};

int ret_packet_deserialize(struct rtp_packet_t *pkt,const void *data,int bytes);

int rtp_packet_serialize(const struct rtp_packet_t *pkt,void *data,int bytes);


#endif // RTPPACKET_H
