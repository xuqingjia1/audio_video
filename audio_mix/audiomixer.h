#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H


#include <map>
#include <mutex>
#include <cstdio>
#include <cstdint>
#include <string>
#include <memory>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}




class AudioMixer
{
public:
    AudioMixer();
    virtual ~AudioMixer();
    int addAudioInput(uint32_t index,uint32_t samplerate,uint32_t channels,uint32_t bitsPerSample,AVSampleFormat format);
    int addAudioOutput(const uint32_t samplerate,const uint32_t channels,const uint32_t bitsPerSample,const AVSampleFormat format);
    int init(const char *duration = "longest");
    int exit();
    int addFrame(uint32_t index,uint8_t *inBuf,uint32_t size);
    int getFrame(uint8_t *outBuf,uint32_t maxOutBufSize);
private:
    struct AudioInfo{
        AudioInfo()
        {
            filterCtx = nullptr;
        }
        uint32_t samplerate;
        uint32_t channels;
        uint32_t bitsPerSample;
        AVSampleFormat format;
        std::string name;

        AVFilterContext *filterCtx;
    };

    bool initialized_ = false;
    std::mutex mutex_;
    std::map<uint32_t, AudioInfo> audio_input_info_;
    std::shared_ptr<AudioInfo> audio_output_info_;
    std::shared_ptr<AudioInfo> audio_mix_info_;
    std::shared_ptr<AudioInfo> audio_sink_info_;

    AVFilterGraph *filter_graph_ = nullptr;
};

#endif // AUDIOMIXER_H
