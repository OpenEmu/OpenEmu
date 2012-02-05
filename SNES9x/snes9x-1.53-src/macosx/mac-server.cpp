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

#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#include "mac-prefix.h"
#include "mac-controls.h"
#include "mac-dialog.h"
#include "mac-joypad.h"
#include "mac-keyboard.h"
#include "mac-os.h"
#include "mac-stringtools.h"
#include "mac-netplay.h"
#include "mac-server.h"

#ifdef SELF_TEST
#include <sys/un.h>
#endif

#define KeyIsPressed(km, k)	(1 & (((unsigned char *) km) [(k) >> 3] >> ((k) & 7)))

enum
{
	kNPSDialogNone,
	kNPSDialogInit,
	kNPSDialogProcess,
	kNPSDialogDone,
	kNPSDialogCancel
};

typedef struct
{
	volatile bool8	listenloop;
	volatile uint32 phasecount;
	volatile uint32 phasespan;
	volatile uint8  header;
	int				socket;
	int				numplayers;

	bool8			dialogcancel;
	int				dialogprocess;
}	serverState;

typedef struct
{
	volatile bool8	padloop;
	volatile bool8	exitsgn;
	bool8			online;
	bool8			ready;
	int				socket;
	int				client;
	int				player;
	char			ip[256];
	char			name[256];
}	clientsState;

static char n_sememu[NP_MAX_PLAYERS][30] =
{
	"/tmp/s9x_s_emu_semaphore1",
	"/tmp/s9x_s_emu_semaphore2",
	"/tmp/s9x_s_emu_semaphore3",
	"/tmp/s9x_s_emu_semaphore4",
	"/tmp/s9x_s_emu_semaphore5"
};

static char n_sempad[NP_MAX_PLAYERS][30] =
{
	"/tmp/s9x_s_pad_semaphore1",
	"/tmp/s9x_s_pad_semaphore2",
	"/tmp/s9x_s_pad_semaphore3",
	"/tmp/s9x_s_pad_semaphore4",
	"/tmp/s9x_s_pad_semaphore5"
};

static serverState  npserver;
static clientsState npplayer[NP_MAX_PLAYERS];

static uint32		npactvpad[NP_MAX_PLAYERS][64],	// [player number][]
					nprecvpad[NP_MAX_PLAYERS][64],	// [player number][]
					npsendpad[NP_MAX_PLAYERS][64],	// [player number][]
					npcachpad[64];

static sem_t		*sememu[NP_MAX_PLAYERS],
					*sempad[NP_MAX_PLAYERS];

static pthread_t	listenthread,
					processthread,
					padthread[NP_MAX_PLAYERS];

static int NPServerAcceptClient (int);
static int NPServerGetMesFromClient (int);
static void NPServerBeginListenLoop (void);
static void NPServerEndListenLoop (void);
static void NPServerDetachProcessThread (void);
static void NPServerShutdownClient (int);
static void NPServerAllotPlayers (void);
static void NPServerSendPlayerList (void);
static void NPServerWaitStartReply (void);
static void NPServerSetPhaseSpan (void);
static bool8 NPServerSendMesToClient (int, int);
static bool8 NPServerGetNameFromClient (int);
static bool8 NPServerSendROMInfoToClient (int);
static bool8 NPServerSendSRAMToClient (int);
static bool8 NPServerSendPlayerListToClient (int);
static void * NPServerListenLoop (void *);
static void * NPServerProcessThread (void *);
static void * NPServerNetPlayThread (void *);
static pascal void NPServerDialogTimerHandler (EventLoopTimerRef, void *);
static pascal OSStatus NPServerDialogEventHandler (EventHandlerCallRef, EventRef, void *);


bool8 NPServerDialog (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	npserver.dialogcancel = true;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;

		err = CreateWindowFromNib(nibRef, CFSTR("ClientList"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerRef		eref;
			EventLoopTimerRef	tref;
			EventHandlerUPP		eventUPP;
			EventLoopTimerUPP	timerUPP;
			EventTypeSpec		windowEvents[] = { { kEventClassCommand, kEventCommandProcess      },
												   { kEventClassCommand, kEventCommandUpdateStatus } };
			HIViewRef			ctl;
			HIViewID			cid = { 'Chse', 0 };

			npserver.dialogprocess = kNPSDialogInit;

			eventUPP = NewEventHandlerUPP(NPServerDialogEventHandler);
			err = InstallWindowEventHandler(tWindowRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) tWindowRef, &eref);

			timerUPP = NewEventLoopTimerUPP(NPServerDialogTimerHandler);
			err = InstallEventLoopTimer(GetCurrentEventLoop(), 0.0f, 0.1f, timerUPP, (void *) tWindowRef, &tref);

			HIViewFindByID(HIViewGetRoot(tWindowRef), cid, &ctl);
			HIViewSetVisible(ctl, false);

			MoveWindowPosition(tWindowRef, kWindowServer, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowServer);

			err = RemoveEventLoopTimer(tref);
			DisposeEventLoopTimerUPP(timerUPP);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eventUPP);

			CFRelease(tWindowRef);
		}

		DisposeNibReference(nibRef);
	}

	return (!npserver.dialogcancel);
}

static pascal void NPServerDialogTimerHandler (EventLoopTimerRef inTimer, void *userData)
{
	WindowRef	window = (WindowRef) userData;
	CFStringRef	ref;
	HIViewRef	ctl, root;
	HIViewID	cid;
	int			n = 0;

	root = HIViewGetRoot(window);

	for (int c = 0; c < NP_MAX_PLAYERS; c++)
	{
		cid.id = c;

		cid.signature = 'Pnum';
		HIViewFindByID(root, cid, &ctl);
		if (npplayer[c].ready)
		{
			char	num[4];

			num[0] = '1' + n;
			num[1] = 'P';
			num[2] = 0;
			SetStaticTextCStr(ctl, num, true);
			n++;
		}

		cid.signature = 'IP__';
		HIViewFindByID(root, cid, &ctl);
		if (npplayer[c].online)
		{
			ref = CFStringCreateWithCString(kCFAllocatorDefault, npplayer[c].ip, kCFStringEncodingUTF8);
			if (ref)
			{
				SetStaticTextCFString(ctl, ref, true);
				CFRelease(ref);
			}
			else
				SetStaticTextCFString(ctl, CFSTR("unknown"), true);
		}
		else
			SetStaticTextCFString(ctl, CFSTR(""), true);

		cid.signature = 'Name';
		HIViewFindByID(root, cid, &ctl);
		if (npplayer[c].online)
		{
			ref = CFStringCreateWithCString(kCFAllocatorDefault, npplayer[c].name, kCFStringEncodingUTF8);
			if (ref)
			{
				SetStaticTextCFString(ctl, ref, true);
				CFRelease(ref);
			}
			else
				SetStaticTextCFString(ctl, CFSTR("unknown"), true);
		}
		else
			SetStaticTextCFString(ctl, CFSTR(""), true);

		cid.signature = 'Stat';
		HIViewFindByID(root, cid, &ctl);
		if (npplayer[c].online)
		{
			if (npplayer[c].ready)
				ref = CFCopyLocalizedString(CFSTR("NPReady"), "NPReady");
			else
				ref = CFCopyLocalizedString(CFSTR("NPConnecting"), "NPConnecting");

			if (ref)
			{
				SetStaticTextCFString(ctl, ref, true);
				CFRelease(ref);
			}
			else
				SetStaticTextCFString(ctl, CFSTR("error"), true);
		}
		else
			SetStaticTextCFString(ctl, CFSTR(""), true);
	}

	switch (npserver.dialogprocess)
	{
		case kNPSDialogNone:
			break;

		case kNPSDialogInit:
			NPNotification("  kNPSDialogInit", -1);
			npserver.dialogprocess = kNPSDialogNone;
			NPServerBeginListenLoop();
			break;

		case kNPSDialogProcess:
			NPNotification("  kNPSDialogProcess", -1);
			npserver.dialogprocess = kNPSDialogNone;
			NPServerEndListenLoop();
			cid.id = 0;
			cid.signature = 'Chse';
			HIViewFindByID(root, cid, &ctl);
			HIViewSetVisible(ctl, true);
			NPServerDetachProcessThread();
			break;

		case kNPSDialogDone:
			NPNotification("  kNPSDialogDone", -1);
			npserver.dialogprocess = kNPSDialogNone;
			npserver.dialogcancel = false;
			QuitAppModalLoopForWindow(window);
			break;

		case kNPSDialogCancel:
			NPNotification("  kNPSDialogCancel", -1);
			npserver.dialogprocess = kNPSDialogNone;
			NPServerEndListenLoop();
			npserver.dialogcancel = true;
			QuitAppModalLoopForWindow(window);
			break;
	}
}

static pascal OSStatus NPServerDialogEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef = (WindowRef) inUserData;

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
							case 'OKAY':
								HIViewRef	ctl, root;
								HIViewID	cid;

								root = HIViewGetRoot(tWindowRef);
								cid.id = 0;
								cid.signature = 'OKAY';
								HIViewFindByID(root, cid, &ctl);
								DeactivateControl(ctl);
								cid.signature = 'CNSL';
								HIViewFindByID(root, cid, &ctl);
								DeactivateControl(ctl);

								npserver.dialogprocess = kNPSDialogProcess;
								result = noErr;
								break;

							case 'CNSL':
								npserver.dialogprocess = kNPSDialogCancel;
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

void NPServerInit (void)
{
	npserver.listenloop = false;
	npserver.phasecount = 0;
	npserver.phasespan  = 0;
	npserver.header     = 0;
	npserver.socket     = -1;
	npserver.numplayers = 0;

	for (int i = 0; i < NP_MAX_PLAYERS; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			npactvpad[i][j] = 0;
			nprecvpad[i][j] = 0;
			npsendpad[i][j] = 0;
		}
	}

	for (int j = 0; j < 64; j++)
		npcachpad[j] = 0;

	for (int c = 0; c < NP_MAX_PLAYERS; c++)
	{
		npplayer[c].padloop = false;
		npplayer[c].exitsgn = false;
		npplayer[c].online  = false;
		npplayer[c].ready   = false;
		npplayer[c].socket  = -1;
		npplayer[c].client  = 0;
		npplayer[c].player  = 0;
		npplayer[c].ip[0]   = 0;
		npplayer[c].name[0] = 0;
	}

	npplayer[0].online = true;
	npplayer[0].ready  = true;

	char	name[256];
	if (gethostname(name, 256) == 0)
	{
		struct hostent	*hn;
		if ((hn = gethostbyname(name)) != NULL)
		{
			struct in_addr	addr;
			memcpy(&addr, hn->h_addr_list[0], sizeof(struct in_addr));
			strcpy(npplayer[0].ip, inet_ntoa(addr));
		}
		else
			strcpy(npplayer[0].ip, "unknown");
	}
	else
		strcpy(npplayer[0].ip, "unknown");

	CFStringRef	ref;
	ref = CFCopyLocalizedString(CFSTR("NPServerName"), "NPServer");
	if (ref)
	{
		Boolean	r;
		r = CFStringGetCString(ref, npplayer[0].name, 256, kCFStringEncodingUTF8);
		if (!r)
			strcpy(npplayer[0].name, "unknown");

		CFRelease(ref);
	}
	else
		strcpy(npplayer[0].name, "unknown");
}

bool8 NPServerStartServer (int port)
{
#ifndef SELF_TEST
	struct sockaddr_in	address;
#else
	struct sockaddr_un	address;
#endif
	int					v = 1;

	NPNotification("Server: Starting server...", -1);

#ifndef SELF_TEST
	if ((npserver.socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
#else
	if ((npserver.socket = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
#endif
	{
		NPError("Server: Failed to create listening socket.", 1001);
		return (false);
	}

	if ((setsockopt(npserver.socket, SOL_SOCKET, SO_REUSEADDR, (char *) &v, sizeof(v))) < 0)
	{
		NPError("Server: Failed to set socket option.", 1002);
		return (false);
	}

	memset(&address, 0, sizeof(address));
#ifndef SELF_TEST
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port        = htons(port);
#else
	address.sun_family      = AF_UNIX;
    strcpy(address.sun_path, SOCK_NAME);
#endif

#ifndef SELF_TEST
	if ((bind(npserver.socket, (struct sockaddr *) &address, sizeof(address))) < 0)
#else
	unlink(SOCK_NAME);
	if ((bind(npserver.socket, (struct sockaddr *) &address, sizeof(address))) < 0)
#endif
	{
		NPError("Server: Failed to bind socket to port number.", 1003);
		return (false);
	}

	if ((listen(npserver.socket, NP_MAX_CLIENTS)) < 0)
	{
		NPError("Server: Failed to get new socket to listen.", 1004);
		return (false);
	}

	npplayer[0].socket = npserver.socket;

	NPNotification("Server: Started server.", -1);
	return (true);
}

void NPServerStopServer (void)
{
	NPNotification("Server: Stopping server...", -1);

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		NPServerShutdownClient(c);

	npplayer[0].online  = false;
	npplayer[0].ready   = false;
	npplayer[0].socket  = -1;
	npplayer[0].client  = 0;
	npplayer[0].player  = 0;
	npplayer[0].ip[0]   = 0;
	npplayer[0].name[0] = 0;

	if (npserver.socket != -1)
	{
		close(npserver.socket);
		npserver.socket = -1;
	}

	NPNotification("Server: Stopped server.", -1);
}

static void NPServerShutdownClient (int c)
{
	if (npplayer[c].online)
	{
		NPNotification("Server: Closing client %d connection...", c);

		if (npplayer[c].socket != -1)
		{
			close(npplayer[c].socket);
			npplayer[c].socket = -1;
		}

		npplayer[c].online  = false;
		npplayer[c].ready   = false;
		npplayer[c].client  = 0;
		npplayer[c].player  = 0;
		npplayer[c].ip[0]   = 0;
		npplayer[c].name[0] = 0;

		NPNotification("Server: Client %d has disconnected.", c);
	}
}

static int NPServerAcceptClient (int port)
{
#ifndef SELF_TEST
	struct sockaddr_in	address;
#else
	struct sockaddr_un	address;
#endif
	int					newfd;
	int					c;
	socklen_t			l;

	NPNotification("Server: Accepting new client connection...", -1);

	for (c = 1; c <= NP_MAX_CLIENTS; c++)
		if (!npplayer[c].online)
			break;

	if (c > NP_MAX_CLIENTS)
	{
		NPError("Server: Maximum number of clients have already connected.", 1101);
		return (-1);
	}

	l = sizeof(address);
	memset(&address, 0, l);

	if ((newfd = accept(port, (struct sockaddr *) &address, &l)) < 0)
	{
		NPError("Server: Can't accept client connection.", 1102);
		return (-1);
	}

	npplayer[c].online = true;
	npplayer[c].socket = newfd;

#ifndef SELF_TEST
	if (address.sin_family == AF_INET)
		strcpy(npplayer[c].ip, inet_ntoa(address.sin_addr));
	else
		strcpy(npplayer[c].ip, "unknown");
#else
		strcpy(npplayer[c].ip, "Unix");
#endif

	NPNotification("Server: new client %d has connected.", c);

	return (c);
}

static void NPServerBeginListenLoop (void)
{
	npserver.listenloop = true;
	pthread_create(&listenthread, NULL, NPServerListenLoop, NULL);
}

static void NPServerEndListenLoop (void)
{
	npserver.listenloop = false;
	pthread_join(listenthread, NULL);
}

static void * NPServerListenLoop (void *)
{
	struct timeval	timeout;
	fd_set			readfds;
	int				maxfd;

	NPNotification("Server: Entered listening loop.", -1);

	while (npserver.listenloop)
	{
		FD_ZERO(&readfds);
		maxfd = 0;

		if (npserver.socket != -1)
		{
			FD_SET(npserver.socket, &readfds);
			maxfd = npserver.socket;
		}

		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		{
			if (npplayer[c].online)
			{
				FD_SET(npplayer[c].socket, &readfds);
				if (maxfd < npplayer[c].socket)
					maxfd = npplayer[c].socket;
			}
		}

		timeout.tv_sec  = 0;
		timeout.tv_usec = 50000;

		if (select(maxfd + 1, &readfds, NULL, NULL, &timeout) > 0)
		{
			for (int c = 1; c <= NP_MAX_CLIENTS; c++)
			{
				if (npplayer[c].online)
				{
					if (FD_ISSET(npplayer[c].socket, &readfds))
					{
						switch (NPServerGetMesFromClient(c))
						{
							case kNPClientNameSent:
								if (!NPServerSendROMInfoToClient(c))
									NPServerShutdownClient(c);
								break;

							case kNPClientROMOpened:
								if (!NPServerSendSRAMToClient(c))
									NPServerShutdownClient(c);
								break;

							case kNPClientSRAMLoaded:
								npplayer[c].ready = true;
								break;

							default:
								NPServerShutdownClient(c);
								break;
						}
					}
				}
			}

			if (FD_ISSET(npserver.socket, &readfds))
			{
				int	client;

				if ((client = NPServerAcceptClient(npserver.socket)) != -1)
				{
					if (!NPServerGetNameFromClient(client))
						NPServerShutdownClient(client);
				}
			}
		}
	}

	NPNotification("Server: Exited listening loop.", -1);

	return (NULL);
}

static bool8 NPServerSendMesToClient (int c, int num)
{
	uint8	mes[2];

	mes[0] = NP_SERVER_MAGIC;
	mes[1] = num;

	if (socket_write(npplayer[c].socket, mes, 2) != 2)
		return (false);

	return (true);
}

static int NPServerGetMesFromClient (int c)
{
	uint8	mes[2];

	if (socket_read(npplayer[c].socket, mes, 2) != 2)
		return (-1);

	if (mes[0] != NP_CLIENT_MAGIC)
		return (-1);

	return ((int) mes[1]);
}

static bool8 NPServerGetNameFromClient (int c)
{
	if (!npplayer[c].online)
		return (false);

	NPNotification("Server: Receiving player name from client %d...", c);

	if (NPServerSendMesToClient(c, kNPServerNameRequest) == false)
	{
		NPError("Server: Failed to send messsage to client.", 1201);
		return (false);
	}

	uint8	mes[4];
	uint32	l;

	if (socket_read(npplayer[c].socket, mes, 4) != 4)
	{
		NPError("Server: Failed to receive name size from client.", 1202);
		return (false);
	}

	l = READ_LONG(mes + 0);

	if (socket_read(npplayer[c].socket, (uint8 *) npplayer[c].name, l) != (int) l)
	{
		NPError("Server: Failed to receive name from client.", 1203);
		return (false);
	}

	npplayer[c].name[l] = 0;

	if (NPServerSendMesToClient(c, kNPServerNameReceived) == false)
	{
		NPError("Server: Failed to send messsage to client.", 1204);
		return (false);
	}

	NPNotification("Server: Received player name from client %d.", c);
	return (true);

	// next: kNPClientNameSent
}

static bool8 NPServerSendROMInfoToClient (int c)
{
	if (!npplayer[c].online)
		return (false);

	NPNotification("Server: Sending ROM information to client %d...", c);

	if (NPServerSendMesToClient(c, kNPServerROMInfoWillSend) == false)
	{
		NPError("Server: Failed to send messsage to client.", 1301);
		return (false);
	}

	if (NPServerGetMesFromClient(c) != kNPClientROMInfoWaiting)
	{
		NPError("Server: Failed to receive messsage from client.", 1302);
		return (false);
	}

	uint8	mes[16];
	uint32	l;
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	l = strlen(fname);

	WRITE_LONG(mes + 0,  Memory.ROMCRC32);
	WRITE_LONG(mes + 4,  deviceSetting);
	WRITE_BYTE(mes + 8,  0);	// reserved
	WRITE_BYTE(mes + 9,  0);	// reserved
	WRITE_BYTE(mes + 10, 0);	// reserved
	WRITE_BYTE(mes + 11, 0);	// reserved
	WRITE_LONG(mes + 12, l);

	if (socket_write(npplayer[c].socket, mes, 16) != 16)
	{
		NPError("Server: Failed to send ROM information to client.", 1303);
		return (false);
	}

	if (socket_write(npplayer[c].socket, (uint8 *) fname, l) != (int) l)
	{
		NPError("Server: Failed to send ROM name to client.", 1304);
		return (false);
	}

	NPNotification("Server: Sent ROM information to client %d.", c);
	return (true);

	// next: kNPClientROMOpened
}

static bool8 NPServerSendSRAMToClient (int c)
{
	if (!npplayer[c].online)
		return (false);

	NPNotification("Server: Sending SRAM to client %d...", c);

	if (NPServerSendMesToClient(c, kNPServerSRAMWillSend) == false)
	{
		NPError("Server: Failed to send messsage to client.", 1401);
		return (false);
	}

	if (NPServerGetMesFromClient(c) != kNPClientSRAMWaiting)
	{
		NPError("Server: Failed to receive messsage from client.", 1402);
		return (false);
	}

	uint8	mes[4];
	uint32	sramsize;

	sramsize = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;

	WRITE_LONG(mes + 0, sramsize);

	if (socket_write(npplayer[c].socket, mes, 4) != 4)
	{
		NPError("Server: Failed to send SRAM size to client.", 1403);
		return (false);
	}

	if (sramsize && (socket_write(npplayer[c].socket, Memory.SRAM, sramsize) != (int) sramsize))
	{
		NPError("Server: Failed to send SRAM to client.", 1404);
		return (false);
	}

	NPNotification("Server: Sent SRAM to client %d.", c);
	return (true);

	// next: kNPClientSRAMLoaded
}

static void NPServerDetachProcessThread (void)
{
	pthread_create(&processthread, NULL, NPServerProcessThread, NULL);
	pthread_detach(processthread);
}

static void * NPServerProcessThread (void *)
{
	NPNotification("Server: Entered process thread.", -1);

	NPServerAllotPlayers();
	NPServerSendPlayerList();
	NPServerSetPhaseSpan();
	NPServerWaitStartReply();

	npserver.dialogprocess = kNPSDialogDone;

	NPNotification("Server: Exited process thread.", -1);
	return (NULL);
}

static void NPServerAllotPlayers (void)
{
	int	n = 1;

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
	{
		if (npplayer[c].ready)
		{
			npplayer[c].client = c;
			npplayer[c].player = n++;
		}
		else
			NPServerShutdownClient(c);
	}

	npplayer[0].client = 0;
	npplayer[0].player = 0;

	npserver.numplayers = n;

	NPNotification("Server: Number of players: %d", n);
}

static bool8 NPServerSendPlayerListToClient (int c)
{
	if (!npplayer[c].online || !npplayer[c].ready)
		return (false);

	NPNotification("Server: Sending player list to client %d...", c);

	if (NPServerSendMesToClient(c, kNPServerPlayerWillSend) == false)
	{
		NPError("Server: Failed to send messsage to client.", 1601);
		return (false);
	}

	if (NPServerGetMesFromClient(c) != kNPClientPlayerWaiting)
	{
		NPError("Server: Failed to receive messsage from client.", 1602);
		return (false);
	}

	for (int i = 0; i < NP_MAX_PLAYERS; i++)
	{
		uint8	mes[10];
		uint32	l;

		l = npplayer[i].ready ? strlen(npplayer[i].name) : 0;

		WRITE_BYTE(mes + 0, (i == c));
		WRITE_BYTE(mes + 1, npplayer[i].ready);
		WRITE_LONG(mes + 2, npplayer[i].player);
		WRITE_LONG(mes + 6, l);

		if (socket_write(npplayer[c].socket, mes, 10) != 10)
		{
			NPError("Server: Failed to send name size to client.", 1603);
			return (false);
		}

		if (l && (socket_write(npplayer[c].socket, (uint8 *) npplayer[i].name, l) != (int) l))
		{
			NPError("Server: Failed to send name to client.", 1604);
			return (false);
		}
	}

	NPNotification("Server: Sent player list to client %d.", c);
	return (true);
}

static void NPServerSendPlayerList (void)
{
	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
	{
		if (npplayer[c].ready)
		{
			if (NPServerSendPlayerListToClient(c) == false)
				NPServerShutdownClient(c);
		}
	}
}

static void NPServerSetPhaseSpan (void)
{
	struct timeval  tv1, tv2;
	uint8			mes[21];
	uint32			dus, dusmax;
	int				l = npserver.numplayers * 4 + 1;

	NPNotification("Server: Testing sending / receiving pad states...", -1);

	dusmax = 0;

	for (int n = 0; n < 5; n++)
	{
		gettimeofday(&tv1, NULL);

		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		{
			if (npplayer[c].ready)
			{
				if (socket_write(npplayer[c].socket, mes, l) != l)
					NPServerShutdownClient(c);
			}
		}

		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		{
			if (npplayer[c].ready)
			{
				if (socket_read(npplayer[c].socket, mes, 5) != 5)
					NPServerShutdownClient(c);
			}
		}

		gettimeofday(&tv2, NULL);

		dus = (tv2.tv_sec * 1000000 + tv2.tv_usec) - (tv1.tv_sec * 1000000 + tv1.tv_usec);

		NPNotification("  %d [usec]", dus);

		if (dusmax < dus)
			dusmax = dus;

		usleep(50000);
	}

	NPNotification("Server: Tested sending / receiving pad states.", -1);

	npserver.phasespan = (uint32) Memory.ROMFramesPerSecond * dusmax / 1000000 + 1;
	if (npserver.phasespan > (uint32) Memory.ROMFramesPerSecond)
		npserver.phasespan = (uint32) Memory.ROMFramesPerSecond;

	NPNotification("  phase span: %d (frames)", npserver.phasespan);

	NPNotification("Server: Sending phase span value to clients...", -1);

	WRITE_LONG(mes + 0, npserver.phasespan);

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
	{
		if (npplayer[c].ready)
		{
			if (socket_write(npplayer[c].socket, mes, 4) != 4)
				NPServerShutdownClient(c);
		}
	}

	NPNotification("Server: Sent phase span value to clients.", -1);
}

static void NPServerWaitStartReply (void)
{
	struct timeval	timeout;
	fd_set			readfds;
	int				maxfd;
	bool8			allok, flag[NP_MAX_PLAYERS];

	NPNotification("Server: Waiting clients reply to start...", -1);

	allok = false;
	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		flag[c] = false;

	while (!allok)
	{
		FD_ZERO(&readfds);
		maxfd = 0;

		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		{
			if (npplayer[c].ready)
			{
				FD_SET(npplayer[c].socket, &readfds);
				if (maxfd < npplayer[c].socket)
					maxfd = npplayer[c].socket;
			}
		}

		timeout.tv_sec  = 0;
		timeout.tv_usec = 50000;

		if (select(maxfd + 1, &readfds, NULL, NULL, &timeout) > 0)
		{
			for (int c = 1; c <= NP_MAX_CLIENTS; c++)
			{
				if (npplayer[c].ready)
				{
					if (FD_ISSET(npplayer[c].socket, &readfds))
					{
						if (NPServerGetMesFromClient(c) == kNPClientStartWait)
							flag[c] = true;
						else
							NPServerShutdownClient(c);
					}
				}
			}
		}

		allok = true;
		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
			if (npplayer[c].ready && !flag[c])
				allok = false;
	}

	NPNotification("Server: All clients are ready to start netplay.", -1);
}

void NPServerDetachNetPlayThread (void)
{
	NPNotification("Server: Detaching pad threads...", -1);

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
	{
		if (npplayer[c].ready)
		{
			npplayer[c].padloop = true;
			npplayer[c].exitsgn = false;

			sememu[c] = sem_open(n_sememu[c], O_CREAT, 0600, 0);
			sempad[c] = sem_open(n_sempad[c], O_CREAT, 0600, 0);

			pthread_create(&padthread[c], NULL, NPServerNetPlayThread, &(npplayer[c].client));
			pthread_detach(padthread[c]);
		}
	}

	NPNotification("Server: Detached pad threads.", -1);
}

void NPServerStopNetPlayThread (void)
{
	NPNotification("Server: Stopping pad threads...", -1);

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
	{
		if (npplayer[c].ready)
		{
			npplayer[c].padloop = false;
			sem_post(sempad[c]);

			while (!npplayer[c].exitsgn)
				sleep(0);

			sem_unlink(n_sememu[c]);
			sem_unlink(n_sempad[c]);
			sem_close(sememu[c]);
			sem_close(sempad[c]);
		}
	}

	NPNotification("Server: Stopped pad threads.", -1);
}

void NPServerStartClients (void)
{
	NPNotification("Server: Sending start flag to clients...", -1);

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
	{
		if (npplayer[c].ready)
		{
			if (NPServerSendMesToClient(c, kNPServerStart) == false)
				NPServerShutdownClient(c);
		}
	}

	NPNotification("Server: Sent start flag to clients.", -1);

	npserver.phasecount = 0;
	npserver.header     = 0;

	for (int c = 1; c <= NP_MAX_CLIENTS; c++)
		if (npplayer[c].ready)
			sem_post(sempad[c]);

	NPNotification("Server: Netplay started.", -1);
}

static void * NPServerNetPlayThread (void *p)
{
	uint8	mes[NP_MAX_PLAYERS * 64 * 4 + 1];
	uint8	count = 0;
	int 	c = *((int *) p), l;

	NPNotification("Server: Entered pad thread for client %d.", c);

	while (npplayer[c].padloop)
	{
		sem_wait(sempad[c]);

		WRITE_BYTE(mes + 0, count | npserver.header);

		for (int i = 0; i < npserver.numplayers; i++)
			for (uint32 j = 0; j < npserver.phasespan; j++)
				WRITE_LONG(mes + (i * npserver.phasespan + j) * 4 + 1, npsendpad[i][j]);

		l = npserver.numplayers * npserver.phasespan * 4 + 1;
		if (socket_write(npplayer[c].socket, mes, l) != l)
		{
			NPServerShutdownClient(c);
			npplayer[c].exitsgn = true;
			sem_post(sememu[c]);
			pthread_exit(NULL);
		}

		l = npserver.phasespan * 4 + 1;
		if (socket_read(npplayer[c].socket, mes, l) != l)
		{
			NPServerShutdownClient(c);
			npplayer[c].exitsgn = true;
			sem_post(sememu[c]);
			pthread_exit(NULL);
		}

		for (uint32 j = 0; j < npserver.phasespan; j++)
			nprecvpad[npplayer[c].player][j] = READ_LONG(mes + j * 4 + 1);

		if ((mes[0] & 0xF) != count)
			NPNotification("Server: Warning: Failed to synchronize client %d.", c);

		count = (count + 1) & 0xF;

		sem_post(sememu[c]);
	}

	npplayer[c].exitsgn = true;

	NPNotification("Server: Exited pad thread for client %d.", c);
	return (NULL);
}

void NPServerProcessInput (void)
{
	static uint8	header = 0;
	static uint32	pos    = 0;
	KeyMap  		myKeys;

	if (npserver.phasecount == 0)
	{
		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
			if (npplayer[c].ready)
				sem_wait(sememu[c]);

		for (uint32 j = 0; j < npserver.phasespan; j++)
			nprecvpad[0][j] = npcachpad[j];

		for (int i = 0; i < npserver.numplayers; i++)
		{
			for (uint32 j = 0; j < npserver.phasespan; j++)
			{
				npactvpad[i][j] = npsendpad[i][j];
				npsendpad[i][j] = nprecvpad[i][j];
			}
		}

		if (npserver.header & 0x80)
		{
			for (int i = 0; i < npserver.numplayers; i++)
			{
				npactvpad[i][npserver.phasespan] = 0;
				npsendpad[i][npserver.phasespan] = 0;
			}

			npserver.phasespan++;
			if (npserver.phasespan > (uint32) Memory.ROMFramesPerSecond)
				npserver.phasespan = (uint32) Memory.ROMFramesPerSecond;

			char	str[256];
			sprintf(str, "delay: %d", npserver.phasespan);
			S9xMessage(0, 0, str);
		}
		else
		if (npserver.header & 0x40)
		{
			npserver.phasespan--;
			if (npserver.phasespan == 0)
				npserver.phasespan = 1;

			char	str[256];
			sprintf(str, "delay: %d", npserver.phasespan);
			S9xMessage(0, 0, str);
		}

		npserver.header = header;
		header = 0;
		pos = 0;
	}

 	for (int i = 0; i < npserver.numplayers; i++)
	{
		controlPad[i] = npactvpad[i][pos];
		ControlPadFlagsToS9xReportButtons(i, controlPad[i]);
	}

	GetKeys(myKeys);

	if (ISpKeyIsPressed(kISpEsc) || KeyIsPressed(myKeys, keyCode[kKeyEsc]))
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

	uint32	pad = 0;

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

	npcachpad[pos] = pad;

	if (KeyIsPressed(myKeys, keyCode[k2PR]))	header |= 0x80;
	if (KeyIsPressed(myKeys, keyCode[k2PL]))	header |= 0x40;

	if (npserver.phasecount == 0)
	{
		npserver.phasecount = npserver.phasespan;

		for (int c = 1; c <= NP_MAX_CLIENTS; c++)
			if (npplayer[c].ready)
				sem_post(sempad[c]);
	}

	npserver.phasecount--;
	pos++;
}
