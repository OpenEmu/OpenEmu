#include <gb/gb.hpp>

#define VIDEO_CPP
namespace GameBoy {

Video video;

void Video::generate_palette() {
  if(system.dmg()) for(unsigned n = 0; n < 4; n++) palette[n] = palette_dmg(n);
  if(system.sgb()) for(unsigned n = 0; n < 4; n++) palette[n] = palette_sgb(n);
  if(system.cgb()) for(unsigned n = 0; n < (1 << 15); n++) palette[n] = palette_cgb(n);
}

Video::Video() {
  palette = new unsigned[1 << 15]();
}

Video::~Video() {
  delete[] palette;
}

unsigned Video::palette_dmg(unsigned color) const {
  unsigned R = monochrome[color][0] * 65535.0;
  unsigned G = monochrome[color][1] * 65535.0;
  unsigned B = monochrome[color][2] * 65535.0;

  return interface->videoColor(color, R, G, B);
}

unsigned Video::palette_sgb(unsigned color) const {
  unsigned R = (3 - color) * 21845;
  unsigned G = (3 - color) * 21845;
  unsigned B = (3 - color) * 21845;

  return interface->videoColor(color, R, G, B);
}

unsigned Video::palette_cgb(unsigned color) const {
  unsigned r = (color >>  0) & 31;
  unsigned g = (color >>  5) & 31;
  unsigned b = (color >> 10) & 31;

  unsigned R = (r * 26 + g *  4 + b *  2);
  unsigned G = (         g * 24 + b *  8);
  unsigned B = (r *  6 + g *  4 + b * 22);

  R = min(960, R);
  G = min(960, G);
  B = min(960, B);

  R = R << 6 | R >> 4;
  G = G << 6 | G >> 4;
  B = B << 6 | B >> 4;

  return interface->videoColor(color, R, G, B);
}

const double Video::monochrome[4][3] = {
  { 0.605, 0.734, 0.059 },
  { 0.543, 0.672, 0.059 },
  { 0.188, 0.383, 0.188 },
  { 0.059, 0.219, 0.059 },
};

}
