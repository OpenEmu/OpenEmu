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

#include "mac-prefix.h"
#include "mac-cart.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-multicart.h"

static pascal OSStatus MultiCartEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus MultiCartPaneEventHandler (EventHandlerCallRef, EventRef, void *);

static int		multiCartDragHilite;
static Boolean	multiCartDialogResult;


void InitMultiCart (void)
{
	CFStringRef	keyRef, pathRef;
	char		key[32];

	multiCartPath[0] = multiCartPath[1] = NULL;

	for (int i = 0; i < 2; i++)
	{
		sprintf(key, "MultiCartPath_%02d", i);
		keyRef = CFStringCreateWithCString(kCFAllocatorDefault, key, CFStringGetSystemEncoding());
		if (keyRef)
		{
			pathRef = (CFStringRef) CFPreferencesCopyAppValue(keyRef, kCFPreferencesCurrentApplication);
			if (pathRef)
				multiCartPath[i] = pathRef;

			CFRelease(keyRef);
		}
	}
}

void DeinitMultiCart (void)
{
	CFStringRef	keyRef;
	char		key[32];

	for (int i = 0; i < 2; i++)
	{
		sprintf(key, "MultiCartPath_%02d", i);
		keyRef = CFStringCreateWithCString(kCFAllocatorDefault, key, CFStringGetSystemEncoding());
		if (keyRef)
		{
			if (multiCartPath[i])
			{
				CFPreferencesSetAppValue(keyRef, multiCartPath[i], kCFPreferencesCurrentApplication);
				CFRelease(multiCartPath[i]);
			}
			else
				CFPreferencesSetAppValue(keyRef, NULL, kCFPreferencesCurrentApplication);

			CFRelease(keyRef);
		}
	}

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

Boolean MultiCartDialog (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	multiCartDragHilite = -1;
	multiCartDialogResult = false;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	window;

		err = CreateWindowFromNib(nibRef, CFSTR("MultiCart"), &window);
		if (err == noErr)
		{
			static int	index[2] = { 0, 1 };

			EventHandlerRef	wRef, cRef[2];
			EventHandlerUPP	wUPP, cUPP[2];
			EventTypeSpec	wEvent[] = { { kEventClassCommand, kEventCommandProcess      },
										 { kEventClassCommand, kEventCommandUpdateStatus } },
							cEvent[] = { { kEventClassControl, kEventControlDraw         },
										 { kEventClassControl, kEventControlDragEnter    },
										 { kEventClassControl, kEventControlDragWithin   },
										 { kEventClassControl, kEventControlDragLeave    },
										 { kEventClassControl, kEventControlDragReceive  } };
			HIViewRef		ctl, root, pane[2];
			HIViewID		cid;

			root = HIViewGetRoot(window);

			wUPP = NewEventHandlerUPP(MultiCartEventHandler);
			err = InstallWindowEventHandler(window, wUPP, GetEventTypeCount(wEvent), wEvent, (void *) window, &wRef);
			err = SetAutomaticControlDragTrackingEnabledForWindow(window, true);

			for (int i = 0; i < 2; i++)
			{
				cid.id = i;

				cid.signature = 'MPan';
				HIViewFindByID(root, cid, &pane[i]);
				cUPP[i] = NewEventHandlerUPP(MultiCartPaneEventHandler);
				err = InstallControlEventHandler(pane[i], cUPP[i], GetEventTypeCount(cEvent), cEvent, (void *) &index[i], &cRef[i]);
				err = SetControlDragTrackingEnabled(pane[i], true);

				cid.signature = 'MNAM';
				HIViewFindByID(root, cid, &ctl);
				SetStaticTextTrunc(ctl, truncEnd, false);
				if (multiCartPath[i])
				{
					CFStringRef	str;
					CFURLRef	url;

					url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, multiCartPath[i], kCFURLPOSIXPathStyle, false);
					str = CFURLCopyLastPathComponent(url);
					SetStaticTextCFString(ctl, str, false);
					CFRelease(str);
					CFRelease(url);
				}
				else
					SetStaticTextCFString(ctl, CFSTR(""), false);
			}

			MoveWindowPosition(window, kWindowMultiCart, false);
			ShowWindow(window);
			err = RunAppModalLoopForWindow(window);
			HideWindow(window);
			SaveWindowPosition(window, kWindowMultiCart);

			for (int i = 0; i < 2; i++)
			{
				err = RemoveEventHandler(cRef[i]);
				DisposeEventHandlerUPP(cUPP[i]);
			}

			err = RemoveEventHandler(wRef);
			DisposeEventHandlerUPP(wUPP);

			CFRelease(window);
		}

		DisposeNibReference(nibRef);
	}

	return (multiCartDialogResult);
}

static pascal OSStatus MultiCartEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	window = (WindowRef) inUserData;
	static int	index = -1;

	switch (GetEventClass(inEvent))
	{
		case kEventClassCommand:
		{
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(false);
						result = noErr;
					}

					break;
				}

				case kEventCommandProcess:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						HIViewRef	ctl, root;
						HIViewID	cid;
						FSRef		ref;
						bool8		r;

						root = HIViewGetRoot(window);

						switch (tHICommand.commandID)
						{
							case 'Cho0':
							case 'Cho1':
							{
								index = (tHICommand.commandID & 0xFF) - '0';
								r = NavBeginOpenROMImageSheet(window, NULL);
								result = noErr;
								break;
							}

							case 'NvDn':
							{
								r = NavEndOpenROMImageSheet(&ref);
								if (r)
								{
									CFStringRef	str;
									CFURLRef	url;

									url = CFURLCreateFromFSRef(kCFAllocatorDefault, &ref);
									str = CFURLCopyLastPathComponent(url);
									cid.signature = 'MNAM';
									cid.id = index;
									HIViewFindByID(root, cid, &ctl);
									SetStaticTextCFString(ctl, str, true);
									CFRelease(str);
									str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
									if (multiCartPath[index])
										CFRelease(multiCartPath[index]);
									multiCartPath[index] = str;
									CFRelease(url);
								}

								index = -1;
								result = noErr;
								break;
							}

							case 'Cle0':
							case 'Cle1':
							{
								index = (tHICommand.commandID & 0xFF) - '0';
								cid.signature = 'MNAM';
								cid.id = index;
								HIViewFindByID(root, cid, &ctl);
								SetStaticTextCFString(ctl, CFSTR(""), true);
								if (multiCartPath[index])
								{
									CFRelease(multiCartPath[index]);
									multiCartPath[index] = NULL;
								}

								index = -1;
								result = noErr;
								break;
							}

							case 'SWAP':
							{
								CFStringRef	str;
								CFURLRef	url;

								str = multiCartPath[0];
								multiCartPath[0] = multiCartPath[1];
								multiCartPath[1] = str;

								cid.signature = 'MNAM';

								for (int i = 0; i < 2; i++)
								{
									cid.id = i;
									HIViewFindByID(root, cid, &ctl);

									if (multiCartPath[i])
									{
										url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, multiCartPath[i], kCFURLPOSIXPathStyle, false);
										str = CFURLCopyLastPathComponent(url);
										SetStaticTextCFString(ctl, str, true);
										CFRelease(str);
										CFRelease(url);
									}
									else
										SetStaticTextCFString(ctl, CFSTR(""), true);
								}

								result = noErr;
								break;
							}

							case 'ok  ':
							{
								QuitAppModalLoopForWindow(window);
								multiCartDialogResult = true;
								result = noErr;
								break;
							}

							case 'not!':
							{
								QuitAppModalLoopForWindow(window);
								multiCartDialogResult = false;
								result = noErr;
								break;
							}
						}
					}
				}
			}
		}
	}

	return (result);
}

static pascal OSStatus MultiCartPaneEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus			err, result = eventNotHandledErr;
	HIViewRef			view;
	DragRef				drag;
	PasteboardRef		pasteboard;
	PasteboardItemID	itemID;
	CFArrayRef			array;
	CFStringRef			flavorType;
	CFIndex				numFlavors;
	ItemCount			numItems;
	int					index = *((int *) inUserData);

	switch (GetEventClass(inEvent))
	{
		case kEventClassControl:
		{
			switch (GetEventKind(inEvent))
			{
				case kEventControlDraw:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						CGContextRef	ctx;

						err = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx);
						if (err == noErr)
						{
							HIThemeFrameDrawInfo	info;
							HIRect					bounds, frame;

							HIViewGetBounds(view, &bounds);

							CGContextSetRGBFillColor(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
							CGContextFillRect(ctx, bounds);

							info.version   = 0;
							info.kind      = kHIThemeFrameTextFieldSquare;
							info.state     = kThemeStateInactive;
							info.isFocused = false;
							err = HIThemeDrawFrame(&bounds, &info, ctx, kHIThemeOrientationNormal);

							if (multiCartDragHilite == index && systemVersion >= 0x1040)
							{
								err = HIThemeSetStroke(kThemeBrushDragHilite, NULL, ctx, kHIThemeOrientationNormal);
								frame = CGRectInset(bounds, 1, 1);
								CGContextBeginPath(ctx);
								CGContextAddRect(ctx, frame);
								CGContextStrokePath(ctx);
							}
						}
					}

					result = noErr;
					break;
				}

				case kEventControlDragEnter:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						err = GetEventParameter(inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof(DragRef), NULL, &drag);
						if (err == noErr)
						{
							err = GetDragPasteboard(drag, &pasteboard);
							if (err == noErr)
							{
								err = PasteboardGetItemCount(pasteboard, &numItems);
								if (err == noErr && numItems == 1)
								{
									err = PasteboardGetItemIdentifier(pasteboard, 1, &itemID);
									if (err == noErr)
									{
										err = PasteboardCopyItemFlavors(pasteboard, itemID, &array);
										if (err == noErr)
										{
											numFlavors = CFArrayGetCount(array);
											for (CFIndex i = 0; i < numFlavors; i++)
											{
												flavorType = (CFStringRef) CFArrayGetValueAtIndex(array, i);
												if (UTTypeConformsTo(flavorType, CFSTR("public.file-url")))
												{
													Boolean	accept = true;

													err = SetEventParameter(inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof(Boolean), &accept);
													if (err == noErr)
													{
														multiCartDragHilite = index;
														HIViewSetNeedsDisplay(view, true);
														result = noErr;
													}
												}
											}

											CFRelease(array);
										}
									}
								}
							}
						}
					}

					break;
				}

				case kEventControlDragWithin:
				{
					result = noErr;
					break;
				}

				case kEventControlDragLeave:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						multiCartDragHilite = -1;
						HIViewSetNeedsDisplay(view, true);
					}

					result = noErr;
					break;
				}

				case kEventControlDragReceive:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						err = GetEventParameter(inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof(DragRef), NULL, &drag);
						if (err == noErr)
						{
							multiCartDragHilite = -1;
							HIViewSetNeedsDisplay(view, true);

							err = GetDragPasteboard(drag, &pasteboard);
							if (err == noErr)
							{
								err = PasteboardGetItemIdentifier(pasteboard, 1, &itemID);
								if (err == noErr)
								{
									err = PasteboardCopyItemFlavors(pasteboard, itemID, &array);
									if (err == noErr)
									{
										numFlavors = CFArrayGetCount(array);
										for (CFIndex i = 0; i < numFlavors; i++)
										{
											flavorType = (CFStringRef) CFArrayGetValueAtIndex(array, i);
											if (UTTypeConformsTo(flavorType, CFSTR("public.file-url")))
											{
												CFDataRef	flavorData;

												err = PasteboardCopyItemFlavorData(pasteboard, itemID, flavorType, &flavorData);
												if (err == noErr)
												{
													CFIndex	dataSize;
													UInt8	*data;

													dataSize = CFDataGetLength(flavorData);
													data = (UInt8 *) malloc(dataSize);
													if (data)
													{
														CFDataGetBytes(flavorData, CFRangeMake(0, dataSize), data);

														HIViewRef	ctl;
														HIViewID	cid;
														CFStringRef	str;
														CFURLRef	url;

														GetControlID(view, &cid);
														cid.signature = 'MNAM';
														HIViewFindByID(view, cid, &ctl);

														url = CFURLCreateWithBytes(kCFAllocatorDefault, data, dataSize, kCFStringEncodingUTF8, NULL);
														str = CFURLCopyLastPathComponent(url);
														SetStaticTextCFString(ctl, str, true);
														CFRelease(str);
														str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
														if (multiCartPath[cid.id])
															CFRelease(multiCartPath[cid.id]);
														multiCartPath[cid.id] = str;
														CFRelease(url);

														result = noErr;

														free(data);
													}

													CFRelease(flavorData);
												}
											}
										}

										CFRelease(array);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return (result);
}
