#include "playthread.h"
#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

#define FILENAME "/Users/mj/Desktop/in.wav"
typedef struct {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
}AudioBuffer;

PlayThread::PlayThread(QObject *parent) : QThread(parent) {
    connect(this, &PlayThread::finished,
            this, &PlayThread::deleteLater);

}

PlayThread::~PlayThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "析构了";
}

void pull_audio_data(void *userData,Uint8* stream,int len) {
    qDebug() << "pull_audio_data" << len;

    SDL_memset(stream,0,len);
    AudioBuffer *buffer = (AudioBuffer *)userData;
    if(buffer->len <= 0) continue;
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;
    SDL_MixAudio(stream,buffer->data,buffer->pullLen,SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void PlayThread::run() {
    // 初始化Audio子系统
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }
    SDL_AudioSpec spec;
    Uint8 *data = nullptr;
    Uint32 len = 0;
    if (!SDL_LoadWAV(FILENAME, &spec, &data, &len)) {
        qDebug() << "SDL_LoadWAV error" << SDL_GetError();
        // 清除所有的子系统
        SDL_Quit();
        return;
    }
    // 音频缓冲区的样本数量
    spec.samples = 1024;
    // 设置回调
    spec.callback = pull_audio_data;
    // 设置userdata
    AudioBuffer buffer;
    buffer.data = data;
    buffer.len = len;
    spec.userdata = &buffer;

    // 打开设备
    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        // 清除所有的子系统
        SDL_Quit();
        return;
    }

    // 开始播放（0是取消暂停）
    SDL_PauseAudio(0);


    // 计算一些参数
    int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
    int bytesPerSample = (sampleSize * spec.channels) >> 3;

    // 存放从文件中读取的数据
    while (!isInterruptionRequested()) {
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if (buffer.len > 0) continue;

        // 文件数据已经读取完毕
        if (buffer.len <= 0) {
            // 剩余的样本数量
            int samples = buffer.pullLen / bytesPerSample;
            int ms = samples * 1000 / spec.freq;
            SDL_Delay(ms);
            break;
        }
    }

    // 释放WAV文件数据
    SDL_FreeWAV(data);

    // 关闭设备
    SDL_CloseAudio();

    // 清除所有的子系统
    SDL_Quit();


}
