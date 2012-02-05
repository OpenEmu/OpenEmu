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


#import "snes9x.h"
#import "memmap.h"
#import "apu.h"
#import "snapshot.h"

#import <Cocoa/Cocoa.h>
#import <sys/time.h>
#import <pthread.h>

#import "mac-prefix.h"
#import "mac-audio.h"
#import "mac-file.h"
#import "mac-os.h"
#import "mac-musicbox.h"

volatile bool8			mboxPause = false;

static volatile bool8	stopNow, showIndicator, headPressed;
static int32			oldCPUCycles;
static uint16			stereo_switch;
static uint8			storedSoundSnapshot[SPC_SAVE_STATE_BLOCK_SIZE];

static void SPCPlayExec (void);
static void SPCPlayFreeze (void);
static void SPCPlayDefrost (void);
static void MusicBoxForceFreeze (void);
static void MusicBoxForceDefrost (void);
static void * SoundTask (void *);


@implementation MusicBoxController

- (id) init
{
	NSUserDefaults	*defaults;
	NSString		*s;
	NSRect			rect;
	NSSize			size;
	BOOL			apuonly, r;
	char			drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	self = [super init];
	if (!self)
		return (self);

	r = [NSBundle loadNibNamed: @"musicbox" owner: self];
	if (!r)
		return (self);

	apuonly = (musicboxmode == kMBXSoundEmulation);

	if (apuonly)
		SPCPlayFreeze();
	else
		MusicBoxForceFreeze();

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	[gametitle setStringValue: [NSString stringWithUTF8String: fname]];

	[led setImage: [NSImage imageNamed: (apuonly ? @"musicbox_ledoff.icns" : @"musicbox_ledon.icns")]];

	if (!apuonly)
	{
		[rewind setState: NSOffState];
		[rewind setEnabled: NO];
	}

	defaults = [NSUserDefaults standardUserDefaults];
	s = [defaults stringForKey: @"frame_musicbox"];
	if (s)
	{
		rect = NSRectFromString(s);
		[window setFrame: rect display: NO];
	}

	if (!savewindowpos)
		[window center];

	size = [window minSize];
	mbxClosedHeight = size.height;
	size = [window maxSize];
	mbxOpenedHeight = size.height;

	rect = [window frame];
	showIndicator = (rect.size.height > mbxClosedHeight) ? true : false;

	[disclosure setState: (showIndicator ? NSOnState : NSOffState)];

	[window makeKeyAndOrderFront: self];

	mboxPause   = false;
	headPressed = false;

	stereo_switch = ~0;
	spc_core->dsp_set_stereo_switch(stereo_switch);

	for (int i = 0; i < MAC_MAX_PLAYERS; i++)
		controlPad[i] = 0;

	stopNow = false;
	MacStartSound();
	pthread_create(&mbxThread, NULL, SoundTask, NULL);

	timer = [[NSTimer scheduledTimerWithTimeInterval: (2.0 / (double) Memory.ROMFramesPerSecond) target: self selector: @selector(updateIndicator:) userInfo: nil repeats: YES] retain];

	return (self);
}

- (void) windowWillClose: (NSNotification *) aNotification
{
	NSUserDefaults	*defaults;
	NSString		*s;
	BOOL			r;

	[timer invalidate];
	[timer release];

	showIndicator = false;

	stopNow = true;
	pthread_join(mbxThread, NULL);
	MacStopSound();

	defaults = [NSUserDefaults standardUserDefaults];
	s = NSStringFromRect([window frame]);
	[defaults setObject: s forKey: @"frame_musicbox"];
	r = [defaults synchronize];

	[NSApp stopModal];
}

- (void) dealloc
{
	stereo_switch = ~0;
	spc_core->dsp_set_stereo_switch(stereo_switch);

	if (musicboxmode == kMBXSoundEmulation)
		SPCPlayDefrost();
	else
		MusicBoxForceDefrost();

	[window release];

	[super dealloc];
}

- (NSWindow *) window
{
	return (window);
}

- (IBAction) handlePauseButton: (id) sender
{
	mboxPause = !mboxPause;
	S9xSetSoundMute(mboxPause);
}

- (IBAction) handleRewindButton: (id) sender
{
	headPressed = true;
}

- (IBAction) handleEffectButton: (id) sender
{
	[window orderOut: self];
	showIndicator = false;
	ConfigureSoundEffects();
	showIndicator = true;
	[window makeKeyAndOrderFront: self];
}

- (IBAction) handleChannelButton: (id) sender
{
	stereo_switch ^= (1 << [sender tag]);
	spc_core->dsp_set_stereo_switch(stereo_switch);
}

- (IBAction) handleDisclosureButton: (id) sender
{
	NSRect	rect;
	float	h;

	showIndicator = !showIndicator;
	rect = [window frame];
	h = rect.size.height;
	rect.size.height = showIndicator ? mbxOpenedHeight : mbxClosedHeight;
	rect.origin.y += (h - rect.size.height);
	[window setFrame: rect display: YES animate: YES];
}

- (void) updateIndicator: (NSTimer *) aTimer
{
	if (showIndicator)
		[indicator setNeedsDisplay: YES];
}

@end

@implementation MusicBoxIndicatorView

- (id) initWithFrame: (NSRect) frame
{
	self = [super initWithFrame: frame];
	if (self)
	{
		NSRect			rect;
		long long		currentTime;
		struct timeval	tv;

		mbxOffsetX   =   0.0f;
		mbxOffsetY   =   0.0f;
		mbxBarWidth  =  12.0f;
		mbxBarHeight = 128.0f;
		mbxBarSpace  =   2.0f;
		mbxLRSpace   =  20.0f;
		mbxRightBarX = (mbxLRSpace + (mbxBarWidth * 8.0f + mbxBarSpace * 7.0f));
		yyscale      = (float) (128.0 / sqrt(64.0));

		rect = [self bounds];
		mbxViewWidth  = rect.size.width;
		mbxViewHeight = rect.size.height;
		mbxMarginX = (mbxViewWidth - ((mbxBarWidth * 8.0f + mbxBarSpace * 7.0f) * 2.0f + mbxLRSpace)) / 2.0f;
		mbxMarginY = (mbxViewHeight - mbxBarHeight) / 2.0f;

		gettimeofday(&tv, NULL);
		currentTime = tv.tv_sec * 1000000 + tv.tv_usec;

		for (int i = 0; i < 8; i++)
		{
			prevLMax[i] = prevRMax[i] = 0;
			prevLVol[i] = prevRVol[i] = 0;
			barTimeL[i] = barTimeR[i] = currentTime;
		}
	}

	return (self);
}

- (void) drawRect: (NSRect) rect
{
	CGContextRef	mboxctx;

	mboxctx = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];

	// Bar

	const float	length[] = { 1.0f, 1.0f };

	CGContextSetLineWidth(mboxctx, mbxBarWidth);
	CGContextSetLineDash(mboxctx, 0, length, 2);
	CGContextSetLineJoin(mboxctx, kCGLineJoinMiter);

	CGContextBeginPath(mboxctx);

	float   x = mbxOffsetX + mbxMarginX + mbxBarWidth / 2.0f;

	for (int h = 0; h < 8; h++)
	{
		// Inactive

		CGContextSetRGBStrokeColor(mboxctx, (196.0f / 256.0f), (200.0f / 256.0f), (176.0f / 256.0f), 1.0f);

		CGContextMoveToPoint   (mboxctx, x,                mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x,                mbxOffsetY + mbxMarginY + mbxBarHeight);

		CGContextMoveToPoint   (mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + mbxBarHeight);

		CGContextStrokePath(mboxctx);

		// Max

		short			vl = (spc_core->dsp_reg_value(h, 0x00) * spc_core->dsp_envx_value(h)) >> 11;
		short			vr = (spc_core->dsp_reg_value(h, 0x01) * spc_core->dsp_envx_value(h)) >> 11;
		long long		currentTime;
		struct timeval	tv;

		if (vl <= 0) vl = 0; else if (vl > 64) vl = 64; else vl = (short) (yyscale * sqrt((double) vl)) & (~0 << 1);
		if (vr <= 0) vr = 0; else if (vr > 64) vr = 64; else vr = (short) (yyscale * sqrt((double) vr)) & (~0 << 1);

		if (vl < prevLVol[h]) vl = ((prevLVol[h] + vl) >> 1);
		if (vr < prevRVol[h]) vr = ((prevRVol[h] + vr) >> 1);

		gettimeofday(&tv, NULL);
		currentTime = tv.tv_sec * 1000000 + tv.tv_usec;

		// left

		if ((vl >= prevLMax[h]) && (vl > prevLVol[h]))
		{
			barTimeL[h] = currentTime;
			prevLMax[h] = vl;
		}
		else
		if ((prevLMax[h] > 0) && (barTimeL[h] + 1000000 > currentTime))
		{
			CGContextSetRGBStrokeColor(mboxctx, (22.0f / 256.0f), (156.0f / 256.0f), (20.0f / 256.0f), (float) (barTimeL[h] + 1000000 - currentTime) / 1000000.0f);

			CGContextMoveToPoint   (mboxctx, x, mbxOffsetY + mbxMarginY + (float) (prevLMax[h] - 2));
			CGContextAddLineToPoint(mboxctx, x, mbxOffsetY + mbxMarginY + (float) (prevLMax[h]    ));

			CGContextStrokePath(mboxctx);
		}
		else
			prevLMax[h] = 0;

		prevLVol[h] = vl;

		// right

		if ((vr >= prevRMax[h]) && (vr > prevRVol[h]))
		{
			barTimeR[h] = currentTime;
			prevRMax[h] = vr;
		}
		else
		if ((prevRMax[h] > 0) && (barTimeR[h] + 1000000 > currentTime))
		{
			CGContextSetRGBStrokeColor(mboxctx, (22.0f / 256.0f), (156.0f / 256.0f), (20.0f / 256.0f), (float) (barTimeR[h] + 1000000 - currentTime) / 1000000.0f);

			CGContextMoveToPoint   (mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + (float) (prevRMax[h] - 2));
			CGContextAddLineToPoint(mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + (float) (prevRMax[h]    ));

			CGContextStrokePath(mboxctx);
		}
		else
			prevRMax[h] = 0;

		prevRVol[h] = vr;

		// Active

		CGContextSetRGBStrokeColor(mboxctx, (22.0f / 256.0f), (22.0f / 256.0f), (20.0f / 256.0f), 1.0f);

		CGContextMoveToPoint   (mboxctx, x,                mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x,                mbxOffsetY + mbxMarginY + (float) vl);
		CGContextStrokePath(mboxctx);
		CGContextMoveToPoint   (mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + (float) vr);
		CGContextStrokePath(mboxctx);

		x += (mbxBarWidth + mbxBarSpace);
	}
}

@end

static void * SoundTask (void *)
{
	long long		last, curr;
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	last = tv.tv_sec * 1000000 + tv.tv_usec;

	while (!stopNow)
	{
		if (!mboxPause)
		{
			if (musicboxmode == kMBXSoundEmulation)
				SPCPlayExec();
			else
				S9xMainLoop();
		}

		if (headPressed)
		{
			showIndicator = false;
			SPCPlayDefrost();
			showIndicator = true;

			headPressed = false;
		}

		last += (1000000 / Memory.ROMFramesPerSecond);
		gettimeofday(&tv, NULL);
		curr = tv.tv_sec * 1000000 + tv.tv_usec;

		if (last > curr)
			usleep((useconds_t) (last - curr));
	}

	return (NULL);
}

static void SPCPlayExec (void)
{
	for (int v = 0; v < Timings.V_Max; v++)
	{
		CPU.Cycles = Timings.H_Max;
		S9xAPUEndScanline();
		CPU.Cycles = 0;
		S9xAPUSetReferenceTime(0);
	}
}

static void MusicBoxForceFreeze (void)
{
	char	filename[PATH_MAX + 1];

	strcpy(filename, S9xGetFreezeFilename(999));
	strcat(filename, ".tmp");

	S9xFreezeGame(filename);
}

static void MusicBoxForceDefrost (void)
{
	char	filename[PATH_MAX + 1];

	strcpy(filename, S9xGetFreezeFilename(999));
	strcat(filename, ".tmp");

	S9xUnfreezeGame(filename);
	remove(filename);
}

static void SPCPlayFreeze (void)
{
	oldCPUCycles = CPU.Cycles;

	S9xSetSoundMute(true);
	S9xAPUSaveState(storedSoundSnapshot);
	S9xSetSoundMute(false);
}

static void SPCPlayDefrost (void)
{
	CPU.Cycles = oldCPUCycles;

	S9xSetSoundMute(true);
	S9xAPULoadState(storedSoundSnapshot);
	S9xSetSoundMute(false);
}

void MusicBoxDialog (void)
{
	MusicBoxController	*controller;
	NSAutoreleasePool	*pool;

	if (!cartOpen)
		return;

	pool = [[NSAutoreleasePool alloc] init];
	controller = [[MusicBoxController alloc] init];
	[pool release];

	if (!controller)
		return;

	[NSApp runModalForWindow: [controller window]];

	pool = [[NSAutoreleasePool alloc] init];
	[controller release];
	[pool release];
}
