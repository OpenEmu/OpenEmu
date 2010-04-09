/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/




#include "snes9x.h"
#include "cpuexec.h"
#include "gfx.h"
#include "ppu.h"
#include "tile.h"
#include "display.h"
#include "controls.h"
#include "screenshot.h"
#include "cheats.h"
#include "movie.h"

#include "font.h"
static int font_width = 8;
static int font_height = 9;

// private text-displaying functions
static void S9xDisplayPressedKeys ();
static void S9xDisplayFrameRate ();
static void S9xDisplayString (const char *string, int linesFromBottom=5, int pixelsFromLeft=1, bool allowWrap=true);
static void S9xDisplayWatchedAddresses ();
// shared params for the above functions and DisplayChar
uint16* display_screen = NULL;
int display_ppl, display_width, display_height;
int display_fontwidth = font_width, display_fontheight = font_height, display_hfontaccessscale = 1, display_vfontaccessscale = 1;
bool8 display_paramsinited = FALSE;
#define DisplayString (S9xCustomDisplayString ? S9xCustomDisplayString : S9xDisplayString)

void ComputeClipWindows ();

extern struct SLineData LineData[240];
extern struct SLineMatrixData LineMatrixData [240];

bool8 S9xGraphicsInit(){
    S9xInitTileRenderer();
    ZeroMemory(BlackColourMap, 256*sizeof(uint16));

#ifdef GFX_MULTI_FORMAT
    if(GFX.BuildPixel==NULL) S9xSetRenderPixelFormat(RGB565);
#endif

    GFX.DoInterlace=0;
    GFX.InterlaceFrame=0;
    Settings.BG_Forced=0;
    IPPU.OBJChanged=TRUE;
    IPPU.DirectColourMapsNeedRebuild=TRUE;
    GFX.RealPPL=GFX.Pitch>>1;
    S9xFixColourBrightness();

    GFX.X2=GFX.ZERO_OR_X2=GFX.ZERO=NULL;
    if(!(GFX.X2=(uint16*)malloc(sizeof(uint16)*0x10000))) goto FAIL;
#if !defined(NEW_COLOUR_BLENDING)
    if(!(GFX.ZERO_OR_X2=(uint16*)malloc(sizeof(uint16)*0x10000))) goto FAIL;
#endif
    if(!(GFX.ZERO=(uint16*)malloc(sizeof(uint16)*0x10000))) goto FAIL;

    GFX.ScreenSize=GFX.Pitch/2*SNES_HEIGHT_EXTENDED*(Settings.SupportHiRes?2:1);
    if(!(GFX.SubScreen=(uint16*)malloc(GFX.ScreenSize*sizeof(uint16)))) goto FAIL;
    if(!(GFX.ZBuffer=(uint8*)malloc(GFX.ScreenSize))) goto FAIL;
    if(!(GFX.SubZBuffer=(uint8*)malloc(GFX.ScreenSize))) goto FAIL;

    uint32 r, g, b;
    /* Lookup table for color addition */
    for(r=0; r<=MAX_RED; r++){
        uint32 r2=r<<1; if(r2>MAX_RED) r2=MAX_RED;
        for(g=0; g<=MAX_GREEN; g++){
            uint32 g2=g<<1; if(g2>MAX_GREEN) g2=MAX_GREEN;
            for(b=0; b<=MAX_BLUE; b++){
                uint32 b2=b<<1; if(b2>MAX_BLUE) b2=MAX_BLUE;
                GFX.X2[BUILD_PIXEL2(r,g,b)]=BUILD_PIXEL2(r2,g2,b2);
                GFX.X2[BUILD_PIXEL2(r,g,b) & ~ALPHA_BITS_MASK]=BUILD_PIXEL2(r2,g2,b2);
            }
        }
    }

#if !defined(NEW_COLOUR_BLENDING)
    /* Lookup table for color subtraction */
    ZeroMemory(GFX.ZERO_OR_X2, 0x10000*sizeof(uint16));
    for(r=0; r<=MAX_RED; r++){
        uint32 r2=r; if(r2&0x10) r2=(r2<<1)&MAX_RED; else r2=0;
#if !defined(OLD_COLOUR_BLENDING)
        if(r2==0) r2=1;
#endif
        for(g=0; g<=MAX_GREEN; g++){
            uint32 g2=g; if(g2&GREEN_HI_BIT) g2=(g2<<1)&MAX_GREEN; else g2=0;
#if !defined(OLD_COLOUR_BLENDING)
        if(g2==0) g2=1;
#endif
            for(b=0; b<=MAX_BLUE; b++){
                uint32 b2=b; if(b2&0x10) b2=(b2<<1)&MAX_BLUE; else b2=0;
#if !defined(OLD_COLOUR_BLENDING)
                if(b2==0) b2=1;
#endif
                GFX.ZERO_OR_X2[BUILD_PIXEL2(r,g,b)]=BUILD_PIXEL2(r2,g2,b2);
                GFX.ZERO_OR_X2[BUILD_PIXEL2(r,g,b) & ~ALPHA_BITS_MASK]=BUILD_PIXEL2(r2,g2,b2);
            }
        }
    }
#endif

    /* Lookup table for 1/2 color subtraction */
    ZeroMemory(GFX.ZERO, 0x10000*sizeof(uint16));
    for(r=0; r<=MAX_RED; r++){
        uint32 r2=r; if(r2&0x10) r2&=~0x10; else r2=0;
        for(g=0; g<=MAX_GREEN; g++){
            uint32 g2=g; if(g2&GREEN_HI_BIT) g2&=~GREEN_HI_BIT; else g2=0;
            for(b=0; b<=MAX_BLUE; b++){
                uint32 b2=b; if(b2&0x10) b2&=~0x10; else b2=0;
                GFX.ZERO[BUILD_PIXEL2(r,g,b)]=BUILD_PIXEL2(r2,g2,b2);
                GFX.ZERO[BUILD_PIXEL2(r,g,b) & ~ALPHA_BITS_MASK]=BUILD_PIXEL2(r2,g2,b2);
            }
        }
    }

	GFX.Repainting = FALSE;

    return TRUE;

FAIL:
    S9xGraphicsDeinit();
    return FALSE;
}

void S9xGraphicsDeinit(void){
    if(GFX.X2) free(GFX.X2); GFX.X2=NULL;
    if(GFX.ZERO_OR_X2) free(GFX.ZERO_OR_X2); GFX.ZERO_OR_X2=NULL;
    if(GFX.ZERO) free(GFX.ZERO); GFX.ZERO=NULL;
    if(GFX.SubScreen) free(GFX.SubScreen); GFX.SubScreen=NULL;
    if(GFX.ZBuffer) free(GFX.ZBuffer); GFX.ZBuffer=NULL;
    if(GFX.SubZBuffer) free(GFX.SubZBuffer); GFX.SubZBuffer=NULL;
}

void S9xBuildDirectColourMaps(void){
    IPPU.XB=mul_brightness[PPU.Brightness];

    for(uint32 p=0; p<8; p++){
        for(uint32 c=0; c<256; c++){
            DirectColourMaps[p][c]=BUILD_PIXEL(IPPU.XB[((c&7)<<2) | ((p&1)<<1)],
                                               IPPU.XB[((c&0x38)>>1) | (p&2)],
                                               IPPU.XB[((c&0xc0)>>3) | (p&4)]);
        }
    }
    IPPU.DirectColourMapsNeedRebuild=FALSE;
}

void S9xStartScreenRefresh(){
    if(GFX.InfoStringTimeout>0 && --GFX.InfoStringTimeout==0)
        GFX.InfoString=NULL;

    if(IPPU.RenderThisFrame){
        if(GFX.DoInterlace && GFX.InterlaceFrame==0){
            GFX.InterlaceFrame=1;
        } else {
            if(!S9xInitUpdate()){
                IPPU.RenderThisFrame=FALSE;
                return;
            }
            GFX.InterlaceFrame=0;
            if(GFX.DoInterlace) GFX.DoInterlace--;
            IPPU.RenderedFramesCount++;
            IPPU.MaxBrightness=PPU.Brightness;
            if(PPU.BGMode==5 || PPU.BGMode==6){
                IPPU.Interlace = (Memory.FillRAM[0x2133]&1);
                IPPU.InterlaceOBJ = (Memory.FillRAM[0x2133]&2);
            }
            IPPU.PseudoHires = Memory.FillRAM[0x2133]&8;
            if(Settings.SupportHiRes && (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.PseudoHires || IPPU.Interlace || IPPU.InterlaceOBJ)){
                GFX.RealPPL = GFX.Pitch>>1;
                IPPU.DoubleWidthPixels = TRUE;
                IPPU.RenderedScreenWidth = 512;
                if(IPPU.Interlace || IPPU.InterlaceOBJ){
                    GFX.PPL = GFX.RealPPL<<1;
                    IPPU.DoubleHeightPixels = TRUE;
                    IPPU.RenderedScreenHeight = PPU.ScreenHeight<<1;
                    GFX.DoInterlace++;
                } else {
                    GFX.PPL = GFX.RealPPL;
                    IPPU.DoubleHeightPixels = FALSE;
                    IPPU.RenderedScreenHeight = PPU.ScreenHeight;
                }
            } else {
                IPPU.DoubleWidthPixels = FALSE;
                IPPU.DoubleHeightPixels = FALSE;
                IPPU.RenderedScreenWidth = 256;
                IPPU.RenderedScreenHeight = PPU.ScreenHeight;
#ifdef USE_GLIDE
                if(Settings.GlideEnable) {
                    // Speed up hack for Glide: render low res. SNES images
                    // into a handy 256x256 sized buffer that can be uploaded
                    // into texture memory with one Glide call without having
                    // to copy it into a second, suitably sized buffer first.
                    GFX.RealPPL = GFX.PPL = 256;
                } else
#endif
#ifdef USE_OPENGL
                if(Settings.OpenGLEnable) {
                    // Ditto for OpenGL
                    GFX.RealPPL = GFX.PPL = 256;
                } else
#endif
                {
                    GFX.RealPPL = GFX.PPL = GFX.Pitch>>1;
                }
            }
        }
        PPU.MosaicStart=0;
        IPPU.PreviousLine=IPPU.CurrentLine=0;
        PPU.RecomputeClipWindows=TRUE;
        ZeroMemory(GFX.ZBuffer, GFX.ScreenSize);
        ZeroMemory(GFX.SubZBuffer, GFX.ScreenSize);
    }
    if(++IPPU.FrameCount % Memory.ROMFramesPerSecond == 0) {
        IPPU.DisplayedRenderedFrameCount = IPPU.RenderedFramesCount;
        IPPU.RenderedFramesCount = 0;
        IPPU.FrameCount = 0;
    }
	++IPPU.TotalEmulatedFrames;
}

void RenderLine(uint8 C) {
    if(IPPU.RenderThisFrame) {
        LineData[C].BG[0].VOffset = PPU.BG[0].VOffset + 1;
        LineData[C].BG[0].HOffset = PPU.BG[0].HOffset;
        LineData[C].BG[1].VOffset = PPU.BG[1].VOffset + 1;
        LineData[C].BG[1].HOffset = PPU.BG[1].HOffset;

        if(PPU.BGMode==7) {
            struct SLineMatrixData *p = &LineMatrixData [C];
            p->MatrixA = PPU.MatrixA;
            p->MatrixB = PPU.MatrixB;
            p->MatrixC = PPU.MatrixC;
            p->MatrixD = PPU.MatrixD;
            p->CentreX = PPU.CentreX;
            p->CentreY = PPU.CentreY;
            p->M7HOFS  = PPU.M7HOFS;
            p->M7VOFS  = PPU.M7VOFS;
        } else {
			LineData[C].BG[2].VOffset = PPU.BG[2].VOffset + 1;
			LineData[C].BG[2].HOffset = PPU.BG[2].HOffset;
			LineData[C].BG[3].VOffset = PPU.BG[3].VOffset + 1;
			LineData[C].BG[3].HOffset = PPU.BG[3].HOffset;
        }
        IPPU.CurrentLine = C + 1;
    } else {
        /* if we're not rendering this frame, we still need to update this */
        // XXX: Check ForceBlank? Or anything else?
        if(IPPU.OBJChanged) S9xSetupOBJ();
        PPU.RangeTimeOver |= GFX.OBJLines[C].RTOFlags;
    }
}

bool watchesCleared=false;
void S9xDisplayWatchedAddresses ()
{
	if(!watchesCleared)
	{
		for(unsigned int i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
			watches[i].on = false;
		watchesCleared = true;
	}

	for(unsigned int i = 0 ; i < sizeof(watches)/sizeof(*watches) ; i++)
	{
		if(!watches[i].on)
			break;

		int32 displayNumber = 0;

		extern struct SCheatData Cheat;
		for(int r=0;r<watches[i].size;r++)
			displayNumber+=(Cheat.CWatchRAM[(watches[i].address - 0x7E0000)+r])<<(8*r);

		char buf [32];

		if(watches[i].format==1)
			sprintf(buf, "%s,%du = %u", watches[i].desc, watches[i].size, (unsigned int)displayNumber);
		else if(watches[i].format==3)
			sprintf(buf, "%s,%dx = %X", watches[i].desc, watches[i].size, (unsigned int)displayNumber);
		else
		{
			// signed
			if(watches[i].size == 1)
				displayNumber = (int32)((int8)displayNumber);
			else if(watches[i].size == 2)
				displayNumber = (int32)((int16)displayNumber);
			else if(watches[i].size == 3)
				if(displayNumber >= 8388608)
					displayNumber -= 16777216;

			sprintf(buf, "%s,%ds = %d", watches[i].desc, watches[i].size, (int)displayNumber);
		}


		DisplayString(buf, 6+i, 1, false);
	}
}

void S9xReRefresh ()
{
	if(Settings.StopEmulation)
		return;

	GFX.Repainting = TRUE;
	S9xDeinitUpdate (IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight/*, Settings.SixteenBit*/);
	GFX.Repainting = FALSE;
}


void S9xEndScreenRefresh() {
    if(IPPU.RenderThisFrame) {
        FLUSH_REDRAW();
        if(GFX.DoInterlace && GFX.InterlaceFrame==0){
            S9xControlEOF();
			S9xContinueUpdate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
        } else {
            if(IPPU.ColorsChanged) {
                uint32 saved = PPU.CGDATA[0];
                IPPU.ColorsChanged = FALSE;
                S9xSetPalette ();
                PPU.CGDATA[0] = saved;
            }

            if(Settings.TakeScreenshot)
                S9xDoScreenshot(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
            S9xControlEOF();

			if(Settings.AutoDisplayMessages || Settings.OpenGLEnable || Settings.GlideEnable)
				S9xDisplayMessages(GFX.Screen, GFX.RealPPL, IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 1);

            S9xDeinitUpdate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
        }
    } else {
        S9xControlEOF();
    }
    S9xApplyCheats();
#ifdef DEBUGGER
    if(CPU.Flags & FRAME_ADVANCE_FLAG) {
        if(ICPU.FrameAdvanceCount) {
            ICPU.FrameAdvanceCount--;
            IPPU.RenderThisFrame = TRUE;
            IPPU.FrameSkip = 0;
        } else {
            CPU.Flags &= ~FRAME_ADVANCE_FLAG;
            CPU.Flags |= DEBUG_MODE_FLAG;
        }
    }
#endif
    if(CPU.SRAMModified) {
        if(!CPU.AutoSaveTimer) {
            if(!(CPU.AutoSaveTimer=Settings.AutoSaveDelay*Memory.ROMFramesPerSecond))
                CPU.SRAMModified=FALSE;
        } else {
            if(!--CPU.AutoSaveTimer) {
                S9xAutoSaveSRAM();
                CPU.SRAMModified=FALSE;
            }
        }
    }
}


void S9xSetupOBJ() {
    int SmallWidth, SmallHeight;
    int LargeWidth, LargeHeight;

    switch(PPU.OBJSizeSelect) {
      case 0:
        SmallWidth = SmallHeight = 8;
        LargeWidth = LargeHeight = 16;
        break;
      case 1:
        SmallWidth = SmallHeight = 8;
        LargeWidth = LargeHeight = 32;
        break;
      case 2:
        SmallWidth = SmallHeight = 8;
        LargeWidth = LargeHeight = 64;
        break;
      case 3:
        SmallWidth = SmallHeight = 16;
        LargeWidth = LargeHeight = 32;
        break;
      case 4:
        SmallWidth = SmallHeight = 16;
        LargeWidth = LargeHeight = 64;
        break;
      default:
      case 5:
        SmallWidth = SmallHeight = 32;
        LargeWidth = LargeHeight = 64;
        break;
      case 6:
        SmallWidth = 16; SmallHeight = 32;
        LargeWidth = 32; LargeHeight = 64;
        break;
      case 7:
        SmallWidth = 16; SmallHeight = 32;
        LargeWidth = LargeHeight = 32;
        break;
    }
    int inc=IPPU.InterlaceOBJ?2:1;

    /* OK, we have three cases here. Either there's no priority, priority is
     * normal FirstSprite, or priority is FirstSprite+Y. The first two are
     * easy, the last is somewhat more ... interesting. So we split them up. */

    int Height;
    uint8 S;

    if(!PPU.OAMPriorityRotation || !(PPU.OAMFlip&PPU.OAMAddr&1)){
        /* normal case */
        uint8 LineOBJ[SNES_HEIGHT_EXTENDED];
        memset(LineOBJ, 0, sizeof(LineOBJ));
        for(int i=0; i<SNES_HEIGHT_EXTENDED; i++){
            GFX.OBJLines[i].RTOFlags=0;
            GFX.OBJLines[i].Tiles=34;
            for(int j=0; j<32; j++){ GFX.OBJLines[i].OBJ[j].Sprite=-1; }
        }
        uint8 FirstSprite=PPU.FirstSprite;
        S=FirstSprite;
        do {
            if(PPU.OBJ[S].Size){
                GFX.OBJWidths[S]=LargeWidth; Height=LargeHeight;
            } else {
                GFX.OBJWidths[S]=SmallWidth; Height=SmallHeight;
            }
            int HPos=PPU.OBJ[S].HPos; if(HPos==-256) HPos=0;
            if(HPos>-GFX.OBJWidths[S] && HPos<=256)
            {
                if(HPos<0){
                    GFX.OBJVisibleTiles[S]=(GFX.OBJWidths[S]+HPos+7)>>3;
                } else if(HPos+GFX.OBJWidths[S]>255){
                    GFX.OBJVisibleTiles[S]=(256-HPos+7)>>3;
                } else {
                    GFX.OBJVisibleTiles[S]=GFX.OBJWidths[S]>>3;
                }
                for(uint8 line=0, Y=(uint8)(PPU.OBJ[S].VPos&0xff); line<Height; Y++, line+=inc){
                    if(Y>=SNES_HEIGHT_EXTENDED) continue;
                    if(LineOBJ[Y]>=32){
                        GFX.OBJLines[Y].RTOFlags|=0x40;
                        continue;
                    }
                    GFX.OBJLines[Y].Tiles-=GFX.OBJVisibleTiles[S];
                    if(GFX.OBJLines[Y].Tiles<0) GFX.OBJLines[Y].RTOFlags|=0x80;
                    GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Sprite=S;
                    if(PPU.OBJ[S].VFlip){
                        // Yes, Width not Height. It so happens that the
                        // sprites with H=2*W flip as two WxW sprites.
                        GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Line=line^(GFX.OBJWidths[S]-1);
                    } else {
                        GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Line=line;
                    }
                    LineOBJ[Y]++;
                }
            }
            S=(S+1)&0x7F;
        } while(S!=FirstSprite);

        for(int Y=1; Y<SNES_HEIGHT_EXTENDED; Y++){
            GFX.OBJLines[Y].RTOFlags |= GFX.OBJLines[Y-1].RTOFlags;
        }
    } else {
        /* evil FirstSprite+Y case */

        /* First, find out which sprites are on which lines */
        uint8 OBJOnLine[SNES_HEIGHT_EXTENDED][128];
        memset(OBJOnLine, 0, sizeof(OBJOnLine));

        for(S=0; S<128; S++){
            if(PPU.OBJ[S].Size){
                GFX.OBJWidths[S]=LargeWidth; Height=LargeHeight;
            } else {
                GFX.OBJWidths[S]=SmallWidth; Height=SmallHeight;
            }
            int HPos=PPU.OBJ[S].HPos; if(HPos==-256) HPos=256;
            if(HPos>-GFX.OBJWidths[S] && HPos<=256)
            {
                if(HPos<0){
                    GFX.OBJVisibleTiles[S]=(GFX.OBJWidths[S]+HPos+7)>>3;
                } else if(HPos+GFX.OBJWidths[S]>=257){
                    GFX.OBJVisibleTiles[S]=(257-HPos+7)>>3;
                } else {
                    GFX.OBJVisibleTiles[S]=GFX.OBJWidths[S]>>3;
                }
                for(uint8 line=0, Y=(uint8)(PPU.OBJ[S].VPos&0xff); line<Height; Y++, line+=inc){
                    if(Y>=SNES_HEIGHT_EXTENDED) continue;
                    if(PPU.OBJ[S].VFlip){
                        // Yes, Width not Height. It so happens that the
                        // sprites with H=2*W flip as two WxW sprites.
                        OBJOnLine[Y][S]=(line^(GFX.OBJWidths[S]-1)) | 0x80;
                    } else {
                        OBJOnLine[Y][S]=line | 0x80;
                    }
                }
            }
        }

        /* Now go through and pull out those OBJ that are actually visible. */
        int j;
        for(int Y=0; Y<SNES_HEIGHT_EXTENDED; Y++){
            GFX.OBJLines[Y].RTOFlags=Y?GFX.OBJLines[Y-1].RTOFlags:0;

            GFX.OBJLines[Y].Tiles=34;
            uint8 FirstSprite=(PPU.FirstSprite+Y)&0x7F;
            S=FirstSprite; j=0;
            do {
                if(OBJOnLine[Y][S]){
                    if(j>=32){
                        GFX.OBJLines[Y].RTOFlags|=0x40;
                        break;
                    }
                    GFX.OBJLines[Y].Tiles-=GFX.OBJVisibleTiles[S];
                    if(GFX.OBJLines[Y].Tiles<0) GFX.OBJLines[Y].RTOFlags|=0x80;
                    GFX.OBJLines[Y].OBJ[j].Sprite=S;
                    GFX.OBJLines[Y].OBJ[j++].Line=OBJOnLine[Y][S]&~0x80;
                }
                S=(S+1)&0x7F;
            } while(S!=FirstSprite);
            if(j<32) GFX.OBJLines[Y].OBJ[j].Sprite=-1;
        }
    }

    IPPU.OBJChanged = FALSE;
}

void DrawOBJS(int D){
    CHECK_SOUND();

    void (*DrawTile)(uint32,uint32,uint32,uint32)=NULL;
    void (*DrawClippedTile)(uint32,uint32,uint32,uint32,uint32,uint32)=NULL;

    int PixWidth = IPPU.DoubleWidthPixels?2:1;
    GFX.Z1=2;
    BG.InterlaceLine = GFX.InterlaceFrame?8:0;

    for(uint32 Y=GFX.StartY, Offset=Y*GFX.PPL; Y<=GFX.EndY; Y++, Offset+=GFX.PPL){
        int I = 0;
        int tiles=GFX.OBJLines[Y].Tiles;
        for (int S = GFX.OBJLines[Y].OBJ[I].Sprite; S >= 0 && I<32; S = GFX.OBJLines[Y].OBJ[++I].Sprite)
        {
            tiles+=GFX.OBJVisibleTiles[S];
            if(tiles<=0){
                continue;
            }

            int BaseTile = (((GFX.OBJLines[Y].OBJ[I].Line<<1) + (PPU.OBJ[S].Name&0xf0))&0xf0) | (PPU.OBJ[S].Name&0x100) | (PPU.OBJ[S].Palette << 10);
            int TileX = PPU.OBJ[S].Name&0x0f;
            int TileLine = (GFX.OBJLines[Y].OBJ[I].Line&7)*8;
            if(IPPU.InterlaceOBJ) TileLine>>=1;
            int TileInc = 1;

            if (PPU.OBJ[S].HFlip)
            {
                TileX = (TileX + (GFX.OBJWidths[S] >> 3) - 1) & 0x0f;
                BaseTile |= H_FLIP;
                TileInc = -1;
            }

            GFX.Z2 = D+PPU.OBJ[S].Priority*4;

            int DrawMode=3;
            int clip=0, next_clip=-1000;
            int X=PPU.OBJ[S].HPos; if(X==-256) X=256;
            for(int t=tiles, O=Offset+X*PixWidth; X<=256 && X<PPU.OBJ[S].HPos+GFX.OBJWidths[S]; TileX=(TileX+TileInc)&0x0f, X+=8, O+=8*PixWidth){
                if(X<-7 || --t<0 || X==256) continue;
                for(int x=X; x<X+8;){
                    if(x>=next_clip){
                        for(; clip<GFX.Clip[4].Count && GFX.Clip[4].Left[clip]<=x; clip++);
                        if(clip==0 || x>=GFX.Clip[4].Right[clip-1]){
                            DrawMode=0;
                            next_clip=((clip<GFX.Clip[4].Count)?GFX.Clip[4].Left[clip]:1000);
                        } else {
                            DrawMode=GFX.Clip[4].DrawMode[clip-1];
                            next_clip=GFX.Clip[4].Right[clip-1];
                            GFX.ClipColors=!(DrawMode&1);
                            if(BG.EnableMath && (PPU.OBJ[S].Palette&4) && (DrawMode&2)){
                                DrawTile=GFX.DrawTileMath;
                                DrawClippedTile=GFX.DrawClippedTileMath;
                            } else {
                                DrawTile=GFX.DrawTileNomath;
                                DrawClippedTile=GFX.DrawClippedTileNomath;
                            }
                        }
                    }

                    if(x==X && x+8<next_clip){
                        if(DrawMode) DrawTile(BaseTile|TileX, O, TileLine, 1);
                        x+=8;
                    } else {
                        int w=(next_clip<=X+8)?next_clip-x:X+8-x;
                        if(DrawMode) DrawClippedTile(BaseTile|TileX, O, x-X, w, TileLine, 1);
                        x+=w;
                    }
                }
            }
        }
    }
}

#define TILE_PLUS(t, x) (((t)&0xfc00)|((t+x)&0x3ff))

static void DrawBackground(int bg, uint8 Zh, uint8 Zl){
    CHECK_SOUND();

    BG.TileAddress = PPU.BG[bg].NameBase<<1;

    uint32 Tile;
    uint16 *SC0, *SC1, *SC2, *SC3;

    SC0=(uint16*)&Memory.VRAM[PPU.BG[bg].SCBase<<1];
    SC1=(PPU.BG[bg].SCSize&1)?SC0+1024:SC0;
    if(SC1>=(uint16*)(Memory.VRAM+0x10000)) SC1-=0x8000;
    SC2=(PPU.BG[bg].SCSize&2)?SC1+1024:SC0;
    if(SC2>=(uint16*)(Memory.VRAM+0x10000)) SC2-=0x8000;
    SC3=(PPU.BG[bg].SCSize&1)?SC2+1024:SC2;
    if(SC3>=(uint16*)(Memory.VRAM+0x10000)) SC3-=0x8000;

    uint32 Lines;
    int OffsetMask = (BG.TileSizeH==16)?0x3ff:0x1ff;
    int OffsetShift = (BG.TileSizeV==16)?4:3;
    int PixWidth = IPPU.DoubleWidthPixels?2:1;

    void (*DrawTile)(uint32,uint32,uint32,uint32);
    void (*DrawClippedTile)(uint32,uint32,uint32,uint32,uint32,uint32);
    for(int clip=0; clip<GFX.Clip[bg].Count; clip++){
        GFX.ClipColors=!(GFX.Clip[bg].DrawMode[clip]&1);
        if(BG.EnableMath && (GFX.Clip[bg].DrawMode[clip]&2)){
            DrawTile=GFX.DrawTileMath;
            DrawClippedTile=GFX.DrawClippedTileMath;
        } else {
            DrawTile=GFX.DrawTileNomath;
            DrawClippedTile=GFX.DrawClippedTileNomath;
        }

        for(uint32 Y = GFX.StartY; Y <= GFX.EndY; Y += Lines) {
            uint32 Y2 = IPPU.Interlace?Y*2+GFX.InterlaceFrame:Y;
            uint32 VOffset = LineData[Y].BG[bg].VOffset+(IPPU.Interlace?1:0);
            uint32 HOffset = LineData[Y].BG[bg].HOffset;
            int VirtAlign = ((Y2 + VOffset)&7)>>(IPPU.Interlace?1:0);

            for(Lines=1; Lines<GFX.LinesPerTile-VirtAlign; Lines++){
                if((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||
                   (HOffset != LineData [Y + Lines].BG[bg].HOffset))
                    break;
            }
            if(Y+Lines>GFX.EndY) Lines=GFX.EndY-Y+1;
            VirtAlign<<=3;

            uint32 TilemapRow = (VOffset+Y2)>>OffsetShift;
            BG.InterlaceLine = ((VOffset+Y2)&1)<<3;
            uint32 t1, t2;
            if(((VOffset+Y2)&8)){
                t1=16; t2=0;
            } else {
                t1=0; t2=16;
            }
            uint16 *b1;
            uint16 *b2;
            if(TilemapRow&0x20) {
                b1=SC2; b2=SC3;
            } else {
                b1=SC0; b2=SC1;
            }
            b1+=(TilemapRow&0x1f)<<5;
            b2+=(TilemapRow&0x1f)<<5;

            uint32 Left = GFX.Clip[bg].Left[clip];
            uint32 Right = GFX.Clip[bg].Right[clip];
            uint32 Offset = Left*PixWidth+Y*GFX.PPL;
            uint32 HPos = (HOffset+Left)&OffsetMask;
            uint32 HTile = HPos>>3;
            uint16 *t;
            if(BG.TileSizeH==8){
                if(HTile>31)
                    t=b2+(HTile&0x1f);
                else
                    t=b1+HTile;
            } else {
                if(HTile>63)
                    t=b2+((HTile>>1)&0x1f);
                else
                    t=b1+(HTile>>1);
            }

            uint32 Width = Right-Left;
            if(HPos&7) {
                uint32 l=HPos&7;
                uint32 w=8-l;
                if(w>Width) w=Width;
                Offset-=l*PixWidth;
                Tile=READ_WORD(t);
                GFX.Z1=GFX.Z2=(Tile&0x2000)?Zh:Zl;
                if(BG.TileSizeV==16)
                    Tile=TILE_PLUS(Tile, ((Tile&V_FLIP)?t2:t1));
                if(BG.TileSizeH==8){
                    DrawClippedTile(Tile, Offset, l, w, VirtAlign, Lines);
                    t++;
                    if(HTile==31) t=b2;
                    else if(HTile==63) t=b1;
                } else {
                    if(!(Tile&H_FLIP)){
                        DrawClippedTile(TILE_PLUS(Tile, (HTile&1)), Offset, l, w, VirtAlign, Lines);
                    } else {
                        DrawClippedTile(TILE_PLUS(Tile, 1-(HTile&1)), Offset, l, w, VirtAlign, Lines);
                    }
                    t+=HTile&1;
                    if(HTile==63) t=b2;
                    else if(HTile==127) t=b1;
                }
                HTile++;
                Offset+=8*PixWidth;
                Width-=w;
            }
            while(Width>=8){
                Tile=READ_WORD(t);
                GFX.Z1=GFX.Z2=(Tile&0x2000)?Zh:Zl;
                if(BG.TileSizeV==16)
                    Tile=TILE_PLUS(Tile, ((Tile&V_FLIP)?t2:t1));
                if(BG.TileSizeH==8){
                    DrawTile(Tile, Offset, VirtAlign, Lines);
                    t++;
                    if(HTile==31) t=b2;
                    else if(HTile==63) t=b1;
                } else {
                    if(!(Tile&H_FLIP)){
                        DrawTile(TILE_PLUS(Tile, (HTile&1)), Offset, VirtAlign, Lines);
                    } else {
                        DrawTile(TILE_PLUS(Tile, 1-(HTile&1)), Offset, VirtAlign, Lines);
                    }
                    t+=HTile&1;
                    if(HTile==63) t=b2;
                    else if(HTile==127) t=b1;
                }
                HTile++;
                Offset+=8*PixWidth;
                Width-=8;
            }
            if(Width){
                Tile=READ_WORD(t);
                GFX.Z1=GFX.Z2=(Tile&0x2000)?Zh:Zl;
                if(BG.TileSizeV==16)
                    Tile=TILE_PLUS(Tile, ((Tile&V_FLIP)?t2:t1));
                if(BG.TileSizeH==8){
                    DrawClippedTile(Tile, Offset, 0, Width, VirtAlign, Lines);
                } else {
                    if(!(Tile&H_FLIP)){
                        DrawClippedTile(TILE_PLUS(Tile, (HTile&1)), Offset, 0, Width, VirtAlign, Lines);
                    } else {
                        DrawClippedTile(TILE_PLUS(Tile, 1-(HTile&1)), Offset, 0, Width, VirtAlign, Lines);
                    }
                }
            }
        }
    }
}

static void DrawBackgroundMosaic(int bg, uint8 Zh, uint8 Zl){
    CHECK_SOUND();

    BG.TileAddress = PPU.BG[bg].NameBase<<1;

    uint32 Tile;
    uint16 *SC0, *SC1, *SC2, *SC3;

    SC0=(uint16*)&Memory.VRAM[PPU.BG[bg].SCBase<<1];
    SC1=(PPU.BG[bg].SCSize&1)?SC0+1024:SC0;
    if(SC1>=(uint16*)(Memory.VRAM+0x10000)) SC1-=0x8000;
    SC2=(PPU.BG[bg].SCSize&2)?SC1+1024:SC0;
    if(SC2>=(uint16*)(Memory.VRAM+0x10000)) SC2-=0x8000;
    SC3=(PPU.BG[bg].SCSize&1)?SC2+1024:SC2;
    if(SC3>=(uint16*)(Memory.VRAM+0x10000)) SC3-=0x8000;

    int Lines;
    int OffsetMask = (BG.TileSizeH==16)?0x3ff:0x1ff;
    int OffsetShift = (BG.TileSizeV==16)?4:3;
    int PixWidth = IPPU.DoubleWidthPixels?2:1;

    void (*DrawPix)(uint32,uint32,uint32,uint32,uint32,uint32);

    int MosaicStart = ((uint32)GFX.StartY-PPU.MosaicStart)%PPU.Mosaic;
    for(int clip=0; clip<GFX.Clip[bg].Count; clip++){
        GFX.ClipColors=!(GFX.Clip[bg].DrawMode[clip]&1);
        if(BG.EnableMath && (GFX.Clip[bg].DrawMode[clip]&2)){
            DrawPix=GFX.DrawMosaicPixelMath;
        } else {
            DrawPix=GFX.DrawMosaicPixelNomath;
        }

        for(uint32 Y = GFX.StartY-MosaicStart; Y <= GFX.EndY; Y += PPU.Mosaic) {
            uint32 Y2 = IPPU.Interlace?Y*2:Y;
            uint32 VOffset = LineData[Y].BG[bg].VOffset+(IPPU.Interlace?1:0);
            uint32 HOffset = LineData[Y].BG[bg].HOffset;

            Lines=PPU.Mosaic-MosaicStart;
            if(Y+MosaicStart+Lines>GFX.EndY) Lines=GFX.EndY-Y-MosaicStart+1;
            int VirtAlign = (((Y2 + VOffset)&7)>>(IPPU.Interlace?1:0))<<3;

            uint32 TilemapRow = (VOffset+Y2)>>OffsetShift;
            BG.InterlaceLine = ((VOffset+Y2)&1)<<3;
            uint32 t1, t2;
            if(((VOffset+Y2)&8)){
                t1=16; t2=0;
            } else {
                t1=0; t2=16;
            }
            uint16 *b1;
            uint16 *b2;
            if(TilemapRow&0x20) {
                b1=SC2; b2=SC3;
            } else {
                b1=SC0; b2=SC1;
            }
            b1+=(TilemapRow&0x1f)<<5;
            b2+=(TilemapRow&0x1f)<<5;

            uint32 Left = GFX.Clip[bg].Left[clip];
            uint32 Right = GFX.Clip[bg].Right[clip];
            uint32 Offset = Left*PixWidth+(Y+MosaicStart)*GFX.PPL;
            uint32 HPos = (HOffset+Left-(Left%PPU.Mosaic))&OffsetMask;
            uint32 HTile = HPos>>3;
            uint16 *t;
            if(BG.TileSizeH==8){
                if(HTile>31)
                    t=b2+(HTile&0x1f);
                else
                    t=b1+HTile;
            } else {
                if(HTile>63)
                    t=b2+((HTile>>1)&0x1f);
                else
                    t=b1+(HTile>>1);
            }

            uint32 Width = Right-Left;
            HPos&=7;
            while(Left<Right){
                uint32 w=PPU.Mosaic-(Left%PPU.Mosaic);
                if(w>Width) w=Width;
                Tile=READ_WORD(t);
                GFX.Z1=GFX.Z2=(Tile&0x2000)?Zh:Zl;
                if(BG.TileSizeV==16)
                    Tile=TILE_PLUS(Tile, ((Tile&V_FLIP)?t2:t1));
                if(BG.TileSizeH==8){
                    DrawPix(Tile, Offset, VirtAlign, HPos&7, w, Lines);
                } else {
                    if(!(Tile&H_FLIP)){
                        DrawPix(TILE_PLUS(Tile, (HTile&1)), Offset, VirtAlign, HPos&7, w, Lines);
                    } else {
                        DrawPix(TILE_PLUS(Tile, 1-(HTile&1)), Offset, VirtAlign, HPos&7, w, Lines);
                    }
                }
                HPos+=PPU.Mosaic;
                while(HPos>=8){
                    HPos-=8;
                    if(BG.TileSizeH==8){
                        t++;
                        if(HTile==31) t=b2;
                        else if(HTile==63) t=b1;
                    } else {
                        t+=HTile&1;
                        if(HTile==63) t=b2;
                        else if(HTile==127) t=b1;
                    }
                    HTile++;
                }
                Offset+=w*PixWidth;
                Width-=w;
                Left+=w;
            }
            MosaicStart=0;
        }
    }
}

static void DrawBackgroundOffset(int bg, uint8 Zh, uint8 Zl, int VOffOff){
    CHECK_SOUND();

    BG.TileAddress = PPU.BG[bg].NameBase<<1;

    uint32 Tile;
    uint16 *SC0, *SC1, *SC2, *SC3;
    uint16 *BPS0, *BPS1, *BPS2, *BPS3;

    BPS0=(uint16*)&Memory.VRAM[PPU.BG[2].SCBase<<1];
    BPS1=(PPU.BG[2].SCSize&1)?BPS0+1024:BPS0;
    if(BPS1>=(uint16*)(Memory.VRAM+0x10000)) BPS1-=0x8000;
    BPS2=(PPU.BG[2].SCSize&2)?BPS1+1024:BPS0;
    if(BPS2>=(uint16*)(Memory.VRAM+0x10000)) BPS2-=0x8000;
    BPS3=(PPU.BG[2].SCSize&1)?BPS2+1024:BPS2;
    if(BPS3>=(uint16*)(Memory.VRAM+0x10000)) BPS3-=0x8000;

    SC0=(uint16*)&Memory.VRAM[PPU.BG[bg].SCBase<<1];
    SC1=(PPU.BG[bg].SCSize&1)?SC0+1024:SC0;
    if(SC1>=(uint16*)(Memory.VRAM+0x10000)) SC1-=0x8000;
    SC2=(PPU.BG[bg].SCSize&2)?SC1+1024:SC0;
    if(SC2>=(uint16*)(Memory.VRAM+0x10000)) SC2-=0x8000;
    SC3=(PPU.BG[bg].SCSize&1)?SC2+1024:SC2;
    if(SC3>=(uint16*)(Memory.VRAM+0x10000)) SC3-=0x8000;

    int OffsetMask = (BG.TileSizeH==16)?0x3ff:0x1ff;
    int OffsetShift = (BG.TileSizeV==16)?4:3;
    int Offset2Mask = (BG.OffsetSizeH==16)?0x3ff:0x1ff;
    int Offset2Shift = (BG.OffsetSizeV==16)?4:3;
    int OffsetEnableMask = 0x2000<<bg;
    int PixWidth = IPPU.DoubleWidthPixels?2:1;

    void (*DrawTile)(uint32,uint32,uint32,uint32);
    void (*DrawClippedTile)(uint32,uint32,uint32,uint32,uint32,uint32);
    for(int clip=0; clip<GFX.Clip[bg].Count; clip++){
        GFX.ClipColors=!(GFX.Clip[bg].DrawMode[clip]&1);
        if(BG.EnableMath && (GFX.Clip[bg].DrawMode[clip]&2)){
            DrawTile=GFX.DrawTileMath;
            DrawClippedTile=GFX.DrawClippedTileMath;
        } else {
            DrawTile=GFX.DrawTileNomath;
            DrawClippedTile=GFX.DrawClippedTileNomath;
        }

        for(uint32 Y = GFX.StartY; Y <= GFX.EndY; Y++) {
            uint32 Y2 = IPPU.Interlace?Y*2+GFX.InterlaceFrame:Y;
            uint32 VOff = LineData[Y].BG[2].VOffset - 1;
            uint32 HOff = LineData[Y].BG[2].HOffset;

            uint32 HOffsetRow = VOff>>Offset2Shift;
            uint32 VOffsetRow = (VOff+VOffOff)>>Offset2Shift;
            uint16 *s, *s1, *s2;
            if(HOffsetRow&0x20){
                s1=BPS2; s2=BPS3;
            } else {
                s1=BPS0; s2=BPS1;
            }
            s1+=(HOffsetRow&0x1f)<<5;
            s2+=(HOffsetRow&0x1f)<<5;
            s=((VOffsetRow&0x20)?BPS2:BPS0)+((VOffsetRow&0x1f)<<5);
            int32 VOffsetOffset = s-s1;

            uint32 Left = GFX.Clip[bg].Left[clip];
            uint32 Right = GFX.Clip[bg].Right[clip];
            uint32 Offset = Left*PixWidth+Y*GFX.PPL;
            uint32 LineHOffset = LineData[Y].BG[bg].HOffset;
            bool8 left_edge = (Left<(8-(LineHOffset&7)));
            uint32 Width = Right-Left;

            while(Left<Right){
                uint32 VOffset, HOffset;
                if(left_edge){
                    // SNES cannot do OPT for leftmost tile column
                    VOffset = LineData[Y].BG[bg].VOffset;
                    HOffset = LineHOffset;
                    left_edge = FALSE;
                } else {
                    int HOffTile = ((HOff+Left-1)&Offset2Mask)>>3;
                    if(BG.OffsetSizeH==8){
                        if(HOffTile>31)
                            s=s2+(HOffTile&0x1f);
                        else
                            s=s1+HOffTile;
                    } else {
                        if(HOffTile>63)
                            s=s2+((HOffTile>>1)&0x1f);
                        else
                            s=s1+(HOffTile>>1);
                    }
                    uint16 HCellOffset = READ_WORD(s);
                    uint16 VCellOffset;
                    if(VOffOff){
                        VCellOffset = READ_WORD(s+VOffsetOffset);
                    } else {
                        if(HCellOffset&0x8000){
                            VCellOffset=HCellOffset;
                            HCellOffset=0;
                        } else {
                            VCellOffset=0;
                        }
                    }
                    if(VCellOffset&OffsetEnableMask){
                        VOffset=VCellOffset+1;
                    } else {
                        VOffset=LineData[Y].BG[bg].VOffset;
                    }
                    if(HCellOffset&OffsetEnableMask){
                        HOffset=(HCellOffset&~7)|(LineHOffset&7);
                    } else {
                        HOffset=LineHOffset;
                    }
                }
                if(IPPU.Interlace) VOffset++;

                int VirtAlign = (((Y2+VOffset)&7)>>(IPPU.Interlace?1:0))<<3;
                int TilemapRow=(VOffset+Y2)>>OffsetShift;
                BG.InterlaceLine = ((VOffset+Y2)&1)<<3;
                uint32 t1, t2;
                if(((VOffset+Y2)&8)){
                    t1=16; t2=0;
                } else {
                    t1=0; t2=16;
                }
                uint16 *b1, *b2;
                if(TilemapRow&0x20) {
                    b1=SC2; b2=SC3;
                } else {
                    b1=SC0; b2=SC1;
                }
                b1+=(TilemapRow&0x1f)<<5;
                b2+=(TilemapRow&0x1f)<<5;

                uint32 HPos = (HOffset+Left)&OffsetMask;
                uint32 HTile = HPos>>3;
                uint16 *t;
                if(BG.TileSizeH==8){
                    if(HTile>31)
                        t=b2+(HTile&0x1f);
                    else
                        t=b1+HTile;
                } else {
                    if(HTile>63)
                        t=b2+((HTile>>1)&0x1f);
                    else
                        t=b1+(HTile>>1);
                }

                uint32 l=HPos&7;
                uint32 w=8-l;
                if(w>Width) w=Width;
                Offset-=l*PixWidth;
                Tile=READ_WORD(t);
                GFX.Z1=GFX.Z2=(Tile&0x2000)?Zh:Zl;
                if(BG.TileSizeV==16)
                    Tile=TILE_PLUS(Tile, ((Tile&V_FLIP)?t2:t1));
                if(BG.TileSizeH==8){
                    DrawClippedTile(Tile, Offset, l, w, VirtAlign, 1);
                } else {
                    if(!(Tile&H_FLIP)){
                        DrawClippedTile(TILE_PLUS(Tile, (HTile&1)), Offset, l, w, VirtAlign, 1);
                    } else {
                        DrawClippedTile(TILE_PLUS(Tile, 1-(HTile&1)), Offset, l, w, VirtAlign, 1);
                    }
                }
                Left+=w;
                Offset+=8*PixWidth;
                Width-=w;
            }
        }
    }
}

static void DrawBackgroundOffsetMosaic(int bg, uint8 Zh, uint8 Zl, int VOffOff){
    CHECK_SOUND();

    BG.TileAddress = PPU.BG[bg].NameBase<<1;

    uint32 Tile;
    uint16 *SC0, *SC1, *SC2, *SC3;
    uint16 *BPS0, *BPS1, *BPS2, *BPS3;

    BPS0=(uint16*)&Memory.VRAM[PPU.BG[2].SCBase<<1];
    BPS1=(PPU.BG[2].SCSize&1)?BPS0+1024:BPS0;
    if(BPS1>=(uint16*)(Memory.VRAM+0x10000)) BPS1-=0x8000;
    BPS2=(PPU.BG[2].SCSize&2)?BPS1+1024:BPS0;
    if(BPS2>=(uint16*)(Memory.VRAM+0x10000)) BPS2-=0x8000;
    BPS3=(PPU.BG[2].SCSize&1)?BPS2+1024:BPS2;
    if(BPS3>=(uint16*)(Memory.VRAM+0x10000)) BPS3-=0x8000;

    SC0=(uint16*)&Memory.VRAM[PPU.BG[bg].SCBase<<1];
    SC1=(PPU.BG[bg].SCSize&1)?SC0+1024:SC0;
    if(SC1>=(uint16*)(Memory.VRAM+0x10000)) SC1-=0x8000;
    SC2=(PPU.BG[bg].SCSize&2)?SC1+1024:SC0;
    if(SC2>=(uint16*)(Memory.VRAM+0x10000)) SC2-=0x8000;
    SC3=(PPU.BG[bg].SCSize&1)?SC2+1024:SC2;
    if(SC3>=(uint16*)(Memory.VRAM+0x10000)) SC3-=0x8000;

    int Lines;
    int OffsetMask = (BG.TileSizeH==16)?0x3ff:0x1ff;
    int OffsetShift = (BG.TileSizeV==16)?4:3;
    int Offset2Mask = (BG.OffsetSizeH==16)?0x3ff:0x1ff;
    int Offset2Shift = (BG.OffsetSizeV==16)?4:3;
    int OffsetEnableMask = 0x2000<<bg;
    int PixWidth = IPPU.DoubleWidthPixels?2:1;

    void (*DrawPix)(uint32,uint32,uint32,uint32,uint32,uint32);

    int MosaicStart = ((uint32)GFX.StartY-PPU.MosaicStart)%PPU.Mosaic;
    for(int clip=0; clip<GFX.Clip[bg].Count; clip++){
        GFX.ClipColors=!(GFX.Clip[bg].DrawMode[clip]&1);
        if(BG.EnableMath && (GFX.Clip[bg].DrawMode[clip]&2)){
            DrawPix=GFX.DrawMosaicPixelMath;
        } else {
            DrawPix=GFX.DrawMosaicPixelNomath;
        }

        for(uint32 Y = GFX.StartY-MosaicStart; Y <= GFX.EndY; Y += PPU.Mosaic) {
            uint32 Y2 = IPPU.Interlace?Y*2:Y;
            Lines=PPU.Mosaic-MosaicStart;
            if(Y+MosaicStart+Lines>GFX.EndY) Lines=GFX.EndY-Y-MosaicStart+1;

            uint32 VOff = LineData[Y].BG[2].VOffset - 1;
            uint32 HOff = LineData[Y].BG[2].HOffset;

            uint32 HOffsetRow = VOff>>Offset2Shift;
            uint32 VOffsetRow = (VOff+VOffOff)>>Offset2Shift;
            uint16 *s, *s1, *s2;
            if(HOffsetRow&0x20){
                s1=BPS2; s2=BPS3;
            } else {
                s1=BPS0; s2=BPS1;
            }
            s1+=(HOffsetRow&0x1f)<<5;
            s2+=(HOffsetRow&0x1f)<<5;
            s=((VOffsetRow&0x20)?BPS2:BPS0)+((VOffsetRow&0x1f)<<5);
            int32 VOffsetOffset = s-s1;

            uint32 Left = GFX.Clip[bg].Left[clip];
            uint32 Right = GFX.Clip[bg].Right[clip];
            uint32 Offset = Left*PixWidth+(Y+MosaicStart)*GFX.PPL;
            uint32 LineHOffset = LineData[Y].BG[bg].HOffset;
            bool8 left_edge = (Left<(8-(LineHOffset&7)));
            uint32 Width = Right-Left;

            while(Left<Right){
                uint32 VOffset, HOffset;
                if(left_edge){
                    // SNES cannot do OPT for leftmost tile column
                    VOffset = LineData[Y].BG[bg].VOffset;
                    HOffset = LineHOffset;
                    left_edge = FALSE;
                } else {
                    int HOffTile = ((HOff+Left-1)&Offset2Mask)>>3;
                    if(BG.OffsetSizeH==8){
                        if(HOffTile>31)
                            s=s2+(HOffTile&0x1f);
                        else
                            s=s1+HOffTile;
                    } else {
                        if(HOffTile>63)
                            s=s2+((HOffTile>>1)&0x1f);
                        else
                            s=s1+(HOffTile>>1);
                    }
                    uint16 HCellOffset = READ_WORD(s);
                    uint16 VCellOffset;
                    if(VOffOff){
                        VCellOffset = READ_WORD(s+VOffsetOffset);
                    } else {
                        if(HCellOffset&0x8000){
                            VCellOffset=HCellOffset;
                            HCellOffset=0;
                        } else {
                            VCellOffset=0;
                        }
                    }
                    if(VCellOffset&OffsetEnableMask){
                        VOffset=VCellOffset+1;
                    } else {
                        VOffset=LineData[Y].BG[bg].VOffset;
                    }
                    if(HCellOffset&OffsetEnableMask){
                        HOffset=(HCellOffset&~7)|(LineHOffset&7);
                    } else {
                        HOffset=LineHOffset;
                    }
                }
                if(IPPU.Interlace) VOffset++;

                int VirtAlign = (((Y2+VOffset)&7)>>(IPPU.Interlace?1:0))<<3;
                int TilemapRow=(VOffset+Y2)>>OffsetShift;
                BG.InterlaceLine = ((VOffset+Y2)&1)<<3;
                uint32 t1, t2;
                if(((VOffset+Y2)&8)){
                    t1=16; t2=0;
                } else {
                    t1=0; t2=16;
                }
                uint16 *b1, *b2;
                if(TilemapRow&0x20) {
                    b1=SC2; b2=SC3;
                } else {
                    b1=SC0; b2=SC1;
                }
                b1+=(TilemapRow&0x1f)<<5;
                b2+=(TilemapRow&0x1f)<<5;

                uint32 HPos = (HOffset+Left-(Left%PPU.Mosaic))&OffsetMask;
                uint32 HTile = HPos>>3;
                uint16 *t;
                if(BG.TileSizeH==8){
                    if(HTile>31)
                        t=b2+(HTile&0x1f);
                    else
                        t=b1+HTile;
                } else {
                    if(HTile>63)
                        t=b2+((HTile>>1)&0x1f);
                    else
                        t=b1+(HTile>>1);
                }

                uint32 w=PPU.Mosaic-(Left%PPU.Mosaic);
                if(w>Width) w=Width;
                Tile=READ_WORD(t);
                GFX.Z1=GFX.Z2=(Tile&0x2000)?Zh:Zl;
                if(BG.TileSizeV==16)
                    Tile=TILE_PLUS(Tile, ((Tile&V_FLIP)?t2:t1));
                if(BG.TileSizeH==8){
                    DrawPix(Tile, Offset, VirtAlign, HPos&7, w, Lines);
                } else {
                    if(!(Tile&H_FLIP)){
                        DrawPix(TILE_PLUS(Tile, (HTile&1)), Offset, VirtAlign, HPos&7, w, Lines);
                    } else if(!(Tile&V_FLIP)){
                        DrawPix(TILE_PLUS(Tile, 1-(HTile&1)), Offset, VirtAlign, HPos&7, w, Lines);
                    }
                }
                Left+=w;
                Offset+=w*PixWidth;
                Width-=w;
            }
            MosaicStart=0;
        }
    }
}

static inline void DrawBackgroundMode7(int bg, void (*DrawMath)(uint32,uint32,int), void (*DrawNomath)(uint32,uint32,int), int D){
    CHECK_SOUND();

    for(int clip=0; clip<GFX.Clip[bg].Count; clip++){
        GFX.ClipColors=!(GFX.Clip[bg].DrawMode[clip]&1);
        if(BG.EnableMath && (GFX.Clip[bg].DrawMode[clip]&2)){
            DrawMath(GFX.Clip[bg].Left[clip], GFX.Clip[bg].Right[clip], D);
        } else {
            DrawNomath(GFX.Clip[bg].Left[clip], GFX.Clip[bg].Right[clip], D);
        }
    }
}

static inline void DrawBackdrop(void){
    CHECK_SOUND();

    uint32 Offset=GFX.StartY*GFX.PPL;

    for(int clip=0; clip<GFX.Clip[5].Count; clip++){
        GFX.ClipColors=!(GFX.Clip[5].DrawMode[clip]&1);
        if(BG.EnableMath && (GFX.Clip[5].DrawMode[clip]&2)){
            GFX.DrawBackdropMath(Offset, GFX.Clip[5].Left[clip], GFX.Clip[5].Right[clip]);
        } else {
            GFX.DrawBackdropNomath(Offset, GFX.Clip[5].Left[clip], GFX.Clip[5].Right[clip]);
        }
    }
}

static inline void RenderScreen(bool8 sub){
    uint8 BGActive;
    int D;

    if (!sub) {
        GFX.S = GFX.Screen;
        GFX.DB = GFX.ZBuffer;
        GFX.Clip = IPPU.Clip[0];
        BGActive=Memory.FillRAM[0x212c] & ~Settings.BG_Forced;
        if(GFX.DoInterlace && GFX.InterlaceFrame) GFX.S+=GFX.RealPPL;
        D=32;
    } else {
        GFX.S = GFX.SubScreen;
        GFX.DB = GFX.SubZBuffer;
        GFX.Clip = IPPU.Clip[1];
        BGActive=Memory.FillRAM[0x212d] & ~Settings.BG_Forced;
        D=(Memory.FillRAM[0x2130]&2)<<4; // 'do math' depth flag
    }

    if(BGActive&0x10){
        BG.TileAddress = PPU.OBJNameBase;
        BG.NameSelect = PPU.OBJNameSelect;
        BG.EnableMath = !sub && (Memory.FillRAM[0x2131]&0x10);
        BG.StartPalette = 128;
        S9xSelectTileConverter(4, FALSE, sub, FALSE);
        S9xSelectTileRenderers(PPU.BGMode, sub, TRUE);
        DrawOBJS(D+4);
    }

    BG.NameSelect = 0;
    S9xSelectTileRenderers(PPU.BGMode, sub, FALSE);
#define DO_BG(n, pal, depth, hires, offset, Zh, Zl, voffoff) \
            if(BGActive&(0x01<<n)){ \
                BG.StartPalette = pal; \
                BG.EnableMath = !sub && (Memory.FillRAM[0x2131]&(1<<n)); \
                BG.TileSizeH = (!hires && PPU.BG[n].BGSize)?16:8; \
                BG.TileSizeV = (PPU.BG[n].BGSize)?16:8; \
                S9xSelectTileConverter(depth, hires, sub, PPU.BGMosaic[n]); \
                if(offset){ \
                    BG.OffsetSizeH = (!hires && PPU.BG[2].BGSize)?16:8; \
                    BG.OffsetSizeV = (PPU.BG[2].BGSize)?16:8; \
                    if(PPU.BGMosaic[n] && (hires || PPU.Mosaic>1)) \
                        DrawBackgroundOffsetMosaic(n, D+Zh, D+Zl, voffoff); \
                    else \
                        DrawBackgroundOffset(n, D+Zh, D+Zl, voffoff); \
                } else { \
                    if(PPU.BGMosaic[n] && (hires || PPU.Mosaic>1)) \
                        DrawBackgroundMosaic(n, D+Zh, D+Zl); \
                    else \
                        DrawBackground(n, D+Zh, D+Zl); \
                } \
            }
    switch(PPU.BGMode){
      case 0:
        DO_BG(0,  0, 2, FALSE, FALSE, 15, 11, 0);
        DO_BG(1, 32, 2, FALSE, FALSE, 14, 10, 0);
        DO_BG(2, 64, 2, FALSE, FALSE,  7,  3, 0);
        DO_BG(3, 96, 2, FALSE, FALSE,  6,  2, 0);
        break;
      case 1:
        DO_BG(0, 0, 4, FALSE, FALSE, 15, 11, 0);
        DO_BG(1, 0, 4, FALSE, FALSE, 14, 10, 0);
        DO_BG(2, 0, 2, FALSE, FALSE, (PPU.BG3Priority?17:7),  3, 0);
        break;
      case 2:
        DO_BG(0, 0, 4, FALSE, TRUE, 15, 7, 8);
        DO_BG(1, 0, 4, FALSE, TRUE, 11, 3, 8);
        break;
      case 3:
        DO_BG(0, 0, 8, FALSE, FALSE, 15, 7, 0);
        DO_BG(1, 0, 4, FALSE, FALSE, 11, 3, 0);
        break;
      case 4:
        DO_BG(0, 0, 8, FALSE, TRUE, 15, 7, 0);
        DO_BG(1, 0, 2, FALSE, TRUE, 11, 3, 0);
        break;
      case 5:
        DO_BG(0, 0, 4, TRUE, FALSE, 15, 7, 0);
        DO_BG(1, 0, 2, TRUE, FALSE, 11, 3, 0);
        break;
      case 6:
        DO_BG(0, 0, 4, TRUE, TRUE, 15, 7, 8);
        break;
      case 7:
        if(BGActive&0x01){
            BG.EnableMath = !sub && (Memory.FillRAM[0x2131]&1);
            DrawBackgroundMode7(0, GFX.DrawMode7BG1Math, GFX.DrawMode7BG1Nomath,D);
        }
        if((Memory.FillRAM[0x2133]&0x40) && (BGActive&0x02)){
            BG.EnableMath = !sub && (Memory.FillRAM[0x2131]&2);
            DrawBackgroundMode7(1, GFX.DrawMode7BG2Math, GFX.DrawMode7BG2Nomath,D);
        }
        break;
    }
#undef DO_BG

    BG.EnableMath = !sub && (Memory.FillRAM[0x2131]&0x20);
    DrawBackdrop();
}


void S9xUpdateScreen() {
    if(IPPU.OBJChanged) S9xSetupOBJ();
    // XXX: Check ForceBlank? Or anything else?
    PPU.RangeTimeOver |= GFX.OBJLines[GFX.EndY].RTOFlags;

    GFX.StartY = IPPU.PreviousLine;
    if ((GFX.EndY = IPPU.CurrentLine - 1) >= PPU.ScreenHeight)
        GFX.EndY = PPU.ScreenHeight - 1;

    if(!PPU.ForcedBlanking){
        /* If force blank, may as well completely skip all this. We only did
         * the OBJ because (AFAWK) the RTO flags are updated even during
         * force-blank */

        if(PPU.RecomputeClipWindows) {
            ComputeClipWindows();
            PPU.RecomputeClipWindows=FALSE;
        }

        if (Settings.SupportHiRes){
            if(!IPPU.DoubleWidthPixels && (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.PseudoHires || IPPU.Interlace || IPPU.InterlaceOBJ)){
#if defined (USE_GLIDE) || defined (USE_OPENGL)
                if (
#ifdef USE_GLIDE
                    (Settings.GlideEnable && GFX.RealPPL == 256) ||
#endif
#ifdef USE_OPENGL
                    (Settings.OpenGLEnable && GFX.RealPPL == 256) ||
#endif
                    0)
                {
                    // Have to back out of the speed up hack where the low res.
                    // SNES image was rendered into a 256x239 sized buffer,
                    // ignoring the true, larger size of the buffer.
                    GFX.RealPPL = GFX.Pitch>>1;
                    for (register int32 y = (int32) GFX.StartY - 1; y >= 0; y--){
                        register uint16 *p = GFX.Screen + y * GFX.PPL + 255;
                        register uint16 *q = GFX.Screen + y * GFX.RealPPL + 510;
                        for (register int x = 255; x >= 0; x--, p--, q -= 2)
                            *q = *(q + 1) = *p;
                    }
                    GFX.PPL = GFX.RealPPL; // = GFX.Pitch>>1 above
                } else
#endif
                {
                    // Have to back out of the regular speed hack
                    for (register uint32 y = 0; y < GFX.StartY; y++)
                    {
                        register uint16 *p = GFX.Screen + y * GFX.PPL + 255;
                        register uint16 *q = GFX.Screen + y * GFX.PPL + 510;

                        for (register int x = 255; x >= 0; x--, p--, q -= 2)
                            *q = *(q + 1) = *p;
                    }
                }
                IPPU.DoubleWidthPixels = TRUE;
                IPPU.RenderedScreenWidth = 512;
            }
            if(!IPPU.DoubleHeightPixels && (IPPU.Interlace || IPPU.InterlaceOBJ)){
                IPPU.DoubleHeightPixels = TRUE;
                GFX.PPL=GFX.RealPPL<<1;
                IPPU.RenderedScreenHeight=PPU.ScreenHeight<<1;
                GFX.DoInterlace=2;
                for (register int32 y = (int32) GFX.StartY - 1; y >= 0; y--){
                    memmove (GFX.Screen + y * GFX.PPL,
                             GFX.Screen + y * GFX.RealPPL,
                             IPPU.RenderedScreenWidth*sizeof(uint16));
                }
            }
        }

        if((Memory.FillRAM[0x2130]&0x30)!=0x30 && (Memory.FillRAM[0x2131]&0x3f))
            GFX.FixedColour = BUILD_PIXEL(IPPU.XB[PPU.FixedColourRed],
                                          IPPU.XB[PPU.FixedColourGreen],
                                          IPPU.XB[PPU.FixedColourBlue]);

        if(PPU.BGMode==5 || PPU.BGMode==6 || IPPU.PseudoHires ||
           ((Memory.FillRAM[0x2130]&0x30)!=0x30 && (Memory.FillRAM[0x2130]&2) &&
            (Memory.FillRAM[0x2131]&0x3f) && (Memory.FillRAM[0x212d]&0x1f))){
            /* If hires (Mode 5/6 or pseudo-hires) or math is to be done
             * involving the subscreen, then we need to render the subscreen...
             */
            RenderScreen(TRUE);
        }
        RenderScreen(FALSE);
    } else {
        GFX.S = GFX.Screen+GFX.StartY*GFX.PPL;
        if(GFX.DoInterlace && GFX.InterlaceFrame) GFX.S+=GFX.RealPPL;
        uint16 black=BUILD_PIXEL(0,0,0);
        for(uint32 l=GFX.StartY; l<=GFX.EndY; l++, GFX.S+=GFX.PPL){
            for(int x=0; x<IPPU.RenderedScreenWidth; x++){
                GFX.S[x]=black;
            }
        }
    }
    IPPU.PreviousLine = IPPU.CurrentLine;
}

extern bool unfreezing_from_stream;

void S9xSetInfoString (const char *string)
{
	if(Settings.InitialInfoStringTimeout > 0)
	{
		GFX.InfoString = string;
		GFX.InfoStringTimeout = Settings.InitialInfoStringTimeout;

		if (Settings.Paused && !unfreezing_from_stream)
		{
			//refresh screen to show new message immediately
			S9xReRefresh();
		}
	}
}

static inline void FontPixToScreen(char p, uint16 *s)
{
	if(p == '#')
	{
		*s = Settings.DisplayColor;
	}
	else if(p == '.')
	{
		static const uint16 black = BUILD_PIXEL(0,0,0);
		*s = black;
	}
}

void DisplayChar(uint16 *s, uint8 c) {
    int line = ((c - 32) >> 4) * display_fontheight;
    int offset = ((c - 32) & 15) * display_fontwidth;
    int h, w;
	if(!display_paramsinited) display_ppl = Settings.OpenGLEnable ? IPPU.RenderedScreenWidth : GFX.RealPPL;
	if(display_hfontaccessscale == 1 && display_vfontaccessscale == 1) {
		for(h=0; h<display_fontheight; h++, line++, s+=display_ppl-display_fontwidth)
			for(w=0; w<display_fontwidth; w++, s++)
				FontPixToScreen(font [(line)] [(offset + w)], s);
	} else {
		for(h=0; h<display_fontheight; h++, line++, s+=display_ppl-display_fontwidth)
			for(w=0; w<display_fontwidth; w++, s++)
				FontPixToScreen(font [(line)/display_vfontaccessscale] [(offset + w)/display_hfontaccessscale], s);
	}
}

static void S9xDisplayFrameRate ()
{
    char string[10];
#ifdef DEBUGGER
    int len = 8;
    sprintf(string, "%02d/%02d %02d", (int)IPPU.DisplayedRenderedFrameCount, (int)Memory.ROMFramesPerSecond, (int)IPPU.FrameCount);
#else
    int len = 5;
    sprintf(string, "%02d/%02d", (int)IPPU.DisplayedRenderedFrameCount, (int)Memory.ROMFramesPerSecond);
#endif

	DisplayString(string, 1, 1+display_width-display_fontheight*len, false);
}

static void S9xDisplayString (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap)
{
	if(linesFromBottom <= 0)
		linesFromBottom = 1;

	uint16 *Screen = display_screen // text draw position, starting on the screen
                  + pixelsFromLeft // with this much horizontal offset
//				    * (Settings.SixteenBit ? 2 : 1)
                  + (display_height - display_fontheight * linesFromBottom) // and this much vertical offset
				    * display_ppl;

    int len = strlen(string);
    int max_chars = display_width / (display_fontwidth-display_hfontaccessscale);
    int char_count = 0;
	int prev_hfont_access_scale = display_hfontaccessscale;

	// squash if it won't fit on 1 line and we're drawing greater than 1x scale and we're not allowing wrapping
	while(len > max_chars && !allowWrap && display_hfontaccessscale > 1)
	{
		display_fontwidth /= display_hfontaccessscale;
		display_hfontaccessscale--;
		display_fontwidth *= display_hfontaccessscale;

		max_chars = display_width / (display_fontwidth-display_hfontaccessscale);
	}

	// loop through and draw the characters
	for(int i = 0 ; i < len ; i++, char_count++)
	{
		if(char_count >= max_chars || (unsigned char)string[i] < 32)
		{
			if(!allowWrap)
				break;

			Screen -= /*Settings.SixteenBit ? (display_fontwidth-display_hfontaccessscale)*sizeof(uint16)*max_chars :*/ (display_fontwidth-display_hfontaccessscale)*max_chars;
			Screen += display_fontheight * display_ppl;
			if(Screen >= display_screen + display_ppl * display_height)
				break;

			char_count -= max_chars;
		}
		if((unsigned char) string[i]<32) continue;

		DisplayChar(Screen, string[i]);
		Screen += /*Settings.SixteenBit ? (display_fontwidth-display_hfontaccessscale)*sizeof(uint16) :*/ (display_fontwidth-display_hfontaccessscale);
	}

	// revert temporary change to font scale, if any
	if(display_hfontaccessscale != prev_hfont_access_scale)
	{
		display_hfontaccessscale = prev_hfont_access_scale;
		display_fontwidth *= display_hfontaccessscale;
	}
}

// input display
static void S9xDisplayPressedKeys ()
{
	uint16 MovieGetJoypad(int i);
	bool MovieGetMouse(int i, uint8 out [5]);
	bool MovieGetScope(int i, uint8 out [6]);
	bool MovieGetJustifier(int i, uint8 out [11]);

	enum controllers controller;
	int8 ids[4];

	int line = 1;
	const static char KeyMap[]=  {'0','1','2','R','L','X','A','>','<','v','^','S','s','Y','B'};
	const static int KeyOrder[]={8,10,7,9, 0, 6,14,13,5, 1, 4,3, 2, 11,12}; // < ^ > v   A B Y X  L R  S s
	char string[255];
	int len;

	S9xGetController(1, &controller, &ids[0],&ids[1],&ids[2],&ids[3]);
	bool singlePlayer = (controller == CTL_NONE || (controller == CTL_JOYPAD && !(MovieGetJoypad(1) & 0xffff)));

	for(int port = 0; port < 2; port++)
    {
		S9xGetController(port, &controller, &ids[0],&ids[1],&ids[2],&ids[3]);
		for(int idid = 0; idid < 4; idid++)
		{
			const int id = ids[idid];
			if(id == -1)
				continue;

			bool skip = false;
			switch(controller)
			{
				default: {
					skip = true;
				}	break;

				case CTL_MP5:
					singlePlayer = false;
					// no break:
				case CTL_JOYPAD: {
					if(line < id+1)
						line = id+1;
					uint16 tempJoypad = MovieGetJoypad(id);
					if(!(tempJoypad & 0xffff) && id > 0)
						skip = true;
					else
					{
						sprintf(string, !singlePlayer?"P%d:                    ":"                       ",id+1);

						for (int i=0; i < 15; i++)
						{
							int j = KeyOrder[i];
							int mask = (1 << (j+1));
							string[strlen("P?: ")+i]= ((tempJoypad & mask)!=0) ? KeyMap[j] : ' ';
						}
					}
				}	break;

				case CTL_MOUSE: {
					uint8 buf [5] = {0};
					MovieGetMouse(id, buf);
					int16 x = ((uint16*)buf)[0];
					int16 y = ((uint16*)buf)[1];
					uint8 buttons = buf[4];
					//if(delta_x < 0) delta_x = 128|-delta_x;
					//if(delta_y < 0) delta_y = 128|-delta_y;
					sprintf(string, "(%4d,%4d) %c%c", x, y, (buttons&0x40)?'L':' ', (buttons&0x80)?'R':' ');
				}	break;

				case CTL_SUPERSCOPE: {
					uint8 buf [6] = {0};
					MovieGetScope(id, buf);
					int16 x = ((uint16*)buf)[0];
					int16 y = ((uint16*)buf)[1];
					uint8 buttons = buf[4];
					sprintf(string, "(%3d,%3d) %c%c%c%c", x, y, (buttons&0x80)?'F':' ', (buttons&0x40)?'C':' ', (buttons&0x20)?'T':' ', (buttons&0x10)?'P':' ');
				}	break;

				case CTL_JUSTIFIER: {
					uint8 buf [11] = {0};
					MovieGetJustifier(port, buf);
					int16 x1 = ((uint16*)buf)[0];
					int16 x2 = ((uint16*)buf)[1];
					int16 y1 = ((uint16*)buf)[2];
					int16 y2 = ((uint16*)buf)[3];
					uint8 buttons = buf[8];
					bool8 offscreen1 = buf[9];
					bool8 offscreen2 = buf[10];
					if(id == 1)
						sprintf(string, "(%3d,%3d) %c%c%c / (%3d,%3d) %c%c%c", x1, y1, (buttons&0x80)?'T':' ', (buttons&0x20)?'S':' ', offscreen1?'O':' ', x2, y2, (buttons&0x40)?'T':' ', (buttons&0x10)?'S':' ', offscreen2?'O':' ');
					else
						sprintf(string, "(%3d,%3d) %c%c%c", x1, y1, (buttons&0x80)?'T':' ', (buttons&0x20)?'S':' ', offscreen1?'O':' ');
				}	break;
			}

			if(skip)
				continue;

			len=strlen(string);

			DisplayString(string, line, 1, false);

			line++;
		}
	}
}

void S9xDisplayMessages(uint16 *screen, int ppl, int width, int height, int scale)
{
	display_screen = screen;
	display_ppl = ppl;
	display_width = width;
	display_height = height;
	display_fontwidth = font_width * scale;
	display_fontheight = font_height * scale;
	display_hfontaccessscale = scale;
	display_vfontaccessscale = scale;
	display_paramsinited = TRUE;

	if (Settings.DisplayFrameRate)
		S9xDisplayFrameRate();

    if (Settings.DisplayPressedKeys==2)
        S9xDisplayPressedKeys();

    if (GFX.FrameDisplay && S9xMovieActive()
#ifdef NETPLAY_SUPPORT
	|| Settings.NetPlay
#endif
	)
		DisplayString(GFX.FrameDisplayString, 4, 1, false);

	if (GFX.InfoString && *GFX.InfoString)
		DisplayString(GFX.InfoString, 5, 1, !GFX.FrameDisplay || (!S9xMovieActive()
#ifdef NETPLAY_SUPPORT
		&& !Settings.NetPlay
#endif
		));

	if (Settings.DisplayWatchedAddresses)
		S9xDisplayWatchedAddresses();

	display_paramsinited = FALSE;
	display_fontwidth = font_width;
	display_fontheight = font_height;
	display_hfontaccessscale = 1;
	display_vfontaccessscale = 1;
}

#include "crosshairs.h"

static uint16 get_crosshair_color(uint8 color){
    switch(color&15){
      case 0: return BUILD_PIXEL(0,0,0);    // transparent, shouldn't be used
      case 1: return BUILD_PIXEL(0,0,0);    // Black
      case 2: return BUILD_PIXEL(8,8,8);    // 25Grey
      case 3: return BUILD_PIXEL(16,16,16); // 50Grey
      case 4: return BUILD_PIXEL(23,23,23); // 75Grey
      case 5: return BUILD_PIXEL(31,31,31); // White
      case 6: return BUILD_PIXEL(31,0,0);   // Red
      case 7: return BUILD_PIXEL(31,16,0);  // Orange
      case 8: return BUILD_PIXEL(31,31,0);  // Yellow
      case 9: return BUILD_PIXEL(0,31,0);   // Green
      case 10: return BUILD_PIXEL(0,31,31); // Cyan
      case 11: return BUILD_PIXEL(0,23,31); // Sky
      case 12: return BUILD_PIXEL(0,0,31);  // Blue
      case 13: return BUILD_PIXEL(23,0,31); // Violet
      case 14: return BUILD_PIXEL(31,0,31); // Magenta
      case 15: return BUILD_PIXEL(31,0,16); // Purple
    }
    return 0; // stupid compiler warning
}

void S9xDrawCrosshair(const char *crosshair, uint8 fgcolor, uint8 bgcolor, int16 x, int16 y) {
    int16 r, rx=1, c, cx=1, W, H, rws;

    W=256; H=PPU.ScreenHeight;
    x-=7; y-=7;
    if(IPPU.DoubleWidthPixels){ cx=2; x*=2; W*=2; }
    if(IPPU.DoubleHeightPixels){ rx=2; y*=2; H*=2; }
    if(crosshair==NULL) return;
    if(GFX.Screen==NULL) return;
    if(Settings.StopEmulation) return;
    uint16 fg, bg;
    fg=get_crosshair_color(fgcolor);
    bg=get_crosshair_color(bgcolor);
#if (defined(__unix) || defined(__linux) || defined(__sun) || defined(__DJGPP)) // XXX: FIXME: why does it crash without this on Linux port? There are no out-of-bound writes without it...
	if(x >= 0 && y >= 0)
#endif
	{
      uint16 *s = GFX.Screen + y * GFX.RealPPL + x;
      rws = Settings.OpenGLEnable ? IPPU.RenderedScreenWidth : GFX.RealPPL;
      for(r=0; r<15*rx; r++, s+=rws-15*cx) {
          if(y+r<0){ s+=15*cx; continue; } // if y is negative, skip line
          if(y+r>=H) break;                // if y is past bottom, stop
          for(c=0; c<15*cx; c++, s++) {
              if(x+c<0 || s<GFX.Screen) continue; // if x is negative or s is invalid, skip pixel
              if(x+c>=W){ s+=15*cx-c; break; }    // if x is past right, go to next line
              uint8 p = crosshair[(r/rx)*15+(c/cx)];

              if(p == '#' && fgcolor) {
                  *s=(fgcolor&0x10)?COLOR_ADD1_2(fg,*s):fg;
              } else if(p == '.' && bgcolor) {
                  *s=(bgcolor&0x10)?COLOR_ADD1_2(*s,bg):bg;
              }
          }
      }
   }
}


#ifdef GFX_MULTI_FORMAT

#define _BUILD_PIXEL(F) \
uint32 BuildPixel##F(uint32 R, uint32 G, uint32 B) \
{ \
    return (BUILD_PIXEL_##F(R,G,B)); \
}\
uint32 BuildPixel2##F(uint32 R, uint32 G, uint32 B) \
{ \
    return (BUILD_PIXEL2_##F(R,G,B)); \
} \
void DecomposePixel##F(uint32 pixel, uint32 &R, uint32 &G, uint32 &B) \
{ \
    DECOMPOSE_PIXEL_##F(pixel,R,G,B); \
}

_BUILD_PIXEL(RGB565)
_BUILD_PIXEL(RGB555)
_BUILD_PIXEL(BGR565)
_BUILD_PIXEL(BGR555)
_BUILD_PIXEL(GBR565)
_BUILD_PIXEL(GBR555)
_BUILD_PIXEL(RGB5551)

#define _BUILD_SETUP(F) \
GFX.BuildPixel = BuildPixel##F; \
GFX.BuildPixel2 = BuildPixel2##F; \
GFX.DecomposePixel = DecomposePixel##F; \
RED_LOW_BIT_MASK = RED_LOW_BIT_MASK_##F; \
GREEN_LOW_BIT_MASK = GREEN_LOW_BIT_MASK_##F; \
BLUE_LOW_BIT_MASK = BLUE_LOW_BIT_MASK_##F; \
RED_HI_BIT_MASK = RED_HI_BIT_MASK_##F; \
GREEN_HI_BIT_MASK = GREEN_HI_BIT_MASK_##F; \
BLUE_HI_BIT_MASK = BLUE_HI_BIT_MASK_##F; \
MAX_RED = MAX_RED_##F; \
MAX_GREEN = MAX_GREEN_##F; \
MAX_BLUE = MAX_BLUE_##F; \
GREEN_HI_BIT = ((MAX_GREEN_##F + 1) >> 1); \
SPARE_RGB_BIT_MASK = SPARE_RGB_BIT_MASK_##F; \
RGB_LOW_BITS_MASK = (RED_LOW_BIT_MASK_##F | \
 		     GREEN_LOW_BIT_MASK_##F | \
		     BLUE_LOW_BIT_MASK_##F); \
RGB_HI_BITS_MASK = (RED_HI_BIT_MASK_##F | \
		    GREEN_HI_BIT_MASK_##F | \
		    BLUE_HI_BIT_MASK_##F); \
RGB_HI_BITS_MASKx2 = ((RED_HI_BIT_MASK_##F | \
		       GREEN_HI_BIT_MASK_##F | \
		       BLUE_HI_BIT_MASK_##F) << 1); \
RGB_REMOVE_LOW_BITS_MASK = ~RGB_LOW_BITS_MASK; \
FIRST_COLOR_MASK = FIRST_COLOR_MASK_##F; \
SECOND_COLOR_MASK = SECOND_COLOR_MASK_##F; \
THIRD_COLOR_MASK = THIRD_COLOR_MASK_##F; \
ALPHA_BITS_MASK = ALPHA_BITS_MASK_##F; \
FIRST_THIRD_COLOR_MASK = FIRST_COLOR_MASK | THIRD_COLOR_MASK; \
TWO_LOW_BITS_MASK = RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 1); \
HIGH_BITS_SHIFTED_TWO_MASK = (( (FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK) & \
                                ~TWO_LOW_BITS_MASK ) >> 2);

bool8 S9xSetRenderPixelFormat (int format)
{
    extern uint32 current_graphic_format;

    current_graphic_format = format;

    switch (format)
    {
    case RGB565:
	_BUILD_SETUP(RGB565)
	return (TRUE);
    case RGB555:
	_BUILD_SETUP(RGB555)
	return (TRUE);
    case BGR565:
	_BUILD_SETUP(BGR565)
	return (TRUE);
    case BGR555:
	_BUILD_SETUP(BGR555)
	return (TRUE);
    case GBR565:
	_BUILD_SETUP(GBR565)
	return (TRUE);
    case GBR555:
	_BUILD_SETUP(GBR555)
	return (TRUE);
    case RGB5551:
        _BUILD_SETUP(RGB5551)
        return (TRUE);
    default:
	break;
    }
    return (FALSE);
}
#endif

void (*S9xCustomDisplayString) (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap) = NULL;
