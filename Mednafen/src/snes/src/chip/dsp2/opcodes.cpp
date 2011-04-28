#ifdef DSP2_CPP

//convert bitmap to bitplane tile
void DSP2::op01() {
//op01 size is always 32 bytes input and output
//the hardware does strange things if you vary the size

unsigned char c0, c1, c2, c3;
unsigned char *p1  = status.parameters;
unsigned char *p2a = status.output;
unsigned char *p2b = status.output + 16; //halfway

//process 8 blocks of 4 bytes each
  for(int j = 0; j < 8; j++) {
    c0 = *p1++;
    c1 = *p1++;
    c2 = *p1++;
    c3 = *p1++;

    *p2a++ = (c0 & 0x10) << 3 |
             (c0 & 0x01) << 6 |
             (c1 & 0x10) << 1 |
             (c1 & 0x01) << 4 |
             (c2 & 0x10) >> 1 |
             (c2 & 0x01) << 2 |
             (c3 & 0x10) >> 3 |
             (c3 & 0x01);

    *p2a++ = (c0 & 0x20) << 2 |
             (c0 & 0x02) << 5 |
             (c1 & 0x20)      |
             (c1 & 0x02) << 3 |
             (c2 & 0x20) >> 2 |
             (c2 & 0x02) << 1 |
             (c3 & 0x20) >> 4 |
             (c3 & 0x02) >> 1;

    *p2b++ = (c0 & 0x40) << 1 |
             (c0 & 0x04) << 4 |
             (c1 & 0x40) >> 1 |
             (c1 & 0x04) << 2 |
             (c2 & 0x40) >> 3 |
             (c2 & 0x04)      |
             (c3 & 0x40) >> 5 |
             (c3 & 0x04) >> 2;

    *p2b++ = (c0 & 0x80)      |
             (c0 & 0x08) << 3 |
             (c1 & 0x80) >> 2 |
             (c1 & 0x08) << 1 |
             (c2 & 0x80) >> 4 |
             (c2 & 0x08) >> 1 |
             (c3 & 0x80) >> 6 |
             (c3 & 0x08) >> 3;
  }
}

//set transparent color
void DSP2::op03() {
  status.op05transparent = status.parameters[0];
}

//replace bitmap using transparent color
void DSP2::op05() {
uint8 color;
// Overlay bitmap with transparency.
// Input:
//
//   Bitmap 1:  i[0] <=> i[size-1]
//   Bitmap 2:  i[size] <=> i[2*size-1]
//
// Output:
//
//   Bitmap 3:  o[0] <=> o[size-1]
//
// Processing:
//
//   Process all 4-bit pixels (nibbles) in the bitmap
//
//   if ( BM2_pixel == transparent_color )
//      pixelout = BM1_pixel
//   else
//      pixelout = BM2_pixel

// The max size bitmap is limited to 255 because the size parameter is a byte
// I think size=0 is an error.  The behavior of the chip on size=0 is to
// return the last value written to DR if you read DR on Op05 with
// size = 0.  I don't think it's worth implementing this quirk unless it's
// proven necessary.

unsigned char c1, c2;
unsigned char *p1 = status.parameters;
unsigned char *p2 = status.parameters + status.op05len;
unsigned char *p3 = status.output;

  color = status.op05transparent & 0x0f;

  for(int n = 0; n < status.op05len; n++) {
    c1 = *p1++;
    c2 = *p2++;
    *p3++ = ( ((c2 >> 4)   == color ) ? c1 & 0xf0 : c2 & 0xf0 ) |
            ( ((c2 & 0x0f) == color ) ? c1 & 0x0f : c2 & 0x0f );
  }
}

//reverse bitmap
void DSP2::op06() {
// Input:
//    size
//    bitmap

int i, j;
  for(i = 0, j = status.op06len - 1; i < status.op06len; i++, j--) {
    status.output[j] = (status.parameters[i] << 4) | (status.parameters[i] >> 4);
  }
}

//multiply
void DSP2::op09() {
  status.out_count = 4;

  status.op09word1 = status.parameters[0] | (status.parameters[1] << 8);
  status.op09word2 = status.parameters[2] | (status.parameters[3] << 8);

uint32 r;
  r = status.op09word1 * status.op09word2;
  status.output[0] = r;
  status.output[1] = r >> 8;
  status.output[2] = r >> 16;
  status.output[3] = r >> 24;
}

//scale bitmap
void DSP2::op0d() {
// Bit accurate hardware algorithm - uses fixed point math
// This should match the DSP2 Op0D output exactly
// I wouldn't recommend using this unless you're doing hardware debug.
// In some situations it has small visual artifacts that
// are not readily apparent on a TV screen but show up clearly
// on a monitor.  Use Overload's scaling instead.
// This is for hardware verification testing.
//
// One note:  the HW can do odd byte scaling but since we divide
// by two to get the count of bytes this won't work well for
// odd byte scaling (in any of the current algorithm implementations).
// So far I haven't seen Dungeon Master use it.
// If it does we can adjust the parameters and code to work with it

uint32 multiplier; // Any size int >= 32-bits
uint32 pixloc;     // match size of multiplier
int    i, j;
uint8  pixelarray[512];
  if(status.op0dinlen <= status.op0doutlen) {
    multiplier = 0x10000; // In our self defined fixed point 0x10000 == 1
  } else {
    multiplier = (status.op0dinlen << 17) / ((status.op0doutlen << 1) + 1);
  }

  pixloc = 0;
  for(i = 0; i < status.op0doutlen * 2; i++) {
    j = pixloc >> 16;

    if(j & 1) {
      pixelarray[i] = (status.parameters[j >> 1] & 0x0f);
    } else {
      pixelarray[i] = (status.parameters[j >> 1] & 0xf0) >> 4;
    }

    pixloc += multiplier;
  }

  for(i = 0; i < status.op0doutlen; i++) {
    status.output[i] = (pixelarray[i << 1] << 4) | pixelarray[(i << 1) + 1];
  }
}

#endif
