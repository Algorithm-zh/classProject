#include "pix.h"
using namespace frame;

int main() {
  Pix pix("rtsp://admin:admin@172.16.19.104:554/stream2", 640, 480);
  pix.start();
  return 0;
}
