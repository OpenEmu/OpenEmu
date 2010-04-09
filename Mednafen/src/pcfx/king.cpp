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

/* Emulation for HuC6261(descendant of the VCE) and the HuC6272(KING) */

#include "pcfx.h"
#include "v810_cpu.h"
#include "vdc.h"
#include "king.h"
#include "../cdrom/scsicd.h"
#include "interrupt.h"
#include "rainbow.h"
#include "soundbox.h"
#include "input.h"
#include "timer.h"
#include "debug.h"
#include <trio/trio.h>

#include "../video.h"
#include "../clamp.h"

//#define KINGDBG(format, ...) printf("%lld - KING: (DB: %02x)" format, (long long)pcfx_timestamp_base + v810_timestamp, SCSICD_GetDB(), ## __VA_ARGS__)
#define KINGDBG(format, ...)

/*
 SCSI Questions(SCSI emulation needs a bit of work, but these questions need to be answered first):

  What happens when there is no more data to transfer during DMA
  and the status SCSI bus phase is entered(before the DMA count reaches 0)?

  Why is the "sequential DMA" bit needed?

  How does the register that somehow controls I/O, C/D, and MSG signals work?  How is it related
  to "bus phase mismatch"?

  Under what conditions are SCSI interrupts generated?

  Which SCSI registers return the values of the SCSI bus, and which return latched values(from previous writes or false DMA)?

  Is real DMA layered on top of false DMA?  Reading the developer documents, it looks that way.

  What triggers the setting of ACK during false DMA?  A timer?  Reading from the upper 16-bits of KING register 0x05?  The lower bits(in which case,
  the value would be latched..)?

  What is the timing for subchannel reading like on a real PC-FX?

 Other questions:

  Does the KRAM mode register setting have any effect on a real PC-FX(or FXGA?)?

*/

// 16 bit YUV format:  upper 8 bits Y, next 4 bits U, lower 4 bits V, transformed to 8-bit U and 8-bit V by adding 0 or 1, who knows.

typedef struct
{
 uint8 AR;

 //
 unsigned int layer_mask_cache[7]; // 0x00 if layer disabled, 0xFF if enabled.

 uint16 priority[2];	/* uint16 0:
			   	bit   3-0: Legacy VDC BG priority?
				bit   7-4: Legacy VDC SPR priority?
				bit  11-8: RAINBOW(MJPEG) priority
			   uint16 1:
				bit   3-0: KING BG0 priority
				bit   7-4: KING BG1 priority
				bit  11-8: KING BG2 priority
				bit 15-12: KING BG3 priority
			*/

 bool8 odd_field;
 bool8 in_hblank;


 uint16 picture_mode;
 uint32 raster_counter;

 uint16 palette_rw_offset; // Read/write offset
 uint16 palette_rw_latch;

 uint16 palette_offset[4]; // 
			   // BMG1 and BMG 0 in [1](BMG1 in upper 8 bits, BMG0 in lower), BMG2 and 3 in [2]
			   // RAINBOW in lower(?) 8 bits of [3]?

 uint16 palette_table[512]; // The YUV palette, woohoo!
 uint32 palette_table_cache[512 * 2]; // 24-bit YUV cache for SPEED(HAH), * 2 to remove need for & 0x1FF in rendering code

 // Chroma keys, minimum value in lower 8 bits, maximum value in upper 8 bits
 uint16 ChromaKeyY; // Register 0xA
 uint16 ChromaKeyU; // Register 0xB
 uint16 ChromaKeyV; // register 0xC

 uint16 CCR; // Register 0x0D, fixed color register, 16-bit YUV
 uint16 BLE; // Register 0x0E, cellophane setting register
	     //  15   14   13   12   11   10   09   08   07   06   05   04   03   02   01   00
	     // F/B |E/D |   IDU   |CTRL BMG3|CTRL BMG2|CTRL BMG1|CTRL BMG0|  VDP SP | VDP BG
	     // IDU is AKA Rainbow
	     // F/B
	     //	 0 = Back cellophane.
	     //  1 = Front cellophane.
	     // E/D
	     //  0 = Back/Front cellophane off
	     //  1 = Back/Front cellophane on
	     // Ctrl format:
	     //  00 = Cellophane disabled.
	     //  01 = Cellophane with 1A/1B(Coeffs 0, 1?)
	     //  02 = Cellophane with 2A/2B(Coeffs 2, 3?)
	     //  03 = Cellophane with 3A/3B(Coeffs 4, 5?)

 uint16 SPBL; // Register 0x0F, Sprite cellophane setting register
	      // Specifies a color palette bank for which cellophane will always be disabled.

 uint16 coefficients[6]; // Cellophane coefficients, YUV, 4-bits each, on the least-significant end(xxxxYYYYUUUUVVVV).
			 // Valid settings: 0(cellophane disabled for layer), 1-8.  9-F are "unsupported".

 uint8 coefficient_mul_table_y[16][256];
 int8 coefficient_mul_table_uv[16][256];
} fx_vce_t;

fx_vce_t fx_vce;

static uint32 priority_0_dummybuf[512];

static uint8 happymap[17][17][17][3];

static void BuildCMT(void)
{
 for(int coeff = 0; coeff < 16; coeff++)
 {
  for(int value = 0; value < 256; value++)
  {
   fx_vce.coefficient_mul_table_y[coeff][value] = (value * coeff / 8); // Y
   fx_vce.coefficient_mul_table_uv[coeff][value] = ((value - 128) * coeff / 8); // UV
  }
 }

}

static INLINE void RebuildLayerMaskCache(void)
{
 for(int x = 0; x < 4; x++)
  fx_vce.layer_mask_cache[x] = ((fx_vce.picture_mode >> (10 + x)) & 1) ? 0xFF : 0x00;
 
 fx_vce.layer_mask_cache[4] = (fx_vce.picture_mode & 0x0100) ? 0xFF : 0x00; // VDC BG
 fx_vce.layer_mask_cache[5] = (fx_vce.picture_mode & 0x0200) ? 0xFF : 0x00; // VDC SPR
 fx_vce.layer_mask_cache[6] = (fx_vce.picture_mode & 0x4000) ? 0xFF : 0x00; // Rainbow
}

static ALWAYS_INLINE void RedoPaletteCache(int n)
{
 uint32 YUV = fx_vce.palette_table[n];
 uint8 Y = (YUV >> 8) & 0xFF;
 uint8 U = (YUV & 0xF0);
 uint8 V = (YUV & 0x0F) << 4;

 fx_vce.palette_table_cache[n] = 
 fx_vce.palette_table_cache[0x200 | n] = (Y << 16) | (U << 8) | (V << 0);
}

enum
{
 BGMODE_INVALID = 0,
 BGMODE_4 = 1,
 BGMODE_16 = 2,
 BGMODE_256 = 3,
 BGMODE_64K = 4,
 BGMODE_16M = 5,
 BGMODE_64K_EXTDOT = 6,
 BGMODE_16M_EXTDOT = 7,
};

typedef struct
{
	uint8 AR;

	uint16 KRAM[2][262144];
	uint32 KRAMRA, KRAMWA;
	uint8 KRAM_Mode;

	// Convenience masks, calculated from KRAM_Mode
	uint32 KRAM_Mask_Full;	// Either 0x3FFFF or 0x27FFF
	uint32 KRAM_Mask_Sub;	// Either 0x1FFFF or 0x07FFF

	uint32 PageSetting;

	uint16 bgmode; // 4 bits each BG: 3333 2222 1111 0000
		       /* Possible settings:
				0x0:  Invalid?
				0x1:  4-color palette, 1 byte for 4 pixels, transparent on entry 0
				0x2:  16-color palette, 1 byte for 2 pixels, transparent on entry 0
				0x3:  256-color palette, 1 byte for 1 pixel, transparent on entry 0
				0x4:  64K color(Y-8, U-4, V-4), 1 halfword for 1 pixel, transparent on Y=0
				0x5:  16M colors(Y-8, Y-8, U-8, V-8, 4 bytes for 2 pixels), transparent on Y=0

				If & 8, enable palette bank mode(only for 4 and 16-colors)???
					BAT format would be PPPPCCCCCCCCCCCC in this mode.
					4 color: 00PPPPnn 16 color: PPPPnnnn, where "n" is the 2 or 4-bit pixel data
			*/
	uint16 priority;

	uint16 BGScrollMode;	// Register 0x16
	uint16 BGSize[4];
	uint16 BGBATAddr[4];
	uint16 BGCGAddr[4];
	uint16 BGXScroll[4];
	uint16 BGYScroll[4];
	
        uint16 BGXScrollCache[4];
        uint16 BGYScrollCache[4];


	uint16 BG0SubBATAddr, BG0SubCGAddr;

	uint16 BGAffinA, BGAffinB, BGAffinC, BGAffinD;
	uint16 BGAffinCenterX, BGAffinCenterY;

	uint16 ADPCMControl;
	uint16 ADPCMBufferMode[2];

	uint16 ADPCMSAL[2];

	uint32 ADPCMEndAddress[2];
	uint32 ADPCMPlayAddress[2];
	uint16 ADPCMIntermediateAddress[2];
	uint16 ADPCMStatus[2]; // Register 0x53, a bit maimed :)


	uint16 RAINBOWTransferControl; // Register 0x40
	uint32 RAINBOWKRAMA;	       // Register 0x41
        uint16 RAINBOWTransferStartPosition; // Register 0x42, line number(0-262)
	uint16 RAINBOWTBC;
	uint32 RAINBOWRasterCounter;
	uint16 RAINBOWTransferBlockCount; // Register 0x43

	uint16 RasterIRQLine; // Register 0x44
	bool8 RasterIRQPending;

	uint32 RAINBOWKRAMReadPos;

	bool8 DMATransferFlipFlop;
	uint32 DMATransferAddr; // Register 0x09
	uint32 DMATransferSize; // Register 0x0A
	uint16 DMAStatus;	// Register 0x0B
	uint8 DMALatch;


	uint16 MPROGControl;    // register 0x15
	uint16 MPROGControlCache;
	uint16 MPROGAddress;
	uint16 MPROGData[0x10];

	bool8 DMAInterrupt;
	uint8 Reg00;
	uint8 Reg01;
	uint8 Reg02;
	uint8 Reg03;


	uint8 SubChannelControl;

	bool8 CDInterrupt, SubChannelInterrupt;
	uint8 SubChannelBuf;
	uint8 data_cache;

	bool8 DRQ;
	bool8 dma_receive_active;
	bool8 dma_send_active;
	int32 dma_cycle_counter;
	int32 lastts;

	#define KING_MAGIC_INTERVAL 10 //4 //32 //10

} king_t;

static king_t *king = NULL;

static INLINE void RecalcKRAM_Mask(void)
{
 king->KRAM_Mask_Full = (king->KRAM_Mode || 1) ? 0x3FFFF : 0x27FFF;
 king->KRAM_Mask_Sub = (king->KRAM_Mode || 1) ? 0x1FFFF : 0x07FFF;
}

static uint8 BGLayerDisable;
static bool RAINBOWLayerDisable;

static void RedoKINGIRQCheck(void);

static ALWAYS_INLINE void REGSETP(uint16 &reg, const uint8 data, const bool msb)
{
 reg &= 0xFF << (msb ? 0 : 8);
 reg |= data << (msb ? 8 : 0);
}

#ifdef WANT_DEBUGGER
static bool KRAMReadBPE = FALSE;
static bool KRAMWriteBPE = FALSE;

void KING_NotifyOfBPE(bool read, bool write)
{
 KRAMReadBPE = read;
 KRAMWriteBPE = write;

 FXVDC_SetAux0BPBpase(fx_vdc_chips[0], (read || write) ? 0x80000 : ~0);
 FXVDC_SetAux0BPBpase(fx_vdc_chips[1], (read || write) ? 0x90000 : ~0);
}

static void (*KINGLog)(const char *, const char *, ...) = NULL;
void KING_SetLogFunc(void (*logfunc)(const char *, const char *, ...))
{
 KINGLog = logfunc;
}

static uint32 *GfxDecode_Buf = NULL;
static int GfxDecode_Width = 0;
static int GfxDecode_Height = 0;
static int GfxDecode_Line = -1;
static int GfxDecode_Layer = 0;
static int GfxDecode_Scroll = 0;
static int GfxDecode_PBN = 0;
static void DoGfxDecode(void);

uint8 KING_MemPeek(uint32 A)
{
 uint8 ret = king->KRAM[(A & 0x80000) ? 1 : 0][A >> 1] >> ((A & 1) * 8);

 return(ret);
}

static void KING_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 int which;

 if(!strcmp(name, "kram0") || !strcmp(name, "kram1"))
 {
  int wk = name[4] - '0';

  while(Length--)
  {
   *Buffer = king->KRAM[wk][(Address >> 1) & 0x3ffff] >> ((Address & 1) * 8);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "vce"))
 {
  while(Length--)
  {
   Address &= 0x3FF;
   *Buffer = fx_vce.palette_table[Address >> 1] >> ((Address & 1) * 8);
   Address++;
   Buffer++;
  }
 }
 else if(trio_sscanf(name, "vdcvram%d", &which) == 1)
 {
  FXVDC_GetAddressSpaceBytes(fx_vdc_chips[which], "vram", Address, Length, Buffer);
 }

}

static void KING_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 int which;

 if(!strcmp(name, "kram0") || !strcmp(name, "kram1"))
 {
  int wk = name[4] - '0';

  while(Length--)
  {
   REGSETP(king->KRAM[wk][(Address >> 1) & 0x3ffff], *Buffer, Address & 1);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "vce"))
 {
  while(Length--)
  {
   Address &= 0x3FF;
   REGSETP(fx_vce.palette_table[Address >> 1], *Buffer, Address & 1);
   RedoPaletteCache(Address >> 1);
   Address++;
   Buffer++;
  }
 }
 else if(trio_sscanf(name, "vdcvram%d", &which) == 1)
 {
  FXVDC_PutAddressSpaceBytes(fx_vdc_chips[which], "vram", Address, Length, Granularity, hl, Buffer);
 }

}
#endif

uint8 KING_RB_Fetch(void)
{
 int page = (king->PageSetting & 0x1000) ? 1 : 0;
 uint8 ret = king->KRAM[page][(king->RAINBOWKRAMReadPos >> 1) & king->KRAM_Mask_Full] >> ((king->RAINBOWKRAMReadPos & 1) * 8);

 king->RAINBOWKRAMReadPos = ((king->RAINBOWKRAMReadPos + 1) & 0x3FFFF) | (king->RAINBOWKRAMReadPos & 0x40000);
 return(ret);
}

static void DoRealDMA(uint8 db)
{
 if(!king->DMATransferFlipFlop)
  king->DMALatch = db;
 else
 {
  king->KRAM[king->PageSetting & 1][king->DMATransferAddr & king->KRAM_Mask_Full] = king->DMALatch | (db << 8);
  king->DMATransferAddr = ((king->DMATransferAddr + 1) & 0x1FFFF) | (king->DMATransferAddr & 0x20000);
  king->DMATransferSize = (king->DMATransferSize - 2) & 0x3FFFF;
  if(!king->DMATransferSize)
  {
   KINGDBG("DMA Done\n");
   king->DMAInterrupt = TRUE;
   RedoKINGIRQCheck();
   king->DMAStatus &= ~1;
   return;
  }
 }
 king->DMATransferFlipFlop ^= 1;
}

uint16 FXVCE_Read16(uint32 A)
{
  // bit  4-0: Register number
  // bit 13-5: Raster counter
  // bit 14: In interlace mode and when on odd fields set bit.
  // bit 15: "0" during blanking(h or v), "1" during active screen area
  //	Raster counter increments a few cycles after the start of hblank.

 if(!(A & 0x4))
 {	
	uint16 fullret = 0;

	fullret |= fx_vce.AR;
	fullret |= fx_vce.odd_field ? 0x4000 : 0x0000;
        fullret |= fx_vce.raster_counter << 5;

	if(fx_vce.in_hblank || fx_vce.raster_counter < 22 || fx_vce.raster_counter == 262) 
	 fullret |= 0 << 15; // Clear on blanking
	else
	 fullret |= 1 << 15; // Set on active display.

	return(fullret);
 }
 else
 {
  switch(fx_vce.AR) // No idea which registers are readable, so make them all readable :b
  {
		default:  break;
	        case 0x00: return(fx_vce.picture_mode);
		case 0x01: return(fx_vce.palette_rw_offset);
		case 0x03: // Boundary Gate reads from 0x03 expecting palette data...
	        case 0x02: 
			   {
				uint16 ret = fx_vce.palette_rw_latch;
				fx_vce.palette_rw_offset = (fx_vce.palette_rw_offset + 1) & 0x1FF;
				fx_vce.palette_rw_latch = fx_vce.palette_table[fx_vce.palette_rw_offset];
				return(ret);
			   }
	        case 0x04: return(fx_vce.palette_offset[0]);
		case 0x05: return(fx_vce.palette_offset[1]);
		case 0x06: return(fx_vce.palette_offset[2]);
		case 0x07: return(fx_vce.palette_offset[3]);
		case 0x08: return(fx_vce.priority[0]);
		case 0x09: return(fx_vce.priority[1]);
                case 0x0a: return(fx_vce.ChromaKeyY);
                case 0x0b: return(fx_vce.ChromaKeyU);
                case 0x0c: return(fx_vce.ChromaKeyV);

                case 0x0d: return(fx_vce.CCR);
                case 0x0e: return(fx_vce.BLE);
                case 0x0f: return(fx_vce.SPBL);
                case 0x10: return(fx_vce.coefficients[0]);
                case 0x11: return(fx_vce.coefficients[1]);

                case 0x12: return(fx_vce.coefficients[2]);
                case 0x13: return(fx_vce.coefficients[3]);

                case 0x14: return(fx_vce.coefficients[4]);
                case 0x15: return(fx_vce.coefficients[5]);
  }
 }

 return(0);
}

void FXVCE_Write16(uint32 A, uint16 V)
{
 if(!(A & 0x4))
 {
  fx_vce.AR = V & 0x1F;
 }
 else
 {
  switch(fx_vce.AR)
  {
		case 0x00: fx_vce.picture_mode = V;
			   RebuildLayerMaskCache();
			   FXVDC_SetDotClock(fx_vdc_chips[0], (fx_vce.picture_mode & 0x08) ? 1 : 0);
			   FXVDC_SetDotClock(fx_vdc_chips[1], (fx_vce.picture_mode & 0x08) ? 1 : 0);
			   break;

		case 0x01: fx_vce.palette_rw_offset = V & 0x1FF;
			   fx_vce.palette_rw_latch = fx_vce.palette_table[fx_vce.palette_rw_offset];
			   break;

		case 0x02: fx_vce.palette_rw_latch = V;
			   fx_vce.palette_table[fx_vce.palette_rw_offset] = fx_vce.palette_rw_latch;
			   RedoPaletteCache(fx_vce.palette_rw_offset);
			   fx_vce.palette_rw_offset = (fx_vce.palette_rw_offset + 1) & 0x1FF;
			   break;

		case 0x04: fx_vce.palette_offset[0] = V; break;
		case 0x05: fx_vce.palette_offset[1] = V; break;
		case 0x06: fx_vce.palette_offset[2] = V; break;
		case 0x07: fx_vce.palette_offset[3] = V & 0x00FF; break;
		case 0x08: fx_vce.priority[0] = V & 0x0777; break;
		case 0x09: fx_vce.priority[1] = V & 0x7777; break;

		case 0x0a: fx_vce.ChromaKeyY = V; break;
                case 0x0b: fx_vce.ChromaKeyU = V; break;
                case 0x0c: fx_vce.ChromaKeyV = V; break;

		case 0x0d: fx_vce.CCR = V; break;
		case 0x0e: fx_vce.BLE = V; break;
		case 0x0f: fx_vce.SPBL = V; break;

		case 0x10: fx_vce.coefficients[0] = V & 0xFFF; break;
                case 0x11: fx_vce.coefficients[1] = V & 0xFFF; break;

                case 0x12: fx_vce.coefficients[2] = V & 0xFFF; break;
                case 0x13: fx_vce.coefficients[3] = V & 0xFFF; break;

                case 0x14: fx_vce.coefficients[4] = V & 0xFFF; break;
                case 0x15: fx_vce.coefficients[5] = V & 0xFFF; break;
  }
 }
}

static void RedoKINGIRQCheck(void)
{
 bool asserted = 0;

 for(int ch = 0; ch < 2; ch++)
  if((king->ADPCMBufferMode[ch] >> 1) & 0x3 & king->ADPCMStatus[ch]) // Trigger end/intermediate IRQ
  {
   //puts("IRQd!");
   asserted = 1;
  }

 if(king->DMAInterrupt && (king->DMAStatus & 0x2))
 {
  asserted = 1;
 }

 if(king->CDInterrupt)
 {
  asserted = 1;
 }

 if(king->SubChannelInterrupt)
  asserted = 1;

 if(king->RasterIRQPending)
  asserted = 1;

 PCFXIRQ_Assert(13, asserted);
}

void KING_CDIRQ(int type)
{
 if(type == SCSICD_IRQ_MAGICAL_REQ)
 {
  if(king->Reg02 & 0x2)
  {
   if(SCSICD_GetIO() != ((king->Reg03 >> 0) & 1) ||
      SCSICD_GetCD() != ((king->Reg03 >> 1) & 1) ||
      SCSICD_GetMSG() != ((king->Reg03 >> 2) & 1))
   {
     KINGDBG("Phase mismatch interrupt asserted.\n");
     king->CDInterrupt = TRUE;
     RedoKINGIRQCheck();
   }
  }
 }
}

void KING_StuffSubchannels(uint8 subchannels, int subindex)
{
 if(king->SubChannelControl & 0x1)
 {
  if(subindex == -2)
   king->SubChannelBuf = 0x00;
  else if(subindex == -1)
   king->SubChannelBuf = 0x80;
  else
   king->SubChannelBuf = (subchannels & 0x7F);

  if(king->SubChannelControl & 0x2)
  {
   king->SubChannelInterrupt = TRUE;
   RedoKINGIRQCheck();
  }
 }


}

uint8 KING_Read8(uint32 A)
{
 uint8 ret = KING_Read16(A & ~1) >> ((A & 1) * 8);

 //printf("Read8: %04x\n", A);
 return(ret);
}

void KING_ResetTS(void)
{
 if(king->dma_receive_active || king->dma_send_active)
  KING_DoMagic();

 king->lastts = 0;
}

static ALWAYS_INLINE void StartKingMagic(void)
{
 king->lastts = v810_timestamp;
 king->dma_cycle_counter = KING_MAGIC_INTERVAL;
 v810_setevent(V810_EVENT_KING, KING_MAGIC_INTERVAL);
}

static ALWAYS_INLINE void StopKingMagic(void)
{
 v810_setevent(V810_EVENT_KING, V810_EVENT_NONONO);
}

void KING_DoMagic(void)
{
 int32 cycles_elapsed;

 cycles_elapsed = v810_timestamp - king->lastts;
 king->lastts = v810_timestamp;

 king->dma_cycle_counter -= cycles_elapsed;

 //printf("%d\n", cycles_elapsed);
 while(king->dma_cycle_counter <= 0)
 {
  king->dma_cycle_counter += KING_MAGIC_INTERVAL;

  if(king->dma_receive_active)
  {
   if(!SCSICD_GetCD() && SCSICD_GetIO())
   {    
    if(SCSICD_GetREQ() && !SCSICD_GetACK())
    {
     if(!king->DRQ)
     {
      king->DRQ = TRUE;
      king->data_cache = SCSICD_GetDB();
      //SCSICD_SetACK(TRUE);
      //v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));

      if(king->DMAStatus & 0x1)
      {
       king->DRQ = FALSE;
       DoRealDMA(king->data_cache);
       SCSICD_SetACK(TRUE);
       v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
      }
     }
    }
    else if(SCSICD_GetACK() && !SCSICD_GetREQ())
    {
     SCSICD_SetACK(FALSE);
     v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
    }
   }
  }
  else if(king->dma_send_active)
  {
   if(!SCSICD_GetIO())
   {
    if(SCSICD_GetREQ() && !SCSICD_GetACK())
    {
     if(!king->DRQ)
     {
      //KINGDBG("Did write: %02x\n", king->data_cache);
      SCSICD_SetDB(king->data_cache);
      SCSICD_SetACK(TRUE);
      v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
      king->DRQ = TRUE;
     }
    }
    else if(SCSICD_GetACK() && !SCSICD_GetREQ())
    {
     SCSICD_SetACK(FALSE);
     v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
    }
   }
  }
 }
 //printf("Post: %d %d\n", king->dma_cycle_counter, KING_MAGIC_INTERVAL);
 v810_setevent(V810_EVENT_KING, king->dma_cycle_counter);
}

uint16 KING_Read16(uint32 A)
{
 int msh = A & 2;

 //printf("KRead16: %08x, %d; %04x\n", A, v810_timestamp, king->AR);

 switch(A & 0x704)
 {
  case 0x600:   // ?CDSRP??  AAAAAAAA
		// C = 0x4000, SCSI interrupt
		// D = 0x2000, DMA IRQ
		// S = 0x1000, CD Subchannel IRQ?
		// R = 0x0800, Raster IRQ
		// P = 0x0400, ADPCM IRQ

	      if(!msh)
	      {
	       uint16 ret = king->AR;

	       if(king->ADPCMStatus[0] & 0x3 & (king->ADPCMBufferMode[0] >> 1))
		ret |= 0x400;

	       if(king->ADPCMStatus[1] & 0x3 & (king->ADPCMBufferMode[1] >> 1))
                ret |= 0x400;

	       if(king->SubChannelInterrupt) ret |= 0x1000;
	
	       // Gaaah, this is probably a hack...Anime Freak FX Vol 4 gets confused and crashes
	       // if both bits are set at once.
	       if(king->DMAInterrupt && (king->DMAStatus & 0x2)) ret |= 0x2000;
	       else if(king->CDInterrupt) ret |= 0x4000;

	       if(king->RasterIRQPending) ret |= 0x800;

		king->SubChannelInterrupt = FALSE;
               king->RasterIRQPending = FALSE;
               RedoKINGIRQCheck();


	       return(ret);
	      }
	      else
	      {
		uint16 ret = 0;

		ret |= SCSICD_GetSEL() ? 0x02: 0x00;
		ret |= SCSICD_GetIO() ? 0x04 : 0x00;
		ret |= SCSICD_GetCD() ? 0x08 : 0x00;
		ret |= SCSICD_GetMSG() ? 0x10 : 0x00;
		ret |= SCSICD_GetREQ() ? 0x20 : 0x00;
		ret |= SCSICD_GetBSY() ? 0x40 : 0x00;
		ret |= SCSICD_GetRST() ? 0x80 : 0x00;

		ret |= king->SubChannelBuf << 8;

		return(ret);
 	      }
	      break; // status...

  case 0x604: switch(king->AR)
	      {
		default: 
			KINGDBG("Unknown 16-bit register read: %02x\n", king->AR);
			break;
		case 0x00:
			return(SCSICD_GetDB());
		case 0x01:
			return(REGGETHW(king->Reg01, msh));
		case 0x02:
			return(REGGETHW(king->Reg02, msh));
		case 0x03:
			return(REGGETHW(king->Reg03, msh));
		case 0x04:
			if(!msh)
			{
	                 uint16 ret = 0;

          	         ret |= SCSICD_GetSEL() ? 0x02: 0x00;
                	 ret |= SCSICD_GetIO() ? 0x04 : 0x00;
	                 ret |= SCSICD_GetCD() ? 0x08 : 0x00;
	                 ret |= SCSICD_GetMSG() ? 0x10 : 0x00;
	                 ret |= SCSICD_GetREQ() ? 0x20 : 0x00;
	                 ret |= SCSICD_GetBSY() ? 0x40 : 0x00;
	                 ret |= SCSICD_GetRST() ? 0x80 : 0x00;

         	         return(ret);	
			}
			return(0x00);

		case 0x05:
		        if(msh)
			{
			 uint8 ret = king->data_cache;
			 //printf("Fooball: %02x\n", ret);
			 if(king->dma_receive_active)
			 {
			  king->DRQ = FALSE;
     			  SCSICD_SetACK(TRUE);
     			  v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
			 }
			 return(ret);
			}
			else
			{
			 uint16 ret = 0;

			 ret |= SCSICD_GetACK() ? 0x01 : 0x00;
			 ret |= SCSICD_GetATN() ? 0x02 : 0x00;

			 if(king->dma_receive_active || king->dma_send_active)
			  if(king->DRQ)
 			   ret |= 0x40;
			
		         // Gaaah, this is probably a hack...Anime Freak FX Vol 4 gets confused and crashes
	                 // if both bits are set at once.
			 if(!king->DMAInterrupt)
			  ret |= king->CDInterrupt ? 0x10 : 0x00;

                         if(SCSICD_GetIO() == ((king->Reg03 >> 0) & 1) && 
			    SCSICD_GetCD() == ((king->Reg03 >> 1) & 1) && 
			    SCSICD_GetMSG() == ((king->Reg03 >> 2) & 1))
                         {
               	          ret |= 0x8;	// Phase match
                         }
			
			 return(ret);
			}
			break;

		case 0x06: // SCSI Input Data Register, same value returned as reading D16-D23 of register 0x05?
			KINGDBG("Input data for...?\n");
			return(king->data_cache);

		case 0x07: 
			// SCSI IRQ acknowledge/reset
			KINGDBG("SCSI IRQ acknowledge\n");
			king->CDInterrupt = FALSE;
                        RedoKINGIRQCheck();
			return(0xFF);

		case 0x08: 	// Sub-channel data
			{
			 uint8 ret = king->SubChannelBuf;
			 king->SubChannelBuf = 0;
			 //puts("Sub-channel data read.");
			 return(REGGETHW(ret, msh));
			}

                case 0x09: return(REGGETHW(king->DMATransferAddr, msh));
                case 0x0A: return(REGGETHW(king->DMATransferSize, msh));
		case 0x0B: // Value read in the BIOS always seems to be discarded...  DMA IRQ acknowledge?
			{
			 uint16 ret = 0;
			 if(!msh)
			 {
			  ret = king->DMAInterrupt ? 1 : 0;
                          KINGDBG("DMA IRQ Acknowledge: %d\n", ret);
                          king->DMAInterrupt = 0;
                          RedoKINGIRQCheck();
			 }
 	 	   	 return(ret);
			}
		case 0x0C: return(REGGETHW(king->KRAMRA, msh));
		case 0x0D: return(REGGETHW(king->KRAMWA, msh));
		case 0x0E:
                         {
                          unsigned int page = (king->KRAMRA & 0x80000000) ? 1 : 0;
                          int32 inc_amount = ((int32)((king->KRAMRA & (0x3FF << 18)) << 4)) >> 22; // Convert from 10-bit signed 2's complement 
                          uint16 ret = king->KRAM[page][king->KRAMRA & king->KRAM_Mask_Full];

	                  #ifdef WANT_DEBUGGER 
			  if(KRAMReadBPE) 
			   PCFXDBG_CheckBP(BPOINT_AUX_READ, (king->KRAMRA & 0x3FFFF) | (page ? 0x40000 : 0), 1);
			  #endif
	
                          king->KRAMRA = (king->KRAMRA &~ 0x1FFFF) | ((king->KRAMRA + inc_amount) & 0x1FFFF);
			  return(ret);
                          }
                          break;
		case 0x0F: return(king->PageSetting);
                case 0x10: return(REGGETHW(king->bgmode, msh));
		case 0x15: return(king->MPROGControl);

		case 0x40: return(0);	// Super Power League FX reads this, but I think it's write-only.

		case 0x53:
			  {
			   uint16 ret = king->ADPCMStatus[0] | (king->ADPCMStatus[1] << 2);
			   king->ADPCMStatus[0] = king->ADPCMStatus[1] = 0;
			   RedoKINGIRQCheck();
			   return(ret);
			  }
			  break;
	      }
	      break;
	      
 }

 return(0);
}

void KING_Write8(uint32 A, uint8 V)
{
 KING_Write16(A & 0x706, V << ((A & 1) ? 8 : 0));
}

static ALWAYS_INLINE void SCSI_Reg0_Write(uint8 V, bool delay_run = 0)
{
 king->Reg00 = V;
 SCSICD_SetDB(V);

 KINGDBG("WriteDB: %02x\n", V);

 if(!delay_run)
  v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
}

static ALWAYS_INLINE void SCSI_Reg2_Write(uint8 V, bool delay_run = 0)
{
 KINGDBG("SCSI Mode: %04x\n", V);

 /* SCSI Mode Register
                               D0 = SED:  When using with sequential DMA mode, you use.
                                          (It sets the normal DMA mode time to "0".)
                                        Sequential DMA, the number of transfer data bytes which are set to the SCSI device (m) HuC6272 (REG.A) the number
                                        of transfer bytes (n) it sets more largely, abbreviates the status message command phase after the n byte
                                        transferring and being something which makes the transfer after the n + 1 byte possible, it is possible to
                                        increase the transfer performance from the slow SCSI device of CD-ROM and the like.
                                        Sequential DMA cannot use with imitation DMA. When this bit is designated as 1, because -ACK the signal
                                        mandatorily ネゲート it is done, other than the stipulated sequence please do not use.

                               D1 = DMA Mode:
                                        When using the SCSI control section with normal DMA mode, "1" is set.
                           */
 if(!(V & 0x2) && (king->Reg02 & 0x2))
 {

  { // HACK(probably)
   king->CDInterrupt = FALSE;
   RedoKINGIRQCheck();
  }

  SCSICD_SetACK(0);

  if(!delay_run)
   v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));

  king->DRQ = FALSE;

  king->dma_receive_active = FALSE;
  king->dma_send_active = FALSE;
  StopKingMagic();
 }

 king->Reg02 = V;
}

static ALWAYS_INLINE void SCSI_Reg3_Write(uint8 V, bool delay_run = 0)
{
 KINGDBG("Set phase match SCSI bus bits: IO: %d, CD: %d, MSG: %d\n", (int)(bool)(V & 1), (int)(bool)(V & 2), (int)(bool)(V & 4));
 king->Reg03 = V & 0x7;

 if(!delay_run)
  v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
}

void KING_Write16(uint32 A, uint16 V)
{
 int msh = A & 0x2;

 if(!(A & 0x4))
 {
  if(!msh) 
   king->AR = V & 0x7F; 
 }
 else
 {
  //if(king->AR == 0x2 || king->AR == 0xB || king->AR == 0x3 || king->AR == 0x01 || king->AR == 0x00 || king->AR == 0x03 || king->AR == 0x05)
  //printf("KING: %02x %04x, %d\n", king->AR, V, fx_vce.raster_counter);

	      switch(king->AR)
	      {
		default: 
			KINGDBG("Unknown 16-bit register write: %02x %04x %d\n", king->AR, V, msh); 
			break;
		case 0x00: if(!msh) 
			   {
			    SCSI_Reg0_Write(V);
			   }
			   break;
		case 0x01: if(!msh)
			   {
			    if(V & 0x80)	// RST, silly KING, resets SCSI internal control registers too!
			    {
			     SCSI_Reg0_Write(0, TRUE);
			     SCSI_Reg2_Write(0, TRUE);
			     SCSI_Reg3_Write(0, TRUE);
			     king->Reg01 = V & 0x80; // Only this bit remains...how lonely.
			    }
			    else
			    {
			     king->Reg01 = V & (1 | 2 | 4 | 0x10 | 0x80);

			     KINGDBG("Set SCSI BUS bits; Assert DB: %d, ATN: %d, SEL: %d, ACK: %d, RST: %d, %02x\n", (int)(bool)(V & 1), (int)(bool)(V & 2), (int)(bool)(V & 4), (int)(bool)(V & 0x10), (int)(bool)(V &0x80), SCSICD_GetDB());

			     SCSICD_SetATN(V & 2);
			     SCSICD_SetSEL(V & 4);
			     SCSICD_SetACK(V & 0x10);
			    }
                            SCSICD_SetRST(V & 0x80);
			    v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp));
			   }
			   break;
		case 0x02:
			   if(!msh)
			   {
			    SCSI_Reg2_Write(V);
			   }
			   break;
		case 0x03: 
			   if(!msh)
			   {
			    SCSI_Reg3_Write(V);
			   }
			   break;

		case 0x05: 
			   if(!msh)	// Start DMA target receive
			   {
                            KINGDBG("DMA target receive: %04x, %d\n", V, msh);
                            king->dma_receive_active = FALSE;
                            king->dma_send_active = TRUE;
			    king->DRQ = TRUE;
			    StartKingMagic();
			   }
			   else
			   {
			    if(king->dma_send_active && king->DRQ)
			    {
			     //KINGDBG("%02x\n", V);
			     king->data_cache = V;
			     king->DRQ = FALSE;
			    }
			   }
			   break;

		case 0x06: break; // Not used for writes?

		case 0x07: KINGDBG("Start DMA initiator receive: %04x\n", V);

			   if(king->Reg02 & 0x2)
			   {
			    king->dma_receive_active = TRUE;
			    king->dma_send_active = FALSE;
			    StartKingMagic();
			   }
			   break;

		case 0x08: // Sub-channel control
			   KINGDBG("Sub-channel control: %02x\n", V);

			   king->SubChannelControl = V & 0x3;
			   king->SubChannelInterrupt = FALSE;
			   RedoKINGIRQCheck();
			   break;

                case 0x09: REGSETHW(king->DMATransferAddr, V, msh); king->DMATransferAddr &= 0x3FFFF; break;
                case 0x0A: REGSETHW(king->DMATransferSize, V, msh); king->DMATransferSize &= 0x3FFFE; king->DMATransferFlipFlop = 0; break;
		case 0x0B: REGSETHW(king->DMAStatus, V, msh);
			   king->DMAStatus &= 0x3;
			   king->DMAInterrupt = 0;
			   RedoKINGIRQCheck();

			   king->DMATransferFlipFlop = 0;

			   KINGDBG("SCSI DMA: %04x, dest=%06x, page=%d, size=%06x(16-bit words)\n", V, king->DMATransferAddr, king->PageSetting & 1, king->DMATransferSize >> 1);

			   #ifdef WANT_DEBUGGER
			   if(KINGLog)
			   {
			    //if(V & 1)
 			    // KINGLog("KING", "SCSI DMA: dest=%06x, page=%d, size=%06x(16-bit words)", king->DMATransferAddr, king->PageSetting & 1, king->DMATransferSize >> 1);
			   }
			   #endif
			   break;
                case 0x0C: REGSETHW(king->KRAMRA, V, msh); break;
                case 0x0D: REGSETHW(king->KRAMWA, V, msh); break;
		case 0x0E: 
			  {
			   unsigned int page = (king->KRAMWA & 0x80000000) ? 1 : 0;
			   int32 inc_amount = ((int32)((king->KRAMWA & (0x3FF << 18)) << 4)) >> 22; // Convert from 10-bit signed 2's complement

		           #ifdef WANT_DEBUGGER 
                           if(KRAMWriteBPE)
                            PCFXDBG_CheckBP(BPOINT_AUX_WRITE, (king->KRAMWA & 0x3FFFF) | (page ? 0x40000 : 0), 1);
			   #endif

			   king->KRAM[page][king->KRAMWA & king->KRAM_Mask_Full] = V;
			   king->KRAMWA = (king->KRAMWA &~ 0x1FFFF) | ((king->KRAMWA + inc_amount) & 0x1FFFF);
			  }
			  break;
		case 0x0F: REGSETHW(king->PageSetting, V, msh); break;
		case 0x10: REGSETHW(king->bgmode, V, msh); break;
		case 0x12: REGSETHW(king->priority, V, msh); break;

		case 0x13: REGSETHW(king->MPROGAddress, V, msh); king->MPROGAddress &= 0xF; break; // Microprogram address
		case 0x14: if(!msh)	// Microprogram data
			   {
			    king->MPROGData[king->MPROGAddress] = V;
			    king->MPROGAddress = (king->MPROGAddress + 1) & 0xF;
			   }
			   break;
		case 0x15: REGSETHW(king->MPROGControl, V, msh); king->MPROGControl &= 0x1; break;

		case 0x16: REGSETHW(king->BGScrollMode, V, msh); king->BGScrollMode &= 0xF; break;

		case 0x20: REGSETHW(king->BGBATAddr[0], V, msh); king->BGBATAddr[0] &= 0x1FF; break;
		case 0x21: REGSETHW(king->BGCGAddr[0], V, msh); king->BGCGAddr[0] &= 0x1FF; break;
		case 0x22: REGSETHW(king->BG0SubBATAddr, V, msh); king->BG0SubBATAddr &= 0x1FF; break;
	  	case 0x23: REGSETHW(king->BG0SubCGAddr, V, msh); king->BG0SubCGAddr &= 0x1FF; break;

		case 0x24: REGSETHW(king->BGBATAddr[1], V, msh); king->BGBATAddr[1] &= 0x1FF; break;
		case 0x25: REGSETHW(king->BGCGAddr[1], V, msh); king->BGCGAddr[1] &= 0x1FF; break;
		case 0x28: REGSETHW(king->BGBATAddr[2], V, msh); king->BGBATAddr[2] &= 0x1FF; break;
		case 0x29: REGSETHW(king->BGCGAddr[2], V, msh); king->BGCGAddr[2] &= 0x1FF; break;
		case 0x2A: REGSETHW(king->BGBATAddr[3], V, msh); king->BGBATAddr[3] &= 0x1FF; break;
		case 0x2B: REGSETHW(king->BGCGAddr[3], V, msh); king->BGCGAddr[3] &= 0x1FF; break;

		case 0x2C: REGSETHW(king->BGSize[0], V, msh); break;
		case 0x2D: REGSETHW(king->BGSize[1], V, msh); king->BGSize[1] &= 0x00FF; break;
		case 0x2E: REGSETHW(king->BGSize[2], V, msh); king->BGSize[2] &= 0x00FF; break;
		case 0x2F: REGSETHW(king->BGSize[3], V, msh); king->BGSize[3] &= 0x00FF; break;

		case 0x30: REGSETHW(king->BGXScroll[0], V, msh); king->BGXScroll[0] &= 0x7FF; break;
		case 0x31: REGSETHW(king->BGYScroll[0], V, msh); king->BGYScroll[0] &= 0x7FF; break;

		case 0x32: REGSETHW(king->BGXScroll[1], V, msh); king->BGXScroll[1] &= 0x3FF; break;
		case 0x33: REGSETHW(king->BGYScroll[1], V, msh); king->BGYScroll[1] &= 0x3FF; break;

		case 0x34: REGSETHW(king->BGXScroll[2], V, msh); king->BGXScroll[2] &= 0x3FF; break;
		case 0x35: REGSETHW(king->BGYScroll[2], V, msh); king->BGYScroll[2] &= 0x3FF; break;

		case 0x36: REGSETHW(king->BGXScroll[3], V, msh); king->BGXScroll[3] &= 0x3FF; break;
		case 0x37: REGSETHW(king->BGYScroll[3], V, msh); king->BGYScroll[3] &= 0x3FF; break;


		case 0x38: REGSETHW(king->BGAffinA, V, msh); break;
		case 0x39: REGSETHW(king->BGAffinB, V, msh); break;
		case 0x3a: REGSETHW(king->BGAffinC, V, msh); break;
		case 0x3b: REGSETHW(king->BGAffinD, V, msh); break;
		case 0x3c: REGSETHW(king->BGAffinCenterX, V, msh); break;
		case 0x3d: REGSETHW(king->BGAffinCenterY, V, msh); break;


		case 0x40: // ------IE
			   // I = 1, interrupt enable??
			   // E = 1, rainbow transfer start
			   if(!msh)
			   {
			    king->RAINBOWTransferControl = V & 0x3;
			   }
			   king->RasterIRQPending = FALSE;
			   RedoKINGIRQCheck();
			   break;
                case 0x41: REGSETHW(king->RAINBOWKRAMA, V, msh); king->RAINBOWKRAMA &= 0x3FFFF; break; // Rainbow transfer address
		case 0x42: if(!msh) king->RAINBOWTransferStartPosition = V & 0x1FF; break; // 0-262
		case 0x43: REGSETHW(king->RAINBOWTransferBlockCount, V, msh); king->RAINBOWTransferBlockCount &= 0x1F; break;
		case 0x44: if(!msh) king->RasterIRQLine = V & 0x1FF; break; // Raster IRQ line

		case 0x50: 
			   if(!msh)
			   {
			    for(int ch = 0; ch < 2; ch++)
			     if(!(king->ADPCMControl & (1 << ch)) && (V & (1 << ch)))
			     { 
			      king->ADPCMPlayAddress[ch] = king->ADPCMSAL[ch] * 256;
			      king->ADPCMStatus[ch] = 0;
			     }
			    king->ADPCMControl = V; 
			    RedoKINGIRQCheck();
			    SoundBox_SetKINGADPCMControl(king->ADPCMControl);
			   }
			   break;
		case 0x51: REGSETHW(king->ADPCMBufferMode[0], V, msh); RedoKINGIRQCheck(); break;
		case 0x52: REGSETHW(king->ADPCMBufferMode[1], V, msh); RedoKINGIRQCheck(); break;

		case 0x58: REGSETHW(king->ADPCMSAL[0], V, msh); king->ADPCMSAL[0] &= 0x3FF; break;
		case 0x59: REGSETHW(king->ADPCMEndAddress[0], V, msh); king->ADPCMEndAddress[0] &= 0x3FFFF; break;
		case 0x5A: REGSETHW(king->ADPCMIntermediateAddress[0], V, msh); king->ADPCMIntermediateAddress[0] &= 0xFFF; break;

		case 0x5C: REGSETHW(king->ADPCMSAL[1], V, msh); king->ADPCMSAL[1] &= 0x3FF; break;
		case 0x5D: REGSETHW(king->ADPCMEndAddress[1], V, msh); king->ADPCMEndAddress[1] &= 0x3FFFF; break;
		case 0x5E: REGSETHW(king->ADPCMIntermediateAddress[1], V, msh); king->ADPCMIntermediateAddress[1] &= 0xFFF; break;

 
		case 0x61: if(king->KRAM_Mode ^ V)
			   {
			    KINGDBG("KRAM Mode Change To: %02x\n", V & 1);
			    king->KRAM_Mode = V & 0x1; 
			    RecalcKRAM_Mask();
			   }
			   break;
	      }

 }
}

uint16 KING_GetADPCMHalfWord(int ch)
{
 int page = (king->PageSetting & 0x0100) ? 1 : 0;
 uint16 ret = king->KRAM[page][king->ADPCMPlayAddress[ch] & king->KRAM_Mask_Full];

 king->ADPCMPlayAddress[ch] = (king->ADPCMPlayAddress[ch] & 0x20000) | ((king->ADPCMPlayAddress[ch] + 1) & 0x1FFFF);

 if(king->ADPCMPlayAddress[ch] == (((king->ADPCMEndAddress[ch] + 1) & 0x1FFFF) | (king->ADPCMEndAddress[ch] & 0x20000)) )
 {
  if(!(king->ADPCMBufferMode[ch] & 1))
  {
   king->ADPCMControl &= ~(1 << ch);
   SoundBox_SetKINGADPCMControl(king->ADPCMControl);
  }
  else
  {
   king->ADPCMPlayAddress[ch] = king->ADPCMSAL[ch] << 8;
  }

  king->ADPCMStatus[ch] |= 1;
  RedoKINGIRQCheck();
 }
 else if(king->ADPCMPlayAddress[ch] == ((uint32)king->ADPCMIntermediateAddress[ch] << 6) )
 {
  king->ADPCMStatus[ch] |= 2;
  RedoKINGIRQCheck();
 }

 return(ret);
}

static uint32 TTRANS_LUT[256];
static const uint32 TRANS_OR = 0x1 << 24;
static const uint32 TRANS_OR_SHIFT = 24;
static uint32 HighDotClockWidth;
extern Blip_Buffer FXsbuf[2]; // FIXME, externals are evil!

#define TTRANS(_testvar) TTRANS_LUT[_testvar] //((_testvar) ? 0 : TRANS_OR)

bool KING_Init(void)
{
 if(!(king = (king_t*)MDFN_malloc(sizeof(king_t), _("KING Data"))))
  return(0);

 HighDotClockWidth = MDFN_GetSettingUI("pcfx.high_dotclock_width");
 BGLayerDisable = 0;

 for(int x = 0; x < 512; x++)
  priority_0_dummybuf[x] = TRANS_OR;

 // Build happy map!
 // Don't change this unless you know what you're doing!
 // There may appear to be a bug in the pixel mixing
 // code elsewhere, because it accesses this array like [vdc][bg][rainbow], but it's not a bug.
 // This multi-dimensional array has no concept of bg, vdc, rainbow, or their orders per-se, it just
 // contains priority information for 3 different layers.

 for(int bg_prio = 0; bg_prio < 17; bg_prio++)
  for(int vdc_prio = 0; vdc_prio < 17; vdc_prio++)
   for(int rainbow_prio = 0; rainbow_prio < 17; rainbow_prio++)
   {
    int bg_prio_test = bg_prio ? bg_prio : 0x10;
    int vdc_prio_test = vdc_prio ? vdc_prio : 0x10;
    int rainbow_prio_test = rainbow_prio ? rainbow_prio : 0x10;

    if(bg_prio_test > 8)
     happymap[bg_prio][vdc_prio][rainbow_prio][0] = 3;
    else
    {
     if(bg_prio_test < vdc_prio_test && bg_prio_test < rainbow_prio_test)
      happymap[bg_prio][vdc_prio][rainbow_prio][0] = 0;
     else if(bg_prio_test > vdc_prio_test && bg_prio_test > rainbow_prio_test)
      happymap[bg_prio][vdc_prio][rainbow_prio][0] = 2;
     else 
      happymap[bg_prio][vdc_prio][rainbow_prio][0] = 1;
    }

    if(vdc_prio_test > 8)
     happymap[bg_prio][vdc_prio][rainbow_prio][1] = 3;
    else
    {
     if(vdc_prio_test < bg_prio_test && vdc_prio_test < rainbow_prio_test)
      happymap[bg_prio][vdc_prio][rainbow_prio][1] = 0;
     else if(vdc_prio_test > bg_prio_test && vdc_prio_test > rainbow_prio_test)
      happymap[bg_prio][vdc_prio][rainbow_prio][1] = 2;
     else
      happymap[bg_prio][vdc_prio][rainbow_prio][1] = 1;
    }

    if(rainbow_prio_test > 8)
     happymap[bg_prio][vdc_prio][rainbow_prio][2] = 3;
    else
    {
     if(rainbow_prio_test < bg_prio_test && rainbow_prio_test < vdc_prio_test)
      happymap[bg_prio][vdc_prio][rainbow_prio][2] = 0;
     else if(rainbow_prio_test > bg_prio_test && rainbow_prio_test > vdc_prio_test)
      happymap[bg_prio][vdc_prio][rainbow_prio][2] = 2;
     else
      happymap[bg_prio][vdc_prio][rainbow_prio][2] = 1;
    }
   }

 for(int x = 0; x < 256; x++)
  TTRANS_LUT[x] = x ? 0 : TRANS_OR;

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddASpace(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "kram0", "KRAM Page 0", 19);
 MDFNDBG_AddASpace(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "kram1", "KRAM Page 1", 19);
 MDFNDBG_AddASpace(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "vdcvram0", "VDC-A VRAM", 17);
 MDFNDBG_AddASpace(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "vdcvram1", "VDC-B VRAM", 17);
 MDFNDBG_AddASpace(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "vce", "VCE Palette RAM", 10);
 #endif

 SCSICD_Init(SCSICD_PCFX, 3, &FXsbuf[0], &FXsbuf[1], 153600 * MDFN_GetSettingUI("pcfx.cdspeed"), 21477273, KING_CDIRQ, KING_StuffSubchannels);

 return(1);
}

void KING_Close(void)
{
 if(king)
 {
  free(king);
  king = NULL;
 }
}


void KING_Reset(void)
{
 memset(&fx_vce, 0, sizeof(fx_vce));
 memset(king, 0, sizeof(king_t));

 king->Reg00 = 0;
 king->Reg01 = 0;
 king->Reg02 = 0;
 king->Reg03 = 0;
 king->dma_receive_active = FALSE;
 king->dma_send_active = FALSE;
 StopKingMagic();


 BuildCMT();

 RedoKINGIRQCheck();
 RebuildLayerMaskCache();
 RecalcKRAM_Mask();

 for(unsigned int x = 0; x < 0x200; x++)
  RedoPaletteCache(x);

 SoundBox_SetKINGADPCMControl(0);

 SCSICD_Power();
}


static INLINE void DRAWBG8x1_4(uint32 *target, uint16 cg, uint32 *palette_ptr, uint32 layer_or)
{
 if(target[0] & TRANS_OR) target[0] = TTRANS(cg >> 14) | palette_ptr[(cg >> 14)] | layer_or;
 if(target[1] & TRANS_OR) target[1] = TTRANS((cg >> 12) & 0x3) | palette_ptr[((cg >> 12) & 0x3)] | layer_or;
 if(target[2] & TRANS_OR) target[2] = TTRANS((cg >> 10) & 0x3) | palette_ptr[((cg >> 10) & 0x3)] | layer_or;
 if(target[3] & TRANS_OR) target[3] = TTRANS((cg >> 8) & 0x3) | palette_ptr[((cg >> 8) & 0x3)] | layer_or;
 if(target[4] & TRANS_OR) target[4] = TTRANS((cg >> 6) & 0x3) | palette_ptr[((cg >> 6) & 0x3)] | layer_or;
 if(target[5] & TRANS_OR) target[5] = TTRANS((cg >> 4) & 0x3) | palette_ptr[((cg >> 4) & 0x3)] | layer_or;
 if(target[6] & TRANS_OR) target[6] = TTRANS((cg >> 2) & 0x3) | palette_ptr[((cg >> 2) & 0x3)] | layer_or;
 if(target[7] & TRANS_OR) target[7] = TTRANS((cg >> 0) & 0x3) | palette_ptr[((cg >> 0) & 0x3)] | layer_or;
}

static INLINE void DRAWBG8x1_16(uint32 *target, uint16 *cgptr, uint32 *palette_ptr, uint32 layer_or)
{
 if(target[0] & TRANS_OR) target[0] = TTRANS(cgptr[0] >> 12) | palette_ptr[((cgptr[0] >> 12))] | layer_or;
 if(target[1] & TRANS_OR) target[1] = TTRANS((cgptr[0] >> 8) & 0xF) | palette_ptr[(((cgptr[0] >> 8) & 0xF))] | layer_or;
 if(target[2] & TRANS_OR) target[2] = TTRANS((cgptr[0] >> 4) & 0xF) | palette_ptr[(((cgptr[0] >> 4) & 0xF))] | layer_or;
 if(target[3] & TRANS_OR) target[3] = TTRANS((cgptr[0] >> 0) & 0xF) | palette_ptr[(((cgptr[0] >> 0) & 0xF))] | layer_or;

 if(target[4] & TRANS_OR) target[4] = TTRANS(cgptr[1] >> 12) | palette_ptr[((cgptr[1] >> 12))] | layer_or;
 if(target[5] & TRANS_OR) target[5] = TTRANS((cgptr[1] >> 8) & 0xF) | palette_ptr[(((cgptr[1] >> 8) & 0xF))] | layer_or;
 if(target[6] & TRANS_OR) target[6] = TTRANS((cgptr[1] >> 4) & 0xF) | palette_ptr[(((cgptr[1] >> 4) & 0xF))] | layer_or;
 if(target[7] & TRANS_OR) target[7] = TTRANS((cgptr[1] >> 0) & 0xF) | palette_ptr[(((cgptr[1] >> 0) & 0xF))] | layer_or;
}

static INLINE void DRAWBG8x1_256(uint32 *target, uint16 *cgptr, uint32 *palette_ptr, uint32 layer_or)
{
 if(target[0] & TRANS_OR) target[0] = TTRANS(cgptr[0] >> 0x8) | palette_ptr[(cgptr[0] >> 0x8)] | layer_or;
 if(target[1] & TRANS_OR) target[1] = TTRANS(cgptr[0] & 0xFF) | palette_ptr[(cgptr[0] & 0xFF)] | layer_or;
 if(target[2] & TRANS_OR) target[2] = TTRANS(cgptr[1] >> 0x8) | palette_ptr[(cgptr[1] >> 0x8)] | layer_or;
 if(target[3] & TRANS_OR) target[3] = TTRANS(cgptr[1] & 0xFF) | palette_ptr[(cgptr[1] & 0xFF)] | layer_or;
 if(target[4] & TRANS_OR) target[4] = TTRANS(cgptr[2] >> 0x8) | palette_ptr[(cgptr[2] >> 0x8)] | layer_or;
 if(target[5] & TRANS_OR) target[5] = TTRANS(cgptr[2] & 0xFF) | palette_ptr[(cgptr[2] & 0xFF)] | layer_or;
 if(target[6] & TRANS_OR) target[6] = TTRANS(cgptr[3] >> 0x8) | palette_ptr[(cgptr[3] >> 0x8)] | layer_or;
 if(target[7] & TRANS_OR) target[7] = TTRANS(cgptr[3] & 0xFF) | palette_ptr[(cgptr[3] & 0xFF)] | layer_or;
}

static INLINE void DRAWBG8x1_64K(uint32 *target, uint16 *cgptr, uint32 *palette_ptr, uint32 layer_or)
{
 if(target[0] & TRANS_OR) target[0] = TTRANS(cgptr[0] >> 8) | ((cgptr[0x0] & 0x00F0) << 8) | ((cgptr[0] & 0x000F)<<4) | ((cgptr[0] & 0xFF00) << 8) | layer_or;
 if(target[1] & TRANS_OR) target[1] = TTRANS(cgptr[1] >> 8) | ((cgptr[0x1] & 0x00F0) << 8) | ((cgptr[1] & 0x000F)<<4) | ((cgptr[1] & 0xFF00) << 8) | layer_or;
 if(target[2] & TRANS_OR) target[2] = TTRANS(cgptr[2] >> 8) | ((cgptr[0x2] & 0x00F0) << 8) | ((cgptr[2] & 0x000F)<<4) | ((cgptr[2] & 0xFF00) << 8) | layer_or;
 if(target[3] & TRANS_OR) target[3] = TTRANS(cgptr[3] >> 8) | ((cgptr[0x3] & 0x00F0) << 8) | ((cgptr[3] & 0x000F)<<4) | ((cgptr[3] & 0xFF00) << 8) | layer_or;
 if(target[4] & TRANS_OR) target[4] = TTRANS(cgptr[4] >> 8) | ((cgptr[0x4] & 0x00F0) << 8) | ((cgptr[4] & 0x000F)<<4) | ((cgptr[4] & 0xFF00) << 8) | layer_or;
 if(target[5] & TRANS_OR) target[5] = TTRANS(cgptr[5] >> 8) | ((cgptr[0x5] & 0x00F0) << 8) | ((cgptr[5] & 0x000F)<<4) | ((cgptr[5] & 0xFF00) << 8) | layer_or;
 if(target[6] & TRANS_OR) target[6] = TTRANS(cgptr[6] >> 8) | ((cgptr[0x6] & 0x00F0) << 8) | ((cgptr[6] & 0x000F)<<4) | ((cgptr[6] & 0xFF00) << 8) | layer_or;
 if(target[7] & TRANS_OR) target[7] = TTRANS(cgptr[7] >> 8) | ((cgptr[0x7] & 0x00F0) << 8) | ((cgptr[7] & 0x000F)<<4) | ((cgptr[7] & 0xFF00) << 8) | layer_or;
}

static INLINE void DRAWBG8x1_16M(uint32 *target, uint16 *cgptr, uint32 *palette_ptr, uint32 layer_or)
{
 if(target[0] & TRANS_OR) target[0] = TTRANS(cgptr[0] >> 8) | ((cgptr[0x0] & 0xFF00) << 8) | (cgptr[1] & 0xFF00) | (cgptr[1] & 0xFF) | layer_or;
 if(target[1] & TRANS_OR) target[1] = TTRANS(cgptr[0] & 0xFF) | ((cgptr[0x0] & 0x00FF) << 16) | (cgptr[1] & 0xFF00) | (cgptr[1] & 0xFF) | layer_or;
 if(target[2] & TRANS_OR) target[2] = TTRANS(cgptr[2] >> 8) | ((cgptr[0x2] & 0xFF00) << 8) | (cgptr[3] & 0xFF00) | (cgptr[3] & 0xFF) | layer_or;
 if(target[3] & TRANS_OR) target[3] = TTRANS(cgptr[2] & 0xFF) | ((cgptr[0x2] & 0x00FF) << 16) | (cgptr[3] & 0xFF00) | (cgptr[3] & 0xFF) | layer_or;
 if(target[4] & TRANS_OR) target[4] = TTRANS(cgptr[4] >> 8) | ((cgptr[0x4] & 0xFF00) << 8) | (cgptr[5] & 0xFF00) | (cgptr[5] & 0xFF) | layer_or;
 if(target[5] & TRANS_OR) target[5] = TTRANS(cgptr[4] & 0xFF) | ((cgptr[0x4] & 0x00FF) << 16) | (cgptr[5] & 0xFF00) | (cgptr[5] & 0xFF) | layer_or;
 if(target[6] & TRANS_OR) target[6] = TTRANS(cgptr[6] >> 8) | ((cgptr[0x6] & 0xFF00) << 8) | (cgptr[7] & 0xFF00) | (cgptr[7] & 0xFF) | layer_or;       
 if(target[7] & TRANS_OR) target[7] = TTRANS(cgptr[6] & 0xFF) | ((cgptr[0x6] & 0x00FF) << 16) | (cgptr[7] & 0xFF00) | (cgptr[7] & 0xFF) | layer_or;
}

// Loop prefix non-endless
#define DRAWBG8x1_LPRE() if(bat_x < bat_width) {

// Loop postfix non-endless
#define DRAWBG8x1_LPOST() } bat_x = (bat_x + 1) & bat_width_mask;

static bool bgmode_warning = 0; // Debug

static void DrawBG(uint32 *target, int n)
{
 uint16 bgmode = (king->bgmode >> (n * 4)) & 0xF;
 bool endless = (king->BGScrollMode >> n) & 0x1;
 uint32 XScroll = king->BGXScroll[n];
 uint32 YScroll = king->BGYScroll[n];
 uint32 bat_offset = king->BGBATAddr[n] * 1024;
 uint32 cg_offset = king->BGCGAddr[n] * 1024;
 uint32 bat_and_cg_bank = (king->PageSetting & 0x0010) ? 1 : 0;
 unsigned int bat_width, bat_width_mask, bat_width_shift;
 int bat_height, bat_height_mask, bat_height_shift;
 uint16 cg_mask[8];
 uint16 cg_remap[8];
 bool rotate_mode = 0;
 uint16 *king_cg_base, *king_bat_base;

 // If the bg mode is invalid, don't draw this layer, duuhhhh
 if(!(bgmode & 0x7))
  return;

 if((bgmode & 0x7) >= 6)
 {
  if(!bgmode_warning)
  {
   printf("Unsupported KING BG Mode for KING BG %d: %02x\n", n, bgmode);
   bgmode_warning = TRUE;
  }
  return;
 }

 bat_width_shift = (king->BGSize[n] & 0xF0) >> 4;
 bat_height_shift = king->BGSize[n] & 0x0F;

 if(!n && !bat_width_shift && !bat_height_shift && (king->BGSize[0] & 0xF000) && (king->BGSize[0] & 0x0F00))
 {
  //puts("Angelique Special Hack");
  bat_offset = king->BG0SubBATAddr * 1024;
  cg_offset = king->BG0SubCGAddr * 1024;
  bat_width_shift = (king->BGSize[n] & 0xF000) >> 12;
  bat_height_shift = (king->BGSize[n] & 0x0F00) >> 8;
 }

 // We don't need to &= cg_offset and bat_offset with 0x1ffff after here, as the effective addresses 
 // calculated with them are anded with 0x1ffff in the rendering code already.
 king_cg_base = &king->KRAM[bat_and_cg_bank][cg_offset & 0x20000 & king->KRAM_Mask_Full];
 king_bat_base = &king->KRAM[bat_and_cg_bank][bat_offset & 0x20000 & king->KRAM_Mask_Full];


 memset(cg_mask, 0, sizeof(cg_mask));
 memset(cg_remap, 0, sizeof(cg_remap));

 // Longterm TODO:  Phase out BATFetchCycle in favor of BATFetchCycles
 bool BATFetchCycle = FALSE;
 bool BATFetchCycles[8] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };

 if(king->MPROGControl & 0x1)
 {
  int remap_thing = 0;

  remap_thing = 0;
  for(int x = 0; x < 8; x++)
  {
   uint16 mpd = king->MPROGData[((cg_offset & 0x20000) ? 0x8 : 0x0) + x];
   if(((mpd >> 6) & 0x3) == n && !(mpd & 0x100) && !(mpd & 0x010))
   {
    cg_mask[remap_thing] = 0xFFFF;

    if(mpd & 0x20)
    {
     cg_remap[remap_thing] = remap_thing;
     if(n == 0) 
     {
      // Rotation/scaling isn't possible in 16M color mode.
      if((bgmode & 0x7) == BGMODE_4 || (bgmode & 0x7) == BGMODE_16 || (bgmode & 0x7) == BGMODE_256 || (bgmode & 0x7) == BGMODE_64K)
       rotate_mode = TRUE;
     }
    }
    else
     cg_remap[remap_thing] = mpd & 0x7;

    remap_thing++;
   }
  }

  for(int x = 0; x < 8; x++)
  {
   uint16 mpd = king->MPROGData[((bat_offset & 0x20000) ? 0x8 : 0x0) + x];
   if(((mpd >> 6) & 0x3) == n && !(mpd & 0x100) && (mpd & 0x010))
   {
    BATFetchCycle = TRUE;
    BATFetchCycles[x] = TRUE;
   }
  }
 }

 if(!BATFetchCycle)
 {
  bgmode &= ~0x8;  
 }

 // BG0 supports a virtual screen up to 1024x1024(0xA by 0xA, 2**10 = 1024).
 // BG1-3 support a virtual screen up to 512x512(0x9 by 0x9, 2 ** 9 = 512)
 //  FIXME:  What happens if a game sets the screen width or height setting too high?? 
 int max_size_setting;
 
 if(!n)
  max_size_setting = 0xA;
 else
  max_size_setting = 0x9;

 if(bat_width_shift < 3) bat_width_shift = 3;
 if(bat_width_shift > max_size_setting) bat_width_shift = max_size_setting;

 bat_width = (1 << bat_width_shift) >> 3;
 bat_width_mask = bat_width - 1;

 if(bat_height_shift < 3) bat_height_shift = 3;
 if(bat_height_shift > max_size_setting) bat_height_shift = max_size_setting;

 bat_height = (1 << bat_height_shift) >> 3;
 bat_height_mask = bat_height - 1;

 if(!endless)
 {
  bat_width_mask = bat_height_mask = (((1 << max_size_setting) * 2) / 8) - 1;
 }

 uint32 YOffset = (YScroll + (fx_vce.raster_counter - 22)) & 0xFFFF;
 uint32 layer_or = n << 28;

 int bat_y = (YOffset >> 3) & bat_height_mask;
 uint32 bat_x = (XScroll >> 3) & bat_width_mask;
 int ysmall = YOffset & 0x7;

 if(rotate_mode)
  target += 8;
 else
  target += 8 - (XScroll & 0x7);

 // If we're in non-endless scroll mode, and we've scrolled past our visible area in the vertical direction, so return.
 // If we're in rotation mode, do NOT do this, as coordinate transformations occur; the height(and width) checking code for
 // non-endless mode in rotation mode is in with the rotation mode rendering code.
 if(!rotate_mode && !endless && bat_y >= bat_height) // Draw this line as transparency and return?
 {
  return;
 }

 // Adjust/corrupt bat_y to be faster in our blitting code
 bat_y = (bat_y << bat_width_shift) >> 3;

 uint32 palette_offset = fx_vce.palette_offset[1 + (n >> 1)] >> ((n & 1) ? 8 : 0);
 palette_offset <<= 1;
 palette_offset &= 0x1FF;

 uint32 *palette_ptr = &fx_vce.palette_table_cache[palette_offset];

 {
  int wmul = (1 << bat_width_shift), wmask = (1 << bat_height_shift) - 1;
  int sexy_y_pos = (YOffset & wmask) * wmul;
  

  #define ROTCODE_PRE	\
         int32 a, b, c, d;	\
         int32 raw_x_coord = (int32)sign_11_to_s16(XScroll) - (int16)king->BGAffinCenterX;	\
         int32 raw_y_coord = fx_vce.raster_counter + (int32)sign_11_to_s16(YScroll) - 22 - (int16)king->BGAffinCenterY;		\
         int32 xaccum;	\
         int32 yaccum;	\
	\
         if(!endless)	\
         {	\
          bat_width_mask = 0xFFFF;	\
          bat_height_mask = 0xFFFF;	\
         }	\
         a = (int16)king->BGAffinA;	\
         b = (int16)king->BGAffinB;	\
         c = (int16)king->BGAffinC;	\
         d = (int16)king->BGAffinD;	\
	\
         xaccum = raw_x_coord * a + raw_y_coord * b;	\
         yaccum = raw_y_coord * d + raw_x_coord * c;	\
         xaccum += (int16)king->BGAffinCenterX << 8;	\
         yaccum += (int16)king->BGAffinCenterY << 8;	\

  #define ROTCODE_LOOP_PRE	\
         for(int x = 0; x < 256; x++)	\
         {	\
          uint16 new_x = (xaccum >> 8);	\
          uint16 new_y = (yaccum >> 8);	\
          uint16 *cgptr;	\
          bat_x = (new_x >> 3) & bat_width_mask;	\
          bat_y = (new_y >> 3) & bat_height_mask;	\
          ysmall = new_y & 0x7;	

  #define ROTCODE_LOOP_POST	\
          xaccum += a;	\
          yaccum += c;	\
         }


  if((bgmode & 0x7) == BGMODE_4 && rotate_mode)
  {
         ROTCODE_PRE;

         ROTCODE_LOOP_PRE;
	 uint32 pbn = 0;
         if(bgmode & 0x8)
         {
          uint16 bat = king_bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & king->KRAM_Mask_Sub];
          pbn = ((bat >> 12) << 2);
          bat &= 0x0FFF;
          cgptr = &king_cg_base[(cg_offset + (bat * 8) + ysmall) & king->KRAM_Mask_Sub];
         }
         else
         {
          sexy_y_pos = (new_y & wmask) * wmul / 8;
          cgptr = &king_cg_base[(cg_offset + bat_x + sexy_y_pos) & king->KRAM_Mask_Sub];
         }
         uint8 ze_cg = (cgptr[0] >> ((7 - (new_x & 7)) << 1)) & 0x03;

         if(endless || (bat_x < bat_width && bat_y < bat_height))
         {
          if(target[x] & TRANS_OR) target[x] = TTRANS(ze_cg) | palette_ptr[pbn + ze_cg] | layer_or;
         }
        ROTCODE_LOOP_POST;
  }
  else if((bgmode & 0x7) == BGMODE_16 && rotate_mode)
  {
         ROTCODE_PRE;

         ROTCODE_LOOP_PRE;
	 uint32 pbn = 0;

         if(bgmode & 0x8)
         {
          uint16 bat = king_bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & king->KRAM_Mask_Sub];
          pbn = ((bat >> 12) << 4);
	  bat &= 0x0FFF;
          cgptr = &king_cg_base[(cg_offset + (bat * 16) + ysmall * 2) & king->KRAM_Mask_Sub];
         }
         else
         {
          sexy_y_pos = (new_y & wmask) * wmul / 4;
          cgptr = &king_cg_base[(cg_offset + (bat_x * 2) + sexy_y_pos) & king->KRAM_Mask_Sub];
         }
         uint8 ze_cg = (cgptr[(new_x >> 2) & 0x1] >> ((3 - (new_x & 3)) << 2)) & 0x0F;

         if(endless || (bat_x < bat_width && bat_y < bat_height))
         {
          if(target[x] & TRANS_OR) target[x] = TTRANS(ze_cg) | palette_ptr[pbn + ze_cg] | layer_or;
         }
        ROTCODE_LOOP_POST;
  }
  else if((bgmode & 0x7) == BGMODE_256 && rotate_mode)
  {
	 ROTCODE_PRE;

	 ROTCODE_LOOP_PRE;
          if(bgmode & 0x8)
          {
           uint16 bat = king_bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & king->KRAM_Mask_Sub];
           cgptr = &king_cg_base[(cg_offset + (bat * 32) + ysmall * 4) & king->KRAM_Mask_Sub];
          }
          else
          {
           sexy_y_pos = (new_y & wmask) * wmul / 2;
           cgptr = &king_cg_base[(cg_offset + (bat_x * 4) + sexy_y_pos) & king->KRAM_Mask_Sub];
          }
          uint8 ze_cg = cgptr[(new_x >> 1) & 0x3] >> (((new_x & 1) ^ 1) << 3);

          if(endless || (bat_x < bat_width && bat_y < bat_height))
          {
           if(target[x] & TRANS_OR) target[x] = TTRANS(ze_cg) | palette_ptr[ze_cg] | layer_or;
          }
	 ROTCODE_LOOP_POST;
  } 
  else if((bgmode & 0x7) == BGMODE_64K && rotate_mode)
  {
	ROTCODE_PRE;
	ROTCODE_LOOP_PRE;
          if(bgmode & 0x8)
          {
           uint16 bat = king_bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & king->KRAM_Mask_Sub];
           cgptr = &king_cg_base[(cg_offset + (bat * 64) + ysmall * 8) & king->KRAM_Mask_Sub];
          }
          else
          {
           sexy_y_pos = (new_y & wmask) * wmul;
           cgptr = &king_cg_base[(cg_offset + (bat_x * 8) + sexy_y_pos) & king->KRAM_Mask_Sub];
          }
          uint16 ze_cg = cgptr[new_x & 0x7];

          if(endless || (bat_x < bat_width && bat_y < bat_height))
          {
           if(target[x] & TRANS_OR) target[x] = TTRANS(ze_cg >> 8) | ((ze_cg & 0x00F0) << 8) | ((ze_cg & 0x000F)<<4) | ((ze_cg & 0xFF00) << 8) | layer_or;
          }
         ROTCODE_LOOP_POST;
  }
  else switch(bgmode & 0x7)
  {
   case 0x01: // 4 color, 1/4 byte per pixel :b
        sexy_y_pos >>= 3;
        for(int x = 0; x < 256 + 8; x+= 8)
        {
	 DRAWBG8x1_LPRE();
         uint16 *cgptr;
	 uint16 cg[1];

	 uint32 pbn = 0;
	 if(bgmode & 0x8)
	 {
          uint16 bat = king_bat_base[(bat_offset + (bat_x + bat_y)) & king->KRAM_Mask_Sub];
          pbn = (bat >> 12) << 2;
          bat &= 0x0FFF;
          cgptr = &king_cg_base[(cg_offset + (bat * 8) + ysmall) & king->KRAM_Mask_Sub];
	 }
	 else
  	  cgptr = &king_cg_base[(cg_offset + (bat_x * 1) + sexy_y_pos) & king->KRAM_Mask_Sub];

	 cg[0] = cgptr[cg_remap[0]] & cg_mask[0];

         DRAWBG8x1_4(target + x, cg[0], palette_ptr + pbn, layer_or);
         DRAWBG8x1_LPOST();
        }
	break;
   case 0x02: // 16 color, 1/2 byte per pixel
	sexy_y_pos >>= 2;
        for(int x = 0; x < 256 + 8; x+= 8)
        {
	 DRAWBG8x1_LPRE();
         uint16 *cgptr;
	 uint16 cg[2];
	 uint32 pbn = 0;

	 if(bgmode & 0x8)
	 {
          uint16 bat = king_bat_base[(bat_offset + (bat_x + bat_y)) & king->KRAM_Mask_Sub];
          pbn = ((bat >> 12) << 4);
          bat &= 0x0FFF;
          cgptr = &king_cg_base[(cg_offset + (bat * 16) + ysmall * 2) & king->KRAM_Mask_Sub];
	 }
	 else 
	  cgptr = &king_cg_base[(cg_offset + (bat_x * 2) + sexy_y_pos) & king->KRAM_Mask_Sub];

         for(int cow = 0; cow < 2; cow++)
          cg[cow] = cgptr[cg_remap[cow]] & cg_mask[cow];

         DRAWBG8x1_16(target + x, cg, palette_ptr + pbn, layer_or);
         DRAWBG8x1_LPOST();
        }
        break;
   case 0x03: // 256 color, 1 byte per pixel palettized - OK
	 sexy_y_pos >>= 1;
         for(int x = 0; x < 256 + 8; x+= 8)
         {
	  DRAWBG8x1_LPRE();
	  uint16 *cgptr;
	  uint16 cg[4];

	  if(bgmode & 0x8)
	  {
           uint16 bat = king_bat_base[(bat_offset + (bat_x + bat_y)) & king->KRAM_Mask_Sub];
           cgptr = &king_cg_base[(cg_offset + (bat * 32) + ysmall * 4) & king->KRAM_Mask_Sub];
	  }
	  else
           cgptr = &king_cg_base[(cg_offset + (bat_x * 4) + sexy_y_pos) & king->KRAM_Mask_Sub];

	  for(int cow = 0; cow < 4; cow++)
           cg[cow] = cgptr[cg_remap[cow]] & cg_mask[cow];

          DRAWBG8x1_256(target + x, cg, palette_ptr, layer_or);
          DRAWBG8x1_LPOST();
        }
	break;

   case 0x04: // 64K color, 2 bytes per pixel - OK
        for(int x = 0; x < 256 + 8; x+=8)
        {
	 DRAWBG8x1_LPRE();
	 uint16 *cgptr;
	 uint16 cg[8];
	 if(bgmode & 0x8)
	 {
          uint16 bat = king_bat_base[(bat_offset + (bat_x + bat_y)) & king->KRAM_Mask_Sub];
          cgptr = &king_cg_base[(cg_offset + (bat * 64) + ysmall * 8) & king->KRAM_Mask_Sub];
	 }
	 else
          cgptr = &king_cg_base[(cg_offset + (bat_x * 8) + sexy_y_pos) & king->KRAM_Mask_Sub];

         for(int cow = 0; cow < 8; cow++)
          cg[cow] = cgptr[cg_remap[cow]] & cg_mask[cow];

         DRAWBG8x1_64K(target + x, cg, palette_ptr, layer_or);
         DRAWBG8x1_LPOST();
        }
	break;
   case 0x05: // 16M color, 2 bytes per pixel - OK
        for(int x = 0; x < 256 + 8; x+=8)
        {
	 DRAWBG8x1_LPRE();
	 uint16 *cgptr;
	 uint16 cg[8];
	 if(bgmode & 0x8)
	 {
          uint16 bat = king_bat_base[(bat_offset + (bat_x + bat_y)) & king->KRAM_Mask_Sub];
          cgptr = &king_cg_base[(cg_offset + (bat * 64) + ysmall * 8) & king->KRAM_Mask_Sub];
	 }
         else 
	  cgptr = &king_cg_base[(cg_offset + (bat_x * 8) + sexy_y_pos) & king->KRAM_Mask_Sub];

         for(int cow = 0; cow < 8; cow++)
          cg[cow] = cgptr[cg_remap[cow]] & cg_mask[cow];

         DRAWBG8x1_16M(target + x, cg, palette_ptr, layer_or);
         DRAWBG8x1_LPOST();
        }
	break;
  #if 0
   case BGMODE_64K_EXTDOT:
	break;

   case BGMODE_16M_EXTDOT:
	{
	 uint32 fat_bat = ((YOffset & ((1 << bat_height_shift) - 1)) << bat_width_shift) + (((XScroll & ~ 7) & ((1 << bat_width_shift) - 1));
	}
        break;
  #endif
  }
 }
}

static int16 UVLUT[65536][3];
static uint8 RGBDeflower[1152]; // 0 is at 384

static void RebuildUVLUT(void)
{
 for(int ur = 0; ur < 256; ur++)
  for(int vr = 0; vr < 256; vr++)
  {
   int r, g, b;
   int u, v;
   
   u = ur - 128;
   v = vr - 128;

   // FIXME:  Use lrint() ?
   r = (int)(0 - 0.000039457070707 * u + 1.139827967171717 * v);
   g = (int)(0 - 0.394610164141414 * u - 0.580500315656566 * v);
   b = (int)(0 + 2.031999684343434 * u - 0.000481376262626 * v);

   UVLUT[vr + ur * 256][0] = r;
   UVLUT[vr + ur * 256][1] = g;
   UVLUT[vr + ur * 256][2] = b;
  }
 for(int x = 0; x < 1152; x++)
 {
  if(x < 384) RGBDeflower[x] = 0;
  else if(x > (384 + 255)) RGBDeflower[x] = 255;
  else
   RGBDeflower[x] = x - 384;
 }
}

static uint32 ALWAYS_INLINE YUV888_TO_RGB888(uint32 yuv)
{
 int32 r, g, b;
 uint8 y = yuv >> 16;

 r = y + UVLUT[yuv & 0xFFFF][0];
 g = y + UVLUT[yuv & 0xFFFF][1];
 b = y + UVLUT[yuv & 0xFFFF][2];

 r = clamp_to_u8(r);
 g = clamp_to_u8(g);
 b = clamp_to_u8(b);

 return((r << FSettings.rshift) | (g << FSettings.gshift) | (b << FSettings.bshift));
}

void KING_RunFrame(fx_vdc_t **vdc_chips, uint32 *pXBuf, MDFN_Rect *LineWidths, int skip)
{
 //MDFN_DispMessage("P0:%06x P1:%06x; I0: %06x I1: %06x", king->ADPCMPlayAddress[0], king->ADPCMPlayAddress[1], king->ADPCMIntermediateAddress[0] << 6, king->ADPCMIntermediateAddress[1] << 6);
 //MDFN_DispMessage("%d %d\n", SCSICD_GetACK(), SCSICD_GetREQ());

 fx_vce.raster_counter = 0;
 for(;;)
 {
  uint32 vdc_linebuffers[2][512]  __attribute__ ((aligned (8)));
  uint32 vdc_linebuffer[512]  __attribute__ ((aligned (8)));
  uint32 vdc_linebuffer_yuved[512]  __attribute__ ((aligned (8)));
  unsigned int width = (fx_vce.picture_mode & 0x08) ? 341 : 256;

  #ifdef WANT_DEBUGGER
  if(GfxDecode_Line == fx_vce.raster_counter)
   DoGfxDecode();
  #endif

  if(fx_vce.raster_counter == king->RAINBOWTransferStartPosition && (king->RAINBOWTransferControl & 1))
  {
   //printf("RBS: %d\n", fx_vce.raster_counter);
   king->RAINBOWKRAMReadPos = king->RAINBOWKRAMA << 1;
   RAINBOW_TransferStart();
   king->RAINBOWTBC = king->RAINBOWTransferBlockCount + 1;
   king->RAINBOWRasterCounter = 0;
  }

  FXVDC_DoLine(vdc_chips[0], vdc_linebuffers[0], skip);
  FXVDC_DoLine(vdc_chips[1], vdc_linebuffers[1], skip);

  uint8 rb_data[256 * 4]; // *2 for YUV data
  int rb_type = -1;

  //if(fx_vce.raster_counter == king->RasterIRQLine && (king->RAINBOWTransferControl & 0x2))
  //{
  // //printf("Wovely: %d, %d, %d\n", fx_vce.raster_counter, king->RAINBOWRasterCounter, king->RAINBOWTransferControl);
  // king->RasterIRQPending = TRUE;
  // RedoKINGIRQCheck();
  //}

  if(king->RAINBOWTBC)
  {
   rb_type = RAINBOW_FetchRaster(rb_data, king->RAINBOWTBC > 1, 0); //skip && fx_vce.raster_counter < (262 - 16));
   king->RAINBOWRasterCounter = (king->RAINBOWRasterCounter + 1) & 0x1FF;

   if(!(king->RAINBOWRasterCounter & 0xF))
   {
    king->RAINBOWTBC--;
    if(!king->RAINBOWTBC)
    {

    }
   }
  }

  if(fx_vce.raster_counter >= 22 && fx_vce.raster_counter < 262)
  {
   int start, end;

   if(width == 256)
    start = 8 + 2 * 8;
   else
    start = 8 + 30; //64; //64;

   end = start + width;

   if(width == 341) // Fix for high dot-clock default layer mixing code, 1023 / 3 = 341(for rightmost pixel) so we ++ here, especially
    end++;	    // so the layer number is set properly!

   // 8 * 2 for left + right padding for scrolling
   uint32 bg_linebuffer[256 + 8 + 8] __attribute__ ((aligned (8)));
   uint32 rainbow_linebuffer[256] __attribute__ ((aligned (8)));

   if(!skip)
   {
    uint32 vdc_poffset[2];
 
    vdc_poffset[0] = ((fx_vce.palette_offset[0] >> 0) & 0xFF) << 1; // BG
    vdc_poffset[1] = ((fx_vce.palette_offset[0] >> 8) & 0xFF) << 1; // SPR

    for(int x = start; x < end; x++)
    {
     static const uint32 layer_num[2] = { 4 << 28, 5 << 28};
     uint32 zort[2];

     zort[0] = vdc_linebuffers[0][x];
     zort[1] = vdc_linebuffers[1][x];

     // SPR combination
     if((fx_vce.picture_mode & 0x80) && (zort[1] & 0x100) && (zort[1] & 0x80) && !(zort[1] & 0x200))
     {
      uint8 cheepix = (zort[1] & 0xF) | ((zort[0] & 0xF) << 4);
      uint32 pali = (cheepix + vdc_poffset[1]) & 0x1FF;

      zort[0] = zort[1] = (pali & 0x1FF) | 0x200;

      if(!cheepix) { zort[0] |= 0x400; zort[1] |= 0x400; }
     }
     // BG combination
     else if((fx_vce.picture_mode & 0x40) && !(zort[1] & 0x100) && (zort[1] & 0x80) && !(zort[1] & 0x200))
     {
      uint8 cheepix = (zort[1] & 0xF) | ((zort[0] & 0xF) << 4);
      uint32 pali = (cheepix + vdc_poffset[0]) & 0x1FF;

      zort[0] = zort[1] = pali;
      
      if(!cheepix) { zort[0] |= 0x400; zort[1] |= 0x400; }
     }
     else
     {
      zort[0] = ((vdc_poffset[(zort[0] >> 8) & 1] + (zort[0]&0xFF)) & 0x1FF) | ((zort[0] & 0x300) << 1);
      zort[1] = ((vdc_poffset[(zort[1] >> 8) & 1] + (zort[1]&0xFF)) & 0x1FF) | ((zort[1] & 0x300) << 1);
     }

     vdc_linebuffer[x - start] = vdc_linebuffers[0][x];

     if(!(zort[1] & 0x400))
     {
      vdc_linebuffer[x - start] = vdc_linebuffers[1][x];
      zort[0] = zort[1];
     }
     // Original:
     //vdc_linebuffer_yuved[x] = fx_vce.palette_table_cache[zort[0] & 0x1FF] | ((zort[0] & 0x200) ? 5 << 28 : 4 << 28) | ((zort[0] & 0x400) ? TRANS_OR : 0);
     // Optimized:
     vdc_linebuffer_yuved[x - start] = fx_vce.palette_table_cache[zort[0] & 0x1FF] | layer_num[(zort[0] >> 9) & 1] | ((zort[0] & 0x400) << (TRANS_OR_SHIFT - 10));
    }

    if(rb_type == 1) // YUV
    {
     #if 0
     // Gah, disabling chroma key creates a super-evil green border of doom in Nirgends during movies :(
     if(fx_vce.picture_mode & 0x08) // No chroma key in 7.16MHz pixel mode
     {
      uint32 *meow = (uint32 *)rb_data;

      for(int x = 0; x < 256; x++)
       rainbow_linebuffer[x] = (meow[x] &~ TRANS_OR) | (6 << 28);
     }
     else // Otherwise, chroma key away!
     #endif
     {
      uint32 *meow = (uint32 *)rb_data;
      unsigned int ymin = fx_vce.ChromaKeyY & 0xFF;
      unsigned int ymax = fx_vce.ChromaKeyY >> 8;
      unsigned int umin = fx_vce.ChromaKeyU & 0xFF;
      unsigned int umax = fx_vce.ChromaKeyU >> 8;
      unsigned int vmin = fx_vce.ChromaKeyV & 0xFF;
      unsigned int vmax = fx_vce.ChromaKeyV >> 8;

      for(int x = 0; x < 256; x++)
      {
       unsigned int y, u, v;
       uint32 pixel = meow[x];
       pixel |= 6 << 28;

       y = (pixel >> 16) & 0xFF;
       u = (pixel >> 8) & 0xFF;
       v = (pixel >> 0) & 0xFF;

       if(y >= ymin && y <= ymax && u >= umin && u <= umax && v >= vmin && v <= vmax)
        pixel |= TRANS_OR;

       rainbow_linebuffer[x] = pixel;
      }
     }
    }
    else if(rb_type == 0)
    {
     uint32 *rb_pptr = &fx_vce.palette_table_cache[((fx_vce.palette_offset[3] >> 0) & 0xFF) << 1];

     for(int x = 0; x < 256; x++)
     {
      rainbow_linebuffer[x] = rb_pptr[rb_data[x]] | TTRANS(rb_data[x]) | (6 << 28);
     }
    }
    else
     MDFN_FastU32MemsetM8(rainbow_linebuffer, TRANS_OR | (6 << 28), 256);


    /*
        4 = Foremost
        1 = Hindmost
        0 = Hidden
    */

    MDFN_FastU32MemsetM8(bg_linebuffer + 8, TRANS_OR | (0 << 28), 256);

    for(int prio = 7; prio >= 1; prio--)
    {
     for(int x = 0; x < 4; x++)
     {
      int thisprio = (king->priority >> (x * 3)) & 0x7;

      if(BGLayerDisable & (1 << x)) continue;

      if(thisprio == prio)
       DrawBG(bg_linebuffer, x);
     }
    }

    // Now we have to mix everything together... I'm scared, mommy.
    // We have, vdc_linebuffer[0] and bg_linebuffer
    // Which layer is specified in bits 28-31:
    //  0 : BG0, 1: BG1, 2: BG2, 3: BG3
    //  4 : VDC BG, 5: VDC SPR
    //  6 : RAINBOW
    uint32 priority_remap[7];
    uint32 ble_cache[8];
    bool ble_cache_any = FALSE;

    //MDFN_DispMessage("%04x %04x, %04x-%04x %04x-%04x %04x-%04x", fx_vce.BLE, fx_vce.CCR, fx_vce.coefficients[0], fx_vce.coefficients[1], fx_vce.coefficients[2], fx_vce.coefficients[3], fx_vce.coefficients[4], fx_vce.coefficients[5]);
    for(int x = 0; x < 4; x++)
    {
     priority_remap[x] = (((fx_vce.priority[1] >> (x * 4)) & 0xF) + 1) & fx_vce.layer_mask_cache[x];
     //printf("RM: %d %d\n", x, priority_remap[x]);
    }
    priority_remap[4] = ((fx_vce.priority[0] & 0xF) + 1) & fx_vce.layer_mask_cache[4];
    priority_remap[5] = (((fx_vce.priority[0] >> 4) & 0xF) + 1) & fx_vce.layer_mask_cache[5];
    priority_remap[6] = (((fx_vce.priority[0] >> 8) & 0xF) + 1) & fx_vce.layer_mask_cache[6];

    // Rainbow layer disabled?
    if(rb_type == -1 || RAINBOWLayerDisable)
     priority_remap[6] = 0;

    for(int x = 0; x < 4; x++)
     ble_cache[x] = (fx_vce.BLE >> (4 + x * 2)) & 0x3;

    ble_cache[4] = (fx_vce.BLE >> 0) & 0x3;
    ble_cache[5] = (fx_vce.BLE >> 2) & 0x3;
    ble_cache[6] = (fx_vce.BLE >> 12) & 0x3;
    ble_cache[7] = 0;

    for(int x = 0; x < 8; x++)
     if(ble_cache[x])
     {
      ble_cache_any = TRUE;
      break;
     }
   
    uint8 *coeff_cache_y_back[3];
    int8 *coeff_cache_u_back[3], *coeff_cache_v_back[3];
    uint8 *coeff_cache_y_fore[3];
    int8 *coeff_cache_u_fore[3], *coeff_cache_v_fore[3];

    for(int x = 0; x < 3; x++)
    {
     coeff_cache_y_fore[x] = fx_vce.coefficient_mul_table_y[(fx_vce.coefficients[x * 2 + 0] >> 8) & 0xF];
     coeff_cache_u_fore[x] = fx_vce.coefficient_mul_table_uv[(fx_vce.coefficients[x * 2 + 0] >> 4) & 0xF];
     coeff_cache_v_fore[x] = fx_vce.coefficient_mul_table_uv[(fx_vce.coefficients[x * 2 + 0] >> 0) & 0xF];

     coeff_cache_y_back[x] = fx_vce.coefficient_mul_table_y[(fx_vce.coefficients[x * 2 + 1] >> 8) & 0xF];
     coeff_cache_u_back[x] = fx_vce.coefficient_mul_table_uv[(fx_vce.coefficients[x * 2 + 1] >> 4) & 0xF];
     coeff_cache_v_back[x] = fx_vce.coefficient_mul_table_uv[(fx_vce.coefficients[x * 2 + 1] >> 0) & 0xF];
    }

    uint32 *target = pXBuf + (MDFNGameInfo->pitch >> 2) * (fx_vce.raster_counter - 22);
    uint32 BPC_Cache = TRANS_OR | (7 << 28); // Backmost pixel color(cache)

    // If at least one layer is enabled with the HuC6261, hindmost color is palette[0]
    // If no layers are on, this color is black.
    // If front cellophane is enabled, this color is forced to black(TODO:  Confirm on a real system.  Black or from CCR).
    // If back cellophane is enabled, this color is forced to the value in CCR
    //  (back and front conditions are handled closer to the pixel mixing loops down below)
    // TODO:  Test on a real PC-FX to see if CCR is used or not if back cellophane is enabled even if all layers are disabled in the HuC6261, 
    //  or if it just outputs black.
    // TODO:  See if enabling front/back cellophane in high dot-clock mode will set the hindmost color, even though the cellophane color mixing
    //  is disabled in high dot-clock mode.
    if(fx_vce.picture_mode & 0x7F00)
     BPC_Cache |= fx_vce.palette_table_cache[0];
    else			
     BPC_Cache |= 0x008080;

#define DOCELLO(pixpoo) \
	if((pixel[pixpoo] >> 28) != 5 || ((fx_vce.SPBL >> ((vdc_linebuffer[x] & 0xF0)>> 4)) & 1))	\
        {	\
         int which_co = (ble_cache[pixel[pixpoo] >> 28] - 1);	\
         uint8 back_y = coeff_cache_y_back[which_co][(zeout >> 16) & 0xFF];	\
         int8 back_u = coeff_cache_u_back[which_co][(zeout >>  8) & 0xFF];	\
         int8 back_v = coeff_cache_v_back[which_co][(zeout >>  0) & 0xFF];	\
         uint8 fore_y = coeff_cache_y_fore[which_co][(pixel[pixpoo] >> 16) & 0xFF];	\
         int8 fore_u = coeff_cache_u_fore[which_co][(pixel[pixpoo] >>  8) & 0xFF];	\
         int8 fore_v = coeff_cache_v_fore[which_co][(pixel[pixpoo] >>  0) & 0xFF];	\
         zeout = (pixel[pixpoo] & 0xFF000000) | ((RGBDeflower + 384)[back_y + fore_y] << 16) | ((RGBDeflower + 384)[back_u + fore_u + 128] << 8) | ((RGBDeflower + 384)[back_v + fore_v + 128] << 0);	\
        } else zeout = pixel[pixpoo];	\

#define DOCELLOSPECIALFRONT() \
        {       \
         uint8 y = coeff_cache_y_back[0][(zeout >> 16) & 0xFF];    \
         int8 u = coeff_cache_u_back[0][(zeout >>  8) & 0xFF];    \
         int8 v = coeff_cache_v_back[0][(zeout >>  0) & 0xFF];    \
         zeout = (zeout & 0xFF000000) | ((RGBDeflower + 384)[CCR_Y_front + y] << 16) | ((RGBDeflower + 384)[CCR_U_front + u + 128] << 8) | 	\
			((RGBDeflower + 384)[CCR_V_front + v + 128] << 0); \
        }

#define	LAYER_MIX_BODY(index_256, index_341) \
      { uint32 pixel[4];	\
      uint32 prio[3];	\
      uint32 zeout = BPC_Cache;	\
      prio[0] = priority_remap[vdc_linebuffer_yuved[index_341] >> 28];  \
      prio[1] = priority_remap[(bg_linebuffer + 8)[index_256] >> 28];	\
      prio[2] = priority_remap[rainbow_linebuffer[index_256] >> 28];	\
      pixel[0] = zeout;	\
      pixel[1] = zeout;	\
      pixel[2] = zeout;	\
      {	\
       uint8 pi0 = happymap[prio[0]][prio[1]][prio[2]][0];	\
       uint8 pi1 = happymap[prio[0]][prio[1]][prio[2]][1];	\
       uint8 pi2 = happymap[prio[0]][prio[1]][prio[2]][2];	\
       if(pixel[pi0] & TRANS_OR) pixel[pi0] = vdc_linebuffer_yuved[index_341];  \
       if(pixel[pi1] & TRANS_OR) pixel[pi1] = (bg_linebuffer + 8)[index_256];	\
       if(pixel[pi2] & TRANS_OR) pixel[pi2] = rainbow_linebuffer[index_256];	\
      }

#define LAYER_MIX_FINAL_NOCELLO	\
       if(!(pixel[0] & TRANS_OR))	\
        zeout = pixel[0];	\
       if(!(pixel[1] & TRANS_OR))	\
         zeout = pixel[1];	\
       if(!(pixel[2] & TRANS_OR))	\
         zeout = pixel[2];	\
       target[x] = YUV888_TO_RGB888(zeout);	\
      }

#define LAYER_MIX_FINAL_BACK_CELLO     \
      if(!(pixel[0] & TRANS_OR))        \
      { \
       if(ble_cache[pixel[0] >> 28])     \
       {        \
        DOCELLO(0);     \
       }        \
       else     \
        zeout = pixel[0];       \
      } \
      if(!(pixel[1] & TRANS_OR))        \
      { \
       if(ble_cache[pixel[1] >> 28])     \
       {        \
        DOCELLO(1);     \
       }        \
       else     \
        zeout = pixel[1];       \
      } \
      if(!(pixel[2] & TRANS_OR))        \
      { \
       if(ble_cache[pixel[2] >> 28])     \
       {        \
        DOCELLO(2);     \
       }        \
       else     \
        zeout = pixel[2];       \
      } \
      target[x] = YUV888_TO_RGB888(zeout);      \
     }

#define LAYER_MIX_FINAL_FRONT_CELLO	\
      if(!(pixel[0] & TRANS_OR))	\
       zeout = pixel[0];	\
      if(!(pixel[1] & TRANS_OR))	\
      {	\
       if(ble_cache[pixel[1] >> 28] && !(zeout & TRANS_OR)) \
       {	\
        DOCELLO(1);	\
       }	\
       else	\
        zeout = pixel[1];	\
      }	\
      if(!(pixel[2] & TRANS_OR))	\
      {	\
       if(ble_cache[pixel[2] >> 28] && !(zeout & TRANS_OR))	\
       {	\
        DOCELLO(2);	\
       }	\
       else	\
        zeout = pixel[2];	\
      }	\
      DOCELLOSPECIALFRONT();	\
      target[x] = YUV888_TO_RGB888(zeout);	\
     }

#define LAYER_MIX_FINAL_CELLO	\
      if(!(pixel[0] & TRANS_OR))	\
       zeout = pixel[0];	\
      if(!(pixel[1] & TRANS_OR))	\
      {	\
       if(ble_cache[pixel[1] >> 28] && !(zeout & TRANS_OR)) 	\
       {	\
        DOCELLO(1);	\
       }	\
       else	\
        zeout = pixel[1];	\
      }	\
      if(!(pixel[2] & TRANS_OR))	\
      {	\
       if(ble_cache[pixel[2] >> 28] && !(zeout & TRANS_OR))	\
       {	\
        DOCELLO(2);	\
       }	\
       else	\
        zeout = pixel[2];	\
      }	\
      target[x] = YUV888_TO_RGB888(zeout);	\
     }

    if(fx_vce.picture_mode & 0x08) // No cellophane in 7.16MHz pixel mode
    {
     if(HighDotClockWidth == 341)
      for(unsigned int x = 0; x < 341; x++)
      {
       LAYER_MIX_BODY(x * 256 / 341, x);
       LAYER_MIX_FINAL_NOCELLO;
      }
     else if(HighDotClockWidth == 256)
      for(unsigned int x = 0; x < 256; x++)
      {
       LAYER_MIX_BODY(x, x * 341 / 256);
       LAYER_MIX_FINAL_NOCELLO;
      }
     else
      for(unsigned int x = 0; x < 1024; x++)
      {
       LAYER_MIX_BODY(x / 4, x / 3);
       LAYER_MIX_FINAL_NOCELLO;
      }
    }
    else if((fx_vce.BLE & 0xC000) == 0xC000) // Front cellophane
    {
     uint8 CCR_Y_front = fx_vce.coefficient_mul_table_y[(fx_vce.coefficients[0] >> 8) & 0xF][(fx_vce.CCR >> 8) & 0xFF];
     int8 CCR_U_front = fx_vce.coefficient_mul_table_uv[(fx_vce.coefficients[0] >> 4) & 0xF][(fx_vce.CCR & 0xF0)];
     int8 CCR_V_front = fx_vce.coefficient_mul_table_uv[(fx_vce.coefficients[0] >> 0) & 0xF][(fx_vce.CCR << 4) & 0xF0];

     BPC_Cache = 0x008080 | (7 << 28) | TRANS_OR;

     for(unsigned int x = 0; x < 256; x++)
     {
      LAYER_MIX_BODY(x, x);
      LAYER_MIX_FINAL_FRONT_CELLO;
     }
    }
    else if((fx_vce.BLE & 0xC000) == 0x4000) // Back cellophane
    {
     BPC_Cache = ((fx_vce.CCR & 0xFF00) << 8) | ((fx_vce.CCR & 0xF0) << 8) | ((fx_vce.CCR & 0x0F) << 4) | (7 << 28) | TRANS_OR;

     for(unsigned int x = 0; x < 256; x++)
     {
      LAYER_MIX_BODY(x, x);
      LAYER_MIX_FINAL_BACK_CELLO;
     }
    }
    else if(ble_cache_any)		     // No front/back cello, but cellophane on at least 1 layer
    {
     for(unsigned int x = 0; x < 256; x++)
     {
      LAYER_MIX_BODY(x, x);
      LAYER_MIX_FINAL_CELLO
     }
    }
    else				     // No cellophane at all
    {
     for(unsigned int x = 0; x < 256; x++)
     {
      LAYER_MIX_BODY(x, x);
      LAYER_MIX_FINAL_NOCELLO
     }
    }
   }
   MDFNGameInfo->DisplayRect.w = (width == 341) ? HighDotClockWidth : width;
   MDFNGameInfo->DisplayRect.x = 0;
   LineWidths[fx_vce.raster_counter - 22] = MDFNGameInfo->DisplayRect;
  }

  int hblank_run = 85 * 4;

  if(fx_vce.raster_counter == 5)
  {
   FXVDC_VSync(vdc_chips[0]);
   FXVDC_VSync(vdc_chips[1]);
  }

  v810_run(1365 - hblank_run);	// Run active display

  // HBlank here:

  FXVDC_DoLineHBlank(vdc_chips[0]);
  FXVDC_DoLineHBlank(vdc_chips[1]);

  fx_vce.in_hblank = TRUE;
  fx_vce.raster_counter = (fx_vce.raster_counter + 1) % 263;

  if(fx_vce.raster_counter == king->RasterIRQLine && (king->RAINBOWTransferControl & 0x2))
  {
   //printf("Wovely: %d, %d, %d\n", fx_vce.raster_counter, king->RAINBOWRasterCounter, king->RAINBOWTransferControl);
   king->RasterIRQPending = TRUE;
   RedoKINGIRQCheck();
  }

  v810_run(hblank_run);		// Run hblank
  fx_vce.in_hblank = FALSE;

  if(!fx_vce.raster_counter)
   break;
 }
}

void KING_SetPixelFormat(int rshift, int gshift, int bshift)
{
 RebuildUVLUT();
}

bool KING_ToggleLayer(int which)
{
 // "BG0\0BG1\0BG2\0BG3\0VDC-A BG\0VDC-A SPR\0VDC-B BG\0VDC-B SPR\0RAINBOW\0",
 if(which < 4)
 {
  BGLayerDisable ^= 1 << which;
  return( !((BGLayerDisable >> which) & 1));
 }
 else if(which == 4 || which == 5)
 {
  return(FXVDC_ToggleLayer(fx_vdc_chips[0], which - 4));
 }
 else if(which == 6 || which == 7)
 {
  return(FXVDC_ToggleLayer(fx_vdc_chips[1], which - 6));
 }
 else if(which == 8)
 {
  RAINBOWLayerDisable = !RAINBOWLayerDisable;
  return(!RAINBOWLayerDisable);
 }
 else
  return(0);
}


int KING_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT KINGStateRegs[] =
 {
  SFVARN(king->AR, "AR"),
  SFARRAY16N(king->KRAM[0], 262144, "KRAM0"),
  SFARRAY16N(king->KRAM[1], 262144, "KRAM1"),
  SFVARN(king->KRAMWA, "KRAMWA"),
  SFVARN(king->KRAMRA, "KRAMRA"),
  SFVARN(king->KRAM_Mode, "KRAM_Mode"),
  SFVARN(king->PageSetting, "PageSetting"),
  SFVARN(king->bgmode, "bgmode"),
  SFVARN(king->priority, "priority"),
  SFVARN(king->BGScrollMode, "BGScrollMode"),
  SFARRAY16N(king->BGSize, 4, "BGSize"),
  SFARRAY16N(king->BGBATAddr, 4, "BGBATAddr"),
  SFARRAY16N(king->BGCGAddr, 4, "BGCGAddr"),
  SFARRAY16N(king->BGXScroll, 4, "BGXScroll"),
  SFARRAY16N(king->BGYScroll, 4, "BGYScroll"),
  SFVARN(king->BG0SubBATAddr, "BG0SubBATAddr"),
  SFVARN(king->BG0SubCGAddr, "BG0SubCGAddr"),
  SFVARN(king->BGAffinA, "BGAffinA"),
  SFVARN(king->BGAffinB, "BGAffinB"),
  SFVARN(king->BGAffinC, "BGAffinC"),
  SFVARN(king->BGAffinD, "BGAffinD"),
  SFVARN(king->BGAffinCenterX, "BGAffinCenterX"),
  SFVARN(king->BGAffinCenterY, "BGAffinCenterY"),

  SFVARN(king->ADPCMControl, "ADPCMControl"),
  SFARRAY16N(king->ADPCMBufferMode, 2, "ADPCMBufferMode"),
  SFARRAY16N(king->ADPCMSAL, 2, "ADPCMSAL"),
  SFARRAY32N(king->ADPCMEndAddress, 2, "ADPCMEndAddress"),
  SFARRAY32N(king->ADPCMPlayAddress, 2, "ADPCMPlayAddress"),
  SFARRAY16N(king->ADPCMIntermediateAddress, 2, "ADPCMIntermediateAddress"),
  SFARRAY16N(king->ADPCMStatus, 2, "ADPCMStatus"),
  SFVARN(king->RAINBOWTransferControl, "RAINBOWTransferControl"),
  SFVARN(king->RAINBOWKRAMA, "RAINBOWKRAMA"),
  SFVARN(king->RAINBOWTransferStartPosition, "RAINBOWTransferStartPosition"),
  SFVARN(king->RAINBOWTransferBlockCount, "RAINBOWTransferBlockCount"),
  SFVARN(king->RasterIRQLine, "RasterIRQLine"),
  SFVARN(king->RasterIRQPending, "RasterIRQPending"),
  SFVARN(king->RAINBOWKRAMReadPos, "RAINBOWKRAMReadPos"),
  SFVARN(king->DMATransferFlipFlop, "DMATransferFlipFlop"),
  SFVARN(king->DMATransferAddr, "DMATransferAddr"),
  SFVARN(king->DMATransferSize, "DMATransferSize"),
  SFVARN(king->DMAStatus, "DMAStatus"),
  SFVARN(king->DMAInterrupt, "DMAInterrupt"),
  SFVARN(king->DMALatch, "DMALatch"),
  SFVARN(king->MPROGControl, "MPROGControl"),
  SFVARN(king->MPROGAddress, "MPROGAddress"),
  SFARRAY16N(king->MPROGData, 16, "MPROGData"),
  SFVARN(king->Reg01, "Port01"),
  SFVARN(king->Reg02, "Port02"),
  SFVARN(king->Reg03, "Port03"),
  SFVARN(king->CDInterrupt, "CDInterrupt"),
  SFVARN(king->data_cache, "data_cache"),
  SFVARN(king->DRQ, "DRQ"),
  SFVARN(king->dma_receive_active, "dma_receive_active"),
  SFVARN(king->dma_send_active, "dma_send_active"),
  SFVARN(king->dma_cycle_counter, "dma_cycle_counter"),

  SFVARN(king->SubChannelBuf, "SubChannelBuf"),
  SFVARN(king->SubChannelInterrupt, "SubChannelInterrupt"),
  SFVARN(king->SubChannelControl, "SubChannelControl"),

  SFEND
 };

 SFORMAT VCEStateRegs[] =
 {
  SFVARN(fx_vce.AR, "AR"),
  SFARRAY16N(fx_vce.priority, 2, "priority"),
  SFVARN(fx_vce.odd_field, "odd_field"),
  SFVARN(fx_vce.in_hblank, "in_hblank"),
  SFVARN(fx_vce.picture_mode, "picture_mode"),
  SFVARN(fx_vce.raster_counter, "raster_counter"),
  SFVARN(fx_vce.palette_rw_offset, "palette_rw_offset"),
  SFVARN(fx_vce.palette_rw_latch, "palette_rw_latch"),
  SFARRAY16N(fx_vce.palette_offset, 4, "palette_offset"),
  SFARRAY16N(fx_vce.palette_table, 512, "palette_table"),
  SFVARN(fx_vce.ChromaKeyY, "ChromaKeyY"),
  SFVARN(fx_vce.ChromaKeyU, "ChromaKeyU"),
  SFVARN(fx_vce.ChromaKeyV, "ChromaKeyV"),
  SFVARN(fx_vce.CCR, "CCR"),
  SFVARN(fx_vce.BLE, "BLE"),
  SFVARN(fx_vce.SPBL, "SPBL"),
  SFARRAY16N(fx_vce.coefficients, 6, "coefficients"),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, KINGStateRegs, "KING");
 
 ret &= MDFNSS_StateAction(sm, load, data_only, VCEStateRegs, "VCE");

 if(load)
 {
  fx_vce.palette_rw_offset &= 0x1FF;
  fx_vce.palette_offset[3] &= 0x00FF;
  fx_vce.priority[0] &= 0x0777;
  fx_vce.priority[1] &= 0x7777;

  for(int x = 0; x < 6; x++)
   fx_vce.coefficients[x] &= 0x0FFF;

  RebuildLayerMaskCache();
  for(int x = 0; x < 0x200; x++)
   RedoPaletteCache(x);

  RecalcKRAM_Mask();
  RedoKINGIRQCheck();
  FXVDC_SetDotClock(fx_vdc_chips[0], (fx_vce.picture_mode & 0x08) ? 1 : 0);
  FXVDC_SetDotClock(fx_vdc_chips[1], (fx_vce.picture_mode & 0x08) ? 1 : 0);
  SoundBox_SetKINGADPCMControl(king->ADPCMControl);

  // FIXME:  Should we just go ahead and save pending events in save states in v810_cpu.cpp?
  if(king->dma_receive_active || king->dma_send_active)
   v810_setevent(V810_EVENT_KING, king->dma_cycle_counter);
 }
 return(ret);
}

#ifdef WANT_DEBUGGER
void KING_SetRegister(const std::string &name, uint32 value)
{

 if(name == "AR")
  king->AR = value & 0x7F;
 else if(name == "MPROGADDR")
  king->MPROGAddress = value & 0xF;
 else if(name == "MPROGCTRL")
  king->MPROGControl = value & 0x1;
 else if(name == "MPROG0" || name == "MPROG1" || name == "MPROG2" || name == "MPROG3" || name == "MPROG4" || name == "MPROG5" || name == "MPROG6" || name == "MPROG7" ||
         name == "MPROG8" || name == "MPROG9" || name == "MPROGA" || name == "MPROGB" || name == "MPROGC" || name == "MPROGD" || name == "MPROGE" || name == "MPROGF")
 {
  int which;
  if(name[5] >= 'A' && name[5] <= 'F')
   which = name[5] + 0xA - 'A';
  else
   which = name[5] - '0';
  king->MPROGData[which] = value;
 }
 else if(name == "PAGESET")
 {
  king->PageSetting = value;
 }
 else if(name == "BGMODE")
 {
  king->bgmode = value;
 }
 else if(name == "BGPRIO")
 {
  king->priority = value;
 }
 else if(name == "BGSCRM")
 {
  king->BGScrollMode = value;
 }
 else if(name == "BGSIZ0")
  king->BGSize[0] = value;
 else if(name == "BGSIZ1")
  king->BGSize[1] = value & 0x00FF;
 else if(name == "BGSIZ2")
  king->BGSize[2] = value & 0x00FF;
 else if(name == "BGSIZ3")
  king->BGSize[3] = value & 0x00FF;
 else if(name == "BGXSC0")
  king->BGXScroll[0] = value & 0x7FF;
 else if(name == "BGXSC1")
  king->BGXScroll[1] = value & 0x3FF;
 else if(name == "BGXSC2")
  king->BGXScroll[2] = value & 0x3FF;
 else if(name == "BGXSC3")
  king->BGXScroll[3] = value & 0x3FF;
 else if(name == "BGYSC0")
  king->BGYScroll[0] = value & 0x7FF;
 else if(name == "BGYSC1")
  king->BGYScroll[1] = value & 0x3FF;
 else if(name == "BGYSC2")
  king->BGYScroll[2] = value & 0x3FF;
 else if(name == "BGYSC3")
  king->BGYScroll[3] = value & 0x3FF;
 else if(name == "BGAFFINA")
  king->BGAffinA = value;
 else if(name == "BGAFFINB")
  king->BGAffinB = value;
 else if(name == "BGAFFINC")
  king->BGAffinC = value;
 else if(name == "BGAFFIND")
  king->BGAffinD = value;
 else if(name == "BGAFFINX")
  king->BGAffinCenterX = value;
 else if(name == "BGAFFINY")
  king->BGAffinCenterY = value;
 else if(name ==  "BGBAT0")
  king->BGBATAddr[0] = value & 0x1FF;
 else if(name ==  "BGBATS")
  king->BG0SubBATAddr = value & 0x1FF;
 else if(name ==  "BGBAT1")
  king->BGBATAddr[1] = value & 0x1FF;
 else if(name ==  "BGBAT2")
  king->BGBATAddr[2] = value & 0x1FF;
 else if(name ==  "BGBAT3")
  king->BGBATAddr[3] = value & 0x1FF;
 else if(name == "BGCG0" || name == "BGCGS" || name == "BGCG1" || name == "BGCG2" || name == "BGCG3")
 {
  char which = name[4];
  if(which == 'S')
   king->BG0SubCGAddr = value & 0x1FF;
  else
   king->BGCGAddr[which - '0'] = value & 0x1FF;
 }
 else if(name == "RTCTRL")
  king->RAINBOWTransferControl = value & 0x3;
 else if(name == "RKRAMA")
  king->RAINBOWKRAMA = value & 0x3FFFF;
 else if(name == "RSTART")
  king->RAINBOWTransferStartPosition = value & 0x1FF;
 else if(name == "RCOUNT")
  king->RAINBOWTransferBlockCount = value & 0x1F;
 else if(name == "RIRQLINE")
  king->RasterIRQLine = value;
 else if(name == "KRAMRA")
 {
  king->KRAMRA = value;
 }
 else if(name == "KRAMWA")
 {
  king->KRAMWA = value;
 }
 else if(name == "DMATA")
  king->DMATransferAddr = value;
 else if(name == "DMATS")
  king->DMATransferSize = value;
 else if(name == "DMASTT")
  king->DMAStatus = value;
 else if(name == "ADPCMCTRL")
 {
  king->ADPCMControl = value;
 }
 else if(name == "ADPCMBM0" || name == "ADPCMBM1")
 {
  unsigned int which = name[7] - '0';
  king->ADPCMBufferMode[which] = value;
 }
 else if(name == "ADPCMPA0" || name == "ADPCMPA1")
 {
  unsigned int which = name[7] - '0';
  king->ADPCMPlayAddress[which] = value;
 }
 else if(name == "ADPCMSA0" || name == "ADPCMSA1")
 {
  unsigned int which = name[7] - '0';
  king->ADPCMSAL[which] = value;
 }
 else if(name == "ADPCMIA0" || name == "ADPCMIA1")
 {
  unsigned int which = name[7] - '0';
  king->ADPCMIntermediateAddress[which] = value;
 }
 else if(name == "ADPCMEA0" || name == "ADPCMEA1")
 {
  unsigned int which = name[7] - '0';
  king->ADPCMEndAddress[which] = value;
 }
 else if(name == "Reg01")
 {
  king->Reg01 = value;
 }
 else if(name == "Reg02")
 {
  king->Reg02 = value;
 }
 else if(name == "Reg03")
 {
  king->Reg03 = value;
 }
 else if(name == "SUBCC")
 {
  king->SubChannelControl = value & 0x3;
 }
}

uint32 KING_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name == "AR")
 {
  value = king->AR;  
 }
 else if(name == "MPROGADDR")
  value = king->MPROGAddress;
 else if(name == "MPROGCTRL")
 {
  value = king->MPROGControl;
 }
 else if(name == "MPROG0" || name == "MPROG1" || name == "MPROG2" || name == "MPROG3" || name == "MPROG4" || name == "MPROG5" || name == "MPROG6" || name == "MPROG7" ||
 	 name == "MPROG8" || name == "MPROG9" || name == "MPROGA" || name == "MPROGB" || name == "MPROGC" || name == "MPROGD" || name == "MPROGE" || name == "MPROGF")
 {
  int which;
  if(name[5] >= 'A' && name[5] <= 'F') 
   which = name[5] + 0xA - 'A';
  else
   which = name[5] - '0';
  value = king->MPROGData[which];
  if(special)
  {
   char buf[256];
   static const char *atypes[4] = { "CG", "CG of BAT", "BAT", "???" };

   snprintf(buf, 256, "Offset: %d, Access Type: %s, Rotation: %d, BG Number: %d, NOP: %d",
        value & 0x7, atypes[(value >> 3) & 0x3], (int)(bool)(value & 0x20),
        (value >> 6) & 0x3, (int)(bool)(value & 0x100));

   *special = std::string(buf);
  }

 }
 else if(name == "PAGESET")
 {
  value = king->PageSetting;

  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "SCSI: %d, BG: %d, ADPCM: %d, RAINBOW: %d", (int)(bool)(value & 0x1), (int)(bool)(value & 0x10), (int)(bool)(value & 0x100), (int)(bool)(value & 0x1000));
   *special = std::string(buf);
  }
 }
 else if(name == "BGMODE")
 {
  value = king->bgmode;
  if(special)
  {
   static const char *bgmodes[16] = { "Disabled", "4-color", "16-color", "256-color", "64K-color", "16M-color", "Undefined", "Undefined",
				      "Disabled", "4-color w/BAT", "16-color w/BAT", "256-color w/BAT", "64K-color w/BAT", "16M-color w/BAT", "Undefined", "Undefined"
				    };
   char buf[256];
   snprintf(buf, 256, "BG0: %2d(%s), BG1: %2d(%s), BG2: %2d(%s), BG3: %2d(%s)", value & 0xF, bgmodes[value & 0xF], (value >> 4) & 0xF, bgmodes[(value >> 4) & 0xF], (value >> 8) & 0xF, bgmodes[(value >> 8) & 0xf], (value >> 12) & 0xF, bgmodes[(value >> 12) & 0xf]);
   *special = std::string(buf);
  }
 }
 else if(name == "BGPRIO")
 {
  value = king->priority;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "BG0: %2d, BG1: %2d, BG2: %2d, BG3: %2d", value & 0x7, (value >> 3) & 0x7, (value >> 6) & 0x7, (value >> 9) & 0x7);
   *special = std::string(buf);
  }
 }
 else if(name == "BGSCRM")
 {
  value = king->BGScrollMode;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "BG0: %s, BG1: %s, BG2: %s, BG3: %s", (value & 1) ? "Endless" : "Non-endless",(value & 2) ? "Endless" : "Non-endless",
	(value & 4) ? "Endless" : "Non-endless", (value & 8) ? "Endless" : "Non-endless");
   *special = std::string(buf);
  }
 }
 else if(name == "BGSIZ0")
  value = king->BGSize[0];

 else if(name == "BGSIZ1")
  value = king->BGSize[1];

 else if(name == "BGSIZ2")
  value = king->BGSize[2];

 else if(name == "BGSIZ3")
  value = king->BGSize[3];

 else if(name == "BGXSC0")
  value = king->BGXScroll[0];
 else if(name == "BGXSC1")
  value = king->BGXScroll[1];
 else if(name == "BGXSC2")
  value = king->BGXScroll[2];
 else if(name == "BGXSC3")
  value = king->BGXScroll[3];

 else if(name == "BGYSC0")
  value = king->BGYScroll[0];
 else if(name == "BGYSC1")
  value = king->BGYScroll[1];
 else if(name == "BGYSC2")
  value = king->BGYScroll[2];
 else if(name == "BGYSC3")
  value = king->BGYScroll[3];
 else if(name == "BGAFFINA")
  value = king->BGAffinA;
 else if(name == "BGAFFINB")
  value = king->BGAffinB;
 else if(name == "BGAFFINC")
  value = king->BGAffinC;
 else if(name == "BGAFFIND")
  value = king->BGAffinD;
 else if(name == "BGAFFINX")
  value = king->BGAffinCenterX;
 else if(name == "BGAFFINY")
  value = king->BGAffinCenterY;



 else if(name ==  "BGBAT0")
  value = king->BGBATAddr[0];
 else if(name ==  "BGBATS")
  value = king->BG0SubBATAddr;
 else if(name ==  "BGBAT1")
  value = king->BGBATAddr[1];
 else if(name ==  "BGBAT2")
  value = king->BGBATAddr[2];
 else if(name ==  "BGBAT3")
  value = king->BGBATAddr[3];
 else if(name == "BGCG0" || name == "BGCGS" || name == "BGCG1" || name == "BGCG2" || name == "BGCG3")
 {
  char which = name[4];
  if(which == 'S')
   value = king->BG0SubCGAddr;
  else
   value = king->BGCGAddr[which - '0'];
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "0x%04x * 1024 = 0x%05x", value, (value * 1024) & 0x3FFFF);
   *special = std::string(buf);
  }
 }

 else if(name == "RTCTRL")
 {
  value = king->RAINBOWTransferControl;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "Raster Interrupt: %s, Rainbow Transfer: %s", (value & 2) ? "On" : "Off", (value & 1) ? "On" : "Off");
   *special = std::string(buf);
  }
 }
 else if(name == "RKRAMA")
  value = king->RAINBOWKRAMA;
 else if(name == "RSTART")
  value = king->RAINBOWTransferStartPosition;
 else if(name == "RCOUNT")
  value = king->RAINBOWTransferBlockCount;
 else if(name == "RIRQLINE")
  value = king->RasterIRQLine;
 else if(name == "KRAMRA")
 {
  value = king->KRAMRA;
 }
 else if(name == "KRAMWA")
 {
  value = king->KRAMWA;
 }
 else if(name == "DMATA")
  value = king->DMATransferAddr;
 else if(name == "DMATS")
  value = king->DMATransferSize;
 else if(name == "DMASTT")
  value = king->DMAStatus;
 else if(name == "ADPCMCTRL")
 {
  value = king->ADPCMControl;

 }
 else if(name == "ADPCMBM0" || name == "ADPCMBM1")
 {
  unsigned int which = name[7] - '0';
  value = king->ADPCMBufferMode[which];

 }
 else if(name == "ADPCMPA0" || name == "ADPCMPA1")
 {
  unsigned int which = name[7] - '0';
  value = king->ADPCMPlayAddress[which];

 }
 else if(name == "ADPCMSA0" || name == "ADPCMSA1")
 {
  unsigned int which = name[7] - '0';
  value = king->ADPCMSAL[which];

 }
 else if(name == "ADPCMIA0" || name == "ADPCMIA1")
 {
  unsigned int which = name[7] - '0';
  value = king->ADPCMIntermediateAddress[which];

 }
 else if(name == "ADPCMEA0" || name == "ADPCMEA1")
 {
  unsigned int which = name[7] - '0';
  value = king->ADPCMEndAddress[which];

 }
 else if(name == "Reg01")
 {
  value = king->Reg01;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "BSY: %d, ATN: %d, SEL: %d, ACK: %d, RST: %d", (int)(bool)(value & 1), (int)(bool)(value & 2), (int)(bool)(value & 4),
	(int)(bool)(value & 0x10), (int)(bool)(value & 0x80));
   *special = std::string(buf);
  }
 }
 else if(name == "Reg02")
 {
  value = king->Reg02;
 }
 else if(name == "Reg03")
 {
  value = king->Reg03;
  if(special)
  {
   char buf[256];

   snprintf(buf, 256, "I/O: %d, C/D: %d, MSG: %d", (int)(bool)(value & 1), (int)(bool)(value & 2), (int)(bool)(value & 4));
   *special = std::string(buf);
  }
 }
 else if(name == "SUBCC")
 {
  value = king->SubChannelControl;
  if(special)
  {
   char buf[256];

   snprintf(buf, 256, "Subchannel reading: %s, Subchannel read IRQ: %s", (value & 0x1) ? "Enabled" : "Disabled", (value & 0x2) ? "On" : "Off");
   *special = std::string(buf);
  }
 }
 else if(name == "BSY")
  value = SCSICD_GetBSY();
 else if(name == "REQ")
  value = SCSICD_GetREQ();
 else if(name == "ACK")
  value = SCSICD_GetACK();
 else if(name == "MSG")
  value = SCSICD_GetMSG();
 else if(name == "CD")
  value = SCSICD_GetCD();
 else if(name == "IO")
  value = SCSICD_GetIO();
 else if(name == "SEL")
  value = SCSICD_GetSEL();

 return(value);
}

void FXVDCVCE_SetRegister(const std::string &name, uint32 value)
{
 if(name == "VCEPRIO0")
 {
  fx_vce.priority[0] = value & 0x0777;
 }
 else if(name == "VCEPRIO1")
 {
  fx_vce.priority[1] = value & 0x7777;
 }
 else if(name == "VCEPICMODE")
 {
  fx_vce.picture_mode = value;
  RebuildLayerMaskCache();
  FXVDC_SetDotClock(fx_vdc_chips[0], (fx_vce.picture_mode & 0x08) ? 1 : 0);
  FXVDC_SetDotClock(fx_vdc_chips[1], (fx_vce.picture_mode & 0x08) ? 1 : 0);
 }
 else if(name == "VCEPALRWOF")
 {
  fx_vce.palette_rw_offset = value;
  fx_vce.palette_rw_offset &= 0x1FF;
 }
 else if(name == "VCEPALRWLA")
  fx_vce.palette_rw_latch = value;
 else if(name == "VCEPALOFS0")
  fx_vce.palette_offset[0] = value;
 else if(name == "VCEPALOFS1")
  fx_vce.palette_offset[1] = value;
 else if(name == "VCEPALOFS2")
  fx_vce.palette_offset[2] = value;
 else if(name == "VCEPALOFS3")
 {
  fx_vce.palette_offset[3] = value;
  fx_vce.palette_offset[3] &= 0x00FF;
 }
 else if(name == "VCECCR")
  fx_vce.CCR = value;
 else if(name == "VCEBLE")
  fx_vce.BLE = value;
 else if(name == "VCESPBL")
  fx_vce.SPBL = value;
 else if(!strncasecmp(name.c_str(), "VCECOEFF", strlen("VCECOEFF")))
 {
  int which = name.c_str()[strlen("VCECOEFF")] - '0';
  fx_vce.coefficients[which] = value;
 }
 else if(name[name.size() - 2] == '-')
 {
  std::string neoname = name.substr(0, name.size() - 2);
  int wv = (name[name.size() - 1] == 'A') ? 0 : 1;

  FXVDC_SetRegister(fx_vdc_chips[wv], neoname, value);
 }
}


uint32 FXVDCVCE_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0;

 if(name == "VCEPRIO0")
 {
  value = fx_vce.priority[0];
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "VDC BG: %2d, VDC SPR: %2d, RAINBOW: %2d", value & 0xF, (value >> 4) & 0xF, (value >> 8) & 0xF);
   *special = std::string(buf);
  }
 }
 else if(name == "VCEPRIO1")
 {
  value = fx_vce.priority[1];
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "BG0: %2d, BG1: %2d, BG2: %2d, BG3: %2d", value & 0xF, (value >> 4) & 0xF, (value >> 8) & 0xF, (value >> 12) & 0xF);
   *special = std::string(buf);
  }
 }
 else if(name == "VCEPICMODE")
 {
  value = fx_vce.picture_mode;
  if(special)
  {
   char buf[256];
   static const char *DCCModes[4] =
   { 
    "263 lines/frame", "262 lines/frame", "Interlaced", "Interlaced+1/2 dot shift" 
   };

   snprintf(buf, 256, "BG0: %s, BG1: %s, BG2: %s, BG3: %s, VDC BG: %s%s, VDC SPR: %s%s, RAINBOW: %s, VDC Clk: %sMHz, %s",
	(value & (1 << 10)) ? "On" : "Off", (value & (1 << 11)) ? "On": "Off",
        (value & (1 << 12)) ? "On" : "Off", (value & (1 << 13)) ? "On": "Off",
	(value & 0x0100) ? "On" : "Off", (value & 0x0040) ? "+merge mode" : "", (value & 0x0200) ? "On" : "Off", (value & 0x0080) ? "+merge mode" : "",
	(value & 0x4000) ? "On" : "Off", (value & 0x0008) ? "7.16":"5.37", DCCModes[value & 0x3]);
   *special = std::string(buf);
  }
 }
 else if(name == "Frame Cntr")
  value = fx_vce.raster_counter;
 else if(name == "VCEPALRWOF")
  value = fx_vce.palette_rw_offset;
 else if(name == "VCEPALRWLA")
  value = fx_vce.palette_rw_latch;
 else if(name == "VCEPALOFS0")
  value = fx_vce.palette_offset[0];
 else if(name == "VCEPALOFS1")
  value = fx_vce.palette_offset[1];
 else if(name == "VCEPALOFS2")
  value = fx_vce.palette_offset[2];
 else if(name == "VCEPALOFS3")
  value = fx_vce.palette_offset[3];
 else if(name == "ChromaKeyY")
  value = fx_vce.ChromaKeyY;
 else if(name == "ChromaKeyU")
  value = fx_vce.ChromaKeyU;
 else if(name == "ChromaKeyV")
  value = fx_vce.ChromaKeyV;

 else if(name == "VCECCR")
  value = fx_vce.CCR;
 else if(name == "VCEBLE")
 {
  value = fx_vce.BLE;
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "%s(%s), Rainbow: %d, BG3: %d, BG2: %d, BG1: %d, BG0: %d, VDC SP: %d, VDC BG: %d", (value & 0x8000) ? "Front" : "Back", (value & 0x4000) ? "On" : "Off", (value >> 12) & 0x3,
		(value >> 10) & 3, (value >> 8) & 3, (value >> 6) & 3, (value >> 4) & 3, (value >> 2) & 3, value & 3);
   *special = std::string(buf);
  }
 }
 else if(name == "VCESPBL")
  value = fx_vce.SPBL;
 else if(!strncasecmp(name.c_str(), "VCECOEFF", strlen("VCECOEFF")))
 {
  int which = name.c_str()[strlen("VCECOEFF")] - '0';
  value = fx_vce.coefficients[which];
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "Y: %1d, U: %1d, V: %1d", (value >> 8) & 0xF, (value >> 4) & 0xf, value & 0xf);
   *special = std::string(buf);
  }
 } 
 else if(name[name.size() - 2] == '-')
 {
  std::string neoname = name.substr(0, name.size() - 2);
  int wv = (name[name.size() - 1] == 'A') ? 0 : 1;

  value = FXVDC_GetRegister(fx_vdc_chips[wv], neoname, special);
 }

 return(value);
}

void KING_SetGraphicsDecode(int line, int which, int w, int h, int xscroll, int yscroll, int pbn)
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
 GfxDecode_PBN = pbn;

 if(GfxDecode_Line == 0xB00B13)
  DoGfxDecode();
}

uint32 *KING_GetGraphicsDecodeBuffer(void)
{
 return(GfxDecode_Buf);
}

static void DoGfxDecode(void)
{
 int pbn = GfxDecode_PBN;

 if(GfxDecode_Layer >= 4 && GfxDecode_Layer <= 7)
 {
  uint32 palette_offset = fx_vce.palette_offset[0] >> (((GfxDecode_Layer - 4) & 1) * 8);
  palette_offset <<= 1;
  palette_offset &= 0x1FF;
  uint32 *palette_ptr = &fx_vce.palette_table_cache[palette_offset];
  uint32 neo_palette[16];

  pbn &= 0x0F;
  palette_ptr += pbn * 16;

  for(int x = 0; x < 16; x++) neo_palette[x] = YUV888_TO_RGB888(palette_ptr[x]) | MK_COLORA(0, 0, 0, 0xFF);
  FXVDC_DoGfxDecode(fx_vdc_chips[(GfxDecode_Layer - 4) / 2], neo_palette, GfxDecode_Buf, GfxDecode_Width, GfxDecode_Height, GfxDecode_Scroll, (GfxDecode_Layer - 4) & 1);
 }
 else if(GfxDecode_Layer < 4)
 {
  int n = GfxDecode_Layer;
  uint16 bgmode = (king->bgmode >> (n * 4)) & 0xF;
  uint32 cg_offset = king->BGCGAddr[n] * 1024;
  uint32 bat_and_cg_bank = (king->PageSetting & 0x0010) ? 1 : 0;
  uint32 *target = GfxDecode_Buf;
  uint32 layer_or = 0;
  uint32 page_addr_or = bat_and_cg_bank ? 0x40000 : 0x00000;
  uint32 palette_offset = fx_vce.palette_offset[1 + (n >> 1)] >> ((n & 1) ? 8 : 0);
  palette_offset <<= 1;
  palette_offset &= 0x1FF;
  uint32 *palette_ptr = &fx_vce.palette_table_cache[palette_offset];
  int tile_width = 8;
  int tile_height = 8;

  if(!(bgmode & 0x8))
  {
   int shiftmoo = (king->BGSize[n] & 0xF0) >> 4;
   if(shiftmoo < 3) shiftmoo = 3;
   if(shiftmoo > 0xA) shiftmoo = 0xA;
   tile_width = 1 << shiftmoo;

   shiftmoo = (king->BGSize[n] & 0x0F);
   if(shiftmoo < 3) shiftmoo = 3;
   if(shiftmoo > 0xA) shiftmoo = 0xA;
   tile_height = 1 << shiftmoo;

   if(tile_width > GfxDecode_Width) tile_width = GfxDecode_Width;
   if(tile_height > GfxDecode_Height) tile_height = GfxDecode_Height;
  }

  switch(bgmode & 0x7)
  {
   default: memset(target, 0, GfxDecode_Width * GfxDecode_Height * sizeof(uint32) * 3); break;
   case 0x01: // 4
   {
    pbn *= 4;
    pbn &= 0x1FF;

    for(int y = 0; y < GfxDecode_Height; y++)
    {
     for(int x = 0; x < GfxDecode_Width; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Width / 8);
      uint16 cg = king->KRAM[bat_and_cg_bank][(cg_offset + (which_tile * 8) + (y & 0x7)) & king->KRAM_Mask_Full];

      DRAWBG8x1_4(target + x, cg, palette_ptr + pbn, layer_or);

      target[x + GfxDecode_Width*2 + 0] = target[x + GfxDecode_Width*2 + 1] = target[x + GfxDecode_Width*2 + 2] = target[x + GfxDecode_Width*2 + 3] =
      target[x + GfxDecode_Width*2 + 4] = target[x + GfxDecode_Width*2 + 5] = target[x + GfxDecode_Width*2 + 6] = target[x + GfxDecode_Width*2 + 7] = ((cg_offset + (which_tile * 8)) & 0x3FFFF) | page_addr_or;

      target[x + GfxDecode_Width*1 + 0]=target[x + GfxDecode_Width*1 + 1]=target[x + GfxDecode_Width*1 + 2]=target[x + GfxDecode_Width*1 + 3] =
      target[x + GfxDecode_Width*1 + 4]=target[x + GfxDecode_Width*1 + 5]=target[x + GfxDecode_Width*1 + 6]=target[x + GfxDecode_Width*1 + 7] = which_tile;
     }
     for(int x = 0; x < GfxDecode_Width; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | MK_COLORA(0, 0, 0, 0xFF);
     target += GfxDecode_Width * 3;
    }
   }
   break;

   case 0x02: // 16
   {
    pbn *= 8;
    pbn &= 0x1FF;
    for(int y = 0; y < GfxDecode_Height; y++)
    {
     for(int x = 0; x < GfxDecode_Width; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Width / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_bank][(cg_offset + (which_tile * 16) + (y & 0x7) * 2) & king->KRAM_Mask_Full];

      DRAWBG8x1_16(target + x, cgptr, palette_ptr + pbn, layer_or);

      target[x + GfxDecode_Width*2 + 0] = target[x + GfxDecode_Width*2 + 1] = target[x + GfxDecode_Width*2 + 2] = target[x + GfxDecode_Width*2 + 3] =
      target[x + GfxDecode_Width*2 + 4] = target[x + GfxDecode_Width*2 + 5] = target[x + GfxDecode_Width*2 + 6] = target[x + GfxDecode_Width*2 + 7] = ((cg_offset + (which_tile * 16)) & 0x3FFFF) | page_addr_or;

      target[x + GfxDecode_Width*1 + 0]=target[x + GfxDecode_Width*1 + 1]=target[x + GfxDecode_Width*1 + 2]=target[x + GfxDecode_Width*1 + 3] =
      target[x + GfxDecode_Width*1 + 4]=target[x + GfxDecode_Width*1 + 5]=target[x + GfxDecode_Width*1 + 6]=target[x + GfxDecode_Width*1 + 7] = which_tile;
     }
     for(int x = 0; x < GfxDecode_Width; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | MK_COLORA(0, 0, 0, 0xFF);
     target += GfxDecode_Width * 3;
    }
   }
   break;

   case 0x03: // 256
   {
    for(int y = 0; y < GfxDecode_Height; y++)
    {
     for(int x = 0; x < GfxDecode_Width; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Width / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_bank][(cg_offset + (which_tile * 32) + (y & 0x7) * 4) & king->KRAM_Mask_Full];

      DRAWBG8x1_256(target + x, cgptr, palette_ptr, layer_or);

      target[x + GfxDecode_Width*2 + 0] = target[x + GfxDecode_Width*2 + 1] = target[x + GfxDecode_Width*2 + 2] = target[x + GfxDecode_Width*2 + 3] =
      target[x + GfxDecode_Width*2 + 4] = target[x + GfxDecode_Width*2 + 5] = target[x + GfxDecode_Width*2 + 6] = target[x + GfxDecode_Width*2 + 7] = ((cg_offset + (which_tile * 32)) & 0x3FFFF) | page_addr_or;

      target[x + GfxDecode_Width*1 + 0]=target[x + GfxDecode_Width*1 + 1]=target[x + GfxDecode_Width*1 + 2]=target[x + GfxDecode_Width*1 + 3] =
      target[x + GfxDecode_Width*1 + 4]=target[x + GfxDecode_Width*1 + 5]=target[x + GfxDecode_Width*1 + 6]=target[x + GfxDecode_Width*1 + 7] = which_tile;
     }

     for(int x = 0; x < GfxDecode_Width; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | MK_COLORA(0, 0, 0, 0xFF);

     target += GfxDecode_Width * 3;
    }
   }
   break;

   case 0x04: // 64K
    for(int y = 0; y < GfxDecode_Height; y++)
    {
     for(int x = 0; x < GfxDecode_Width; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Width / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_bank][(cg_offset + (which_tile * 64) + (y & 0x7) * 8) & king->KRAM_Mask_Full];

      DRAWBG8x1_64K(target + x, cgptr, palette_ptr, layer_or);
     }

     for(int x = 0; x < GfxDecode_Width; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | MK_COLORA(0, 0, 0, 0xFF);

     target += GfxDecode_Width * 3;
    }
   break;

   case 0x05: // 16M
    for(int y = 0; y < GfxDecode_Height; y++)
    {
     for(int x = 0; x < GfxDecode_Width; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Width / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_bank][(cg_offset + (which_tile * 64) + (y & 0x7) * 8) & king->KRAM_Mask_Full];

      DRAWBG8x1_16M(target + x, cgptr, palette_ptr, layer_or);
     }

     for(int x = 0; x < GfxDecode_Width; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | MK_COLORA(0, 0, 0, 0xFF);

     target += GfxDecode_Width * 3;
    }
   break;

  }
 }
 else
  memset(GfxDecode_Buf, 0, GfxDecode_Width * GfxDecode_Height * sizeof(uint32) * 3);
}

#endif
