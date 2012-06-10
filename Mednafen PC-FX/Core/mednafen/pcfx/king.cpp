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
/* Note: Some technical comments may be outdated */

/*
 Current issues:
	VCE "natural" priorities for the layers when their priorities are the same(technically an illegal condition) are probably not correct.  A game test case: "Miraculum" erroneously sets
	up the priority	registers like this after exiting the airship(I believe).

	SCSI RST interrupt timing is guessed(and with nothing to go on), so it definitely needs to be tested on the real thing.

	The data bus is not handled/asserted properly.  Excluding pseudo-DMA and DMA modes(which I'd need to test), the data bus will only be asserted if the lower bit of register 0x1 is set, and the
	phase match bits must match the state of the C/D, I/O, and MSG signals(IE there isn't a bus mismatch state).

	Raw subchannel reading timing is probably wrong.

	KRAM mode register is not emulated(I'm not even sure what it does exactly).
*/

#include "pcfx.h"
#include "king.h"
#include "../cdrom/scsicd.h"
#include "interrupt.h"
#include "rainbow.h"
#include "soundbox.h"
#include "input.h"
#include "timer.h"
#include "debug.h"
#include <trio/trio.h>
#include <math.h>
#include "../video.h"
#include "../clamp.h"

#ifdef __MMX__
#include <mmintrin.h>
#endif

#define KINGDBG(format, ...) (void)0
//#define KINGDBG FXDBG
#define KING_UNDEF FXDBG
#define ADPCMDBG(format, ...) (void)0
//FXDBG

/*
 SCSI Questions(this list needs to be revised more and merged into the issues list at the beginning of the file):

  What happens when there is no more data to transfer during DMA and the status SCSI bus phase is entered(before the DMA count reaches 0)?

  Why is the "sequential DMA" bit needed?

  Which SCSI registers return the values of the SCSI bus, and which return latched values(from previous writes or pseudo-DMA)?

  Is real DMA layered on top of pseudo-DMA?  Reading the developer documents, it looks that way.

  What triggers the setting of ACK during pseudo-DMA?  A timer?  Reading from the upper 16-bits of KING register 0x05?  The lower bits(in which case,
  the value would be latched..)?

*/

// 16 bit YUV format:  upper 8 bits Y, next 4 bits U, lower 4 bits V, transformed to 8-bit U and 8-bit V by shifting in 0 in lower bits.

typedef struct
{
 uint8 AR;

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

 bool odd_field;	/* TRUE if interlaced mode is enabled and we're in the odd field, FALSE otherwise. */

 bool in_hblank;	/* TRUE if we're in H-blank */
 bool in_vdc_hsync;

 bool frame_interlaced;

 uint16 picture_mode;

 bool dot_clock;	 // Cached from picture_mode in hblank
 uint32 dot_clock_ratio; // Cached from picture mode in hblank
 int32 clock_divider;

 int32 vdc_event[2];


 uint32 raster_counter;

 uint16 palette_rw_offset; // Read/write offset
 uint16 palette_rw_latch;

 uint16 palette_offset[4]; // 
			   // BMG1 and BMG 0 in [1](BMG1 in upper 8 bits, BMG0 in lower), BMG2 and 3 in [2]
			   // RAINBOW in lower(?) 8 bits of [3]?

 uint16 palette_table[512]; // The YUV palette, woohoo!

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
} fx_vce_t;

fx_vce_t fx_vce;

//
// VCE render cache, including registers cached at hblank
//
typedef struct
{
 uint16 priority[2];
 uint16 picture_mode;

 uint16 palette_offset[4];
 uint32 palette_table_cache[512 * 2]; // 24-bit YUV cache for SPEED(HAH), * 2 to remove need for & 0x1FF in rendering code

 uint16 ChromaKeyY;
 uint16 ChromaKeyU;
 uint16 ChromaKeyV;

 uint16 CCR;
 uint16 BLE;

 uint16 SPBL;

 uint16 coefficients[6];

 uint8 coefficient_mul_table_y[16][256];
 int8 coefficient_mul_table_uv[16][256];

 uint32 LayerPriority[8];	// [LAYER_n] = ordered_priority_for_n(real priority 0-15 mapped to 1-7)
				//	       priority = 0, layer is disabled(via the layer enable bit not being set)
} vce_rendercache_t;

static vce_rendercache_t vce_rendercache;

static int32 scsicd_ne;

enum
{
 HPHASE_ACTIVE = 0,
 HPHASE_HBLANK_PART1,
 HPHASE_HBLANK_PART3,
 HPHASE_HBLANK_PART4,
 HPHASE_COUNT
};

static int32 HPhase;
static int32 HPhaseCounter;
static int32 vdc_lb_pos;

static MDFN_ALIGN(8) uint16 vdc_linebuffers[2][512];
static MDFN_ALIGN(8) uint32 vdc_linebuffer[512];
static MDFN_ALIGN(8) uint32 vdc_linebuffer_yuved[512];
static MDFN_ALIGN(8) uint32 rainbow_linebuffer[256];

// 8 * 2 for left + right padding for scrolling
static MDFN_ALIGN(8) uint32 bg_linebuffer[256 + 8 + 8];



// Don't change these enums, there are some hardcoded values still used(particularly, LAYER_NONE).
enum
{
 LAYER_NONE = 0,
 LAYER_BG0,
 LAYER_BG1,
 LAYER_BG2,
 LAYER_BG3,
 LAYER_VDC_BG,
 LAYER_VDC_SPR,
 LAYER_RAINBOW
};

static uint8 VCEPrioMap[8][8][8][4];	// [n][n][n][3] is dummy, for padding to a power of 2.

static void BuildCMT(void)
{
 for(int coeff = 0; coeff < 16; coeff++)
 {
  for(int value = 0; value < 256; value++)
  {
   vce_rendercache.coefficient_mul_table_y[coeff][value] = (value * coeff / 8); // Y
   vce_rendercache.coefficient_mul_table_uv[coeff][value] = ((value - 128) * coeff / 8); // UV
  }
 }

}

static INLINE void RebuildLayerPrioCache(void)
{
 vce_rendercache_t *vr = &vce_rendercache;

 vr->LayerPriority[LAYER_NONE] = 0;

 for(int n = 0; n < 4; n++)
 {
  if(((fx_vce.picture_mode >> (10 + n)) & 1))
  {
   vr->LayerPriority[LAYER_BG0 + n] = (((vce_rendercache.priority[1] >> (n * 4)) & 0xF) + 1);
   if(vr->LayerPriority[LAYER_BG0 + n] > 8)
   {
    printf("KING BG%d Priority Too Large: %d\n", n, vr->LayerPriority[LAYER_BG0 + n] - 1);
    vr->LayerPriority[LAYER_BG0 + n] = 0;
   }
  }
  else
   vr->LayerPriority[LAYER_BG0 + n] = 0;
 }

 if(fx_vce.picture_mode & 0x0100)
 {
  vr->LayerPriority[LAYER_VDC_BG] = ((vce_rendercache.priority[0] & 0xF) + 1);
  if(vr->LayerPriority[LAYER_VDC_BG] > 8)
  {
   printf("VDC BG Priority Too Large: %d\n", vr->LayerPriority[LAYER_VDC_BG] - 1);
   vr->LayerPriority[LAYER_VDC_BG] = 0;
  }
 }
 else
  vr->LayerPriority[LAYER_VDC_BG] = 0;

 if(fx_vce.picture_mode & 0x0200)
 {
  vr->LayerPriority[LAYER_VDC_SPR] = (((vce_rendercache.priority[0] >> 4) & 0xF) + 1);
  if(vr->LayerPriority[LAYER_VDC_SPR] > 8)
  {
   printf("VDC SPR Priority Too Large: %d\n", vr->LayerPriority[LAYER_VDC_SPR] - 1);
   vr->LayerPriority[LAYER_VDC_SPR] = 0;
  }
 }
 else 
  vr->LayerPriority[LAYER_VDC_SPR] = 0;

 if(fx_vce.picture_mode & 0x4000)
 {
  vr->LayerPriority[LAYER_RAINBOW] = (((vce_rendercache.priority[0] >> 8) & 0xF) + 1);
  if(vr->LayerPriority[LAYER_RAINBOW] > 8)
  {
   printf("RAINBOW Priority Too Large: %d\n", vr->LayerPriority[LAYER_RAINBOW] - 1);
   vr->LayerPriority[LAYER_RAINBOW] = 0;
  }
 }
 else
  vr->LayerPriority[LAYER_RAINBOW] = 0;


 // At this point, all entries in vr->LayerPriority should be one of 0 through 8(inclusive).

 int RemapPriority = 1;
 bool Done[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

 for(unsigned int i = 1; i < (1 + 8); i++)
 {
  for(int n = 0; n < 4; n++)
  {
   if(vr->LayerPriority[LAYER_BG0 + n] == i && !Done[LAYER_BG0 + n])
   {
    vr->LayerPriority[LAYER_BG0 + n] = RemapPriority++;
    Done[LAYER_BG0 + n] = true;
   }
  }

  if(vr->LayerPriority[LAYER_VDC_BG] == i && !Done[LAYER_VDC_BG])
  {
   vr->LayerPriority[LAYER_VDC_BG] = RemapPriority++;
   Done[LAYER_VDC_BG] = true;
  }

  if(vr->LayerPriority[LAYER_VDC_SPR] == i && !Done[LAYER_VDC_SPR])
  {
   vr->LayerPriority[LAYER_VDC_SPR] = RemapPriority++;
   Done[LAYER_VDC_SPR] = true;
  }

  if(vr->LayerPriority[LAYER_RAINBOW] == i && !Done[LAYER_RAINBOW])
  {
   vr->LayerPriority[LAYER_RAINBOW] = RemapPriority++;
   Done[LAYER_RAINBOW] = true;
  }
 }
 assert(RemapPriority <= 8);

 //if(fx_vce.raster_counter == 50)
 // MDFN_DispMessage("%d BG0: %d %d %d %d, VBG: %d, VSPR: %d, RAIN: %d", vr->LayerPriority[0], vr->LayerPriority[1], vr->LayerPriority[2], vr->LayerPriority[3],
 //						vr->LayerPriority[4], vr->LayerPriority[5], vr->LayerPriority[6], vr->LayerPriority[7]);
}

// Call this function in FX VCE hblank(or at the end/immediate start of active display)
static void DoHBlankVCECaching(void)
{
 const fx_vce_t *source = &fx_vce;
 vce_rendercache_t *dest = &vce_rendercache;

 dest->picture_mode = source->picture_mode;

 fx_vce.dot_clock = (bool)(fx_vce.picture_mode & 0x08);
 fx_vce.dot_clock_ratio = (fx_vce.picture_mode & 0x08) ? 3 : 4;

 for(int i = 0; i < 2; i++)
  dest->priority[i] = source->priority[i];
 

 for(int i = 0; i < 4; i++)
  dest->palette_offset[i] = source->palette_offset[i];

 dest->ChromaKeyY = source->ChromaKeyY;
 dest->ChromaKeyU = source->ChromaKeyU;
 dest->ChromaKeyV = source->ChromaKeyV;

 dest->CCR = source->CCR;
 dest->BLE = source->BLE;
 dest->SPBL = source->SPBL;

 for(int i = 0; i < 6; i++)
  dest->coefficients[i] = source->coefficients[i];

 RebuildLayerPrioCache();
}

static INLINE void RedoPaletteCache(int n)
{
 uint32 YUV = fx_vce.palette_table[n];
 uint8 Y = (YUV >> 8) & 0xFF;
 uint8 U = (YUV & 0xF0);
 uint8 V = (YUV & 0x0F) << 4;

 vce_rendercache.palette_table_cache[n] = 
 vce_rendercache.palette_table_cache[0x200 | n] = (Y << 16) | (U << 8) | (V << 0);
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
	
	uint32 KRAMRA, KRAMWA;
	uint8 KRAM_Mode;

	uint32 PageSetting;
        uint16 *RainbowPagePtr, *DMAPagePtr;    // Calculated off of PageSetting

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

	uint8 BGBATAddr[4];
	uint8 BGCGAddr[4];
        uint8 BG0SubBATAddr, BG0SubCGAddr;

	uint16 BGXScroll[4];
	uint16 BGYScroll[4];
	
        uint16 BGXScrollCache[4];
        uint16 BGYScrollCache[4];

	uint16 BGAffinA, BGAffinB, BGAffinC, BGAffinD;
	uint16 BGAffinCenterX, BGAffinCenterY;

	uint16 ADPCMControl;
	uint16 ADPCMBufferMode[2];

	uint16 ADPCMSAL[2];

	uint32 ADPCMEndAddress[2];
	uint32 ADPCMPlayAddress[2];
	uint16 ADPCMIntermediateAddress[2];
	uint16 ADPCMStatus[2]; // Register 0x53, a bit maimed :)
	bool ADPCMIRQPending;

	uint16 RAINBOWTransferControl; // Register 0x40
	uint32 RAINBOWKRAMA;	       // Register 0x41
        uint16 RAINBOWTransferStartPosition; // Register 0x42, line number(0-262)
	uint16 RAINBOWTransferBlockCount; // Register 0x43


	bool RAINBOWStartPending;
	int32 RAINBOWBusyCount, RAINBOWBlockCount;

	uint16 RasterIRQLine; // Register 0x44
	bool RasterIRQPending;

	uint32 RAINBOWKRAMReadPos;

	bool DMATransferFlipFlop;
	uint32 DMATransferAddr; // Register 0x09
	uint32 DMATransferSize; // Register 0x0A
	uint16 DMAStatus;	// Register 0x0B
	uint8 DMALatch;


	uint16 MPROGControl;    // register 0x15
	uint16 MPROGControlCache;
	uint16 MPROGAddress;
	uint16 MPROGData[0x10];

	bool DMAInterrupt;
	uint8 Reg00;
	uint8 Reg01;
	uint8 Reg02;
	uint8 Reg03;


	uint8 SubChannelControl;

	bool CDInterrupt, SubChannelInterrupt;
	uint8 SubChannelBuf;
	uint8 data_cache;

	bool DRQ;
	bool dma_receive_active;
	bool dma_send_active;
	int32 dma_cycle_counter;
	int32 lastts;


        uint16 KRAM[2][262144];

	#define KING_MAGIC_INTERVAL 10 //4 //32 //10
} king_t;

static king_t *king = NULL;

static uint8 BGLayerDisable;
static bool RAINBOWLayerDisable;

static void RedoKINGIRQCheck(void);

static INLINE void REGSETP(uint16 &reg, const uint8 data, const bool msb)
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

 //FXVDC_SetAux0BPBpase(fx_vdc_chips[0], (read || write) ? 0x80000 : ~0);
 //FXVDC_SetAux0BPBpase(fx_vdc_chips[1], (read || write) ? 0x90000 : ~0);
}

static void (*KINGLog)(const char *, const char *, ...) = NULL;
void KING_SetLogFunc(void (*logfunc)(const char *, const char *, ...))
{
 KINGLog = logfunc;
}

static MDFN_Surface *GfxDecode_Buf = NULL;
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
  //FXVDC_GetAddressSpaceBytes(fx_vdc_chips[which], "vram", Address, Length, Buffer);
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
  //FXVDC_PutAddressSpaceBytes(fx_vdc_chips[which], "vram", Address, Length, Granularity, hl, Buffer);
 }

}
#endif

static void RecalcKRAMPagePtrs(void)
{
 king->RainbowPagePtr = king->KRAM[(king->PageSetting & 0x1000) ? 1 : 0];
 king->DMAPagePtr = king->KRAM[king->PageSetting & 1];
}

uint8 KING_RB_Fetch(void)
{
 uint8 ret = king->RainbowPagePtr[(king->RAINBOWKRAMReadPos >> 1) & 0x3FFFF] >> ((king->RAINBOWKRAMReadPos & 1) * 8);

 king->RAINBOWKRAMReadPos = ((king->RAINBOWKRAMReadPos + 1) & 0x3FFFF) | (king->RAINBOWKRAMReadPos & 0x40000);

 return(ret);
}

static void DoRealDMA(uint8 db)
{
 if(!king->DMATransferFlipFlop)
  king->DMALatch = db;
 else
 {
  king->DMAPagePtr[king->DMATransferAddr & 0x3FFFF] = king->DMALatch | (db << 8);
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

 if(king->ADPCMIRQPending)
 {
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

 PCFXIRQ_Assert(PCFXIRQ_SOURCE_KING, asserted);
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

uint8 KING_Read8(const v810_timestamp_t timestamp, uint32 A)
{
 uint8 ret = KING_Read16(timestamp, A & ~1) >> ((A & 1) * 8);

 //printf("Read8: %04x\n", A);
 return(ret);
}

void KING_EndFrame(v810_timestamp_t timestamp)
{
 PCFX_SetEvent(PCFX_EVENT_KING, KING_Update(timestamp));
 scsicd_ne = SCSICD_Run(timestamp);

 SCSICD_ResetTS();

 king->lastts = 0;

 if(king->dma_cycle_counter & 0x40000000)
 {
  king->dma_cycle_counter = 0x7FFFFFFF;
 }
}

//static INLINE void StartKingMagic(void)
//{
// king->lastts = v810_timestamp;
// king->dma_cycle_counter = KING_MAGIC_INTERVAL;
// PCFX_SetEvent(PCFX_EVENT_KING, KING_MAGIC_INTERVAL);
//}

static INLINE int32 CalcNextEvent(int32 next_event)
{
 if(king->dma_cycle_counter < next_event)
  next_event = king->dma_cycle_counter;

 if(scsicd_ne < next_event)
  next_event = scsicd_ne;

 return(next_event);
}

static int32 CalcNextExternalEvent(int32 next_event)
{
 // 100 = Hack to make the emulator go faster during CD DMA transfers.
 if(king->dma_cycle_counter < next_event)
  next_event = 100;	//king->dma_cycle_counter;

 if(scsicd_ne < next_event)
  next_event = scsicd_ne;

 if(next_event > HPhaseCounter)
  next_event = HPhaseCounter;

 //printf("KING: %d %d %d; %d\n", king->dma_cycle_counter, scsicd_ne, HPhaseCounter, next_event);

 for(int chip = 0; chip < 2; chip++)
 {
  int fwoom = (fx_vce.vdc_event[chip] * fx_vce.dot_clock_ratio - fx_vce.clock_divider);

  if(fwoom < 1)
   fwoom = 1;

  if(next_event > fwoom)
   next_event = fwoom;
 }

 return(next_event);
}

static void MDFN_FASTCALL KING_RunGfx(int32 clocks);

v810_timestamp_t MDFN_FASTCALL KING_Update(const v810_timestamp_t timestamp)
{
 int32 clocks = timestamp - king->lastts;
 uint32 running_timestamp = king->lastts;

 //printf("KING Run for: %d\n", clocks);

 king->lastts = timestamp;

 KING_RunGfx(clocks);

 while(clocks > 0)
 {
  int32 chunk_clocks = CalcNextEvent(clocks);

  running_timestamp += chunk_clocks;
  clocks -= chunk_clocks;

  scsicd_ne -= chunk_clocks;
  if(scsicd_ne <= 0)
   scsicd_ne = SCSICD_Run(running_timestamp);

  king->dma_cycle_counter -= chunk_clocks;
  if(king->dma_cycle_counter <= 0)
  {
   //assert(king->dma_receive_active || king->dma_send_active);
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
       //PCFX_SetEvent(PCFX_EVENT_SCSI, SCSICD_Run(timestamp));

       if(king->DMAStatus & 0x1)
       {
        king->DRQ = FALSE;
        DoRealDMA(king->data_cache);
        SCSICD_SetACK(TRUE);
        scsicd_ne = SCSICD_Run(running_timestamp);
       }
      }
     }
     else if(SCSICD_GetACK() && !SCSICD_GetREQ())
     {
      SCSICD_SetACK(FALSE);
      scsicd_ne = SCSICD_Run(running_timestamp);
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
       scsicd_ne = SCSICD_Run(running_timestamp);
       king->DRQ = TRUE;
      }
     }
     else if(SCSICD_GetACK() && !SCSICD_GetREQ())
     {
      SCSICD_SetACK(FALSE);
      scsicd_ne = SCSICD_Run(running_timestamp);
     }
    }
   }
  }
 } // end while(clocks > 0)

 return(timestamp + CalcNextExternalEvent(0x4FFFFFFF));
}

uint16 KING_Read16(const v810_timestamp_t timestamp, uint32 A)
{
 int msh = A & 2;
 uint16 ret = 0;

 KING_Update(timestamp);

 //printf("KRead16: %08x, %d; %04x\n", A, timestamp, king->AR);

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
	       ret = king->AR;

	       if(king->ADPCMIRQPending)
		ret |= 0x400;

	       if(king->SubChannelInterrupt)
		ret |= 0x1000;
	
	       // Gaaah, this is probably a hack...Anime Freak FX Vol 4 gets confused and crashes
	       // if both bits are set at once.
	       if(king->DMAInterrupt && (king->DMAStatus & 0x2))
		ret |= 0x2000;
	       else if(king->CDInterrupt)
		ret |= 0x4000;

	       if(king->RasterIRQPending)
		ret |= 0x800;

	       king->SubChannelInterrupt = FALSE;
               king->RasterIRQPending = FALSE;
               RedoKINGIRQCheck();
	      }
	      else
	      {
		ret |= SCSICD_GetSEL() ? 0x02: 0x00;
		ret |= SCSICD_GetIO() ? 0x04 : 0x00;
		ret |= SCSICD_GetCD() ? 0x08 : 0x00;
		ret |= SCSICD_GetMSG() ? 0x10 : 0x00;
		ret |= SCSICD_GetREQ() ? 0x20 : 0x00;
		ret |= SCSICD_GetBSY() ? 0x40 : 0x00;
		ret |= SCSICD_GetRST() ? 0x80 : 0x00;

		ret |= king->SubChannelBuf << 8;
	      }
	      break; // status...

  case 0x604: switch(king->AR)
	      {
		default: 
			KINGDBG("Unknown 16-bit register read: %02x\n", king->AR);
			break;

		case 0x00:
			ret = SCSICD_GetDB();
			break;

		case 0x01:
			ret = REGGETHW(king->Reg01, msh);
			break;

		case 0x02:
			ret = REGGETHW(king->Reg02, msh);
			break;

		case 0x03:
			ret = REGGETHW(king->Reg03, msh);
			break;

		case 0x04:
			if(!msh)
			{
          	         ret |= SCSICD_GetSEL() ? 0x02: 0x00;
                	 ret |= SCSICD_GetIO() ? 0x04 : 0x00;
	                 ret |= SCSICD_GetCD() ? 0x08 : 0x00;
	                 ret |= SCSICD_GetMSG() ? 0x10 : 0x00;
	                 ret |= SCSICD_GetREQ() ? 0x20 : 0x00;
	                 ret |= SCSICD_GetBSY() ? 0x40 : 0x00;
	                 ret |= SCSICD_GetRST() ? 0x80 : 0x00;
			}
			break;

		case 0x05:
			if(king->Reg01 & 0x80)
                        {
			 ret = 0x00;
		         break;
			}

		        if(msh)
			{
			 ret = king->data_cache;
			 //printf("Fooball: %02x\n", ret);
			 if(king->dma_receive_active)
			 {
			  king->DRQ = FALSE;
     			  SCSICD_SetACK(TRUE);
     			  scsicd_ne = 1;
			 }
			}
			else
			{
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
			}
			break;

		case 0x06: // SCSI Input Data Register, same value returned as reading D16-D23 of register 0x05?
			KINGDBG("Input data for...?\n");
			ret = king->data_cache;
			break;

		case 0x07: 
			// SCSI IRQ acknowledge/reset
			KINGDBG("SCSI IRQ acknowledge\n");
			king->CDInterrupt = FALSE;
                        RedoKINGIRQCheck();
			ret = 0xFF;
			break;

		case 0x08: 	// Sub-channel data
			if(!msh)
			{
			 ret = king->SubChannelBuf;
			 king->SubChannelBuf = 0;
			 //puts("Sub-channel data read.");
			}
			break;

                case 0x09: 
			ret = REGGETHW(king->DMATransferAddr, msh);
			break;

                case 0x0A: 
			ret = REGGETHW(king->DMATransferSize, msh);
			break;

		case 0x0B: // Value read in the BIOS always seems to be discarded...  DMA IRQ acknowledge?
			if(!msh)
			{
			 ret = king->DMAInterrupt ? 1 : 0;
                         KINGDBG("DMA IRQ Acknowledge: %d\n", ret);
                         king->DMAInterrupt = 0;
                         RedoKINGIRQCheck();
			}
			break;

		case 0x0C: 
			ret = REGGETHW(king->KRAMRA, msh);
			break;

		case 0x0D:
			ret = REGGETHW(king->KRAMWA, msh);
			break;

		case 0x0E:
			{
                         unsigned int page = (king->KRAMRA & 0x80000000) ? 1 : 0;
                         int32 inc_amount = ((int32)((king->KRAMRA & (0x3FF << 18)) << 4)) >> 22; // Convert from 10-bit signed 2's complement 

                         ret = king->KRAM[page][king->KRAMRA & 0x3FFFF];

	                 #ifdef WANT_DEBUGGER 
			 if(KRAMReadBPE) 
			  PCFXDBG_CheckBP(BPOINT_AUX_READ, (king->KRAMRA & 0x3FFFF) | (page ? 0x40000 : 0), 1);
			 #endif
	
                         king->KRAMRA = (king->KRAMRA &~ 0x1FFFF) | ((king->KRAMRA + inc_amount) & 0x1FFFF);
                        }
			break;

		case 0x0F: ret = king->PageSetting;
			   break;

                case 0x10: ret = REGGETHW(king->bgmode, msh);
			   break;

		case 0x15: ret = king->MPROGControl;
			   break;

		//case 0x40: break; 	// Super Power League FX reads this, but I think it's write-only.

		case 0x53:
			  {
			   ret = king->ADPCMStatus[0] | (king->ADPCMStatus[1] << 2);

			   king->ADPCMStatus[0] = king->ADPCMStatus[1] = 0;
			   king->ADPCMIRQPending = 0;

			   RedoKINGIRQCheck();

			   ADPCMDBG("Status read: %02x\n", ret);
			  }
			  break;
	      }
	      break;
	      
 }

 PCFX_SetEvent(PCFX_EVENT_KING, timestamp + CalcNextExternalEvent(0x4FFFFFFF));    // TODO: Optimize this to only be called when necessary.

 return(ret);
}

void KING_Write8(const v810_timestamp_t timestamp, uint32 A, uint8 V)
{
 KING_Write16(timestamp, A & 0x706, V << ((A & 1) ? 8 : 0));
}

static INLINE void SCSI_Reg0_Write(const v810_timestamp_t timestamp, uint8 V, bool delay_run = 0)
{
 king->Reg00 = V;
 SCSICD_SetDB(V);

 KINGDBG("WriteDB: %02x\n", V);

 if(!delay_run)
 {
  scsicd_ne = 1; //SCSICD_Run(timestamp);
 }
}

static INLINE void SCSI_Reg2_Write(const v810_timestamp_t timestamp, uint8 V, bool delay_run = 0)
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
  {
   scsicd_ne = 1; //SCSICD_Run(timestamp);
  }
  king->DRQ = FALSE;

  king->dma_receive_active = FALSE;
  king->dma_send_active = FALSE;
  king->dma_cycle_counter = 0x7FFFFFFF;
 }

 king->Reg02 = V;
}

static INLINE void SCSI_Reg3_Write(const v810_timestamp_t timestamp, uint8 V, bool delay_run = 0)
{
 KINGDBG("Set phase match SCSI bus bits: IO: %d, CD: %d, MSG: %d\n", (int)(bool)(V & 1), (int)(bool)(V & 2), (int)(bool)(V & 4));
 king->Reg03 = V & 0x7;

 if(!delay_run)
 {
  scsicd_ne = 1; //SCSICD_Run(timestamp);
 }
}

void KING_Write16(const v810_timestamp_t timestamp, uint32 A, uint16 V)
{
 int msh = A & 0x2;

 //printf("Write16: %08x %04x\n", A, V);


 if(!(A & 0x4))
 {
  if(!msh) 
   king->AR = V & 0x7F; 
 }
 else
 {
  //if(king->AR != 0x0E)
  // printf("KING: %02x %04x, %d\n", king->AR, V, fx_vce.raster_counter);
  KING_Update(timestamp);

  if(king->AR >= 0x50 && king->AR <= 0x5E)
  {
   //ADPCMDBG("Write: %02x(%d), %04x", king->AR, msh, V);
  }

	      switch(king->AR)
	      {
		default: 
			KINGDBG("Unknown 16-bit register write: %02x %04x %d\n", king->AR, V, msh); 
			break;

		case 0x00: if(king->Reg01 & 0x80)
			    break;

			   if(!msh) 
			   {
			    SCSI_Reg0_Write(timestamp, V);
			   }
			   break;

		case 0x01: if(!msh)
			   {
                            KINGDBG("Set SCSI BUS bits; Assert DB: %d, ATN: %d, SEL: %d, ACK: %d, RST: %d, %02x\n",
                                (int)(bool)(V & 1), (int)(bool)(V & 2), (int)(bool)(V & 4),
                                (int)(bool)(V & 0x10), (int)(bool)(V &0x80), SCSICD_GetDB());

			    if(V & 0x80)	// RST, silly KING, resets SCSI internal control registers too!
			    {
			     if(!(king->Reg01 & 0x80))
			     {
			      SCSI_Reg0_Write(timestamp, 0, TRUE);
			      SCSI_Reg2_Write(timestamp, 0, TRUE);
			      SCSI_Reg3_Write(timestamp, 0, TRUE);
			      king->data_cache = 0x00;

			      //king->CDInterrupt = true;
			      //RedoKINGIRQCheck();
			      //puts("KING RST IRQ");
			     }

			     king->Reg01 = V & 0x80; // Only this bit remains...how lonely.
			    }
			    else
			    {
			     king->Reg01 = V & (1 | 2 | 4 | 0x10 | 0x80);

			     SCSICD_SetATN(V & 2);
			     SCSICD_SetSEL(V & 4);
			     SCSICD_SetACK(V & 0x10);
			    }
                            SCSICD_SetRST(V & 0x80);
			    scsicd_ne = 1;
			   }
			   break;

		case 0x02: if(king->Reg01 & 0x80)
                            break;

			   if(!msh)
			   {
			    SCSI_Reg2_Write(timestamp, V);
			   }
			   break;

		case 0x03: if(king->Reg01 & 0x80)
                            break;

			   if(!msh)
			   {
			    SCSI_Reg3_Write(timestamp, V);
			   }
			   break;

		case 0x05: if(king->Reg01 & 0x80)
                            break;

			   if(!msh)	// Start DMA target receive
			   {
                            KINGDBG("DMA target receive: %04x, %d\n", V, msh);
                            king->dma_receive_active = FALSE;
                            king->dma_send_active = TRUE;
			    king->DRQ = TRUE;
			    //StartKingMagic();
			    king->dma_cycle_counter = KING_MAGIC_INTERVAL;
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

		case 0x07: if(king->Reg01 & 0x80)
                            break;

			   KINGDBG("Start DMA initiator receive: %04x\n", V);

			   if(king->Reg02 & 0x2)
			   {
			    king->dma_receive_active = TRUE;
			    king->dma_send_active = FALSE;
			    //StartKingMagic();
			    king->dma_cycle_counter = KING_MAGIC_INTERVAL;
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

			   king->KRAM[page][king->KRAMWA & 0x3FFFF] = V;
			   king->KRAMWA = (king->KRAMWA &~ 0x1FFFF) | ((king->KRAMWA + inc_amount) & 0x1FFFF);
			  }
			  break;


		// Page settings(0/1) for BG, DMA, ADPCM, and RAINBOW transfers.
		case 0x0F: REGSETHW(king->PageSetting, V, msh);
			   RecalcKRAMPagePtrs();
			   break;


		// Background Modes
		case 0x10: REGSETHW(king->bgmode, V, msh);
			   break;


		// Background priorities and affine transform master enable.
		case 0x12: if(!msh)
			   {
			    king->priority = V;
			    if(king->priority & ~0x1FFF)
			    {
			     KING_UNDEF("Invalid priority bits set: %04x\n", king->priority);
			    }
			   }
			   break;


		// Microprogram Address
		case 0x13: if(!msh)
			   {
			    king->MPROGAddress = V & 0xF;
			   }
			   break;


		// Microprogram Data Port
		case 0x14: if(!msh)
			   {
			    king->MPROGData[king->MPROGAddress] = V;
			    king->MPROGAddress = (king->MPROGAddress + 1) & 0xF;
			   }
			   break;

		case 0x15: REGSETHW(king->MPROGControl, V, msh); king->MPROGControl &= 0x1; break;

		case 0x16: REGSETHW(king->BGScrollMode, V, msh); king->BGScrollMode &= 0xF; break;

		case 0x20: REGSETHW(king->BGBATAddr[0], V, msh); break;
		case 0x21: REGSETHW(king->BGCGAddr[0], V, msh); break;
		case 0x22: REGSETHW(king->BG0SubBATAddr, V, msh); break;
	  	case 0x23: REGSETHW(king->BG0SubCGAddr, V, msh); break;

		case 0x24: REGSETHW(king->BGBATAddr[1], V, msh); break;
		case 0x25: REGSETHW(king->BGCGAddr[1], V, msh); break;
		case 0x28: REGSETHW(king->BGBATAddr[2], V, msh); break;
		case 0x29: REGSETHW(king->BGCGAddr[2], V, msh); break;
		case 0x2A: REGSETHW(king->BGBATAddr[3], V, msh); break;
		case 0x2B: REGSETHW(king->BGCGAddr[3], V, msh); break;

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
			   // E = 1, rainbow transfer enable
			   if(!msh)
			   {
			    king->RAINBOWTransferControl = V & 0x3;
			    if(!(V & 1))
			    {
			     //if(king->RAINBOWBusyCount || king->RAINBOWBlockCount)
			     // puts("RAINBOW transfer reset");
			     // Not sure if this is completely correct or not.  Test cases: "Tonari no Princess Rolfee", (others?)
			     //king->RAINBOWBusyCount = 0;
			     //king->RAINBOWBlockCount = 0;
			     //RAINBOW_ForceTransferReset();
			     king->RAINBOWBlockCount = 0;
			    }
			   }
			   king->RasterIRQPending = FALSE;
			   RedoKINGIRQCheck();
                           //printf("Transfer Control: %d, %08x\n", fx_vce.raster_counter,  king->RAINBOWTransferControl);
			   break;

		// Rainbow transfer address
                case 0x41: REGSETHW(king->RAINBOWKRAMA, V, msh); 
			   king->RAINBOWKRAMA &= 0x3FFFF;
			   //printf("KRAM Transfer Addr: %d, %08x\n", fx_vce.raster_counter,  king->RAINBOWKRAMA);
			   break;

		// 0-262
		case 0x42: if(!msh) 
			   {
			    king->RAINBOWTransferStartPosition = V & 0x1FF;
			    //fprintf(stderr, "%d\n", king->RAINBOWTransferStartPosition);
	                    //printf("RAINBOW Start Line: %d, %08x\n", fx_vce.raster_counter,  king->RAINBOWTransferStartPosition);
			   }
			   break;

		case 0x43: REGSETHW(king->RAINBOWTransferBlockCount, V, msh);
			   king->RAINBOWTransferBlockCount &= 0x1F;
                           //printf("KRAM Transfer Block Count: %d, %08x\n", fx_vce.raster_counter,  king->RAINBOWTransferBlockCount);
			   break;

		// Raster IRQ line
		case 0x44: if(!msh)
			   {
			    king->RasterIRQLine = V & 0x1FF;
                            //printf("Raster IRQ scanline: %d, %08x\n", fx_vce.raster_counter, king->RasterIRQLine);
			   }
			   break;

		case 0x50: 
			   if(!msh)
			   {
			    for(int ch = 0; ch < 2; ch++)
			    {
			     if(!(king->ADPCMControl & (1 << ch)) && (V & (1 << ch)))
			     { 
			      king->ADPCMPlayAddress[ch] = king->ADPCMSAL[ch] * 256;
			     }
			    }
			    king->ADPCMControl = V; 
			    RedoKINGIRQCheck();
			    SoundBox_SetKINGADPCMControl(king->ADPCMControl);
			   }
			   break;

		case 0x51: REGSETHW(king->ADPCMBufferMode[0], V, msh);
			   RedoKINGIRQCheck();
			   break;

		case 0x52: REGSETHW(king->ADPCMBufferMode[1], V, msh);
			   RedoKINGIRQCheck();
			   break;

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
			   }
			   break;
	      }

  PCFX_SetEvent(PCFX_EVENT_KING, timestamp + CalcNextExternalEvent(0x4FFFFFFF));	// TODO: Optimize this to only be called when necessary.
 }
}

uint16 KING_GetADPCMHalfWord(int ch)
{
 int page = (king->PageSetting & 0x0100) ? 1 : 0;
 uint16 ret = king->KRAM[page][king->ADPCMPlayAddress[ch] & 0x3FFFF];

 king->ADPCMPlayAddress[ch] = (king->ADPCMPlayAddress[ch] & 0x20000) | ((king->ADPCMPlayAddress[ch] + 1) & 0x1FFFF);

 if(!(king->ADPCMPlayAddress[ch] & 0x1FFFF))
 {
  ADPCMDBG("Ch %d Wrapped", ch);
 }

 if(king->ADPCMPlayAddress[ch] == (((king->ADPCMEndAddress[ch] + 1) & 0x1FFFF) | (king->ADPCMEndAddress[ch] & 0x20000)) )
 {
  ADPCMDBG("Ch %d End", ch);

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

  if(king->ADPCMBufferMode[ch] & (0x1 << 1))
  {
   king->ADPCMIRQPending = TRUE;
   RedoKINGIRQCheck();
  }
 }
 else if(king->ADPCMPlayAddress[ch] == ((uint32)king->ADPCMIntermediateAddress[ch] << 6) )
 {
  ADPCMDBG("Ch %d Intermediate", ch);
  king->ADPCMStatus[ch] |= 2;

  if(king->ADPCMBufferMode[ch] & (0x2 << 1))
  {
   king->ADPCMIRQPending = TRUE;
   RedoKINGIRQCheck();
  }
 }

 return(ret);
}

static uint32 HighDotClockWidth;
extern Blip_Buffer FXsbuf[2]; // FIXME, externals are evil!

bool KING_Init(void)
{
 if(!(king = (king_t*)MDFN_malloc(sizeof(king_t), _("KING Data"))))
  return(0);

 HighDotClockWidth = MDFN_GetSettingUI("pcfx.high_dotclock_width");
 BGLayerDisable = 0;

 BuildCMT();

 // Build VCE priority map.
 // Don't change this unless you know what you're doing!
 // There may appear to be a bug in the pixel mixing
 // code elsewhere, because it accesses this array like [vdc][bg][rainbow], but it's not a bug.
 // This multi-dimensional array has no concept of bg, vdc, rainbow, or their orders per-se, it just
 // contains priority information for 3 different layers.

 for(int bg_prio = 0; bg_prio < 8; bg_prio++)
  for(int vdc_prio = 0; vdc_prio < 8; vdc_prio++)
   for(int rainbow_prio = 0; rainbow_prio < 8; rainbow_prio++)
   {
    int bg_prio_test = bg_prio ? bg_prio : 0x10;
    int vdc_prio_test = vdc_prio ? vdc_prio : 0x10;
    int rainbow_prio_test = rainbow_prio ? rainbow_prio : 0x10;

    if(bg_prio_test >= 8)
     VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][0] = 3;
    else
    {
     if(bg_prio_test < vdc_prio_test && bg_prio_test < rainbow_prio_test)
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][0] = 0;
     else if(bg_prio_test > vdc_prio_test && bg_prio_test > rainbow_prio_test)
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][0] = 2;
     else 
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][0] = 1;
    }

    if(vdc_prio_test >= 8)
     VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][1] = 3;
    else
    {
     if(vdc_prio_test < bg_prio_test && vdc_prio_test < rainbow_prio_test)
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][1] = 0;
     else if(vdc_prio_test > bg_prio_test && vdc_prio_test > rainbow_prio_test)
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][1] = 2;
     else
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][1] = 1;
    }

    if(rainbow_prio_test >= 8)
     VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][2] = 3;
    else
    {
     if(rainbow_prio_test < bg_prio_test && rainbow_prio_test < vdc_prio_test)
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][2] = 0;
     else if(rainbow_prio_test > bg_prio_test && rainbow_prio_test > vdc_prio_test)
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][2] = 2;
     else
      VCEPrioMap[bg_prio][vdc_prio][rainbow_prio][2] = 1;
    }
   }

 #ifdef WANT_DEBUGGER
 ASpace_Add(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "kram0", "KRAM Page 0", 19);
 ASpace_Add(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "kram1", "KRAM Page 1", 19);
 ASpace_Add(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "vdcvram0", "VDC-A VRAM", 17);
 ASpace_Add(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "vdcvram1", "VDC-B VRAM", 17);
 ASpace_Add(KING_GetAddressSpaceBytes, KING_PutAddressSpaceBytes, "vce", "VCE Palette RAM", 10);
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
 SCSICD_Close();
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
 king->dma_cycle_counter = 0x7FFFFFFF;


 RecalcKRAMPagePtrs();

 HPhase = HPHASE_HBLANK_PART1;
 HPhaseCounter = 1;
 vdc_lb_pos = 0;

 memset(vdc_linebuffers, 0, sizeof(vdc_linebuffers));
 memset(vdc_linebuffer, 0, sizeof(vdc_linebuffer));
 memset(vdc_linebuffer_yuved, 0, sizeof(vdc_linebuffer_yuved));
 memset(rainbow_linebuffer, 0, sizeof(rainbow_linebuffer));
 memset(bg_linebuffer, 0, sizeof(bg_linebuffer));


 king->dma_cycle_counter = 0x7FFFFFFF;
 scsicd_ne = 1;	// FIXME

 RedoKINGIRQCheck();

 for(unsigned int x = 0; x < 0x200; x++)
  RedoPaletteCache(x);

 DoHBlankVCECaching();

 SoundBox_SetKINGADPCMControl(0);

 SCSICD_Power(0);	// FIXME

 memset(king->KRAM, 0xFF, sizeof(king->KRAM));
}


static INLINE void DRAWBG8x1_4(uint32 *target, const uint16 *cg, const uint32 *palette_ptr, const uint32 layer_or)
{
 if(*cg >> 14) target[0] = palette_ptr[(*cg >> 14)] | layer_or;
 if((*cg >> 12) & 0x3) target[1] = palette_ptr[((*cg >> 12) & 0x3)] | layer_or;
 if((*cg >> 10) & 0x3) target[2] = palette_ptr[((*cg >> 10) & 0x3)] | layer_or;
 if((*cg >> 8) & 0x3) target[3] = palette_ptr[((*cg >> 8) & 0x3)] | layer_or;
 if((*cg >> 6) & 0x3) target[4] = palette_ptr[((*cg >> 6) & 0x3)] | layer_or;
 if((*cg >> 4) & 0x3) target[5] = palette_ptr[((*cg >> 4) & 0x3)] | layer_or;
 if((*cg >> 2) & 0x3) target[6] = palette_ptr[((*cg >> 2) & 0x3)] | layer_or;
 if((*cg >> 0) & 0x3) target[7] = palette_ptr[((*cg >> 0) & 0x3)] | layer_or;
}

static INLINE void DRAWBG8x1_16(uint32 *target, const uint16 *cgptr, const uint32 *palette_ptr, const uint32 layer_or)
{
 if(cgptr[0] >> 12) target[0] = palette_ptr[((cgptr[0] >> 12))] | layer_or;
 if((cgptr[0] >> 8) & 0xF) target[1] = palette_ptr[(((cgptr[0] >> 8) & 0xF))] | layer_or;
 if((cgptr[0] >> 4) & 0xF) target[2] = palette_ptr[(((cgptr[0] >> 4) & 0xF))] | layer_or;
 if((cgptr[0] >> 0) & 0xF) target[3] = palette_ptr[(((cgptr[0] >> 0) & 0xF))] | layer_or;

 if(cgptr[1] >> 12) target[4] = palette_ptr[((cgptr[1] >> 12))] | layer_or;
 if((cgptr[1] >> 8) & 0xF) target[5] = palette_ptr[(((cgptr[1] >> 8) & 0xF))] | layer_or;
 if((cgptr[1] >> 4) & 0xF) target[6] = palette_ptr[(((cgptr[1] >> 4) & 0xF))] | layer_or;
 if((cgptr[1] >> 0) & 0xF) target[7] = palette_ptr[(((cgptr[1] >> 0) & 0xF))] | layer_or;
}

static INLINE void DRAWBG8x1_256(uint32 *target, const uint16 *cgptr, const uint32 *palette_ptr, const uint32 layer_or)
{
 if(cgptr[0] >> 8) target[0] = palette_ptr[(cgptr[0] >> 0x8)] | layer_or;
 if(cgptr[0] & 0xFF) target[1] = palette_ptr[(cgptr[0] & 0xFF)] | layer_or;
 if(cgptr[1] >> 8) target[2] = palette_ptr[(cgptr[1] >> 0x8)] | layer_or;
 if(cgptr[1] & 0xFF) target[3] = palette_ptr[(cgptr[1] & 0xFF)] | layer_or;
 if(cgptr[2] >> 8) target[4] = palette_ptr[(cgptr[2] >> 0x8)] | layer_or;
 if(cgptr[2] & 0xFF) target[5] = palette_ptr[(cgptr[2] & 0xFF)] | layer_or;
 if(cgptr[3] >> 8) target[6] = palette_ptr[(cgptr[3] >> 0x8)] | layer_or;
 if(cgptr[3] & 0xFF) target[7] = palette_ptr[(cgptr[3] & 0xFF)] | layer_or;
}

static INLINE void DRAWBG8x1_64K(uint32 *target, const uint16 *cgptr, const uint32 *palette_ptr, const uint32 layer_or)
{
 if(cgptr[0] & 0xFF00) target[0] = ((cgptr[0x0] & 0x00F0) << 8) | ((cgptr[0] & 0x000F)<<4) | ((cgptr[0] & 0xFF00) << 8) | layer_or;
 if(cgptr[1] & 0xFF00) target[1] = ((cgptr[0x1] & 0x00F0) << 8) | ((cgptr[1] & 0x000F)<<4) | ((cgptr[1] & 0xFF00) << 8) | layer_or;
 if(cgptr[2] & 0xFF00) target[2] = ((cgptr[0x2] & 0x00F0) << 8) | ((cgptr[2] & 0x000F)<<4) | ((cgptr[2] & 0xFF00) << 8) | layer_or;
 if(cgptr[3] & 0xFF00) target[3] = ((cgptr[0x3] & 0x00F0) << 8) | ((cgptr[3] & 0x000F)<<4) | ((cgptr[3] & 0xFF00) << 8) | layer_or;
 if(cgptr[4] & 0xFF00) target[4] = ((cgptr[0x4] & 0x00F0) << 8) | ((cgptr[4] & 0x000F)<<4) | ((cgptr[4] & 0xFF00) << 8) | layer_or;
 if(cgptr[5] & 0xFF00) target[5] = ((cgptr[0x5] & 0x00F0) << 8) | ((cgptr[5] & 0x000F)<<4) | ((cgptr[5] & 0xFF00) << 8) | layer_or;
 if(cgptr[6] & 0xFF00) target[6] = ((cgptr[0x6] & 0x00F0) << 8) | ((cgptr[6] & 0x000F)<<4) | ((cgptr[6] & 0xFF00) << 8) | layer_or;
 if(cgptr[7] & 0xFF00) target[7] = ((cgptr[0x7] & 0x00F0) << 8) | ((cgptr[7] & 0x000F)<<4) | ((cgptr[7] & 0xFF00) << 8) | layer_or;
}

static INLINE void DRAWBG8x1_16M(uint32 *target, const uint16 *cgptr, const uint32 *palette_ptr, const uint32 layer_or)
{
 if(cgptr[0] >> 8) target[0] = ((cgptr[0x0] & 0xFF00) << 8) | (cgptr[1] & 0xFF00) | (cgptr[1] & 0xFF) | layer_or;
 if(cgptr[0] & 0xFF) target[1] = ((cgptr[0x0] & 0x00FF) << 16) | (cgptr[1] & 0xFF00) | (cgptr[1] & 0xFF) | layer_or;
 if(cgptr[2] >> 8) target[2] = ((cgptr[0x2] & 0xFF00) << 8) | (cgptr[3] & 0xFF00) | (cgptr[3] & 0xFF) | layer_or;
 if(cgptr[2] & 0xFF) target[3] = ((cgptr[0x2] & 0x00FF) << 16) | (cgptr[3] & 0xFF00) | (cgptr[3] & 0xFF) | layer_or;
 if(cgptr[4] >> 8) target[4] = ((cgptr[0x4] & 0xFF00) << 8) | (cgptr[5] & 0xFF00) | (cgptr[5] & 0xFF) | layer_or;
 if(cgptr[4] & 0xFF) target[5] = ((cgptr[0x4] & 0x00FF) << 16) | (cgptr[5] & 0xFF00) | (cgptr[5] & 0xFF) | layer_or;
 if(cgptr[6] >> 8) target[6] = ((cgptr[0x6] & 0xFF00) << 8) | (cgptr[7] & 0xFF00) | (cgptr[7] & 0xFF) | layer_or;
 if(cgptr[6] & 0xFF) target[7] = ((cgptr[0x6] & 0x00FF) << 16) | (cgptr[7] & 0xFF00) | (cgptr[7] & 0xFF) | layer_or;
}

static bool bgmode_warning = 0; // Debug

#include "king-bgfast.inc"

static INLINE int32 max(int32 a, int32 b)
{
 if(a > b)
  return(a);

 return(b);
}

static void DrawBG(uint32 *target, int n, bool sub)
{
 // TODO: Verify behavior when size is out of bounds on BG1-3.
 // With BG0 at least, it behaves as if the size is at its minimum, with caveats(TO BE INVESTIGATED).
 const uint32 bg_ss_table[2][0x10] = 
 {
  { 0x3, 0x3, 0x3, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0x3, 0x3, 0x3, 0x3, 0x3 },
  { 0x3, 0x3, 0x3, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3 },
 };

 const bool bg_ss_invalid_table[2][0x10] =
 {
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
 };

#if 0
 const uint32 cg_per_mode[0x8] = 
 {
  0, // Invalid mode
  1, // 2-bit mode
  2, // 4-bit mode
  4, // 8-bit mode
  8, // 16-bit mode
  8, // 16-bit mode
  8, // 16-bit mode
  8, // 16-bit mode
 };
#endif
 const uint32 layer_or = (LAYER_BG0 + n) << 28;

 const uint32 palette_offset = ((fx_vce.palette_offset[1 + (n >> 1)] >> ((n & 1) ? 8 : 0)) << 1) & 0x1FF;
 const uint32 *palette_ptr = &vce_rendercache.palette_table_cache[palette_offset];
 const uint32 bat_and_cg_page = (king->PageSetting & 0x0010) ? 1 : 0;

 const uint16 bgmode = (king->bgmode >> (n * 4)) & 0xF;
 const bool endless = (king->BGScrollMode >> n) & 0x1;
 const uint32 XScroll = king->BGXScroll[n];
 const uint32 YScroll = king->BGYScroll[n];

 const uint32 YOffset = (YScroll + (fx_vce.raster_counter - 22)) & 0xFFFF;

 const uint32 bat_offset = king->BGBATAddr[n] * 1024;
 const uint32 bat_sub_offset = n ? bat_offset : (king->BG0SubBATAddr * 1024);
 const uint16 *bat_base = &king->KRAM[bat_and_cg_page][bat_offset & 0x20000];
 const uint16 *bat_sub_base = &king->KRAM[bat_and_cg_page][bat_sub_offset & 0x20000];

 const uint32 cg_offset = king->BGCGAddr[n] * 1024;
 const uint32 cg_sub_offset = n ? cg_offset : (king->BG0SubCGAddr * 1024);
 const uint16 *cg_base = &king->KRAM[bat_and_cg_page][cg_offset & 0x20000];
 const uint16 *cg_sub_base = &king->KRAM[bat_and_cg_page][cg_sub_offset & 0x20000];

 const int bat_bitsize_mask = (n ? 0x3FF : 0x7FF) >> 3;

 const uint32 bat_width_shift = bg_ss_table[(bool)n][(king->BGSize[n] & 0xF0) >> 4];
 const bool bat_width_invalid = bg_ss_invalid_table[(bool)n][(king->BGSize[n] & 0xF0) >> 4];
 const uint32 bat_width = (1 << bat_width_shift) >> 3;

 const int32 bat_height_shift = bg_ss_table[(bool)n][king->BGSize[n] & 0x0F];
 //const bool bat_height_invalid = bg_ss_invalid_table[(bool)n][king->BGSize[n] & 0x0F];
 const int32 bat_height = (1 << bat_height_shift) >> 3;

 const bool bat_sub_width_invalid = n ? bat_width_invalid : bg_ss_invalid_table[(bool)n][(king->BGSize[n] & 0xF000) >> 12];
 const uint32 bat_sub_width_shift = n ? bat_width_shift : bg_ss_table[(bool)n][(king->BGSize[n] & 0xF000) >> 12];
 const uint32 bat_sub_width = (1 << bat_sub_width_shift) >> 3;
 const uint32 bat_sub_width_mask = bat_sub_width - 1;
 const uint32 bat_sub_width_test = endless ? (bat_bitsize_mask + 1) : max(bat_width, bat_sub_width);

 const int32 bat_sub_height_shift = n ? bat_height_shift : bg_ss_table[(bool)n][(king->BGSize[n] & 0x0F00) >> 8];
 const int32 bat_sub_height = (1 << bat_sub_height_shift) >> 3;
 const int32 bat_sub_height_mask = bat_sub_height - 1;
 const int32 bat_sub_height_test = endless ? (bat_bitsize_mask + 1) : max(bat_height, bat_sub_height);

 uint16 cg_mask[8];
 uint16 cg_remap[8];
 bool cg_ofbat[8];

 uint16 cg_sub_mask[8];
 uint16 cg_sub_remap[8];
 bool cg_sub_ofbat[8];

 bool BATFetchCycle = FALSE;
 bool BATSubFetchCycle = FALSE;

 const bool rotate_mode = (n == 0) && (king->priority & 0x1000);

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

 memset(cg_mask, 0, sizeof(cg_mask));
 memset(cg_remap, 0, sizeof(cg_remap));
 memset(cg_ofbat, 0, sizeof(cg_ofbat));
 memset(cg_sub_mask, 0, sizeof(cg_sub_mask));
 memset(cg_sub_remap, 0, sizeof(cg_sub_remap));
 memset(cg_sub_ofbat, 0, sizeof(cg_sub_ofbat));

 if(king->MPROGControl & 0x1)
 {
  int remap_thing = 0;
  int remap_sub_thing = 0;

  for(int x = 0; x < 8; x++)
  {
   uint16 mpd;

   // Forcing CG and BAT to 0 if the affine bit != rotate_mode is not technically correct.
   // If there is a mismatch, it's more likely the effective CG and BAT address for the pixel/segment
   // being drawn won't be calculated correctly, likely being just the initial offsets.

   mpd = king->MPROGData[((cg_offset & 0x20000) ? 0x8 : 0x0) + x];
   if(((mpd >> 6) & 0x3) == n && !(mpd & 0x100) && !(mpd & 0x010))
   {
    cg_mask[remap_thing] = 0xFFFF;
    cg_remap[remap_thing] = mpd & 0x7;
    cg_ofbat[remap_thing] = mpd & 0x8;

    if((bool)(mpd & 0x20) != rotate_mode)
    {
     KINGDBG("Affine bit != rotate_mode?");
     cg_mask[remap_thing] = 0;
    }
    remap_thing++;
   }

   mpd = king->MPROGData[((cg_sub_offset & 0x20000) ? 0x8 : 0x0) + x];
   if(((mpd >> 6) & 0x3) == n && !(mpd & 0x100) && !(mpd & 0x010))
   {
    cg_sub_mask[remap_sub_thing] = 0xFFFF;
    cg_sub_remap[remap_sub_thing] = mpd & 0x7;
    cg_sub_ofbat[remap_sub_thing] = mpd & 0x8;

    if((bool)(mpd & 0x20) != rotate_mode)
    {
     KINGDBG("Affine bit != rotate_mode? (SUB)");
     cg_sub_mask[remap_sub_thing] = 0;
    }
    remap_sub_thing++;
   }
  }

  for(int x = 0; x < 8; x++)
  {
   uint16 mpd;

   mpd = king->MPROGData[((bat_offset & 0x20000) ? 0x8 : 0x0) + x];
   if(((mpd >> 6) & 0x3) == n && !(mpd & 0x100) && (mpd & 0x010) && (bool)(mpd & 0x020) == rotate_mode)
    BATFetchCycle = TRUE;

   mpd = king->MPROGData[((bat_sub_offset & 0x20000) ? 0x8 : 0x0) + x];
   if(((mpd >> 6) & 0x3) == n && !(mpd & 0x100) && (mpd & 0x010) && (bool)(mpd & 0x020) == rotate_mode)
    BATSubFetchCycle = TRUE;
  }
 }

 int bat_y = (YOffset >> 3) & bat_bitsize_mask;
 uint32 bat_x = (XScroll >> 3) & bat_bitsize_mask;
 int ysmall = YOffset & 0x7;

 const uint32 bat_invalid_y_mask = bat_width_invalid ? 0 : 0xFFFFFFFF;
 const uint32 bat_invalid_sub_y_mask = bat_sub_width_invalid ? 0 : 0xFFFFFFFF;

 if(rotate_mode)
  target += 8;
 else
  target += 8 - (XScroll & 0x7);


 {
  int32 wmul = (1 << bat_width_shift), wmask = ((1 << bat_height_shift) - 1) & bat_invalid_y_mask;
  int32 sexy_y_pos = (YOffset & wmask) * wmul;
  
  int32 wmul_sub = (1 << bat_sub_width_shift), wmask_sub = ((1 << bat_sub_height_shift) - 1) & bat_invalid_sub_y_mask;
  int32 sexy_y_sub_pos = (YOffset & wmask_sub) * wmul_sub;


  #define ROTCODE_PRE	\
	 const int32 bat_width_mask = endless ? (bat_width - 1) : 0xFFFF;		\
	 const int32 bat_height_mask = endless ? (bat_height - 1) : 0xFFFF;		\
         int32 a, b, c, d;	\
         int32 raw_x_coord = (int32)sign_11_to_s16(XScroll) - (int16)king->BGAffinCenterX;	\
         int32 raw_y_coord = fx_vce.raster_counter + (int32)sign_11_to_s16(YScroll) - 22 - (int16)king->BGAffinCenterY;		\
         int32 xaccum;	\
         int32 yaccum;	\
	\
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
          const uint16 *cgptr;	\
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
          uint16 bat = bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & 0x1FFFF];
          pbn = ((bat >> 12) << 2);
          bat &= 0x0FFF;
          cgptr = &cg_base[(cg_offset + (bat * 8) + ysmall) & 0x1FFFF];
         }
         else
         {
          cgptr = &cg_base[(cg_offset + bat_x + ((new_y & wmask) * wmul / 8)) & 0x1FFFF];
         }
         uint8 ze_cg = (cgptr[0] >> ((7 - (new_x & 7)) << 1)) & 0x03;

         if(endless || (bat_x < bat_width && bat_y < bat_height))
         {
          if(ze_cg) target[x] = palette_ptr[pbn + ze_cg] | layer_or;
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
          uint16 bat = bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & 0x1FFFF];
          pbn = ((bat >> 12) << 4);
	  bat &= 0x0FFF;
          cgptr = &cg_base[(cg_offset + (bat * 16) + ysmall * 2) & 0x1FFFF];
         }
         else
         {
          cgptr = &cg_base[(cg_offset + (bat_x * 2) + ((new_y & wmask) * wmul / 4)) & 0x1FFFF];
         }
         uint8 ze_cg = (cgptr[(new_x >> 2) & 0x1] >> ((3 - (new_x & 3)) << 2)) & 0x0F;

         if(endless || (bat_x < bat_width && bat_y < bat_height))
         {
          if(ze_cg) target[x] = palette_ptr[pbn + ze_cg] | layer_or;
         }
        ROTCODE_LOOP_POST;
  }
  else if((bgmode & 0x7) == BGMODE_256 && rotate_mode)
  {
	 ROTCODE_PRE;

	 ROTCODE_LOOP_PRE;
          if(bgmode & 0x8)
          {
           uint16 bat = bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & 0x1FFFF];
           cgptr = &cg_base[(cg_offset + (bat * 32) + ysmall * 4) & 0x1FFFF];
          }
          else
          {
           cgptr = &cg_base[(cg_offset + (bat_x * 4) + ((new_y & wmask) * wmul / 2)) & 0x1FFFF];
          }
          uint8 ze_cg = cgptr[(new_x >> 1) & 0x3] >> (((new_x & 1) ^ 1) << 3);

          if(endless || (bat_x < bat_width && bat_y < bat_height))
          {
           if(ze_cg) target[x] = palette_ptr[ze_cg] | layer_or;
          }
	 ROTCODE_LOOP_POST;
  } 
  else if((bgmode & 0x7) == BGMODE_64K && rotate_mode)
  {
	ROTCODE_PRE;
	ROTCODE_LOOP_PRE;
          if(bgmode & 0x8)
          {
           uint16 bat = bat_base[(bat_offset + (bat_x + ((bat_y << bat_width_shift) >> 3)  )) & 0x1FFFF];
           cgptr = &cg_base[(cg_offset + (bat * 64) + ysmall * 8) & 0x1FFFF];
          }
          else
          {
           cgptr = &cg_base[(cg_offset + (bat_x * 8) + ((new_y & wmask) * wmul)) & 0x1FFFF];
          }
          uint16 ze_cg = cgptr[new_x & 0x7];

          if(endless || (bat_x < bat_width && bat_y < bat_height))
          {
           if(ze_cg >> 8) target[x] = ((ze_cg & 0x00F0) << 8) | ((ze_cg & 0x000F)<<4) | ((ze_cg & 0xFF00) << 8) | layer_or;
          }
         ROTCODE_LOOP_POST;
  }
  else switch(bgmode & 0x7)
  {
#define DRAWBG8x1_MAC(cg_needed, blit_suffix, pbn_arg)	\
			 for(int x = 0; x < 256 + 8; x+= 8)     	\
                         {                                      	\
                          if(bat_x < bat_width && bat_y < bat_height)	\
                          {                                     	\
                           uint32 eff_bat_loc = bat_offset;     	\
                           uint16 bat = 0;                      	\
                           uint16 pbn = 0;                      	\
  		  	   const uint16 *cgptr[2];			\
			   uint16 cg[cg_needed];			\
									\
                           if(bgmode & 0x8)                     	\
                            eff_bat_loc += bat_x + (((bat_y & bat_invalid_y_mask) << bat_width_shift) >> 3);       	\
                                                                	\
                           eff_bat_loc &= 0x1FFFF;		  	\
                                                                	\
                           if(BATFetchCycle)                    	\
                            bat = bat_base[eff_bat_loc];		\
									\
			   if(bgmode & 0x08)				\
			    pbn = bat >> 12;				\
			   bat &= 0xFFF;				\
									\
                           cgptr[0] = &cg_base[(cg_offset + (bat_x * cg_needed) + sexy_y_pos)	\
				& 0x1FFFF];      						\
		           cgptr[1] = &cg_base[(cg_offset + (bat * 8 * cg_needed) + ysmall * cg_needed) \
				& 0x1FFFF];							\
													\
		           for(int cow = 0; cow < cg_needed; cow++)					\
		            cg[cow] = cgptr[cg_ofbat[cow]][cg_remap[cow]] & cg_mask[cow];		\
													\
		           DRAWBG8x1_##blit_suffix(target + x, cg, palette_ptr + pbn_arg, layer_or);	\
			  }                                     					\
			  else if(bat_x < bat_sub_width_test && bat_y < bat_sub_height_test)		\
			  {										\
                           uint32 eff_bat_loc = bat_sub_offset;         \
                           uint16 bat = 0;                              \
                           uint16 pbn = 0;                              \
                           const uint16 *cgptr[2];                      \
                           uint16 cg[cg_needed];                        \
                                                                        \
                           if(bgmode & 0x8)                             \
                            eff_bat_loc += (bat_x & bat_sub_width_mask) + (((bat_y & bat_invalid_sub_y_mask & bat_sub_height_mask) << bat_sub_width_shift) >> 3);           \
                                                                        \
                           eff_bat_loc &= 0x1FFFF;          \
                                                                        \
                           if(BATSubFetchCycle)                         \
                            bat = bat_sub_base[eff_bat_loc];            \
                                                                        \
                           if(bgmode & 0x08)                            \
                            pbn = bat >> 12;                            \
                           bat &= 0xFFF;                                \
                                                                        \
                           cgptr[0] = &cg_sub_base[(cg_sub_offset + ((bat_x & bat_sub_width_mask) * cg_needed) + sexy_y_sub_pos)   \
                                & 0x1FFFF];                                                 \
                           cgptr[1] = &cg_sub_base[(cg_sub_offset + (bat * 8 * cg_needed) + ysmall * cg_needed) \
                                & 0x1FFFF];                                                 \
                                                                                                        \
                           for(int cow = 0; cow < cg_needed; cow++)                                     \
                            cg[cow] = cgptr[cg_sub_ofbat[cow]][cg_sub_remap[cow]] & cg_sub_mask[cow];   \
                                                                                                        \
                           DRAWBG8x1_##blit_suffix(target + x, cg, palette_ptr + pbn_arg, layer_or);    \
			  }										\
                          bat_x = (bat_x + 1) & bat_bitsize_mask; 					\
                         }


   case 0x01: // 4 color, 1/4 byte per pixel :b
        sexy_y_pos >>= 3;
	sexy_y_sub_pos >>= 3;

	DRAWBG8x1_MAC(1, 4, (pbn << 2));
	break;

   case 0x02: // 16 color, 1/2 byte per pixel
	sexy_y_pos >>= 2;
	sexy_y_sub_pos >>= 2;

	DRAWBG8x1_MAC(2, 16, (pbn << 4));
        break;

   case 0x03: // 256 color, 1 byte per pixel palettized - OK
	sexy_y_pos >>= 1;
	sexy_y_sub_pos >>= 1;

	DRAWBG8x1_MAC(4, 256, 0);
	break;

   case 0x04: // 64K color, 2 bytes per pixel - OK
	DRAWBG8x1_MAC(8, 64K, 0);
	break;

   case 0x05: // 16M color, 2 bytes per pixel - OK
	DRAWBG8x1_MAC(8, 16M, 0);
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
static uint32 CbCrLUT[65536];

static void RebuildUVLUT(const MDFN_PixelFormat &format)
{
 for(int ur = 0; ur < 256; ur++)
 {
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
 
   CbCrLUT[vr + ur * 256] = clamp_to_u8(128 + ((r * -9699 + g * -19071 + b * 28770) >> 16)) << format.Cbshift;
   CbCrLUT[vr + ur * 256] |= clamp_to_u8(128 + ((r * 28770 + g * -24117 + b * -4653) >> 16)) << format.Crshift;

   //printf("%d %d %d, %08x\n", r, g, b, CbCrLUT[vr + ur * 256]);
  }
 }
 for(int x = 0; x < 1152; x++)
 {
  if(x < 384) RGBDeflower[x] = 0;
  else if(x > (384 + 255)) RGBDeflower[x] = 255;
  else
   RGBDeflower[x] = x - 384;
 }
}

// FIXME
static int rs, gs, bs;

static uint32 INLINE YUV888_TO_RGB888(uint32 yuv)
{
 int32 r, g, b;
 uint8 y = yuv >> 16;

 r = y + UVLUT[yuv & 0xFFFF][0];
 g = y + UVLUT[yuv & 0xFFFF][1];
 b = y + UVLUT[yuv & 0xFFFF][2];

 r = clamp_to_u8(r);
 g = clamp_to_u8(g);
 b = clamp_to_u8(b);

 return((r << rs) | (g << gs) | (b << bs));
}

static uint32 INLINE YUV888_TO_YCbCr888(uint32 yuv)
{
 uint32 y;

 y = 16 + ((((yuv >> 16) & 0xFF) * 220) >> 8);

 return(y | CbCrLUT[yuv & 0xFFFF]);
}

// FIXME: 
//static unsigned int lines_per_frame; //= (fx_vce.picture_mode & 0x1) ? 262 : 263;
static VDC **vdc_chips;
static MDFN_Surface *surface;
static MDFN_Rect *DisplayRect;
static MDFN_Rect *LineWidths;
static int skip;

void KING_StartFrame(VDC **arg_vdc_chips, EmulateSpecStruct *espec)	//MDFN_Surface *arg_surface, MDFN_Rect *arg_DisplayRect, MDFN_Rect *arg_LineWidths, int arg_skip)
{
 ::vdc_chips = arg_vdc_chips;
 ::surface = espec->surface;
 ::DisplayRect = &espec->DisplayRect;
 ::LineWidths = espec->LineWidths;
 ::skip = espec->skip;

 //MDFN_DispMessage("P0:%06x P1:%06x; I0: %06x I1: %06x", king->ADPCMPlayAddress[0], king->ADPCMPlayAddress[1], king->ADPCMIntermediateAddress[0] << 6, king->ADPCMIntermediateAddress[1] << 6);
 //MDFN_DispMessage("%d %d\n", SCSICD_GetACK(), SCSICD_GetREQ());

 // For the case of interlaced mode(clear ~0 state)
 LineWidths[0].x = 0;
 LineWidths[0].w = 0;

 // These 2 should be overwritten in the big loop below.
 DisplayRect->x = 0;
 DisplayRect->w = 256;

 DisplayRect->y = MDFN_GetSettingUI("pcfx.slstart");
 DisplayRect->h = MDFN_GetSettingUI("pcfx.slend") - DisplayRect->y + 1;

 if(fx_vce.frame_interlaced)
 {
  skip = false;

  espec->InterlaceOn = true;
  espec->InterlaceField = fx_vce.odd_field;
  DisplayRect->y *= 2;
  DisplayRect->h *= 2;
 }
}

static int rb_type;
//  unsigned int width = (fx_vce.picture_mode & 0x08) ? 341 : 256;

static void DrawActive(void)
{
 rb_type = -1;

 #ifdef WANT_DEBUGGER
 if(GfxDecode_Buf && GfxDecode_Line == (int32)fx_vce.raster_counter)
  DoGfxDecode();
 #endif

 if(fx_vce.raster_counter == king->RAINBOWTransferStartPosition && (king->RAINBOWTransferControl & 1))
 {
  king->RAINBOWStartPending = TRUE;

  //printf("Rainbow start pending: line=%d, busycount=%d, blockcount=%d\n", fx_vce.raster_counter, king->RAINBOWBusyCount, king->RAINBOWBlockCount);

  //if(fx_vce.raster_counter == 262)
  // puts("MOOO");
 }

 if(fx_vce.raster_counter < 262)
 {
  if(king->RAINBOWBusyCount)
  {
   king->RAINBOWBusyCount--;
   if(!king->RAINBOWBusyCount)
    RAINBOW_SwapBuffers();
  }

  if(!king->RAINBOWBusyCount)
  {
   bool WantDecode = FALSE;
   bool FirstDecode = FALSE;

   if(!king->RAINBOWBlockCount && king->RAINBOWStartPending)
   {
    //printf("Rainbow start real: %d %d\n", fx_vce.raster_counter, king->RAINBOWTransferBlockCount);
    king->RAINBOWBlockCount = king->RAINBOWTransferBlockCount;
    if(king->RAINBOWBlockCount)
    {
     king->RAINBOWKRAMReadPos = king->RAINBOWKRAMA << 1;
     FirstDecode = TRUE;
    }
   }

   if(king->RAINBOWBlockCount)
   {
    king->RAINBOWBlockCount--;
    WantDecode = TRUE;
   }

   if(WantDecode)
   {
    king->RAINBOWBusyCount = 16;

    if(fx_vce.raster_counter == 262)
     king->RAINBOWBusyCount++;

    // If we ever change the emulation time range from the current 0 through 262/263, we will need to readjust this
    // statement to prevent the previous frame's skip value to mess up the current frame's graphics data, since
    // RAINBOW data is delayed by 16 scanlines from when it's decoded(16 + 15 maximum delay).
    RAINBOW_DecodeBlock(FirstDecode, skip && fx_vce.raster_counter < 246);
   }
  }

  rb_type = RAINBOW_FetchRaster(skip ? NULL : rainbow_linebuffer, LAYER_RAINBOW << 28, &vce_rendercache.palette_table_cache[((fx_vce.palette_offset[3] >> 0) & 0xFF) << 1]);

  king->RAINBOWStartPending = FALSE;
 } // end   if(fx_vce.raster_counter < 262)

 if(fx_vce.raster_counter >= 22 && fx_vce.raster_counter < 262)
 {
  if(!skip)
  {
   if(rb_type == 1) // YUV
   {
    // Only chroma key when we're not in 7.16MHz pixel mode
    if(!(fx_vce.picture_mode & 0x08))
    {
     const unsigned int ymin = fx_vce.ChromaKeyY & 0xFF;
     const unsigned int ymax = fx_vce.ChromaKeyY >> 8;
     const unsigned int umin = fx_vce.ChromaKeyU & 0xFF;
     const unsigned int umax = fx_vce.ChromaKeyU >> 8;
     const unsigned int vmin = fx_vce.ChromaKeyV & 0xFF;
     const unsigned int vmax = fx_vce.ChromaKeyV >> 8;

     if((fx_vce.ChromaKeyY | fx_vce.ChromaKeyU | fx_vce.ChromaKeyV) == 0)
     {
      //puts("Opt: 0 chroma key");
      for(int x = 0; x < 256; x++)
      {
       if(!(rainbow_linebuffer[x] & 0xFFFFFF))
        rainbow_linebuffer[x] = 0;
      }
     }
     else if(ymin == ymax && umin == umax && vmin == vmax)
     {
      const uint32 compare_color = (ymin << 16) | (umin << 8) | (vmin << 0);

      //puts("Opt: Single color chroma key");

      for(int x = 0; x < 256; x++)
      {
       if((rainbow_linebuffer[x] & 0xFFFFFF) == compare_color)
        rainbow_linebuffer[x] = 0;
      }
     }
     else if(ymin <= ymax && umin <= umax && vmin <= vmax)
     {
      const uint32 yv_min_sub = (ymin << 16) | vmin;
      const uint32 u_min_sub = umin << 8;
      const uint32 yv_max_add = ((0xFF - ymax) << 16) | (0xFF - vmax);
      const uint32 u_max_add = (0xFF - umax) << 8;

      for(int x = 0; x < 256; x++)
      {
       const uint32 pixel = rainbow_linebuffer[x];
       const uint32 yv = pixel & 0xFF00FF;
       const uint32 u = pixel & 0x00FF00;
       uint32 testie;

       testie = ((yv - yv_min_sub) | (yv + yv_max_add)) & 0xFF00FF00;
       testie |= ((u - u_min_sub) | (u + u_max_add)) & 0x00FF00FF;

       if(!testie)
        rainbow_linebuffer[x] = 0;
      }
     }
     else
     {
      //puts("Opt: color keying off\n");
     }
    }
   }

    /*
        4 = Foremost
        1 = Hindmost
        0 = Hidden
    */

   MDFN_FastU32MemsetM8(bg_linebuffer + 8, 0, 256);

    // Only bother to draw the BGs if the microprogram is enabled.
   if(king->MPROGControl & 0x1)
   {
    for(int prio = 1; prio <= 7; prio++)
    {
     for(int x = 0; x < 4; x++)
     {
      int thisprio = (king->priority >> (x * 3)) & 0x7;

      if(BGLayerDisable & (1 << x)) continue;

      if(thisprio == prio)
      {
       //if(fx_vce.raster_counter == 50)
        // CanDrawBG_Fast(x);

       // TODO/FIXME: TEST MORE
       if(CanDrawBG_Fast(x)) // && (rand() & 1))
	DrawBG_Fast(bg_linebuffer, x);
       else
        DrawBG(bg_linebuffer, x, 0);
      }
     }
    }
   }

  } // end if(!skip)
 } // end if(fx_vce.raster_counter >= 22 && fx_vce.raster_counter < 262)
}

static INLINE void VDC_PIXELMIX(bool SPRCOMBO_ON, bool BGCOMBO_ON)
{
    static const uint32 vdc_layer_num[2] = { LAYER_VDC_BG << 28, LAYER_VDC_SPR << 28};
    const uint32 vdc_poffset[2] = {
                                ((fx_vce.palette_offset[0] >> 0) & 0xFF) << 1, // BG
                                ((fx_vce.palette_offset[0] >> 8) & 0xFF) << 1 // SPR
                               };

    const int width = fx_vce.dot_clock ? 342 : 256; // 342, not 341, to prevent garbage pixels in high dot clock mode.

    for(int x = 0; x < width; x++)
    {
     const uint32 zort[2] = { vdc_linebuffers[0][x], vdc_linebuffers[1][x] };
     uint32 tmp_pixel;
   
     /* SPR combination */
     if(SPRCOMBO_ON && (zort[1] & 0x18F) > 0x180)
      tmp_pixel = (zort[1] & 0xF) | ((zort[0] & 0xF) << 4) | 0x100;
     /* BG combination  */                                                      
     else if(BGCOMBO_ON && ((zort[1] ^ 0x100) & 0x18F) > 0x180)
      tmp_pixel = (zort[1] & 0xF) | ((zort[0] & 0xF) << 4);
     else
      tmp_pixel = (zort[1] & 0xF) ? zort[1] : zort[0];

     vdc_linebuffer[x] = tmp_pixel;
     vdc_linebuffer_yuved[x] = 0;
     if(tmp_pixel & 0xF)
      vdc_linebuffer_yuved[x] = vce_rendercache.palette_table_cache[(tmp_pixel & 0xFF) + vdc_poffset[(tmp_pixel >> 8) & 1]] | vdc_layer_num[(tmp_pixel >> 8) & 1];
    }
}

static void MixVDC(void) NO_INLINE;
static void MixVDC(void)
{
    // Optimization for when both layers are disabled in the VCE.
    if(!vce_rendercache.LayerPriority[LAYER_VDC_BG] && !vce_rendercache.LayerPriority[LAYER_VDC_SPR])
    {
     MDFN_FastU32MemsetM8(vdc_linebuffer_yuved, 0, 512);
    }
    else switch(fx_vce.picture_mode & 0xC0)
    {
     case 0x00: VDC_PIXELMIX(0, 0); break;      // None on
     case 0x40: VDC_PIXELMIX(0, 1); break;      // BG combo on
     case 0x80: VDC_PIXELMIX(1, 0); break;      // SPR combo on
     case 0xC0: VDC_PIXELMIX(1, 1); break;      // Both on
    }
}


static void MixLayers(void)
{
 uint32 *pXBuf = surface->pixels;

    // Now we have to mix everything together... I'm scared, mommy.
    // We have, vdc_linebuffer[0] and bg_linebuffer
    // Which layer is specified in bits 28-31(check the enum earlier on)
    uint32 priority_remap[8];
    uint32 ble_cache[8];
    bool ble_cache_any = FALSE;

    for(int n = 0; n < 8; n++)
    {
     priority_remap[n] = vce_rendercache.LayerPriority[n];
     //printf("%d: %d\n", n, priority_remap[n]);
    }

    // Rainbow layer disabled?
    if(rb_type == -1 || RAINBOWLayerDisable)
     priority_remap[LAYER_RAINBOW] = 0;

    ble_cache[LAYER_NONE] = 0;
    for(int x = 0; x < 4; x++)
     ble_cache[LAYER_BG0 + x] = (vce_rendercache.BLE >> (4 + x * 2)) & 0x3;

    ble_cache[LAYER_VDC_BG] = (vce_rendercache.BLE >> 0) & 0x3;
    ble_cache[LAYER_VDC_SPR] = (vce_rendercache.BLE >> 2) & 0x3;
    ble_cache[LAYER_RAINBOW] = (vce_rendercache.BLE >> 12) & 0x3;

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
     coeff_cache_y_fore[x] = vce_rendercache.coefficient_mul_table_y[(vce_rendercache.coefficients[x * 2 + 0] >> 8) & 0xF];
     coeff_cache_u_fore[x] = vce_rendercache.coefficient_mul_table_uv[(vce_rendercache.coefficients[x * 2 + 0] >> 4) & 0xF];
     coeff_cache_v_fore[x] = vce_rendercache.coefficient_mul_table_uv[(vce_rendercache.coefficients[x * 2 + 0] >> 0) & 0xF];

     coeff_cache_y_back[x] = vce_rendercache.coefficient_mul_table_y[(vce_rendercache.coefficients[x * 2 + 1] >> 8) & 0xF];
     coeff_cache_u_back[x] = vce_rendercache.coefficient_mul_table_uv[(vce_rendercache.coefficients[x * 2 + 1] >> 4) & 0xF];
     coeff_cache_v_back[x] = vce_rendercache.coefficient_mul_table_uv[(vce_rendercache.coefficients[x * 2 + 1] >> 0) & 0xF];
    }

    uint32 *target;
    uint32 BPC_Cache = (LAYER_NONE << 28); // Backmost pixel color(cache)

    if(fx_vce.frame_interlaced)
     target = pXBuf + surface->pitch32 * ((fx_vce.raster_counter - 22) * 2 + fx_vce.odd_field);
    else
     target = pXBuf + surface->pitch32 * (fx_vce.raster_counter - 22);
    

    // If at least one layer is enabled with the HuC6261, hindmost color is palette[0]
    // If no layers are on, this color is black.
    // If front cellophane is enabled, this color is forced to black(TODO:  Confirm on a real system.  Black or from CCR).
    // If back cellophane is enabled, this color is forced to the value in CCR
    //  (back and front conditions are handled closer to the pixel mixing loops down below)
    // TODO:  Test on a real PC-FX to see if CCR is used or not if back cellophane is enabled even if all layers are disabled in the HuC6261, 
    //  or if it just outputs black.
    // TODO:  See if enabling front/back cellophane in high dot-clock mode will set the hindmost color, even though the cellophane color mixing
    //  is disabled in high dot-clock mode.
    if(vce_rendercache.picture_mode & 0x7F00)
     BPC_Cache |= vce_rendercache.palette_table_cache[0];
    else			
     BPC_Cache |= 0x008080;

#define DOCELLO(pixpoo) \
	if((pixel[pixpoo] >> 28) != LAYER_VDC_SPR || ((vce_rendercache.SPBL >> ((vdc_linebuffer[x] & 0xF0)>> 4)) & 1))	\
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
      pixel[0] = 0;	\
      pixel[1] = 0;	\
      pixel[2] = 0;	\
      {	\
       uint8 pi0 = VCEPrioMap[prio[0]][prio[1]][prio[2]][0];	\
       uint8 pi1 = VCEPrioMap[prio[0]][prio[1]][prio[2]][1];	\
       uint8 pi2 = VCEPrioMap[prio[0]][prio[1]][prio[2]][2];	\
       /*assert(pi0 == 3 || !pixel[pi0]);*/ pixel[pi0] = vdc_linebuffer_yuved[index_341]; 	\
       /*assert(pi1 == 3 || !pixel[pi1]);*/ pixel[pi1] = (bg_linebuffer + 8)[index_256];	\
       /*assert(pi2 == 3 || !pixel[pi2]);*/ pixel[pi2] = rainbow_linebuffer[index_256];		\
      }

#define LAYER_MIX_FINAL_NOCELLO	\
       if(pixel[0])	\
        zeout = pixel[0];	\
       if(pixel[1])	\
         zeout = pixel[1];	\
       if(pixel[2])	\
         zeout = pixel[2];	\
       target[x] = YUV888_TO_xxx(zeout);	\
      }

// For back cellophane, the hindmost pixel is always a valid pixel to mix with, a "layer" in its own right,
// so we don't need to check the current pixel value before mixing.
#define LAYER_MIX_FINAL_BACK_CELLO     \
      if(pixel[0])        \
      { \
       if(ble_cache[pixel[0] >> 28])     \
       {        \
        DOCELLO(0);     \
       }        \
       else     \
        zeout = pixel[0];       \
      } \
      if(pixel[1])        \
      { \
       if(ble_cache[pixel[1] >> 28])     \
       {        \
        DOCELLO(1);     \
       }        \
       else     \
        zeout = pixel[1];       \
      } \
      if(pixel[2])        \
      { \
       if(ble_cache[pixel[2] >> 28])     \
       {        \
        DOCELLO(2);     \
       }        \
       else     \
        zeout = pixel[2];       \
      } \
      target[x] = YUV888_TO_xxx(zeout);      \
     }

// ..however, for front and "normal" cellophane, we need to make sure that the
// layer is indeed a real layer(KBG, VDC, RAINBOW) before mixing.
// Note: We need to check the upper 4 bits in determining whether the previous pixel is from a real layer or not, because the default
// hindmost non-layer color in front cellophane and normal cellophane modes is black, and black is represented in YUV as non-zero.  We COULD bias/XOR each of U/V
// by 0x80 in the rendering code so that it would work if we just tested for the non-zeroness of the previous pixel, and adjust the YUV->RGB
// to compensate...TODO as a future possible optimization(MAYBE, it would obfuscate things more than they already are).
//
// Also, since the hindmost real layer pixel will never mix with anything behind it, we can leave
// out a few checks for the first possible hindmost real pixel.
//
// Also, the front cellophane effect itself doesn't need to check if the effective pixel output is a real layer (TODO: Confirm on real hardware!)
//
#define LAYER_MIX_FINAL_FRONT_CELLO	\
      if(pixel[0])	\
       zeout = pixel[0];	\
      if(pixel[1])	\
      {	\
       if(ble_cache[pixel[1] >> 28] && (zeout & (0xF << 28))) \
       {	\
        DOCELLO(1);	\
       }	\
       else	\
        zeout = pixel[1];	\
      }	\
      if(pixel[2])	\
      {	\
       if(ble_cache[pixel[2] >> 28] && (zeout & (0xF << 28)))	\
       {	\
        DOCELLO(2);	\
       }	\
       else	\
        zeout = pixel[2];	\
      }	\
      DOCELLOSPECIALFRONT();	\
      target[x] = YUV888_TO_xxx(zeout);	\
     }

#define LAYER_MIX_FINAL_CELLO	\
      if(pixel[0])	\
       zeout = pixel[0];	\
      if(pixel[1])	\
      {	\
       if(ble_cache[pixel[1] >> 28] && (zeout & (0xF << 28))) 	\
       {	\
        DOCELLO(1);	\
       }	\
       else	\
        zeout = pixel[1];	\
      }	\
      if(pixel[2])	\
      {	\
       if(ble_cache[pixel[2] >> 28] && (zeout & (0xF << 28)))	\
       {	\
        DOCELLO(2);	\
       }	\
       else	\
        zeout = pixel[2];	\
      }	\
      target[x] = YUV888_TO_xxx(zeout);	\
     }

    if(surface->format.colorspace == MDFN_COLORSPACE_YCbCr)
    {
     #define YUV888_TO_xxx YUV888_TO_YCbCr888
     #include "king_mix_body.inc"
     #undef YUV888_TO_xxx
    }
    else
    {
     #define YUV888_TO_xxx YUV888_TO_RGB888
     #include "king_mix_body.inc"
     #undef YUV888_TO_xxx
    }
    DisplayRect->w = fx_vce.dot_clock ? HighDotClockWidth : 256;
    DisplayRect->x = 0;

	// FIXME
    if(fx_vce.frame_interlaced)
     LineWidths[(fx_vce.raster_counter - 22) * 2 + fx_vce.odd_field] = *DisplayRect;
    else
     LineWidths[fx_vce.raster_counter - 22] = *DisplayRect;
}

static INLINE void RunVDCs(const int master_cycles, uint16 *pixels0, uint16 *pixels1)
{
 int32 div_clocks;

 fx_vce.clock_divider += master_cycles;
 div_clocks = fx_vce.clock_divider / fx_vce.dot_clock_ratio;
 fx_vce.clock_divider -= div_clocks * fx_vce.dot_clock_ratio;

 if(pixels0)
  pixels0 += vdc_lb_pos;

 if(pixels1)
  pixels1 += vdc_lb_pos;

 assert((vdc_lb_pos + div_clocks) <= 512);

 fx_vce.vdc_event[0] = vdc_chips[0]->Run(div_clocks, pixels0, pixels0 ? false : true);
 fx_vce.vdc_event[1] = vdc_chips[1]->Run(div_clocks, pixels1, pixels1 ? false : true);

 vdc_lb_pos += div_clocks;

// printf("%d\n", vdc_lb_pos);
// if(fx_vce.dot_clock)
//  assert(vdc_lb_pos <= 342);
// else
//  assert(vdc_lb_pos <= 257);
}

static void MDFN_FASTCALL KING_RunGfx(int32 clocks)
{
 while(clocks > 0)
 {
  int32 chunk_clocks = clocks;

  if(chunk_clocks > HPhaseCounter)
   chunk_clocks = HPhaseCounter;

  clocks -= chunk_clocks;
  HPhaseCounter -= chunk_clocks;

  if(skip)
   RunVDCs(chunk_clocks, NULL, NULL);
  else if(fx_vce.in_hblank)
  {
   static uint16 dummybuf[1024];
   RunVDCs(chunk_clocks, dummybuf, dummybuf);
  }
  else
  {
   RunVDCs(chunk_clocks, vdc_linebuffers[0], vdc_linebuffers[1]);
  }

  assert(HPhaseCounter >= 0);

  while(HPhaseCounter <= 0)
  {
   HPhase = (HPhase + 1) % HPHASE_COUNT;
   switch(HPhase)
   {
    case HPHASE_ACTIVE: vdc_lb_pos = 0;
			fx_vce.in_hblank = false;
			DoHBlankVCECaching();
			DrawActive();
			HPhaseCounter += 1024;
			break;

    case HPHASE_HBLANK_PART1:
                        if(!skip)
                        {
                         if(fx_vce.raster_counter >= 22 && fx_vce.raster_counter < 262)
                         {
                          MixVDC();
                          MixLayers();
                         }
                        }
			fx_vce.in_hblank = true;
                        fx_vce.in_vdc_hsync = true;

                        for(int chip = 0; chip < 2; chip++)
                         vdc_chips[chip]->HSync(true);

			HPhaseCounter += 48;
			break;

    case HPHASE_HBLANK_PART3:
			fx_vce.raster_counter = (fx_vce.raster_counter + 1) % ((fx_vce.picture_mode & 0x1) ? 262 : 263); //lines_per_frame;

                        if(fx_vce.raster_counter == 0)
                         for(int chip = 0; chip < 2; chip++)
                          vdc_chips[chip]->VSync(true);

                        if(fx_vce.raster_counter == 3)
                         for(int chip = 0; chip < 2; chip++)
                          vdc_chips[chip]->VSync(false);

			if(!fx_vce.raster_counter)
			{
			 bool previous_interlaced = fx_vce.frame_interlaced;

			 fx_vce.frame_interlaced = fx_vce.picture_mode & 2;

			 if(fx_vce.frame_interlaced && previous_interlaced)
			  fx_vce.odd_field ^= 1;

			 if(!fx_vce.frame_interlaced)
			  fx_vce.odd_field = 0;

			 PCFX_V810.Exit();
			}

			if(fx_vce.raster_counter == king->RasterIRQLine && (king->RAINBOWTransferControl & 0x2))
			{
			 //printf("Wovely: %d, %d, %d\n", fx_vce.raster_counter, king->RAINBOWRasterCounter, king->RAINBOWTransferControl);

			 //if(fx_vce.raster_counter == 262)
			 //{
			 // printf("Rainbow raster IRQ on line 262?\n");
			 //}
			 //else
			 {
			  king->RasterIRQPending = TRUE;
			  RedoKINGIRQCheck();
			 }
			}

			// This -18(and +18) may or may not be correct in regards to how a real PC-FX adjusts the VDC layer horizontal position
			// versus the KING and RAINBOW layers.

			if(fx_vce.dot_clock)
			 HPhaseCounter += 173 - 18;
			else
			 HPhaseCounter += 165;
			break;

    case HPHASE_HBLANK_PART4:
			fx_vce.in_vdc_hsync = false;
                        for(int chip = 0; chip < 2; chip++)
			 vdc_chips[chip]->HSync(false);

			if(fx_vce.dot_clock)
			 HPhaseCounter += 120 + 18;
			else
 		  	 HPhaseCounter += 128;
			break;

   } // end: switch(HPhase)
  } // end: while(HPhaseCounter <= 0)
 } // end: while(clocks > 0)
} // end KING_RunGfx()

void KING_SetPixelFormat(const MDFN_PixelFormat &format) 
{
 rs = format.Rshift;
 gs = format.Gshift;
 bs = format.Bshift;
 RebuildUVLUT(format);
}

void KING_SetLayerEnableMask(uint64 mask)
{
 uint64 ms = mask;
 // "BG0\0BG1\0BG2\0BG3\0VDC-A BG\0VDC-A SPR\0VDC-B BG\0VDC-B SPR\0RAINBOW\0",

 BGLayerDisable = (~ms) & 0xF;
 ms >>= 4;

 for(unsigned chip = 0; chip < 2; chip++)
 {
  fx_vdc_chips[chip]->SetLayerEnableMask(ms & 0x3);
  ms >>= 2;
 }

 RAINBOWLayerDisable = (~ms) & 0x1;
 ms >>= 1;

#if 0
 if(which < 4)
 {
  BGLayerDisable ^= 1 << which;
  return( !((BGLayerDisable >> which) & 1));
 }
 else if(which == 4 || which == 5)
 {
  return(fx_vdc_chips[0]->ToggleLayer(which - 4));
 }
 else if(which == 6 || which == 7)
 {
  return(fx_vdc_chips[1]->ToggleLayer(which - 6));
 }
 else if(which == 8)
 {
  RAINBOWLayerDisable = !RAINBOWLayerDisable;
  return(!RAINBOWLayerDisable);
 }
 else
  return(0);
#endif
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

  SFARRAYN(king->BGBATAddr, 4, "BGBATAddr"),
  SFARRAYN(king->BGCGAddr, 4, "BGCGAddr"),
  SFVARN(king->BG0SubBATAddr, "BG0SubBATAddr"),
  SFVARN(king->BG0SubCGAddr, "BG0SubCGAddr"),

  SFARRAY16N(king->BGXScroll, 4, "BGXScroll"),
  SFARRAY16N(king->BGYScroll, 4, "BGYScroll"),
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
  SFVARN(king->ADPCMIRQPending, "ADPCMIRQPending"),
  SFVARN(king->RAINBOWTransferControl, "RAINBOWTransferControl"),
  SFVARN(king->RAINBOWKRAMA, "RAINBOWKRAMA"),
  SFVARN(king->RAINBOWTransferStartPosition, "RAINBOWTransferStartPosition"),
  SFVARN(king->RAINBOWTransferBlockCount, "RAINBOWTransferBlockCount"),

  SFVARN(king->RAINBOWStartPending, "RAINBOWStartPending"),
  SFVARN(king->RAINBOWBusyCount, "RAINBOWBusyCount"),
  SFVARN(king->RAINBOWBlockCount, "RAINBOWBlockCount"),

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
  SFVARN(king->Reg00, "Port00"),
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

  SFVAR(scsicd_ne),
  // scsicd_ne here??
  SFEND
 };

 SFORMAT VCEStateRegs[] =
 {
  SFVARN(fx_vce.AR, "AR"),
  SFARRAY16N(fx_vce.priority, 2, "priority"),
  SFVARN(fx_vce.odd_field, "odd_field"),
  SFVARN(fx_vce.in_hblank, "in_hblank"),
  SFVARN(fx_vce.in_vdc_hsync, "in_vdc_hsync"),
  SFVARN(fx_vce.picture_mode, "picture_mode"),

  SFVARN(HPhase, "HPhase"),
  SFVARN(HPhaseCounter, "HPhaseCounter"),
  SFVAR(vdc_lb_pos),

  SFVARN(fx_vce.dot_clock, "dot_clock"),
  SFVARN(fx_vce.clock_divider, "clock_divider"),

  SFARRAY32N(fx_vce.vdc_event, 2, "vdc_event"),

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


  // HB render cache:
  // FIXME
  SFARRAY16N(vce_rendercache.priority, 2, "rc_priority"),
  SFVARN(vce_rendercache.picture_mode, "rc_picture_mode"),
  SFARRAY16N(vce_rendercache.palette_offset, 4, "rc_palette_offset"),
  SFVARN(vce_rendercache.ChromaKeyY, "rc_ChromaKeyY"),
  SFVARN(vce_rendercache.ChromaKeyU, "rc_ChromaKeyU"),
  SFVARN(vce_rendercache.ChromaKeyV, "rc_ChromaKeyV"),
  SFVARN(vce_rendercache.CCR, "rc_CCR"),
  SFVARN(vce_rendercache.BLE, "rc_BLE"),
  SFVARN(vce_rendercache.SPBL, "rc_SPBL"),
  SFARRAY16N(vce_rendercache.coefficients, 6, "rc_coefficients"),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, KINGStateRegs, "KING");
 
 ret &= MDFNSS_StateAction(sm, load, data_only, VCEStateRegs, "VCE");

 if(load)
 {
  RecalcKRAMPagePtrs();

  fx_vce.dot_clock_ratio = fx_vce.dot_clock ? 3 : 4;

  fx_vce.palette_rw_offset &= 0x1FF;
  fx_vce.palette_offset[3] &= 0x00FF;
  fx_vce.priority[0] &= 0x0777;
  fx_vce.priority[1] &= 0x7777;

  for(int x = 0; x < 6; x++)
   fx_vce.coefficients[x] &= 0x0FFF;

  for(int x = 0; x < 0x200; x++)
   RedoPaletteCache(x);

  vdc_lb_pos &= 0x1FF; // FIXME: Better checks(in case we remove the assert() elsewhere)?

  RedoKINGIRQCheck();
  SoundBox_SetKINGADPCMControl(king->ADPCMControl);
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
  RecalcKRAMPagePtrs();
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
 else if(name == "AFFINA")
  king->BGAffinA = value;
 else if(name == "AFFINB")
  king->BGAffinB = value;
 else if(name == "AFFINC")
  king->BGAffinC = value;
 else if(name == "AFFIND")
  king->BGAffinD = value;
 else if(name == "AFFINX")
  king->BGAffinCenterX = value;
 else if(name == "AFFINY")
  king->BGAffinCenterY = value;
 else if(name ==  "BGBAT0")
  king->BGBATAddr[0] = value;
 else if(name ==  "BGBATS")
  king->BG0SubBATAddr = value;
 else if(name ==  "BGBAT1")
  king->BGBATAddr[1] = value;
 else if(name ==  "BGBAT2")
  king->BGBATAddr[2] = value;
 else if(name ==  "BGBAT3")
  king->BGBATAddr[3] = value;
 else if(name == "BGCG0" || name == "BGCGS" || name == "BGCG1" || name == "BGCG2" || name == "BGCG3")
 {
  char which = name[4];
  if(which == 'S')
   king->BG0SubCGAddr = value;
  else
   king->BGCGAddr[which - '0'] = value;
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
  SoundBox_SetKINGADPCMControl(king->ADPCMControl);
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
 else if(name == "ADPCMStat")
 {

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

   trio_snprintf(buf, 256, "Offset: %d, Access Type: %s, Rotation: %d, BG Number: %d, NOP: %d",
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
   trio_snprintf(buf, 256, "SCSI: %d, BG: %d, ADPCM: %d, RAINBOW: %d", (int)(bool)(value & 0x1), (int)(bool)(value & 0x10), (int)(bool)(value & 0x100), (int)(bool)(value & 0x1000));
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
   trio_snprintf(buf, 256, "BG0: %2d(%s), BG1: %2d(%s), BG2: %2d(%s), BG3: %2d(%s)", value & 0xF, bgmodes[value & 0xF], (value >> 4) & 0xF, bgmodes[(value >> 4) & 0xF], (value >> 8) & 0xF, bgmodes[(value >> 8) & 0xf], (value >> 12) & 0xF, bgmodes[(value >> 12) & 0xf]);
   *special = std::string(buf);
  }
 }
 else if(name == "BGPRIO")
 {
  value = king->priority;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "Affine enable: %s - BG0: %2d, BG1: %2d, BG2: %2d, BG3: %2d", (value & (1 << 12)) ? "Yes" : "No", value & 0x7, (value >> 3) & 0x7, (value >> 6) & 0x7, (value >> 9) & 0x7);
   *special = std::string(buf);
  }
 }
 else if(name == "BGSCRM")
 {
  value = king->BGScrollMode;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "BG0: %s, BG1: %s, BG2: %s, BG3: %s", (value & 1) ? "Endless" : "Non-endless",(value & 2) ? "Endless" : "Non-endless",
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
 else if(name == "AFFINA" || name == "AFFINB" || name == "AFFINC" || name == "AFFIND")
 {
  const uint16 *coeffs[4] = { &king->BGAffinA, &king->BGAffinB, &king->BGAffinC, &king->BGAffinD };
  value = *coeffs[name[5] - 'A'];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "%f", (double)(int16)value / 256);
   *special = std::string(buf);
  }
 }
 else if(name == "AFFINX")
  value = king->BGAffinCenterX;
 else if(name == "AFFINY")
  value = king->BGAffinCenterY;
 else if(name == "BGBAT0" || name == "BGBATS" || name == "BGBAT1" || name == "BGBAT2" || name == "BGBAT3")
 {
  char which = name[5];
  if(which == 'S')
   value = king->BG0SubBATAddr;
  else
   value = king->BGBATAddr[which - '0'];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "0x%04x * 1024 = 0x%05x", value, (value * 1024) & 0x3FFFF);
   *special = std::string(buf);
  }
 }

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
   trio_snprintf(buf, 256, "0x%04x * 1024 = 0x%05x", value, (value * 1024) & 0x3FFFF);
   *special = std::string(buf);
  }
 }

 else if(name == "RTCTRL")
 {
  value = king->RAINBOWTransferControl;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "Raster Interrupt: %s, Rainbow Transfer: %s", (value & 2) ? "On" : "Off", (value & 1) ? "On" : "Off");
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
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "0x%03x * 64 = 0x%08x", king->ADPCMIntermediateAddress[which], king->ADPCMIntermediateAddress[which] << 6);
   *special = std::string(buf);
  }
 }
 else if(name == "ADPCMEA0" || name == "ADPCMEA1")
 {
  unsigned int which = name[7] - '0';
  value = king->ADPCMEndAddress[which];
 }
 else if(name == "ADPCMStat")
 {
  value = king->ADPCMStatus[0] | (king->ADPCMStatus[1] << 2);
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "Ch0 End: %d, Ch0 Intermediate: %d, Ch1 End: %d, Ch1 Intermediate: %d", (int)(bool)(value & 0x1),
												   (int)(bool)(value & 0x2),
												   (int)(bool)(value & 0x4),
												   (int)(bool)(value & 0x8));
   *special = std::string(buf);
  }
 }
 else if(name == "Reg01")
 {
  value = king->Reg01;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "BSY: %d, ATN: %d, SEL: %d, ACK: %d, RST: %d", (int)(bool)(value & 1), (int)(bool)(value & 2), (int)(bool)(value & 4),
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

   trio_snprintf(buf, 256, "I/O: %d, C/D: %d, MSG: %d", (int)(bool)(value & 1), (int)(bool)(value & 2), (int)(bool)(value & 4));
   *special = std::string(buf);
  }
 }
 else if(name == "SUBCC")
 {
  value = king->SubChannelControl;
  if(special)
  {
   char buf[256];

   trio_snprintf(buf, 256, "Subchannel reading: %s, Subchannel read IRQ: %s", (value & 0x1) ? "Enabled" : "Disabled", (value & 0x2) ? "On" : "Off");
   *special = std::string(buf);
  }
 }
 else if(name == "DB")
  value = SCSICD_GetDB();
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
 else if(name == "PRIO1")
 {
  fx_vce.priority[1] = value & 0x7777;
 }
 else if(name == "PICMODE")
 {
  fx_vce.picture_mode = value;
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
}


uint32 FXVDCVCE_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name == "PRIO0")
 {
  value = fx_vce.priority[0];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "VDC BG: %2d, VDC SPR: %2d, RAINBOW: %2d", value & 0xF, (value >> 4) & 0xF, (value >> 8) & 0xF);
   *special = std::string(buf);
  }
 }
 else if(name == "PRIO1")
 {
  value = fx_vce.priority[1];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "BG0: %2d, BG1: %2d, BG2: %2d, BG3: %2d", value & 0xF, (value >> 4) & 0xF, (value >> 8) & 0xF, (value >> 12) & 0xF);
   *special = std::string(buf);
  }
 }
 else if(name == "PICMODE")
 {
  value = fx_vce.picture_mode;
  if(special)
  {
   char buf[256];
   static const char *DCCModes[4] =
   { 
    "263 lines/frame", "262 lines/frame", "Interlaced", "Interlaced+1/2 dot shift" 
   };

   trio_snprintf(buf, 256, "BG0: %s, BG1: %s, BG2: %s, BG3: %s, VDC BG: %s%s, VDC SPR: %s%s, RAINBOW: %s, VDC Clk: %sMHz, %s",
	(value & (1 << 10)) ? "On" : "Off", (value & (1 << 11)) ? "On": "Off",
        (value & (1 << 12)) ? "On" : "Off", (value & (1 << 13)) ? "On": "Off",
	(value & 0x0100) ? "On" : "Off", (value & 0x0040) ? "+merge mode" : "", (value & 0x0200) ? "On" : "Off", (value & 0x0080) ? "+merge mode" : "",
	(value & 0x4000) ? "On" : "Off", (value & 0x0008) ? "7.16":"5.37", DCCModes[value & 0x3]);
   *special = std::string(buf);
  }
 }
 else if(name == "Line")
  value = fx_vce.raster_counter;
 else if(name == "PALRWOF")
  value = fx_vce.palette_rw_offset;
 else if(name == "PALRWLA")
  value = fx_vce.palette_rw_latch;
 else if(name == "PALOFS0")
  value = fx_vce.palette_offset[0];
 else if(name == "PALOFS1")
  value = fx_vce.palette_offset[1];
 else if(name == "PALOFS2")
  value = fx_vce.palette_offset[2];
 else if(name == "PALOFS3")
  value = fx_vce.palette_offset[3];
 else if(name == "CKeyY")
  value = fx_vce.ChromaKeyY;
 else if(name == "CKeyU")
  value = fx_vce.ChromaKeyU;
 else if(name == "CKeyV")
  value = fx_vce.ChromaKeyV;

 else if(name == "CCR")
  value = fx_vce.CCR;
 else if(name == "BLE")
 {
  value = fx_vce.BLE;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "%s(%s), Rainbow: %d, BG3: %d, BG2: %d, BG1: %d, BG0: %d, VDC SP: %d, VDC BG: %d", (value & 0x8000) ? "Front" : "Back", (value & 0x4000) ? "On" : "Off", (value >> 12) & 0x3,
		(value >> 10) & 3, (value >> 8) & 3, (value >> 6) & 3, (value >> 4) & 3, (value >> 2) & 3, value & 3);
   *special = std::string(buf);
  }
 }
 else if(name == "SPBL")
  value = fx_vce.SPBL;
 else if(!strncasecmp(name.c_str(), "COEFF", strlen("COEFF")))
 {
  int which = name.c_str()[strlen("COEFF")] - '0';
  value = fx_vce.coefficients[which];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "Y: %1d, U: %1d, V: %1d", (value >> 8) & 0xF, (value >> 4) & 0xf, value & 0xf);
   *special = std::string(buf);
  }
 } 

 return(value);
}

void KING_SetGraphicsDecode(MDFN_Surface *decode_surface, int line, int which, int xscroll, int yscroll, int pbn)
{
 GfxDecode_Buf = decode_surface;
 GfxDecode_Line = line;
 GfxDecode_Layer = which;
 GfxDecode_Scroll = yscroll;
 GfxDecode_PBN = pbn;

 if(GfxDecode_Buf && GfxDecode_Line == -1)
  DoGfxDecode();
}

static void DoGfxDecode(void)
{
 const uint32 alpha_or = (0xFF << GfxDecode_Buf->format.Ashift);
 int pbn = GfxDecode_PBN;

 if(GfxDecode_Layer >= 4 && GfxDecode_Layer <= 7)
 {
  uint32 palette_offset = fx_vce.palette_offset[0] >> (((GfxDecode_Layer - 4) & 1) * 8);
  palette_offset <<= 1;
  palette_offset &= 0x1FF;
  uint32 *palette_ptr = &vce_rendercache.palette_table_cache[palette_offset];
  uint32 neo_palette[16];

  if(pbn == -1)
  {
   for(int x = 0; x < 16; x++)
   {
    int ccval = (int)(255 * pow((double)x / 15, 1.0f / 2.2));
    neo_palette[x] = GfxDecode_Buf->MakeColor(ccval, ccval, ccval, 0xFF);
   }
  }
  else
  {
   pbn &= 0x0F;
   palette_ptr += pbn * 16;

   for(int x = 0; x < 16; x++) 
    neo_palette[x] = YUV888_TO_RGB888(palette_ptr[x]) | alpha_or;
  }

  //FXVDC_DoGfxDecode(fx_vdc_chips[(GfxDecode_Layer - 4) / 2], neo_palette, GfxDecode_Buf, GfxDecode_Scroll, (GfxDecode_Layer - 4) & 1);
  fx_vdc_chips[(GfxDecode_Layer - 4) / 2]->DoGfxDecode(GfxDecode_Buf->pixels, neo_palette, GfxDecode_Buf->MakeColor(0, 0, 0, 0xFF), (GfxDecode_Layer - 4) & 1, GfxDecode_Buf->w, GfxDecode_Buf->h, GfxDecode_Scroll);
 }
 else if(GfxDecode_Layer < 4)
 {
  int n = GfxDecode_Layer;
  uint16 bgmode = (king->bgmode >> (n * 4)) & 0xF;
  uint32 cg_offset = king->BGCGAddr[n] * 1024;
  uint32 bat_and_cg_page = (king->PageSetting & 0x0010) ? 1 : 0;
  uint32 *target = GfxDecode_Buf->pixels;
  uint32 layer_or = 0;
  uint32 page_addr_or = bat_and_cg_page ? 0x40000 : 0x00000;
  uint32 palette_offset = fx_vce.palette_offset[1 + (n >> 1)] >> ((n & 1) ? 8 : 0);
  palette_offset <<= 1;
  palette_offset &= 0x1FF;
  uint32 *palette_ptr = &vce_rendercache.palette_table_cache[palette_offset];
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

   if(tile_width > GfxDecode_Buf->w) tile_width = GfxDecode_Buf->w;
   if(tile_height > GfxDecode_Buf->h) tile_height = GfxDecode_Buf->h;
  }

  switch(bgmode & 0x7)
  {
   default: memset(target, 0, GfxDecode_Buf->w * GfxDecode_Buf->h * sizeof(uint32) * 3);
	    break;

   case 0x01: // 4
   {
    pbn *= 4;
    pbn &= 0x1FF;

    for(int y = 0; y < GfxDecode_Buf->h; y++)
    {
     for(int x = 0; x < GfxDecode_Buf->w; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Buf->w / 8);
      uint16 cg = king->KRAM[bat_and_cg_page][(cg_offset + (which_tile * 8) + (y & 0x7)) & 0x3FFFF];

      DRAWBG8x1_4(target + x, &cg, palette_ptr + pbn, layer_or);

      target[x + GfxDecode_Buf->w * 2 + 0] = target[x + GfxDecode_Buf->w * 2 + 1] = target[x + GfxDecode_Buf->w * 2 + 2] = target[x + GfxDecode_Buf->w * 2 + 3] =
      target[x + GfxDecode_Buf->w * 2 + 4] = target[x + GfxDecode_Buf->w * 2 + 5] = target[x + GfxDecode_Buf->w * 2 + 6] = target[x + GfxDecode_Buf->w * 2 + 7] = ((cg_offset + (which_tile * 8)) & 0x3FFFF) | page_addr_or;

      target[x + GfxDecode_Buf->w*1 + 0]=target[x + GfxDecode_Buf->w*1 + 1]=target[x + GfxDecode_Buf->w*1 + 2]=target[x + GfxDecode_Buf->w*1 + 3] =
      target[x + GfxDecode_Buf->w*1 + 4]=target[x + GfxDecode_Buf->w*1 + 5]=target[x + GfxDecode_Buf->w*1 + 6]=target[x + GfxDecode_Buf->w*1 + 7] = which_tile;
     }
     for(int x = 0; x < GfxDecode_Buf->w; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | alpha_or;
     target += GfxDecode_Buf->w * 3;
    }
   }
   break;

   case 0x02: // 16
   {
    pbn *= 8;
    pbn &= 0x1FF;
    for(int y = 0; y < GfxDecode_Buf->h; y++)
    {
     for(int x = 0; x < GfxDecode_Buf->w; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Buf->w / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_page][(cg_offset + (which_tile * 16) + (y & 0x7) * 2) & 0x3FFFF];

      DRAWBG8x1_16(target + x, cgptr, palette_ptr + pbn, layer_or);

      target[x + GfxDecode_Buf->w*2 + 0] = target[x + GfxDecode_Buf->w*2 + 1] = target[x + GfxDecode_Buf->w*2 + 2] = target[x + GfxDecode_Buf->w*2 + 3] =
      target[x + GfxDecode_Buf->w*2 + 4] = target[x + GfxDecode_Buf->w*2 + 5] = target[x + GfxDecode_Buf->w*2 + 6] = target[x + GfxDecode_Buf->w*2 + 7] = ((cg_offset + (which_tile * 16)) & 0x3FFFF) | page_addr_or;

      target[x + GfxDecode_Buf->w*1 + 0]=target[x + GfxDecode_Buf->w*1 + 1]=target[x + GfxDecode_Buf->w*1 + 2]=target[x + GfxDecode_Buf->w*1 + 3] =
      target[x + GfxDecode_Buf->w*1 + 4]=target[x + GfxDecode_Buf->w*1 + 5]=target[x + GfxDecode_Buf->w*1 + 6]=target[x + GfxDecode_Buf->w*1 + 7] = which_tile;
     }
     for(int x = 0; x < GfxDecode_Buf->w; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | alpha_or;
     target += GfxDecode_Buf->w * 3;
    }
   }
   break;

   case 0x03: // 256
   {
    for(int y = 0; y < GfxDecode_Buf->h; y++)
    {
     for(int x = 0; x < GfxDecode_Buf->w; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Buf->w / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_page][(cg_offset + (which_tile * 32) + (y & 0x7) * 4) & 0x3FFFF];

      DRAWBG8x1_256(target + x, cgptr, palette_ptr, layer_or);

      target[x + GfxDecode_Buf->w*2 + 0] = target[x + GfxDecode_Buf->w*2 + 1] = target[x + GfxDecode_Buf->w*2 + 2] = target[x + GfxDecode_Buf->w*2 + 3] =
      target[x + GfxDecode_Buf->w*2 + 4] = target[x + GfxDecode_Buf->w*2 + 5] = target[x + GfxDecode_Buf->w*2 + 6] = target[x + GfxDecode_Buf->w*2 + 7] = ((cg_offset + (which_tile * 32)) & 0x3FFFF) | page_addr_or;

      target[x + GfxDecode_Buf->w*1 + 0]=target[x + GfxDecode_Buf->w*1 + 1]=target[x + GfxDecode_Buf->w*1 + 2]=target[x + GfxDecode_Buf->w*1 + 3] =
      target[x + GfxDecode_Buf->w*1 + 4]=target[x + GfxDecode_Buf->w*1 + 5]=target[x + GfxDecode_Buf->w*1 + 6]=target[x + GfxDecode_Buf->w*1 + 7] = which_tile;
     }

     for(int x = 0; x < GfxDecode_Buf->w; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | alpha_or;

     target += GfxDecode_Buf->w * 3;
    }
   }
   break;

   case 0x04: // 64K
    for(int y = 0; y < GfxDecode_Buf->h; y++)
    {
     for(int x = 0; x < GfxDecode_Buf->w; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Buf->w / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_page][(cg_offset + (which_tile * 64) + (y & 0x7) * 8) & 0x3FFFF];

      DRAWBG8x1_64K(target + x, cgptr, palette_ptr, layer_or);
     }

     for(int x = 0; x < GfxDecode_Buf->w; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | alpha_or;

     target += GfxDecode_Buf->w * 3;
    }
   break;

   case 0x05: // 16M
    for(int y = 0; y < GfxDecode_Buf->h; y++)
    {
     for(int x = 0; x < GfxDecode_Buf->w; x+=8)
     {
      int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Buf->w / 8);
      uint16 *cgptr = &king->KRAM[bat_and_cg_page][(cg_offset + (which_tile * 64) + (y & 0x7) * 8) & 0x3FFFF];

      DRAWBG8x1_16M(target + x, cgptr, palette_ptr, layer_or);
     }

     for(int x = 0; x < GfxDecode_Buf->w; x++)
      target[x] = YUV888_TO_RGB888(target[x]) | alpha_or;

     target += GfxDecode_Buf->w * 3;
    }
   break;

  }
 }
 else
  memset(GfxDecode_Buf->pixels, 0, GfxDecode_Buf->w * GfxDecode_Buf->h * sizeof(uint32) * 3);
}

#endif
