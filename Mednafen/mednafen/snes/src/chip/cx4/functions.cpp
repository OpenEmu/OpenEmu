#ifdef CX4_CPP

#include <math.h>
#define Tan(a) (CosTable[a] ? ((((int32)SinTable[a]) << 16) / CosTable[a]) : 0x80000000)
#define sar(b, n) ((b) >> (n))
#ifdef PI
#undef PI
#endif
#define PI 3.1415926535897932384626433832795

//Wireframe Helpers
void Cx4::C4TransfWireFrame() {
  double c4x = (double)C4WFXVal;
  double c4y = (double)C4WFYVal;
  double c4z = (double)C4WFZVal - 0x95;
  double tanval, c4x2, c4y2, c4z2;

  //Rotate X
  tanval = -(double)C4WFX2Val * PI * 2 / 128;
  c4y2   = c4y * ::cos(tanval) - c4z * ::sin(tanval);
  c4z2   = c4y * ::sin(tanval) + c4z * ::cos(tanval);

  //Rotate Y
  tanval = -(double)C4WFY2Val * PI * 2 / 128;
  c4x2   = c4x * ::cos(tanval)  + c4z2 * ::sin(tanval);
  c4z    = c4x * -::sin(tanval) + c4z2 * ::cos(tanval);

  //Rotate Z
  tanval = -(double)C4WFDist * PI * 2 / 128;
  c4x    = c4x2 * ::cos(tanval) - c4y2 * ::sin(tanval);
  c4y    = c4x2 * ::sin(tanval) + c4y2 * ::cos(tanval);

  //Scale
  C4WFXVal = (int16)(c4x * C4WFScale / (0x90 * (c4z + 0x95)) * 0x95);
  C4WFYVal = (int16)(c4y * C4WFScale / (0x90 * (c4z + 0x95)) * 0x95);
}

void Cx4::C4CalcWireFrame() {
  C4WFXVal = C4WFX2Val - C4WFXVal;
  C4WFYVal = C4WFY2Val - C4WFYVal;

  if(abs(C4WFXVal) > abs(C4WFYVal)) {
    C4WFDist = abs(C4WFXVal) + 1;
    C4WFYVal = (256 * (long)C4WFYVal) / abs(C4WFXVal);
    C4WFXVal = (C4WFXVal < 0) ? -256 : 256;
  } else if(C4WFYVal != 0) {
    C4WFDist = abs(C4WFYVal) + 1;
    C4WFXVal = (256 * (long)C4WFXVal) / abs(C4WFYVal);
    C4WFYVal = (C4WFYVal < 0) ? -256 : 256;
  } else {
    C4WFDist = 0;
  }
}

void Cx4::C4TransfWireFrame2() {
  double c4x = (double)C4WFXVal;
  double c4y = (double)C4WFYVal;
  double c4z = (double)C4WFZVal;
  double tanval, c4x2, c4y2, c4z2;

  //Rotate X
  tanval = -(double)C4WFX2Val * PI * 2 / 128;
  c4y2   = c4y * ::cos(tanval) - c4z * ::sin(tanval);
  c4z2   = c4y * ::sin(tanval) + c4z * ::cos(tanval);

  //Rotate Y
  tanval = -(double)C4WFY2Val * PI * 2 / 128;
  c4x2   = c4x * ::cos(tanval)  + c4z2 * ::sin(tanval);
  c4z    = c4x * -::sin(tanval) + c4z2 * ::cos(tanval);

  //Rotate Z
  tanval = -(double)C4WFDist * PI * 2 / 128;
  c4x    = c4x2 * ::cos(tanval) - c4y2 * ::sin(tanval);
  c4y    = c4x2 * ::sin(tanval) + c4y2 * ::cos(tanval);

  //Scale
  C4WFXVal = (int16)(c4x * C4WFScale / 0x100);
  C4WFYVal = (int16)(c4y * C4WFScale / 0x100);
}

void Cx4::C4DrawWireFrame() {
  uint32 line = readl(0x1f80);
  uint32 point1, point2;
  int16 X1, Y1, Z1;
  int16 X2, Y2, Z2;
  uint8 Color;

  for(int32 i = ram[0x0295]; i > 0; i--, line += 5) {
    if(bus.read(line) == 0xff && bus.read(line + 1) == 0xff) {
      int32 tmp = line - 5;
      while(bus.read(tmp + 2) == 0xff && bus.read(tmp + 3) == 0xff && (tmp + 2) >= 0) { tmp -= 5; }
      point1 = (read(0x1f82) << 16) | (bus.read(tmp + 2) << 8) | bus.read(tmp + 3);
    } else {
      point1 = (read(0x1f82) << 16) | (bus.read(line) << 8) | bus.read(line + 1);
    }
    point2 = (read(0x1f82) << 16) | (bus.read(line + 2) << 8) | bus.read(line + 3);

    X1=(bus.read(point1 + 0) << 8) | bus.read(point1 + 1);
    Y1=(bus.read(point1 + 2) << 8) | bus.read(point1 + 3);
    Z1=(bus.read(point1 + 4) << 8) | bus.read(point1 + 5);
    X2=(bus.read(point2 + 0) << 8) | bus.read(point2 + 1);
    Y2=(bus.read(point2 + 2) << 8) | bus.read(point2 + 3);
    Z2=(bus.read(point2 + 4) << 8) | bus.read(point2 + 5);
    Color = bus.read(line + 4);
    C4DrawLine(X1, Y1, Z1, X2, Y2, Z2, Color);
  }
}

void Cx4::C4DrawLine(int32 X1, int32 Y1, int16 Z1, int32 X2, int32 Y2, int16 Z2, uint8 Color) {
  //Transform coordinates
  C4WFXVal  = (int16)X1;
  C4WFYVal  = (int16)Y1;
  C4WFZVal  = Z1;
  C4WFScale = read(0x1f90);
  C4WFX2Val = read(0x1f86);
  C4WFY2Val = read(0x1f87);
  C4WFDist  = read(0x1f88);
  C4TransfWireFrame2();
  X1 = (C4WFXVal + 48) << 8;
  Y1 = (C4WFYVal + 48) << 8;

  C4WFXVal = (int16)X2;
  C4WFYVal = (int16)Y2;
  C4WFZVal = Z2;
  C4TransfWireFrame2();
  X2 = (C4WFXVal + 48) << 8;
  Y2 = (C4WFYVal + 48) << 8;

  //Get line info
  C4WFXVal  = (int16)(X1 >> 8);
  C4WFYVal  = (int16)(Y1 >> 8);
  C4WFX2Val = (int16)(X2 >> 8);
  C4WFY2Val = (int16)(Y2 >> 8);
  C4CalcWireFrame();
  X2 = (int16)C4WFXVal;
  Y2 = (int16)C4WFYVal;

  //Render line
  for(int32 i = C4WFDist ? C4WFDist : 1; i > 0; i--) {
    if(X1 > 0xff && Y1 > 0xff && X1 < 0x6000 && Y1 < 0x6000) {
      uint16 addr = (((Y1 >> 8) >> 3) << 8) - (((Y1 >> 8) >> 3) << 6) + (((X1 >> 8) >> 3) << 4) + ((Y1 >> 8) & 7) * 2;
      uint8 bit = 0x80 >> ((X1 >> 8) & 7);
      ram[addr + 0x300] &= ~bit;
      ram[addr + 0x301] &= ~bit;
      if(Color & 1) ram[addr + 0x300] |= bit;
      if(Color & 2) ram[addr + 0x301] |= bit;
    }
    X1 += X2;
    Y1 += Y2;
  }
}

void Cx4::C4DoScaleRotate(int row_padding) {
  int16 A, B, C, D;

  //Calculate matrix
  int32 XScale = readw(0x1f8f);
  int32 YScale = readw(0x1f92);

  if(XScale & 0x8000)XScale = 0x7fff;
  if(YScale & 0x8000)YScale = 0x7fff;

  if(readw(0x1f80) == 0) {  //no rotation
    A = (int16)XScale;
    B = 0;
    C = 0;
    D = (int16)YScale;
  } else if(readw(0x1f80) == 128) {  //90 degree rotation
    A = 0;
    B = (int16)(-YScale);
    C = (int16)XScale;
    D = 0;
  } else if(readw(0x1f80) == 256) {  //180 degree rotation
    A = (int16)(-XScale);
    B = 0;
    C = 0;
    D = (int16)(-YScale);
  } else if(readw(0x1f80) == 384) {  //270 degree rotation
    A = 0;
    B = (int16)YScale;
    C = (int16)(-XScale);
    D = 0;
  } else {
    A = (int16)  sar(CosTable[readw(0x1f80) & 0x1ff] * XScale, 15);
    B = (int16)(-sar(SinTable[readw(0x1f80) & 0x1ff] * YScale, 15));
    C = (int16)  sar(SinTable[readw(0x1f80) & 0x1ff] * XScale, 15);
    D = (int16)  sar(CosTable[readw(0x1f80) & 0x1ff] * YScale, 15);
  }

  //Calculate Pixel Resolution
  uint8 w = read(0x1f89) & ~7;
  uint8 h = read(0x1f8c) & ~7;

  //Clear the output RAM
  memset(ram, 0, (w + row_padding / 4) * h / 2);

  int32 Cx = (int16)readw(0x1f83);
  int32 Cy = (int16)readw(0x1f86);

  //Calculate start position (i.e. (Ox, Oy) = (0, 0))
  //The low 12 bits are fractional, so (Cx<<12) gives us the Cx we want in
  //the function. We do Cx*A etc normally because the matrix parameters
  //already have the fractional parts.
  int32 LineX = (Cx << 12) - Cx * A - Cx * B;
  int32 LineY = (Cy << 12) - Cy * C - Cy * D;

  //Start loop
  uint32 X, Y;
  uint8 byte;
  int32 outidx = 0;
  uint8 bit    = 0x80;

  for(int32 y = 0; y < h; y++) {
    X = LineX;
    Y = LineY;
    for(int32 x = 0; x < w; x++) {
      if((X >> 12) >= w || (Y >> 12) >= h) {
        byte = 0;
      } else {
        uint32 addr = (Y >> 12) * w + (X >> 12);
        byte = read(0x600 + (addr >> 1));
        if(addr & 1) { byte >>= 4; }
      }

      //De-bitplanify
      if(byte & 1) ram[outidx     ] |= bit;
      if(byte & 2) ram[outidx +  1] |= bit;
      if(byte & 4) ram[outidx + 16] |= bit;
      if(byte & 8) ram[outidx + 17] |= bit;

      bit >>= 1;
      if(!bit) {
        bit     = 0x80;
        outidx += 32;
      }

      X += A;  //Add 1 to output x => add an A and a C
      Y += C;
    }
    outidx += 2 + row_padding;
    if(outidx & 0x10) {
      outidx &= ~0x10;
    } else {
      outidx -= w * 4 + row_padding;
    }
    LineX += B;  //Add 1 to output y => add a B and a D
    LineY += D;
  }
}

#endif
