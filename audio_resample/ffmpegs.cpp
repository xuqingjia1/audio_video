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

}
