#include "playthread.h"
#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

#define FILENAME "/Users/xuqingjia/code/video/audio_video/sdl_play_pcm/in.pcm"
#define SAMPLE_RATE 44100
#define SAMPLE_FORMAT AUDIO_S16LSB
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
#define CHANNELS 2
#define SAMPLES 1024
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3)
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

typedef struct {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;


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

void pull_audio_data (void *userdata,Uint8 *stream,int len) {
    qDebug() << "pull_audio_data" << len;
    SDL_memset(stream,0,len);
    AudioBuffer *buffer = (AudioBuffer *)userdata;
    if(buffer->len <= 0) {
        return;
    }
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;
    SDL_MixAudio(stream,buffer->data,buffer->pullLen,SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void PlayThread::run() {
    if(SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }
    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = SAMPLE_FORMAT;
    spec.channels = CHANNELS;
    spec.samples = SAMPLES;
    spec.callback = pull_audio_data;
    AudioBuffer buffer;
    spec.userdata = &buffer;
    if(SDL_OpenAudio(&spec,nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        SDL_Quit();
        return;
    }
    QFile file(FILENAME);
    if(!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
        SDL_CloseAudio();
        SDL_Quit();
        return;
    }

    SDL_PauseAudio(0);
    Uint8 data[BUFFER_SIZE];
    while(!isInterruptionRequested()) {
        if(buffer.len > 0) continue;
        buffer.len = file.read((char *)data,BUFFER_SIZE);
        if(buffer.len <= 0) {
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
            break;
        }
        buffer.data = data;
    }
    file.close();
    SDL_CloseAudio();
    SDL_Quit();
}



