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

#include <math.h>
#include "pcfx.h"
#include "../video.h"
#include "vdc.h"
#include "interrupt.h"
#include "debug.h"

#define ALPHA_MASK	0x200

static bool unlimited_sprites;

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

static ALWAYS_INLINE void FixTileCache(fx_vdc_t *vdc, uint16 A)
{
 uint32 charname = (A >> 4);
 uint32 y = (A & 0x7);
 uint8 *tc = vdc->bg_tile_cache[charname][y];

 uint32 bitplane01 = vdc->VRAM[y + charname * 16];
 uint32 bitplane23 = vdc->VRAM[y+ 8 + charname * 16];

 for(int x = 0; x < 8; x++)
 {
  uint32 raw_pixel = ((bitplane01 >> x) & 1);
  raw_pixel |= ((bitplane01 >> (x + 8)) & 1) << 1;
  raw_pixel |= ((bitplane23 >> x) & 1) << 2;
  raw_pixel |= ((bitplane23 >> (x + 8)) & 1) << 3;
  tc[7 - x] = raw_pixel;
 }
/*
  uint32 bitplane01 = vdc->VRAM[tiny_YOffset + charname * 16];
  uint32 bitplane23 = vdc->VRAM[tiny_YOffset + 8 + charname * 16];
  uint32 raw_pixel;
  unsigned int tiny_XOffset = 7 - (vdc->BG_XOffset & 7);

  raw_pixel = ((bitplane01 >> tiny_XOffset) & 1);
  raw_pixel |= ((bitplane01 >> (tiny_XOffset + 8)) & 1) << 1;
  raw_pixel |= ((bitplane23 >> tiny_XOffset) & 1) << 2;
  raw_pixel |= ((bitplane23 >> (tiny_XOffset + 8)) & 1) << 3;
 */
}

#define REGSETP(_reg, _data, _msb) { _reg &= 0xFF << (_msb ? 0 : 8); _reg |= _data << (_msb ? 8 : 0); }
#define REGGETP(_reg, _msb) ((_reg >> (_msb ? 8 : 0)) & 0xFF)

uint16 FXVDC_Read16(fx_vdc_t *vdc, uint32 A)
{
 uint16 ret = 0;

 if(!(A & 2))
 {
  ret = vdc->status;
  vdc->status &= ~0x3F;
  PCFXIRQ_Assert(vdc->ilevel, 0);
 }
 else
 {
  ret = vdc->read_buffer;

  if(vdc->select == 0x2) // VRR - VRAM Read Register
  {
   #ifdef WANT_DEBUGGER
   if(vdc->Aux0BPBase != ~0)
    PCFXDBG_CheckBP(BPOINT_AUX_READ, vdc->Aux0BPBase + vdc->MARR, 1);
   #endif
   vdc->MARR += vram_inc_tab[(vdc->CR >> 11) & 0x3];
   vdc->read_buffer = vdc->VRAM[vdc->MARR];
  }
 }
 return(ret);
}

static void DoDMA(fx_vdc_t *vdc)
{
    // Assuming one cycle for reads, one cycle for write, with DMA?
     for(int i = 0; i < 455; i++)
     {
      if(!vdc->DMAReadWrite)
       vdc->DMAReadBuffer = vdc->VRAM[vdc->SOUR];
      else
      {
       vdc->VRAM[vdc->DESR] = vdc->DMAReadBuffer;
       FixTileCache(vdc, vdc->DESR);

       vdc->SOUR += (((vdc->DCR & 0x4) >> 1) ^ 2) - 1;
       vdc->DESR += (((vdc->DCR & 0x8) >> 2) ^ 2) - 1;
       vdc->LENR--;
       if(vdc->LENR == 0xFFFF)  // DMA is done.
       {
        vdc->DMARunning = 0;
        if(vdc->DCR & 0x02)
        {
         vdc->status |= VDCS_DV;
	 PCFXIRQ_Assert(vdc->ilevel, 1);
	 //puts("DMA IRQ");
        }
        break;
       }
      }
      vdc->DMAReadWrite ^= 1;
     } // for()
}
uint32 v810_getPC(void);

void FXVDC_Write16(fx_vdc_t *vdc, uint32 A, uint16 V)
{
 A &= 0x3;

 if(!(A & 2))
  vdc->select = V & 0x1F;
 else
 {
           switch(vdc->select & 0x1F)
	   {
	    case 0x00: vdc->MAWR = V; break;

	    case 0x01: vdc->MARR = V;
		       #ifdef WANT_DEBUGGER 
	               if(vdc->Aux0BPBase != ~0)
          	        PCFXDBG_CheckBP(BPOINT_AUX_READ, vdc->Aux0BPBase + vdc->MARR, 1);
		       #endif
		       vdc->read_buffer = vdc->VRAM[vdc->MARR];
	               break;

	    case 0x02: 
	               #ifdef WANT_DEBUGGER 
             	       if(vdc->Aux0BPBase != ~0)
		       {
		        PCFXDBG_CheckBP(BPOINT_AUX_WRITE, vdc->Aux0BPBase + vdc->MAWR, 1);
		       }
		       #endif
		       vdc->VRAM[vdc->MAWR] = V;
		       FixTileCache(vdc, vdc->MAWR);
	               vdc->MAWR += vram_inc_tab[(vdc->CR >> 11) & 0x3];
		       break;

	    case 0x05: vdc->CR = V; break;
	    case 0x06: vdc->RCR = V & 0x3FF; break;
	    case 0x07: vdc->BXR = V; break;
	    case 0x08: vdc->BYR = V; vdc->BG_YOffset = vdc->BYR; break;
	    case 0x09: vdc->MWR = V; break;
	    case 0x0a: vdc->HSR = V; break;
	    case 0x0b: vdc->HDR = V; break;
	    case 0x0c: vdc->VSR = V; break;
	    case 0x0d: vdc->VDR = V; break;
	    case 0x0e: vdc->VCR = V; break;
	    case 0x0f: vdc->DCR = V; break;
	    case 0x10: vdc->SOUR = V; break;
	    case 0x11: vdc->DESR = V; break;
	    case 0x12: vdc->LENR = V; 
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
		       break;
	    case 0x13: vdc->SATB = V; vdc->SATBPending = 1; break;
	    default: break;
	   }
 }
}

static const unsigned int bat_width_tab[4] = { 32, 64, 128, 128 };
static const unsigned int bat_width_shift_tab[4] = { 5, 6, 7, 7 };
static const unsigned int bat_height_tab[2] = { 32, 64 };
static const unsigned int ClockModeWidths[2] = { 288, 384};

static const uint32 trans_lut[256] =
{
 ALPHA_MASK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


static void DrawBG(fx_vdc_t *vdc, uint32 *target)
{
 unsigned int width = (M_vdc_HDW + 1) * 8;

 if(width > ClockModeWidths[vdc->dot_clock])
  width = ClockModeWidths[vdc->dot_clock];

 int start = ((M_vdc_HDS) * 8) + 8;
 int end = start + width;

 if(start > 512) start = 512;
 if(end > 512) end = 512;

 int bat_width = bat_width_tab[(vdc->MWR >> 4) & 3];
 int bat_width_mask = bat_width - 1;
 int bat_width_shift = bat_width_shift_tab[(vdc->MWR >> 4) & 3];
 int bat_height_mask = bat_height_tab[(vdc->MWR >> 6) & 1] - 1;

 if(!(vdc->CR & 0x80)) // BG is disabled
 {
  uint32 color = (vdc->CR & 0x40) ? 0x100 : 0x000;
  MDFN_FastU32MemsetM8(target, color | ALPHA_MASK, 512);
  return;
 }

 uint32 overscan_color = 0x100 | ALPHA_MASK;

 //if(enabled)
 {
  int bat_y = ((vdc->BG_YOffset >> 3) & bat_height_mask) << bat_width_shift;
  int first_end = start + 8 - (vdc->BG_XOffset & 7);

  // Clear the left overscan area
  MDFN_FastU32MemsetM8(target, overscan_color, 50); //(start + 1) &~1);

  for(int x = start; x < first_end; x++)
  {
   int bat_x = (vdc->BG_XOffset >> 3) & bat_width_mask;

   uint16 bat = vdc->VRAM[bat_x | bat_y];
   int palette_index = ((bat >> 12) & 0x0F) << 4;
   uint32 raw_pixel;

   raw_pixel = vdc->bg_tile_cache[bat & 0xFFF][vdc->BG_YOffset & 7][vdc->BG_XOffset & 0x7];
   target[x] = palette_index | raw_pixel | (raw_pixel ? 0 : ALPHA_MASK);

   vdc->BG_XOffset++;
  }
  int bat_boom = (vdc->BG_XOffset >> 3) & bat_width_mask;
  int line_sub = vdc->BG_YOffset & 7;
  for(int x = first_end; x < end; x+=8) // This will draw past the right side of the buffer, but since our pitch is 1024, and max width is ~512, we're safe.  Also,
					// any overflow that is on the visible screen are will be hidden by the overscan color code below this code.
  {
   uint16 bat = vdc->VRAM[bat_boom | bat_y];

   uint32 pix_or = (bat >> 8) & 0xF0;
   uint8 *pix_lut = vdc->bg_tile_cache[bat & 0xFFF][line_sub];

   (target + 0)[x] = pix_lut[0] | trans_lut[pix_lut[0]] | pix_or;
   (target + 1)[x] = pix_lut[1] | trans_lut[pix_lut[1]] | pix_or;
   (target + 2)[x] = pix_lut[2] | trans_lut[pix_lut[2]] | pix_or;
   (target + 3)[x] = pix_lut[3] | trans_lut[pix_lut[3]] | pix_or;
   (target + 4)[x] = pix_lut[4] | trans_lut[pix_lut[4]] | pix_or;
   (target + 5)[x] = pix_lut[5] | trans_lut[pix_lut[5]] | pix_or;
   (target + 6)[x] = pix_lut[6] | trans_lut[pix_lut[6]] | pix_or;
   (target + 7)[x] = pix_lut[7] | trans_lut[pix_lut[7]] | pix_or;

   bat_boom = (bat_boom + 1) & bat_width_mask;
   vdc->BG_XOffset++;
  }
//  MDFN_FastU32MemsetM8(target, overscan_color, (start + 1) & ~1);
  {
   uint32 end_begin = ((end + 1) & ~ 1);
//   MDFN_FastU32MemsetM8(target + end_begin, overscan_color, 512 - end_begin);
  }
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

static void DrawSprites(fx_vdc_t *vdc, uint32 *target, int enabled)
{
 int active_sprites = 0;
 SPRLE SpriteList[64 * 2]; // (see unlimited_sprites option, *2 to accomodate 32-pixel-width sprites ) //16];
 uint32 sprite_line_buf[512] __attribute__ ((aligned (16)));

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
     PCFXIRQ_Assert(vdc->ilevel, 1);
     //puts("OR IRQ");
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


    if((vdc->MWR & 0xC) == 4)
    {
     if(vdc->SAT[i * 4 + 2] & 1)
     {
      SpriteList[active_sprites].pattern_data[2] = 0; //vdc->VRAM[no * 64 + (y_offset & 15) ];
      SpriteList[active_sprites].pattern_data[3] = 0; //vdc->VRAM[no * 64 + (y_offset & 15) + 16];
      SpriteList[active_sprites].pattern_data[0] = vdc->VRAM[no * 64 + (y_offset & 15) + 32];
      SpriteList[active_sprites].pattern_data[1] = vdc->VRAM[no * 64 + (y_offset & 15) + 48];
     }
     else
     {
      SpriteList[active_sprites].pattern_data[0] = vdc->VRAM[no * 64 + (y_offset & 15) ];
      SpriteList[active_sprites].pattern_data[1] = vdc->VRAM[no * 64 + (y_offset & 15) + 16];
      SpriteList[active_sprites].pattern_data[2] = 0; //vdc->VRAM[no * 64 + (y_offset & 15) + 32];
      SpriteList[active_sprites].pattern_data[3] = 0; //vdc->VRAM[no * 64 + (y_offset & 15) + 48];
     }
    }
    else
    {
	//if(y_offset == 0)
        //printf("%d %d\n", vdc->RCRCount, no * 64);

     SpriteList[active_sprites].pattern_data[0] = vdc->VRAM[no * 64 + (y_offset & 15) ];
     SpriteList[active_sprites].pattern_data[1] = vdc->VRAM[no * 64 + (y_offset & 15) + 16];
     SpriteList[active_sprites].pattern_data[2] = vdc->VRAM[no * 64 + (y_offset & 15) + 32];
     SpriteList[active_sprites].pattern_data[3] = vdc->VRAM[no * 64 + (y_offset & 15) + 48];
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

 uint32 display_width = (M_vdc_HDW + 1) * 8;

 if(display_width > ClockModeWidths[vdc->dot_clock])
  display_width = ClockModeWidths[vdc->dot_clock];

 uint32 start = ((M_vdc_HDS) * 8) + 8;
 uint32 end = start + display_width;

 if(start > 512) start = 512;
 if(end > 512) end = 512;

 MDFN_FastU32MemsetM8(sprite_line_buf, ALPHA_MASK, 512);

 for(int i = (active_sprites - 1) ; i >= 0; i--)
 {
  int32 pos = SpriteList[i].x - 0x20 + start;
  uint32 prio_or = 0;

  if(SpriteList[i].flags & SPRF_PRIORITY) 
   prio_or = ALPHA_MASK << 1;

  prio_or |= (ALPHA_MASK << 2);	// For sprite #0 hit detection

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

    if(sprite_line_buf[tx] & (ALPHA_MASK << 2))
    {
     vdc->status |= VDCS_CR;
     //puts("CR IRQ");
     PCFXIRQ_Assert(vdc->ilevel, 1);
    }
    sprite_line_buf[tx] = pi | raw_pixel | prio_or;
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
    sprite_line_buf[tx] = pi | raw_pixel | prio_or;
   }
  }
 }

 if(enabled)
  for(unsigned int x = start; x < end; x++)
  {
   if(!(sprite_line_buf[x] & ALPHA_MASK))
   {
    if((target[x] & ALPHA_MASK) || (sprite_line_buf[x] & (ALPHA_MASK << 1)))
     target[x] = sprite_line_buf[x];
   }
  }
}

enum
{
 VDC_PM_VSW = 0,
 VDC_PM_VDS,
 VDC_PM_VDW,
 VDC_PM_VCR,
};

static void DoVBlank(fx_vdc_t *vdc)
{
 //printf("VBlank: %d\n", vdc->ilevel);

 vdc->VBlank_Test_Occurred = TRUE;

 if(vdc->CR & 0x08)
 {
  vdc->status |= VDCS_VD;
  PCFXIRQ_Assert(vdc->ilevel, 1);
 }

 if(vdc->SATBPending || (vdc->DCR & 0x10))
 {
  vdc->SATBPending = 0;
  vdc->sat_dma_slcounter = 2;
  for(int x = 0; x < 256; x++)
   vdc->SAT[x] = vdc->VRAM[(vdc->SATB + x) & 0xFFFF];
 }

}

void FXVDC_VSync(fx_vdc_t *vdc)
{
 if(!vdc->VBlank_Test_Occurred)
 {
  //printf("Poo: %d\n", vdc->ilevel);
  DoVBlank(vdc);
 }

 vdc->PeriodCounter = M_vdc_VDS;
 vdc->PeriodMode = VDC_PM_VDS;
 vdc->VBlank_Test_Occurred = FALSE;
}

void FXVDC_DoLine(fx_vdc_t *vdc, uint32 *linebuffer, int skip)
{
 static const unsigned int ClockPixelWidths[3] = { 341, 455, 682 };
 int have_free_time = TRUE;

 if(vdc->VBlank_Pending)
 {
  vdc->VBlank_Pending = FALSE;
  DoVBlank(vdc);
 }

 if(vdc->PeriodMode == VDC_PM_VDW)
 {
  if(!vdc->burst_mode)
   have_free_time = FALSE;
 }

 if(have_free_time) // We're outside of the active display area.  Weehee
 {
  if(vdc->DMARunning)
   DoDMA(vdc);
 }

 if(vdc->burst_mode)
 {
  if(!skip)
   MDFN_FastU32MemsetM8(linebuffer, 0x000 | ALPHA_MASK, 512);
 }
 else if(vdc->PeriodMode == VDC_PM_VDW)
 {
  if(!vdc->RCRCount)
   vdc->BG_YOffset = vdc->BYR;
  else
   vdc->BG_YOffset++;

  vdc->BG_XOffset = vdc->BXR;
  // Make sure we at least draw transparency if a layer is disabled
  if(!skip && vdc->LayerDisable)
    MDFN_FastU32MemsetM8(linebuffer, 0x000 | ALPHA_MASK, 512);
  if(!(vdc->LayerDisable & 1) && !skip)
   DrawBG(vdc, linebuffer);
  if(vdc->CR & 0x40)
   DrawSprites(vdc, linebuffer, !(skip || (vdc->LayerDisable & 0x2)));
 }
 else // Hmm, overscan...
 {
  MDFN_FastU32MemsetM8(linebuffer, 0x000 | ALPHA_MASK, 512);
 }

 if(vdc->sat_dma_slcounter)
 {
  vdc->sat_dma_slcounter--;
  if(!vdc->sat_dma_slcounter)
  {
   if(vdc->DCR & 0x01)
   {
    vdc->status |= VDCS_DS;
    PCFXIRQ_Assert(vdc->ilevel, 1);
   }
  }
 }
 
}


void FXVDC_DoLineHBlank(fx_vdc_t *vdc)
{
 vdc->PeriodCounter--;
 vdc->RCRCount++;

 if(!vdc->PeriodCounter)
 {
  if(vdc->PeriodMode == VDC_PM_VSW)
  {
   vdc->PeriodCounter = M_vdc_VDS;
   vdc->PeriodMode = VDC_PM_VDS;
  }
  else if(vdc->PeriodMode == VDC_PM_VDS)
  {
   vdc->PeriodCounter = M_vdc_VDW + 1;
   vdc->PeriodMode = VDC_PM_VDW;

   vdc->burst_mode = !(vdc->CR & 0xC0);
   vdc->RCRCount = 0;
  }
  else if(vdc->PeriodMode == VDC_PM_VDW)
  {
   vdc->VBlank_Pending = TRUE;
   vdc->PeriodCounter = M_vdc_VCR;
   vdc->PeriodMode = VDC_PM_VCR;
  }
  else if(vdc->PeriodMode == VDC_PM_VCR)
  {
   vdc->PeriodMode = VDC_PM_VSW;
   vdc->PeriodCounter = M_vdc_VSW;
   //printf("bork: %d");
  }
 }

 if((int)vdc->RCRCount == ((int)vdc->RCR - 0x40) && (vdc->CR & 0x04))
 {
  vdc->status |= VDCS_RR;
  PCFXIRQ_Assert(vdc->ilevel, 1);
 }
}

void FXVDC_Reset(fx_vdc_t *vdc)
{
 vdc->read_buffer = 0xFFFF;

 vdc->HSR = vdc->HDR = vdc->VSR = vdc->VDR = vdc->VCR = 0xFF; // Needed for Body Conquest 2 -_-

 vdc->VBlank_Pending = 0;
}

void FXVDC_Power(fx_vdc_t *vdc)
{
 //memset(vdc, 0, sizeof(fx_vdc_t));
 FXVDC_Reset(vdc);
}

fx_vdc_t *FXVDC_Init(int ilevel, bool disable_sprite_limit)
{
 fx_vdc_t *ret = (fx_vdc_t *)calloc(1, sizeof(fx_vdc_t));

 ret->ilevel = ilevel;
 ret->Aux0BPBase = ~0;

 unlimited_sprites = disable_sprite_limit;

 return(ret);
}

void FXVDC_Close(fx_vdc_t *vdc)
{
 free(vdc);
}

void FXVDC_SetAux0BPBpase(fx_vdc_t *vdc, uint32 new_base)
{
 vdc->Aux0BPBase = new_base;
}

#ifdef WANT_DEBUGGER
void FXVDC_GetAddressSpaceBytes(fx_vdc_t *vdc, const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "vram"))
 {
  while(Length--)
  {
   Address &= 0x1FFFF;
   *Buffer = vdc->VRAM[Address >> 1] >> ((Address & 1) * 8);
   Address++;
   Buffer++;
  }
 }
}

void FXVDC_PutAddressSpaceBytes(fx_vdc_t *vdc, const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "vram"))
 {
  while(Length--)
  {
   Address &= 0x1FFFF;
   REGSETP(vdc->VRAM[Address >> 1], *Buffer, Address & 1);
   FixTileCache(vdc, Address >> 1);
   Address++;
   Buffer++;
  }
 }
}


void FXVDC_SetRegister(fx_vdc_t *vdc, const std::string &name, uint32 value)
{
 if(name == "MAWR")
  vdc->MAWR = value;
 else if(name == "MARR")
  vdc->MARR = value;
 else if(name == "Status")
 {
  vdc->status = value & 0x7F;
  PCFXIRQ_Assert(vdc->ilevel, (value & 0x3F) ? TRUE : FALSE);
 }
 else if(name == "CR")
 {
  vdc->CR = value;
 }

}

uint32 FXVDC_GetRegister(fx_vdc_t *vdc, const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name == "MAWR")
  value = vdc->MAWR;
 else if(name == "MARR")
  value = vdc->MARR;
 else if(name == "Status")
  value = vdc->status;
 else if(name == "CR")
 {
  value = vdc->CR;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "Sprite Hit IRQ: %s, Sprite Overflow IRQ: %s, RCR IRQ: %s, VBlank IRQ: %s, Sprites: %s, Background: %s", (value & 1) ? "On" : "Off", (value & 2) ? "On" : "Off",
       (value & 4) ? "On" : "Off", (value & 8) ? "On" : "Off", (value & 0x40) ? "On" : "Off", (value & 0x80) ? "On" : "Off");
   *special = std::string(buf);
  }
 }
 else if(name == "RCR")
  value = vdc->RCR;
 else if(name == "BXR")
  value = vdc->BXR;
 else if(name == "BYR")
  value = vdc->BYR;
 else if(name == "MWR")
  value = vdc->MWR;
 else if(name == "HSR")
  value = vdc->HSR;
 else if(name == "HDR")
  value = vdc->HDR;
 else if(name == "VSR")
 {
  value = vdc->VSR;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "VSW: %02x, VDS: %02x", value & 0x1F, (value >> 8) & 0xFF);
   *special = std::string(buf);
  }
 }
 else if(name == "VDR")
  value = vdc->VDR;
 else if(name == "VCR")
  value = vdc->VCR;
 else if(name == "DCR")
  value = vdc->DCR;
 else if(name == "SOUR")
  value = vdc->SOUR;
 else if(name == "DESR")
  value = vdc->DESR;
 else if(name == "LENR")
  value = vdc->LENR;
 else if(name == "SATB")
  value = vdc->SATB;

 return(value);
}
#endif


int FXVDC_StateAction(StateMem *sm, int load, int data_only, fx_vdc_t *vdc, const char *name)
{
 SFORMAT VDC_StateRegs[] = 
 {
	SFVARN(vdc->PeriodMode, "Period_Mode"),
	SFVARN(vdc->PeriodCounter, "Period_Counter"),
        SFVARN(vdc->sat_dma_slcounter, "sat_dma_slcounter"),
	SFVARN(vdc->VBlank_Test_Occurred, "VB_test_occurred"),

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

        SFARRAY16N(vdc->VRAM, 65536, "VRAM"),
        SFVARN(vdc->DMAReadBuffer, "DMAReadBuffer"),
        SFVARN(vdc->DMAReadWrite, "DMAReadWrite"),
        SFVARN(vdc->DMARunning, "DMARunning"),
        SFVARN(vdc->SATBPending, "SATBPending"),
        SFVARN(vdc->burst_mode, "burst_mode"),

        SFVARN(vdc->BG_YOffset, "BG_YOffset"),
        SFVARN(vdc->BG_XOffset, "BG_XOffset"),
	SFVARN(vdc->VDS_cache, "VDS_cache"),
        SFVARN(vdc->VSW_cache, "VSW_cache"),
        SFVARN(vdc->VDW_cache, "VDW_cache"),
        SFVARN(vdc->VCR_cache, "VCR_cache"),
        SFVARN(vdc->VBlankFL_cache, "VBlankFL_cache"),
	SFEND
  };
  int ret = MDFNSS_StateAction(sm, load, data_only, VDC_StateRegs, name);

  if(load)
  {
   for(int x = 0; x < 65536; x++)
    FixTileCache(vdc, x);
  }

 return(ret);
}

void FXVDC_SetDotClock(fx_vdc_t *vdc, unsigned int clock)
{
 vdc->dot_clock = clock;
}

bool FXVDC_ToggleLayer(fx_vdc_t *vdc, int which)
{
 vdc->LayerDisable ^= 1 << which;

 return(!((vdc->LayerDisable >> which) & 1));
}


void FXVDC_DoGfxDecode(fx_vdc_t *vdc, uint32 *palette_ptr, uint32 *target, int w, int h, int scroll, bool DecodeSprites)
{
 if(DecodeSprites)
 {
  for(int y = 0; y < h; y++)
  {
   for(int x = 0; x < w; x += 16)
   {
    int which_tile = (x / 16) + (scroll + (y / 16)) * (w / 16);

    if(which_tile >= 0x400)
    {
     for(int sx = 0; sx < 16; sx++)
     {
      target[x + sx] = MK_COLORA(0, 0, 0, 0);
      target[x + w * 1 + sx] = 0;
      target[x + w * 2 + sx] = 0;
     }
     continue;
    }
    //which_tile &= 0x3FF;

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

   if(which_tile >= 0x1000)
   {
    for(int sx = 0; sx < 8; sx++)
    {
     target[x + sx] = MK_COLORA(0, 0, 0, 0);
     target[x + w * 1 + sx] = 0;
     target[x + w * 2 + sx] = 0;
    }
    continue;
   }
   //which_tile &= 0xFFF;

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
