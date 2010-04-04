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

/* VDC and VCE emulation */

/*
"Tonight I hooked up my Turbo Duo(with no games or CDs in it)'s video output to my PC sound card, recorded it, 
and did a FFT and looked at the spectrum(around the line rate, 15-16KHz), and I also counted the number 
of samples between the ~60Hz peaks(just to verify that the math shown below is remotely accurate).

The spectrum peaked at 15734 Hz.  21477272.727272... / 3 / 15734 = 455.00(CPU cycles per scanline)"
*/

#define VDC_DEBUG(x) 
//printf("%s: %d\n", x, vdc->display_counter);
#define VDC_UNDEFINED(x) { }
//{ printf("%s: %d\n", x, vdc->display_counter); }


#include <math.h>
#include "pce.h"
#include "../video.h"
#include "vdc.h"
#include "huc.h"
#include "adpcm.h"
#include "debug.h"
#include <trio/trio.h>

static uint8 *CustomColorMap = NULL; // 1024 * 3
static uint32 CustomColorMapLen;	// 512 or 1024

static uint32 systemColorMap32[512], bw_systemColorMap32[512];
static uint32 amask;    // Alpha channel maskaroo
static uint32 amask_shift;
static uint32 userle; // User layer enable.
static bool unlimited_sprites;

static const int VRAM_Size = 0x8000;
static const int VRAM_SizeMask = VRAM_Size - 1; //0x7FFF;
static const int VRAM_BGTileNoMask = VRAM_SizeMask / 16; //0x7FF;

#define ULE_BG0		1
#define ULE_SPR0	2
#define ULE_BG1		4
#define ULE_SPR1	8

static const unsigned int bat_width_tab[4] = { 32, 64, 128, 128 };
static const unsigned int bat_width_shift_tab[4] = { 5, 6, 7, 7 };
static const unsigned int bat_height_tab[2] = { 32, 64 };
static const unsigned int ClockModeWidths[3] = { 288, 384, 576 };
static const unsigned int ClockPixelWidths[3] = { 341, 455, 682 };

static unsigned int VDS;
static unsigned int VSW;
static unsigned int VDW;
static unsigned int VCR;
static unsigned int VBlankFL;

extern MDFNGI EmulatedPCE;

typedef struct
{
	uint8 CR;

	bool8 lc263;	// 263 line count if set, 262 if not
	bool8 bw;	// Black and White
	uint8 dot_clock; // Dot Clock(5, 7, or 10 MHz = 0, 1, 2)
	uint16 color_table[0x200];
	uint32 color_table_cache[0x200];
	uint16 ctaddress;
} vce_t;

vce_t vce;

typedef struct
{
        uint32 display_counter;

        int32 sat_dma_slcounter;

        uint8 select;
        uint16 MAWR;    // Memory Address Write Register
        uint16 MARR;    // Memory Address Read Register

	#ifdef WANT_DEBUGGER
	uint16 MAWR_BP, MARR_BP; // Special variables so VRAM breakpoints will work correctly when block transfer instructions are used.
	#endif

        uint16 CR;      // Control Register
        uint16 RCR;     // Raster Compare Register
        uint16 BXR;     // Background X-Scroll Register
        uint16 BYR;     // Background Y-Scroll Register
        uint16 MWR;     // Memory Width Register

        uint16 HSR;     // Horizontal Sync Register
        uint16 HDR;     // Horizontal Display Register
        uint16 VSR;
        uint16 VDR;

        uint16 VCR;
        uint16 DCR;
        uint16 SOUR;
        uint16 DESR;
        uint16 LENR;
        uint16 SATB;

        uint8 sgx_priority;

        uint32 RCRCount;

        uint16 read_buffer;
        uint8 write_latch;
        uint8 status;

        uint16 SAT[0x100];
        uint16 VRAM[VRAM_Size];
        uint8 bg_tile_cache[VRAM_Size / 16][8][8] __attribute__ ((aligned (8))); // Tile, y, x

        uint16 DMAReadBuffer;
        bool8 DMAReadWrite;
        bool8 DMARunning;
        bool8 SATBPending;
        bool8 burst_mode;

        uint32 BG_YOffset;      // Reloaded from BYR at start of display area?
        uint32 BG_XOffset;      // Reloaded from BXR at each scanline, methinks.
} vdc_t;

static int TotalChips = 0;
static vdc_t *vdc_chips[2] = { NULL, NULL };

static ALWAYS_INLINE void FixPCache(int entry)
{
 uint32 *cm32 = vce.bw ? bw_systemColorMap32 : systemColorMap32;

 if(!(entry & 0xFF))
 {
  for(int x = 0; x < 16; x++)
   vce.color_table_cache[(entry & 0x100) + (x << 4)] = cm32[vce.color_table[entry & 0x100]] | amask;
 }
 if(!(entry & 0xF))
  return;

 vce.color_table_cache[entry] = cm32[vce.color_table[entry]];
}

static ALWAYS_INLINE void FixTileCache(vdc_t *which_vdc, uint16 A)
{
 uint32 charname = (A >> 4);
 uint32 y = (A & 0x7);
 uint8 *tc = which_vdc->bg_tile_cache[charname][y];

 uint32 bitplane01 = which_vdc->VRAM[y + charname * 16];
 uint32 bitplane23 = which_vdc->VRAM[y+ 8 + charname * 16];

 for(int x = 0; x < 8; x++)
 {
  uint32 raw_pixel = ((bitplane01 >> x) & 1);
  raw_pixel |= ((bitplane01 >> (x + 8)) & 1) << 1;
  raw_pixel |= ((bitplane23 >> x) & 1) << 2;
  raw_pixel |= ((bitplane23 >> (x + 8)) & 1) << 3;
  tc[7 - x] = raw_pixel;
 }
}

static ALWAYS_INLINE void SetVCECR(uint8 V)
{
 if(((V & 0x80) >> 7) != vce.bw)
 {
  vce.bw = V & 0x80;
  for(int x = 0; x < 512; x++)
   FixPCache(x);
 }

 vce.lc263 = (V & 0x04);
 vce.dot_clock = V & 1;
 if(V & 2)
  vce.dot_clock = 2;
 vce.CR = V;
}

#ifdef WANT_DEBUGGER
static void DoGfxDecode(void);
static uint32 *GfxDecode_Buf = NULL;
static int GfxDecode_Width = 0;
static int GfxDecode_Height = 0;
static int GfxDecode_Line = -1;
static int GfxDecode_Layer = 0;
static int GfxDecode_Scroll = 0;
static int GfxDecode_Pbn = 0;
#endif

static unsigned int frame_counter;
static uint32 ContinuePoint;
static int32 need_vbi[2] = { 0, 0 };
static int32 line_leadin1 = 0;
static int32 magical, cyc_tot;

#define REGSETP(_reg, _data, _msb) { _reg &= 0xFF << ((_msb) ? 0 : 8); _reg |= (_data) << ((_msb) ? 8 : 0); }
#define REGGETP(_reg, _msb) ((_reg >> ((_msb) ? 8 : 0)) & 0xFF)

uint32 VDC_GetRegister(const std::string &oname, std::string *special)
{
 char neoname[256];
 char chip_c = 0;
 int chip = 0;
 std::string name;
 vdc_t *vdc;

 trio_sscanf(oname.c_str(), "%63[^-]%*[-]%c", neoname, &chip_c);

 if(chip_c == 'B')
  chip = 1;

 vdc = vdc_chips[chip];

 name = std::string(neoname);

 if(name == "MAWR")
  return(vdc_chips[chip]->MAWR);
 else if(name == "MARR")
  return(vdc_chips[chip]->MARR);
 else if(name == "CR")
 {
  uint16 value = vdc_chips[chip]->CR;

  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "Sprite Hit IRQ: %s, Sprite Overflow IRQ: %s, RCR IRQ: %s, VBlank IRQ: %s, Sprites: %s, Background: %s", (value & 1) ? "On" : "Off", (value & 2) ? "On" : "Off",
        (value & 4) ? "On" : "Off", (value & 8) ? "On" : "Off", (value & 0x40) ? "On" : "Off", (value & 0x80) ? "On" : "Off");
    *special = std::string(buf);
  }
  return(value);
 }
 else if(name == "RCR")
  return(vdc_chips[chip]->RCR);
 else if(name == "BXR")
  return(vdc_chips[chip]->BXR);
 else if(name == "BYR")
  return(vdc_chips[chip]->BYR);
 else if(name == "MWR")
  return(vdc_chips[chip]->MWR);
 else if(name == "HSR")
  return(vdc_chips[chip]->HSR);
 else if(name == "HDR")
  return(vdc_chips[chip]->HDR);
 else if(name == "VSR")
  return(vdc_chips[chip]->VSR);
 else if(name == "VDR")
  return(vdc_chips[chip]->VDR);
 else if(name == "VCR")
  return(vdc_chips[chip]->VCR);
 else if(name == "DCR")
 {
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "SATB DMA IRQ: %s, VRAM DMA IRQ: %s, DMA Source Address: %s, DMA Dest Address: %s, Auto SATB DMA: %s",
	(vdc->DCR & 0x1) ? "On" : "Off", (vdc->DCR & 0x2) ? "On" : "Off", (vdc->DCR & 0x4) ? "Decrement" : "Increment", (vdc->DCR & 0x8) ? "Decrement" : "Increment", 
	(vdc->DCR & 0x10) ? "On" : "Off");

   *special = std::string(buf);
  }
  return(vdc_chips[chip]->DCR);
 }
 else if(name == "SOUR")
  return(vdc_chips[chip]->SOUR);
 else if(name == "DESR")
  return(vdc_chips[chip]->DESR);
 else if(name == "LENR")
  return(vdc_chips[chip]->LENR);
 else if(name == "SATB")
  return(vdc_chips[chip]->SATB);
 else if(name == "Frm Cntr")
  return(frame_counter);
 else if(name == "VCECR")
  return(vce.CR);
 return(0);
}

void VDC_SetRegister(std::string name, uint32 value)
{
 if(name == "MAWR")
  vdc_chips[0]->MAWR = value & 0xFFFF;
 else if(name == "MARR")
  vdc_chips[0]->MARR = value & 0xFFFF;
 else if(name == "CR")
  vdc_chips[0]->CR = value & 0xFFFF;
 else if(name == "RCR")
  vdc_chips[0]->RCR = value & 0x3FF;
 else if(name == "BXR")
  vdc_chips[0]->BXR = value & 0x3FF;
 else if(name == "BYR")
  vdc_chips[0]->BYR = value & 0x1FF;
 else if(name == "MWR")
  vdc_chips[0]->MWR = value & 0xFFFF;
 else if(name == "HSR")
  vdc_chips[0]->HSR = value & 0xFFFF;
 else if(name == "HDR")
  vdc_chips[0]->HDR = value & 0xFFFF;
 else if(name == "VSR")
  vdc_chips[0]->VSR = value & 0xFFFF;
 else if(name == "VDR")
  vdc_chips[0]->VDR = value & 0xFFFF;
 else if(name == "VCR")
  vdc_chips[0]->VCR = value & 0xFFFF;
 else if(name == "DCR")
  vdc_chips[0]->DCR = value & 0xFFFF;
 else if(name == "SOUR")
  vdc_chips[0]->SOUR = value & 0xFFFF;
 else if(name == "DESR")
  vdc_chips[0]->DESR = value & 0xFFFF;
 else if(name == "LENR")
  vdc_chips[0]->LENR = value & 0xFFFF;
 else if(name == "SATB")
  vdc_chips[0]->SATB = value & 0xFFFF;
 else if(name == "Frm Cntr")
  frame_counter = value % (vce.lc263 ? 263 : 262);
 else if(name == "VCECR")
 {
  SetVCECR(value);
 }
 if(name == "MAWR-B")
  vdc_chips[1]->MAWR = value & 0xFFFF;
 else if(name == "MARR-B")
  vdc_chips[1]->MARR = value & 0xFFFF;
 else if(name == "CR-B")
  vdc_chips[1]->CR = value & 0xFFFF;
 else if(name == "RCR-B")
  vdc_chips[1]->RCR = value & 0x3FF;
 else if(name == "BXR-B")
  vdc_chips[1]->BXR = value & 0x3FF;
 else if(name == "BYR-B")
  vdc_chips[1]->BYR = value & 0x1FF;
 else if(name == "MWR-B")
  vdc_chips[1]->MWR = value & 0xFFFF;
 else if(name == "HSR-B")
  vdc_chips[1]->HSR = value & 0xFFFF;
 else if(name == "HDR-B")
  vdc_chips[1]->HDR = value & 0xFFFF;
 else if(name == "VSR-B")
  vdc_chips[1]->VSR = value & 0xFFFF;
 else if(name == "VDR-B")
  vdc_chips[1]->VDR = value & 0xFFFF;
 else if(name == "VCR-B")
  vdc_chips[1]->VCR = value & 0xFFFF;
 else if(name == "DCR-B")
  vdc_chips[1]->DCR = value & 0xFFFF;
 else if(name == "SOUR-B")
  vdc_chips[1]->SOUR = value & 0xFFFF;
 else if(name == "DESR-B")
  vdc_chips[1]->DESR = value & 0xFFFF;
 else if(name == "LENR-B")
  vdc_chips[1]->LENR = value & 0xFFFF;
 else if(name == "SATB-B")
  vdc_chips[1]->SATB = value & 0xFFFF;
}

static int sgfx = 0;

#ifdef WANT_DEBUGGER
void VDC_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 int chip;

 if(!strcmp(name, "vce"))
 {
  while(Length--)
  {
   Address &= 0x3FF;
   *Buffer = vce.color_table[Address >> 1] >> ((Address & 1) * 8);
   Address++;
   Buffer++;
  }
 }
 else if(trio_sscanf(name, "vdcvram%d", &chip) == 1)
 {
  while(Length--)
  {
   Address &= (VRAM_Size << 1) - 1;
   *Buffer = vdc_chips[chip]->VRAM[Address >> 1] >> ((Address & 1) * 8);
   Address++;
   Buffer++;
  }
 }
}

void VDC_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 int chip;

 if(!strcmp(name, "vce"))
 {
  while(Length--)
  {
   Address &= 0x3FF;
   REGSETP(vce.color_table[Address >> 1], *Buffer & ((Address & 1) ? 0x01 : 0xFF), Address & 1);
   FixPCache(Address >> 1);
   Address++;
   Buffer++;
  }
 }
 else if(trio_sscanf(name, "vdcvram%d", &chip) == 1)
 {
  while(Length--)
  {
   Address &= (VRAM_Size << 1) - 1;
   REGSETP(vdc_chips[chip]->VRAM[Address >> 1], *Buffer, Address & 1);
   FixTileCache(vdc_chips[chip], Address >> 1);
   Address++;
   Buffer++;
  }
 }
}
#endif

typedef struct
{
	uint8 priority[2];
	uint16 winwidths[2];
	uint8 st_mode;
} vpc_t;

vpc_t vpc;

// Some virtual vdc macros to make code simpler to read
#define M_vdc_HSW	(vdc->HSR & 0x1F)	// Horizontal Synchro Width
#define M_vdc_HDS	((vdc->HSR >> 8) & 0x7F) // Horizontal Display Start
#define M_vdc_HDW	(vdc->HDR & 0x7F)	// Horizontal Display Width
#define M_vdc_HDE	((vdc->HDR >> 8) & 0x7F) // Horizontal Display End

#define M_vdc_VSW	(vdc->VSR & 0x1F)	// Vertical synchro width
#define M_vdc_VDS	((vdc->VSR >> 8) & 0xFF) // Vertical Display Start
#define M_vdc_VDW	(vdc->VDR & 0x1FF)	// Vertical Display Width(Height? :b)
#define M_vdc_VCR	(vdc->VCR & 0xFF)

static const unsigned int vram_inc_tab[4] = { 1, 32, 64, 128 };

#define VDCS_CR		0x01 // Sprite #0 collision interrupt occurred
#define VDCS_OR		0x02 // sprite overflow "" ""
#define VDCS_RR		0x04 // RCR             ""  ""
#define VDCS_DS		0x08 // VRAM to SAT DMA completion interrupt occurred
#define VDCS_DV		0x10 // VRAM to VRAM DMA completion interrupt occurred
#define VDCS_VD		0x20 // Vertical blank interrupt occurred
#define VDCS_BSY	0x40 // VDC is waiting for a CPU access slot during the active display area??

void VDC_SetPixelFormat(int rshift, int gshift, int bshift)
{
 int used[4] = {0, 0, 0, 0};

 used[rshift >> 3] = 1;
 used[gshift >> 3] = 1;
 used[bshift >> 3] = 1;

 for(int x = 0; x < 4; x++)
  if(!used[x])
  {
   amask = 1 << (x << 3);
   amask_shift = (x << 3);
  }

 for(int x = 0; x < 512; x++)
 {
  int r, g, b;
  int sc_r, sc_g, sc_b;

  if(CustomColorMap)
  {
   r = CustomColorMap[x * 3 + 0];
   g = CustomColorMap[x * 3 + 1];
   b = CustomColorMap[x * 3 + 2];
  }
  else
  {
   b = 36 * (x & 0x007);
   r = 36 * ((x & 0x038) >> 3);
   g = 36 * ((x & 0x1c0) >> 6);
  }

  if(CustomColorMap && CustomColorMapLen == 1024)
  {
   sc_r = CustomColorMap[(512 + x) * 3 + 0];
   sc_g = CustomColorMap[(512 + x) * 3 + 1];
   sc_b = CustomColorMap[(512 + x) * 3 + 2];
  }
  else
  {
   double y;

   y = round(0.300 * r + 0.589 * g + 0.111 * b);

   if(y < 0)
    y = 0;

   if(y > 255)
    y = 255;

   sc_r = sc_g = sc_b = y;
  }

  systemColorMap32[x] = (r << rshift) + (g << gshift) + (b << bshift);
  bw_systemColorMap32[x] = (sc_r << rshift) + (sc_g << gshift) + (sc_b << bshift);
 }

 // I know the temptation is there, but don't combine these two loops just
 // because they loop 512 times ;)
 for(int x = 0; x < 512; x++)
  FixPCache(x);
}

DECLFR(VCE_Read)
{
 switch(A & 0x7)
 {
  case 4: return(vce.color_table[vce.ctaddress & 0x1FF]);
  case 5: {
	   uint8 ret = vce.color_table[vce.ctaddress & 0x1FF] >> 8;
	   ret &= 1;
	   ret |= 0xFE;
	   if(!PCE_InDebug)
	    vce.ctaddress++;
	   return(ret);
	 }
 }
 return(0xFF);
}

DECLFW(VCE_Write)
{
 //printf("%04x %02x, %04x\n", A, V, HuCPU.PC);
 switch(A&0x7)
 {
  case 0: SetVCECR(V); break;
  case 2: vce.ctaddress &= 0x100; vce.ctaddress |= V; break;
  case 3: vce.ctaddress &= 0x0FF; vce.ctaddress |= (V & 1) << 8; break;
  case 4: vce.color_table[vce.ctaddress & 0x1FF] &= 0x100;
	  vce.color_table[vce.ctaddress & 0x1FF] |= V;
	  FixPCache(vce.ctaddress & 0x1FF);
          break;
  case 5: vce.color_table[vce.ctaddress & 0x1FF] &= 0xFF;
	  vce.color_table[vce.ctaddress & 0x1FF] |= (V & 1) << 8;
	  FixPCache(vce.ctaddress & 0x1FF);
	  vce.ctaddress++;
	  break;
 }
}


bool VDC_ToggleLayer(int which)
{
 userle ^= 1 << which;
 return((userle >> which) & 1);
}

#ifdef WANT_DEBUGGER 

void VDC_BP_Start(void)
{
 for(int chip = 0; chip < TotalChips; chip++)
 {
  vdc_chips[chip]->MAWR_BP = vdc_chips[chip]->MAWR;
  vdc_chips[chip]->MARR_BP = vdc_chips[chip]->MARR;
 }
}

DECLFR(VDC_Read_TestBP)
{
 int msb = A & 1;
 int chip = 0;
 vdc_t *vdc;

 if(sgfx)
 {
  A &= 0x1F;
  if(A & 0x8) return(0);
  chip = (A & 0x10) >> 4;
  vdc = vdc_chips[chip];
  A &= 0x3;
 }
 else
 {
  A &= 0x3;
  vdc = vdc_chips[0];
 }

 switch(A)
 {
  case 0x2:
  case 0x3:
           if(vdc->select == 0x2) // VRR - VRAM Read Register
            if(msb)
	    {
             vdc->MARR_BP += vram_inc_tab[(vdc->CR >> 11) & 0x3];
             PCEDBG_CheckBP(BPOINT_AUX_READ, vdc->MARR_BP | (chip ? 0x10000 : 0x00000), 1);
	    }
           break;
 }

 return(0);
}
#endif

DECLFR(VDC_Read)
{
 uint8 ret = 0;
 int msb = A & 1;
 int chip = 0;
 vdc_t *vdc;

 if(sgfx)
 {
  A &= 0x1F;
  switch(A)
  {
   case 0x8: return(vpc.priority[0]);
   case 0x9: return(vpc.priority[1]);
   case 0xA: return(vpc.winwidths[0]);
   case 0xB: return(vpc.winwidths[0] >> 8);
   case 0xC: return(vpc.winwidths[1]);
   case 0xD: return(vpc.winwidths[1] >> 8);
   case 0xE: return(0);
  }
  if(A & 0x8) return(0);
  chip = (A & 0x10) >> 4;
  vdc = vdc_chips[chip];
  A &= 0x3;
 }
 else 
 {
  vdc = vdc_chips[0];
  A &= 0x3;
 }

 switch(A)
 {
  case 0x0: ret = vdc->status;

	    if(!PCE_InDebug)
	    {
		    vdc->status &= ~0x3F;

	            if(sgfx)
	            {
	             if(!(vdc_chips[0]->status & 0x3F) && !(vdc_chips[1]->status & 0x3F))
		      HuC6280_IRQEnd(MDFN_IQIRQ1);
	            }
	            else
	              HuC6280_IRQEnd(MDFN_IQIRQ1); // Clear VDC IRQ line
	    }
	    break;
  case 0x2:
  case 0x3:
	   ret = REGGETP(vdc->read_buffer, msb);
	   if(vdc->select == 0x2) // VRR - VRAM Read Register
	   {
	    if(msb) 
	    {
	     if(!PCE_InDebug)
	     {
	      vdc->MARR += vram_inc_tab[(vdc->CR >> 11) & 0x3];

	      if(vdc->MARR >= VRAM_Size)
	       VDC_UNDEFINED("Unmapped VRAM VRR read");

	      vdc->read_buffer = vdc->VRAM[vdc->MARR & VRAM_SizeMask];
	     }
	    }
	   }
	   break;
 }

 //if(HuCPU.isopread && (A == 1 || A == 3)) //(A == 2 || A == 3)) // && A == 1)
 if(A == 1)
 {
  //if(vdc->display_counter >= (VDS + VSW) && vdc->display_counter < (VDS + VSW + VDW + 1) && vce.dot_clock > 0)
  if(vce.dot_clock > 0)
   ret = 0x40;
  //printf("%d %d %02x\n", vdc->display_counter, vce.dot_clock, ret);
  //ret = 0x40;
 }
 return(ret);
}

DECLFW(VDC_Write_ST)
{
 if(sgfx)
  A |= vpc.st_mode ? 0x10 : 0;

 VDC_Write(A, V);
}

#ifdef WANT_DEBUGGER 
DECLFW(VDC_Write_ST_TestBP)
{
 if(sgfx)
  A |= vpc.st_mode ? 0x10 : 0;

 VDC_Write_TestBP(A, V);
}
#endif

static void DoDMA(vdc_t *vdc)
{
    // Assuming one cycle for reads, one cycle for write, with DMA?
     for(int i = 0; i < 455; i++)
     {
      if(!vdc->DMAReadWrite)
      {
       if(vdc->SOUR >= VRAM_Size)
        VDC_UNDEFINED("Unmapped VRAM DMA read");

       vdc->DMAReadBuffer = vdc->VRAM[vdc->SOUR & VRAM_SizeMask];
      }
      else
      {
       if(vdc->DESR < VRAM_Size)
       {
        vdc->VRAM[vdc->DESR] = vdc->DMAReadBuffer;
        FixTileCache(vdc, vdc->DESR);
       }

       //if(vdc->DCR & 0xC) 
	//printf("Pllal: %02x\n", vdc->DCR);

       vdc->SOUR += (((vdc->DCR & 0x4) >> 1) ^ 2) - 1;
       vdc->DESR += (((vdc->DCR & 0x8) >> 2) ^ 2) - 1;
       vdc->LENR--;
       if(vdc->LENR == 0xFFFF)  // DMA is done.
       {
        vdc->DMARunning = 0;
        if(vdc->DCR & 0x02)
        {
         vdc->status |= VDCS_DV;
         HuC6280_IRQBegin(MDFN_IQIRQ1);
	 VDC_DEBUG("DMA IRQ");
        }
        break;
       }
      }
      vdc->DMAReadWrite ^= 1;
     } // for()
}

#ifdef WANT_DEBUGGER 
DECLFW(VDC_Write_TestBP)
{
 int msb = A & 1;
 int chip = 0;
 vdc_t *vdc;

 if(sgfx)
 {
  A &= 0x1F;
  if(A & 0x8) return;

  chip = (A & 0x10) >> 4;
  vdc = vdc_chips[chip];
  A &= 0x3;
 }
 else
 {
  A &= 0x3;
  vdc = vdc_chips[0];
 }

 switch(A)
 {
  case 0x2:
  case 0x3:
           switch(vdc->select & 0x1F)
           {
            case 0x01: if(msb)
                        PCEDBG_CheckBP(BPOINT_AUX_READ, (vdc->MARR_BP & 0x00FF) | (V << 8) | (chip ? 0x10000 : 0x00000), 1);
                       break;
            case 0x02: if(msb)
		       {
                        PCEDBG_CheckBP(BPOINT_AUX_WRITE, vdc->MAWR_BP | (chip ? 0x10000 : 0x00000), 1);
                        vdc->MAWR_BP += vram_inc_tab[(vdc->CR >> 11) & 0x3];
		       }
                       break;
           }
 }
}
#endif

DECLFW(VDC_Write)
{
 int msb = A & 1;
 int chip = 0;
 vdc_t *vdc;

 //printf("VDC Write: %04x %02x\n", A, V);
 if(sgfx)
 {
  A &= 0x1F;
  switch(A)
  {
   case 0x8: vpc.priority[0] = V; break;
   case 0x9: vpc.priority[1] = V; break;
   case 0xA: vpc.winwidths[0] &= 0x300; vpc.winwidths[0] |= V; break;
   case 0xB: vpc.winwidths[0] &= 0x0FF; vpc.winwidths[0] |= (V & 3) << 8; break;
   case 0xC: vpc.winwidths[1] &= 0x300; vpc.winwidths[1] |= V; break;
   case 0xD: vpc.winwidths[1] &= 0x0FF; vpc.winwidths[1] |= (V & 3) << 8; break;
   case 0xE: vpc.st_mode = V & 1; break;
  }
  if(A & 0x8) return;

  chip = (A & 0x10) >> 4;
  vdc = vdc_chips[chip];
  A &= 0x3;
 }
 else
 {
  vdc = vdc_chips[0];
  A &= 0x3;
 }
 //if((A == 0x2 || A == 0x3) && ((vdc->select & 0x1f) >= 0x09) && ((vdc->select & 0x1f) <= 0x13))

 //printf("%04x, %02x: %02x, %d\n", A, vdc->select, V, vdc->display_counter);

 switch(A)
 {
  case 0x0: vdc->select = V & 0x1F; break;
  case 0x2:
  case 0x3:
	   switch(vdc->select & 0x1F)
	   {
	    case 0x00: REGSETP(vdc->MAWR, V, msb); break;
	    case 0x01: REGSETP(vdc->MARR, V, msb);
		       if(msb)
		       {
			if(vdc->MARR >= VRAM_Size)
                         VDC_UNDEFINED("Unmapped VRAM VRR(MARR set) read");

			vdc->read_buffer = vdc->VRAM[vdc->MARR];
		       }
	               break;
	    case 0x02: if(!msb) vdc->write_latch = V;
		       else
		       {
			if(vdc->MAWR < VRAM_Size)
			{
			 // A hack to fix Crest of Wolf, and maybe others.
                         while(vdc->DMARunning)
                          DoDMA(vdc);

 			 vdc->VRAM[vdc->MAWR & VRAM_SizeMask] = (V << 8) | vdc->write_latch;
			 FixTileCache(vdc, vdc->MAWR & VRAM_SizeMask);
			} 
			else
			{
	                 //VDC_UNDEFINED("Unmapped VRAM write");
			 //printf("VROOM: %04x, %02x\n", vdc->MAWR, (vdc->CR >> 11) & 0x3);
			}
	                vdc->MAWR += vram_inc_tab[(vdc->CR >> 11) & 0x3];
		       }
		       break;
	    case 0x05: REGSETP(vdc->CR, V, msb); break;
	    case 0x06: REGSETP(vdc->RCR, V, msb); vdc->RCR &= 0x3FF; break;
	    case 0x07: REGSETP(vdc->BXR, V, msb); vdc->BXR &= 0x3FF; break;
	    case 0x08: REGSETP(vdc->BYR, V, msb); vdc->BYR &= 0x1FF;
		       vdc->BG_YOffset = vdc->BYR; // Set it on LSB and MSB writes(only changing on MSB breaks Youkai Douchuuki)
		       //printf("%04x\n", HuCPU.PC);
		       break;
	    case 0x09: REGSETP(vdc->MWR, V, msb); break;
	    case 0x0a: REGSETP(vdc->HSR, V, msb); break;
	    case 0x0b: REGSETP(vdc->HDR, V, msb); break;
	    case 0x0c: REGSETP(vdc->VSR, V, msb); break;
	    case 0x0d: REGSETP(vdc->VDR, V, msb); break;
	    case 0x0e: REGSETP(vdc->VCR, V, msb); break;
	    case 0x0f: REGSETP(vdc->DCR, V, msb); break;
	    case 0x10: REGSETP(vdc->SOUR, V, msb); break;
	    case 0x11: REGSETP(vdc->DESR, V, msb); break;
	    case 0x12: REGSETP(vdc->LENR, V, msb);
		       if(msb)
		       {
			vdc->DMARunning = 1;
		        vdc->DMAReadWrite = 0;
			if(vdc->burst_mode && !(vdc->DCR & 0x02))
			 DoDMA(vdc);	// Do one line's worth of DMA transfers
					// because Cosmic Fantasy 4 is evil
					// and uses timed writes to the DMA
					// start register, rather than waiting until
					// the machine says we're done,
					// which would require cycle-accurate VDC emulation...like that's
					// going to happen when I don't even have accurate values
					// for HuC6280 instruction timings. :b
		       }
		       break;
	    case 0x13: REGSETP(vdc->SATB, V, msb); vdc->SATBPending = 1; break;
//	    default: printf("Oops 2: %04x %02x\n", vdc->select, V);break;
	   }
	   break;
 }
}

static INLINE void CalcWidthStartEnd(vdc_t *vdc, uint32 &display_width, uint32 &start, uint32 &end)
{
 display_width = (M_vdc_HDW + 1) * 8;

 if(display_width > ClockModeWidths[vce.dot_clock])
  display_width = ClockModeWidths[vce.dot_clock];

 start = (ClockModeWidths[vce.dot_clock] - display_width) / 2;;

 // Semi-hack for Asuka 120%
 if(vce.dot_clock == 1 && M_vdc_HDS == 5 && M_vdc_HDE == 6 && M_vdc_HDW == 43 && M_vdc_HSW == 2)
  start += 8;
 else if(vce.dot_clock == 0 && M_vdc_HDS == 2 && M_vdc_HDE == 3 && M_vdc_HDW == 33 && M_vdc_HSW == 2)
  start += 4;
 // and for Addams Family
 else if(vce.dot_clock == 1 && M_vdc_HDS == 4 && M_vdc_HDE == 4 && M_vdc_HDW == 43 && M_vdc_HSW == 9)
  start += 4;

 //MDFN_DispMessage("dc: %d, %d %d %d %d; %d %d\n", vce.dot_clock, M_vdc_HDS, M_vdc_HDE, M_vdc_HDW, M_vdc_HSW, start, (M_vdc_HDS + 1) * 8);

 end = start + display_width;
 if(end > ClockModeWidths[vce.dot_clock])
  end = ClockModeWidths[vce.dot_clock];

}

static void DrawBG(vdc_t *vdc, uint32 *target, int enabled)
{
 uint32 width;
 uint32 start;
 uint32 end;
 int bat_width = bat_width_tab[(vdc->MWR >> 4) & 3];
 int bat_width_mask = bat_width - 1;
 int bat_width_shift = bat_width_shift_tab[(vdc->MWR >> 4) & 3];
 int bat_height_mask = bat_height_tab[(vdc->MWR >> 6) & 1] - 1;

 CalcWidthStartEnd(vdc, width, start, end);

 if(!enabled)
 {
  uint32 color = MK_COLOR(0x00, 0xFE, 0x00) | amask;

  for(int x = start; x < end; x++)
   target[x] = color;
  return;
 }

 if(!(vdc->CR & 0x80)) // BG is disabled
 {
  uint32 color;

  if(vdc->CR & 0x40)
   color = vce.color_table_cache[0x100];
  else
   color = vce.color_table_cache[0x000];

  MDFN_FastU32MemsetM8(target, color, 576);

  //for(int x = start; x < end; x++)
  // target[x] = color;
  return;
 }

 uint32 overscan_color = vce.color_table_cache[0x100];

 {
  int bat_y = ((vdc->BG_YOffset >> 3) & bat_height_mask) << bat_width_shift;
  int first_end = start + 8 - (vdc->BG_XOffset & 7);
  uint32 dohmask = 0xFFFFFFFF;

  if((vdc->MWR & 0x3) == 0x3)
  {
   if(vdc->MWR & 0x80)
    dohmask = 0xCCCCCCCC;
   else
    dohmask = 0x33333333;
  }

  // Clear the left overscan area
  for(int x = 0; x < start; x++)
   target[x] = overscan_color;

  // Draw the first pixels of the first tile, depending on the lower 3 bits of the xscroll/xoffset register, to
  // we can render the rest of the line in 8x1 chunks, which is faster.
  for(int x = start; x < first_end; x++)
  {
   int bat_x = (vdc->BG_XOffset >> 3) & bat_width_mask;

   uint16 bat = vdc->VRAM[bat_x | bat_y];
   int palette_index = ((bat >> 12) & 0x0F) << 4;
   uint32 raw_pixel;

   raw_pixel = vdc->bg_tile_cache[bat & VRAM_BGTileNoMask][vdc->BG_YOffset & 7][vdc->BG_XOffset & 0x7] & dohmask;
   target[x] = vce.color_table_cache[palette_index | raw_pixel];

   if((bat & 0xFFF) > VRAM_BGTileNoMask)
    VDC_UNDEFINED("Unmapped BG tile read");

   vdc->BG_XOffset++;
  }

  int bat_boom = (vdc->BG_XOffset >> 3) & bat_width_mask;
  int line_sub = vdc->BG_YOffset & 7;

  if((vdc->MWR & 0x3) == 0x3)
  {
   for(int x = first_end; x < end; x+=8)
   {
    uint16 bat = vdc->VRAM[bat_boom | bat_y];
    uint32 *lut = &vce.color_table_cache[((bat >> 8) & 0xF0)];
    uint8 *pix_lut = vdc->bg_tile_cache[bat & VRAM_BGTileNoMask][line_sub];

    if((bat & 0xFFF) > VRAM_BGTileNoMask)
     VDC_UNDEFINED("Unmapped BG tile read");


    (target + 0)[x] = lut[pix_lut[0] & dohmask];
    (target + 1)[x] = lut[pix_lut[1] & dohmask];
    (target + 2)[x] = lut[pix_lut[2] & dohmask];
    (target + 3)[x] = lut[pix_lut[3] & dohmask];
    (target + 4)[x] = lut[pix_lut[4] & dohmask];
    (target + 5)[x] = lut[pix_lut[5] & dohmask];
    (target + 6)[x] = lut[pix_lut[6] & dohmask];
    (target + 7)[x] = lut[pix_lut[7] & dohmask];

    bat_boom = (bat_boom + 1) & bat_width_mask;
    vdc->BG_XOffset++;

   }
  }
  else
  for(int x = first_end; x < end; x+=8) // This will draw past the right side of the buffer, but since our pitch is 1024, and max width is ~512, we're safe.  Also,
					// any overflow that is on the visible screen are will be hidden by the overscan color code below this code.
  {
   uint16 bat = vdc->VRAM[bat_boom | bat_y];
   uint32 *lut = &vce.color_table_cache[((bat >> 8) & 0xF0)];
   uint8 *pix_lut = vdc->bg_tile_cache[bat & VRAM_BGTileNoMask][line_sub];

   if((bat & 0xFFF) > VRAM_BGTileNoMask)
    VDC_UNDEFINED("Unmapped BG tile read");

   #ifdef LSB_FIRST
    #if SIZEOF_LONG == 8
    uint64 doh = *(uint64 *)pix_lut;

    (target + 0)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 1)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 2)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 3)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 4)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 5)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 6)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 7)[x] = lut[doh];
    #else
    uint32 doh = *(uint32 *)pix_lut;
    (target + 0)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 1)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 2)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 3)[x] = lut[doh];
    doh = *(uint32 *)(pix_lut + 4);
    (target + 4)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 5)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 6)[x] = lut[doh & 0xFF];
    doh >>= 8;
    (target + 7)[x] = lut[doh];
    #endif
   #else
   (target + 0)[x] = lut[pix_lut[0]];
   (target + 1)[x] = lut[pix_lut[1]];
   (target + 2)[x] = lut[pix_lut[2]];
   (target + 3)[x] = lut[pix_lut[3]];
   (target + 4)[x] = lut[pix_lut[4]];
   (target + 5)[x] = lut[pix_lut[5]];
   (target + 6)[x] = lut[pix_lut[6]];
   (target + 7)[x] = lut[pix_lut[7]];
   #endif
   bat_boom = (bat_boom + 1) & bat_width_mask;
   vdc->BG_XOffset++;
  }

  for(int x = end; x < ClockModeWidths[vce.dot_clock]; x++)
   target[x] = overscan_color;
 }
}

#define SPRF_PRIORITY	0x00080
#define SPRF_HFLIP	0x00800
#define SPRF_VFLIP	0x08000
#define SPRF_SPRITE0	0x10000

static const unsigned int sprite_height_tab[4] = { 16, 32, 64, 64 };
static const unsigned int sprite_height_no_mask[4] = { ~0, ~2, ~6, ~6 };
static const unsigned int sprite_width_tab[2] = { 16, 32 };

typedef struct
{
	uint32 x;
	uint32 flags;
	uint8 palette_index;
	uint16 pattern_data[4];
} SPRLE;

static void DrawSprites(vdc_t *vdc, uint32 *target, int enabled)
{
 int active_sprites = 0;
 SPRLE SpriteList[64 * 2]; // (see unlimited_sprites option, *2 to accomodate 32-pixel-width sprites ) //16];
 uint32 sprite_line_buf[1024] __attribute__ ((aligned (16)));

 // First, grab the up to 16 sprites.
 for(int i = 0; i < 64; i++)
 {
  int16 y = (vdc->SAT[i * 4 + 0] & 0x3FF) - 0x40;
  uint16 x = (vdc->SAT[i * 4 + 1] & 0x3FF);
  uint16 no = (vdc->SAT[i * 4 + 2] >> 1) & 0x3FF;	// Todo, cg mode bit
  uint16 flags = (vdc->SAT[i * 4 + 3]);

  uint32 palette_index = (flags & 0xF) << 4;
  uint32 height = sprite_height_tab[(flags >> 12) & 3];
  uint32 width = sprite_width_tab[(flags >> 8) & 1];

  if((int32)vdc->RCRCount >= y && (int32)vdc->RCRCount < (int32)(y + height))
  {
   bool second_half = 0;
   uint32 y_offset = vdc->RCRCount - y;
   if(y_offset > height) continue;

   breepbreep:

   if(active_sprites == 16)
   {
    if(vdc->CR & 0x2)
    {
     vdc->status |= VDCS_OR;
     HuC6280_IRQBegin(MDFN_IQIRQ1);
     VDC_DEBUG("Overflow IRQ");
    }
    if(!unlimited_sprites)
     break;
   }


   {
    if(flags & SPRF_VFLIP)
     y_offset = height - 1 - y_offset;

    no &= sprite_height_no_mask[(flags >> 12) & 3];
    no |= (y_offset & 0x30) >> 3;
    if(width == 32) no &= ~1;
    if(second_half)
     no |= 1;

    SpriteList[active_sprites].flags = flags;

    if(flags & SPRF_HFLIP && width == 32)
     no ^= 1;
    //printf("Found: %d %d\n", vdc->RCRCount, x);
    SpriteList[active_sprites].x = x;
    SpriteList[active_sprites].palette_index = palette_index;

    if((no * 64) >= VRAM_Size)
    {
     VDC_UNDEFINED("Unmapped VRAM sprite tile read");
     SpriteList[active_sprites].pattern_data[0] = 0;
     SpriteList[active_sprites].pattern_data[1] = 0;
     SpriteList[active_sprites].pattern_data[2] = 0;
     SpriteList[active_sprites].pattern_data[3] = 0;
    }
    else 
    {
     if((vdc->MWR & 0xC) == 4)
     {
      if(vdc->SAT[i * 4 + 2] & 1)
      {
       SpriteList[active_sprites].pattern_data[0] = vdc->VRAM[no * 64 + (y_offset & 15) + 32];
       SpriteList[active_sprites].pattern_data[1] = vdc->VRAM[no * 64 + (y_offset & 15) + 48];
       SpriteList[active_sprites].pattern_data[2] = 0; 
       SpriteList[active_sprites].pattern_data[3] = 0;
      }
      else
      {
       SpriteList[active_sprites].pattern_data[0] = vdc->VRAM[no * 64 + (y_offset & 15) ];
       SpriteList[active_sprites].pattern_data[1] = vdc->VRAM[no * 64 + (y_offset & 15) + 16];
       SpriteList[active_sprites].pattern_data[2] = 0;
       SpriteList[active_sprites].pattern_data[3] = 0;
      }
     }
     else
     {
      SpriteList[active_sprites].pattern_data[0] = vdc->VRAM[no * 64 + (y_offset & 15) ];
      SpriteList[active_sprites].pattern_data[1] = vdc->VRAM[no * 64 + (y_offset & 15) + 16];
      SpriteList[active_sprites].pattern_data[2] = vdc->VRAM[no * 64 + (y_offset & 15) + 32];
      SpriteList[active_sprites].pattern_data[3] = vdc->VRAM[no * 64 + (y_offset & 15) + 48];
     }
    }

    SpriteList[active_sprites].flags |= i ? 0 : SPRF_SPRITE0;

    active_sprites++;

    if(width == 32 && !second_half)
    {
     second_half = 1;
     x += 16;
     y_offset = vdc->RCRCount - y;	// Fix the y offset so that sprites that are hflipped + vflipped display properly
     goto breepbreep;
    }
   }
  }
 }

 uint32 display_width, start, end;

 CalcWidthStartEnd(vdc, display_width, start, end);

 MDFN_FastU32MemsetM8(sprite_line_buf, amask, (end + 1) & ~1);

 for(int i = (active_sprites - 1) ; i >= 0; i--)
 {
  int32 pos = SpriteList[i].x - 0x20 + start;
  uint32 prio_or = 0;

  if(SpriteList[i].flags & SPRF_PRIORITY) 
   prio_or = amask << 1;

  prio_or |= (amask << 2);	// For sprite #0 hit detection

  //printf("%u %u %u %u\n", SpriteList[i].x, start, max_x, display_width);

  if((SpriteList[i].flags & SPRF_SPRITE0) && (vdc->CR & 0x01))
  for(uint32 x = 0; x < 16; x++)
  {
   uint32 raw_pixel;
   uint32 pi = SpriteList[i].palette_index;
   uint32 rev_x = 15 - x;

   if(SpriteList[i].flags & SPRF_HFLIP)
    rev_x = x;

   raw_pixel = (SpriteList[i].pattern_data[0] >> rev_x)  & 1;
   raw_pixel |= ((SpriteList[i].pattern_data[1] >> rev_x) & 1) << 1;
   raw_pixel |= ((SpriteList[i].pattern_data[2] >> rev_x) & 1) << 2;
   raw_pixel |= ((SpriteList[i].pattern_data[3] >> rev_x) & 1) << 3;

   if(raw_pixel)
   {
    pi |= 0x100;
    uint32 tx = pos + x;

    if(tx >= end) // Covers negative and overflowing the right side.
     continue;

    if(sprite_line_buf[tx] & (amask << 2))
    {
     vdc->status |= VDCS_CR;
     VDC_DEBUG("Sprite hit IRQ");
     HuC6280_IRQBegin(MDFN_IQIRQ1);
    }
    sprite_line_buf[tx] = vce.color_table_cache[pi | raw_pixel] | prio_or;
   }
  }
  else
  for(uint32 x = 0; x < 16; x++)
  {
   uint32 raw_pixel;
   uint32 pi = SpriteList[i].palette_index;
   uint32 rev_x = 15 - x;

   if(SpriteList[i].flags & SPRF_HFLIP)
    rev_x = x;

   raw_pixel = (SpriteList[i].pattern_data[0] >> rev_x)  & 1;
   raw_pixel |= ((SpriteList[i].pattern_data[1] >> rev_x) & 1) << 1;
   raw_pixel |= ((SpriteList[i].pattern_data[2] >> rev_x) & 1) << 2;
   raw_pixel |= ((SpriteList[i].pattern_data[3] >> rev_x) & 1) << 3;

   if(raw_pixel)
   {
    pi |= 0x100;
    uint32 tx = pos + x;
    if(tx >= end) // Covers negative and overflowing the right side.
     continue;
    sprite_line_buf[tx] = vce.color_table_cache[pi | raw_pixel] | prio_or;
   }
  }
 }

 if(enabled)
  for(unsigned int x = start; x < end; x++)
  {
   if(!(sprite_line_buf[x] & amask))
   {
    if((target[x] & amask) || (sprite_line_buf[x] & (amask << 1)))
     target[x] = sprite_line_buf[x];
   }
  }
}

// Icky kludge for saving/loading states in the debugger(called from HuC6280_Run)
#define CHECKCPOINT() switch(ContinuePoint) { default: case 0: goto ContinuePoint0; case 1: goto ContinuePoint1; case 2: goto ContinuePoint2; case 3: goto ContinuePoint3; }

void VDC_RunFrame(uint32 *pXBuf, MDFN_Rect *LineWidths, int skip)
{
 vdc_t *vdc = vdc_chips[0];
 int max_dc = 0;

 CHECKCPOINT();
 ContinuePoint0:

 do
 {
  #ifdef WANT_DEBUGGER
  if(GfxDecode_Line >=0 && frame_counter == GfxDecode_Line)
   DoGfxDecode();
  #endif

  vdc = vdc_chips[0];

  if(frame_counter == 0)
  {
   VDS = M_vdc_VDS;
   VSW = M_vdc_VSW;
   VDW = M_vdc_VDW;
   VCR = M_vdc_VCR;
   VBlankFL = VDS + VSW + VDW + 1;

   if(VBlankFL > 261)
    VBlankFL = 261;
  }

  uint32 line_buffer[2][1024];	// For super grafx emulation

  need_vbi[0] = need_vbi[1] = 0;
  line_leadin1 = 0;

  magical = M_vdc_HDS + (M_vdc_HDW + 1) + M_vdc_HDE;
  magical = (magical + 2) & ~1;
  magical -= M_vdc_HDW + 1;
  cyc_tot = magical * 8; //ClockPixelWidths[vce.dot_clock] - magical * 8;
  cyc_tot-=2;
  switch(vce.dot_clock)
  {
   case 0: cyc_tot = 4 * cyc_tot / 3; break;
   case 1: break;
   case 2: cyc_tot = 2 * cyc_tot / 3; break;
  }

  if(cyc_tot < 0) cyc_tot = 0;
  line_leadin1 = cyc_tot;

  if(!skip)
   EmulatedPCE.DisplayRect.w = ClockModeWidths[vce.dot_clock];

  if(max_dc < vce.dot_clock)
   max_dc = vce.dot_clock;

  if(!skip)
  {
   int os_hide;
   int po;

   switch(vce.dot_clock)
   {
    default:
    case 0: os_hide = 16; po = 16; break;
    case 1: os_hide = 21; po = 21; break;
    case 2: os_hide = 32; po = 32; break;
   }
   EmulatedPCE.DisplayRect.x = po;
   EmulatedPCE.DisplayRect.w -= os_hide * 2;
  }

  for(int chip = 0; chip < TotalChips; chip++)
  {
   vdc = vdc_chips[chip];
   if(frame_counter == 0)
   {
    vdc->display_counter = 0;
    vdc->burst_mode = !(vdc->CR & 0xC0);
   }

   if(vdc->display_counter == (VDS + VSW))
   {
    vdc->burst_mode = !(vdc->CR & 0xC0);
    vdc->RCRCount = 0;
   }
   int have_free_time = 1;

   if(!vdc->burst_mode && vdc->display_counter >= (VDS + VSW) && vdc->display_counter < (VDS + VSW + VDW + 1))
    have_free_time = 0;

   if(have_free_time) // We're outside of the active display area.  Weehee
   {
    if(vdc->DMARunning)
     DoDMA(vdc);
   }

   if(vdc->display_counter == VBlankFL)
   {
    need_vbi[chip] = 1;
    if(vdc->SATBPending || (vdc->DCR & 0x10))
    {
     vdc->SATBPending = 0;
     vdc->sat_dma_slcounter = 2;

     if(vdc->SATB > (VRAM_Size - 0x100))
      VDC_UNDEFINED("Unmapped VRAM SATB DMA read");

     if(vdc->SATB < VRAM_Size)
     {
      uint32 len = 256;
      if(vdc->SATB > (VRAM_Size - 0x100)) //0x7F00)
       len = VRAM_Size - vdc->SATB;
      memcpy(vdc->SAT, &vdc->VRAM[vdc->SATB], len * sizeof(uint16));
     }
    }
   }
   if((int)vdc->RCRCount == ((int)vdc->RCR - 0x40) && (vdc->CR & 0x04))
   {
    VDC_DEBUG("RCR IRQ");
    vdc->status |= VDCS_RR;
    HuC6280_IRQBegin(MDFN_IQIRQ1); 
   }
  }

  ContinuePoint = 1;
  HuC6280_Run(line_leadin1);
  CHECKCPOINT();
  ContinuePoint1:

  for(int chip = 0; chip < TotalChips; chip++)
  {
   uint32 *target_ptr;
   vdc = vdc_chips[chip];

   if(sgfx)
    target_ptr = line_buffer[chip];
   else
    target_ptr = pXBuf + (frame_counter - 14) * 1024;

   if(frame_counter >= 14 && frame_counter < (14 + 242) && !skip)
    LineWidths[frame_counter - 14] = EmulatedPCE.DisplayRect;

   if(vdc->burst_mode)
   {
    if(frame_counter >= 14 && frame_counter < (14 + 242))
    {
     uint32 color;

     if(!skip)
     {
      if(vce.bw)
       color = bw_systemColorMap32[vce.color_table[0x100]] | amask;
      else
       color = systemColorMap32[vce.color_table[0x100]] | amask;
      MDFN_FastU32MemsetM8(target_ptr, color, 576); //512);
     }
    }
   }
   else if(vdc->display_counter >= (VDS + VSW) && vdc->display_counter < (VDS + VSW + VDW + 1))
   {
    if(vdc->display_counter == (VDS + VSW))
     vdc->BG_YOffset = vdc->BYR;
    else
     vdc->BG_YOffset++;
    vdc->BG_XOffset = vdc->BXR;
    if(frame_counter >= 14 && frame_counter < (14 + 242))
    {
     if(!skip)
      DrawBG(vdc, target_ptr, userle & (chip ? ULE_BG1 : ULE_BG0));
     if(vdc->CR & 0x40)
      DrawSprites(vdc, target_ptr, skip ? 0 : (userle & (chip ? ULE_SPR1 : ULE_SPR0)));
    }
    // Draw screen, joy.
   }
   else // Hmm, overscan...
   {
    if(frame_counter >= 14 && frame_counter < (14 + 242))
    {
     uint32 color;
     if(vce.bw)
      color = bw_systemColorMap32[vce.color_table[0x100]] | amask;
     else
      color = systemColorMap32[vce.color_table[0x100]] | amask;
     MDFN_FastU32MemsetM8(target_ptr, color, 576); //512);
    }
   }
  }

  for(int chip = 0; chip < TotalChips; chip++)
   if((vdc_chips[chip]->CR & 0x08) && need_vbi[chip])
    vdc_chips[chip]->status |= VDCS_VD;

  ContinuePoint = 2;
  HuC6280_Run(2);
  CHECKCPOINT();
  ContinuePoint2:

  for(int chip = 0; chip < TotalChips; chip++)
   if(vdc_chips[chip]->status & VDCS_VD)
   {
    VDC_DEBUG("VBlank IRQ");
    HuC6280_IRQBegin(MDFN_IQIRQ1);   
   }

  ContinuePoint = 3;
  HuC6280_Run(455 - line_leadin1 - 2);
  CHECKCPOINT();
  ContinuePoint3:

  for(int chip = 0; chip < TotalChips; chip++)
  {
   vdc = vdc_chips[chip];
   vdc->RCRCount++;

   //vdc->BG_YOffset = (vdc->BG_YOffset + 1);
   vdc->display_counter++;

   if(vdc->sat_dma_slcounter)
   {
    vdc->sat_dma_slcounter--;
    if(!vdc->sat_dma_slcounter)
    {
     if(vdc->DCR & 0x01)
     {
      VDC_DEBUG("Sprite DMA IRQ");
      vdc->status |= VDCS_DS;
      HuC6280_IRQBegin(MDFN_IQIRQ1);
     }
    }
   }

   if(vdc->display_counter == (VDS + VSW + VDW + VCR + 3))
   {
    vdc->display_counter = 0;
   }
  }

  if(sgfx && !skip)
  if(frame_counter >= 14 && frame_counter < (14 + 242))
  {
   int start = EmulatedPCE.DisplayRect.x;
   if(vce.dot_clock == 1 && M_vdc_HDS == 5 && M_vdc_HDE == 6 && M_vdc_HDW == 43 && M_vdc_HSW == 2)
    start += 8;
   else if(vce.dot_clock == 0 && M_vdc_HDS == 2 && M_vdc_HDE == 3 && M_vdc_HDW == 33 && M_vdc_HSW == 2)
    start += 4;

   int end = start + EmulatedPCE.DisplayRect.w;

   uint32 *main_target = pXBuf + (frame_counter - 14) * 1024;

   static const int prio_select[4] = { 1, 1, 0, 0 };
   static const int prio_shift[4] = { 4, 0, 4, 0 };

   for(int x = start; x < end; x++)
   {
    int in_window = 0;
    if(x >= start && x < (start + vpc.winwidths[0] - 0x40))
     in_window |= 1;
    if(x >= start && x < (start + vpc.winwidths[1] - 0x40))
     in_window |= 2;

    uint8 pb = (vpc.priority[prio_select[in_window]] >> prio_shift[in_window]) & 0xF;

    uint32 vdc2_pixel, vdc1_pixel;

    vdc2_pixel = vdc1_pixel = vce.color_table_cache[0];

    if(pb & 1)
     vdc1_pixel = line_buffer[0][x];
    if(pb & 2)
     vdc2_pixel = line_buffer[1][x];

/* Dai MakaiMura uses setting 1, and expects VDC #2 sprites in front of VDC #1 background, but
   behind VDC #1's sprites.
 */
    switch(pb >> 2)
    {
     case 1:
		if((vdc2_pixel & (amask << 2)) && !(vdc1_pixel & (amask << 2)))
			vdc1_pixel |= amask;				
		break;
     case 2:
		if((vdc1_pixel & (amask << 2)) && !(vdc2_pixel & (amask << 2)) && !(vdc2_pixel & amask))
			vdc1_pixel |= amask;
		break;
    }
    main_target[x] = (vdc1_pixel & amask) ? vdc2_pixel : vdc1_pixel;
   }
  }
  frame_counter = (frame_counter + 1) % (vce.lc263 ? 263 : 262);
  //printf("%d\n", vce.lc263);
 } while(frame_counter != VBlankFL); // big frame loop!

  //printf("%d\n", max_dc);

  if(!skip)
  {
   EmulatedPCE.DisplayRect.w = ClockModeWidths[max_dc];
   int os_hide = 0;

   switch(max_dc)
   {
    default:
    case 0: os_hide = 16; break;
    case 1: os_hide = 21; break;
    case 2: os_hide = 32; break;
   }

   EmulatedPCE.DisplayRect.x = os_hide;
   EmulatedPCE.DisplayRect.w -= os_hide * 2;
  }
 ContinuePoint = 0;
}

void VDC_Reset(void)
{
 ContinuePoint = 0;

 vdc_chips[0]->read_buffer = 0xFFFF;

 vpc.priority[0] = vpc.priority[1] = 0x11;

 vdc_chips[0]->HSR = vdc_chips[0]->HDR = vdc_chips[0]->VSR = vdc_chips[0]->VDR = vdc_chips[0]->VCR = 0xFF; // Needed for Body Conquest 2

 if(vdc_chips[1])
 {
  vdc_chips[1]->read_buffer = 0xFFFF;
  vdc_chips[1]->HSR = vdc_chips[1]->HDR = vdc_chips[1]->VSR = vdc_chips[1]->VDR = vdc_chips[1]->VCR = 0xFF; // and for HES playback to not go bonkers
 }
 frame_counter = 0;


 vce.CR = 0;
 vce.lc263 = 0;
 vce.bw = 0;
 vce.dot_clock = 0;
 vce.ctaddress = 0;

 memset(vce.color_table, 0, sizeof(vce.color_table));

 for(int x = 0; x < 512; x++)
  FixPCache(x);

}

void VDC_Power(void)
{
 for(int chip = 0; chip < TotalChips; chip++)
  memset(vdc_chips[chip], 0, sizeof(vdc_t));
 VDC_Reset();
}

#include <errno.h>
// Warning:  As of 0.8.x, this custom colormap function will only work if it's called from VDC_Init(), in the Load()
// game load path.
static bool LoadCustomColormap(const char *path)
{
 if(path[0] == 0 || !strcmp(path, "0") || !strcmp(path, "none"))
  return(TRUE);

 MDFN_printf(_("Loading custom color map from \"%s\"...\n"),  path);
 MDFN_indent(1);
 gzFile gp = gzopen(path, "rb");
 if(!gp)
 {
  MDFN_printf(_("Error opening file: %m\n"), errno);        // FIXME, zlib and errno...
  MDFN_indent(-1);
  return(FALSE);
 }
 else if((CustomColorMap = (uint8*)MDFN_malloc(1024 * 3, _("custom color map"))))
 {
  long length_read;

  length_read = gzread(gp, CustomColorMap, 1024 * 3);

  if(length_read == 512 * 3)
   MDFN_printf("Palette only has 512 entries.  Calculating the strip-colorburst entries.\n");

  if(length_read != 1024 * 3 && length_read != 512 * 3)
  {
   MDFN_printf(_("Error reading file\n"));
   MDFN_indent(-1);
   MDFN_free(CustomColorMap);
   CustomColorMap = NULL;
   return(FALSE);
  }

  CustomColorMapLen = length_read / 3;
 }
 MDFN_indent(-1);

 return(TRUE);
}

void VDC_Init(int sgx)
{
 unlimited_sprites = MDFN_GetSettingB("pce.nospritelimit");
 userle = ~0;
 sgfx = sgx;
 TotalChips = sgx ? 2 : 1;

 for(int chip = 0; chip < TotalChips; chip++)
 {
  vdc_chips[chip] = (vdc_t *)MDFN_malloc(sizeof(vdc_t), "VDC");
 }

 LoadCustomColormap(MDFN_GetSettingS("pce.colormap").c_str());

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddASpace(VDC_GetAddressSpaceBytes, VDC_PutAddressSpaceBytes, "vce", "VCE Palette RAM", 10);

 if(sgx)
 {
  MDFNDBG_AddASpace(VDC_GetAddressSpaceBytes, VDC_PutAddressSpaceBytes, "vdcvram0", "VDC-A VRAM", 16);
  MDFNDBG_AddASpace(VDC_GetAddressSpaceBytes, VDC_PutAddressSpaceBytes, "vdcvram1", "VDC-B VRAM", 16);
 }
 else
  MDFNDBG_AddASpace(VDC_GetAddressSpaceBytes, VDC_PutAddressSpaceBytes, "vdcvram0", "VDC VRAM", 16);
 #endif
}

void VDC_Close(void)
{
 for(int chip = 0; chip < TotalChips; chip++)
 {
  if(vdc_chips[chip])
   MDFN_free(vdc_chips[chip]);
  vdc_chips[chip] = NULL;
 }
 TotalChips = 0;
}

int VDC_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT VCE_StateRegs[] =
 {
  SFVARN(vce.CR, "VCECR"),
  SFVARN(vce.lc263, "lc263"),
  SFVARN(vce.bw, "bw"),
  SFVARN(vce.dot_clock, "dot clock"),
  SFVARN(vce.ctaddress, "ctaddress"),
  SFARRAY16N(vce.color_table, 0x200, "color_table"),
  SFEND
 };


 int ret = MDFNSS_StateAction(sm, load, data_only, VCE_StateRegs, "VCE");

 int max_chips = 1;
 if(sgfx) max_chips = 2;

 if(sgfx)
 {
  SFORMAT VPC_StateRegs[] =
  {
   SFVARN(vpc.st_mode, "st_mode"),
   SFARRAYN(vpc.priority, 2, "priority"),
   SFARRAY16N(vpc.winwidths, 2, "winwidths"),
   SFEND
  };
  ret &= MDFNSS_StateAction(sm, load, data_only, VPC_StateRegs, "VPC");
 }

 for(int chip = 0; chip < max_chips; chip++)
 {
  vdc_t *vdc = vdc_chips[chip];
  SFORMAT VDC_StateRegs[] = 
  {
	SFVARN(vdc->display_counter, "display_counter"),
        SFVARN(vdc->sat_dma_slcounter, "sat_dma_slcounter"),

	SFVARN(vdc->sgx_priority, "sgx_priority"),
        SFVARN(vdc->select, "select"),
        SFVARN(vdc->MAWR, "MAWR"),
        SFVARN(vdc->MARR, "MARR"),
        SFVARN(vdc->CR, "CR"),
        SFVARN(vdc->RCR, "RCR"),
        SFVARN(vdc->BXR, "BXR"),
        SFVARN(vdc->BYR, "BYR"),
        SFVARN(vdc->MWR, "MWR"),

        SFVARN(vdc->HSR, "HSR"),
        SFVARN(vdc->HDR, "HDR"),
        SFVARN(vdc->VSR, "VSR"),
        SFVARN(vdc->VDR, "VDR"),

        SFVARN(vdc->VCR, "VCR"),
        SFVARN(vdc->DCR, "DCR"),
        SFVARN(vdc->SOUR, "SOUR"),
        SFVARN(vdc->DESR, "DESR"),
        SFVARN(vdc->LENR, "LENR"),
        SFVARN(vdc->SATB, "SATB"),

        SFVARN(vdc->RCRCount, "RCRCount"),


        SFVARN(vdc->read_buffer, "read_buffer"),
        SFVARN(vdc->write_latch, "write_latch"),
        SFVARN(vdc->status, "status"),
        SFARRAY16N(vdc->SAT, 0x100, "SAT"),

        SFARRAY16N(vdc->VRAM, VRAM_Size, "VRAM"),
        SFVARN(vdc->DMAReadBuffer, "DMAReadBuffer"),
        SFVARN(vdc->DMAReadWrite, "DMAReadWrite"),
        SFVARN(vdc->DMARunning, "DMARunning"),
        SFVARN(vdc->SATBPending, "SATBPending"),
        SFVARN(vdc->burst_mode, "burst_mode"),

        SFVARN(vdc->BG_YOffset, "BG_YOffset"),
        SFVARN(vdc->BG_XOffset, "BG_XOffset"),
	SFVAR(frame_counter),
	SFVARN(VDS, "VDS_cache"),
        SFVARN(VSW, "VSW_cache"),
        SFVARN(VDW, "VDW_cache"),
        SFVARN(VCR, "VCR_cache"),
        SFVARN(VBlankFL, "VBlankFL_cache"),

	SFVAR(ContinuePoint),
	SFARRAY32(need_vbi, 2),
	SFVAR(line_leadin1),
	SFVAR(magical),
	SFVAR(cyc_tot),
	SFEND
  };

  if(load)
   ContinuePoint = 0; // For older save states

  ret &= MDFNSS_StateAction(sm, load, data_only, VDC_StateRegs, chip ? "VDC1" : "VDC0");
  if(load)
  {
   for(int x = 0; x < VRAM_Size; x++)
    FixTileCache(vdc, x);
   for(int x = 0; x < 512; x++)
    FixPCache(x);
  }

 }

 return(ret);
}

#ifdef WANT_DEBUGGER
static void DoGfxDecode(void)
{
 uint32 *palette_ptr;
 uint32 *target = GfxDecode_Buf;
 int w = GfxDecode_Width;
 int h = GfxDecode_Height;
 int scroll = GfxDecode_Scroll;
 bool DecodeSprites = GfxDecode_Layer & 1;
 vdc_t *vdc = vdc_chips[(GfxDecode_Layer & 2) ? 1 : 0];

 uint32 neo_palette[16];

 if(GfxDecode_Pbn == -1)
 {
  for(int x = 0; x <  16; x++)
   neo_palette[x] = MK_COLORA(x * 17, x * 17, x * 17, 0xFF);
 }
 else
  for(int x = 0; x < 16; x++)
   neo_palette[x] = vce.color_table_cache[x | (DecodeSprites ? 0x100 : 0x000) | ((GfxDecode_Pbn & 0xF) << 4)] | MK_COLORA(0, 0, 0, 0xFF);

 palette_ptr = neo_palette;

 if(DecodeSprites)
 {
  for(int y = 0; y < h; y++)
  {
   for(int x = 0; x < w; x += 16)
   {
    int which_tile = (x / 16) + (scroll + (y / 16)) * (w / 16);

    if(which_tile >= 0x200)
    {
     for(int sx = 0; sx < 16; sx++)
     {
      target[x + sx] = MK_COLORA(0, 0, 0, 0);
      target[x + w * 1 + sx] = 0;
      target[x + w * 2 + sx] = 0;
     }
     continue;
    }

    uint16 cg[4];
    cg[0] = vdc->VRAM[which_tile * 64 + (y & 15)];
    cg[1] = vdc->VRAM[which_tile * 64 + (y & 15) + 16];
    cg[2] = vdc->VRAM[which_tile * 64 + (y & 15) + 32];
    cg[3] = vdc->VRAM[which_tile * 64 + (y & 15) + 48];
    for(int sx = 0; sx < 16; sx++)
    {
     int rev_sx = 15 - sx;
     target[x + sx] = palette_ptr[(((cg[0] >> rev_sx) & 1) << 0) |
                (((cg[1] >> rev_sx) & 1) << 1) | (((cg[2] >> rev_sx) & 1) << 2) | (((cg[3] >> rev_sx) & 1) << 3)];
     target[x + w * 1 + sx] = which_tile;
     target[x + w * 2 + sx] = which_tile * 64;
    }
   }
   target += w * 3;
  }
 }
 else for(int y = 0; y < h; y++)
 {
  for(int x = 0; x < w; x+=8)
  {
   int which_tile = (x / 8) + (scroll + (y / 8)) * (w / 8);

   if(which_tile >= 0x800)
   {
    for(int sx = 0; sx < 8; sx++)
    {
     target[x + sx] = MK_COLORA(0, 0, 0, 0);
     target[x + w * 1 + sx] = 0;
     target[x + w * 2 + sx] = 0;
    }
    continue;
   }

   target[x + 0] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][0]];
   target[x + 1] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][1]];
   target[x + 2] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][2]];
   target[x + 3] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][3]];
   target[x + 4] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][4]];
   target[x + 5] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][5]];
   target[x + 6] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][6]];
   target[x + 7] = palette_ptr[ vdc->bg_tile_cache[which_tile][y & 0x7][7]];

   target[x + w*1 + 0]=target[x + w*1 + 1]=target[x + w*1 + 2]=target[x + w*1 + 3] =
   target[x + w*1 + 4]=target[x + w*1 + 5]=target[x + w*1 + 6]=target[x + w*1 + 7] = which_tile;

   target[x + w*2 + 0]=target[x + w*2 + 1]=target[x + w*2 + 2]=target[x + w*2 + 3] =
   target[x + w*2 + 4]=target[x + w*2 + 5]=target[x + w*2 + 6]=target[x + w*2 + 7] = which_tile * 16;
  }
  target += w * 3;
 }
}


void VDC_SetGraphicsDecode(int line, int which, int w, int h, int xscroll, int yscroll, int pbn)
{
 if(line == -1)
 {
  if(GfxDecode_Buf)
  {
   free(GfxDecode_Buf);
   GfxDecode_Buf = NULL;
  }
 }
 else
  GfxDecode_Buf = (uint32*)realloc(GfxDecode_Buf, w * h * sizeof(uint32) * 3); // *2 for extra address info.

 GfxDecode_Line = line;
 GfxDecode_Width = w;
 GfxDecode_Height = h;
 GfxDecode_Layer = which;
 GfxDecode_Scroll = yscroll;
 GfxDecode_Pbn = pbn;

 if(GfxDecode_Line == 0xB00B13)
  DoGfxDecode();
}

uint32 *VDC_GetGraphicsDecodeBuffer(void)
{
 return(GfxDecode_Buf);
}
#endif
