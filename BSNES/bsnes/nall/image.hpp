#ifndef NALL_IMAGE_HPP
#define NALL_IMAGE_HPP

#include <nall/bmp.hpp>
#include <nall/filemap.hpp>
#include <nall/interpolation.hpp>
#include <nall/png.hpp>
#include <nall/stdint.hpp>
#include <algorithm>

namespace nall {

struct image {
  uint8_t *data;
  unsigned width;
  unsigned height;
  unsigned pitch;

  bool endian;  //0 = little, 1 = big
  unsigned depth;
  unsigned stride;

  struct Channel {
    uint64_t mask;
    unsigned depth;
    unsigned shift;
  } alpha, red, green, blue;

  typedef double (*interpolation)(double, double, double, double, double);
  static inline unsigned bitDepth(uint64_t color);
  static inline unsigned bitShift(uint64_t color);
  static inline uint64_t normalize(uint64_t color, unsigned sourceDepth, unsigned targetDepth);

  inline image& operator=(const image &source);
  inline image& operator=(image &&source);
  inline image(const image &source);
  inline image(image &&source);
  inline image(bool endian, unsigned depth, uint64_t alphaMask, uint64_t redMask, uint64_t greenMask, uint64_t blueMask);
  inline image();
  inline ~image();

  inline uint64_t read(const uint8_t *data) const;
  inline void write(uint8_t *data, uint64_t value) const;

  inline void free();
  inline void allocate(unsigned width, unsigned height);
  inline void clear(uint64_t color);
  inline bool load(const string &filename);
//inline bool loadBMP(const uint8_t *data, unsigned size);
  inline bool loadPNG(const uint8_t *data, unsigned size);
  inline void scale(unsigned width, unsigned height, interpolation op);
  inline void transform(bool endian, unsigned depth, uint64_t alphaMask, uint64_t redMask, uint64_t greenMask, uint64_t blueMask);
  inline void alphaBlend(uint64_t alphaColor);

protected:
  inline uint64_t interpolate(double mu, const uint64_t *s, interpolation op);
  inline void scaleX(unsigned width, interpolation op);
  inline void scaleY(unsigned height, interpolation op);
  inline bool loadBMP(const string &filename);
  inline bool loadPNG(const string &filename);
};

//static

unsigned image::bitDepth(uint64_t color) {
  unsigned depth = 0;
  if(color) while((color & 1) == 0) color >>= 1;
  while((color & 1) == 1) { color >>= 1; depth++; }
  return depth;
}

unsigned image::bitShift(uint64_t color) {
  unsigned shift = 0;
  if(color) while((color & 1) == 0) { color >>= 1; shift++; }
  return shift;
}

uint64_t image::normalize(uint64_t color, unsigned sourceDepth, unsigned targetDepth) {
  while(sourceDepth < targetDepth) {
    color = (color << sourceDepth) | color;
    sourceDepth += sourceDepth;
  }
  if(targetDepth < sourceDepth) color >>= (sourceDepth - targetDepth);
  return color;
}

//public

image& image::operator=(const image &source) {
  free();

  width = source.width;
  height = source.height;
  pitch = source.pitch;

  endian = source.endian;
  stride = source.stride;

  alpha = source.alpha;
  red = source.red;
  green = source.green;
  blue = source.blue;

  data = new uint8_t[width * height * stride];
  memcpy(data, source.data, width * height * stride);
  return *this;
}

image& image::operator=(image &&source) {
  width = source.width;
  height = source.height;
  pitch = source.pitch;

  endian = source.endian;
  stride = source.stride;

  alpha = source.alpha;
  red = source.red;
  green = source.green;
  blue = source.blue;

  data = source.data;
  source.data = nullptr;
  return *this;
}

image::image(const image &source) : data(nullptr) {
  operator=(source);
}

image::image(image &&source) : data(nullptr) {
  operator=(std::forward<image>(source));
}

image::image(bool endian, unsigned depth, uint64_t alphaMask, uint64_t redMask, uint64_t greenMask, uint64_t blueMask) : data(nullptr) {
  width = 0, height = 0, pitch = 0;

  this->endian = endian;
  this->depth = depth;
  this->stride = (depth / 8) + ((depth & 7) > 0);

  alpha.mask = alphaMask, red.mask = redMask, green.mask = greenMask, blue.mask = blueMask;
  alpha.depth = bitDepth(alpha.mask), alpha.shift = bitShift(alpha.mask);
  red.depth = bitDepth(red.mask), red.shift = bitShift(red.mask);
  green.depth = bitDepth(green.mask), green.shift = bitShift(green.mask);
  blue.depth = bitDepth(blue.mask), blue.shift = bitShift(blue.mask);
}

image::image() : data(nullptr) {
  width = 0, height = 0, pitch = 0;

  this->endian = 0;
  this->depth = 32;
  this->stride = 4;

  alpha.mask = 255u << 24, red.mask = 255u << 16, green.mask = 255u << 8, blue.mask = 255u << 0;
  alpha.depth = bitDepth(alpha.mask), alpha.shift = bitShift(alpha.mask);
  red.depth = bitDepth(red.mask), red.shift = bitShift(red.mask);
  green.depth = bitDepth(green.mask), green.shift = bitShift(green.mask);
  blue.depth = bitDepth(blue.mask), blue.shift = bitShift(blue.mask);
}

image::~image() {
  free();
}

uint64_t image::read(const uint8_t *data) const {
  uint64_t result = 0;
  if(endian == 0) {
    for(signed n = stride - 1; n >= 0; n--) result = (result << 8) | data[n];
  } else {
    for(signed n = 0; n < stride; n++) result = (result << 8) | data[n];
  }
  return result;
}

void image::write(uint8_t *data, uint64_t value) const {
  if(endian == 0) {
    for(signed n = 0; n < stride; n++) { data[n] = value; value >>= 8; }
  } else {
    for(signed n = stride - 1; n >= 0; n--) { data[n] = value; value >>= 8; }
  }
}

void image::free() {
  if(data) delete[] data;
  data = nullptr;
}

void image::allocate(unsigned width, unsigned height) {
  if(data != nullptr && this->width == width && this->height == height) return;
  free();
  data = new uint8_t[width * height * stride]();
  pitch = width * stride;
  this->width = width;
  this->height = height;
}

void image::clear(uint64_t color) {
  uint8_t *dp = data;
  for(unsigned n = 0; n < width * height; n++) {
    write(dp, color);
    dp += stride;
  }
}

bool image::load(const string &filename) {
  if(loadBMP(filename) == true) return true;
  if(loadPNG(filename) == true) return true;
  return false;
}

void image::scale(unsigned outputWidth, unsigned outputHeight, interpolation op) {
  if(width != outputWidth) scaleX(outputWidth, op);
  if(height != outputHeight) scaleY(outputHeight, op);
}

void image::transform(bool outputEndian, unsigned outputDepth, uint64_t outputAlphaMask, uint64_t outputRedMask, uint64_t outputGreenMask, uint64_t outputBlueMask) {
  image output(outputEndian, outputDepth, outputAlphaMask, outputRedMask, outputGreenMask, outputBlueMask);
  output.allocate(width, height);

  #pragma omp parallel for
  for(unsigned y = 0; y < height; y++) {
    uint8_t *dp = output.data + output.pitch * y;
    uint8_t *sp = data + pitch * y;
    for(unsigned x = 0; x < width; x++) {
      uint64_t color = read(sp);
      sp += stride;

      uint64_t a = (color & alpha.mask) >> alpha.shift;
      uint64_t r = (color & red.mask) >> red.shift;
      uint64_t g = (color & green.mask) >> green.shift;
      uint64_t b = (color & blue.mask) >> blue.shift;

      a = normalize(a, alpha.depth, output.alpha.depth);
      r = normalize(r, red.depth, output.red.depth);
      g = normalize(g, green.depth, output.green.depth);
      b = normalize(b, blue.depth, output.blue.depth);

      output.write(dp, (a << output.alpha.shift) | (r << output.red.shift) | (g << output.green.shift) | (b << output.blue.shift));
      dp += output.stride;
    }
  }

  operator=(std::move(output));
}

void image::alphaBlend(uint64_t alphaColor) {
  uint64_t alphaR = (alphaColor & red.mask) >> red.shift;
  uint64_t alphaG = (alphaColor & green.mask) >> green.shift;
  uint64_t alphaB = (alphaColor & blue.mask) >> blue.shift;

  #pragma omp parallel for
  for(unsigned y = 0; y < height; y++) {
    uint8_t *dp = data + pitch * y;
    for(unsigned x = 0; x < width; x++) {
      uint64_t color = read(dp);

      uint64_t colorA = (color & alpha.mask) >> alpha.shift;
      uint64_t colorR = (color & red.mask) >> red.shift;
      uint64_t colorG = (color & green.mask) >> green.shift;
      uint64_t colorB = (color & blue.mask) >> blue.shift;
      double alphaScale = (double)colorA / (double)((1 << alpha.depth) - 1);

      colorA = (1 << alpha.depth) - 1;
      colorR = (colorR * alphaScale) + (alphaR * (1.0 - alphaScale));
      colorG = (colorG * alphaScale) + (alphaG * (1.0 - alphaScale));
      colorB = (colorB * alphaScale) + (alphaB * (1.0 - alphaScale));

      write(dp, (colorA << alpha.shift) | (colorR << red.shift) | (colorG << green.shift) | (colorB << blue.shift));
      dp += stride;
    }
  }
}

//protected

uint64_t image::interpolate(double mu, const uint64_t *s, double (*op)(double, double, double, double, double)) {
  uint64_t aa = (s[0] & alpha.mask) >> alpha.shift, ar = (s[0] & red.mask) >> red.shift,
           ag = (s[0] & green.mask) >> green.shift, ab = (s[0] & blue.mask) >> blue.shift;
  uint64_t ba = (s[1] & alpha.mask) >> alpha.shift, br = (s[1] & red.mask) >> red.shift,
           bg = (s[1] & green.mask) >> green.shift, bb = (s[1] & blue.mask) >> blue.shift;
  uint64_t ca = (s[2] & alpha.mask) >> alpha.shift, cr = (s[2] & red.mask) >> red.shift,
           cg = (s[2] & green.mask) >> green.shift, cb = (s[2] & blue.mask) >> blue.shift;
  uint64_t da = (s[3] & alpha.mask) >> alpha.shift, dr = (s[3] & red.mask) >> red.shift,
           dg = (s[3] & green.mask) >> green.shift, db = (s[3] & blue.mask) >> blue.shift;

  int64_t A = op(mu, aa, ba, ca, da);
  int64_t R = op(mu, ar, br, cr, dr);
  int64_t G = op(mu, ag, bg, cg, dg);
  int64_t B = op(mu, ab, bb, cb, db);

  A = max(0, min(A, (1 << alpha.depth) - 1));
  R = max(0, min(R, (1 << red.depth) - 1));
  G = max(0, min(G, (1 << green.depth) - 1));
  B = max(0, min(B, (1 << blue.depth) - 1));

  return (A << alpha.shift) | (R << red.shift) | (G << green.shift) | (B << blue.shift);
}

void image::scaleX(unsigned outputWidth, interpolation op) {
  uint8_t *outputData = new uint8_t[outputWidth * height * stride];
  unsigned outputPitch = outputWidth * stride;
  double step = (double)width / (double)outputWidth;
  const uint8_t *terminal = data + pitch * height;

  #pragma omp parallel for
  for(unsigned y = 0; y < height; y++) {
    uint8_t *dp = outputData + outputPitch * y;
    uint8_t *sp = data + pitch * y;

    double fraction = 0.0;
    uint64_t s[4] = { sp < terminal ? read(sp) : 0 };  //B,C (0,1) = center of kernel { 0, 0, 1, 2 }
    s[1] = s[0];
    s[2] = sp + stride < terminal ? read(sp += stride) : s[1];
    s[3] = sp + stride < terminal ? read(sp += stride) : s[2];

    for(unsigned x = 0; x < width; x++) {
      while(fraction <= 1.0) {
        if(dp >= outputData + outputPitch * height) break;
        write(dp, interpolate(fraction, (const uint64_t*)&s, op));
        dp += stride;
        fraction += step;
      }

      s[0] = s[1]; s[1] = s[2]; s[2] = s[3];
      if(sp + stride < terminal) s[3] = read(sp += stride);
      fraction -= 1.0;
    }
  }

  free();
  data = outputData;
  width = outputWidth;
  pitch = width * stride;
}

void image::scaleY(unsigned outputHeight, interpolation op) {
  uint8_t *outputData = new uint8_t[width * outputHeight * stride];
  double step = (double)height / (double)outputHeight;
  const uint8_t *terminal = data + pitch * height;

  #pragma omp parallel for
  for(unsigned x = 0; x < width; x++) {
    uint8_t *dp = outputData + stride * x;
    uint8_t *sp = data + stride * x;

    double fraction = 0.0;
    uint64_t s[4] = { sp < terminal ? read(sp) : 0 };
    s[1] = s[0];
    s[2] = sp + pitch < terminal ? read(sp += pitch) : s[1];
    s[3] = sp + pitch < terminal ? read(sp += pitch) : s[2];

    for(unsigned y = 0; y < height; y++) {
      while(fraction <= 1.0) {
        if(dp >= outputData + pitch * outputHeight) break;
        write(dp, interpolate(fraction, (const uint64_t*)&s, op));
        dp += pitch;
        fraction += step;
      }

      s[0] = s[1]; s[1] = s[2]; s[2] = s[3];
      if(sp + pitch < terminal) s[3] = read(sp += pitch);
      fraction -= 1.0;
    }
  }

  free();
  data = outputData;
  height = outputHeight;
}

bool image::loadBMP(const string &filename) {
  uint32_t *outputData;
  unsigned outputWidth, outputHeight;
  if(bmp::read(filename, outputData, outputWidth, outputHeight) == false) return false;

  allocate(outputWidth, outputHeight);
  const uint32_t *sp = outputData;
  uint8_t *dp = data;

  for(unsigned y = 0; y < outputHeight; y++) {
    for(unsigned x = 0; x < outputWidth; x++) {
      uint32_t color = *sp++;
      uint64_t a = normalize((uint8_t)(color >> 24), 8, alpha.depth);
      uint64_t r = normalize((uint8_t)(color >> 16), 8, red.depth);
      uint64_t g = normalize((uint8_t)(color >>  8), 8, green.depth);
      uint64_t b = normalize((uint8_t)(color >>  0), 8, blue.depth);
      write(dp, (a << alpha.shift) | (r << red.shift) | (g << green.shift) | (b << blue.shift));
      dp += stride;
    }
  }

  delete[] outputData;
  return true;
}

bool image::loadPNG(const uint8_t *pngData, unsigned pngSize) {
  png source;
  if(source.decode(pngData, pngSize) == false) return false;

  allocate(source.info.width, source.info.height);
  const uint8_t *sp = source.data;
  uint8_t *dp = data;

  auto decode = [&]() -> uint64_t {
    uint64_t p, r, g, b, a;

    switch(source.info.colorType) {
    case 0:  //L
      r = g = b = source.readbits(sp);
      a = (1 << source.info.bitDepth) - 1;
      break;
    case 2:  //R,G,B
      r = source.readbits(sp);
      g = source.readbits(sp);
      b = source.readbits(sp);
      a = (1 << source.info.bitDepth) - 1;
      break;
    case 3:  //P
      p = source.readbits(sp);
      r = source.info.palette[p][0];
      g = source.info.palette[p][1];
      b = source.info.palette[p][2];
      a = (1 << source.info.bitDepth) - 1;
      break;
    case 4:  //L,A
      r = g = b = source.readbits(sp);
      a = source.readbits(sp);
      break;
    case 6:  //R,G,B,A
      r = source.readbits(sp);
      g = source.readbits(sp);
      b = source.readbits(sp);
      a = source.readbits(sp);
      break;
    }

    a = normalize(a, source.info.bitDepth, alpha.depth);
    r = normalize(r, source.info.bitDepth, red.depth);
    g = normalize(g, source.info.bitDepth, green.depth);
    b = normalize(b, source.info.bitDepth, blue.depth);

    return (a << alpha.shift) | (r << red.shift) | (g << green.shift) | (b << blue.shift);
  };

  for(unsigned y = 0; y < height; y++) {
    for(unsigned x = 0; x < width; x++) {
      write(dp, decode());
      dp += stride;
    }
  }

  return true;
}

bool image::loadPNG(const string &filename) {
  filemap map;
  if(map.open(filename, filemap::mode::read) == false) return false;
  return loadPNG(map.data(), map.size());
}

}

#endif
