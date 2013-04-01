/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PSXDEV_GTE_TESTING
#include "psx.h"
#include "gte.h"
#endif

static uint32 ReciprocalTable[0x8000] =
{
 #include "gte_divrecip.inc"
};

/* Notes:

 AVSZ3/AVSZ4:
	OTZ is MAC0 >> 12
	OTZ overflow/underflow flag is set in an overflow condition even if MAC0 == 0.
	sf field bit has no effect?

 FLAG register:
	Bits present mask: 0xfffff000

	Checksum bit can't be directly set, it's apparently calculated like (bool)(FLAGS & 0x7f87e000)

	Instructions effectively clear it 0 at start. (todo: test "invalid" instructions)

 X/Y FIFO [3] register write pushes a copy down to [2]

*/

#ifndef PSXDEV_GTE_TESTING
namespace MDFN_IEN_PSX
{
#endif

typedef struct
{
 int16 MX[3][3];
 int16 dummy;
}  __attribute__((__packed__)) gtematrix;

typedef struct
{
 union
 {
  struct
  {
   uint8 R;
   uint8 G;
   uint8 B;
   uint8 CD;
  };
  uint8 Raw8[4];
 };
} gtergb;

typedef struct
{
 int16 X;
 int16 Y;
} gtexy;

int16 Lm_B(unsigned int which, int32 value, int lm);
uint8 Lm_C(unsigned int which, int32 value);



int32 Lm_G(unsigned int which, int32 value);
int32 Lm_H(int32 value);

void MAC_to_RGB_FIFO(void);
void MAC_to_IR(int lm);

void MultiplyMatrixByVector(const gtematrix *matrix, const int16 *v, const int32 *crv, uint32 sf, int lm);

static uint32 CR[32];
static uint32 FLAGS;	// Temporary for instruction execution, copied into CR[31] at end of instruction execution.

typedef union
{
 gtematrix All[4];
 int32 Raw[4][5];	// Don't read from this(Raw[][]), only write(and when writing, if running on a big-endian platform, swap the upper 16-bits with the lower 16-bits)
 int16 Raw16[4][10];

 struct
 {
  gtematrix Rot;
  gtematrix Light;
  gtematrix Color;
  gtematrix AbbyNormal;
 };
} Matrices_t;

static Matrices_t Matrices;

static union
{
 int32 All[4][4];	// Really only [4][3], but [4] to ease address calculation.
  
 struct
 {
  int32 T[4];
  int32 B[4];
  int32 FC[4];
  int32 Null[4];
 };
} CRVectors;

static int32 OFX;
static int32 OFY;
static uint16 H;
static int16 DQA;
static int32 DQB;
 
static int16 ZSF3;
static int16 ZSF4;


// Begin DR
static int16 Vectors[3][4];
static gtergb RGB;
static uint16 OTZ;

static int16 IR[4];

#define IR0 IR[0]
#define IR1 IR[1]
#define IR2 IR[2]
#define IR3 IR[3]

static gtexy XY_FIFO[4];
static uint16 Z_FIFO[4];
static gtergb RGB_FIFO[3];
static int32 MAC[4];
static uint32 LZCS;
static uint32 LZCR;

static uint32 Reg23;
// end DR

int32 RTPS(uint32 instr);
int32 RTPT(uint32 instr);

int32 NCLIP(uint32 instr);

void NormColor(uint32 sf, int lm, uint32 v);
int32 NCS(uint32 instr);
int32 NCT(uint32 instr);


void NormColorColor(uint32 v, uint32 sf, int lm);
int32 NCCS(uint32 instr);
int32 NCCT(uint32 instr);

void NormColorDepthCue(uint32 v, uint32 sf, int lm);
int32 NCDS(uint32 instr);
int32 NCDT(uint32 instr);

int32 AVSZ3(uint32 instr);
int32 AVSZ4(uint32 instr);

int32 OP(uint32 instr);

int32 GPF(uint32 instr);
int32 GPL(uint32 instr);

void DepthCue(int mult_IR123, int RGB_from_FIFO, uint32 sf, int lm);
int32 DCPL(uint32 instr);
int32 DPCS(uint32 instr);
int32 DPCT(uint32 instr);
int32 INTPL(uint32 instr);

int32 SQR(uint32 instr);
int32 MVMVA(uint32 instr);

static INLINE uint8 Sat5(int16 cc)
{
 if(cc < 0)
  cc = 0;
 if(cc > 0x1F)
  cc = 0x1F;
 return(cc);
}



void GTE_Power(void)
{
 memset(CR, 0, sizeof(CR));
 //memset(DR, 0, sizeof(DR));

 memset(Matrices.All, 0, sizeof(Matrices.All));
 memset(CRVectors.All, 0, sizeof(CRVectors.All));
 OFX = 0;
 OFY = 0;
 H = 0;
 DQA = 0;
 DQB = 0;
 ZSF3 = 0;
 ZSF4 = 0;


 memset(Vectors, 0, sizeof(Vectors));
 memset(&RGB, 0, sizeof(RGB));
 OTZ = 0;
 IR0 = 0;
 IR1 = 0;
 IR2 = 0;
 IR3 = 0;

 memset(XY_FIFO, 0, sizeof(XY_FIFO));
 memset(Z_FIFO, 0, sizeof(Z_FIFO));
 memset(RGB_FIFO, 0, sizeof(RGB_FIFO));
 memset(MAC, 0, sizeof(MAC));
 LZCS = 0;
 LZCR = 0;

 Reg23 = 0;
}

// TODO: Don't save redundant state, regarding CR cache variables
int GTE_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY32(CR, 32),
  SFVAR(FLAGS),

  SFARRAY16(&Matrices.Raw16[0][0], 4 * 10),

  SFARRAY32(&CRVectors.All[0][0], 4 * 4),

  SFVAR(OFX),
  SFVAR(OFY),
  SFVAR(H),
  SFVAR(DQA),
  SFVAR(DQB),

  SFVAR(ZSF3),
  SFVAR(ZSF4),
  SFARRAY16(&Vectors[0][0], 3 * 4),

  SFARRAY(RGB.Raw8, 4),
  SFVAR(OTZ),
  SFARRAY16(IR, 4),

  SFVAR(XY_FIFO[0].X),
  SFVAR(XY_FIFO[0].Y),
  SFVAR(XY_FIFO[1].X),
  SFVAR(XY_FIFO[1].Y),
  SFVAR(XY_FIFO[2].X),
  SFVAR(XY_FIFO[2].Y),
  SFVAR(XY_FIFO[3].X),
  SFVAR(XY_FIFO[3].Y),

  SFARRAY16(Z_FIFO, 4),

  SFARRAY(RGB_FIFO[0].Raw8, 4),
  SFARRAY(RGB_FIFO[1].Raw8, 4),
  SFARRAY(RGB_FIFO[2].Raw8, 4),
  SFARRAY(RGB_FIFO[3].Raw8, 4),

  SFARRAY32(MAC, 4),

  SFVAR(LZCS),
  SFVAR(LZCR),
  SFVAR(Reg23),

  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "GTE");

 if(load)
 {

 }

 return(ret);
}


void GTE_WriteCR(unsigned int which, uint32 value)
{
 static const uint32 mask_table[32] = {
	/* 0x00 */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* 0x08 */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* 0x10 */
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/* 0x18 */
	0xFFFFFFFF, 0xFFFFFFFF, 0x0000FFFF, 0x0000FFFF, 0xFFFFFFFF, 0x0000FFFF, 0x0000FFFF, 0xFFFFFFFF
 };

 //PSX_WARNING("[GTE] Write CR %d, 0x%08x", which, value);

 value &= mask_table[which];

 CR[which] = value | (CR[which] & ~mask_table[which]);

 if(which < 24)
 {
  int we = which >> 3;
  which &= 0x7;

  if(which >= 5)
   CRVectors.All[we][which - 5] = value;
  else
  {
   #ifdef MSB_FIRST
   Matrices.Raw[we][which] = (value << 16) | (value >> 16);
   #else
   Matrices.Raw[we][which] = value;
   #endif
  }
  return;
 }

 switch(which)
 {
  case 24:
	OFX = value;
	break;

  case 25:
	OFY = value;
	break;

  case 26:
	H = value;
	break;

  case 27:
	DQA = value;
	break;

  case 28:
	DQB = value;
	break;

  case 29:
	ZSF3 = value;
	break;

  case 30:
	ZSF4 = value;
	break;

  case 31:
	CR[31] = (value & 0x7ffff000) | ((value & 0x7f87e000) ? (1 << 31) : 0);
	break;
 }
}

uint32 GTE_ReadCR(unsigned int which)
{
 uint32 ret = 0;

 switch(which)
 {
  default:
	ret = CR[which];
	if(which == 4 || which == 12 || which == 20)
	 ret = (int16)ret;
	break;

  case 24:
	ret = OFX;
	break;

  case 25:
	ret = OFY;
	break;

  case 26:
	ret = (int16)H;
	break;

  case 27:
	ret = (int16)DQA;
	break;

  case 28:
	ret = DQB;
	break;

  case 29:
	ret = (int16)ZSF3;
	break;

  case 30:
	ret = (int16)ZSF4;
	break;

  case 31:
	ret = CR[31];
	break;
 }

 return(ret);
}

void GTE_WriteDR(unsigned int which, uint32 value)
{
 switch(which & 0x1F)
 {
  case 0:
	Vectors[0][0] = value;
	Vectors[0][1] = value >> 16;
	break;

  case 1:
	Vectors[0][2] = value;
	break;

  case 2:
	Vectors[1][0] = value;
	Vectors[1][1] = value >> 16;
	break;

  case 3:
	Vectors[1][2] = value;
	break;

  case 4:
	Vectors[2][0] = value;
	Vectors[2][1] = value >> 16;
	break;

  case 5:
	Vectors[2][2] = value;
	break;

  case 6:
	RGB.R = value >> 0;
	RGB.G = value >> 8;
	RGB.B = value >> 16;
	RGB.CD = value >> 24;
	break;

  case 7:
	OTZ = value;
	break;

  case 8:
	IR0 = value;
	break;

  case 9:
	IR1 = value;
	break;

  case 10:
	IR2 = value;
	break;

  case 11:
	IR3 = value;
	break;

  case 12:
	XY_FIFO[0].X = value;
	XY_FIFO[0].Y = value >> 16;
	break;

  case 13:
	XY_FIFO[1].X = value;
	XY_FIFO[1].Y = value >> 16;
	break;

  case 14:
	XY_FIFO[2].X = value;
	XY_FIFO[2].Y = value >> 16;
	XY_FIFO[3].X = value;
	XY_FIFO[3].Y = value >> 16;
	break;

  case 15:
	XY_FIFO[3].X = value;
	XY_FIFO[3].Y = value >> 16;

	XY_FIFO[0] = XY_FIFO[1];
	XY_FIFO[1] = XY_FIFO[2];
	XY_FIFO[2] = XY_FIFO[3];
	break;

  case 16:
	Z_FIFO[0] = value;
	break;

  case 17:
	Z_FIFO[1] = value;
	break;

  case 18:
	Z_FIFO[2] = value;
	break;

  case 19:
	Z_FIFO[3] = value;
	break;

  case 20:
	RGB_FIFO[0].R = value;
	RGB_FIFO[0].G = value >> 8;
	RGB_FIFO[0].B = value >> 16;
	RGB_FIFO[0].CD = value >> 24;
	break;

  case 21:
	RGB_FIFO[1].R = value;
	RGB_FIFO[1].G = value >> 8;
	RGB_FIFO[1].B = value >> 16;
	RGB_FIFO[1].CD = value >> 24;
	break;

  case 22:
	RGB_FIFO[2].R = value;
	RGB_FIFO[2].G = value >> 8;
	RGB_FIFO[2].B = value >> 16;
	RGB_FIFO[2].CD = value >> 24;
	break;

  case 23:
	Reg23 = value;
	break;

  case 24:
	MAC[0] = value;
	break;

  case 25:
	MAC[1] = value;
	break;

  case 26:
	MAC[2] = value;
	break;

  case 27:
	MAC[3] = value;
	break;

  case 28:
	IR1 = ((value >> 0) & 0x1F) << 7;
	IR2 = ((value >> 5) & 0x1F) << 7;
	IR3 = ((value >> 10) & 0x1F) << 7;
	break;

  case 29:	// Read-only
	break;

  case 30:
	LZCS = value;
	{
	 uint32 test = value & 0x80000000;
	 LZCR = 0;

	 while((value & 0x80000000) == test && LZCR < 32)
	 {
	  LZCR++;
	  value <<= 1;
	 }
	}
	break;

  case 31:	// Read-only
	break;
 }
}

uint32 GTE_ReadDR(unsigned int which)
{
 uint32 ret = 0;

 switch(which & 0x1F)
 {
  case 0:
	ret = (uint16)Vectors[0][0] | ((uint16)Vectors[0][1] << 16);
	break;

  case 1:
	ret = (int16)Vectors[0][2];
	break;

  case 2:
	ret = (uint16)Vectors[1][0] | ((uint16)Vectors[1][1] << 16);
	break;

  case 3:
	ret = (int16)Vectors[1][2];
	break;

  case 4:
	ret = (uint16)Vectors[2][0] | ((uint16)Vectors[2][1] << 16);
	break;

  case 5:
	ret = (int16)Vectors[2][2];
	break;

  case 6:
	ret = RGB.R | (RGB.G << 8) | (RGB.B << 16) | (RGB.CD << 24);
	break;

  case 7:
	ret = (uint16)OTZ;
	break;

  case 8:
	ret = (int16)IR0;
	break;

  case 9:
	ret = (int16)IR1;
	break;

  case 10:
	ret = (int16)IR2;
	break;

  case 11:
	ret = (int16)IR3;
	break;

  case 12:
	ret = (uint16)XY_FIFO[0].X | ((uint16)XY_FIFO[0].Y << 16);
	break;

  case 13:
	ret = (uint16)XY_FIFO[1].X | ((uint16)XY_FIFO[1].Y << 16);
	break;

  case 14:
	ret = (uint16)XY_FIFO[2].X | ((uint16)XY_FIFO[2].Y << 16);
	break;

  case 15:
	ret = (uint16)XY_FIFO[3].X | ((uint16)XY_FIFO[3].Y << 16);
	break;

  case 16:
	ret = (uint16)Z_FIFO[0];
	break;

  case 17:
	ret = (uint16)Z_FIFO[1];
	break;

  case 18:
	ret = (uint16)Z_FIFO[2];
	break;

  case 19:
	ret = (uint16)Z_FIFO[3];
	break;

  case 20:
	ret = RGB_FIFO[0].R | (RGB_FIFO[0].G << 8) | (RGB_FIFO[0].B << 16) | (RGB_FIFO[0].CD << 24);
	break;

  case 21:
	ret = RGB_FIFO[1].R | (RGB_FIFO[1].G << 8) | (RGB_FIFO[1].B << 16) | (RGB_FIFO[1].CD << 24);
	break;

  case 22:
	ret = RGB_FIFO[2].R | (RGB_FIFO[2].G << 8) | (RGB_FIFO[2].B << 16) | (RGB_FIFO[2].CD << 24);
	break;

  case 23:
	ret = Reg23;
	break;

  case 24:
	ret = MAC[0];
	break;

  case 25:
	ret = MAC[1];
	break;

  case 26:
	ret = MAC[2];
	break;

  case 27:
	ret = MAC[3];
	break;

  case 28:
  case 29:
	ret = Sat5(IR1 >> 7) | (Sat5(IR2 >> 7) << 5) | (Sat5(IR3 >> 7) << 10);
	break;

  case 30:
	ret = LZCS;
	break;

  case 31:
	ret = LZCR;
	break;
 }
 return(ret);
}

#define sign_x_to_s64(_bits, _value) (((int64)((uint64)(_value) << (64 - _bits))) >> (64 - _bits))
INLINE int64 A_MV(unsigned which, int64 value)
{
 if(value >= (1LL << 43))
  FLAGS |= 1 << (30 - which);

 if(value < -(1LL << 43))
  FLAGS |= 1 << (27 - which);

 return sign_x_to_s64(44, value);
}

INLINE int64 F(int64 value)
{
 if(value < -2147483648LL)
 {
  // flag set here
  FLAGS |= 1 << 15;
 }

 if(value > 2147483647LL)
 {
  // flag set here
  FLAGS |= 1 << 16;
 }
 return(value);
}


INLINE int16 Lm_B(unsigned int which, int32 value, int lm)
{
 int32 tmp = lm << 15;

 if(value < (-32768 + tmp))
 {
  // set flag here
  FLAGS |= 1 << (24 - which);
  value = -32768 + tmp;
 }

 if(value > 32767)
 {
  // Set flag here
  FLAGS |= 1 << (24 - which);
  value = 32767;
 }

 return(value);
}


INLINE int16 Lm_B_PTZ(unsigned int which, int32 value, int32 ftv_value, int lm)
{
 int32 tmp = lm << 15;

 if(ftv_value < -32768)
 {
  FLAGS |= 1 << (24 - which);
 }

 if(ftv_value > 32767)
 {
  FLAGS |= 1 << (24 - which);
 }

 if(value < (-32768 + tmp))
 {
  value = -32768 + tmp;
 }

 if(value > 32767)
 {
  value = 32767;
 }

 return(value);
}

INLINE uint8 Lm_C(unsigned int which, int32 value)
{
 if(value & ~0xFF)
 {
  // Set flag here
  FLAGS |= 1 << (21 - which);	// Tested with GPF

  if(value < 0)
   value = 0;

  if(value > 255)
   value = 255;
 }

 return(value);
}

INLINE int32 Lm_D(int32 value, int unchained)
{
 // Not sure if we should have it as int64, or just chain on to and special case when the F flags are set.
 if(!unchained)
 {
  if(FLAGS & (1 << 15))
  {
   FLAGS |= 1 << 18;
   return(0);
  }

  if(FLAGS & (1 << 16))
  {
   FLAGS |= 1 << 18;
   return(0xFFFF);
  }
 }

 if(value < 0)
 {
  // Set flag here
  value = 0;
  FLAGS |= 1 << 18;	// Tested with AVSZ3
 }
 else if(value > 65535)
 {
  // Set flag here.
  value = 65535;
  FLAGS |= 1 << 18;	// Tested with AVSZ3
 }

 return(value);
}

INLINE int32 Lm_G(unsigned int which, int32 value)
{
 if(value < -1024)
 {
  // Set flag here
  value = -1024;
  FLAGS |= 1 << (14 - which);
 }

 if(value > 1023)
 {
  // Set flag here.
  value = 1023;
  FLAGS |= 1 << (14 - which);
 }

 return(value);
}

// limit to 4096, not 4095
INLINE int32 Lm_H(int32 value)
{
#if 0
 if(FLAGS & (1 << 15))
 {
  value = 0;
  FLAGS |= 1 << 12;
  return value;
 }

 if(FLAGS & (1 << 16))
 {
  value = 4096;
  FLAGS |= 1 << 12;
  return value;
 }
#endif

 if(value < 0)
 {
  value = 0;
  FLAGS |= 1 << 12;
 }

 if(value > 4096)
 {
  value = 4096;
  FLAGS |= 1 << 12;
 }

 return(value);
}

INLINE void MAC_to_RGB_FIFO(void)
{
 RGB_FIFO[0] = RGB_FIFO[1];
 RGB_FIFO[1] = RGB_FIFO[2];
 RGB_FIFO[2].R = Lm_C(0, MAC[1] >> 4);
 RGB_FIFO[2].G = Lm_C(1, MAC[2] >> 4);
 RGB_FIFO[2].B = Lm_C(2, MAC[3] >> 4);
 RGB_FIFO[2].CD = RGB.CD;
}


INLINE void MAC_to_IR(int lm)
{
 IR1 = Lm_B(0, MAC[1], lm);
 IR2 = Lm_B(1, MAC[2], lm);
 IR3 = Lm_B(2, MAC[3], lm);
}

INLINE void MultiplyMatrixByVector(const gtematrix *matrix, const int16 *v, const int32 *crv, uint32 sf, int lm)
{
 unsigned i;

 for(i = 0; i < 3; i++)
 {
  int64 tmp;
  int32 mulr[3];

  tmp = (int64)crv[i] << 12;

  if(matrix == &Matrices.AbbyNormal)
  {
   if(i == 0)
   {
    mulr[0] = -((RGB.R << 4) * v[0]);
    mulr[1] = (RGB.R << 4) * v[1];
    mulr[2] = IR0 * v[2];
   }
   else
   {
    mulr[0] = (int16)CR[i] * v[0];
    mulr[1] = (int16)CR[i] * v[1];
    mulr[2] = (int16)CR[i] * v[2];
   }
  }
  else
  {
   mulr[0] = matrix->MX[i][0] * v[0];
   mulr[1] = matrix->MX[i][1] * v[1];
   mulr[2] = matrix->MX[i][2] * v[2];
  }

  tmp = A_MV(i, tmp + mulr[0]);
  if(crv == CRVectors.FC)
  {
   Lm_B(i, tmp >> sf, FALSE);
   tmp = 0;
  }

  tmp = A_MV(i, tmp + mulr[1]);
  tmp = A_MV(i, tmp + mulr[2]);

  MAC[1 + i] = tmp >> sf;
 }


 MAC_to_IR(lm);
}


INLINE void MultiplyMatrixByVector_PT(const gtematrix *matrix, const int16 *v, const int32 *crv, uint32 sf, int lm)
{
 int64 tmp[3];
 unsigned i;

 for(i = 0; i < 3; i++)
 {
  int32 mulr[3];

  tmp[i] = (int64)crv[i] << 12;

  mulr[0] = matrix->MX[i][0] * v[0];
  mulr[1] = matrix->MX[i][1] * v[1];
  mulr[2] = matrix->MX[i][2] * v[2];

  tmp[i] = A_MV(i, tmp[i] + mulr[0]);
  tmp[i] = A_MV(i, tmp[i] + mulr[1]);
  tmp[i] = A_MV(i, tmp[i] + mulr[2]);

  MAC[1 + i] = tmp[i] >> sf;
 }

 IR1 = Lm_B(0, MAC[1], lm);
 IR2 = Lm_B(1, MAC[2], lm);
 //printf("FTV: %08x %08x\n", crv[2], (uint32)(tmp[2] >> 12));
 IR3 = Lm_B_PTZ(2, MAC[3], tmp[2] >> 12, lm);

 Z_FIFO[0] = Z_FIFO[1];
 Z_FIFO[1] = Z_FIFO[2];
 Z_FIFO[2] = Z_FIFO[3];
 Z_FIFO[3] = Lm_D(tmp[2] >> 12, TRUE);
}


#define DECODE_FIELDS							\
 const uint32 sf MDFN_NOWARN_UNUSED = (instr & (1 << 19)) ? 12 : 0;		\
 const uint32 mx MDFN_NOWARN_UNUSED = (instr >> 17) & 0x3;			\
 const uint32 v_i = (instr >> 15) & 0x3;				\
 const int32* cv MDFN_NOWARN_UNUSED = CRVectors.All[(instr >> 13) & 0x3];	\
 const int lm MDFN_NOWARN_UNUSED = (instr >> 10) & 1;			\
 int16 v[3] MDFN_NOWARN_UNUSED;					\
 if(v_i == 3)							\
 {								\
  v[0] = IR1;							\
  v[1] = IR2;							\
  v[2] = IR3;							\
 }								\
 else								\
 {								\
  v[0] = Vectors[v_i][0];					\
  v[1] = Vectors[v_i][1];					\
  v[2] = Vectors[v_i][2];					\
 }


int32 SQR(uint32 instr)
{
 DECODE_FIELDS;

 MAC[1] = ((IR1 * IR1) >> sf);
 MAC[2] = ((IR2 * IR2) >> sf);
 MAC[3] = ((IR3 * IR3) >> sf);

 MAC_to_IR(lm);

 return(5);
}


int32 MVMVA(uint32 instr)
{
 DECODE_FIELDS;

 MultiplyMatrixByVector(&Matrices.All[mx], v, cv, sf, lm);

 return(8);
}

static INLINE unsigned CountLeadingZeroU16(uint16 val)
{
 unsigned ret = 0;

 while(!(val & 0x8000) && ret < 16)
 {
  val <<= 1;
  ret++;
 }

 return ret;
}

static INLINE uint32 Divide(uint32 dividend, uint32 divisor)
{
 //if((Z_FIFO[3] * 2) > H)
 if((divisor * 2) > dividend)
 {
  unsigned shift_bias = CountLeadingZeroU16(divisor);

  dividend <<= shift_bias;
  divisor <<= shift_bias;

  return ((int64)dividend * ReciprocalTable[divisor & 0x7FFF] + 32768) >> 16;
 }
 else
 {
  FLAGS |= 1 << 17;
  return 0x1FFFF;
 }
}

static INLINE void TransformXY(int64 h_div_sz)
{
 MAC[0] = F((int64)OFX + IR1 * h_div_sz) >> 16;
 XY_FIFO[3].X = Lm_G(0, MAC[0]);

 MAC[0] = F((int64)OFY + IR2 * h_div_sz) >> 16;
 XY_FIFO[3].Y = Lm_G(1, MAC[0]);

 XY_FIFO[0] = XY_FIFO[1];
 XY_FIFO[1] = XY_FIFO[2];
 XY_FIFO[2] = XY_FIFO[3];
}

static INLINE void TransformDQ(int64 h_div_sz)
{
 MAC[0] = F((int64)DQB + DQA * h_div_sz);
 IR0 = Lm_H(((int64)DQB + DQA * h_div_sz) >> 12);
}

int32 RTPS(uint32 instr)
{
 DECODE_FIELDS;
 int64 h_div_sz;

 MultiplyMatrixByVector_PT(&Matrices.Rot, Vectors[0], CRVectors.T, sf, lm);
 h_div_sz = Divide(H, Z_FIFO[3]);

 TransformXY(h_div_sz);
 TransformDQ(h_div_sz);

 return(15);
}

int32 RTPT(uint32 instr)
{
 DECODE_FIELDS;
 int i;

 for(i = 0; i < 3; i++)
 {
  int64 h_div_sz;

  MultiplyMatrixByVector_PT(&Matrices.Rot, Vectors[i], CRVectors.T, sf, lm);
  h_div_sz = Divide(H, Z_FIFO[3]);

  TransformXY(h_div_sz);

  if(i == 2)
   TransformDQ(h_div_sz);
 }

 return(23);
}

INLINE void NormColor(uint32 sf, int lm, uint32 v)
{
 int16 tmp_vector[3];

 MultiplyMatrixByVector(&Matrices.Light, Vectors[v], CRVectors.Null, sf, lm);

 tmp_vector[0] = IR1; tmp_vector[1] = IR2; tmp_vector[2] = IR3;
 MultiplyMatrixByVector(&Matrices.Color, tmp_vector, CRVectors.B, sf, lm);

 MAC_to_RGB_FIFO();
}

int32 NCS(uint32 instr)
{
 DECODE_FIELDS;

 NormColor(sf, lm, 0);

 return(14);
}

int32 NCT(uint32 instr)
{
 DECODE_FIELDS;
 int i;

 for(i = 0; i < 3; i++)
  NormColor(sf, lm, i);

 return(30);
}

INLINE void NormColorColor(uint32 v, uint32 sf, int lm)
{
 int16 tmp_vector[3];

 MultiplyMatrixByVector(&Matrices.Light, Vectors[v], CRVectors.Null, sf, lm);

 tmp_vector[0] = IR1; tmp_vector[1] = IR2; tmp_vector[2] = IR3;
 MultiplyMatrixByVector(&Matrices.Color, tmp_vector, CRVectors.B, sf, lm);

 MAC[1] = ((RGB.R << 4) * IR1) >> sf;
 MAC[2] = ((RGB.G << 4) * IR2) >> sf;
 MAC[3] = ((RGB.B << 4) * IR3) >> sf;

 MAC_to_IR(lm);

 MAC_to_RGB_FIFO();
}

int32 NCCS(uint32 instr)
{
 DECODE_FIELDS;

 NormColorColor(0, sf, lm);
 return(17);
}


int32 NCCT(uint32 instr)
{
 int i;
 DECODE_FIELDS;

 for(i = 0; i < 3; i++)
  NormColorColor(i, sf, lm);

 return(39);
}

INLINE void DepthCue(int mult_IR123, int RGB_from_FIFO, uint32 sf, int lm)
{
 int32 RGB_temp[3];
 int32 IR_temp[3] = { IR1, IR2, IR3 };
 int i;

 //assert(sf);

 if(RGB_from_FIFO)
 {
  RGB_temp[0] = RGB_FIFO[0].R << 4;
  RGB_temp[1] = RGB_FIFO[0].G << 4;
  RGB_temp[2] = RGB_FIFO[0].B << 4;
 }
 else
 {
  RGB_temp[0] = RGB.R << 4;
  RGB_temp[1] = RGB.G << 4;
  RGB_temp[2] = RGB.B << 4;
 }

 if(mult_IR123)
 {
  for(i = 0; i < 3; i++)
  {
   MAC[1 + i] = A_MV(i, (((int64)CRVectors.FC[i] << 12) - RGB_temp[i] * IR_temp[i])) >> sf;
   MAC[1 + i] = A_MV(i, (RGB_temp[i] * IR_temp[i] + IR0 * Lm_B(i, MAC[1 + i], FALSE))) >> sf;
  }
 }
 else
 {
  for(i = 0; i < 3; i++)
  {
   MAC[1 + i] = A_MV(i, (((int64)CRVectors.FC[i] << 12) - (RGB_temp[i] << 12))) >> sf;
   MAC[1 + i] = A_MV(i, (((int64)RGB_temp[i] << 12) + IR0 * Lm_B(i, MAC[1 + i], FALSE))) >> sf;
  }
 }

 MAC_to_IR(lm);

 MAC_to_RGB_FIFO();
}


int32 DCPL(uint32 instr)
{
 DECODE_FIELDS;

 DepthCue(TRUE, FALSE, sf, lm);

 return(8);
}


int32 DPCS(uint32 instr)
{
 DECODE_FIELDS;

 DepthCue(FALSE, FALSE, sf, lm);

 return(8);
}

int32 DPCT(uint32 instr)
{
 int i;
 DECODE_FIELDS;

 for(i = 0; i < 3; i++)
 {
  DepthCue(FALSE, TRUE, sf, lm);
 }

 return(17);
}

int32 INTPL(uint32 instr)
{
 DECODE_FIELDS;

 MAC[1] = A_MV(0, (((int64)CRVectors.FC[0] << 12) - (IR1 << 12))) >> sf;
 MAC[2] = A_MV(1, (((int64)CRVectors.FC[1] << 12) - (IR2 << 12))) >> sf;
 MAC[3] = A_MV(2, (((int64)CRVectors.FC[2] << 12) - (IR3 << 12))) >> sf;

 MAC[1] = A_MV(0, (((int64)IR1 << 12) + IR0 * Lm_B(0, MAC[1], FALSE)) >> sf);
 MAC[2] = A_MV(1, (((int64)IR2 << 12) + IR0 * Lm_B(1, MAC[2], FALSE)) >> sf);
 MAC[3] = A_MV(2, (((int64)IR3 << 12) + IR0 * Lm_B(2, MAC[3], FALSE)) >> sf);

 MAC_to_IR(lm);

 MAC_to_RGB_FIFO();

 return(8);
}


INLINE void NormColorDepthCue(uint32 v, uint32 sf, int lm)
{
 int16 tmp_vector[3];

 MultiplyMatrixByVector(&Matrices.Light, Vectors[v], CRVectors.Null, sf, lm);

 tmp_vector[0] = IR1; tmp_vector[1] = IR2; tmp_vector[2] = IR3;
 MultiplyMatrixByVector(&Matrices.Color, tmp_vector, CRVectors.B, sf, lm);

 DepthCue(TRUE, FALSE, sf, lm);
}

int32 NCDS(uint32 instr)
{
 DECODE_FIELDS;

 NormColorDepthCue(0, sf, lm);

 return(19);
}

int32 NCDT(uint32 instr)
{
 int i;
 DECODE_FIELDS;

 for(i = 0; i < 3; i++)
 {
  NormColorDepthCue(i, sf, lm);
 }

 return(44);
}

int32 CC(uint32 instr)
{
 DECODE_FIELDS;
 int16 tmp_vector[3];

 tmp_vector[0] = IR1; tmp_vector[1] = IR2; tmp_vector[2] = IR3;
 MultiplyMatrixByVector(&Matrices.Color, tmp_vector, CRVectors.B, sf, lm);

 MAC[1] = ((RGB.R << 4) * IR1) >> sf;
 MAC[2] = ((RGB.G << 4) * IR2) >> sf;
 MAC[3] = ((RGB.B << 4) * IR3) >> sf;

 MAC_to_IR(lm);

 MAC_to_RGB_FIFO();

 return(11);
}

int32 CDP(uint32 instr)
{
 DECODE_FIELDS;
 int16 tmp_vector[3];

 tmp_vector[0] = IR1; tmp_vector[1] = IR2; tmp_vector[2] = IR3;
 MultiplyMatrixByVector(&Matrices.Color, tmp_vector, CRVectors.B, sf, lm);

 DepthCue(TRUE, FALSE, sf, lm);

 return(13);
}

int32 NCLIP(uint32 instr)
{
 DECODE_FIELDS;

 MAC[0] = F( (int64)(XY_FIFO[0].X * (XY_FIFO[1].Y - XY_FIFO[2].Y)) + (XY_FIFO[1].X * (XY_FIFO[2].Y - XY_FIFO[0].Y)) + (XY_FIFO[2].X * (XY_FIFO[0].Y - XY_FIFO[1].Y))
	  );

 return(8);
}

int32 AVSZ3(uint32 instr)
{
 DECODE_FIELDS;

 MAC[0] = F(((int64)ZSF3 * (Z_FIFO[1] + Z_FIFO[2] + Z_FIFO[3])));

 OTZ = Lm_D(MAC[0] >> 12, FALSE);

 return(5);
}

int32 AVSZ4(uint32 instr)
{
 DECODE_FIELDS;

 MAC[0] = F(((int64)ZSF4 * (Z_FIFO[0] + Z_FIFO[1] + Z_FIFO[2] + Z_FIFO[3])));

 OTZ = Lm_D(MAC[0] >> 12, FALSE);

 return(5);
}


// -32768 * -32768 - 32767 * -32768 = 2147450880
// (2 ^ 31) - 1 =		      2147483647
int32 OP(uint32 instr)
{
 DECODE_FIELDS;

 MAC[1] = ((Matrices.Rot.MX[1][1] * IR3) - (Matrices.Rot.MX[2][2] * IR2)) >> sf;
 MAC[2] = ((Matrices.Rot.MX[2][2] * IR1) - (Matrices.Rot.MX[0][0] * IR3)) >> sf;
 MAC[3] = ((Matrices.Rot.MX[0][0] * IR2) - (Matrices.Rot.MX[1][1] * IR1)) >> sf;

 MAC_to_IR(lm);

 return(6);
}

int32 GPF(uint32 instr)
{
 DECODE_FIELDS;

 MAC[1] = (IR0 * IR1) >> sf;
 MAC[2] = (IR0 * IR2) >> sf;
 MAC[3] = (IR0 * IR3) >> sf;

 MAC_to_IR(lm);

 MAC_to_RGB_FIFO();

 return(5);
}

int32 GPL(uint32 instr)
{
 DECODE_FIELDS;

 MAC[1] = A_MV(0, ((int64)MAC[1] << sf) + (IR0 * IR1)) >> sf;
 MAC[2] = A_MV(1, ((int64)MAC[2] << sf) + (IR0 * IR2)) >> sf;
 MAC[3] = A_MV(2, ((int64)MAC[3] << sf) + (IR0 * IR3)) >> sf;

 MAC_to_IR(lm);

 MAC_to_RGB_FIFO();

 return(5);
}

/*

---------------------------------------------------------------------------------------------
| 24 23 22 21 20 | 19 | 18 17 | 16 15 | 14 13 | 12  11 | 10 | 9  8  7  6 | 5  4  3  2  1  0 |
|-------------------------------------------------------------------------------------------|
|    (unused)    | sf |  mx   |   v   |   cv  |(unused)| lm |  (unused)  |     opcode       |
---------------------------------------------------------------------------------------------
 (unused) = unused, ignored

 sf = shift 12

 mx = matrix selection

 v = source vector

 cv = add vector(translation/back/far color(bugged)/none)

 (unused) = unused, ignored

 lm = limit negative results to 0

 (unused) = unused, ignored

 opcode = operation code 
*/

int32 GTE_Instruction(uint32 instr)
{
 const unsigned code = instr & 0x3F;
 int32 ret = 1;

 FLAGS = 0;

 switch(code)
 {
  default: 
#ifndef PSXDEV_GTE_TESTING
	   PSX_WARNING("[GTE] Unknown instruction code: 0x%02x", code);
#endif
	   break;

  case 0x00:	// alternate?
  case 0x01:
	ret = RTPS(instr);
	break;

/*
  case 0x02:	// UNSTABLE?
	break;

  case 0x03:	// UNSTABLE?
	break;

  case 0x04:	// Probably simple with v,cv,sf,mx,lm ignored.  Same calculation as 0x3B?
	break;

  case 0x05:	// UNSTABLE?
	break;
*/

  case 0x06:
	ret = NCLIP(instr);
	break;

/*
  case 0x07:	// UNSTABLE?
	break;

  case 0x08:	// UNSTABLE?
	break;

  case 0x09:	// UNSTABLE?
	break;

  case 0x0A:	// UNSTABLE?
	break;

  case 0x0B:	// UNSTABLE?
	break;

*/

  case 0x0C:
	ret = OP(instr);
	break;

/*
  case 0x0D:	// UNSTABLE?
	break;

  case 0x0E:	// UNSTABLE?
	break;

  case 0x0F:	// UNSTABLE?
	break;
*/

  case 0x10:
	ret = DPCS(instr);
	break;

  case 0x11:
	ret = INTPL(instr);
	break;

  case 0x12:
	ret = MVMVA(instr);
	break;

  case 0x13:
	ret = NCDS(instr);
	break;

  case 0x14:
	ret = CDP(instr);
	break;


/*
  case 0x15:	// does one push on RGB FIFO, what else...
	break;
*/

  case 0x16:
	ret = NCDT(instr);
	break;

/*
  case 0x17:	// PARTIALLY UNSTABLE(depending on sf or v or cv or mx or lm???), similar behavior under some conditions to 0x16?
	break;

  case 0x18:
	break;

  case 0x19:
	break;
*/

  case 0x1A:	// Alternate for 0x29?
	ret = DCPL(instr);
	break;

  case 0x1B:
	ret = NCCS(instr);
	break;

  case 0x1C:
	ret = CC(instr);
	break;

/*
  case 0x1D:
	break;
*/

  case 0x1E:
	ret = NCS(instr);
	break;

/*
  case 0x1F:
	break;
*/

  case 0x20:
	ret = NCT(instr);
	break;
/*
  case 0x21:
	break;

  case 0x22:	// UNSTABLE?
	break;

  case 0x23:
	break;

  case 0x24:
	break;

  case 0x25:
	break;

  case 0x26:
	break;

  case 0x27:
	break;
*/

  case 0x28:
	ret = SQR(instr);
	break;

  case 0x29:
	ret = DCPL(instr);
	break;

  case 0x2A:
	ret = DPCT(instr);
	break;

/*
  case 0x2B:
	break;

  case 0x2C:
	break;
*/

  case 0x2D:
	ret = AVSZ3(instr);
	break;

  case 0x2E:
	ret = AVSZ4(instr);
	break;

/*
  case 0x2F:	// UNSTABLE?
	break;
*/

  case 0x30:
	ret = RTPT(instr);
	break;

/*
  case 0x31:	// UNSTABLE?
	break;

  case 0x32:	// UNSTABLE?
	break;

  case 0x33:	// UNSTABLE?
	break;

  case 0x34:	// UNSTABLE?
	break;

  case 0x35:	// UNSTABLE?
	break;

  case 0x36:	// UNSTABLE?
	break;

  case 0x37:	// UNSTABLE?
	break;

  case 0x38:
	break;

  case 0x39:	// Probably simple with v,cv,sf,mx,lm ignored.
	break;

  case 0x3A:	// Probably simple with v,cv,sf,mx,lm ignored.
	break;

  case 0x3B:	// Probably simple with v,cv,sf,mx,lm ignored.  Same calculation as 0x04?
	break;

  case 0x3C:	// UNSTABLE?
	break;
*/

  case 0x3D:
	ret = GPF(instr);
	break;

  case 0x3E:
	ret = GPL(instr);
	break;
	
  case 0x3F:
	ret = NCCT(instr);
	break;
 }

 if(FLAGS & 0x7f87e000)
  FLAGS |= 1 << 31;

 CR[31] = FLAGS;

 return(ret - 1);
}

#ifndef PSXDEV_GTE_TESTING
}
#endif
