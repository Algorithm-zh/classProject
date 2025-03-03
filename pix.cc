#include "pix.h"
#include <libavformat/avformat.h>
using namespace frame;



Pix::Pix(const std::string path, int width, int heigh)  {
  path_ = path;
  width_ = width;
  heigh_ = heigh;
  frame_number = 0;
}

void Pix::start() {
  capture_frames_from_rtsp();
}

// 保存帧为JPEG格式
void Pix::save_frame_as_jpeg(AVFrame* yuv_frame) {
    // 为保存的图像分配缓冲区
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width_, heigh_, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(num_bytes * sizeof(uint8_t));

    // 创建rgb图像帧
    AVFrame* rgb_frame = av_frame_alloc();
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, width_, heigh_, 1);

    struct SwsContext* sws_ctx = sws_getContext(width_, heigh_, (AVPixelFormat)yuv_frame->format, 
                                                width_, heigh_, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
    
    sws_scale(sws_ctx, yuv_frame->data, yuv_frame->linesize, 0, heigh_, rgb_frame->data, rgb_frame->linesize);

    // 设置输出文件名
    char filename[1024];
    snprintf(filename, sizeof(filename), "../image/frame%03d.jpg", frame_number);

    // 打开JPEG文件
    FILE* outfile = fopen(filename, "wb");
    if (!outfile) {
        std::cerr << "无法打开文件保存：" << filename << std::endl;
        return;
    }

    // 创建JPEG压缩对象
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    // 设置JPEG压缩参数
    cinfo.image_width = width_;
    cinfo.image_height = heigh_;
    cinfo.input_components = 3;  
    cinfo.in_color_space = JCS_RGB;  

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 90, TRUE); // 设置质量为90

    // 开始压缩
    jpeg_start_compress(&cinfo, TRUE);

    // 写入每行扫描数据
    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPROW row_pointer = rgb_frame->data[0] + cinfo.next_scanline * rgb_frame->linesize[0];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    // 结束压缩并释放资源
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);

    std::cout << "保存帧: " << filename << std::endl;

    // 释放内存
    av_frame_free(&rgb_frame);
    av_freep(&buffer);
    sws_freeContext(sws_ctx);
}

// 从RTSP流捕获视频帧
void Pix::capture_frames_from_rtsp() {
    AVFormatContext* format_context = nullptr;

    // 打开RTSP流
    if (avformat_open_input(&format_context, path_.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "无法打开RTSP流" << std::endl;
        return;
    }

    // 查找流信息
    if (avformat_find_stream_info(format_context, nullptr) < 0) {
        std::cerr << "无法获取流信息" << std::endl;
        return;
    }
    
    AVCodec* codec;                                        
    int video_stream = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    
    if (video_stream == -1) {
        std::cerr << "未找到视频流" << std::endl;
        return;
    }

    // 获取视频流解码器上下文
    AVCodecParameters* codec_params = format_context->streams[video_stream]->codecpar;
    codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        std::cerr << "找不到解码器" << std::endl;
        return;
    }

    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_context, codec_params) < 0) {
        std::cerr << "无法复制编码参数到解码器上下文" << std::endl;
        return;
    }

    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        std::cerr << "无法打开解码器" << std::endl;
        return;
    }

    AVPacket packet;
    AVFrame* frame = av_frame_alloc();

    // 开始读取视频流并抽取帧
    while (av_read_frame(format_context, &packet) >= 0) {
        if (packet.stream_index == video_stream) {
            // 解码帧
            if (avcodec_send_packet(codec_context, &packet) < 0) {
                std::cerr << "解码包失败" << std::endl;
                break;
            }

            while (avcodec_receive_frame(codec_context, frame) == 0) {
                // 每隔一定时间抽取一帧并保存为JPEG格式
                save_frame_as_jpeg(frame);
                frame_number ++;
            }
        }
        av_packet_unref(&packet);
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // 清理资源
    av_frame_free(&frame);
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
}
 
