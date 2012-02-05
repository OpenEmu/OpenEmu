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


#include "port.h"

#include <IOKit/hid/IOHIDUsageTables.h>
#include "HID_Utilities_External.h"

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-joypad.h"

#define	kUp(i)					(i * 4)
#define	kDn(i)					(i * 4 + 1)
#define	kLf(i)					(i * 4 + 2)
#define	kRt(i)					(i * 4 + 3)

#define	kPadElemTypeNone		0
#define	kPadElemTypeHat4		1
#define	kPadElemTypeHat8		2
#define	kPadElemTypeAxis		3
#define	kPadElemTypeButton		4
#define	kPadElemTypeOtherHat4	5
#define	kPadElemTypeOtherHat8	6

#define	kPadXAxis				1
#define	kPadYAxis				0
#define	kPadHat					0

#define	kMaskUp					0x0800
#define	kMaskDn					0x0400
#define	kMaskLf					0x0200
#define	kMaskRt					0x0100

typedef	hu_device_t		*pRecDevice;
typedef	hu_element_t	*pRecElement;

typedef struct	actionStruct
{
	pRecDevice 	fDevice;
	pRecElement	fElement;
	long		fValue;
	long		fOldValue;
}	actionRec, *actionPtr;

typedef struct	padDirectionInfo
{
	int			type;
	pRecDevice	device [2];
	pRecElement	element[2];
	long		max    [2];
	long		maxmid [2];
	long		mid    [2];
	long		midmin [2];
	long		min    [2];
}	directionInfo;

static actionRec		gActionRecs[kNeedCount];
static directionInfo	gDirectionInfo[MAC_MAX_PLAYERS];
static int				gDirectionHint[MAC_MAX_PLAYERS];

static const HIViewID	gControlIDs[kNeedCount] =
{
	{ '1_Up', 0 },
	{ '1_Dn', 0 },
	{ '1_Lf', 0 },
	{ '1_Rt', 0 },

	{ '2_Up', 0 },
	{ '2_Dn', 0 },
	{ '2_Lf', 0 },
	{ '2_Rt', 0 },

	{ '3_Up', 0 },
	{ '3_Dn', 0 },
	{ '3_Lf', 0 },
	{ '3_Rt', 0 },

	{ '4_Up', 0 },
	{ '4_Dn', 0 },
	{ '4_Lf', 0 },
	{ '4_Rt', 0 },

	{ '5_Up', 0 },
	{ '5_Dn', 0 },
	{ '5_Lf', 0 },
	{ '5_Rt', 0 },

	{ '6_Up', 0 },
	{ '6_Dn', 0 },
	{ '6_Lf', 0 },
	{ '6_Rt', 0 },

	{ '7_Up', 0 },
	{ '7_Dn', 0 },
	{ '7_Lf', 0 },
	{ '7_Rt', 0 },

	{ '8_Up', 0 },
	{ '8_Dn', 0 },
	{ '8_Lf', 0 },
	{ '8_Rt', 0 },

	{ '1__B', 0 },
	{ '1__A', 0 },
	{ '1__X', 0 },
	{ '1__Y', 0 },
	{ '1__L', 0 },
	{ '1__R', 0 },
	{ '1Sel', 0 },
	{ '1Srt', 0 },

	{ '2__B', 0 },
	{ '2__A', 0 },
	{ '2__X', 0 },
	{ '2__Y', 0 },
	{ '2__L', 0 },
	{ '2__R', 0 },
	{ '2Sel', 0 },
	{ '2Srt', 0 },

	{ '3__B', 0 },
	{ '3__A', 0 },
	{ '3__X', 0 },
	{ '3__Y', 0 },
	{ '3__L', 0 },
	{ '3__R', 0 },
	{ '3Sel', 0 },
	{ '3Srt', 0 },

	{ '4__B', 0 },
	{ '4__A', 0 },
	{ '4__X', 0 },
	{ '4__Y', 0 },
	{ '4__L', 0 },
	{ '4__R', 0 },
	{ '4Sel', 0 },
	{ '4Srt', 0 },

	{ '5__B', 0 },
	{ '5__A', 0 },
	{ '5__X', 0 },
	{ '5__Y', 0 },
	{ '5__L', 0 },
	{ '5__R', 0 },
	{ '5Sel', 0 },
	{ '5Srt', 0 },

	{ '6__B', 0 },
	{ '6__A', 0 },
	{ '6__X', 0 },
	{ '6__Y', 0 },
	{ '6__L', 0 },
	{ '6__R', 0 },
	{ '6Sel', 0 },
	{ '6Srt', 0 },

	{ '7__B', 0 },
	{ '7__A', 0 },
	{ '7__X', 0 },
	{ '7__Y', 0 },
	{ '7__L', 0 },
	{ '7__R', 0 },
	{ '7Sel', 0 },
	{ '7Srt', 0 },

	{ '8__B', 0 },
	{ '8__A', 0 },
	{ '8__X', 0 },
	{ '8__Y', 0 },
	{ '8__L', 0 },
	{ '8__R', 0 },
	{ '8Sel', 0 },
	{ '8Srt', 0 },

	{ '__FF', 0 },
	{ '_Frz', 0 },
	{ '_DeF', 0 },
	{ '_Snp', 0 },
	{ '_Esc', 0 },
	{ '_SPC', 0 },
	{ 'MouL', 0 },
	{ 'MouR', 0 },
	{ 'ScoT', 0 },
	{ 'ScoP', 0 },
	{ 'ScoC', 0 },
	{ 'Ofsc', 0 },
	{ '__Fn', 0 },
	{ '_Alt', 0 },
	{ 'FFUp', 0 },
	{ 'FFDn', 0 },
	{ '__TC', 0 }
};

static char	gNeeds[kNeedCount][64] =
{
	"1P Up",
	"1P Down",
	"1P Left",
	"1P Right",

	"2P Up",
	"2P Down",
	"2P Left",
	"2P Right",

	"3P Up",
	"3P Down",
	"3P Left",
	"3P Right",

	"4P Up",
	"4P Down",
	"4P Left",
	"4P Right",

	"5P Up",
	"5P Down",
	"5P Left",
	"5P Right",

	"6P Up",
	"6P Down",
	"6P Left",
	"6P Right",

	"7P Up",
	"7P Down",
	"7P Left",
	"7P Right",

	"8P Up",
	"8P Down",
	"8P Left",
	"8P Right",

	"1P B Button",
	"1P A Button",
	"1P X Button",
	"1P Y Button",
	"1P L Button",
	"1P R Button",
	"1P Select",
	"1P Start",

	"2P B Button",
	"2P A Button",
	"2P X Button",
	"2P Y Button",
	"2P L Button",
	"2P R Button",
	"2P Select",
	"2P Start",

	"3P B Button",
	"3P A Button",
	"3P X Button",
	"3P Y Button",
	"3P L Button",
	"3P R Button",
	"3P Select",
	"3P Start",

	"4P B Button",
	"4P A Button",
	"4P X Button",
	"4P Y Button",
	"4P L Button",
	"4P R Button",
	"4P Select",
	"4P Start",

	"5P B Button",
	"5P A Button",
	"5P X Button",
	"5P Y Button",
	"5P L Button",
	"5P R Button",
	"5P Select",
	"5P Start",

	"6P B Button",
	"6P A Button",
	"6P X Button",
	"6P Y Button",
	"6P L Button",
	"6P R Button",
	"6P Select",
	"6P Start",

	"7P B Button",
	"7P A Button",
	"7P X Button",
	"7P Y Button",
	"7P L Button",
	"7P R Button",
	"7P Select",
	"7P Start",

	"8P B Button",
	"8P A Button",
	"8P X Button",
	"8P Y Button",
	"8P L Button",
	"8P R Button",
	"8P Select",
	"8P Start",

	"Fast Forward",
	"Freeze Game",
	"Defrost Game",
	"Screenshot",
	"Break",
	"Save SPC",
	"Mouse Left",
	"Mouse Right",
	"Scope Turbo",
	"Scope Pause",
	"Scope Cursor",
	"Offscreen",
	"Fn Modifier",
	"Alt Modifier",
	"Turbo Speed Up",
	"Turbo Speed Down",
	"Turbo Control Modifier"
};

static int	gIconNumber[kNeedCount] =
{
	0,
	1,
	2,
	3,

	12,
	13,
	14,
	15,

	24,
	25,
	26,
	27,

	36,
	37,
	38,
	39,

	48,
	49,
	50,
	51,

	60,
	61,
	62,
	63,

	72,
	73,
	74,
	75,

	84,
	85,
	86,
	87,

	5,
	7,
	6,
	4,
	8,
	9,
	11,
	10,

	17,
	19,
	18,
	16,
	20,
	21,
	23,
	22,

	29,
	31,
	30,
	28,
	32,
	33,
	35,
	34,

	41,
	43,
	42,
	40,
	44,
	45,
	47,
	46,

	53,
	55,
	54,
	52,
	56,
	57,
	59,
	58,

	65,
	67,
	66,
	64,
	68,
	69,
	71,
	70,

	77,
	79,
	78,
	76,
	80,
	81,
	83,
	82,

	89,
	91,
	90,
	88,
	92,
	93,
	95,
	94,

	101,
	102,
	103,
	104,
	114,
	105,
	116,
	117,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	115
};

static void JoypadSetDirectionInfo (void);
static pascal void IdleTimer (EventLoopTimerRef, void *);
static pascal OSStatus ControllerEventHandler (EventHandlerCallRef, EventRef, void *);


void SaveControllerSettings (void)
{
	CFStringRef	keyCFStringRef;
 	Boolean		syncFlag;

	JoypadSetDirectionInfo();

    for (int a = 0; a < kNeedCount; a++)
    {
		char	needCStr[64], num[10];

		strcpy(needCStr, gNeeds[a]);
		if (padSetting > 1)
		{
			sprintf(num, "_%d", padSetting);
			strcat(needCStr, num);
		}

		keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s"), needCStr);
        if (keyCFStringRef)
        {
			if (gActionRecs[a].fDevice && gActionRecs[a].fElement)
				syncFlag = HIDSaveElementPref(keyCFStringRef, kCFPreferencesCurrentApplication, gActionRecs[a].fDevice, gActionRecs[a].fElement);
            else
				CFPreferencesSetAppValue(keyCFStringRef, NULL, kCFPreferencesCurrentApplication);

			CFRelease(keyCFStringRef);
        }
    }

	for (int a = 0; a < MAC_MAX_PLAYERS; a++)
	{
		keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("DirectionHint_%d_%d"), a, padSetting);
		if (keyCFStringRef)
		{
			CFNumberRef	numRef;
			CFIndex		v;

			v = (CFIndex) gDirectionHint[a];
			numRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &v);
			if (numRef)
			{
				CFPreferencesSetAppValue(keyCFStringRef, numRef, kCFPreferencesCurrentApplication);
				CFRelease(numRef);
			}

			CFRelease(keyCFStringRef);
		}
	}

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

void LoadControllerSettings (void)
{
	CFStringRef	keyCFStringRef;

    for (int a = 0; a < kNeedCount; a++)
    {
		pRecDevice	pDevice  = NULL;
		pRecElement	pElement = NULL;
		Boolean		r = false;
		char		needCStr[64], num[10];

		strcpy(needCStr, gNeeds[a]);
		if (padSetting > 1)
		{
			sprintf(num, "_%d", padSetting);
			strcat(needCStr, num);
		}

		keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s"), needCStr);
		if (keyCFStringRef)
		{
			r = HIDRestoreElementPref(keyCFStringRef, kCFPreferencesCurrentApplication, &pDevice, &pElement);
			if (r && pDevice && pElement)
			{
				gActionRecs[a].fDevice  = pDevice;
				gActionRecs[a].fElement = pElement;
			}
			else
			{
				gActionRecs[a].fDevice  = NULL;
				gActionRecs[a].fElement = NULL;
			}

			CFRelease(keyCFStringRef);
		}
	}

	for (int a = 0; a < MAC_MAX_PLAYERS; a++)
	{
		keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("DirectionHint_%d_%d"), a, padSetting);
		if (keyCFStringRef)
		{
			Boolean	r;

			gDirectionHint[a] = (int) CFPreferencesGetAppIntegerValue(keyCFStringRef, kCFPreferencesCurrentApplication, &r);
			if (!r)
				gDirectionHint[a] = kPadElemTypeNone;

			CFRelease(keyCFStringRef);
		}
		else
			gDirectionHint[a] = kPadElemTypeNone;
	}

	JoypadSetDirectionInfo();
}

static pascal OSStatus ControllerEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef;

	tWindowRef = (WindowRef) inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
					QuitAppModalLoopForWindow(tWindowRef);
					result = noErr;
					break;
			}

			break;

		case kEventClassCommand:
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(true);
						result = noErr;
					}

					break;

				case kEventCommandProcess:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						if (tHICommand.commandID == 'CLRa')
						{
							ClearPadSetting();
							result = noErr;
						}
						else
						{
							SInt32	command = -1, count;

							for (count = 0; count < kNeedCount; count++)
								if (tHICommand.commandID == gControlIDs[count].signature)
									command = count;

							if (command >= 0)
							{
								pRecDevice	pDevice;
								pRecElement	pElement;

								FlushEventQueue(GetCurrentEventQueue());

								if (HIDConfigureAction(&pDevice, &pElement, 2.5f))
								{
									if (command < MAC_MAX_PLAYERS * 4)	// Direction
									{
										int		i    = command >> 2;	// Player
										long	curv = HIDGetElementValue(pDevice, pElement);

										if (pElement->usage == kHIDUsage_GD_Hatswitch)	// Hat Switch
										{
											gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = pDevice;
											gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = pElement;

											if (pDevice->vendorID == 1103)	// Thrustmaster
												gDirectionInfo[i].type = (pElement->max > 4) ? kPadElemTypeOtherHat8 : kPadElemTypeOtherHat4;
											else
											{
												if (pElement->max > 4)
												{
													if (((command % 4 == 0) && (curv == 0)) ||	// Up    : 0
														((command % 4 == 1) && (curv == 4)) ||	// Down  : 4
														((command % 4 == 2) && (curv == 6)) ||	// Left  : 6
														((command % 4 == 3) && (curv == 2)))	// Right : 2
														gDirectionInfo[i].type = kPadElemTypeOtherHat8;
													else
														gDirectionInfo[i].type = kPadElemTypeHat8;
												}
												else
												{
													if (((command % 4 == 0) && (curv == 0)) ||	// Up    : 0
														((command % 4 == 1) && (curv == 2)) ||	// Down  : 2
														((command % 4 == 2) && (curv == 3)) ||	// Left  : 3
														((command % 4 == 3) && (curv == 1)))	// Right : 1
														gDirectionInfo[i].type = kPadElemTypeOtherHat4;
													else
														gDirectionInfo[i].type = kPadElemTypeHat4;
												}
											}

											gDirectionInfo[i].device [kPadHat] = pDevice;
											gDirectionInfo[i].element[kPadHat] = pElement;
											gDirectionInfo[i].max    [kPadHat] = pElement->max;
											gDirectionInfo[i].min    [kPadHat] = pElement->min;
										}
										else
										if (pElement->max - pElement->min > 1)			// Axis (maybe)
										{
											if ((command % 4 == 0) || (command % 4 == 1))	// Up or Dn
											{
												gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = pDevice;
												gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = pElement;

												gDirectionInfo[i].type               = kPadElemTypeAxis;
												gDirectionInfo[i].device [kPadYAxis] = pDevice;
												gDirectionInfo[i].element[kPadYAxis] = pElement;
												gDirectionInfo[i].max    [kPadYAxis] = pElement->max;
												gDirectionInfo[i].min    [kPadYAxis] = pElement->min;
												gDirectionInfo[i].mid    [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;
												gDirectionInfo[i].maxmid [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].mid[kPadYAxis]) >> 1;
												gDirectionInfo[i].midmin [kPadYAxis] = (gDirectionInfo[i].mid[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;
											}
											else											// Lf or Rt
											{
												gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = pDevice;
												gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = pElement;

												gDirectionInfo[i].type               = kPadElemTypeAxis;
												gDirectionInfo[i].device [kPadXAxis] = pDevice;
												gDirectionInfo[i].element[kPadXAxis] = pElement;
												gDirectionInfo[i].max    [kPadXAxis] = pElement->max;
												gDirectionInfo[i].min    [kPadXAxis] = pElement->min;
												gDirectionInfo[i].mid    [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
												gDirectionInfo[i].maxmid [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].mid[kPadXAxis]) >> 1;
												gDirectionInfo[i].midmin [kPadXAxis] = (gDirectionInfo[i].mid[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
											}
										}
										else											// Button (maybe)
										{
											gActionRecs[command].fDevice  = pDevice;
											gActionRecs[command].fElement = pElement;
											gDirectionInfo[i].type = kPadElemTypeButton;
										}

										gDirectionHint[i] = gDirectionInfo[i].type;
									}
									else
									{
										gActionRecs[command].fDevice  = pDevice;
										gActionRecs[command].fElement = pElement;
									}
								}
								else
								{
									if (command < MAC_MAX_PLAYERS * 4)	// Direction
									{
										int	i = command >> 2;	// Player

										gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = NULL;
										gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = NULL;

										gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeNone;
										gDirectionInfo[i].device [0] = gDirectionInfo[i].device [1] = NULL;
										gDirectionInfo[i].element[0] = gDirectionInfo[i].element[1] = NULL;
									}
									else
									{
										gActionRecs[command].fDevice  = NULL;
										gActionRecs[command].fElement = NULL;
									}
								}

								gActionRecs[command].fValue    = 0;
								gActionRecs[command].fOldValue = -2;

								FlushEventQueue(GetCurrentEventQueue());

								result = noErr;
							}
						}
					}

					break;
			}

			break;
	}

	return (result);
}

static pascal void IdleTimer (EventLoopTimerRef inTimer, void *userData)
{
	static uint32	old[MAC_MAX_PLAYERS] = { ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0 };

	HIViewRef	ctl, root;
	uint32		pad[MAC_MAX_PLAYERS];

	root = HIViewGetRoot((WindowRef) userData);

	for (int i = 0; i < MAC_MAX_PLAYERS; i++)
	{
		pad[i] = 0;
		JoypadScanDirection(i, &(pad[i]));

		if (old[i] != pad[i])
		{
			old[i]  = pad[i];

			HIViewFindByID(root, gControlIDs[kUp(i)], &ctl);
			SetControl32BitValue(ctl, (pad[i] & kMaskUp) ? 1 : 0);
			HIViewFindByID(root, gControlIDs[kDn(i)], &ctl);
			SetControl32BitValue(ctl, (pad[i] & kMaskDn) ? 1 : 0);
			HIViewFindByID(root, gControlIDs[kLf(i)], &ctl);
			SetControl32BitValue(ctl, (pad[i] & kMaskLf) ? 1 : 0);
			HIViewFindByID(root, gControlIDs[kRt(i)], &ctl);
			SetControl32BitValue(ctl, (pad[i] & kMaskRt) ? 1 : 0);
		}
	}

	for (int i = MAC_MAX_PLAYERS * 4; i < kNeedCount; i++)
	{
		gActionRecs[i].fValue = ISpKeyIsPressed(i);

		if (gActionRecs[i].fOldValue != gActionRecs[i].fValue)
		{
			gActionRecs[i].fOldValue  = gActionRecs[i].fValue;

			HIViewFindByID(root, gControlIDs[i], &ctl);
			SetControl32BitValue(ctl, (gActionRecs[i].fValue ? 1 : 0));
		}
	}
}

void SetUpHID (void)
{
	pRecDevice	device;

	HIDBuildDeviceList(NULL, NULL);
	device = HIDGetFirstDevice();
	if (!device)
	{
		hidExist = false;
		return;
	}

	hidExist = true;

	ClearPadSetting();

	LoadControllerSettings();
}

void ClearPadSetting (void)
{
	for (int i = 0; i < MAC_MAX_PLAYERS; i++)
	{
		gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeNone;
		gDirectionInfo[i].device [0] = gDirectionInfo[i].device [1] = NULL;
		gDirectionInfo[i].element[0] = gDirectionInfo[i].element[1] = NULL;
	}

	for (int i = 0; i < kNeedCount; i++)
	{
		gActionRecs[i].fDevice   = NULL;
		gActionRecs[i].fElement  = NULL;
		gActionRecs[i].fValue    = 0;
		gActionRecs[i].fOldValue = -2;
	}
}

void ReleaseHID (void)
{
	if (hidExist)
		HIDReleaseDeviceList();
}

void ConfigureHID (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	if (!hidExist)
		return;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;

		err = CreateWindowFromNib(nibRef, CFSTR("Controllers"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerRef				eref;
			EventLoopTimerRef			tref;
			EventHandlerUPP				eventUPP;
			EventLoopTimerUPP			timerUPP;
			EventTypeSpec				windowEvents[] = { { kEventClassCommand, kEventCommandProcess      },
														   { kEventClassCommand, kEventCommandUpdateStatus },
														   { kEventClassWindow,  kEventWindowClose         } };
			HIViewRef					ctl, root;
			HIViewID					cid;
			CFStringRef					str1, str2;
			ControlButtonContentInfo	info;

			LoadControllerSettings();

			root = HIViewGetRoot(tWindowRef);
			cid.id = 0;
			cid.signature = 'PRES';
			HIViewFindByID(root, cid, &ctl);
			str1 = CFCopyLocalizedString(CFSTR("PresetNum"), "PresetNum");
			str2 = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, str1, padSetting);
			SetStaticTextCFString(ctl, str2, false);
			CFRelease(str2);
			CFRelease(str1);

			if (systemVersion >= 0x1040)
			{
				info.contentType = kControlContentCGImageRef;
				for (int i = 0; i < kNeedCount; i++)
				{
					HIViewFindByID(root, gControlIDs[i], &ctl);
					info.u.imageRef = macIconImage[gIconNumber[i]];
					err = SetBevelButtonContentInfo(ctl, &info);
				}
			}
		#ifdef MAC_PANTHER_SUPPORT
			else
			{
				info.contentType = kControlContentIconRef;
				for (int i = 0; i < kNeedCount; i++)
				{
					HIViewFindByID(root, gControlIDs[i], &ctl);
					info.u.iconRef = macIconRef[gIconNumber[i]];
					err = SetBevelButtonContentInfo(ctl, &info);
				}
			}
		#endif

			eventUPP = NewEventHandlerUPP(ControllerEventHandler);
			err = InstallWindowEventHandler(tWindowRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) tWindowRef, &eref);

			timerUPP = NewEventLoopTimerUPP(IdleTimer);
			err = InstallEventLoopTimer(GetCurrentEventLoop(), 0.0f, 0.1f, timerUPP, (void *) tWindowRef, &tref);

			MoveWindowPosition(tWindowRef, kWindowControllers, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowControllers);

			err = RemoveEventLoopTimer(tref);
			DisposeEventLoopTimerUPP(timerUPP);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eventUPP);

			CFRelease(tWindowRef);

			SaveControllerSettings();
		}

		DisposeNibReference(nibRef);
	}
}

long ISpKeyIsPressed (int needID)
{
	return (gActionRecs[needID].fDevice ? HIDGetElementValue(gActionRecs[needID].fDevice, gActionRecs[needID].fElement) : 0);
}

void JoypadScanDirection (int i, uint32 *pad)
{
	long	state;

	switch (gDirectionInfo[i].type)
	{
		case kPadElemTypeAxis:							// Axis (maybe)
			if (gDirectionInfo[i].device[kPadYAxis])	// Y-Axis
			{
				state = HIDGetElementValue(gDirectionInfo[i].device[kPadYAxis], gDirectionInfo[i].element[kPadYAxis]);
				if (state >= gDirectionInfo[i].maxmid[kPadYAxis])
					*pad |= kMaskDn;
				else
				if (state <= gDirectionInfo[i].midmin[kPadYAxis])
					*pad |= kMaskUp;
			}

			if (gDirectionInfo[i].device[kPadXAxis])	// X-Axis
			{
				state = HIDGetElementValue(gDirectionInfo[i].device[kPadXAxis], gDirectionInfo[i].element[kPadXAxis]);
				if (state >= gDirectionInfo[i].maxmid[kPadXAxis])
					*pad |= kMaskRt;
				else
				if (state <= gDirectionInfo[i].midmin[kPadXAxis])
					*pad |= kMaskLf;
			}

			break;

		case kPadElemTypeHat8:							// Hat Switch (8 Directions)
			if (gDirectionInfo[i].device[kPadHat])
			{
				state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
				switch (state)
				{
					case 1:	*pad |=  kMaskUp           ;	break;
					case 2:	*pad |= (kMaskUp | kMaskRt);	break;
					case 3:	*pad |=  kMaskRt		   ;	break;
					case 4:	*pad |= (kMaskRt | kMaskDn);	break;
					case 5:	*pad |=  kMaskDn		   ;	break;
					case 6:	*pad |= (kMaskDn | kMaskLf);	break;
					case 7:	*pad |=  kMaskLf		   ;	break;
					case 8:	*pad |= (kMaskLf | kMaskUp);	break;
				}
			}

			break;

		case kPadElemTypeHat4:							// Hat Switch (4 Directions)
			if (gDirectionInfo[i].device[kPadHat])
			{
				state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
				switch (state)
				{
					case 1:	*pad |=  kMaskUp;	break;
					case 2:	*pad |=  kMaskRt;	break;
					case 3:	*pad |=  kMaskDn;	break;
					case 4:	*pad |=  kMaskLf;	break;
				}
			}

			break;

		case kPadElemTypeOtherHat8:						// Hat Switch (8 Directions, Start at 0)
			if (gDirectionInfo[i].device[kPadHat])
			{
				state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
				switch (state)
				{
					case 0:	*pad |=  kMaskUp		   ;	break;
					case 1:	*pad |= (kMaskUp | kMaskRt);	break;
					case 2:	*pad |=  kMaskRt		   ;	break;
					case 3:	*pad |= (kMaskRt | kMaskDn);	break;
					case 4:	*pad |=  kMaskDn		   ;	break;
					case 5:	*pad |= (kMaskDn | kMaskLf);	break;
					case 6:	*pad |=  kMaskLf		   ;	break;
					case 7:	*pad |= (kMaskLf | kMaskUp);	break;
				}
			}

			break;

		case kPadElemTypeOtherHat4:						// Hat Switch (4 Directions, Start at 0)
			if (gDirectionInfo[i].device[kPadHat])
			{
				state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
				switch (state)
				{
					case 0:	*pad |=  kMaskUp;	break;
					case 1:	*pad |=  kMaskRt;	break;
					case 2:	*pad |=  kMaskDn;	break;
					case 3:	*pad |=  kMaskLf;	break;
				}
			}

			break;

		case kPadElemTypeButton:						// Button (maybe)
			if (gActionRecs[kUp(i)].fDevice && HIDGetElementValue(gActionRecs[kUp(i)].fDevice, gActionRecs[kUp(i)].fElement))
				*pad |= kMaskUp;
			if (gActionRecs[kDn(i)].fDevice && HIDGetElementValue(gActionRecs[kDn(i)].fDevice, gActionRecs[kDn(i)].fElement))
				*pad |= kMaskDn;
			if (gActionRecs[kLf(i)].fDevice && HIDGetElementValue(gActionRecs[kLf(i)].fDevice, gActionRecs[kLf(i)].fElement))
				*pad |= kMaskLf;
			if (gActionRecs[kRt(i)].fDevice && HIDGetElementValue(gActionRecs[kRt(i)].fDevice, gActionRecs[kRt(i)].fElement))
				*pad |= kMaskRt;

			break;
	}
}

static void JoypadSetDirectionInfo (void)
{
	for (int i = 0; i < MAC_MAX_PLAYERS; i++)
	{
		if (((gActionRecs[kUp(i)].fDevice) && (gActionRecs[kUp(i)].fElement)) &&
			((gActionRecs[kDn(i)].fDevice) && (gActionRecs[kDn(i)].fElement)) &&
			((gActionRecs[kLf(i)].fDevice) && (gActionRecs[kLf(i)].fElement)) &&
			((gActionRecs[kRt(i)].fDevice) && (gActionRecs[kRt(i)].fElement)))
		{
			if ((gActionRecs[kUp(i)].fDevice  == gActionRecs[kDn(i)].fDevice)  &&
				(gActionRecs[kDn(i)].fDevice  == gActionRecs[kLf(i)].fDevice)  &&
				(gActionRecs[kLf(i)].fDevice  == gActionRecs[kRt(i)].fDevice)  &&
				(gActionRecs[kUp(i)].fElement == gActionRecs[kDn(i)].fElement) &&
				(gActionRecs[kDn(i)].fElement == gActionRecs[kLf(i)].fElement) &&
				(gActionRecs[kLf(i)].fElement == gActionRecs[kRt(i)].fElement) &&
				(gActionRecs[kUp(i)].fElement->usage == kHIDUsage_GD_Hatswitch))				// Hat Switch
			{
				if ((gDirectionHint[i] == kPadElemTypeHat8) || (gDirectionHint[i] == kPadElemTypeOtherHat8) ||
					(gDirectionHint[i] == kPadElemTypeHat4) || (gDirectionHint[i] == kPadElemTypeOtherHat4))
					gDirectionInfo[i].type = gDirectionHint[i];
				else																			// Assuming...
				{
					if ((gActionRecs[kUp(i)].fDevice->vendorID == 1103) || (gActionRecs[kUp(i)].fElement->min == 0))
						gDirectionInfo[i].type = (gActionRecs[kUp(i)].fElement->max > 4) ? kPadElemTypeOtherHat8 : kPadElemTypeOtherHat4;
					else
						gDirectionInfo[i].type = (gActionRecs[kUp(i)].fElement->max > 4) ? kPadElemTypeHat8      : kPadElemTypeHat4;

					gDirectionHint[i] = gDirectionInfo[i].type;
				}

				gDirectionInfo[i].device [kPadHat] =  gActionRecs[kUp(i)].fDevice;
				gDirectionInfo[i].element[kPadHat] =  gActionRecs[kUp(i)].fElement;
				gDirectionInfo[i].max    [kPadHat] =  gActionRecs[kUp(i)].fElement->max;
				gDirectionInfo[i].min    [kPadHat] =  gActionRecs[kUp(i)].fElement->min;
			}
			else
			if ((gActionRecs[kUp(i)].fDevice  == gActionRecs[kDn(i)].fDevice)               &&
				(gActionRecs[kLf(i)].fDevice  == gActionRecs[kRt(i)].fDevice)               &&
				(gActionRecs[kUp(i)].fElement == gActionRecs[kDn(i)].fElement)              &&
				(gActionRecs[kLf(i)].fElement == gActionRecs[kRt(i)].fElement)              &&
				(gActionRecs[kUp(i)].fElement->max - gActionRecs[kUp(i)].fElement->min > 1) &&
				(gActionRecs[kLf(i)].fElement->max - gActionRecs[kLf(i)].fElement->min > 1))	// Axis (maybe)
			{
				gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeAxis;

				gDirectionInfo[i].device [kPadYAxis] = gActionRecs[kUp(i)].fDevice;
				gDirectionInfo[i].element[kPadYAxis] = gActionRecs[kUp(i)].fElement;
				gDirectionInfo[i].max    [kPadYAxis] = gActionRecs[kUp(i)].fElement->max;
				gDirectionInfo[i].min    [kPadYAxis] = gActionRecs[kUp(i)].fElement->min;
				gDirectionInfo[i].mid    [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;
				gDirectionInfo[i].maxmid [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].mid[kPadYAxis]) >> 1;
				gDirectionInfo[i].midmin [kPadYAxis] = (gDirectionInfo[i].mid[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;

				gDirectionInfo[i].device [kPadXAxis] = gActionRecs[kLf(i)].fDevice;
				gDirectionInfo[i].element[kPadXAxis] = gActionRecs[kLf(i)].fElement;
				gDirectionInfo[i].max    [kPadXAxis] = gActionRecs[kLf(i)].fElement->max;
				gDirectionInfo[i].min    [kPadXAxis] = gActionRecs[kLf(i)].fElement->min;
				gDirectionInfo[i].mid    [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
				gDirectionInfo[i].maxmid [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].mid[kPadXAxis]) >> 1;
				gDirectionInfo[i].midmin [kPadXAxis] = (gDirectionInfo[i].mid[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
			}
			else																				// Button (maybe)
				gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeButton;
		}
		else
		{
			gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = NULL;
			gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = NULL;

			gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeNone;
			gDirectionInfo[i].device [0] = gDirectionInfo[i].device [1] = NULL;
			gDirectionInfo[i].element[0] = gDirectionInfo[i].element[1] = NULL;
		}
	}
}
