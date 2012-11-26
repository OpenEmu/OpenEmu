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
#include "timer.h"

/*
 TODO:
	Test and clean up line, particularly polyline, drawing.

	"abe" transparency testing might be not correct, the transparency in regards to mask bit setting and evaluation may not be correct.

	Not everything is returned in the status port read yet(double check).

	"dfe" bit of drawing mode is probably not implemented 100% correctly.

	Initialize more stuff in the Power() function.

	Fix triangle span rendering order(it's bottom-to-up sometimes on the real thing, to avoid negative x step/increment values).
*/

/*
 GPU display timing master clock is nominally 53.693182 MHz for NTSC PlayStations, and 53.203425 MHz for PAL PlayStations.

 Non-interlaced NTSC mode line timing notes(real-world times calculated via PS1 timer and math with nominal CPU clock value):

	263 lines per frame

	~16714.85 us per frame, average.
	~63.55456 us per line, average.

	Multiplying the results of counter 0 in pixel clock mode by the clock divider of the current dot clock mode/width gives a result that's slightly less
	than expected; the dot clock divider is probably being reset each scanline.

 Non-interlaced PAL mode(but with an NTSC source clock in an NTSC PS1; calculated same way as NTSC values):

	314 lines per frame

	~19912.27 us per frame, average.
	~63.41486 us per line, average.

 FB X and Y display positions can be changed during active display; and Y display position appears to be treated as an offset to the current Y readout
 position that gets reset around vsync time.

*/

/*
 Known problematic games to do regression testing on:

	Dukes of Hazzard: Racing For Home
		Sensitive about the GPU busy status flag being set long enough; double-check if we ever make CPU emulation more timing-accurate(
		the fix will likely just involve reducing the timing granularity for DMA and GPU updates).

	Final Fantasy 7
		WHERE DO I BEGIN?!  (Currently broken as of Jan. 1, 2012)

	Pro Pinball (series)
		Sensitive to correct interlace and draw line skipping emulation.

	Valkyrie Profile
		Battle scenes will go all kaka with no graphics updates if GPU LL DMA completes too soon.

*/

namespace MDFN_IEN_PSX
{
//FILE *fp;

static const int32 dither_table[4][4] =
{
 { -4,  0, -3,  1 },
 {  2, -2,  3, -1 },
 { -3,  1, -4,  0 },
 {  3, -1,  2, -2 },
};

PS_GPU::PS_GPU(bool pal_clock_and_tv) : BlitterFIFO(0x20) // 0x10 on actual PS1 GPU, 0x20 here(see comment at top of gpu.h)	// 0x10)
{
 HardwarePALType = pal_clock_and_tv;

 for(int y = 0; y < 4; y++)
  for(int x = 0; x < 4; x++)
   for(int v = 0; v < 512; v++)
   {
    int value = v + dither_table[y][x];

    value >>= 3;
 
    if(value < 0)
     value = 0;

    if(value > 0x1F)
     value = 0x1F;

    DitherLUT[y][x][v] = value;
   }

 if(HardwarePALType == false)	// NTSC clock
 {
  GPUClockRatio = 103896; // 65536 * 53693181.818 / (44100 * 768)
 }
 else	// PAL clock
 {
  GPUClockRatio = 102948; // 65536 * 53203425 / (44100 * 768)
 }

 memset(RGB8SAT_Under, 0, sizeof(RGB8SAT_Under));

 for(int i = 0; i < 256; i++)
  RGB8SAT[i] = i;

 memset(RGB8SAT_Over, 0xFF, sizeof(RGB8SAT_Over));
}

PS_GPU::~PS_GPU()
{

}

void PS_GPU::SoftReset(void) // Control command 0x00
{
 DMAControl = 0;

 if(DrawTimeAvail < 0)
  DrawTimeAvail = 0;

 BlitterFIFO.Flush();
 InCmd = INCMD_NONE;

 DisplayMode = 0;
 DisplayOff = 1;
 DisplayFB_XStart = 0;
 DisplayFB_YStart = 0;

 HorizStart = 0;
 HorizEnd = 0;

 VertStart = 0;
 VertEnd = 0;
}

void PS_GPU::Power(void)
{
 memset(GPURAM, 0, sizeof(GPURAM));

 DMAControl = 0;

 ClipX0 = 0;
 ClipY0 = 0;
 ClipX1 = 1023;
 ClipY1 = 1023;

 OffsX = 0;
 OffsY = 0;

 dtd = false;
 dfe = false;

 MaskSetOR = 0;
 MaskEvalAND = 0;

 tww = 0;
 twh = 0;
 twx = 0;
 twy = 0;

 RecalcTexWindowLUT();

 TexPageX = 0;
 TexPageY = 0;
 SpriteFlip = 0;

 abr = 0;
 TexMode = 0;

 BlitterFIFO.Flush();

 InCmd = INCMD_NONE;
 FBRW_X = 0;
 FBRW_Y = 0;
 FBRW_W = 0;
 FBRW_H = 0;
 FBRW_CurY = 0;
 FBRW_CurX = 0;

 DisplayMode = 0;
 DisplayOff = 1;
 DisplayFB_XStart = 0;
 DisplayFB_YStart = 0;

 HorizStart = 0;
 HorizEnd = 0;

 VertStart = 0;
 VertEnd = 0;

 //
 //
 //
 DisplayFB_CurYOffset = 0;
 DisplayFB_CurLineYReadout = 0;
 DisplayHeightCounter = 0;


 // TODO: factor out in a separate function.
 LinesPerField = 263;
 VisibleStartLine = 23;

 //
 //
 //
 FrameInterlaced = false;
 PALMode = false;
 HeightMode = false;
 scanline = 0;
 field = 0;
 field_atvs = 0;
 PhaseChange = 0;

 //
 //
 //
 DotClockCounter = 0;
 GPUClockCounter = 0;
 LineClockCounter = 3412 - 200;
 LinePhase = 0;

 DrawTimeAvail = 0;

 lastts = 0;
}

void PS_GPU::ResetTS(void)
{
 lastts = 0;
}

#define COORD_FBS 20
#define COORD_MF_INT(n) ((n) << COORD_FBS)

static INLINE int32 COORD_GET_INT(int32 n)
{
 return(n >> COORD_FBS);
}


template<int BlendMode, bool MaskEval_TA, bool textured>
INLINE void PS_GPU::PlotPixel(int32 x, int32 y, uint16 fore_pix)
{
 y &= 511;	// More Y precision bits than GPU RAM installed in (non-arcade, at least) Playstation hardware.

 if(BlendMode >= 0 && (fore_pix & 0x8000))
 {
  uint16 bg_pix = GPURAM[y][x];	// Don't use bg_pix for mask evaluation, it's modified in blending code paths.
  uint16 pix; // = fore_pix & 0x8000;

/*
 static const int32 tab[4][2] =
 {
  { 2,  2 },
  { 4,  4 },
  { 4, -4 },
  { 4,  1 }
 };
*/
  // Efficient 15bpp pixel math algorithms from blargg
  switch(BlendMode)
  {
   case 0:
	bg_pix |= 0x8000;
	pix = ((fore_pix + bg_pix) - ((fore_pix ^ bg_pix) & 0x0421)) >> 1;
	break;
	  
   case 1:
       {
	bg_pix &= ~0x8000;

	uint32 sum = fore_pix + bg_pix;
	uint32 carry = (sum - ((fore_pix ^ bg_pix) & 0x8421)) & 0x8420;

	pix = (sum - carry) | (carry - (carry >> 5));
       }
       break;

   case 2:
       {
	bg_pix |= 0x8000;
        fore_pix &= ~0x8000;

	uint32 diff = bg_pix - fore_pix + 0x108420;
	uint32 borrow = (diff - ((bg_pix ^ fore_pix) & 0x108420)) & 0x108420;

	pix = (diff - borrow) & (borrow - (borrow >> 5));
       }
       break;

   case 3:
       {
	bg_pix &= ~0x8000;
	fore_pix = ((fore_pix >> 2) & 0x1CE7) | 0x8000;

	uint32 sum = fore_pix + bg_pix;
	uint32 carry = (sum - ((fore_pix ^ bg_pix) & 0x8421)) & 0x8420;

	pix = (sum - carry) | (carry - (carry >> 5));
       }
       break;
  }

  if(!MaskEval_TA || !(GPURAM[y][x] & 0x8000))
   GPURAM[y][x] = (textured ? pix : (pix & 0x7FFF)) | MaskSetOR;
 }
 else
 {
  if(!MaskEval_TA || !(GPURAM[y][x] & 0x8000))
   GPURAM[y][x] = (textured ? fore_pix : (fore_pix & 0x7FFF)) | MaskSetOR;
 }
}

INLINE uint16 PS_GPU::ModTexel(uint16 texel, int32 r, int32 g, int32 b, const int32 dither_x, const int32 dither_y)
{
 uint16 ret = texel & 0x8000;

 ret |= DitherLUT[dither_y][dither_x][(((texel & 0x1F) * r) >> (5 - 1))] << 0;
 ret |= DitherLUT[dither_y][dither_x][(((texel & 0x3E0) * g) >> (10 - 1))] << 5;
 ret |= DitherLUT[dither_y][dither_x][(((texel & 0x7C00) * b) >> (15 - 1))] << 10;

 return(ret);
}

template<uint32 TexMode_TA>
INLINE uint16 PS_GPU::GetTexel(const uint32 clut_offset, int32 u_arg, int32 v_arg)
{
     uint32 u = TexWindowXLUT[u_arg];
     uint32 v = TexWindowYLUT[v_arg];
     uint32 fbtex_x = TexPageX + (u >> (2 - TexMode_TA));
     uint32 fbtex_y = TexPageY + v;
     uint16 fbw = GPURAM[fbtex_y][fbtex_x & 1023];

     if(TexMode_TA != 2)
     {
      if(TexMode_TA == 0)
       fbw = (fbw >> ((u & 3) * 4)) & 0xF;
      else
       fbw = (fbw >> ((u & 1) * 8)) & 0xFF;
 
      fbw = GPURAM[(clut_offset >> 10) & 511][(clut_offset + fbw) & 1023];
     }

     return(fbw);
}

INLINE bool PS_GPU::LineSkipTest(unsigned y)
{
 //DisplayFB_XStart >= OffsX && DisplayFB_YStart >= OffsY &&
 // ((y & 1) == (DisplayFB_CurLineYReadout & 1))

 if((DisplayMode & 0x24) != 0x24)
  return false;

 if(HeightMode && !dfe && ((y & 1) == ((DisplayFB_YStart + !field_atvs) & 1))/* && !DisplayOff*/) //&& (y >> 1) >= DisplayFB_YStart && (y >> 1) < (DisplayFB_YStart + (VertEnd - VertStart)))
  return true;

 return false;
}

#include "gpu_polygon.inc"
#include "gpu_sprite.inc"
#include "gpu_line.inc"

// Special RAM write mode(16 pixels at a time), does *not* appear to use mask drawing environment settings.
void PS_GPU::Command_FBFill(const uint32 *cb)
{
 int32 r = cb[0] & 0xFF;
 int32 g = (cb[0] >> 8) & 0xFF;
 int32 b = (cb[0] >> 16) & 0xFF;
 const uint16 fill_value = ((r >> 3) << 0) | ((g >> 3) << 5) | ((b >> 3) << 10);

 int32 destX = (cb[1] >>  0) & 0x3F0;
 int32 destY = (cb[1] >> 16) & 0x3FF;

 int32 width =  (((cb[2] >> 0) & 0x3FF) + 0xF) & ~0xF;
 int32 height = (cb[2] >> 16) & 0x1FF;

 //printf("[GPU] FB Fill %d:%d w=%d, h=%d\n", destX, destY, width, height);
 DrawTimeAvail -= 46;	// Approximate
 DrawTimeAvail -= ((width * height) >> 3) + (height * 9);

 for(int32 y = 0; y < height; y++)
 {
  const int32 d_y = (y + destY) & 511;

  if(LineSkipTest(d_y))
   continue;

  for(int32 x = 0; x < width; x++)
  {
   const int32 d_x = (x + destX) & 1023;

   GPURAM[d_y][d_x] = fill_value;
  }
 }

}

void PS_GPU::Command_FBCopy(const uint32 *cb)
{
 int32 sourceX = (cb[1] >> 0) & 0x3FF;
 int32 sourceY = (cb[1] >> 16) & 0x3FF;
 int32 destX = (cb[2] >> 0) & 0x3FF;
 int32 destY = (cb[2] >> 16) & 0x3FF;

 int32 width = (cb[3] >> 0) & 0x3FF;
 int32 height = (cb[3] >> 16) & 0x1FF;

 if(!width)
  width = 0x400;

 if(!height)
  height = 0x200;

 //printf("FB Copy: %d %d %d %d %d %d\n", sourceX, sourceY, destX, destY, width, height);

 DrawTimeAvail -= (width * height) * 2;

 for(int32 y = 0; y < height; y++)
 {
  for(int32 x = 0; x < width; x++)
  {
   int32 s_y = (y + sourceY) & 511;
   int32 s_x = (x + sourceX) & 1023;
   int32 d_y = (y + destY) & 511;
   int32 d_x = (x + destX) & 1023;

   if(!(GPURAM[d_y][d_x] & MaskEvalAND))
    GPURAM[d_y][d_x] = GPURAM[s_y][s_x] | MaskSetOR;
  }
 }

}

void PS_GPU::Command_FBWrite(const uint32 *cb)
{
 assert(InCmd == INCMD_NONE);

 FBRW_X = (cb[1] >>  0) & 0x3FF;
 FBRW_Y = (cb[1] >> 16) & 0x3FF;

 FBRW_W = (cb[2] >>  0) & 0x7FF;
 FBRW_H = (cb[2] >> 16) & 0x3FF;

 if(FBRW_W > 0x400)
  FBRW_W &= 0x3FF;

 if(FBRW_H > 0x200)
  FBRW_H &= 0x1FF;

 FBRW_CurX = FBRW_X;
 FBRW_CurY = FBRW_Y;

 if(FBRW_W != 0 && FBRW_H != 0)
  InCmd = INCMD_FBWRITE;
}

void PS_GPU::Command_FBRead(const uint32 *cb)
{
 assert(InCmd == INCMD_NONE);

 FBRW_X = (cb[1] >>  0) & 0x3FF;
 FBRW_Y = (cb[1] >> 16) & 0x3FF;

 FBRW_W = (cb[2] >>  0) & 0x7FF;
 FBRW_H = (cb[2] >> 16) & 0x3FF;

 if(FBRW_W > 0x400)
  FBRW_W &= 0x3FF;

 if(FBRW_H > 0x200)
  FBRW_H &= 0x1FF;

 FBRW_CurX = FBRW_X;
 FBRW_CurY = FBRW_Y;

 if(FBRW_W != 0 && FBRW_H != 0)
  InCmd = INCMD_FBREAD;
}


void PS_GPU::RecalcTexWindowLUT(void)
{
 const unsigned TexWindowX_AND = ~(tww << 3);
 const unsigned TexWindowX_OR = (twx & tww) << 3;

 const unsigned TexWindowY_AND = ~(twh << 3);
 const unsigned TexWindowY_OR = (twy & twh) << 3;

// printf("TWX: 0x%02x, TWW: 0x%02x\n", twx, tww);
// printf("TWY: 0x%02x, TWH: 0x%02x\n", twy, twh);

 for(unsigned x = 0; x < 256; x++)
 {
  TexWindowXLUT[x] = (x & TexWindowX_AND) | TexWindowX_OR;
 }

 for(unsigned y = 0; y < 256; y++)
 {
  TexWindowYLUT[y] = (y & TexWindowY_AND) | TexWindowY_OR;
 }

 memset(TexWindowXLUT_Pre, TexWindowXLUT[0], sizeof(TexWindowXLUT_Pre));
 memset(TexWindowXLUT_Post, TexWindowXLUT[255], sizeof(TexWindowXLUT_Post));

 memset(TexWindowYLUT_Pre, TexWindowYLUT[0], sizeof(TexWindowYLUT_Pre));
 memset(TexWindowYLUT_Post, TexWindowYLUT[255], sizeof(TexWindowYLUT_Post));
}

void PS_GPU::Command_DrawMode(const uint32 *cb)
{
 TexPageX = (*cb & 0xF) * 64;
 TexPageY = (*cb & 0x10) * 16;

 SpriteFlip = *cb & 0x3000;

 abr = (*cb >> 5) & 0x3;
 TexMode = (*cb >> 7) & 0x3;

 dtd = (*cb >> 9) & 1;
 dfe = (*cb >> 10) & 1;
 //printf("*******************DFE: %d -- scanline=%d\n", dfe, scanline);
}

void PS_GPU::Command_TexWindow(const uint32 *cb)
{
 tww = (*cb & 0x1F);
 twh = ((*cb >> 5) & 0x1F);
 twx = ((*cb >> 10) & 0x1F);
 twy = ((*cb >> 15) & 0x1F);

 RecalcTexWindowLUT();
}

void PS_GPU::Command_Clip0(const uint32 *cb)
{
 ClipX0 = *cb & 1023;
 ClipY0 = (*cb >> 10) & 1023;
}

void PS_GPU::Command_Clip1(const uint32 *cb)
{
 ClipX1 = *cb & 1023;
 ClipY1 = (*cb >> 10) & 1023;
}

void PS_GPU::Command_DrawingOffset(const uint32 *cb)
{
 OffsX = sign_x_to_s32(11, (*cb & 2047));
 OffsY = sign_x_to_s32(11, ((*cb >> 11) & 2047));

 //fprintf(stderr, "[GPU] Drawing offset: %d(raw=%d) %d(raw=%d) -- %d\n", OffsX, *cb, OffsY, *cb >> 11, scanline);
}

void PS_GPU::Command_MaskSetting(const uint32 *cb)
{
 //printf("Mask setting: %08x\n", *cb);
 MaskSetOR = (*cb & 1) ? 0x8000 : 0x0000;
 MaskEvalAND = (*cb & 2) ? 0x8000 : 0x0000;
}

void PS_GPU::Command_ClearCache(const uint32 *cb)
{

}

CTEntry PS_GPU::Commands[4][256] =
{
 #define BLENDMODE_MAC 0
 {
  #include "gpu_command_table.inc"
 },
 #undef BLENDMODE_MAC

 #define BLENDMODE_MAC 1
 {
  #include "gpu_command_table.inc"
 },
 #undef BLENDMODE_MAC

 #define BLENDMODE_MAC 2
 {
  #include "gpu_command_table.inc"
 },
 #undef BLENDMODE_MAC

 #define BLENDMODE_MAC 3
 {
  #include "gpu_command_table.inc"
 },
 #undef BLENDMODE_MAC
};

static uint64 PrimitiveCounter[256] = { 0 }; // Debug

void PS_GPU::ProcessFIFO(void)
{
 if(!BlitterFIFO.CanRead())
  return;

 switch(InCmd)
 {
  default:
	abort();
	break;

  case INCMD_NONE:
	break;

  case INCMD_FBREAD:
	puts("BOGUS SALAMANDERS, CAPTAIN!");
	return;

  case INCMD_FBWRITE:
       {
  	uint32 InData = BlitterFIFO.ReadUnit();

  	for(int i = 0; i < 2; i++)
  	{
   	 if(!(GPURAM[FBRW_CurY & 511][FBRW_CurX & 1023] & MaskEvalAND))
    	  GPURAM[FBRW_CurY & 511][FBRW_CurX & 1023] = InData | MaskSetOR;

	 FBRW_CurX++;
   	 if(FBRW_CurX == (FBRW_X + FBRW_W))
	 {
	  FBRW_CurX = FBRW_X;
	  FBRW_CurY++;
	  if(FBRW_CurY == (FBRW_Y + FBRW_H))
	  {
	   InCmd = INCMD_NONE;
	   break;	// Break out of the for() loop.
	  }
	 }
	 InData >>= 16;
  	}
  	return;
       }
       break;

  case INCMD_QUAD:
       {
	if(DrawTimeAvail < 0)
	 return;

	const uint32 cc = InCmd_CC;
	const CTEntry *command = &Commands[abr][cc];
	unsigned vl = 1 + (bool)(cc & 0x4) + (bool)(cc & 0x10);
	uint32 CB[3];

	if(BlitterFIFO.CanRead() >= vl)
	{
	 for(unsigned i = 0; i < vl; i++)
	 {
	  CB[i] = BlitterFIFO.ReadUnit();
	 }

	 ((this)->*(command->func[TexMode | (MaskEvalAND ? 0x4 : 0x0)]))(CB);
	}
	return;
       }
       break;

  case INCMD_PLINE:
       {
  	if(DrawTimeAvail < 0)
	 return;

	const uint32 cc = InCmd_CC;
	const CTEntry *command = &Commands[abr][cc];
	unsigned vl = 1 + (bool)(InCmd_CC & 0x10);
	uint32 CB[2];

  	if((BlitterFIFO.ReadUnit(true) & 0xF000F000) == 0x50005000)
	{
	 BlitterFIFO.ReadUnit();
	 InCmd = INCMD_NONE;
	 return;
	}

	if(BlitterFIFO.CanRead() >= vl)
	{
	 for(unsigned i = 0; i < vl; i++)
	 {
	  CB[i] = BlitterFIFO.ReadUnit();
	 }

	 ((this)->*(command->func[TexMode | (MaskEvalAND ? 0x4 : 0x0)]))(CB);
	}
	return;
       }
       break;
 }

 const uint32 cc = BlitterFIFO.ReadUnit(true) >> 24;
 const CTEntry *command = &Commands[0][cc];

 if(DrawTimeAvail < 0 && !command->ss_cmd)
  return;

 if(BlitterFIFO.CanRead() >= command->len)
 {
  uint32 CB[0x10];

  for(unsigned i = 0; i < command->len; i++)
   CB[i] = BlitterFIFO.ReadUnit();

  if(!command->ss_cmd)
   DrawTimeAvail -= 2;

  PrimitiveCounter[cc]++;

  if(!command->func[TexMode])
  {
   //if(CB[0])
   // PSX_WARNING("[GPU] Unknown command: %08x, %d", CB[0], scanline);
  }
  else
  {
#if 0
   PSX_WARNING("[GPU] Command: %08x %s %d %d %d", CB[0], command->name, command->len, scanline, DrawTimeAvail);
   if(1)
   {
    printf("[GPU]    ");
    for(unsigned i = 0; i < command->len; i++)
     printf("0x%08x ", CB[i]);
    printf("\n");
   }
#endif
   // A very very ugly kludge to support texture mode specialization. fixme/cleanup/SOMETHING in the future.
   if(cc >= 0x20 && cc <= 0x3F && (cc & 0x4))
   {
    uint32 tpage;

    tpage = CB[4 + ((cc >> 4) & 0x1)] >> 16;

    TexPageX = (tpage & 0xF) * 64;
    TexPageY = (tpage & 0x10) * 16;

    SpriteFlip = tpage & 0x3000;

    abr = (tpage >> 5) & 0x3;
    TexMode = (tpage >> 7) & 0x3;
   }

   command = &Commands[abr][cc];

   //int32 olddt = DrawTimeAvail;
   ((this)->*(command->func[TexMode | (MaskEvalAND ? 0x4 : 0x0)]))(CB);
   //printf("COMMAND: %08x -- %8d ---- scanline=%d -- adta=%8d\n", CB[0], DrawTimeAvail - olddt, scanline, DrawTimeAvail);
  }
 }
}

INLINE void PS_GPU::WriteCB(uint32 InData)
{
 if(BlitterFIFO.CanRead() >= 0x10 && (InCmd != INCMD_NONE || (BlitterFIFO.CanRead() - 0x10) >= Commands[0][BlitterFIFO.ReadUnit(true) >> 24].fifo_fb_len))
 {
  MDFN_DispMessage("GPU FIFO overflow!!!");
  fprintf(stderr, "GPU FIFO overflow!!!");
  return;
 }

 BlitterFIFO.WriteUnit(InData);
 ProcessFIFO();
}

void PS_GPU::Write(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 V <<= (A & 3) * 8;

 if(A & 4)	// GP1 ("Control")
 {
  uint32 command = V >> 24;

  //fputc(1, fp);
  //fwrite(&V, 1, 4, fp);


  V &= 0x00FFFFFF;

  //PSX_WARNING("[GPU] Control command: %02x %06x %d", command, V, scanline);

  switch(command)
  {
   default: PSX_WARNING("[GPU] Unknown control command %02x - %06x", command, V);
	    break;

   case 0x00:	// Reset GPU
	SoftReset();
	break;

   case 0x01:	// Reset command buffer
	if(DrawTimeAvail < 0)
	 DrawTimeAvail = 0;
	BlitterFIFO.Flush();
	InCmd = INCMD_NONE;
	break;

   case 0x02: 	// Reset IRQ ???
   	break;

   case 0x03:	// Display enable
	DisplayOff = V & 1;
	break;

   case 0x04:	// DMA Setup
	DMAControl = V & 0x3;
	break;

   case 0x05:	// Start of display area in framebuffer
	DisplayFB_XStart = V & 0x3FF;
	DisplayFB_YStart = (V >> 10) & 0x1FF;
	//printf("Xstart: %d -- Ystart: %d -- scanline=%d\n", DisplayFB_XStart, DisplayFB_YStart, scanline);
	break;

   case 0x06:	// Horizontal display range
	HorizStart = V & 0xFFF;
	HorizEnd = (V >> 12) & 0xFFF;
	break;

   case 0x07:
	VertStart = V & 0x3FF;
	VertEnd = (V >> 10) & 0x3FF;	// & 0x3FF or & 0x7FF?
	//printf("VS: %04x\n", VertStart);
	break;

   case 0x08:
	DisplayMode = V & 0xFF;
	//printf("DM: %02x, %d\n", V, scanline);
	break;

   case 0x10:	// GPU info(?)
	switch(V & 0xF)
	{
	 default: PSX_WARNING("[GPU] Unknown control command GPU info param - %01x", V & 0xF);
		  DataReadBuffer = 0;
		  //assert(0);
		  break;

	 case 0x2: DataReadBuffer = (tww << 0) | (twh << 5) | (twx << 10) | (twy << 15);
		   break;

	 case 0x3: DataReadBuffer = (ClipY0 << 10) | ClipX0;
		   break;

 	 case 0x4: DataReadBuffer = (ClipY1 << 10) | ClipX1;
		   break;

	 case 0x5:
	 case 0x6: DataReadBuffer = (OffsX & 2047) | ((OffsY & 2047) << 11);
		   break;

	 case 0x7: DataReadBuffer = 2;
		   break;
	}
	break;

  }
 }
 else		// GP0 ("Data")
 {
  //uint32 command = V >> 24;
  //printf("Meow command: %02x\n", command);
  //assert(!(DMAControl & 2));
  WriteCB(V);
 }
}


void PS_GPU::WriteDMA(uint32 V)
{
 WriteCB(V);
}

uint32 PS_GPU::Read(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 ret = 0;

 if(A & 4)	// Status
 {
  ret = (((DisplayMode << 1) & 0x7F) | ((DisplayMode >> 6) & 1)) << 16;

  ret |= DMAControl << 29;

  ret |= (DisplayFB_CurLineYReadout & 1) << 31;

  ret |= field << 13;

  if(DMAControl & 0x02)
   ret |= 1 << 25;

  ret |= DisplayOff << 23;

  if(InCmd == INCMD_NONE && DrawTimeAvail >= 0 && BlitterFIFO.CanRead() == 0x00)	// GPU idle bit.
   ret |= 1 << 26;

  if(InCmd == INCMD_FBREAD)	// Might want to more accurately emulate this in the future?
   ret |= (1 << 27);

  ret |= CalcFIFOReadyBit() << 28;		// FIFO has room bit? (kinda).

  //
  //
  ret |= TexPageX >> 6;
  ret |= TexPageY >> 4;
  ret |= abr << 5;
  ret |= TexMode << 7;

  ret |= dtd << 9;
  ret |= dfe << 10;

  if(MaskSetOR)
   ret |= 1 << 11;

  if(MaskEvalAND)
   ret |= 1 << 12;
 }
 else		// "Data"
 {
  if(InCmd == INCMD_FBREAD)
  {
   DataReadBuffer = 0;
   for(int i = 0; i < 2; i++)
   {
    DataReadBuffer |= GPURAM[FBRW_CurY & 511][FBRW_CurX & 1023] << (i * 16);

    FBRW_CurX++;
    if(FBRW_CurX == (FBRW_X + FBRW_W))
    {
     FBRW_CurX = FBRW_X;
     FBRW_CurY++;
     if(FBRW_CurY == (FBRW_Y + FBRW_H))
     {
      InCmd = INCMD_NONE;
      break;
     }
    }
   }
  }

  ret = DataReadBuffer;
 }

 if(DMAControl & 2)
 {
  //PSX_WARNING("[GPU READ WHEN (DMACONTROL&2)] 0x%08x - ret=0x%08x, scanline=%d", A, ret, scanline);
 }

 return(ret >> ((A & 3) * 8));
}

INLINE void PS_GPU::ReorderRGB_Var(uint32 out_Rshift, uint32 out_Gshift, uint32 out_Bshift, bool bpp24, const uint16 *src, uint32 *dest, const int32 dx_start, const int32 dx_end, int32 fb_x)
{
     if(bpp24)	// 24bpp
     {
      for(int32 x = dx_start; x < dx_end; x++)
      {
       uint32 srcpix;

       srcpix = src[(fb_x >> 1) + 0] | (src[((fb_x >> 1) + 1) & 0x7FF] << 16);
       srcpix >>= (fb_x & 1) * 8;

       dest[x] = (((srcpix >> 0) << out_Rshift) & (0xFF << out_Rshift)) | (((srcpix >> 8) << out_Gshift) & (0xFF << out_Gshift)) |
       		 (((srcpix >> 16) << out_Bshift) & (0xFF << out_Bshift));

       fb_x = (fb_x + 3) & 0x7FF;
      }
     }				// 15bpp
     else
     {
      for(int32 x = dx_start; x < dx_end; x++)
      {
       uint32 srcpix = src[fb_x >> 1];

       //dest[x] = (((srcpix >> 0) << out_Rshift) & (0xFF << out_Rshift)) | (((srcpix >> 8) << out_Gshift) & (0xFF << out_Gshift)) |
       //		 (((srcpix >> 16) << out_Bshift) & (0xFF << out_Bshift));
       dest[x] = OutputLUT[srcpix & 0x7FFF];

       fb_x = (fb_x + 2) & 0x7FF;
      }
     }

}


template<uint32 out_Rshift, uint32 out_Gshift, uint32 out_Bshift>
void PS_GPU::ReorderRGB(bool bpp24, const uint16 *src, uint32 *dest, const int32 dx_start, const int32 dx_end, int32 fb_x)
{
 ReorderRGB_Var(out_Rshift, out_Gshift, out_Bshift, bpp24, src, dest, dx_start, dx_end, fb_x);
}

pscpu_timestamp_t PS_GPU::Update(const pscpu_timestamp_t sys_timestamp)
{
 static const uint32 DotClockRatios[5] = { 10, 8, 5, 4, 7 };
 const uint32 dmc = (DisplayMode & 0x40) ? 4 : (DisplayMode & 0x3);
 const uint32 dmw = 2720 / DotClockRatios[dmc];	// Must be <= 768

 int32 sys_clocks = sys_timestamp - lastts;
 int32 gpu_clocks;

 //printf("GPUISH: %d\n", sys_timestamp - lastts);

 if(!sys_clocks)
  goto TheEnd;

 DrawTimeAvail += sys_clocks << 1;

 if(DrawTimeAvail > 256)
  DrawTimeAvail = 256;

 ProcessFIFO();

 //puts("GPU Update Start");

 GPUClockCounter += (uint64)sys_clocks * GPUClockRatio;

 gpu_clocks = GPUClockCounter >> 16;
 GPUClockCounter -= gpu_clocks << 16;

 while(gpu_clocks > 0)
 {
  int32 chunk_clocks = gpu_clocks;
  int32 dot_clocks;

  if(chunk_clocks > LineClockCounter)
  {
   //printf("Chunk: %u, LCC: %u\n", chunk_clocks, LineClockCounter);
   chunk_clocks = LineClockCounter;
  }

  gpu_clocks -= chunk_clocks;
  LineClockCounter -= chunk_clocks;

  DotClockCounter += chunk_clocks;
  dot_clocks = DotClockCounter / DotClockRatios[DisplayMode & 0x3];
  DotClockCounter -= dot_clocks * DotClockRatios[DisplayMode & 0x3];

  TIMER_AddDotClocks(dot_clocks);


  if(!LineClockCounter)
  {
   PSX_SetEventNT(PSX_EVENT_TIMER, TIMER_Update(sys_timestamp));  // We could just call this at the top of GPU_Update(), but do it here for slightly less CPU usage(presumably).

   LinePhase = (LinePhase + 1) & 1;

   if(LinePhase)
   {
    TIMER_SetHRetrace(true);
    LineClockCounter = 200;
    TIMER_ClockHRetrace();
   }
   else
   {
    TIMER_SetHRetrace(false);
    if(PALMode)
     LineClockCounter = 3405 - 200;
    else
     LineClockCounter = 3412 + PhaseChange - 200;

    scanline = (scanline + 1) % LinesPerField;
    PhaseChange = !PhaseChange;

    if(scanline == (LinesPerField - 1))
    {
     //printf("Exit: scanline=%u, st=%u\n", scanline, sys_timestamp);
     PSX_RequestMLExit();
    }

#ifdef WANT_DEBUGGER
    DBG_GPUScanlineHook(scanline);
#endif

    // printf("[GPU] DTA scanline=%3d --- %8d\n", scanline, DrawTimeAvail);

    if(scanline == 0)
    {
#if 0
	printf("Primitive Counts for previous frame:\n");
	for(unsigned i = 0; i < 256; i++)
	 if(PrimitiveCounter[i])
	 {
	  printf("0x%02x: %8u\n", i, (unsigned int)PrimitiveCounter[i]);
	 }

	printf("\n");
	memset(PrimitiveCounter, 0, sizeof(PrimitiveCounter));
#endif

     IRQ_Assert(IRQ_VSYNC, true);
     IRQ_Assert(IRQ_VSYNC, false);
    }

    // Might not be right:
    if(scanline == 0)
     TIMER_SetVBlank(true);
    else if(scanline == VisibleStartLine)
     TIMER_SetVBlank(false);


    if(scanline == 0)
    {
     field_atvs = field;
     DisplayHeightCounter = 0;

     PALMode = (bool)(DisplayMode & 0x08);

     if(PALMode)	// PAL
      VisibleStartLine = 26;
     else	// NTSC
      VisibleStartLine = 22;

     FrameInterlaced = (bool)(DisplayMode & 0x20);
     HeightMode = (bool)(DisplayMode & 0x04) && FrameInterlaced;

     if(espec)
     {
      if(PALMode != HardwarePALType)
      {
       const uint32 black = surface->MakeColor(0, 0, 0);

       DisplayRect->w = 384;
       DisplayRect->h = (HardwarePALType ? 288 : 240);

       for(int32 y = 0; y < DisplayRect->h; y++)
       {
        uint32 *dest = surface->pixels + y * surface->pitch32;

        LineWidths[y].x = 0;
        LineWidths[y].w = 384;

        for(int32 x = 0; x < 384; x++)
        {
         dest[x] = black;
        }
       }
       char buffer[256];

       trio_snprintf(buffer, sizeof(buffer), _("VIDEO STANDARD MISMATCH"));
       DrawTextTrans(surface->pixels + ((DisplayRect->h / 2) - (13 / 2)) * surface->pitch32, surface->pitch32 << 2, DisplayRect->w, (UTF8*)buffer,
		surface->MakeColor(0x00, 0xFF, 0x00), true, MDFN_FONT_6x13_12x13);
      }
      else
      {
       espec->InterlaceOn = FrameInterlaced;
       espec->InterlaceField = !field;
       DisplayRect->h = (HardwarePALType ? 288 : 240) << FrameInterlaced;
      }
     }

     DisplayFB_CurYOffset = 0;
    }

    const uint32 VS_Adjust = 7; //PALMode ? (34 - 6) : 7;

    if(scanline == (5U + field_atvs))
    {
     if(FrameInterlaced)
     {
      skip = false;
      field = !field;

      if(PALMode)	// PAL
       LinesPerField = 313 - field;
      else			// NTSC
       LinesPerField = 263 - field;
     }
     else
     {
      field = 0;	// May not be correct.

      if(PALMode)	// PAL
       LinesPerField = 314;
      else			// NTSC
       LinesPerField = 263;
     }
    }

    if(scanline == (VertStart + VS_Adjust))
    {
     DisplayHeightCounter = (VertEnd - VertStart);

     // Note to self: X-Men Mutant Academy relies on this being set on the proper scanline in 480i mode(otherwise it locks up on startup).
     if(HeightMode)
      DisplayFB_CurYOffset = field;

     //printf("%d\n", DisplayFB_CurYOffset);
    }

    // Needs to occur even in vblank.
    DisplayFB_CurLineYReadout = (DisplayFB_YStart + DisplayFB_CurYOffset) & 0x1FF;

    if(scanline >= VisibleStartLine && scanline < (VisibleStartLine + (HardwarePALType ? 288 : 240)) && !skip && espec && PALMode == HardwarePALType)
    {
     uint32 *dest;	// = surface->pixels + (scanline - VisibleStartLine) * surface->pitch32;
     int32 dest_line;
     int32 fb_x = DisplayFB_XStart * 2;
     int32 dx_start = HorizStart, dx_end = HorizEnd;

     dest_line = ((scanline - VisibleStartLine) << FrameInterlaced) + field;
     dest = surface->pixels + dest_line * surface->pitch32;

     if(dx_end < dx_start)
      dx_end = dx_start;

     dx_start = dx_start / DotClockRatios[dmc];
     dx_end = dx_end / DotClockRatios[dmc];

     dx_start -= 528 / DotClockRatios[dmc];
     dx_end -= 528 / DotClockRatios[dmc];

     if(dx_start < 0)
     {
      fb_x -= dx_start * ((DisplayMode & 0x10) ? 3 : 2);
      fb_x &= 0x7FF; //0x3FF;
      dx_start = 0;
     }

     if((uint32)dx_end > dmw)
      dx_end = dmw;

     if(!DisplayHeightCounter || DisplayOff)
      dx_start = dx_end = 0;

     // TODO, but there are problems with this, as not all blitter busy cycles(crudely abstracted with DrawTimeAvail) are GPU RAM access cycles.
     // Also, it shouldn't be here per-se, since this code won't be all if we're frameskipping or there's a video standard mismatch
     //DrawTimeAvail -= (dx_end - dx_start) + ((DisplayMode & 0x10) ? ((dx_end - dx_start + 1) >> 1) : 0);

     LineWidths[dest_line].x = 0;
     LineWidths[dest_line].w = dmw;

     {
      const uint16 *src = GPURAM[DisplayFB_CurLineYReadout];
      const uint32 black = surface->MakeColor(0, 0, 0);

      for(int32 x = 0; x < dx_start; x++)
       dest[x] = black;

      //printf("%d %d %d - %d %d\n", scanline, dx_start, dx_end, HorizStart, HorizEnd);
      if(surface->format.Rshift == 0 && surface->format.Gshift == 8 && surface->format.Bshift == 16)
       ReorderRGB<0, 8, 16>(DisplayMode & 0x10, src, dest, dx_start, dx_end, fb_x);
      else if(surface->format.Rshift == 8 && surface->format.Gshift == 16 && surface->format.Bshift == 24)
       ReorderRGB<8, 16, 24>(DisplayMode & 0x10, src, dest, dx_start, dx_end, fb_x);
      else if(surface->format.Rshift == 16 && surface->format.Gshift == 8 && surface->format.Bshift == 0)
       ReorderRGB<16, 8, 0>(DisplayMode & 0x10, src, dest, dx_start, dx_end, fb_x);
      else if(surface->format.Rshift == 24 && surface->format.Gshift == 16 && surface->format.Bshift == 8)
       ReorderRGB<24, 16, 8>(DisplayMode & 0x10, src, dest, dx_start, dx_end, fb_x);
      else
       ReorderRGB_Var(surface->format.Rshift, surface->format.Gshift, surface->format.Bshift, DisplayMode & 0x10, src, dest, dx_start, dx_end, fb_x);

      for(uint32 x = dx_end; x < dmw; x++)
       dest[x] = black;
     }

     //if(scanline == 64)
     // printf("%u\n", sys_timestamp - ((uint64)gpu_clocks * 65536) / GPUClockRatio);

     PSX_GPULineHook(sys_timestamp, sys_timestamp - ((uint64)gpu_clocks * 65536) / GPUClockRatio, scanline == 0, dest, &surface->format, dmw, (528 - 146) / DotClockRatios[dmc], (HardwarePALType ? 53203425 : 53693182) / DotClockRatios[dmc]);
    }
    else
    {
     PSX_GPULineHook(sys_timestamp, sys_timestamp - ((uint64)gpu_clocks * 65536) / GPUClockRatio, scanline == 0, NULL, &surface->format, 0, 0, 0);
    }

    if(DisplayHeightCounter)
    {
     if(HeightMode)
      DisplayFB_CurYOffset = (DisplayFB_CurYOffset + 2) & 0x1FF;
     else
      DisplayFB_CurYOffset = (DisplayFB_CurYOffset + 1) & 0x1FF;

     DisplayHeightCounter--;
    }
   }
  }
 }

 //puts("GPU Update End");

 TheEnd:
 lastts = sys_timestamp;

 {
  int32 next_dt = LineClockCounter;

  next_dt = (((int64)next_dt << 16) - GPUClockCounter + GPUClockRatio - 1) / GPUClockRatio;

  next_dt = std::max<int32>(1, next_dt);
  next_dt = std::min<int32>(128, next_dt);

  //printf("%d\n", next_dt);

  return(sys_timestamp + next_dt);
 }
}

void PS_GPU::StartFrame(EmulateSpecStruct *espec_arg)
{
 if(!espec_arg)
 {
  espec = NULL;
  surface = NULL;
  DisplayRect = NULL;
  LineWidths = NULL;
  skip = true;
  return;
 }

 espec = espec_arg;

 surface = espec->surface;
 DisplayRect = &espec->DisplayRect;
 LineWidths = espec->LineWidths;
 skip = espec->skip;

 DisplayRect->x = 0;
 DisplayRect->y = 0;
 DisplayRect->w = 256;
 DisplayRect->h = 240;

 // Clear ~0 state.
 LineWidths[0].x = LineWidths[0].w = 0;

 for(int i = 0; i < 240; i++)
 {
  LineWidths[i].x = 0;
  LineWidths[i].w = 0;
 }

 if(espec->VideoFormatChanged)
 {
  for(int rc = 0; rc < 0x8000; rc++)
  {
   uint32 r, g, b;

   r = ((rc >> 0) & 0x1F) * 255 / 31;
   g = ((rc >> 5) & 0x1F) * 255 / 31;
   b = ((rc >> 10) & 0x1F) * 255 / 31;
   OutputLUT[rc] = espec->surface->format.MakeColor(r, g, b, 0);
  }
 }
}

}
