struct Pixel {
  bool enable;
  unsigned priority;
  unsigned color;

  //objects only
  bool translucent;
  bool mosaic;

  alwaysinline void write(bool e) { enable = e; }
  alwaysinline void write(bool e, unsigned p, unsigned c) { enable = e; priority = p; color = c; }
  alwaysinline void write(bool e, unsigned p, unsigned c, bool t, bool m) { enable = e; priority = p; color = c; translucent = t; mosaic = m; }
} layer[6][240];

bool windowmask[3][240];
unsigned vmosaic[5];
unsigned hmosaic[5];

struct Object {
  uint8  y;
  uint1  affine;
  uint1  affinesize;
  uint2  mode;
  uint1  mosaic;
  uint1  colors;      //0 = 16, 1 = 256
  uint2  shape;       //0 = square, 1 = horizontal, 2 = vertical

  uint9  x;
  uint5  affineparam;
  uint1  hflip;
  uint1  vflip;
  uint2  size;

  uint10 character;
  uint2  priority;
  uint4  palette;

  //ancillary data
  unsigned width;
  unsigned height;
} object[128];

struct ObjectParam {
  int16 pa;
  int16 pb;
  int16 pc;
  int16 pd;
} objectparam[32];

struct Tile {
  uint10 character;
  uint1  hflip;
  uint1  vflip;
  uint4  palette;
};
