#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}
#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs()
{

}
void FFmpegs::resampleAudio(ResampleAudioSpec &in, ResampleAudioSpec &out) {
    resampleAudio(in.filename,in.sampleRate,in.sampleFmt,in.chLayout,out.filename,out.sampleRate,out.sampleFmt,out.chLayout);
}

void FFmpegs::resampleAudio(const char *inFilename, int inSampleRate, AVSampleFormat inSampleFmt, int inChLayout, const char *outFilename, int outSampleRate, AVSampleFormat outSampleFmt, int outChLayout) {
    QFile inFile(inFilename);
    QFile outFile(outFilename);
    uint8_t **inData = nullptr;
    int inLinesize = 0;
    int inChs = av_get_channel_layout_nb_channels(inChLayout);
    int inBytesPerSample = inChs * av_get_bytes_per_sample(inSampleFmt);
    int inSamples = 1024;
    int len = 0;

    uint8_t **outData = nullptr;
    int outLinesize = 0;
    int outChs = av_get_channel_layout_nb_channels(outChLayout);
    int outBytesPerSample = outChs * av_get_bytes_per_sample(outSampleFmt);
    int outSamples = av_rescale_rnd(outSampleRate,inSamples,inSampleRate,AV_ROUND_UP);
    qDebug() << "输入缓冲区" << inSampleRate << inSamples;
    qDebug() << "输出缓冲区" << outSampleRate << outSamples;
    int ret = 0;
    SwrContext *ctx = swr_alloc_set_opts(nullptr,outChLayout,outSampleFmt,outSampleRate,inChLayout,inSampleFmt,inSampleRate,0,nullptr);
    if (!ctx) {
        qDebug() << "swr_alloc_set_opts error";
        goto end;
    }
    ret = swr_init(ctx);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "swr_init error:" << errbuf;
        goto end;
    }

    ret = av_samples_alloc_array_and_samples(&inData,&inLinesize,inChs,inSamples,inSampleFmt,1);

    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
        goto end;
    }
    ret = av_samples_alloc_array_and_samples(&outData,&outLinesize,outChs,outSamples,outSampleFmt,1);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
        goto end;
    }
    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << inFilename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << outFilename;
        goto end;
    }
    while((len = inFile.read((char *)inData[0],inLinesize)) > 0) {
        inSamples = len / inBytesPerSample;
        ret = swr_convert(ctx,outData,outSamples,(const uint8_t **)inData,inSamples);
        if (ret < 0) {
            ERROR_BUF(ret);
            qDebug() << "swr_convert error:" << errbuf;
            goto end;
        }
        outFile.write((char *)outData[0],ret * outBytesPerSample);
    }
    while((ret = swr_convert(ctx,outData,outSamples,nullptr,0)) > 0) {
        outFile.write((char *)outData[0],ret * outBytesPerSample);
    }

end:
    // 释放资源
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放输入缓冲区
    if (inData) {
        av_freep(&inData[0]);
    }
    av_freep(&inData);

    // 释放输出缓冲区
    if (outData) {
        av_freep(&outData[0]);
    }
    av_freep(&outData);

    // 释放重采样上下文
    swr_free(&ctx);

//    void *ptr = malloc(100);
//    freep(&ptr);
//    free(ptr);
//    ptr = nullptr;
}
