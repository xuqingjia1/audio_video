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

// 输入缓冲区的大小
#define IN_DATA_SIZE 20480
// 需要再次读取输入文件数据的阈值
#define REFILL_THRESH 4096

FFmpegs::FFmpegs() {

}

static int decode(AVCodecContext *ctx,AVPacket *pkt,AVFrame *frame,QFile &outFile) {

}

void FFmpegs::aacDecode(const char *inFilename, AudioDecodeSpec *out) {

}
