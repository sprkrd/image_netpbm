#include <iostream>
#include "netpbm.hpp"
using namespace std;

int main() {
  ash::img_t img(256,256);
  for (int x = 0; x < 256; ++x) {
    for (int y = 0; y < 256; ++y) {
      auto& pixel = img(x,y);
      pixel.r = x;
      pixel.g = (x+y)/2;
      pixel.b = y;
    }
  }
  img.save("gradient.ppm", false);

  img = ash::img_t::load("gradient.ppm");
  cout << img.width() << endl;
  cout << img.height() << endl;
  cout << img(0,0) << endl;
  cout << img(255,255) << endl;
}
