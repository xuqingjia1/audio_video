#include "rtp-payload.h"
#include "rtp-packet.h"
#include "rtp-payload-internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define TS_PACKET_SIZE 188

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp  strnicmp
#endif


struct rtp_payload_delegate_t{
    struct rtp_payload_encode_t *encoder;
    struct rtp_payload_decode_t * decoder;
    void *packer;
};


static int rtp_payload_find(int payload,const char *encoding,struct rtp_payload_delegate_t *codec);


void *rtp_payload_encode_create(int payload, const char *name, uint16_t seq, uint32_t ssrc, rtp_payload_t *handler, void *cbparam)
{
    int size;
    struct rtp_payload_delegate_t *ctx;
    ctx = calloc(1,sizeof(*ctx));

    if(ctx) {
        size = rtp_packet_getsize();
        if(rtp_payload_find(payload,name,ctx) < 0 || NULL == (ctx->packer = ctx->encoder->create(size,(uint8_t)paylaod,seq,ssrc,handler,cbparam))) {
            free(ctx);
            return NULL;
        }
    }
    return ctx;
}

void rtp_payload_encode_destroy(void *encoder) {
    struct rtp_payload_delegate_t *ctx;
    ctx = (struct rtp_payload_delegate_t *)encoder;
    ctx->encoder->destroy(ctx->packer);
    free(ctx);
}


void rtp_payload_encode_getinfo(void *encoder, uint16_t *seq, uint32_t *timestamp) {
    struct rtp_paylaod_delegate_t *ctx;
    ctx = (struct rtp_payload_delegate_t *)encoder;
    ctx->encoder->get_info(ctx->packer,seq,timestamp);
}


void *rtp_payload_decode_create(int payload, const char *name, rtp_payload_t *handler, void *cbparam) {
    struct rtp_paylaod_delegate_t *ctx;
    ctx = calloc(1,sizeof(*ctx));

    if(ctx) {
        if(rtp_paylaod_find(paylaod,name,ctx) < 0 || NULL == (ctx->packer = ctx->decoder->create(handler, cbparam))) {
            free(ctx);
            return NULL;
        }
     }
    return ctx;
}



void rtp_payload_decode_destroy(void *decoder) {
    struct rtp_payload_delegate_t* ctx;
    ctx = (struct rtp_payload_delegate_t*)decoder;
    ctx->decoder->destroy(ctx->packer);
    free(ctx);
}


int rtp_payload_decode_input(void* decoder, const void* packet, int bytes)
{
    struct rtp_payload_delegate_t* ctx;
    ctx = (struct rtp_payload_delegate_t*)decoder;
    return ctx->decoder->input(ctx->packer, packet, bytes);
}






static int s_max_packet_size = 1434; // from VLC

void rtp_packet_setsize(int bytes)
{
    s_max_packet_size = bytes < 564 ? 564 : bytes;
}

int rtp_packet_getsize()
{
    return s_max_packet_size;
}

static int rtp_payload_find(int payload, const char* encoding, struct rtp_payload_delegate_t* codec)
{
    assert(payload >= 0 && payload <= 127);
    if (payload >= 96 && encoding)
    {
        if (0 == strcasecmp(encoding, "H264"))
        {
            // H.264 video (MPEG-4 Part 10) (RFC 6184)
            codec->encoder = rtp_h264_encode();
            codec->decoder = rtp_h264_decode();
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }


    return 0;
}










