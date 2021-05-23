#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}


typedef struct {
    const char *filename;
    AVSampleFormat sampleFmt;
    int sampleRate;
    int chLayout;
}ResampleAudioSpec;

class FFmpegs
{
public:
    FFmpegs();
    static void resampleAudio(ResampleAudioSpec &in,ResampleAudioSpec &out);
    static void resampleAudio(const char *inFilename,int inSampleRate,AVSampleFormat inSampleFmt,int inChLayout,const char * outFilename,int outSampleRate,AVSampleFormat outSampleFmt,int outChLayout);
};

#endif // FFMPEGS_H
