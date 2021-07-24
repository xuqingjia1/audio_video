#include "audiothread.h"
#include "ffmpegs.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}


AudioThread::AudioThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

void AudioThread::run() {
    VideoDecodeSpec out;
    out.filename = "/Users/xuqingjia/code/video/audio_video/decode.yuv";

    FFmpegs::h264Decode("/Users/xuqingjia/code/video/audio_video/out.h264", out);

    qDebug() << "width:"<< out.width << "height:" << out.height
             << "fps:" << out.fps << "fmt:"<< av_get_pix_fmt_name(out.pixFmt);
}
