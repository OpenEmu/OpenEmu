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


#include "snes9x.h"
#include "memmap.h"

#include "mac-prefix.h"
#include "mac-audio.h"
#include "mac-joypad.h"
#include "mac-keyboard.h"
#include "mac-os.h"
#include "mac-render.h"
#include "mac-stringtools.h"
#include "mac-dialog.h"

int	autofireLastTabIndex = 1;

static int	tabList[] = { 2, 257, 258 };

static void RomInfoCopyToClipboard (void);
static void RomInfoBuildInfoText (char *);
static void AutofireSetAllIconImages (int, HIViewRef);
static void AutofireSetIconImages (int, HIViewRef);
static void AutofireReadAllSettings (int, HIViewRef);
static void AutofireReadSetting (int, uint16, HIViewRef);
static void AutofireWriteAllSettings (int, HIViewRef);
static void AutofireWriteSetting (int, uint16 *, HIViewRef);
static void AutofireSelectTabPane (HIViewRef, SInt16);
static OSStatus UpdateTextControlView (HIViewRef);
static pascal void AutofireSliderActionProc (HIViewRef, HIViewPartCode);
static pascal OSStatus RomInfoEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus AutofireTabEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus AutofireWindowEventHandler (EventHandlerCallRef, EventRef, void *);


static OSStatus UpdateTextControlView (HIViewRef control)
{
	OSStatus			err;
	WindowAttributes	attr;

	err = GetWindowAttributes(GetControlOwner(control), &attr);
	if (err == noErr)
	{
		if (attr & kWindowCompositingAttribute)
			err = HIViewSetNeedsDisplay(control, true);
		else
			Draw1Control(control);
	}

	return (err);
}

OSStatus SetStaticTextCStr (HIViewRef control, char *text, Boolean draw)
{
	OSStatus	err;

	if (!control || !text)
		return (paramErr);

	err = SetControlData(control, 0, kControlStaticTextTextTag, strlen(text), text);
	if ((err == noErr) && draw)
		err = UpdateTextControlView(control);

	return (err);
}

OSStatus SetStaticTextCFString (HIViewRef control, CFStringRef text, Boolean draw)
{
	OSStatus	err;

	if (!control || !text)
		return (paramErr);

	err = SetControlData(control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &text);
	if ((err == noErr) && draw)
		err = UpdateTextControlView(control);

	return (err);
}

OSStatus SetStaticTextTrunc (HIViewRef control, TruncCode mode, Boolean draw)
{
	OSStatus	err;
	TruncCode	trunc;
	Boolean		multiline;

	if (!control)
		return (paramErr);

	trunc = mode;
	multiline = false;

	err = SetControlData(control, 0, kControlStaticTextIsMultilineTag, sizeof(Boolean), &multiline);
	if (err == noErr)
	{
		err = SetControlData(control, 0, kControlStaticTextTruncTag, sizeof(TruncCode), &trunc);
		if ((err == noErr) && draw)
			err = UpdateTextControlView(control);
	}

	return (err);
}

OSStatus GetEditTextCStr (HIViewRef control, char *text)
{
	OSStatus	err;
	Size		actualSize;

	if (!control || !text)
		return (paramErr);

	err = GetControlData(control, 0, kControlEditTextTextTag, 255, text, &actualSize);
	if (err == noErr)
		text[actualSize] = 0;

	return (err);
}

OSStatus SetEditTextCStr (HIViewRef control, char *text, Boolean draw)
{
	OSStatus	err;

	if (!control || !text)
		return (paramErr);

	err = SetControlData(control, 0, kControlEditTextTextTag, strlen(text), text);
	if ((err == noErr) && draw)
		err = UpdateTextControlView(control);

	return (err);
}

OSStatus CopyEditTextCFString (HIViewRef control, CFStringRef *text)
{
	OSStatus	err;
	Size		actualSize;

	if (!control || !text)
		return (paramErr);

	err = GetControlData(control, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), text, &actualSize);

	return (err);
}

OSStatus SetEditTextCFString (HIViewRef control, CFStringRef text, Boolean draw)
{
	OSStatus	err;

	if (!control || !text)
		return (paramErr);

	err = SetControlData(control, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &text);
	if ((err == noErr) && draw)
		err = UpdateTextControlView(control);

	return (err);
}

OSStatus SetEditTextSelection (HIViewRef control, SInt16 selStart, SInt16 selEnd)
{
	OSStatus					err;
	ControlEditTextSelectionRec	selection;

	if (!control)
		return (paramErr);

	selection.selStart = selStart;
	selection.selEnd   = selEnd;

	err = SetControlData(control, 0, kControlEditTextSelectionTag, sizeof(selection), &selection);
	if (err == noErr)
		err = UpdateTextControlView(control);

	return (err);
}

void StartCarbonModalDialog (void)
{
	HiliteMenu(0);
	if (gWindow)
		HideWindow(gWindow);
}

void FinishCarbonModalDialog (void)
{
	if (gWindow)
		ShowWindow(gWindow);
}

void MoveWindowPosition (WindowRef window, int which, Boolean resize)
{
	if (savewindowpos)
	{
		MoveWindow(window, windowPos[which].h, windowPos[which].v, false);

		if (resize)
		{
			if ((windowSize[which].width > 0) && (windowSize[which].height > 0))
				SizeWindow(window, (short) windowSize[which].width, (short) windowSize[which].height, false);
		}
	}
	else
		RepositionWindow(window, NULL, kWindowAlertPositionOnMainScreen);
}

void SaveWindowPosition (WindowRef window, int which)
{
	Rect	rct;

	GetWindowBounds(window, kWindowContentRgn, &rct);
	windowPos[which].h = rct.left;
	windowPos[which].v = rct.top;
	windowSize[which].width  = (float) (rct.right  - rct.left);
	windowSize[which].height = (float) (rct.bottom - rct.top );
}

void AppearanceAlert (AlertType type, int stringID1, int stringID2)
{
	OSStatus		err;
	DialogRef		dialog;
	DialogItemIndex	outItemHit;
	CFStringRef		key1, key2, mes1, mes2;
	char			label1[32], label2[32];

	sprintf(label1, "AlertMes_%02d", stringID1);
	sprintf(label2, "AlertMes_%02d", stringID2);

	key1 = CFStringCreateWithCString(kCFAllocatorDefault, label1, CFStringGetSystemEncoding());
	key2 = CFStringCreateWithCString(kCFAllocatorDefault, label2, CFStringGetSystemEncoding());

	if (key1) mes1 = CFCopyLocalizedString(key1, "mes1");	else mes1 = NULL;
	if (key2) mes2 = CFCopyLocalizedString(key2, "mes2");	else mes2 = NULL;

	PlayAlertSound();

	err = CreateStandardAlert(type, mes1, mes2, NULL, &dialog);
	err = RunStandardAlert(dialog, NULL, &outItemHit);

	if (key1) CFRelease(key1);
	if (key2) CFRelease(key2);
	if (mes1) CFRelease(mes1);
	if (mes2) CFRelease(mes2);
}

void AboutDialog (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;

		err = CreateWindowFromNib(nibRef, CFSTR("About"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerRef		eref;
			EventHandlerUPP		eventUPP;
			EventTypeSpec		windowEvents[] = { { kEventClassWindow,  kEventWindowClose         },
												   { kEventClassCommand, kEventCommandUpdateStatus } };
			ControlFontStyleRec	frec;
			HIViewRef			ctl, root;
			HIViewID			cid;
			char				text[32];

			err = ChangeWindowAttributes(tWindowRef, kWindowNoAttributes, kWindowInWindowMenuAttribute);

			if (systemVersion >= 0x1040)
				frec.font = FMGetFontFromATSFontRef(ATSFontFindFromName(CFSTR("Lucida Grande"), kATSOptionFlagsDefault));
		#ifdef MAC_PANTHER_SUPPORT
			else
				frec.font = kThemeSystemFont;
		#endif
			frec.just = teCenter;

			root = HIViewGetRoot(tWindowRef);
			cid.id = 0;

			cid.signature = 'VERS';
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "Version %s (%s)", VERSION, MAC_VERSION);
			SetStaticTextCStr(ctl, text, false);
			frec.flags = kControlUseFontMask | kControlUseSizeMask | kControlUseJustMask;
			frec.size  = 10;
			err = SetControlFontStyle(ctl, &frec);

			cid.signature = 'NAME';
			HIViewFindByID(root, cid, &ctl);
			frec.flags = kControlUseFontMask | kControlUseSizeMask | kControlUseFaceMask | kControlUseJustMask;
			frec.size  = 14;
			frec.style = 1;
			err = SetControlFontStyle(ctl, &frec);

			eventUPP = NewEventHandlerUPP(DefaultEventHandler);
			err = InstallWindowEventHandler(tWindowRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) tWindowRef, &eref);

			MoveWindowPosition(tWindowRef, kWindowAbout, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowAbout);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eventUPP);

			CFRelease(tWindowRef);
		}

		DisposeNibReference(nibRef);
	}
}

pascal OSStatus DefaultEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef = (WindowRef) inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
					QuitAppModalLoopForWindow(tWindowRef);
					result = noErr;
			}

			break;

		case kEventClassCommand:
			switch (GetEventKind(inEvent))
			{
				case kEventCommandUpdateStatus:
					HICommand	tHICommand;

					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(true);
						result = noErr;
					}
			}

			break;
	}

	return (result);
}

void ConfigureAutofire (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;

		err = CreateWindowFromNib(nibRef, CFSTR("AutoFire"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerRef		wRef, tRef;
			EventHandlerUPP		wUPP, tUPP;
			EventTypeSpec		wEvent[] = { { kEventClassWindow,  kEventWindowClose         },
											 { kEventClassCommand, kEventCommandProcess      },
											 { kEventClassCommand, kEventCommandUpdateStatus } },
								tEvent[] = { { kEventClassControl, kEventControlHit          } };
			ControlActionUPP	actionUPP;
			HIViewRef			ctl, root;
			HIViewID			cid;

			root = HIViewGetRoot(tWindowRef);

			wUPP = NewEventHandlerUPP(AutofireWindowEventHandler);
			err = InstallWindowEventHandler(tWindowRef, wUPP, GetEventTypeCount(wEvent), wEvent, (void *) tWindowRef, &wRef);

			cid.signature = 'Ftab';
			cid.id = 256;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, autofireLastTabIndex);
			AutofireSelectTabPane(ctl, autofireLastTabIndex);
			tUPP = NewEventHandlerUPP(AutofireTabEventHandler);
			err = InstallControlEventHandler(ctl, tUPP, GetEventTypeCount(tEvent), tEvent, 0, &tRef);

			actionUPP = NewControlActionUPP(AutofireSliderActionProc);

			for (int player = 0; player < 2; player++)
			{
				AutofireSetAllIconImages(player + 1, root);
				AutofireReadAllSettings(player + 1, root);

				cid.id = player + 1;
				cid.signature = 'Slid';
				HIViewFindByID(root, cid, &ctl);
				SetControlAction(ctl, actionUPP);
			}

			MoveWindowPosition(tWindowRef, kWindowAutoFire, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowAutoFire);

			for (int player = 0; player < 2; player++)
				AutofireWriteAllSettings(player + 1, root);

			autofire = (autofireRec[0].buttonMask || autofireRec[1].buttonMask) ? true : false;

			err = RemoveEventHandler(tRef);
			DisposeEventHandlerUPP(tUPP);

			err = RemoveEventHandler(wRef);
			DisposeEventHandlerUPP(wUPP);

			DisposeControlActionUPP(actionUPP);

			CFRelease(tWindowRef);
		}

		DisposeNibReference(nibRef);
	}
}

static void AutofireSetAllIconImages (int player, HIViewRef parent)
{
	AutofireSetIconImages(player * 1,    parent);
	AutofireSetIconImages(player * 11,   parent);
	AutofireSetIconImages(player * 111,  parent);
	AutofireSetIconImages(player * 1111, parent);
}

static void AutofireSetIconImages (int sig, HIViewRef parent)
{
	OSStatus					err;
	ControlButtonContentInfo	info;
	HIViewRef					ctl;
	HIViewID					cid;
	int							ofs;

	cid.id = sig;
	ofs = macPadIconIndex + ((sig % 2) ? 0 : 12);

	if (systemVersion >= 0x1040)
	{
		info.contentType = kControlContentCGImageRef;

		cid.signature = 'AChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[7 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'BChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[5 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'XChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[6 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'YChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[4 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'LChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[8 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'RChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[9 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Up  ';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[0 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Down';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[1 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Left';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[2 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Righ';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[3 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Star';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[10 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Sele';
		HIViewFindByID(parent, cid, &ctl);
		info.u.imageRef = macIconImage[11 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);
	}
#ifdef MAC_PANTHER_SUPPORT
	else
	{
		info.contentType = kControlContentIconRef;

		cid.signature = 'AChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[7 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'BChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[5 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'XChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[6 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'YChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[4 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'LChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[8 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'RChk';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[9 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Up  ';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[0 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Down';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[1 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Left';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[2 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Righ';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[3 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Star';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[10 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);

		cid.signature = 'Sele';
		HIViewFindByID(parent, cid, &ctl);
		info.u.iconRef = macIconRef[11 + ofs];
		err = SetBevelButtonContentInfo(ctl, &info);
	}
#endif
}

static void AutofireReadAllSettings (int player, HIViewRef parent)
{
	HIViewRef	ctl;
	HIViewID	cid;
	char		num[10];

	AutofireReadSetting(player * 1,    autofireRec[player - 1].buttonMask, parent);
	AutofireReadSetting(player * 11,   autofireRec[player - 1].toggleMask, parent);
	AutofireReadSetting(player * 111,  autofireRec[player - 1].tcMask,     parent);
	AutofireReadSetting(player * 1111, autofireRec[player - 1].invertMask, parent);

	cid.id = player;

	cid.signature = 'Num_';
	HIViewFindByID(parent, cid, &ctl);
	sprintf(num, "%ld", autofireRec[player - 1].frequency);
	SetStaticTextCStr(ctl, num, false);

	cid.signature = 'Slid';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, autofireRec[player - 1].frequency);
}

static void AutofireReadSetting (int sig, uint16 target, HIViewRef parent)
{
	HIViewRef	ctl;
	HIViewID	cid;

	cid.id = sig;

	cid.signature = 'AChk';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0080) ? 1 : 0);

	cid.signature = 'BChk';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x8000) ? 1 : 0);

	cid.signature = 'XChk';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0040) ? 1 : 0);

	cid.signature = 'YChk';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x4000) ? 1 : 0);

	cid.signature = 'LChk';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0020) ? 1 : 0);

	cid.signature = 'RChk';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0010) ? 1 : 0);

	cid.signature = 'Up  ';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0800) ? 1 : 0);

	cid.signature = 'Down';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0400) ? 1 : 0);

	cid.signature = 'Left';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0200) ? 1 : 0);

	cid.signature = 'Righ';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x0100) ? 1 : 0);

	cid.signature = 'Star';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x1000) ? 1 : 0);

	cid.signature = 'Sele';
	HIViewFindByID(parent, cid, &ctl);
	SetControl32BitValue(ctl, (target & 0x2000) ? 1 : 0);
}

static void AutofireWriteAllSettings (int player, HIViewRef parent)
{
	HIViewRef	ctl;
	HIViewID	cid;

	AutofireWriteSetting(player * 1,    &(autofireRec[player - 1].buttonMask), parent);
	AutofireWriteSetting(player * 11,   &(autofireRec[player - 1].toggleMask), parent);
	AutofireWriteSetting(player * 111,  &(autofireRec[player - 1].tcMask),     parent);
	AutofireWriteSetting(player * 1111, &(autofireRec[player - 1].invertMask), parent);

	cid.id = player;
	cid.signature = 'Slid';
	HIViewFindByID(parent, cid, &ctl);
	autofireRec[player - 1].frequency = GetControl32BitValue(ctl);
}

static void AutofireWriteSetting (int sig, uint16 *target, HIViewRef parent)
{
	HIViewRef	ctl;
	HIViewID	cid;

	cid.id = sig;
	*target = 0x0000;

	cid.signature = 'AChk';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0080;

	cid.signature = 'BChk';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x8000;

	cid.signature = 'XChk';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0040;

	cid.signature = 'YChk';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x4000;

	cid.signature = 'LChk';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0020;

	cid.signature = 'RChk';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0010;

	cid.signature = 'Up  ';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0800;

	cid.signature = 'Down';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0400;

	cid.signature = 'Left';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0200;

	cid.signature = 'Righ';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x0100;

	cid.signature = 'Star';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x1000;

	cid.signature = 'Sele';
	HIViewFindByID(parent, cid, &ctl);
	if (GetControl32BitValue(ctl))
		(*target) |= 0x2000;
}

static void AutofireSelectTabPane (HIViewRef tabControl, SInt16 index)
{
	HIViewRef	sup, userPane, selectedPane = NULL;
	HIViewID	cid;

	autofireLastTabIndex = index;

	sup = HIViewGetSuperview(tabControl);
	cid.signature = 'Ftab';

	for (int i = 1; i < tabList[0] + 1; i++)
	{
		cid.id = tabList[i];
		HIViewFindByID(sup, cid, &userPane);

		if (i == index)
			selectedPane = userPane;
		else
			HIViewSetVisible(userPane, false);
	}

	if (selectedPane != NULL)
		HIViewSetVisible(selectedPane, true);

	HIViewSetNeedsDisplay(tabControl, true);
}

static pascal OSStatus AutofireTabEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	HIViewRef	ctl;
	HIViewID	cid;
	SInt32		value;

	err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ctl);
	if (err == noErr)
	{
		GetControlID(ctl, &cid);
		value = GetControl32BitValue(ctl);

		if ((cid.id == 256) && (value != autofireLastTabIndex))
		{
			AutofireSelectTabPane(ctl, value);
			result = noErr;
		}
	}

	return (result);
}

static pascal OSStatus AutofireWindowEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef = (WindowRef) inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
					QuitAppModalLoopForWindow(tWindowRef);
					result = noErr;
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
					HIViewRef	root;
					int			player = -1;

					root = HIViewGetRoot(tWindowRef);

					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						switch (tHICommand.commandID)
						{
							case 'DEF1':
								player = 0;
								break;

							case 'DEF2':
								player = 1;
								break;
						}

						if (player != -1)
						{
							autofireRec[player].buttonMask = 0x0000;
							autofireRec[player].toggleMask = 0xFFF0;
							autofireRec[player].tcMask     = 0x0000;
							autofireRec[player].invertMask = 0x0000;
							autofireRec[player].frequency  = 10;
							AutofireReadAllSettings(player + 1, root);

							result = noErr;
						}
					}
			}
	}

	return (result);
}

static pascal void AutofireSliderActionProc (HIViewRef slider, HIViewPartCode partCode)
{
	HIViewRef	ctl;
	HIViewID	cid;
	char		num[10];

	GetControlID(slider, &cid);
	cid.signature = 'Num_';
	HIViewFindByID(HIViewGetSuperview(slider), cid, &ctl);

	sprintf(num, "%ld", GetControl32BitValue(slider));
	SetStaticTextCStr(ctl, num, true);
}

void RomInfoDialog (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	if (!cartOpen)
		return;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;

		err = CreateWindowFromNib(nibRef, CFSTR("RomInfo"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerRef	eref;
			EventHandlerUPP	eventUPP;
			EventTypeSpec	windowEvents[] = { { kEventClassWindow,  kEventWindowClose         },
											   { kEventClassCommand, kEventCommandProcess      },
											   { kEventClassCommand, kEventCommandUpdateStatus } };
			CFStringRef		sref;
			HIViewRef		ctl, root;
			HIViewID		cid;
			char			text[256];

			eventUPP = NewEventHandlerUPP(RomInfoEventHandler);
			err = InstallWindowEventHandler(tWindowRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) tWindowRef, &eref);

			root = HIViewGetRoot(tWindowRef);
			cid.id = 0;

			cid.signature = 'Name';	// Cart Name
			HIViewFindByID(root, cid, &ctl);

			strcpy(text, Memory.RawROMName);
			sref = CopyFixNameStrings(text, Memory.ROMRegion);
			if (!sref)
				SetStaticTextCFString(ctl, CFSTR("unknown"), false);
			else
			{
				SetStaticTextCFString(ctl, sref, false);
				CFRelease(sref);
			}

			cid.signature = 'Code';	// Game Code
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.ROMId);
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Cont';	// Contents
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.KartContents());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Map ';	// ROM Map
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.MapType());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Spee';	// ROM Speed
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "0x%02X (%s)", Memory.ROMSpeed, ((Memory.ROMSpeed & 0x10) != 0) ? "FastROM" : "SlowROM");
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Type';	// ROM Type
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "0x%02X", Memory.ROMType);
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'SizC';	// Actual ROM Size
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%dMbits", Memory.CalculatedSize / 0x20000);
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'SizH';	// ROM Size written in info block
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.Size());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'SRAM';	// SRAM Size
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.StaticRAMSize());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'SumC';	// Actual checksum
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "0x%04X", Memory.CalculatedChecksum);
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'SumH';	// Checksum written in info block
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "0x%04X", Memory.ROMChecksum);
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'ComH';	// Checksum complement written in info block : SumH + ComH = 0xFFFF
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "0x%04X", Memory.ROMComplementChecksum);
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Outp';	// Video output (NTSC or PAL)
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", (Memory.ROMRegion > 12 || Memory.ROMRegion < 2) ? "NTSC 60Hz" : "PAL 50Hz");
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Vers';	// Revision
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.Revision());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Lice';	// Licensee
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.PublishingCompany());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'Regi';	// Region
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "%s", Memory.Country());
			SetStaticTextCStr(ctl, text, false);

			cid.signature = 'CRC ';	// CRC32
			HIViewFindByID(root, cid, &ctl);
			sprintf(text, "0x%08X", Memory.ROMCRC32);
			SetStaticTextCStr(ctl, text, false);

			MoveWindowPosition(tWindowRef, kWindowRomInfo, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowRomInfo);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eventUPP);

			CFRelease(tWindowRef);
		}

		DisposeNibReference(nibRef);
	}
}

static void RomInfoCopyToClipboard (void)
{
	OSStatus			err;
	PasteboardRef		clipboard;
	PasteboardSyncFlags	sync;
	CFDataRef			cfdata;
	char				text[1024];

	RomInfoBuildInfoText(text);

	err = PasteboardCreate(kPasteboardClipboard, &clipboard);
	if (err == noErr)
	{
		err = PasteboardClear(clipboard);
		if (err == noErr)
		{
			sync = PasteboardSynchronize(clipboard);
			if (!(sync & kPasteboardModified) && (sync & kPasteboardClientIsOwner))
			{
				cfdata = CFDataCreate(kCFAllocatorDefault, (UInt8 *) text, (CFIndex) strlen(text));
				if (cfdata)
				{
					err = PasteboardPutItemFlavor(clipboard, (PasteboardItemID) 1, CFSTR("com.apple.traditional-mac-plain-text"), cfdata, 0);
					CFRelease(cfdata);
				}
			}
		}

		CFRelease(clipboard);
	}
}

static void RomInfoBuildInfoText (char *romtext)
{
	char	s1[256], s2[1024];

	sprintf(s1, "Snes9x version: %s\nMac port version: %s, ", VERSION, MAC_VERSION);
#ifdef __BIG_ENDIAN__
	strcat(s1, "PowerPC\n\n");
#else
	strcat(s1, "Intel\n\n");
#endif
	Memory.MakeRomInfoText(s2);
	sprintf(romtext, "%s%s", s1, s2);
}

static pascal OSStatus RomInfoEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef = (WindowRef) inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
					QuitAppModalLoopForWindow(tWindowRef);
					result = noErr;
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
							case 'Clip':
								RomInfoCopyToClipboard();
								result = noErr;
						}
					}
			}
	}

	return (result);
}

void RegisterHelpBook (void)
{
	OSStatus	err;
	CFBundleRef	bundleRef;
	CFURLRef	bundleURL;
	FSRef		fref;

	bundleRef = CFBundleGetMainBundle();
	if (bundleRef)
	{
		bundleURL = CFBundleCopyBundleURL(bundleRef);
		if (bundleURL)
		{
			if (CFURLGetFSRef(bundleURL, &fref))
				err = AHRegisterHelpBook(&fref);

			CFRelease(bundleURL);
		}
	}
}

void SetHIViewID (HIViewID *cid, OSType signature, SInt32 value)
{
	// Since HIViewID.id conflicts Objective-C 'id'...

	cid->signature = signature;
	cid->id = value;
}
