#pragma once
#include <cstdint>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavdevice/avdevice.h>
}

#include <iostream>
#include <string>
#include <cstdio>
#include <jpeglib.h>


namespace frame {
  class Pix {
    public:
      Pix(const std::string path, int width, int height);
      void set_interval(int64_t interval);
      void start();
      ~Pix();
    private:
      int64_t interval_{0};
      void save_frame_as_jpeg();
      void capture_frames_from_rtsp();
      std::string path_;
      int width_;
      int heigh_;
      int frame_number{0};
      AVFormatContext* format_context{nullptr};
      AVCodec* codec{nullptr};                                        
      AVFrame* yuv_frame{nullptr};
      int video_stream{-1};
      AVCodecContext* codec_context{nullptr};
      AVCodecParameters* codec_params{nullptr};
  };
}
 

