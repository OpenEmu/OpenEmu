#ifndef _PCE_VDC_H
#define _PCE_VDC_H

namespace PCE_Fast
{

#define REGSETP(_reg, _data, _msb) { _reg &= 0xFF << ((_msb) ? 0 : 8); _reg |= (_data) << ((_msb) ? 8 : 0); }
#define REGGETP(_reg, _msb) ((_reg >> ((_msb) ? 8 : 0)) & 0xFF)

#define VDC_DEBUG(x)
//printf("%s: %d\n", x, vdc->display_counter);
#define VDC_UNDEFINED(x) { }
//{ printf("%s: %d\n", x, vdc->display_counter); }

static const unsigned int vram_inc_tab[4] = { 1, 32, 64, 128 };

typedef struct
{
        uint8 priority[2];
        uint16 winwidths[2];
        uint8 st_mode;
} vpc_t;

extern vpc_t vpc;

static const int VRAM_Size = 0x8000;
static const int VRAM_SizeMask = VRAM_Size - 1; //0x7FFF;
static const int VRAM_BGTileNoMask = VRAM_SizeMask / 16; //0x7FF;

typedef struct
{
        uint8 CR;

        bool lc263;    // 263 line count if set, 262 if not
        bool bw;       // Black and White
        uint8 dot_clock; // Dot Clock(5, 7, or 10 MHz = 0, 1, 2)
        uint16 color_table[0x200];
        uint32 color_table_cache[0x200];
        uint16 ctaddress;
} vce_t;

extern vce_t vce;

typedef struct
{
        int16 y;
	uint16 height;
        uint16 x;
        uint16 no;
        uint16 flags;
        bool cgmode;
} SAT_Cache_t;

typedef struct
{
        uint32 display_counter;

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

        uint16 DMAReadBuffer;
        bool DMAReadWrite;
        bool DMARunning;
        bool SATBPending;
        bool burst_mode;

        uint32 BG_YOffset;
        uint32 BG_XOffset;



        int SAT_Cache_Valid;          // 64 through 128, depending on the number of 32-pixel-wide sprites.
        SAT_Cache_t SAT_Cache[128];     //64];

	uint16 SAT[0x100];

        uint16 VRAM[65536];	//VRAM_Size];
        uint64 bg_tile_cache[65536][8]; 	// Tile, y, x
        uint8 spr_tile_cache[1024][16][16];	// Tile, y, x
        uint8 spr_tile_clean[1024];     //VRAM_Size / 64];
} vdc_t;

extern vdc_t *vdc_chips[2];
extern int VDC_TotalChips;


void VDC_SetPixelFormat(const MDFN_PixelFormat &format);
void VDC_RunFrame(MDFN_Surface *surface, MDFN_Rect *DisplayRect, MDFN_Rect *LineWidths, int skip);
bool VDC_ToggleLayer(int which);

DECLFW(VDC_Write);
DECLFW(VDC_Write_ST);

DECLFR(VCE_Read);

static INLINE uint8 VDC_Read(unsigned int A, bool SGX)
{
 uint8 ret = 0;
 int msb = A & 1;
 int chip = 0;
 vdc_t *vdc;

 if(SGX)
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

            vdc->status &= ~0x3F;

            if(SGX)
            {
             if(!(vdc_chips[0]->status & 0x3F) && !(vdc_chips[1]->status & 0x3F))
              HuC6280_IRQEnd(MDFN_IQIRQ1);
            }
            else
              HuC6280_IRQEnd(MDFN_IQIRQ1); // Clear VDC IRQ line

            break;

  case 0x2:
  case 0x3:
           ret = REGGETP(vdc->read_buffer, msb);
           if(vdc->select == 0x2) // VRR - VRAM Read Register
           {
            if(msb)
            {
             vdc->MARR += vram_inc_tab[(vdc->CR >> 11) & 0x3];

             if(vdc->MARR >= VRAM_Size)
              VDC_UNDEFINED("Unmapped VRAM VRR read");

             vdc->read_buffer = vdc->VRAM[vdc->MARR & VRAM_SizeMask];
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

DECLFW(VCE_Write);

void VDC_Init(int sgx);
void VDC_Close(void);
void VDC_Reset(void);
void VDC_Power(void);

int VDC_StateAction(StateMem *sm, int load, int data_only);

};

#endif
