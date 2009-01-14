#include "Filters.h"


int   LUT16to32[65536];
int   RGBtoYUV[65536];

void InitLUTs(void)
{
	int i, j, k, r, g, b, Y, u, v;
	
	for (i=0; i<65536; i++)
		LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	
	for (i=0; i<32; i++)
		for (j=0; j<64; j++)
			for (k=0; k<32; k++)
			{
				r = i << 3;
				g = j << 2;
				b = k << 3;
				Y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b)>>3);
				RGBtoYUV[ (i << 11) + (j << 5) + k ] = (Y<<16) + (u<<8) + v;
			}
}
