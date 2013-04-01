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

#include "psx.h"
#include "mdec.h"

#include "../cdrom/SimpleFIFO.h"
#include <math.h>

#if defined(__SSE2__)
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

#if defined(ARCH_POWERPC_ALTIVEC) && defined(HAVE_ALTIVEC_H)
 #include <altivec.h>
#endif

namespace MDFN_IEN_PSX
{


static bool block_ready;
static int32 block_y[2][2][8][8];
static int32 block_cb[8][8];	// [y >> 1][x >> 1]
static int32 block_cr[8][8];	// [y >> 1][x >> 1]

static int32 run_time;
static uint32 Command;

static uint8 QMatrix[2][64];
static uint32 QMIndex;

static int16 IDCTMatrix[64] MDFN_ALIGN(16);
static uint32 IDCTMIndex;

static uint8 QScale;

static int16 Coeff[6][64] MDFN_ALIGN(16);
static uint32 CoeffIndex;
static uint32 DecodeWB;

static SimpleFIFO<uint16> InputBuffer(65536);
static SimpleFIFO<uint16> OutBuffer(384);

static uint32 InCounter;
static bool BlockEnd;
static bool DecodeEnd;

static const uint8 ZigZag[64] =
{
 0x00, 0x01, 0x08, 0x10, 0x09, 0x02, 0x03, 0x0A,
 0x11, 0x18, 0x20, 0x19, 0x12, 0x0B, 0x04, 0x05,
 0x0C, 0x13, 0x1A, 0x21, 0x28, 0x30, 0x29, 0x22,
 0x1B, 0x14, 0x0D, 0x06, 0x07, 0x0E, 0x15, 0x1C,
 0x23, 0x2A, 0x31, 0x38, 0x39, 0x32, 0x2B, 0x24,
 0x1D, 0x16, 0x0F, 0x17, 0x1E, 0x25, 0x2C, 0x33,
 0x3A, 0x3B, 0x34, 0x2D, 0x26, 0x1F, 0x27, 0x2E,
 0x35, 0x3C, 0x3D, 0x36, 0x2F, 0x37, 0x3E, 0x3F
};

void MDEC_Power(void)
{
 run_time = 0;
 block_ready = false;

 Command = 0;
 memset(QMatrix, 0, sizeof(QMatrix));
 QMIndex = 0;

 memset(IDCTMatrix, 0, sizeof(IDCTMatrix));
 IDCTMIndex = 0;

 QScale = 0;

 memset(Coeff, 0, sizeof(Coeff));
 CoeffIndex = 0;
 DecodeWB = 0;

 OutBuffer.Flush();

 InCounter = 0;
 BlockEnd = 0;
 DecodeEnd = 0;
}

int MDEC_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(block_ready),

  SFARRAY32(&block_y[0][0][0][0], sizeof(block_y) / sizeof(block_y[0][0][0][0])),
  SFARRAY32(&block_cb[0][0], sizeof(block_cb) / sizeof(block_cb[0][0])),
  SFARRAY32(&block_cr[0][0], sizeof(block_cr) / sizeof(block_cr[0][0])),

  SFVAR(run_time),
  SFVAR(Command),

  SFARRAY(&QMatrix[0][0], sizeof(QMatrix) / sizeof(QMatrix[0][0])),
  SFVAR(QMIndex),

  SFARRAY16(&IDCTMatrix[0], sizeof(IDCTMatrix) / sizeof(IDCTMatrix[0])),
  SFVAR(IDCTMIndex),

  SFVAR(QScale),

  SFARRAY16(&Coeff[0][0], sizeof(Coeff) / sizeof(Coeff[0][0])),
  SFVAR(CoeffIndex),
  SFVAR(DecodeWB),

#define SFFIFO16(fifoobj)  SFARRAY16(&fifoobj.data[0], fifoobj.data.size()),	\
			 SFVAR(fifoobj.read_pos),				\
			 SFVAR(fifoobj.write_pos),				\
			 SFVAR(fifoobj.in_count)

  SFFIFO16(InputBuffer),
  SFFIFO16(OutBuffer),
#undef SFFIFO

  SFVAR(InCounter),
  SFVAR(BlockEnd),
  SFVAR(DecodeEnd),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MDEC");

 if(load)
 {

 }

 return(ret);
}


static INLINE void WriteQTab(uint8 V)
{
 QMatrix[QMIndex >> 6][QMIndex & 0x3F] = V;
 QMIndex = (QMIndex + 1) & 0x7F;
}

static void DecodeImage(void);
static INLINE void WriteImageData(uint16 V)
{
 const uint32 qmw = (bool)(DecodeWB < 2);

  //printf("MDEC DMA SubWrite: %04x\n", V);

  if(!CoeffIndex)
  {
   if(DecodeWB == 0 && V == 0xFE00)
   {
    InputBuffer.Flush();
    return;
   }
   QScale = V >> 10;
   Coeff[DecodeWB][ZigZag[0]] = sign_10_to_s16(V & 0x3FF) * QMatrix[qmw][0];
   CoeffIndex++;
  }
  else
  {
   if(V == 0xFE00)
   {
    BlockEnd = true;
    while(CoeffIndex < 64)
     Coeff[DecodeWB][ZigZag[CoeffIndex++]] = 0;
   }
   else
   {
    uint32 rlcount = V >> 10;

    for(uint32 i = 0; i < rlcount && CoeffIndex < 64; i++)
    {
     Coeff[DecodeWB][ZigZag[CoeffIndex]] = 0;
     CoeffIndex++;
    }

    if(CoeffIndex < 64)
    {
     Coeff[DecodeWB][ZigZag[CoeffIndex]] = (sign_10_to_s16(V & 0x3FF) * QScale * QMatrix[qmw][CoeffIndex]) >> 3;	// Arithmetic right shift or division(negative differs)?
     CoeffIndex++;
    }
   }
  }

  if(CoeffIndex == 64 && BlockEnd)
  {
   BlockEnd = false;
   CoeffIndex = 0;

   //printf("Block %d finished\n", DecodeWB);

   DecodeWB++;
   if(DecodeWB == 6)
   {
    DecodeWB = 0;

    DecodeImage();
   }
  }
}

static void IDCT(int16 *in_coeff, int32 *out_coeff)
{
#if defined(__SSE2__)
 for(int i = 0; i < 8 * 8; i++)
 {
  __m128i sum = _mm_set1_epi32(0);
  __m128i m0 = _mm_load_si128((__m128i *)&IDCTMatrix[((i & 7) * 8) + 0]);
  int32 tmp[4] MDFN_ALIGN(16);

  for(int v = 0; v < 8; v++)
  {
   __m128i c = _mm_load_si128((__m128i *)&in_coeff[v * 8]);
   __m128i m1 = _mm_set1_epi16(IDCTMatrix[(i & ~7) + v]);
   __m128i m = _mm_mulhi_epi16(m0, m1);

   sum = _mm_add_epi32(sum, _mm_madd_epi16(c, m));
  }

  sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, (3 << 0) | (2 << 2) | (1 << 4) | (0 << 6)));
  sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, (1 << 0) | (0 << 2)));
  sum = _mm_srai_epi32(sum, 16);

  //_mm_store_ss((float *)(out_coeff + i), (__m128)sum);
  _mm_store_si128((__m128i*)tmp, sum);

  out_coeff[i] = tmp[0]; //((tmp[0] + tmp[1]) + (tmp[2] + tmp[3])) >> 16;
 }
#elif defined(ARCH_POWERPC_ALTIVEC)
 static const uint8 snail_cake[16] MDFN_ALIGN(16) = {
			  0x00, 0x01, 0x10, 0x11,
			  0x04, 0x05, 0x14, 0x15,
			  0x08, 0x09, 0x18, 0x19,
			  0x0C, 0x0D, 0x1C, 0x1D };
 vector unsigned char snail_pizza = vec_ld(0, snail_cake);

 for(int i = 0; i < 8 * 8; i++)
 {
  vector signed int sum = vec_splat_s32(0);
  vector signed short m0 = vec_ld((((i & 7) * 8) + 0) * sizeof(signed short), IDCTMatrix);
  int32 tmp[4] MDFN_ALIGN(16);

  for(int v = 0; v < 8; v++)
  {
   vector signed short c = vec_ld((v * 8) * sizeof(signed short), in_coeff);
   vector signed short m1 = vec_splats(IDCTMatrix[(i & ~7) + v]);
   #if 0
   vector signed short m = vec_madds(m0, m1, vec_splat_s16(0)); // Has incorrect behavior for what we need.
   #endif
   vector signed int te = vec_mule(m0, m1);
   vector signed int to = vec_mulo(m0, m1);
   vector signed short m = (vector signed short)vec_perm(te, to, snail_pizza);

   sum = vec_msum(c, m, sum);
  }
  vec_st(sum, 0, tmp);
  out_coeff[i] = ((tmp[0] + tmp[1]) + (tmp[2] + tmp[3])) >> 16;
 }
#else
 for(int y = 0; y < 8; y++)
 {
  for(int x = 0; x < 8; x++)
  {
   int32 sum = 0;

   for(int v = 0; v < 8; v++)
   {
    int16 *c = &in_coeff[v * 8];
    int16 *m0 = &IDCTMatrix[(x * 8) + 0];
    int16 *m1 = &IDCTMatrix[(y * 8) + v];

    for(int u = 0; u < 8; u++)
    {
     sum += c[u] * ((m0[u] * m1[0]) >> 16);
    }
   }
   out_coeff[y * 8 + x] = sum >> 16;
  }
 }
#endif
}

static void YCbCr_to_RGB(const int32 y, const int32 cb, const int32 cr, int &r, int &g, int &b)
{
 r = (y + 128) + ((91881 * cr) >> 16);
 g = (y + 128) - ((22525 * cb) >> 16) - ((46812 * cr) >> 16);
 b = (y + 128) + ((116130 * cb) >> 16);

 if(r < 0) r = 0;
 if(r > 255) r = 255;

 if(g < 0) g = 0;
 if(g > 255) g = 255;

 if(b < 0) b = 0;
 if(b > 255) b = 255;
}

static void DecodeImage(void)
{
 //puts("DECODE");
 run_time -= 2048; //4096; //256; //1024; //2048; //8192; //1024; //4096;

 IDCT(Coeff[0], &block_cr[0][0]);
 IDCT(Coeff[1], &block_cb[0][0]);
 IDCT(Coeff[2], &block_y[0][0][0][0]);
 IDCT(Coeff[3], &block_y[0][1][0][0]);
 IDCT(Coeff[4], &block_y[1][0][0][0]);
 IDCT(Coeff[5], &block_y[1][1][0][0]);

 block_ready = true;
}

static void EncodeImage(void)
{
 //printf("ENCODE, %d\n", (Command & 0x08000000) ? 256 : 384);

 block_ready = false;

 if(!(Command & 0x08000000))
 {
  uint8 output[16][16][3];	// [y][x][cc]

  for(int y = 0; y < 16; y++)
  {
   for(int x = 0; x < 16; x++)
   {
    int r, g, b;

    YCbCr_to_RGB(block_y[(y >> 3) & 1][(x >> 3) & 1][y & 7][x & 7], block_cb[y >> 1][x >> 1], block_cr[y >> 1][x >> 1], r, g, b);

    //r = y * 15;
    //g = x * 15;
    //b = 0;

    output[y][x][0] = r;
    output[y][x][1] = g;
    output[y][x][2] = b;
   }
  }

  for(int i = 0; i < 384; i++)
  {
   if(OutBuffer.CanWrite())
    OutBuffer.WriteUnit((&output[0][0][0])[i * 2 + 0] | ((&output[0][0][0])[i * 2 + 1] << 8));
  }
 }
 else
 {
  uint16 pixel_or = (Command & 0x02000000) ? 0x8000 : 0x0000;

  for(int y = 0; y < 16; y++)
  {
   for(int x = 0; x < 16; x++)
   {
    int r, g, b;

    YCbCr_to_RGB(block_y[(y >> 3) & 1][(x >> 3) & 1][y & 7][x & 7], block_cb[y >> 1][x >> 1], block_cr[y >> 1][x >> 1], r, g, b);

    if(OutBuffer.CanWrite())
     OutBuffer.WriteUnit(pixel_or | ((r >> 3) << 0) | ((g >> 3) << 5) | ((b >> 3) << 10));
   }
  }
 }
}

void MDEC_DMAWrite(uint32 V)
{
 if(Command == 0x60000000)
 {
  for(unsigned i = 0; i < 2; i++)
  {
   IDCTMatrix[((IDCTMIndex & 0x7) << 3) | ((IDCTMIndex >> 3) & 0x7)] = (int16)(V & 0xFFFF);
   IDCTMIndex = (IDCTMIndex + 1) & 0x3F;

   V >>= 16;
  }
 }
 else if(Command == 0x40000001)
 {
  for(int i = 0; i < 4; i++)
  {
   WriteQTab((uint8)V);
   V >>= 8;
  }
 }
 else if((Command & 0xF5FF0000) == 0x30000000)
 {
  if(InCounter > 0)
  {
   for(int vi = 0; vi < 2; vi++)
   {
    if(InputBuffer.CanWrite())
     InputBuffer.WriteUnit(V);

    V >>= 16;
   }
   InCounter--;
  }
 }
 else
 {
  printf("MYSTERY1: %08x\n", V);
 }
}

uint32 MDEC_DMARead(void)
{
 uint32 V = 0;

 if((Command & 0xF5FF0000) == 0x30000000 && OutBuffer.CanRead() >= 2)
 {
  V = OutBuffer.ReadUnit() | (OutBuffer.ReadUnit() << 16);
 }
 else
 {
  puts("BONUS GNOMES");
  V = rand();
 }
 return(V);
}

// Test case related to this: GameShark Version 4.0 intro movie(coupled with (clever) abuse of DMA channel 0).
bool MDEC_DMACanWrite(void)
{
 if((Command & 0xF5FF0000) == 0x30000000)
  return(InCounter > 0);

 return(true);	// TODO: Refine/correct.
}

bool MDEC_DMACanRead(void)
{
 return(OutBuffer.CanRead() >= 2); //(OutBuffer.CanRead() >= 2) || ((Command & 0xF5FF0000) != 0x30000000);
}

void MDEC_Write(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 PSX_WARNING("[MDEC] Write: 0x%08x 0x%08x, %d", A, V, timestamp);
 if(A & 4)
 {
  if(V & 0x80000000) // Reset?
  {
   block_ready = false;
   run_time = 0;
   QMIndex = 0;
   IDCTMIndex = 0;

   QScale = 0;

   memset(Coeff, 0, sizeof(Coeff));
   CoeffIndex = 0;
   DecodeWB = 0;

   InputBuffer.Flush();
   OutBuffer.Flush();

   InCounter = 0;
   BlockEnd = false;
  }
 }
 else
 {
  Command = V;

  if((Command & 0xF5FF0000) == 0x30000000)
  {
   InputBuffer.Flush();
   OutBuffer.Flush();

   block_ready = false;
   BlockEnd = false;
   CoeffIndex = 0;
   DecodeWB = 0;

   InCounter = V & 0xFFFF;
  }
  else if(Command == 0x60000000)
  {
   IDCTMIndex = 0;
  }
  else if(Command == 0x40000001)
  {
   QMIndex = 0;
  }
 }
}

uint32 MDEC_Read(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 ret = 0;

 if(A & 4)
 {
  ret = 0;

  if(InputBuffer.CanRead())
   ret |= 0x20000000;
 }
 else
 {
  ret = Command;
 }

 //PSX_WARNING("[MDEC] Read: 0x%08x 0x%08x -- %d %d", A, ret, InputBuffer.CanRead(), InCounter);

 return(ret);
}

void MDEC_Run(int32 clocks)
{
 run_time += clocks;

 while(run_time > 0)
 {
  run_time--;

  if(block_ready && !OutBuffer.CanRead())
   EncodeImage();

  if(block_ready && OutBuffer.CanRead())
   break;

  if(!InputBuffer.CanRead())
   break;

  WriteImageData(InputBuffer.ReadUnit());
 }

 if(run_time > 0)
  run_time = 0;
}

}
