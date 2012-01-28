S9xHandlePortCommand/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)
S9xHandlePortCommand
  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


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
                             Andreas Naive (andreasnaive@gmail.com),
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
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

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
 ***********************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#include "port.h"
#include "controls.h"

#include "mac-prefix.h"
#include "mac-joypad.h"
#include "mac-keyboard.h"
#include "mac-os.h"
#include "mac-controls.h"

#define	ASSIGN_BUTTONf(n, s)	S9xMapButton (n, cmd = S9xGetCommandT(s), false)
#define	ASSIGN_BUTTONt(n, s)	S9xMapButton (n, cmd = S9xGetCommandT(s), true)
#define	ASSIGN_POINTRf(n, s)	S9xMapPointer(n, cmd = S9xGetCommandT(s), false)
#define	ASSIGN_POINTRt(n, s)	S9xMapPointer(n, cmd = S9xGetCommandT(s), true)

#define KeyIsPressed(km, k)		(1 & (((unsigned char *) km) [(k) >> 3] >> ((k) & 7)))


void S9xSetupDefaultKeymap (void)
{
	s9xcommand_t	cmd;

	ASSIGN_BUTTONf(kMacCMapPad1PX,         "Joypad1 X");
	ASSIGN_BUTTONf(kMacCMapPad1PA,         "Joypad1 A");
	ASSIGN_BUTTONf(kMacCMapPad1PB,         "Joypad1 B");
	ASSIGN_BUTTONf(kMacCMapPad1PY,         "Joypad1 Y");
	ASSIGN_BUTTONf(kMacCMapPad1PL,         "Joypad1 L");
	ASSIGN_BUTTONf(kMacCMapPad1PR,         "Joypad1 R");
	ASSIGN_BUTTONf(kMacCMapPad1PSelect,    "Joypad1 Select");
	ASSIGN_BUTTONf(kMacCMapPad1PStart,     "Joypad1 Start");
	ASSIGN_BUTTONf(kMacCMapPad1PUp,        "Joypad1 Up");
	ASSIGN_BUTTONf(kMacCMapPad1PDown,      "Joypad1 Down");
	ASSIGN_BUTTONf(kMacCMapPad1PLeft,      "Joypad1 Left");
	ASSIGN_BUTTONf(kMacCMapPad1PRight,     "Joypad1 Right");

	ASSIGN_BUTTONf(kMacCMapPad2PX,         "Joypad2 X");
	ASSIGN_BUTTONf(kMacCMapPad2PA,         "Joypad2 A");
	ASSIGN_BUTTONf(kMacCMapPad2PB,         "Joypad2 B");
	ASSIGN_BUTTONf(kMacCMapPad2PY,         "Joypad2 Y");
	ASSIGN_BUTTONf(kMacCMapPad2PL,         "Joypad2 L");
	ASSIGN_BUTTONf(kMacCMapPad2PR,         "Joypad2 R");
	ASSIGN_BUTTONf(kMacCMapPad2PSelect,    "Joypad2 Select");
	ASSIGN_BUTTONf(kMacCMapPad2PStart,     "Joypad2 Start");
	ASSIGN_BUTTONf(kMacCMapPad2PUp,        "Joypad2 Up");
	ASSIGN_BUTTONf(kMacCMapPad2PDown,      "Joypad2 Down");
	ASSIGN_BUTTONf(kMacCMapPad2PLeft,      "Joypad2 Left");
	ASSIGN_BUTTONf(kMacCMapPad2PRight,     "Joypad2 Right");

	ASSIGN_BUTTONf(kMacCMapPad3PX,         "Joypad3 X");
	ASSIGN_BUTTONf(kMacCMapPad3PA,         "Joypad3 A");
	ASSIGN_BUTTONf(kMacCMapPad3PB,         "Joypad3 B");
	ASSIGN_BUTTONf(kMacCMapPad3PY,         "Joypad3 Y");
	ASSIGN_BUTTONf(kMacCMapPad3PL,         "Joypad3 L");
	ASSIGN_BUTTONf(kMacCMapPad3PR,         "Joypad3 R");
	ASSIGN_BUTTONf(kMacCMapPad3PSelect,    "Joypad3 Select");
	ASSIGN_BUTTONf(kMacCMapPad3PStart,     "Joypad3 Start");
	ASSIGN_BUTTONf(kMacCMapPad3PUp,        "Joypad3 Up");
	ASSIGN_BUTTONf(kMacCMapPad3PDown,      "Joypad3 Down");
	ASSIGN_BUTTONf(kMacCMapPad3PLeft,      "Joypad3 Left");
	ASSIGN_BUTTONf(kMacCMapPad3PRight,     "Joypad3 Right");

	ASSIGN_BUTTONf(kMacCMapPad4PX,         "Joypad4 X");
	ASSIGN_BUTTONf(kMacCMapPad4PA,         "Joypad4 A");
	ASSIGN_BUTTONf(kMacCMapPad4PB,         "Joypad4 B");
	ASSIGN_BUTTONf(kMacCMapPad4PY,         "Joypad4 Y");
	ASSIGN_BUTTONf(kMacCMapPad4PL,         "Joypad4 L");
	ASSIGN_BUTTONf(kMacCMapPad4PR,         "Joypad4 R");
	ASSIGN_BUTTONf(kMacCMapPad4PSelect,    "Joypad4 Select");
	ASSIGN_BUTTONf(kMacCMapPad4PStart,     "Joypad4 Start");
	ASSIGN_BUTTONf(kMacCMapPad4PUp,        "Joypad4 Up");
	ASSIGN_BUTTONf(kMacCMapPad4PDown,      "Joypad4 Down");
	ASSIGN_BUTTONf(kMacCMapPad4PLeft,      "Joypad4 Left");
	ASSIGN_BUTTONf(kMacCMapPad4PRight,     "Joypad4 Right");

	ASSIGN_BUTTONf(kMacCMapPad5PX,         "Joypad5 X");
	ASSIGN_BUTTONf(kMacCMapPad5PA,         "Joypad5 A");
	ASSIGN_BUTTONf(kMacCMapPad5PB,         "Joypad5 B");
	ASSIGN_BUTTONf(kMacCMapPad5PY,         "Joypad5 Y");
	ASSIGN_BUTTONf(kMacCMapPad5PL,         "Joypad5 L");
	ASSIGN_BUTTONf(kMacCMapPad5PR,         "Joypad5 R");
	ASSIGN_BUTTONf(kMacCMapPad5PSelect,    "Joypad5 Select");
	ASSIGN_BUTTONf(kMacCMapPad5PStart,     "Joypad5 Start");
	ASSIGN_BUTTONf(kMacCMapPad5PUp,        "Joypad5 Up");
	ASSIGN_BUTTONf(kMacCMapPad5PDown,      "Joypad5 Down");
	ASSIGN_BUTTONf(kMacCMapPad5PLeft,      "Joypad5 Left");
	ASSIGN_BUTTONf(kMacCMapPad5PRight,     "Joypad5 Right");

	ASSIGN_BUTTONf(kMacCMapPad6PX,         "Joypad6 X");
	ASSIGN_BUTTONf(kMacCMapPad6PA,         "Joypad6 A");
	ASSIGN_BUTTONf(kMacCMapPad6PB,         "Joypad6 B");
	ASSIGN_BUTTONf(kMacCMapPad6PY,         "Joypad6 Y");
	ASSIGN_BUTTONf(kMacCMapPad6PL,         "Joypad6 L");
	ASSIGN_BUTTONf(kMacCMapPad6PR,         "Joypad6 R");
	ASSIGN_BUTTONf(kMacCMapPad6PSelect,    "Joypad6 Select");
	ASSIGN_BUTTONf(kMacCMapPad6PStart,     "Joypad6 Start");
	ASSIGN_BUTTONf(kMacCMapPad6PUp,        "Joypad6 Up");
	ASSIGN_BUTTONf(kMacCMapPad6PDown,      "Joypad6 Down");
	ASSIGN_BUTTONf(kMacCMapPad6PLeft,      "Joypad6 Left");
	ASSIGN_BUTTONf(kMacCMapPad6PRight,     "Joypad6 Right");

	ASSIGN_BUTTONf(kMacCMapPad7PX,         "Joypad7 X");
	ASSIGN_BUTTONf(kMacCMapPad7PA,         "Joypad7 A");
	ASSIGN_BUTTONf(kMacCMapPad7PB,         "Joypad7 B");
	ASSIGN_BUTTONf(kMacCMapPad7PY,         "Joypad7 Y");
	ASSIGN_BUTTONf(kMacCMapPad7PL,         "Joypad7 L");
	ASSIGN_BUTTONf(kMacCMapPad7PR,         "Joypad7 R");
	ASSIGN_BUTTONf(kMacCMapPad7PSelect,    "Joypad7 Select");
	ASSIGN_BUTTONf(kMacCMapPad7PStart,     "Joypad7 Start");
	ASSIGN_BUTTONf(kMacCMapPad7PUp,        "Joypad7 Up");
	ASSIGN_BUTTONf(kMacCMapPad7PDown,      "Joypad7 Down");
	ASSIGN_BUTTONf(kMacCMapPad7PLeft,      "Joypad7 Left");
	ASSIGN_BUTTONf(kMacCMapPad7PRight,     "Joypad7 Right");

	ASSIGN_BUTTONf(kMacCMapPad8PX,         "Joypad8 X");
	ASSIGN_BUTTONf(kMacCMapPad8PA,         "Joypad8 A");
	ASSIGN_BUTTONf(kMacCMapPad8PB,         "Joypad8 B");
	ASSIGN_BUTTONf(kMacCMapPad8PY,         "Joypad8 Y");
	ASSIGN_BUTTONf(kMacCMapPad8PL,         "Joypad8 L");
	ASSIGN_BUTTONf(kMacCMapPad8PR,         "Joypad8 R");
	ASSIGN_BUTTONf(kMacCMapPad8PSelect,    "Joypad8 Select");
	ASSIGN_BUTTONf(kMacCMapPad8PStart,     "Joypad8 Start");
	ASSIGN_BUTTONf(kMacCMapPad8PUp,        "Joypad8 Up");
	ASSIGN_BUTTONf(kMacCMapPad8PDown,      "Joypad8 Down");
	ASSIGN_BUTTONf(kMacCMapPad8PLeft,      "Joypad8 Left");
	ASSIGN_BUTTONf(kMacCMapPad8PRight,     "Joypad8 Right");

	ASSIGN_BUTTONt(kMacCMapMouse1PL,       "Mouse1 L");
	ASSIGN_BUTTONt(kMacCMapMouse1PR,       "Mouse1 R");
	ASSIGN_BUTTONt(kMacCMapMouse2PL,       "Mouse2 L");
	ASSIGN_BUTTONt(kMacCMapMouse2PR,       "Mouse2 R");

	ASSIGN_BUTTONt(kMacCMapScopeOffscreen, "Superscope AimOffscreen");
	ASSIGN_BUTTONt(kMacCMapScopeFire,      "Superscope Fire");
	ASSIGN_BUTTONt(kMacCMapScopeCursor,    "Superscope Cursor");
	ASSIGN_BUTTONt(kMacCMapScopeTurbo,     "Superscope ToggleTurbo");
	ASSIGN_BUTTONt(kMacCMapScopePause,     "Superscope Pause");

	ASSIGN_BUTTONt(kMacCMapLGun1Offscreen, "Justifier1 AimOffscreen");
	ASSIGN_BUTTONt(kMacCMapLGun1Trigger,   "Justifier1 Trigger");
	ASSIGN_BUTTONt(kMacCMapLGun1Start,     "Justifier1 Start");
	ASSIGN_BUTTONt(kMacCMapLGun2Offscreen, "Justifier2 AimOffscreen");
	ASSIGN_BUTTONt(kMacCMapLGun2Trigger,   "Justifier2 Trigger");
	ASSIGN_BUTTONt(kMacCMapLGun2Start,     "Justifier2 Start");

	ASSIGN_POINTRt(kMacCMapMouse1Pointer,     "Pointer Mouse1");
	ASSIGN_POINTRt(kMacCMapMouse2Pointer,     "Pointer Mouse2");
	ASSIGN_POINTRt(kMacCMapSuperscopePointer, "Pointer Superscope");
	ASSIGN_POINTRt(kMacCMapJustifier1Pointer, "Pointer Justifier1");

	ASSIGN_POINTRf(PseudoPointerBase,         "Pointer Justifier2");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 0, "ButtonToPointer 1u Med");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 1, "ButtonToPointer 1d Med");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 2, "ButtonToPointer 1l Med");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 3, "ButtonToPointer 1r Med");
}

bool S9xPollButton (uint32 id, bool *pressed)
{
	#define kmControlKey	0x3B

	KeyMap	keys;

	GetKeys(keys);

	*pressed = false;

	if (id & k_MO)	// mouse
	{
		switch (id & 0xFF)
		{
			case 0:	*pressed = ISpKeyIsPressed(kISpMouseL);														break;
			case 1: *pressed = ISpKeyIsPressed(kISpMouseR);
		}
	}
	else
	if (id & k_SS)	// superscope
	{
		switch (id & 0xFF)
		{
			case 0:	*pressed = ISpKeyIsPressed(kISpOffScreen) | KeyIsPressed(keys, keyCode[kKeyOffScreen]);		break;
			case 2:	*pressed = ISpKeyIsPressed(kISpScopeC)    | KeyIsPressed(keys, keyCode[kKeyScopeCursor]);	break;
			case 3:	*pressed = ISpKeyIsPressed(kISpScopeT)    | KeyIsPressed(keys, keyCode[kKeyScopeTurbo]);	break;
			case 4:	*pressed = ISpKeyIsPressed(kISpScopeP)    | KeyIsPressed(keys, keyCode[kKeyScopePause]);	break;
			case 1:	*pressed = ISpKeyIsPressed(kISpMouseL);
		}
	}
	else
	if (id & k_LG)	// justifier
	{
		if (id & k_C1)
		{
			switch (id & 0xFF)
			{
				case 0: *pressed = ISpKeyIsPressed(kISpOffScreen) | KeyIsPressed(keys, keyCode[kKeyOffScreen]);	break;
				case 1:	*pressed = ISpKeyIsPressed(kISpMouseL);													break;
				case 2: *pressed = ISpKeyIsPressed(kISpMouseR);
			}
		}
		else
		{
			switch (id & 0xFF)
			{
				case 0: *pressed = ISpKeyIsPressed(kISp2PStart)   | KeyIsPressed(keys, keyCode[k2PStart]);		break;
				case 1:	*pressed = ISpKeyIsPressed(kISp2PB)       | KeyIsPressed(keys, keyCode[k2PB]);			break;
				case 2: *pressed = ISpKeyIsPressed(kISp2PA)       | KeyIsPressed(keys, keyCode[k2PA]);
			}
		}
	}

	return (true);
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
	if (id & k_PT)
	{
		if ((id & k_MO) && fullscreen)
			GetGameScreenPointer(x, y, true);
		else
			GetGameScreenPointer(x, y, false);
	}
	else
		*x = *y = 0;

	return (true);
}

bool S9xPollAxis (uint32 id, int16 *value)
{
	return (false);
}

s9xcommand_t S9xGetPortCommandT (const char *name)
{
	s9xcommand_t	cmd;

    ZeroMemory(&cmd, sizeof(cmd));
    cmd.type = S9xBadMapping;

	return (cmd);
}

char * S9xGetPortCommandName (s9xcommand_t command)
{
	static char	str[] = "PortCommand";

	return (str);
}

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

bool8 S9xMapInput (const char *name, s9xcommand_t *cmd)
{
	return (true);
}

void ControlPadFlagsToS9xReportButtons (int n, uint32 p)
{
	uint32	base = k_HD | k_BT | k_JP | (0x100 << n);

	S9xReportButton(base +  0, (p & 0x0040));
	S9xReportButton(base +  1, (p & 0x0080));
	S9xReportButton(base +  2, (p & 0x8000));
	S9xReportButton(base +  3, (p & 0x4000));
	S9xReportButton(base +  4, (p & 0x0020));
	S9xReportButton(base +  5, (p & 0x0010));
	S9xReportButton(base +  6, (p & 0x2000));
	S9xReportButton(base +  7, (p & 0x1000));
	S9xReportButton(base +  8, (p & 0x0800));
	S9xReportButton(base +  9, (p & 0x0400));
	S9xReportButton(base + 10, (p & 0x0200));
	S9xReportButton(base + 11, (p & 0x0100));
}

void ControlPadFlagsToS9xPseudoPointer (uint32 p)
{
	// prevent screwiness caused by trying to move the pointer left+right or up+down
	if ((p & 0x0c00) == 0x0c00) p &= ~0x0c00;
	if ((p & 0x0300) == 0x0300) p &= ~0x0300;

	// checks added to prevent a lack of right/down movement from breaking left/up movement
	if (!(p & 0x0400))
		S9xReportButton(kMacCMapPseudoPtrBase + 0, (p & 0x0800));
	if (!(p & 0x0800))
		S9xReportButton(kMacCMapPseudoPtrBase + 1, (p & 0x0400));
	if (!(p & 0x0100))
		S9xReportButton(kMacCMapPseudoPtrBase + 2, (p & 0x0200));
	if (!(p & 0x0200))
		S9xReportButton(kMacCMapPseudoPtrBase + 3, (p & 0x0100));
}
