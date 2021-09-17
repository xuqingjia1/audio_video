#include "audiomixer.h"

AudioMixer::AudioMixer(): initialized(false),filter_graph_(nullptr),audio_output_info_(nullptr)
{
    audio_mix_info_.reset(new AudioInfo);
    audio_mix_info_->name = "amix";     // 混音用的

    audio_sink_info_.reset(new AudioInfo);
    audio_sink_info_->name = "sink";    // 输出
}

AudioMixer::~AudioMixer()
{
    if(initialized_) {
        exit();
    }
}




