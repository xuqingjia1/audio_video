#ifndef RTPPAYLOADHELPER_H
#define RTPPAYLOADHELPER_H

#include "rtp-packet.h"
#include "rtp-payload.h"

struct rtp_payload_helper_t{
    struct rtp_payload_t handler;
    void *cbparam;
    int lost;
    uint16_t seq;
    uint32_t timestamp;
    uint8_t *ptr;
    int size,capacity,maxsize;
    int __flags;
};
void* rtp_payload_helper_create(struct rtp_payload_t *handler, void* cbparam);
void rtp_payload_helper_destroy(void* helper);

int rtp_payload_check(struct rtp_payload_helper_t* helper, const struct rtp_packet_t* pkt);

int rtp_payload_write(struct rtp_payload_helper_t* helper, const struct rtp_packet_t* pkt);

int rtp_payload_onframe(struct rtp_payload_helper_t *helper);



#endif // RTPPAYLOADHELPER_H
