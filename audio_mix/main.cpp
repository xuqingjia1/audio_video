#include <iostream>
#include "audiomixer.h"

//ffmpeg -i buweishui_1m.mp3 -i huiguniang.mp3 -filter_complex amix=inputs=2:duration=longest:dropout_transition=3 out.mp3 -y

#define PCM1_FRAME_SIZE (4096*2)        // 需要和格式匹配 4*1024*2*
#define PCM2_FRAME_SIZE (4096)
#define PCM_OUT_FRAME_SIZE (40000)




using namespace std;

int main(int argc,char **argv)
{
    AudioMixer amix;
    // 输入流
    amix.addAudioInput(0, 48000, 2, 32, AV_SAMPLE_FMT_FLT); // 48000_2_f32le.pcm
    amix.addAudioInput(1, 48000, 2, 16, AV_SAMPLE_FMT_S16); // 48000_2_s16le.pcm
    // 输出流
    amix.addAudioOutput(96000, 2, 16, AV_SAMPLE_FMT_S16);

    // init之前，要先添加输入源和输出源
    if (amix.init("longest") < 0) {
        return -1;
    }

    int len1 = 0, len2 = 0;
    uint8_t buf1[PCM1_FRAME_SIZE];
    uint8_t buf2[PCM2_FRAME_SIZE];
    FILE *file1 = fopen("48000_2_f32le.pcm", "rb");
    if(!file1) {
        printf("fopen 48000_2_f32le.pcm failed\n");
        return -1;
    }
    FILE *file2 = fopen("48000_2_s16le.pcm", "rb");
    if(!file2) {
        printf("fopen 48000_2_s16le.pcm failed\n");
        return -1;
    }
    FILE* file_out = fopen("output.pcm", "wb");
    if(!file_out) {
        printf("fopen output.pcm failed\n");
        return -1;
    }

    uint8_t out_buf[PCM_OUT_FRAME_SIZE];
    uint32_t out_size = 0;
    int file1_finish = 0;
    int file2_finish = 0;
    while (1) {
        len1 = fread(buf1, 1, PCM1_FRAME_SIZE, file1);
        len2 = fread(buf2, 1, PCM2_FRAME_SIZE, file2);
        if (len1 > 0 || len2 > 0) {
            if (len1 > 0) {
                if(amix.addFrame(0, buf1, len1) < 0) {
                    printf("amix.addFrame(0, buf1, len1) failed\n");
                    break;
                }
            } else {
                if(file1_finish == 0) {
                    file1_finish = 1;
                    if(amix.addFrame(0, NULL, 0) < 0) {     // 空包冲刷，人家才知道你某一路某一数据
                        printf("amix.addFrame(0, buf1, len1) failed\n");
                    }
                }
            }

            if (len2 > 0)
            {
                if(amix.addFrame(1, buf2, len2) < 0) {   // 空包冲刷，人家才知道你某一路某一数据
                    printf("amix.addFrame(1, buf2, len2) failed\n");
                    break;
                }
            } else {
                if(file2_finish == 0) {
                    file2_finish = 1;
                    if(amix.addFrame(1, NULL, 0) < 0) {
                        printf("amix.addFrame(1, buf2, len2) failed\n");
                    }
                }
            }

            int ret = 0; //amix.getFrame(out_buf, 10240);

            while ((ret = amix.getFrame(out_buf, 10240)) >=0) {
                out_size += ret;
                if(out_size % (1024*1024) ==0)
                    printf("mix audio: %d, out_size:%u\n", ret, out_size);
                fwrite(out_buf, 1, ret, file_out);
            }
        }
        else
        {
            printf("two file finish\n");
            break;
        }
    }
    printf("end, out_size:%u\n", out_size);
    amix.exit();
    if(file_out)
        fclose(file_out);
    if(file1)
        fclose(file1);
    if(file2)
        fclose(file2);
    getchar();


    return 0;
}
