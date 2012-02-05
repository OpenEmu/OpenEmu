/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/


#ifndef _DSP1_H_
#define _DSP1_H_

enum
{
	M_DSP1_LOROM_S,
	M_DSP1_LOROM_L,
	M_DSP1_HIROM,
	M_DSP2_LOROM,
	M_DSP3_LOROM,
	M_DSP4_LOROM
};

struct SDSP0
{
	uint32	maptype;
	uint32	boundary;
};

struct SDSP1
{
	bool8	waiting4command;
	bool8	first_parameter;
	uint8	command;
	uint32	in_count;
	uint32	in_index;
	uint32	out_count;
	uint32	out_index;
	uint8	parameters[512];
	uint8	output[512];

	int16	CentreX;
	int16	CentreY;
	int16	VOffset;

	int16	VPlane_C;
	int16	VPlane_E;

	// Azimuth and Zenith angles
	int16	SinAas;
	int16	CosAas;
	int16	SinAzs;
	int16	CosAzs;

	// Clipped Zenith angle
	int16	SinAZS;
	int16	CosAZS;
	int16	SecAZS_C1;
	int16	SecAZS_E1;
	int16	SecAZS_C2;
	int16	SecAZS_E2;

	int16	Nx;
	int16	Ny;
	int16	Nz;
	int16	Gx;
	int16	Gy;
	int16	Gz;
	int16	C_Les;
	int16	E_Les;
	int16	G_Les;

	int16	matrixA[3][3];
	int16	matrixB[3][3];
	int16	matrixC[3][3];

	int16	Op00Multiplicand;
	int16	Op00Multiplier;
	int16	Op00Result;

	int16	Op20Multiplicand;
	int16	Op20Multiplier;
	int16	Op20Result;

	int16	Op10Coefficient;
	int16	Op10Exponent;
	int16	Op10CoefficientR;
	int16	Op10ExponentR;

	int16	Op04Angle;
	int16	Op04Radius;
	int16	Op04Sin;
	int16	Op04Cos;

	int16	Op0CA;
	int16	Op0CX1;
	int16	Op0CY1;
	int16	Op0CX2;
	int16	Op0CY2;

	int16	Op02FX;
	int16	Op02FY;
	int16	Op02FZ;
	int16	Op02LFE;
	int16	Op02LES;
	int16	Op02AAS;
	int16	Op02AZS;
	int16	Op02VOF;
	int16	Op02VVA;
	int16	Op02CX;
	int16	Op02CY;

	int16	Op0AVS;
	int16	Op0AA;
	int16	Op0AB;
	int16	Op0AC;
	int16	Op0AD;

	int16	Op06X;
	int16	Op06Y;
	int16	Op06Z;
	int16	Op06H;
	int16	Op06V;
	int16	Op06M;

	int16	Op01m;
	int16	Op01Zr;
	int16	Op01Xr;
	int16	Op01Yr;

	int16	Op11m;
	int16	Op11Zr;
	int16	Op11Xr;
	int16	Op11Yr;

	int16	Op21m;
	int16	Op21Zr;
	int16	Op21Xr;
	int16	Op21Yr;

	int16	Op0DX;
	int16	Op0DY;
	int16	Op0DZ;
	int16	Op0DF;
	int16	Op0DL;
	int16	Op0DU;

	int16	Op1DX;
	int16	Op1DY;
	int16	Op1DZ;
	int16	Op1DF;
	int16	Op1DL;
	int16	Op1DU;

	int16	Op2DX;
	int16	Op2DY;
	int16	Op2DZ;
	int16	Op2DF;
	int16	Op2DL;
	int16	Op2DU;

	int16	Op03F;
	int16	Op03L;
	int16	Op03U;
	int16	Op03X;
	int16	Op03Y;
	int16	Op03Z;

	int16	Op13F;
	int16	Op13L;
	int16	Op13U;
	int16	Op13X;
	int16	Op13Y;
	int16	Op13Z;

	int16	Op23F;
	int16	Op23L;
	int16	Op23U;
	int16	Op23X;
	int16	Op23Y;
	int16	Op23Z;

	int16	Op14Zr;
	int16	Op14Xr;
	int16	Op14Yr;
	int16	Op14U;
	int16	Op14F;
	int16	Op14L;
	int16	Op14Zrr;
	int16	Op14Xrr;
	int16	Op14Yrr;

	int16	Op0EH;
	int16	Op0EV;
	int16	Op0EX;
	int16	Op0EY;

	int16	Op0BX;
	int16	Op0BY;
	int16	Op0BZ;
	int16	Op0BS;

	int16	Op1BX;
	int16	Op1BY;
	int16	Op1BZ;
	int16	Op1BS;

	int16	Op2BX;
	int16	Op2BY;
	int16	Op2BZ;
	int16	Op2BS;

	int16	Op28X;
	int16	Op28Y;
	int16	Op28Z;
	int16	Op28R;

	int16	Op1CX;
	int16	Op1CY;
	int16	Op1CZ;
	int16	Op1CXBR;
	int16	Op1CYBR;
	int16	Op1CZBR;
	int16	Op1CXAR;
	int16	Op1CYAR;
	int16	Op1CZAR;
	int16	Op1CX1;
	int16	Op1CY1;
	int16	Op1CZ1;
	int16	Op1CX2;
	int16	Op1CY2;
	int16	Op1CZ2;

	uint16	Op0FRamsize;
	uint16	Op0FPass;

	int16	Op2FUnknown;
	int16	Op2FSize;

	int16	Op08X;
	int16	Op08Y;
	int16	Op08Z;
	int16	Op08Ll;
	int16	Op08Lh;

	int16	Op18X;
	int16	Op18Y;
	int16	Op18Z;
	int16	Op18R;
	int16	Op18D;

	int16	Op38X;
	int16	Op38Y;
	int16	Op38Z;
	int16	Op38R;
	int16	Op38D;
};

struct SDSP2
{
	bool8	waiting4command;
	uint8	command;
	uint32	in_count;
	uint32	in_index;
	uint32	out_count;
	uint32	out_index;
	uint8	parameters[512];
	uint8	output[512];

	bool8	Op05HasLen;
	int32	Op05Len;
	uint8	Op05Transparent;

	bool8	Op06HasLen;
	int32	Op06Len;

	uint16	Op09Word1;
	uint16	Op09Word2;

	bool8	Op0DHasLen;
	int32	Op0DOutLen;
	int32	Op0DInLen;
};

struct SDSP3
{
	uint16	DR;
	uint16	SR;
	uint16	MemoryIndex;

	int16	WinLo;
	int16	WinHi;
	int16	AddLo;
	int16	AddHi;

	uint16	Codewords;
	uint16	Outwords;
	uint16	Symbol;
	uint16	BitCount;
	uint16	Index;
	uint16	Codes[512];
	uint16	BitsLeft;
	uint16	ReqBits;
	uint16	ReqData;
	uint16	BitCommand;
	uint8	BaseLength;
	uint16	BaseCodes;
	uint16	BaseCode;
	uint8	CodeLengths[8];
	uint16	CodeOffsets[8];
	uint16	LZCode;
	uint8	LZLength;

	uint16	X;
	uint16	Y;

	uint8	Bitmap[8];
	uint8	Bitplane[8];
	uint16	BMIndex;
	uint16	BPIndex;
	uint16	Count;

	int16	op3e_x;
	int16	op3e_y;

	int16	op1e_terrain[0x2000];
	int16	op1e_cost[0x2000];
	int16	op1e_weight[0x2000];

	int16	op1e_cell;
	int16	op1e_turn;
	int16	op1e_search;

	int16	op1e_x;
	int16	op1e_y;

	int16	op1e_min_radius;
	int16	op1e_max_radius;

	int16	op1e_max_search_radius;
	int16	op1e_max_path_radius;

	int16	op1e_lcv_radius;
	int16	op1e_lcv_steps;
	int16	op1e_lcv_turns;
};

struct SDSP4
{
	bool8	waiting4command;
	bool8	half_command;
	uint16	command;
	uint32	in_count;
	uint32	in_index;
	uint32	out_count;
	uint32	out_index;
	uint8	parameters[512];
	uint8	output[512];
	uint8	byte;
	uint16	address;

	// op control
	int8	Logic;				// controls op flow

	// projection format
	int16	lcv;				// loop-control variable
	int16	distance;			// z-position into virtual world
	int16	raster;				// current raster line
	int16	segments;			// number of raster lines drawn

	// 1.15.16 or 1.15.0 [sign, integer, fraction]
	int32	world_x;			// line of x-projection in world
	int32	world_y;			// line of y-projection in world
	int32	world_dx;			// projection line x-delta
	int32	world_dy;			// projection line y-delta
	int16	world_ddx;			// x-delta increment
	int16	world_ddy;			// y-delta increment
	int32	world_xenv;			// world x-shaping factor
	int16	world_yofs;			// world y-vertical scroll
	int16	view_x1;			// current viewer-x
	int16	view_y1;			// current viewer-y
	int16	view_x2;			// future viewer-x
	int16	view_y2;			// future viewer-y
	int16	view_dx;			// view x-delta factor
	int16	view_dy;			// view y-delta factor
	int16	view_xofs1;			// current viewer x-vertical scroll
	int16	view_yofs1;			// current viewer y-vertical scroll
	int16	view_xofs2;			// future viewer x-vertical scroll
	int16	view_yofs2;			// future viewer y-vertical scroll
	int16	view_yofsenv;		// y-scroll shaping factor
	int16	view_turnoff_x;		// road turnoff data
	int16	view_turnoff_dx;	// road turnoff delta factor

	// drawing area
	int16	viewport_cx;		// x-center of viewport window
	int16	viewport_cy;		// y-center of render window
	int16	viewport_left;		// x-left of viewport
	int16	viewport_right;		// x-right of viewport
	int16	viewport_top;		// y-top of viewport
	int16	viewport_bottom;	// y-bottom of viewport

	// sprite structure
	int16	sprite_x;			// projected x-pos of sprite
	int16	sprite_y;			// projected y-pos of sprite
	int16	sprite_attr;		// obj attributes
	bool8	sprite_size;		// sprite size: 8x8 or 16x16
	int16	sprite_clipy;		// visible line to clip pixels off
	int16	sprite_count;

	// generic projection variables designed for two solid polygons + two polygon sides
	int16	poly_clipLf[2][2];	// left clip boundary
	int16	poly_clipRt[2][2];	// right clip boundary
	int16	poly_ptr[2][2];		// HDMA structure pointers
	int16	poly_raster[2][2];	// current raster line below horizon
	int16	poly_top[2][2];		// top clip boundary
	int16	poly_bottom[2][2];	// bottom clip boundary
	int16	poly_cx[2][2];		// center for left/right points
	int16	poly_start[2];		// current projection points
	int16	poly_plane[2];		// previous z-plane distance

	// OAM
	int16	OAM_attr[16];		// OAM (size, MSB) data
	int16	OAM_index;			// index into OAM table
	int16	OAM_bits;			// offset into OAM table
	int16	OAM_RowMax;			// maximum number of tiles per 8 aligned pixels (row)
	int16	OAM_Row[32];		// current number of tiles per row
};

extern struct SDSP0	DSP0;
extern struct SDSP1	DSP1;
extern struct SDSP2	DSP2;
extern struct SDSP3	DSP3;
extern struct SDSP4	DSP4;

uint8 S9xGetDSP (uint16);
void S9xSetDSP (uint8, uint16);
void S9xResetDSP (void);
uint8 DSP1GetByte (uint16);
void DSP1SetByte (uint8, uint16);
uint8 DSP2GetByte (uint16);
void DSP2SetByte (uint8, uint16);
uint8 DSP3GetByte (uint16);
void DSP3SetByte (uint8, uint16);
uint8 DSP4GetByte (uint16);
void DSP4SetByte (uint8, uint16);
void DSP3_Reset (void);

extern uint8 (*GetDSP) (uint16);
extern void (*SetDSP) (uint8, uint16);

#endif
