#include "gbafilter.h"

#include <math.h>

extern int systemColorDepth;
extern int systemRedShift;
extern int systemGreenShift;
extern int systemBlueShift;

extern u16 systemColorMap16[0x10000];
extern u32 systemColorMap32[0x10000];

static const unsigned char curve[32] = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0e, 0x10, 0x12,
										 0x14, 0x16, 0x18, 0x1c, 0x20, 0x28, 0x30, 0x38,
										 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x80,
										 0x88, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0};

//								output           R   G   B
static const unsigned char influence[3 * 3] = { 16,  4,  4, // red
												 8, 16,  8, // green
												 0,  8, 16};// blue

inline void swap(short & a, short & b)
{
	short temp = a;
	a = b;
	b = temp;
}

void gbafilter_pal(u16 * buf, int count)
{
	short temp[3 * 3], s;
	unsigned pix;
	u8 red, green, blue;

	while (count--)
	{
		pix = *buf;

		s = curve[(pix >> systemGreenShift) & 0x1f];
		temp[3] = s * influence[3];
		temp[4] = s * influence[4];
		temp[5] = s * influence[5];

		s = curve[(pix >> systemRedShift) & 0x1f];
		temp[0] = s * influence[0];
		temp[1] = s * influence[1];
		temp[2] = s * influence[2];

		s = curve[(pix >> systemBlueShift) & 0x1f];
		temp[6] = s * influence[6];
		temp[7] = s * influence[7];
		temp[8] = s * influence[8];

		if (temp[0] < temp[3]) swap(temp[0], temp[3]);
		if (temp[0] < temp[6]) swap(temp[0], temp[6]);
		if (temp[3] < temp[6]) swap(temp[3], temp[6]);
		temp[3] <<= 1;
		temp[0] <<= 2;
		temp[0] += temp[3] + temp[6];

		red = ((int(temp[0]) * 160) >> 17) + 4;
		if (red > 31) red = 31;

		if (temp[2] < temp[5]) swap(temp[2], temp[5]);
		if (temp[2] < temp[8]) swap(temp[2], temp[8]);
		if (temp[5] < temp[8]) swap(temp[5], temp[8]);
		temp[5] <<= 1;
		temp[2] <<= 2;
		temp[2] += temp[5] + temp[8];

		blue = ((int(temp[2]) * 160) >> 17) + 4;
		if (blue > 31) blue = 31;

		if (temp[1] < temp[4]) swap(temp[1], temp[4]);
		if (temp[1] < temp[7]) swap(temp[1], temp[7]);
		if (temp[4] < temp[7]) swap(temp[4], temp[7]);
		temp[4] <<= 1;
		temp[1] <<= 2;
		temp[1] += temp[4] + temp[7];

		green = ((int(temp[1]) * 160) >> 17) + 4;
		if (green > 31) green = 31;

		pix  = red << systemRedShift;
		pix += green << systemGreenShift;
		pix += blue << systemBlueShift;

		*buf++ = pix;
	}
}

void gbafilter_pal32(u32 * buf, int count)
{
	short temp[3 * 3], s;
	unsigned pix;
	u8 red, green, blue;

	while (count--)
	{
		pix = *buf;

		s = curve[(pix >> systemGreenShift) & 0x1f];
		temp[3] = s * influence[3];
		temp[4] = s * influence[4];
		temp[5] = s * influence[5];

		s = curve[(pix >> systemRedShift) & 0x1f];
		temp[0] = s * influence[0];
		temp[1] = s * influence[1];
		temp[2] = s * influence[2];

		s = curve[(pix >> systemBlueShift) & 0x1f];
		temp[6] = s * influence[6];
		temp[7] = s * influence[7];
		temp[8] = s * influence[8];

		if (temp[0] < temp[3]) swap(temp[0], temp[3]);
		if (temp[0] < temp[6]) swap(temp[0], temp[6]);
		if (temp[3] < temp[6]) swap(temp[3], temp[6]);
		temp[3] <<= 1;
		temp[0] <<= 2;
		temp[0] += temp[3] + temp[6];

		//red = ((int(temp[0]) * 160) >> 17) + 4;
		red = ((int(temp[0]) * 160) >> 14) + 32;

		if (temp[2] < temp[5]) swap(temp[2], temp[5]);
		if (temp[2] < temp[8]) swap(temp[2], temp[8]);
		if (temp[5] < temp[8]) swap(temp[5], temp[8]);
		temp[5] <<= 1;
		temp[2] <<= 2;
		temp[2] += temp[5] + temp[8];

		//blue = ((int(temp[2]) * 160) >> 17) + 4;
		blue = ((int(temp[2]) * 160) >> 14) + 32;

		if (temp[1] < temp[4]) swap(temp[1], temp[4]);
		if (temp[1] < temp[7]) swap(temp[1], temp[7]);
		if (temp[4] < temp[7]) swap(temp[4], temp[7]);
		temp[4] <<= 1;
		temp[1] <<= 2;
		temp[1] += temp[4] + temp[7];

		//green = ((int(temp[1]) * 160) >> 17) + 4;
		green = ((int(temp[1]) * 160) >> 14) + 32;

		//pix  = red << redshift;
		//pix += green << greenshift;
		//pix += blue << blueshift;

		pix  = red << (systemRedShift - 3);
		pix += green << (systemGreenShift - 3);
		pix += blue << (systemBlueShift - 3);

		*buf++ = pix;
	}
}

// for palette mode to work with the three spoony filters in 32bpp depth

void gbafilter_pad(u8 * buf, int count)
{
	union
	{
		struct
		{
			u8 r;
			u8 g;
			u8 b;
			u8 a;
		} part;
		unsigned whole;
	}
	mask;

	mask.whole  = 0x1f << systemRedShift;
	mask.whole += 0x1f << systemGreenShift;
	mask.whole += 0x1f << systemBlueShift;

	switch (systemColorDepth)
	{
	case 24:
		while (count--)
		{
			*buf++ &= mask.part.r;
			*buf++ &= mask.part.g;
			*buf++ &= mask.part.b;
		}
		break;
	case 32:
		while (count--)
		{
			*((u32*)buf) &= mask.whole;
			buf += 4;
		}
	}
}

/*
void UpdateSystemColorMaps(int lcd)
{
  switch(systemColorDepth) {
  case 16:
    {
      for(int i = 0; i < 0x10000; i++) {
        systemColorMap16[i] = ((i & 0x1f) << systemRedShift) |
          (((i & 0x3e0) >> 5) << systemGreenShift) |
          (((i & 0x7c00) >> 10) << systemBlueShift);
      }
	  if (lcd == 1) gbafilter_pal(systemColorMap16, 0x10000);
    }
    break;
  case 24:
  case 32:
    {
      for(int i = 0; i < 0x10000; i++) {
        systemColorMap32[i] = ((i & 0x1f) << systemRedShift) |
          (((i & 0x3e0) >> 5) << systemGreenShift) |
          (((i & 0x7c00) >> 10) << systemBlueShift);
      }
	  if (lcd == 1) gbafilter_pal32(systemColorMap32, 0x10000);
    }
    break;
  }
}
*/
