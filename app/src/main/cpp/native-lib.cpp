#include <jni.h>
#include <string>
#include "FFmpegMusic.h"
#include "FFmpegVideo.h"
#include <android/native_window_jni.h>

#include <android/native_window_jni.h>
#include <unistd.h>

extern "C"
{
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
//像素处理
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>

#include <unistd.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}

const char *inputPath;
int64_t *totalTime;
FFmpegVideo *ffmpegVideo;
FFmpegMusic *ffmpegMusic;
JavaCall *javaCall;
pthread_t p_tid;
int isPlay;
ANativeWindow *window = 0;
int64_t duration;
AVFormatContext *pFormatCtx;
AVPacket *packet;
int step = 0;
jboolean isSeek = false;
_JavaVM *javaVM = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_videoStreamPlay(JNIEnv *env, jobject instance,
                                                      jstring videoPath, jobject surface) {
    const char *input = env->GetStringUTFChars(videoPath, NULL);
    if (input == NULL) {
        LOGD("字符串转换失败......");
        return;
    }
    //注册FFmpeg所有编解码器，以及相关协议。
    av_register_all();
    //分配结构体
    AVFormatContext *formatContext = avformat_alloc_context();
    //打开视频数据源。由于Android 对SDK存储权限的原因，如果没有为当前项目赋予SDK存储权限，打开本地视频文件时会失败
    int open_state = avformat_open_input(&formatContext, input, NULL, NULL);
    if (open_state < 0) {
        char errbuf[128];
        if (av_strerror(open_state, errbuf, sizeof(errbuf)) == 0) {
            LOGD("打开视频输入流信息失败，失败原因： %s", errbuf);
        }
        return;
    }
    //为分配的AVFormatContext 结构体中填充数据
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGD("读取输入的视频流信息失败。");
        return;
    }
    int video_stream_index = -1;//记录视频流所在数组下标
    LOGD("当前视频数据，包含的数据流数量：%d", formatContext->nb_streams);
    //找到"视频流".AVFormatContext 结构体中的nb_streams字段存储的就是当前视频文件中所包含的总数据流数量——
    //视频流，音频流，字幕流
    for (int i = 0; i < formatContext->nb_streams; i++) {

        //如果是数据流的编码格式为AVMEDIA_TYPE_VIDEO——视频流。
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;//记录视频流下标
            break;
        }
    }
    if (video_stream_index == -1) {
        LOGD("没有找到 视频流。");
        return;
    }
    //通过编解码器的id——codec_id 获取对应（视频）流解码器
    AVCodecParameters *codecParameters = formatContext->streams[video_stream_index]->codecpar;
    AVCodec *videoDecoder = avcodec_find_decoder(codecParameters->codec_id);

    if (videoDecoder == NULL) {
        LOGD("未找到对应的流解码器。");
        return;
    }
    //通过解码器分配(并用  默认值   初始化)一个解码器context
    AVCodecContext *codecContext = avcodec_alloc_context3(videoDecoder);

    if (codecContext == NULL) {
        LOGD("分配 解码器上下文失败。");
        return;
    }
    //更具指定的编码器值填充编码器上下文
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        LOGD("填充编解码器上下文失败。");
        return;
    }
    //通过所给的编解码器初始化编解码器上下文
    if (avcodec_open2(codecContext, videoDecoder, NULL) < 0) {
        LOGD("初始化 解码器上下文失败。");
        return;
    }
    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;
    //分配存储压缩数据的结构体对象AVPacket
    //如果是视频流，AVPacket会包含一帧的压缩数据。
    //但如果是音频则可能会包含多帧的压缩数据
    AVPacket *packet = av_packet_alloc();
    //分配解码后的每一数据信息的结构体（指针）
    AVFrame *frame = av_frame_alloc();
    //分配最终显示出来的目标帧信息的结构体（指针）
    AVFrame *outFrame = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            (size_t) av_image_get_buffer_size(dstFormat, codecContext->width, codecContext->height,
                                              1));
    //更具指定的数据初始化/填充缓冲区
    av_image_fill_arrays(outFrame->data, outFrame->linesize, out_buffer, dstFormat,
                         codecContext->width, codecContext->height, 1);
    //初始化SwsContext
    SwsContext *swsContext = sws_getContext(
            codecContext->width   //原图片的宽
            , codecContext->height  //源图高
            , codecContext->pix_fmt //源图片format
            , codecContext->width  //目标图的宽
            , codecContext->height  //目标图的高
            , dstFormat, SWS_BICUBIC, NULL, NULL, NULL
    );
    if (swsContext == NULL) {
        LOGD("swsContext==NULL");
        return;
    }
    //Android 原生绘制工具
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    //定义绘图缓冲区
    ANativeWindow_Buffer outBuffer;
    //通过设置宽高限制缓冲区中的像素数量，而非屏幕的物流显示尺寸。
    //如果缓冲区与物理屏幕的显示尺寸不相符，则实际显示可能会是拉伸，或者被压缩的图像
    ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width, codecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);
    //循环读取数据流的下一帧
    while (av_read_frame(formatContext, packet) == 0) {

        if (packet->stream_index == video_stream_index) {
            //讲原始数据发送到解码器
            int sendPacketState = avcodec_send_packet(codecContext, packet);
            if (sendPacketState == 0) {
                int receiveFrameState = avcodec_receive_frame(codecContext, frame);
                if (receiveFrameState == 0) {
                    //锁定窗口绘图界面
                    ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
                    //对输出图像进行色彩，分辨率缩放，滤波处理
                    sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
                              frame->height, outFrame->data, outFrame->linesize);
                    uint8_t *dst = (uint8_t *) outBuffer.bits;
                    //解码后的像素数据首地址
                    //这里由于使用的是RGBA格式，所以解码图像数据只保存在data[0]中。但如果是YUV就会有data[0]
                    //data[1],data[2]
                    uint8_t *src = outFrame->data[0];
                    //获取一行字节数
                    int oneLineByte = outBuffer.stride * 4;
                    //复制一行内存的实际数量
                    int srcStride = outFrame->linesize[0];
                    for (int i = 0; i < codecContext->height; i++) {
                        memcpy(dst + i * oneLineByte, src + i * srcStride, srcStride);
                    }
                    //解锁
                    ANativeWindow_unlockAndPost(nativeWindow);
                    //进行短暂休眠。如果休眠时间太长会导致播放的每帧画面有延迟感，如果短会有加速播放的感觉。
                    //一般一每秒60帧——16毫秒一帧的时间进行休眠
                    usleep(1000 * 20);//20毫秒

                } else if (receiveFrameState == AVERROR(EAGAIN)) {
                    LOGD("从解码器-接收-数据失败：AVERROR(EAGAIN)");
                } else if (receiveFrameState == AVERROR_EOF) {
                    LOGD("从解码器-接收-数据失败：AVERROR_EOF");
                } else if (receiveFrameState == AVERROR(EINVAL)) {
                    LOGD("从解码器-接收-数据失败：AVERROR(EINVAL)");
                } else {
                    LOGD("从解码器-接收-数据失败：未知");
                }
            } else if (sendPacketState == AVERROR(EAGAIN)) {//发送数据被拒绝，必须尝试先读取数据
                LOGD("向解码器-发送-数据包失败：AVERROR(EAGAIN)");//解码器已经刷新数据但是没有新的数据包能发送给解码器
            } else if (sendPacketState == AVERROR_EOF) {
                LOGD("向解码器-发送-数据失败：AVERROR_EOF");
            } else if (sendPacketState == AVERROR(EINVAL)) {//遍解码器没有打开，或者当前是编码器，也或者需要刷新数据
                LOGD("向解码器-发送-数据失败：AVERROR(EINVAL)");
            } else if (sendPacketState == AVERROR(ENOMEM)) {//数据包无法压如解码器队列，也可能是解码器解码错误
                LOGD("向解码器-发送-数据失败：AVERROR(ENOMEM)");
            } else {
                LOGD("向解码器-发送-数据失败：未知");
            }
        }
        av_packet_unref(packet);
    }
    //内存释放
    ANativeWindow_release(nativeWindow);
    av_frame_free(&outFrame);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(videoPath, input);
}

void call_video_play(AVFrame *frame) {
    if (!window) {
        return;
    }
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        return;
    }

    LOGE("绘制 宽%d,高%d", frame->width, frame->height);
    LOGE("绘制 宽%d,高%d  行字节 %d ", window_buffer.width, window_buffer.height, frame->linesize[0]);
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int dstStride = window_buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}

void init() {
    LOGE("开启解码线程")
    //1.注册组件
    av_register_all();
    avformat_network_init();
    //封装格式上下文
    pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, inputPath, NULL, NULL) != 0) {
        LOGE("%s", "打开输入视频文件失败");
    }
    //3.获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
    }

    //得到播放总时间
    if (pFormatCtx->duration != AV_NOPTS_VALUE) {
        duration = pFormatCtx->duration;//微秒
    }
}

void seekTo(int mesc) {
    if (mesc <= 0) {
        mesc = 0;
    }
    //清空vector
    ffmpegMusic->queue.clear();
    ffmpegVideo->queue.clear();
    //跳帧
    /* if (av_seek_frame(pFormatCtx, -1,  mesc * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD) < 0) {
         LOGE("failed")
     } else {
         LOGE("success")
     }*/

    av_seek_frame(pFormatCtx, ffmpegVideo->index, (int64_t) (mesc / av_q2d(ffmpegVideo->time_base)),
                  AVSEEK_FLAG_BACKWARD);
    av_seek_frame(pFormatCtx, ffmpegMusic->index, (int64_t) (mesc / av_q2d(ffmpegMusic->time_base)),
                  AVSEEK_FLAG_BACKWARD);

}

void *begin(void *args) {
    //找到视频流和音频流
    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        //获取解码器
        AVCodecContext *avCodecContext = pFormatCtx->streams[i]->codec;
        AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);

        //copy一个解码器，
        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
        avcodec_copy_context(codecContext, avCodecContext);
        if (avcodec_open2(codecContext, avCodec, NULL) < 0) {
            LOGE("打开失败")
            continue;
        }
        //如果是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            ffmpegVideo->index = i;
            ffmpegVideo->setAvCodecContext(codecContext);
            ffmpegVideo->time_base = pFormatCtx->streams[i]->time_base;
            if (window) {
                ANativeWindow_setBuffersGeometry(window, ffmpegVideo->codec->width,
                                                 ffmpegVideo->codec->height,
                                                 WINDOW_FORMAT_RGBA_8888);
            }
        }//如果是音频流
        else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            ffmpegMusic->index = i;
            ffmpegMusic->setAvCodecContext(codecContext);
            ffmpegMusic->time_base = pFormatCtx->streams[i]->time_base;
        }
    }
    javaCall->onStart();
    //开启播放
    ffmpegVideo->setFFmepegMusic(ffmpegMusic);
    ffmpegMusic->play();
    ffmpegVideo->play();
    isPlay = 1;
    //seekTo(0);
    //解码packet,并压入队列中
    packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //跳转到某一个特定的帧上面播放
    int ret;
    while (isPlay) {
        //
        ret = av_read_frame(pFormatCtx, packet);
        if (ret == 0) {
            if (ffmpegVideo && ffmpegVideo->isPlay && packet->stream_index == ffmpegVideo->index
                    ) {
                //将视频packet压入队列
                ffmpegVideo->put(packet);
            } else if (ffmpegMusic && ffmpegMusic->isPlay &&
                       packet->stream_index == ffmpegMusic->index) {
                ffmpegMusic->put(packet);
            }
            av_packet_unref(packet);
        } else if (ret == AVERROR_EOF) {
            // 读完了
            //读取完毕 但是不一定播放完毕
            while (isPlay) {
                if (ffmpegVideo->queue.empty() && ffmpegMusic->queue.empty()) {
                    break;
                }
//                LOGE("等待播放完成");
                av_usleep(10000);
            }
        }
    }
    //解码完过后可能还没有播放完
    isPlay = 0;
    if (ffmpegMusic && ffmpegMusic->isPlay) {
        ffmpegMusic->stop();
    }
    if (ffmpegVideo && ffmpegVideo->isPlay) {
        ffmpegVideo->stop();
    }
    //释放
    av_free_packet(packet);
    avformat_free_context(pFormatCtx);
    pthread_exit(0);
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        if (LOG_SHOW) {
            LOGE("GetEnv failed!");
        }
        return result;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_play(JNIEnv *env, jobject instance, jstring inputPath_) {
    inputPath = env->GetStringUTFChars(inputPath_, 0);
    init();
    ffmpegVideo = new FFmpegVideo;
    ffmpegMusic = new FFmpegMusic;
    javaCall = new JavaCall(javaVM, env, &instance);
    ffmpegVideo->setPlayCall(call_video_play);
    ffmpegVideo->setJavaCall(javaCall);
    ffmpegMusic->setJavaCall(javaCall);
    pthread_create(&p_tid, NULL, begin, NULL);//开启begin线程
    env->ReleaseStringUTFChars(inputPath_, inputPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_display(JNIEnv *env, jobject instance, jobject surface) {
    //得到界面
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
    if (ffmpegVideo && ffmpegVideo->codec) {
        ANativeWindow_setBuffersGeometry(window, ffmpegVideo->codec->width,
                                         ffmpegVideo->codec->height,
                                         WINDOW_FORMAT_RGBA_8888);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_release(JNIEnv *env, jobject instance) {
    //释放资源
    if (isPlay) {
        isPlay = 0;
        pthread_join(p_tid, 0);
    }
    if (ffmpegVideo) {
        if (ffmpegVideo->isPlay) {
            ffmpegVideo->stop();
        }
        delete (ffmpegVideo);
        ffmpegVideo = 0;
    }
    if (ffmpegMusic) {
        if (ffmpegMusic->isPlay) {
            ffmpegMusic->stop();
        }
        delete (ffmpegMusic);
        ffmpegMusic = 0;
    }
    javaCall->onRelease();
}

extern "C"
JNIEXPORT void JNICALL Java_com_custom_ffmpegdemo_FFMpegUtil_stop(JNIEnv *env, jobject instance) {
    //点击暂停按钮
    ffmpegMusic->pause();
    ffmpegVideo->pause();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_getTotalTime(JNIEnv *env, jobject instance) {
    //获取视频总时间
    return (jint) duration;
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_getCurrentPosition(JNIEnv *env, jobject instance) {
    //获取音频播放时间
    return ffmpegMusic->clock;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_seekTo(JNIEnv *evn, jobject instance, jint msec) {
    seekTo(msec / 1000);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_custom_ffmpegdemo_FFMpegUtil_stepBack(JNIEnv *env, jobject instance) {

}

extern "C"
JNIEXPORT void JNICALL Java_com_custom_ffmpegdemo_FFMpegUtil_stepUp(JNIEnv *env, jobject instance) {
    //点击快进按钮
    seekTo(5);
}



