/***********************************************************************************
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


#import <Cocoa/Cocoa.h>

#import "mac-cocoatools.h"


void CocoaPlayFreezeDefrostSound (void)
{
	NSAutoreleasePool	*pool;
	NSBundle			*bundle;
	NSString			*path;
	NSSound				*sound;
	BOOL				r;

	pool = [[NSAutoreleasePool alloc] init];

	bundle = [NSBundle mainBundle];
	path = [bundle pathForSoundResource: @"freeze_defrost"];
	if (path)
	{
		sound = [[NSSound alloc] initWithContentsOfFile: path byReference: YES];
		if (sound)
		{
			r = [sound play];
			[sound release];
		}
	}

	[pool release];
}

void CocoaAddStatTextToView (NSView *view, NSString *label, float x, float y, float w, float h, NSTextField **out)
{
	NSTextField	*control;

	control = [[[NSTextField alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setStringValue: NSLocalizedString(label, @"")];
	[control setBezeled: NO];
	[control setDrawsBackground: NO];
	[control setEditable: NO];
	[control setSelectable: NO];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddEditTextToView (NSView *view, NSString *label, float x, float y, float w, float h, NSTextField **out)
{
	NSTextField	*control;

	control = [[[NSTextField alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setStringValue: NSLocalizedString(label, @"")];
	[control setBezeled: YES];
	[control setDrawsBackground: YES];
	[control setEditable: YES];
	[control setSelectable: YES];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddMPushBtnToView (NSView *view, NSString *label, float x, float y, float w, float h, NSButton **out)
{
	NSButton	*control;

	control = [[[NSButton alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setTitle: NSLocalizedString(label, @"")];
	[control setBezelStyle: NSRoundedBezelStyle];
	[control setButtonType: NSMomentaryPushInButton];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddCheckBoxToView (NSView *view, NSString *label, float x, float y, float w, float h, NSButton **out)
{
	NSButton	*control;

	control = [[[NSButton alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setTitle: NSLocalizedString(label, @"")];
	[control setButtonType: NSSwitchButton];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddPopUpBtnToView (NSView *view, NSArray *array, float x, float y, float w, float h, NSPopUpButton **out)
{
	NSPopUpButton	*control;
	NSMenu			*menu;
	int				n;

	menu = [[[NSMenu alloc] init] autorelease];

	n = [array count];
	for (int i = 0; i < n; i++)
	{
		NSString	*item = [array objectAtIndex: i];
		if ([item isEqualToString: @"---"])
			[menu addItem: [NSMenuItem separatorItem]];
		else
			[menu addItemWithTitle: item action: NULL keyEquivalent: @""];
	}

	control = [[[NSPopUpButton alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setPullsDown: NO];
	[control setMenu: menu];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}
