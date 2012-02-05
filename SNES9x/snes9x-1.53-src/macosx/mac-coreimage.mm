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


#import "port.h"

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>

#import "mac-prefix.h"
#import "mac-dialog.h"
#import "mac-os.h"
#import "mac-coreimage.h"

enum
{
	kCITypeNone    = 0,
	kCITypeBoolean = 1000,
	kCITypeScalar,
	kCITypeColor
};

#define	mCoreImageFilter		501
#define	FIXEDRANGE				0x10000
#define	kCommandFilterMenuBase	0x41000000
#define	kCommandCheckBoxBase	0x49000000
#define	kCommandSliderBase		0x51000000
#define	kCommandColorButtonBase	0x59000000
#define	kCIFilterNamePrefKey	CFSTR("CoreImageFilterName")

typedef struct {
	char	name[256];
	char	displayName[256];
	int		type;
	union {
		struct {
			bool8	cur;
		}	b;
		
		struct {
			float	max, min, cur;
		}	s;
		
		struct {
			float	r, g, b, a;
		}	c;
	}	u;	
}	FilterParam;

static NSMutableArray	*ciFilterNameList          = NULL;
static NSMutableArray	*ciFilterLocalizedNameList = NULL;
static NSArray			*ciFilterInputKeys         = NULL;
static CIFilter			*ciFilter                  = NULL;
static CIContext		*ciContext                 = NULL;
static FilterParam		*ciFilterParam             = NULL;
static CFStringRef		ciFilterName               = NULL;
static HIViewRef		ciFilterUIPane             = NULL;
static MenuRef			ciFilterMenu               = NULL;
static CGColorSpaceRef	cgColor                    = NULL;
static MPSemaphoreID	cisem                      = NULL;
static bool8			ciFilterHasInputCenter     = false;
static bool8			ciFilterHasInputImage      = false;
static int				ciFilterInputKeysCount     = 0;

static void LoadFilterPrefs (void);
static void SaveFilterPrefs (void);
static void FilterParamToFilter (void);
static void FilterToFilterParam (void);
static void BuildCoreImageFilterListAndMenu (void);
static void ReleaseCoreImageFilterListAndMenu (void);
static void ReplaceFilterUI (WindowRef);
static void FilterUIAddSubviews (WindowRef, HIViewRef);
static void FilterUISetValues (HIViewRef);
static bool8 IsCoreImageFilterSupported (CIFilter *);
static pascal OSStatus CoreImageFilterEventHandler (EventHandlerCallRef, EventRef, void *);


void InitCoreImage (void)
{
	OSStatus			err;
	NSAutoreleasePool	*pool;

	pool = [[NSAutoreleasePool alloc] init];
	
	ciFilterName = (CFStringRef) CFPreferencesCopyAppValue(kCIFilterNamePrefKey, kCFPreferencesCurrentApplication);
	if (!ciFilterName)
		ciFilterName = CFStringCreateCopy(kCFAllocatorDefault, CFSTR("CIGammaAdjust"));
	
	BuildCoreImageFilterListAndMenu();
	
	err = MPCreateBinarySemaphore(&cisem);
	
	[pool release];
}

void DeinitCoreImage (void)
{
	OSStatus			err;
	NSAutoreleasePool	*pool;

	pool = [[NSAutoreleasePool alloc] init];
	
	err = MPDeleteSemaphore(cisem);
	
	ReleaseCoreImageFilterListAndMenu();
	
	CFPreferencesSetAppValue(kCIFilterNamePrefKey, ciFilterName, kCFPreferencesCurrentApplication);
	
	CFRelease(ciFilterName);
	
	[pool release];
}	

void InitCoreImageFilter (void)
{
	NSAutoreleasePool	*pool;
	
	pool = [[NSAutoreleasePool alloc] init];
	
	ciFilter = [[CIFilter filterWithName: (NSString *) ciFilterName] retain];
	[ciFilter setDefaults];
	
	ciFilterInputKeys = [[ciFilter inputKeys] retain];
	ciFilterInputKeysCount = [ciFilterInputKeys count];
	
	ciFilterParam = new FilterParam [ciFilterInputKeysCount];
	memset(ciFilterParam, 0, sizeof(FilterParam) * ciFilterInputKeysCount);
	
	ciFilterHasInputCenter = false;
	ciFilterHasInputImage  = false;
	
	LoadFilterPrefs();
	
	[pool release];
}

void DeinitCoreImageFilter (void)
{
	NSAutoreleasePool	*pool;

	pool = [[NSAutoreleasePool alloc] init];
	
	SaveFilterPrefs();
	
	ciFilterHasInputCenter = false;
	ciFilterHasInputImage  = false;
	
	delete [] ciFilterParam;
	
	[ciFilterInputKeys release];
	ciFilterInputKeysCount = 0;
	
	[ciFilter release];
	
	[pool release];
}

static void LoadFilterPrefs (void)
{
	CFDataRef	data;
	int			n = sizeof(FilterParam) * ciFilterInputKeysCount;
	
	data = (CFDataRef) CFPreferencesCopyAppValue(ciFilterName, kCFPreferencesCurrentApplication);
	if (data)
	{
		if (CFDataGetLength(data) == n)
		{
			CFDataGetBytes(data, CFRangeMake(0, n), (UInt8 *) ciFilterParam);
			FilterParamToFilter();
		}
		
		CFRelease(data);
	}
	
	FilterToFilterParam();
}

static void SaveFilterPrefs (void)
{
	CFDataRef	data;
	int			n = sizeof(FilterParam) * ciFilterInputKeysCount;

	data = CFDataCreate(kCFAllocatorDefault, (UInt8 *) ciFilterParam, n);
	if (data)
	{
		CFPreferencesSetAppValue(ciFilterName, data, kCFPreferencesCurrentApplication);
		CFRelease(data);
	}
}

static void FilterParamToFilter (void)
{
	NSString	*key;
	NSNumber	*num;
	CIColor		*color;
	
	for (int i = 0; i < ciFilterInputKeysCount; i++)
	{
		key = [NSString stringWithUTF8String: ciFilterParam[i].name];
		if (key)
		{		
			switch (ciFilterParam[i].type)
			{
				case kCITypeBoolean:
					num = [NSNumber numberWithBool: ciFilterParam[i].u.b.cur];
					[ciFilter setValue: num forKey: key];
					break;
					
				case kCITypeScalar:
					num = [NSNumber numberWithFloat: ciFilterParam[i].u.s.cur];
					[ciFilter setValue: num forKey: key];
					break;
					
				case kCITypeColor:
					color = [CIColor colorWithRed: ciFilterParam[i].u.c.r green: ciFilterParam[i].u.c.g
											 blue: ciFilterParam[i].u.c.b alpha: ciFilterParam[i].u.c.a];
					[ciFilter setValue: color forKey: key];
					break;
					
				default:
					break;
			}
		}
	}
}

static void FilterToFilterParam (void)
{
	NSDictionary	*attr;
	NSString		*key, *label, *className, *typeName;
	NSNumber		*num;
	CIColor			*color;
	id				param;
	
	attr = [ciFilter attributes];
	ciFilterHasInputCenter = false;
	ciFilterHasInputImage  = false;

    for (int i = 0; i < ciFilterInputKeysCount; i++)
    {
		key = [ciFilterInputKeys objectAtIndex: i];
		param = [attr objectForKey: key];
		
		strncpy(ciFilterParam[i].name, [key UTF8String], sizeof(ciFilterParam[i].name));
		ciFilterParam[i].displayName[0] = 0;
		
        if ([param isKindOfClass: [NSDictionary class]])
        {
			label = [(NSDictionary *) param objectForKey: kCIAttributeDisplayName];
			if (!label)
				label = [NSString stringWithString: key];
			strncpy(ciFilterParam[i].displayName, [label UTF8String], sizeof(ciFilterParam[i].displayName));
			
			className = [(NSDictionary *) param objectForKey: kCIAttributeClass];
			
            if ([className isEqualToString: @"NSNumber"])
            {
                typeName = [(NSDictionary *) param objectForKey: kCIAttributeType];
				
                if ([typeName isEqualToString: kCIAttributeTypeBoolean])
				{
					ciFilterParam[i].type = kCITypeBoolean;
					
					num = [ciFilter valueForKey: key];
    				ciFilterParam[i].u.b.cur = [num boolValue];
				}
                else
				{
                    ciFilterParam[i].type = kCITypeScalar;
					
					num = [ciFilter valueForKey: key];
    				ciFilterParam[i].u.s.cur = [num floatValue];
					
					num = [(NSDictionary *) param objectForKey: kCIAttributeSliderMax];
				    if (!num)
				        num = [(NSDictionary *) param objectForKey: kCIAttributeMax];
				    ciFilterParam[i].u.s.max = [num floatValue];
					
					num = [(NSDictionary *) param objectForKey: kCIAttributeSliderMin];
				    if (!num)
				        num = [(NSDictionary *) param objectForKey: kCIAttributeMin];
				    ciFilterParam[i].u.s.min = [num floatValue];
				}
            }
            else
			if ([className isEqualToString: @"CIColor"])
			{
				ciFilterParam[i].type = kCITypeColor;
				
				color = [ciFilter valueForKey: key];
				ciFilterParam[i].u.c.r = [color red];
				ciFilterParam[i].u.c.g = [color green];
				ciFilterParam[i].u.c.b = [color blue];
				ciFilterParam[i].u.c.a = [color alpha];
			}
            else
			{
				ciFilterParam[i].type = kCITypeNone;
				
				if ([className isEqualToString: @"CIVector"] && [key isEqualToString: @"inputCenter"])
					ciFilterHasInputCenter = true;
					
				if ([className isEqualToString: @"CIImage" ] && [key isEqualToString: @"inputImage" ])
					ciFilterHasInputImage  = true;
			}
		}
    }
}

static void BuildCoreImageFilterListAndMenu (void)
{
	NSArray		*categories, *filterNames;
	OSStatus	err;
	
	categories = [NSArray arrayWithObject: kCICategoryStillImage];
	filterNames = [CIFilter filterNamesInCategories: categories];
	
	ciFilterNameList = [[NSMutableArray alloc] initWithCapacity: 1];
	ciFilterLocalizedNameList = [[NSMutableArray alloc] initWithCapacity: 1];
	err = CreateNewMenu(mCoreImageFilter, 0, &ciFilterMenu);
	
	int	n = [filterNames count], m = 0;
	for (int i = 0; i < n; i++)
	{
		CIFilter	*filter;
		NSString	*name, *localName;
		
		name = [filterNames objectAtIndex: i];
		filter = [CIFilter filterWithName: name];
		
		if (IsCoreImageFilterSupported(filter))
		{
			[ciFilterNameList addObject: name];
			
			localName = [CIFilter localizedNameForFilterName: name];
			if (!localName)
				localName = [NSString stringWithString: name];
			
			[ciFilterLocalizedNameList addObject: localName];
			
			err = AppendMenuItemTextWithCFString(ciFilterMenu, (CFStringRef) localName, 0, kCommandFilterMenuBase + m, NULL);
			m++;
		}
	}
}

static void ReleaseCoreImageFilterListAndMenu (void)
{
	CFRelease(ciFilterMenu);
	[ciFilterLocalizedNameList release];
	[ciFilterNameList release];
}

static bool8 IsCoreImageFilterSupported (CIFilter *filter)
{
	NSDictionary	*attr;
	NSArray			*inputKeys;
	NSString		*key, *className;
	id				param;
	bool8			result = true, hasInputImage = false;
	
	attr = [filter attributes];
	inputKeys = [filter inputKeys];
	
	int	n = [inputKeys count];
	for (int i = 0; i < n; i++)
	{
	    key = [inputKeys objectAtIndex: i];
		param = [attr objectForKey: key];
	    
		if ([param isKindOfClass: [NSDictionary class]])
	    {
	        className = [(NSDictionary *) param objectForKey: kCIAttributeClass];
			
			if ([className isEqualToString: @"CIImage"])
			{
				if (![key isEqualToString: @"inputImage"])
					result = false;
				else
					hasInputImage = true;
			}
			else
			if ([className isEqualToString: @"CIVector"])
			{
				if (![key isEqualToString: @"inputCenter"])
					result = false;
			}
			else
			if (![className isEqualToString: @"NSNumber"] && ![className isEqualToString: @"CIColor"])
				result = false;
		}
	}
	
	if (hasInputImage == false)
		result = false;
	
	return (result);
}

void ConfigureCoreImageFilter (void)
{
	NSAutoreleasePool	*pool;
	OSStatus			err;
	IBNibRef			nibRef;
	
	pool = [[NSAutoreleasePool alloc] init];
	
	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	window;
		
		err = CreateWindowFromNib(nibRef, CFSTR("CIFilter"), &window);
		if (err == noErr)
		{
			EventHandlerRef	eref;
			EventHandlerUPP	eUPP;
			EventTypeSpec	event[] = { { kEventClassWindow,  kEventWindowClose         },
										{ kEventClassCommand, kEventCommandProcess      },
										{ kEventClassCommand, kEventCommandUpdateStatus } };
			HIViewRef		ctl, root;
			HIViewID		cid;
			Rect			rct;
			int				value;
			
			ciFilterUIPane = NULL;
			
			FilterToFilterParam();
			
			root = HIViewGetRoot(window);
						
			SetHIViewID(&cid, 'FILT', 0);
			rct.left   = 74;
			rct.top    = 20;
			rct.right  = 74 + 279;
			rct.bottom = 20 +  20;
			err = CreatePopupButtonControl(window, &rct, NULL, -12345, false, 0, 0, 0, &ctl);
			HIViewSetID(ctl, cid);
			int	n = CountMenuItems(ciFilterMenu);
			SetControlPopupMenuHandle(ctl, ciFilterMenu);
			HIViewSetMaximum(ctl, n);
			value = [ciFilterNameList indexOfObject: (NSString *) ciFilterName];
			HIViewSetValue(ctl, value + 1);
			
			ReplaceFilterUI(window);
			
			eUPP = NewEventHandlerUPP(CoreImageFilterEventHandler);
			err = InstallWindowEventHandler(window, eUPP, GetEventTypeCount(event), event, (void *) window, &eref);
						
			MoveWindowPosition(window, kWindowCoreImageFilter, false);
			ShowWindow(window);
			err = RunAppModalLoopForWindow(window);
			HideWindow(window);
			SaveWindowPosition(window, kWindowCoreImageFilter);
			
			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eUPP);
			
			FilterParamToFilter();
			
			CFRelease(window);
		}
		
		DisposeNibReference(nibRef);
	}
	
	[pool release];
}

static void ReplaceFilterUI (WindowRef window)
{
	OSStatus	err;
	HIRect		frame;
	Rect		bounds, rct;
		
	if (ciFilterUIPane)
	{
		HIViewSetVisible(ciFilterUIPane, false);
		DisposeControl(ciFilterUIPane);
		ciFilterUIPane = NULL;
	}
	
	GetWindowBounds(window, kWindowStructureRgn, &bounds);

	rct.left   = 15;
	rct.right  = bounds.right - bounds.left - 15;
	rct.top    = 81;
	rct.bottom = rct.top + 40;
	err = CreateUserPaneControl(window, &rct, kControlSupportsEmbedding, &ciFilterUIPane);
	HIViewSetVisible(ciFilterUIPane, false);
	FilterUIAddSubviews(window, ciFilterUIPane);
	
	HIViewGetFrame(ciFilterUIPane, &frame);
	bounds.bottom = bounds.top + (short) (frame.origin.y + frame.size.height + 30);
	
	err = TransitionWindow(window, kWindowSlideTransitionEffect, kWindowResizeTransitionAction, &bounds);
	HIViewSetVisible(ciFilterUIPane, true);
}

static void FilterUIAddSubviews (WindowRef window, HIViewRef parent)
{
	OSStatus			err;
	CFMutableStringRef	label;
	CFStringRef			str;
	HIViewRef			ctl;
	HIViewID			cid;
	HIRect				bounds, frame;
	Rect				rct;
	SInt32				value;
	
	HIViewGetFrame(parent, &bounds);
	rct.left   = 0;
	rct.top    = 0;
	rct.right  = 200;
	rct.bottom = 20;
	
	int	m = 0;
	for (int i = 0; i < ciFilterInputKeysCount; i++)
    {
		str = CFStringCreateWithCString(kCFAllocatorDefault, ciFilterParam[i].displayName, kCFStringEncodingUTF8);
		if (!str)
			str = CFStringCreateCopy(kCFAllocatorDefault, CFSTR("Parameter"));
		label = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, str);
		CFRelease(str);
		
		switch (ciFilterParam[i].type)
		{
			case kCITypeBoolean:
			{
				err = CreateCheckBoxControl(window, &rct, label, ciFilterParam[i].u.b.cur, true, &ctl);
				SetHIViewID(&cid, kCommandCheckBoxBase + i, i);
				HIViewSetID(ctl, cid);
				HIViewSetCommandID(ctl, cid.signature);
				err = HIViewAddSubview(parent, ctl);
				frame.origin.x = 5.0f;
				frame.origin.y = (float) (m * 28);
				frame.size.width  = bounds.size.width - 10.0f;
				frame.size.height = 20.0f;
				err = HIViewSetFrame(ctl, &frame);
				m++;
				
				break;
			}
			
			case kCITypeScalar:
			{
				CFStringAppend(label, CFSTR(" :"));
				err = CreateStaticTextControl(window, &rct, label, NULL, &ctl);
				SetStaticTextTrunc(ctl, truncEnd, true);
				err = HIViewAddSubview(parent, ctl);
				frame.origin.x = 5.0f;
				frame.origin.y = (float) (m * 28);
				frame.size.width  = 120.0f;
				frame.size.height = 20.0f;
				err = HIViewSetFrame(ctl, &frame);
				
				value = (SInt32) ((ciFilterParam[i].u.s.cur - ciFilterParam[i].u.s.min) / (ciFilterParam[i].u.s.max - ciFilterParam[i].u.s.min) * (float) FIXEDRANGE);
				err = CreateSliderControl(window, &rct, value, 0, FIXEDRANGE, kControlSliderDoesNotPoint, 0, false, NULL, &ctl);
				SetHIViewID(&cid, kCommandSliderBase + i, i);
				HIViewSetID(ctl, cid);
				HIViewSetCommandID(ctl, cid.signature);
				err = HIViewAddSubview(parent, ctl);
				frame.origin.x = 135.0f;
				frame.origin.y = (float) (m * 28) - 1.0f;
				frame.size.width  = bounds.size.width - 140.0f;
				frame.size.height = 20.0f;
				err = HIViewSetFrame(ctl, &frame);
				m++;
				
				break;
			}
			
			case kCITypeColor:
			{
				CFStringAppend(label, CFSTR("..."));
				err = CreatePushButtonControl(window, &rct, label, &ctl);
				SetHIViewID(&cid, kCommandColorButtonBase + i, i);
				HIViewSetID(ctl, cid);
				HIViewSetCommandID(ctl, cid.signature);
				err = HIViewAddSubview(parent, ctl);
				frame.origin.x = bounds.size.width - 180.0f;
				frame.origin.y = (float) (m * 28);
				frame.size.width  = 175.0f;
				frame.size.height = 20.0f;
				err = HIViewSetFrame(ctl, &frame);
				m++;
				
				break;
			}
			
			default:
				break;
		}
		
		CFRelease(label);
	}
	
	if (m)
	{
		str = CFCopyLocalizedString(CFSTR("ResetCIFilter"), "Reset");
		err = CreatePushButtonControl(window, &rct, str, &ctl);
		SetHIViewID(&cid, 'rSET', 0);
		HIViewSetID(ctl, cid);
		HIViewSetCommandID(ctl, cid.signature);
		err = HIViewAddSubview(parent, ctl);
		frame.origin.x = bounds.size.width - 180.0f;
		frame.origin.y = (float) (m * 28 + 12);
		frame.size.width  = 175.0f;
		frame.size.height = 20.0f;
		err = HIViewSetFrame(ctl, &frame);
		CFRelease(str);
		bounds.size.height = frame.origin.y + 32.0f;
	}
	else
		bounds.size.height = 4.0f;

	err = HIViewSetFrame(parent, &bounds);
}

static void FilterUISetValues (HIViewRef parent)
{
	HIViewRef	ctl;
	HIViewID	cid;
	SInt32		value;
	
	for (int i = 0; i < ciFilterInputKeysCount; i++)
    {
		switch (ciFilterParam[i].type)
		{
			case kCITypeBoolean:
				SetHIViewID(&cid, kCommandCheckBoxBase + i, i);
				HIViewFindByID(parent, cid, &ctl);
				HIViewSetValue(ctl, ciFilterParam[i].u.b.cur);
				break;
				
			case kCITypeScalar:
				value = (SInt32) ((ciFilterParam[i].u.s.cur - ciFilterParam[i].u.s.min) / (ciFilterParam[i].u.s.max - ciFilterParam[i].u.s.min) * (float) FIXEDRANGE);
				SetHIViewID(&cid, kCommandSliderBase + i, i);
				HIViewFindByID(parent, cid, &ctl);
				HIViewSetValue(ctl, value);
				break;
				
			default:
				break;
		}
	}
}

static pascal OSStatus CoreImageFilterEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	window = (WindowRef) inUserData;
	
	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
					QuitAppModalLoopForWindow(window);
					result = noErr;
			}
			
			break;
			
		case kEventClassCommand:
			switch (GetEventKind(inEvent))
			{
				HICommandExtended	tHICommand;

				case kEventCommandUpdateStatus:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommandExtended), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(true);
						result = noErr;
					}

					break;

				case kEventCommandProcess:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommandExtended), NULL, &tHICommand);
					if (err == noErr)
					{
						err = MPWaitOnSemaphore(cisem, kDurationForever);
						
						if (tHICommand.commandID == 'rSET')
						{
							[ciFilter setDefaults];
							FilterToFilterParam();
							FilterUISetValues(ciFilterUIPane);
							
							result = noErr;
						}
						else
						{
							unsigned long	i = tHICommand.commandID & 0x00FFFFFF;
							
							switch (tHICommand.commandID & 0xFF000000)
							{
								case kCommandFilterMenuBase:
									DeinitCoreImageFilter();
									
									CFRelease(ciFilterName);
									ciFilterName = CFStringCreateCopy(kCFAllocatorDefault, (CFStringRef) [ciFilterNameList objectAtIndex: i]);
									
									InitCoreImageFilter();
									
									ReplaceFilterUI(window);
									
									break;
									
								case kCommandCheckBoxBase:
									ciFilterParam[i].u.b.cur = !(ciFilterParam[i].u.b.cur);
									FilterParamToFilter();
									result = noErr;
									
									break;
									
								case kCommandSliderBase:
									SInt32	value;
									
									value = HIViewGetValue(tHICommand.source.control);
									ciFilterParam[i].u.s.cur = ciFilterParam[i].u.s.min + (ciFilterParam[i].u.s.max - ciFilterParam[i].u.s.min) * (float) value / (float) FIXEDRANGE;
									FilterParamToFilter();
									result = noErr;
									
									break;
									
								case kCommandColorButtonBase:
									NColorPickerInfo	info;
									
									memset(&info, 0, sizeof(NColorPickerInfo));
									info.placeWhere = kCenterOnMainScreen;
									info.flags      = kColorPickerDialogIsMoveable | kColorPickerDialogIsModal;
									info.theColor.color.rgb.red   = (int) (65535.0 * ciFilterParam[i].u.c.r);
									info.theColor.color.rgb.green = (int) (65535.0 * ciFilterParam[i].u.c.g);
									info.theColor.color.rgb.blue  = (int) (65535.0 * ciFilterParam[i].u.c.b);
									
									err = NPickColor(&info);
									
									if ((err == noErr) && info.newColorChosen)
									{
										ciFilterParam[i].u.c.r = (float) info.theColor.color.rgb.red   / 65535.0f;
										ciFilterParam[i].u.c.g = (float) info.theColor.color.rgb.green / 65535.0f;
										ciFilterParam[i].u.c.b = (float) info.theColor.color.rgb.blue  / 65535.0f;
									}
									
									FilterParamToFilter();
									result = noErr;
									
									break;
							}
						}
						
						err = MPSignalSemaphore(cisem);
					}
			}
	}
	
	return (result);
}

void InitCoreImageContext (CGLContextObj cglctx, CGLPixelFormatObj cglpix)
{
	NSAutoreleasePool	*pool;

	pool = [[NSAutoreleasePool alloc] init];

	FilterToFilterParam();

	cgColor = CGColorSpaceCreateDeviceRGB();

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	ciContext = [[CIContext contextWithCGLContext: cglctx pixelFormat: cglpix options: NULL] retain];
#else
	ciContext = [[CIContext contextWithCGLContext: cglctx pixelFormat: cglpix colorSpace: cgColor options: NULL] retain];
#endif

	[pool release];
}

void DeinitCoreImageContext (void)
{
	NSAutoreleasePool	*pool;

	pool = [[NSAutoreleasePool alloc] init];

	[ciContext release];
	CGColorSpaceRelease(cgColor);

	[pool release];
}

void DrawWithCoreImageFilter (CGRect src, CGImageRef img)
{
	OSStatus			err;
	NSAutoreleasePool	*pool;

	pool = [[NSAutoreleasePool alloc] init];

	err = MPWaitOnSemaphore(cisem, kDurationForever);

	if (ciFilterHasInputImage)
	{
		CIImage		*image;

		image = [CIImage imageWithCGImage: img];
		[ciFilter setValue: image  forKey: @"inputImage" ];
	}

	if (ciFilterHasInputCenter)
	{
		CIVector	*vector;

		vector = [CIVector vectorWithX: (src.origin.x + src.size.width / 2) Y: (src.origin.y + src.size.height / 2)];
		[ciFilter setValue: vector forKey: @"inputCenter"];
	}

	[ciContext drawImage: [ciFilter valueForKey: @"outputImage"] atPoint: CGPointZero fromRect: src];

	err = MPSignalSemaphore(cisem);

	[pool release];
}
