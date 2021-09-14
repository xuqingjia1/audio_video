#include <stdio.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#define STREAM_DURATION 2.0
#define STREAM_FRAME_RATE 25
#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P
#define SCALE_FLAGS SWS_BICUBIC

typedef struct OutputStream{
    AVStream *st;
    AVCodecContext *enc;
    int64_t next_pts;
    int samples_count;
    AVFrame *frame;
    AVFrame *tmp_frame;
    float t,tincr,tincr2;
    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
}OutputStream;


static void add_stream(OutputStream *ost,AVFormatContext *oc,AVCodec **codec,enum AVCodecID codec_id) {
    AVCodecContext *codec_ctx;
    int i;
    *codec = avcodec_find_encoder(codec_id);
    if(!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    ost->st = avformat_new_stream(oc,NULL);
    if (!ost->st)
    {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    ost->st->id = oc->nb_streams - 1;
    codec_ctx = avcodec_alloc_context3(*codec);
    if(!codec_ctx) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }

    ost->enc = codec_ctx;
    switch((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        codec_ctx->codec_id = codec_id;
        codec_ctx->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        codec_ctx->bit_rate = 64000;
        codec_ctx->sample_rate = 44100;
        if((*codec)->supported_samplerates) {
            codec_ctx->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++)
            {
                if ((*codec)->supported_samplerates[i] == 44100)
                    codec_ctx->sample_rate = 44100;
            }
        }
        codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
        codec_ctx->channels = av_get_channel_layout_nb_channels(codec_ctx->channel_layout);
        if ((*codec)->channel_layouts)
        {
            codec_ctx->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        codec_ctx->channels        = av_get_channel_layout_nb_channels(codec_ctx->channel_layout);
        ost->st->time_base = (AVRational){1,codec_ctx->sample_rate};  //时间
        break;
      case AVMEDIA_TYPE_VIDEO:
        codec_ctx->codec_id = codec_id;
        codec_ctx->bit_rate = 400000;
        codec_ctx->width = 352;
        codec_ctx->height = 288;
        codec_ctx->max_b_frames = 1;
        ost->st->time_base = (AVRational){1,STREAM_FRAME_RATE}; //时间
        codec_ctx->time_base = ost->st->time_base; //为什么这里需要设置
        codec_ctx->gop_size = STREAM_FRAME_RATE;
        codec_ctx->pix_fmt = STREAM_PIX_FMT;
        break;
    default:
        break;
    }
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;    //
}
static AVFrame *alloc_frame(enum AVPixelFormat pix_fmt,int width,int height) {
    AVFrame *picture;
    int ret;
    picture = av_frame_alloc();
    if(!picture) {
        return NULL;
    }
    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;
    if (ret < 0)
    {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

}




static void open_video(AVFormatContext *oc,AVCodec *codec,OutputStream *ost,AVDictionary *opt_arg) {
    int ret ;
    AVCodecContext *codec_ctx = ost->enc;
    AVDictionary *opt = NULL;
    ret = avcodec_open2(codec_ctx,codec,&opt);
    av_dict_free(&opt);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
        exit(1);
    }
    ost->frame = alloc_picture(codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height);
    if (!ost->frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        // 编码器格式需要的数据不是 AV_PIX_FMT_YUV420P才需要 调用图像scale
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, codec_ctx->width, codec_ctx->height);
        if (!ost->tmp_frame)
        {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }


    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, codec_ctx);
    if (ret < 0)
    {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
}
/* Prepare a dummy image. */
static void fill_yuv_image(AVFrame *pict, int frame_index,
                           int width, int height)
{
    int x, y, i;

    i = frame_index;

    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++)
    {
        for (x = 0; x < width / 2; x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}

int main(int argc,char **argv)
{
    OutputStream video_st = {0};
    OutputStream audio_st = {0};
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *audio_codec,*video_codec;
    int ret;
    int have_video = 0,have_audio = 0;
    int encode_video = 0,encode_audio = 0;
    AVDictionary *opt = NULL;
    int i;
    if (argc < 2)
    {
        printf("usage: %s output_file\n"
               "API example program to output a media file with libavformat.\n"
               "This program generates a synthetic audio and video stream, encodes and\n"
               "muxes them into a file named output_file.\n"
               "The output format is automatically guessed according to the file extension.\n"
               "Raw images can also be output by using '%%d' in the filename.\n"
               "\n", argv[0]);
        return 1;
    }

    filename = argv[1];
    for (i = 2; i+1 < argc; i+=2)
    {
        if (!strcmp(argv[i], "-flags") || !strcmp(argv[i], "-fflags"))
            av_dict_set(&opt, argv[i]+1, argv[i+1], 0);
    }

    avformat_alloc_output_context2(&oc,NULL,NULL,filename);
    if(!oc) {
        // 如果不能根据文件后缀名找到合适的格式，那缺省使用flv格式
        printf("Could not deduce output format from file extension: using flv.\n");
        avformat_alloc_output_context2(&oc, NULL, "flv", filename);
    }
    if (!oc)
        return 1;
    fmt = oc->oformat;
    fmt->video_codec = AV_CODEC_ID_H264;
    fmt->audio_codec = AV_CODEC_ID_AAC;
    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {
        add_stream(&video_st, oc, &video_codec, fmt->video_codec);
        have_video = 1;
        encode_video = 1;
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE)
    {
        add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec);
        have_audio = 1;
        encode_audio = 1;
    }

    if(have_video) {

    }







    return 0;
}
