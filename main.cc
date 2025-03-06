#include "pix.h"
using namespace frame;

int main() {
  Pix pix("/dev/video0", 640, 480);
  //pix.set_interval(5000);
  pix.start();
  return 0;
}

