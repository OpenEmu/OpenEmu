/********************************************************************************
 SEGA Genesis / Mega Drive Driver for FBA 
 ********************************************************************************
 This is part of Pico Library v0936

 (c) Copyright 2004 Dave, All rights reserved.
 (c) Copyright 2006 notaz, All rights reserved.
 Free for non-commercial use.

 For commercial use, separate licencing terms must be obtained. 
 ********************************************************************************

 PicoOpt bits LSb->MSb:
 enable_ym2612&dac, enable_sn76496, enable_z80, stereo_sound,
 alt_renderer, 6button_gamepad, accurate_timing, accurate_sprites,
 draw_no_32col_border, external_ym2612

 ********************************************************************************
 Port by OopsWare
 ********************************************************************************/

#include "burnint.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2612.h"
#include "sn76496.h"
#include "megadrive.h"
#include "bitswap.h"
#include "sekdebug.h"

#define OSC_NTSC 53693175
#define OSC_PAL  53203424

#define MAX_CARTRIDGE_SIZE	0xc00000
#define MAX_SRAM_SIZE		0x010000

static INT32 cycles_68k, cycles_z80;

typedef void (*MegadriveCb)();
static MegadriveCb MegadriveCallback;

struct PicoVideo {
	UINT8 reg[0x20];
	UINT32 command;		// 32-bit Command
	UINT8 pending;		// 1 if waiting for second half of 32-bit command
	UINT8 type;			// Command type (v/c/vsram read/write)
	UINT16 addr;		// Read/Write address
	INT32 status;					// Status bits
	UINT8 pending_ints;	// pending interrupts: ??VH????
	UINT8 pad[0x13];	//
};

struct PicoMisc {
	UINT32 Z80Run;
	UINT32 Bank68k;
	UINT8 Rotate;

//	UINT8 Pad[3];
	
//	UINT32 SRamReg;
	UINT32 SRamStart;
	UINT32 SRamEnd;
	UINT32 SRamDetected;
	UINT32 SRamActive;
	UINT32 SRamHandlersInstalled;
	UINT32 SRamReadOnly;
	UINT32 SRamHasSerialEEPROM;
	
	UINT8 I2CMem;
	UINT8 I2CClk;
	
	UINT16 JCartIOData[2];
	
	UINT8 L3AltPDat;
	UINT8 L3AltPCmd;
	
	UINT16 SquirrelkingExtra;
	
	UINT16 Lionk2ProtData;
	UINT16 Lionk2ProtData2;
	
	UINT32 RealtecBankAddr;
	UINT32 RealtecBankSize;
};

struct TileStrip
{
	INT32 nametab; // Position in VRAM of name table (for this tile line)
	INT32 line;    // Line number in pixels 0x000-0x3ff within the virtual tilemap 
	INT32 hscroll; // Horizontal scroll value in pixels for the line
	INT32 xmask;   // X-Mask (0x1f - 0x7f) for horizontal wraparound in the tilemap
	INT32 *hc;     // cache for high tile codes and their positions
	INT32 cells;   // cells (tiles) to draw (32 col mode doesn't need to update whole 320)
};

struct MegadriveJoyPad {
	UINT16 pad[4];
	UINT8  padTHPhase[4];
	UINT8  padDelay[4];
};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

static UINT8 *RomMain;
static UINT8 *OriginalRom;

static UINT8 *Ram68K;
static UINT8 *RamZ80;

static UINT8 *SRam;
static UINT8 *RamIO;

static UINT16 *RamPal;
static UINT16 *RamVid;
static UINT16 *RamSVid;
static struct PicoVideo *RamVReg;
static struct PicoMisc *RamMisc;
static struct MegadriveJoyPad *JoyPad;

UINT16 *MegadriveCurPal;

static UINT16 *MegadriveBackupRam;

static UINT8 *HighCol;
static UINT8 *HighColFull;

static INT32 *HighCacheA;
static INT32 *HighCacheB;
static INT32 *HighCacheS;
static INT32 *HighPreSpr;
static INT8 *HighSprZ;

UINT8 MegadriveReset = 0;
UINT8 bMegadriveRecalcPalette = 0;

UINT8 MegadriveJoy1[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
UINT8 MegadriveJoy2[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
UINT8 MegadriveJoy3[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
UINT8 MegadriveJoy4[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
UINT8 MegadriveDIP[2] = {0, 0};

static UINT32 RomNum = 0;
static UINT32 RomSize = 0;
static UINT32 SRamSize = 0;

static INT32 SpriteBlocks;

static INT32 Scanline = 0;

static INT32 Z80HasBus = 0;
static INT32 MegadriveZ80Reset = 0;
static INT32 RomNoByteswap;

static UINT8 Hardware;
static UINT8 DrvSECAM = 0;	// NTSC 

void MegadriveCheckHardware()
{
	Hardware = MegadriveDIP[0] & 0xe0;
	if (MegadriveDIP[0] & 0x01) {
		// Auto Detect Region and SECAM
		UINT32 support = 0;
		for (INT32 i = 0; i < 4; i++) {
			UINT32 v = RomMain[0x1f0 + i];
			if (v <= 0x20) continue;

			switch (v) {
				case 0x30:
				case 0x31:
				case 0x32:
				case 0x33:
				case 0x34:
				case 0x35:
				case 0x36:
				case 0x37:
				case 0x38:
				case 0x39: {
					support |= v - 0x30;
					break;
				}
				
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x46: {
					support |= v - 0x41;
					break;
				}
				
				case 0x45: {
					// Japan
					support |= 0x08;
					break;
				}
				
				case 0x4a: {
					// Europe
					support |= 0x01;
					break;
				}
				
				case 0x55: {
					// USA
					support |= 0x04;
					break;
				}
				
				case 0x61:
				case 0x62:
				case 0x63:
				case 0x64:
				case 0x65:
				case 0x66: {
					support |= v - 0x61;
					break;
				}
			}
		}

		bprintf(PRINT_IMPORTANT, _T("Autodetecting Cartridge (Hardware Code: %02x%02x%02x%02x):\n"), RomMain[0x1f0], RomMain[0x1f1], RomMain[0x1f2], RomMain[0x1f3]);
		Hardware = 0x80;
		
		if (support & 0x02) {
			Hardware = 0x40; // Japan PAL
			bprintf(PRINT_IMPORTANT, _T("Japan PAL supported ???\n"));
		}
		
		if (support & 0x01) {
			Hardware = 0x00; // Japan NTSC
			bprintf(PRINT_IMPORTANT, _T("Japan NTSC supported\n"));
		}		
		
		if (support & 0x08) {
			Hardware = 0xc0; // Europe PAL
			bprintf(PRINT_IMPORTANT, _T("Europe PAL supported\n"));
		}
		
		if (support & 0x04) {
			Hardware = 0x80; // USA NTSC
			bprintf(PRINT_IMPORTANT, _T("USA NTSC supported\n"));
		}
		
		if ((Hardware & 0xc0) == 0xc0) {
			bprintf(PRINT_IMPORTANT, _T("Emulating Europe PAL Machine\n"));
		} else {
			if ((Hardware & 0x80) == 0x80) {
				bprintf(PRINT_IMPORTANT, _T("Emulating USA NTSC Machine\n"));
			} else {
				if ((Hardware & 0x40) == 0x40) {
					bprintf(PRINT_IMPORTANT, _T("Emulating Japan PAL Machine ???\n"));
				} else {
					if ((Hardware & 0x00) == 0x00) {
						bprintf(PRINT_IMPORTANT, _T("Emulating Japan NTSC Machine\n"));
					}
				}
			}
		}
		
		// CD-ROM
		Hardware |= MegadriveDIP[0] & 0x20;
	}
	
	if ((Hardware & 0x20) != 0x20) bprintf(PRINT_IMPORTANT, _T("Emulating Mega-CD Add-on\n"));
}

//-----------------------------------------------------------------

inline static void CalcCol(INT32 index, UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x000f) << 4;	// Red
	g = (nColour & 0x00f0) << 0; 	// Green
	b = (nColour & 0x0f00) >> 4;	// Blue

	r |= r >> 4;
	g |= g >> 4;
	b |= b >> 4;
	
	RamPal[index] = nColour;
	
	// Normal Color
	MegadriveCurPal[index + 0x00] = BurnHighCol(r, g, b, 0);
	
	// Shadow Color
	MegadriveCurPal[index + 0x40] = MegadriveCurPal[index + 0xc0] = BurnHighCol(r>>1, g>>1, b>>1, 0);
	
	// Highlight Color
	r += 0x80; if (r > 0xFF) r = 0xFF;
	g += 0x80; if (g > 0xFF) g = 0xFF;
	b += 0x80; if (b > 0xFF) b = 0xFF;
	MegadriveCurPal[index + 0x80] = BurnHighCol(r, g, b, 0);
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	RomMain 	= Next; Next += MAX_CARTRIDGE_SIZE;	// 68000 ROM, Max enough 
	
	RamStart	= Next;
	
	Ram68K		= Next; Next += 0x010000;
	RamZ80		= Next; Next += 0x002000;
	SRam		= Next; Next += MAX_SRAM_SIZE;		// SRam
	RamIO		= Next; Next += 0x000010;			// I/O
	
	RamPal		= (UINT16 *) Next; Next += 0x000040 * sizeof(UINT16);
	RamSVid		= (UINT16 *) Next; Next += 0x000040 * sizeof(UINT16);	// VSRam
	RamVid		= (UINT16 *) Next; Next += 0x008000 * sizeof(UINT16);	// Video Ram
	
	RamVReg		= (struct PicoVideo *)Next; Next += sizeof(struct PicoVideo);
	RamMisc		= (struct PicoMisc *)Next; Next += sizeof(struct PicoMisc);
	
	RamEnd		= Next;

	MegadriveCurPal		= (UINT16 *) Next; Next += 0x000040 * sizeof(UINT16) * 4;
	
	HighColFull	= Next; Next += (8 + 320 + 8) * 240;
	
	HighCacheA	= (INT32 *) Next; Next += (41+1) * sizeof(INT32);	// caches for high layers
	HighCacheB	= (INT32 *) Next; Next += (41+1) * sizeof(INT32);
	HighCacheS	= (INT32 *) Next; Next += (80+1) * sizeof(INT32);	// and sprites
	HighPreSpr	= (INT32 *) Next; Next += (80*2+1) * sizeof(INT32);	// slightly preprocessed sprites
	HighSprZ	= (INT8*) Next; Next += (320+8+8);				// Z-buffer for accurate sprites and shadow/hilight mode
	
	JoyPad		= (struct MegadriveJoyPad *) Next; Next += sizeof(struct MegadriveJoyPad);

	MemEnd		= Next;
	return 0;
}

UINT16 __fastcall MegadriveReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xa11100: {
			UINT16 retVal = rand() & 0xffff;
			if (Z80HasBus || MegadriveZ80Reset) {
				retVal |= 0x100;
			} else {
				retVal &= 0xfeff;
			}
			return retVal;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

UINT8 __fastcall MegadriveReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xa04000:
		case 0xa04001:
		case 0xa04002:
		case 0xa04003: {
			if (!Z80HasBus && !MegadriveZ80Reset) {
				return BurnYM2612Read(0, 0);
			} else {
				return 0;
			}
		}
				
		case 0xa11100: {
			UINT8 retVal = rand() & 0xff;
			if (Z80HasBus || MegadriveZ80Reset) {
				retVal |= 0x01;
			} else {
				retVal &= 0xfe;
			}
			return retVal;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

void __fastcall MegadriveWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if(sekAddress >= 0xA13004 && sekAddress < 0xA13040) {
		// dumb 12-in-1 or 4-in-1 banking support
		sekAddress &= 0x3f; 
		sekAddress <<= 16;
		INT32 len = RomSize - sekAddress;
		if (len <= 0) return; // invalid/missing bank
		if (len > 0x200000) len = 0x200000; // 2 megs
		// code which does this is in RAM so this is safe.
		memcpy(RomMain, RomMain + sekAddress, len); 
		return;
	}

	switch (sekAddress) {
		case 0xa04000: {
			if (!Z80HasBus && !MegadriveZ80Reset) {
				BurnYM2612Write(0, 0, byteValue);
			}
			return;
		}
	
		case 0xa04001: {
			if (!Z80HasBus && !MegadriveZ80Reset) {
				BurnYM2612Write(0, 1, byteValue);
			}
			return;
		}
	
		case 0xa04002: {
			if (!Z80HasBus && !MegadriveZ80Reset) {
				BurnYM2612Write(0, 2, byteValue);
			}
			return;
		}
	
		case 0xa04003: {
			if (!Z80HasBus && !MegadriveZ80Reset) {
				BurnYM2612Write(0, 3, byteValue);
			}
			return;
		}
		
		case 0xA11100: {
			if (byteValue & 1) {
				RamMisc->Z80Run = 0;
				Z80HasBus = 0;
			} else {
				RamMisc->Z80Run = 1;
				Z80HasBus = 1;
			}
			return;
		}

		case 0xA11200: {
			if (!(byteValue & 1)) {
				ZetOpen(0);
				ZetReset();
				ZetClose();

				BurnYM2612Reset();
				MegadriveZ80Reset = 1;	
			} else {
				MegadriveZ80Reset = 0;
			}
			return;
		}
	
//		case 0xA130F1: {
			// sram access register
//			RamMisc->SRamReg = byteValue & 0x03;
//			return;
//		}
			
		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
		}		
	}
}

void __fastcall MegadriveWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0xa11100: {
			if (wordValue & 0x100) {
				RamMisc->Z80Run = 0;
				Z80HasBus = 0;
			} else {
				RamMisc->Z80Run = 1;
				Z80HasBus = 1;
			}
			return;
		}
		
		case 0xa11200: {
			if (!(wordValue & 0x100)) {
				ZetOpen(0);
				ZetReset();
				ZetClose();

				BurnYM2612Reset();
				MegadriveZ80Reset = 1;
			} else {
				MegadriveZ80Reset = 0;
			}
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
		}
	}
}

//---------------------------------------------------------------
// Megadrive Video Port Read Write
//---------------------------------------------------------------

static INT32 rendstatus = 0;

// calculate the number of cycles 68k->VDP dma operation would take
static INT32 DmaSlowBurn(INT32 len)
{
	// test: Legend of Galahad, Time Killers
	INT32 burn,maxlen,line=Scanline;
	//if(line == -1) line=vcounts[SekCyclesDone()>>8];
	maxlen = (224-line) * 18;
	if(len <= maxlen)
		burn = len*(((cycles_68k<<8)/18))>>8;
	else {
		burn  = maxlen*(((cycles_68k<<8)/18))>>8;
		burn += (len-maxlen)*(((cycles_68k<<8)/180))>>8;
	}
	return burn;
}

static INT32 GetDmaLength()
{
  INT32 len = 0;
  // 16-bit words to transfer:
  len  = RamVReg->reg[0x13];
  len |= RamVReg->reg[0x14]<<8;
  // Charles MacDonald:
  if(!len) len = 0xffff;
  return len;
}

static void DmaSlow(INT32 len)
{
	UINT16 *pd=0, *pdend, *r;
	UINT32 a = RamVReg->addr, a2, d;
	UINT8 inc = RamVReg->reg[0xf];
	UINT32 source, burn;

	source  = RamVReg->reg[0x15] <<  1;
	source |= RamVReg->reg[0x16] <<  9;
	source |= RamVReg->reg[0x17] << 17;

  //dprintf("DmaSlow[%i] %06x->%04x len %i inc=%i blank %i [%i|%i]", Pico.video.type, source, a, len, inc,
  //         (Pico.video.status&8)||!(Pico.video.reg[1]&0x40), Pico.m.scanline, SekCyclesDone());

	if ((source & 0xe00000) == 0xe00000) { // RAM
		pd    = (UINT16 *)(Ram68K + (source & 0xfffe));
		pdend = (UINT16 *)(Ram68K + 0x10000); 
	} else if( source < RomSize) {	// ROM
		pd    = (UINT16 *)(RomMain + (source & ~1)); 
		pdend = (UINT16 *)(RomMain + RomSize); 
	} else return; // Invalid source address

	// CPU is stopped during DMA, so we burn some cycles to compensate that
	if((RamVReg->status & 8) || !(RamVReg->reg[1] & 0x40)) { 	// vblank?
		burn = (len*(((cycles_68k<<8)/167))>>8); 						// very approximate
		if(!(RamVReg->status & 8)) burn += burn>>1;				// a hack for Legend of Galahad
	} else burn = DmaSlowBurn(len);
	
	//SekCyclesBurn(burn);
	SekRunAdjust( 0 - burn );
	
	if(!(RamVReg->status & 8))
		SekRunEnd();
	//dprintf("DmaSlow burn: %i @ %06x", burn, SekPc);

	switch ( RamVReg->type ) {
	case 1: // vram
		r = RamVid;
		for(; len; len--) {
			d = *pd++;
			if(a&1) d=(d<<8)|(d>>8);
			r[a>>1] = (UINT16)d; // will drop the upper bits
			// AutoIncrement
			a = (UINT16)(a+inc);
			// didn't src overlap?
			if(pd >= pdend) pd -= 0x8000; // should be good for RAM, bad for ROM
		}
		rendstatus |= 0x10;
		break;
    
	case 3: // cram
		//dprintf("DmaSlow[%i] %06x->%04x len %i inc=%i blank %i [%i|%i]", Pico.video.type, source, a, len, inc,
		//         (Pico.video.status&8)||!(Pico.video.reg[1]&0x40), Pico.m.scanline, SekCyclesDone());
		for(a2 = a&0x7f; len; len--) {
			CalcCol( a2>>1, BURN_ENDIAN_SWAP_INT16(*pd) );
			pd++;
			// AutoIncrement
			a2+=inc;
			// didn't src overlap?
			if(pd >= pdend) pd-=0x8000;
			// good dest?
			if(a2 >= 0x80) break; // Todds Adventures in Slime World / Andre Agassi tennis
		}
		a = (a&0xff00) | a2;
		break;

	case 5: // vsram[a&0x003f]=d;
		r = RamSVid;
		for(a2=a&0x7f; len; len--) {
			r[a2>>1] = (UINT16)*pd++;
			// AutoIncrement
			a2+=inc;
			// didn't src overlap?
			if(pd >= pdend) pd-=0x8000;
			// good dest?
			if(a2 >= 0x80) break;
		}
		a=(a&0xff00)|a2;
		break;
	}
	// remember addr
	RamVReg->addr = (UINT16)a;
}

static void DmaCopy(INT32 len)
{
	UINT8 * vr = (UINT8 *) RamVid;
	UINT8 * vrs;
	//UINT16 a = Pico.video.addr;
	//UINT8 inc = RamVReg->reg[0xf];
	INT32 source;
	
	//dprintf("DmaCopy len %i [%i|%i]", len, Pico.m.scanline, SekCyclesDone());

	source  = RamVReg->reg[0x15];
	source |= RamVReg->reg[0x16]<<8;
	vrs = vr + source;
	
	if (source+len > 0x10000) 
		len = 0x10000 - source; // clip??
	
	for(;len;len--) {
		vr[RamVReg->addr] = *vrs++;
		// AutoIncrement
		//a = (u16)(a + inc);
		RamVReg->addr += RamVReg->reg[0xf];
	}
	rendstatus |= 0x10;
}

static void DmaFill(INT32 data)
{
	INT32 len = GetDmaLength();
	UINT8 *vr = (UINT8 *) RamVid;
	UINT8 high = (UINT8) (data >> 8);
	UINT16 a = RamVReg->addr;
	UINT8 inc = RamVReg->reg[0xf];
  
	//dprintf("DmaFill len %i inc %i [%i|%i]", len, inc, Pico.m.scanline, SekCyclesDone());

	// from Charles MacDonald's genvdp.txt:
	// Write lower byte to address specified
	vr[a] = (UINT8) data;
	a = (UINT16)(a+inc);

	if(!inc) len=1;

	for(;len;len--) {
		// Write upper byte to adjacent address
		// (here we are byteswapped, so address is already 'adjacent')
		vr[a] = high;
		// Increment address register
		a = (UINT16)(a+inc);
	}
	// remember addr
	RamVReg->addr = a;
	// update length
	RamVReg->reg[0x13] = RamVReg->reg[0x14] = 0; // Dino Dini's Soccer (E) (by Haze)

	rendstatus |= 0x10;
}

static void CommandChange()
{
	//struct PicoVideo *pvid=&Pico.video;
	UINT32 cmd = RamVReg->command;
	UINT32 addr = 0;

	// Get type of transfer 0xc0000030 (v/c/vsram read/write)
	RamVReg->type = (UINT8)(((cmd >> 2) & 0xc) | (cmd >> 30));

	// Get address 0x3fff0003
	addr  = (cmd >> 16) & 0x3fff;
	addr |= (cmd << 14) & 0xc000;
	RamVReg->addr = (UINT16)addr;
	//dprintf("addr set: %04x", addr);

	// Check for dma:
	if (cmd & 0x80) {
		// Command DMA
		if ((RamVReg->reg[1] & 0x10) == 0) return; // DMA not enabled
		INT32 len = GetDmaLength();
		switch ( RamVReg->reg[0x17]>>6 ) {
		case 0x00:
		case 0x01:
			DmaSlow(len);	// 68000 to VDP
			break;
		case 0x03:
			DmaCopy(len);	// VRAM Copy
			break;
		case 0x02:			// DMA Fill Flag ???
		default:
			;//bprintf(PRINT_NORMAL, _T("Video Command DMA Unknown %02x len %d\n"), RamVReg->reg[0x17]>>6, len);
		}
	}
}

// H-counter table for hvcounter reads in 40col mode
// based on Gens code
const UINT8 hcounts_40[] = {
	0x07,0x07,0x08,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,
	0x0e,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x13,0x14,
	0x14,0x15,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,
	0x1b,0x1b,0x1c,0x1c,0x1d,0x1d,0x1d,0x1e,0x1e,0x1f,0x1f,0x20,0x20,0x20,0x21,0x21,
	0x22,0x22,0x23,0x23,0x23,0x24,0x24,0x25,0x25,0x25,0x26,0x26,0x27,0x27,0x28,0x28,
	0x28,0x29,0x29,0x2a,0x2a,0x2a,0x2b,0x2b,0x2c,0x2c,0x2d,0x2d,0x2d,0x2e,0x2e,0x2f,
	0x2f,0x30,0x30,0x30,0x31,0x31,0x32,0x32,0x32,0x33,0x33,0x34,0x34,0x35,0x35,0x35,
	0x36,0x36,0x37,0x37,0x38,0x38,0x38,0x39,0x39,0x3a,0x3a,0x3a,0x3b,0x3b,0x3c,0x3c,
	0x3d,0x3d,0x3d,0x3e,0x3e,0x3f,0x3f,0x3f,0x40,0x40,0x41,0x41,0x42,0x42,0x42,0x43,
	0x43,0x44,0x44,0x45,0x45,0x45,0x46,0x46,0x47,0x47,0x47,0x48,0x48,0x49,0x49,0x4a,
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4c,0x4d,0x4d,0x4d,0x4e,0x4e,0x4f,0x4f,0x4f,0x50,0x50,
	0x51,0x51,0x52,0x52,0x52,0x53,0x53,0x54,0x54,0x55,0x55,0x55,0x56,0x56,0x57,0x57,
	0x57,0x58,0x58,0x59,0x59,0x5a,0x5a,0x5a,0x5b,0x5b,0x5c,0x5c,0x5c,0x5d,0x5d,0x5e,
	0x5e,0x5f,0x5f,0x5f,0x60,0x60,0x61,0x61,0x62,0x62,0x62,0x63,0x63,0x64,0x64,0x64,
	0x65,0x65,0x66,0x66,0x67,0x67,0x67,0x68,0x68,0x69,0x69,0x6a,0x6a,0x6a,0x6b,0x6b,
	0x6c,0x6c,0x6c,0x6d,0x6d,0x6e,0x6e,0x6f,0x6f,0x6f,0x70,0x70,0x71,0x71,0x71,0x72,
	0x72,0x73,0x73,0x74,0x74,0x74,0x75,0x75,0x76,0x76,0x77,0x77,0x77,0x78,0x78,0x79,
	0x79,0x79,0x7a,0x7a,0x7b,0x7b,0x7c,0x7c,0x7c,0x7d,0x7d,0x7e,0x7e,0x7f,0x7f,0x7f,
	0x80,0x80,0x81,0x81,0x81,0x82,0x82,0x83,0x83,0x84,0x84,0x84,0x85,0x85,0x86,0x86,
	0x86,0x87,0x87,0x88,0x88,0x89,0x89,0x89,0x8a,0x8a,0x8b,0x8b,0x8c,0x8c,0x8c,0x8d,
	0x8d,0x8e,0x8e,0x8e,0x8f,0x8f,0x90,0x90,0x91,0x91,0x91,0x92,0x92,0x93,0x93,0x94,
	0x94,0x94,0x95,0x95,0x96,0x96,0x96,0x97,0x97,0x98,0x98,0x99,0x99,0x99,0x9a,0x9a,
	0x9b,0x9b,0x9b,0x9c,0x9c,0x9d,0x9d,0x9e,0x9e,0x9e,0x9f,0x9f,0xa0,0xa0,0xa1,0xa1,
	0xa1,0xa2,0xa2,0xa3,0xa3,0xa3,0xa4,0xa4,0xa5,0xa5,0xa6,0xa6,0xa6,0xa7,0xa7,0xa8,
	0xa8,0xa9,0xa9,0xa9,0xaa,0xaa,0xab,0xab,0xab,0xac,0xac,0xad,0xad,0xae,0xae,0xae,
	0xaf,0xaf,0xb0,0xb0,0xe4,0xe4,0xe4,0xe5,0xe5,0xe6,0xe6,0xe6,0xe7,0xe7,0xe8,0xe8,
	0xe9,0xe9,0xe9,0xea,0xea,0xeb,0xeb,0xeb,0xec,0xec,0xed,0xed,0xee,0xee,0xee,0xef,
	0xef,0xf0,0xf0,0xf1,0xf1,0xf1,0xf2,0xf2,0xf3,0xf3,0xf3,0xf4,0xf4,0xf5,0xf5,0xf6,
	0xf6,0xf6,0xf7,0xf7,0xf8,0xf8,0xf9,0xf9,0xf9,0xfa,0xfa,0xfb,0xfb,0xfb,0xfc,0xfc,
	0xfd,0xfd,0xfe,0xfe,0xfe,0xff,0xff,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,
	0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x08,0x08,0x08,0x09,0x09,0x0a,
	0x0a,0x0b,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x10,
};

// H-counter table for hvcounter reads in 32col mode
const UINT8 hcounts_32[] = {
	0x05,0x05,0x05,0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,0x09,0x09,0x09,0x0a,0x0a,
	0x0a,0x0b,0x0b,0x0b,0x0c,0x0c,0x0c,0x0d,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x0f,0x10,
	0x10,0x10,0x11,0x11,0x11,0x12,0x12,0x12,0x13,0x13,0x13,0x14,0x14,0x14,0x15,0x15,
	0x15,0x16,0x16,0x17,0x17,0x17,0x18,0x18,0x18,0x19,0x19,0x19,0x1a,0x1a,0x1a,0x1b,
	0x1b,0x1b,0x1c,0x1c,0x1c,0x1d,0x1d,0x1d,0x1e,0x1e,0x1f,0x1f,0x1f,0x20,0x20,0x20,
	0x21,0x21,0x21,0x22,0x22,0x22,0x23,0x23,0x23,0x24,0x24,0x24,0x25,0x25,0x26,0x26,
	0x26,0x27,0x27,0x27,0x28,0x28,0x28,0x29,0x29,0x29,0x2a,0x2a,0x2a,0x2b,0x2b,0x2b,
	0x2c,0x2c,0x2c,0x2d,0x2d,0x2e,0x2e,0x2e,0x2f,0x2f,0x2f,0x30,0x30,0x30,0x31,0x31,
	0x31,0x32,0x32,0x32,0x33,0x33,0x33,0x34,0x34,0x34,0x35,0x35,0x36,0x36,0x36,0x37,
	0x37,0x37,0x38,0x38,0x38,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3c,0x3c,
	0x3d,0x3d,0x3d,0x3e,0x3e,0x3e,0x3f,0x3f,0x3f,0x40,0x40,0x40,0x41,0x41,0x41,0x42,
	0x42,0x42,0x43,0x43,0x43,0x44,0x44,0x45,0x45,0x45,0x46,0x46,0x46,0x47,0x47,0x47,
	0x48,0x48,0x48,0x49,0x49,0x49,0x4a,0x4a,0x4a,0x4b,0x4b,0x4b,0x4c,0x4c,0x4d,0x4d,
	0x4d,0x4e,0x4e,0x4e,0x4f,0x4f,0x4f,0x50,0x50,0x50,0x51,0x51,0x51,0x52,0x52,0x52,
	0x53,0x53,0x53,0x54,0x54,0x55,0x55,0x55,0x56,0x56,0x56,0x57,0x57,0x57,0x58,0x58,
	0x58,0x59,0x59,0x59,0x5a,0x5a,0x5a,0x5b,0x5b,0x5c,0x5c,0x5c,0x5d,0x5d,0x5d,0x5e,
	0x5e,0x5e,0x5f,0x5f,0x5f,0x60,0x60,0x60,0x61,0x61,0x61,0x62,0x62,0x62,0x63,0x63,
	0x64,0x64,0x64,0x65,0x65,0x65,0x66,0x66,0x66,0x67,0x67,0x67,0x68,0x68,0x68,0x69,
	0x69,0x69,0x6a,0x6a,0x6a,0x6b,0x6b,0x6c,0x6c,0x6c,0x6d,0x6d,0x6d,0x6e,0x6e,0x6e,
	0x6f,0x6f,0x6f,0x70,0x70,0x70,0x71,0x71,0x71,0x72,0x72,0x72,0x73,0x73,0x74,0x74,
	0x74,0x75,0x75,0x75,0x76,0x76,0x76,0x77,0x77,0x77,0x78,0x78,0x78,0x79,0x79,0x79,
	0x7a,0x7a,0x7b,0x7b,0x7b,0x7c,0x7c,0x7c,0x7d,0x7d,0x7d,0x7e,0x7e,0x7e,0x7f,0x7f,
	0x7f,0x80,0x80,0x80,0x81,0x81,0x81,0x82,0x82,0x83,0x83,0x83,0x84,0x84,0x84,0x85,
	0x85,0x85,0x86,0x86,0x86,0x87,0x87,0x87,0x88,0x88,0x88,0x89,0x89,0x89,0x8a,0x8a,
	0x8b,0x8b,0x8b,0x8c,0x8c,0x8c,0x8d,0x8d,0x8d,0x8e,0x8e,0x8e,0x8f,0x8f,0x8f,0x90,
	0x90,0x90,0x91,0x91,0xe8,0xe8,0xe8,0xe9,0xe9,0xe9,0xea,0xea,0xea,0xeb,0xeb,0xeb,
	0xec,0xec,0xec,0xed,0xed,0xed,0xee,0xee,0xee,0xef,0xef,0xf0,0xf0,0xf0,0xf1,0xf1,
	0xf1,0xf2,0xf2,0xf2,0xf3,0xf3,0xf3,0xf4,0xf4,0xf4,0xf5,0xf5,0xf5,0xf6,0xf6,0xf6,
	0xf7,0xf7,0xf8,0xf8,0xf8,0xf9,0xf9,0xf9,0xfa,0xfa,0xfa,0xfb,0xfb,0xfb,0xfc,0xfc,
	0xfc,0xfd,0xfd,0xfd,0xfe,0xfe,0xfe,0xff,0xff,0x00,0x00,0x00,0x01,0x01,0x01,0x02,
	0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x07,0x07,0x07,
	0x08,0x08,0x08,0x09,0x09,0x09,0x0a,0x0a,0x0a,0x0b,0x0b,0x0b,0x0c,0x0c,0x0c,0x0d,
};

UINT16 __fastcall MegadriveVideoReadWord(UINT32 sekAddress)
{
	//bprintf(PRINT_NORMAL, _T("Video Attempt to read word value of location %x\n"), sekAddress);
	//return MegadriveVDPRead((sekAddress - 0xc00000) >> 1);
	
	if (sekAddress > 0xC0001F)
		bprintf(PRINT_NORMAL, _T("Video Attempt to read word value of location %x\n"), sekAddress);
	
	UINT16 res = 0;
	
	switch (sekAddress & 0x1c) {
	case 0x00:	// data
		switch (RamVReg->type) {
			case 0: res = BURN_ENDIAN_SWAP_INT16(RamVid [(RamVReg->addr >> 1) & 0x7fff]); break;
			case 4: res = BURN_ENDIAN_SWAP_INT16(RamSVid[(RamVReg->addr >> 1) & 0x003f]); break;
			case 8: res = BURN_ENDIAN_SWAP_INT16(RamPal [(RamVReg->addr >> 1) & 0x003f]); break;
		}
		RamVReg->addr += RamVReg->reg[0xf];
		break;

	case 0x04:	// command
		res = RamVReg->status;
		//if(PicoOpt&0x10) d|=0x0020; 							// sprite collision (Shadow of the Beast)
		if(RamMisc->Rotate++&8) res |= 0x0100; else res |= 0x0200;	// Toggle fifo full empty (who uses that stuff?)
		if(!(RamVReg->reg[1] & 0x40)) res |= 0x0008;			// set V-Blank if display is disabled
		if(m68k_ICount < 84+4) res |= 0x0004;					// H-Blank (Sonic3 vs)
		RamVReg->pending = 0;		// ctrl port reads clear write-pending flag (Charles MacDonald)		
		break;
	
	case 0x08: 	// H-counter info
		{
			UINT32 hc = 50;
	
			INT32 lineCycles = (cycles_68k - m68k_ICount) & 0x1ff;
			res = Scanline; // V-Counter
	
			if(RamVReg->reg[12]&1) 
				hc = hcounts_40[lineCycles];
			else hc = hcounts_32[lineCycles];
	
			if(lineCycles > cycles_68k-12) res++; // Wheel of Fortune

			if( Hardware & 0x40 ) {
				if(res >= 0x103) res -= 56; // based on Gens
			} else {
				if(res >= 0x0EB) res -= 6;
			}
		
			if((RamVReg->reg[12]&6) == 6) {
				// interlace mode 2 (Combat Cars (UE) [!])
				res <<= 1;
				if (res & 0xf00) res |= 1;
			}
			
			//dprintf("hv: %02x %02x (%i) @ %06x", hc, d, SekCyclesDone(), SekPc);
			res &= 0xff; 
			res <<= 8;
			res |= hc;
		}
		break;
		
	default:	
		bprintf(PRINT_NORMAL, _T("Video Attempt to read word value of location %x, %x\n"), sekAddress, sekAddress & 0x1c);
		break;
	}	
	
	return res;
}

UINT8 __fastcall MegadriveVideoReadByte(UINT32 sekAddress)
{
	//bprintf(PRINT_NORMAL, _T("Video Attempt to read byte value of location %x\n"), sekAddress);
	UINT16 res = MegadriveVideoReadWord(sekAddress & ~1);
	if ((sekAddress&1)==0) res >>= 8;
	return res & 0xff;
}

void __fastcall MegadriveVideoWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if (sekAddress > 0xC0001F)
		bprintf(PRINT_NORMAL, _T("Video Attempt to write word value %x to location %x\n"), wordValue, sekAddress);

	switch (sekAddress & 0x1c) {
	case 0x00:	// data
		if (RamVReg->pending)
			CommandChange();
    	RamVReg->pending = 0;
		if ((RamVReg->command & 0x80) && (RamVReg->reg[1]&0x10) && (RamVReg->reg[0x17]>>6)==2) {

			DmaFill(wordValue);

		} else {
			//UINT32 a=Pico.video.addr;
			switch (RamVReg->type) {
			case 1: 
				// If address is odd, bytes are swapped (which game needs this?)
				if (RamVReg->addr & 1) {
					bprintf(PRINT_NORMAL, _T("Video address is odd, bytes are swapped!!!\n"));
					wordValue = (wordValue<<8)|(wordValue>>8);
				}
				RamVid[(RamVReg->addr >> 1) & 0x7fff] = BURN_ENDIAN_SWAP_INT16(wordValue);
            	rendstatus |= 0x10; 
            	break;
			case 3: 
				//Pico.m.dirtyPal = 1;
				//dprintf("w[%i] @ %04x, inc=%i [%i|%i]", Pico.video.type, a, Pico.video.reg[0xf], Pico.m.scanline, SekCyclesDone());
				CalcCol((RamVReg->addr >> 1) & 0x003f, wordValue);
				break;
			case 5:
				RamSVid[(RamVReg->addr >> 1) & 0x003f] = BURN_ENDIAN_SWAP_INT16(wordValue); 
				break;
			}
			//dprintf("w[%i] @ %04x, inc=%i [%i|%i]", Pico.video.type, a, Pico.video.reg[0xf], Pico.m.scanline, SekCyclesDone());
			//AutoIncrement();
			RamVReg->addr += RamVReg->reg[0xf];
		}
    	return;
		
	case 0x04:	// command
		if(RamVReg->pending) {
			// Low word of command:
			RamVReg->command &= 0xffff0000;
			RamVReg->command |= wordValue;
			RamVReg->pending = 0;
			CommandChange();
		} else {
			if((wordValue & 0xc000) == 0x8000) {
				INT32 num = (wordValue >> 8) & 0x1f;
				RamVReg->reg[num] = wordValue & 0xff;
				
				// update IRQ level (Lemmings, Wiz 'n' Liz intro, ... )
				// may break if done improperly:
				// International Superstar Soccer Deluxe (crash), Street Racer (logos), Burning Force (gfx), Fatal Rewind (hang), Sesame Street Counting Cafe
				if(num < 2) {
					
					INT32 lines = (RamVReg->reg[1] & 0x20) | (RamVReg->reg[0] & 0x10);
					INT32 pints = (RamVReg->pending_ints & lines);
					if(pints & 0x20) SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
					else if(pints & 0x10) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
					else SekSetIRQLine(0, SEK_IRQSTATUS_NONE);

				}

				if (num == 5) rendstatus |= 1;
//				else if(num == 0xc) Pico.m.dirtyPal = 2; // renderers should update their palettes if sh/hi mode is changed
				RamVReg->type = 0; // register writes clear command (else no Sega logo in Golden Axe II)
			} else {
				// High word of command:
				RamVReg->command &= 0x0000ffff;
				RamVReg->command |= wordValue << 16;
				RamVReg->pending = 1;
			}
		}
    	return;
	
	case 0x10:
	case 0x14:
		// PSG Sound
		//bprintf(PRINT_NORMAL, _T("PSG Attempt to write word value %04x to location %08x\n"), wordValue, sekAddress);
		SN76496Write(0, wordValue & 0xFF);
		return;
	
	}
}

void __fastcall MegadriveVideoWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	//bprintf(PRINT_NORMAL, _T("Video Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	MegadriveVideoWriteWord(sekAddress, (byteValue << 8) | byteValue);
}

// -- Z80 Ram Read Write ------------------------------------------

UINT8 __fastcall MegadriveZ80RamReadByte(UINT32 sekAddress)
{
	if (!Z80HasBus && !MegadriveZ80Reset) {
		return RamZ80[sekAddress & 0x1fff];
	}
		
	return 0;
}

UINT16 __fastcall MegadriveZ80RamReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Z80Ram Attempt to read word value of location %x\n"), sekAddress);
	return 0;
}

void __fastcall MegadriveZ80RamWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (!Z80HasBus && !MegadriveZ80Reset) {
		RamZ80[sekAddress & 0x1fff] = byteValue;
		return;
	}
	bprintf(PRINT_NORMAL, _T("Z80Ram Attempt to write byte value %x to location %x and didn't have the Bus!\n"), byteValue, sekAddress);
}

void __fastcall MegadriveZ80RamWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if (!Z80HasBus && !MegadriveZ80Reset) {
		RamZ80[sekAddress & 0x1fff] = wordValue >> 8;
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("Z80Ram Attempt to write word value %x to location %x and didn't have the Bus!\n"), wordValue, sekAddress);
//	MegadriveZ80RamWriteByte(sekAddress, wordValue >> 0x08);
}

// -- I/O Read Write ------------------------------------------

static INT32 PadRead(INT32 i)
{
	INT32 pad=0,value=0,TH;
	pad = ~(JoyPad->pad[i]);					// Get inverse of pad MXYZ SACB RLDU
	TH = RamIO[i+1] & 0x40;

	/*if(PicoOpt & 0x20)*/ {					// 6 button gamepad enabled
		INT32 phase = JoyPad->padTHPhase[i];

		if(phase == 2 && !TH) {
			value = (pad&0xc0)>>2;				// ?0SA 0000
			goto end;
		} else if(phase == 3 && TH) {
			value=(pad&0x30)|((pad>>8)&0xf);	// ?1CB MXYZ
			goto end;
		} else if(phase == 3 && !TH) {
			value=((pad&0xc0)>>2)|0x0f;			// ?0SA 1111
		goto end;
		}
	}

	if(TH) value=(pad&0x3f);              // ?1CB RLDU
	else   value=((pad&0xc0)>>2)|(pad&3); // ?0SA 00DU

end:

	// orr the bits, which are set as output
	value |= RamIO[i+1] & RamIO[i+4];

	return value; // will mirror later
}

UINT8 __fastcall MegadriveIOReadByte(UINT32 sekAddress)
{
	if (sekAddress > 0xA1001F)
		bprintf(PRINT_NORMAL, _T("IO Attempt to read byte value of location %x\n"), sekAddress);

	INT32 offset = (sekAddress >> 1) & 0xf;
	switch (offset) {
	case 0:	// Get Hardware 
		return Hardware;
	case 1: // Pad 1
		return (RamIO[1] & 0x80) | PadRead(0);
	case 2: // Pad 2
		return (RamIO[2] & 0x80) | PadRead(1);
	default:
		//bprintf(PRINT_NORMAL, _T("IO Attempt to read byte value of location %x\n"), sekAddress);
		return RamIO[offset];
	}
	return 0;
}

UINT16 __fastcall MegadriveIOReadWord(UINT32 sekAddress)
{
	//if (sekAddress > 0xA1001F)
	//	bprintf(PRINT_NORMAL, _T("IO Attempt to read word value of location %x\n"), sekAddress);

	UINT8 res = MegadriveIOReadByte(sekAddress);
	return res | (res << 8);
}

void __fastcall MegadriveIOWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (sekAddress > 0xA1001F)
		bprintf(PRINT_NORMAL, _T("IO Attempt to byte byte value %x to location %x\n"), byteValue, sekAddress);

	INT32 offset = (sekAddress >> 1) & 0xf;
	// 6-Button Support
	switch( offset ) {
	case 1:
		JoyPad->padDelay[0] = 0;
		if(!(RamIO[1] & 0x40) && (byteValue&0x40)) 
			JoyPad->padTHPhase[0] ++;
		break;
	case 2:
		JoyPad->padDelay[1] = 0;
		if(!(RamIO[2] & 0x40) && (byteValue&0x40)) 
			JoyPad->padTHPhase[1] ++;
		break;
	}
	RamIO[offset] = byteValue;
}

void __fastcall MegadriveIOWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	//if (sekAddress > 0xA1001F)	
	//	bprintf(PRINT_NORMAL, _T("IO Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
		
	MegadriveIOWriteByte(sekAddress, wordValue & 0xff);
}

// -- YM2612/YM2612 FM Chip ----------------------------------------------------------

inline static INT32 MegadriveSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)SekTotalCycles() * nSoundRate / (OSC_NTSC / 7);
}

inline static double MegadriveGetTime()
{
	return (double)SekTotalCycles() / (OSC_NTSC / 7);
}

inline static INT32 MegadriveSynchroniseStreamPAL(INT32 nSoundRate)
{
	return (INT64)SekTotalCycles() * nSoundRate / (OSC_PAL / 7);
}

inline static double MegadriveGetTimePAL()
{
	return (double)SekTotalCycles() / (OSC_PAL / 7);
}

// ---------------------------------------------------------------

static INT32 MegadriveResetDo()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2612Reset();
	
	MegadriveZ80Reset = 1;
	Z80HasBus = 1;
	
#if 0
	FILE * f = fopen("Megadrive.bin", "wb+");
	fwrite(RomMain, 1, 0x200000, f);
	fclose(f);
#endif
	
	MegadriveCheckHardware();
	
	if (Hardware & 0x40) {
		BurnSetRefreshRate(50.0);
		Reinitialise();
		
		BurnYM2612Exit();
		BurnYM2612Init(1, OSC_PAL / 7, NULL, MegadriveSynchroniseStreamPAL, MegadriveGetTimePAL, 0);
		BurnTimerAttachSek(OSC_PAL / 7);
		
		BurnYM2612Reset();
		
		SN76496Exit();
		SN76496Init(0, OSC_PAL / 15, 1);
		SN76496SetVolShift(0, 1);
	} else {
		BurnSetRefreshRate(60.0);
		Reinitialise();
		
		BurnYM2612Exit();
		BurnYM2612Init(1, OSC_NTSC / 7, NULL, MegadriveSynchroniseStream, MegadriveGetTime, 0);
		BurnTimerAttachSek(OSC_NTSC / 7);
		
		BurnYM2612Reset();
		
		SN76496Exit();
		SN76496Init(0, OSC_NTSC / 15, 1);
		SN76496SetVolShift(0, 1);
	}

	// other reset
//	memset(RamMisc, 0, sizeof(struct PicoMisc));
	memset(JoyPad, 0, sizeof(struct MegadriveJoyPad));
	
	// default VDP register values (based on Fusion)
	memset(RamVReg, 0, sizeof(struct PicoVideo));
	RamVReg->reg[0x00] = 0x04;
	RamVReg->reg[0x01] = 0x04;
	RamVReg->reg[0x0c] = 0x81;
	RamVReg->reg[0x0f] = 0x02;
	
	RamVReg->status = 0x3408 | ((MegadriveDIP[0] & 0x40) >> 6);
	
	return 0;
}

INT32 __fastcall MegadriveIrqCallback(INT32 irq)
{
	switch ( irq ) {
	case 4:	RamVReg->pending_ints  =  0x00; break;
	case 6:	RamVReg->pending_ints &= ~0x20; break;
	}
	return -1;
}

// ----------------------------------------------------------------
// Z80 Read/Write
// ----------------------------------------------------------------

UINT8 __fastcall MegadriveZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read %02x\n"), a);
		}
	}	

	return 0;
}

void __fastcall MegadriveZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write %02x, %02%x\n"), a, d);
		}
	}
}

UINT8 __fastcall MegadriveZ80ProgRead(UINT16 a)
{
	if (a >= 0x6100 && a <= 0x7eff) {
		return 0xff;
	}
	
	if (a >= 0x8000) {
		UINT32 addr68k = RamMisc->Bank68k;
		addr68k += a & 0x7fff;
		if (addr68k <= 0x3fffff) return RomMain[addr68k ^ 1];
		
		bprintf(PRINT_NORMAL, _T("%Z80 trying to read 68k address %06X\n"), addr68k);
		return 0;
	}
	
	switch (a) {
		case 0x4000:
		case 0x4001:
		case 0x4002: {
			return BurnYM2612Read(0, 0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read %04x\n"), a);
		}
	}
	
	return 0;
}

UINT32 Z80BankPartial = 0;
UINT32 Z80BankPos = 0;

void __fastcall MegadriveZ80ProgWrite(UINT16 a, UINT8 d)
{
	if (a == 0x6000 || a == 0x6001) {
		Z80BankPartial |= (d & 0x01) << 23;
		Z80BankPos++;

		if (Z80BankPos < 9) {
			Z80BankPartial >>= 1;
		} else {
			Z80BankPos = 0;
			RamMisc->Bank68k = Z80BankPartial;
			Z80BankPartial = 0;
		}
		return;
	}
	
	if (a >= 0x8000) {
		UINT32 addr68k = RamMisc->Bank68k;
		addr68k += a & 0x7fff;
		
		if (addr68k <= 0x3fffff) return;
		
		if (addr68k == 0xc00011) {
			SN76496Write(0, d);
			return;
		}
		
		if ((addr68k >= 0xe00000) && (addr68k <= 0xffffff)) {
			addr68k &=0xffff;
			UINT16 *Ram = (UINT16*)Ram68K;
			if (addr68k & 0x01) {
				Ram[addr68k >> 1] = (Ram[addr68k >> 1] & 0xff00) | d;
			} else {
				Ram[addr68k >> 1] = (Ram[addr68k >> 1] & 0x00ff) | (d << 8);
			}
			return;
		}
		
		bprintf(PRINT_NORMAL, _T("Z80-Bank68K Attempt to write byte value %02x to location %06x\n"), d, addr68k);
		return;
	}
	
	switch (a) {
		case 0x4000: {
			SekOpen(0);
			BurnYM2612Write(0, 0, d);
			SekClose();
			return;
		}
		
		case 0x4001: {
			SekOpen(0);
			BurnYM2612Write(0, 1, d);
			SekClose();
			return;
		}
		
		case 0x4002: {
			SekOpen(0);
			BurnYM2612Write(0, 2, d);
			SekClose();
			return;
		}
		
		case 0x4003: {
			SekOpen(0);
			BurnYM2612Write(0, 3, d);
			SekClose();
			return;
		}
		
		case 0x7f11:
		case 0x7f13:
		case 0x7f15:
		case 0x7f17: {
			SN76496Write(0, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write %04x, %02x\n"), a, d);
		}
	}
}

static INT32 MegadriveLoadRoms(bool bLoad)
{
	struct BurnRomInfo ri;
	ri.nType = 0;
	ri.nLen = 0;
	INT32 nOffset = -1;
	UINT32 i;
	INT32 nRet = 0;
	
	if (!bLoad) {
		do {
			ri.nLen = 0;
			ri.nType = 0;
			BurnDrvGetRomInfo(&ri, ++nOffset);
			if(ri.nLen) RomNum++;
			RomSize += ri.nLen;
		} while (ri.nLen);
		
		bprintf(PRINT_NORMAL, _T("68K Rom, Num %i, Size %x\n"), RomNum, RomSize);
	}
	
	if (bLoad) {
		INT32 Offset = 0;
		
		for (i = 0; i < RomNum; i++) {
			BurnDrvGetRomInfo(&ri, i);
			
			switch (ri.nType & 0x0f) {
				case SEGA_MD_ROM_OFFS_000000: Offset = 0x000000; break;
				case SEGA_MD_ROM_OFFS_000001: Offset = 0x000001; break;
				case SEGA_MD_ROM_OFFS_020000: Offset = 0x020000; break;
				case SEGA_MD_ROM_OFFS_080000: Offset = 0x080000; break;
				case SEGA_MD_ROM_OFFS_100000: Offset = 0x100000; break;
				case SEGA_MD_ROM_OFFS_100001: Offset = 0x100001; break;
				case SEGA_MD_ROM_OFFS_200000: Offset = 0x200000; break;				
			}
			
			switch (ri.nType & 0xf0) {
				case SEGA_MD_ROM_LOAD_NORMAL: {
					nRet = BurnLoadRom(RomMain + Offset, i, 1); if (nRet) return 1;
					break;
				}
				
				case SEGA_MD_ROM_LOAD16_WORD_SWAP: {
					nRet = BurnLoadRom(RomMain + Offset, i, 1); if (nRet) return 1;
					BurnByteswap(RomMain + Offset, ri.nLen);
					break;
				}
				
				case SEGA_MD_ROM_LOAD16_BYTE: {
					nRet = BurnLoadRom(RomMain + Offset, i, 2); if (nRet) return 1;
					break;
				}
				
				case SEGA_MD_ROM_LOAD16_WORD_SWAP_CONTINUE_040000_100000: {
					nRet = BurnLoadRom(RomMain + Offset, i, 1); if (nRet) return 1;
					memcpy(RomMain + 0x100000, RomMain + 0x040000, 0x40000);
					BurnByteswap(RomMain + Offset, 0x140000);
					break;
				}
			}
		}
	}
	
	return 0;
}

// Custom Cartridge Mapping

UINT8 __fastcall JCartCtrlReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("JCartCtrlRead Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall JCartCtrlReadWord(UINT32 /*sekAddress*/)
{
	UINT16 retData = 0;
	
	UINT8 JPad3 = ~(JoyPad->pad[2] & 0xff);
	UINT8 JPad4 = ~(JoyPad->pad[3] & 0xff);
	
	if (RamMisc->JCartIOData[0] & 0x40) {
		retData = (RamMisc->JCartIOData[0] & 0x40) | JPad3 | (JPad4 << 8);
	} else {
		retData = ((JPad3 & 0xc0) >> 2) | (JPad3 & 0x03);
		retData += (((JPad4 & 0xc0) >> 2) | (JPad4 & 0x03)) << 8;
	}
	
	return retData;
}

void __fastcall JCartCtrlWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	bprintf(PRINT_NORMAL, _T("JCartCtrlWrite byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall JCartCtrlWriteWord(UINT32 /*sekAddress*/, UINT16 wordValue)
{
	RamMisc->JCartIOData[0] = (wordValue & 1) << 6;
	RamMisc->JCartIOData[1] = (wordValue & 1) << 6;
}

void __fastcall Ssf2BankWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0xa130f1: {
			if (byteValue == 2) memcpy(RomMain + 0x000000, RomMain + 0x400000 + (((byteValue & 0x0f) - 2) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130f3: {
			memcpy(RomMain + 0x080000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130f5: {
			memcpy(RomMain + 0x100000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130f7: {
			memcpy(RomMain + 0x180000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130f9: {
			memcpy(RomMain + 0x200000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130fb: {
			memcpy(RomMain + 0x280000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130fd: {
			memcpy(RomMain + 0x300000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
		
		case 0xa130ff: {
			memcpy(RomMain + 0x380000, RomMain + 0x400000 + ((byteValue & 0xf) * 0x080000), 0x080000);
			return;
		}
	}	
}

UINT8 __fastcall LK3AltProtReadByte(UINT32 sekAddress)
{
	INT32 Offset = (sekAddress - 0x600000) >> 1;
	Offset &= 0x07;
	
	UINT8 retData = 0;
	
	switch (Offset) {
		case 0x02: {
			switch (RamMisc->L3AltPCmd) {
				case 1:
					retData = RamMisc->L3AltPDat >> 1;
					break;

				case 2:
					retData = RamMisc->L3AltPDat >> 4;
					retData |= (RamMisc->L3AltPDat & 0x0f) << 4;
					break;

				default:
					retData =  (BIT(RamMisc->L3AltPDat, 7) << 0);
					retData |= (BIT(RamMisc->L3AltPDat, 6) << 1);
					retData |= (BIT(RamMisc->L3AltPDat, 5) << 2);
					retData |= (BIT(RamMisc->L3AltPDat, 4) << 3);
					retData |= (BIT(RamMisc->L3AltPDat, 3) << 4);
					retData |= (BIT(RamMisc->L3AltPDat, 2) << 5);
					retData |= (BIT(RamMisc->L3AltPDat, 1) << 6);
					retData |= (BIT(RamMisc->L3AltPDat, 0) << 7);
					break;
			}
			break;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("LK3AltProt Read Byte %x\n"), sekAddress);
	
	return retData;
}

UINT16 __fastcall LK3AltProtReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("LK3AltProt Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall LK3AltProtWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	INT32 Offset = (sekAddress - 0x600000) >> 1;
	Offset &= 0x07;
	
	switch (Offset) {
		case 0x00:
			RamMisc->L3AltPDat = byteValue;
			return;
		
		case 0x01:
			RamMisc->L3AltPCmd = byteValue;
			return;
	}
	
//	bprintf(PRINT_NORMAL, _T("LK3AltProt write byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall LK3AltProtWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("LK3AltProt write word value %04x to location %08x\n"), wordValue, sekAddress);
}

void __fastcall LK3AltBankWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	INT32 Offset = (sekAddress - 0x700000) >> 1;
	Offset &= 0x07;
	
	if (Offset == 0) {
		memcpy(RomMain, OriginalRom + ((byteValue & 0xff) * 0x8000), 0x8000);
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("LK3AltBank write byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall LK3AltBankWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("LK3AltBank write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall RedclifProtReadByte(UINT32 /*sekAddress*/)
{
	return (UINT8)-0x56;
}

UINT16 __fastcall RedclifProtReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("RedclifeProt Read Word %x\n"), sekAddress);
	
	return 0;
}

UINT8 __fastcall RedclifProt2ReadByte(UINT32 /*sekAddress*/)
{
	return 0x55;
}

UINT16 __fastcall RedclifProt2ReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("RedclifeProt2 Read Word %x\n"), sekAddress);
	
	return 0;
}

UINT8 __fastcall RadicaBankSelectReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("RadicaBankSelect Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall RadicaBankSelectReadWord(UINT32 sekAddress)
{
	INT32 Bank = ((sekAddress - 0xa13000) >> 1) & 0x3f;
	memcpy(RomMain, RomMain + 0x400000 + (Bank * 0x10000), 0x400000);
	
	return 0;
}

UINT8 __fastcall Kof99A13000ReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Kof99A13000 Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall Kof99A13000ReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xa13000: return 0x00;
		case 0xa13002: return 0x01;
		case 0xa1303e: return 0x1f;
		
	}
	
	bprintf(PRINT_NORMAL, _T("Kof99A13000 Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall SoulbladReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400002: return 0x98;
		case 0x400004: return 0xc0;
		case 0x400006: return 0xf0;
		
	}
	
	bprintf(PRINT_NORMAL, _T("Soulblad Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall SoulbladReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Soulblad Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall MjloverProt1ReadByte(UINT32 /*sekAddress*/)
{
	return 0x90;
}

UINT16 __fastcall MjloverProt1ReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("MjloverProt1 Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall MjloverProt2ReadByte(UINT32 /*sekAddress*/)
{
	return 0xd3;
}

UINT16 __fastcall MjloverProt2ReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("MjloverProt2 Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall SquirrelKingExtraReadByte(UINT32 /*sekAddress*/)
{
	return RamMisc->SquirrelkingExtra;
}

UINT16 __fastcall SquirrelKingExtraReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("SquirrelKingExtra Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall SquirrelKingExtraWriteByte(UINT32 /*sekAddress*/, UINT8 byteValue)
{
	RamMisc->SquirrelkingExtra = byteValue;
}

void __fastcall SquirrelKingExtraWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("SquirrelKingExtra write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall SmouseProtReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400000: return 0x55;
		case 0x400002: return 0x0f;
		case 0x400004: return 0xaa;
		case 0x400005: return 0xf0;
	}
	
	return 0;
}

UINT16 __fastcall SmouseProtReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("SmouseProt Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall SmbProtReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Smbprot Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall SmbProtReadWord(UINT32 /*sekAddress*/)
{
	return 0x0c;
}

UINT8 __fastcall Smb2ProtReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Smb2Prot Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall Smb2ProtReadWord(UINT32 /*sekAddress*/)
{
	return 0x0a;
}

void __fastcall KaijuBankWriteByte(UINT32 /*sekAddress*/, UINT8 byteValue)
{
	memcpy(RomMain + 0x000000, RomMain + 0x400000 + (byteValue & 0x7f) * 0x8000, 0x8000);
}

void __fastcall KaijuBankWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("KaijuBank write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall Chinfi3ProtReadByte(UINT32 /*sekAddress*/)
{
	UINT8 retDat = 0;
	
	if (SekGetPC(0) == 0x01782) // makes 'VS' screen appear
	{
		retDat = SekDbgGetRegister(SEK_REG_D3) & 0xff;
//		retDat <<= 8;
		return retDat;
	}
	else if (SekGetPC(0) == 0x1c24) // background gfx etc.
	{
		retDat = SekDbgGetRegister(SEK_REG_D3) & 0xff;
//		retDat <<= 8;
		return retDat;
	}
	else if (SekGetPC(0) == 0x10c4a) // unknown
	{
		return rand() & 0xff;//space->machine().rand();
	}
	else if (SekGetPC(0) == 0x10c50) // unknown
	{
		return rand() & 0xff;//space->machine().rand();
	}
	else if (SekGetPC(0) == 0x10c52) // relates to the game speed..
	{
		retDat = SekDbgGetRegister(SEK_REG_D4) & 0xff;
//		retDat <<= 8;
		return retDat;
	}
	else if (SekGetPC(0) == 0x061ae)
	{
		retDat = SekDbgGetRegister(SEK_REG_D3) & 0xff;
//		retDat <<= 8;
		return retDat;
	}
	else if (SekGetPC(0) == 0x061b0)
	{
		retDat = SekDbgGetRegister(SEK_REG_D3) & 0xff;
//		retDat <<= 8;
		return retDat;
	}
	
	return 0;
}

UINT16 __fastcall Chinfi3ProtReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Chinfi3Prot Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall Chinfi3BankWriteByte(UINT32 /*sekAddress*/, UINT8 byteValue)
{
	if (byteValue == 0xf1) // *hit player
	{
		INT32 x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(RomMain + x, RomMain + 0x410000, 0x10000);
		}
	}
	else if (byteValue == 0xd7) // title screen..
	{
		INT32 x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(RomMain + x, RomMain + 0x470000, 0x10000);
		}
	}
	else if (byteValue == 0xd3) // character hits floor
	{
		INT32 x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(RomMain + x, RomMain + 0x430000, 0x10000);
		}
	}
	else if (byteValue == 0x00)
	{
		INT32 x;
		for (x = 0; x < 0x100000; x += 0x10000)
		{
			memcpy(RomMain + x, RomMain + 0x400000 + x, 0x10000);
		}
	}
}

void __fastcall Chinfi3BankWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Chinfi3Bank write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall Lionk2ProtReadByte(UINT32 sekAddress)
{
	switch(sekAddress) {
		case 0x400002: {
			return RamMisc->Lionk2ProtData;
		}
		
		case 0x400006: {
			return RamMisc->Lionk2ProtData2;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Lion2Prot Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall Lionk2ProtReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Lion2Prot Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall Lionk2ProtWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x400000: {
			RamMisc->Lionk2ProtData = byteValue;
			return;
		}
		
		case 0x400004: {
			RamMisc->Lionk2ProtData2 = byteValue;
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Lion2Prot write byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall Lionk2ProtWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Lion2Prot write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall BuglExtraReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("BuglExtra Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall BuglExtraReadWord(UINT32 /*sekAddress*/)
{
	return 0x28;
}

UINT8 __fastcall Elfwor400000ReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400000: return 0x55;
		case 0x400002: return 0x0f;
		case 0x400004: return 0xc9;
		case 0x400006: return 0x18;
	}
	
	bprintf(PRINT_NORMAL, _T("Elfwor400000 Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall Elfwor400000ReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Elfwor400000 Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall RockmanX3ExtraReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("RockmanX3Extra Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall RockmanX3ExtraReadWord(UINT32 /*sekAddress*/)
{
	return 0x0c;
}

UINT8 __fastcall SbubExtraReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400000: return 0x55;
		case 0x400002: return 0x0f;
	}
	
	bprintf(PRINT_NORMAL, _T("SbubExtra Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall SbubExtraReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("SbubExtra Read Word %x\n"), sekAddress);

	return 0;
}

UINT8 __fastcall Kof98ReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Kof98 Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall Kof98ReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x480000: return 0xaa00;
		case 0x4800e0: return 0xaa00;
		case 0x4824a0: return 0xaa00;
		case 0x488880: return 0xaa00;
		case 0x4a8820: return 0x0a00;
		case 0x4f8820: return 0x0000;
	}
	
	bprintf(PRINT_NORMAL, _T("Kof98 Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall RealtecWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x400000: {
			INT32 BankData = (byteValue >> 1) & 0x7;

			RamMisc->RealtecBankAddr = (RamMisc->RealtecBankAddr & 0x7) | BankData << 3;

			memcpy(RomMain, RomMain + (RamMisc->RealtecBankAddr * 0x20000) + 0x400000, RamMisc->RealtecBankSize * 0x20000);
			memcpy(RomMain + RamMisc->RealtecBankSize * 0x20000, RomMain + (RamMisc->RealtecBankAddr * 0x20000) + 0x400000, RamMisc->RealtecBankSize * 0x20000);
			return;
		}
		
		case 0x402000:{
			RamMisc->RealtecBankAddr = 0;
			RamMisc->RealtecBankSize = byteValue & 0x1f;
			return;
		}
		
		case 0x404000: {
			INT32 BankData = byteValue & 0x3;

			RamMisc->RealtecBankAddr = (RamMisc->RealtecBankAddr & 0xf8) | BankData;

			memcpy(RomMain, RomMain + (RamMisc->RealtecBankAddr * 0x20000)+ 0x400000, RamMisc->RealtecBankSize * 0x20000);
			memcpy(RomMain + RamMisc->RealtecBankSize * 0x20000, RomMain + (RamMisc->RealtecBankAddr * 0x20000) + 0x400000, RamMisc->RealtecBankSize * 0x20000);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Realtec write byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall RealtecWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Realtec write word value %04x to location %08x\n"), wordValue, sekAddress);
}

void __fastcall Sup19in1BankWriteByte(UINT32 sekAddress, UINT8 /*byteValue*/)
{
	INT32 Offset = (sekAddress - 0xa13000) >> 1;
	
	memcpy(RomMain + 0x000000, RomMain + 0x400000 + ((Offset << 1) * 0x10000), 0x80000);
}

void __fastcall Sup19in1BankWriteWord(UINT32 sekAddress, UINT16 /*wordValue*/)
{
	INT32 Offset = (sekAddress - 0xa13000) >> 1;
	
	memcpy(RomMain + 0x000000, RomMain + 0x400000 + ((Offset << 1) * 0x10000), 0x80000);
}

void __fastcall Mc12in1BankWriteByte(UINT32 sekAddress, UINT8 /*byteValue*/)
{
	INT32 Offset = (sekAddress - 0xa13000) >> 1;
	memcpy(RomMain + 0x000000, OriginalRom + ((Offset & 0x3f) << 17), 0x100000);
}

void __fastcall Mc12in1BankWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Mc12in1Bank write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall TopfigReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x645b45: return 0x9f;
		
		case 0x6bd295: {
			static INT32 x = -1;

			if (SekGetPC(0) == 0x1771a2) {
				return 0x50;
			} else {
				x++;
				return (UINT8)x;
			}
		}
		
		case 0x6f5345: {
			static INT32 x = -1;

			if (SekGetPC(0) == 0x4C94E) {
				return SekDbgGetRegister(SEK_REG_D0) & 0xff;
			} else {
				x++;
				return (UINT8)x;
			}
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Topfig Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall TopfigReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Topfig Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall TopfigWriteByte(UINT32 /*sekAddress*/, UINT8 byteValue)
{
	if (byteValue == 0x002a)
	{
		memcpy(RomMain + 0x060000, RomMain + 0x570000, 0x8000); // == 0x2e*0x8000?!

	}
	else if (byteValue==0x0035) // characters ingame
	{
		memcpy(RomMain + 0x020000, RomMain + 0x5a8000, 0x8000); // == 0x35*0x8000
	}
	else if (byteValue==0x000f) // special moves
	{
		memcpy(RomMain + 0x058000, RomMain + 0x478000, 0x8000); // == 0xf*0x8000
	}
	else if (byteValue==0x0000)
	{
		memcpy(RomMain + 0x060000, RomMain + 0x460000, 0x8000);
		memcpy(RomMain + 0x020000, RomMain + 0x420000, 0x8000);
		memcpy(RomMain + 0x058000, RomMain + 0x458000, 0x8000);
	}
}

void __fastcall TopfigWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Topfig write word value %04x to location %08x\n"), wordValue, sekAddress);
}

static void SetupCustomCartridgeMappers()
{
	if (((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_CM_JCART) || ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_CM_JCART_SEPROM)) {
		SekOpen(0);
		SekMapHandler(7, 0x38fffe, 0x38ffff, SM_READ | SM_WRITE);
		SekSetReadByteHandler(7, JCartCtrlReadByte);
		SekSetReadWordHandler(7, JCartCtrlReadWord);
		SekSetWriteByteHandler(7, JCartCtrlWriteByte);
		SekSetWriteWordHandler(7, JCartCtrlWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SSF2) {
		OriginalRom = (UINT8*)BurnMalloc(0x500000);
		memcpy(OriginalRom, RomMain, 0x500000);
		
		memcpy(RomMain + 0x800000, OriginalRom + 0x400000, 0x100000);
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x400000);
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x400000);
		
		SekOpen(0);
		SekMapHandler(7, 0xa130f0, 0xa130ff, SM_WRITE);
		SekSetWriteByteHandler(7, Ssf2BankWriteByte);
		SekClose();
	}
	
	if (((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_LIONK3) || ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SKINGKONG)) {
		RamMisc->L3AltPDat = 0;
		RamMisc->L3AltPCmd = 0;
		
		OriginalRom = (UINT8*)BurnMalloc(0x200000);
		memcpy(OriginalRom, RomMain, 0x200000);
		
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x200000);
		memcpy(RomMain + 0x200000, OriginalRom + 0x000000, 0x200000);
		
		SekOpen(0);
		SekMapHandler(7, 0x600000, 0x6fffff, SM_READ | SM_WRITE);
		SekSetReadByteHandler(7, LK3AltProtReadByte);
		SekSetReadWordHandler(7, LK3AltProtReadWord);
		SekSetWriteByteHandler(7, LK3AltProtWriteByte);
		SekSetWriteWordHandler(7, LK3AltProtWriteWord);
		SekMapHandler(8, 0x700000, 0x7fffff, SM_WRITE);
		SekSetWriteByteHandler(8, LK3AltBankWriteByte);
		SekSetWriteWordHandler(8, LK3AltBankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SDK99) {
		RamMisc->L3AltPDat = 0;
		RamMisc->L3AltPCmd = 0;
		
		OriginalRom = (UINT8*)BurnMalloc(0x300000);
		memcpy(OriginalRom, RomMain, 0x300000);
		
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x300000);
		memcpy(RomMain + 0x300000, OriginalRom + 0x000000, 0x100000);
		
		SekOpen(0);
		SekMapHandler(7, 0x600000, 0x6fffff, SM_READ | SM_WRITE);
		SekSetReadByteHandler(7, LK3AltProtReadByte);
		SekSetReadWordHandler(7, LK3AltProtReadWord);
		SekSetWriteByteHandler(7, LK3AltProtWriteByte);
		SekSetWriteWordHandler(7, LK3AltProtWriteWord);
		SekMapHandler(8, 0x700000, 0x7fffff, SM_WRITE);
		SekSetWriteByteHandler(8, LK3AltBankWriteByte);
		SekSetWriteWordHandler(8, LK3AltBankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_REDCL_EN) {
		OriginalRom = (UINT8*)BurnMalloc(0x200005);
		memcpy(OriginalRom, RomMain, 0x200005);
		for (UINT32 i = 0; i < RomSize; i++) {
			OriginalRom[i] ^= 0x40;
		}
		
		memcpy(RomMain + 0x000000, OriginalRom + 0x000004, 0x200000);
	
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400001, SM_READ);
		SekSetReadByteHandler(7, RedclifProt2ReadByte);
		SekSetReadWordHandler(7, RedclifProt2ReadWord);
		SekMapHandler(8, 0x400004, 0x400005, SM_READ);
		SekSetReadByteHandler(8, RedclifProtReadByte);
		SekSetReadWordHandler(8, RedclifProtReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_RADICA) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x400000);
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x400000);
		memcpy(RomMain + 0x800000, OriginalRom + 0x000000, 0x400000);
	
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa1307f, SM_READ);
		SekSetReadByteHandler(7, RadicaBankSelectReadByte);
		SekSetReadWordHandler(7, RadicaBankSelectReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_KOF99) {
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa1303f, SM_READ);
		SekSetReadByteHandler(7, Kof99A13000ReadByte);
		SekSetReadWordHandler(7, Kof99A13000ReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SOULBLAD) {
		SekOpen(0);
		SekMapHandler(7, 0x400002, 0x400007, SM_READ);
		SekSetReadByteHandler(7, SoulbladReadByte);
		SekSetReadWordHandler(7, SoulbladReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_MJLOVER) {
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400001, SM_READ);
		SekSetReadByteHandler(7, MjloverProt1ReadByte);
		SekSetReadWordHandler(7, MjloverProt1ReadWord);
		SekMapHandler(8, 0x401000, 0x401001, SM_READ);
		SekSetReadByteHandler(8, MjloverProt2ReadByte);
		SekSetReadWordHandler(8, MjloverProt2ReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SQUIRRELK) {
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400007, SM_READ | SM_WRITE);
		SekSetReadByteHandler(7, SquirrelKingExtraReadByte);
		SekSetReadWordHandler(7, SquirrelKingExtraReadWord);
		SekSetWriteByteHandler(7, SquirrelKingExtraWriteByte);
		SekSetWriteWordHandler(7, SquirrelKingExtraWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SMOUSE) {
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400007, SM_READ);
		SekSetReadByteHandler(7, SmouseProtReadByte);
		SekSetReadWordHandler(7, SmouseProtReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SMB) {
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa13001, SM_READ);
		SekSetReadByteHandler(7, SmbProtReadByte);
		SekSetReadWordHandler(7, SmbProtReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SMB2) {
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa13001, SM_READ);
		SekSetReadByteHandler(7, Smb2ProtReadByte);
		SekSetReadWordHandler(7, Smb2ProtReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_KAIJU) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x400000, OriginalRom, 0x200000);
		memcpy(RomMain + 0x600000, OriginalRom, 0x200000);
		memcpy(RomMain + 0x000000, OriginalRom, 0x200000);
	
		SekOpen(0);
		SekMapHandler(7, 0x700000, 0x7fffff, SM_WRITE);
		SekSetWriteByteHandler(7, KaijuBankWriteByte);
		SekSetWriteWordHandler(7, KaijuBankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_CHINFIGHT3) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x200000);
		memcpy(RomMain + 0x600000, OriginalRom + 0x000000, 0x200000);
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x200000);
		
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x4fffff, SM_READ);
		SekSetReadByteHandler(7, Chinfi3ProtReadByte);
		SekSetReadWordHandler(7, Chinfi3ProtReadWord);
		SekMapHandler(8, 0x600000, 0x6fffff, SM_WRITE);
		SekSetWriteByteHandler(8, Chinfi3BankWriteByte);
		SekSetWriteWordHandler(8, Chinfi3BankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_LIONK2) {
		RamMisc->Lionk2ProtData = 0;
		RamMisc->Lionk2ProtData2 = 0;
		
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400007, SM_READ | SM_WRITE);
		SekSetReadByteHandler(7, Lionk2ProtReadByte);
		SekSetReadWordHandler(7, Lionk2ProtReadWord);
		SekSetWriteByteHandler(7, Lionk2ProtWriteByte);
		SekSetWriteWordHandler(7, Lionk2ProtWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_BUGSLIFE) {
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa13001, SM_READ);
		SekSetReadByteHandler(7, BuglExtraReadByte);
		SekSetReadWordHandler(7, BuglExtraReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_ELFWOR) {
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400007, SM_READ);
		SekSetReadByteHandler(7, Elfwor400000ReadByte);
		SekSetReadWordHandler(7, Elfwor400000ReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_ROCKMANX3) {
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa13001, SM_READ);
		SekSetReadByteHandler(7, RockmanX3ExtraReadByte);
		SekSetReadWordHandler(7, RockmanX3ExtraReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SBUBBOB) {
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x400003, SM_READ);
		SekSetReadByteHandler(7, SbubExtraReadByte);
		SekSetReadWordHandler(7, SbubExtraReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_KOF98) {
		SekOpen(0);
		SekMapHandler(7, 0x480000, 0x4fffff, SM_READ);
		SekSetReadByteHandler(7, Kof98ReadByte);
		SekSetReadWordHandler(7, Kof98ReadWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_REALTEC) {
		RamMisc->RealtecBankAddr = 0;
		RamMisc->RealtecBankSize = 0;
		
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x080000);
		
		for (INT32 i = 0; i < 0x400000; i += 0x2000) {
			memcpy(RomMain + i, OriginalRom + 0x7e000, 0x2000);
		}
		
		SekOpen(0);
		SekMapHandler(7, 0x400000, 0x40400f, SM_WRITE);
		SekSetWriteByteHandler(7, RealtecWriteByte);
		SekSetWriteWordHandler(7, RealtecWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_MC_SUP19IN1) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x400000);
		
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa13039, SM_WRITE);
		SekSetWriteByteHandler(7, Sup19in1BankWriteByte);
		SekSetWriteWordHandler(7, Sup19in1BankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_MC_SUP15IN1) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x200000);
		
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa13039, SM_WRITE);
		SekSetWriteByteHandler(7, Sup19in1BankWriteByte);
		SekSetWriteWordHandler(7, Sup19in1BankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_12IN1) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x200000);
		
		SekOpen(0);
		SekMapHandler(7, 0xa13000, 0xa1303f, SM_WRITE);
		SekSetWriteByteHandler(7, Mc12in1BankWriteByte);
		SekSetWriteWordHandler(7, Mc12in1BankWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_TOPFIGHTER) {
		OriginalRom = (UINT8*)BurnMalloc(RomSize);
		memcpy(OriginalRom, RomMain, RomSize);
		
		memcpy(RomMain + 0x000000, OriginalRom + 0x000000, 0x200000);
		memcpy(RomMain + 0x200000, OriginalRom + 0x000000, 0x200000);
		memcpy(RomMain + 0x400000, OriginalRom + 0x000000, 0x200000);
		memcpy(RomMain + 0x600000, OriginalRom + 0x000000, 0x200000);
		
		SekOpen(0);
		SekMapHandler(7, 0x600000, 0x6fffff, SM_READ);
		SekSetReadByteHandler(7, TopfigReadByte);
		SekSetReadWordHandler(7, TopfigReadWord);		
		SekMapHandler(8, 0x700000, 0x7fffff, SM_WRITE);
		SekSetWriteByteHandler(8, TopfigWriteByte);
		SekSetWriteWordHandler(8, TopfigWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_POKEMON) {
		UINT16 *ROM16 = (UINT16 *)RomMain;

		ROM16[0x0dd19e/2] = 0x47F8;
		ROM16[0x0dd1a0/2] = 0xFFF0;
		ROM16[0x0dd1a2/2] = 0x4E63;
		ROM16[0x0dd46e/2] = 0x4EF8;
		ROM16[0x0dd470/2] = 0x0300;
		ROM16[0x0dd49c/2] = 0x6002;
	}
	
/*	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_POKEMON2) {
		UINT16 *ROM16 = (UINT16 *)RomMain;

		ROM16[0x06036/2] = 0xE000;
		ROM16[0x02540/2] = 0x6026;
		ROM16[0x01ED0/2] = 0x6026;
		ROM16[0x02476/2] = 0x6022;

		ROM16[0x7E300/2] = 0x60FE;
	}*/
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_MULAN) {
		UINT16 *ROM16 = (UINT16 *)RomMain;

		ROM16[0x06036/2] = 0xE000;
	}	
}

// SRAM and EEPROM Handling

UINT8 __fastcall MegadriveSRAMReadByte(UINT32 sekAddress)
{
	if (RamMisc->SRamActive) {
		return SRam[(sekAddress - RamMisc->SRamStart) ^ 1];
	} else {
		return RomMain[sekAddress ^ 1];
	}
}

UINT16 __fastcall MegadriveSRAMReadWord(UINT32 sekAddress)
{
	if (RamMisc->SRamActive) {
		UINT16 *Ram = (UINT16*)SRam;
		return Ram[(sekAddress - RamMisc->SRamStart) >> 1];
	} else {
		UINT16 *Rom = (UINT16*)RomMain;
		return Rom[sekAddress >> 1];
	}
}

void __fastcall MegadriveSRAMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (RamMisc->SRamActive) {
		if (!RamMisc->SRamReadOnly) {
			SRam[(sekAddress - RamMisc->SRamStart) ^ 1] = byteValue;
			return;
		}
	}
}

void __fastcall MegadriveSRAMWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if (RamMisc->SRamActive) {
		if (!RamMisc->SRamReadOnly) {
			UINT16 *Ram = (UINT16*)SRam;
			Ram[(sekAddress - RamMisc->SRamStart) >> 1] = wordValue;
			return;
		}
	}
}

static void InstallSRAMHandlers(bool MaskAddr)
{
	UINT32 Mask = MaskAddr ? 0x3fffff : 0xffffff;
	
	memset(SRam, 0xff, MAX_SRAM_SIZE);
	memcpy((UINT8*)MegadriveBackupRam, SRam, RamMisc->SRamEnd - RamMisc->SRamStart + 1);
	
	SekOpen(0);
	SekMapHandler(6, RamMisc->SRamStart & Mask, RamMisc->SRamEnd & Mask, SM_READ | SM_WRITE);
	SekSetReadByteHandler(6, MegadriveSRAMReadByte);
	SekSetReadWordHandler(6, MegadriveSRAMReadWord);
	SekSetWriteByteHandler(6, MegadriveSRAMWriteByte);
	SekSetWriteWordHandler(6, MegadriveSRAMWriteWord);
	SekClose();

	RamMisc->SRamHandlersInstalled = 1;	
}

UINT8 __fastcall Megadrive6658ARegReadByte(UINT32 sekAddress)
{
	if (sekAddress & 1) return RamMisc->SRamActive;
	
	bprintf(PRINT_NORMAL, _T("Megadrive6658AReg Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall Megadrive6658ARegReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Megadrive6658AReg Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall Megadrive6658ARegWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (sekAddress & 1) {
		if (byteValue == 1) {
			RamMisc->SRamActive = 1;
			return;
		}
	
		if (byteValue == 0) {
			RamMisc->SRamActive = 0;
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("6658A Reg write byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall Megadrive6658ARegWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("6658A Reg write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall WboyVEEPROMReadByte(UINT32 sekAddress)
{
	if (sekAddress & 1) return ~RamMisc->I2CMem & 1;
	
	bprintf(PRINT_NORMAL, _T("WboyVEEPROM Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall WboyVEEPROMReadWord(UINT32 sekAddress)
{	
	bprintf(PRINT_NORMAL, _T("WboyVEEPROM Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall WboyVEEPROMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (sekAddress & 1) {	
		RamMisc->I2CClk = (byteValue & 0x0002) >> 1;
		RamMisc->I2CMem = (byteValue & 0x0001);
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("WboyVEEPROM write byte value %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall WboyVEEPROMWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("WboyVEEPROM write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall NbajamEEPROMReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Nbajam Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall NbajamEEPROMReadWord(UINT32 /*sekAddress*/)
{
	return RamMisc->I2CMem & 1;
}

void __fastcall NbajamEEPROMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	bprintf(PRINT_NORMAL, _T("Nbajam write byte value %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall NbajamEEPROMWriteWord(UINT32 /*sekAddress*/, UINT16 wordValue)
{
	RamMisc->I2CClk = (wordValue & 0x0002) >> 1;
	RamMisc->I2CMem = (wordValue & 0x0001);
}

UINT8 __fastcall NbajamteEEPROMReadByte(UINT32 sekAddress)
{
	if (sekAddress & 1) return RamMisc->I2CMem & 1;
	
	bprintf(PRINT_NORMAL, _T("Nbajamte Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall NbajamteEEPROMReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Nbajamte Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall NbajamteEEPROMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (sekAddress & 1) {
//		RamMisc->I2CClk = (wordValue & 0x0002) >> 1;
		RamMisc->I2CMem = (byteValue & 0x0001);
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("Nbajamte write byte value %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall NbajamteEEPROMWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Nbajamte write word value %04x to location %08x\n"), wordValue, sekAddress);
}

UINT8 __fastcall EANhlpaEEPROMReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("EANhlpa Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall EANhlpaEEPROMReadWord(UINT32 /*sekAddress*/)
{
	return (RamMisc->I2CMem & 1) << 7;
}

void __fastcall EANhlpaEEPROMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	bprintf(PRINT_NORMAL, _T("EANhlpa write byte value %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall EANhlpaEEPROMWriteWord(UINT32 /*sekAddress*/, UINT16 wordValue)
{
	RamMisc->I2CClk = ((wordValue & 0x0040) >> 6);
	RamMisc->I2CMem = ((wordValue & 0x0080) >> 7);
}

UINT8 __fastcall CodemastersEEPROMReadByte(UINT32 sekAddress)
{
	if (sekAddress & 1) return RamMisc->I2CMem & 1;
	
	bprintf(PRINT_NORMAL, _T("Codemasters Read Byte %x\n"), sekAddress);
	
	return 0;
}

UINT16 __fastcall CodemastersEEPROMReadWord(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("Codemasters Read Word %x\n"), sekAddress);

	return 0;
}

void __fastcall CodemastersEEPROMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (sekAddress & 1) {
		RamMisc->I2CClk = (byteValue & 0x0002) >> 1;
		RamMisc->I2CMem = (byteValue & 0x0001);
		return;
	} else {
		RamMisc->I2CClk = (byteValue & 0x0002) >> 1;
		RamMisc->I2CMem = (byteValue & 0x0001);
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("Codemasters write byte value %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall CodemastersEEPROMWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("Codemasters write word value %04x to location %08x\n"), wordValue, sekAddress);
}

void __fastcall MegadriveSRAMToggleWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	bprintf(PRINT_NORMAL, _T("SRam Toggle byte  %02x to location %08x\n"), byteValue, sekAddress);
}

void __fastcall MegadriveSRAMToggleWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	bprintf(PRINT_NORMAL, _T("SRam Toggle word value %04x to location %08x\n"), wordValue, sekAddress);
}

static void MegadriveSetupSRAM()
{
	SRamSize = 0;
	RamMisc->SRamStart = 0;
	RamMisc->SRamEnd = 0;
	RamMisc->SRamDetected = 0;
	RamMisc->SRamHandlersInstalled = 0;
	RamMisc->SRamActive = 0;
	RamMisc->SRamReadOnly = 0;
	RamMisc->SRamHasSerialEEPROM = 0;
	MegadriveBackupRam = NULL;
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_00400) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_00800) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_01000) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_04000) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_10000)) {
		RamMisc->SRamStart = 0x200000;
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_00400) RamMisc->SRamEnd = 0x2003ff;
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_00800) RamMisc->SRamEnd = 0x2007ff;
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_01000) RamMisc->SRamEnd = 0x200fff;
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_04000) RamMisc->SRamEnd = 0x203fff;
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_SRAM_10000) RamMisc->SRamEnd = 0x20ffff;
		
		RamMisc->SRamDetected = 1;
		MegadriveBackupRam = (UINT16*)RomMain + RamMisc->SRamStart;
		
		SekOpen(0);
		SekMapHandler(5, 0xa130f0, 0xa130f1, SM_WRITE);
		SekSetWriteByteHandler(5, MegadriveSRAMToggleWriteByte);
		SekSetWriteWordHandler(5, MegadriveSRAMToggleWriteWord);
		SekClose();
		
		if (RomSize <= RamMisc->SRamStart) {
			RamMisc->SRamActive = 1;
			InstallSRAMHandlers(false);
		}
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_BEGGAR) {
		RamMisc->SRamStart = 0x400000;
		RamMisc->SRamEnd = 0x40ffff;
		
		RamMisc->SRamDetected = 1;
		MegadriveBackupRam = (UINT16*)RomMain + RamMisc->SRamStart;
		
		RamMisc->SRamActive = 1;		
		InstallSRAMHandlers(false);
	}
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MEGADRIVE_FRAM_00400) {
		RamMisc->SRamStart = 0x200000;
		RamMisc->SRamEnd = 0x2003ff;
		
		RamMisc->SRamDetected = 1;
		MegadriveBackupRam = (UINT16*)RomMain + RamMisc->SRamStart;
		
		SekOpen(0);
		SekMapHandler(5, 0xa130f0, 0xa130f1, SM_READ | SM_WRITE);
		SekSetReadByteHandler(5, Megadrive6658ARegReadByte);
		SekSetReadWordHandler(5, Megadrive6658ARegReadWord);
		SekSetWriteByteHandler(5, Megadrive6658ARegWriteByte);
		SekSetWriteWordHandler(5, Megadrive6658ARegWriteWord);
		SekClose();
		
		InstallSRAMHandlers(false);
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_SEGA_EEPROM) {
		RamMisc->SRamHasSerialEEPROM = 1;
		SekOpen(0);
		SekMapHandler(5, 0x200000, 0x200001, SM_READ | SM_WRITE);
		SekSetReadByteHandler(5, WboyVEEPROMReadByte);
		SekSetReadWordHandler(5, WboyVEEPROMReadWord);
		SekSetWriteByteHandler(5, WboyVEEPROMWriteByte);
		SekSetWriteWordHandler(5, WboyVEEPROMWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_NBA_JAM) {
		RamMisc->SRamHasSerialEEPROM = 1;
		SekOpen(0);
		SekMapHandler(5, 0x200000, 0x200001, SM_READ | SM_WRITE);
		SekSetReadByteHandler(5, NbajamEEPROMReadByte);
		SekSetReadWordHandler(5, NbajamEEPROMReadWord);
		SekSetWriteByteHandler(5, NbajamEEPROMWriteByte);
		SekSetWriteWordHandler(5, NbajamEEPROMWriteWord);
		SekClose();
	}
	
	if (((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_NBA_JAM_TE) || ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_NFL_QB_96) || ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_C_SLAM)) {
		RamMisc->SRamHasSerialEEPROM = 1;
		SekOpen(0);
		SekMapHandler(5, 0x200000, 0x200001, SM_READ | SM_WRITE);
		SekSetReadByteHandler(5, NbajamteEEPROMReadByte);
		SekSetReadWordHandler(5, NbajamteEEPROMReadWord);
		SekSetWriteByteHandler(5, NbajamteEEPROMWriteByte);
		SekSetWriteWordHandler(5, NbajamteEEPROMWriteWord);
		SekClose();
	}
	
	if ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_EA_NHLPA) {
		RamMisc->SRamHasSerialEEPROM = 1;
		SekOpen(0);
		SekMapHandler(5, 0x200000, 0x200001, SM_READ | SM_WRITE);
		SekSetReadByteHandler(5, EANhlpaEEPROMReadByte);
		SekSetReadWordHandler(5, EANhlpaEEPROMReadWord);
		SekSetWriteByteHandler(5, EANhlpaEEPROMWriteByte);
		SekSetWriteWordHandler(5, EANhlpaEEPROMWriteWord);
		SekClose();
	}
	
	if (((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_CODE_MASTERS) || ((BurnDrvGetHardwareCode() & 0xff) == HARDWARE_SEGA_MEGADRIVE_PCB_CM_JCART_SEPROM)) {
		RamMisc->SRamHasSerialEEPROM = 1;
		SekOpen(0);
		SekMapHandler(5, 0x300000, 0x300001, SM_WRITE);
		SekSetWriteByteHandler(5, CodemastersEEPROMWriteByte);
		SekSetWriteWordHandler(5, CodemastersEEPROMWriteWord);
		SekMapHandler(6, 0x380000, 0x380001, SM_READ);
		SekSetReadByteHandler(6, CodemastersEEPROMReadByte);
		SekSetReadWordHandler(6, CodemastersEEPROMReadWord);
		SekClose();
	}
	
	if (!RamMisc->SRamDetected && !RamMisc->SRamHasSerialEEPROM) {
		// check if cart has battery save 
		if (RomMain[0x1b1] == 'R' && RomMain[0x1b0] == 'A') {
			// SRAM info found in header 
			RamMisc->SRamStart = (RomMain[0x1b5] << 24 | RomMain[0x1b4] << 16 | RomMain[0x1b7] << 8 | RomMain[0x1b6]);
			RamMisc->SRamEnd = (RomMain[0x1b9] << 24 | RomMain[0x1b8] << 16 | RomMain[0x1bb] << 8 | RomMain[0x1ba]);

			if ((RamMisc->SRamStart > RamMisc->SRamEnd) || ((RamMisc->SRamEnd - RamMisc->SRamStart) >= 0x10000)) {
				RamMisc->SRamEnd = RamMisc->SRamStart + 0x0FFFF;
			}

			// for some games using serial EEPROM, difference between SRAM end to start is 0 or 1. Currently EEPROM is not emulated.
			if ((RamMisc->SRamEnd - RamMisc->SRamStart) < 2) {
				RamMisc->SRamHasSerialEEPROM = 1;
			} else {
				RamMisc->SRamDetected = 1;
			}
		} else {
			// set default SRAM positions, with size = 64k
			RamMisc->SRamStart = 0x200000;
			RamMisc->SRamEnd = RamMisc->SRamStart + 0xffff;
		}

		if (RamMisc->SRamStart & 1) RamMisc->SRamStart -= 1;

		if (!(RamMisc->SRamEnd & 1)) RamMisc->SRamEnd += 1;

		// calculate backup RAM location 
		MegadriveBackupRam = (UINT16*) (RomMain + (RamMisc->SRamStart & 0x3fffff));

		if (RamMisc->SRamDetected) {
			bprintf(PRINT_IMPORTANT, _T("SRAM detected in header: start %06x - end %06x\n"), RamMisc->SRamStart, RamMisc->SRamEnd);
		}

		// Enable SRAM handlers only if the game does not use EEPROM.
		if (!RamMisc->SRamHasSerialEEPROM) {
			// Info from DGen: If SRAM does not overlap main ROM, set it active by default since a few games can't manage to properly switch it on/off. 
			if (RomSize <= RamMisc->SRamStart) {
				RamMisc->SRamActive = 1;
			}

			SekOpen(0);
			SekMapHandler(5, 0xa130f0, 0xa130f1, SM_WRITE);
			SekSetWriteByteHandler(5, MegadriveSRAMToggleWriteByte);
			SekSetWriteWordHandler(5, MegadriveSRAMToggleWriteWord);
			SekClose();

			// Sonic 1 included in Sonic Classics doesn't have SRAM and does lots of ROM access at this range, then only install read write handlers if SRAM is active to not slow down emulation.
			if (RamMisc->SRamActive) InstallSRAMHandlers(true);
		}
	}
}

INT32 MegadriveInit()
{
	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();	

	MegadriveLoadRoms(0);
	MegadriveLoadRoms(1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	        SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(RomMain,		0x000000, 0x3FFFFF, SM_ROM);	// 68000 ROM
		SekMapMemory(Ram68K,		0xFF0000, 0xFFFFFF, SM_RAM);	// 68000 RAM
		
		SekMapHandler(1,			0xC00000, 0xC0001F, SM_RAM);	// Video Port
		SekMapHandler(2,			0xA00000, 0xA01FFF, SM_RAM);	// Z80 Ram
		SekMapHandler(3,			0xA02000, 0xA03FFF, SM_RAM);	// Z80 Ram
		SekMapHandler(4,			0xA10000, 0xA1001F, SM_RAM);	// I/O
		
		SekSetReadByteHandler (0, MegadriveReadByte);
		SekSetReadWordHandler (0, MegadriveReadWord);
		SekSetWriteByteHandler(0, MegadriveWriteByte);
		SekSetWriteWordHandler(0, MegadriveWriteWord);
		
		SekSetReadByteHandler (1, MegadriveVideoReadByte);
		SekSetReadWordHandler (1, MegadriveVideoReadWord);
		SekSetWriteByteHandler(1, MegadriveVideoWriteByte);
		SekSetWriteWordHandler(1, MegadriveVideoWriteWord);

		SekSetReadByteHandler (2, MegadriveZ80RamReadByte);
		SekSetReadWordHandler (2, MegadriveZ80RamReadWord);
		SekSetWriteByteHandler(2, MegadriveZ80RamWriteByte);
		SekSetWriteWordHandler(2, MegadriveZ80RamWriteWord);
		
		SekSetReadByteHandler (3, MegadriveZ80RamReadByte);
		SekSetReadWordHandler (3, MegadriveZ80RamReadWord);
		SekSetWriteByteHandler(3, MegadriveZ80RamWriteByte);
		SekSetWriteWordHandler(3, MegadriveZ80RamWriteWord);

		SekSetReadByteHandler (4, MegadriveIOReadByte);
		SekSetReadWordHandler (4, MegadriveIOReadWord);
		SekSetWriteByteHandler(4, MegadriveIOWriteByte);
		SekSetWriteWordHandler(4, MegadriveIOWriteWord);

		SekSetIrqCallback( MegadriveIrqCallback );
		SekClose();
	}
	
	{
		ZetInit(0);
		ZetOpen(0);
		
		ZetMapArea(0x0000, 0x1FFF, 0, RamZ80);
		ZetMapArea(0x0000, 0x1FFF, 1, RamZ80);
		ZetMapArea(0x0000, 0x1FFF, 2, RamZ80);

		ZetMapArea(0x2000, 0x3FFF, 0, RamZ80);
		ZetMapArea(0x2000, 0x3FFF, 1, RamZ80);
		ZetMapArea(0x2000, 0x3FFF, 2, RamZ80);
		
		ZetMemEnd();
		
		ZetSetReadHandler(MegadriveZ80ProgRead);
		ZetSetWriteHandler(MegadriveZ80ProgWrite);
		ZetSetInHandler(MegadriveZ80PortRead);
		ZetSetOutHandler(MegadriveZ80PortWrite);
		ZetClose();
	}
	
	// OSC_NTSC / 7
	BurnSetRefreshRate(60.0);

	DrvSECAM = 0;
	BurnYM2612Init(1, OSC_NTSC / 7, NULL, MegadriveSynchroniseStream, MegadriveGetTime, 0);
	BurnTimerAttachSek(OSC_NTSC / 7);
	
	SN76496Init(0, OSC_NTSC / 15, 1);
	SN76496SetVolShift(0, 1);
	
	MegadriveSetupSRAM();
	SetupCustomCartridgeMappers();
	
	if (MegadriveCallback) MegadriveCallback();
	
	pBurnDrvPalette = (UINT32*)MegadriveCurPal;
	
	MegadriveResetDo();	

	return 0;
}

INT32 MegadriveExit()
{
	SekExit();
	ZetExit();

	BurnYM2612Exit();
	SN76496Exit();
	
	BurnFree(Mem);
	BurnFree(OriginalRom);
	
	MegadriveCallback = NULL;
	cycles_68k = 0;
	cycles_z80 = 0;
	RomNoByteswap = 0;
	MegadriveReset = 0;
	RomSize = 0;
	RomNum = 0;
	SRamSize = 0;
	Scanline = 0;
	Z80HasBus = 0;
	MegadriveZ80Reset = 0;
	Hardware = 0;
	DrvSECAM = 0;
	HighCol = NULL;
	
	return 0;
}

//---------------------------------------------------------------
// Megadrive Draw
//---------------------------------------------------------------

static INT32 TileNorm(INT32 sx,INT32 addr,INT32 pal)
{
	UINT8 *pd = HighCol+sx;
	UINT32 pack=0; 
	UINT32 t=0;

	pack = BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid + addr)); // Get 8 pixels
	if (pack) {
		t=pack&0x0000f000; if (t) pd[0]=(UINT8)(pal|(t>>12));
		t=pack&0x00000f00; if (t) pd[1]=(UINT8)(pal|(t>> 8));
		t=pack&0x000000f0; if (t) pd[2]=(UINT8)(pal|(t>> 4));
		t=pack&0x0000000f; if (t) pd[3]=(UINT8)(pal|(t    ));
		t=pack&0xf0000000; if (t) pd[4]=(UINT8)(pal|(t>>28));
		t=pack&0x0f000000; if (t) pd[5]=(UINT8)(pal|(t>>24));
		t=pack&0x00f00000; if (t) pd[6]=(UINT8)(pal|(t>>20));
		t=pack&0x000f0000; if (t) pd[7]=(UINT8)(pal|(t>>16));
		return 0;
	}
	return 1; // Tile blank
}

static INT32 TileFlip(INT32 sx,INT32 addr,INT32 pal)
{
	UINT8 *pd = HighCol+sx;
	UINT32 pack=0; 
	UINT32 t=0;

	pack = BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid + addr)); // Get 8 pixels
	if (pack) {
		t=pack&0x000f0000; if (t) pd[0]=(UINT8)(pal|(t>>16));
		t=pack&0x00f00000; if (t) pd[1]=(UINT8)(pal|(t>>20));
		t=pack&0x0f000000; if (t) pd[2]=(UINT8)(pal|(t>>24));
		t=pack&0xf0000000; if (t) pd[3]=(UINT8)(pal|(t>>28));
		t=pack&0x0000000f; if (t) pd[4]=(UINT8)(pal|(t    ));
		t=pack&0x000000f0; if (t) pd[5]=(UINT8)(pal|(t>> 4));
		t=pack&0x00000f00; if (t) pd[6]=(UINT8)(pal|(t>> 8));
		t=pack&0x0000f000; if (t) pd[7]=(UINT8)(pal|(t>>12));
		return 0;
	}
	return 1; // Tile blank
}

// tile renderers for hacky operator sprite support
#define sh_pix(x) \
  if(!t); \
  else if(t==0xe) pd[x]=(UINT8)((pd[x]&0x3f)|0x80); /* hilight */ \
  else if(t==0xf) pd[x]=(UINT8)((pd[x]&0x3f)|0xc0); /* shadow  */ \
  else pd[x]=(UINT8)(pal|t);

static INT32 TileNormSH(INT32 sx,INT32 addr,INT32 pal)
{
	UINT32 pack=0; UINT32 t=0;
	UINT8 *pd = HighCol+sx;

	pack=BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid+addr)); // Get 8 pixels
	if (pack) {
		t=(pack&0x0000f000)>>12; sh_pix(0);
		t=(pack&0x00000f00)>> 8; sh_pix(1);
		t=(pack&0x000000f0)>> 4; sh_pix(2);
		t=(pack&0x0000000f)    ; sh_pix(3);
		t=(pack&0xf0000000)>>28; sh_pix(4);
		t=(pack&0x0f000000)>>24; sh_pix(5);
		t=(pack&0x00f00000)>>20; sh_pix(6);
		t=(pack&0x000f0000)>>16; sh_pix(7);
		return 0;
	}
	return 1; // Tile blank
}

static INT32 TileFlipSH(INT32 sx,INT32 addr,INT32 pal)
{
	UINT32 pack=0; UINT32 t=0;
	UINT8 *pd = HighCol+sx;

	pack=BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid+addr)); // Get 8 pixels
	if (pack) {
		t=(pack&0x000f0000)>>16; sh_pix(0);
		t=(pack&0x00f00000)>>20; sh_pix(1);
		t=(pack&0x0f000000)>>24; sh_pix(2);
		t=(pack&0xf0000000)>>28; sh_pix(3);
		t=(pack&0x0000000f)    ; sh_pix(4);
		t=(pack&0x000000f0)>> 4; sh_pix(5);
		t=(pack&0x00000f00)>> 8; sh_pix(6);
		t=(pack&0x0000f000)>>12; sh_pix(7);
		return 0;
	}
	return 1; // Tile blank
}

static INT32 TileNormZ(INT32 sx,INT32 addr,INT32 pal,INT32 zval)
{
	UINT32 pack=0; 
	UINT32 t=0;
	UINT8 *pd = HighCol+sx;
	INT8 *zb = HighSprZ+sx;
	INT32 collision = 0, zb_s;

	pack=BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid+addr)); // Get 8 pixels
	if (pack) {
		t=pack&0x0000f000; if(t) { zb_s=zb[0]; if(zb_s) collision=1; if(zval>zb_s) { pd[0]=(UINT8)(pal|(t>>12)); zb[0]=(INT8)zval; } }
		t=pack&0x00000f00; if(t) { zb_s=zb[1]; if(zb_s) collision=1; if(zval>zb_s) { pd[1]=(UINT8)(pal|(t>> 8)); zb[1]=(INT8)zval; } }
		t=pack&0x000000f0; if(t) { zb_s=zb[2]; if(zb_s) collision=1; if(zval>zb_s) { pd[2]=(UINT8)(pal|(t>> 4)); zb[2]=(INT8)zval; } }
		t=pack&0x0000000f; if(t) { zb_s=zb[3]; if(zb_s) collision=1; if(zval>zb_s) { pd[3]=(UINT8)(pal|(t    )); zb[3]=(INT8)zval; } }
		t=pack&0xf0000000; if(t) { zb_s=zb[4]; if(zb_s) collision=1; if(zval>zb_s) { pd[4]=(UINT8)(pal|(t>>28)); zb[4]=(INT8)zval; } }
		t=pack&0x0f000000; if(t) { zb_s=zb[5]; if(zb_s) collision=1; if(zval>zb_s) { pd[5]=(UINT8)(pal|(t>>24)); zb[5]=(INT8)zval; } }
		t=pack&0x00f00000; if(t) { zb_s=zb[6]; if(zb_s) collision=1; if(zval>zb_s) { pd[6]=(UINT8)(pal|(t>>20)); zb[6]=(INT8)zval; } }
		t=pack&0x000f0000; if(t) { zb_s=zb[7]; if(zb_s) collision=1; if(zval>zb_s) { pd[7]=(UINT8)(pal|(t>>16)); zb[7]=(INT8)zval; } }
		if(collision) RamVReg->status |= 0x20;
		return 0;
	}
	return 1; // Tile blank
}

static INT32 TileFlipZ(INT32 sx,INT32 addr,INT32 pal,INT32 zval)
{
	UINT32 pack=0; 
	UINT32 t=0;
	UINT8 *pd = HighCol+sx;
	INT8 *zb = HighSprZ+sx;
	INT32 collision = 0, zb_s;
	
	pack=BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid+addr)); // Get 8 pixels
	if (pack) {
		t=pack&0x000f0000; if(t) { zb_s=zb[0]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[0]=(UINT8)(pal|(t>>16)); zb[0]=(INT8)zval; } }
		t=pack&0x00f00000; if(t) { zb_s=zb[1]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[1]=(UINT8)(pal|(t>>20)); zb[1]=(INT8)zval; } }
		t=pack&0x0f000000; if(t) { zb_s=zb[2]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[2]=(UINT8)(pal|(t>>24)); zb[2]=(INT8)zval; } }
		t=pack&0xf0000000; if(t) { zb_s=zb[3]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[3]=(UINT8)(pal|(t>>28)); zb[3]=(INT8)zval; } }
		t=pack&0x0000000f; if(t) { zb_s=zb[4]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[4]=(UINT8)(pal|(t    )); zb[4]=(INT8)zval; } }
		t=pack&0x000000f0; if(t) { zb_s=zb[5]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[5]=(UINT8)(pal|(t>> 4)); zb[5]=(INT8)zval; } }
		t=pack&0x00000f00; if(t) { zb_s=zb[6]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[6]=(UINT8)(pal|(t>> 8)); zb[6]=(INT8)zval; } }
		t=pack&0x0000f000; if(t) { zb_s=zb[7]&0x1f; if(zb_s) collision=1; if(zval>zb_s) { pd[7]=(UINT8)(pal|(t>>12)); zb[7]=(INT8)zval; } }
		if(collision) RamVReg->status |= 0x20;
		return 0;
 	}
	return 1; // Tile blank
}


#define sh_pixZ(x) \
  if(t) { \
    if(zb[x]) collision=1; \
    if(zval>zb[x]) { \
      if     (t==0xe) { pd[x]=(UINT8)((pd[x]&0x3f)|0x80); /* hilight */ } \
      else if(t==0xf) { pd[x]=(UINT8)((pd[x]&0x3f)|0xc0); /* shadow  */ } \
      else            { zb[x]=(INT8)zval; pd[x]=(UINT8)(pal|t); } \
    } \
  }

static INT32 TileNormZSH(INT32 sx,INT32 addr,INT32 pal,INT32 zval)
{
	UINT32 pack=0; 
	UINT32 t=0;
	UINT8 *pd = HighCol+sx;
	INT8 *zb = HighSprZ+sx;
	INT32 collision = 0;

	pack=BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid+addr)); // Get 8 pixels
	if (pack) {
		t=(pack&0x0000f000)>>12; sh_pixZ(0);
		t=(pack&0x00000f00)>> 8; sh_pixZ(1);
		t=(pack&0x000000f0)>> 4; sh_pixZ(2);
		t=(pack&0x0000000f)    ; sh_pixZ(3);
		t=(pack&0xf0000000)>>28; sh_pixZ(4);
		t=(pack&0x0f000000)>>24; sh_pixZ(5);
		t=(pack&0x00f00000)>>20; sh_pixZ(6);
		t=(pack&0x000f0000)>>16; sh_pixZ(7);
		if(collision) RamVReg->status |= 0x20;
		return 0;
	}
	return 1; // Tile blank
}

static INT32 TileFlipZSH(INT32 sx,INT32 addr,INT32 pal,INT32 zval)
{
	UINT32 pack=0; 
	UINT32 t=0;
	UINT8 *pd = HighCol+sx;
	INT8 *zb = HighSprZ+sx;
	INT32 collision = 0;
	
	pack=BURN_ENDIAN_SWAP_INT32(*(UINT32 *)(RamVid+addr)); // Get 8 pixels
	if (pack) {
		t=(pack&0x000f0000)>>16; sh_pixZ(0);
		t=(pack&0x00f00000)>>20; sh_pixZ(1);
		t=(pack&0x0f000000)>>24; sh_pixZ(2);
		t=(pack&0xf0000000)>>28; sh_pixZ(3);
		t=(pack&0x0000000f)    ; sh_pixZ(4);
		t=(pack&0x000000f0)>> 4; sh_pixZ(5);
		t=(pack&0x00000f00)>> 8; sh_pixZ(6);
		t=(pack&0x0000f000)>>12; sh_pixZ(7);
		if(collision) RamVReg->status |= 0x20;
		return 0;
	}
	return 1; // Tile blank
}

static void DrawStrip(struct TileStrip *ts, INT32 sh)
{
	INT32 tilex=0,dx=0,ty=0,code=0,addr=0,cells;
	INT32 oldcode=-1,blank=-1; // The tile we know is blank
	INT32 pal=0;

	// Draw tiles across screen:
	tilex = (-ts->hscroll)>>3;
	ty = (ts->line&7)<<1; // Y-Offset into tile
	dx = ((ts->hscroll-1)&7)+1;
	cells = ts->cells;
	if(dx != 8) cells++; // have hscroll, need to draw 1 cell more

	for (; cells; dx+=8,tilex++,cells--) {
		INT32 zero=0;

		code=BURN_ENDIAN_SWAP_INT16(RamVid[ts->nametab + (tilex&ts->xmask)]);
		if (code==blank) continue;
		if (code>>15) { // high priority tile
			INT32 cval = code | (dx<<16) | (ty<<25);
			if(code&0x1000) cval^=7<<26;
			*ts->hc++ = cval; // cache it
			continue;
		}

		if (code!=oldcode) {
			oldcode = code;
			// Get tile address/2:
			addr=(code&0x7ff)<<4;
			addr+=ty;
			if (code&0x1000) addr^=0xe; // Y-flip
			pal=((code>>9)&0x30)|(sh<<6);
		}

		if (code&0x0800) zero=TileFlip(dx,addr,pal);
		else             zero=TileNorm(dx,addr,pal);

		if (zero) blank=code; // We know this tile is blank now
	}

	// terminate the cache list
	*ts->hc = 0;
}

static void DrawStripVSRam(struct TileStrip *ts, INT32 plane)
{
	INT32 tilex=0,dx=0,ty=0,code=0,addr=0,cell=0,nametabadd=0;
	INT32 oldcode=-1,blank=-1; // The tile we know is blank
	INT32 pal=0,scan=Scanline;

	// Draw tiles across screen:
	tilex=(-ts->hscroll)>>3;
	dx=((ts->hscroll-1)&7)+1;
	if(dx != 8) {
		INT32 vscroll, line;
		cell--; // have hscroll, start with negative cell
		// also calculate intial VS stuff
		vscroll = BURN_ENDIAN_SWAP_INT16(RamSVid[plane]);

		// Find the line in the name table
		line = (vscroll+scan)&ts->line&0xffff;		// ts->line is really ymask ..
		nametabadd = (line>>3)<<(ts->line>>24);		// .. and shift[width]
		ty = (line&7)<<1;							// Y-Offset into tile
	}

	for (; cell < ts->cells; dx+=8,tilex++,cell++) {
		INT32 zero=0;

		if((cell&1)==0) {
			INT32 line,vscroll;
			vscroll = BURN_ENDIAN_SWAP_INT16(RamSVid[plane+(cell&~1)]);

			// Find the line in the name table
			line = (vscroll+scan)&ts->line&0xffff;	// ts->line is really ymask ..
			nametabadd = (line>>3)<<(ts->line>>24);	// .. and shift[width]
			ty = (line&7)<<1; 						// Y-Offset into tile
		}

		code = BURN_ENDIAN_SWAP_INT16(RamVid[ts->nametab + nametabadd + (tilex&ts->xmask)]);
		if (code==blank) continue;
		if (code>>15) { // high priority tile
			INT32 cval = code | (dx<<16) | (ty<<25);
			if(code&0x1000) cval^=7<<26;
			*ts->hc++ = cval; // cache it
			continue;
		}

		if (code!=oldcode) {
			oldcode = code;
			// Get tile address/2:
			addr=(code&0x7ff)<<4;
			if (code&0x1000) addr+=14-ty; else addr+=ty; // Y-flip
			pal=((code>>9)&0x30);
		}

		if (code&0x0800) zero=TileFlip(dx,addr,pal);
		else             zero=TileNorm(dx,addr,pal);

		if (zero) blank=code; // We know this tile is blank now
	}

	// terminate the cache list
	*ts->hc = 0;
}

static void DrawStripInterlace(struct TileStrip *ts)
{
	INT32 tilex=0,dx=0,ty=0,code=0,addr=0,cells;
	INT32 oldcode=-1,blank=-1; // The tile we know is blank
	INT32 pal=0;

	// Draw tiles across screen:
	tilex=(-ts->hscroll)>>3;
	ty=(ts->line&15)<<1; // Y-Offset into tile
	dx=((ts->hscroll-1)&7)+1;
	cells = ts->cells;
	if(dx != 8) cells++; // have hscroll, need to draw 1 cell more

	for (; cells; dx+=8,tilex++,cells--) {
		INT32 zero=0;

		code=BURN_ENDIAN_SWAP_INT16(RamVid[ts->nametab+(tilex&ts->xmask)]);
		if (code==blank) continue;
		if (code>>15) { // high priority tile
			INT32 cval = (code&0xfc00) | (dx<<16) | (ty<<25);
			cval |= (code&0x3ff)<<1;
			if(code&0x1000) cval^=0xf<<26;
			*ts->hc++ = cval; // cache it
			continue;
		}

		if (code!=oldcode) {
			oldcode = code;
			// Get tile address/2:
			addr=(code&0x7ff)<<5;
			if (code&0x1000) addr+=30-ty; else addr+=ty; // Y-flip
			pal=((code>>9)&0x30);
		}

		if (code&0x0800) zero=TileFlip(dx,addr,pal);
		else             zero=TileNorm(dx,addr,pal);

		if (zero) blank=code; // We know this tile is blank now
	}

	// terminate the cache list
	*ts->hc = 0;
}

static void DrawLayer(INT32 plane, INT32 *hcache, INT32 maxcells, INT32 sh)
{
	const INT8 shift[4]={5,6,5,7}; // 32,64 or 128 sized tilemaps (2 is invalid)
	struct TileStrip ts;
	INT32 width, height, ymask;
	INT32 vscroll, htab;

	ts.hc = hcache;
	ts.cells = maxcells;

	// Work out the TileStrip to draw

	// Work out the name table size: 32 64 or 128 tiles (0-3)
	width  = RamVReg->reg[16];
	height = (width>>4)&3; 
	width &= 3;

	ts.xmask=(1<<shift[width])-1; // X Mask in tiles (0x1f-0x7f)
	ymask=(height<<8)|0xff;       // Y Mask in pixels
	if(width == 1)   ymask&=0x1ff;
	else if(width>1) ymask =0x0ff;

	// Find name table:
	if (plane==0) ts.nametab=(RamVReg->reg[2] & 0x38)<< 9; // A
	else          ts.nametab=(RamVReg->reg[4] & 0x07)<<12; // B

	htab = RamVReg->reg[13] << 9; // Horizontal scroll table address
	if ( RamVReg->reg[11] & 2)     htab += Scanline<<1; // Offset by line
	if ((RamVReg->reg[11] & 1)==0) htab &= ~0xf; // Offset by tile
	htab += plane; // A or B

	// Get horizontal scroll value, will be masked later
	ts.hscroll = BURN_ENDIAN_SWAP_INT16(RamVid[htab & 0x7fff]);

	if((RamVReg->reg[12]&6) == 6) {
		// interlace mode 2
		vscroll = BURN_ENDIAN_SWAP_INT16(RamSVid[plane]); // Get vertical scroll value

		// Find the line in the name table
		ts.line=(vscroll+(Scanline<<1))&((ymask<<1)|1);
		ts.nametab+=(ts.line>>4)<<shift[width];

		DrawStripInterlace(&ts);
	} else if( RamVReg->reg[11]&4) {
		// shit, we have 2-cell column based vscroll
		// luckily this doesn't happen too often
		ts.line = ymask | (shift[width]<<24); // save some stuff instead of line
		DrawStripVSRam(&ts, plane);
	} else {
		vscroll = BURN_ENDIAN_SWAP_INT16(RamSVid[plane]); // Get vertical scroll value

		// Find the line in the name table
		ts.line = (vscroll+Scanline)&ymask;
		ts.nametab += (ts.line>>3)<<shift[width];

		DrawStrip(&ts, sh);
	}
}

static void DrawWindow(INT32 tstart, INT32 tend, INT32 prio, INT32 sh)
{
	INT32 tilex=0, ty=0, nametab, code=0;
	INT32 blank = -1; // The tile we know is blank

	// Find name table line:
	nametab  = (RamVReg->reg[3] & 0x3c)<<9;
	if (RamVReg->reg[12] & 1) nametab += (Scanline>>3)<<6;	// 40-cell mode
	else					  nametab += (Scanline>>3)<<5;	// 32-cell mode

	tilex = tstart<<1;
	tend <<= 1;

	ty = (Scanline & 7)<<1; // Y-Offset into tile

	if(!(rendstatus & 2)) {
		// check the first tile code
		code = BURN_ENDIAN_SWAP_INT16(RamVid[nametab + tilex]);
		// if the whole window uses same priority (what is often the case), we may be able to skip this field
		if((code>>15) != prio) return;
	}

	// Draw tiles across screen:
	for (; tilex < tend; tilex++) {
		INT32 addr=0, zero=0, pal;

		code = BURN_ENDIAN_SWAP_INT16(RamVid[nametab + tilex]);
		if(code==blank) continue;
		if((code>>15) != prio) {
			rendstatus |= 2;
			continue;
		}

		pal=((code>>9)&0x30);

		if(sh) {
			INT32 tmp, *zb = (INT32 *)(HighCol+8+(tilex<<3));
			if(prio) {
				tmp = *zb;
				if(!(tmp&0x00000080)) tmp&=~0x000000c0; if(!(tmp&0x00008000)) tmp&=~0x0000c000;
				if(!(tmp&0x00800000)) tmp&=~0x00c00000; if(!(tmp&0x80000000)) tmp&=~0xc0000000;
				*zb++=tmp; tmp = *zb;
				if(!(tmp&0x00000080)) tmp&=~0x000000c0; if(!(tmp&0x00008000)) tmp&=~0x0000c000;
				if(!(tmp&0x00800000)) tmp&=~0x00c00000; if(!(tmp&0x80000000)) tmp&=~0xc0000000;
				*zb++=tmp;
			} else {
				pal |= 0x40;
			}
		}

		// Get tile address/2:
		addr = (code&0x7ff)<<4;
		if (code&0x1000) addr += 14-ty; else addr += ty; // Y-flip

		if (code&0x0800) zero = TileFlip(8+(tilex<<3),addr,pal);
		else             zero = TileNorm(8+(tilex<<3),addr,pal);

		if (zero) blank = code; // We know this tile is blank now
	}
	// terminate the cache list
	//*hcache = 0;
}

static void DrawTilesFromCache(INT32 *hc, INT32 sh)
{
	INT32 code, addr, zero, dx;
	INT32 pal;
	INT16 blank=-1; // The tile we know is blank

	// *ts->hc++ = code | (dx<<16) | (ty<<25); // cache it

	while((code = *hc++)) {
		if(!sh && (INT16)code == blank) continue;

		// Get tile address/2:
		addr=(code&0x7ff)<<4;
		addr+=(UINT32)code>>25; // y offset into tile
		dx=(code>>16)&0x1ff;
		if(sh) {
			UINT8 *zb = HighCol+dx;
			if(!(*zb&0x80)) *zb&=0x3f; zb++; if(!(*zb&0x80)) *zb&=0x3f; zb++;
			if(!(*zb&0x80)) *zb&=0x3f; zb++; if(!(*zb&0x80)) *zb&=0x3f; zb++;
			if(!(*zb&0x80)) *zb&=0x3f; zb++; if(!(*zb&0x80)) *zb&=0x3f; zb++;
			if(!(*zb&0x80)) *zb&=0x3f; zb++; if(!(*zb&0x80)) *zb&=0x3f; zb++;
		}

		pal=((code>>9)&0x30);

		if (code&0x0800) zero=TileFlip(dx,addr,pal);
		else             zero=TileNorm(dx,addr,pal);

		if(zero) blank=(INT16)code;
	}
}

// Index + 0  :    hhhhvvvv ab--hhvv yyyyyyyy yyyyyyyy // a: offscreen h, b: offs. v, h: horiz. size
// Index + 4  :    xxxxxxxx xxxxxxxx pccvhnnn nnnnnnnn // x: x coord + 8

static void DrawSprite(INT32 *sprite, INT32 **hc, INT32 sh)
{
	INT32 width=0,height=0;
	INT32 row=0,code=0;
	INT32 pal;
	INT32 tile=0,delta=0;
	INT32 sx, sy;
	INT32 (*fTileFunc)(INT32 sx,INT32 addr,INT32 pal);

	// parse the sprite data
	sy=sprite[0];
	code=sprite[1];
	sx=code>>16;		// X
	width=sy>>28;
	height=(sy>>24)&7;	// Width and height in tiles
	sy=(sy<<16)>>16;	// Y

	row=Scanline-sy;	// Row of the sprite we are on

	if (code&0x1000) row=(height<<3)-1-row; // Flip Y

	tile=code&0x7ff;	// Tile number
	tile+=row>>3;		// Tile number increases going down
	delta=height;		// Delta to increase tile by going right
	if (code&0x0800) { tile+=delta*(width-1); delta=-delta; } // Flip X

	tile<<=4; tile+=(row&7)<<1; // Tile address

	if(code&0x8000) { // high priority - cache it
		*(*hc)++ = (tile<<16)|((code&0x0800)<<5)|((sx<<6)&0x0000ffc0)|((code>>9)&0x30)|((sprite[0]>>16)&0xf);
	} else {
		delta<<=4; // Delta of address
		pal=((code>>9)&0x30)|(sh<<6);

		if(sh && (code&0x6000) == 0x6000) {
			if(code&0x0800) fTileFunc=TileFlipSH;
			else            fTileFunc=TileNormSH;
		} else {
			if(code&0x0800) fTileFunc=TileFlip;
			else            fTileFunc=TileNorm;
		}

		for (; width; width--,sx+=8,tile+=delta) {
			if(sx<=0)   continue;
			if(sx>=328) break; // Offscreen

			tile&=0x7fff; // Clip tile address
			fTileFunc(sx,tile,pal);
		}
	}
}

// Index + 0  :    hhhhvvvv s---hhvv yyyyyyyy yyyyyyyy // s: skip flag, h: horiz. size
// Index + 4  :    xxxxxxxx xxxxxxxx pccvhnnn nnnnnnnn // x: x coord + 8

static void DrawSpriteZ(INT32 pack, INT32 pack2, INT32 shpri, INT32 sprio)
{
	INT32 width=0,height=0;
	INT32 row=0;
	INT32 pal;
	INT32 tile=0,delta=0;
	INT32 sx, sy;
	INT32 (*fTileFunc)(INT32 sx,INT32 addr,INT32 pal,INT32 zval);

	// parse the sprite data
	sx    =  pack2>>16;			// X
	sy    = (pack <<16)>>16;	// Y
	width =  pack >>28;
	height= (pack >>24)&7;		// Width and height in tiles

	row = Scanline-sy; 			// Row of the sprite we are on

	if (pack2&0x1000) row=(height<<3)-1-row; // Flip Y

	tile = pack2&0x7ff; 		// Tile number
	tile+= row>>3;				// Tile number increases going down
	delta=height;				// Delta to increase tile by going right
	if (pack2&0x0800) { 		// Flip X
		tile += delta*(width-1); 
		delta = -delta; 
	} 

	tile<<=4; 
	tile+=(row&7)<<1; // Tile address
	delta<<=4; // Delta of address
	pal=((pack2>>9)&0x30);
	if((shpri&1)&&!(shpri&2)) pal|=0x40;

	shpri&=1;
	if((pack2&0x6000) != 0x6000) shpri = 0;
	shpri |= (pack2&0x0800)>>10;
	switch(shpri) {
	default:
	case 0: fTileFunc=TileNormZ;   break;
	case 1: fTileFunc=TileNormZSH; break;
	case 2: fTileFunc=TileFlipZ;   break;
	case 3: fTileFunc=TileFlipZSH; break;
	}

	for (; width; width--,sx+=8,tile+=delta) {
		if(sx<=0)   continue;
		if(sx>=328) break; // Offscreen

		tile&=0x7fff; // Clip tile address
		fTileFunc(sx,tile,pal,sprio);
	}
}


static void DrawSpriteInterlace(UINT32 *sprite)
{
	INT32 width=0,height=0;
	INT32 row=0,code=0;
	INT32 pal;
	INT32 tile=0,delta=0;
	INT32 sx, sy;
	
	// parse the sprite data
	sy=sprite[0];
	height=sy>>24;
	sy=(sy&0x3ff)-0x100; // Y
	width=(height>>2)&3; height&=3;
	width++; height++; // Width and height in tiles
	
	row=(Scanline<<1)-sy; // Row of the sprite we are on
	
	code=sprite[1];
	sx=((code>>16)&0x1ff)-0x78; // X
	
	if (code&0x1000) row^=(16<<height)-1; // Flip Y
	
	tile=code&0x3ff; // Tile number
	tile+=row>>4; // Tile number increases going down
	delta=height; // Delta to increase tile by going right
	if (code&0x0800) { tile+=delta*(width-1); delta=-delta; } // Flip X
	
	tile<<=5; tile+=(row&15)<<1; // Tile address
	
	delta<<=5; // Delta of address
	pal=((code>>9)&0x30); // Get palette pointer

	for (; width; width--,sx+=8,tile+=delta) {
		if(sx<=0)   continue;
		if(sx>=328) break; // Offscreen

		tile&=0x7fff; // Clip tile address
		if (code&0x0800) TileFlip(sx,tile,pal);
		else             TileNorm(sx,tile,pal);
	}
}


static void DrawAllSpritesInterlace(INT32 pri, INT32 maxwidth)
{
	INT32 i,u,table,link=0,sline=Scanline<<1;
	UINT32 *sprites[80]; // Sprite index
	
	table = RamVReg->reg[5]&0x7f;
	if (RamVReg->reg[12]&1) table&=0x7e; // Lowest bit 0 in 40-cell mode
	table<<=8; // Get sprite table address/2
	
	for (i=u=0; u < 80 && i < 21; u++) {
		UINT32 *sprite;
		INT32 code, sx, sy, height;

		sprite=(UINT32 *)(RamVid+((table+(link<<2))&0x7ffc)); // Find sprite

		// get sprite info
		code = BURN_ENDIAN_SWAP_INT32(sprite[0]);
		sx = BURN_ENDIAN_SWAP_INT32(sprite[1]);
		if(((sx>>15)&1) != pri) goto nextsprite; // wrong priority sprite
		
		// check if it is on this line
		sy = (code&0x3ff)-0x100;
		height = (((code>>24)&3)+1)<<4;
		if(sline < sy || sline >= sy+height) goto nextsprite; // no
		
		// check if sprite is not hidden offscreen
		sx = (sx>>16)&0x1ff;
		sx -= 0x78; // Get X coordinate + 8
		if(sx <= -8*3 || sx >= maxwidth) goto nextsprite;
		
		// sprite is good, save it's pointer
		sprites[i++]=sprite;
		
		nextsprite:
		// Find next sprite
		link=(code>>16)&0x7f;
		if(!link) break; // End of sprites
	}

	// Go through sprites backwards:
	for (i-- ;i>=0; i--)
		DrawSpriteInterlace(sprites[i]);
}

static void DrawSpritesFromCache(INT32 *hc, INT32 sh)
{
	INT32 code, tile, sx, delta, width;
	INT32 pal;
	INT32 (*fTileFunc)(INT32 sx,INT32 addr,INT32 pal);
	
	// *(*hc)++ = (tile<<16)|((code&0x0800)<<5)|((sx<<6)&0x0000ffc0)|((code>>9)&0x30)|((sprite[0]>>24)&0xf);
	
	while((code=*hc++)) {
		pal=(code&0x30);
		delta=code&0xf;
		width=delta>>2; delta&=3;
		width++; delta++; // Width and height in tiles
		if (code&0x10000) delta=-delta; // Flip X
		delta<<=4;
		tile=((UINT32)code>>17)<<1;
		sx=(code<<16)>>22; // sx can be negative (start offscreen), so sign extend
		
		if(sh && pal == 0x30) { //
			if(code&0x10000) fTileFunc=TileFlipSH;
			else             fTileFunc=TileNormSH;
		} else {
			if(code&0x10000) fTileFunc=TileFlip;
			else             fTileFunc=TileNorm;
		}

		for (; width; width--,sx+=8,tile+=delta) {
			if(sx<=0)   continue;
			if(sx>=328) break; // Offscreen

			tile&=0x7fff; // Clip tile address
			fTileFunc(sx,tile,pal);
		}
	}
}

// Index + 0  :    ----hhvv -lllllll -------y yyyyyyyy
// Index + 4  :    -------x xxxxxxxx pccvhnnn nnnnnnnn
// v
// Index + 0  :    hhhhvvvv ab--hhvv yyyyyyyy yyyyyyyy // a: offscreen h, b: offs. v, h: horiz. size
// Index + 4  :    xxxxxxxx xxxxxxxx pccvhnnn nnnnnnnn // x: x coord + 8

static void PrepareSprites(INT32 full)
{
	INT32 u=0,link=0,sblocks=0;
	INT32 table=0;
	INT32 *pd = HighPreSpr;
	
	table=RamVReg->reg[5]&0x7f;
	if (RamVReg->reg[12]&1) table&=0x7e; // Lowest bit 0 in 40-cell mode
	table<<=8; // Get sprite table address/2
	
	if (!full) {
		INT32 pack;
		// updates: tilecode, sx
		for (u=0; u < 80 && (pack = *pd); u++, pd+=2) {
			UINT32 *sprite;
			INT32 code, code2, sx, sy, skip=0;
			
			sprite=(UINT32 *)(RamVid+((table+(link<<2))&0x7ffc)); // Find sprite
			
			// parse sprite info
			code  = BURN_ENDIAN_SWAP_INT32(sprite[0]);
			code2 = BURN_ENDIAN_SWAP_INT32(sprite[1]);
			code2 &= ~0xfe000000;
			code2 -=  0x00780000; // Get X coordinate + 8 in upper 16 bits
			sx = code2>>16;

			if((sx <= 8-((pack>>28)<<3) && sx >= -0x76) || sx >= 328) skip=1<<23;
			else if ((sy = (pack<<16)>>16) < 240 && sy > -32) {
				INT32 sbl = (2<<(pack>>28))-1;
				sblocks |= sbl<<(sy>>3);
			}

			*pd = (pack&~(1<<23))|skip;
			*(pd+1) = code2;
			
			// Find next sprite
			link=(code>>16)&0x7f;
			if(!link) break; // End of sprites
		}
		SpriteBlocks |= sblocks;
	} else {
		for (; u < 80; u++) {
			UINT32 *sprite;
			INT32 code, code2, sx, sy, hv, height, width, skip=0, sx_min;
			
			sprite=(UINT32 *)(RamVid+((table+(link<<2))&0x7ffc)); // Find sprite
			
			// parse sprite info
			code = BURN_ENDIAN_SWAP_INT32(sprite[0]);
			sy = (code&0x1ff)-0x80;
			hv = (code>>24)&0xf;
			height = (hv&3)+1;
			
			if(sy > 240 || sy + (height<<3) <= 0) skip|=1<<22;
			
			width  = (hv>>2)+1;
			code2 = BURN_ENDIAN_SWAP_INT32(sprite[1]);
			sx = (code2>>16)&0x1ff;
			sx -= 0x78; // Get X coordinate + 8
			sx_min = 8-(width<<3);
    
			if((sx <= sx_min && sx >= -0x76) || sx >= 328) skip|=1<<23;
			else if (sx > sx_min && !skip) {
				INT32 sbl = (2<<height)-1;
				INT32 shi = sy>>3;
				if(shi < 0) shi=0; // negative sy
				sblocks |= sbl<<shi;
			}
    
			*pd++ = (width<<28)|(height<<24)|skip|(hv<<16)|((UINT16)sy);
			*pd++ = (sx<<16)|((UINT16)code2);
			
			// Find next sprite
			link=(code>>16)&0x7f;
			if(!link) break; // End of sprites
		}
		SpriteBlocks = sblocks;
		*pd = 0; // terminate
	}
}

static void DrawAllSprites(INT32 *hcache, INT32 maxwidth, INT32 prio, INT32 sh)
{
	INT32 i,u,n;
	INT32 sx1seen=0; // sprite with x coord 1 or 0 seen
	INT32 ntiles = 0; // tile counter for sprite limit emulation
	INT32 *sprites[40]; // Sprites to draw in fast mode
	INT32 *ps, pack, rs = rendstatus, scan=Scanline;

	if(rs&8) {
		DrawAllSpritesInterlace(prio, maxwidth);
		return;
	}
	if(rs&0x11) {
		//dprintf("PrepareSprites(%i) [%i]", (rs>>4)&1, scan);
		PrepareSprites(rs&0x10);
		rendstatus=rs&~0x11;
	}
	if (!(SpriteBlocks & (1<<(scan>>3)))) return;

	if(((rs&4)||sh)&&prio==0)
		memset(HighSprZ, 0, 328);
	if(!(rs&4)&&prio) {
		if(hcache[0]) DrawSpritesFromCache(hcache, sh);
		return;
	}

	ps = HighPreSpr;

	// Index + 0  :    hhhhvvvv ab--hhvv yyyyyyyy yyyyyyyy // a: offscreen h, b: offs. v, h: horiz. size
	// Index + 4  :    xxxxxxxx xxxxxxxx pccvhnnn nnnnnnnn // x: x coord + 8

	for(i=u=n=0; (pack = *ps) && n < 20; ps+=2, u++) {
		INT32 sx, sy, row, pack2;

		if(pack & 0x00400000) continue;

		// get sprite info
		pack2 = *(ps+1);
		sx =  pack2>>16;
		sy = (pack <<16)>>16;
		row = scan-sy;

		//dprintf("x: %i y: %i %ix%i", sx, sy, (pack>>28)<<3, (pack>>21)&0x38);

		if(sx == -0x77) sx1seen |= 1; // for masking mode 2

		// check if it is on this line
		if(row < 0 || row >= ((pack>>21)&0x38)) continue; // no
		n++; // number of sprites on this line (both visible and hidden, max is 20) [broken]

		// sprite limit
		ntiles += pack>>28;
		if(ntiles > 40) break;

		if(pack & 0x00800000) continue;

		// masking sprite?
		if(sx == -0x78) {
			if(!(sx1seen&1) || sx1seen==3) {
				break; // this sprite is not drawn and remaining sprites are masked
			}
			if((sx1seen>>8) == 0) sx1seen=(i+1)<<8;
			continue;
		}
		else if(sx == -0x77) {
			// masking mode2 (Outrun, Galaxy Force II, Shadow of the beast)
			if(sx1seen>>8) { 
				i=(sx1seen>>8)-1; 
				break; 
			} // seen both 0 and 1
			sx1seen |= 2;
			continue;
		}

		// accurate sprites
		//dprintf("P:%i",((sx>>15)&1));
		if(rs&4) {
			// might need to skip this sprite
			if((pack2&0x8000) ^ (prio<<15)) continue;
			DrawSpriteZ(pack,pack2,sh|(prio<<1),(INT8)(0x1f-n));
			continue;
		}

		// sprite is good, save it's pointer
		sprites[i++]=ps;
	}

	// Go through sprites backwards:
	if(!(rs&4)) {
		for (i--; i>=0; i--)
			DrawSprite(sprites[i],&hcache,sh);

		// terminate cache list
		*hcache = 0;
	}
}


static void BackFill(INT32 reg7, INT32 sh)
{
	// Start with a blank scanline (background colour):
	UINT32 *pd = (UINT32 *)(HighCol+8);
	UINT32 *end= (UINT32 *)(HighCol+8+320);
	UINT32 back = reg7 & 0x3f;
	back |= sh<<6;
	back |= back<<8;
	back |= back<<16;
	do { pd[0]=pd[1]=pd[2]=pd[3]=back; pd+=4; } while (pd < end);
}


static INT32 DrawDisplay(INT32 sh)
{
	INT32 maxw, maxcells;
	INT32 win=0, edge=0, hvwind=0;
	
	if(RamVReg->reg[12] & 1) {
		maxw = 328; maxcells = 40;
	} else {
		maxw = 264; maxcells = 32;
	}
	
	// Find out if the window is on this line:
	win = RamVReg->reg[0x12];
	edge = (win & 0x1f)<<3;
  	
  	if (win&0x80) { if (Scanline>=edge) hvwind=1; }
	else          { if (Scanline< edge) hvwind=1; }
	
	if(!hvwind) { // we might have a vertical window here 
		win = RamVReg->reg[0x11];
		edge = win&0x1f;
		if(win&0x80) {
			if(!edge) hvwind=1;
			else if(edge < (maxcells>>1)) hvwind=2;
		} else {
			if(!edge);
			else if(edge < (maxcells>>1)) hvwind=2;
			else hvwind=1;
		}
	}	
	
	DrawLayer(1, HighCacheB, maxcells, sh);
	if(hvwind == 1)
		DrawWindow(0, maxcells>>1, 0, sh); // HighCacheAW
	else if(hvwind == 2) {
		// ahh, we have vertical window
		DrawLayer(0, HighCacheA, (win&0x80) ? edge<<1 : maxcells, sh);
		DrawWindow((win&0x80) ? edge : 0, (win&0x80) ? maxcells>>1 : edge, 0, sh); // HighCacheW
	} else
		DrawLayer(0, HighCacheA, maxcells, sh);
	DrawAllSprites(HighCacheS, maxw, 0, sh);
	
	if(HighCacheB[0]) 
		DrawTilesFromCache(HighCacheB, sh);
	if(hvwind == 1)
		DrawWindow(0, maxcells>>1, 1, sh);
	else if(hvwind == 2) {
		if(HighCacheA[0]) DrawTilesFromCache(HighCacheA, sh);
		DrawWindow((win&0x80) ? edge : 0, (win&0x80) ? maxcells>>1 : edge, 1, sh);
	} else
		if(HighCacheA[0]) DrawTilesFromCache(HighCacheA, sh);
	DrawAllSprites(HighCacheS, maxw, 1, sh);

	return 0;
}

static void PicoFrameStart()
{
	// prepare to do this frame
	rendstatus = 0x80 >> 5;							// accurate sprites
	RamVReg->status &= ~0x0020;
	if((RamVReg->reg[12]&6) == 6) rendstatus |= 8;	// interlace mode
	//if(Pico.m.dirtyPal) Pico.m.dirtyPal = 2; 		// reset dirty if needed
	PrepareSprites(1);
}

static INT32 PicoLine(INT32 /*scan*/)
{
	INT32 sh = (RamVReg->reg[0xC] & 8)>>3; // shadow/hilight?

	BackFill(RamVReg->reg[7], sh);
	if (RamVReg->reg[1] & 0x40)
		DrawDisplay(sh);
	
	{
		INT32 num = Scanline + 1;
		if (!(RamVReg->reg[1] & 8)) num += 8;
		HighCol = HighColFull + ( num * (8 + 320 + 8) );
	};

	return 0;
}

static void MegadriveDraw()
{
	UINT16 * pDest = (UINT16 *)pBurnDraw;

	if ((RamVReg->reg[12]&1) || !(MegadriveDIP[1] & 0x03)) {
	
		for (INT32 j=0; j<224; j++) {
			UINT8 * pSrc = HighColFull + (j+9)*(8+320+8) + 8;
			for (INT32 i=0;i<320;i++)
				pDest[i] = MegadriveCurPal[ pSrc[i] ];
			pDest += 320;
		}
	
	} else {  
		
		if (( MegadriveDIP[1] & 0x03 ) == 0x01 ) {
			// Center 
			pDest += 32;
			for (INT32 j=0; j<224; j++) {
				UINT8 * pSrc = HighColFull + (j+9)*(8+320+8) + 8;

				memset((UINT8 *)pDest -  32*2, 0, 64);
				
				for (INT32 i=0;i<256;i++)
					pDest[i] = MegadriveCurPal[ pSrc[i] ];
				
				memset((UINT8 *)pDest + 256*2, 0, 64);
				
				pDest += 320;
			}
		} else {
			// Zoom
			for (INT32 j=0; j<224; j++) {
				UINT8 * pSrc = HighColFull + (j+9)*(8+320+8) + 8;
				UINT32 delta = 0;
				for (INT32 i=0;i<320;i++) {
					pDest[i] = MegadriveCurPal[ pSrc[ delta >> 16 ] ];
					delta += 0xCCCC;
				}
				pDest += 320;
			}
		}
		
	}
}

#define TOTAL_68K_CYCLES	((double)OSC_NTSC / 7) / 60
#define TOTAL_Z80_CYCLES	((double)OSC_NTSC / 15) / 60
#define TOTAL_68K_CYCLES_PAL	((double)OSC_PAL / 7) / 50
#define TOTAL_Z80_CYCLES_PAL	((double)OSC_PAL / 15) / 50

INT32 MegadriveFrame()
{
	if (MegadriveReset) {
		MegadriveResetDo();
		MegadriveReset = 0;
	}

	if (bMegadriveRecalcPalette) {
		for (INT32 i=0;i<0x40;i++)
			CalcCol(i, BURN_ENDIAN_SWAP_INT16(RamPal[i]));
		bMegadriveRecalcPalette = 0;	
	}
	
	JoyPad->pad[0] = JoyPad->pad[1] = JoyPad->pad[2] = JoyPad->pad[3] = 0;
	for (INT32 i = 0; i < 12; i++) {
		JoyPad->pad[0] |= (MegadriveJoy1[i] & 1) << i;
		JoyPad->pad[1] |= (MegadriveJoy2[i] & 1) << i;
		JoyPad->pad[2] |= (MegadriveJoy3[i] & 1) << i;
		JoyPad->pad[3] |= (MegadriveJoy4[i] & 1) << i;
	}
	
	
	SekNewFrame();
	ZetNewFrame();
	
	HighCol = HighColFull;
	PicoFrameStart();

	INT32 lines,lines_vis = 224,line_sample;
	INT32 done_z80 = 0;
	INT32 hint = RamVReg->reg[10]; // Hint counter
	INT32 total_68k_cycles, total_z80_cycles;
	
	if( Hardware & 0x40 ) {
		lines  = 313;
		line_sample = 68;
		if( RamVReg->reg[1]&8 ) lines_vis = 240;
		total_68k_cycles = (INT32)(INT64)(TOTAL_68K_CYCLES_PAL * nBurnCPUSpeedAdjust / 0x100);
		total_z80_cycles = (INT32)TOTAL_Z80_CYCLES_PAL;
	} else {
		lines  = 262;
		line_sample = 93;
		total_68k_cycles = (INT32)(INT64)(TOTAL_68K_CYCLES * nBurnCPUSpeedAdjust / 0x100);
		total_z80_cycles = (INT32)TOTAL_Z80_CYCLES;
	}
	
	cycles_68k = total_68k_cycles / lines;
	cycles_z80 = total_z80_cycles / lines;
  
	RamVReg->status &= ~0x88; // clear V-Int, come out of vblank
	
	for (INT32 y=0; y<lines; y++) {

		Scanline = y;

		/*if(PicoOpt&0x20)*/ {
			// pad delay (for 6 button pads)
			if(JoyPad->padDelay[0]++ > 25) JoyPad->padTHPhase[0] = 0;
			if(JoyPad->padDelay[1]++ > 25) JoyPad->padTHPhase[1] = 0;
		}

		// H-Interrupts:
		if((y <= lines_vis) && (--hint < 0)) { // y <= lines_vis: Comix Zone, Golden Axe
			//dprintf("rhint:old @ %06x", SekPc);
			hint = RamVReg->reg[10]; // Reload H-Int counter
			RamVReg->pending_ints |= 0x10;
			if (RamVReg->reg[0] & 0x10) {
				SekOpen(0);
				SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
				SekClose();
			}
		}

		// V-Interrupt:
		if (y == lines_vis) {
			//dprintf("vint: @ %06x [%i|%i]", SekPc, y, SekCycleCnt);
			RamVReg->status |= 0x88; // V-Int happened, go into vblank
			
			// there must be a gap between H and V ints, also after vblank bit set (Mazin Saga, Bram Stoker's Dracula)
			SekOpen(0);
//			done_68k+=SekRun(128); 
			BurnTimerUpdate((y * cycles_68k) + 128 - cycles_68k);
			SekClose();

			RamVReg->pending_ints |= 0x20;
			if(RamVReg->reg[1] & 0x20) {
				SekOpen(0);
				SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
				SekClose();
			}
		}

		// decide if we draw this line
		if ((!(RamVReg->reg[1]&8) && y<=224) || ((RamVReg->reg[1]&8) && y<240))
			PicoLine(y);

		// Run scanline
		SekOpen(0);
		BurnTimerUpdate(y * cycles_68k);
		SekClose();
		
		if (Z80HasBus && !MegadriveZ80Reset) {
			ZetOpen(0);
			done_z80 += ZetRun(cycles_z80);
			if (y == line_sample) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			if (y == line_sample + 1) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			ZetClose();
		}
	}
	
	if (pBurnDraw) MegadriveDraw();

	SekOpen(0);
	BurnTimerEndFrame(total_68k_cycles);
	SekClose();
	
	if (Z80HasBus && !MegadriveZ80Reset) {
		if (done_z80 < total_z80_cycles) {
			ZetOpen(0);
			ZetRun(total_z80_cycles - done_z80);
			ZetClose();
		}
	}
	
	if (pBurnSoundOut) {
		SekOpen(0);
		BurnYM2612Update(pBurnSoundOut, nBurnSoundLen);
		SekClose();
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
	}
	
	return 0;
}

INT32 MegadriveScan(INT32 /*nAction*/, INT32 * /*pnMin*/)
{
	//BurnYM2612Scan(nAction, pnMin);	
	
	return 1;
}
