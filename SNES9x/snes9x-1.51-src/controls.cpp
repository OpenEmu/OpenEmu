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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>

#include "snes9x.h"
#include "memmap.h"
#include "controls.h"
#include "crosshairs.h"
#include "ppu.h"
#include "display.h"
#include "cpuexec.h"
#include "snapshot.h"
#include "spc7110.h"
#include "movie.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif

#ifdef __WIN32__
#define snprintf _snprintf // needs ANSI compliant name
#endif

using namespace std;

#define NONE           -2
#define MP5            -1
#define JOYPAD0         0
#define JOYPAD1         1
#define JOYPAD2         2
#define JOYPAD3         3
#define JOYPAD4         4
#define JOYPAD5         5
#define JOYPAD6         6
#define JOYPAD7         7
#define MOUSE0          8
#define MOUSE1          9
#define SUPERSCOPE     10
#define ONE_JUSTIFIER  11
#define TWO_JUSTIFIERS 12
#define NUMCTLS        13 // This must be LAST

#define POLL_ALL       NUMCTLS

static map<uint32, s9xcommand_t> keymap;
static set<uint32> pollmap[NUMCTLS+1];
static vector<s9xcommand_t *> multis;
struct exemulti {
    int32 pos;
    bool8 data1;
    s9xcommand_t *script;
};
static set<struct exemulti *> exemultis;

static struct {
    int16 x, y;
    int16 V_adj; bool8 V_var;
    int16 H_adj; bool8 H_var;
    bool8 mapped;
} pseudopointer[8];
static uint8 pseudobuttons[256];

struct crosshair {
    uint8 set;
    uint8 img;
    uint8 fg;
    uint8 bg;
};

static uint8 turbo_time;
static struct {
    uint16 buttons;
    uint16 turbos;
    uint16 toggleturbo;
    uint16 togglestick;
    uint8 turbo_ct;
} joypad[8];
static struct {
    uint8 delta_x, delta_y;
    int16 old_x, old_y;
    int16 cur_x, cur_y;
    uint8 buttons;
    uint32 ID;
    struct crosshair crosshair;
} mouse[2];

#define SUPERSCOPE_FIRE   0x80
#define SUPERSCOPE_CURSOR 0x40
#define SUPERSCOPE_TURBO  0x20
#define SUPERSCOPE_PAUSE  0x10
#define SUPERSCOPE_OFFSCREEN  0x02
static struct {
    int16 x, y;
    uint8 phys_buttons;
    uint8 next_buttons;
    uint8 read_buttons;
    uint32 ID;
    struct crosshair crosshair;
} superscope;

#define JUSTIFIER_TRIGGER 0x80
#define JUSTIFIER_START   0x20
#define JUSTIFIER_SELECT  0x08
static struct {
    int16 x[2], y[2];
    uint8 buttons;
    bool8 offscreen[2];
    uint32 ID[2];
    struct crosshair crosshair[2];
} justifier;
static struct {
    int8 pads[4];
} mp5[2];

uint8 read_idx[2 /* ports */][2 /* per port */];

bool8 pad_read = 0, pad_read_last = 0;

#define FLAG_IOBIT0   (Memory.FillRAM[0x4213]&0x40)
#define FLAG_IOBIT1   (Memory.FillRAM[0x4213]&0x80)
#define FLAG_IOBIT(n) ((n)?(FLAG_IOBIT1):(FLAG_IOBIT0))
static bool8 FLAG_LATCH=false;
static int32 curcontrollers[2]={ NONE, NONE };
static int32 newcontrollers[2]={ JOYPAD0, NONE };

/*******************/

// Note: these should be in asciibetical order!
#define THE_COMMANDS \
    S(BGLayeringHack), \
    S(BeginRecordingMovie), \
    S(ClipWindows), \
    S(Debugger), \
    S(DecEmuTurbo), \
    S(DecFrameRate), \
    S(DecFrameTime), \
    S(DecTurboSpeed), \
    S(DumpSPC7110Log), \
    S(EmuTurbo), \
    S(EndRecordingMovie), \
    S(ExitEmu), \
    S(IncEmuTurbo), \
    S(IncFrameRate), \
    S(IncFrameTime), \
    S(IncTurboSpeed), \
    S(InterpolateSound), \
    S(LoadFreezeFile), \
    S(LoadMovie), \
    S(LoadOopsFile), \
/*    S(Mode7Interpolate),*/ \
    S(Pause), \
    S(QuickLoad000), S(QuickLoad001), S(QuickLoad002), S(QuickLoad003), S(QuickLoad004), S(QuickLoad005), S(QuickLoad006), S(QuickLoad007), S(QuickLoad008), S(QuickLoad009), S(QuickLoad010), \
    S(QuickSave000), S(QuickSave001), S(QuickSave002), S(QuickSave003), S(QuickSave004), S(QuickSave005), S(QuickSave006), S(QuickSave007), S(QuickSave008), S(QuickSave009), S(QuickSave010), \
    S(Reset), \
    S(SaveFreezeFile), \
    S(SaveSPC), \
    S(Screenshot), \
    S(SeekToFrame), \
    S(SoftReset), \
    S(SoundChannel0), S(SoundChannel1), S(SoundChannel2), S(SoundChannel3), S(SoundChannel4), S(SoundChannel5), S(SoundChannel6), S(SoundChannel7), \
    S(SoundChannelsOn), \
    S(SwapJoypads), \
    S(SynchronizeSound), \
    S(ToggleBG0), S(ToggleBG1), S(ToggleBG2), S(ToggleBG3), \
    S(ToggleEmuTurbo), \
    S(ToggleHDMA), \
    S(ToggleSprites), \
    S(ToggleTransparency), \
	// end

#define S(x) x

enum command_numbers {
	THE_COMMANDS
    LAST_COMMAND  // must be last!
};

#undef S
#define S(x) #x

static const char *command_names[LAST_COMMAND+1]={
	THE_COMMANDS
    NULL // This MUST be last!
};

#undef S
#undef THE_COMMANDS

static const char *color_names[32]={
    "Trans", "Black", "25Grey", "50Grey", "75Grey", "White", "Red", "Orange", "Yellow", "Green", "Cyan", "Sky", "Blue", "Violet", "MagicPink", "Purple",
    NULL, "tBlack", "t25Grey", "t50Grey", "t75Grey", "tWhite", "tRed", "tOrange", "tYellow", "tGreen", "tCyan", "tSky", "tBlue", "tViolet", "tMagicPink", "tPurple"
};

static const char *speed_names[4]={ "Var", "Slow", "Med", "Fast" };

static const int ptrspeeds[4]={ 1, 1, 4, 8 };

/*******************/

static char buf[256];
static string& operator+=(string &s, int i){
    snprintf(buf, sizeof(buf), "%d", i);
    s.append(buf);
    return s;
}

static string& operator+=(string &s, double d){
    snprintf(buf, sizeof(buf), "%g", d);
    s.append(buf);
    return s;
}

static void S9xDisplayStateChange(const char *str, bool8 on){
    snprintf(buf, sizeof(buf), "%s %s", str, on?"on":"off");
    S9xSetInfoString(buf);
}

static void DoGunLatch(int x, int y){
    x+=40;
    if(x>295) x=295;
    if(x<40) x=40;
    if(y>PPU.ScreenHeight-1) y=PPU.ScreenHeight-1;
    if(y<0) y=0;

    PPU.GunVLatch = (uint16) (y + 1);
    PPU.GunHLatch = (uint16) x;
}

#define MAP_UNKNOWN -1
#define MAP_NONE    0
#define MAP_BUTTON  1
#define MAP_AXIS    2
#define MAP_POINTER 3
static int maptype(int t){
    switch(t){
      case S9xNoMapping:
        return MAP_NONE;

      case S9xButtonJoypad:
      case S9xButtonMouse:
      case S9xButtonSuperscope:
      case S9xButtonJustifier:
      case S9xButtonCommand:
      case S9xButtonPseudopointer:
      case S9xButtonPort:
      case S9xButtonMulti:
        return MAP_BUTTON;

      case S9xAxisJoypad:
      case S9xAxisPseudopointer:
      case S9xAxisPseudobuttons:
      case S9xAxisPort:
        return MAP_AXIS;

      case S9xPointer:
      case S9xPointerPort:
        return MAP_POINTER;

      default:
        return MAP_UNKNOWN;
    }
}

void S9xControlsReset(void){
    S9xControlsSoftReset();
    mouse[0].buttons&=~0x30;
    mouse[1].buttons&=~0x30;
    justifier.buttons&=~JUSTIFIER_SELECT;
}

void S9xControlsSoftReset(void){
    int i, j;

    for(set<struct exemulti *>::iterator it=exemultis.begin(); it!=exemultis.end(); it++){
        delete *it;
    }
    exemultis.clear();
    for(i=0; i<2; i++){
        for(j=0; j<2; j++){
            read_idx[i][j]=0;
        }
    }
    FLAG_LATCH=false;
}

void S9xUnmapAllControls(void){
    int i;

    S9xControlsReset();
    keymap.clear();
    for(i=0; i<(int)multis.size(); i++){
        free(multis[i]);
    }
    multis.clear();
    for(i=0; i<NUMCTLS+1; i++){
        pollmap[i].clear();
    }
    for(i=0; i<8; i++){
        pseudopointer[i].x=0;
        pseudopointer[i].y=0;
        pseudopointer[i].H_adj=0;
        pseudopointer[i].V_adj=0;
        pseudopointer[i].H_var=0;
        pseudopointer[i].V_var=0;
        pseudopointer[i].mapped=false;
        joypad[i].buttons=0;
        joypad[i].turbos=0;
        joypad[i].turbo_ct=0;
    }
    for(i=0; i<2; i++){
        mouse[i].old_x=mouse[i].old_y=0;
        mouse[i].cur_x=mouse[i].cur_y=0;
        mouse[i].buttons=1;
        mouse[i].ID=InvalidControlID;
        if(!(mouse[i].crosshair.set&1)) mouse[i].crosshair.img=0; // no image for mouse because its only logical position is game-specific, not known by the emulator
        if(!(mouse[i].crosshair.set&2)) mouse[i].crosshair.fg=5;
        if(!(mouse[i].crosshair.set&4)) mouse[i].crosshair.bg=1;
        justifier.x[i]=justifier.y[i]=0;
        justifier.offscreen[i]=0;
        justifier.ID[i]=InvalidControlID;
        if(!(justifier.crosshair[i].set&1)) justifier.crosshair[i].img=4;
        if(!(justifier.crosshair[i].set&2)) justifier.crosshair[i].fg=i?14:12;
        if(!(justifier.crosshair[i].set&4)) justifier.crosshair[i].bg=1;
    }
    justifier.buttons=0;
    superscope.x=superscope.y=0;
    superscope.phys_buttons=0;
    superscope.next_buttons=0;
    superscope.read_buttons=0;
    superscope.ID=InvalidControlID;
    if(!(superscope.crosshair.set&1)) superscope.crosshair.img=2;
    if(!(superscope.crosshair.set&2)) superscope.crosshair.fg=5;
    if(!(superscope.crosshair.set&4)) superscope.crosshair.bg=1;
    ZeroMemory(pseudobuttons, sizeof(pseudobuttons));
    turbo_time=1;
}

void S9xSetController(int port, enum controllers controller, int8 id1, int8 id2, int8 id3, int8 id4){
    if(port<0 || port>1) return;
    switch(controller){
      case CTL_NONE:
        break;
      case CTL_JOYPAD:
        if(id1<0 || id1>7) break;
        newcontrollers[port]=JOYPAD0+id1;
        return;
      case CTL_MOUSE:
        if(id1<0 || id1>1) break;
        if(!Settings.MouseMaster){
            S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select SNES Mouse: MouseMaster disabled");
            break;
        }
        newcontrollers[port]=MOUSE0+id1;
        return;
      case CTL_SUPERSCOPE:
        if(!Settings.SuperScopeMaster){
            S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select SNES Superscope: SuperScopeMaster disabled");
            break;
        }
        newcontrollers[port]=SUPERSCOPE;
        return;
      case CTL_JUSTIFIER:
        if(id1<0 || id1>1) break;
        if(!Settings.JustifierMaster){
            S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select Konami Justifier: JustifierMaster disabled");
            break;
        }
        newcontrollers[port]=ONE_JUSTIFIER+id1;
        return;
      case CTL_MP5:
        if(id1<-1 || id1>7) break;
        if(id2<-1 || id2>7) break;
        if(id3<-1 || id3>7) break;
        if(id4<-1 || id4>7) break;
        if(!Settings.MultiPlayer5Master){
            S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select MP5: MultiPlayer5Master disabled");
            break;
        }
        newcontrollers[port]=MP5;
        mp5[port].pads[0]=(id1<0)?NONE:JOYPAD0+id1;
        mp5[port].pads[1]=(id2<0)?NONE:JOYPAD0+id2;
        mp5[port].pads[2]=(id3<0)?NONE:JOYPAD0+id3;
        mp5[port].pads[3]=(id4<0)?NONE:JOYPAD0+id4;
        return;
      default:
        fprintf(stderr, "Unknown controller type %d\n", controller);
        break;
    }
    newcontrollers[port]=NONE;
    return;

}

bool S9xVerifyControllers(void){
    bool ret=false;
    int port;
    int i, used[NUMCTLS];

    for(i=0; i<NUMCTLS; used[i++]=0);

    for(port=0; port<2; port++){
        switch(i=newcontrollers[port]){
          case MOUSE0: case MOUSE1:
            if(!Settings.MouseMaster){
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select SNES Mouse: MouseMaster disabled");
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            if(used[i]++>0){
                snprintf(buf, sizeof(buf), "Mouse%d used more than once! Disabling extra instances", i-MOUSE0+1);
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, buf);
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            break;
          case SUPERSCOPE:
            if(!Settings.SuperScopeMaster){
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select SNES Superscope: SuperScopeMaster disabled");
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            if(used[i]++>0){
                snprintf(buf, sizeof(buf), "Superscope used more than once! Disabling extra instances");
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, buf);
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            break;
          case ONE_JUSTIFIER: case TWO_JUSTIFIERS:
            if(!Settings.JustifierMaster){
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select Konami Justifier: JustifierMaster disabled");
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            if(used[ONE_JUSTIFIER]++>0){
                snprintf(buf, sizeof(buf), "Justifier used more than once! Disabling extra instances");
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, buf);
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            break;
          case MP5:
            if(!Settings.MultiPlayer5Master){
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, "Cannot select MP5: MultiPlayer5Master disabled");
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            for(i=0; i<4; i++){
                if(mp5[port].pads[i]!=NONE){
                    if(used[mp5[port].pads[i]-JOYPAD0]++>0){
                        snprintf(buf, sizeof(buf), "Joypad%d used more than once! Disabling extra instances", mp5[port].pads[i]-JOYPAD0+1);
                        S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, buf);
                        mp5[port].pads[i]=NONE;
                        ret=true;
                        break;
                    }
                }
            }
            break;
          case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
          case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
            if(used[i-JOYPAD0]++>0){
                snprintf(buf, sizeof(buf), "Joypad%d used more than once! Disabling extra instances", i-JOYPAD0+1);
                S9xMessage(S9X_CONFIG_INFO, S9X_ERROR, buf);
                newcontrollers[port]=NONE;
                ret=true;
                break;
            }
            break;
          default:
            break;
        }
    }

    return ret;
}

void S9xGetController(int port, enum controllers *controller, int8 *id1, int8 *id2, int8 *id3, int8 *id4){
    int i;

    *controller=CTL_NONE;
    *id1=*id2=*id3=*id4=-1;
    if(port<0 || port>1) return;
    switch(i=newcontrollers[port]){
      case MP5:
        *controller=CTL_MP5;
        *id1=(mp5[port].pads[0]==NONE)?-1:mp5[port].pads[0]-JOYPAD0;
        *id2=(mp5[port].pads[1]==NONE)?-1:mp5[port].pads[1]-JOYPAD0;
        *id3=(mp5[port].pads[2]==NONE)?-1:mp5[port].pads[2]-JOYPAD0;
        *id4=(mp5[port].pads[3]==NONE)?-1:mp5[port].pads[3]-JOYPAD0;
        return;

      case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
      case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
        *controller=CTL_JOYPAD;
        *id1=i-JOYPAD0;
        return;

      case MOUSE0: case MOUSE1:
        *controller=CTL_MOUSE;
        *id1=i-MOUSE0;
        return;

      case SUPERSCOPE:
        *controller=CTL_SUPERSCOPE;
		*id1=1;
        return;

      case ONE_JUSTIFIER: case TWO_JUSTIFIERS:
        *controller=CTL_JUSTIFIER;
        *id1=i-ONE_JUSTIFIER;
        return;
    }
}

void S9xReportControllers(void){
    int port, i;
    static char buf[128]; // static because S9xMessage keeps our pointer instead of copying
	char *c=buf;

    S9xVerifyControllers();
    for(port=0; port<2; port++){
        c+=sprintf(c, "Port %d: ", port+1);
        switch(newcontrollers[port]){
          case NONE:
            c+=sprintf(c, "<none>. ");
            break;

          case MP5:
            c+=sprintf(c, "MP5 with pads");
            for(i=0; i<4; i++){
                if(mp5[port].pads[i]==NONE){
                    c+=sprintf(c, " <none>. ");
                } else {
                    c+=sprintf(c, " #%d. ", mp5[port].pads[i]+1-JOYPAD0);
                }
            }
            break;

          case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
          case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
            c+=sprintf(c, "Pad #%d. ", (int)(newcontrollers[port]-JOYPAD0+1));
            break;

          case MOUSE0: case MOUSE1:
            c+=sprintf(c, "Mouse #%d. ", (int)(newcontrollers[port]-MOUSE0+1));
            break;

          case SUPERSCOPE:
            if(port==0) c+=sprintf(c, "Superscope (cannot fire). ");
            else c+=sprintf(c, "Superscope. ");
            break;

          case ONE_JUSTIFIER:
            if(port==0) c+=sprintf(c, "Blue Justifier (cannot fire). ");
            else c+=sprintf(c, "Blue Justifier. ");
            break;

          case TWO_JUSTIFIERS:
            if(port==0) c+=sprintf(c, "Blue and Pink Justifiers (cannot fire). ");
            else c+=sprintf(c, "Blue and Pink Justifiers. ");
            break;
        }
    }
    S9xMessage(S9X_INFO, S9X_CONFIG_INFO, buf);
}


char *S9xGetCommandName(s9xcommand_t command){
    string s;
    char c;

    switch(command.type){
      case S9xButtonJoypad:
        if(command.button.joypad.buttons==0) return strdup("None");
        if(command.button.joypad.buttons&0x000f) return strdup("None");
        s="Joypad";
        s+=command.button.joypad.idx+1;
        c=' ';
        if(command.button.joypad.toggle){ if(c) s+=c; s+="Toggle"; c=0; }
        if(command.button.joypad.sticky){ if(c) s+=c; s+="Sticky"; c=0; }
        if(command.button.joypad.turbo){ if(c) s+=c; s+="Turbo"; c=0; }

        c=' ';
        if(command.button.joypad.buttons&SNES_UP_MASK){ s+=c; s+="Up"; c='+'; }
        if(command.button.joypad.buttons&SNES_DOWN_MASK){ s+=c; s+="Down"; c='+'; }
        if(command.button.joypad.buttons&SNES_LEFT_MASK){ s+=c; s+="Left"; c='+'; }
        if(command.button.joypad.buttons&SNES_RIGHT_MASK){ s+=c; s+="Right"; c='+'; }
        if(command.button.joypad.buttons&SNES_A_MASK){ s+=c; s+="A"; c='+'; }
        if(command.button.joypad.buttons&SNES_B_MASK){ s+=c; s+="B"; c='+'; }
        if(command.button.joypad.buttons&SNES_X_MASK){ s+=c; s+="X"; c='+'; }
        if(command.button.joypad.buttons&SNES_Y_MASK){ s+=c; s+="Y"; c='+'; }
        if(command.button.joypad.buttons&SNES_TL_MASK){ s+=c; s+="L"; c='+'; }
        if(command.button.joypad.buttons&SNES_TR_MASK){ s+=c; s+="R"; c='+'; }
        if(command.button.joypad.buttons&SNES_START_MASK){ s+=c; s+="Start"; c='+'; }
        if(command.button.joypad.buttons&SNES_SELECT_MASK){ s+=c; s+="Select"; c='+'; }
        break;

      case S9xButtonMouse:
        if(!command.button.mouse.left && !command.button.mouse.right) return strdup("None");
        s="Mouse";
        s+=command.button.mouse.idx+1;
        s+=" ";
        if(command.button.mouse.left) s+="L";
        if(command.button.mouse.right) s+="R";
        break;

      case S9xButtonSuperscope:
        if(!command.button.scope.fire && !command.button.scope.cursor && !command.button.scope.turbo && !command.button.scope.pause && !command.button.scope.aim_offscreen) return strdup("None");
        s="Superscope";
        if(command.button.scope.aim_offscreen) s+=" AimOffscreen";
        c=' ';
        if(command.button.scope.fire){ s+=c; s+="Fire"; c='+'; }
        if(command.button.scope.cursor){ s+=c; s+="Cursor"; c='+'; }
        if(command.button.scope.turbo){ s+=c; s+="ToggleTurbo"; c='+'; }
        if(command.button.scope.pause){ s+=c; s+="Pause"; c='+'; }
        break;

      case S9xButtonJustifier:
        if(!command.button.justifier.trigger && !command.button.justifier.start && !command.button.justifier.aim_offscreen) return strdup("None");
        s="Justifier";
        s+=command.button.justifier.idx+1;
        if(command.button.justifier.aim_offscreen) s+=" AimOffscreen";
        c=' ';
        if(command.button.justifier.trigger){ s+=c; s+="Trigger"; c='+'; }
        if(command.button.justifier.start){ s+=c; s+="Start"; c='+'; }
        break;

      case S9xButtonCommand:
        if(command.button.command>=LAST_COMMAND) return strdup("None");
        return strdup(command_names[command.button.command]);

      case S9xPointer:
        if(!command.pointer.aim_mouse0 && !command.pointer.aim_mouse1 && !command.pointer.aim_scope && !command.pointer.aim_justifier0 && !command.pointer.aim_justifier1) return strdup("None");
        s="Pointer";
        c=' ';
        if(command.pointer.aim_mouse0){ s+=c; s+="Mouse1"; c='+'; }
        if(command.pointer.aim_mouse1){ s+=c; s+="Mouse2"; c='+'; }
        if(command.pointer.aim_scope){ s+=c; s+="Superscope"; c='+'; }
        if(command.pointer.aim_justifier0){ s+=c; s+="Justifier1"; c='+'; }
        if(command.pointer.aim_justifier1){ s+=c; s+="Justifier2"; c='+'; }
        break;

      case S9xButtonPseudopointer:
        if(!command.button.pointer.UD && !command.button.pointer.LR) return strdup("None");
        if(command.button.pointer.UD==-2 || command.button.pointer.LR==-2) return strdup("None");
        s="ButtonToPointer ";
        s+=command.button.pointer.idx+1;
        if(command.button.pointer.UD) s+=(command.button.pointer.UD==1)?'d':'u';
        if(command.button.pointer.LR) s+=(command.button.pointer.LR==1)?'r':'l';
        s+=" ";
        s+=speed_names[command.button.pointer.speed_type];
        break;

      case S9xAxisJoypad:
        s="Joypad";
        s+=command.axis.joypad.idx+1;
        s+=" Axis ";
        switch(command.axis.joypad.axis){
          case 0:
            s+=(command.axis.joypad.invert?"Right/Left":"Left/Right");
            break;
          case 1:
            s+=(command.axis.joypad.invert?"Down/Up":"Up/Down");
            break;
          case 2:
            s+=(command.axis.joypad.invert?"A/Y":"Y/A");
            break;
          case 3:
            s+=(command.axis.joypad.invert?"B/X":"X/B");
            break;
          case 4:
            s+=(command.axis.joypad.invert?"R/L":"L/R");
            break;
          default:
            return strdup("None");
        }
        s+=" T=";
        s+=int((command.axis.joypad.threshold+1)*1000/256)/10.0;
        s+="%";
        break;

      case S9xAxisPseudopointer:
        s="AxisToPointer ";
        s+=command.axis.pointer.idx+1;
        s+=command.axis.pointer.HV?'v':'h';
        s+=" ";
        if(command.axis.pointer.invert) s+="-";
        s+=speed_names[command.axis.pointer.speed_type];
        break;

      case S9xAxisPseudobuttons:
        s="AxisToButtons ";
        s+=command.axis.button.negbutton;
        s+="/";
        s+=command.axis.button.posbutton;
        s+=" T=";
        s+=int((command.axis.button.threshold+1)*1000/256)/10.0;
        s+="%";
        break;

      case S9xButtonPort:
      case S9xAxisPort:
      case S9xPointerPort:
        return strdup("BUG: Port should have handled this instead of calling S9xGetCommandName()");

      case S9xNoMapping:
        return strdup("None");

      case S9xButtonMulti:
        {
            if(command.button.multi_idx>=(int)multis.size()) return strdup("None");
            s="{";
            if(multis[command.button.multi_idx]->multi_press) s="+{";
            bool sep=false;
            for(s9xcommand_t *c=multis[command.button.multi_idx]; c->multi_press!=3; c++){
                if(c->type==S9xNoMapping){
                    s+=";";
                    sep=false;
                } else {
                    if(sep) s+=",";
                    if(c->multi_press==1) s+="+";
                    if(c->multi_press==2) s+="-";
                    s+=S9xGetCommandName(*c);
                    sep=true;
                }
            }
            s+="}";
        }
        break;

      default:
        return strdup("BUG: Unknown command type");
    }

    return strdup(s.c_str());
}

static bool strless(const char *a, const char *b){
    return strcmp(a, b)<0;
}

static int findstr(const char *needle, const char **haystack, int numstr){
    const char **r;
    r=lower_bound(haystack, haystack+numstr, needle, strless);
    if(r>=haystack+numstr || strcmp(needle,*r)) return -1;
    return r-haystack;
}

static int get_threshold(const char **ss){
    int i;
    const char *s=*ss;

    if(s[0]!='T' || s[1]!='=') return -1;
    s+=2;
    i=0;
    if(s[0]=='0'){
        if(s[1]!='.') return -1;
        s++;
    } else {
        do {
            if(*s<'0' || *s>'9') return -1;
            i=i*10+10*(*s-'0');
            if(i>1000) return -1;
            s++;
        } while(*s!='.' && *s!='%');
    }
    if(*s=='.'){
        if(s[1]<'0' || s[1]>'9' || s[2]!='%') return -1;
        i+=s[1]-'0';
    }
    if(i>1000) return -1;
    *ss=s;
    return i;
}

s9xcommand_t S9xGetCommandT(const char *name){
    s9xcommand_t cmd;
    int i, j;
    const char *s;
    ZeroMemory(&cmd, sizeof(cmd));
    cmd.type=S9xBadMapping;
    cmd.multi_press=0;
    cmd.button_norpt=0;

    if(!strcmp(name,"None")){
        cmd.type=S9xNoMapping;
    } else if(!strncmp(name,"Joypad",6)){
        if(name[6]<'1' || name[6]>'8' || name[7]!=' ') return cmd;
        if(!strncmp(name+8,"Axis ",5)){
            cmd.axis.joypad.idx = name[6]-'1';
            s=name+13;
            if(!strncmp(s,"Left/Right ",11)){ j=0; i=0; s+=11; }
            else if(!strncmp(s,"Right/Left ",11)){ j=0; i=1; s+=11; }
            else if(!strncmp(s,"Up/Down ",8)){ j=1; i=0; s+=8; }
            else if(!strncmp(s,"Down/Up ",8)){ j=1; i=1; s+=8; }
            else if(!strncmp(s,"Y/A ",4)){ j=2; i=0; s+=4; }
            else if(!strncmp(s,"A/Y ",4)){ j=2; i=1; s+=4; }
            else if(!strncmp(s,"X/B ",4)){ j=3; i=0; s+=4; }
            else if(!strncmp(s,"B/X ",4)){ j=3; i=1; s+=4; }
            else if(!strncmp(s,"L/R ",4)){ j=4; i=0; s+=4; }
            else if(!strncmp(s,"R/L ",4)){ j=4; i=1; s+=4; }
            else { return cmd; }
            cmd.axis.joypad.axis=j;
            cmd.axis.joypad.invert=i;
            i=get_threshold(&s);
            if(i<0) return cmd;
            cmd.axis.joypad.threshold=(i-1)*256/1000;
            cmd.type=S9xAxisJoypad;
        } else {
            cmd.button.joypad.idx = name[6]-'1';
            s=name+8;
            i=0;
            if((cmd.button.joypad.toggle=strncmp(s,"Toggle",6)?0:1)) s+=i=6;
            if((cmd.button.joypad.sticky=strncmp(s,"Sticky",6)?0:1)) s+=i=6;
            if((cmd.button.joypad.turbo=strncmp(s,"Turbo",5)?0:1)) s+=i=5;
            if(cmd.button.joypad.toggle && !(cmd.button.joypad.sticky || cmd.button.joypad.turbo)) return cmd;
            if(i){
                if(*s!=' ') return cmd;
                s++;
            }

            i=0;
            if(!strncmp(s,"Up",2)){ i|=SNES_UP_MASK; s+=2; if(*s=='+') s++; }
            if(!strncmp(s,"Down",4)){ i|=SNES_DOWN_MASK; s+=4; if(*s=='+') s++; }
            if(!strncmp(s,"Left",4)){ i|=SNES_LEFT_MASK; s+=4; if(*s=='+') s++; }
            if(!strncmp(s,"Right",5)){ i|=SNES_RIGHT_MASK; s+=5; if(*s=='+') s++; }
            if(*s=='A'){ i|=SNES_A_MASK; s++; if(*s=='+') s++; }
            if(*s=='B'){ i|=SNES_B_MASK; s++; if(*s=='+') s++; }
            if(*s=='X'){ i|=SNES_X_MASK; s++; if(*s=='+') s++; }
            if(*s=='Y'){ i|=SNES_Y_MASK; s++; if(*s=='+') s++; }
            if(*s=='L'){ i|=SNES_TL_MASK; s++; if(*s=='+') s++; }
            if(*s=='R'){ i|=SNES_TR_MASK; s++; if(*s=='+') s++; }
            if(!strncmp(s,"Start",5)){ i|=SNES_START_MASK; s+=5; if(*s=='+') s++; }
            if(!strncmp(s,"Select",6)){ i|=SNES_SELECT_MASK; s+=6; }
            if(i==0 || *s!=0 || *(s-1)=='+') return cmd;
            cmd.button.joypad.buttons=i;
            cmd.type=S9xButtonJoypad;
        }
    } else if(!strncmp(name,"Mouse",5)){
        if(name[5]<'1' || name[5]>'2' || name[6]!=' ') return cmd;
        cmd.button.mouse.idx = name[5]-'1';
        s=name+7;
        i=0;
        if((cmd.button.mouse.left=(*s=='L'))) s+=i=1;
        if((cmd.button.mouse.right=(*s=='R'))) s+=i=1;
        if(i==0 || *s!=0) return cmd;
        cmd.type=S9xButtonMouse;
    } else if(!strncmp(name,"Superscope ",11)){
        s=name+11;
        i=0;
        if((cmd.button.scope.aim_offscreen=strncmp(s,"AimOffscreen",12)?0:1)){ s+=i=12; if(*s==' '){ s++; } else if(*s!=0){ return cmd; }}
        if((cmd.button.scope.fire=strncmp(s,"Fire",4)?0:1)){ s+=i=4; if(*s=='+') s++; }
        if((cmd.button.scope.cursor=strncmp(s,"Cursor",6)?0:1)){ s+=i=6; if(*s=='+') s++; }
        if((cmd.button.scope.turbo=strncmp(s,"ToggleTurbo",11)?0:1)){ s+=i=11; if(*s=='+') s++; }
        if((cmd.button.scope.pause=strncmp(s,"Pause",5)?0:1)){ s+=i=5; }
        if(i==0 || *s!=0 || *(s-1)=='+') return cmd;
        cmd.type=S9xButtonSuperscope;
    } else if(!strncmp(name,"Justifier",9)){
        if(name[9]<'1' || name[9]>'2' || name[10]!=' ') return cmd;
        cmd.button.justifier.idx = name[9]-'1';
        s=name+11;
        i=0;
        if((cmd.button.justifier.aim_offscreen=strncmp(s,"AimOffscreen",12)?0:1)){ s+=i=12; if(*s==' '){ s++; } else if(*s!=0){ return cmd; }}
        if((cmd.button.justifier.trigger=strncmp(s,"Trigger",7)?0:1)){ s+=i=7; if(*s=='+') s++; }
        if((cmd.button.justifier.start=strncmp(s,"Start",5)?0:1)){ s+=i=5; }
        if(i==0 || *s!=0 || *(s-1)=='+') return cmd;
        cmd.type=S9xButtonJustifier;
    } else if(!strncmp(name,"Pointer ",8)){
        s=name+8;
        i=0;
        if((cmd.pointer.aim_mouse0=strncmp(s,"Mouse1",6)?0:1)){ s+=i=6; if(*s=='+') s++; }
        if((cmd.pointer.aim_mouse1=strncmp(s,"Mouse2",6)?0:1)){ s+=i=6; if(*s=='+') s++; }
        if((cmd.pointer.aim_scope=strncmp(s,"Superscope",10)?0:1)){ s+=i=10; if(*s=='+') s++; }
        if((cmd.pointer.aim_justifier0=strncmp(s,"Justifier1",10)?0:1)){ s+=i=10; if(*s=='+') s++; }
        if((cmd.pointer.aim_justifier1=strncmp(s,"Justifier2",10)?0:1)){ s+=i=10; }
        if(i==0 || *s!=0 || *(s-1)=='+') return cmd;
        cmd.type=S9xPointer;
    } else if(!strncmp(name,"ButtonToPointer ",16)){
        if(name[16]<'1' || name[16]>'8') return cmd;
        cmd.button.pointer.idx = name[16]-'1';
        s=name+17;
        i=0;
        if((cmd.button.pointer.UD=(*s=='u'?-1:(*s=='d'?1:0)))) s+=i=1;
        if((cmd.button.pointer.LR=(*s=='l'?-1:(*s=='r'?1:0)))) s+=i=1;
        if(i==0 || *(s++)!=' ') return cmd;
        for(i=0; i<4; i++){
            if(!strcmp(s,speed_names[i])) break;
        }
        if(i>3) return cmd;
        cmd.button.pointer.speed_type=i;
        cmd.type=S9xButtonPseudopointer;
    } else if(!strncmp(name,"AxisToPointer ",14)){
        if(name[14]<'1' || name[14]>'8') return cmd;
        cmd.axis.pointer.idx = name[14]-'1';
        s=name+15;
        i=0;
        if(*s=='h') cmd.axis.pointer.HV=0;
        else if(*s=='v') cmd.axis.pointer.HV=1;
        else return cmd;
        if(s[1]!=' ') return cmd;
        s+=2;
        if((cmd.axis.pointer.invert=*s=='-')) s++;
        for(i=0; i<4; i++){
            if(!strcmp(s,speed_names[i])) break;
        }
        if(i>3) return cmd;
        cmd.axis.pointer.speed_type=i;
        cmd.type=S9xAxisPseudopointer;
    } else if(!strncmp(name,"AxisToButtons ",14)){
        s=name+14;
        if(s[0]=='0'){
            if(s[1]!='/') return cmd;
            cmd.axis.button.negbutton=0;
            s+=2;
        } else {
            i=0;
            do {
                if(*s<'0' || *s>'9') return cmd;
                i=i*10+*s-'0';
                if(i>255) return cmd;
            } while(*++s!='/');
            cmd.axis.button.negbutton=i;
            s++;
        }
        if(s[0]=='0'){
            if(s[1]!=' ') return cmd;
            cmd.axis.button.posbutton=0;
            s+=2;
        } else {
            i=0;
            do {
                if(*s<'0' || *s>'9') return cmd;
                i=i*10+*s-'0';
                if(i>255) return cmd;
            } while(*++s!=' ');
            cmd.axis.button.posbutton=i;
            s++;
        }
        i=get_threshold(&s);
        if(i<0) return cmd;
        cmd.axis.button.threshold=(i-1)*256/1000;
        cmd.type=S9xAxisPseudobuttons;
    } else if(!strncmp(name, "MULTI#",6)){
        i=strtol(name+6,(char **)&s,10);
        if(s!=NULL && *s!='\0') return cmd;
        if(i>=(int)multis.size()) return cmd;
        cmd.button.multi_idx=i;
        cmd.type=S9xButtonMulti;
    } else if(((name[0]=='+' && name[1]=='{') || name[0]=='{') &&
              name[strlen(name)-1]=='}'){
        if(multis.size()>2147483640){
            fprintf(stderr, "Too many multis!");
            return cmd;
        }
        string x;
        int n;
        j=2;
        for(i=(name[0]=='+')?2:1; name[i]!='\0'; i++){
            if(name[i]==',' || name[i]==';'){
                if(name[i]==';') j++;
                if(++j>2147483640){
                    fprintf(stderr, "Multi too long!");
                    return cmd;
                }
            }
            if(name[i]=='{') return cmd;
        }
        s9xcommand_t *c=(s9xcommand_t *)calloc(j,sizeof(s9xcommand_t));
        if(c==NULL){
            perror("malloc error while parsing multi");
            return cmd;
        }
        n=0; i=(name[0]=='+')?2:1;
        do {
            if(name[i]==';'){
                c[n].type=S9xNoMapping;
                c[n].multi_press=0;
                c[n].button_norpt=0;
                j=i;
            } else if(name[i]==','){
                free(c);
                return cmd;
            } else {
                uint8 press=0;
                if(name[0]=='+'){
                    if(name[i]=='+'){
                        press=1;
                    } else if(name[i]=='-'){
                        press=2;
                    } else {
                        free(c);
                        return cmd;
                    }
                    i++;
                }
                for(j=i; name[j]!=';' && name[j]!=',' && name[j]!='}'; j++);
                x.assign(name+i,j-i);
                c[n]=S9xGetCommandT(x.c_str());
                c[n].multi_press=press;
                if(maptype(c[n].type)!=MAP_BUTTON){
                    free(c);
                    return cmd;
                }
                if(name[j]==';') j--;
            }
            i=j+1; n++;
        } while(name[i]!='\0');
        c[n].type=S9xNoMapping; c[n].multi_press=3;

        multis.push_back(c);
        cmd.button.multi_idx=multis.size()-1;
        cmd.type=S9xButtonMulti;
    } else {
        i=findstr(name, command_names, LAST_COMMAND);
        if(i<0) return cmd;
        cmd.type = S9xButtonCommand;
        cmd.button.command = i;
    }

    return cmd;
}

const char **S9xGetAllSnes9xCommands(void){
    return command_names;
}

s9xcommand_t S9xGetMapping(uint32 id){
    if(keymap.count(id)==0){
        s9xcommand_t cmd;
        cmd.type=S9xNoMapping;
        return cmd;
    } else {
        return keymap[id];
    }
}

static const char *maptypename(int t){
    switch(t){
      case MAP_NONE:    return "unmapped";
      case MAP_BUTTON:  return "button";
      case MAP_AXIS:    return "axis";
      case MAP_POINTER: return "pointer";
      default:          return "unknown";
    }
}

void S9xUnmapID(uint32 id){
    int i;
    for(i=0; i<NUMCTLS+1; i++){
        pollmap[i].erase(id);
    }
    if(mouse[0].ID==id) mouse[0].ID=InvalidControlID;
    if(mouse[1].ID==id) mouse[1].ID=InvalidControlID;
    if(superscope.ID==id) superscope.ID=InvalidControlID;
    if(justifier.ID[0]==id) justifier.ID[0]=InvalidControlID;
    if(justifier.ID[1]==id) justifier.ID[1]=InvalidControlID;
    if(id>=PseudoPointerBase) pseudopointer[id-PseudoPointerBase].mapped=false;
    keymap.erase(id);
}

bool S9xMapButton(uint32 id, s9xcommand_t mapping, bool poll){
    int t;

    if(id==InvalidControlID){
        fprintf(stderr, "Cannot map InvalidControlID\n");
        return false;
    }
    t=maptype(mapping.type);
    if(t==MAP_NONE){ S9xUnmapID(id); return true; }
    if(t!=MAP_BUTTON) return false;
    t=maptype(S9xGetMapping(id).type);
    if(t!=MAP_NONE && t!=MAP_BUTTON) fprintf(stderr, "WARNING: Remapping ID 0x%08x from %s to button\n", id, maptypename(t));
    if(id>=PseudoPointerBase){
        fprintf(stderr, "ERROR: Refusing to map pseudo-pointer #%d as a button\n", id-PseudoPointerBase);
        return false;
    }

    t=-1;
    if(poll){
        if(id>=PseudoButtonBase){
            fprintf(stderr, "INFO: Ignoring attempt to set pseudo-button #%d to polling\n", id-PseudoButtonBase);
        } else switch(mapping.type){
          case S9xButtonJoypad:
            t=JOYPAD0+mapping.button.joypad.idx;
            break;
          case S9xButtonMouse:
            t=MOUSE0+mapping.button.mouse.idx;
            break;
          case S9xButtonSuperscope:
            t=SUPERSCOPE;
            break;
          case S9xButtonJustifier:
            t=ONE_JUSTIFIER+mapping.button.justifier.idx;
            break;
          case S9xButtonCommand:
          case S9xButtonPseudopointer:
          case S9xButtonPort:
          case S9xButtonMulti:
            t=POLL_ALL;
            break;
        }
    }

    S9xUnmapID(id);
    keymap[id]=mapping;
    if(t>=0) pollmap[t].insert(id);
    return true;
}

void S9xReportButton(uint32 id, bool pressed){
    if(keymap.count(id)==0) return;
    if(keymap[id].type==S9xNoMapping) return;
    if(maptype(keymap[id].type)!=MAP_BUTTON){
        fprintf(stderr, "ERROR: S9xReportButton called on %s ID 0x%08x\n", maptypename(maptype(keymap[id].type)), id);
        return;
    }

	if(keymap[id].type==S9xButtonCommand)        // skips the "already-pressed check" unless it's a command, as a hack to work around the following problem:
    if(keymap[id].button_norpt==pressed) return; // FIXME: this makes the controls "stick" after loading a savestate while recording a movie and holding any button
    keymap[id].button_norpt=pressed;
    S9xApplyCommand(keymap[id], pressed, 0);
}


bool S9xMapPointer(uint32 id, s9xcommand_t mapping, bool poll){
    int t;

    if(id==InvalidControlID){
        fprintf(stderr, "Cannot map InvalidControlID\n");
        return false;
    }
    t=maptype(mapping.type);
    if(t==MAP_NONE){ S9xUnmapID(id); return true; }
    if(t!=MAP_POINTER) return false;
    t=maptype(S9xGetMapping(id).type);
    if(t!=MAP_NONE && t!=MAP_POINTER) fprintf(stderr, "WARNING: Remapping ID 0x%08x from %s to pointer\n", id, maptypename(t));
    if(id<PseudoPointerBase && id>=PseudoButtonBase){
        fprintf(stderr, "ERROR: Refusing to map pseudo-button #%d as a pointer\n", id-PseudoButtonBase);
        return false;
    }

    if(mapping.type==S9xPointer){
        if(mapping.pointer.aim_mouse0 && mouse[0].ID!=InvalidControlID && mouse[0].ID!=id){ fprintf(stderr, "ERROR: Rejecting attempt to control Mouse1 with two pointers\n"); return false; }
        if(mapping.pointer.aim_mouse1 && mouse[1].ID!=InvalidControlID && mouse[1].ID!=id){ fprintf(stderr, "ERROR: Rejecting attempt to control Mouse2 with two pointers\n"); return false; }
        if(mapping.pointer.aim_scope && superscope.ID!=InvalidControlID && superscope.ID!=id){ fprintf(stderr, "ERROR: Rejecting attempt to control SuperScope with two pointers\n"); return false; }
        if(mapping.pointer.aim_justifier0 && justifier.ID[0]!=InvalidControlID && justifier.ID[0]!=id){ fprintf(stderr, "ERROR: Rejecting attempt to control Justifier1 with two pointers\n"); return false; }
        if(mapping.pointer.aim_justifier1 && justifier.ID[1]!=InvalidControlID && justifier.ID[1]!=id){ fprintf(stderr, "ERROR: Rejecting attempt to control Justifier2 with two pointers\n"); return false; }
    }

    S9xUnmapID(id);
    if(poll){
        if(id>=PseudoPointerBase){
            fprintf(stderr, "INFO: Ignoring attempt to set pseudo-pointer #%d to polling\n", id-PseudoPointerBase);
        } else switch(mapping.type){
          case S9xPointer:
            if(mapping.pointer.aim_mouse0) pollmap[MOUSE0].insert(id);
            if(mapping.pointer.aim_mouse1) pollmap[MOUSE1].insert(id);
            if(mapping.pointer.aim_scope) pollmap[SUPERSCOPE].insert(id);
            if(mapping.pointer.aim_justifier0) pollmap[ONE_JUSTIFIER].insert(id);
            if(mapping.pointer.aim_justifier1) pollmap[TWO_JUSTIFIERS].insert(id);
            break;
          case S9xPointerPort:
            pollmap[POLL_ALL].insert(id);
            break;
        }
    }

    if(id>=PseudoPointerBase) pseudopointer[id-PseudoPointerBase].mapped=true;
    keymap[id]=mapping;
    if(mapping.pointer.aim_mouse0) mouse[0].ID=id;
    if(mapping.pointer.aim_mouse1) mouse[1].ID=id;
    if(mapping.pointer.aim_scope) superscope.ID=id;
    if(mapping.pointer.aim_justifier0) justifier.ID[0]=id;
    if(mapping.pointer.aim_justifier1) justifier.ID[1]=id;
    return true;
}

void S9xReportPointer(uint32 id, int16 x, int16 y){
    if(keymap.count(id)==0) return;
    if(keymap[id].type==S9xNoMapping) return;
    if(maptype(keymap[id].type)!=MAP_POINTER){
        fprintf(stderr, "ERROR: S9xReportPointer called on %s ID 0x%08x\n", maptypename(maptype(keymap[id].type)), id);
        return;
    }

    S9xApplyCommand(keymap[id], x, y);
}


bool S9xMapAxis(uint32 id, s9xcommand_t mapping, bool poll){
    int t;

    if(id==InvalidControlID){
        fprintf(stderr, "Cannot map InvalidControlID\n");
        return false;
    }
    t=maptype(mapping.type);
    if(t==MAP_NONE){ S9xUnmapID(id); return true; }
    if(t!=MAP_AXIS) return false;
    t=maptype(S9xGetMapping(id).type);
    if(t!=MAP_NONE && t!=MAP_AXIS) fprintf(stderr, "WARNING: Remapping ID 0x%08x from %s to axis\n", id, maptypename(t));
    if(id>=PseudoPointerBase){
        fprintf(stderr, "ERROR: Refusing to map pseudo-pointer #%d as an axis\n", id-PseudoPointerBase);
        return false;
    }
    if(id>=PseudoButtonBase){
        fprintf(stderr, "ERROR: Refusing to map pseudo-button #%d as an axis\n", id-PseudoButtonBase);
        return false;
    }

    t=-1;
    if(poll){
        switch(mapping.type){
          case S9xAxisJoypad:
            t=JOYPAD0+mapping.axis.joypad.idx;
            break;
          case S9xAxisPseudopointer:
          case S9xAxisPseudobuttons:
          case S9xAxisPort:
            t=POLL_ALL;
            break;
        }
    }

    S9xUnmapID(id);
    keymap[id]=mapping;
    if(t>=0) pollmap[t].insert(id);
    return true;
}

void S9xReportAxis(uint32 id, int16 value){
    if(keymap.count(id)==0) return;
    if(keymap[id].type==S9xNoMapping) return;
    if(maptype(keymap[id].type)!=MAP_AXIS){
        fprintf(stderr, "ERROR: S9xReportAxis called on %s ID 0x%08x\n", maptypename(maptype(keymap[id].type)), id);
        return;
    }

    S9xApplyCommand(keymap[id], value, 0);
}


static int32 ApplyMulti(s9xcommand_t *multi, int32 pos, int16 data1){
    while(1){
        if(multi[pos].multi_press==3) return -1;
        if(multi[pos].type==S9xNoMapping) break;
        if(multi[pos].multi_press){
            S9xApplyCommand(multi[pos], multi[pos].multi_press==1, 0);
        } else {
            S9xApplyCommand(multi[pos], data1, 0);
        }
        pos++;
    }
    return pos+1;
}

void S9xApplyCommand(s9xcommand_t cmd, int16 data1, int16 data2){
    int i;

    switch(cmd.type){
      case S9xNoMapping:
        return;

      case S9xButtonJoypad:
        if(cmd.button.joypad.toggle){
            if(!data1) return;
            uint16 r=cmd.button.joypad.buttons;
            if(cmd.button.joypad.turbo) joypad[cmd.button.joypad.idx].toggleturbo^=r;
            if(cmd.button.joypad.sticky) joypad[cmd.button.joypad.idx].togglestick^=r;
        } else {
            uint16 r,s,t,st;
            s=t=st=0;
            r=cmd.button.joypad.buttons;
            st=r & joypad[cmd.button.joypad.idx].togglestick & joypad[cmd.button.joypad.idx].toggleturbo;
            r ^= st;
            t=r & joypad[cmd.button.joypad.idx].toggleturbo;
            r ^= t;
            s=r & joypad[cmd.button.joypad.idx].togglestick;
            r ^= s;
            if(cmd.button.joypad.turbo && cmd.button.joypad.sticky){
                uint16 x=r; r=st; st=x;
                x=s; s=t; t=x;
            } else if(cmd.button.joypad.turbo){
                uint16 x=r; r=t; t=x;
                x=s; s=st; st=x;
            } else if(cmd.button.joypad.sticky){
                uint16 x=r; r=s; s=x;
                x=t; t=st; st=x;
            }
            if(data1){
				if(!Settings.UpAndDown && !S9xMoviePlaying()) // if up+down isn't allowed AND we are NOT playing a movie,
				{
                if(cmd.button.joypad.buttons&(SNES_LEFT_MASK|SNES_RIGHT_MASK)){
                    // if we're pressing left or right, then unpress and unturbo
                    // them both first so we don't end up hittnig left AND right
                    // accidentally. Note though that the user can still do it on
                    // purpose, if Settings.UpAndDown = true.
                    // This is a feature, look up glitches in tLoZ:aLttP to find
                    // out why.
                    joypad[cmd.button.joypad.idx].buttons &= ~(SNES_LEFT_MASK|SNES_RIGHT_MASK);
                    joypad[cmd.button.joypad.idx].turbos &= ~(SNES_LEFT_MASK|SNES_RIGHT_MASK);
                }
                if(cmd.button.joypad.buttons&(SNES_UP_MASK|SNES_DOWN_MASK)){
                    // and ditto for up/down
                    joypad[cmd.button.joypad.idx].buttons &= ~(SNES_UP_MASK|SNES_DOWN_MASK);
                    joypad[cmd.button.joypad.idx].turbos &= ~(SNES_UP_MASK|SNES_DOWN_MASK);
                }
                }//end up+down protection
                joypad[cmd.button.joypad.idx].buttons |= r;
                joypad[cmd.button.joypad.idx].turbos |= t;
                joypad[cmd.button.joypad.idx].buttons ^= s;
                joypad[cmd.button.joypad.idx].buttons &= ~(joypad[cmd.button.joypad.idx].turbos & st);
                joypad[cmd.button.joypad.idx].turbos ^= st;
            } else {
                joypad[cmd.button.joypad.idx].buttons &= ~r;
                joypad[cmd.button.joypad.idx].buttons &= ~(joypad[cmd.button.joypad.idx].turbos & t);
                joypad[cmd.button.joypad.idx].turbos &= ~t;
            }
        }
        return;

      case S9xButtonMouse:
        i=0;
        if(cmd.button.mouse.left) i|=0x40;
        if(cmd.button.mouse.right) i|=0x80;
        if(data1){
            mouse[cmd.button.mouse.idx].buttons |= i;
        } else {
            mouse[cmd.button.mouse.idx].buttons &= ~i;
        }
        return;

      case S9xButtonSuperscope:
        i=0;
        if(cmd.button.scope.fire) i|=SUPERSCOPE_FIRE;
        if(cmd.button.scope.cursor) i|=SUPERSCOPE_CURSOR;
        if(cmd.button.scope.pause) i|=SUPERSCOPE_PAUSE;
        if(cmd.button.scope.aim_offscreen) i|=SUPERSCOPE_OFFSCREEN;
        if(data1){
            superscope.phys_buttons |= i;
            if(cmd.button.scope.turbo){
                superscope.phys_buttons^=SUPERSCOPE_TURBO;
                if(superscope.phys_buttons&SUPERSCOPE_TURBO){
                    superscope.next_buttons |= superscope.phys_buttons&(SUPERSCOPE_FIRE|SUPERSCOPE_CURSOR);
                } else {
                    superscope.next_buttons &= ~(SUPERSCOPE_FIRE|SUPERSCOPE_CURSOR);
                }
            }
            superscope.next_buttons |= i&(SUPERSCOPE_FIRE|SUPERSCOPE_CURSOR|SUPERSCOPE_PAUSE);
			if(!S9xMovieActive()) // PPU modification during non-recordable command screws up movie synchronization
            if((superscope.next_buttons&(SUPERSCOPE_FIRE|SUPERSCOPE_CURSOR)) &&
               curcontrollers[1]==SUPERSCOPE &&
               !(superscope.phys_buttons&SUPERSCOPE_OFFSCREEN)){
                DoGunLatch(superscope.x, superscope.y);
            }
        } else {
            superscope.phys_buttons &= ~i;
            superscope.next_buttons &= SUPERSCOPE_OFFSCREEN|~i;
        }
        return;

      case S9xButtonJustifier:
        i=0;
        if(cmd.button.justifier.trigger) i|=JUSTIFIER_TRIGGER;
        if(cmd.button.justifier.start) i|=JUSTIFIER_START;
        if(cmd.button.justifier.aim_offscreen) justifier.offscreen[cmd.button.justifier.idx]=data1?1:0;
        i>>=cmd.button.justifier.idx;
        if(data1){
            justifier.buttons |= i;
        } else {
            justifier.buttons &= ~i;
        }
        return;

      case S9xButtonCommand:
        if(((enum command_numbers)cmd.button.command)>=LAST_COMMAND){
            fprintf(stderr, "Unknown command %04x\n", cmd.button.command);
            return;
        }
        if(!data1){
            switch(i=cmd.button.command){
              case EmuTurbo:
                Settings.TurboMode = FALSE;
                break;
            }
        } else {
            switch((enum command_numbers)(i=cmd.button.command)){
              case ExitEmu:
                S9xExit();
                break;
              case Reset:
                S9xReset();
                break;
              case SoftReset:
				S9xMovieUpdateOnReset ();
				if(S9xMoviePlaying())
					S9xMovieStop (TRUE);
                S9xSoftReset();
                break;
              case EmuTurbo:
                Settings.TurboMode = TRUE;
                break;
              case ToggleEmuTurbo:
                Settings.TurboMode = !Settings.TurboMode;
                S9xDisplayStateChange("Turbo Mode", Settings.TurboMode);
                break;
              case BGLayeringHack:
                Settings.BGLayering = !Settings.BGLayering;
                S9xDisplayStateChange("Background layering hack", Settings.BGLayering);
                break;
              case ClipWindows:
                Settings.DisableGraphicWindows = !Settings.DisableGraphicWindows;
                S9xDisplayStateChange("Graphic clip windows",
                                      !Settings.DisableGraphicWindows);
                break;
              case Debugger:
#ifdef DEBUGGER
                CPU.Flags |= DEBUG_MODE_FLAG;
#endif
                break;
              case IncFrameRate:
                if (Settings.SkipFrames == AUTO_FRAMERATE)
                    Settings.SkipFrames = 1;
                else
                    if (Settings.SkipFrames < 10)
                        Settings.SkipFrames++;

                if (Settings.SkipFrames == AUTO_FRAMERATE)
                    S9xSetInfoString ("Auto frame skip");
                else {
                    sprintf (buf, "Frame skip: %d",
                             Settings.SkipFrames - 1);
                    S9xSetInfoString (buf);
                }
                break;
              case DecFrameRate:
                if (Settings.SkipFrames <= 1)
                    Settings.SkipFrames = AUTO_FRAMERATE;
                else
                    if (Settings.SkipFrames != AUTO_FRAMERATE)
                        Settings.SkipFrames--;

                if (Settings.SkipFrames == AUTO_FRAMERATE)
                    S9xSetInfoString ("Auto frame skip");
                else {
                    sprintf (buf, "Frame skip: %d",
                             Settings.SkipFrames - 1);
                    S9xSetInfoString (buf);
                }
                break;
              case IncEmuTurbo:
                if(Settings.TurboSkipFrames<20) Settings.TurboSkipFrames += 1;
                else if(Settings.TurboSkipFrames<200) Settings.TurboSkipFrames += 5;
                sprintf (buf, "Turbo Frame Skip: %d",
                         Settings.TurboSkipFrames);
                S9xSetInfoString (buf);
                break;
              case DecEmuTurbo:
                if(Settings.TurboSkipFrames>20) Settings.TurboSkipFrames -= 5;
                else if(Settings.TurboSkipFrames>0) Settings.TurboSkipFrames -= 1;
                sprintf (buf, "Turbo Frame Skip: %d",
                         Settings.TurboSkipFrames);
                S9xSetInfoString (buf);
                break;
              case IncFrameTime:
                // Increase emulated frame time by 1ms
                Settings.FrameTime += 1000;
                sprintf (buf, "Emulated frame time: %dms",
                         Settings.FrameTime / 1000);
                S9xSetInfoString (buf);
                break;
              case DecFrameTime:
                // Decrease emulated frame time by 1ms
                if (Settings.FrameTime >= 1000)
                    Settings.FrameTime -= 1000;
                sprintf (buf, "Emulated frame time: %dms",
                         Settings.FrameTime / 1000);
                S9xSetInfoString (buf);
                break;
              case IncTurboSpeed:
                if(turbo_time>=120) break;
                turbo_time++;
                sprintf (buf, "Turbo speed: %d",
                         turbo_time);
                S9xSetInfoString (buf);
                break;
              case DecTurboSpeed:
                if(turbo_time<=1) break;
                turbo_time--;
                sprintf (buf, "Turbo speed: %d",
                         turbo_time);
                S9xSetInfoString (buf);
                break;
              case InterpolateSound:
                Settings.InterpolatedSound ^= 1;
                S9xDisplayStateChange("Interpolated sound", Settings.InterpolatedSound);
                break;
              case LoadFreezeFile:
                S9xUnfreezeGame(S9xChooseFilename(TRUE));
                break;
              case SaveFreezeFile:
                S9xFreezeGame(S9xChooseFilename(FALSE));
                break;
              case LoadOopsFile:
                {
                    char def [PATH_MAX];
                    char filename [PATH_MAX];
                    char drive [_MAX_DRIVE];
                    char dir [_MAX_DIR];
                    char ext [_MAX_EXT];

                    _splitpath (Memory.ROMFilename, drive, dir, def, ext);
                    sprintf (filename, "%s%s%s.%.*s",
                             S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, def,
                             _MAX_EXT-1, "oops");
                    if (S9xLoadSnapshot (filename))
                    {
                        sprintf (buf, "%s.%.*s loaded", def, _MAX_EXT-1, "oops");
                        S9xSetInfoString (buf);
                    }
                    else
                    {
                        S9xMessage (S9X_ERROR, S9X_FREEZE_FILE_NOT_FOUND,
                                    "Oops file not found");
                    }
                }
                break;
/*              case Mode7Interpolate:
                Settings.Mode7Interpolate ^= TRUE;
                S9xDisplayStateChange ("Mode 7 Interpolation",
                                       Settings.Mode7Interpolate);
                break;*/
              case Pause:
                Settings.Paused ^= 1;
                S9xDisplayStateChange ("Pause", Settings.Paused);
#if defined(NETPLAY_SUPPORT) && !defined(__WIN32__)
                S9xNPSendPause(Settings.Paused);
#endif
                break;
              case QuickLoad000: case QuickLoad001: case QuickLoad002: case QuickLoad003: case QuickLoad004: case QuickLoad005: case QuickLoad006: case QuickLoad007: case QuickLoad008: case QuickLoad009: case QuickLoad010:
                {
                    char def [PATH_MAX];
                    char filename [PATH_MAX];
                    char drive [_MAX_DRIVE];
                    char dir [_MAX_DIR];
                    char ext [_MAX_EXT];

                    _splitpath (Memory.ROMFilename, drive, dir, def, ext);
                    sprintf (filename, "%s%s%s.%03d",
                             S9xGetDirectory (SNAPSHOT_DIR), SLASH_STR, def,
                             i - QuickLoad000);
                    if (S9xLoadSnapshot (filename))
                    {
                        sprintf (buf, "%s.%03d loaded", def, i - QuickLoad000);
                        S9xSetInfoString (buf);
                    }
                    else
                    {
                        static char *digits = "t123456789";
                        _splitpath (Memory.ROMFilename, drive, dir, def, ext);
                        sprintf (filename, "%s%s%s.zs%c",
                                 S9xGetDirectory (SNAPSHOT_DIR), SLASH_STR,
                                 def, digits [i - QuickLoad000]);
                        if (S9xLoadSnapshot (filename))
                        {
                            sprintf (buf,
                                     "Loaded ZSNES freeze file %s.zs%c",
                                     def, digits [i - QuickLoad000]);
                            S9xSetInfoString (buf);
                        }
                        else
                            S9xMessage (S9X_ERROR, S9X_FREEZE_FILE_NOT_FOUND,
                                        "Freeze file not found");
                    }
                }
                break;
              case QuickSave000: case QuickSave001: case QuickSave002: case QuickSave003: case QuickSave004: case QuickSave005: case QuickSave006: case QuickSave007: case QuickSave008: case QuickSave009: case QuickSave010:
                {
                    char def [PATH_MAX];
                    char filename [PATH_MAX];
                    char drive [_MAX_DRIVE];
                    char dir [_MAX_DIR];
                    char ext [_MAX_EXT];

                    _splitpath (Memory.ROMFilename, drive, dir, def, ext);
                    sprintf (filename, "%s%s%s.%03d",
                             S9xGetDirectory (SNAPSHOT_DIR), SLASH_STR, def,
                             i - QuickSave000);
                    sprintf (buf, "%s.%03d saved", def, i - QuickSave000);
                    S9xSetInfoString (buf);
                    Snapshot(filename);
                }
                break;
              case SaveSPC:
                {
                    char def [PATH_MAX];
                    char filename [PATH_MAX];
                    char drive [_MAX_DRIVE];
                    char dir [_MAX_DIR];
                    char ext [_MAX_EXT];

                    _splitpath (Memory.ROMFilename, drive, dir, def, ext);
                    strcpy (ext, "spc");
                    _makepath (filename, drive, S9xGetDirectory (SPC_DIR),
                               def, ext);
                    if (S9xSPCDump (filename))
                        sprintf (buf, "%s.%s saved", def, ext);
                    else
                        sprintf (buf, "%s.%s not saved (%s)",
                                 def, ext, strerror (errno));

                    S9xSetInfoString (buf);
                }
                break;
              case Screenshot:
                Settings.TakeScreenshot=TRUE;
                break;
              case SoundChannel0: case SoundChannel1: case SoundChannel2: case SoundChannel3: case SoundChannel4: case SoundChannel5: case SoundChannel6: case SoundChannel7:
                S9xToggleSoundChannel(i-SoundChannel0);
                sprintf (buf, "Sound channel %d toggled", i-SoundChannel0);
                S9xSetInfoString (buf);
                break;
              case SoundChannelsOn:
                S9xToggleSoundChannel(8);
                S9xSetInfoString ("All sound channels on");
                break;
              case SynchronizeSound:
                Settings.SoundSync ^= 1;
                S9xDisplayStateChange ("Synchronised sound",
                                       Settings.SoundSync);
                break;
              case ToggleBG0:
                Settings.BG_Forced ^= 1;
                S9xDisplayStateChange ("BG#0", !(Settings.BG_Forced & 1));
                break;
              case ToggleBG1:
                Settings.BG_Forced ^= 2;
                S9xDisplayStateChange ("BG#1", !(Settings.BG_Forced & 2));
                break;
              case ToggleBG2:
                Settings.BG_Forced ^= 4;
                S9xDisplayStateChange ("BG#2", !(Settings.BG_Forced & 4));
                break;
              case ToggleBG3:
                Settings.BG_Forced ^= 8;
                S9xDisplayStateChange ("BG#3", !(Settings.BG_Forced & 8));
                break;
              case ToggleSprites:
                Settings.BG_Forced ^= 16;
                S9xDisplayStateChange ("Sprites", !(Settings.BG_Forced & 16));
                break;
              case ToggleHDMA:
                Settings.DisableHDMA = !Settings.DisableHDMA;
                S9xDisplayStateChange ("HDMA emulation", !Settings.DisableHDMA);
                break;
              case ToggleTransparency:
                Settings.Transparency = !Settings.Transparency;
                S9xDisplayStateChange ("Transparency effects",
                                       Settings.Transparency);
                break;

              case DumpSPC7110Log:
                if(Settings.SPC7110) Do7110Logging();
                break;

			  case BeginRecordingMovie: {
                if(S9xMovieActive()) S9xMovieStop(FALSE);

                S9xMovieCreate(S9xChooseMovieFilename(FALSE),
                               0xFF,
                               //MOVIE_OPT_FROM_SNAPSHOT
                               MOVIE_OPT_FROM_RESET,
                               NULL, 0);
              }  break;

              case LoadMovie:
                if(S9xMovieActive()) S9xMovieStop(FALSE);
                S9xMovieOpen(S9xChooseMovieFilename(TRUE), FALSE);
                break;

              case EndRecordingMovie:
                if(S9xMovieActive()) S9xMovieStop(FALSE);
                break;

              case SwapJoypads:
                if((curcontrollers[0]!=NONE && !(curcontrollers[0]>=JOYPAD0 && curcontrollers[0]<=JOYPAD7))){
                    S9xSetInfoString("Cannot swap pads: port 1 is not a joypad");
                    break;
                }
                if((curcontrollers[1]!=NONE && !(curcontrollers[1]>=JOYPAD0 && curcontrollers[1]<=JOYPAD7))){
                    S9xSetInfoString("Cannot swap pads: port 2 is not a joypad");
                    break;
                }
                newcontrollers[1]=curcontrollers[0];
                newcontrollers[0]=curcontrollers[1];
                strcpy(buf, "Swap pads: P1=");
                i=14;
                if(newcontrollers[0]==NONE){
                    strcpy(buf+i, "<none>");
                    i+=6;
                } else {
                    sprintf(buf+i, "Joypad%d", newcontrollers[0]-JOYPAD0+1);
                    i+=7;
                }
                strcpy(buf+i, " P2=");
                i+=4;
                if(newcontrollers[1]==NONE){
                    strcpy(buf+i, "<none>");
                    i+=6;
                } else {
                    sprintf(buf+i, "Joypad%d", newcontrollers[1]-JOYPAD0+1);
                    i+=7;
                }
                S9xSetInfoString(buf);
                break;

              case SeekToFrame: {
                if (!S9xMovieActive())
                {
                	S9xSetInfoString("No movie in progress.");
                	return;
                }
                char msg[128];
                sprintf(msg, "Select frame number (current: %d)", S9xMovieGetFrameCounter());

        	const char *frameno = S9xStringInput(msg);
        	if (!frameno)
        	  return;
        	int frameDest = atoi(frameno);
        	if (frameDest > 0 && frameDest > (int)S9xMovieGetFrameCounter())
        	{
        	  int distance = frameDest - S9xMovieGetFrameCounter();
        	  Settings.HighSpeedSeek = distance;
        	}
              } // braces for vlocalitylocality
			  break;

              case LAST_COMMAND: break;
                /* no default, so we get compiler warnings */
            }
        }
        return;

      case S9xPointer:
        if(cmd.pointer.aim_mouse0){
            mouse[0].cur_x=data1;
            mouse[0].cur_y=data2;
        }
        if(cmd.pointer.aim_mouse1){
            mouse[1].cur_x=data1;
            mouse[1].cur_y=data2;
        }
        if(cmd.pointer.aim_scope){
            superscope.x=data1;
            superscope.y=data2;
        }
        if(cmd.pointer.aim_justifier0){
            justifier.x[0]=data1;
            justifier.y[0]=data2;
        }
        if(cmd.pointer.aim_justifier1){
            justifier.x[1]=data1;
            justifier.y[1]=data2;
        }
        return;

      case S9xButtonPseudopointer:
        if(data1){
            if(cmd.button.pointer.UD){
                if(!pseudopointer[cmd.button.pointer.idx].V_adj) pseudopointer[cmd.button.pointer.idx].V_adj=cmd.button.pointer.UD*ptrspeeds[cmd.button.pointer.speed_type];
                pseudopointer[cmd.button.pointer.idx].V_var=(cmd.button.pointer.speed_type==0);
            }
            if(cmd.button.pointer.LR){
                if(!pseudopointer[cmd.button.pointer.idx].H_adj) pseudopointer[cmd.button.pointer.idx].H_adj=cmd.button.pointer.LR*ptrspeeds[cmd.button.pointer.speed_type];
                pseudopointer[cmd.button.pointer.idx].H_var=(cmd.button.pointer.speed_type==0);
            }
        } else {
            if(cmd.button.pointer.UD){
                pseudopointer[cmd.button.pointer.idx].V_adj=0;
                pseudopointer[cmd.button.pointer.idx].V_var=false;
            }
            if(cmd.button.pointer.LR){
                pseudopointer[cmd.button.pointer.idx].H_adj=0;
                pseudopointer[cmd.button.pointer.idx].H_var=false;
            }
        }
        return;

      case S9xAxisJoypad:
        {
            uint16 pos, neg;
            switch(cmd.axis.joypad.axis){
              case 0: neg=SNES_LEFT_MASK; pos=SNES_RIGHT_MASK; break;
              case 1: neg=SNES_UP_MASK; pos=SNES_DOWN_MASK; break;
              case 2: neg=SNES_Y_MASK; pos=SNES_A_MASK; break;
              case 3: neg=SNES_X_MASK; pos=SNES_B_MASK; break;
              case 4: neg=SNES_TL_MASK; pos=SNES_TR_MASK; break;
              default: return;
            }
            if(cmd.axis.joypad.invert) data1=-data1;
            uint16 p, r;
            p=r=0;
            if(data1>((cmd.axis.joypad.threshold+1)*127)){
                p|=pos;
            } else {
                r|=pos;
            }
            if(data1<=((cmd.axis.joypad.threshold+1)*-127)){
                p|=neg;
            } else {
                r|=neg;
            }
            joypad[cmd.axis.joypad.idx].buttons |= p;
            joypad[cmd.axis.joypad.idx].buttons &= ~r;
            joypad[cmd.axis.joypad.idx].turbos &= ~(p|r);
        }
        return;

      case S9xAxisPseudopointer:
        if(data1==0){
            if(cmd.axis.pointer.HV){
                pseudopointer[cmd.axis.pointer.idx].V_adj=0;
                pseudopointer[cmd.axis.pointer.idx].V_var=false;
            } else {
                pseudopointer[cmd.axis.pointer.idx].H_adj=0;
                pseudopointer[cmd.axis.pointer.idx].H_var=false;
            }
        } else {
            if(cmd.axis.pointer.invert) data1=-data1;
            if(cmd.axis.pointer.HV){
                if(!pseudopointer[cmd.axis.pointer.idx].V_adj) pseudopointer[cmd.axis.pointer.idx].V_adj=(int16)((int32)data1*ptrspeeds[cmd.axis.pointer.speed_type]/32767);
                pseudopointer[cmd.axis.pointer.idx].V_var=(cmd.axis.pointer.speed_type==0);
            } else {
                if(!pseudopointer[cmd.axis.pointer.idx].H_adj) pseudopointer[cmd.axis.pointer.idx].H_adj=(int16)((int32)data1*ptrspeeds[cmd.axis.pointer.speed_type]/32767);
                pseudopointer[cmd.axis.pointer.idx].H_var=(cmd.axis.pointer.speed_type==0);
            }
        }
        return;

      case S9xAxisPseudobuttons:
        if(data1>((cmd.axis.button.threshold+1)*127)){
            if(!pseudobuttons[cmd.axis.button.posbutton]){
                pseudobuttons[cmd.axis.button.posbutton]=1;
                S9xReportButton(PseudoButtonBase+cmd.axis.button.posbutton, true);
            }
        } else {
            if(pseudobuttons[cmd.axis.button.posbutton]){
                pseudobuttons[cmd.axis.button.posbutton]=0;
                S9xReportButton(PseudoButtonBase+cmd.axis.button.posbutton, false);
            }
        }
        if(data1<=((cmd.axis.button.threshold+1)*-127)){
            if(!pseudobuttons[cmd.axis.button.negbutton]){
                pseudobuttons[cmd.axis.button.negbutton]=1;
                S9xReportButton(PseudoButtonBase+cmd.axis.button.negbutton, true);
            }
        } else {
            if(pseudobuttons[cmd.axis.button.negbutton]){
                pseudobuttons[cmd.axis.button.negbutton]=0;
                S9xReportButton(PseudoButtonBase+cmd.axis.button.negbutton, false);
            }
        }
        return;

      case S9xButtonPort:
      case S9xAxisPort:
      case S9xPointerPort:
        S9xHandlePortCommand(cmd, data1, data2);
        return;

      case S9xButtonMulti:
        if(cmd.button.multi_idx>=(int)multis.size()) return;
        if(multis[cmd.button.multi_idx]->multi_press && !data1) return;
        i=ApplyMulti(multis[cmd.button.multi_idx], 0, data1);
        if(i>=0){
            struct exemulti *e=new struct exemulti;
            e->pos=i;
            e->data1=data1!=0;
            e->script=multis[cmd.button.multi_idx];
            exemultis.insert(e);
        }
        return;



      default:
        fprintf(stderr, "WARNING: Unknown command type %d\n", cmd.type);
        return;
    }
}


static void do_polling(int mp){
    set<uint32>::iterator itr;

    if(pollmap[mp].empty()) return;
	if(S9xMoviePlaying()) return;
    for(itr=pollmap[mp].begin(); itr!=pollmap[mp].end(); itr++){
        switch(maptype(keymap[*itr].type)){
          case MAP_BUTTON:
            bool pressed;
            if(S9xPollButton(*itr, &pressed)) S9xReportButton(*itr, pressed);
            break;

          case MAP_AXIS:
            int16 value;
            if(S9xPollAxis(*itr, &value)) S9xReportAxis(*itr, value);
            break;

          case MAP_POINTER:
            int16 x, y;
            if(S9xPollPointer(*itr, &x, &y)) S9xReportPointer(*itr, x, y);
            break;

          default:
            break;
        }
    }
}

static void UpdatePolledMouse(int i) {
	int16 j;
    j=mouse[i-MOUSE0].cur_x-mouse[i-MOUSE0].old_x;
    if(j<-127){
        mouse[i-MOUSE0].delta_x=0xff;
        mouse[i-MOUSE0].old_x-=127;
    } else if(j<0){
        mouse[i-MOUSE0].delta_x=0x80 | -j;
        mouse[i-MOUSE0].old_x=mouse[i-MOUSE0].cur_x;
    } else if(j>127){
        mouse[i-MOUSE0].delta_x=0x7f;
        mouse[i-MOUSE0].old_x+=127;
    } else {
        mouse[i-MOUSE0].delta_x=(uint8)j;
        mouse[i-MOUSE0].old_x=mouse[i-MOUSE0].cur_x;
    }
    j=mouse[i-MOUSE0].cur_y-mouse[i-MOUSE0].old_y;
    if(j<-127){
        mouse[i-MOUSE0].delta_y=0xff;
        mouse[i-MOUSE0].old_y-=127;
    } else if(j<0){
        mouse[i-MOUSE0].delta_y=0x80 | -j;
        mouse[i-MOUSE0].old_y=mouse[i-MOUSE0].cur_y;
    } else if(j>127){
        mouse[i-MOUSE0].delta_y=0x7f;
        mouse[i-MOUSE0].old_y+=127;
    } else {
        mouse[i-MOUSE0].delta_y=(uint8)j;
        mouse[i-MOUSE0].old_y=mouse[i-MOUSE0].cur_y;
    }
}

void S9xSetJoypadLatch(bool latch){
    if(!latch && FLAG_LATCH){
        // 1 written, 'plug in' new controllers now
        curcontrollers[0]=newcontrollers[0];
        curcontrollers[1]=newcontrollers[1];
    }
    if(latch && !FLAG_LATCH){
        int i, j, n;

        for(n=0; n<2; n++){
            for(j=0; j<2; j++){
                read_idx[n][j]=0;
            }
            switch(i=curcontrollers[n]){
              case MP5:
                for(j=0, i=mp5[n].pads[j]; j<4; i=mp5[n].pads[++j]){
                    if(i==NONE) continue;
                    do_polling(i);
                }
                break;
              case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
              case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
                do_polling(i);
                break;

              case MOUSE0: case MOUSE1:
                do_polling(i);
				if(!S9xMoviePlaying())
					UpdatePolledMouse(i);
                break;
              case SUPERSCOPE:
                if(superscope.next_buttons&SUPERSCOPE_FIRE){
                    superscope.next_buttons&=~SUPERSCOPE_TURBO;
                    superscope.next_buttons|=superscope.phys_buttons&SUPERSCOPE_TURBO;
                }
                if(superscope.next_buttons&(SUPERSCOPE_FIRE|SUPERSCOPE_CURSOR)){
                    superscope.next_buttons&=~SUPERSCOPE_OFFSCREEN;
                    superscope.next_buttons|=superscope.phys_buttons&SUPERSCOPE_OFFSCREEN;
                }
                superscope.read_buttons=superscope.next_buttons;
                superscope.next_buttons&=~SUPERSCOPE_PAUSE;
                if(!(superscope.phys_buttons&SUPERSCOPE_TURBO)) superscope.next_buttons&=~(SUPERSCOPE_CURSOR|SUPERSCOPE_FIRE);
                do_polling(i);
                break;
              case TWO_JUSTIFIERS:
                do_polling(TWO_JUSTIFIERS);
                /* fall through */
              case ONE_JUSTIFIER:
                justifier.buttons^=JUSTIFIER_SELECT;
                do_polling(ONE_JUSTIFIER);
                break;
              default:
                break;
            }
        }
    }
    FLAG_LATCH=latch;
}

uint8 S9xReadJOYSERn(int n){
	int i, j, r;

    if(n>1) n-=0x4016;
    assert(n==0 || n==1);

    uint8 bits=(OpenBus&~3)|((n==1)?0x1c:0);

    if(FLAG_LATCH){
        switch(i=curcontrollers[n]){
          case MP5:
            return bits|2;
          case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
          case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
            return bits|((joypad[i-JOYPAD0].buttons&0x8000)?1:0);
          case MOUSE0: case MOUSE1:
            mouse[i-MOUSE0].buttons+=0x10;
            if((mouse[i-MOUSE0].buttons&0x30)==0x30)
                mouse[i-MOUSE0].buttons&=0xcf;
            return bits;
          case SUPERSCOPE:
            return bits|((superscope.read_buttons&0x80)?1:0);
          case ONE_JUSTIFIER: case TWO_JUSTIFIERS:
            return bits;
          default:
            return bits;
        }
    } else {
        switch(i=curcontrollers[n]){
          case MP5:
            r=read_idx[n][FLAG_IOBIT(n)?0:1]++;
            j=FLAG_IOBIT(n)?0:2;
            for(i=0; i<2; i++, j++){
                if(mp5[n].pads[j]==NONE) continue;
                if(r>=16){
                    bits|=1<<i;
                } else {
                    bits|=((joypad[mp5[n].pads[j]-JOYPAD0].buttons&(0x8000>>r))?1:0)<<i;
                }
            }
            return bits;
          case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
          case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
            if(read_idx[n][0]>=16){
                read_idx[n][0]++;
                return bits|1;
            } else {
                return bits|((joypad[i-JOYPAD0].buttons&(0x8000>>read_idx[n][0]++))?1:0);
            }
          case MOUSE0: case MOUSE1:
            if(read_idx[n][0]<8){
                read_idx[n][0]++;
                return bits;
            } else if(read_idx[n][0]<16){
                return bits|((mouse[i-MOUSE0].buttons&(0x8000>>read_idx[n][0]++))?1:0);
            } else if(read_idx[n][0]<24){
                return bits|((mouse[i-MOUSE0].delta_y&(0x800000>>read_idx[n][0]++))?1:0);
            } else if(read_idx[n][0]<32){
                return bits|((mouse[i-MOUSE0].delta_x&(0x80000000>>read_idx[n][0]++))?1:0);
            } else {
                read_idx[n][0]++;
                return bits|1;
            }
          case SUPERSCOPE:
            if(read_idx[n][0]<8){
                return bits|((superscope.read_buttons&(0x80>>read_idx[n][0]++))?1:0);
            } else {
                read_idx[n][0]++;
                return bits|1;
            }
          case ONE_JUSTIFIER:
            if(read_idx[n][0]<24){
                return bits|((0xaa7000>>read_idx[n][0]++)&1);
            } else if(read_idx[n][0]<32){
                bits|=((justifier.buttons &
                        (JUSTIFIER_TRIGGER|JUSTIFIER_START|JUSTIFIER_SELECT) &
                        (0x80000000>>read_idx[n][0]++))?1:0);
                return bits;
            } else {
                read_idx[n][0]++;
                return bits|1;
            }
          case TWO_JUSTIFIERS:
            if(read_idx[n][0]<24){
                return bits|((0xaa7000>>read_idx[n][0]++)&1);
            } else if(read_idx[n][0]<32){
                return bits|((justifier.buttons&(0x80000000>>read_idx[n][0]++))?1:0);
            } else {
                read_idx[n][0]++;
                return bits|1;
            }
          default:
            read_idx[n][0]++;
            return bits;
        }
    }
}

void S9xDoAutoJoypad(void){
    int n, i, j;

    S9xSetJoypadLatch(1);
    S9xSetJoypadLatch(0);
	S9xMovieUpdate(false);

    for(n=0; n<2; n++){
        switch(i=curcontrollers[n]){
          case MP5:
            j=FLAG_IOBIT(n)?0:2;
            for(i=0; i<2; i++, j++){
                if(mp5[n].pads[j]==NONE){
                    WRITE_WORD(Memory.FillRAM+0x4218+n*2+i*4, 0);
                } else {
                    WRITE_WORD(Memory.FillRAM+0x4218+n*2+i*4, joypad[mp5[n].pads[j]-JOYPAD0].buttons);
                }
            }
            read_idx[n][FLAG_IOBIT(n)?0:1]=16;
            break;
          case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
          case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
            read_idx[n][0]=16;
            WRITE_WORD(Memory.FillRAM+0x4218+n*2, joypad[i-JOYPAD0].buttons);
            WRITE_WORD(Memory.FillRAM+0x421c+n*2, 0);
            break;
          case MOUSE0: case MOUSE1:
            read_idx[n][0]=16;
            WRITE_WORD(Memory.FillRAM+0x4218+n*2, mouse[i-MOUSE0].buttons);
            WRITE_WORD(Memory.FillRAM+0x421c+n*2, 0);
            break;
          case SUPERSCOPE:
            read_idx[n][0]=16;
            Memory.FillRAM[0x4218+n*2]=0xff;
            Memory.FillRAM[0x4219+n*2]=superscope.read_buttons;
            WRITE_WORD(Memory.FillRAM+0x421c+n*2, 0);
            break;
          case ONE_JUSTIFIER: case TWO_JUSTIFIERS:
            read_idx[n][0]=16;
            WRITE_WORD(Memory.FillRAM+0x4218+n*2, 0x000e);
            WRITE_WORD(Memory.FillRAM+0x421c+n*2, 0);
            break;
          default:
            WRITE_WORD(Memory.FillRAM+0x4218+n*2, 0);
            WRITE_WORD(Memory.FillRAM+0x421c+n*2, 0);
            break;
        }
    }
}

void S9xControlEOF(void){
    int i, j, n;
    struct crosshair *c;

    PPU.GunVLatch=1000; /* i.e., never latch */
    PPU.GunHLatch=0;

    for(n=0; n<2; n++){
        switch(i=curcontrollers[n]){
          case MP5:
            for(j=0, i=mp5[n].pads[j]; j<4; i=mp5[n].pads[++j]){
                if(i==NONE) continue;
                if(++joypad[i-JOYPAD0].turbo_ct>=turbo_time){
                    joypad[i-JOYPAD0].turbo_ct=0;
                    joypad[i-JOYPAD0].buttons ^= joypad[i-JOYPAD0].turbos;
                }
            }
            break;
          case JOYPAD0: case JOYPAD1: case JOYPAD2: case JOYPAD3:
          case JOYPAD4: case JOYPAD5: case JOYPAD6: case JOYPAD7:
            if(++joypad[i-JOYPAD0].turbo_ct>=turbo_time){
                joypad[i-JOYPAD0].turbo_ct=0;
                joypad[i-JOYPAD0].buttons ^= joypad[i-JOYPAD0].turbos;
            }
            break;
          case MOUSE0: case MOUSE1:
            c=&mouse[i-MOUSE0].crosshair;
            if(IPPU.RenderThisFrame)
                S9xDrawCrosshair(S9xGetCrosshair(c->img), c->fg, c->bg,
                                 mouse[i-MOUSE0].cur_x, mouse[i-MOUSE0].cur_y);
            break;
          case SUPERSCOPE:
            if(n==1 && !(superscope.phys_buttons&SUPERSCOPE_OFFSCREEN)){
                if(superscope.next_buttons&(SUPERSCOPE_FIRE|SUPERSCOPE_CURSOR))
                    DoGunLatch(superscope.x, superscope.y);

                c=&superscope.crosshair;
                if(IPPU.RenderThisFrame)
                    S9xDrawCrosshair(S9xGetCrosshair(c->img), c->fg, c->bg,
                                     superscope.x, superscope.y);
            }
            break;
          case TWO_JUSTIFIERS:
            if(n==1 && !justifier.offscreen[1]){
                c=&justifier.crosshair[1];
                if(IPPU.RenderThisFrame)
                    S9xDrawCrosshair(S9xGetCrosshair(c->img), c->fg, c->bg,
                                     justifier.x[1], justifier.y[1]);
            }
            i=(justifier.buttons&JUSTIFIER_SELECT)?1:0;
            goto do_justifier;
          case ONE_JUSTIFIER:
            i=(justifier.buttons&JUSTIFIER_SELECT)?-1:0;
do_justifier:
            if(n==1){
                if(i>=0 && !justifier.offscreen[i])
                    DoGunLatch(justifier.x[i], justifier.y[i]);

                if(!justifier.offscreen[0]){
                    c=&justifier.crosshair[0];
                    if(IPPU.RenderThisFrame)
                        S9xDrawCrosshair(S9xGetCrosshair(c->img), c->fg, c->bg,
                                         justifier.x[0], justifier.y[0]);
                }
            }
            break;
          default:
            break;
        }
    }

    for(n=0; n<8; n++){
        if(!pseudopointer[n].mapped) continue;
        if(pseudopointer[n].H_adj){
            pseudopointer[n].x+=pseudopointer[n].H_adj;
            if(pseudopointer[n].x<0) pseudopointer[n].x=0;
            if(pseudopointer[n].x>255) pseudopointer[n].x=255;
            if(pseudopointer[n].H_var){
                if(pseudopointer[n].H_adj<0){
                    if(pseudopointer[n].H_adj>-ptrspeeds[3]) pseudopointer[n].H_adj--;
                } else {
                    if(pseudopointer[n].H_adj<ptrspeeds[3]) pseudopointer[n].H_adj++;
                }
            }
        }
        if(pseudopointer[n].V_adj){
            pseudopointer[n].y+=pseudopointer[n].V_adj;
            if(pseudopointer[n].y<0) pseudopointer[n].y=0;
            if(pseudopointer[n].y>PPU.ScreenHeight-1) pseudopointer[n].y=PPU.ScreenHeight-1;
            if(pseudopointer[n].V_var){
                if(pseudopointer[n].V_adj<0){
                    if(pseudopointer[n].V_adj>-ptrspeeds[3]) pseudopointer[n].V_adj--;
                } else {
                    if(pseudopointer[n].V_adj<ptrspeeds[3]) pseudopointer[n].V_adj++;
                }
            }
        }
        S9xReportPointer(PseudoPointerBase+n, pseudopointer[n].x, pseudopointer[n].y);
    }

    set<struct exemulti *>::iterator it, jt;
    for(it=exemultis.begin(); it!=exemultis.end(); it++){
        i=ApplyMulti((*it)->script, (*it)->pos, (*it)->data1);
        if(i>=0){
            (*it)->pos=i;
        } else {
            jt=it;
            it--;
            delete *jt;
            exemultis.erase(jt);
        }
    }
    do_polling(POLL_ALL);
    S9xMovieUpdate();
	pad_read_last = pad_read;
	pad_read = false;
}

void S9xSetControllerCrosshair(enum crosscontrols ctl, int8 idx, const char *fg, const char *bg){
    struct crosshair *c;
    int8 fgcolor=-1, bgcolor=-1;
    int i, j;

    if(idx<-1 || idx>31){
        fprintf(stderr, "S9xSetControllerCrosshair() called with invalid index\n");
        return;
    }

    switch(ctl){
      case X_MOUSE1: c=&mouse[0].crosshair; break;
      case X_MOUSE2: c=&mouse[1].crosshair; break;
      case X_SUPERSCOPE: c=&superscope.crosshair; break;
      case X_JUSTIFIER1: c=&justifier.crosshair[0]; break;
      case X_JUSTIFIER2: c=&justifier.crosshair[1]; break;
      default:
        fprintf(stderr, "S9xSetControllerCrosshair() called with an invalid controller ID %d\n", ctl);
        return;
    }

    if(fg!=NULL){
        fgcolor=0;
        if(*fg=='t'){ fg++; fgcolor=16; }
        for(i=0; i<16; i++){
            for(j=0; color_names[i][j] && fg[j]==color_names[i][j]; j++);
            if(isalnum(fg[j])) continue;
            if(!color_names[i][j]) break;
        }
        fgcolor|=i;
        if(i>15 || fgcolor==16){
            fprintf(stderr, "S9xSetControllerCrosshair() called with invalid fgcolor\n");
            return;
        }
    }

    if(bg!=NULL){
        bgcolor=0;
        if(*bg=='t'){ bg++; bgcolor=16; }
        for(i=0; i<16; i++){
            for(j=0; color_names[i][j] && bg[j]==color_names[i][j]; j++);
            if(isalnum(bg[j])) continue;
            if(!color_names[i][j]) break;
        }
        bgcolor|=i;
        if(i>15 || bgcolor==16){
            fprintf(stderr, "S9xSetControllerCrosshair() called with invalid bgcolor\n");
            return;
        }
    }

    if(idx!=-1){ c->set|=1; c->img=idx; }
    if(fgcolor!=-1){ c->set|=2; c->fg=fgcolor; }
    if(bgcolor!=-1){c->set|=4; c->bg=bgcolor; }
}

void S9xGetControllerCrosshair(enum crosscontrols ctl, int8 *idx, const char **fg, const char **bg){
    struct crosshair *c;

    switch(ctl){
      case X_MOUSE1: c=&mouse[0].crosshair; break;
      case X_MOUSE2: c=&mouse[1].crosshair; break;
      case X_SUPERSCOPE: c=&superscope.crosshair; break;
      case X_JUSTIFIER1: c=&justifier.crosshair[0]; break;
      case X_JUSTIFIER2: c=&justifier.crosshair[1]; break;
      default:
        fprintf(stderr, "S9xGetControllerCrosshair() called with an invalid controller ID %d\n", ctl);
        return;
    }

    if(idx!=NULL) *idx=c->img;
    if(fg!=NULL) *fg=color_names[c->fg];
    if(bg!=NULL) *bg=color_names[c->bg];
}

uint16 MovieGetJoypad(int i){
    if(i<0 || i>7) return 0;
    return joypad[i].buttons;
}

void MovieSetJoypad(int i, uint16 buttons){
    if(i<0 || i>7) return;
    joypad[i].buttons=buttons;
}

// from movie.cpp, used for MovieGetX functions to avoid platform-dependent byte order in the file
extern void Write16(uint16 v, uint8*& ptr);
extern uint16 Read16(const uint8*& ptr);

bool MovieGetMouse(int i, uint8 out [5]){
    if(i<0 || i>1 || (curcontrollers[i] != MOUSE0 && curcontrollers[i] != MOUSE1)) return false;
	const int n = curcontrollers[i]-MOUSE0;
	uint8* ptr = out;
	Write16(mouse[n].cur_x, ptr);
	Write16(mouse[n].cur_y, ptr);
	*ptr++ = mouse[n].buttons;
	return true;
}

void MovieSetMouse(int i, const uint8 in [5], bool inPolling){
    if(i<0 || i>1 || (curcontrollers[i] != MOUSE0 && curcontrollers[i] != MOUSE1)) return;
	const int n = curcontrollers[i]-MOUSE0;
	const uint8* ptr = in;
	mouse[n].cur_x = Read16(ptr);
	mouse[n].cur_y = Read16(ptr);
	mouse[n].buttons = *ptr++;
	if(inPolling)
		UpdatePolledMouse(curcontrollers[i]);
}

bool MovieGetScope(int i, uint8 out [6]){
    if(i<0 || i>1 || (curcontrollers[i] != SUPERSCOPE)) return false;
	uint8* ptr = out;
	Write16(superscope.x, ptr);
	Write16(superscope.y, ptr);
	*ptr++ = superscope.phys_buttons;
	*ptr++ = superscope.next_buttons;
	return true;
}

void MovieSetScope(int i, const uint8 in [6]){
    if(i<0 || i>1 || (curcontrollers[i] != SUPERSCOPE)) return;
	const uint8* ptr = in;
	superscope.x = Read16(ptr);
	superscope.y = Read16(ptr);
	superscope.phys_buttons = *ptr++;
	superscope.next_buttons = *ptr++;
}

bool MovieGetJustifier(int i, uint8 out [11]){
    if(i<0 || i>1 || (curcontrollers[i] != ONE_JUSTIFIER && curcontrollers[i] != TWO_JUSTIFIERS)) return false;
	uint8* ptr = out;
	Write16(justifier.x[0], ptr);
	Write16(justifier.x[1], ptr);
	Write16(justifier.y[0], ptr);
	Write16(justifier.y[1], ptr);
	*ptr++ = justifier.buttons;
	*ptr++ = justifier.offscreen[0];
	*ptr++ = justifier.offscreen[1];
	return true;
}

void MovieSetJustifier(int i, const uint8 in [11]){
    if(i<0 || i>1 || (curcontrollers[i] != ONE_JUSTIFIER && curcontrollers[i] != TWO_JUSTIFIERS)) return;
	const uint8* ptr = in;
	justifier.x[0] = Read16(ptr);
	justifier.x[1] = Read16(ptr);
	justifier.y[0] = Read16(ptr);
	justifier.y[1] = Read16(ptr);
	justifier.buttons = *ptr++;
	justifier.offscreen[0] = *ptr++;
	justifier.offscreen[1] = *ptr++;
}

void S9xControlPreSave(struct SControlSnapshot *s){
    int i, j;

    ZeroMemory(s, sizeof(*s));
    s->ver=3;
    for(i=0; i<2; i++){
        s->port1_read_idx[i]=read_idx[0][i];
        s->port2_read_idx[i]=read_idx[1][i];
    }
    for(i=0; i<2; i++){
        s->mouse_speed[i]=(mouse[i].buttons&0x30)>>4;
    }
    s->justifier_select=((justifier.buttons&JUSTIFIER_SELECT)?1:0);

#define COPY(x) {memcpy((char*)s->internal+i, &(x), sizeof(x)); i+=sizeof(x);}
	i=0;
	for(j=0; j<8; j++)
		COPY(joypad[j].buttons);
	for(j=0; j<2; j++) {
		COPY(mouse[j].delta_x);
		COPY(mouse[j].delta_y);
		COPY(mouse[j].old_x);
		COPY(mouse[j].old_y);
		COPY(mouse[j].cur_x);
		COPY(mouse[j].cur_y);
		COPY(mouse[j].buttons);
	}
	COPY(superscope.x);
	COPY(superscope.y);
	COPY(superscope.phys_buttons);
	COPY(superscope.next_buttons);
	COPY(superscope.read_buttons);
	for(j=0; j<2; j++) COPY(justifier.x[j]);
	for(j=0; j<2; j++) COPY(justifier.y[j]);
	COPY(justifier.buttons);
	for(j=0; j<2; j++) COPY(justifier.offscreen[j]);
	for(j=0; j<2; j++)
		for(int k=0; k<2; k++)
			COPY(mp5[j].pads[k]);
	assert(i==sizeof(s->internal));
#undef COPY
	s->pad_read=pad_read;
	s->pad_read_last=pad_read_last;
}

void S9xControlPostLoad(struct SControlSnapshot *s){
    int i, j;

    if(curcontrollers[0]==MP5 && s->ver<1){
        // Crap. Old snes9x didn't support this.
        S9xMessage(S9X_WARNING, S9X_FREEZE_FILE_INFO, "Old savestate has no support for MP5 in port 1.");
        newcontrollers[0]=curcontrollers[0];
        curcontrollers[0]=mp5[0].pads[0];
    }
    for(i=0; i<2; i++){
        read_idx[0][i]=s->port1_read_idx[i];
        read_idx[1][i]=s->port2_read_idx[i];
    }

    mouse[0].buttons|=(s->mouse_speed[0]&3)<<4;
    mouse[1].buttons|=(s->mouse_speed[1]&3)<<4;
    if(s->justifier_select&1){
        justifier.buttons|=JUSTIFIER_SELECT;
    } else {
        justifier.buttons&=~JUSTIFIER_SELECT;
    }
    FLAG_LATCH=(Memory.FillRAM[0x4016]&1)==1;

	if(s->ver>1)
	{
#define COPY(x) {memcpy(&(x), (char*)s->internal+i, sizeof(x)); i+=sizeof(x);}
		i=0;
		for(j=0; j<8; j++)
			COPY(joypad[j].buttons);
		for(j=0; j<2; j++) {
			COPY(mouse[j].delta_x);
			COPY(mouse[j].delta_y);
			COPY(mouse[j].old_x);
			COPY(mouse[j].old_y);
			COPY(mouse[j].cur_x);
			COPY(mouse[j].cur_y);
			COPY(mouse[j].buttons);
		}
		COPY(superscope.x);
		COPY(superscope.y);
		COPY(superscope.phys_buttons);
		COPY(superscope.next_buttons);
		COPY(superscope.read_buttons);
		for(j=0; j<2; j++) COPY(justifier.x[j]);
		for(j=0; j<2; j++) COPY(justifier.y[j]);
		COPY(justifier.buttons);
		for(j=0; j<2; j++) COPY(justifier.offscreen[j]);
		for(j=0; j<2; j++)
			for(int k=0; k<2; k++)
				COPY(mp5[j].pads[k]);
		assert(i==sizeof(s->internal));
#undef COPY
	}
	if(s->ver>2)
	{
		pad_read=s->pad_read;
		pad_read_last=s->pad_read_last;
	}
}
