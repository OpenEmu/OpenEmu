#ifndef __PCE_VDC_H
#define __PCE_VDC_H

#include "mednafen/lepacker.h"

#define VDC_PIXEL_OUT_MASK	0x01FF

// This bit will be set for a non-sprite pixel if the BG layer is disabled(via ToggleLayer()),
#define VDC_BGDISABLE_OUT_MASK	0x0200

// HSync and VSync out bits are only valid when the EX bits in VDC's CR 
// are set so that the VDC will output sync signals rather than
// input them.  If it is not configured in this manner, the bit(s) shall always be 0.
#define VDC_HSYNC_OUT_MASK	0x2000
#define VDC_VSYNC_OUT_MASK	0x4000

// The DISP bit can either denote active display area(1 = active, 0 = inactive),
// colorburst insertion period(0 = insert colorburst, 1 = not in colorburst period; may not be emulated correctly),
// or "internal horizontal synchronous signal"(may not be emulated correctly), depending on the TE
// bits in the VDC's CR.
#define VDC_DISP_OUT_MASK	0x8000

#define VDC_REGSETP(_reg, _data, _msb) { _reg &= 0xFF << ((_msb) ? 0 : 8); _reg |= (_data) << ((_msb) ? 8 : 0); }
#define VDC_REGGETP(_reg, _msb) ((_reg >> ((_msb) ? 8 : 0)) & 0xFF)

static const unsigned int vram_inc_tab[4] = { 1, 32, 64, 128 };

#define VDC_IS_BSY	(pending_read || pending_write)

typedef struct
{
        uint32 x;
        uint32 flags;
        uint8 palette_index;
        uint16 pattern_data[4];
} SPRLE;

typedef struct
{
	// In the case the VDC access doesn't cause a VRAM read/write, only ReadCount/WriteCount will be set to 0.
	uint32 ReadStart;
	uint32 ReadCount;
	uint32 WriteStart;
	uint32 WriteCount;
} VDC_SimulateResult;

class VDC
{
	public:

	// The VRAM size is specified in 16-bit words.
	VDC(bool nospritelimit, uint32 par_VRAM_Size);
	~VDC();

#if 0
	void *operator new(size_t bcount)
	{
	 void *ret = calloc(1, bcount);
 	 return(ret);
	}

	void operator delete(void *ptr)
	{
	 free(ptr);
	}
#endif

	int32 Reset(void) MDFN_WARN_UNUSED_RESULT;

	// ResetSimulate(), SimulateWrite(), and SimulateRead() are intended to handle VRAM read/write breakpoints.
	// SimulateWrite() and SimulateRead() will return the VRAM address that will EVENTUALLY be written(upper 32-bits) and/or read(lower 32-bits) to
	// due to the access, or 0xFFFFFFFF in the upper or lower 32-bits if no VRAM access of that type occurs.
	//
	// The feature is intended to support block moves to VRAM in a single instruction.  It may not function properly if the address passed to SimulateRead()
	// or SimulateWrite() alternates(even if just once) between the data port high byte and control port between calls to ResetSimulate()
	//  Call to reset simulation state.


	INLINE void ResetSimulate(void)
	{
	 Simulate_MAWR = MAWR;
	 Simulate_MARR = MARR;

	 Simulate_select = select;
	 Simulate_CR = CR;

	 Simulate_LENR = LENR;
	}

        INLINE void SimulateRead(uint32 A, VDC_SimulateResult *result)
        {
	 result->ReadCount = 0;
	 result->WriteCount = 0;
         result->ReadStart = 0;
         result->WriteStart = 0;

	 if((A & 0x3) == 0x3 && Simulate_select == 0x02)
	 {
	  Simulate_MARR += vram_inc_tab[(Simulate_CR >> 11) & 0x3];

	  result->ReadStart = Simulate_MARR;
	  result->ReadCount = 1;
	 }
        }

	INLINE void SimulateWrite(uint32 A, uint8 V, VDC_SimulateResult *result)
	{
         result->ReadCount = 0;
         result->WriteCount = 0;
	 result->ReadStart = 0;
	 result->WriteStart = 0;

	 const unsigned int msb = A & 1;

	 switch(A & 0x3)
	 {
	  case 0x00: Simulate_select = V & 0x1F;
		     break;

	  case 0x02:
	  case 0x03:

		  switch(Simulate_select)
		  {
	          	case 0x00: VDC_REGSETP(Simulate_MAWR, V, msb);
				   break;

	         	case 0x01: VDC_REGSETP(Simulate_MARR, V, msb);
	                           Simulate_MARR += vram_inc_tab[(Simulate_CR >> 11) & 0x3];

				   result->ReadStart = Simulate_MARR;
				   result->ReadCount = 1;
				   break;

			case 0x02: if(msb)
				   {
				    result->WriteStart = Simulate_MAWR;
				    result->WriteCount = 1;

				    Simulate_MAWR += vram_inc_tab[(Simulate_CR >> 11) & 0x3];
				   }
				   break;

			case 0x12: VDC_REGSETP(Simulate_LENR, V, msb);
				   if(msb)
				   {
                                    result->ReadStart = SOUR;
                                    result->ReadCount = Simulate_LENR + 1;

				    if(DCR & 0x4)
				     result->ReadStart = (result->ReadStart - (result->ReadCount - 1)) & 0xFFFF;

                                    result->WriteStart = DESR;
                                    result->WriteCount = Simulate_LENR + 1;      

				    if(DCR & 0x8)
				     result->WriteStart = (result->WriteStart - (result->WriteCount - 1)) & 0xFFFF;
				   }
				   break;

		  }
		  break;
	 }
	}


	int32 HSync(bool);
	int32 VSync(bool);


	void Write(uint32 A, uint8 V, int32 &next_event);
	uint8 Read(uint32 A, int32 &next_event, bool peek = FALSE);

        void Write16(bool A, uint16 V);
        uint16 Read16(bool A, bool peek = FALSE);

	int32 Run(int32 clocks, /*bool hs, bool vs,*/ uint16 *pixels, bool skip);


	void FixTileCache(uint16);
	void SetLayerEnableMask(uint64 mask);

	void RunDMA(int32, bool force_completion = FALSE);
	void RunSATDMA(int32, bool force_completion = FALSE);

	void IncRCR(void);
	void DoVBIRQTest(void);
	void HDS_Start(void);

	void StateExtra(MDFN::LEPacker &sl_packer, bool load);
	int StateAction(StateMem *sm, int load, int data_only, const char *sname);

	// Peek(VRAM/SAT) and Poke(VRAM/SAT) work in 16-bit VRAM word units.
	INLINE uint16 PeekVRAM(uint16 Address)
	{
	 if(Address < VRAM_Size)
 	  return(VRAM[Address]);
	 else
	  return(0);
	}

        INLINE uint16 PeekSAT(uint8 Address)
        {
	 return(SAT[Address]);
        }

	INLINE void PokeVRAM(uint16 Address, const uint16 Data)
	{
	 if(Address < VRAM_Size)
	 {
	  VRAM[Address] = Data;
	  FixTileCache(Address);
	 }
	}

	INLINE void PokeSAT(uint8 Address, const uint16 Data)
	{
	 SAT[Address] = Data;
	}


	// Register enums for GetRegister() and SetRegister()
	enum
	{
	 GSREG_MAWR = 0,
	 GSREG_MARR,
	 GSREG_CR,
	 GSREG_RCR,
	 GSREG_BXR,
	 GSREG_BYR,
	 GSREG_MWR,
	 GSREG_HSR,
	 GSREG_HDR,
	 GSREG_VSR,
	 GSREG_VDR,
	 GSREG_VCR,
	 GSREG_DCR,
	 GSREG_SOUR,
	 GSREG_DESR,
	 GSREG_LENR,
	 GSREG_DVSSR,

	 GSREG_SELECT,
	 GSREG_STATUS,

	 __GSREG_COUNT
	};

	// Pass NULL if you don't want more information about the special meaning of the value in the specified
	// register.  Otherwise, pass a buffer of at least 256 bytes in size.
	uint32 GetRegister(const unsigned int id, char *special, const uint32 special_len);
	void SetRegister(const unsigned int id, const uint32 value);

	#ifdef WANT_DEBUGGER
        bool DoGfxDecode(uint32 *target, const uint32 *color_table, const uint32 TransparentColor, bool DecodeSprites,
	        int32 w, int32 h, int32 scroll);
	#endif

	INLINE bool PeekIRQ(void)
	{
	 return((bool)(status & 0x3F));
	}

	INLINE void SetIRQHook(void (*irqh)(bool))
	{
	 IRQHook = irqh;
	}

	INLINE void SetWSHook(bool (*wsh)(int32))
	{
	 WSHook = wsh;
	}

	private:

	int TimeFromHDSStartToBYRLatch(void);
	int TimeFromBYRLatchToBXRLatch(void);

	enum
	{
	 HPHASE_HDS = 0,
	 HPHASE_HDS_PART2,
	 HPHASE_HDS_PART3,
	 HPHASE_HDW,
	 HPHASE_HDW_FINAL,
	 HPHASE_HDE,
	 HPHASE_HSW,
	 HPHASE_COUNT
	};

	enum
	{
	 VPHASE_VDS = 0,
	 VPHASE_VDW,
	 VPHASE_VCR,
	 VPHASE_VSW,
	 VPHASE_COUNT
	};

	int VRAM_Size; // = 0x8000;
	int VRAM_SizeMask; // = VRAM_Size - 1; //0x7FFF;
	int VRAM_BGTileNoMask; // = VRAM_SizeMask / 16; //0x7FF;

	void (*IRQHook)(bool);
	bool (*WSHook)(int32);

	void DoWaitStates(void);
	void CheckAndCommitPending(void);

	INLINE int32 CalcNextEvent(void)
	{
	 int32 next_event = HPhaseCounter;

	 if(sat_dma_counter > 0 && sat_dma_counter < next_event)
	  next_event = sat_dma_counter;

	 if(sprite_cg_fetch_counter > 0 && sprite_cg_fetch_counter < next_event)
	  next_event = sprite_cg_fetch_counter;

	 if(DMARunning)
	 {
	  assert(VDMA_CycleCounter < 2);

	  int32 next_vram_dma_event = ((LENR + 1) * 4) - (DMAReadWrite * 2) - VDMA_CycleCounter;

	  assert(next_vram_dma_event > 0);

	  if(next_vram_dma_event > 0 && next_vram_dma_event < next_event)
	   next_event = next_vram_dma_event;

	  //printf("Next VRAM DMA event: %d(LENR = %d)\n", next_vram_dma_event, LENR);
	 }

         assert(next_event > 0);
	 return(next_event);
	}

	bool in_exhsync, in_exvsync;

	void CalcWidthStartEnd(uint32 &display_width, uint32 &start, uint32 &end);
	void DrawBG(uint16 *target, int enabled);
	void DrawSprites(uint16 *target, int enabled);
	void FetchSpriteData(void);


	uint8 Simulate_select;
	uint16 Simulate_MAWR;
	uint16 Simulate_MARR;
	uint16 Simulate_CR;
	uint16 Simulate_LENR;

        int32 sat_dma_counter;

        uint8 select;
        uint16 MAWR;    // Memory Address Write Register
        uint16 MARR;    // Memory Address Read Register

        uint16 CR;      // Control Register
	uint16 CR_cache;	// Cache for BG/SPR enable
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
        uint16 DVSSR;

	// Internal SAT DMA transfer variables.
	//uint16 SAT_SOUR;
	//uint16 SAT_DESR;
	//uint16 SAT_LENR;

	int32 VDMA_CycleCounter;

        uint32 RCRCount;

	bool pending_read;
	uint16 pending_read_addr;
        uint16 read_buffer;

        uint8 write_latch; // LSB

	bool pending_write;
	uint16 pending_write_addr;
	uint16 pending_write_latch;

        uint8 status;

        uint16 SAT[0x100];

        uint16 VRAM[65536]; //VRAM_Size];

	union
	{
	 uint64 bg_tile_cache64[65536 / 16][8];			// Tile, y, x
	 uint8 bg_tile_cache[65536 / 16][8][8];
	};

        uint16 DMAReadBuffer;
        bool DMAReadWrite;
        bool DMARunning;
	bool DMAPending;
        bool SATBPending;
        bool burst_mode;

        uint32 BG_YOffset;      // Reloaded from BYR at start of display area?
        uint32 BG_XOffset;      // Reloaded from BXR at each scanline, methinks.

	uint32 HSW_cache, HDS_cache, HDW_cache, HDE_cache;

	uint32 VDS_cache;
	uint32 VSW_cache;
	uint32 VDW_cache;
	uint32 VCR_cache;
	uint16 MWR_cache;


	uint32 BG_YMoo;
	bool NeedRCRInc, NeedVBIRQTest, NeedSATDMATest, NeedBGYInc;
	int HPhase, VPhase;
	int32 HPhaseCounter, VPhaseCounter;

	int32 sprite_cg_fetch_counter;


	int32 mystery_counter;
	bool mystery_phase;

	uint16 linebuf[1024 + 512];
	uint32 pixel_desu;
	int32 pixel_copy_count;
	uint32 userle; // User layer enable.
	bool unlimited_sprites;

	int active_sprites;
	SPRLE SpriteList[64 * 2]; // (see unlimited_sprites option, *2 to accommodate 32-pixel-width sprites ) //16];
};

#endif
