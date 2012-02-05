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
#include "cheats.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-stringtools.h"
#include "mac-cheat.h"

#define	kDataBrowser	'BRSR'
#define	kCmCheckBox		'CHK_'
#define	kCmAddress		'ADDR'
#define	kCmValue		'VALU'
#define	kCmDescription	'DESC'
#define	kNewButton		'NEW_'
#define	kDelButton		'DEL_'
#define	kAllButton		'ALL_'

extern SCheatData	Cheat;

typedef struct
{
	uint32	id;
	uint32	address;
	uint8	value;
	bool8	valid;
	bool8	enabled;
	char	description[22];
}	CheatItem;

static WindowRef	wRef;
static HIViewRef	dbRef;
static CheatItem	citem[MAX_CHEATS];
static uint32		numofcheats;

static void InitCheatItems (void);
static void ImportCheatItems (void);
static void DetachCheatItems (void);
static void AddCheatItem (void);
static void DeleteCheatItem (void);
static void EnableAllCheatItems (void);
static pascal void DBItemNotificationCallBack (HIViewRef, DataBrowserItemID, DataBrowserItemNotification);
static pascal Boolean DBCompareCallBack (HIViewRef, DataBrowserItemID, DataBrowserItemID, DataBrowserPropertyID);
static pascal OSStatus DBClientDataCallback (HIViewRef, DataBrowserItemID, DataBrowserPropertyID, DataBrowserItemDataRef, Boolean);
static pascal OSStatus CheatEventHandler (EventHandlerCallRef, EventRef, void *);


static void InitCheatItems (void)
{
	for (unsigned int i = 0; i < MAX_CHEATS; i++)
	{
		citem[i].id      = i + 1;
		citem[i].valid   = false;
		citem[i].enabled = false;
		citem[i].address = 0;
		citem[i].value   = 0;
		sprintf(citem[i].description, "Cheat %03" PRIu32, citem[i].id);
	}
}

static void ImportCheatItems (void)
{
	for (unsigned int i = 0; i < Cheat.num_cheats; i++)
	{
		citem[i].valid   = true;
		citem[i].enabled = Cheat.c[i].enabled;
		citem[i].address = Cheat.c[i].address;
		citem[i].value   = Cheat.c[i].byte;
		strcpy(citem[i].description, Cheat.c[i].name);
	}
}

static void DetachCheatItems (void)
{
	S9xDeleteCheats(); // Cheat.num_cheats = 0

	for (unsigned int i = 0; i < MAX_CHEATS; i++)
	{
		if (citem[i].valid)
		{
			strcpy(Cheat.c[Cheat.num_cheats].name, citem[i].description);
			S9xAddCheat(citem[i].enabled, false, citem[i].address, citem[i].value); // Cheat.num_cheats++
		}
	}

	S9xApplyCheats();
}

void ConfigureCheat (void)
{
	if (!cartOpen)
		return;

	OSStatus	err;
	IBNibRef	nibRef;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		err = CreateWindowFromNib(nibRef, CFSTR("CheatEntry"), &wRef);
		if (err == noErr)
		{
			DataBrowserCallbacks	callbacks;
			EventHandlerRef			eref;
			EventHandlerUPP			eUPP;
			EventTypeSpec			events[] = { { kEventClassCommand, kEventCommandProcess      },
												 { kEventClassCommand, kEventCommandUpdateStatus },
												 { kEventClassWindow,  kEventWindowClose         } };
			HIViewRef				ctl, root;
			HIViewID				cid;

			root = HIViewGetRoot(wRef);
			cid.id = 0;
			cid.signature = kDataBrowser;
			HIViewFindByID(root, cid, &dbRef);

		#ifdef MAC_PANTHER_SUPPORT
			if (systemVersion < 0x1040)
			{
				HISize	minSize;
				Rect	rct;

				GetWindowBounds(wRef, kWindowContentRgn, &rct);
				minSize.width  = (float) (rct.right  - rct.left);
				minSize.height = (float) (rct.bottom - rct.top );
				err = SetWindowResizeLimits(wRef, &minSize, NULL);
			}
		#endif

			callbacks.version = kDataBrowserLatestCallbacks;
			err = InitDataBrowserCallbacks(&callbacks);
			callbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP(DBClientDataCallback);
			callbacks.u.v1.itemCompareCallback = NewDataBrowserItemCompareUPP(DBCompareCallBack);
			callbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP(DBItemNotificationCallBack);
			err = SetDataBrowserCallbacks(dbRef, &callbacks);

			if (systemVersion >= 0x1040)
				err = DataBrowserChangeAttributes(dbRef, kDataBrowserAttributeListViewAlternatingRowColors, kDataBrowserAttributeNone);

			InitCheatItems();
			ImportCheatItems();

			DataBrowserItemID	*id;

			id = new DataBrowserItemID[MAX_CHEATS];
			if (!id)
				QuitWithFatalError(0, "cheat 01");

			numofcheats = 0;

			for (unsigned int i = 0; i < MAX_CHEATS; i++)
			{
				if (citem[i].valid)
				{
					id[numofcheats] = citem[i].id;
					numofcheats++;
				}
			}

			if (numofcheats)
				err = AddDataBrowserItems(dbRef, kDataBrowserNoItem, numofcheats, id, kDataBrowserItemNoProperty);

			delete [] id;

			cid.signature = kNewButton;
			HIViewFindByID(root, cid, &ctl);
			if (numofcheats == MAX_CHEATS)
				err = DeactivateControl(ctl);
			else
				err = ActivateControl(ctl);

			cid.signature = kAllButton;
			HIViewFindByID(root, cid, &ctl);
			if (numofcheats == 0)
				err = DeactivateControl(ctl);
			else
				err = ActivateControl(ctl);

			cid.signature = kDelButton;
			HIViewFindByID(root, cid, &ctl);
			err = DeactivateControl(ctl);

			eUPP = NewEventHandlerUPP(CheatEventHandler);
			err = InstallWindowEventHandler(wRef, eUPP, GetEventTypeCount(events), events, (void *) wRef, &eref);

			err = SetKeyboardFocus(wRef, dbRef, kControlFocusNextPart);

			MoveWindowPosition(wRef, kWindowCheatEntry, true);
			ShowWindow(wRef);
			err = RunAppModalLoopForWindow(wRef);
			HideWindow(wRef);
			SaveWindowPosition(wRef, kWindowCheatEntry);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eUPP);

			DisposeDataBrowserItemNotificationUPP(callbacks.u.v1.itemNotificationCallback);
			DisposeDataBrowserItemCompareUPP(callbacks.u.v1.itemCompareCallback);
			DisposeDataBrowserItemDataUPP(callbacks.u.v1.itemDataCallback);

			CFRelease(wRef);

			DetachCheatItems();
		}

		DisposeNibReference(nibRef);
	}
}

static void AddCheatItem (void)
{
	OSStatus			err;
	HIViewRef			ctl, root;
	HIViewID			cid;
	DataBrowserItemID	id[1];
	unsigned int		i;

	if (numofcheats == MAX_CHEATS)
		return;

	for (i = 0; i < MAX_CHEATS; i++)
		if (citem[i].valid == false)
			break;

	if (i == MAX_CHEATS)
		return;

	numofcheats++;
	citem[i].valid   = true;
	citem[i].enabled = false;
	citem[i].address = 0;
	citem[i].value   = 0;
	sprintf(citem[i].description, "Cheat %03" PRIu32, citem[i].id);

	id[0] = citem[i].id;
	err = AddDataBrowserItems(dbRef, kDataBrowserNoItem, 1, id, kDataBrowserItemNoProperty);
	err = RevealDataBrowserItem(dbRef, id[0], kCmAddress, true);

	root = HIViewGetRoot(wRef);
	cid.id = 0;

	if (numofcheats == MAX_CHEATS)
	{
		cid.signature = kNewButton;
		HIViewFindByID(root, cid, &ctl);
		err = DeactivateControl(ctl);
	}

	if (numofcheats)
	{
		cid.signature = kAllButton;
		HIViewFindByID(root, cid, &ctl);
		err = ActivateControl(ctl);
	}
}

static void DeleteCheatItem (void)
{
	OSStatus	err;
	HIViewRef	ctl, root;
	HIViewID	cid;
	Handle		selectedItems;
	ItemCount	selectionCount;

	selectedItems = NewHandle(0);
	if (!selectedItems)
		return;

	err = GetDataBrowserItems(dbRef, kDataBrowserNoItem, true, kDataBrowserItemIsSelected, selectedItems);
	selectionCount = (GetHandleSize(selectedItems) / sizeof(DataBrowserItemID));

	if (selectionCount == 0)
	{
		DisposeHandle(selectedItems);
		return;
	}

	err = RemoveDataBrowserItems(dbRef, kDataBrowserNoItem, selectionCount, (DataBrowserItemID *) *selectedItems, kDataBrowserItemNoProperty);

	for (unsigned int i = 0; i < selectionCount; i++)
	{
		citem[((DataBrowserItemID *) (*selectedItems))[i] - 1].valid   = false;
		citem[((DataBrowserItemID *) (*selectedItems))[i] - 1].enabled = false;
		numofcheats--;
	}

	DisposeHandle(selectedItems);

	root = HIViewGetRoot(wRef);
	cid.id = 0;

	if (numofcheats < MAX_CHEATS)
	{
		cid.signature = kNewButton;
		HIViewFindByID(root, cid, &ctl);
		err = ActivateControl(ctl);
	}

	if (numofcheats == 0)
	{
		cid.signature = kAllButton;
		HIViewFindByID(root, cid, &ctl);
		err = DeactivateControl(ctl);
	}
}

static void EnableAllCheatItems (void)
{
	OSStatus	err;

	for (unsigned int i = 0; i < MAX_CHEATS; i++)
		if (citem[i].valid)
			citem[i].enabled = true;

	err = UpdateDataBrowserItems(dbRef, kDataBrowserNoItem, kDataBrowserNoItem, NULL, kDataBrowserItemNoProperty, kCmCheckBox);
}

static pascal OSStatus DBClientDataCallback (HIViewRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue)
{
	OSStatus 	err, result;
	CFStringRef	str;
	Boolean		r;
	uint32		address;
	uint8		value;
	char		code[256];

	result = noErr;

	switch (property)
	{
		case kCmCheckBox:
			ThemeButtonValue	buttonValue;

            if (changeValue)
			{
				err = GetDataBrowserItemDataButtonValue(itemData, &buttonValue);
				citem[itemID - 1].enabled = (buttonValue == kThemeButtonOn) ? true : false;
 	        }
			else
				err = SetDataBrowserItemDataButtonValue(itemData, citem[itemID - 1].enabled ? kThemeButtonOn : kThemeButtonOff);

			break;

		case kCmAddress:
			if (changeValue)
			{
				err = GetDataBrowserItemDataText(itemData, &str);
				r = CFStringGetCString(str, code, 256, CFStringGetSystemEncoding());
				CFRelease(str);
				if (r)
				{
					Boolean	translated;

					if (S9xProActionReplayToRaw(code, address, value) == NULL)
						translated = true;
					else
					if (S9xGameGenieToRaw(code, address, value) == NULL)
						translated = true;
					else
					{
						translated = false;
						if (sscanf(code, "%" SCNx32, &address) != 1)
							address = 0;
						else
							address &= 0xFFFFFF;
					}

					citem[itemID - 1].address = address;
					sprintf(code, "%06" PRIX32, address);
					str = CFStringCreateWithCString(kCFAllocatorDefault, code, CFStringGetSystemEncoding());
					err = SetDataBrowserItemDataText(itemData, str);
					CFRelease(str);

					if (translated)
					{
						DataBrowserItemID	id[1];

						citem[itemID - 1].value = value;
						id[0] = itemID;
						err = UpdateDataBrowserItems(browser, kDataBrowserNoItem, 1, id, kDataBrowserItemNoProperty, kCmValue);
					}
				}
			}
			else
			{
				sprintf(code, "%06" PRIX32, citem[itemID - 1].address);
				str = CFStringCreateWithCString(kCFAllocatorDefault, code, CFStringGetSystemEncoding());
				err = SetDataBrowserItemDataText(itemData, str);
				CFRelease(str);
			}

			break;

		case kCmValue:
			if (changeValue)
			{
				err = GetDataBrowserItemDataText(itemData, &str);
				r = CFStringGetCString(str, code, 256, CFStringGetSystemEncoding());
				CFRelease(str);
				if (r)
				{
					uint32	byte;

					if (sscanf(code, "%" SCNx32, &byte) == 1)
						citem[itemID - 1].value = (uint8) byte;
					else
					{
						citem[itemID - 1].value = 0;
						err = SetDataBrowserItemDataText(itemData, CFSTR("00"));
					}
				}
			}
			else
			{
				sprintf(code, "%02" PRIX8, citem[itemID - 1].value);
				str = CFStringCreateWithCString(kCFAllocatorDefault, code, CFStringGetSystemEncoding());
				err = SetDataBrowserItemDataText(itemData, str);
				CFRelease(str);
			}

			break;

		case kCmDescription:
			if (changeValue)
			{
				code[0] = 0;
				err = GetDataBrowserItemDataText(itemData, &str);
				strcpy(code, GetMultiByteCharacters(str, 19));
				CFRelease(str);

				if (code[0] == 0)
				{
					code[0] = ' ';
					code[1] = 0;
				}

				strcpy(citem[itemID - 1].description, code);
			}
			else
			{
				str = CFStringCreateWithCString(kCFAllocatorDefault, citem[itemID - 1].description, CFStringGetSystemEncoding());
				err = SetDataBrowserItemDataText(itemData, str);
				CFRelease(str);
			}

			break;

		case kDataBrowserItemIsActiveProperty:
			err = SetDataBrowserItemDataBooleanValue(itemData, true);
			break;

		case kDataBrowserItemIsEditableProperty:
			err = SetDataBrowserItemDataBooleanValue(itemData, true);
			break;

		default:
			result = errDataBrowserPropertyNotSupported;
	}

	return (result);
}

static pascal Boolean DBCompareCallBack (HIViewRef browser, DataBrowserItemID itemOne, DataBrowserItemID itemTwo, DataBrowserPropertyID sortProperty)
{
	Boolean	result = false;

	switch (sortProperty)
	{
		case kCmCheckBox:
			result = (citem[itemOne - 1].enabled && !(citem[itemTwo - 1].enabled)) ? true : false;
			break;

		case kCmAddress:
			result = (citem[itemOne - 1].address <    citem[itemTwo - 1].address)  ? true : false;
			break;

		case kCmValue:
			result = (citem[itemOne - 1].value   <    citem[itemTwo - 1].value)    ? true : false;
			break;

		case kCmDescription:
			result = (strcmp(citem[itemOne - 1].description, citem[itemTwo - 1].description) < 0) ? true : false;
	}

	return (result);
}

static pascal void DBItemNotificationCallBack (HIViewRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message)
{
	OSStatus	err;
	HIViewRef	ctl;
	HIViewID	cid = { kDelButton, 0 };
	ItemCount	selectionCount;

	switch (message)
	{
		case kDataBrowserSelectionSetChanged:
			HIViewFindByID(HIViewGetRoot(wRef), cid, &ctl);

			err = GetDataBrowserItemCount(browser, kDataBrowserNoItem, true, kDataBrowserItemIsSelected, &selectionCount);
			if (selectionCount == 0)
				err = DeactivateControl(ctl);
			else
				err = ActivateControl(ctl);
	}
}

static pascal OSStatus CheatEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
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
						switch (tHICommand.commandID)
						{
							case kNewButton:
								AddCheatItem();
								result = noErr;
								break;

							case kDelButton:
								DeleteCheatItem();
								result = noErr;
								break;

							case kAllButton:
								EnableAllCheatItems();
								result = noErr;
						}
					}
			}
	}

	return (result);
}
