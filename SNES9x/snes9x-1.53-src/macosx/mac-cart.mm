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
#import "snapshot.h"
#import "movie.h"

#import <wchar.h>
#import <Cocoa/Cocoa.h>

#import "mac-cocoatools.h"
#import "mac-prefix.h"
#import "mac-dialog.h"
#import "mac-os.h"
#import "mac-quicktime.h"
#import "mac-screenshot.h"
#import "mac-snes9x.h"
#import "mac-stringtools.h"
#import "mac-cart.h"

typedef struct
{
	NavDialogRef	nref;
	WindowRef		parent;
	FSRef			ref;
	Boolean			reply;
	char			name[PATH_MAX + 1];
#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	IBNibRef		customNib;
	WindowRef		customWindow;
	HIViewRef		customPane;
	short			customWidth, customHeight;
#endif
}	NavState;

extern wchar_t	macRecordWChar[MOVIE_MAX_METADATA];

static NavEventUPP			gSheetEventUPP;
static NavObjectFilterUPP	gSheetFilterUPP;
static NavState				gSheetNav;
static NSAutoreleasePool	*gPool;
static NSPopUpButton		*gPopM, *gPopI, *gPopV, *gPopH;

static pascal void NavGenericOpenEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavGenericSaveEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal Boolean NavOpenCartFilter (AEDesc *, void *, NavCallBackUserData, NavFilterModes);
static pascal Boolean NavDefrostFromFilter (AEDesc *, void *, NavCallBackUserData, NavFilterModes);
static pascal Boolean NavPlayMovieFromFilter (AEDesc *, void *, NavCallBackUserData, NavFilterModes);
#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
static void GlobalPointToWindowLocalPoint (Point *, WindowRef);
static pascal void NavOpenCartEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavRecordMovieToEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavPlayMovieFromEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavQTMovieRecordToEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
#endif
#ifdef MAC_TIGER_PANTHER_SUPPORT
static pascal Boolean NavDefrostFromPreview (NavCBRecPtr, NavCallBackUserData);
static pascal Boolean NavPlayMovieFromPreview (NavCBRecPtr, NavCallBackUserData);
#endif


@interface NavController : NSObject

- (id) init;
- (void) dealloc;
- (IBAction) handleCompressButton: (id) sender;

@end

@implementation NavController

- (id) init
{
	self = [super init];
	return (self);
}

- (void) dealloc
{
	[super dealloc];
}

- (IBAction) handleCompressButton: (id) sender
{
	MacQTVideoConfig();
}

@end


bool8 NavOpenROMImage (FSRef *ref)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavObjectFilterUPP			filterUPP;
	NavState					nav;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		HIViewID	cid = { 'PANE', 1000 };
		Rect		rct;

		err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
		if (err)
			return (false);

		err = CreateWindowFromNib(nav.customNib, CFSTR("OpenROM"), &(nav.customWindow));
		if (err)
		{
			DisposeNibReference(nav.customNib);
			return (false);
		}

		GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
		nav.customWidth  = rct.right  - rct.left;
		nav.customHeight = rct.bottom - rct.top;

		HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
	}
#endif

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.preferenceKey = 3;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = CFCopyLocalizedString(CFSTR("OpenROMMes"), "Open");
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;
	nav.parent = NULL;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavOpenCartEventHandler : NavGenericOpenEventHandler);
#else
	eventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
#endif
	filterUPP = NewNavObjectFilterUPP(NavOpenCartFilter);

	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, NULL, filterUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		if (systemVersion >= 0x1060)
		{
			NSAutoreleasePool	*pool;
			NSView				*view;
			NSTextField			*txtM, *txtI, *txtV, *txtH;
			NSPopUpButton		*popM, *popI, *popV, *popH;
			NSArray				*aryM, *aryI, *aryV, *aryH;

			pool = [[NSAutoreleasePool alloc] init];

			view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 52)] autorelease];

			aryM = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM6", @""), NSLocalizedString(@"OpenROM7", @""), nil];
			aryI = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMa", @""), NSLocalizedString(@"OpenROMb", @""), NSLocalizedString(@"OpenROMc", @""), NSLocalizedString(@"OpenROMd", @""), nil];
			aryV = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM8", @""), NSLocalizedString(@"OpenROM9", @""), nil];
			aryH = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMe", @""), NSLocalizedString(@"OpenROMf", @""), nil];

			CocoaAddStatTextToView(view, @"OpenROM1",   5, 32,  84, 14, &txtM);
			CocoaAddStatTextToView(view, @"OpenROM3", 214, 32,  96, 14, &txtI);
			CocoaAddStatTextToView(view, @"OpenROM2",   5,  7,  84, 14, &txtV);
			CocoaAddStatTextToView(view, @"OpenROM4", 214,  7,  96, 14, &txtH);
			CocoaAddPopUpBtnToView(view, aryM,         91, 27, 116, 22, &popM);
			CocoaAddPopUpBtnToView(view, aryI,        312, 27, 116, 22, &popI);
			CocoaAddPopUpBtnToView(view, aryV,         91,  2, 116, 22, &popV);
			CocoaAddPopUpBtnToView(view, aryH,        312,  2, 116, 22, &popH);

			[txtM setAlignment: NSRightTextAlignment];
			[txtI setAlignment: NSRightTextAlignment];
			[txtV setAlignment: NSRightTextAlignment];
			[txtH setAlignment: NSRightTextAlignment];

			[popM selectItemAtIndex: romDetect       ];
			[popI selectItemAtIndex: interleaveDetect];
			[popV selectItemAtIndex: videoDetect     ];
			[popH selectItemAtIndex: headerDetect    ];

			[(NSOpenPanel *) nav.nref setAccessoryView: view];

			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);

			romDetect        = [popM indexOfSelectedItem];
			interleaveDetect = [popI indexOfSelectedItem];
			videoDetect      = [popV indexOfSelectedItem];
			headerDetect     = [popH indexOfSelectedItem];

			[pool release];
		}
	#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
		else
		{
			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);
		}
	#endif
	}

	DisposeNavObjectFilterUPP(filterUPP);
	DisposeNavEventUPP(eventUPP);

	CFRelease(dialogOptions.windowTitle);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		CFRelease(nav.customWindow);
		DisposeNibReference(nav.customNib);
	}
#endif

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			*ref = nav.ref;
			return (true);
		}
		else
			return (false);
	}
}

bool8 NavBeginOpenROMImageSheet (WindowRef parent, CFStringRef mes)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		HIViewID	cid = { 'PANE', 1000 };
		Rect		rct;

		err = CreateNibReference(kMacS9XCFString, &(gSheetNav.customNib));
		if (err)
			return (false);

		err = CreateWindowFromNib(gSheetNav.customNib, CFSTR("OpenROM"), &(gSheetNav.customWindow));
		if (err)
		{
			DisposeNibReference(gSheetNav.customNib);
			return (false);
		}

		GetWindowBounds(gSheetNav.customWindow, kWindowContentRgn, &rct);
		gSheetNav.customWidth  = rct.right  - rct.left;
		gSheetNav.customHeight = rct.bottom - rct.top;

		HIViewFindByID(HIViewGetRoot(gSheetNav.customWindow), cid, &(gSheetNav.customPane));
	}
#endif

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.preferenceKey = 3;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = CFCopyLocalizedString(CFSTR("OpenROMMes"), "Open");
	dialogOptions.message = mes;
	dialogOptions.modality = kWindowModalityWindowModal;
	dialogOptions.parentWindow = parent;
	gSheetNav.parent = parent;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	gSheetEventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavOpenCartEventHandler : NavGenericOpenEventHandler);
#else
	gSheetEventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
#endif
	gSheetFilterUPP = NewNavObjectFilterUPP(NavOpenCartFilter);

	err = NavCreateChooseFileDialog(&dialogOptions, NULL, gSheetEventUPP, NULL, gSheetFilterUPP, &gSheetNav, &(gSheetNav.nref));
	if (err == noErr)
	{
		if (systemVersion >= 0x1060)
		{
			NSView		*view;
			NSTextField	*txtM, *txtI, *txtV, *txtH;
			NSArray		*aryM, *aryI, *aryV, *aryH;

			gPool = [[NSAutoreleasePool alloc] init];

			view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 52)] autorelease];

			aryM = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM6", @""), NSLocalizedString(@"OpenROM7", @""), nil];
			aryI = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMa", @""), NSLocalizedString(@"OpenROMb", @""), NSLocalizedString(@"OpenROMc", @""), NSLocalizedString(@"OpenROMd", @""), nil];
			aryV = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM8", @""), NSLocalizedString(@"OpenROM9", @""), nil];
			aryH = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMe", @""), NSLocalizedString(@"OpenROMf", @""), nil];

			CocoaAddStatTextToView(view, @"OpenROM1",   5, 32,  84, 14, &txtM);
			CocoaAddStatTextToView(view, @"OpenROM3", 214, 32,  96, 14, &txtI);
			CocoaAddStatTextToView(view, @"OpenROM2",   5,  7,  84, 14, &txtV);
			CocoaAddStatTextToView(view, @"OpenROM4", 214,  7,  96, 14, &txtH);
			CocoaAddPopUpBtnToView(view, aryM,         91, 27, 116, 22, &gPopM);
			CocoaAddPopUpBtnToView(view, aryI,        312, 27, 116, 22, &gPopI);
			CocoaAddPopUpBtnToView(view, aryV,         91,  2, 116, 22, &gPopV);
			CocoaAddPopUpBtnToView(view, aryH,        312,  2, 116, 22, &gPopH);

			[txtM setAlignment: NSRightTextAlignment];
			[txtI setAlignment: NSRightTextAlignment];
			[txtV setAlignment: NSRightTextAlignment];
			[txtH setAlignment: NSRightTextAlignment];

			[gPopM selectItemAtIndex: romDetect       ];
			[gPopI selectItemAtIndex: interleaveDetect];
			[gPopV selectItemAtIndex: videoDetect     ];
			[gPopH selectItemAtIndex: headerDetect    ];

			[(NSOpenPanel *) gSheetNav.nref setAccessoryView: view];

			err = NavDialogRun(gSheetNav.nref);
			if (err)
			{
				[gPool release];
				NavDialogDispose(gSheetNav.nref);
				DisposeNavObjectFilterUPP(gSheetFilterUPP);
				DisposeNavEventUPP(gSheetEventUPP);
				CFRelease(dialogOptions.windowTitle);
				return (false);
			}
		}
	#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
		else
		{
			err = NavDialogRun(gSheetNav.nref);
			if (err)
			{
				NavDialogDispose(gSheetNav.nref);
				DisposeNavObjectFilterUPP(gSheetFilterUPP);
				DisposeNavEventUPP(gSheetEventUPP);
				CFRelease(dialogOptions.windowTitle);
				CFRelease(gSheetNav.customWindow);
				DisposeNibReference(gSheetNav.customNib);
				return (false);
			}
		}
	#endif
	}
	else
	{
		DisposeNavObjectFilterUPP(gSheetFilterUPP);
		DisposeNavEventUPP(gSheetEventUPP);
		CFRelease(dialogOptions.windowTitle);
	#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
		if (systemVersion < 0x1060)
		{
			CFRelease(gSheetNav.customWindow);
			DisposeNibReference(gSheetNav.customNib);
		}
	#endif
		return (false);
	}

	CFRelease(dialogOptions.windowTitle);
	return (true);
}

bool8 NavEndOpenROMImageSheet (FSRef *ref)
{
	if (systemVersion >= 0x1060)
	{
		romDetect        = [gPopM indexOfSelectedItem];
		interleaveDetect = [gPopI indexOfSelectedItem];
		videoDetect      = [gPopV indexOfSelectedItem];
		headerDetect     = [gPopH indexOfSelectedItem];

		[gPool release];
	}

	DisposeNavObjectFilterUPP(gSheetFilterUPP);
	DisposeNavEventUPP(gSheetEventUPP);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		CFRelease(gSheetNav.customWindow);
		DisposeNibReference(gSheetNav.customNib);
	}
#endif

	if (gSheetNav.reply)
	{
		*ref = gSheetNav.ref;
		return (true);
	}
	else
		return (false);
}

bool8 NavFreezeTo (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef, saveRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
	numRef  = CFCopyLocalizedString(CFSTR("FreezeToPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("FreezeToMes"), "Freeze");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
	CFStringAppendCString(saveRef, ".frz", CFStringGetSystemEncoding());

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.preferenceKey = 4;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.saveFileName = saveRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;
	nav.parent = NULL;

	eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);

	err = NavCreatePutFileDialog(&dialogOptions, 'SAVE', '~9X~', eventUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
		if (err)
			NavDialogDispose(nav.nref);
	}

	DisposeNavEventUPP(eventUPP);

	CFRelease(saveRef);
	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			char	s[PATH_MAX + 1];

			err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
			snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);

			return (true);
		}
		else
			return (false);
	}
}

bool8 NavDefrostFrom (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavObjectFilterUPP			filterUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
#ifdef MAC_TIGER_PANTHER_SUPPORT
	NavPreviewUPP				previewUPP;
#endif

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
	numRef  = CFCopyLocalizedString(CFSTR("DefrostFromPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("DefrostFromMes"), "Defrost");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.preferenceKey = 5;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;
	nav.parent = NULL;

	eventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
	filterUPP = NewNavObjectFilterUPP(NavDefrostFromFilter);
#ifdef MAC_TIGER_PANTHER_SUPPORT
	previewUPP = NewNavPreviewUPP(NavDefrostFromPreview);
#endif

#ifdef MAC_TIGER_PANTHER_SUPPORT
	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, previewUPP, filterUPP, &nav, &(nav.nref));
#else
	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, NULL, filterUPP, &nav, &(nav.nref));
#endif
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
		if (err)
			NavDialogDispose(nav.nref);
	}

#ifdef MAC_TIGER_PANTHER_SUPPORT
	DisposeNavPreviewUPP(previewUPP);
#endif
	DisposeNavObjectFilterUPP(filterUPP);
	DisposeNavEventUPP(eventUPP);

	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			err = FSRefMakePath(&(nav.ref), (unsigned char *) path, PATH_MAX);
			return (true);
		}
		else
			return (false);
	}
}

bool8 NavRecordMovieTo (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef, saveRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		HIViewID	cid = { 'PANE', 1000 };
		Rect		rct;

		err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
		if (err)
			return (false);

		err = CreateWindowFromNib(nav.customNib, CFSTR("RecordSMV"), &(nav.customWindow));
		if (err)
		{
			DisposeNibReference(nav.customNib);
			return (false);
		}

		GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
		nav.customWidth  = rct.right  - rct.left;
		nav.customHeight = rct.bottom - rct.top;

		HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
	}
#endif

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
	numRef  = CFCopyLocalizedString(CFSTR("RecordToPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("RecordToMes"), "Record");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
	CFStringAppendCString(saveRef, ".smv", CFStringGetSystemEncoding());

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.preferenceKey = 6;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.saveFileName = saveRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;
	nav.parent = NULL;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavRecordMovieToEventHandler : NavGenericSaveEventHandler);
#else
	eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);
#endif

	err = NavCreatePutFileDialog(&dialogOptions, 'SMOV', '~9X~', eventUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		if (systemVersion >= 0x1060)
		{
			NSAutoreleasePool	*pool;
			NSView				*view;
			NSTextField			*text;
			NSButton			*cb1p, *cb2p, *cb3p, *cb4p, *cb5p, *cbre;
			NSString			*comment;

			pool = [[NSAutoreleasePool alloc] init];

			view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 54)] autorelease];

			CocoaAddStatTextToView(view, @"RecordSMV6",   5, 33, 104, 14, NULL);
			CocoaAddStatTextToView(view, @"RecordSMV7",   5,  8,  63, 14, NULL);
			CocoaAddEditTextToView(view, @"",            74,  6, 351, 19, &text);
			CocoaAddCheckBoxToView(view, @"RecordSMV1", 112, 31,  29, 18, &cb1p);
			CocoaAddCheckBoxToView(view, @"RecordSMV2", 150, 31,  29, 18, &cb2p);
			CocoaAddCheckBoxToView(view, @"RecordSMV3", 188, 31,  29, 18, &cb3p);
			CocoaAddCheckBoxToView(view, @"RecordSMV4", 226, 31,  29, 18, &cb4p);
			CocoaAddCheckBoxToView(view, @"RecordSMV5", 264, 31,  29, 18, &cb5p);
			CocoaAddCheckBoxToView(view, @"RecordSMV8", 356, 31,  51, 18, &cbre);

			[cb1p setState: (macRecordFlag & (1 << 0) ? NSOnState : NSOffState)];
			[cb2p setState: (macRecordFlag & (1 << 1) ? NSOnState : NSOffState)];
			[cb3p setState: (macRecordFlag & (1 << 2) ? NSOnState : NSOffState)];
			[cb4p setState: (macRecordFlag & (1 << 3) ? NSOnState : NSOffState)];
			[cb5p setState: (macRecordFlag & (1 << 4) ? NSOnState : NSOffState)];
			[cbre setState: (macRecordFlag & (1 << 5) ? NSOnState : NSOffState)];

			[(NSOpenPanel *) nav.nref setAccessoryView: view];

			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);

			macRecordFlag  = 0;
			macRecordFlag |= ((([cb1p state] == NSOnState) ? 1 : 0) << 0);
			macRecordFlag |= ((([cb2p state] == NSOnState) ? 1 : 0) << 1);
			macRecordFlag |= ((([cb3p state] == NSOnState) ? 1 : 0) << 2);
			macRecordFlag |= ((([cb4p state] == NSOnState) ? 1 : 0) << 3);
			macRecordFlag |= ((([cb5p state] == NSOnState) ? 1 : 0) << 4);
			macRecordFlag |= ((([cbre state] == NSOnState) ? 1 : 0) << 5);

			comment = [text stringValue];
			if (comment)
			{
				NSUInteger	cflen;
				unichar		unistr[MOVIE_MAX_METADATA];

				cflen = [comment length];
				if (cflen > MOVIE_MAX_METADATA - 1)
					cflen = MOVIE_MAX_METADATA - 1;

				[comment getCharacters: unistr range: NSMakeRange(0, cflen)];

				for (unsigned i = 0; i < cflen; i++)
					macRecordWChar[i] = (wchar_t) unistr[i];
				macRecordWChar[cflen] = 0;
			}
			else
				macRecordWChar[0] = 0;

			[pool release];
		}
	#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
		else
		{
			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);
		}
	#endif
	}

	DisposeNavEventUPP(eventUPP);

	CFRelease(saveRef);
	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		CFRelease(nav.customWindow);
		DisposeNibReference(nav.customNib);
	}
#endif

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			char	s[PATH_MAX + 1];

			err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
			snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);

			return (true);
		}
		else
			return (false);
	}
}

bool8 NavPlayMovieFrom (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavObjectFilterUPP			filterUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
#ifdef MAC_TIGER_PANTHER_SUPPORT
	NavPreviewUPP				previewUPP;
#endif

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		HIViewID	cid = { 'PANE', 1000 };
		Rect		rct;

		err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
		if (err)
			return (false);

		err = CreateWindowFromNib(nav.customNib, CFSTR("PlaySMV"), &(nav.customWindow));
		if (err)
		{
			DisposeNibReference(nav.customNib);
			return (false);
		}

		GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
		nav.customWidth  = rct.right  - rct.left;
		nav.customHeight = rct.bottom - rct.top;

		HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
	}
#endif

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
	numRef  = CFCopyLocalizedString(CFSTR("PlayFromPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("PlayFromMes"), "Play");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.preferenceKey = 6;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;
	nav.parent = NULL;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavPlayMovieFromEventHandler : NavGenericOpenEventHandler);
#else
	eventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
#endif
	filterUPP = NewNavObjectFilterUPP(NavPlayMovieFromFilter);
#ifdef MAC_TIGER_PANTHER_SUPPORT
	previewUPP = NewNavPreviewUPP(NavPlayMovieFromPreview);
#endif

#ifdef MAC_TIGER_PANTHER_SUPPORT
	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, previewUPP, filterUPP, &nav, &(nav.nref));
#else
	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, NULL, filterUPP, &nav, &(nav.nref));
#endif
	if (err == noErr)
	{
		if (systemVersion >= 0x1060)
		{
			NSAutoreleasePool	*pool;
			NSView				*view;
			NSButton			*cbro, *cbqt;

			pool = [[NSAutoreleasePool alloc] init];

			view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 326, 29)] autorelease];

			CocoaAddCheckBoxToView(view, @"PlaySMV1",   5, 6,  76, 18, &cbro);
			CocoaAddCheckBoxToView(view, @"PlaySMV2", 100, 6, 220, 18, &cbqt);

			[cbro setState: (macPlayFlag & (1 << 0) ? NSOnState : NSOffState)];
			[cbqt setState: (macPlayFlag & (1 << 1) ? NSOnState : NSOffState)];

			[(NSOpenPanel *) nav.nref setAccessoryView: view];

			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);

			macPlayFlag  = 0;
			macPlayFlag |= ((([cbro state] == NSOnState) ? 1 : 0) << 0);
			macPlayFlag |= ((([cbqt state] == NSOnState) ? 1 : 0) << 1);

			[pool release];
		}
	#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
		else
		{
			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);
		}
	#endif
	}

#ifdef MAC_TIGER_PANTHER_SUPPORT
	DisposeNavPreviewUPP(previewUPP);
#endif
	DisposeNavObjectFilterUPP(filterUPP);
	DisposeNavEventUPP(eventUPP);

	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		CFRelease(nav.customWindow);
		DisposeNibReference(nav.customNib);
	}
#endif

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			err = FSRefMakePath(&(nav.ref), (unsigned char *) path, PATH_MAX);
			return (true);
		}
		else
			return (false);
	}
}

bool8 NavQTMovieRecordTo (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef, saveRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		HIViewID	cid = { 'PANE', 1000 };
		Rect		rct;

		err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
		if (err)
			return (false);

		err = CreateWindowFromNib(nav.customNib, CFSTR("QTMovie"), &(nav.customWindow));
		if (err)
		{
			DisposeNibReference(nav.customNib);
			return (false);
		}

		GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
		nav.customWidth  = rct.right  - rct.left;
		nav.customHeight = rct.bottom - rct.top;

		HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
	}
#endif

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
	numRef  = CFCopyLocalizedString(CFSTR("QTRecordPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("QTRecordMes"), "QT");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
	CFStringAppendCString(saveRef, ".mov", CFStringGetSystemEncoding());

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.preferenceKey = 7;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.saveFileName = saveRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;
	nav.parent = NULL;

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavQTMovieRecordToEventHandler : NavGenericSaveEventHandler);
#else
	eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);
#endif

	err = NavCreatePutFileDialog(&dialogOptions, 'MooV', 'TVOD', eventUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		if (systemVersion >= 0x1060)
		{
			NSAutoreleasePool	*pool;
			NSView				*view;
			NSPopUpButton		*fpop;
			NSButton			*cbds, *cbov, *comp;
			NSArray				*array;
			NavController		*controller;

			pool = [[NSAutoreleasePool alloc] init];

			controller = [[[NavController alloc] init] autorelease];
			view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 31)] autorelease];

			array = [NSArray arrayWithObjects: @"0", @"1", @"2", @"3", @"4", @"5", nil];
			CocoaAddStatTextToView(view, @"QTRecord3", 185, 9,  70, 14, NULL);
			CocoaAddCheckBoxToView(view, @"QTRecord1",   5, 7,  85, 18, &cbds);
			CocoaAddCheckBoxToView(view, @"QTRecord2",  97, 7,  71, 18, &cbov);
			CocoaAddMPushBtnToView(view, @"QTRecord4", 313, 1, 117, 28, &comp);
			CocoaAddPopUpBtnToView(view, array,        255, 4,  50, 22, &fpop);

			[cbds setState: (macQTMovFlag & (1 << 0) ? NSOnState : NSOffState)];
			[cbov setState: (macQTMovFlag & (1 << 1) ? NSOnState : NSOffState)];
			[fpop selectItemAtIndex: (macQTMovFlag >> 8)];

			[comp setTarget: controller];
			[comp setAction: @selector(handleCompressButton:)];

			[(NSOpenPanel *) nav.nref setAccessoryView: view];

			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);

			macQTMovFlag  = 0;
			macQTMovFlag |= ((([cbds state] == NSOnState) ? 1 : 0) << 0);
			macQTMovFlag |= ((([cbov state] == NSOnState) ? 1 : 0) << 1);
			macQTMovFlag |= ([fpop indexOfSelectedItem] << 8);

			[pool release];
		}
	#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
		else
		{
			err = NavDialogRun(nav.nref);
			if (err)
				NavDialogDispose(nav.nref);
		}
	#endif
	}

	DisposeNavEventUPP(eventUPP);

	CFRelease(saveRef);
	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	if (systemVersion < 0x1060)
	{
		CFRelease(nav.customWindow);
		DisposeNibReference(nav.customNib);
	}
#endif

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			char	s[PATH_MAX + 1];

			err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
			snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);

			return (true);
		}
		else
			return (false);
	}
}

bool8 NavBeginChooseFolderSheet (WindowRef parent)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	dialogOptions.preferenceKey = 8;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.modality = kWindowModalityWindowModal;
	dialogOptions.parentWindow = parent;
	gSheetNav.parent = parent;

	gSheetEventUPP = NewNavEventUPP(NavGenericOpenEventHandler);

	err = NavCreateChooseFolderDialog(&dialogOptions, gSheetEventUPP, NULL, &gSheetNav, &(gSheetNav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(gSheetNav.nref);
		if (err)
		{
			NavDialogDispose(gSheetNav.nref);
			DisposeNavEventUPP(gSheetEventUPP);
			return (false);
		}
	}
	else
	{
		DisposeNavEventUPP(gSheetEventUPP);
		return (false);
	}

	return (true);
}

bool8 NavEndChooseFolderSheet (FSRef *ref)
{
	DisposeNavEventUPP(gSheetEventUPP);

	if (gSheetNav.reply)
	{
		*ref = gSheetNav.ref;
		return (true);
	}
	else
		return (false);
}

static pascal void NavGenericOpenEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	AEDesc			resultDesc;
	SInt32 			count;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			break;

		case kNavCBStart:
			nav->reply = false;
			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionChoose:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AECountItems(&(reply.selection), &count);
						if ((err == noErr) && (count == 1))
						{
							err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
							if (err == noErr)
							{
								err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
								if (err == noErr)
									nav->reply = true;

								err = AEDisposeDesc(&resultDesc);
							}
						}

						err = NavDisposeReply(&reply);
					}

					break;
			}

			break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);

			if (nav->parent)
			{
				EventRef	event;
				HICommand	cmd;

				cmd.commandID          = 'NvDn';
				cmd.attributes         = kEventAttributeUserEvent;
				cmd.menu.menuRef       = NULL;
				cmd.menu.menuItemIndex = 0;

				err = CreateEvent(kCFAllocatorDefault, kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &event);
				if (err == noErr)
				{
					err = SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(HICommand), &cmd);
					if (err == noErr)
						err = SendEventToWindow(event, nav->parent);

					ReleaseEvent(event);
				}
			}

			break;
	}
}

static pascal void NavGenericSaveEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	AEDesc			resultDesc;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			break;

		case kNavCBStart:
			nav->reply = false;
			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionSaveAs:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
						if (err == noErr)
						{
							err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
							if (err == noErr)
							{
								Boolean	r;

								r = CFStringGetCString(reply.saveFileName, nav->name, PATH_MAX, kCFStringEncodingUTF8);
								if (r)
									nav->reply = true;
							}

							err = AEDisposeDesc(&resultDesc);
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
static pascal void NavOpenCartEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	AEDesc			resultDesc;
	Point			pt;
	SInt32 			count;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'CCTL')
							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'CCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			cid.id = 101;
			HIViewFindByID(nav->customPane, cid, &ctl);
			SetControl32BitValue(ctl, romDetect        + 1);

			cid.id = 102;
			HIViewFindByID(nav->customPane, cid, &ctl);
			SetControl32BitValue(ctl, interleaveDetect + 1);

			cid.id = 103;
			HIViewFindByID(nav->customPane, cid, &ctl);
			SetControl32BitValue(ctl, videoDetect      + 1);

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			SetControl32BitValue(ctl, headerDetect     + 1);

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			cid.signature = 'CCTL';

			cid.id = 101;
			HIViewFindByID(nav->customPane, cid, &ctl);
			romDetect        = GetControl32BitValue(ctl) - 1;

			cid.id = 102;
			HIViewFindByID(nav->customPane, cid, &ctl);
			interleaveDetect = GetControl32BitValue(ctl) - 1;

			cid.id = 103;
			HIViewFindByID(nav->customPane, cid, &ctl);
			videoDetect      = GetControl32BitValue(ctl) - 1;

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			headerDetect     = GetControl32BitValue(ctl) - 1;

			break;

		case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionChoose:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AECountItems(&(reply.selection), &count);
						if ((err == noErr) && (count == 1))
						{
							err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
							if (err == noErr)
							{
								err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
								if (err == noErr)
									nav->reply = true;

								err = AEDisposeDesc(&resultDesc);
							}
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);

			if (nav->parent)
			{
				EventRef	event;
				HICommand	cmd;

				cmd.commandID          = 'NvDn';
				cmd.attributes         = kEventAttributeUserEvent;
				cmd.menu.menuRef       = NULL;
				cmd.menu.menuItemIndex = 0;

				err = CreateEvent(kCFAllocatorDefault, kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &event);
				if (err == noErr)
				{
					err = SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(HICommand), &cmd);
					if (err == noErr)
						err = SendEventToWindow(event, nav->parent);

					ReleaseEvent(event);
				}
			}

			break;
	}
}

static pascal void NavPlayMovieFromEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef 		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	AEDesc			resultDesc;
	Point			pt;
	SInt32 			count;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'PCTL')
							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'PCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				SetControl32BitValue(ctl, (macPlayFlag & (1 << (cid.id - 101))) ? true : false);
			}

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			macPlayFlag = 0;
			cid.signature = 'PCTL';

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				if (GetControl32BitValue(ctl))
					macPlayFlag |= (1 << (cid.id - 101));
			}

			break;

		case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionChoose:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AECountItems(&(reply.selection), &count);
						if ((err == noErr) && (count == 1))
						{
							err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
							if (err == noErr)
							{
								err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
								if (err == noErr)
									nav->reply = true;

								err = AEDisposeDesc(&resultDesc);
							}
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

static pascal void NavRecordMovieToEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef 		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	CFStringRef		sref;
	CFIndex			cflen;
	AEDesc			resultDesc;
	Point			pt;
	UniChar			unistr[MOVIE_MAX_METADATA];

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'RCTL')
						{
							if (cid.id == 107)
							{
								HIViewRef	tmp;

								err = GetKeyboardFocus(callBackParms->window, &tmp);
								if ((err == noErr) && (tmp != ctl))
									err = SetKeyboardFocus(callBackParms->window, ctl, kControlFocusNextPart);
							}

							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);
						}
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'RCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			for (cid.id = 101; cid.id <= 106; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				SetControl32BitValue(ctl, (macRecordFlag & (1 << (cid.id - 101))) ? true : false);
			}

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;
			break;

		case kNavCBAccept:
			macRecordFlag = 0;
			cid.signature = 'RCTL';

			for (cid.id = 101; cid.id <= 106; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				if (GetControl32BitValue(ctl))
					macRecordFlag |= (1 << (cid.id - 101));
			}

			cid.id = 107;
			HIViewFindByID(nav->customPane, cid, &ctl);
			CopyEditTextCFString(ctl, &sref);
			if (sref)
			{
				cflen = CFStringGetLength(sref);
				if (cflen > MOVIE_MAX_METADATA - 1)
					cflen = MOVIE_MAX_METADATA - 1;

				CFStringGetCharacters(sref, CFRangeMake(0, cflen), unistr);

				for (int i = 0; i < cflen; i++)
					macRecordWChar[i] = (wchar_t) unistr[i];
				macRecordWChar[cflen] = 0;

				CFRelease(sref);
			}
			else
				macRecordWChar[0] = 0;

			break;

		case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionSaveAs:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
						if (err == noErr)
						{
							err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
							if (err == noErr)
							{
								Boolean	r;

								r = CFStringGetCString(reply.saveFileName, nav->name, PATH_MAX, kCFStringEncodingUTF8);
								if (r)
									nav->reply = true;
							}

							err = AEDisposeDesc(&resultDesc);
						}

						err = NavDisposeReply(&reply);
					}

					break;
			}

			break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

static pascal void NavQTMovieRecordToEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef 		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	AEDesc			resultDesc;
	Point			pt;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'QCTL')
							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);

						if (cid.id == 103)
							MacQTVideoConfig();
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'QCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				SetControl32BitValue(ctl, (macQTMovFlag & (1 << (cid.id - 101))) ? true : false);
			}

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			SetControl32BitValue(ctl, ((macQTMovFlag & 0xFF00) >> 8) + 1);

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			macQTMovFlag = 0;
			cid.signature = 'QCTL';

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				if (GetControl32BitValue(ctl))
					macQTMovFlag |= (1 << (cid.id - 101));
			}

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			macQTMovFlag |= ((GetControl32BitValue(ctl) - 1) << 8);

			break;

		case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionSaveAs:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
						if (err == noErr)
						{
							err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
							if (err == noErr)
							{
								Boolean	r;

								r = CFStringGetCString(reply.saveFileName, nav->name, PATH_MAX, kCFStringEncodingUTF8);
								if (r)
									nav->reply = true;
							}

							err = AEDisposeDesc(&resultDesc);
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}
#endif

static pascal Boolean NavOpenCartFilter (AEDesc *theItem, void *ninfo, NavCallBackUserData callbackUD, NavFilterModes filterMode)
{
	OSStatus	err;
	AEDesc 		resultDesc;
	Boolean		result = true;

	err = AECoerceDesc(theItem, typeFSRef, &resultDesc);
	if (err == noErr)
	{
		FSRef	ref;

		err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
		if (err == noErr)
		{
			FSCatalogInfo	catinfo;
			HFSUniStr255	unistr;

			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags, &catinfo, &unistr, NULL, NULL);
			if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask) && (unistr.length > 4))
			{
				UInt16	i = unistr.length;

				if  ((unistr.unicode[i - 4] == '.') &&
				   (((unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'r') && (unistr.unicode[i - 1] == 'm')) ||
					((unistr.unicode[i - 3] == 'f') && (unistr.unicode[i - 2] == 'r') && (unistr.unicode[i - 1] == 'z')) ||
					((unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'm') && (unistr.unicode[i - 1] == 'v')) ||
					((unistr.unicode[i - 3] == 'm') && (unistr.unicode[i - 2] == 'o') && (unistr.unicode[i - 1] == 'v')) ||
					((unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'p') && (unistr.unicode[i - 1] == 'c')) ||
					((unistr.unicode[i - 3] == 'r') && (unistr.unicode[i - 2] == 't') && (unistr.unicode[i - 1] == 'c')) ||
					((unistr.unicode[i - 3] == 'd') && (unistr.unicode[i - 2] == 'a') && (unistr.unicode[i - 1] == 't')) ||
					((unistr.unicode[i - 3] == 'o') && (unistr.unicode[i - 2] == 'u') && (unistr.unicode[i - 1] == 't')) ||
					((unistr.unicode[i - 3] == 'c') && (unistr.unicode[i - 2] == 'h') && (unistr.unicode[i - 1] == 't')) ||
					((unistr.unicode[i - 3] == 'i') && (unistr.unicode[i - 2] == 'p') && (unistr.unicode[i - 1] == 's')) ||
					((unistr.unicode[i - 3] == 'u') && (unistr.unicode[i - 2] == 'p') && (unistr.unicode[i - 1] == 's')) ||
					((unistr.unicode[i - 3] == 'p') && (unistr.unicode[i - 2] == 'n') && (unistr.unicode[i - 1] == 'g'))))
					result = false;
			}
		}

		AEDisposeDesc(&resultDesc);
	}

	return (result);
}

static pascal Boolean NavDefrostFromFilter (AEDesc *theItem, void *ninfo, NavCallBackUserData callbackUD, NavFilterModes filterMode)
{
	OSStatus	err;
	AEDesc 		resultDesc;
	Boolean		result = true;

	err = AECoerceDesc(theItem, typeFSRef, &resultDesc);
	if (err == noErr)
	{
		FSRef	ref;

		err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
		if (err == noErr)
		{
			FSCatalogInfo	catinfo;
			HFSUniStr255	unistr;

			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &catinfo, &unistr, NULL, NULL);
			if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask))
			{
				if (((FileInfo *) &catinfo.finderInfo)->fileType != 'SAVE')
				{
					UInt16	i = unistr.length;

					if (i < 4)
						result = false;
					else
					if ((unistr.unicode[i - 4] == '.') && (unistr.unicode[i - 3] == 'f') && (unistr.unicode[i - 2] == 'r'))
						result = true;
					else
						result = false;
				}
			}
		}

		AEDisposeDesc(&resultDesc);
	}

	return (result);
}

static pascal Boolean NavPlayMovieFromFilter (AEDesc *theItem, void *ninfo, NavCallBackUserData callbackUD, NavFilterModes filterMode)
{
	OSStatus	err;
	AEDesc 		resultDesc;
	Boolean		result = true;

	err = AECoerceDesc(theItem, typeFSRef, &resultDesc);
	if (err == noErr)
	{
		FSRef	ref;

		err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
		if (err == noErr)
		{
			FSCatalogInfo	catinfo;
			HFSUniStr255	unistr;

			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &catinfo, &unistr, NULL, NULL);
			if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask))
			{
				if (((FileInfo *) &catinfo.finderInfo)->fileType != 'SMOV')
				{
					UInt16	i = unistr.length;

					if (i < 4)
						result = false;
					else
					if  ((unistr.unicode[i - 4] == '.') && (unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'm') && (unistr.unicode[i - 1] == 'v'))
						result = true;
					else
						result = false;
				}
			}
		}

		AEDisposeDesc(&resultDesc);
	}

	return (result);
}

#ifdef MAC_TIGER_PANTHER_SUPPORT
static pascal Boolean NavDefrostFromPreview (NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	if (systemVersion >= 0x1050)
		return (true);

	OSStatus	err;
	Boolean		previewShowing, result = false;

	err = NavCustomControl(callBackParms->context, kNavCtlIsPreviewShowing, &previewShowing);
	if ((err == noErr) && previewShowing)
	{
		AEDesc	resultDesc;

		err = AECoerceDesc((AEDesc *) callBackParms->eventData.eventDataParms.param, typeFSRef, &resultDesc);
		if (err == noErr)
		{
			FSRef	ref;

			err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
			if (err == noErr)
			{
				FSCatalogInfo	catinfo;

				err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags | kFSCatInfoContentMod, &catinfo, NULL, NULL, NULL);
				if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask))
				{
					CFMutableStringRef	sref;

					sref = CFStringCreateMutable(kCFAllocatorDefault, 0);
					if (sref)
					{
						CFAbsoluteTime		at;
						CFDateFormatterRef	format;
						CFLocaleRef			locale;
						CFStringRef			datstr;

						err = UCConvertUTCDateTimeToCFAbsoluteTime(&(catinfo.contentModDate), &at);
						locale = CFLocaleCopyCurrent();
						format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterMediumStyle);
						datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
						CFStringAppend(sref, datstr);
						CFRelease(datstr);
						CFRelease(format);
						CFRelease(locale);

						CGContextRef	ctx;
						CGRect			bounds;
						CGrafPtr		port;
						Rect			rct;
						int				rightedge;

						port = GetWindowPort(callBackParms->window);
						GetWindowBounds(callBackParms->window, kWindowContentRgn, &rct);

						err = QDBeginCGContext(port, &ctx);

						rightedge = (callBackParms->previewRect.right < (rct.right - rct.left - 11)) ? callBackParms->previewRect.right : (rct.right - rct.left - 11);

						bounds.origin.x = (float) callBackParms->previewRect.left;
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.bottom);
						bounds.size.width  = (float) (rightedge - callBackParms->previewRect.left);
						bounds.size.height = (float) (callBackParms->previewRect.bottom - callBackParms->previewRect.top);

						CGContextClipToRect(ctx, bounds);

						CGContextSetRGBFillColor(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
						CGContextFillRect(ctx, bounds);
						CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);

						bounds.origin.x = (float) (callBackParms->previewRect.left + ((callBackParms->previewRect.right - callBackParms->previewRect.left - 128) >> 1));
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 120 - 25);
						bounds.size.width  = 128.0f;
						bounds.size.height = 120.0f;

						DrawThumbnailResource(&ref, ctx, bounds);

						HIThemeTextInfo	textinfo;

						textinfo.version             = 0;
						textinfo.state               = kThemeStateActive;
						textinfo.fontID              = kThemeSmallSystemFont;
						textinfo.verticalFlushness   = kHIThemeTextVerticalFlushTop;
						textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
						textinfo.options             = 0;
						textinfo.truncationPosition  = kHIThemeTextTruncationMiddle;
						textinfo.truncationMaxLines  = 0;
						textinfo.truncationHappened  = false;

						bounds.origin.x = (float) (callBackParms->previewRect.left + 10);
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 153 - 20);
						bounds.size.width  = (float) (callBackParms->previewRect.right - callBackParms->previewRect.left - 20);
						bounds.size.height = 20.0f;

						err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);

						CGContextSynchronize(ctx);

						err = QDEndCGContext(port, &ctx);

						CFRelease(sref);

						result = true;
					}
				}
			}

			AEDisposeDesc(&resultDesc);
		}
	}

	return (result);
}

static pascal Boolean NavPlayMovieFromPreview (NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	if (systemVersion >= 0x1050)
		return (true);

	OSStatus	err;
	Boolean		previewShowing, result = false;

	err = NavCustomControl(callBackParms->context, kNavCtlIsPreviewShowing, &previewShowing);
	if ((err == noErr) && previewShowing)
	{
		AEDesc	resultDesc;

		err = AECoerceDesc((AEDesc *) callBackParms->eventData.eventDataParms.param, typeFSRef, &resultDesc);
		if (err == noErr)
		{
			FSRef	ref;

			err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
			if (err == noErr)
			{
				char	path[PATH_MAX + 1];

				err = FSRefMakePath(&ref, (unsigned char *) path, PATH_MAX);
				if (err == noErr)
				{
					MovieInfo	movinfo;
					int			r;

					r = S9xMovieGetInfo(path, &movinfo);
					if (r == SUCCESS)
					{
						UTCDateTime		utctime;
						CGContextRef	ctx;
						CGRect			bounds;
						CGrafPtr		port;
						Rect			rct;
						CFStringRef		sref;
						UInt64			t;
						int				rightedge, border, width, l, sec, min, hr, n;
						char			cstr[256], cbuf[512];
						UniChar			unistr[MOVIE_MAX_METADATA];

						sref = CFCopyLocalizedString(CFSTR("MoviePrevBorder"), "1");
						if (sref)
						{
							border = CFStringGetIntValue(sref);
							CFRelease(sref);
						}
						else
							border = 1;

						sref = CFCopyLocalizedString(CFSTR("MoviePrevWidth"), "1");
						if (sref)
						{
							width = CFStringGetIntValue(sref);
							CFRelease(sref);
						}
						else
							width = 1;

						// Date

						t = (UInt64) movinfo.TimeCreated + ((365 * 66 + 17) * 24 * 60 * 60);
						utctime.highSeconds = (UInt16) (t >> 32);
						utctime.lowSeconds  = (UInt32) (t & 0xFFFFFFFF);
						utctime.fraction    = 0;

						CFAbsoluteTime		at;
						CFDateFormatterRef	format;
						CFLocaleRef			locale;
						CFStringRef			datstr;
						Boolean				e;

						err = UCConvertUTCDateTimeToCFAbsoluteTime(&utctime, &at);
						locale = CFLocaleCopyCurrent();
						format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterNoStyle);
						datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
						e = CFStringGetCString(datstr, cbuf, sizeof(cbuf), CFStringGetSystemEncoding());
						CFRelease(datstr);
						CFRelease(format);
						strcat(cbuf, "\n");
						format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterMediumStyle);
						datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
						e = CFStringGetCString(datstr, cstr, sizeof(cstr), CFStringGetSystemEncoding());
						CFRelease(datstr);
						CFRelease(format);
						strcat(cbuf, cstr);
						strcat(cbuf, "\n");
						CFRelease(locale);

						// Length

						l = (movinfo.LengthFrames + 30) / ((movinfo.Opts & MOVIE_OPT_PAL) ? 50 : 60);
						sec = l % 60;	l /= 60;
						min = l % 60;	l /= 60;
						hr  = l % 60;

						sprintf(cstr, "%02d:%02d:%02d\n", hr, min, sec);
						strcat(cbuf, cstr);

						// Frames

						sprintf(cstr, "%d\n", movinfo.LengthFrames);
						strcat(cbuf, cstr);

						//

						port = GetWindowPort(callBackParms->window);
						GetWindowBounds(callBackParms->window, kWindowContentRgn, &rct);

						err = QDBeginCGContext(port, &ctx);

						rightedge = (callBackParms->previewRect.right < (rct.right - rct.left - 11)) ? callBackParms->previewRect.right : (rct.right - rct.left - 11);

						bounds.origin.x = (float) callBackParms->previewRect.left;
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.bottom);
						bounds.size.width  = (float) (rightedge - callBackParms->previewRect.left);
						bounds.size.height = (float) (callBackParms->previewRect.bottom - callBackParms->previewRect.top);

						CGContextClipToRect(ctx, bounds);

						CGContextSetRGBFillColor(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
						CGContextFillRect(ctx, bounds);
						CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);

						// Thumbnail

						bounds.origin.x  = (float) (callBackParms->previewRect.left + ((callBackParms->previewRect.right - callBackParms->previewRect.left - 128) >> 1));
						bounds.origin.y  = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 120 - 25);
						bounds.size.width  = 128.0f;
						bounds.size.height = 120.0f;

						DrawThumbnailResource(&ref, ctx, bounds);

						// Text

						HIThemeTextInfo	textinfo;

						textinfo.version            = 0;
						textinfo.state              = kThemeStateActive;
						textinfo.fontID             = kThemeSmallSystemFont;
						textinfo.verticalFlushness  = kHIThemeTextVerticalFlushTop;
						textinfo.options            = 0;
						textinfo.truncationPosition = kHIThemeTextTruncationMiddle;
						textinfo.truncationMaxLines = 0;
						textinfo.truncationHappened = false;

						bounds.origin.x = (float) (((callBackParms->previewRect.right - callBackParms->previewRect.left - width) >> 1) + callBackParms->previewRect.left + border + 7);
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 153 - 60);
						bounds.size.width  = (float) callBackParms->previewRect.right - bounds.origin.x;
						bounds.size.height = 60.0f;

						sref = CFStringCreateWithCString(kCFAllocatorDefault, cbuf, CFStringGetSystemEncoding());
						if (sref)
						{
							textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
							err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);
							CFRelease(sref);
						}

						bounds.origin.x = (float) callBackParms->previewRect.left;
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 153 - 60);
						bounds.size.width  = (float) (((callBackParms->previewRect.right - callBackParms->previewRect.left - width) >> 1) + border);
						bounds.size.height = 60.0f;

						sref = CFCopyLocalizedString(CFSTR("MoviePrevMes"), "MovieInfo");
						if (sref)
						{
							textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushRight;
							err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);
							CFRelease(sref);
						}

						bounds.origin.x = (float) (((callBackParms->previewRect.right - callBackParms->previewRect.left - 132) >> 1) + callBackParms->previewRect.left);
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.bottom + 10);
						bounds.size.width  = 132.0f;
						bounds.size.height = (float) (callBackParms->previewRect.bottom - callBackParms->previewRect.top - 223 - 10);

						n = wcslen(movinfo.Metadata);

						for (int i = 0; i < n; i++)
							unistr[i] = (UniChar) movinfo.Metadata[i];
						unistr[n] = 0;

						sref = CFStringCreateWithCharacters(kCFAllocatorDefault, unistr, n);
						if (sref)
						{
							textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
							err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);
							CFRelease(sref);
						}

						CGContextSynchronize(ctx);

						err = QDEndCGContext(port, &ctx);

						result = true;
					}
				}
			}

			AEDisposeDesc(&resultDesc);
		}
	}

	return (result);
}
#endif

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
static void GlobalPointToWindowLocalPoint (Point *pt, WindowRef window)
{
	if (systemVersion >= 0x1040)
	{
		HIViewRef	view;
		HIPoint		cpt = CGPointMake((float) pt->h, (float) pt->v);

		HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &view);
		HIPointConvert(&cpt, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, view);
		pt->h = (short) cpt.x;
		pt->v = (short) cpt.y;
	}
#ifdef MAC_PANTHER_SUPPORT
	else
		QDGlobalToLocalPoint(GetWindowPort(window), pt);
#endif
}
#endif
