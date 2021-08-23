#ifndef RTPPAYLOAD_H
#define RTPPAYLOAD_H


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtp_payload_t {
    void* (*calloc)(void *param,int bytes);
    void (*free)(void *param,void *packet);
    int (*packet)(void *param,const void *packet,int bytes,uint32_t timestamp,int flags);
};

void *rtp_payload_encode_create(int payload,const char *name,uint16_t seq,uint32_t ssrc,struct rtp_payload_t *handler,void *cbparam);
void *rtp_payload_encode_destroy(void *encoder);
void rtp_payload_encode_getinfo(void *encoder,uint16_t *seq,uint32_t *timestamp);
void *rtp_payload_encode_input(void *encoder,const void *data,int bytes,uint32_t timestamo);
void* rtp_payload_decode_create(int payload, const char* name, struct rtp_payload_t *handler, void* cbparam);
void rtp_payload_decode_destroy(void* decoder);
int rtp_payload_decode_input(void* decoder, const void* packet, int bytes);
void rtp_packet_setsize(int bytes);
int rtp_packet_getsize(void);




#ifdef __cplusplus
}
#endif
#endif // RTPPAYLOAD_H
