CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(pix)

include_directories(.)
SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(pix main.cc pix.cc)
TARGET_LINK_LIBRARIES(pix avformat avcodec avutil swscale jpeg)
