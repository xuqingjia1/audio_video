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

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt,int width,int height)
{
    AVFrame *picture;
    int ret;
    picture = av_frame_alloc();
    if(!picture) {
        return NULL;
    }
    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;
    ret = av_frame_get_buffer(picture,32);
    if (ret < 0)
    {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,uint64_t channel_layout,int sample_rate,int nb_samples) {
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame)
    {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if(nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0)
        {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }
    return frame;

}
static void open_audio(AVFormatContext *oc,AVCodec *codec,OutputStream *ost,AVDictionary *opt_org) {
    AVCodecContext *codec_ctx;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;
    av_dict_copy(&opt,opt_org,0);

    ret = avcodec_open2(codec_ctx,codec,&opt);



    av_dict_free(&opt);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
        exit(1);
    }

    ost->t = 0;
    ost->tincr = 2 * M_PI * 110.0 / codec_ctx->sample_rate;
    ost->tincr2 = 2 * M_PI * 110.0 / codec_ctx->sample_rate / codec_ctx->sample_rate;

    nb_samples = codec_ctx->frame_size;
    ost->frame     = alloc_audio_frame(codec_ctx->sample_fmt, codec_ctx->channel_layout,
                                       codec_ctx->sample_rate, nb_samples);
    // 分配送给信号生成PCM的帧, 并申请对应的buffer
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, codec_ctx->channel_layout,
                                       codec_ctx->sample_rate, nb_samples);
    ret = avcodec_parameters_from_context(ost->st->codecpar,codec_ctx);
    if (ret < 0)
    {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx)
    {
        fprintf(stderr, "Could not allocate resampler context\n");
        exit(1);
    }
    av_opt_set_int       (ost->swr_ctx, "in_channel_count",   codec_ctx->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     codec_ctx->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (ost->swr_ctx, "out_channel_count",  codec_ctx->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    codec_ctx->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     codec_ctx->sample_fmt,     0);
    if ((ret = swr_init(ost->swr_ctx)) < 0)
    {
        fprintf(stderr, "Failed to initialize the resampling context\n");
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


static void close_stream(AVFormatContext *oc,OutputStream *ost) {
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

static AVFrame *get_video_frame(OutputStream *ost)
{
    AVCodecContext *codec_ctx = ost->enc;
    if(av_compare_ts(ost->next_pts,codec_ctx->time_base,STREAM_DURATION,(AVRational){ 1, 1 }) >= 0) {
        return NULL;
    }

    if (av_frame_make_writable(ost->frame) < 0)
        exit(1);

    if(codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P) {
        if (!ost->sws_ctx)
        {
            ost->sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height,
                                          AV_PIX_FMT_YUV420P,
                                          codec_ctx->width, codec_ctx->height,
                                          codec_ctx->pix_fmt,
                                          SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr
                        ,
                        "Could not initialize the conversion context\n");
                exit(1);
            }
        }
        fill_yuv_image(ost->tmp_frame,ost->next_pts,codec_ctx->width,codec_ctx->height);
        sws_scale(ost->sws_ctx, (const uint8_t * const *) ost->tmp_frame->data,
                  ost->tmp_frame->linesize, 0, codec_ctx->height, ost->frame->data,
                  ost->frame->linesize);
    }
    else {
        fill_yuv_image(ost->frame, ost->next_pts, codec_ctx->width, codec_ctx->height);
    }


    ost->frame->pts = ost->next_pts++;
    return ost->frame;


}
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

static int write_frame(AVFormatContext *fmt_ctx,const AVRational *time_base,AVStream *st,AVPacket *pkt) {
    av_packet_rescale_ts(pkt,*time_base,st->time_base);
    pkt->stream_index = st->index;
    log_packet(fmt_ctx,pkt);
    return av_interleaved_write_frame(fmt_ctx,pkt);

}
static int write_video_frame(AVFormatContext *oc,OutputStream *ost) {
    int ret;
    AVCodecContext *codec_ctx;
    AVFrame *frame;
    int got_packet = 0;
    AVPacket *pkt = {0};
    codec_ctx = ost->enc;
    av_init_packet(&pkt);
    ret = avcodec_encode_video2(codec_ctx,&pkt,frame,&got_packet);
    if (ret < 0)
    {
        fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
        exit(1);
    }

    if(got_packet) {
        ret = write_frame(oc,&codec_ctx->time_base,ost->st,&pkt);
    }
    else {
        ret = 0;
    }

    if (ret < 0)
    {
        fprintf(stderr, "Error while writing video frame: %s\n", av_err2str(ret));
        exit(1);
    }

    // 这里之所以有两个判断条件
    // frame非NULL: 表示还在产生YUV数据帧
    // got_packet为1: 编码器还有缓存的帧
    return (frame || got_packet) ? 0 : 1;
}

static AVFrame *get_audio_frame(OutputStream *ost) {
    AVFrame *frame = ost->tmp_frame;
    int j,i,v;
    int16_t *q = (int16_t *)frame->data[0];
    if (av_compare_ts(ost->next_pts, ost->enc->time_base,
                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
        return NULL;

    for (j = 0; j <frame->nb_samples; j++)
    {
        v = (int)(sin(ost->t) * 10000);
        for (i = 0; i < ost->enc->channels; i++)
            *q++ = v;
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }

    frame->pts = ost->next_pts; // 使用samples作为计数 设置pts 0, nb_samples(1024) 2048
    ost->next_pts  += frame->nb_samples;    // 音频PTS使用采样samples叠加
    return frame;
}

static int write_audio_frame(AVFormatContext *oc,OutputStream *ost) {
    AVCodecContext *codec_ctx;
    AVPacket pkt = {0};
    AVFrame *frame;
    int ret;
    int got_packet;
    int dst_nb_samples;
    av_init_packet(&pkt);
    codec_ctx = ost->enc;
    if(frame) {
        dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, codec_ctx->sample_rate) + frame->nb_samples,
                                        codec_ctx->sample_rate, codec_ctx->sample_rate, AV_ROUND_UP);
        av_assert0(dst_nb_samples == frame->nb_samples);
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0)
            exit(1);
        ret = swr_convert(ost->swr_ctx,
                          ost->frame->data, dst_nb_samples,
                          (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0)
        {
            fprintf(stderr, "Error while converting\n");
            exit(1);
        }
        frame->pts = av_rescale_q(ost->samples_count, (AVRational){1, codec_ctx->sample_rate},
                                  codec_ctx->time_base);
        ost->samples_count += dst_nb_samples;
    }
    ret = avcodec_encode_audio2(codec_ctx, &pkt, frame, &got_packet);
    if (ret < 0)
    {
        fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
        exit(1);
    }

    if (got_packet)
    {
        ret = write_frame(oc, &codec_ctx->time_base, ost->st, &pkt);
        if (ret < 0)
        {
            fprintf(stderr, "Error while writing audio frame: %s\n",
                    av_err2str(ret));
            exit(1);
        }
    }
    // frame == NULL 读取不到frame（比如读完了5秒的frame）; got_packet == 0 没有帧了
    return (frame || got_packet) ? 0 : 1;

}


int main(int argc,char **argv)
{
    OutputStream video_st = {0};
    OutputStream audio_st = {0};
    const char *filename = "/Users/xuqingjia/code/video/audio_video/1.flv";
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *audio_codec,*video_codec;
    int ret;
    int have_video = 0,have_audio = 0;
    int encode_video = 0,encode_audio = 0;
    AVDictionary *opt = NULL;
    int i;
//    if (argc < 2)
//    {
//        printf("usage: %s output_file\n"
//               "API example program to output a media file with libavformat.\n"
//               "This program generates a synthetic audio and video stream, encodes and\n"
//               "muxes them into a file named output_file.\n"
//               "The output format is automatically guessed according to the file extension.\n"
//               "Raw images can also be output by using '%%d' in the filename.\n"
//               "\n", argv[0]);
//        return 1;
//    }

//    filename = argv[1];
//    for (i = 2; i+1 < argc; i+=2)
//    {
//        if (!strcmp(argv[i], "-flags") || !strcmp(argv[i], "-fflags"))
//            av_dict_set(&opt, argv[i]+1, argv[i+1], 0);
//    }

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

    if (have_video)
        open_video(oc, video_codec, &video_st, opt);

    if (have_audio)
        open_audio(oc, audio_codec, &audio_st, opt);

    if (!(fmt->flags & AVFMT_NOFILE))
    {
        // 打开对应的输出文件，没有则创建
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            fprintf(stderr, "Could not open '%s': %s\n", filename,
                    av_err2str(ret));
            return 1;
        }
    }

    ret = avformat_write_header(oc,&opt);
    if (ret < 0)
    {
        fprintf(stderr, "Error occurred when opening output file: %s\n",
                av_err2str(ret));
        return 1;
    }

    while(encode_audio || encode_video) {
        if(encode_video && (!encode_audio || av_compare_ts(video_st.next_pts,video_st.enc->time_base,audio_st.next_pts, audio_st.enc->time_base) <= 0)) {
            printf("\nwrite_video_frame\n");
            encode_video = !write_video_frame(oc, &video_st);
        }else {
            printf("\nwrite_audio_frame\n");
            encode_audio = !write_audio_frame(oc, &audio_st);
        }
    }

     av_write_trailer(oc);

     if (have_video)
         close_stream(oc, &video_st);
     if (have_audio)
         close_stream(oc, &audio_st);

     if (!(fmt->flags & AVFMT_NOFILE))
         /* Close the output file. */
         avio_closep(&oc->pb);

     avformat_free_context(oc);


    return 0;
}
