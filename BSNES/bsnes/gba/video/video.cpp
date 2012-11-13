#include <gba/gba.hpp>

namespace GameBoyAdvance {

Video video;

void Video::generate_palette() {
  for(unsigned color = 0; color < (1 << 15); color++) {
    uint5 b = color >> 10;
    uint5 g = color >>  5;
    uint5 r = color >>  0;

    uint16 R = r << 11 | r << 6 | r << 1 | r >> 4;
    uint16 G = g << 11 | g << 6 | g << 1 | g >> 4;
    uint16 B = b << 11 | b << 6 | b << 1 | b >> 4;

    palette[color] = interface->videoColor(color, R, G, B);
  }
}

Video::Video() {
  palette = new uint32[1 << 15]();
}

Video::~Video() {
  delete[] palette;
}

}
