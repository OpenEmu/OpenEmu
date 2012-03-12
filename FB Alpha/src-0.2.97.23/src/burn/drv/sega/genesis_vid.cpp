#include "genesis_vid.h"
#include "burnint.h"

INT32 GenesisScreenNumber;
UINT32 *GenesisPalette = NULL;
UINT16 GenesisPaletteBase;
UINT8 VdpBgColour;

UINT8 *VdpVRAM = NULL;
UINT8 *VdpVSRAM = NULL;
UINT16 *VdpTransLookup = NULL;
UINT8 GenesisVdpRegs[32];
UINT16 GenesisBgPalLookup[4];
UINT16 GenesisSpPalLookup[4];
UINT32 VdpScrollABase;
UINT32 VdpScrollBBase;
UINT32 VdpWindowBase;
UINT32 VdpSpriteBase;

INT32 VdpHScrollMask;
UINT32 VdpHScrollSize;
UINT32 VdpHScrollBase;
UINT8 VdpVScrollMode;

UINT8 VdpWindowWidth;
UINT8 VdpScrollWidth;
UINT8 VdpScrollHeight;
UINT8 VdpWindowRight;
UINT8 VdpWindowDown;
UINT32 VdpWindowHPos;
UINT32 VdpWindowVPos;

UINT8 VdpCmdPart;
UINT8 VdpCode;
UINT32 VdpAddress;

#define VRAM_SIZE			0x10000
#define VRAM_MASK			(VRAM_SIZE - 1)
#define VSRAM_SIZE			0x80
#define VSRAM_MASK			(VSRAM_SIZE - 1)
#define CRAM_SIZE			0x40

#define VDP_VRAM_BYTE(x)	(VdpVRAM[(x) & VRAM_MASK])
#define VDP_VSRAM_BYTE(x)	(VdpVSRAM[(x) & VSRAM_MASK])
#define VDP_VRAM_WORD(x)	((VDP_VRAM_BYTE(x) << 8) | VDP_VRAM_BYTE((x) + 1))
#define VDP_VSRAM_WORD(x)	((VDP_VSRAM_BYTE(x) << 8) | VDP_VSRAM_BYTE((x) + 1))

#define EXTRACT_PIXEL(x,i)	(((x) >> (((i) ^ 1) * 4)) & 0x0f)

static void VdpRegisterWrite(INT32 d, INT32 /*vBlank*/)
{	
	UINT8 RegNum = (d & 0x1f00) >> 8;
	UINT8 RegDat = d & 0x00ff;
	
	GenesisVdpRegs[RegNum] = RegDat;
	
	switch (RegNum) {
		case 0x01: {
			// Video Modes
			return;
		}
		
		case 0x02: {
			// Scroll A Name Table Base
			VdpScrollABase = (RegDat & 0x38) << 10;
			return;
		}
		
		case 0x03: {
			// Window Name Table Base
			VdpWindowBase = (RegDat & 0x3e) << 10;
			return;
		}
		
		case 0x04: {
			// Scroll B Name Table Base
			VdpScrollBBase = (RegDat & 0x07) << 13;
			return;
		}
		
		case 0x05: {
			// Sprite Table Base
			VdpSpriteBase = (RegDat & 0x7e) << 9;
			return;
		}
		
		case 0x07: {
			// BG Colour
			VdpBgColour = RegDat & 0x3f;
			return;
		}
		
		case 0x0b: {
			// Scroll Modes
			static const UINT16 MaskTable[4] = { 0x000, 0x007, 0xff8, 0xfff };
			VdpVScrollMode = (RegDat & 0x04) >> 2;
			VdpHScrollMask = MaskTable[RegDat & 3];
			VdpHScrollSize = 4 * ((VdpHScrollMask < 224) ? (VdpHScrollMask + 1) : 224);
			return;
		}
		
		case 0x0c: {
			// Video Modes
			switch (RegDat & 0x81) {
				case 0x00: {
					// 32 cell
					VdpWindowWidth = 32;
					return;
				}
				
				case 0x01: {
					// 40 cell corrupted
					VdpWindowWidth = 64;
					return;
				}
				
				case 0x80: {
					// illegal
					VdpWindowWidth = 64;
					return;
				}
				
				case 0x81: {
					// 40 cell
					VdpWindowWidth = 64;
					return;
				}
			}
			return;
		}
		
		case 0x0d: {
			// H Scroll Base
			VdpHScrollBase = (RegDat & 0x3f) << 10;
			return;
		}
		
		case 0x10: {
			// Scroll Size
			static const UINT8 SizeTable[4] = { 32, 64, 128, 128 };
			VdpScrollWidth = SizeTable[RegDat & 0x03];
			VdpScrollHeight = SizeTable[(RegDat & 0x30) >> 4];
			return;
		}
		
		case 0x11: {
			// Window H Pos
			VdpWindowRight = RegDat & 0x80;
			VdpWindowHPos = (RegDat & 0x1f) << 4;
			return;
		}
		
		case 012: {
			// Window V Pos
			VdpWindowDown = RegDat & 0x80;
			VdpWindowVPos = (RegDat & 0x1f) << 3;
			return;
		}
	}
}

static UINT16 VDPDataRead()
{
	INT32 read = 0;
	
	VdpCmdPart = 0;
	
	switch (VdpCode & 0xf) {
		case 0x00: {
			read = VDP_VRAM_WORD(VdpAddress & ~1);
			break;
		}
		
		case 0x04: {
			read = VDP_VSRAM_WORD(VdpAddress & ~1);
			break;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Genesis VDP Data Read %x\n"), VdpCode);
//		}
	}
	
	VdpAddress += GenesisVdpRegs[15];
	return read;
}

static UINT16 VDPControlRead()
{
	INT32 status = 0x3600;
	
	VdpCmdPart = 0;
	
	// If Vblank status |= 0x8
	// if Hblank status |= 0x4
	
	return status;
}

UINT16 GenesisVDPRead(UINT32 offset)
{
	switch(offset) {
		case 0x00:
		case 0x01: {
			return VDPDataRead();
		}
		
		case 0x02:
		case 0x03: {
			return VDPControlRead();
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Genesis VDP Read %x\n"), offset);
	
	return 0;
}

static void VDPControlWrite(UINT16 data)
{
	if (!VdpCmdPart) {
		if ((data & 0xc000) == 0x8000) {
			VdpRegisterWrite(data, 0);
		} else {
			VdpCode = (VdpCode & 0x3c) | ((data >> 14) & 0x03);
			VdpAddress = (VdpAddress & 0xc000) | (data & 0x3fff);
			VdpCmdPart = 1;
		}
	} else {
		VdpCode = (VdpCode & 0x03) | ((data >> 2) & 0x3c);
		VdpAddress = (VdpAddress & 0x3fff) | ((data << 14) & 0xc000);
		VdpCmdPart = 0;
		// DMA Stuff here
	}
}

inline static UINT8 pal3bit(UINT8 bits)
{
	bits &= 7;
	return (bits << 5) | (bits << 2) | (bits >> 1);
}

static void VDPDataWrite(UINT16 data)
{
	VdpCmdPart = 0;
	
	// DMA Stuff here
	
	switch (VdpCode & 0xf) {
		case 0x01: {
			if (VdpAddress & 1) data = ((data & 0xff) << 8) | ((data >> 8) & 0xff);
			VDP_VRAM_BYTE(VdpAddress & ~1) = data >> 8;
			VDP_VRAM_BYTE(VdpAddress | 1 ) = data;
			break;
		}
		
		case 0x03: {
			INT32 offset = (VdpAddress >> 1) % CRAM_SIZE;
			//palette_set_color(Machine, offset + genesis_palette_base, pal3bit(data >> 1), pal3bit(data >> 5), pal3bit(data >> 9));
			//System16Palette[offset + 0x1800 /*GenesisPaletteBase*/] = BurnHighCol(pal3bit(data >> 1), pal3bit(data >> 5), pal3bit(data >> 9), 0);
			GenesisPalette[offset + GenesisPaletteBase] = BurnHighCol(pal3bit(data >> 1), pal3bit(data >> 5), pal3bit(data >> 9), 0);
			break;
		}
		
		case 0x05: {
			if (VdpAddress & 1) data = ((data & 0xff) << 8) | ((data >> 8) & 0xff);
			VDP_VSRAM_BYTE(VdpAddress & ~1) = data >> 8;
			VDP_VSRAM_BYTE(VdpAddress |  1) = data;
			break;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Genesis VDP Data Write %x\n"), VdpCode);
//		}
	}
	
	VdpAddress += GenesisVdpRegs[15];
}

void GenesisVDPWrite(UINT32 offset, UINT16 data)
{
	switch (offset) {
		case 0x00:
		case 0x01: {
			VDPDataWrite(data);
			return;
		}
		
		case 0x02:
		case 0x03: {
			VDPControlWrite(data);
			return;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Genesis VDP Write %x\n"), data);
}

INT32 StartGenesisVDP(INT32 ScreenNum, UINT32* pal)
{
	INT32 i;
	static const UINT8 VdpInit[24] =
	{
		0x04, 0x44, 0x30, 0x3C, 0x07, 0x6C, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0x01, 0x37, 0x00, 0x02,
		0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x80,
	};
	
	GenesisScreenNumber = ScreenNum;
	
	GenesisPalette = pal;
	
	VdpVRAM = (UINT8*)BurnMalloc(VRAM_SIZE);
	VdpVSRAM = (UINT8*)BurnMalloc(VSRAM_SIZE);
	VdpTransLookup = (UINT16*)BurnMalloc(0x1000 * sizeof(UINT16));
	
	memset(VdpVRAM, 0, VRAM_SIZE);
	memset(VdpVSRAM, 0, VSRAM_SIZE);
	
	// Init the transparency lookup table
	for (i = 0; i < 0x1000; i++) {
		INT32 OriginalColour = i & 0x7ff;
		INT32 HalfBright = i & 0x800;

		if (OriginalColour & 0x100)
			VdpTransLookup[i] = OriginalColour;
		else if (HalfBright)
			VdpTransLookup[i] = OriginalColour | 0x800;
		else
			VdpTransLookup[i] = OriginalColour | 0x1000;
	}
	
	// Reset Palettes
	GenesisPaletteBase = 0;
	GenesisBgPalLookup[0] = GenesisSpPalLookup[0] = 0x00;
	GenesisBgPalLookup[1] = GenesisSpPalLookup[1] = 0x10;
	GenesisBgPalLookup[2] = GenesisSpPalLookup[2] = 0x20;
	GenesisBgPalLookup[3] = GenesisSpPalLookup[3] = 0x30;
	
	// Reset VDP
	for (i = 0; i < 24; i++) {
		VdpRegisterWrite(0x8000 | (i << 8) | VdpInit[i], 1);
	}
	VdpCmdPart = 0;
	VdpCode = 0;
	VdpAddress = 0;
	
	return 0;
}

void GenesisVDPExit()
{
	BurnFree(VdpVRAM);
	BurnFree(VdpVSRAM);
	BurnFree(VdpTransLookup);
	
	memset(GenesisVdpRegs, 0, sizeof(GenesisVdpRegs));
	memset(GenesisBgPalLookup, 0, sizeof(GenesisBgPalLookup));
	memset(GenesisSpPalLookup, 0, sizeof(GenesisSpPalLookup));
	
	GenesisPalette = NULL;
	
	GenesisScreenNumber = 0;
	GenesisPaletteBase = 0;
	VdpBgColour = 0;
	VdpScrollABase = 0;
	VdpScrollBBase = 0;
	VdpWindowBase = 0;
	VdpSpriteBase = 0;
	VdpHScrollMask = 0;
	VdpHScrollSize = 0;
	VdpHScrollBase = 0;
	VdpVScrollMode = 0;
	VdpWindowWidth = 0;
	VdpScrollWidth = 0;
	VdpScrollHeight = 0;
	VdpWindowRight = 0;
	VdpWindowDown = 0;
	VdpWindowHPos = 0;
	VdpWindowVPos = 0;
	VdpCmdPart = 0;
	VdpCode = 0;
	VdpAddress = 0;
}

void GenesisVDPScan()
{
	struct BurnArea ba;
	
	memset(&ba, 0, sizeof(ba));
	ba.Data = VdpVRAM;
	ba.nLen = VRAM_SIZE;
	ba.szName = "GenVDP VRAM";
	BurnAcb(&ba);
	
	memset(&ba, 0, sizeof(ba));
	ba.Data = VdpVSRAM;
	ba.nLen = VSRAM_SIZE;
	ba.szName = "GenVDP VSRAM";
	BurnAcb(&ba);

	ScanVar(GenesisVdpRegs, 32, "GenVDP Regs");
	
	SCAN_VAR(VdpBgColour);
	SCAN_VAR(VdpScrollABase);
	SCAN_VAR(VdpScrollBBase);
	SCAN_VAR(VdpWindowBase);
	SCAN_VAR(VdpSpriteBase);
	SCAN_VAR(VdpHScrollMask);
	SCAN_VAR(VdpHScrollSize);
	SCAN_VAR(VdpHScrollBase);
	SCAN_VAR(VdpVScrollMode);
	SCAN_VAR(VdpWindowWidth);
	SCAN_VAR(VdpScrollWidth);
	SCAN_VAR(VdpScrollHeight);
	SCAN_VAR(VdpWindowRight);
	SCAN_VAR(VdpWindowDown);
	SCAN_VAR(VdpWindowHPos);
	SCAN_VAR(VdpWindowVPos);
	SCAN_VAR(VdpCmdPart);
	SCAN_VAR(VdpCode);
	SCAN_VAR(VdpAddress);
}

inline INT32 vdp_gethscroll(INT32 plane, INT32 line)
{
	line &= VdpHScrollMask;
	return 0x400 - (VDP_VRAM_WORD(VdpHScrollBase + (4 * line) + plane) & 0x3ff);
}


/* Note: We expect plane = 0 for Scroll A, plane = 2 for Scroll B
   A Column is 8 Pixels Wide                                     */
static INT32 vdp_getvscroll(INT32 plane, INT32 column)
{
	UINT32 vsramoffset;

	switch (VdpVScrollMode)
	{
		case 0x00: /* Overall Scroll */
			return VDP_VSRAM_WORD(plane) & 0x7ff;

		case 0x01: /* Column Scroll */
			if (column == 40) column = 39; /* Fix Minor Innacuracy Only affects PotoPoto */
			vsramoffset = (4 * (column >> 1)) + plane;
			return VDP_VSRAM_WORD(vsramoffset) & 0x7ff;
	}
	return 0;
}

static void get_scroll_tiles(UINT32 line, INT32 scrollnum, UINT32 scrollbase, UINT32 *tiles, INT32 *offset)
{
	INT32 linehscroll = vdp_gethscroll(scrollnum, line);
	INT32 column;

	/* adjust for partial tiles and then pre-divide hscroll to get the tile offset */
	*offset = -(linehscroll % 8);
	linehscroll /= 8;

	/* loop over columns */
	for (column = 0; column < 41; column++)
	{
		INT32 columnvscroll = vdp_getvscroll(scrollnum, (column - (linehscroll & 1)) & 0x3f) + line;

		/* determine the base of the tilemap row */
		INT32 temp = ((columnvscroll / 8) & (VdpScrollHeight - 1)) * VdpScrollWidth;
		INT32 tilebase = scrollbase + 2 * temp;

		/* offset into the tilemap based on the column */
		temp = (linehscroll + column) & (VdpScrollWidth - 1);
		tilebase += 2 * temp;

		/* get the tile info */
		*tiles++ = ((columnvscroll % 8) << 16) | VDP_VRAM_WORD(tilebase);
	}
}

static void get_window_tiles(UINT32 line, UINT32 scrollbase, UINT32 *tiles)
{
	INT32 column;

	/* loop over columns */
	for (column = 0; column < 40; column++)
	{
		/* determine the base of the tilemap row */
		INT32 temp = (line / 8) * VdpWindowWidth + column;
		INT32 tilebase = scrollbase + 2 * temp;

		/* get the tile info */
		*tiles++ = ((line % 8) << 16) | VDP_VRAM_WORD(tilebase);
	}
}

static void drawline_tiles(UINT32 *tiles, UINT16 *bmap, UINT32 pri, INT32 offset, INT32 lclip, INT32 rclip)
{
	/* adjust for the 8-pixel slop */
	bmap += offset;
	if (lclip > rclip)
		return;

	/* loop over columns */
	for ( ; offset < 320; offset += 8, bmap += 8)
	{
		UINT32 tile = *tiles++;

		/* if the tile is the correct priority, draw it */
		if (((tile >> 15) & 1) == pri && offset < 320)
		{
			INT32 colbase = GenesisBgPalLookup[(tile & 0x6000) >> 13];
			UINT32 *tp = (UINT32 *)&VDP_VRAM_BYTE((tile & 0x7ff) * 32);
			UINT32 mytile;
			INT32 col;

			/* vertical flipping */
			if (!(tile & 0x1000))
				mytile = tp[tile >> 16];
			else
				mytile = tp[(tile >> 16) ^ 7];

			/* skip if all-transparent */
			if (!mytile)
				continue;

			/* non-clipped */
			if (offset >= lclip && offset <= rclip - 7)
			{
				/* non-flipped */
				if (!(tile & 0x0800))
				{
					col = EXTRACT_PIXEL(mytile, 0); if (col) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 7); if (col) bmap[7] = colbase + col;
				}

				/* horizontal flip */
				else
				{
					col = EXTRACT_PIXEL(mytile, 7); if (col) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 0); if (col) bmap[7] = colbase + col;
				}
			}

			/* clipped */
			else if (offset >= lclip - 8 && offset <= rclip)
			{
				/* non-flipped */
				if (!(tile & 0x0800))
				{
					col = EXTRACT_PIXEL(mytile, 0); if (col && (offset + 0) >= lclip && (offset + 0) <= rclip) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col && (offset + 1) >= lclip && (offset + 1) <= rclip) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col && (offset + 2) >= lclip && (offset + 2) <= rclip) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col && (offset + 3) >= lclip && (offset + 3) <= rclip) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col && (offset + 4) >= lclip && (offset + 4) <= rclip) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col && (offset + 5) >= lclip && (offset + 5) <= rclip) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col && (offset + 6) >= lclip && (offset + 6) <= rclip) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 7); if (col && (offset + 7) >= lclip && (offset + 7) <= rclip) bmap[7] = colbase + col;
				}

				/* horizontal flip */
				else
				{
					col = EXTRACT_PIXEL(mytile, 7); if (col && (offset + 0) >= lclip && (offset + 0) <= rclip) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col && (offset + 1) >= lclip && (offset + 1) <= rclip) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col && (offset + 2) >= lclip && (offset + 2) <= rclip) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col && (offset + 3) >= lclip && (offset + 3) <= rclip) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col && (offset + 4) >= lclip && (offset + 4) <= rclip) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col && (offset + 5) >= lclip && (offset + 5) <= rclip) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col && (offset + 6) >= lclip && (offset + 6) <= rclip) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 0); if (col && (offset + 7) >= lclip && (offset + 7) <= rclip) bmap[7] = colbase + col;
				}
			}
		}
	}
}

static void draw8pixs(UINT16 *bmap, INT32 patno, INT32 /*priority*/, INT32 colbase, INT32 patline)
{
	UINT32 tile = *(UINT32 *)&VDP_VRAM_BYTE(patno * 32 + 4 * patline);
	INT32 col;

	/* skip if all-transparent */
	if (!tile)
		return;

	/* non-transparent */
	if ((colbase & 0x30) != 0x30 || !(GenesisVdpRegs[12] & 0x08))
	{
		col = EXTRACT_PIXEL(tile, 0); if (col) bmap[0] = colbase + col;
		col = EXTRACT_PIXEL(tile, 1); if (col) bmap[1] = colbase + col;
		col = EXTRACT_PIXEL(tile, 2); if (col) bmap[2] = colbase + col;
		col = EXTRACT_PIXEL(tile, 3); if (col) bmap[3] = colbase + col;
		col = EXTRACT_PIXEL(tile, 4); if (col) bmap[4] = colbase + col;
		col = EXTRACT_PIXEL(tile, 5); if (col) bmap[5] = colbase + col;
		col = EXTRACT_PIXEL(tile, 6); if (col) bmap[6] = colbase + col;
		col = EXTRACT_PIXEL(tile, 7); if (col) bmap[7] = colbase + col;
	}

	/* transparent */
	else
	{
		col = EXTRACT_PIXEL(tile, 0);
		if (col)
		{
			if (col < 0x0e) bmap[0] = colbase + col;
			else bmap[0] = VdpTransLookup[((col & 1) << 11) | (bmap[0] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 1);
		if (col)
		{
			if (col < 0x0e) bmap[1] = colbase + col;
			else bmap[1] = VdpTransLookup[((col & 1) << 11) | (bmap[1] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 2);
		if (col)
		{
			if (col < 0x0e) bmap[2] = colbase + col;
			else bmap[2] = VdpTransLookup[((col & 1) << 11) | (bmap[2] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 3);
		if (col)
		{
			if (col < 0x0e) bmap[3] = colbase + col;
			else bmap[3] = VdpTransLookup[((col & 1) << 11) | (bmap[3] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 4);
		if (col)
		{
			if (col < 0x0e) bmap[4] = colbase + col;
			else bmap[4] = VdpTransLookup[((col & 1) << 11) | (bmap[4] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 5);
		if (col)
		{
			if (col < 0x0e) bmap[5] = colbase + col;
			else bmap[5] = VdpTransLookup[((col & 1) << 11) | (bmap[5] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 6);
		if (col)
		{
			if (col < 0x0e) bmap[6] = colbase + col;
			else bmap[6] = VdpTransLookup[((col & 1) << 11) | (bmap[6] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 7);
		if (col)
		{
			if (col < 0x0e) bmap[7] = colbase + col;
			else bmap[7] = VdpTransLookup[((col & 1) << 11) | (bmap[7] & 0x7ff)];
		}
	}
}


/* draw a horizontally-flipped section of a sprite */
static void draw8pixs_hflip(UINT16 *bmap, INT32 patno, INT32 /*priority*/, INT32 colbase, INT32 patline)
{
	UINT32 tile = *(UINT32 *)&VDP_VRAM_BYTE(patno * 32 + 4 * patline);
	INT32 col;

	/* skip if all-transparent */
	if (!tile)
		return;

	/* non-transparent */
	if ((colbase & 0x30) != 0x30 || !(GenesisVdpRegs[12] & 0x08))
	{
		col = EXTRACT_PIXEL(tile, 7); if (col) bmap[0] = colbase + col;
		col = EXTRACT_PIXEL(tile, 6); if (col) bmap[1] = colbase + col;
		col = EXTRACT_PIXEL(tile, 5); if (col) bmap[2] = colbase + col;
		col = EXTRACT_PIXEL(tile, 4); if (col) bmap[3] = colbase + col;
		col = EXTRACT_PIXEL(tile, 3); if (col) bmap[4] = colbase + col;
		col = EXTRACT_PIXEL(tile, 2); if (col) bmap[5] = colbase + col;
		col = EXTRACT_PIXEL(tile, 1); if (col) bmap[6] = colbase + col;
		col = EXTRACT_PIXEL(tile, 0); if (col) bmap[7] = colbase + col;
	}

	/* transparent */
	else
	{
		col = EXTRACT_PIXEL(tile, 7);
		if (col)
		{
			if (col < 0x0e) bmap[0] = colbase + col;
			else bmap[0] = VdpTransLookup[((col & 1) << 11) | (bmap[0] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 6);
		if (col)
		{
			if (col < 0x0e) bmap[1] = colbase + col;
			else bmap[1] = VdpTransLookup[((col & 1) << 11) | (bmap[1] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 5);
		if (col)
		{
			if (col < 0x0e) bmap[2] = colbase + col;
			else bmap[2] = VdpTransLookup[((col & 1) << 11) | (bmap[2] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 4);
		if (col)
		{
			if (col < 0x0e) bmap[3] = colbase + col;
			else bmap[3] = VdpTransLookup[((col & 1) << 11) | (bmap[3] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 3);
		if (col)
		{
			if (col < 0x0e) bmap[4] = colbase + col;
			else bmap[4] = VdpTransLookup[((col & 1) << 11) | (bmap[4] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 2);
		if (col)
		{
			if (col < 0x0e) bmap[5] = colbase + col;
			else bmap[5] = VdpTransLookup[((col & 1) << 11) | (bmap[5] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 1);
		if (col)
		{
			if (col < 0x0e) bmap[6] = colbase + col;
			else bmap[6] = VdpTransLookup[((col & 1) << 11) | (bmap[6] & 0x7ff)];
		}
		col = EXTRACT_PIXEL(tile, 0);
		if (col)
		{
			if (col < 0x0e) bmap[7] = colbase + col;
			else bmap[7] = VdpTransLookup[((col & 1) << 11) | (bmap[7] & 0x7ff)];
		}
	}
}

static void drawline_sprite(INT32 line, UINT16 *bmap, INT32 priority, UINT8 *spritebase)
{
	INT32 spriteypos   = (((spritebase[0] & 0x01) << 8) | spritebase[1]) - 0x80;
	INT32 spritexpos   = (((spritebase[6] & 0x01) << 8) | spritebase[7]) - 0x80;
	INT32 spriteheight = ((spritebase[2] & 0x03) + 1) * 8;
	INT32 spritewidth  = (((spritebase[2] & 0x0c) >> 2) + 1) * 8;
	INT32 spriteattr, patno, patflip, patline, colbase, x;

	/* skip if out of range */
	if (line < spriteypos || line >= spriteypos + spriteheight)
		return;
	if (spritexpos + spritewidth < 0 || spritexpos >= 320)
		return;

	/* extract the remaining data */
	spriteattr = (spritebase[4] << 8) | spritebase[5];
	patno      = spriteattr & 0x07FF;
	patflip    = (spriteattr & 0x1800) >> 11;
	patline    = line - spriteypos;

	/* determine the color base */
	colbase = GenesisSpPalLookup[(spriteattr & 0x6000) >> 13];

	/* adjust for the X position */
	spritewidth >>= 3;
	spriteheight >>= 3;

	/* switch off the flip mode */
	bmap += spritexpos;
	switch (patflip)
	{
		case 0x00: /* No Flip */
			for (x = 0; x < spritewidth; x++, bmap += 8)
			{
				if (spritexpos >= -7 && spritexpos < 320)
					draw8pixs(bmap, patno, priority, colbase, patline);
				spritexpos += 8;
				patno += spriteheight;
			}
			break;

		case 0x01: /* Horizontal Flip */
			patno += spriteheight * (spritewidth - 1);
			for (x = 0; x < spritewidth; x++, bmap += 8)
			{
				if (spritexpos >= -7 && spritexpos < 320)
					draw8pixs_hflip(bmap, patno, priority, colbase, patline);
				spritexpos += 8;
				patno -= spriteheight;
			}
			break;

		case 0x02: /* Vertical Flip */
			patline = 8 * spriteheight - patline - 1;
			for (x = 0; x < spritewidth; x++, bmap += 8)
			{
				if (spritexpos >= -7 && spritexpos < 320)
					draw8pixs(bmap, patno, priority, colbase, patline);
				spritexpos += 8;
				patno += spriteheight;
			}
			break;

		case 0x03: /* Both Flip */
			patno += spriteheight * (spritewidth - 1);
			patline = 8 * spriteheight - patline - 1;
			for (x = 0; x < spritewidth; x++, bmap += 8)
			{
				if (spritexpos >= -7 && spritexpos < 320)
					draw8pixs_hflip(bmap, patno, priority, colbase, patline);
				spritexpos += 8;
				patno -= spriteheight;
			}
			break;
	}
}

void vdp_drawline(UINT16 *bitmap, UINT32 line, INT32 bgfill)
{
	INT32 lowsprites, highsprites, link;
	UINT32 scrolla_tiles[41], scrollb_tiles[41], window_tiles[41];
	INT32 scrolla_offset, scrollb_offset;
	UINT8 *lowlist[81], *highlist[81];
	INT32 bgcolor = bgfill ? bgfill : GenesisBgPalLookup[0];
	INT32 window_lclip, window_rclip;
	INT32 scrolla_lclip, scrolla_rclip;
	INT32 column, sprite;

	/* clear to the background color */
	for (column = 0; column < 320; column++)
		bitmap[column] = bgcolor;

	/* if display is disabled, stop */
	if (!(GenesisVdpRegs[1] & 0x40))
		return;

	/* Sprites need to be Drawn in Reverse order .. may as well sort them here */
	link = lowsprites = highsprites = 0;
	for (sprite = 0; sprite < 80; sprite++)
	{
		UINT8 *spritebase = &VDP_VRAM_BYTE(VdpSpriteBase + 8 * link);

		/* sort into high/low priorities */
		if (spritebase[4] & 0x0080)
			highlist[++highsprites] = spritebase;
		else
			lowlist[++lowsprites] = spritebase;

		/* get the link; if 0, stop processing */
		link = spritebase[3] & 0x7F;
		if (!link)
			break;
	}

	/* get tiles for the B scroll layer */
	get_scroll_tiles(line, 2, VdpScrollBBase, scrollb_tiles, &scrollb_offset);

	/* get tiles for the A scroll layer */
	get_scroll_tiles(line, 0, VdpScrollABase, scrolla_tiles, &scrolla_offset);

	/* get tiles for the window layer */
	get_window_tiles(line, VdpWindowBase, window_tiles);

	/* compute the windowing for this line */
	if ((VdpWindowDown && line >= VdpWindowVPos) || (!VdpWindowDown && line < VdpWindowVPos))
		window_lclip = 0, window_rclip = 320 - 1;
	else if (VdpWindowRight)
		window_lclip = VdpWindowHPos, window_rclip = 320 - 1;
	else
		window_lclip = 0, window_rclip = VdpWindowHPos - 1;

	/* compute the clipping of the scroll A layer */
	if (window_lclip == 0)
	{
		scrolla_lclip = window_rclip + 1;
		scrolla_rclip = 320 - 1;
	}
	else
	{
		scrolla_lclip = 0;
		scrolla_rclip = window_lclip - 1;
	}

	/* Scroll B Low */
	drawline_tiles(scrollb_tiles, bitmap, 0, scrollb_offset, 0, 320 - 1);

	/* Scroll A Low */
	drawline_tiles(scrolla_tiles, bitmap, 0, scrolla_offset, scrolla_lclip, scrolla_rclip);

	/* Window Low */
	drawline_tiles(window_tiles, bitmap, 0, 0, window_lclip, window_rclip);

	/* Sprites Low */
	for (sprite = lowsprites; sprite > 0; sprite--)
		drawline_sprite(line, bitmap, 0, lowlist[sprite]);

	/* Scroll B High */
	drawline_tiles(scrollb_tiles, bitmap, 1, scrollb_offset, 0, 320 - 1);

	/* Scroll A High */
	drawline_tiles(scrolla_tiles, bitmap, 1, scrolla_offset, scrolla_lclip, scrolla_rclip);

	/* Window High */
	drawline_tiles(window_tiles, bitmap, 1, 0, window_lclip, window_rclip);

	/* Sprites High */
	for (sprite = highsprites; sprite > 0; sprite--)
		drawline_sprite(line, bitmap, 1, highlist[sprite]);
}
