#ifndef __PCFX_VDC_H
#define __PCFX_VDC_H

typedef struct
{
	int32 PeriodCounter;
	int32 PeriodMode;

        int32 sat_dma_slcounter;

        uint8 select;
        uint16 MAWR;    // Memory Address Write Register
        uint16 MARR;    // Memory Address Read Register
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

        uint32 RCRCount;

        uint16 read_buffer;
        uint8 write_latch;
        uint8 status;

        uint16 SAT[0x100];
        uint16 VRAM[65536];
        uint8 bg_tile_cache[2048 * 2][8][8] __attribute__ ((aligned (8))); // Tile, y, x

        uint16 DMAReadBuffer;
        bool8 DMAReadWrite;
        bool8 DMARunning;
        bool8 SATBPending;
        bool8 burst_mode;

        uint32 BG_YOffset;      // Reloaded from BYR at start of display area?
        uint32 BG_XOffset;      // Reloaded from BXR at each scanline, methinks.

        unsigned int VDS_cache;
        unsigned int VSW_cache;
        unsigned int VDW_cache;
        unsigned int VCR_cache;
        unsigned int VBlankFL_cache;
        unsigned int dot_clock;

        unsigned int ilevel;

	uint16 line_BXR_Cache, line_BYR_Cache;

	uint32 LayerDisable;

	uint32 Aux0BPBase;
	bool8 VBlank_Test_Occurred; // Since last vsync
	bool8 VBlank_Pending;
} fx_vdc_t;

void FXVDC_Reset(fx_vdc_t *vdc);
void FXVDC_Power(fx_vdc_t *vdc);

void FXVDC_Write16(fx_vdc_t *vdc, uint32 A, uint16 V);
uint16 FXVDC_Read16(fx_vdc_t *vdc, uint32 A);

void FXVDC_SetDotClock(fx_vdc_t *vdc, unsigned int);

fx_vdc_t *FXVDC_Init(int ilevel, bool disable_sprite_limit);
void FXVDC_Close(fx_vdc_t *vdc);

void FXVDC_VSync(fx_vdc_t *vdc);

void FXVDC_DoLineHBlank(fx_vdc_t *vdc);
void FXVDC_DoLine(fx_vdc_t *vdc, uint32 *linebuffer, int skip);
int FXVDC_StateAction(StateMem *sm, int load, int data_only, fx_vdc_t *vdc, const char *name);

#ifdef WANT_DEBUGGER
void FXVDC_SetRegister(fx_vdc_t *vdc, const std::string &name, uint32 value);
uint32 FXVDC_GetRegister(fx_vdc_t *vdc, const std::string &name, std::string *special);
void FXVDC_GetAddressSpaceBytes(fx_vdc_t *vdc, const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void FXVDC_PutAddressSpaceBytes(fx_vdc_t *vdc, const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer);
#endif

bool FXVDC_ToggleLayer(fx_vdc_t *vdc, int which);
void FXVDC_DoGfxDecode(fx_vdc_t *vdc, uint32 *palette_ptr, uint32 *target, int w, int h, int scroll, bool DecodeSprites);

void FXVDC_SetAux0BPBpase(fx_vdc_t *vdc, uint32 new_base); // pass ~0 to disable
#endif
