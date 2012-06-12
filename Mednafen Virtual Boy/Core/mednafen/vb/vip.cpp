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

#include "vb.h"
#include "vip.h"
#include <math.h>

#define VIP_DBGMSG(format, ...) { }
//#define VIP_DBGMSG(format, ...) printf(format "\n", ## __VA_ARGS__)

namespace MDFN_IEN_VB
{

static uint8 FB[2][2][0x6000];
static uint16 CHR_RAM[0x8000 / sizeof(uint16)];
static uint16 DRAM[0x20000 / sizeof(uint16)];

// Helper functions for the V810 VIP RAM read/write handlers.
//  "Memory Array 16 (Write/Read) (16/8)"
#define VIP__GETP16(array, address) ( (uint16 *)&((uint8 *)(array))[(address)] )

#ifdef MSB_FIRST
#define VIP__GETP8(array, address) ( &((uint8 *)(array))[(address) ^ 1] )
#else
#define VIP__GETP8(array, address) ( &((uint8 *)(array))[(address)] )
#endif

static INLINE void VIP_MA16W16(uint16 *array, const uint32 v810_address, const uint16 value)
{
 *(VIP__GETP16(array, v810_address)) = value;
}

static INLINE uint16 VIP_MA16R16(uint16 *array, const uint32 v810_address)
{
 return *(VIP__GETP16(array, v810_address));
}

static INLINE void VIP_MA16W8(uint16 *array, const uint32 v810_address, const uint8 value)
{
 *(VIP__GETP8(array, v810_address)) = value;
}

static INLINE uint8 VIP_MA16R8(uint16 *array, const uint32 v810_address)
{
 return *(VIP__GETP8(array, v810_address));
}


#define INT_SCAN_ERR	0x0001
#define INT_LFB_END	0x0002
#define INT_RFB_END	0x0004
#define INT_GAME_START	0x0008
#define INT_FRAME_START	0x0010

#define INT_SB_HIT	0x2000
#define INT_XP_END	0x4000
#define INT_TIME_ERR	0x8000

static uint16 InterruptPending;
static uint16 InterruptEnable;

static uint8 BRTA, BRTB, BRTC, REST;
static uint8 Repeat;

static void CopyFBColumnToTarget_Anaglyph(void) NO_INLINE;
static void CopyFBColumnToTarget_AnaglyphSlow(void) NO_INLINE;
static void CopyFBColumnToTarget_CScope(void) NO_INLINE;
static void CopyFBColumnToTarget_SideBySide(void) NO_INLINE;
static void CopyFBColumnToTarget_VLI(void) NO_INLINE;
static void CopyFBColumnToTarget_HLI(void) NO_INLINE;
static void (*CopyFBColumnToTarget)(void) = NULL;
static uint32 VB3DMode;
static uint32 VB3DReverse;
static uint32 VBPrescale;
static uint32 VBSBS_Separation;
static uint32 HLILUT[256];
static uint32 ColorLUT[2][256];
static int32 BrightnessCache[4];
static uint32 BrightCLUT[2][4];

static double ColorLUTNoGC[2][256][3];
static uint32 AnaSlowColorLUT[256][256];

// A few settings:
static bool InstantDisplayHack;
static bool AllowDrawSkip;

static bool VidSettingsDirty;
static bool ParallaxDisabled;
static uint32 Anaglyph_Colors[2];
static uint32 Default_Color;

static void MakeColorLUT(const MDFN_PixelFormat &format)
{
 for(int lr = 0; lr < 2; lr++)
 {
  for(int i = 0; i < 256; i++)
  {
   double r, g, b;
   double r_prime, g_prime, b_prime;

   r = g = b = (double)i / 255;

   // TODO: Use correct gamma curve, instead of approximation.
   r_prime = pow(r, 1.0 / 2.2);
   g_prime = pow(g, 1.0 / 2.2);
   b_prime = pow(b, 1.0 / 2.2);

   switch(VB3DMode)
   {
    case VB3DMODE_ANAGLYPH:
	r_prime = r_prime * ((Anaglyph_Colors[lr ^ VB3DReverse] >> 16) & 0xFF) / 255;
	g_prime = g_prime * ((Anaglyph_Colors[lr ^ VB3DReverse] >> 8) & 0xFF) / 255;
	b_prime = b_prime * ((Anaglyph_Colors[lr ^ VB3DReverse] >> 0) & 0xFF) / 255;
	break;
    default:
        r_prime = r_prime * ((Default_Color >> 16) & 0xFF) / 255;
        g_prime = g_prime * ((Default_Color >> 8) & 0xFF) / 255;
        b_prime = b_prime * ((Default_Color >> 0) & 0xFF) / 255;
	break;
   }
   ColorLUTNoGC[lr][i][0] = pow(r_prime, 2.2 / 1.0);
   ColorLUTNoGC[lr][i][1] = pow(g_prime, 2.2 / 1.0);
   ColorLUTNoGC[lr][i][2] = pow(b_prime, 2.2 / 1.0);

   ColorLUT[lr][i] = format.MakeColor((int)(r_prime * 255), (int)(g_prime * 255), (int)(b_prime * 255), 0);
  }
 }

 // Anaglyph slow-mode LUT calculation
 for(int l_b = 0; l_b < 256; l_b++)
 {
  for(int r_b = 0; r_b < 256; r_b++)
  {
   double r, g, b;
   double r_prime, g_prime, b_prime;

   r = ColorLUTNoGC[0][l_b][0] + ColorLUTNoGC[1][r_b][0];
   g = ColorLUTNoGC[0][l_b][1] + ColorLUTNoGC[1][r_b][1];
   b = ColorLUTNoGC[0][l_b][2] + ColorLUTNoGC[1][r_b][2];

   if(r > 1.0)
    r = 1.0;
   if(g > 1.0)
    g = 1.0;
   if(b > 1.0)
    b = 1.0;

   r_prime = pow(r, 1.0 / 2.2);
   g_prime = pow(g, 1.0 / 2.2);
   b_prime = pow(b, 1.0 / 2.2);

   AnaSlowColorLUT[l_b][r_b] = format.MakeColor((int)(r_prime * 255), (int)(g_prime * 255), (int)(b_prime * 255), 0);
  }
 }
}

static void RecalcBrightnessCache(void)
{
 //printf("BRTA: %d, BRTB: %d, BRTC: %d, Rest: %d\n", BRTA, BRTB, BRTC, REST);
 int32 CumulativeTime = (BRTA + 1 + BRTB + 1 + BRTC + 1 + REST + 1) + 1;
 int32 MaxTime = 128;

 BrightnessCache[0] = 0;
 BrightnessCache[1] = 0;
 BrightnessCache[2] = 0;
 BrightnessCache[3] = 0;

 for(int i = 0; i < Repeat + 1; i++)
 {
  int32 btemp[4];

  if((i * CumulativeTime) >= MaxTime)
   break;

  btemp[1] = (i * CumulativeTime) + BRTA;
  if(btemp[1] > MaxTime)
   btemp[1] = MaxTime;
  btemp[1] -= (i * CumulativeTime);
  if(btemp[1] < 0)
   btemp[1] = 0;


  btemp[2] = (i * CumulativeTime) + BRTA + 1 + BRTB;
  if(btemp[2] > MaxTime)
   btemp[2] = MaxTime;
  btemp[2] -= (i * CumulativeTime) + BRTA + 1;
  if(btemp[2] < 0)
   btemp[2] = 0;

  //btemp[3] = (i * CumulativeTime) + BRTA + 1 + BRTB + 1 + BRTC;
  //if(btemp[3] > MaxTime)
  // btemp[3] = MaxTime;
  //btemp[3] -= (i * CumulativeTime);
  //if(btemp[3] < 0)
  // btemp[3] = 0;

  btemp[3] = (i * CumulativeTime) + BRTA + BRTB + BRTC + 1;
  if(btemp[3] > MaxTime)
   btemp[3] = MaxTime;
  btemp[3] -= (i * CumulativeTime) + 1;
  if(btemp[3] < 0)
   btemp[3] = 0;

  BrightnessCache[1] += btemp[1];
  BrightnessCache[2] += btemp[2];
  BrightnessCache[3] += btemp[3];
 }

 //printf("BC: %d %d %d %d\n", BrightnessCache[0], BrightnessCache[1], BrightnessCache[2], BrightnessCache[3]);

 for(int i = 0; i < 4; i++)
  BrightnessCache[i] = 255 * BrightnessCache[i] / MaxTime;

 for(int lr = 0; lr < 2; lr++)
  for(int i = 0; i < 4; i++)
  {
   BrightCLUT[lr][i] = ColorLUT[lr][BrightnessCache[i]];
  }
}

static void Recalc3DModeStuff(bool non_rgb_output = false)
{
 switch(VB3DMode)
 {
  default: 
	   if(((Anaglyph_Colors[0] & 0xFF) && (Anaglyph_Colors[1] & 0xFF)) ||
		((Anaglyph_Colors[0] & 0xFF00) && (Anaglyph_Colors[1] & 0xFF00)) ||
		((Anaglyph_Colors[0] & 0xFF0000) && (Anaglyph_Colors[1] & 0xFF0000)) ||
		non_rgb_output)
	   {
            CopyFBColumnToTarget = CopyFBColumnToTarget_AnaglyphSlow;
	   }
           else
            CopyFBColumnToTarget = CopyFBColumnToTarget_Anaglyph;
           break;

  case VB3DMODE_CSCOPE:
           CopyFBColumnToTarget = CopyFBColumnToTarget_CScope;
           break;

  case VB3DMODE_SIDEBYSIDE:
           CopyFBColumnToTarget = CopyFBColumnToTarget_SideBySide;
           break;

  case VB3DMODE_VLI:
           CopyFBColumnToTarget = CopyFBColumnToTarget_VLI;
           break;

  case VB3DMODE_HLI:
           CopyFBColumnToTarget = CopyFBColumnToTarget_HLI;
           break;
 }
 RecalcBrightnessCache();
}

void VIP_Set3DMode(uint32 mode, bool reverse, uint32 prescale, uint32 sbs_separation)
{
 VB3DMode = mode;
 VB3DReverse = reverse ? 1 : 0;
 VBPrescale = prescale;
 VBSBS_Separation = sbs_separation;

 VidSettingsDirty = true;

 for(uint32 p = 0; p < 256; p++)
 {
  uint32 v;
  uint8 s[4];

  s[0] = (p >> 0) & 0x3;
  s[1] = (p >> 2) & 0x3;
  s[2] = (p >> 4) & 0x3;
  s[3] = (p >> 6) & 0x3;

  v = 0;
  for(unsigned int i = 0, shifty = 0; i < 4; i++)
  {
   for(unsigned int ps = 0; ps < prescale; ps++)
   {
    v |= s[i] << shifty;
    shifty += 2;
   }
  }

  HLILUT[p] = v;
 }
}

void VIP_SetParallaxDisable(bool disabled)
{
 ParallaxDisabled = disabled;
}

void VIP_SetDefaultColor(uint32 default_color)
{
 Default_Color = default_color;

 VidSettingsDirty = true;
}


void VIP_SetAnaglyphColors(uint32 lcolor, uint32 rcolor)
{
 Anaglyph_Colors[0] = lcolor;
 Anaglyph_Colors[1] = rcolor;

 VidSettingsDirty = true;
}

void VIP_SetInstantDisplayHack(bool val)
{
 InstantDisplayHack = val;
}

void VIP_SetAllowDrawSkip(bool val)
{
 AllowDrawSkip = val;
}


static uint16 FRMCYC;

static uint16 DPCTRL;
static bool DisplayActive;

#define XPCTRL_XP_RST	0x0001
#define XPCTRL_XP_EN	0x0002
static uint16 XPCTRL;
static uint16 SBCMP;	// Derived from XPCTRL

static uint16 SPT[4];	// SPT0~SPT3, 5f848~5f84e
static uint16 GPLT[4];
static uint8 GPLT_Cache[4][4];

static INLINE void Recalc_GPLT_Cache(int which)
{
 for(int i = 0; i < 4; i++)
  GPLT_Cache[which][i] = (GPLT[which] >> (i * 2)) & 3;
}

static uint16 JPLT[4];
static uint8 JPLT_Cache[4][4];

static INLINE void Recalc_JPLT_Cache(int which)
{
 for(int i = 0; i < 4; i++)
  JPLT_Cache[which][i] = (JPLT[which] >> (i * 2)) & 3;
}


static uint16 BKCOL;

//
//
//
static int32 CalcNextEvent(void);

static int32 last_ts;

static int32 Column;
static int32 ColumnCounter;

static int32 DisplayRegion;
static bool DisplayFB;

static int32 GameFrameCounter;

static int32 DrawingCounter;
static bool DrawingActive;
static bool DrawingFB;
static uint32 DrawingBlock;
static int32 SB_Latch;
static int32 SBOUT_InactiveTime;

//static uint8 CTA_L, CTA_R;

static void CheckIRQ(void)
{
 VBIRQ_Assert(VBIRQ_SOURCE_VIP, (bool)(InterruptEnable & InterruptPending));

 #if 0
 printf("%08x\n", InterruptEnable & InterruptPending);
 if((bool)(InterruptEnable & InterruptPending))
  puts("IRQ asserted");
 else
  puts("IRQ not asserted"); 
 #endif
}


bool VIP_Init(void)
{
 InstantDisplayHack = false;
 AllowDrawSkip = false;
 ParallaxDisabled = false;
 Anaglyph_Colors[0] = 0xFF0000;
 Anaglyph_Colors[1] = 0x0000FF;
 VB3DMode = VB3DMODE_ANAGLYPH;
 Default_Color = 0xFFFFFF;
 VB3DReverse = 0;
 VBPrescale = 1;
 VBSBS_Separation = 0;

 VidSettingsDirty = true;

 return(true);
}

void VIP_Power(void)
{
 Repeat = 0;
 SB_Latch = 0;
 SBOUT_InactiveTime = -1;
 last_ts = 0;

 Column = 0;
 ColumnCounter = 259;

 DisplayRegion = 0;
 DisplayFB = 0;

 GameFrameCounter = 0;

 DrawingCounter = 0;
 DrawingActive = false;
 DrawingFB = 0;
 DrawingBlock = 0;

 DPCTRL = 2;
 DisplayActive = false;



 memset(FB, 0, 0x6000 * 2 * 2);
 memset(CHR_RAM, 0, 0x8000);
 memset(DRAM, 0, 0x20000);

 InterruptPending = 0;
 InterruptEnable = 0;

 BRTA = 0;
 BRTB = 0;
 BRTC = 0;
 REST = 0;

 FRMCYC = 0;

 XPCTRL = 0;
 SBCMP = 0;

 for(int i = 0; i < 4; i++)
 {
  SPT[i] = 0;
  GPLT[i] = 0;
  JPLT[i] = 0;

  Recalc_GPLT_Cache(i);
  Recalc_JPLT_Cache(i);
 }

 BKCOL = 0;
}

static INLINE uint16 ReadRegister(int32 &timestamp, uint32 A)
{
 uint16 ret = 0;	//0xFFFF;

 if(A & 1)
  VIP_DBGMSG("Misaligned VIP Read: %08x", A);

 switch(A & 0xFE)
 {
  default: VIP_DBGMSG("Unknown VIP register read: %08x", A);
	   break;

  case 0x00: ret = InterruptPending;
	     break;

  case 0x02: ret = InterruptEnable;
	     break;

  case 0x20: //printf("Read DPSTTS at %d\n", timestamp);
	     ret = DPCTRL & 0x702;
	     if((DisplayRegion & 1) && DisplayActive)
	     {
	      unsigned int DPBSY = 1 << ((DisplayRegion >> 1) & 1);

	      if(DisplayFB)
	       DPBSY <<= 2;

	      ret |= DPBSY << 2;
	     }
	     //if(!(DisplayRegion & 1))	// FIXME? (Had to do it this way for Galactic Pinball...)
              ret |= 1 << 6;
	     break;

  // Note: Upper bits of BRTA, BRTB, BRTC, and REST(?) are 0 when read(on real hardware)
  case 0x24: ret = BRTA;
             break;

  case 0x26: ret = BRTB;
             break;

  case 0x28: ret = BRTC;
             break;

  case 0x2A: ret = REST;
             break;

  case 0x30: ret = 0xFFFF;
	     break;

  case 0x40: ret = XPCTRL & 0x2;
	     if(DrawingActive)
	     {
	      ret |= (1 + DrawingFB) << 2;
	     }
	     if(timestamp < SBOUT_InactiveTime)
	     {
	      ret |= 0x8000;
	      ret |= /*DrawingBlock*/SB_Latch << 8;
	     }
	     break;     // XPSTTS, read-only

  case 0x48:
  case 0x4a:
  case 0x4c:
  case 0x4e: ret = SPT[(A >> 1) & 3];
             break;

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66: ret = GPLT[(A >> 1) & 3];
             break;

  case 0x68:
  case 0x6a:
  case 0x6c:
  case 0x6e: ret = JPLT[(A >> 1) & 3];
             break;

  case 0x70: ret = BKCOL;
             break;
 }

 return(ret);
}

static INLINE void WriteRegister(int32 &timestamp, uint32 A, uint16 V)
{
 if(A & 1)
  VIP_DBGMSG("Misaligned VIP Write: %08x %04x", A, V);

 switch(A & 0xFE)
 {
  default: VIP_DBGMSG("Unknown VIP register write: %08x %04x", A, V);
           break;

  case 0x00: break; // Interrupt pending, read-only

  case 0x02: {
	      InterruptEnable = V & 0xE01F;

	      VIP_DBGMSG("Interrupt Enable: %04x", V);

	      if(V & 0x2000)
	       VIP_DBGMSG("Warning: VIP SB Hit Interrupt enable: %04x\n", V);
	      CheckIRQ();
	     }
	     break;

  case 0x04: InterruptPending &= ~V;
	     CheckIRQ();
	     break;

  case 0x20: break; // Display control, read-only.

  case 0x22: DPCTRL = V & (0x703); // Display-control, write-only
	     if(V & 1)
	     {
	      DisplayActive = false;
	      InterruptPending &= ~(INT_TIME_ERR | INT_FRAME_START | INT_GAME_START | INT_RFB_END | INT_LFB_END | INT_SCAN_ERR);
	      CheckIRQ();
	     }
	     break;

  case 0x24: BRTA = V & 0xFF;	// BRTA
	     RecalcBrightnessCache();
	     break;

  case 0x26: BRTB = V & 0xFF;	// BRTB
	     RecalcBrightnessCache();
	     break;

  case 0x28: BRTC = V & 0xFF;	// BRTC
	     RecalcBrightnessCache();
	     break;

  case 0x2A: REST = V & 0xFF;	// REST
	     RecalcBrightnessCache();
	     break;

  case 0x2E: FRMCYC = V & 0xF;	// FRMCYC, write-only?
	     break;

  case 0x30: break;	// CTA, read-only(

  case 0x40: break;	// XPSTTS, read-only

  case 0x42: XPCTRL = V & 0x0002;	// XPCTRL, write-only
	     SBCMP = (V >> 8) & 0x1F;

	     if(V & 1)
	     {
	      VIP_DBGMSG("XPRST");
	      DrawingActive = 0;
	      DrawingCounter = 0;
              InterruptPending &= ~(INT_SB_HIT | INT_XP_END | INT_TIME_ERR);
	      CheckIRQ();
	     }
	     break;

  case 0x44: break;	// Version Control, read-only?

  case 0x48:
  case 0x4a:
  case 0x4c:
  case 0x4e: SPT[(A >> 1) & 3] = V & 0x3FF;
	     break;

  case 0x60:
  case 0x62: 
  case 0x64:
  case 0x66: GPLT[(A >> 1) & 3] = V & 0xFC;
	     Recalc_GPLT_Cache((A >> 1) & 3);
	     break;

  case 0x68:
  case 0x6a:
  case 0x6c:
  case 0x6e: JPLT[(A >> 1) & 3] = V & 0xFC;
             Recalc_JPLT_Cache((A >> 1) & 3);
             break;

  case 0x70: BKCOL = V & 0x3;
	     break;

 }
}

//
// Don't update the VIP state on reads/writes, the event system will update it with enough precision as far as VB software cares.
//

uint8 VIP_Read8(int32 &timestamp, uint32 A)
{
 uint8 ret = 0; //0xFF;

 //VIP_Update(timestamp);

 switch(A >> 16)
 {
  case 0x0:
  case 0x1:
           if((A & 0x7FFF) >= 0x6000)
           {
            ret = VIP_MA16R8(CHR_RAM, (A & 0x1FFF) | ((A >> 2) & 0x6000));
           }
           else
           {
            ret = FB[(A >> 15) & 1][(A >> 16) & 1][A & 0x7FFF];
           }
           break;

  case 0x2:
  case 0x3: ret = VIP_MA16R8(DRAM, A & 0x1FFFF);
            break;

  case 0x4:
  case 0x5: if(A >= 0x5E000)
	     ret = ReadRegister(timestamp, A);
	    else
	     VIP_DBGMSG("Unknown VIP Read: %08x", A);
            break;

  case 0x6: break;

  case 0x7: if(A >= 0x8000)
            {
             ret = VIP_MA16R8(CHR_RAM, A & 0x7FFF);
            }
	    else
	     VIP_DBGMSG("Unknown VIP Read: %08x", A);
            break;

  default: VIP_DBGMSG("Unknown VIP Read: %08x", A);
	   break;
 }


 //VB_SetEvent(VB_EVENT_VIP, timestamp + CalcNextEvent());

 return(ret);
}

uint16 VIP_Read16(int32 &timestamp, uint32 A)
{
 uint16 ret = 0; //0xFFFF;

 //VIP_Update(timestamp); 

 switch(A >> 16)
 {
  case 0x0:
  case 0x1:
           if((A & 0x7FFF) >= 0x6000)
           {
            ret = VIP_MA16R16(CHR_RAM, (A & 0x1FFF) | ((A >> 2) & 0x6000));
           }
           else
           {
            ret = LoadU16_LE((uint16 *)&FB[(A >> 15) & 1][(A >> 16) & 1][A & 0x7FFF]);
           }
           break;

  case 0x2:
  case 0x3: ret = VIP_MA16R16(DRAM, A & 0x1FFFF);
            break;

  case 0x4:
  case 0x5: 
	    if(A >= 0x5E000)
	     ret = ReadRegister(timestamp, A);
            else
             VIP_DBGMSG("Unknown VIP Read: %08x", A);
            break;

  case 0x6: break;

  case 0x7: if(A >= 0x8000)
            {
             ret = VIP_MA16R16(CHR_RAM, A & 0x7FFF);
            }
	    else
	     VIP_DBGMSG("Unknown VIP Read: %08x", A);
            break;

  default: VIP_DBGMSG("Unknown VIP Read: %08x", A);
           break;
 }


 //VB_SetEvent(VB_EVENT_VIP, timestamp + CalcNextEvent());
 return(ret);
}

void VIP_Write8(int32 &timestamp, uint32 A, uint8 V)
{
 //VIP_Update(timestamp); 

 //if(A >= 0x3DC00 && A < 0x3E000)
 // printf("%08x %02x\n", A, V);

 switch(A >> 16)
 {
  case 0x0:
  case 0x1:
	   if((A & 0x7FFF) >= 0x6000)
	    VIP_MA16W8(CHR_RAM, (A & 0x1FFF) | ((A >> 2) & 0x6000), V);
	   else
	    FB[(A >> 15) & 1][(A >> 16) & 1][A & 0x7FFF] = V;
	   break;

  case 0x2:
  case 0x3: VIP_MA16W8(DRAM, A & 0x1FFFF, V);
	    break;

  case 0x4:
  case 0x5: if(A >= 0x5E000)
 	     WriteRegister(timestamp, A, V);
            else
             VIP_DBGMSG("Unknown VIP Write: %08x %02x", A, V);
	    break;

  case 0x6: VIP_DBGMSG("Unknown VIP Write: %08x %02x", A, V);
	    break;

  case 0x7: if(A >= 0x8000)
	     VIP_MA16W8(CHR_RAM, A & 0x7FFF, V);
	    else
	     VIP_DBGMSG("Unknown VIP Write: %08x %02x", A, V);
	    break;

  default: VIP_DBGMSG("Unknown VIP Write: %08x %02x", A, V);
           break;
 }

 //VB_SetEvent(VB_EVENT_VIP, timestamp + CalcNextEvent());
}

void VIP_Write16(int32 &timestamp, uint32 A, uint16 V)
{
 //VIP_Update(timestamp); 

 //if(A >= 0x3DC00 && A < 0x3E000)
 // printf("%08x %04x\n", A, V);

 switch(A >> 16)
 {
  case 0x0:
  case 0x1:
           if((A & 0x7FFF) >= 0x6000)
            VIP_MA16W16(CHR_RAM, (A & 0x1FFF) | ((A >> 2) & 0x6000), V);
           else
            StoreU16_LE((uint16 *)&FB[(A >> 15) & 1][(A >> 16) & 1][A & 0x7FFF], V);
           break;

  case 0x2:
  case 0x3: VIP_MA16W16(DRAM, A & 0x1FFFF, V);
            break;

  case 0x4:
  case 0x5: if(A >= 0x5E000)
 	     WriteRegister(timestamp, A, V);
            else
             VIP_DBGMSG("Unknown VIP Write: %08x %04x", A, V);
            break;

  case 0x6: VIP_DBGMSG("Unknown VIP Write: %08x %04x", A, V);
	    break;

  case 0x7: if(A >= 0x8000)
             VIP_MA16W16(CHR_RAM, A & 0x7FFF, V);
	    else
	     VIP_DBGMSG("Unknown VIP Write: %08x %04x", A, V);
            break;

  default: VIP_DBGMSG("Unknown VIP Write: %08x %04x", A, V);
           break;
 }


 //VB_SetEvent(VB_EVENT_VIP, timestamp + CalcNextEvent());
}

static MDFN_Surface *surface;
static bool skip;

void VIP_StartFrame(EmulateSpecStruct *espec)
{
// puts("Start frame");

 if(espec->VideoFormatChanged || VidSettingsDirty)
 {
  MakeColorLUT(espec->surface->format);
  Recalc3DModeStuff(espec->surface->format.colorspace != MDFN_COLORSPACE_RGB);

  VidSettingsDirty = false;
 }

 espec->DisplayRect.x = 0;
 espec->DisplayRect.y = 0;

 switch(VB3DMode)
 {
  default:
	espec->DisplayRect.w = 384;
	espec->DisplayRect.h = 224;
	break;

  case VB3DMODE_VLI:
	espec->DisplayRect.w = 768 * VBPrescale;
	espec->DisplayRect.h = 224;
	break;

  case VB3DMODE_HLI:
        espec->DisplayRect.w = 384;
        espec->DisplayRect.h = 448 * VBPrescale;
        break;

  case VB3DMODE_CSCOPE:
	espec->DisplayRect.w = 512;
	espec->DisplayRect.h = 384;
	break;

  case VB3DMODE_SIDEBYSIDE:
	espec->DisplayRect.w = 768 + VBSBS_Separation;
	espec->DisplayRect.h = 224;
	break;
 }

 surface = espec->surface;
 skip = espec->skip;
}

void VIP_ResetTS(void)
{
 if(SBOUT_InactiveTime >= 0)
  SBOUT_InactiveTime -= last_ts;
 last_ts = 0;
}

static int32 CalcNextEvent(void)
{
 return(ColumnCounter);
}

#include "vip_draw.inc"

static INLINE void CopyFBColumnToTarget_Anaglyph_BASE(const bool DisplayActive_arg, const int lr)
{
     const int fb = DisplayFB;
     uint32 *target = surface->pixels + Column;
     const int32 pitch32 = surface->pitch32;
     const uint8 *fb_source = &FB[fb][lr][64 * Column];

     for(int y = 56; y; y--)
     {
      uint32 source_bits = *fb_source;

      for(int y_sub = 4; y_sub; y_sub--)
      {
       uint32 pixel = BrightCLUT[lr][source_bits & 3];

       if(!DisplayActive_arg)
        pixel = 0;

       if(lr)
	*target |= pixel;
       else
        *target = pixel;

       source_bits >>= 2;
       target += pitch32;
      }
      fb_source++;
     }
}

static void CopyFBColumnToTarget_Anaglyph(void)
{
 const int lr = (DisplayRegion & 2) >> 1;

 if(!DisplayActive)
 {
  if(!lr)
   CopyFBColumnToTarget_Anaglyph_BASE(0, 0);
  else
   CopyFBColumnToTarget_Anaglyph_BASE(0, 1);
 }
 else
 {
  if(!lr)
   CopyFBColumnToTarget_Anaglyph_BASE(1, 0);
  else
   CopyFBColumnToTarget_Anaglyph_BASE(1, 1);
 }
}

static uint32 AnaSlowBuf[384][224];

static INLINE void CopyFBColumnToTarget_AnaglyphSlow_BASE(const bool DisplayActive_arg, const int lr)
{
     const int fb = DisplayFB;
     const uint8 *fb_source = &FB[fb][lr][64 * Column];

     if(!lr)
     {
      uint32 *target = AnaSlowBuf[Column];

      for(int y = 56; y; y--)
      {
       uint32 source_bits = *fb_source;

       for(int y_sub = 4; y_sub; y_sub--)
       {
        uint32 pixel = BrightnessCache[source_bits & 3];

        if(!DisplayActive_arg)
         pixel = 0;

        *target = pixel;
        source_bits >>= 2;
        target++;
       }
       fb_source++;
      }

     }
     else
     {
      uint32 *target = surface->pixels + Column;
      const uint32 *left_src = AnaSlowBuf[Column];
      const int32 pitch32 = surface->pitch32;

      for(int y = 56; y; y--)
      {
       uint32 source_bits = *fb_source;

       for(int y_sub = 4; y_sub; y_sub--)
       {
        uint32 pixel = AnaSlowColorLUT[*left_src][DisplayActive_arg ? BrightnessCache[source_bits & 3] : 0];

        *target = pixel;

        source_bits >>= 2;
        target += pitch32;
        left_src++;
       }
       fb_source++;
      }
     }
}

static void CopyFBColumnToTarget_AnaglyphSlow(void)
{
 const int lr = (DisplayRegion & 2) >> 1;

 if(!DisplayActive)
 {
  if(!lr)
   CopyFBColumnToTarget_AnaglyphSlow_BASE(0, 0);
  else
   CopyFBColumnToTarget_AnaglyphSlow_BASE(0, 1);
 }
 else
 {
  if(!lr)
   CopyFBColumnToTarget_AnaglyphSlow_BASE(1, 0);
  else
   CopyFBColumnToTarget_AnaglyphSlow_BASE(1, 1);
 }
}


static void CopyFBColumnToTarget_CScope_BASE(const bool DisplayActive_arg, const int lr, const int dest_lr)
{
     const int fb = DisplayFB;
     uint32 *target = surface->pixels + (dest_lr ? 512 - 16 - 1 : 16) + (dest_lr ? Column : 383 - Column) * surface->pitch32;
     const uint8 *fb_source = &FB[fb][lr][64 * Column];

     for(int y = 56; y; y--)
     {
      uint32 source_bits = *fb_source;

      for(int y_sub = 4; y_sub; y_sub--)
      {
       if(DisplayActive_arg)
        *target = BrightCLUT[lr][source_bits & 3];
       else
	*target = 0;

       source_bits >>= 2;
       if(dest_lr)
        target--;
       else
	target++;
      }
      fb_source++;
     }
}

static void CopyFBColumnToTarget_CScope(void)
{
 const int lr = (DisplayRegion & 2) >> 1;

 if(!DisplayActive)
 {
  if(!lr)
   CopyFBColumnToTarget_CScope_BASE(0, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_CScope_BASE(0, 1, 1 ^ VB3DReverse);
 }
 else
 {
  if(!lr)
   CopyFBColumnToTarget_CScope_BASE(1, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_CScope_BASE(1, 1, 1 ^ VB3DReverse);
 }
}

static void CopyFBColumnToTarget_SideBySide_BASE(const bool DisplayActive_arg, const int lr, const int dest_lr)
{
     const int fb = DisplayFB;
     uint32 *target = surface->pixels + Column + (dest_lr ? (384 + VBSBS_Separation) : 0);
     const int32 pitch32 = surface->pitch32;
     const uint8 *fb_source = &FB[fb][lr][64 * Column];

     for(int y = 56; y; y--)
     {
      uint32 source_bits = *fb_source;

      for(int y_sub = 4; y_sub; y_sub--)
      {
       if(DisplayActive_arg)
        *target = BrightCLUT[lr][source_bits & 3];
       else
	*target = 0;
       source_bits >>= 2;
       target += pitch32;
      }
      fb_source++;
     }
}

static void CopyFBColumnToTarget_SideBySide(void)
{
 const int lr = (DisplayRegion & 2) >> 1;

 if(!DisplayActive)
 {
  if(!lr)
   CopyFBColumnToTarget_SideBySide_BASE(0, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_SideBySide_BASE(0, 1, 1 ^ VB3DReverse);
 }
 else
 {
  if(!lr)
   CopyFBColumnToTarget_SideBySide_BASE(1, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_SideBySide_BASE(1, 1, 1 ^ VB3DReverse);
 }
}

static INLINE void CopyFBColumnToTarget_VLI_BASE(const bool DisplayActive_arg, const int lr, const int dest_lr)
{
     const int fb = DisplayFB;
     uint32 *target = surface->pixels + Column * 2 * VBPrescale + dest_lr;
     const int32 pitch32 = surface->pitch32;
     const uint8 *fb_source = &FB[fb][lr][64 * Column];

     for(int y = 56; y; y--)
     {
      uint32 source_bits = *fb_source;

      for(int y_sub = 4; y_sub; y_sub--)
      {
       uint32 tv;

       if(DisplayActive_arg)
        tv = BrightCLUT[0][source_bits & 3];
       else
        tv = 0;

       for(uint32 ps = 0; ps < VBPrescale; ps++)
	target[ps * 2] = tv;

       source_bits >>= 2;
       target += pitch32;
      }
      fb_source++;
     }
}

static void CopyFBColumnToTarget_VLI(void)
{
 const int lr = (DisplayRegion & 2) >> 1;

 if(!DisplayActive)
 {
  if(!lr)
   CopyFBColumnToTarget_VLI_BASE(0, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_VLI_BASE(0, 1, 1 ^ VB3DReverse);
 }
 else
 {
  if(!lr)
   CopyFBColumnToTarget_VLI_BASE(1, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_VLI_BASE(1, 1, 1 ^ VB3DReverse);
 }
}

static INLINE void CopyFBColumnToTarget_HLI_BASE(const bool DisplayActive_arg, const int lr, const int dest_lr)
{
     const int fb = DisplayFB;
     const int32 pitch32 = surface->pitch32;
     uint32 *target = surface->pixels + Column + dest_lr * pitch32;
     const uint8 *fb_source = &FB[fb][lr][64 * Column];

if(VBPrescale <= 4)
     for(int y = 56; y; y--)
     {
      uint32 source_bits = HLILUT[*fb_source];

      for(int y_sub = 4 * VBPrescale; y_sub; y_sub--)
      {
       if(DisplayActive_arg)
        *target = BrightCLUT[0][source_bits & 3];
       else
        *target = 0;

       target += pitch32 * 2;
       source_bits >>= 2;
      }
      fb_source++;
     }
else
     for(int y = 56; y; y--)
     {
      uint32 source_bits = *fb_source;

      for(int y_sub = 4; y_sub; y_sub--)
      {
       for(uint32 ps = 0; ps < VBPrescale; ps++)
       {
        if(DisplayActive_arg)
         *target = BrightCLUT[0][source_bits & 3];
        else
         *target = 0;

        target += pitch32 * 2;
       }

       source_bits >>= 2;
      }
      fb_source++;
     }
}

static void CopyFBColumnToTarget_HLI(void)
{
 const int lr = (DisplayRegion & 2) >> 1;

 if(!DisplayActive)
 {
  if(!lr)
   CopyFBColumnToTarget_HLI_BASE(0, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_HLI_BASE(0, 1, 1 ^ VB3DReverse);
 }
 else
 {
  if(!lr)
   CopyFBColumnToTarget_HLI_BASE(1, 0, 0 ^ VB3DReverse);
  else
   CopyFBColumnToTarget_HLI_BASE(1, 1, 1 ^ VB3DReverse);
 }
}


v810_timestamp_t MDFN_FASTCALL VIP_Update(const v810_timestamp_t timestamp)
{
 int32 clocks = timestamp - last_ts;
 int32 running_timestamp = timestamp;

 while(clocks > 0)
 {
  int32 chunk_clocks = clocks;

  if(DrawingCounter > 0 && chunk_clocks > DrawingCounter)
   chunk_clocks = DrawingCounter;
  if(chunk_clocks > ColumnCounter)
   chunk_clocks = ColumnCounter;

  running_timestamp += chunk_clocks;

  if(DrawingCounter > 0)
  {
   DrawingCounter -= chunk_clocks;
   if(DrawingCounter <= 0)
   {
    MDFN_ALIGN(8) uint8 DrawingBuffers[2][512 * 8];	// Don't decrease this from 512 unless you adjust vip_draw.inc(including areas that draw off-visible >= 384 and >= -7 for speed reasons)

    if(skip && InstantDisplayHack && AllowDrawSkip)
    {
#if 0
     for(int lr = 0; lr < 2; lr++)
     {
      uint8 *FB_Target = FB[DrawingFB][lr] + DrawingBlock * 2;
      for(int x = 0; x < 384; x++)
      {
       FB_Target[64 * x + 0] = BKCOL;
       FB_Target[64 * x + 1] = BKCOL;
      }
     }
#endif
    }
    else
    {
     VIP_DrawBlock(DrawingBlock, DrawingBuffers[0] + 8, DrawingBuffers[1] + 8);

     for(int lr = 0; lr < 2; lr++)
     {
      uint8 *FB_Target = FB[DrawingFB][lr] + DrawingBlock * 2;

      for(int x = 0; x < 384; x++)
      {
       FB_Target[64 * x + 0] = (DrawingBuffers[lr][8 + x + 512 * 0] << 0)
				  | (DrawingBuffers[lr][8 + x + 512 * 1] << 2)
				  | (DrawingBuffers[lr][8 + x + 512 * 2] << 4)
				  | (DrawingBuffers[lr][8 + x + 512 * 3] << 6);

       FB_Target[64 * x + 1] = (DrawingBuffers[lr][8 + x + 512 * 4] << 0) 
                                  | (DrawingBuffers[lr][8 + x + 512 * 5] << 2)
                                  | (DrawingBuffers[lr][8 + x + 512 * 6] << 4) 
                                  | (DrawingBuffers[lr][8 + x + 512 * 7] << 6);

      }
     }
    }

    SBOUT_InactiveTime = running_timestamp + 1120;
    SB_Latch = DrawingBlock;	// Not exactly correct, but probably doesn't matter.

    DrawingBlock++;
    if(DrawingBlock == 28)
    {
     DrawingActive = false;

     InterruptPending |= INT_XP_END;
     CheckIRQ();
    }
    else
     DrawingCounter += 1120 * 4;
   }
  }

  ColumnCounter -= chunk_clocks;
  if(ColumnCounter == 0)
  {
   if(DisplayRegion & 1)
   {
    if(!(Column & 3))
    {
     const int lr = (DisplayRegion & 2) >> 1;
     uint16 ctdata = VIP_MA16R16(DRAM, 0x1DFFE - ((Column >> 2) * 2) - (lr ? 0 : 0x200));

     if((ctdata >> 8) != Repeat)
     {
      Repeat = ctdata >> 8;
      RecalcBrightnessCache();
     }
    }
    if(!skip && !InstantDisplayHack)
     CopyFBColumnToTarget();
   }

   ColumnCounter = 259;
   Column++;
   if(Column == 384)
   {
    Column = 0;

    if(DisplayActive)
    {
     if(DisplayRegion & 1)	// Did we just finish displaying an active region?
     {
      if(DisplayRegion & 2)	// finished displaying right eye
       InterruptPending |= INT_RFB_END;
      else		// Otherwise, left eye
       InterruptPending |= INT_LFB_END;

      CheckIRQ();
     }
    }

    DisplayRegion = (DisplayRegion + 1) & 3;

    if(DisplayRegion == 0)	// New frame start
    {
     DisplayActive = DPCTRL & 0x2;

     if(DisplayActive)
     {
      InterruptPending |= INT_FRAME_START;
      CheckIRQ();
     }
     GameFrameCounter++;
     if(GameFrameCounter > FRMCYC) // New game frame start?
     {
      InterruptPending |= INT_GAME_START;
      CheckIRQ();

      if(XPCTRL & XPCTRL_XP_EN)
      {
       DisplayFB ^= 1;

       DrawingBlock = 0;
       DrawingActive = true;
       DrawingCounter = 1120 * 4;
       DrawingFB = DisplayFB ^ 1;
      }

      GameFrameCounter = 0;
     }

     if(!skip && InstantDisplayHack)
     {
	// Ugly kludge, fix in the future.
	int32 save_DisplayRegion = DisplayRegion;
	int32 save_Column = Column;
	uint8 save_Repeat = Repeat;

	for(int lr = 0; lr < 2; lr++)
	{
	 DisplayRegion = lr << 1;
	 for(Column = 0; Column < 384; Column++)
	 {
	  if(!(Column & 3))
	  {
	   uint16 ctdata = VIP_MA16R16(DRAM, 0x1DFFE - ((Column >> 2) * 2) - (lr ? 0 : 0x200));

	   if((ctdata >> 8) != Repeat)
	   {
	    Repeat = ctdata >> 8;
	    RecalcBrightnessCache();
	   }
	  }

          CopyFBColumnToTarget();
	 }
	}
	DisplayRegion = save_DisplayRegion;
	Column = save_Column;
	Repeat = save_Repeat;
	RecalcBrightnessCache();
     }

     VB_ExitLoop();
    }
   }
  }

  clocks -= chunk_clocks;
 }

 last_ts = timestamp;

 return(timestamp + CalcNextEvent());
}


int VIP_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(FB[0][0], 0x6000 * 2 * 2),
  SFARRAY16(CHR_RAM, 0x8000 / sizeof(uint16)),
  SFARRAY16(DRAM, 0x20000 / sizeof(uint16)),

  SFVAR(InterruptPending),
  SFVAR(InterruptEnable),

  SFVAR(BRTA),
  SFVAR(BRTB), 
  SFVAR(BRTC),
  SFVAR(REST),

  SFVAR(FRMCYC),
  SFVAR(DPCTRL),

  SFVAR(DisplayActive),

  SFVAR(XPCTRL),
  SFVAR(SBCMP),
  SFARRAY16(SPT, 4),
  SFARRAY16(GPLT, 4),	// FIXME
  SFARRAY16(JPLT, 4),
  
  SFVAR(BKCOL),

  SFVAR(Column),
  SFVAR(ColumnCounter),

  SFVAR(DisplayRegion),
  SFVAR(DisplayFB),
  
  SFVAR(GameFrameCounter),

  SFVAR(DrawingCounter),

  SFVAR(DrawingActive),
  SFVAR(DrawingFB),
  SFVAR(DrawingBlock),

  SFVAR(SB_Latch),
  SFVAR(SBOUT_InactiveTime),

  SFVAR(Repeat),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "VIP");

 if(load)
 {
  RecalcBrightnessCache();
  for(int i = 0; i < 4; i++)
  {
   Recalc_GPLT_Cache(i);
   Recalc_JPLT_Cache(i);
  }
 }

 return(ret);
}

uint32 VIP_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 ret = 0xDEADBEEF;

 switch(id)
 {
  case VIP_GSREG_IPENDING:
	ret = InterruptPending;
	break;

  case VIP_GSREG_IENABLE:
	ret = InterruptEnable;
	break;

  case VIP_GSREG_DPCTRL:
	ret = DPCTRL;
	break;

  case VIP_GSREG_BRTA:
	ret = BRTA;
	break;

  case VIP_GSREG_BRTB:
	ret = BRTB;
	break;

  case VIP_GSREG_BRTC:
	ret = BRTC;
	break;

  case VIP_GSREG_REST:
	ret = REST;
	break;

  case VIP_GSREG_FRMCYC:
	ret = FRMCYC;
	break;

  case VIP_GSREG_XPCTRL:
	ret = XPCTRL | (SBCMP << 8);
	break;

  case VIP_GSREG_SPT0:
  case VIP_GSREG_SPT1:
  case VIP_GSREG_SPT2:
  case VIP_GSREG_SPT3:
	ret = SPT[id - VIP_GSREG_SPT0];
	break;

  case VIP_GSREG_GPLT0:
  case VIP_GSREG_GPLT1:
  case VIP_GSREG_GPLT2:
  case VIP_GSREG_GPLT3:
	ret = GPLT[id - VIP_GSREG_GPLT0];
	break;

  case VIP_GSREG_JPLT0:
  case VIP_GSREG_JPLT1:
  case VIP_GSREG_JPLT2:
  case VIP_GSREG_JPLT3:
	ret = JPLT[id - VIP_GSREG_JPLT0];
	break;

  case VIP_GSREG_BKCOL:
	ret = BKCOL;
	break;

 }

 if(id == VIP_GSREG_IPENDING || id == VIP_GSREG_IENABLE)
 {
        if(special)
         trio_snprintf(special, special_len, "%s: %s%s%s%s%s%s%s%s",
		(id == VIP_GSREG_IPENDING) ? "Interrupts Pending" : "Interrupts Enabled",
                (ret & INT_SCAN_ERR) ? "SCAN_ERR " : "",
                (ret & INT_LFB_END) ? "LFB_END " : "",
                (ret & INT_RFB_END) ? "RFB_END " : "",
                (ret & INT_GAME_START) ? "GAME_START " : "",
                (ret & INT_FRAME_START) ? "FRAME_START " : "",
                (ret & INT_SB_HIT) ? "SB_HIT " : "",
                (ret & INT_XP_END) ? "XP_END " : "",
                (ret & INT_TIME_ERR) ? "TIME_ERR " : "");

 }

 return(ret);
}

void VIP_SetRegister(const unsigned int id, const uint32 value)
{
 switch(id)
 {
  case VIP_GSREG_IPENDING:
        InterruptPending = value & 0xE01F;
	CheckIRQ();
        break;

  case VIP_GSREG_IENABLE:
        InterruptEnable = value & 0xE01F;
	CheckIRQ();
        break;

  case VIP_GSREG_DPCTRL:
	DPCTRL = value & 0x703;	// FIXME(Lower bit?)
        break;

  case VIP_GSREG_BRTA:
        BRTA = value & 0xFF;
        RecalcBrightnessCache();
        break;

  case VIP_GSREG_BRTB:
        BRTB = value & 0xFF;
        RecalcBrightnessCache();
        break;

  case VIP_GSREG_BRTC:
        BRTC = value & 0xFF;
        RecalcBrightnessCache();
        break;

  case VIP_GSREG_REST:
        REST = value & 0xFF;
        RecalcBrightnessCache();
        break;

  case VIP_GSREG_FRMCYC:
        FRMCYC = value & 0xF;
        break;

  case VIP_GSREG_XPCTRL:
	XPCTRL = value & 0x2;
	SBCMP = (value >> 8) & 0x1f;
        break;

  case VIP_GSREG_SPT0:
  case VIP_GSREG_SPT1:
  case VIP_GSREG_SPT2:
  case VIP_GSREG_SPT3:
        SPT[id - VIP_GSREG_SPT0] = value & 0x3FF;
        break;

  case VIP_GSREG_GPLT0:
  case VIP_GSREG_GPLT1:
  case VIP_GSREG_GPLT2:
  case VIP_GSREG_GPLT3:
        GPLT[id - VIP_GSREG_GPLT0] = value & 0xFC;
	Recalc_GPLT_Cache(id - VIP_GSREG_GPLT0);
        break;

  case VIP_GSREG_JPLT0:
  case VIP_GSREG_JPLT1:
  case VIP_GSREG_JPLT2:
  case VIP_GSREG_JPLT3:
        JPLT[id - VIP_GSREG_JPLT0] = value & 0xFC;
	Recalc_JPLT_Cache(id - VIP_GSREG_JPLT0);
        break;

  case VIP_GSREG_BKCOL:
        BKCOL = value & 0x03;
        break;
 }
}


}
