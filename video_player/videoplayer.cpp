#include "videoplayer.h"
#include <thread>
#include <QDebug>

#define AUDIO_MAX_PKT_SIZE 1000
#define VIDEO_MAX_PKT_SIZE 500

#pragma mark - 构造、析构
VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent) {
    // 初始化Audio子系统
    if (SDL_Init(SDL_INIT_AUDIO)) {
        // 返回值不是0，就代表失败
        qDebug() << "SDL_Init error" << SDL_GetError();
        emit playFailed(this);
        return;
    }
}

VideoPlayer::~VideoPlayer() {
    // 不再对外发送消息
    disconnect();

    stop();

    SDL_Quit();
}

void VideoPlayer::play() {
    if(_state == Playing) return;
    if(_state == Stopped) {
        std::thread([this](){
            readFile();
        }).detach();
    }else {
        setState(Playing);
    }
}

void VideoPlayer::pause() {
    if(!_state != Playing) return;

    setState(Paused);
}



void VideoPlayer::stop() {
    if(_state == Stopped) return;
    _state = Stopped;
    free();
    emit stateChanged(this);
}

bool VideoPlayer::isPlaying() {
    return _state = Playing;
}

VideoPlayer::State VideoPlayer::getState() {
    return _state;
}

void VideoPlayer::setFileName(QString &filename) {
    const char *name = filename.toStdString().c_str();
    memcpy(_filename,name,strlen(name) + 1);
}

int VideoPlayer::getDuration() {
    return _fmtCtx ? round(_fmtCtx->duration * av_q2d(AV_TIME_BASE_Q)) : 0;
}

int VideoPlayer::getTime() {
    return round(_aTime);
}


void VideoPlayer::setTime(int seekTime) {
    _seekTime = seekTime;
}

void VideoPlayer::setVolumn(int volumn) {
    _volumn = volumn;
}

int VideoPlayer::getVolumn() {
    return _volumn;
}

void VideoPlayer::setMute(bool mute) {
    _mute = mute
}

bool VideoPlayer::isMute() {
    return _mute;
}


void VideoPlayer::setState(State state) {
    if (state == _state) return;

    _state = state;

    emit stateChanged(this);
}

void VideoPlayer::free() {
    while (_hasAudio && !_aCanFree);
    while (_hasVideo && !_vCanFree);
    while (!_fmtCtxCanFree);
    avformat_close_input(&_fmtCtx);
    _fmtCtxCanFree = false;
    _seekTime = -1;

    freeAudio();
    freeVideo();
}

void VideoPlayer::fataError() {
    // 配合stop能够调用成功
    _state = Playing;
    stop();
    emit playFailed(this);
}




int VideoPlayer::initDecoder(AVCodecContext **decodeCtx, AVStream **stream, AVMediaType type) {
    int ret = av_find_best_stream(_fmtCtx,type,-1,-1,nullptr,0);
    RET(av_find_best_stream);

    int streamIdx = ret;
    *stream = _fmtCtx->streams[streamIdx];
    if (!*stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    AVCodec *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decoder not found" << (*stream)->codecpar->codec_id;
        return -1;
    }
    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!decodeCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    ret = avcodec_parameters_to_context(*decodeCtx,(*stream)->codecpar);

    RET(avcodec_parameters_to_context);

    // 打开解码器
    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    RET(avcodec_open2);

    return 0;
}


void VideoPlayer::readFile() {
    int ret = 0;
    ret = avformat_open_input(&_fmtCtx,_filename,nullptr,nullptr);
    END(avformat_open_input);


    ret = avformat_find_stream_info(_fmtCtx,nullptr);
    END(avformat_find_stream_info);

    // 打印流信息到控制台
    av_dump_format(_fmtCtx, 0, _filename, 0);
    fflush(stderr);

    _hasAudio = initAudioInfo() >= 0;
    _hasVideo = initVideoInfo() >= 0;
    if (!_hasAudio && !_hasVideo) {
        fataError();
        return;
    }

    // 到此为止，初始化完毕
    emit initFinished(this);

    // 改变状态
    setState(Playing);

    // 音频解码子线程：开始工作
    SDL_PauseAudio(0);
    std::thread([this](){
        decodeVideo();
    }).detach();

    AVPacket pkt;
    while(_state != Stopped) {
        if(_seekTime >= 0) {
            int streamIdx;
            if(_hasAudio) {
                streamIdx = _aStream->index;
            }else {
                streamIdx = _vStream->index;
            }


            AVRational timeBase = _fmtCtx->streams[streamIdx]->time_base;
            int64_t ts = _seekTime / av_q2d(timeBase);

            ret = av_seek_frame(_fmtCtx,streamIdx,ts,AVSEEK_FLAG_BACKWARD);

            if(ret < 0) {
                qDebug() << "seek失败" << _seekTime << ts << streamIdx;
                _seekTime = -1;
            }else {
                qDebug() << "seek成功" << _seekTime << ts << streamIdx;
                _vSeekTime = _seekTime;
                _aSeekTime = _seekTime;
                _seekTime = -1;
                // 恢复时钟
                _aTime = 0;
                _vTime = 0;
                // 清空之前读取的数据包
                clearAudioPktList();
                clearVideoPktList();
            }
        }

        int vSize = _vPktList.size();
        int aSize = _aPktList.size();

        if(vSize >= VIDEO_MAX_PKT_SIZE || aSize >= AUDIO_MAX_PKT_SIZE)) {
            continue;
        }


        ret = av_read_frame(_fmtCtx,&pkt);
        if(ret == 0) {
            if (pkt.stream_index == _aStream->index) { // 读取到的是音频数据
                addAudioPkt(pkt);
            } else if (pkt.stream_index == _vStream->index) { // 读取到的是视频数据
                addVideoPkt(pkt);
            } else { // 如果不是音频、视频流，直接释放
                av_packet_unref(&pkt);
            }
        }else if(ret == AVERROR_EOF) {
            if (vSize == 0 && aSize == 0) {
                // 说明文件正常播放完毕
                _fmtCtxCanFree = true;
                break;
            }
        }else {
            ERROR_BUF;
            qDebug() << "av_read_frame error" << errbuf;
            continue;
        }



    }
    if(_fmtCtxCanFree) {
        stop();
    }else {
        _fmtCtxCanFree = true;

    }


}















