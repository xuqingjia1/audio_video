#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs() {

}

static int check_sample_fmt(const AVCodec *codec,enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;
    while(*p != AV_SAMPLE_FMT_NONE) {
        if(*p == sample_fmt) {
            return 1;
        }
        p++;
    }
    return 0;
}

static int encode(AVCodecContext *ctx,AVFrame *frame,AVPacket *pkt,QFile *outFile) {
    int ret = avcodec_send_frame(crx,frame);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }
    while(true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // 继续读取数据到frame，然后送到编码器
            return 0;
        } else if (ret < 0) { // 其他错误
            return ret;
        }

        // 成功从编码器拿到编码后的数据
        // 将编码后的数据写入文件
        outFile.write((char *) pkt->data, pkt->size);

        // 释放pkt内部的资源
        av_packet_unref(pkt);
    }
}

void FFmpegs::aacEncode(AudioEncodeSpec &in, const char *outFilename) {
    // 文件
    QFile inFile(in.filename);
    QFile outFile(outFilename);
    int ret = 0;
    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    codec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!codec) {
        qDebug() << "encoder not found";
        return;
    }
    if (!check_sample_fmt(codec, in.sampleFmt)) {
        qDebug() << "unsupported sample format"
                 << av_get_sample_fmt_name(in.sampleFmt);
        return;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    ctx->sample_rate = in.sampleRate;
    ctx->sample_fmt = in.sampleFmt;
    ctx->channel_layout = in.chLayout;
    ctx->bit_rate = 32000;
    ctx->profile = FF_PROFILE_AAC_HE_V2;

    ret = avcodec_open2(ctx,codec,nullptr) ;
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }
    frame->nb_samples = ctx->frame_size;
    frame->format = ctx->sample_fmt;
    frame->channel_layout = ctx->channel_layout;

    ret = av_frame_get_buffer(frame,0);
    if (ret) {
        ERROR_BUF(ret);
        qDebug() << "av_frame_get_buffer error" << errbuf;
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << outFilename;
        goto end;
    }

    while((ret = inFile.read((char *)frame->data[0],frame->linesize[0])) > 0) {
        if(ret < frame->linesize[0]) {
            int bytes = av_get_bytes_per_sample((AVSampleFormat) frame->format);
            int ch = av_get_channel_layout_nb_channels(frame->channel_layout);
            // 设置真正有效的样本帧数量
            // 防止编码器编码了一些冗余数据
            frame->nb_samples = ret / (bytes * ch);
        }

        // 进行编码
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }
    }

    encode(ctx, nullptr, pkt, outFile);


end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    qDebug() << "线程正常结束";
}