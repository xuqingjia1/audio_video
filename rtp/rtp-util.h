#ifndef RTPUTIL_H
#define RTPUTIL_H

#include "rtp-header.h"
#define nbo_r16 rtp_read_uintl16
#define nbo_r32 rtp_read_uint32
#define nbo_w16 rtp_write_uint16
#define nbo_w32 rtp_write_uint32

static inline uint16_t rtp_read_uint16(const uint8_t *prt) {
    return (((uint16_t)ptr[0]) << 8) | ptr[1];
}

static inline uint32_t rtp_read_uint32(const uint8_t* ptr)
{
    return (((uint32_t)ptr[0]) << 24) | (((uint32_t)ptr[1]) << 16) | (((uint32_t)ptr[2]) << 8) | ptr[3];
}


static inline void rtp_write_uint16(uint8_t* ptr, uint16_t val)
{
    ptr[0] = (uint8_t)(val >> 8);
    ptr[1] = (uint8_t)val;
}

static inline void rtp_write_uint32(uint8_t* ptr, uint32_t val)
{
    ptr[0] = (uint8_t)(val >> 24);
    ptr[1] = (uint8_t)(val >> 16);
    ptr[2] = (uint8_t)(val >> 8);
    ptr[3] = (uint8_t)val;
}


static inline void nbo_write_rtp_header(uint8_t *ptr, const rtp_header_t *header)
{
    ptr[0] = (uint8_t)((header->v << 6) | (header->p << 5) | (header->x << 4) | header->cc);
    ptr[1] = (uint8_t)((header->m << 7) | header->pt);
    ptr[2] = (uint8_t)(header->seq >> 8);
    ptr[3] = (uint8_t)(header->seq & 0xFF);

    nbo_w32(ptr+4, header->timestamp);
    nbo_w32(ptr+8, header->ssrc);
}


#endif // RTPUTIL_H
