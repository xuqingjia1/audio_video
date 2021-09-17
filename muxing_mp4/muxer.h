#ifndef MUXER_H
#define MUXER_H

#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}



class Muxer
{
public:
    Muxer();
    ~Muxer();
    int Init(const char *url);
    void DeInit();
    int AddStream(AVCodecContext *codec_ctx);
    int SendHeader();
    int SendPacket(AVPacket *packet);
    int SendTrailer();
    int Open();
    int GetAudioStreamIndex();
    int GetVideoStreamIndex();

private:
    AVFormatContext *fmt_ctx_ = NULL;
    std::string url_ = "";
    AVCodecContext *aud_codec_ctx_ = NULL;
    AVStream *aud_stream_ = NULL;
    AVCodecContext *vid_codec_ctx_ = NULL;
    AVStream *vid_stream_ = NULL;
    int audio_index_ = -1;
    int video_index_ = -1;
};

#endif // MUXER_H
