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

/* VDC emulation */

#include "mednafen/mednafen.h"
#include "mednafen/video.h"
#include "mednafen/lepacker.h"

#include <trio/trio.h>
#include <math.h>
#include "vdc.h"

#define VDC_DEBUG(x, ...)     { }
//#define VDC_DEBUG(x, ...)       printf(x ": HPhase=%d, HPhaseCounter=%d, RCRCount=%d\n", ## __VA_ARGS__, HPhase, HPhaseCounter, RCRCount);

#define VDC_UNDEFINED(format, ...)   { }
//#define VDC_UNDEFINED(format, ...)      printf(format " RCRCount=%d" "\n", ## __VA_ARGS__, RCRCount)

#define VDC_WARNING(format, ...)      { }
//#define VDC_WARNING(format, ...)     { printf(format "\n", ## __VA_ARGS__); }

#define ULE_BG		1
#define ULE_SPR		2

static const unsigned int bat_width_tab[4] = { 32, 64, 128, 128 };
static const unsigned int bat_width_shift_tab[4] = { 5, 6, 7, 7 };
static const unsigned int bat_height_tab[2] = { 32, 64 };

void VDC::FixTileCache(uint16 A)
{
 uint32 charname = (A >> 4);
 uint32 y = (A & 0x7);
 uint8 *tc = bg_tile_cache[charname][y];

 uint32 bitplane01 = VRAM[y + charname * 16];
 uint32 bitplane23 = VRAM[y+ 8 + charname * 16];

 for(int x = 0; x < 8; x++)
 {
  uint32 raw_pixel = ((bitplane01 >> x) & 1);
  raw_pixel |= ((bitplane01 >> (x + 8)) & 1) << 1;
  raw_pixel |= ((bitplane23 >> x) & 1) << 2;
  raw_pixel |= ((bitplane23 >> (x + 8)) & 1) << 3;
  tc[7 - x] = raw_pixel;
 }
}

// Some virtual vdc macros to make code simpler to read
#define M_vdc_HSW	(HSR & 0x1F)	// Horizontal Synchro Width
#define M_vdc_HDS	((HSR >> 8) & 0x7F) // Horizontal Display Start
#define M_vdc_HDW	(HDR & 0x7F)	// Horizontal Display Width
#define M_vdc_HDE	((HDR >> 8) & 0x7F) // Horizontal Display End

#define M_vdc_VSW	(VSR & 0x1F)	// Vertical synchro width
#define M_vdc_VDS	((VSR >> 8) & 0xFF) // Vertical Display Start
#define M_vdc_VDW	(VDR & 0x1FF)	// Vertical Display Width(Height? :b)
#define M_vdc_VCR	(VCR & 0xFF)

#define M_vdc_EX	((CR >> 4) & 0x3)
#define M_vdc_TE	((CR >> 8) & 0x3)

#define VDCS_CR		0x01 // Sprite #0 collision interrupt occurred
#define VDCS_OR		0x02 // sprite overflow "" ""
#define VDCS_RR		0x04 // RCR             ""  ""
#define VDCS_DS		0x08 // VRAM to SAT DMA completion interrupt occurred
#define VDCS_DV		0x10 // VRAM to VRAM DMA completion interrupt occurred
#define VDCS_VD		0x20 // Vertical blank interrupt occurred
#define VDCS_BSY	0x40 // VDC is waiting for a CPU access slot during the active display area??

uint32 VDC::GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 value = 0xDEADBEEF;

 switch(id)
 {
  case GSREG_SELECT:
	value = select;
	break;

  case GSREG_STATUS:
	value = status;
	break;

  case GSREG_MAWR:
	value = MAWR;
	break;

  case GSREG_MARR:
	value = MARR;
	break;

  case GSREG_CR:
	value = CR;

	if(special)
	{
	 trio_snprintf(special, special_len, "Sprite Hit IRQ: %s, Sprite Overflow IRQ: %s, RCR IRQ: %s, VBlank IRQ: %s, Sprites: %s, Background: %s", (value & 1) ? "On" : "Off", (value & 2) ? "On" : "Off",
	        (value & 4) ? "On" : "Off", (value & 8) ? "On" : "Off", (value & 0x40) ? "On" : "Off", (value & 0x80) ? "On" : "Off");
	}
	break;

  case GSREG_RCR:
	value = RCR;
	break;

  case GSREG_BXR:
	value = BXR;
	break;

  case GSREG_BYR:
	value = BYR;
	break;

  case GSREG_MWR:
	value = MWR;

	if(special)
	{
	 trio_snprintf(special, special_len, "CG Mode: %d, BAT Width: %d(tiles), BAT Height: %d(tiles)", (int)(bool)(value & 0x80), 
											     bat_width_tab[(value >> 4) & 0x3],
											     bat_height_tab[(value >> 6) & 0x1]);
	}
	break;

  case GSREG_HSR:
	value = HSR;
	if(special)
	{
	 trio_snprintf(special, special_len, "HSW: %02x, HDS: %02x", value & 0x1F, (value >> 8) & 0x7F);
	}
	break;

  case GSREG_HDR:
	value = HDR;
	if(special)
	{
	 trio_snprintf(special, special_len, "HDW: %02x, HDE: %02x", value & 0x7F, (value >> 8) & 0x7F);
	}
	break;


  case GSREG_VSR:
	value = VSR;
	if(special)
	{
	 trio_snprintf(special, special_len, "VSW: %02x, VDS: %02x", value & 0x1F, (value >> 8) & 0xFF);
	}
	break;

  case GSREG_VDR:
	value = VDR;
	break;

  case GSREG_VCR:
	value = VCR;
	break;

  case GSREG_DCR:
	value = DCR;
	if(special)
	{
	 trio_snprintf(special, special_len, "SATB DMA IRQ: %s, VRAM DMA IRQ: %s, DMA Source Address: %s, DMA Dest Address: %s, Auto SATB DMA: %s",
        	(DCR & 0x1) ? "On" : "Off", (DCR & 0x2) ? "On" : "Off", (DCR & 0x4) ? "Decrement" : "Increment", (DCR & 0x8) ? "Decrement" : "Increment", 
	        (DCR & 0x10) ? "On" : "Off");
	}
	break;

  case GSREG_SOUR:
	value = SOUR;
	break;

 case GSREG_DESR:
	value = DESR;
	break;

  case GSREG_LENR:
	value = LENR;
	break;

  case GSREG_DVSSR:
	value = DVSSR;
	break;
 }

 return(value);
}

void VDC::SetRegister(const unsigned int id, const uint32 value)
{
 switch(id)
 {
  default: break;

  case GSREG_STATUS:
	status = value & 0x3F;
	break;

  case GSREG_SELECT:
	select = value & 0x1F;
	break;

  case GSREG_MAWR:
	MAWR = value & 0xFFFF;
	break;

  case GSREG_MARR:
	MARR = value & 0xFFFF;
	break;

  case GSREG_CR:
	CR = value & 0xFFFF;
	break;

  case GSREG_RCR:
	RCR = value & 0x3FF;
	break;

  case GSREG_BXR:
	BXR = value & 0x3FF;
	break;

  case GSREG_BYR:
	BYR = value & 0x1FF;
	break;

  case GSREG_MWR:
	MWR = value & 0xFFFF;
	break;

  case GSREG_HSR:
	HSR = value & 0xFFFF;
	break;

  case GSREG_HDR:
	HDR = value & 0xFFFF;
	break;

  case GSREG_VSR:
	VSR = value & 0xFFFF;
	break;

  case GSREG_VDR:
	VDR = value & 0xFFFF;
	break;

  case GSREG_VCR:
	VCR = value & 0xFFFF;
	break;

  case GSREG_DCR:
	DCR = value & 0xFFFF;
	break;

  case GSREG_SOUR:
	SOUR = value & 0xFFFF;
	break;

  case GSREG_DESR:
	DESR = value & 0xFFFF;
	break;

  case GSREG_LENR:
	LENR = value & 0xFFFF;
	break;

  case GSREG_DVSSR:
	DVSSR = value & 0xFFFF;
	break;
 }
}

bool VDC::ToggleLayer(int which)
{
 userle ^= 1 << which;
 return((userle >> which) & 1);
}

void VDC::RunSATDMA(int32 cycles, bool force_completion)
{
 assert(sat_dma_counter > 0);

 if(force_completion)
  cycles = sat_dma_counter;

 sat_dma_counter -= cycles;
 if(sat_dma_counter <= 0)
 {
  if(DCR & 0x01)
  {
   VDC_DEBUG("Sprite DMA IRQ");
   status |= VDCS_DS;
   IRQHook(TRUE);
  }
  CheckAndCommitPending();
  burst_mode = true;
 }
}

void VDC::RunDMA(int32 cycles, bool force_completion)
{
 int num_transfers = 0;

 if(force_completion)
 {
  VDMA_CycleCounter = 0;

  num_transfers = 65536 * 2;
 }
 else
 {
  VDMA_CycleCounter += cycles;
  num_transfers = VDMA_CycleCounter >> 1;
  VDMA_CycleCounter -= num_transfers << 1;
 }

 while(num_transfers--)
 {
  if(!DMAReadWrite)
  {
   if(SOUR >= VRAM_Size)
    VDC_UNDEFINED("Unmapped VRAM DMA read");

   DMAReadBuffer = VRAM[SOUR];
   //printf("DMA Read: %04x, %04x\n", SOUR, DMAReadBuffer);
  }
  else
  {
   if(DESR < VRAM_Size)
   {
    VRAM[DESR] = DMAReadBuffer;
    FixTileCache(DESR);
   }

   SOUR += (((DCR & 0x4) >> 1) ^ 2) - 1;
   DESR += (((DCR & 0x8) >> 2) ^ 2) - 1;
   LENR--;

   if(LENR == 0xFFFF)  // DMA is done.
   {
    DMARunning = 0;	// Clear this BEFORE CheckAndCommitPending()

    CheckAndCommitPending();

    if(DCR & 0x02)
    {
     status |= VDCS_DV;
     IRQHook(TRUE);
     VDC_DEBUG("DMA IRQ");
    }
    break;
   }
  }
  DMAReadWrite ^= 1;
 }
}

/*
<RyphZomb> ChrlyMac: Was it you who determined exactly how many VDC clocks the SAT DMA took?
<RyphZomb> I know someone did, but I can't remember the results...
<ChrlyMac> 1024
<ChrlyMac> It happens at the VDW->VCR transition
*/

void VDC::IncRCR(void)
{
 NeedBGYInc = true;
 RCRCount++;

 VPhaseCounter--;

 if(VPhaseCounter <= 0)
 {
  VPhase = (VPhase + 1) % VPHASE_COUNT;
  switch(VPhase)
  {
   case VPHASE_VDS: VPhaseCounter = VDS_cache + 2;
		    break;

   case VPHASE_VDW: VPhaseCounter = VDW_cache + 1;
		    //BG_YMoo = BYR - 1;
		    RCRCount = 0;
		    burst_mode = !(CR & 0xC0);
		    NeedVBIRQTest = true;
		    NeedSATDMATest = true;

		    if(!burst_mode)
		    {
		     if(sat_dma_counter > 0)
		     {
		      printf("SAT DMA cancelled???\n");
		      sat_dma_counter = 0;
		      CheckAndCommitPending();
		     }
		     if(DMARunning)
		     {
		      printf("DMA Running Cancelled\n");
		      DMARunning = false;
		      CheckAndCommitPending();
		     }
		    }
		    break;

   case VPHASE_VCR: VPhaseCounter = VCR_cache;
		    break;

   case VPHASE_VSW: VPhaseCounter = VSW_cache + 1;
		    MWR_cache = MWR;
		    VDS_cache = M_vdc_VDS;
		    VSW_cache = M_vdc_VSW;
		    VDW_cache = M_vdc_VDW;
		    VCR_cache = M_vdc_VCR;
		    //VDC_WARNING("VSW Started");
		    break;
  }
 }

 if(VPhase == VPHASE_VDW && !burst_mode)
 {
  FetchSpriteData();
 }

 if((int)RCRCount == ((int)RCR - 0x40) && (CR & 0x04))
 {
  VDC_DEBUG("RCR IRQ");
  status |= VDCS_RR;
  IRQHook(TRUE);
 }
}

void VDC::DoVBIRQTest(void)
{
 if(CR & 0x08)
 {
  VDC_DEBUG("VBlank IRQ");
  status |= VDCS_VD;
  IRQHook(TRUE);
 }
}

static const int Cycles_Between_RCRIRQ_And_HDWEnd = 4;

int VDC::TimeFromHDSStartToBYRLatch(void)
{
 int ret = 1;

 if(HDS_cache > 2)
  ret += ((HDS_cache + 1) * 8) - 24 + 2;


 //printf("%d, %d\n", HDS_cache, ret);

 return(ret);
}

int VDC::TimeFromBYRLatchToBXRLatch(void)
{
 int ret = 2;

 if(HDS_cache > 2)
  ret = 1;

 return(ret);
}

void VDC::HDS_Start(void)
{
 if(NeedRCRInc)
 {
  if(NeedBGYInc)
  {
   NeedBGYInc = false;
   if(0 == RCRCount)
    BG_YMoo = BYR;
   else
    BG_YMoo++;
  }

  IncRCR();
  NeedRCRInc = false;
 }

 if(sprite_cg_fetch_counter > 0)
 {
  VDC_WARNING("Sprite truncation on %d.  Wanted sprites: %d, cycles needed but not left: %d\n", RCRCount, active_sprites, sprite_cg_fetch_counter);
  sprite_cg_fetch_counter = 0;
  CheckAndCommitPending();
 }

 HSW_cache = M_vdc_HSW;
 HDS_cache = M_vdc_HDS;
 HDW_cache = M_vdc_HDW;
 HDE_cache = M_vdc_HDE;

 VDC_DEBUG("HDS Start!  HSW: %d, HDW: %d, HDW: %d, HDE: %d\n", HSW_cache, HDS_cache, HDW_cache, HDE_cache);

 CR_cache = CR;

 HPhase = HPHASE_HDS;
 HPhaseCounter = TimeFromHDSStartToBYRLatch();
}

int32 VDC::HSync(bool hb)
{
 if(M_vdc_EX)
 {
  in_exhsync = 0;
  return(CalcNextEvent());
 }
 in_exhsync = hb;

 if(hb) // Going into hsync
 {
  mystery_counter = 48;
  mystery_phase = false;
 }
 else // Leaving hsync
 {
  HPhase = HPHASE_HSW;
  HPhaseCounter = 8;

  //HDS_Start();
  //HPhaseCounter += 8;

  pixel_copy_count = 0;
 }


 return(CalcNextEvent());
}

int32 VDC::VSync(bool vb)
{
 if(M_vdc_EX >= 0x2)
 {
  in_exvsync = 0;
  return(CalcNextEvent());
 }
 in_exvsync = vb;

 //printf("VBlank: %d\n", vb);
 if(vb) // Going into vsync
 {
  NeedRCRInc = false;
  NeedBGYInc = false;
/*  if(NeedRCRInc)
  {
   IncRCR();
   NeedRCRInc = false;
  }
*/
  MWR_cache = MWR;

  VDS_cache = M_vdc_VDS;
  VSW_cache = M_vdc_VSW;
  VDW_cache = M_vdc_VDW;
  VCR_cache = M_vdc_VCR;

  VPhase = VPHASE_VSW;
  VPhaseCounter = VSW_cache + 1;
 }
 else	// Leaving vsync
 {

 }
 return(CalcNextEvent());
}

//int32 VDC::Run(int32 clocks, bool hs, bool vs, uint16 *pixels, bool skip)
int32 VDC::Run(int32 clocks, uint16 *pixels, bool skip)
{
 //uint16 *spixels = pixels;

 //puts("Run begin");
 //fflush(stdout);

 while(clocks > 0)
 {
  int32 chunk_clocks = clocks;

  if(chunk_clocks > HPhaseCounter)
  {
   chunk_clocks = HPhaseCounter;
  }

  if(sat_dma_counter > 0 && chunk_clocks > sat_dma_counter)
   chunk_clocks = sat_dma_counter;

  if(sprite_cg_fetch_counter > 0 && chunk_clocks > sprite_cg_fetch_counter)
   chunk_clocks = sprite_cg_fetch_counter;

  if(mystery_counter > 0 && chunk_clocks > mystery_counter)
   chunk_clocks = mystery_counter;

  if(mystery_counter > 0)
  {
   mystery_counter -= chunk_clocks;
   if(mystery_counter <= 0)
   {
    mystery_phase = !mystery_phase;
    if(mystery_phase)
     mystery_counter = 16;
    else
     CheckAndCommitPending();
   }
  }

  if(sprite_cg_fetch_counter > 0)
  {
   sprite_cg_fetch_counter -= chunk_clocks;
   if(sprite_cg_fetch_counter <= 0)
    CheckAndCommitPending();
  }

  if(VPhase != VPHASE_VDW)
  {
   if(NeedSATDMATest)
   {
    NeedSATDMATest = false;
    if(SATBPending || (DCR & 0x10))
    {
        SATBPending = 0;

        sat_dma_counter = 1024;

        if(DVSSR > (VRAM_Size - 0x100))
         VDC_UNDEFINED("Unmapped VRAM DVSSR DMA read");

        if(DVSSR < VRAM_Size)
        {
         uint32 len = 256;
         if(DVSSR > (VRAM_Size - 0x100))
          len = VRAM_Size - DVSSR;
         memcpy(SAT, &VRAM[DVSSR], len * sizeof(uint16));
        }
    }
   }
  }




  if(DMAPending && burst_mode)
  {
   VDC_DEBUG("DMA Started");
   DMAPending = false;
   DMARunning = true;
   VDMA_CycleCounter = 0;
   DMAReadWrite = 0;
  }

  if(sat_dma_counter > 0)
   RunSATDMA(chunk_clocks);
  else if(DMARunning)
   RunDMA(chunk_clocks);

  if(pixel_copy_count > 0)
  {
   if(!skip)
   {
    for(int i = 0; i < chunk_clocks; i++)
     pixels[i] = linebuf[pixel_desu + i];
    //memcpy(pixels, linebuf + pixel_desu, chunk_clocks * sizeof(uint16));

    if(M_vdc_TE == 0x1)
     for(int i = 0; i < chunk_clocks; i++)
      pixels[i] |= VDC_DISP_OUT_MASK;
   }

   pixel_desu += chunk_clocks;
   pixel_copy_count -= chunk_clocks;
  }
  else
  {
   uint16 pix = 0x100;

   if(M_vdc_TE == 0x1)
   {
    if(HPhase != HPHASE_HDS && HPhase != HPHASE_HDS_PART2 && HPhase != HPHASE_HDS_PART3)
     pix |= VDC_DISP_OUT_MASK;
   }

   if(HPhase == HPHASE_HSW)
   {
    if(M_vdc_EX >= 0x1)
     pix |= VDC_HSYNC_OUT_MASK;
 
    if(M_vdc_TE >= 0x2)
     pix |= VDC_DISP_OUT_MASK;
   }
   if(VPhase == VPHASE_VSW && M_vdc_EX >= 0x2)
    pix |= VDC_VSYNC_OUT_MASK;

   if(!(userle & 1))
    pix |= VDC_BGDISABLE_OUT_MASK;

   if(!skip)
   {
    for(int i = 0; i < chunk_clocks; i++)
     pixels[i] = pix;
   }
  }

  HPhaseCounter -= chunk_clocks;

  assert(HPhaseCounter >= 0);

  while(HPhaseCounter <= 0)
  {
   HPhase = (HPhase + 1) % HPHASE_COUNT;
  
   switch(HPhase)
   { 
    case HPHASE_HDS: HDS_Start();
		     break;


    case HPHASE_HDS_PART2:
                     HPhaseCounter = TimeFromBYRLatchToBXRLatch();	

		     if(NeedBGYInc)
		     {
		      NeedBGYInc = false;

		      if(0 == RCRCount)
		       BG_YMoo = BYR;
		      else
		       BG_YMoo++;
		     }
                     BG_YOffset = BG_YMoo;
		     break;

    case HPHASE_HDS_PART3:
		     HPhaseCounter = (HDS_cache + 1) * 8 - TimeFromHDSStartToBYRLatch() - TimeFromBYRLatchToBXRLatch();

		     assert(HPhaseCounter > 0);

		     BG_XOffset = BXR;
		     break;

    case HPHASE_HDW: 
		     NeedRCRInc = true;
		     if(VPhase != VPHASE_VDW && NeedVBIRQTest)
		     {
		      DoVBIRQTest();
		      NeedVBIRQTest = false;
		     }
                     CheckAndCommitPending();

		     HPhaseCounter = (HDW_cache + 1) * 8 - Cycles_Between_RCRIRQ_And_HDWEnd;
		     if(VPhase == VPHASE_VDW)
		     {
		      if(!burst_mode)
		      {
		       pixel_desu = 0;
		       pixel_copy_count = (HDW_cache + 1) * 8;

		       // BG off, sprite on: fill = 0x100.  bg off, sprite off: fill = 0x000
		       if(!(CR_cache & 0x80))
		       {
		        uint16 fill_val;

		        if(!(CR_cache & 0xC0))	// Sprites and BG off
			 fill_val = 0x000;			
		        else	// Only BG off
			 fill_val = 0x100 | ((userle & ULE_BG) ? 0 : VDC_BGDISABLE_OUT_MASK);

			if(!(userle & ULE_BG))
			 fill_val |= VDC_BGDISABLE_OUT_MASK;

			for(int i = 0; i < pixel_copy_count; i++)
			 linebuf[i] = fill_val;
		       }

		       if(!skip)
 		        if(CR_cache & 0x80)
			{
		         DrawBG(linebuf, userle & ULE_BG);
			}
			//printf("%d %02x %02x\n", RCRCount, CR, CR_cache);
		       if(CR_cache & 0x40)
		        DrawSprites(linebuf, (userle & ULE_SPR) && !skip);
		      }
		     }
		     break;

    case HPHASE_HDW_FINAL:
		     if(NeedRCRInc)
		     {
		      IncRCR();
		      NeedRCRInc = false;
		     }
		     HPhaseCounter = Cycles_Between_RCRIRQ_And_HDWEnd;
		     break;

    case HPHASE_HDE: //if(!burst_mode) //if(VPhase == VPHASE_VDW)	//if(!burst_mode)
		     // lastats = 16;	// + 16;
		     //else
		     // lastats = 16;
		     HPhaseCounter = (HDE_cache + 1) * 8;
		     break;

    case HPHASE_HSW: HPhaseCounter = (HSW_cache + 1) * 8; break;
   }
  }
  pixels += chunk_clocks;
  clocks -= chunk_clocks;
 }

 //puts("Run end");
 //fflush(stdout);

 return(CalcNextEvent());
}


void VDC::CalcWidthStartEnd(uint32 &display_width, uint32 &start, uint32 &end)
{
 display_width = (M_vdc_HDW + 1) * 8;

 start = 0;
 end = start + display_width;
}

void VDC::DrawBG(uint16 *target, int enabled)
{
 uint32 width;
 uint32 start;
 uint32 end;
 int bat_width = bat_width_tab[(MWR_cache >> 4) & 3];
 int bat_width_mask = bat_width - 1;
 int bat_width_shift = bat_width_shift_tab[(MWR_cache >> 4) & 3];
 int bat_height_mask = bat_height_tab[(MWR_cache >> 6) & 1] - 1;

 CalcWidthStartEnd(width, start, end);

 if(!enabled)
 {
  for(uint32 x = start; x < end; x++)
   target[x] = 0x000 | VDC_BGDISABLE_OUT_MASK;
  return;
 }

 {
  int bat_y = ((BG_YOffset >> 3) & bat_height_mask) << bat_width_shift;
  uint32 first_end = start + 8 - (BG_XOffset & 7);
  uint32 dohmask = 0xFFFFFFFF;

  if((MWR_cache & 0x3) == 0x3)
  {
   if(MWR_cache & 0x80)
    dohmask = 0xCCCCCCCC;
   else
    dohmask = 0x33333333;
  }

  // Draw the first pixels of the first tile, depending on the lower 3 bits of the xscroll/xoffset register, to
  // we can render the rest of the line in 8x1 chunks, which is faster.
  for(uint32 x = start; x < first_end; x++)
  {
   int bat_x = (BG_XOffset >> 3) & bat_width_mask;
   uint16 bat = VRAM[bat_x | bat_y];
   const uint8 pal_or = ((bat >> 8) & 0xF0);
   int palette_index = ((bat >> 12) & 0x0F) << 4;
   uint32 raw_pixel;

   raw_pixel = bg_tile_cache[bat & 0xFFF][BG_YOffset & 7][BG_XOffset & 0x7] & dohmask;
   target[x] = palette_index | raw_pixel | pal_or;

   if((bat & 0xFFF) > VRAM_BGTileNoMask)
    VDC_UNDEFINED("Unmapped BG tile read");

   BG_XOffset++;
  }

  int bat_boom = (BG_XOffset >> 3) & bat_width_mask;
  int line_sub = BG_YOffset & 7;

  if((MWR_cache & 0x3) == 0x3)
  {
   for(uint32 x = first_end; x < end; x+=8)
   {
    const uint16 bat = VRAM[bat_boom | bat_y];
    const uint8 pal_or = ((bat >> 8) & 0xF0);
    uint8 *pix_lut = bg_tile_cache[bat & 0xFFF][line_sub];

    if((bat & 0xFFF) > VRAM_BGTileNoMask)
     VDC_UNDEFINED("Unmapped BG tile read");


    (target + 0)[x] = (pix_lut[0] & dohmask) | pal_or;
    (target + 1)[x] = (pix_lut[1] & dohmask) | pal_or;
    (target + 2)[x] = (pix_lut[2] & dohmask) | pal_or;
    (target + 3)[x] = (pix_lut[3] & dohmask) | pal_or;
    (target + 4)[x] = (pix_lut[4] & dohmask) | pal_or;
    (target + 5)[x] = (pix_lut[5] & dohmask) | pal_or;
    (target + 6)[x] = (pix_lut[6] & dohmask) | pal_or;
    (target + 7)[x] = (pix_lut[7] & dohmask) | pal_or;

    bat_boom = (bat_boom + 1) & bat_width_mask;
    BG_XOffset++;

   }
  }
  else
  for(uint32 x = first_end; x < end; x+=8) // This will draw past the right side of the buffer, but since our pitch is 1024, and max width is ~512, we're safe.  Also,
					// any overflow that is on the visible screen are will be hidden by the overscan color code below this code.
  {
   const uint16 bat = VRAM[bat_boom | bat_y];
   const uint8 pal_or = ((bat >> 8) & 0xF0);
   uint8 *pix_lut = bg_tile_cache[bat & 0xFFF][line_sub];

   if((bat & 0xFFF) > VRAM_BGTileNoMask)
    VDC_UNDEFINED("Unmapped BG tile read");

   #ifdef LSB_FIRST
    #if SIZEOF_LONG == 8
    uint64 doh = *(uint64 *)pix_lut;

    (target + 0)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 1)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 2)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 3)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 4)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 5)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 6)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 7)[x] = (doh) | pal_or;
    #else
    uint32 doh = *(uint32 *)pix_lut;
    (target + 0)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 1)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 2)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 3)[x] = doh | pal_or;
    doh = *(uint32 *)(pix_lut + 4);
    (target + 4)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 5)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 6)[x] = (doh & 0xFF) | pal_or;
    doh >>= 8;
    (target + 7)[x] = doh | pal_or;
    #endif
   #else
   (target + 0)[x] = pix_lut[0] | pal_or;
   (target + 1)[x] = pix_lut[1] | pal_or;
   (target + 2)[x] = pix_lut[2] | pal_or;
   (target + 3)[x] = pix_lut[3] | pal_or;
   (target + 4)[x] = pix_lut[4] | pal_or;
   (target + 5)[x] = pix_lut[5] | pal_or;
   (target + 6)[x] = pix_lut[6] | pal_or;
   (target + 7)[x] = pix_lut[7] | pal_or;
   #endif

   bat_boom = (bat_boom + 1) & bat_width_mask;
   BG_XOffset++;
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

void VDC::FetchSpriteData(void)
{
 active_sprites = 0;

 // First, grab the up to 16 sprites.
 for(int i = 0; i < 64; i++)
 {
  int16 y = (SAT[i * 4 + 0] & 0x3FF) - 0x40;
  uint16 x = (SAT[i * 4 + 1] & 0x3FF);
  uint16 no = (SAT[i * 4 + 2] >> 1) & 0x3FF;	// Todo, cg mode bit
  uint16 flags = (SAT[i * 4 + 3]);

  uint32 palette_index = (flags & 0xF) << 4;
  uint32 height = sprite_height_tab[(flags >> 12) & 3];
  uint32 width = sprite_width_tab[(flags >> 8) & 1];

  if((int32)RCRCount >= y && (int32)RCRCount < (int32)(y + height))
  {
   bool second_half = 0;
   uint32 y_offset = RCRCount - y;
   if(y_offset > height) continue;


   breepbreep:

   if(active_sprites == 16)
   {
    if(CR & 0x2)
    {
     status |= VDCS_OR;
     IRQHook(TRUE);
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
    //printf("Found: %d %d\n", RCRCount, x);
    SpriteList[active_sprites].x = x;
    SpriteList[active_sprites].palette_index = palette_index;

    if((no * 64) >= VRAM_Size)
     VDC_UNDEFINED("Unmapped VRAM sprite tile read");

    if((MWR_cache & 0xC) == 4)
    {
     if(SAT[i * 4 + 2] & 1)
     {
      SpriteList[active_sprites].pattern_data[0] = VRAM[no * 64 + (y_offset & 15) + 32];
      SpriteList[active_sprites].pattern_data[1] = VRAM[no * 64 + (y_offset & 15) + 48];
      SpriteList[active_sprites].pattern_data[2] = 0; 
      SpriteList[active_sprites].pattern_data[3] = 0;
     }
     else
     {
      SpriteList[active_sprites].pattern_data[0] = VRAM[no * 64 + (y_offset & 15) ];
      SpriteList[active_sprites].pattern_data[1] = VRAM[no * 64 + (y_offset & 15) + 16];
      SpriteList[active_sprites].pattern_data[2] = 0;
      SpriteList[active_sprites].pattern_data[3] = 0;
     }
    }
    else
    {
     SpriteList[active_sprites].pattern_data[0] = VRAM[no * 64 + (y_offset & 15) ];
     SpriteList[active_sprites].pattern_data[1] = VRAM[no * 64 + (y_offset & 15) + 16];
     SpriteList[active_sprites].pattern_data[2] = VRAM[no * 64 + (y_offset & 15) + 32];
     SpriteList[active_sprites].pattern_data[3] = VRAM[no * 64 + (y_offset & 15) + 48];
    }

    SpriteList[active_sprites].flags |= i ? 0 : SPRF_SPRITE0;

    active_sprites++;

    if(width == 32 && !second_half)
    {
     second_half = 1;
     x += 16;
     y_offset = RCRCount - y;	// Fix the y offset so that sprites that are hflipped + vflipped display properly
     goto breepbreep;
    }
   }
  }
 }

 sprite_cg_fetch_counter = ((active_sprites < 16) ? active_sprites : 16) * 4;
}

void VDC::DrawSprites(uint16 *target, int enabled)
{
 MDFN_ALIGN(16) uint16 sprite_line_buf[1024];

 uint32 display_width, start, end;

 CalcWidthStartEnd(display_width, start, end);

 for(unsigned int i = start; i < end; i++)
  sprite_line_buf[i] = 0;

 for(int i = (active_sprites - 1) ; i >= 0; i--)
 {
  int32 pos = SpriteList[i].x - 0x20 + start;
  uint32 prio_or = 0;

  if(SpriteList[i].flags & SPRF_PRIORITY) 
   prio_or = 0x200;

  if((SpriteList[i].flags & SPRF_SPRITE0) && (CR & 0x01))
  {
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

     if(sprite_line_buf[tx] & 0xF)
     {
      status |= VDCS_CR;
      VDC_DEBUG("Sprite hit IRQ");
      IRQHook(TRUE);
     }
     sprite_line_buf[tx] = pi | raw_pixel | prio_or;
    }
   }
  } // End sprite hit loop
  else
  {
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
  } // End non-sprite-hit loop
 }

 if(enabled)
 {
  for(unsigned int x = start; x < end; x++)
  {
   if(sprite_line_buf[x] & 0x0F)
   {
    if(!(target[x] & 0x0F) || (sprite_line_buf[x] & 0x200))
     target[x] = sprite_line_buf[x] & 0x1FF;
   }
  }
 }
 active_sprites = 0;
}

void VDC::DoWaitStates(void)
{
 //bool did_wait = VDC_IS_BSY;

 while(VDC_IS_BSY)
 {
  int32 to_wait = CalcNextEvent();

  if(!WSHook || !WSHook(to_wait))
  {
   if(DMARunning)
   {
    VDC_WARNING("VRAM DMA completion forced.");
    RunDMA(0, TRUE);
   }

   if(sat_dma_counter > 0)
   {
    VDC_WARNING("SAT DMA completion forced.");
    RunSATDMA(0, TRUE);
   }

   if(mystery_phase)
   {
    bool backup_mystery_phase = mystery_phase;
    mystery_phase = false;
    CheckAndCommitPending();
    mystery_phase = backup_mystery_phase;
   }

   break;
  }
 }

 //if(did_wait)
 // printf("End of wait stating: %d %d\n", VDMA_CycleCounter, sat_dma_counter);

 assert(!pending_read);
 assert(!pending_write);
}

uint8 VDC::Read(uint32 A, int32 &next_event, bool peek)
{
 uint8 ret = 0;
 int msb = A & 1;

 A &= 0x3;

 switch(A)
 {
  case 0x0: ret = status | (VDC_IS_BSY ? 0x40 : 0x00);

            if(!peek)
            {
             status &= ~0x3F;
             IRQHook(FALSE);
            }
            break;

  case 0x2:
  case 0x3:
	   if(!peek)
	   {
	    // Should we only wait on MSB reads...
	    DoWaitStates(); 
	   }

           ret = VDC_REGGETP(read_buffer, msb);

           if(select == 0x2) // VRR - VRAM Read Register
           {
            if(msb)
            {
             if(!peek)
             {
	      pending_read = TRUE;
	      pending_read_addr = MARR;
	      MARR += vram_inc_tab[(CR >> 11) & 0x3];

	      CheckAndCommitPending();
             }
            }
       	   }
           break;
 }

 return(ret);
}

uint16 VDC::Read16(bool A, bool peek)
{
 uint16 ret = 0;

 if(!A)
 {
  ret = status | (VDC_IS_BSY ? 0x40 : 0x00);

  if(!peek)
  {
   status &= ~0x3F;
   IRQHook(FALSE);
  }
 }
 else
 {
  if(!peek)
   DoWaitStates(); 

  ret = read_buffer;

  if(select == 0x2) // VRR - VRAM Read Register
  {
   if(!peek)
   {
    pending_read = TRUE;
    pending_read_addr = MARR;
    MARR += vram_inc_tab[(CR >> 11) & 0x3];

    CheckAndCommitPending();
   }
  }
 }

 return(ret);
}


void VDC::CheckAndCommitPending(void)
{
 if(sat_dma_counter <= 0 && !DMARunning /* && sprite_cg_fetch_counter <= 0*/ && !mystery_phase)
 {
  if(pending_write)
  {
   if(pending_write_addr < VRAM_Size)
   {
    VRAM[pending_write_addr] = pending_write_latch;
    FixTileCache(pending_write_addr);
   }
   //else
   // VDC_UNDEFINED("Unmapped VRAM write");

   pending_write = FALSE;
  }

  if(pending_read)
  {
   if(pending_read_addr >= VRAM_Size)
    VDC_UNDEFINED("Unmapped VRAM VRR read");

   read_buffer = VRAM[pending_read_addr];
   pending_read = FALSE;
  }
 }
}


void VDC::Write(uint32 A, uint8 V, int32 &next_event)
{
 int msb = A & 1;

 A &= 0x3;

 //if((A == 0x2 || A == 0x3) && (select >= 0xF && select <= 0x12))
 //if((A == 2 || A == 3) && select != 2)
 // printf("VDC Write(RCRCount=%d): A=%02x, Select=%02x, V=%02x\n", RCRCount, A, select, V);

 switch(A)
 {
  case 0x0: select = V & 0x1F;
	    break;

  case 0x2:
  case 0x3:
	   //if((select & 0x1F) >= 0x9 && (select & 0x1F) <= 0x1F)
	   //	VDC_DEBUG("%02x %d, %02x", select & 0x1F, msb, V);

           switch(select & 0x1F)
           {
            case 0x00: VDC_REGSETP(MAWR, V, msb);
		       break;

            case 0x01: VDC_REGSETP(MARR, V, msb);
       	               if(msb)
                       {
			DoWaitStates();

			pending_read = TRUE;
			pending_read_addr = MARR;
	                MARR += vram_inc_tab[(CR >> 11) & 0x3];

			CheckAndCommitPending();
                       }
                       break;

            case 0x02: if(!msb) 
		       {
			write_latch = V;
		       }
                       else
                       {
			// We must call CommitPendingWrite at the end of SAT/VRAM DMA for this to work!
			DoWaitStates();

			pending_write = TRUE;
			pending_write_addr = MAWR;
			pending_write_latch = write_latch | (V << 8);
	                MAWR += vram_inc_tab[(CR >> 11) & 0x3];

			CheckAndCommitPending();
                       }
                       break;

            case 0x05: VDC_REGSETP(CR, V, msb);
			//printf("CR: %04x, %d\n", CR, msb);
                       break;

            case 0x06: VDC_REGSETP(RCR, V, msb);
		       RCR &= 0x3FF;
		       break;

            case 0x07: VDC_REGSETP(BXR, V, msb);
                       BXR &= 0x3FF;
                       //VDC_DEBUG("BXR Set");
                       break;

            case 0x08: VDC_REGSETP(BYR, V, msb);
		       BYR &= 0x1FF;
                       BG_YMoo = BYR; // Set it on LSB and MSB writes(only changing on MSB breaks Youkai Douchuuki)
                       //VDC_DEBUG("BYR Set");
                       break;

            case 0x09: VDC_REGSETP(MWR, V, msb); break;
       	    case 0x0a: VDC_REGSETP(HSR, V, msb); break;
            case 0x0b: VDC_REGSETP(HDR, V, msb); break;
       	    case 0x0c: VDC_REGSETP(VSR, V, msb); break;
            case 0x0d: VDC_REGSETP(VDR, V, msb); break;
       	    case 0x0e: VDC_REGSETP(VCR, V, msb); break;
            case 0x0f: VDC_REGSETP(DCR, V, msb);
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set DCR during DMA: %04x\n", DCR);
                       }

                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set DCR while DMAPending: %04x\n", DCR);
                       }

		       break;

            case 0x10: VDC_REGSETP(SOUR, V, msb); 
		       if(DMARunning)
		       {
		        VDC_UNDEFINED("Set SOUR during DMA: %04x\n", SOUR);
		       }

                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set SOUR while DMAPending: %04x\n", SOUR);
                       }
		       break;

            case 0x11: VDC_REGSETP(DESR, V, msb);
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set DESR during DMA: %04x\n", DESR);
                       }
                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set DESR while DMAPending: %04x\n", DESR);
                       }
		       break;

            case 0x12: VDC_REGSETP(LENR, V, msb);
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set LENR during DMA: %04x\n", LENR);
                       }

                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set LENR while DMAPending: %04x\n", LENR);
                       }

                       if(msb)
       	               {
                        VDC_DEBUG("DMA: %04x %04x %04x, %02x", SOUR, DESR, LENR, DCR);
			DMAPending = 1;
                       }
                       break;

            case 0x13: VDC_REGSETP(DVSSR, V, msb);
		       SATBPending = 1;
		       break;

            default:   VDC_WARNING("Unknown VDC register write: %04x %02x", select, V);
		       break;
           }
           break;
 }
}


void VDC::Write16(bool A, uint16 V)
{
 if(!A)
  select = V & 0x1F;
 else
 {
  switch(select & 0x1F)
  {
            case 0x00: MAWR = V;
		       break;


            case 0x01: MARR = V;

		       DoWaitStates();

		       pending_read = TRUE;
		       pending_read_addr = MARR;

	               MARR += vram_inc_tab[(CR >> 11) & 0x3];

		       CheckAndCommitPending();
                       break;


            case 0x02: // We must call CommitPendingWrite at the end of SAT/VRAM DMA for this to work!
			DoWaitStates();

			pending_write = TRUE;
			pending_write_addr = MAWR;
			pending_write_latch = V;
	                MAWR += vram_inc_tab[(CR >> 11) & 0x3];

			CheckAndCommitPending();
                       break;

            case 0x05: CR = V;
                       break;

            case 0x06: RCR = V & 0x3FF;
		       break;

            case 0x07: BXR = V & 0x3FF;
                       //VDC_DEBUG("BXR Set");
                       break;

            case 0x08: BYR = V & 0x1FF;
                       BG_YMoo = BYR;
                       //VDC_DEBUG("BYR Set");
                       break;

            case 0x09: MWR = V; break;
       	    case 0x0a: HSR = V; break;
            case 0x0b: HDR = V; break;
       	    case 0x0c: VSR = V; break;
            case 0x0d: VDR = V; break;
       	    case 0x0e: VCR = V; break;

            case 0x0f: DCR = V;
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set DCR during DMA: %04x\n", DCR);
                       }

                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set DCR while DMAPending: %04x\n", DCR);
                       }

                       break;

            case 0x10: SOUR = V;
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set SOUR during DMA: %04x\n", SOUR);
                       }

                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set SOUR while DMAPending: %04x\n", SOUR);
                       }
                       break;

            case 0x11: DESR = V;
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set DESR during DMA: %04x\n", DESR);
                       }
                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set DESR while DMAPending: %04x\n", DESR);
                       }
                       break;

            case 0x12: LENR = V;
                       if(DMARunning)
                       {
                        VDC_UNDEFINED("Set LENR during DMA: %04x\n", LENR);
                       }

                       if(DMAPending)
                       {
                        VDC_UNDEFINED("Set LENR while DMAPending: %04x\n", LENR);
                       }

                       VDC_DEBUG("DMA: %04x %04x %04x, %02x", SOUR, DESR, LENR, DCR);

		       DMAPending = 1;
                       break;

            case 0x13: DVSSR = V;
		       SATBPending = 1;
		       break;

            default:   VDC_WARNING("Oops 2: %04x %02x", select, V);
		       break;
  }
 }

}



int32 VDC::Reset(void)
{
 memset(VRAM, 0, sizeof(VRAM));
 memset(SAT, 0, sizeof(SAT));
 memset(SpriteList, 0, sizeof(SpriteList));

 for(uint32 A = 0; A < 65536; A += 16)
  FixTileCache(A);

 pending_read = false;
 pending_read_addr = 0xFFFF;
 read_buffer = 0xFFFF;
 write_latch = 0;

 pending_write = false;
 pending_write_addr = 0xFFFF;
 pending_write_latch = 0xFFFF;

 status = 0;

 HSR = 0;
 HDR = 0;
 VSR = 0;
 VDR = 0;
 VCR = 0;

 HSW_cache = M_vdc_HSW;
 HDS_cache = M_vdc_HDS;
 HDW_cache = M_vdc_HDW;
 HDE_cache = M_vdc_HDE;

 VDS_cache = M_vdc_VDS;
 VSW_cache = M_vdc_VSW;
 VDW_cache = M_vdc_VDW;
 VCR_cache = M_vdc_VCR;



 MAWR = 0;
 MARR = 0;

 CR = CR_cache = 0;
 RCR = 0;
 BXR = 0;
 BYR = 0;
 MWR = 0;
 MWR_cache = 0;

 DCR = 0;
 SOUR = 0;
 DESR = 0;
 LENR = 0;
 DVSSR = 0;

 VDMA_CycleCounter = 0;

 RCRCount = 0;

 DMAReadBuffer = 0;
 DMAReadWrite = 0;
 DMARunning = 0;
 DMAPending = 0;
 SATBPending = 0;
 burst_mode = 0;

 BG_XOffset = 0;
 BG_YOffset = 0;
 BG_YMoo = 0;

 sat_dma_counter = 0;
 select = 0;

 pixel_copy_count = 0;


 NeedRCRInc = false;
 NeedVBIRQTest = false;
 NeedSATDMATest = false;
 NeedBGYInc = false;

 HPhase = 0;
 VPhase = 0;
 HPhaseCounter = 1;
 VPhaseCounter = 1;

 sprite_cg_fetch_counter = 0;

 mystery_counter = 0;
 mystery_phase = false;

 pixel_desu = 0;
 pixel_copy_count = 0;
 active_sprites = 0;

 return(CalcNextEvent());
}

VDC::VDC(bool nospritelimit, uint32 par_VRAM_Size)
{
 unlimited_sprites = nospritelimit; //MDFN_GetSettingB("pce.nospritelimit");
 userle = ~0;

 assert(par_VRAM_Size == round_up_pow2(par_VRAM_Size));
 assert(par_VRAM_Size >= 16 && par_VRAM_Size <= 65536);

 VRAM_Size = par_VRAM_Size;
 VRAM_SizeMask = VRAM_Size - 1;
 VRAM_BGTileNoMask = VRAM_SizeMask / 16;

 WSHook = NULL;
 IRQHook = NULL;

 in_exhsync = false;
 in_exvsync = false;
}

VDC::~VDC()
{

}

void VDC::StateExtra(MDFN::LEPacker &sl_packer, bool load)
{
 sl_packer.set_read_mode(load);

 for(int i = 0; i < 64 * 2; i++)
 {
  sl_packer ^ SpriteList[i].x;
  sl_packer ^ SpriteList[i].flags;
  sl_packer ^ SpriteList[i].palette_index;

  for(int pd = 0; pd < 4; pd++)
   sl_packer ^ SpriteList[i].pattern_data[pd];
 }
}

int VDC::StateAction(StateMem *sm, int load, int data_only, const char *sname)
{
 int ret = 1;
 MDFN::LEPacker sl_packer;

 StateExtra(sl_packer, false);

 SFORMAT StateRegs[] = 
 {
	SFVAR(in_exhsync),
	SFVAR(in_exvsync),

        SFVARN(sat_dma_counter, "sat_dma_counter"),

        SFVARN(select, "select"),
        SFVARN(MAWR, "MAWR"),
        SFVARN(MARR, "MARR"),
        SFVARN(CR, "CR"),
	SFVAR(CR_cache),
        SFVARN(RCR, "RCR"),
        SFVARN(BXR, "BXR"),
        SFVARN(BYR, "BYR"),
        SFVARN(MWR, "MWR"),

        SFVARN(HSR, "HSR"),
        SFVARN(HDR, "HDR"),
        SFVARN(VSR, "VSR"),
        SFVARN(VDR, "VDR"),

        SFVARN(VCR, "VCR"),
        SFVARN(DCR, "DCR"),
        SFVARN(SOUR, "SOUR"),
        SFVARN(DESR, "DESR"),
        SFVARN(LENR, "LENR"),
        SFVARN(DVSSR, "SATB"),


	SFVAR(VDMA_CycleCounter),

        SFVARN(RCRCount, "RCRCount"),


	SFVAR(pending_read),
	SFVAR(pending_read_addr),
        SFVAR(read_buffer),

        SFVAR(write_latch),

	SFVAR(pending_write),
	SFVAR(pending_write_addr),
	SFVAR(pending_write_latch),

        SFVARN(status, "status"),

        SFARRAY16N(SAT, 0x100, "SAT"),

        SFARRAY16N(VRAM, VRAM_Size, "VRAM"),

        SFVARN(DMAReadBuffer, "DMAReadBuffer"),
        SFVARN(DMAReadWrite, "DMAReadWrite"),
        SFVARN(DMARunning, "DMARunning"),
	SFVAR(DMAPending),
        SFVARN(SATBPending, "SATBPending"),
        SFVARN(burst_mode, "burst_mode"),

        SFVARN(BG_YOffset, "BG_YOffset"),
        SFVARN(BG_XOffset, "BG_XOffset"),

	SFVAR(HSW_cache),
	SFVAR(HDS_cache),
	SFVAR(HDW_cache),
	SFVAR(HDE_cache),

	SFVARN(VDS_cache, "VDS_cache"),
        SFVARN(VSW_cache, "VSW_cache"),
        SFVARN(VDW_cache, "VDW_cache"),
        SFVARN(VCR_cache, "VCR_cache"),
	SFVARN(MWR_cache, "MWR_cache"),


	SFVAR(BG_YMoo),
	SFVAR(NeedRCRInc),
	SFVAR(NeedVBIRQTest),
	SFVAR(NeedSATDMATest),
	SFVAR(NeedBGYInc),

	SFVAR(HPhase),
	SFVAR(VPhase),
	SFVAR(HPhaseCounter),
	SFVAR(VPhaseCounter),

	SFVAR(sprite_cg_fetch_counter),

	SFVAR(mystery_counter),
	SFVAR(mystery_phase),

	SFVAR(active_sprites),

	SFARRAYN(&sl_packer[0], sl_packer.size(), "ExtraState"),
	//SFARRAY(SpriteListTemp, sizeof(SpriteListTemp)),

	SFEND
  };

  ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, sname);

  if(load)
  {
   StateExtra(sl_packer, true);

   for(int x = 0; x < VRAM_Size; x++)
    FixTileCache(x);
  }

 return(ret);
}

#ifdef WANT_DEBUGGER
bool VDC::DoGfxDecode(uint32 *target, const uint32 *color_table, const uint32 TransparentColor, bool DecodeSprites, 
	int32 w, int32 h, int32 scroll)
{
 const uint32 *palette_ptr = color_table;

 if(DecodeSprites)
 {
  for(int y = 0; y < h; y++)
  {
   for(int x = 0; x < w; x += 16)
   {
    int which_tile = (x / 16) + (scroll + (y / 16)) * (w / 16);

    if(which_tile >= VRAM_Size / 64)
    {
     for(int sx = 0; sx < 16; sx++)
     {
      target[x + sx] = TransparentColor;
      target[x + w * 1 + sx] = 0;
      target[x + w * 2 + sx] = 0;
     }
     continue;
    }

    uint16 cg[4];
    cg[0] = VRAM[which_tile * 64 + (y & 15)];
    cg[1] = VRAM[which_tile * 64 + (y & 15) + 16];
    cg[2] = VRAM[which_tile * 64 + (y & 15) + 32];
    cg[3] = VRAM[which_tile * 64 + (y & 15) + 48];
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

   if(which_tile >= (VRAM_Size / 16))
   {
    for(int sx = 0; sx < 8; sx++)
    {
     target[x + sx] = TransparentColor;
     target[x + w * 1 + sx] = 0;
     target[x + w * 2 + sx] = 0;
    }
    continue;
   }

   target[x + 0] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][0]];
   target[x + 1] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][1]];
   target[x + 2] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][2]];
   target[x + 3] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][3]];
   target[x + 4] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][4]];
   target[x + 5] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][5]];
   target[x + 6] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][6]];
   target[x + 7] = palette_ptr[ bg_tile_cache[which_tile][y & 0x7][7]];

   target[x + w*1 + 0]=target[x + w*1 + 1]=target[x + w*1 + 2]=target[x + w*1 + 3] =
   target[x + w*1 + 4]=target[x + w*1 + 5]=target[x + w*1 + 6]=target[x + w*1 + 7] = which_tile;

   target[x + w*2 + 0]=target[x + w*2 + 1]=target[x + w*2 + 2]=target[x + w*2 + 3] =
   target[x + w*2 + 4]=target[x + w*2 + 5]=target[x + w*2 + 6]=target[x + w*2 + 7] = which_tile * 16;
  }
  target += w * 3;
 }

 return(1);
}
#endif

