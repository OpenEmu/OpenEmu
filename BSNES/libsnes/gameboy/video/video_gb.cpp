#include <gameboy/gameboy.hpp>

#define VIDEO_CPP
namespace GameBoy {

Video video;

unsigned Video::palette_dmg(unsigned color) const {
  unsigned R = monochrome[color][0] * 1023.0;
  unsigned G = monochrome[color][1] * 1023.0;
  unsigned B = monochrome[color][2] * 1023.0;

  return (R << 20) + (G << 10) + (B << 0);
}

unsigned Video::palette_sgb(unsigned color) const {
  unsigned R = (3 - color) * 341;
  unsigned G = (3 - color) * 341;
  unsigned B = (3 - color) * 341;

  return (R << 20) + (G << 10) + (B << 0);
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

  return (R << 20) + (G << 10) + (B << 0);
}

void Video::generate(Format format) {
  if(system.dmg()) for(unsigned n = 0; n < 4; n++) palette[n] = palette_dmg(n);
  if(system.sgb()) for(unsigned n = 0; n < 4; n++) palette[n] = palette_sgb(n);
  if(system.cgb()) for(unsigned n = 0; n < (1 << 15); n++) palette[n] = palette_cgb(n);

  if(format.i == Format::RGB24) {
    for(unsigned n = 0; n < (1 << 15); n++) {
      unsigned color = palette[n];
      palette[n] = ((color >> 6) & 0xff0000) + ((color >> 4) & 0x00ff00) + ((color >> 2) & 0x0000ff);
    }
  }

  if(format.i == Format::RGB16) {
    for(unsigned n = 0; n < (1 << 15); n++) {
      unsigned color = palette[n];
      palette[n] = ((color >> 14) & 0xf800) + ((color >> 9) & 0x07e0) + ((color >> 5) & 0x001f);
    }
  }

  if(format.i == Format::RGB15) {
    for(unsigned n = 0; n < (1 << 15); n++) {
      unsigned color = palette[n];
      palette[n] = ((color >> 15) & 0x7c00) + ((color >> 10) & 0x03e0) + ((color >> 5) & 0x001f);
    }
  }
}

Video::Video() {
  palette = new unsigned[1 << 15];
}

Video::~Video() {
  delete[] palette;
}

const double Video::monochrome[4][3] = {
  { 0.605, 0.734, 0.059 },
  { 0.543, 0.672, 0.059 },
  { 0.188, 0.383, 0.188 },
  { 0.059, 0.219, 0.059 },
};

}
