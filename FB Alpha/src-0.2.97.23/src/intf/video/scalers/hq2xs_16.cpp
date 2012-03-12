// hq2xs, hq2xbold, hq3xs, hq3xbold filters, added by regret
// source from Snes9X v1.51+ (http://code.google.com/p/snes9x151-rerecording/)

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef unsigned int uint32;

#define Mask_2	0x07E0	// 00000 111111 00000
#define Mask13	0xF81F	// 11111 000000 11111
#define Mask_1	0x001F	// 00000 000000 11111
#define Mask_3	0xF800	// 11111 000000 00000
#define NUMBITS (16)

static int RGBtoYUV[1<<NUMBITS];
static uint16 RGBtoBright[1<<NUMBITS];

#define Interp05(c1, c2) \
	(((c1) == (c2)) ? (c1) : \
	((((((c1) & Mask_2)      + ((c2) & Mask_2)) >> 1) & Mask_2) + \
	 (((((c1) & Mask13)      + ((c2) & Mask13)) >> 1) & Mask13)))
#define Interp01(c1, c2) \
	((((c1) == (c2)) ? (c1) : \
	(((((((c1) & Mask_2) *  3) + ((c2) & Mask_2)) >> 2) & Mask_2) + \
	 ((((((c1) & Mask13) *  3) + ((c2) & Mask13)) >> 2) & Mask13))))
#define Halve(c1) \
	((((((c1) & Mask_2)) >> 1) & Mask_2) + \
	 (((((c1) & Mask13)) >> 1) & Mask13))
#define ThreeQuarters(c1) \
	((((((c1) & Mask_2)*3) >> 2) & Mask_2) + \
	 (((((c1) & Mask13)*3) >> 2) & Mask13))

#define Ymask	0xFF0000
#define Umask	0x00FF00
#define Vmask	0x0000FF
#define trY		0x300000
#define trU		0x000700
#define trV		0x000006

#define Absolute(c) \
	(!((c) & (1 << 31)) ? (c) : (~(c) + 1))

static inline bool Diff(int c1, int c2)
{
	int c1y = (c1 & Ymask) - (c2 & Ymask);
	if (Absolute(c1y) > trY) return true;
	int c1u = (c1 & Umask) - (c2 & Umask);
	if (Absolute(c1u) > trU) return true;
	int c1v = (c1 & Vmask) - (c2 & Vmask);
	if (Absolute(c1v) > trV) return true;

	return false;
}

void InitLUTs(void)
{
	int c, r, g, b, y, u, v;

	for (c = 0 ; c < (1<<NUMBITS) ; c++)
  	{
		b = (int)((c & 0x1F)) << 3;
		g = (int)((c & 0x7E0)) >> 3;
		r = (int)((c & 0xF800)) >> 8;
		RGBtoBright[c] = r+r+r + g+g+g + b+b;

		y = (int)( 0.256788f*r + 0.504129f*g + 0.097906f*b + 0.5f) + 16;
		u = (int)(-0.148223f*r - 0.290993f*g + 0.439216f*b + 0.5f) + 128;
		v = (int)( 0.439216f*r - 0.367788f*g - 0.071427f*b + 0.5f) + 128;

		RGBtoYUV[c] = (y << 16) + (u << 8) + v;

//		y = (r + g + b) >> 2;
//    	u = 128 + ((r - b) >> 2);
//    	v = 128 + ((-r + 2 * g - b) >> 3);
//
//		RGBtoYUVLQ[c] = (y << 16) + (u << 8) + v;
	}
}

#define Interp02(c1, c2, c3) \
	((((((c1) & Mask_2) *  2 + ((c2) & Mask_2)     + ((c3) & Mask_2)    ) >> 2) & Mask_2) + \
	(((((c1) & Mask13) *  2 + ((c2) & Mask13)     + ((c3) & Mask13)    ) >> 2) & Mask13))

#define Interp06(c1, c2, c3) \
	((((((c1) & Mask_2) *  5 + ((c2) & Mask_2) * 2 + ((c3) & Mask_2)    ) >> 3) & Mask_2) + \
	(((((c1) & Mask13) *  5 + ((c2) & Mask13) * 2 + ((c3) & Mask13)    ) >> 3) & Mask13))

#define Interp07(c1, c2, c3) \
	((((((c1) & Mask_2) *  6 + ((c2) & Mask_2)     + ((c3) & Mask_2)    ) >> 3) & Mask_2) + \
	(((((c1) & Mask13) *  6 + ((c2) & Mask13)     + ((c3) & Mask13)    ) >> 3) & Mask13))

#define Interp09(c1, c2, c3) \
	((((((c1) & Mask_2) *  2 + ((c2) & Mask_2) * 3 + ((c3) & Mask_2) * 3) >> 3) & Mask_2) + \
	(((((c1) & Mask13) *  2 + ((c2) & Mask13) * 3 + ((c3) & Mask13) * 3) >> 3) & Mask13))

#define Interp10(c1, c2, c3) \
	((((((c1) & Mask_2) * 14 + ((c2) & Mask_2)     + ((c3) & Mask_2)    ) >> 4) & Mask_2) + \
	(((((c1) & Mask13) * 14 + ((c2) & Mask13)     + ((c3) & Mask13)    ) >> 4) & Mask13))


// HQ2xS

#define PIXEL00_0		*(dp) = w5
#define PIXEL00_10		*(dp) = Interp01(w5, w1)
#define PIXEL00_11		*(dp) = Interp01(w5, w4)
#define PIXEL00_12		*(dp) = Interp01(w5, w2)
#define PIXEL00_20		*(dp) = Interp02(w5, w4, w2)
#define PIXEL00_21		*(dp) = Interp02(w5, w1, w2)
#define PIXEL00_22		*(dp) = Interp02(w5, w1, w4)
#define PIXEL00_60		*(dp) = Interp06(w5, w2, w4)
#define PIXEL00_61		*(dp) = Interp06(w5, w4, w2)
#define PIXEL00_70		*(dp) = Interp07(w5, w4, w2)
#define PIXEL00_90		*(dp) = Interp09(w5, w4, w2)
#define PIXEL00_100		*(dp) = Interp10(w5, w4, w2)

#define PIXEL01_0		*(dp + 1) = w5
#define PIXEL01_10		*(dp + 1) = Interp01(w5, w3)
#define PIXEL01_11		*(dp + 1) = Interp01(w5, w2)
#define PIXEL01_12		*(dp + 1) = Interp01(w5, w6)
#define PIXEL01_20		*(dp + 1) = Interp02(w5, w2, w6)
#define PIXEL01_21		*(dp + 1) = Interp02(w5, w3, w6)
#define PIXEL01_22		*(dp + 1) = Interp02(w5, w3, w2)
#define PIXEL01_60		*(dp + 1) = Interp06(w5, w6, w2)
#define PIXEL01_61		*(dp + 1) = Interp06(w5, w2, w6)
#define PIXEL01_70		*(dp + 1) = Interp07(w5, w2, w6)
#define PIXEL01_90		*(dp + 1) = Interp09(w5, w2, w6)
#define PIXEL01_100		*(dp + 1) = Interp10(w5, w2, w6)

#define PIXEL10_0		*(dp + dst1line) = w5
#define PIXEL10_10		*(dp + dst1line) = Interp01(w5, w7)
#define PIXEL10_11		*(dp + dst1line) = Interp01(w5, w8)
#define PIXEL10_12		*(dp + dst1line) = Interp01(w5, w4)
#define PIXEL10_20		*(dp + dst1line) = Interp02(w5, w8, w4)
#define PIXEL10_21		*(dp + dst1line) = Interp02(w5, w7, w4)
#define PIXEL10_22		*(dp + dst1line) = Interp02(w5, w7, w8)
#define PIXEL10_60		*(dp + dst1line) = Interp06(w5, w4, w8)
#define PIXEL10_61		*(dp + dst1line) = Interp06(w5, w8, w4)
#define PIXEL10_70		*(dp + dst1line) = Interp07(w5, w8, w4)
#define PIXEL10_90		*(dp + dst1line) = Interp09(w5, w8, w4)
#define PIXEL10_100		*(dp + dst1line) = Interp10(w5, w8, w4)

#define PIXEL11_0		*(dp + dst1line + 1) = w5
#define PIXEL11_10		*(dp + dst1line + 1) = Interp01(w5, w9)
#define PIXEL11_11		*(dp + dst1line + 1) = Interp01(w5, w6)
#define PIXEL11_12		*(dp + dst1line + 1) = Interp01(w5, w8)
#define PIXEL11_20		*(dp + dst1line + 1) = Interp02(w5, w6, w8)
#define PIXEL11_21		*(dp + dst1line + 1) = Interp02(w5, w9, w8)
#define PIXEL11_22		*(dp + dst1line + 1) = Interp02(w5, w9, w6)
#define PIXEL11_60		*(dp + dst1line + 1) = Interp06(w5, w8, w6)
#define PIXEL11_61		*(dp + dst1line + 1) = Interp06(w5, w6, w8)
#define PIXEL11_70		*(dp + dst1line + 1) = Interp07(w5, w6, w8)
#define PIXEL11_90		*(dp + dst1line + 1) = Interp09(w5, w6, w8)
#define PIXEL11_100		*(dp + dst1line + 1) = Interp10(w5, w6, w8)


#define HQ2XCASES \
	case 0: case 1: case 4: case 32: case 128: case 5: case 132: case 160: case 33: case 129: case 36: case 133: case 164: case 161: case 37: case 165: PIXEL00_20; PIXEL01_20; PIXEL10_20; PIXEL11_20; break; \
	case 2: case 34: case 130: case 162: PIXEL00_22; PIXEL01_21; PIXEL10_20; PIXEL11_20; break; \
	case 16: case 17: case 48: case 49: PIXEL00_20; PIXEL01_22; PIXEL10_20; PIXEL11_21; break; \
	case 64: case 65: case 68: case 69: PIXEL00_20; PIXEL01_20; PIXEL10_21; PIXEL11_22; break; \
	case 8: case 12: case 136: case 140: PIXEL00_21; PIXEL01_20; PIXEL10_22; PIXEL11_20; break; \
	case 3: case 35: case 131: case 163: PIXEL00_11; PIXEL01_21; PIXEL10_20; PIXEL11_20; break; \
	case 6: case 38: case 134: case 166: PIXEL00_22; PIXEL01_12; PIXEL10_20; PIXEL11_20; break; \
	case 20: case 21: case 52: case 53: PIXEL00_20; PIXEL01_11; PIXEL10_20; PIXEL11_21; break; \
	case 144: case 145: case 176: case 177: PIXEL00_20; PIXEL01_22; PIXEL10_20; PIXEL11_12; break; \
	case 192: case 193: case 196: case 197: PIXEL00_20; PIXEL01_20; PIXEL10_21; PIXEL11_11; break; \
	case 96: case 97: case 100: case 101: PIXEL00_20; PIXEL01_20; PIXEL10_12; PIXEL11_22; break; \
	case 40: case 44: case 168: case 172: PIXEL00_21; PIXEL01_20; PIXEL10_11; PIXEL11_20; break; \
	case 9: case 13: case 137: case 141: PIXEL00_12; PIXEL01_20; PIXEL10_22; PIXEL11_20; break; \
	case 18: case 50: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_20; PIXEL10_20; PIXEL11_21; break; \
	case 80: case 81: PIXEL00_20; PIXEL01_22; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_20; break; \
	case 72: case 76: PIXEL00_21; PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_20; PIXEL11_22; break; \
	case 10: case 138: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_20; PIXEL01_21; PIXEL10_22; PIXEL11_20; break; \
	case 66: PIXEL00_22; PIXEL01_21; PIXEL10_21; PIXEL11_22; break; \
	case 24: PIXEL00_21; PIXEL01_22; PIXEL10_22; PIXEL11_21; break; \
	case 7: case 39: case 135: PIXEL00_11; PIXEL01_12; PIXEL10_20; PIXEL11_20; break; \
	case 148: case 149: case 180: PIXEL00_20; PIXEL01_11; PIXEL10_20; PIXEL11_12; break; \
	case 224: case 228: case 225: PIXEL00_20; PIXEL01_20; PIXEL10_12; PIXEL11_11; break; \
	case 41: case 169: case 45: PIXEL00_12; PIXEL01_20; PIXEL10_11; PIXEL11_20; break; \
	case 22: case 54: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_20; PIXEL11_21; break; \
	case 208: case 209: PIXEL00_20; PIXEL01_22; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 104: case 108: PIXEL00_21; PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_22; break; \
	case 11: case 139: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_21; PIXEL10_22; PIXEL11_20; break; \
	case 19: case 51: if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL00_11, PIXEL01_10; else PIXEL00_60, PIXEL01_90; PIXEL10_20; PIXEL11_21; break; \
	case 146: case 178: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10, PIXEL11_12; else PIXEL01_90, PIXEL11_61; PIXEL10_20; break; \
	case 84: case 85: PIXEL00_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL01_11, PIXEL11_10; else PIXEL01_60, PIXEL11_90; PIXEL10_21; break; \
	case 112: case 113: PIXEL00_20; PIXEL01_22; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL10_12, PIXEL11_10; else PIXEL10_61, PIXEL11_90; break; \
	case 200: case 204: PIXEL00_21; PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10, PIXEL11_11; else PIXEL10_90, PIXEL11_60; break; \
	case 73: case 77: if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL00_12, PIXEL10_10; else PIXEL00_61, PIXEL10_90; PIXEL01_20; PIXEL11_22; break; \
	case 42: case 170: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10, PIXEL10_11; else PIXEL00_90, PIXEL10_60; PIXEL01_21; PIXEL11_20; break; \
	case 14: case 142: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10, PIXEL01_12; else PIXEL00_90, PIXEL01_61; PIXEL10_22; PIXEL11_20; break; \
	case 67: PIXEL00_11; PIXEL01_21; PIXEL10_21; PIXEL11_22; break; \
	case 70: PIXEL00_22; PIXEL01_12; PIXEL10_21; PIXEL11_22; break; \
	case 28: PIXEL00_21; PIXEL01_11; PIXEL10_22; PIXEL11_21; break; \
	case 152: PIXEL00_21; PIXEL01_22; PIXEL10_22; PIXEL11_12; break; \
	case 194: PIXEL00_22; PIXEL01_21; PIXEL10_21; PIXEL11_11; break; \
	case 98: PIXEL00_22; PIXEL01_21; PIXEL10_12; PIXEL11_22; break; \
	case 56: PIXEL00_21; PIXEL01_22; PIXEL10_11; PIXEL11_21; break; \
	case 25: PIXEL00_12; PIXEL01_22; PIXEL10_22; PIXEL11_21; break; \
	case 26: case 31: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_22; PIXEL11_21; break; \
	case 82: case 214: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 88: case 248: PIXEL00_21; PIXEL01_22; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 74: case 107: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_21; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_22; break; \
	case 27: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_10; PIXEL10_22; PIXEL11_21; break; \
	case 86: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_21; PIXEL11_10; break; \
	case 216: PIXEL00_21; PIXEL01_22; PIXEL10_10; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 106: PIXEL00_10; PIXEL01_21; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_22; break; \
	case 30: PIXEL00_10; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_22; PIXEL11_21; break; \
	case 210: PIXEL00_22; PIXEL01_10; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 120: PIXEL00_21; PIXEL01_22; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_10; break; \
	case 75: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_21; PIXEL10_10; PIXEL11_22; break; \
	case 29: PIXEL00_12; PIXEL01_11; PIXEL10_22; PIXEL11_21; break; \
	case 198: PIXEL00_22; PIXEL01_12; PIXEL10_21; PIXEL11_11; break; \
	case 184: PIXEL00_21; PIXEL01_22; PIXEL10_11; PIXEL11_12; break; \
	case 99: PIXEL00_11; PIXEL01_21; PIXEL10_12; PIXEL11_22; break; \
	case 57: PIXEL00_12; PIXEL01_22; PIXEL10_11; PIXEL11_21; break; \
	case 71: PIXEL00_11; PIXEL01_12; PIXEL10_21; PIXEL11_22; break; \
	case 156: PIXEL00_21; PIXEL01_11; PIXEL10_22; PIXEL11_12; break; \
	case 226: PIXEL00_22; PIXEL01_21; PIXEL10_12; PIXEL11_11; break; \
	case 60: PIXEL00_21; PIXEL01_11; PIXEL10_11; PIXEL11_21; break; \
	case 195: PIXEL00_11; PIXEL01_21; PIXEL10_21; PIXEL11_11; break; \
	case 102: PIXEL00_22; PIXEL01_12; PIXEL10_12; PIXEL11_22; break; \
	case 153: PIXEL00_12; PIXEL01_22; PIXEL10_22; PIXEL11_12; break; \
	case 58: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_11; PIXEL11_21; break; \
	case 83: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 92: PIXEL00_21; PIXEL01_11; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 202: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; PIXEL01_21; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; PIXEL11_11; break; \
	case 78: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; PIXEL11_22; break; \
	case 154: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_22; PIXEL11_12; break; \
	case 114: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 89: PIXEL00_12; PIXEL01_22; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 90: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 55: case 23: if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL00_11, PIXEL01_0; else PIXEL00_60, PIXEL01_90; PIXEL10_20; PIXEL11_21; break; \
	case 182: case 150: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0, PIXEL11_12; else PIXEL01_90, PIXEL11_61; PIXEL10_20; break; \
	case 213: case 212: PIXEL00_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL01_11, PIXEL11_0; else PIXEL01_60, PIXEL11_90; PIXEL10_21; break; \
	case 241: case 240: PIXEL00_20; PIXEL01_22; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL10_12, PIXEL11_0; else PIXEL10_61, PIXEL11_90; break; \
	case 236: case 232: PIXEL00_21; PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0, PIXEL11_11; else PIXEL10_90, PIXEL11_60; break; \
	case 109: case 105: if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL00_12, PIXEL10_0; else PIXEL00_61, PIXEL10_90; PIXEL01_20; PIXEL11_22; break; \
	case 171: case 43: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0, PIXEL10_11; else PIXEL00_90, PIXEL10_60; PIXEL01_21; PIXEL11_20; break; \
	case 143: case 15: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0, PIXEL01_12; else PIXEL00_90, PIXEL01_61; PIXEL10_22; PIXEL11_20; break; \
	case 124: PIXEL00_21; PIXEL01_11; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_10; break; \
	case 203: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_21; PIXEL10_10; PIXEL11_11; break; \
	case 62: PIXEL00_10; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_11; PIXEL11_21; break; \
	case 211: PIXEL00_11; PIXEL01_10; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 118: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_12; PIXEL11_10; break; \
	case 217: PIXEL00_12; PIXEL01_22; PIXEL10_10; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 110: PIXEL00_10; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_22; break; \
	case 155: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_10; PIXEL10_22; PIXEL11_12; break; \
	case 188: PIXEL00_21; PIXEL01_11; PIXEL10_11; PIXEL11_12; break; \
	case 185: PIXEL00_12; PIXEL01_22; PIXEL10_11; PIXEL11_12; break; \
	case 61: PIXEL00_12; PIXEL01_11; PIXEL10_11; PIXEL11_21; break; \
	case 157: PIXEL00_12; PIXEL01_11; PIXEL10_22; PIXEL11_12; break; \
	case 103: PIXEL00_11; PIXEL01_12; PIXEL10_12; PIXEL11_22; break; \
	case 227: PIXEL00_11; PIXEL01_21; PIXEL10_12; PIXEL11_11; break; \
	case 230: PIXEL00_22; PIXEL01_12; PIXEL10_12; PIXEL11_11; break; \
	case 199: PIXEL00_11; PIXEL01_12; PIXEL10_21; PIXEL11_11; break; \
	case 220: PIXEL00_21; PIXEL01_11; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 158: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_22; PIXEL11_12; break; \
	case 234: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; PIXEL01_21; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_11; break; \
	case 242: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 59: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_11; PIXEL11_21; break; \
	case 121: PIXEL00_12; PIXEL01_22; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 87: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 79: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; PIXEL11_22; break; \
	case 122: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 94: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 218: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 91: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 229: PIXEL00_20; PIXEL01_20; PIXEL10_12; PIXEL11_11; break; \
	case 167: PIXEL00_11; PIXEL01_12; PIXEL10_20; PIXEL11_20; break; \
	case 173: PIXEL00_12; PIXEL01_20; PIXEL10_11; PIXEL11_20; break; \
	case 181: PIXEL00_20; PIXEL01_11; PIXEL10_20; PIXEL11_12; break; \
	case 186: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_11; PIXEL11_12; break; \
	case 115: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 93: PIXEL00_12; PIXEL01_11; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 206: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; PIXEL11_11; break; \
	case 205: case 201: PIXEL00_12; PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_10; else PIXEL10_70; PIXEL11_11; break; \
	case 174: case 46: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_10; else PIXEL00_70; PIXEL01_12; PIXEL10_11; PIXEL11_20; break; \
	case 179: case 147: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_10; else PIXEL01_70; PIXEL10_20; PIXEL11_12; break; \
	case 117: case 116: PIXEL00_20; PIXEL01_11; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_10; else PIXEL11_70; break; \
	case 189: PIXEL00_12; PIXEL01_11; PIXEL10_11; PIXEL11_12; break; \
	case 231: PIXEL00_11; PIXEL01_12; PIXEL10_12; PIXEL11_11; break; \
	case 126: PIXEL00_10; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_10; break; \
	case 219: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_10; PIXEL10_10; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 125: if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL00_12, PIXEL10_0; else PIXEL00_61, PIXEL10_90; PIXEL01_11; PIXEL11_10; break; \
	case 221: PIXEL00_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL01_11, PIXEL11_0; else PIXEL01_60, PIXEL11_90; PIXEL10_10; break; \
	case 207: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0, PIXEL01_12; else PIXEL00_90, PIXEL01_61; PIXEL10_10; PIXEL11_11; break; \
	case 238: PIXEL00_10; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0, PIXEL11_11; else PIXEL10_90, PIXEL11_60; break; \
	case 190: PIXEL00_10; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0, PIXEL11_12; else PIXEL01_90, PIXEL11_61; PIXEL10_11; break; \
	case 187: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0, PIXEL10_11; else PIXEL00_90, PIXEL10_60; PIXEL01_10; PIXEL11_12; break; \
	case 243: PIXEL00_11; PIXEL01_10; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL10_12, PIXEL11_0; else PIXEL10_61, PIXEL11_90; break; \
	case 119: if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL00_11, PIXEL01_0; else PIXEL00_60, PIXEL01_90; PIXEL10_12; PIXEL11_10; break; \
	case 237: case 233: PIXEL00_12; PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; PIXEL11_11; break; \
	case 175: case 47: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; PIXEL01_12; PIXEL10_11; PIXEL11_20; break; \
	case 183: case 151: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; PIXEL10_20; PIXEL11_12; break; \
	case 245: case 244: PIXEL00_20; PIXEL01_11; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break; \
	case 250: PIXEL00_10; PIXEL01_10; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 123: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_10; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_10; break; \
	case 95: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_10; PIXEL11_10; break; \
	case 222: PIXEL00_10; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_10; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 252: PIXEL00_21; PIXEL01_11; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break; \
	case 249: PIXEL00_12; PIXEL01_22; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 235: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_21; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; PIXEL11_11; break; \
	case 111: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_22; break; \
	case 63: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_11; PIXEL11_21; break; \
	case 159: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; PIXEL10_22; PIXEL11_12; break; \
	case 215: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; PIXEL10_21; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 246: PIXEL00_22; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break; \
	case 254: PIXEL00_10; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break; \
	case 253: PIXEL00_12; PIXEL01_11; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break; \
	case 251: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; PIXEL01_10; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 239: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; PIXEL01_12; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; PIXEL11_11; break; \
	case 127: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_20; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_20; PIXEL11_10; break; \
	case 191: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; PIXEL10_11; PIXEL11_12; break; \
	case 223: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_20; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; PIXEL10_10; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_20; break; \
	case 247: PIXEL00_11; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; PIXEL10_12; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break; \
	case 255: if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) PIXEL00_0; else PIXEL00_100; if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) PIXEL01_0; else PIXEL01_100; if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) PIXEL10_0; else PIXEL10_100; if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) PIXEL11_0; else PIXEL11_100; break;


void RenderHQ2XS(unsigned char *src, unsigned int srcpitch, unsigned char *dst, unsigned int dstpitch, int nWidth, int nHeight, int type)
{
	uint8 *srcPtr = src;
	uint8 *dstPtr = dst;
	uint32 srcPitch = srcpitch;
	uint32 dstPitch = dstpitch;
	int width = nWidth;
	int height = nHeight;

	int	w1, w2, w3, w4, w5, w6, w7, w8, w9;

	uint32 src1line = srcPitch >> 1;
	uint32 dst1line = dstPitch >> 1;
	uint16 *sp = (uint16 *) srcPtr;
	uint16 *dp = (uint16 *) dstPtr;

	const int* RGBtoYUVtable = RGBtoYUV;

	uint32 pattern;
	int l, y;

	while (height--)
	{
		sp--;

		w1 = *(sp - src1line);
		w4 = *(sp);
		w7 = *(sp + src1line);

		sp++;

		w2 = *(sp - src1line);
		w5 = *(sp);
		w8 = *(sp + src1line);

		for (l = width; l; l--)
		{
			sp++;

			w3 = *(sp - src1line);
			w6 = *(sp);
			w9 = *(sp + src1line);

			pattern = 0;

			if (type == 0)	// hq2xs
			{
				bool nosame = true;
				if(w1 == w5 || w3 == w5 || w7 == w5 || w9 == w5)
					nosame = false;

				if(nosame)
				{
					const uint16 avg = (RGBtoBright[w1] + RGBtoBright[w2] + RGBtoBright[w3] + RGBtoBright[w4] + RGBtoBright[w5] + RGBtoBright[w6] + RGBtoBright[w7] + RGBtoBright[w8] + RGBtoBright[w9]) / 9;
					const bool diff5 = RGBtoBright[w5] > avg;
					if((RGBtoBright[w1] > avg) != diff5) pattern |= (1 << 0);
					if((RGBtoBright[w2] > avg) != diff5) pattern |= (1 << 1);
					if((RGBtoBright[w3] > avg) != diff5) pattern |= (1 << 2);
					if((RGBtoBright[w4] > avg) != diff5) pattern |= (1 << 3);
					if((RGBtoBright[w6] > avg) != diff5) pattern |= (1 << 4);
					if((RGBtoBright[w7] > avg) != diff5) pattern |= (1 << 5);
					if((RGBtoBright[w8] > avg) != diff5) pattern |= (1 << 6);
					if((RGBtoBright[w9] > avg) != diff5) pattern |= (1 << 7);
				}
				else
				{
					y = RGBtoYUV[w5];
					if ((w1 != w5) && (Diff(y, RGBtoYUV[w1]))) pattern |= (1 << 0);
					if ((w2 != w5) && (Diff(y, RGBtoYUV[w2]))) pattern |= (1 << 1);
					if ((w3 != w5) && (Diff(y, RGBtoYUV[w3]))) pattern |= (1 << 2);
					if ((w4 != w5) && (Diff(y, RGBtoYUV[w4]))) pattern |= (1 << 3);
					if ((w6 != w5) && (Diff(y, RGBtoYUV[w6]))) pattern |= (1 << 4);
					if ((w7 != w5) && (Diff(y, RGBtoYUV[w7]))) pattern |= (1 << 5);
					if ((w8 != w5) && (Diff(y, RGBtoYUV[w8]))) pattern |= (1 << 6);
					if ((w9 != w5) && (Diff(y, RGBtoYUV[w9]))) pattern |= (1 << 7);
				}
			}
			else if (type == 1)	// hq2xbold
			{
				const uint16 avg = (RGBtoBright[w1] + RGBtoBright[w2] + RGBtoBright[w3] + RGBtoBright[w4] + RGBtoBright[w5] + RGBtoBright[w6] + RGBtoBright[w7] + RGBtoBright[w8] + RGBtoBright[w9]) / 9;
				const bool diff5 = RGBtoBright[w5] > avg;
				if ((w1 != w5) && ((RGBtoBright[w1] > avg) != diff5)) pattern |= (1 << 0);
				if ((w2 != w5) && ((RGBtoBright[w2] > avg) != diff5)) pattern |= (1 << 1);
				if ((w3 != w5) && ((RGBtoBright[w3] > avg) != diff5)) pattern |= (1 << 2);
				if ((w4 != w5) && ((RGBtoBright[w4] > avg) != diff5)) pattern |= (1 << 3);
				if ((w6 != w5) && ((RGBtoBright[w6] > avg) != diff5)) pattern |= (1 << 4);
				if ((w7 != w5) && ((RGBtoBright[w7] > avg) != diff5)) pattern |= (1 << 5);
				if ((w8 != w5) && ((RGBtoBright[w8] > avg) != diff5)) pattern |= (1 << 6);
				if ((w9 != w5) && ((RGBtoBright[w9] > avg) != diff5)) pattern |= (1 << 7);
			}

			switch (pattern)
			{
				HQ2XCASES
			}

			w1 = w2; w4 = w5; w7 = w8;
			w2 = w3; w5 = w6; w8 = w9;

			dp += 2;
		}

		dp += ((dst1line - width) << 1);
		sp +=  (src1line - width);
	}
}


// HQ3xS

#define PIXEL00_1M  *(dp) = Interp01(w5, w1);
#define PIXEL00_1U  *(dp) = Interp01(w5, w2);
#define PIXEL00_1L  *(dp) = Interp01(w5, w4);
#define PIXEL00_2   *(dp) = Interp02(w5, w4, w2);
#define PIXEL00_4   *(dp) = Interp04(w5, w4, w2);
#define PIXEL00_5   *(dp) = Interp05(w4, w2);
#define PIXEL00_C   *(dp) = w5;

#define PIXEL01_1   *(dp + 1) = Interp01(w5, w2);
#define PIXEL01_3   *(dp + 1) = Interp03(w5, w2);
#define PIXEL01_6   *(dp + 1) = Interp01(w2, w5);
#define PIXEL01_C   *(dp + 1) = w5;

#define PIXEL02_1M  *(dp + 2) = Interp01(w5, w3);
#define PIXEL02_1U  *(dp + 2) = Interp01(w5, w2);
#define PIXEL02_1R  *(dp + 2) = Interp01(w5, w6);
#define PIXEL02_2   *(dp + 2) = Interp02(w5, w2, w6);
#define PIXEL02_4   *(dp + 2) = Interp04(w5, w2, w6);
#define PIXEL02_5   *(dp + 2) = Interp05(w2, w6);
#define PIXEL02_C   *(dp + 2) = w5;

#define PIXEL10_1   *(dp + dst1line) = Interp01(w5, w4);
#define PIXEL10_3   *(dp + dst1line) = Interp03(w5, w4);
#define PIXEL10_6   *(dp + dst1line) = Interp01(w4, w5);
#define PIXEL10_C   *(dp + dst1line) = w5;

#define PIXEL11     *(dp + dst1line + 1) = w5;

#define PIXEL12_1   *(dp + dst1line + 2) = Interp01(w5, w6);
#define PIXEL12_3   *(dp + dst1line + 2) = Interp03(w5, w6);
#define PIXEL12_6   *(dp + dst1line + 2) = Interp01(w6, w5);
#define PIXEL12_C   *(dp + dst1line + 2) = w5;

#define PIXEL20_1M  *(dp + dst1line + dst1line) = Interp01(w5, w7);
#define PIXEL20_1D  *(dp + dst1line + dst1line) = Interp01(w5, w8);
#define PIXEL20_1L  *(dp + dst1line + dst1line) = Interp01(w5, w4);
#define PIXEL20_2   *(dp + dst1line + dst1line) = Interp02(w5, w8, w4);
#define PIXEL20_4   *(dp + dst1line + dst1line) = Interp04(w5, w8, w4);
#define PIXEL20_5   *(dp + dst1line + dst1line) = Interp05(w8, w4);
#define PIXEL20_C   *(dp + dst1line + dst1line) = w5;

#define PIXEL21_1   *(dp + dst1line + dst1line + 1) = Interp01(w5, w8);
#define PIXEL21_3   *(dp + dst1line + dst1line + 1) = Interp03(w5, w8);
#define PIXEL21_6   *(dp + dst1line + dst1line + 1) = Interp01(w8, w5);
#define PIXEL21_C   *(dp + dst1line + dst1line + 1) = w5;

#define PIXEL22_1M  *(dp + dst1line + dst1line + 2) = Interp01(w5, w9);
#define PIXEL22_1D  *(dp + dst1line + dst1line + 2) = Interp01(w5, w8);
#define PIXEL22_1R  *(dp + dst1line + dst1line + 2) = Interp01(w5, w6);
#define PIXEL22_2   *(dp + dst1line + dst1line + 2) = Interp02(w5, w6, w8);
#define PIXEL22_4   *(dp + dst1line + dst1line + 2) = Interp04(w5, w6, w8);
#define PIXEL22_5   *(dp + dst1line + dst1line + 2) = Interp05(w6, w8);
#define PIXEL22_C   *(dp + dst1line + dst1line + 2) = w5;

#define Interp03(c1, c2) \
	(((c1 == c2) ? c1 : \
	((((((c1) & Mask_2) *  7 + ((c2) & Mask_2)) >> 3) & Mask_2) + \
	(((((c1) & Mask13) *  7 + ((c2) & Mask13)) >> 3) & Mask13))))

#define Interp04(c1, c2, c3) \
	((((((c1) & Mask_2) *  2 + ((c2) & Mask_2) * 7 + ((c3) & Mask_2) * 7) >> 4) & Mask_2) + \
	(((((c1) & Mask13) *  2 + ((c2) & Mask13) * 7 + ((c3) & Mask13) * 7) >> 4) & Mask13))

#define HQ3XCASES \
	case 0: case 1: case 4: case 32: case 128: case 5: case 132: case 160: case 33: case 129: case 36: case 133: case 164: case 161: case 37: case 165: { PIXEL00_2 PIXEL01_1 PIXEL02_2 PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_2 PIXEL21_1 PIXEL22_2 break; }\
	case 2: case 34: case 130: case 162: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_2 PIXEL21_1 PIXEL22_2 break; }\
	case 16: case 17: case 48: case 49: { PIXEL00_2 PIXEL01_1 PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1M break; }\
	case 64: case 65: case 68: case 69: { PIXEL00_2 PIXEL01_1 PIXEL02_2 PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 8: case 12: case 136: case 140: { PIXEL00_1M PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_1 PIXEL22_2 break; }\
	case 3: case 35: case 131: case 163: { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_2 PIXEL21_1 PIXEL22_2 break; }\
	case 6: case 38: case 134: case 166: { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_2 PIXEL21_1 PIXEL22_2 break; }\
	case 20: case 21: case 52: case 53: { PIXEL00_2 PIXEL01_1 PIXEL02_1U PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1M break; }\
	case 144: case 145: case 176: case 177: { PIXEL00_2 PIXEL01_1 PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1D break; }\
	case 192: case 193: case 196: case 197: { PIXEL00_2 PIXEL01_1 PIXEL02_2 PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 96: case 97: case 100: case 101: { PIXEL00_2 PIXEL01_1 PIXEL02_2 PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 40: case 44: case 168: case 172: { PIXEL00_1M PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1D PIXEL21_1 PIXEL22_2 break; }\
	case 9: case 13: case 137: case 141: { PIXEL00_1U PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_1 PIXEL22_2 break; }\
	case 18: case 50: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_1M PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_1 PIXEL11 PIXEL20_2 PIXEL21_1 PIXEL22_1M break; }\
	case 80: case 81: { PIXEL00_2 PIXEL01_1 PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_1M } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 72: case 76: { PIXEL00_1M PIXEL01_1 PIXEL02_2 PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_1M PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 10: case 138: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_1 PIXEL22_2 break; }\
	case 66: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 24: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 7: case 39: case 135: { PIXEL00_1L PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_2 PIXEL21_1 PIXEL22_2 break; }\
	case 148: case 149: case 180: { PIXEL00_2 PIXEL01_1 PIXEL02_1U PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1D break; }\
	case 224: case 228: case 225: { PIXEL00_2 PIXEL01_1 PIXEL02_2 PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1R break; }\
	case 41: case 169: case 45: { PIXEL00_1U PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1D PIXEL21_1 PIXEL22_2 break; }\
	case 22: case 54: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_1 PIXEL11 PIXEL20_2 PIXEL21_1 PIXEL22_1M break; }\
	case 208: case 209: { PIXEL00_2 PIXEL01_1 PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 104: case 108: { PIXEL00_1M PIXEL01_1 PIXEL02_2 PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 11: case 139: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_1 PIXEL22_2 break; }\
	case 19: case 51: { if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL12_C } else { PIXEL00_2 PIXEL01_6 PIXEL02_5 PIXEL12_1 } PIXEL10_1 PIXEL11 PIXEL20_2 PIXEL21_1 PIXEL22_1M break; }\
	case 146: case 178: { if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_1M PIXEL12_C PIXEL22_1D } else { PIXEL01_1 PIXEL02_5 PIXEL12_6 PIXEL22_2 } PIXEL00_1M PIXEL10_1 PIXEL11 PIXEL20_2 PIXEL21_1 break; }\
	case 84: case 85: { if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL02_1U PIXEL12_C PIXEL21_C PIXEL22_1M } else { PIXEL02_2 PIXEL12_6 PIXEL21_1 PIXEL22_5 } PIXEL00_2 PIXEL01_1 PIXEL10_1 PIXEL11 PIXEL20_1M break; }\
	case 112: case 113: { if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL20_1L PIXEL21_C PIXEL22_1M } else { PIXEL12_1 PIXEL20_2 PIXEL21_6 PIXEL22_5 } PIXEL00_2 PIXEL01_1 PIXEL02_1M PIXEL10_1 PIXEL11 break; }\
	case 200: case 204: { if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_1M PIXEL21_C PIXEL22_1R } else { PIXEL10_1 PIXEL20_5 PIXEL21_6 PIXEL22_2 } PIXEL00_1M PIXEL01_1 PIXEL02_2 PIXEL11 PIXEL12_1 break; }\
	case 73: case 77: { if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL00_1U PIXEL10_C PIXEL20_1M PIXEL21_C } else { PIXEL00_2 PIXEL10_6 PIXEL20_5 PIXEL21_1 } PIXEL01_1 PIXEL02_2 PIXEL11 PIXEL12_1 PIXEL22_1M break; }\
	case 42: case 170: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M PIXEL01_C PIXEL10_C PIXEL20_1D } else { PIXEL00_5 PIXEL01_1 PIXEL10_6 PIXEL20_2 } PIXEL02_1M PIXEL11 PIXEL12_1 PIXEL21_1 PIXEL22_2 break; }\
	case 14: case 142: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL10_C } else { PIXEL00_5 PIXEL01_6 PIXEL02_2 PIXEL10_1 } PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_1 PIXEL22_2 break; }\
	case 67: { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 70: { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 28: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 152: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 194: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 98: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 56: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 25: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 26: case 31: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL10_C } else { PIXEL00_4 PIXEL10_3 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C PIXEL12_C } else { PIXEL02_4 PIXEL12_3 } PIXEL11 PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 82: case 214: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C } else { PIXEL01_3 PIXEL02_4 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL21_C PIXEL22_C } else { PIXEL21_3 PIXEL22_4 } break; }\
	case 88: case 248: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C } else { PIXEL10_3 PIXEL20_4 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL22_C } else { PIXEL12_3 PIXEL22_4 } break; }\
	case 74: case 107: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C } else { PIXEL00_4 PIXEL01_3 } PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C PIXEL21_C } else { PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 27: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 86: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_1 PIXEL11 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 216: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 106: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 30: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_C PIXEL11 PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 210: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 120: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 75: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 29: { PIXEL00_1U PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1M break; }\
	case 198: { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 184: { PIXEL00_1M PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1D break; }\
	case 99: { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 57: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 71: { PIXEL00_1L PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 156: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 226: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1R break; }\
	case 60: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 195: { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 102: { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 153: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 58: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 83: { PIXEL00_1L PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 92: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 202: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C PIXEL22_1R break; }\
	case 78: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C PIXEL02_1R PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C PIXEL22_1M break; }\
	case 154: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 114: { PIXEL00_1M PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1L PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 89: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 90: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 55: case 23: { if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL00_1L PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL00_2 PIXEL01_6 PIXEL02_5 PIXEL12_1 } PIXEL10_1 PIXEL11 PIXEL20_2 PIXEL21_1 PIXEL22_1M break; }\
	case 182: case 150: { if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C PIXEL22_1D } else { PIXEL01_1 PIXEL02_5 PIXEL12_6 PIXEL22_2 } PIXEL00_1M PIXEL10_1 PIXEL11 PIXEL20_2 PIXEL21_1 break; }\
	case 213: case 212: { if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL02_1U PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL02_2 PIXEL12_6 PIXEL21_1 PIXEL22_5 } PIXEL00_2 PIXEL01_1 PIXEL10_1 PIXEL11 PIXEL20_1M break; }\
	case 241: case 240: { if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL20_1L PIXEL21_C PIXEL22_C } else { PIXEL12_1 PIXEL20_2 PIXEL21_6 PIXEL22_5 } PIXEL00_2 PIXEL01_1 PIXEL02_1M PIXEL10_1 PIXEL11 break; }\
	case 236: case 232: { if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C PIXEL22_1R } else { PIXEL10_1 PIXEL20_5 PIXEL21_6 PIXEL22_2 } PIXEL00_1M PIXEL01_1 PIXEL02_2 PIXEL11 PIXEL12_1 break; }\
	case 109: case 105: { if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL00_1U PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL00_2 PIXEL10_6 PIXEL20_5 PIXEL21_1 } PIXEL01_1 PIXEL02_2 PIXEL11 PIXEL12_1 PIXEL22_1M break; }\
	case 171: case 43: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C PIXEL20_1D } else { PIXEL00_5 PIXEL01_1 PIXEL10_6 PIXEL20_2 } PIXEL02_1M PIXEL11 PIXEL12_1 PIXEL21_1 PIXEL22_2 break; }\
	case 143: case 15: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL02_1R PIXEL10_C } else { PIXEL00_5 PIXEL01_6 PIXEL02_2 PIXEL10_1 } PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_1 PIXEL22_2 break; }\
	case 124: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 203: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 62: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_C PIXEL11 PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 211: { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 118: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_1 PIXEL11 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 217: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 110: { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 155: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 188: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1D break; }\
	case 185: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1D break; }\
	case 61: { PIXEL00_1U PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 157: { PIXEL00_1U PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 103: { PIXEL00_1L PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 227: { PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1R break; }\
	case 230: { PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1R break; }\
	case 199: { PIXEL00_1L PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 220: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 158: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_C PIXEL11 PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 234: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C PIXEL02_1M PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1R break; }\
	case 242: { PIXEL00_1M PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL20_1L if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 59: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 121: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 87: { PIXEL00_1L if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_1 PIXEL11 PIXEL20_1M PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 79: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1R PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C PIXEL22_1M break; }\
	case 122: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 94: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL10_C PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 218: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_C PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 91: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 229: { PIXEL00_2 PIXEL01_1 PIXEL02_2 PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1R break; }\
	case 167: { PIXEL00_1L PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_2 PIXEL21_1 PIXEL22_2 break; }\
	case 173: { PIXEL00_1U PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1D PIXEL21_1 PIXEL22_2 break; }\
	case 181: { PIXEL00_2 PIXEL01_1 PIXEL02_1U PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1D break; }\
	case 186: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1D break; }\
	case 115: { PIXEL00_1L PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1L PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 93: { PIXEL00_1U PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 206: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C PIXEL02_1R PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C PIXEL22_1R break; }\
	case 205: case 201: { PIXEL00_1U PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_1M } else { PIXEL20_2 } PIXEL21_C PIXEL22_1R break; }\
	case 174: case 46: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_1M } else { PIXEL00_2 } PIXEL01_C PIXEL02_1R PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1D PIXEL21_1 PIXEL22_2 break; }\
	case 179: case 147: { PIXEL00_1L PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_1M } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1D break; }\
	case 117: case 116: { PIXEL00_2 PIXEL01_1 PIXEL02_1U PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1L PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_1M } else { PIXEL22_2 } break; }\
	case 189: { PIXEL00_1U PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1D break; }\
	case 231: { PIXEL00_1L PIXEL01_C PIXEL02_1R PIXEL10_1 PIXEL11 PIXEL12_1 PIXEL20_1L PIXEL21_C PIXEL22_1R break; }\
	case 126: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_4 PIXEL12_3 } PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 219: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_4 PIXEL01_3 PIXEL10_3 } PIXEL02_1M PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_4 } break; }\
	case 125: { if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL00_1U PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL00_2 PIXEL10_6 PIXEL20_5 PIXEL21_1 } PIXEL01_1 PIXEL02_1U PIXEL11 PIXEL12_C PIXEL22_1M break; }\
	case 221: { if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL02_1U PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL02_2 PIXEL12_6 PIXEL21_1 PIXEL22_5 } PIXEL00_1U PIXEL01_1 PIXEL10_C PIXEL11 PIXEL20_1M break; }\
	case 207: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL02_1R PIXEL10_C } else { PIXEL00_5 PIXEL01_6 PIXEL02_2 PIXEL10_1 } PIXEL11 PIXEL12_1 PIXEL20_1M PIXEL21_C PIXEL22_1R break; }\
	case 238: { if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C PIXEL22_1R } else { PIXEL10_1 PIXEL20_5 PIXEL21_6 PIXEL22_2 } PIXEL00_1M PIXEL01_C PIXEL02_1R PIXEL11 PIXEL12_1 break; }\
	case 190: { if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C PIXEL22_1D } else { PIXEL01_1 PIXEL02_5 PIXEL12_6 PIXEL22_2 } PIXEL00_1M PIXEL10_C PIXEL11 PIXEL20_1D PIXEL21_1 break; }\
	case 187: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C PIXEL20_1D } else { PIXEL00_5 PIXEL01_1 PIXEL10_6 PIXEL20_2 } PIXEL02_1M PIXEL11 PIXEL12_C PIXEL21_1 PIXEL22_1D break; }\
	case 243: { if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL20_1L PIXEL21_C PIXEL22_C } else { PIXEL12_1 PIXEL20_2 PIXEL21_6 PIXEL22_5 } PIXEL00_1L PIXEL01_C PIXEL02_1M PIXEL10_1 PIXEL11 break; }\
	case 119: { if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL00_1L PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL00_2 PIXEL01_6 PIXEL02_5 PIXEL12_1 } PIXEL10_1 PIXEL11 PIXEL20_1L PIXEL21_C PIXEL22_1M break; }\
	case 237: case 233: { PIXEL00_1U PIXEL01_1 PIXEL02_2 PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C } else { PIXEL20_2 } PIXEL21_C PIXEL22_1R break; }\
	case 175: case 47: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C } else { PIXEL00_2 } PIXEL01_C PIXEL02_1R PIXEL10_C PIXEL11 PIXEL12_1 PIXEL20_1D PIXEL21_1 PIXEL22_2 break; }\
	case 183: case 151: { PIXEL00_1L PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_2 PIXEL21_1 PIXEL22_1D break; }\
	case 245: case 244: { PIXEL00_2 PIXEL01_1 PIXEL02_1U PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1L PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_C } else { PIXEL22_2 } break; }\
	case 250: { PIXEL00_1M PIXEL01_C PIXEL02_1M PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C } else { PIXEL10_3 PIXEL20_4 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL22_C } else { PIXEL12_3 PIXEL22_4 } break; }\
	case 123: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C } else { PIXEL00_4 PIXEL01_3 } PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C PIXEL21_C } else { PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 95: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL10_C } else { PIXEL00_4 PIXEL10_3 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C PIXEL12_C } else { PIXEL02_4 PIXEL12_3 } PIXEL11 PIXEL20_1M PIXEL21_C PIXEL22_1M break; }\
	case 222: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C } else { PIXEL01_3 PIXEL02_4 } PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL21_C PIXEL22_C } else { PIXEL21_3 PIXEL22_4 } break; }\
	case 252: { PIXEL00_1M PIXEL01_1 PIXEL02_1U PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C } else { PIXEL10_3 PIXEL20_4 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_C } else { PIXEL22_2 } break; }\
	case 249: { PIXEL00_1U PIXEL01_1 PIXEL02_1M PIXEL10_C PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL22_C } else { PIXEL12_3 PIXEL22_4 } break; }\
	case 235: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C } else { PIXEL00_4 PIXEL01_3 } PIXEL02_1M PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C } else { PIXEL20_2 } PIXEL21_C PIXEL22_1R break; }\
	case 111: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C } else { PIXEL00_2 } PIXEL01_C PIXEL02_1R PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C PIXEL21_C } else { PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 63: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C PIXEL12_C } else { PIXEL02_4 PIXEL12_3 } PIXEL10_C PIXEL11 PIXEL20_1D PIXEL21_1 PIXEL22_1M break; }\
	case 159: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL10_C } else { PIXEL00_4 PIXEL10_3 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C } else { PIXEL02_2 } PIXEL11 PIXEL12_C PIXEL20_1M PIXEL21_1 PIXEL22_1D break; }\
	case 215: { PIXEL00_1L PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL21_C PIXEL22_C } else { PIXEL21_3 PIXEL22_4 } break; }\
	case 246: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C } else { PIXEL01_3 PIXEL02_4 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1L PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_C } else { PIXEL22_2 } break; }\
	case 254: { PIXEL00_1M if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C } else { PIXEL01_3 PIXEL02_4 } PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C } else { PIXEL10_3 PIXEL20_4 } if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL21_C PIXEL22_C } else { PIXEL12_3 PIXEL21_3 PIXEL22_2 } break; }\
	case 253: { PIXEL00_1U PIXEL01_1 PIXEL02_1U PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_C } else { PIXEL22_2 } break; }\
	case 251: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C } else { PIXEL00_4 PIXEL01_3 } PIXEL02_1M PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL10_C PIXEL20_C PIXEL21_C } else { PIXEL10_3 PIXEL20_2 PIXEL21_3 } if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL12_C PIXEL22_C } else { PIXEL12_3 PIXEL22_4 } break; }\
	case 239: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C } else { PIXEL00_2 } PIXEL01_C PIXEL02_1R PIXEL10_C PIXEL11 PIXEL12_1 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C } else { PIXEL20_2 } PIXEL21_C PIXEL22_1R break; }\
	case 127: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL01_C PIXEL10_C } else { PIXEL00_2 PIXEL01_3 PIXEL10_3 } if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C PIXEL12_C } else { PIXEL02_4 PIXEL12_3 } PIXEL11 if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C PIXEL21_C } else { PIXEL20_4 PIXEL21_3 } PIXEL22_1M break; }\
	case 191: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C } else { PIXEL02_2 } PIXEL10_C PIXEL11 PIXEL12_C PIXEL20_1D PIXEL21_1 PIXEL22_1D break; }\
	case 223: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C PIXEL10_C } else { PIXEL00_4 PIXEL10_3 } if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL01_C PIXEL02_C PIXEL12_C } else { PIXEL01_3 PIXEL02_2 PIXEL12_3 } PIXEL11 PIXEL20_1M if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL21_C PIXEL22_C } else { PIXEL21_3 PIXEL22_4 } break; }\
	case 247: { PIXEL00_1L PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C } else { PIXEL02_2 } PIXEL10_1 PIXEL11 PIXEL12_C PIXEL20_1L PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_C } else { PIXEL22_2 } break; }\
	case 255: { if (Diff(RGBtoYUVtable[w4], RGBtoYUVtable[w2])) { PIXEL00_C } else { PIXEL00_2 } PIXEL01_C if (Diff(RGBtoYUVtable[w2], RGBtoYUVtable[w6])) { PIXEL02_C } else { PIXEL02_2 } PIXEL10_C PIXEL11 PIXEL12_C if (Diff(RGBtoYUVtable[w8], RGBtoYUVtable[w4])) { PIXEL20_C } else { PIXEL20_2 } PIXEL21_C if (Diff(RGBtoYUVtable[w6], RGBtoYUVtable[w8])) { PIXEL22_C } else { PIXEL22_2 } break; }


void RenderHQ3XS(unsigned char *src, unsigned int srcpitch, unsigned char *dst, unsigned int dstpitch, int nWidth, int nHeight, int type)
{
	uint8 *srcPtr = src;
	uint8 *dstPtr = dst;
	uint32 srcPitch = srcpitch;
	uint32 dstPitch = dstpitch;
	int width = nWidth;
	int height = nHeight;

	int	w1, w2, w3, w4, w5, w6, w7, w8, w9;

	uint32 src1line = srcPitch >> 1;
	uint32 dst1line = dstPitch >> 1;
	uint16 *sp = (uint16 *) srcPtr;
	uint16 *dp = (uint16 *) dstPtr;

	const int* RGBtoYUVtable = RGBtoYUV;

	uint32 pattern;
	int l, y;

	while (height--)
	{
		sp--;

		w1 = *(sp - src1line);
		w4 = *(sp);
		w7 = *(sp + src1line);

		sp++;

		w2 = *(sp - src1line);
		w5 = *(sp);
		w8 = *(sp + src1line);

		for (l = width; l; l--)
		{
			sp++;

			w3 = *(sp - src1line);
			w6 = *(sp);
			w9 = *(sp + src1line);

			pattern = 0;

			if (type == 0)	// hq3xs
			{
				bool nosame = true;
				if(w1 == w5 || w3 == w5 || w7 == w5 || w9 == w5)
					nosame = false;

				if(nosame)
				{
					const uint16 avg = (RGBtoBright[w1] + RGBtoBright[w2] + RGBtoBright[w3] + RGBtoBright[w4] + RGBtoBright[w5] + RGBtoBright[w6] + RGBtoBright[w7] + RGBtoBright[w8] + RGBtoBright[w9]) / 9;
					const bool diff5 = RGBtoBright[w5] > avg;
					if((RGBtoBright[w1] > avg) != diff5) pattern |= (1 << 0);
					if((RGBtoBright[w2] > avg) != diff5) pattern |= (1 << 1);
					if((RGBtoBright[w3] > avg) != diff5) pattern |= (1 << 2);
					if((RGBtoBright[w4] > avg) != diff5) pattern |= (1 << 3);
					if((RGBtoBright[w6] > avg) != diff5) pattern |= (1 << 4);
					if((RGBtoBright[w7] > avg) != diff5) pattern |= (1 << 5);
					if((RGBtoBright[w8] > avg) != diff5) pattern |= (1 << 6);
					if((RGBtoBright[w9] > avg) != diff5) pattern |= (1 << 7);
				}
				else
				{
					y = RGBtoYUV[w5];
					if ((w1 != w5) && (Diff(y, RGBtoYUV[w1]))) pattern |= (1 << 0);
					if ((w2 != w5) && (Diff(y, RGBtoYUV[w2]))) pattern |= (1 << 1);
					if ((w3 != w5) && (Diff(y, RGBtoYUV[w3]))) pattern |= (1 << 2);
					if ((w4 != w5) && (Diff(y, RGBtoYUV[w4]))) pattern |= (1 << 3);
					if ((w6 != w5) && (Diff(y, RGBtoYUV[w6]))) pattern |= (1 << 4);
					if ((w7 != w5) && (Diff(y, RGBtoYUV[w7]))) pattern |= (1 << 5);
					if ((w8 != w5) && (Diff(y, RGBtoYUV[w8]))) pattern |= (1 << 6);
					if ((w9 != w5) && (Diff(y, RGBtoYUV[w9]))) pattern |= (1 << 7);
				}
			}
			else if (type == 1)	// hq3xbold
			{
				const uint16 avg = (RGBtoBright[w1] + RGBtoBright[w2] + RGBtoBright[w3] + RGBtoBright[w4] + RGBtoBright[w5] + RGBtoBright[w6] + RGBtoBright[w7] + RGBtoBright[w8] + RGBtoBright[w9]) / 9;
				const bool diff5 = RGBtoBright[w5] > avg;
				if ((w1 != w5) && ((RGBtoBright[w1] > avg) != diff5)) pattern |= (1 << 0);
				if ((w2 != w5) && ((RGBtoBright[w2] > avg) != diff5)) pattern |= (1 << 1);
				if ((w3 != w5) && ((RGBtoBright[w3] > avg) != diff5)) pattern |= (1 << 2);
				if ((w4 != w5) && ((RGBtoBright[w4] > avg) != diff5)) pattern |= (1 << 3);
				if ((w6 != w5) && ((RGBtoBright[w6] > avg) != diff5)) pattern |= (1 << 4);
				if ((w7 != w5) && ((RGBtoBright[w7] > avg) != diff5)) pattern |= (1 << 5);
				if ((w8 != w5) && ((RGBtoBright[w8] > avg) != diff5)) pattern |= (1 << 6);
				if ((w9 != w5) && ((RGBtoBright[w9] > avg) != diff5)) pattern |= (1 << 7);
			}

			switch (pattern)
			{
				HQ3XCASES
			}

			w1 = w2; w4 = w5; w7 = w8;
			w2 = w3; w5 = w6; w8 = w9;

			dp += 3;
		}

		dp += ((dst1line - width) * 3);
		sp +=  (src1line - width);
	}
}
