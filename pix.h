#pragma once
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include <iostream>
#include <string>
#include <cstdio>
#include <jpeglib.h>


namespace frame {
  class Pix {
    public:
      Pix(const std::string path, int width, int height);
      void start();
      ~Pix() = default;
    private:
      void save_frame_as_jpeg(AVFrame* yuv_frame);
      void capture_frames_from_rtsp();
      std::string path_;
      int width_;
      int heigh_;
      int frame_number;
  };
}
