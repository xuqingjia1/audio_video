#ifndef RTPPROFILE_H
#define RTPPROFILE_H

struct rtp_profile_t{
    int payload;
    int avtype;
    int channels;
    int frequency;
    char name[32];
};

enum
{
    RTP_PAYLOAD_PCMU		= 0,  // ITU-T G.711 PCM µ-Law audio 64 kbit/s (rfc3551)
    RTP_PAYLOAD_PCMA		= 8,  // ITU-T G.711 PCM A-Law audio 64 kbit/s (rfc3551)
    RTP_PAYLOAD_G722		= 9,  // ITU-T G.722 audio 64 kbit/s (rfc3551)
    RTP_PAYLOAD_G729		= 18, // ITU-T G.729 and G.729a audio 8 kbit/s (rfc3551)
    RTP_PAYLOAD_MP3			= 14, // MPEG-1/MPEG-2 audio (rfc2250)

    RTP_PAYLOAD_JPEG		= 26, // JPEG video (rfc2435)
    RTP_PAYLOAD_MPV			= 32, // MPEG-1 and MPEG-2 video (rfc2250)
    RTP_PAYLOAD_MP2T		= 33, // MPEG-2 transport stream (rfc2250)
    RTP_PAYLOAD_H263		= 34, // H.263 video, first version (1996) (rfc2190)

    RTP_PAYLOAD_MP4V		= 96, // MP4V-ES MPEG-4 Visual (rfc6416)
    RTP_PAYLOAD_H264		= 97, // H.264 video (MPEG-4 Part 10) (rfc6184)
    RTP_PAYLOAD_H265		= 98, // H.265 video (MPEG-H Part 2) (rfc7798)
    RTP_PAYLOAD_MP2P		= 99, // MPEG-2 Program Streams video (rfc2250)
    RTP_PAYLOAD_MP4A		= 100, // MP4A-LATM MPEG-4 Audio (rfc6416)
    RTP_PAYLOAD_OPUS		= 101, // RTP Payload Format for the Opus Speech and Audio Codec (rfc7587)
    RTP_PAYLOAD_MP4ES		= 102, // MPEG4-generic audio/video MPEG-4 Elementary Streams (rfc3640)
    RTP_PAYLOAD_VP8			= 103,
    RTP_PAYLOAD_VP9			= 104,
};

#endif // RTPPROFILE_H
