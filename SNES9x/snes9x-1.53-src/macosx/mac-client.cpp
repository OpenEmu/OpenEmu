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
#include "apu.h"
#include "snapshot.h"
#include "cheats.h"
#include "display.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#include "mac-prefix.h"
#include "mac-cart.h"
#include "mac-cheatfinder.h"
#include "mac-controls.h"
#include "mac-dialog.h"
#include "mac-file.h"
#include "mac-joypad.h"
#include "mac-keyboard.h"
#include "mac-os.h"
#include "mac-snes9x.h"
#include "mac-stringtools.h"
#include "mac-netplay.h"
#include "mac-client.h"

#ifdef SELF_TEST
#include <sys/un.h>
#endif

#define KeyIsPressed(km, k)	(1 & (((unsigned char *) km) [(k) >> 3] >> ((k) & 7)))

enum
{
	kNPCDialogNone,
	kNPCDialogInit,
	kNPCDialogConnect,
	kNPCDialogConnectFailed,
	kNPCDialogOpenBegin,
	kNPCDialogOpenEnd,
	kNPCDialogPrepare,
	kNPCDialogPrepareFailed,
	kNPCDialogShowList,
	kNPCDialogDone,
	kNPCDialogCancel
};

typedef struct
{
	volatile bool8	padloop;
	volatile bool8	exitsgn;
	volatile uint32 phasecount;
	volatile uint32 phasespan;
	volatile uint8	header;
	bool8			online;
	int				socket;
	int				numplayers;
	char			name[256];
	char			serverIP[256];

	int				savedDeviceSetting;
	int				savedAutoSaveDelay;

	bool8			configsaved;
	bool8			dialogcancel;
	bool8			dialogsheet;
	int				dialogprocess;
}	clientState;

typedef struct
{
	bool8			ready;
	int				player;
	char			name[256];
}	clientsInfo;

typedef struct
{
	uint32			crc32;
	int				input;
	int				length;
	char			fname[PATH_MAX + 1];
}	cROMInfo;

static char			n_csememu[] = "/tmp/s9x_c_emu_semaphore",
					n_csempad[] = "/tmp/s9x_c_pad_semaphore";

static clientState	npclient;
static clientsInfo	npcinfo[NP_MAX_PLAYERS];

static cROMInfo		nprominfo;

static uint32		npcactvpad[NP_MAX_PLAYERS][64],	// [player number]
					npcrecvpad[NP_MAX_PLAYERS][64],	// [player number]
					npcsendpad[64],
					npccachpad[64];

static WindowRef	mRef, sRef;
static sem_t		*csememu, *csempad;
static pthread_t	connectthread, preparethread, gamepadthread;

static int NPClientGetMesFromServer (void);
static void NPClientDetachConnectThread (void);
static void NPClientDetachPrepareThread (void);
static void NPClientBeginPlayerListSheet (void);
static void NPClientEndPlayerListSheet (void);
static bool8 NPClientConnectToServer (int);
static bool8 NPClientSendMesToServer (int);
static bool8 NPClientSendNameToServer (void);
static bool8 NPClientGetROMInfoFromServer (void);
static bool8 NPClientBeginOpenROMImage (WindowRef);
static bool8 NPClientEndOpenROMImage (void);
static bool8 NPClientROMReadyToServer (void);
static bool8 NPClientGetSRAMFromServer (void);
static bool8 NPClientGetPlayerListFromServer (void);
static bool8 NPClientReplyPhaseSpanTest (void);
static void * NPClientConnectThread (void *);
static void * NPClientPrepareThread (void *);
static void * NPClientNetPlayThread (void *);
static pascal void NPClientDialogTimerHandler (EventLoopTimerRef, void *);
static pascal OSStatus NPClientDialogEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus NPClientSheetEventHandler (EventHandlerCallRef, EventRef, void *);


bool8 NPClientDialog (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	npclient.dialogcancel = true;
	npclient.dialogsheet  = false;
	npclient.configsaved  = false;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		err = CreateWindowFromNib(nibRef, CFSTR("Connect"), &mRef);
		if (err == noErr)
		{
			err = CreateWindowFromNib(nibRef, CFSTR("PlayerList"), &sRef);
			if (err == noErr)
			{
				EventHandlerRef		eref, seref;
				EventLoopTimerRef	tref;
				EventHandlerUPP		eventUPP, sheetUPP;
				EventLoopTimerUPP	timerUPP;
				EventTypeSpec		windowEvents[] = { { kEventClassCommand, kEventCommandProcess      },
													   { kEventClassCommand, kEventCommandUpdateStatus } };
				CFStringRef			ref;
				HIViewRef			ctl, root;
				HIViewID			cid;

				npclient.dialogprocess = kNPCDialogInit;

				eventUPP = NewEventHandlerUPP(NPClientDialogEventHandler);
				err = InstallWindowEventHandler(mRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) mRef, &eref);

				timerUPP = NewEventLoopTimerUPP(NPClientDialogTimerHandler);
				err = InstallEventLoopTimer(GetCurrentEventLoop(), 0.0f, 0.1f, timerUPP, (void *) mRef, &tref);

				sheetUPP = NewEventHandlerUPP(NPClientSheetEventHandler);
				err = InstallWindowEventHandler(sRef, sheetUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) sRef, &seref);

				root = HIViewGetRoot(mRef);
				cid.id = 0;

				cid.signature = 'CHAS';
				HIViewFindByID(root, cid, &ctl);
				HIViewSetVisible(ctl, false);

				cid.signature = 'SVIP';
				HIViewFindByID(root, cid, &ctl);
				SetEditTextCStr(ctl, npServerIP, false);

				cid.signature = 'CLNM';
				HIViewFindByID(root, cid, &ctl);
				ref = CFStringCreateWithCString(kCFAllocatorDefault, npName, kCFStringEncodingUTF8);
				if (ref)
				{
					SetEditTextCFString(ctl, ref, false);
					CFRelease(ref);
				}
				else
					SetEditTextCFString(ctl, CFSTR("unknown"), false);

				MoveWindowPosition(mRef, kWindowClient, false);
				ShowWindow(mRef);
				err = HIViewAdvanceFocus(root, 0);
				err = RunAppModalLoopForWindow(mRef);
				HideWindow(mRef);
				SaveWindowPosition(mRef, kWindowClient);

				err = RemoveEventHandler(seref);
				DisposeEventHandlerUPP(sheetUPP);

				err = RemoveEventLoopTimer(tref);
				DisposeEventLoopTimerUPP(timerUPP);

				err = RemoveEventHandler(eref);
				DisposeEventHandlerUPP(eventUPP);

				CFRelease(sRef);
			}

			CFRelease(mRef);
		}

		DisposeNibReference(nibRef);
	}

	return (!npclient.dialogcancel);
}

static pascal OSStatus NPClientDialogEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;

	switch (GetEventClass(inEvent))
	{
		case kEventClassCommand:
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(false);
						result = noErr;
					}

					break;

				case kEventCommandProcess:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						switch (tHICommand.commandID)
						{
							case 'OK__':
								CFStringRef	ref;
								HIViewRef	ctl, root;
								HIViewID	cid;

								root = HIViewGetRoot(mRef);
								cid.id = 0;

								cid.signature = 'SVIP';
								HIViewFindByID(root, cid, &ctl);
								GetEditTextCStr(ctl, npclient.serverIP);
								DeactivateControl(ctl);
								if (npclient.serverIP[0] == 0)
									strcpy(npclient.serverIP, "127.0.0.1");
								strcpy(npServerIP, npclient.serverIP);
								printf("%s\n", npServerIP);

								cid.signature = 'CLNM';
								HIViewFindByID(root, cid, &ctl);
								CopyEditTextCFString(ctl, &ref);
								DeactivateControl(ctl);
								if (ref)
								{
									Boolean	r;

									r = CFStringGetCString(ref, npclient.name, 256, kCFStringEncodingUTF8);
									if (!r)
										strcpy(npclient.name, "unknown");
									else
									if (npclient.name[0] == 0)
										strcpy(npclient.name, "Guest");

									CFRelease(ref);
								}
								else
									strcpy(npclient.name, "unknown");
								strcpy(npName, npclient.name);
								printf("%s\n", npName);

								cid.signature = 'OK__';
								HIViewFindByID(root, cid, &ctl);
								DeactivateControl(ctl);

								cid.signature = 'NOT_';
								HIViewFindByID(root, cid, &ctl);
								DeactivateControl(ctl);

								npclient.dialogcancel  = false;
								npclient.dialogprocess = kNPCDialogConnect;

								result = noErr;
								break;

							case 'NOT_':
								npclient.dialogcancel  = true;
								npclient.dialogprocess = kNPCDialogCancel;

								result = noErr;
								break;

							case 'NvDn':
								npclient.dialogcancel  = false;
								npclient.dialogprocess = kNPCDialogOpenEnd;

								result = noErr;
								break;
						}
					}

					break;
			}

			break;
	}

	return (result);
}

static pascal void NPClientDialogTimerHandler (EventLoopTimerRef inTimer, void *userData)
{
	WindowRef	window = (WindowRef) userData;
	HIViewRef	ctl;
	HIViewID	cid = { 'CHAS', 0 };

	HIViewFindByID(HIViewGetRoot(mRef), cid, &ctl);

	switch (npclient.dialogprocess)
	{
		case kNPCDialogNone:
			break;

		case kNPCDialogCancel:
			NPNotification("  kNPCDialogCancel", -1);
			npclient.dialogprocess = kNPCDialogNone;
			npclient.dialogcancel  = true;
			QuitAppModalLoopForWindow(mRef);
			break;

		case kNPCDialogInit:
			NPNotification("  kNPCDialogInit", -1);
			npclient.dialogprocess = kNPCDialogNone;
			break;

		case kNPCDialogConnect:
			NPNotification("  kNPCDialogConnect", -1);
			npclient.dialogprocess = kNPCDialogNone;
			HIViewSetVisible(ctl, true);
			NPClientDetachConnectThread();
			break;

		case kNPCDialogConnectFailed:
			NPNotification("  kNPCDialogConnectFailed", -1);
			npclient.dialogprocess = kNPCDialogNone;
			npclient.dialogcancel  = true;
			QuitAppModalLoopForWindow(mRef);
			break;

		case kNPCDialogOpenBegin:
			NPNotification("  kNPCDialogOpenBegin", -1);
			npclient.dialogprocess = kNPCDialogNone;
			HIViewSetVisible(ctl, false);
			NPClientStoreConfig();
			if (!NPClientBeginOpenROMImage(window))
			{
				NPClientDisconnect();
				NPClientRestoreConfig();
				npclient.dialogprocess = kNPCDialogCancel;
			}

			break;

		case kNPCDialogOpenEnd:
			NPNotification("  kNPCDialogOpenEnd", -1);
			npclient.dialogprocess = kNPCDialogNone;
			if (!NPClientEndOpenROMImage())
			{
				NPClientDisconnect();
				NPClientRestoreConfig();
				npclient.dialogprocess = kNPCDialogCancel;
			}
			else
				npclient.dialogprocess = kNPCDialogPrepare;

			break;

		case kNPCDialogPrepare:
			NPNotification("  kNPCDialogPrepare", -1);
			npclient.dialogprocess = kNPCDialogNone;
			HIViewSetVisible(ctl, true);
			NPClientDetachPrepareThread();
			break;

		case kNPCDialogPrepareFailed:
			NPNotification("  kNPCDialogPrepareFailed", -1);
			npclient.dialogprocess = kNPCDialogNone;
			NPClientRestoreConfig();
			npclient.dialogcancel = true;
			QuitAppModalLoopForWindow(mRef);
			break;

		case kNPCDialogShowList:
			NPNotification("  kNPCDialogShowList", -1);
			npclient.dialogprocess = kNPCDialogNone;
			HIViewSetVisible(ctl, false);
			npclient.dialogsheet = true;
			NPClientBeginPlayerListSheet();
			break;

		case kNPCDialogDone:
			NPNotification("  kNPCDialogDone", -1);
			npclient.dialogprocess = kNPCDialogNone;
			NPClientEndPlayerListSheet();
			npclient.dialogsheet  = false;
			npclient.dialogcancel = false;
			QuitAppModalLoopForWindow(mRef);
			break;
	}
}

static void NPClientDetachConnectThread (void)
{
	pthread_create(&connectthread, NULL, NPClientConnectThread, NULL);
	pthread_detach(connectthread);
}

static void * NPClientConnectThread (void *)
{
	NPNotification("Client: Entered connection thread.", -1);

	if ((NPClientConnectToServer(NP_PORT) == false) ||
		(NPClientSendNameToServer()       == false) ||
		(NPClientGetROMInfoFromServer()   == false))
	{
		NPClientDisconnect();
		npclient.dialogprocess = kNPCDialogConnectFailed;
		return (NULL);
	}

	npclient.dialogprocess = kNPCDialogOpenBegin;
	NPNotification("Client: Exited connection thread.", -1);
	return (NULL);
}

void NPClientInit (void)
{
	npclient.padloop     = false;
	npclient.exitsgn     = false;
	npclient.phasecount  = 0;
	npclient.phasespan   = 0;
	npclient.header      = 0;
	npclient.online      = false;
	npclient.socket      = -1;
	npclient.numplayers  = 0;
	npclient.name[0]     = 0;
	npclient.serverIP[0] = 0;

	nprominfo.crc32      = 0;
	nprominfo.input      = 0;
	nprominfo.length     = 0;
	nprominfo.fname[0]   = 0;

	for (int i = 0; i < NP_MAX_PLAYERS; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			npcactvpad[i][j] = 0;
			npcrecvpad[i][j] = 0;
		}
	}

	for (int j = 0; j < 64; j++)
	{
		npcsendpad[j] = 0;
		npccachpad[j] = 0;
	}

	for (int c = 0; c < NP_MAX_PLAYERS; c++)
	{
		npcinfo[c].ready   = false;
		npcinfo[c].player  = 0;
		npcinfo[c].name[0] = 0;
	}
}

static bool8 NPClientConnectToServer (int port)
{
#ifndef SELF_TEST
	struct sockaddr_in	address;
#else
	struct sockaddr_un	address;
#endif

	NPNotification("Client: Connecting to server...", -1);

	memset(&address, 0, sizeof(address));
#ifndef SELF_TEST
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = inet_addr(npclient.serverIP);
	address.sin_port        = htons(port);
#else
	address.sun_family      = AF_UNIX;
	strcpy(address.sun_path, SOCK_NAME);
#endif

#ifndef SELF_TEST
	if (address.sin_addr.s_addr == INADDR_NONE)
	{
		NPError("Client: Server IP is invalid.", 5001);
		return (false);
	}
#endif

#ifndef SELF_TEST
	if ((npclient.socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
#else
	if ((npclient.socket = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
#endif
	{
		NPError("Client: Failed to create socket.", 5002);
		return (false);
	}

	if (connect(npclient.socket, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		NPError("Client: Failed to connect to server.", 5003);
		return (false);
	}

	npclient.online = true;

	NPNotification("Client: Connected to server.", -1);
	return (true);
}

void NPClientDisconnect (void)
{
	if (npclient.socket != -1)
	{
		NPNotification("Client: Disconnecting from server...", -1);

		close(npclient.socket);
		npclient.socket = -1;

		NPNotification("Client: Disconnected from server.", -1);
	}

	npclient.online      = false;
	npclient.name[0]     = 0;
	npclient.serverIP[0] = 0;
}

static bool8 NPClientSendMesToServer (int num)
{
	uint8	mes[2];

	mes[0] = NP_CLIENT_MAGIC;
	mes[1] = num;

	if (socket_write(npclient.socket, mes, 2) != 2)
		return (false);

	return (true);
}

static int NPClientGetMesFromServer (void)
{
	uint8	mes[2];

	if (socket_read(npclient.socket, mes, 2) != 2)
		return (-1);

	if (mes[0] != NP_SERVER_MAGIC)
		return (-1);

	return ((int) mes[1]);
}

static bool8 NPClientSendNameToServer (void)
{
	if (!npclient.online)
		return (false);

	NPNotification("Client: Sending player name to server...", -1);

	if (NPClientGetMesFromServer() != kNPServerNameRequest)
	{
		NPError("Client: Failed to receive messsage from server.", 5101);
		return (false);
	}

	uint8	mes[4];
	uint32	l;

	l = strlen(npclient.name);
	WRITE_LONG(mes + 0, l);

	if (socket_write(npclient.socket, mes, 4) != 4)
	{
		NPError("Client: Failed to send name size to server.", 5102);
		return (false);
	}

	if (socket_write(npclient.socket, (uint8 *) npclient.name, l) != (int) l)
	{
		NPError("Client: Failed to send name to server.", 5103);
		return (false);
	}

	if (NPClientGetMesFromServer() != kNPServerNameReceived)
	{
		NPError("Client: Failed to receive messsage from server.", 5104);
		return (false);
	}

	if (NPClientSendMesToServer(kNPClientNameSent) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5105);
		return (false);
	}

	NPNotification("Client: Sent player name to server.", -1);
	return (true);
}

static bool8 NPClientGetROMInfoFromServer (void)
{
	if (!npclient.online)
		return (false);

	NPNotification("Client: Receiving ROM information from server...", -1);

	if (NPClientGetMesFromServer() != kNPServerROMInfoWillSend)
	{
		NPError("Client: Failed to receive messsage from server.", 5201);
		return (false);
	}

	if (NPClientSendMesToServer(kNPClientROMInfoWaiting) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5202);
		return (false);
	}

	uint8	mes[16];
	uint32	l;

	if (socket_read(npclient.socket, mes, 16) != 16)
	{
		NPError("Client: Failed to receive ROM information from server.", 5203);
		return (false);
	}

	nprominfo.crc32      = READ_LONG(mes + 0);
	nprominfo.input      = READ_LONG(mes + 4);

	l = READ_LONG(mes + 12);

	if (socket_read(npclient.socket, (uint8 *) nprominfo.fname, l) != (int) l)
	{
		NPError("Client: Failed to receive ROM name from server.", 5204);
		return (false);
	}

	nprominfo.fname[l] = 0;
	nprominfo.length   = l;

	NPNotification("Client: Received ROM information from server.", -1);
	return (true);
}

void NPClientStoreConfig (void)
{
	npclient.savedDeviceSetting = deviceSetting;
	npclient.savedAutoSaveDelay = Settings.AutoSaveDelay;

	npclient.configsaved = true;

	deviceSetting = nprominfo.input;
	Settings.AutoSaveDelay = 0;

	ChangeInputDevice();
}

void NPClientRestoreConfig (void)
{
	if (npclient.configsaved)
	{
		deviceSetting = npclient.savedDeviceSetting;
		Settings.AutoSaveDelay = npclient.savedAutoSaveDelay;

		npclient.configsaved = false;

		ChangeInputDevice();
	}
}

static bool8 NPClientBeginOpenROMImage (WindowRef window)
{
	CFStringRef			numRef, romRef, baseRef;
	CFMutableStringRef	mesRef;
	SInt32				replaceAt;
	bool8				r;

	DeinitGameWindow();

	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	cartOpen = false;

	ResetCheatFinder();

	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, nprominfo.fname, kCFStringEncodingUTF8);
	numRef  = CFCopyLocalizedString(CFSTR("NPROMNamePos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("NPROMNameMes"), "NPROM");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);

	r = NavBeginOpenROMImageSheet(window, mesRef);

	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	return (r);
}

static bool8 NPClientEndOpenROMImage (void)
{
	OSStatus	err;
	FSRef		cartRef;
	char		filename[PATH_MAX + 1];
	bool8		r;

	r = NavEndOpenROMImageSheet(&cartRef);
	if (!r)
	{
		cartOpen = false;
		return (false);
	}

	CheckSaveFolder(&cartRef);

	Settings.ForceLoROM          = (romDetect        == kLoROMForce       );
	Settings.ForceHiROM          = (romDetect        == kHiROMForce       );
	Settings.ForceHeader         = (headerDetect     == kHeaderForce      );
	Settings.ForceNoHeader       = (headerDetect     == kNoHeaderForce    );
	Settings.ForceInterleaved    = (interleaveDetect == kInterleaveForce  );
	Settings.ForceInterleaved2   = (interleaveDetect == kInterleave2Force );
	Settings.ForceInterleaveGD24 = (interleaveDetect == kInterleaveGD24   );
	Settings.ForceNotInterleaved = (interleaveDetect == kNoInterleaveForce);
	Settings.ForcePAL            = (videoDetect      == kPALForce         );
	Settings.ForceNTSC           = (videoDetect      == kNTSCForce        );

	GFX.InfoString = NULL;
	GFX.InfoStringTimeout = 0;

	S9xResetSaveTimer(true);

	err = FSRefMakePath(&cartRef, (unsigned char *) filename, PATH_MAX);

	SNES9X_InitSound();

	if (Memory.LoadROM(filename) /*&& (Memory.ROMCRC32 == nprominfo.crc32)*/)
	{
		ChangeTypeAndCreator(filename, 'CART', '~9X~');
		cartOpen = true;
		return (true);
	}
	else
	{
		cartOpen = false;
		return (false);
	}
}

static void NPClientDetachPrepareThread (void)
{
	pthread_create(&preparethread, NULL, NPClientPrepareThread, NULL);
	pthread_detach(preparethread);
}

static void * NPClientPrepareThread (void *)
{
	NPNotification("Client: Entered preparing thread.", -1);

	if ((NPClientROMReadyToServer()        == false) ||
		(NPClientGetSRAMFromServer()       == false) ||
		(NPClientGetPlayerListFromServer() == false) ||
		(NPClientReplyPhaseSpanTest()      == false))
	{
		NPClientDisconnect();
		npclient.dialogprocess = kNPCDialogPrepareFailed;
		return (NULL);
	}

	npclient.dialogprocess = kNPCDialogShowList;
	NPNotification("Client: Exited preparing thread.", -1);
	return (NULL);
}

static bool8 NPClientROMReadyToServer (void)
{
	if (!npclient.online)
		return (false);

	NPNotification("Client: Sending ROM ready sign to server...", -1);

	if (NPClientSendMesToServer(kNPClientROMOpened) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5401);
		return (false);
	}

	NPNotification("Client: Sent ROM ready sign to server.", -1);
	return (true);
}

static bool8 NPClientGetSRAMFromServer (void)
{
	if (!npclient.online)
		return (false);

	NPNotification("Client: Receiving SRAM from server...", -1);

	if (NPClientGetMesFromServer() != kNPServerSRAMWillSend)
	{
		NPError("Client: Failed to receive messsage from server.", 5501);
		return (false);
	}

	if (NPClientSendMesToServer(kNPClientSRAMWaiting) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5502);
		return (false);
	}

	uint8	mes[4];
	uint32	sramsize;

	if (socket_read(npclient.socket, mes, 4) != 4)
	{
		NPError("Client: Failed to receive SRAM size from server.", 5503);
		return (false);
	}

	sramsize = READ_LONG(mes + 0);

	if (sramsize != (uint32) (Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0))
	{
		NPError("Client: SRAM size mismatch.", 5504);
		return (false);
	}

	if (sramsize && (socket_read(npclient.socket, Memory.SRAM, sramsize) != (int) sramsize))
	{
		NPError("Server: Failed to receive SRAM from server.", 5505);
		return (false);
	}

	if (NPClientSendMesToServer(kNPClientSRAMLoaded) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5506);
		return (false);
	}

	NPNotification("Client: Received SRAM from server.", -1);
	return (true);
}

static bool8 NPClientGetPlayerListFromServer (void)
{
	if (!npclient.online)
		return (false);

	NPNotification("Client: Receiving player list from server...", -1);

	if (NPClientGetMesFromServer() != kNPServerPlayerWillSend)
	{
		NPError("Client: Failed to receive messsage from server.", 5701);
		return (false);
	}

	if (NPClientSendMesToServer(kNPClientPlayerWaiting) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5702);
		return (false);
	}

	for (int i = 0; i < NP_MAX_PLAYERS; i++)
	{
		uint8	mes[10];
		uint32	l;

		if (socket_read(npclient.socket, mes, 10) != 10)
		{
			NPError("Client: Failed to receive messsage from server.", 5703);
			return (false);
		}

		npcinfo[i].ready  = READ_BYTE(mes + 1);
		npcinfo[i].player = READ_LONG(mes + 2);

		l = READ_LONG(mes + 6);

		if (l && (socket_read(npclient.socket, (uint8 *) npcinfo[i].name, l) != (int) l))
		{
			NPError("Client: Failed to receive messsage from server.", 5704);
			return (false);
		}

		npcinfo[i].name[l] = 0;
	}

	npclient.numplayers = 0;
	for (int i = 0; i < NP_MAX_PLAYERS; i++)
		if (npcinfo[i].ready)
			npclient.numplayers++;

	NPNotification("Client: Received player list from server.", -1);
	NPNotification("Client: Number of players: %d", npclient.numplayers);

	return (true);
}

static bool8 NPClientReplyPhaseSpanTest (void)
{
	uint8   mes[21];
	int		l = npclient.numplayers * 4 + 1;

	NPNotification("Client: Replying sending / receiving pad states test...", -1);

	for (int n = 0; n < 5; n++)
	{
		if (socket_read(npclient.socket, mes, l) != l)
			return (false);

		if (socket_write(npclient.socket, mes, 5) != 5)
			return (false);
	}

	NPNotification("Client: Replied sending / receiving pad states test.", -1);

	NPNotification("Client: Receiving phase span value from server...", -1);

	if (socket_read(npclient.socket, mes, 4) != 4)
		return (false);

	npclient.phasespan = READ_LONG(mes + 0);

	NPNotification("  phase span: %d (frames)", npclient.phasespan);

	NPNotification("Client: Received phase span value from server.", -1);

	return (true);
}

static void NPClientBeginPlayerListSheet (void)
{
	OSStatus	err;
	CFStringRef	ref;
	HIViewRef	ctl, root;
	HIViewID	cid;

	root = HIViewGetRoot(sRef);
	cid.signature = 'PLNM';

	for (int i = 0; i < NP_MAX_PLAYERS; i++)
	{
		if (npcinfo[i].ready)
		{
			cid.id = npcinfo[i].player;
			HIViewFindByID(root, cid, &ctl);
			ref = CFStringCreateWithCString(kCFAllocatorDefault, npcinfo[i].name, kCFStringEncodingUTF8);
			if (ref)
			{
				SetStaticTextCFString(ctl, ref, false);
				CFRelease(ref);
			}
			else
				SetStaticTextCFString(ctl, CFSTR("unknown"), false);
		}
	}

	err = ShowSheetWindow(sRef, mRef);
}

static void NPClientEndPlayerListSheet (void)
{
	OSStatus	err;

	err = HideSheetWindow(sRef);
}

static pascal OSStatus NPClientSheetEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	if (!npclient.dialogsheet)
		return (eventNotHandledErr);

	OSStatus	err, result = eventNotHandledErr;

	switch (GetEventClass(inEvent))
	{
		case kEventClassCommand:
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(false);
						result = noErr;
					}

					break;

				case kEventCommandProcess:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						switch (tHICommand.commandID)
						{
							case 'ok  ':
								npclient.dialogprocess = kNPCDialogDone;
								result = noErr;
								break;
						}
					}

					break;
			}

			break;
	}

	return (result);
}

void NPClientDetachNetPlayThread (void)
{
	NPNotification("Client: Detaching pad thread...", -1);

	npclient.padloop = true;
	npclient.exitsgn = false;

	csememu = sem_open(n_csememu, O_CREAT, 0600, 0);
	csempad = sem_open(n_csempad, O_CREAT, 0600, 0);

	pthread_create(&gamepadthread, NULL, NPClientNetPlayThread, NULL);
	pthread_detach(gamepadthread);

	NPNotification("Client: Detached pad thread.", -1);
}

void NPClientStopNetPlayThread (void)
{
	NPNotification("Client: Stopping pad thread...", -1);

	npclient.padloop = false;
	sem_post(csempad);
	sem_post(csememu);

	while (!npclient.exitsgn)
		sleep(0);

	sem_unlink(n_csememu);
	sem_unlink(n_csempad);
	sem_close(csememu);
	sem_close(csempad);

	NPNotification("Client: Stopped pad thread.", -1);
}

bool8 NPClientNetPlayWaitStart (void)
{
	NPNotification("Client: Waiting start flag...", -1);

	if (NPClientSendMesToServer(kNPClientStartWait) == false)
	{
		NPError("Client: Failed to send messsage to server.", 5801);
		return (false);
	}

	if (NPClientGetMesFromServer() != kNPServerStart)
	{
		NPError("Client: Failed to send messsage to server.", 5802);
		return (false);
	}

	npclient.phasecount = 0;
	npclient.header     = 0;

	sem_post(csempad);

	NPNotification("Client: Netplay started.", -1);
	return (true);
}

static void * NPClientNetPlayThread (void *)
{
	uint8	mes[NP_MAX_PLAYERS * 64 * 4 + 1];
	uint8	count = 0;
	int		l;

	NPNotification("Client: Entered pad thread.", -1);

	while (npclient.padloop)
	{
		sem_wait(csempad);

		l = npclient.numplayers * npclient.phasespan * 4 + 1;
		if (socket_read(npclient.socket, mes, l) != l)
		{
			npclient.exitsgn = true;
			sem_post(csememu);
			pthread_exit(NULL);
		}

		if ((mes[0] & 0xF) != count)
			NPNotification("Client: Warning: Failed to synchronize server.", -1);

		npclient.header = mes[0] & 0xF0;

		for (int i = 0; i < npclient.numplayers; i++)
			for (uint32 j = 0; j < npclient.phasespan; j++)
				npcrecvpad[i][j] = READ_LONG(mes + (i * npclient.phasespan + j) * 4 + 1);

		WRITE_BYTE(mes + 0, count);

		for (uint32 j = 0; j < npclient.phasespan; j++)
			WRITE_LONG(mes + j * 4 + 1, npcsendpad[j]);

		l = npclient.phasespan * 4 + 1;
		if (socket_write(npclient.socket, mes, l) != l)
		{
			npclient.exitsgn = true;
			sem_post(csememu);
			pthread_exit(NULL);
		}

		count = (count + 1) & 0xF;

		sem_post(csememu);
	}

	npclient.exitsgn = true;

	NPNotification("Client: Exited pad thread.", -1);
	return (NULL);
}

void NPClientProcessInput (void)
{
	static uint32	pos = 0;
	KeyMap			myKeys;

	if (npclient.exitsgn)
	{
		if (s9xthreadrunning)
		{
			if (!eventQueued)
			{
				PostQueueToSubEventLoop();
				eventQueued = true;
			}
		}
		else
			running = false;

		return;
	}

	if (npclient.phasecount == 0)
	{
		sem_wait(csememu);

		for (int i = 0; i < npclient.numplayers; i++)
			for (uint32 j = 0; j < npclient.phasespan; j++)
				npcactvpad[i][j] = npcrecvpad[i][j];

		for (uint32 j = 0; j < npclient.phasespan; j++)
			npcsendpad[j] = npccachpad[j];

		if (npclient.header & 0x80)
		{
			npcsendpad[npclient.phasespan] = 0;
			for (int i = 0; i < npclient.numplayers; i++)
				npcactvpad[i][npclient.phasespan] = 0;

			npclient.phasespan++;
			if (npclient.phasespan > (uint32) Memory.ROMFramesPerSecond)
				npclient.phasespan = (uint32) Memory.ROMFramesPerSecond;

			char	str[256];
			sprintf(str, "delay: %d", npclient.phasespan);
			S9xMessage(0, 0, str);
		}
		else
		if (npclient.header & 0x40)
		{
			npclient.phasespan--;
			if (npclient.phasespan == 0)
				npclient.phasespan = 1;

			char	str[256];
			sprintf(str, "delay: %d", npclient.phasespan);
			S9xMessage(0, 0, str);
		}

		npclient.header = 0;
		pos = 0;
	}

	for (int i = 0; i < npclient.numplayers; i++)
	{
 		controlPad[i] = npcactvpad[i][pos];
		ControlPadFlagsToS9xReportButtons(i, controlPad[i]);
	}

	GetKeys(myKeys);

	uint32  pad = 0;

	JoypadScanDirection(0, &pad);
	if (ISpKeyIsPressed(kISp1PR     ))	pad |= 0x0010;
	if (ISpKeyIsPressed(kISp1PL     ))	pad |= 0x0020;
	if (ISpKeyIsPressed(kISp1PX     ))	pad |= 0x0040;
	if (ISpKeyIsPressed(kISp1PA     ))	pad |= 0x0080;
	if (ISpKeyIsPressed(kISp1PStart ))	pad |= 0x1000;
	if (ISpKeyIsPressed(kISp1PSelect))	pad |= 0x2000;
	if (ISpKeyIsPressed(kISp1PY     ))	pad |= 0x4000;
	if (ISpKeyIsPressed(kISp1PB     ))	pad |= 0x8000;

	if (KeyIsPressed(myKeys, keyCode[k1PR]     ))	pad |= 0x0010;
	if (KeyIsPressed(myKeys, keyCode[k1PL]     ))	pad |= 0x0020;
	if (KeyIsPressed(myKeys, keyCode[k1PX]     ))	pad |= 0x0040;
	if (KeyIsPressed(myKeys, keyCode[k1PA]     ))	pad |= 0x0080;
	if (KeyIsPressed(myKeys, keyCode[k1PRight] ))	pad |= 0x0100;
	if (KeyIsPressed(myKeys, keyCode[k1PLeft]  ))	pad |= 0x0200;
	if (KeyIsPressed(myKeys, keyCode[k1PDown]  ))	pad |= 0x0400;
	if (KeyIsPressed(myKeys, keyCode[k1PUp]    ))	pad |= 0x0800;
	if (KeyIsPressed(myKeys, keyCode[k1PStart] ))	pad |= 0x1000;
	if (KeyIsPressed(myKeys, keyCode[k1PSelect]))	pad |= 0x2000;
	if (KeyIsPressed(myKeys, keyCode[k1PY]     ))	pad |= 0x4000;
	if (KeyIsPressed(myKeys, keyCode[k1PB]     ))	pad |= 0x8000;

	npccachpad[pos] = pad;

	if (npclient.phasecount == 0)
	{
		npclient.phasecount = npclient.phasespan;
		sem_post(csempad);
	}

	npclient.phasecount--;
	pos++;
}
