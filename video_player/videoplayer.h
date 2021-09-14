#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H


#include <QObject>
#include <list>
#include "condmutex.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#define ERROR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

#define CODE(func, code) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        code; \
    }

#define END(func) CODE(func, fataError(); return;)
#define RET(func) CODE(func, return ret;)
#define CONTINUE(func) CODE(func, continue;)
#define BREAK(func) CODE(func, break;)



class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    typedef enum{
        Stopped = 0,
        Playing,
        Paused
    }State;

    typedef enum {
        Min = 0,
        Max = 100
    }Volumn;

    typedef struct {
        int width;
        int height;
        AVPixelFormat pixFmt;
        int size;
    }VideoSwsSpec;

    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();


    void play();
    void pause();
    void stop();
    bool isPlaying();
    State getState();
    void setFileName(QString &filename);
    int getDuration();
    int getTime();
    void setTime(int seekTime);
    void setVolumn(int volumn);
    int getVolumn();
    void setMute(bool mute);
    bool isMute();



signals:
    void stateChanged(VideoPlayer *player);
    void timeChanged(VideoPlayer *player);
    void initFinished(VideoPlayer *player);
    void playFailed(VideoPlayer *player);
    void frameDecoded(VideoPlayer *player,uint8_t *data,VideoSwsSpec &spec);

private:
    typedef struct {
        int sampleRate;
        AVSampleFormat sampleFmt;
        int chLayout;
        int chs;
        int bytesPerSampleFrame;
    }AudioSwrSpec;

    AVCodecContext *_aDecodeCtx = nullptr;
    AVStream *_aStream = nullptr;
    std::list<AVPacket> _aPktList;
    CondMutex _aMutex;
    SwrContext *_aSwrCtx = nullptr;
    AudioSwrSpec _aSwrInSpec,_aSwrOutSpec;
    AVFrame *_aSwrInFrame = nullptr,*_aSwrOutFrame = nullptr;
    int _aSwrOutIdx = 0;
    int _aSwrOutSize = 0;
    double _aTime = 0;
    bool _aCanFree = false;
    int _aSeekTime = -1;
    bool _hasAudio = false;


    int initAudioInfo();
    int initSwr();
    int initSDL();
    int addAudioPkt(AVPacket &pkt);
    void clearAudioPktList();
    static void sdlAudioCallbackFunc(void *userdata,Uint8 *stream,int len);
    void sdlAudioCallback(Uint8 *stream,int len);
    int decodeAudio();


    AVCodecContext *_vDecodeCtx = nullptr;
    AVStream *_vStream = nullptr;
    AVFrame *_vSwsInFrame = nullptr, *_vSwsOutFrame = nullptr;
    SwsContext *_vSwsCtx = nullptr;
    VideoSwsSpec _vSwsOutSpec;
    std::list<AVPacket> _vPktList;
    CondMutex _vMutex;
    double _vTime = 0;
    bool _vCanFree = false;
    int _vSeekTime = -1;
    bool _hasVideo = false;

    int initVideoInfo();
    int initSws();
    void addVideoPkt(AVPacket &pkt);
    void clearVideoPktList();
    void decodeVideo();


    AVFormatContext *_fmtCtx = nullptr;
    bool _fmtCtxCanFree = false;
    int _volumn = Max;
    int _mute = false;
    State _state = Stopped;
    char _filename[512];
    int _seekTime = -1;
    int initDecoder(AVCodecContext **decodeCtx,AVStream **stream,AVMediaType type);
    void setState(State state);
    void readFile();
    void free();
    void freeAudio();
    void freeVideo();
    void fataError();







};

#endif // VIDEOPLAYER_H
