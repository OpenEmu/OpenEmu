/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
 *  Copyright (C) 2003 Xodnizel
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

#include        "../nes.h"
#include        "../x6502.h"
#include	"../nsf.h"
#include        "../sound.h"

#include        "../cart.h"
#include	"ppu.h"
#include        "palette.h"
#include        "../input.h"  
#include	"../ntsc/nes_ntsc.h"

#define VBlankON        (PPU[0]&0x80)   /* Generate VBlank NMI */
#define Sprite16        (PPU[0]&0x20)   /* Sprites 8x16/8x8        */
#define BGAdrHI         (PPU[0]&0x10)   /* BG pattern adr $0000/$1000 */
#define SpAdrHI         (PPU[0]&0x08)   /* Sprite pattern adr $0000/$1000 */
#define INC32           (PPU[0]&0x04)   /* auto increment 1/32  */

#define SpriteON        (PPU[1]&0x10)   /* Show Sprite             */
#define ScreenON        (PPU[1]&0x08)   /* Show screen             */

#define PPU_status      (PPU[2])

static nes_ntsc_emph_t *NTSCBlitter = NULL;
static nes_ntsc_setup_t setup;

static void FetchSpriteData(void);
static void RefreshLine(int lastpixel);
static void RefreshSprites(void);

static void Fixit1(void);
static uint32 ppulut1[256], ppulut2[256], ppulut3[128];

static void DoGfxDecode(void);
static MDFN_Surface *GfxDecode_Buf = NULL;
static int GfxDecode_Line = -1;
static int GfxDecode_Layer = 0;
static int GfxDecode_Scroll = 0;
static int GfxDecode_Pbn = 0;

static void makeppulut(void)
{
 int x;
 int y;

 for(x=0;x<256;x++)
 {
  ppulut1[x]=0;
  for(y=0;y<8;y++) 
  {
   ppulut1[x] |= ((x>>(7 - y))&0x1) << (y*4);
  }
  ppulut2[x]=ppulut1[x]<<1;
 }

 {

  int cc,xo,pixel;

  for(cc=0;cc<16;cc++)
  {
   for(xo=0;xo<8;xo++)
   {
    ppulut3[xo|(cc<<3)]=0;
    for(pixel=0;pixel<8;pixel++)
    {
     int shiftr;
      shiftr=(pixel+xo)/8;
      shiftr*=2;
      ppulut3[xo|(cc<<3)]|=(( cc>>shiftr )&3)<<(2+pixel*4);
    }
//    printf("%08x\n",ppulut3[xo|(cc<<3)]);
   }
  }

 }
} 
  
static int ppudead;
static int kook;
int fceuindbg=0;

int MMC5Hack;
uint32 MMC5HackCHRBank;
uint32 MMC5HackVROMMask;
uint8 *MMC5HackExNTARAMPtr;
uint8 *MMC5HackVROMPTR;
uint8 MMC5HackCHRMode=0;
uint8 MMC5HackSPMode;   
uint8 MMC5HackSPScroll; 
uint8 MMC5HackSPPage;   


uint8 VRAMBuffer,PPUGenLatch;
uint8 *vnapage[4];
uint8 PPUNTARAM;  
uint8 PPUCHRRAM;  

void (*GameHBIRQHook)(void), (*GameHBIRQHook2)(void);
void (*PPU_hook)(uint32 A);

static uint8 vtoggle;
static uint8 XOffset;
static uint32 TempAddr, RefreshAddr;
  
static int maxsprites; 
    
/* scanline is equal to the current visible scanline we're on. */
     
int scanline;
static uint32 scanlines_per_frame;
    
#define V_FLIP  0x80
#define H_FLIP  0x40
#define SP_BACK 0x20

typedef struct {
        uint8 y,no,atr,x;
} SPR;

typedef struct {
        uint8 ca[2],atr,x;
} SPRB;

uint8 PPU[4];
uint8 PPUSPL;
uint8 NTARAM[0x800],PALRAM[0x20],PALRAMCache[0x20];

uint8 SPRAM[0x100];
static SPRB SPRBUF[64];	// 8] if we didn't have an excess sprites option.

static uint32 PALRAMLUTCache[0x200];

void MDFNNES_SetPixelFormat(const MDFN_PixelFormat &pixel_format)
{
 for(int x = 0; x < 0x200; x++)
 {
  int r = ActiveNESPalette[x].r;
  int g = ActiveNESPalette[x].g;
  int b = ActiveNESPalette[x].b;

  PALRAMLUTCache[x] = pixel_format.MakeColor(r, g, b);
 }
}

#define MMC5SPRVRAMADR(V)      &MMC5SPRVPage[(V)>>10][(V)]
#define MMC5BGVRAMADR(V)      &MMC5BGVPage[(V)>>10][(V)]  
#define VRAMADR(V)      &VPage[(V)>>10][(V)]


static int32 sphitx;
static uint8 sphitdata;

static DECLFR(A2002)
{
	uint8 ret;
        
	if(!fceuindbg)
 	 if(sphitx != 0x100)
 	  MDFNPPU_LineUpdate();
	ret = PPU_status;
        ret|=PPUGenLatch&0x1F;

	if(!fceuindbg)
	{
	 vtoggle = 0;
	 PPU_status &= 0x7F;
	 PPUGenLatch = ret;
	}
	return ret;
}

static DECLFR(A200x)	/* Not correct for $2004 reads. */
{
	MDFNPPU_LineUpdate();
	return PPUGenLatch;
}

#define GETLASTPIXEL    (PAL?((timestamp*48-linestartts)/15) : ((timestamp*48-linestartts)>>4) )

static uint8 *Pline;
static int firstpixel;
static int linestartts;

static DECLFR(A2004)
{
        if(Pline) //InPPUActiveArea)
        {
         int poopix = GETLASTPIXEL;

         if(poopix > 320 && poopix < 340)
          return(0);
         return(0xFF);
        }
        else
        {
         uint8 ret = SPRAM[PPU[3]];
         return(ret);
        }
}

static DECLFR(A2007)
{
        uint8 ret;
        uint32 tmp = RefreshAddr & 0x3FFF;

        MDFNPPU_LineUpdate();
        ret = VRAMBuffer;

        if(!fceuindbg)
        {
         if(PPU_hook) PPU_hook(tmp);
         PPUGenLatch=VRAMBuffer;

         if(tmp<0x2000)
          VRAMBuffer=VPage[tmp>>10][tmp];
         else
         {
          VRAMBuffer=vnapage[(tmp>>10)&0x3][tmp&0x3FF];
          if(tmp >= 0x3f00)
          {
           tmp &= (tmp & 3)? 0x1F : 0x0C;
           ret = PALRAM[tmp];
          }
         }
        }

        if(!fceuindbg)
        {
         if (INC32) RefreshAddr+=32;
         else RefreshAddr++;
         if(PPU_hook) PPU_hook(RefreshAddr&0x3fff);
        }
        return ret;
}


static DECLFW(B2000)
{
                //printf("%04x:$%02x, %d\n",A,V&0x38,scanline);

		MDFNPPU_LineUpdate();
                PPUGenLatch=V;
                if(!(PPU[0]&0x80) && (V&0x80) && (PPU_status&0x80))
                {
                 //printf("Trigger NMI, %d, %d\n",timestamp,ppudead);
                 TriggerNMI2();
                }
                PPU[0]=V;
                TempAddr&=0xF3FF;
                TempAddr|=(V&3)<<10;
}

static int RCBGOn;
static int RCSPROn;
static int rendis;
static void RedoRenderCache(void)
{
	RCSPROn = SpriteON;

	RCBGOn = ScreenON;
}

static DECLFW(B2001)
{
		//printf("%04x:$%02x, %d\n",A,V,scanline);
		MDFNPPU_LineUpdate();
                PPUGenLatch=V;
		PPU[1]=V;
		RedoRenderCache();
}
 
static DECLFW(B2002)
{
	PPUGenLatch=V;
}

static DECLFW(B2003)
{
        PPUGenLatch=V;
        PPU[3]=V;
        PPUSPL=V&0xFC;
}

static DECLFW(B2004)
{
        PPUGenLatch=V;
        SPRAM[PPU[3]]=V;
        PPU[3]++;
}
 
static DECLFW(B2005)
{
	uint32 tmp = TempAddr;

	MDFNPPU_LineUpdate();
	PPUGenLatch = V;
	
	if(!vtoggle)
	{
	 tmp &= 0xFFE0;
	 tmp |= V>>3;
	 XOffset = V&7;
	}
	else
	{
	 tmp &= 0x8C1F;
	 tmp|=((V&~0x7)<<2);
         tmp|=(V&7)<<12;
        }
        TempAddr=tmp;
        vtoggle^=1;  
}

static uint8 poopoo(void)
{
 if((RefreshAddr & 0x3F00) == 0x3F00)
  return(RefreshAddr & 0xF);
 return(0);
}


static DECLFW(B2006)
{
		MDFNPPU_LineUpdate();
                PPUGenLatch=V;
                if(!vtoggle)  
                {
                 TempAddr&=0x00FF;
                 TempAddr|=(V&0x3f)<<8;
                }
                else
                {   
                 TempAddr&=0xFF00;
                 TempAddr|=V;

                 RefreshAddr=TempAddr;

                 if(PPU_hook)
                  PPU_hook(RefreshAddr);
		 //printf("%d, %04x\n",scanline,RefreshAddr);
                }
                vtoggle^=1;
}
 
static DECLFW(B2007)
{
                        uint32 tmp=RefreshAddr&0x3FFF;

			MDFNPPU_LineUpdate();

                        PPUGenLatch=V;
                        if(tmp >= 0x3F00)
                        {
                         tmp &= (tmp & 3)? 0x1F : 0x0C;
                         PALRAM[tmp] = PALRAMCache[tmp] = V & 0x3F;
                        }
                        else if(tmp<0x2000)
                        {
                          if(PPUCHRRAM&(1<<(tmp>>10)))
                            VPage[tmp>>10][tmp]=V;
                        }   
                        else
                        {
                         if(PPUNTARAM&(1<<((tmp&0xF00)>>10)))
                          vnapage[((tmp&0xF00)>>10)][tmp&0x3FF]=V;
                        }
                        if (INC32) RefreshAddr+=32;
                        else RefreshAddr++;
                        if(PPU_hook) PPU_hook(RefreshAddr&0x3fff);
}
 
static DECLFW(B4014)
{
        uint32 t=V<<8;
        int x;

        for(x=0;x<256;x++)
         X6502_DMW(0x2004,X6502_DMR(t+x));
}

static void ResetRL(uint8 *target)
{
 Pline=target; 
 firstpixel = 0;

 linestartts=timestamp*48+X.count;
 //printf("Line: %d\n", GETLASTPIXEL);
 //MDFNPPU_LineUpdate();
}

static MDFN_ALIGN(16) uint8 sprlinebuf[256+8];
static MDFN_ALIGN(16) uint8 emphlinebuf[256];

void MDFNPPU_LineUpdate(void)
{
 if(!fceuindbg)
  if(Pline)
  {
   int l=GETLASTPIXEL;
   RefreshLine(l);
  }
} 
  
bool MDFNNES_ToggleLayer(int which) 
{
 rendis ^= 1 << which;

 RedoRenderCache();

 return(((rendis >> which) & 1) ^ 1);
}

static void EndRL(void)
{
 RefreshLine(341);
 Pline=0;
}
 

static uint8 NT_TMP = 0;
static uint8 MMC5NT_TMP;
static uint32 pshift[2];
static uint32 atlatch;
static uint8 xs, ys;
static int tochange;

static INLINE void FetchNT(int MMC5Ex)
{
	uint8 *C;

	if(PPU_hook)
	 PPU_hook((RefreshAddr & 0xfff) | 0x2000);

        C = vnapage[(RefreshAddr>>10)&3];

	NT_TMP = C[RefreshAddr&0x3ff];   /* Fetch name table byte. */

	if(MMC5Ex == 1)
	 MMC5NT_TMP = MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff];
	else if((MMC5Ex == 3 || MMC5Ex == 4) && ((tochange<=0 && MMC5HackSPMode&0x40) || (tochange>0 && !(MMC5HackSPMode&0x40))) )
	{
	 NT_TMP = MMC5HackExNTARAMPtr[xs|(ys<<5)];
	// printf("OK: %d, %d\n",scanline,xs);
	}
}

static INLINE void FetchAT(int MMC5Ex)
{
	uint8 cc, zz;
	uint8 *C;
	
	C = vnapage[(RefreshAddr >> 10) & 3];
	zz = RefreshAddr & 0x1f;

	if(MMC5Ex == 1)
	 cc=(MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff] & 0xC0)>>6;
	else if((MMC5Ex == 3 || MMC5Ex == 4) && ((tochange<=0 && MMC5HackSPMode&0x40) || (tochange>0 && !(MMC5HackSPMode&0x40))) )
	{
         cc=MMC5HackExNTARAMPtr[0x3c0+(xs>>2)+((ys&0x1C)<<1)];
         cc=((cc >> ((xs&2) + ((ys&0x2)<<1))) &3);
	}
	else
	{
         cc=C[0x3c0+(zz>>2)+((RefreshAddr&0x380)>>4)];  /* Fetch attribute table byte. */
         cc=((cc >> ((zz&2) + ((RefreshAddr&0x40)>>4))) &3);
	}
        atlatch |= cc<<4;

	if((RefreshAddr&0x1f)==0x1f)
	 RefreshAddr^=0x41F;
	else
	 RefreshAddr++;

	if(PPU_hook)
	 PPU_hook(RefreshAddr & 0x3FFF);
}

static INLINE void FetchCD1(int MMC5Ex)
{
 uint32 vofs = ((PPU[0]&0x10)<<8) | ((RefreshAddr>>12)&7);
 uint32 vadr = (NT_TMP << 4) | vofs;
 uint8 *C = VRAMADR(vadr);

 if(MMC5Ex == 1)
 {
  C = MMC5HackVROMPTR;
  C += (((MMC5NT_TMP) & 0x3f & MMC5HackVROMMask) << 12) + (vadr & 0xfff);
 }
 else if(MMC5Ex == 2)
 {
  C = MMC5BGVRAMADR(vadr);
 }
 else if(MMC5Ex == 3 || MMC5Ex == 4)
 {
  if(((tochange<=0 && MMC5HackSPMode&0x40) || (tochange>0 && !(MMC5HackSPMode&0x40))) )
  {
   C = MMC5HackVROMPTR + ((NT_TMP << 4) | ((RefreshAddr >> 12)&7));
   C += ((MMC5HackSPPage & 0x3f & MMC5HackVROMMask) << 12);
  }
  else
  {
   C = MMC5BGVRAMADR(vadr);
  }
 }

 pshift[0] |= C[0];

 if(PPU_hook)
  PPU_hook(vadr);
}

static INLINE void FetchCD2(int MMC5Ex)
{
 uint32 vofs = ((PPU[0]&0x10)<<8) | ((RefreshAddr>>12)&7);
 uint32 vadr = (NT_TMP << 4) | vofs;
 uint8 *C = VRAMADR(vadr);

 if(MMC5Ex == 1)
 {
  C = MMC5HackVROMPTR;
  C += (((MMC5NT_TMP) & 0x3f & MMC5HackVROMMask) << 12) + (vadr & 0xfff);
 }
 else if(MMC5Ex == 2)
 {
  C = MMC5BGVRAMADR(vadr);
 }
 else if(MMC5Ex == 3 || MMC5Ex == 4)
 {
  if(((tochange<=0 && MMC5HackSPMode&0x40) || (tochange>0 && !(MMC5HackSPMode&0x40))) )
  {
   C = MMC5HackVROMPTR + ((NT_TMP << 4) | ((RefreshAddr >> 12)&7));
   C += ((MMC5HackSPPage & 0x3f & MMC5HackVROMMask) << 12);
  }
  else
  {
   C = MMC5BGVRAMADR(vadr);
  }
  //printf("%d, %d\n",scanline,xs);
  xs++;
  tochange--;
 }

 pshift[1] |= C[8];

 pshift[0] <<= 8;
 pshift[1] <<= 8;
 atlatch >>= 2;

 if(PPU_hook)
  PPU_hook(vadr | 8);
}

static int spork=0;     /* spork the world.  Any sprites on this line?
                           Then this will be set to 1.
                        */
                          
static INLINE void Fixit2(void)
{
        uint32 rad=RefreshAddr;
        rad&=0xFBE0;
        rad|=TempAddr&0x041f;
        RefreshAddr=rad;
}

static void RefreshLine(int lastpixel)
{
 static int norecurse = 0;
 int x;

 if(norecurse)
  return;
 norecurse = 1;

 if(ScreenON || SpriteON)
 {
  uint8 pix_mask = 0x3F;

  if(PPU[1]&0x01)
   pix_mask = 0x30;

  PALRAMCache[0x0]=PALRAMCache[0x4]=PALRAMCache[0x8]=PALRAMCache[0xC]=PALRAM[0] | 64;

  if(MMC5Hack && geniestage != 1)
  {
   if(MMC5HackCHRMode==0 && (MMC5HackSPMode&0x80))
   {
    const int MMC5Mode = 4;

    ys=((scanline>>3)+MMC5HackSPScroll)&0x1F;
    if(ys>=0x1E) ys-=0x1E;

    #include "ppu-subline.h"
   }
   else if (MMC5HackCHRMode==1 && (MMC5HackSPMode&0x80))
   {
    const int MMC5Mode = 3;

    ys=((scanline>>3)+MMC5HackSPScroll)&0x1F;
    if(ys>=0x1E) ys-=0x1E;

    #include "ppu-subline.h"
   }
   else if(MMC5HackCHRMode == 1)
   {
    const int MMC5Mode = 1;
    #include "ppu-subline.h"
   }
   else
   {
    const int MMC5Mode = 2;
    #include "ppu-subline.h"
   }
  }
  else
  {
   const int MMC5Mode = 0;
   #include "ppu-subline.h"
  }
 }
 else
 {
  int count = lastpixel - firstpixel;

  if((count + firstpixel) > 256) count = 256 - firstpixel;

  if(count > 0)
  {
   if(rendis & 1)
   {
    memset(emphlinebuf + firstpixel, 0, count);
    memset(Pline + firstpixel, 0x40 | 0x3C, count);
   }
   else
   {
    memset(emphlinebuf + firstpixel, PPU[1] >> 5, count);
    memset(Pline + firstpixel, PALRAM[poopoo()], count);
   }
  }
  //pshift[0] = pshift[1] = atlatch = 0;
 }

 if(InputScanlineHook)
  InputScanlineHook(Pline,0,firstpixel,lastpixel);
 firstpixel = lastpixel;

 norecurse = 0;
}

static void Fixit1(void)
{
    uint32 rad=RefreshAddr;

    if((rad&0x7000)==0x7000)
    {
     rad^=0x7000;
     if((rad&0x3E0)==0x3A0)
     {
      rad^=0x3A0;
      rad^=0x800;
     }
     else
     {
      if((rad&0x3E0)==0x3e0)
       rad^=0x3e0;
      else rad+=0x20;
     }
    }
    else
     rad+=0x1000;
    RefreshAddr=rad;
}

#include "ppu-fastrl.h"

// FIXME, use MakeColor() method
#define MEOW_OUT(n, a) { uint32 otmp; NES_NTSC_RAW_OUT(n, otmp); a = (((otmp >> 21) & 0xFF) << pixel_format.Rshift) | (((otmp >> 11) & 0xFF) << pixel_format.Gshift) | (((otmp >>  1) & 0xFF) << pixel_format.Bshift); }

static void nes_ntsc_blit(const MDFN_PixelFormat &pixel_format, nes_ntsc_emph_t const* ntsc, uint8 const* nes_in, uint8 const* nes_emph_in, long in_pitch,
                int burst_phase, int width, int height, uint32* rgb_out, long out_pitch )
{
        /* determine how many chunks to blit, less one for the final chunk */
        int chunk_count = width / nes_ntsc_out_chunk - 1;

                /* begin row and read first nes pixel */
                uint8 const* line_in = nes_in;
		uint8 const* line_emph_in = nes_emph_in;
                NES_NTSC_BEGIN_ROW( ntsc, burst_phase, nes_ntsc_black, nes_ntsc_black, (*line_in++ & 0x3F) | (*line_emph_in++ << 6));
                uint32* line_out = rgb_out;
                int n;

                /* blit main chunks, each using 3 nes pixels to generate 7 output pixels */
                for ( n = chunk_count; n; --n )
                {
                        /* order of NES_NTSC_COLOR_IN and NES_NTSC_RGB16_OUT must not be altered */

                        /* you can use NES_NTSC_RGB24_OUT and NES_NTSC_RGB15_OUT for other RGB
                        output formats, or NES_NTSC_RAW_OUT for the internal format */
                        NES_NTSC_COLOR_IN( 0, (line_in [0] & 0x3F) | (line_emph_in [0] << 6) );
                        MEOW_OUT( 0, line_out [0] );
                        MEOW_OUT( 1, line_out [1] );

                        NES_NTSC_COLOR_IN( 1, (line_in [1] & 0x3F) | (line_emph_in [1] << 6) );
                        MEOW_OUT( 2, line_out [2] );
                        MEOW_OUT( 3, line_out [3] );

                        NES_NTSC_COLOR_IN( 2, (line_in [2] & 0x3F) | (line_emph_in [2] << 6) );
                        MEOW_OUT( 4, line_out [4] );
                        MEOW_OUT( 5, line_out [5] );
                        MEOW_OUT( 6, line_out [6] );
                        line_in  += 3;
			line_emph_in += 3;
                        line_out += 7;
                }

                /* you can eliminate the need for the final chunk below by padding your nes
                input with three extra black pixels at the end of each row */

                /* finish final pixels without starting any new ones */
                NES_NTSC_COLOR_IN( 0, nes_ntsc_black );
                MEOW_OUT( 0, line_out [0] );
                MEOW_OUT( 1, line_out [1] );

                NES_NTSC_COLOR_IN( 1, nes_ntsc_black );
                MEOW_OUT( 2, line_out [2] );
                MEOW_OUT( 3, line_out [3] );

                NES_NTSC_COLOR_IN( 2, nes_ntsc_black );
                MEOW_OUT( 4, line_out [4] );
                MEOW_OUT( 5, line_out [5] );
                MEOW_OUT( 6, line_out [6] );
}

static int BurstPhase = 0;

void MMC5_hb(int);     /* Ugh ugh ugh. */
static void DoLine(MDFN_Surface *surface, int skip)
{
 uint8 target[256];

 if(InputScanlineHook) // Frame skipping will break zapper emulation soooo muchlybadlydoubleplusungoodly.
  skip = 0;

 if(RCSPROn) RefreshSprites();

 ResetRL(target);

 if(scanline >= 0 && MMC5Hack && (ScreenON || SpriteON)) MMC5_hb(scanline);

 X6502_Run(256);
 if(firstpixel < 240)
 {
  int newfirst = firstpixel;

  if(newfirst & 7)
  {
   newfirst = (newfirst + 7) &~7;
   //printf("%d\n",newfirst);
   RefreshLine(newfirst);
  }

  Pline=0;	// We don't want any PPU_hook()-calling-RefreshLine()-business going on!

  FastRefreshLine(newfirst >> 3, target);

  if(RCSPROn && spork)
   FastCopySprites(newfirst >> 3, target, skip);
  else if(rendis & 1)
   MDFN_FastU32MemsetM8((uint32 *)(target + newfirst), 0x40404040 | 0x3C3C3C3C, (256 - newfirst) / sizeof(uint32));

  if(!skip)
   FastLineEffects(newfirst >> 3, target);

  if(InputScanlineHook)
   InputScanlineHook(target, 0, newfirst, 256);

  firstpixel = 256;
  if(ScreenON || SpriteON)
   Fixit1();

  Pline = target; // Restore it!
 }
 else
  MDFNPPU_LineUpdate();

 if(scanline >= 0)
 {
  uint32 *real_target = NULL;
  uint16 *real_target16 = NULL;

  if(surface->pixels)
   real_target = surface->pixels + scanline * surface->pitchinpix;

  if(surface->pixels16)
   real_target16 = surface->pixels16 + scanline * surface->pitchinpix;

  if(NTSCBlitter)
  {
   if(!skip)
   {
    // TODO:  Factor this out/make it more elegant.
    if(surface->format.colorspace != MDFN_COLORSPACE_RGB || surface->format.bpp != 32)
    {
     MDFN_PixelFormat nf;

     memset(&nf, 0, sizeof(nf));

     nf.bpp = 32;
     nf.colorspace = MDFN_COLORSPACE_RGB;
     
     nf.Rshift = 0;
     nf.Gshift = 8;
     nf.Bshift = 16;
     nf.Ashift = 24;

     surface->SetFormat(nf, false);
    }
    nes_ntsc_blit(surface->format, NTSCBlitter, target, emphlinebuf, nes_ntsc_min_in_width, setup.merge_fields ? scanline % 3 : BurstPhase, nes_ntsc_min_out_width, 1, real_target, surface->pitch32 * sizeof(uint32));
   }
   BurstPhase = (BurstPhase + 1) % 3;
  }
  else
  {
   if(!skip) 
   {
    if(real_target16)
     for(int x = 0; x < 256; x++)
      real_target16[x] = PALRAMLUTCache[(target[x] & 0x3F) | (emphlinebuf[x] << 6)];
    else
     for(int x = 0; x < 256; x++)
      real_target[x] = PALRAMLUTCache[(target[x] & 0x3F) | (emphlinebuf[x] << 6)];
   }
  }
 }
 sphitx=0x100;

 if(ScreenON || SpriteON)
  FetchSpriteData();

 if(GameHBIRQHook && (ScreenON || SpriteON) && ((PPU[0]&0x38)!=0x18))
 {
  X6502_Run(10);
  GameHBIRQHook();
  X6502_Run(85-16-10);
 }
 else
 {
  X6502_Run(85-16);

  // A semi-hack for Star Trek: 25th Anniversary
  if(GameHBIRQHook && (ScreenON || SpriteON) && ((PPU[0]&0x38)!=0x18))
   GameHBIRQHook();
 }

 if(GameHBIRQHook2 && (ScreenON || SpriteON))
  GameHBIRQHook2();

 if(scanline == -1 && !PAL)
 {
  kook ^= 1;
  if(ScreenON && kook)
  {
   X6502_Run(15);
   BurstPhase = (BurstPhase + 2) % 3;
  }
  else
  {
   X6502_Run(16);
   BurstPhase = (BurstPhase + 1) % 3;
  }
 }
 else
  X6502_Run(16);

 EndRL();


 scanline++;
}

static uint8 numsprites,SpriteBlurp;

static void FetchSpriteData(void)
{
        uint8 ns,sb;
        uint8 H;
        int vofs;
        uint8 P0=PPU[0];

        H=8;

        ns=sb=0;

        vofs=(unsigned int)(P0&0x8&(((P0&0x20)^0x20)>>2))<<9;
        H+=(P0&0x20)>>2;

        for(int n = 0;n < 64;n++)
        {
                SPR *spr = (SPR *)&SPRAM[n * 4];

                if(n < 2)
                {
                 spr = (SPR *)&SPRAM[(n * 4 + PPUSPL) & 0xFF];
                }

                if((unsigned int)(scanline-spr->y)>=H) continue;

                if(ns<maxsprites)
                {
                 SPRB *dst = &SPRBUF[ns];

                 if(n == 0)
		  sb=1;

                  uint8 *C;
                  int t;
                  unsigned int vadr;

                  t = (int)scanline-(spr->y);

                  if (Sprite16)
                   vadr = ((spr->no&1)<<12) + ((spr->no&0xFE)<<4);
                  else
                   vadr = (spr->no<<4)+vofs;

                  if (spr->atr&V_FLIP)
                  {
                        vadr+=7;
                        vadr-=t;
                        vadr+=(P0&0x20)>>1;
                        vadr-=t&8;
                  }
                  else
                  {
                        vadr+=t;
                        vadr+=t&8;
                  }

                  if(MMC5Hack && geniestage!=1 && Sprite16) C = MMC5SPRVRAMADR(vadr);
                  else C = VRAMADR(vadr);
                  dst->ca[0]=C[0];

		  if(PPU_hook && ns < 8)
		  {
                   PPU_hook(0x2000);
                   PPU_hook(vadr);
                  }

                  dst->ca[1]=C[8];

                  if(PPU_hook && ns<8)
                   PPU_hook(vadr | 8);

                  dst->x=spr->x;
                  dst->atr=spr->atr;

                 ns++;
                }
                else
                {
                  PPU_status|=0x20;
                  break;
                }
         }
        //if(ns>=7)
        //printf("%d %d\n",scanline,ns);
        if(ns>8) PPU_status|=0x20;      /* Handle case when >8 sprites per
                                           scanline option is enabled. */
        else if(PPU_hook)
        {
         for(int n = 0; n < (8-ns); n++)
         {
                 PPU_hook(0x2000);
                 PPU_hook(vofs);
         }
        }
        numsprites=ns;
        SpriteBlurp=sb;
}



static void RefreshSprites(void)
{
        spork = 0;

        MDFN_FastU32MemsetM8((uint32 *)sprlinebuf, 0x80808080, 256 / sizeof(uint32));

        if(!numsprites)
	 return;

	for(int n = numsprites - 1; n >= 0; n--)
	{
	 register uint32 pixdata;
	 register uint8 J,atr;
	 register SPRB *spr = &SPRBUF[n];

	 int x = spr->x;
         uint8 *C;
         uint8 *VB;
                
         pixdata=ppulut1[spr->ca[0]]|ppulut2[spr->ca[1]];
         J=spr->ca[0]|spr->ca[1];
         atr=spr->atr;

                       if(J)
                       {
                        if(n==0 && SpriteBlurp && !(PPU_status&0x40))
                        {
                         sphitx=x;
                         sphitdata=J;
                         if(atr&H_FLIP)
                          sphitdata=    ((J<<7)&0x80) |
                                        ((J<<5)&0x40) |
                                        ((J<<3)&0x20) |
                                        ((J<<1)&0x10) |
                                        ((J>>1)&0x08) |
                                        ((J>>3)&0x04) |
                                        ((J>>5)&0x02) |
                                        ((J>>7)&0x01);                                          
                        }

         C = sprlinebuf+x;
         VB = (PALRAM+0x10)+((atr&3)<<2);
	 uint8 pbit = (atr & SP_BACK) ? 0x00 : 0x40;

         if (atr&H_FLIP)
         {
	   if(J & 0x80) C[7]=VB[pixdata&3] | pbit;
           pixdata>>=4;
	   if(J & 0x40) C[6]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x20) C[5]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x10) C[4]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x08) C[3]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x04) C[2]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x02) C[1]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x01) C[0]=VB[pixdata] | pbit;
         } else  {
	   if(J & 0x80) C[0]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x40) C[1]=VB[pixdata&3] | pbit;
           pixdata>>=4;
	   if(J & 0x20) C[2]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x10) C[3]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x08) C[4]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x04) C[5]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x02) C[6]=VB[pixdata&3] | pbit;
           pixdata>>=4;
           if(J & 0x01) C[7]=VB[pixdata] | pbit;
         }
        }
      }
     SpriteBlurp=0;
     spork=1;

     if(rendis & 2) 
      MDFN_FastU32MemsetM8((uint32 *)sprlinebuf, 0x80808080, 256 / sizeof(uint32));
}

void MDFNPPU_Reset(void)
{
        VRAMBuffer=PPU[0]=PPU[1]=PPU_status=PPU[3]=0;
        PPUSPL=0;
        PPUGenLatch=0;
        RefreshAddr=TempAddr=0;
	XOffset = 0;
        vtoggle = 0;
        ppudead = 1;
	kook = 0;
	RedoRenderCache();
}

void MDFNPPU_Power(void)
{
        memset(NTARAM, 0x00, 0x800);
        memset(PALRAM, 0x00, 0x20); 
        memset(SPRAM, 0x00, 0x100); 
        MDFNPPU_Reset();
}


int MDFNPPU_Loop(MDFN_Surface *surface, int skip)
{
  if(ppudead) /* Needed for Knight Rider, Time Lord, possibly others. */
  {
   if(!skip)
    surface->Fill(0, 0, 0, 0);

   X6502_Run(27384 - (256 + 85));
   ppudead = 0;
  }
  else
  {
   X6502_Run(256+85);
   PPU_status |= 0x80;
   PPU[3]=PPUSPL=0;             /* Not sure if this is correct.  According
                                  to Matt Conte and my own tests, it is.  Timing is probably
                                  off, though.  NOTE:  Not having this here
                                  breaks a Super Donkey Kong game. */
                                /* I need to figure out the true nature and length
                                   of this delay. 
                                */
   X6502_Run(12);

   if(MDFNGameInfo->GameType == GMT_PLAYER)
    DoNSFFrame();
   else
   {
    if(VBlankON)
     TriggerNMI();
   }
   X6502_Run((scanlines_per_frame-242)*(256+85)-12);
   PPU_status&=0x1F;
   scanline = -1;
   DoLine(surface, skip);

   /* Clean this stuff up later. */
   spork=numsprites=0;

   if(MDFNGameInfo->GameType == GMT_PLAYER)
    X6502_Run((256+85)*240);
   else
   {
    for(scanline=0;scanline<240;)       //scanline is incremented in  DoLine.  Evil. :/
    {
     if(GfxDecode_Buf && scanline == GfxDecode_Line)
      DoGfxDecode();
     DoLine(surface, skip);
    }
    if(MMC5Hack && (ScreenON || SpriteON)) MMC5_hb(scanline);
   }
  } /* else... to if(ppudead) */

  if(skip)
   return(0);
  else
   return(1);
}

uint32 NESPPU_GetRegister(const std::string &name, std::string *special)
{
 if(name == "PPU0")
 {
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "VBlank NMI: %s, Sprite size: 8x%d, BG CHR: 0x%04x, SPR CHR: 0x%04x, VRAM Addr Increment: %d", (PPU[0] & 0x80) ? "On" : "Off",
	(PPU[0] & 0x20) ? 16 : 8, (PPU[0] & 0x10) ? 0x1000 : 0x0000, (PPU[0] & 0x08) ? 0x1000 : 0x0000,
	(PPU[0] & 0x04) ? 32 : 1);
   *special = std::string(buf);
  }
  return(PPU[0]);
 }
 else if(name == "PPU1")
 {
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "Sprites: %s, Background: %s, Leftmost 8 SPR Pixels: %s, Leftmost 8 BG Pixels: %s",
	(PPU[1] & 0x10) ? "On" : "Off", (PPU[1] & 0x08) ? "On" : "Off",
	(PPU[1] & 0x04) ? "On" : "Off", (PPU[1] & 0x02) ? "On" : "Off");
   *special = std::string(buf);
  }
  return(PPU[1]);
 }
 else if(name == "PPU2")
  return(PPU[2]);
 else if(name == "PPU3")
  return(PPU[3]);
 else if(name == "XOffset")
  return(XOffset);
 else if(name == "RAddr")
  return(RefreshAddr);
 else if(name == "TAddr")
  return(TempAddr);
 else if(name == "VRAM Buf")
  return(VRAMBuffer);
 else if(name == "V-Toggle")
  return(vtoggle);
 else if(name == "Scanline")
 {
  if(scanline == -1) return(261);
  else return(scanline);
 }
 else return(0);
}

void NESPPU_SetRegister(const std::string &name, uint32 value)
{
 if(name == "PPU0")
 {
  PPU[0] = value & 0xFF;
 }
 else if(name == "PPU1")
 {
  PPU[1] = value & 0xFF;
  RedoRenderCache();
 }
 else if(name == "PPU2")
 {
  PPU[2] = value & 0xE0;
 }
 else if(name == "PPU3")
 {
  PPU[3] = value & 0xFF;
 }
 else if(name == "XOffset")
 {
  XOffset = value & 0x07;
 }
 else if(name == "RAddr")
 {
  RefreshAddr = value & 0x3FFF;
 }
 else if(name == "TAddr")
 {
  TempAddr = value & 0x3FFF;
 }
 else if(name == "VRAM Buf")
  VRAMBuffer = value & 0xFF;
 else if(name == "V-Toggle")
  vtoggle = value ? 1 : 0;
}


int MDFNPPU_StateAction(StateMem *sm, int load, int data_only)
{
 uint16 TempAddrT,RefreshAddrT;

 SFORMAT PPU_STATEINFO[]=
 {
  SFARRAYN(NTARAM, 0x800, "NTAR"),
  SFARRAYN(PALRAM, 0x20, "PRAM"),
  SFARRAYN(SPRAM, 0x100, "SPRA"),
  SFARRAYN(PPU, 0x4, "PPUR"),
  SFVARN(BurstPhase, "BurstPhase"),
  SFVARN(kook, "KOOK"),
  SFVARN(ppudead, "DEAD"),
  SFVARN(PPUSPL, "PSPL"),
  SFVARN(XOffset, "XOFF"),
  SFVARN(vtoggle, "VTOG"),
  SFVARN(RefreshAddrT, "RADD"),
  SFVARN(TempAddrT, "TADD"),
  SFVARN(VRAMBuffer, "VBUF"),
  SFVARN(PPUGenLatch, "PGEN"),
  SFEND
 };

 if(!load)
 {
  TempAddrT=TempAddr;
  RefreshAddrT=RefreshAddr;
 }

 std::vector <SSDescriptor> love;
 love.push_back(SSDescriptor(PPU_STATEINFO, "PPU"));
 int ret = MDFNSS_StateAction(sm, load, data_only, love);

 if(load)
 {
  int x;

  TempAddr=TempAddrT;
  RefreshAddr=RefreshAddrT;

  for(x=0;x<0x20;x++)
  {
   PALRAM[x] &= 0x3F;
   PALRAMCache[x] = PALRAM[x];
  }
  RedoRenderCache();
 }
 return(ret);
}

static MDFN_Rect PPUDisplayRect;

void NESPPU_GetDisplayRect(MDFN_Rect *DisplayRect)
{
 memcpy(DisplayRect, &PPUDisplayRect, sizeof(MDFN_Rect));
}

static void RedoRL(void)
{
 PPUDisplayRect.x = NTSCBlitter ? 4 : 0;
 PPUDisplayRect.w = NTSCBlitter ? 602 - 4 - 2: 256;

 if(PAL)
 {
  PPUDisplayRect.y = MDFN_GetSettingUI("nes.slstartp");
  PPUDisplayRect.h = MDFN_GetSettingUI("nes.slendp") - PPUDisplayRect.y + 1;
 }
 else
 {
  PPUDisplayRect.y = MDFN_GetSettingUI("nes.slstart");
  PPUDisplayRect.h = MDFN_GetSettingUI("nes.slend") - PPUDisplayRect.y + 1;
 }

 if(MDFN_GetSettingUI("nes.clipsides"))
 {
  if(NTSCBlitter)
  {
   PPUDisplayRect.x += 18;
   PPUDisplayRect.w -= 36;
  }
  else
  {
   PPUDisplayRect.x += 8;
   PPUDisplayRect.w -= 16;
  }
 }

 MDFNGameInfo->nominal_width = NTSCBlitter ? PPUDisplayRect.w / 2 : PPUDisplayRect.w;
 MDFNGameInfo->nominal_height = PPUDisplayRect.h;

 MDFNGameInfo->lcm_width = PPUDisplayRect.w;
 MDFNGameInfo->lcm_height = MDFNGameInfo->nominal_height;

 MDFNGameInfo->fb_width = (NTSCBlitter ? 768 : 256);
}

void MDFNPPU_Init(void) 
{
 makeppulut();
 rendis = 0;
 maxsprites = MDFN_GetSettingB("nes.no8lim") ? 64 : 8;

 if(MDFN_GetSettingB("nes.ntscblitter") && !PAL && !NESIsVSUni && MDFNGameInfo->GameType != GMT_PLAYER)
 {
  static float matrix[6];

  memset(&setup, 0, sizeof(setup));

  if(MDFN_GetSettingB("nes.ntsc.matrix"))
  {
   matrix[0] = MDFN_GetSettingF("nes.ntsc.matrix.0");
   matrix[1] = MDFN_GetSettingF("nes.ntsc.matrix.1");
   matrix[2] = MDFN_GetSettingF("nes.ntsc.matrix.2");
   matrix[3] = MDFN_GetSettingF("nes.ntsc.matrix.3");
   matrix[4] = MDFN_GetSettingF("nes.ntsc.matrix.4");
   matrix[5] = MDFN_GetSettingF("nes.ntsc.matrix.5");

   setup.decoder_matrix = matrix;
  }

  setup.merge_fields = MDFN_GetSettingB("nes.ntsc.mergefields");
  setup.hue = MDFN_GetSettingF("nes.ntsc.hue");
  setup.saturation = MDFN_GetSettingF("nes.ntsc.saturation");
  setup.sharpness = MDFN_GetSettingF("nes.ntsc.sharpness");
  setup.brightness = MDFN_GetSettingF("nes.ntsc.brightness");
  setup.contrast = MDFN_GetSettingF("nes.ntsc.contrast");

  std::string preset = MDFN_GetSettingS("nes.ntsc.preset");

  if(preset == "composite")
   setup = nes_ntsc_composite;
  else if(preset == "svideo")
   setup = nes_ntsc_svideo;
  else if(preset == "rgb")
   setup = nes_ntsc_rgb;
  else if(preset == "monochrome")
   setup = nes_ntsc_monochrome;

  NTSCBlitter = (nes_ntsc_emph_t *)calloc(1, sizeof(nes_ntsc_emph_t));
  nes_ntsc_init_emph(NTSCBlitter, &setup);  
  
 }
 else
  NTSCBlitter = NULL;

 if(PAL)
  scanlines_per_frame=312;
 else
  scanlines_per_frame=262;

 if(MDFNGameInfo->GameType != GMT_PLAYER)
  RedoRL();


 for(int x = 0x2000;x < 0x4000; x += 8)
 {
         SetReadHandler(x, x, A200x);
         BWrite[x] = B2000;

         SetReadHandler(x+1, x+1, A200x);
         BWrite[x+1] = B2001;

         SetReadHandler(x+2, x+2, A2002);
         BWrite[x+2] = B2002;

         SetReadHandler(x+3, x+3, A200x);
         BWrite[x+3] = B2003;

         SetReadHandler(x+4, x+4, A2004);
         BWrite[x+4] = B2004;

         SetReadHandler(x+5, x+5, A200x);
         BWrite[x+5] = B2005;

         SetReadHandler(x+6, x+6, A200x);
         BWrite[x+6] = B2006;

         SetReadHandler(x+7, x+7, A2007);
         BWrite[x+7] = B2007;
 }
 BWrite[0x4014] = B4014;

}

void NESPPU_SettingChanged(const char *name)
{
 if(!strcmp(name, "nes.no8lim"))
 {
  maxsprites = MDFN_GetSettingB("nes.no8lim") ? 64 : 8;
 }
}

void NESPPU_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "ppu"))
 {
  while(Length--)
  {
   Address &= 0x3FFF;

   if(Address < 0x2000)
    *Buffer = VPage[Address >> 10][Address];
   else if(Address >= 0x3f00)
    *Buffer = PALRAM[Address & ((Address & 3)? 0x1F : 0x0C)];
   else
    *Buffer = vnapage[(Address >> 10) & 0x3][Address & 0x3FF];

   Address++;
   Buffer++;
  }
 }
}

void NESPPU_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "ppu"))
 {
  while(Length--)
  {
   Address &= 0x3FFF;

   if(Address < 0x2000)
    VPage[Address >> 10][Address] = *Buffer;
   else if(Address >= 0x3f00)
   {
    uint32 tmp = Address & ((Address & 3)? 0x1F : 0x0C);
    PALRAM[tmp] = PALRAMCache[tmp] = *Buffer;
   }
   else
    vnapage[(Address >> 10) & 0x3][Address & 0x3FF] = *Buffer;

   Address++;
   Buffer++;
  }
 }
}


void NESPPU_SetGraphicsDecode(MDFN_Surface *surface, int line, int which, int xscroll, int yscroll, int pbn)
{
 GfxDecode_Buf = surface;
 GfxDecode_Line = line;
 GfxDecode_Layer = which;
 GfxDecode_Scroll = yscroll;
 GfxDecode_Pbn = pbn;

 if(GfxDecode_Line == -1)
  DoGfxDecode();
}

static void DoGfxDecode(void)
{
 uint32 *target = GfxDecode_Buf->pixels;
 int pbn = GfxDecode_Pbn & 0x3;
 uint32 neo_palette[4];

 if(GfxDecode_Pbn == -1)
 {
  for(int x = 0; x <  4; x++)
   neo_palette[x] = GfxDecode_Buf->format.MakeColor(x * 85, x * 85, x * 85, 0xFF);
 }
 else
  for(int x = 0; x < 4; x++)
   neo_palette[x] = PALRAMLUTCache[PALRAMCache[pbn * 4 + x] & 0x3F] | GfxDecode_Buf->format.MakeColor(0, 0, 0, 0xFF);

  for(int y = 0; y < GfxDecode_Buf->h; y++)
  {
   for(int x = 0; x < GfxDecode_Buf->w; x+=8)
   {
    int which_tile = (x / 8) + (GfxDecode_Scroll + (y / 8)) * (GfxDecode_Buf->w / 8);

    uint8 *cg_ptr;
    uint8 cg[2];
    int tile_c = 0;

    if(MMC5Hack)
    {
     if(GfxDecode_Layer) // Sprites
     {
      //which_tile &= 0x1FF;
      cg_ptr = MMC5SPRVRAMADR(which_tile * 16);
     }
     else
     {
      switch(MMC5HackCHRMode)
      {
       default: //which_tile &= 0x1FF;
                cg_ptr = MMC5BGVRAMADR(which_tile * 16);
		break;
      }
     }
    }
    else
    {
     if(GfxDecode_Layer) // Sprites
     {
      pbn |= 0x4;
      if(Sprite16)
      {
       tile_c = 0x200;
       cg_ptr = VRAMADR(which_tile * 16);
      }
      else
      {
       tile_c = 0x100;

       if(SpAdrHI)
        cg_ptr = VRAMADR(0x1000 + which_tile * 16);
       else
        cg_ptr = VRAMADR(0x0000 + which_tile * 16);
      }
     }
     else // Background
     {
      tile_c = 0x100;
      if(BGAdrHI)
       cg_ptr = VRAMADR(0x1000 + which_tile * 16);
      else
       cg_ptr = VRAMADR(0x0000 + which_tile * 16);
     }
    }

    if(which_tile >= tile_c)
    {
     for(int sx = 0; sx < 8; sx++) target[x + sx] = GfxDecode_Buf->format.MakeColor(0, 0, 0, 0);
     continue;
    }

    cg[0] = cg_ptr[0 + (y & 0x7)];
    cg[1] = cg_ptr[8 + (y & 0x7)];

    for(int sx = 0; sx < 8; sx++)
     target[x + sx] = neo_palette[((cg[0] >> (7-sx)) & 0x1) | (((cg[1] >> (7-sx)) & 0x1) << 1)];

    target[x + GfxDecode_Buf->w*2 + 0] = target[x + GfxDecode_Buf->w*2 + 1] = target[x + GfxDecode_Buf->w*2 + 2] = target[x + GfxDecode_Buf->w*2 + 3] =
    target[x + GfxDecode_Buf->w*2 + 4] = target[x + GfxDecode_Buf->w*2 + 5] = target[x + GfxDecode_Buf->w*2 + 6] = target[x + GfxDecode_Buf->w*2 + 7] = which_tile * 16;

    target[x + GfxDecode_Buf->w*1 + 0]=target[x + GfxDecode_Buf->w*1 + 1]=target[x + GfxDecode_Buf->w*1 + 2]=target[x + GfxDecode_Buf->w*1 + 3] =
    target[x + GfxDecode_Buf->w*1 + 4]=target[x + GfxDecode_Buf->w*1 + 5]=target[x + GfxDecode_Buf->w*1 + 6]=target[x + GfxDecode_Buf->w*1 + 7] = which_tile;
   }
   target += GfxDecode_Buf->w * 3;
  }



}
