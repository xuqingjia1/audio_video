#ifndef RTPPAYLOADINTERNAL_H
#define RTPPAYLOADINTERNAL_H


#include "rtp-payload.h"
#include "rtp-packet.h"
#include "rtp-param.h"
#include "rtp-util.h"

struct rtp_payload_encode_t{
    void *(*create)(int size,uint8_t payload,uint16_t seq,uint32_t ssrc,struct rtp_payload_t *handler,void *cbparam);
    void (*destory)(void *packer);
    void (*get_info)(void *packer,uint16_t *seq,uint32_t *timestamp);
    int (*input)(void *packer,const void *data,int bytes,uint32_t time);
};


struct rtp_payload_decode_t {
    void* (*create)(struct rtp_payload_t *handler, void* param);
    void (*destroy)(void* packer);
    int (*input)(void* decoder, const void* packet, int bytes);
};

struct rtp_payload_encode_t *rtp_h264_encode(void);

struct rtp_payload_decode_t *rtp_h264_decode(void);


int rtp_packet_serialize_header(const struct rtp_packet_t *pkt, void* data, int bytes);


#endif // RTPPAYLOADINTERNAL_H
