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


private:
    typedef struct {
        int sampleRate;
        AVSampleFormat sampleFmt;
        int chLayout;
        int chs;
        int bytesPerSampleFrame;
    }AudioSwrSpec;


};

#endif // VIDEOPLAYER_H
