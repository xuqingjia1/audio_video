#include "audiothread.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "ffmpegs.h"

extern "C" {
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
}

#ifdef Q_OS_WIN
    // 格式名称
    #define FMT_NAME "dshow"
    // 设备名称
    #define DEVICE_NAME "audio=线路输入 (3- 魅声T800)"
    // PCM文件名
    #define FILEPATH "F:/"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILEPATH "/Users/xuqingjia/code/video/audio_video"
#endif

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
void showSpec(AVFormatContext *ctx) {
    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频参数
    AVCodecParameters *params = stream->codecpar;
    // 声道数
    qDebug() << params->channels;
    // 采样率
    qDebug() << params->sample_rate;
    // 采样格式
    qDebug() << params->format;
    // 每一个样本的一个声道占用多少个字节
    qDebug() << av_get_bytes_per_sample((AVSampleFormat) params->format);
    // 编码ID（可以看出采样格式）
    qDebug() << params->codec_id;
    // 每一个样本的一个声道占用多少位
    qDebug() << av_get_bits_per_sample(params->codec_id);
}

void AudioThread::run() {
    qDebug() << this << "开始执行-------";

    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if(!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }
    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx,DEVICE_NAME,fmt,nullptr);
    if(ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "打开设备失败" << errbuf;
        return;
    }
    // 打印一下录音设备的参数信息
    // showSpec(ctx);

    // 文件名
    QString filename = FILEPATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    QString wavFilename = filename;
    filename += ".pcm";
    wavFilename += ".wav";
    QFile file(filename);

    // 打开文件
    // WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就会清空文件内容
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 数据包
    AVPacket pkt;
    while (!isInterruptionRequested()) {
        // 不断采集数据
        ret = av_read_frame(ctx, &pkt);

        if (ret == 0) { // 读取成功
            // 将数据写入文件
            file.write((const char *) pkt.data, pkt.size);
        } else if (ret == AVERROR(EAGAIN)) { // 资源临时不可用
            continue;
        } else { // 其他错误
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }
//    while (!_stop && av_read_frame(ctx, &pkt) == 0) {
//        // 将数据写入文件
//        file.write((const char *) pkt.data, pkt.size);
//    }

    // 释放资源
    // 关闭文件
    file.close();

    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频参数
    AVCodecParameters *params = stream->codecpar;

    // pcm转wav文件
    WAVHeader header;
    header.sampleRate = params->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(params->codec_id);
    header.numChannels = params->channels;
    if (params->codec_id >= AV_CODEC_ID_PCM_F32BE) {
        header.audioFormat = AUDIO_FORMAT_FLOAT;
    }
    FFmpegs::pcm2wav(header,
                     filename.toUtf8().data(),
                     wavFilename.toUtf8().data());

    // 关闭设备
    avformat_close_input(&ctx);

    qDebug() << this << "正常结束----------";

//    char wavHeader[44];
//    wavHeader[0] = 'R';
//    wavHeader[1] = 'I';
//    wavHeader[2] = 'F';
//    wavHeader[3] = 'F';

//    // 10 00 00 00
//    wavHeader[4] = 0x10;
//    wavHeader[5] = 0x00;
//    wavHeader[6] = 0x00;
//    wavHeader[7] = 0x00;
}

void AudioThread::setStop(bool stop) {
    _stop = stop;
}
