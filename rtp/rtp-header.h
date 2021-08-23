#ifndef RTPHEADER_H
#define RTPHEADER_H

#include <stdint.h>

typedef struct _rtp_header_t {
    uint32_t v:2;
    uint32_t p:1;
    uint32_t x:1;
    uint32_t cc:4;
    uint32_t m:1;
    uint32_t pt:7;
    uint32_t seq:16;
    uint32_t timestamp;
    uint32_t ssrc;
}rtp_header_t;


#define RTP_V(v)	((v >> 30) & 0x03) /* protocol version */
#define RTP_P(v)	((v >> 29) & 0x01) /* padding flag */
#define RTP_X(v)	((v >> 28) & 0x01) /* header extension flag */
#define RTP_CC(v)	((v >> 24) & 0x0F) /* CSRC count */
#define RTP_M(v)	((v >> 23) & 0x01) /* marker bit */
#define RTP_PT(v)	((v >> 16) & 0x7F) /* payload type */
#define RTP_SEQ(v)	((v >> 00) & 0xFFFF) /* sequence number */



#endif // RTPHEADER_H
