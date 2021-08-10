#include "videoplayer.h"
#include <QDebug>


int VideoPlayer::initAudioInfo() {
    int ret = initDecoder(&_aDecodeCtx,&_aStream,AVMEDIA_TYPE_AUDIO);
    RET(initDecoder);

    ret = initSwr();
    RET(initSwr);
    ret = initSDL();
    RET(initSDL);
    return 0;
}


int VideoPlayer::initSwr() {
    _aSwrInSpec.sampleFmt = _aDecodeCtx->sample_fmt;
    _aSwrInSpec.sampleRate = _aDecodeCtx->sample_rate;
    _aSwrInSpec.chLayout = _aDecodeCtx->channel_layout;
    _aSwrInSpec.chs = _aDecodeCtx->channels;


    _aSwrOutSpec.sampleFmt = AV_SAMPLE_FMT_S16;
    _aSwrOutSpec.sampleRate = 44100;
    _aSwrOutSpec.chLayout = AV_CH_LAYOUT_STEREO;
    _aSwrOutSpec.chs = av_get_channel_layout_nb_channels(_aSwrOutSpec.chLayout);
    _aSwrOutSpec.bytesPerSampleFrame = _aSwrOutSpec.chs * av_get_bytes_per_sample(_aSwrOutSpec.sampleFmt);


    _aSwrCtx = swr_alloc_set_opts(nullptr,_aSwrOutSpec.chLayout,_aSwrOutSpec.sampleFmt,_aSwrOutSpec.sampleRate,_aSwrInSpec.chLayout,_aSwrInSpec.sampleFmt,_aSwrInSpec.sampleRate,0,nullptr);

    if(!_aSwrCtx) {
        qDebug() << "swr_alloc_set_opts error";
        return -1;
    }

    int ret = swr_init(_aSwrCtx);

    RET(swr_init);

    _aSwrInFrame = av_frame_alloc();
    if(!_aSwrInFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }
    _aSwrOutFrame = av_frame_alloc();
    if (!_aSwrOutFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }
    ret = av_samples_alloc(_aSwrOutFrame->data,_aSwrOutFrame->linesize,_aSwrOutSpec.chs,4096,_aSwrOutSpec.sampleFmt,1);
    RET(av_samples_alloc);
    return 0;
}


int VideoPlayer::initSDL() {
    SDL_AudioSpec spec;
    spec.freq = _aSwrOutSpec.sampleRate;
    spec.format = AUDIO_S16LSB;
    spec.channels = _aSwrOutSpec.chs;
    spec.samples = 512;
    spec.callback = sdlAudioCallbackFunc;
    spec.userdata = this;
    // 打开音频设备
    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        return -1;
    }
    return 0;
}




void VideoPlayer::addAudioPkt(AVPacket &pkt) {
    _aMutex.lock();
    _aPktList.push_back(pkt);
    _aMutex.signal();
    _aMutex.unlock();
}

void VideoPlayer::clearAudioPktList() {
    _aMutex.lock();
    for(AVPacket &pkt : _aPktList) {
        av_packet_unref(&pkt);
    }

    _aPktList.clear();
    _aMutex.unlock();
}

void VideoPlayer::freeAudio() {
    _aTime = 0;
    _aSwrOutIdx = 0;
    _aSwrOutSize = 0;
    _aStream = nullptr;
    _aCanFree = false;
    _aSeekTime = -1;

    clearAudioPktList();
    avcodec_free_context(&_aDecodeCtx);
    swr_free(&_aSwrCtx);
    av_frame_free(&_aSwrInFrame);
    if (_aSwrOutFrame) {
        av_freep(&_aSwrOutFrame->data[0]);
        av_frame_free(&_aSwrOutFrame);
    }

    // 停止播放
    SDL_PauseAudio(1);
    SDL_CloseAudio();
}


void VideoPlayer::sdlAudioCallbackFunc(void *userdata, Uint8 *stream, int len) {
    VideoPlayer *player = (VideoPlayer *) userdata;
    player->sdlAudioCallback(stream,len);
}

void VideoPlayer::sdlAudioCallback(Uint8 *stream, int len) {
    SDL_memset(stream,0,len);
    while(len > 0) {
        if(_state == Paused) break;
        if(_state == Stopped) {
            _aCanFree = true;
            break;
        }

        if(_aSwrOutIdx >= _aSwrOutSize) {
            _aSwrOutSize = decodeAudio();
            _aSwrOutIdx = 0;
            if(_aSwrOutSize <= 0) {
                _aSwrOutSize = 1024;
                memset(_aSwrOutFrame->data[0],0,_aSwrOutSize);
            }
        }

        int fillLen = _aSwrOutSize - _aSwrOutIdx;
        fillLen = std::min(fillLen,len);
        int volumn = _mute ? 0 : *((_volumn * 1.0 / Max) * SDL_MIX_MAXVOLUME);

        SDL_MixAudio(stream,_aSwrOutFrame->data[0] + _aSwrOutIdx,fillLen,volumn);

        len -= fillLen;
        stream += fillLen;
        _aSwrOutIdx += fillLen;
    }
}


int VideoPlayer::decodeAudio() {
    _aMutex.lock();
    if(_aPktList.empty()) {
        _aMutex.unlock();
        return 0;
    }


    AVPacket pkt = _aPktList.front();
    _aPktList.pop_front();
    _aMutex.unlock();


    if(pkt.pts != AV_NOPTS_VALUE) {
        _aTime = av_q2d(_aStream->time_base) * pkt.pts;
        emit timeChanged(this);
    }

    if(!_aSeekTime >= 0) {
        if(_aTime < _aSeekTime) {
            av_packet_unref(&pkt);
            return 0;
        }else {
            _aSeekTime = -1;
        }
    }

    // 发送压缩数据到解码器
    int ret = avcodec_send_packet(_aDecodeCtx, &pkt);
    // 释放pkt
    av_packet_unref(&pkt);
    RET(avcodec_send_packet);

    // 获取解码后的数据
    ret = avcodec_receive_frame(_aDecodeCtx, _aSwrInFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 0;
    } else RET(avcodec_receive_frame);

    // 重采样输出的样本数
    int outSamples = av_rescale_rnd(_aSwrOutSpec.sampleRate,
                                    _aSwrInFrame->nb_samples,
                                    _aSwrInSpec.sampleRate, AV_ROUND_UP);

    // 由于解码出来的PCM。跟SDL要求的PCM格式可能不一致
    // 需要进行重采样
    ret = swr_convert(_aSwrCtx,
                      _aSwrOutFrame->data,
                      outSamples,
                      (const uint8_t **) _aSwrInFrame->data,
                      _aSwrInFrame->nb_samples);
    RET(swr_convert);

    return ret * _aSwrOutSpec.bytesPerSampleFrame;




}



