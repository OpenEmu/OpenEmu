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


#ifdef NETPLAY_SUPPORT
#ifdef _DEBUG
	#define NP_DEBUG 1
#endif

#define NP_DEBUG 3 // FF-FIXME

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>

#ifdef __WIN32__
	#include <winsock.h>
	#include <process.h>
	#include "win32/wsnes9x.h"

	#define ioctl ioctlsocket
	#define close(h) if(h){closesocket(h);}
	#define read(a,b,c) recv(a, b, c, 0)
	#define write(a,b,c) send(a, b, c, 0)
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/stat.h>

	#include <netdb.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <sys/param.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	#ifdef __SVR4
		#include <sys/stropts.h>
	#endif
#endif

#ifdef USE_THREADS
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "netplay.h"
#include "snapshot.h"
#include "display.h"

void S9xNPClientLoop (void *);
bool8 S9xNPLoadROM (uint32 len);
bool8 S9xNPLoadROMDialog (const char *);
bool8 S9xNPGetROMImage (uint32 len);
void S9xNPGetSRAMData (uint32 len);
void S9xNPGetFreezeFile (uint32 len);

unsigned long START = 0;

bool8 S9xNPConnect ();

bool8 S9xNPConnectToServer (const char *hostname, int port,
                            const char *rom_name)
{
    if (!S9xNPInitialise ())
        return (FALSE);

    S9xNPDisconnect ();

    NetPlay.MySequenceNum = 0;
    NetPlay.ServerSequenceNum = 0;
    NetPlay.Connected = FALSE;
    NetPlay.Abort = FALSE;
    NetPlay.Player = 0;
    NetPlay.Paused = FALSE;
    NetPlay.PercentageComplete = 0;
    NetPlay.Socket = 0;
    if (NetPlay.ServerHostName)
        free ((char *) NetPlay.ServerHostName);
    NetPlay.ServerHostName = strdup (hostname);
    if (NetPlay.ROMName)
        free ((char *) NetPlay.ROMName);
    NetPlay.ROMName = strdup (rom_name);
    NetPlay.Port = port;
    NetPlay.PendingWait4Sync = FALSE;

#ifdef __WIN32__
    if (GUI.ClientSemaphore == NULL)
        GUI.ClientSemaphore = CreateSemaphore (NULL, 0, NP_JOYPAD_HIST_SIZE, NULL);

    if (NetPlay.ReplyEvent == NULL)
        NetPlay.ReplyEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

    _beginthread (S9xNPClientLoop, 0, NULL);

    return (TRUE);
#endif

    return S9xNPConnect();
}

bool8 S9xNPConnect ()
{
    struct sockaddr_in address;
    struct hostent *hostinfo;
    unsigned int addr;

    address.sin_family = AF_INET;
    address.sin_port = htons (NetPlay.Port);
#ifdef NP_DEBUG
    printf ("CLIENT: Looking up server's hostname (%s) @%ld\n", NetPlay.ServerHostName, S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Looking up server's hostname...");
    if ((int) (addr = inet_addr (NetPlay.ServerHostName)) == -1)
    {
	if ((hostinfo = gethostbyname (NetPlay.ServerHostName)))
	{
	    memcpy ((char *)&address.sin_addr, hostinfo->h_addr,
		    hostinfo->h_length);
	}
	else
	{
            S9xNPSetError ("\
Unable to look up server's IP address from hostname.\n\n\
Unknown hostname or may be your nameserver isn't set\n\
up correctly?");
	    return (FALSE);
	}
    }
    else
    {
	memcpy ((char *)&address.sin_addr, &addr, sizeof (addr));
    }

#ifdef NP_DEBUG
    printf ("CLIENT: Creating socket @%ld\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Creating network socket...");
    if ((NetPlay.Socket = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        S9xNPSetError ("Creating network socket failed.");
	return (FALSE);
    }

#ifdef NP_DEBUG
    printf ("CLIENT: Trying to connect to server @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Trying to connect to Snes9X server...");

    if (connect (NetPlay.Socket, (struct sockaddr *) &address, sizeof (address)) < 0)
    {
        char buf [100];
#ifdef __WIN32__
        if (WSAGetLastError () == WSAECONNREFUSED)
#else
	if (errno == ECONNREFUSED)
#endif
        {
            S9xNPSetError ("\
Connection to remote server socket refused:\n\n\
Is there actually a Snes9X NetPlay server running\n\
on the remote machine on this port?");
        }
        else
        {
            sprintf (buf, "Connection to server failed with error number %d",
#ifdef __WIN32__
                     WSAGetLastError ()
#else
		     errno
#endif
		     );
            S9xNPDisconnect ();
        }
	return (FALSE);
    }
    NetPlay.Connected = TRUE;

#ifdef NP_DEBUG
    printf ("CLIENT: Sending 'HELLO' message @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Sending 'HELLO' message...");
    /* Send the server a HELLO packet*/
    int len = 7 + 4 + strlen (NetPlay.ROMName) + 1;
    uint8 *tmp = new uint8 [len];
    uint8 *ptr = tmp;

    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = NP_CLNT_HELLO;
    WRITE_LONG (ptr, len);
    ptr += 4;
#ifdef __WIN32__
    uint32 ft = Settings.FrameTime;

    WRITE_LONG (ptr, ft);
#else
    WRITE_LONG (ptr, Settings.FrameTime);
#endif
    ptr += 4;
    strcpy ((char *) ptr, NetPlay.ROMName);

    if (!S9xNPSendData (NetPlay.Socket, tmp, len))
    {
        S9xNPSetError ("Sending 'HELLO' message failed.");
	S9xNPDisconnect ();
	delete tmp;
	return (FALSE);
    }
    delete tmp;

#ifdef NP_DEBUG
    printf ("CLIENT: Waiting for 'WELCOME' reply from server @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Waiting for 'HELLO' reply from server...");

    uint8 header [7];

    if (!S9xNPGetData (NetPlay.Socket, header, 7) ||
        header [0] != NP_SERV_MAGIC || header [1] != 0 ||
        (header [2] & 0x1f) != NP_SERV_HELLO)
    {
        S9xNPSetError ("Error in 'HELLO' reply packet received from server.");
	S9xNPDisconnect ();
	return (FALSE);
    }
#ifdef NP_DEBUG
    printf ("CLIENT: Got 'WELCOME' reply @%ld\n", S9xGetMilliTime () - START);
#endif
    len = READ_LONG (&header [3]);
    if (len > 256)
    {
        S9xNPSetError ("Error in 'HELLO' reply packet received from server.");
	S9xNPDisconnect ();
	return (FALSE);
    }
    uint8 *data = new uint8 [len];
    if (!S9xNPGetData (NetPlay.Socket, data, len - 7))
    {
        S9xNPSetError ("Error in 'HELLO' reply packet received from server.");
        delete data;
	S9xNPDisconnect ();
	return (FALSE);
    }

    if (data [0] != NP_VERSION)
    {
        S9xNPSetError ("\
The Snes9X NetPlay server implements a different\n\
version of the protocol. Disconnecting.");
        delete data;
	S9xNPDisconnect ();
        return (FALSE);
    }

    NetPlay.FrameCount = READ_LONG (&data [2]);

    if (!(header [2] & 0x80) &&
        strcmp ((char *) data + 4 + 2, NetPlay.ROMName) != 0)
    {
        if (!S9xNPLoadROMDialog ((char *) data + 4 + 2))
        {
            delete data;
            S9xNPDisconnect ();
            return (FALSE);
        }
    }
    NetPlay.Player = data [1];
    delete data;

    NetPlay.PendingWait4Sync = TRUE;
    Settings.NetPlay = TRUE;
    S9xNPResetJoypadReadPos ();
    NetPlay.ServerSequenceNum = 1;

#ifdef NP_DEBUG
    printf ("CLIENT: Sending 'READY' to server @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Sending 'READY' to the server...");

    return (S9xNPSendReady ((header [2] & 0x80) ?
                            NP_CLNT_WAITING_FOR_ROM_IMAGE :
                            NP_CLNT_READY));
}

bool8 S9xNPSendReady (uint8 op)
{
    uint8 ready [7];
    uint8 *ptr = ready;
    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = op;
    WRITE_LONG (ptr, 7);
    ptr += 4;

    if (!S9xNPSendData (NetPlay.Socket, ready, 7))
    {
	S9xNPDisconnect ();
        S9xNPSetError ("Sending 'READY' message failed.");
	return (FALSE);
    }

    return (TRUE);
}

bool8 S9xNPSendPause (bool8 paused)
{
#ifdef NP_DEBUG
    printf ("CLIENT: Pause - %s @%ld\n", paused ? "YES" : "NO", S9xGetMilliTime () - START);
#endif
    uint8 pause [7];
    uint8 *ptr = pause;
    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = NP_CLNT_PAUSE | (paused ? 0x80 : 0);
    WRITE_LONG (ptr, 7);
    ptr += 4;

    if (!S9xNPSendData (NetPlay.Socket, pause, 7))
    {
        S9xNPSetError ("Sending 'PAUSE' message failed.");
	S9xNPDisconnect ();
	return (FALSE);
    }

    return (TRUE);
}

#ifdef __WIN32__
void S9xNPClientLoop (void *)
{
    NetPlay.Waiting4EmulationThread = FALSE;

    if (S9xNPConnect ())
    {
        S9xClearPause (PAUSE_NETPLAY_CONNECT);
        while (NetPlay.Connected)
        {
            if (S9xNPWaitForHeartBeat ())
            {
                LONG prev;
                if (!ReleaseSemaphore (GUI.ClientSemaphore, 1, &prev))
                {
#ifdef NP_DEBUG
                    printf ("CLIENT: ReleaseSemaphore failed - already hit max count (%d) %ld\n", NP_JOYPAD_HIST_SIZE, S9xGetMilliTime () - START);
#endif
                    S9xNPSetWarning ("NetPlay: Client may be out of sync with server.");
                }
                else
                {
                    if (!NetPlay.Waiting4EmulationThread &&
                        prev == (int) NetPlay.MaxBehindFrameCount)
                    {
                        NetPlay.Waiting4EmulationThread = TRUE;
                        S9xNPSendPause (TRUE);
                    }
                }
            }
            else
                S9xNPDisconnect ();
        }
    }
    else
    {
        S9xClearPause (PAUSE_NETPLAY_CONNECT);
    }
#ifdef NP_DEBUG
    printf ("CLIENT: Client thread exiting @%ld\n", S9xGetMilliTime () - START);
#endif
}
#endif

bool8 S9xNPCheckForHeartBeat (uint32 time_msec)
{
    fd_set read_fds;
    struct timeval timeout;
    int res;
    int i;

    int max_fd = NetPlay.Socket;

    FD_ZERO (&read_fds);
    FD_SET (NetPlay.Socket, &read_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = time_msec * 1000;
    res = select (max_fd + 1, &read_fds, NULL, NULL, &timeout);

    i = (res > 0 && FD_ISSET(NetPlay.Socket, &read_fds));

#if defined(NP_DEBUG) && NP_DEBUG >= 4
    printf ("CLIENT: S9xCheckForHeartBeat %s @%ld\n", (i?"successful":"still waiting"), S9xGetMilliTime () - START);
#endif

    return i;
}

bool8 S9xNPWaitForHeartBeatDelay (uint32 time_msec)
{
    if (!S9xNPCheckForHeartBeat(time_msec))
        return FALSE;

    if (!S9xNPWaitForHeartBeat())
    {
        S9xNPDisconnect();
	return FALSE;
    }

    return TRUE;
}

bool8 S9xNPWaitForHeartBeat ()
{
    uint8 header [3 + 4 + 4 * 5];

    while (S9xNPGetData (NetPlay.Socket, header, 3 + 4))
    {
        if (header [0] != NP_SERV_MAGIC)
        {
            S9xNPSetError ("Bad magic value from server while waiting for heart-beat message\n");
            S9xNPDisconnect ();
            return (FALSE);
        }
        if (header [1] != NetPlay.ServerSequenceNum)
        {
            char buf [200];
            sprintf (buf, "Unexpected message sequence number from server, expected %d, got %d\n", NetPlay.ServerSequenceNum, header [1]);
            S9xNPSetWarning (buf);
            NetPlay.ServerSequenceNum = header [1] + 1;
        }
        else
            NetPlay.ServerSequenceNum++;

        if ((header [2] & 0x1f) == NP_SERV_JOYPAD)
        {
            // Top 2 bits + 1 of opcode is joypad data count.
            int num = (header [2] >> 6) + 1;

            if (num)
            {
                if (!S9xNPGetData (NetPlay.Socket, header + 3 + 4, num * 4))
                {
                    S9xNPSetError ("Error while receiving 'JOYPAD' message.");
                    S9xNPDisconnect ();
                    return (FALSE);
                }
            }
            NetPlay.Frame [NetPlay.JoypadWriteInd] = READ_LONG (&header [3]);

			int i;

			for (i = 0; i < num; i++)
                NetPlay.Joypads [NetPlay.JoypadWriteInd][i] = READ_LONG (&header [3 + 4 + i * sizeof (uint32)]);

			for (i = 0; i < NP_MAX_CLIENTS; i++)
				NetPlay.JoypadsReady [NetPlay.JoypadWriteInd][i] = TRUE;

			NetPlay.Paused = (header [2] & 0x20) != 0;

            NetPlay.JoypadWriteInd = (NetPlay.JoypadWriteInd + 1) % NP_JOYPAD_HIST_SIZE;

            if (NetPlay.JoypadWriteInd != (NetPlay.JoypadReadInd + 1) % NP_JOYPAD_HIST_SIZE)
            {
                //printf ("(%d)", (NetPlay.JoypadWriteInd - NetPlay.JoypadReadInd) % NP_JOYPAD_HIST_SIZE); fflush (stdout);
            }
//printf ("CLIENT: HB: @%d\n", S9xGetMilliTime () - START);
            return (TRUE);
        }
        else
        {
            uint32 len = READ_LONG (&header [3]);
	    switch (header [2] & 0x1f)
	    {
	    case NP_SERV_RESET:
#ifdef NP_DEBUG
                printf ("CLIENT: RESET received @%ld\n", S9xGetMilliTime () - START);
#endif
                S9xNPDiscardHeartbeats ();
		S9xReset ();
                NetPlay.FrameCount = READ_LONG (&header [3]);
                S9xNPResetJoypadReadPos ();
                S9xNPSendReady ();
                break;
	    case NP_SERV_PAUSE:
                NetPlay.Paused = (header [2] & 0x20) != 0;
				if (NetPlay.Paused)
					S9xNPSetWarning("CLIENT: Server has paused.");
				else
					S9xNPSetWarning("CLIENT: Server has resumed.");
                break;
            case NP_SERV_LOAD_ROM:
#ifdef NP_DEBUG
                printf ("CLIENT: LOAD_ROM received @%ld\n", S9xGetMilliTime () - START);
#endif
                S9xNPDiscardHeartbeats ();
                if (S9xNPLoadROM (len - 7))
                    S9xNPSendReady (NP_CLNT_LOADED_ROM);
                break;
            case NP_SERV_ROM_IMAGE:
#ifdef NP_DEBUG
                printf ("CLIENT: ROM_IMAGE received @%ld\n", S9xGetMilliTime () - START);
#endif
                S9xNPDiscardHeartbeats ();
                if (S9xNPGetROMImage (len - 7))
                    S9xNPSendReady (NP_CLNT_RECEIVED_ROM_IMAGE);
                break;
            case NP_SERV_SRAM_DATA:
#ifdef NP_DEBUG
                printf ("CLIENT: SRAM_DATA received @%ld\n", S9xGetMilliTime () - START);
#endif
                S9xNPDiscardHeartbeats ();
                S9xNPGetSRAMData (len - 7);
                break;
            case NP_SERV_FREEZE_FILE:
#ifdef NP_DEBUG
                printf ("CLIENT: FREEZE_FILE received @%ld\n", S9xGetMilliTime () - START);
#endif
                S9xNPDiscardHeartbeats ();
                S9xNPGetFreezeFile (len - 7);
                S9xNPResetJoypadReadPos ();
                S9xNPSendReady ();
                break;
            default:
#ifdef NP_DEBUG
                printf ("CLIENT: UNKNOWN received @%ld\n", S9xGetMilliTime () - START);
#endif
                S9xNPDisconnect ();
                return (FALSE);
	    }
	}
    }

    S9xNPDisconnect ();
    return (FALSE);
}

bool8 S9xNPLoadROMDialog (const char *rom_name)
{
    NetPlay.Answer = FALSE;

#ifdef __WIN32__
    ResetEvent (NetPlay.ReplyEvent);

#ifdef NP_DEBUG
    printf ("CLIENT: Asking GUI thread to open ROM load dialog...\n");
#endif

    PostMessage (GUI.hWnd, WM_USER + 3, (uint32) rom_name, (uint32) rom_name);

#ifdef NP_DEBUG
    printf ("CLIENT: Waiting for reply from GUI thread...\n");
#endif

    WaitForSingleObject (NetPlay.ReplyEvent, INFINITE);

#ifdef NP_DEBUG
    printf ("CLIENT: Got reply from GUI thread (%d)\n", NetPlay.Answer);
#endif

#else
	NetPlay.Answer = TRUE;
#endif

    return (NetPlay.Answer);
}

bool8 S9xNPLoadROM (uint32 len)
{
    uint8 *data = new uint8 [len];

    S9xNPSetAction ("Receiving ROM name...");
    if (!S9xNPGetData (NetPlay.Socket, data, len))
    {
        S9xNPSetError ("Error while receiving ROM name.");
        delete data;
        S9xNPDisconnect ();
        return (FALSE);
    }

    S9xNPSetAction ("Opening LoadROM dialog...");
    if (!S9xNPLoadROMDialog ((char *) data))
    {
        S9xNPSetError ("Disconnected from NetPlay server because you are playing a different game!");
        delete data;
        S9xNPDisconnect ();
        return (FALSE);
    }
    delete data;
    return (TRUE);
}

bool8 S9xNPGetROMImage (uint32 len)
{
    uint8 rom_info [5];

    S9xNPSetAction ("Receiving ROM information...");
    if (!S9xNPGetData (NetPlay.Socket, rom_info, 5))
    {
        S9xNPSetError ("Error while receiving ROM information.");
        S9xNPDisconnect ();
        return (FALSE);
    }
    uint32 CalculatedSize = READ_LONG (&rom_info [1]);
#ifdef NP_DEBUG
    printf ("CLIENT: Hi-ROM: %s, Size: %04x\n", rom_info [0] ? "Y" : "N", CalculatedSize);
#endif
    if (CalculatedSize + 5 >= len ||
        CalculatedSize >= CMemory::MAX_ROM_SIZE)
    {
        S9xNPSetError ("Size error in ROM image data received from server.");
        S9xNPDisconnect ();
        return (FALSE);
    }

    Memory.HiROM = rom_info [0];
    Memory.LoROM = !Memory.HiROM;
    Memory.HeaderCount = 0;
    Memory.CalculatedSize = CalculatedSize;

    // Load up ROM image
#ifdef NP_DEBUG
    printf ("CLIENT: Receiving ROM image @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Receiving ROM image...");
    if (!S9xNPGetData (NetPlay.Socket, Memory.ROM, Memory.CalculatedSize))
    {
        S9xNPSetError ("Error while receiving ROM image from server.");
        Settings.StopEmulation = TRUE;
        S9xNPDisconnect ();
        return (FALSE);
    }
#ifdef NP_DEBUG
    printf ("CLIENT: Receiving ROM filename @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Receiving ROM filename...");
    uint32 filename_len = len - Memory.CalculatedSize - 5;
    if (filename_len > PATH_MAX ||
        !S9xNPGetData (NetPlay.Socket, (uint8 *) Memory.ROMFilename, filename_len))
    {
        S9xNPSetError ("Error while receiving ROM filename from server.");
        S9xNPDisconnect ();
        Settings.StopEmulation = TRUE;
        return (FALSE);
    }
    Memory.InitROM ();
    S9xReset ();
    S9xNPResetJoypadReadPos ();
    Settings.StopEmulation = FALSE;

#ifdef __WIN32__
    PostMessage (GUI.hWnd, WM_NULL, 0, 0);
#endif

    return (TRUE);
}

void S9xNPGetSRAMData (uint32 len)
{
    if (len > 0x10000)
    {
        S9xNPSetError ("Length error in S-RAM data received from server.");
        S9xNPDisconnect ();
        return;
    }
    S9xNPSetAction ("Receiving S-RAM data...");
    if (len > 0 && !S9xNPGetData (NetPlay.Socket, Memory.SRAM, len))
    {
        S9xNPSetError ("Error while receiving S-RAM data from server.");
        S9xNPDisconnect ();
    }
	S9xNPSetAction ("", TRUE);
}

void S9xNPGetFreezeFile (uint32 len)
{
    uint8 frame_count [4];

#ifdef NP_DEBUG
    printf ("CLIENT: Receiving freeze file information @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Receiving freeze file information...");
    if (!S9xNPGetData (NetPlay.Socket, frame_count, 4))
    {
        S9xNPSetError ("Error while receiving freeze file information from server.");
        S9xNPDisconnect ();
        return;
    }
    NetPlay.FrameCount = READ_LONG (frame_count);

#ifdef NP_DEBUG
    printf ("CLIENT: Receiving freeze file @%ld...\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("Receiving freeze file...");
    uint8 *data = new uint8 [len];
    if (!S9xNPGetData (NetPlay.Socket, data, len - 4))
    {
        S9xNPSetError ("Error while receiving freeze file from server.");
        S9xNPDisconnect ();
        delete data;
        return;
    }
	S9xNPSetAction ("", TRUE);

    //FIXME: Setting umask here wouldn't hurt.
    FILE *file;
#ifdef HAVE_MKSTEMP
    int fd;
    char fname[] = "/tmp/snes9x_fztmpXXXXXX";
    if ((fd = mkstemp(fname)) >= 0)
    {
        if ((file = fdopen (fd, "wb")))
#else
    char fname [L_tmpnam];
    if (tmpnam (fname))
    {
        if ((file = fopen (fname, "wb")))
#endif
        {
            if (fwrite (data, 1, len, file) == len)
            {
                fclose(file);
#ifndef __WIN32__
		/* We need .s96 extension, else .s96 is addded by unix code */
                char buf[PATH_MAX +1 ];

                strncpy(buf, fname, PATH_MAX);
                strcat(buf, ".s96");

                rename(fname, buf);

                if (!S9xUnfreezeGame (buf))
#else
                if (!S9xUnfreezeGame (fname))
#endif
                    S9xNPSetError ("Unable to load freeze file just received.");
            } else {
                S9xNPSetError ("Failed to write to temporary freeze file.");
                fclose(file);
            }
        } else
            S9xNPSetError ("Failed to create temporary freeze file.");
        remove (fname);
    } else
        S9xNPSetError ("Unable to get name for temporary freeze file.");
    delete data;
}

uint32 S9xNPGetJoypad (int which1)
{
    if (Settings.NetPlay && which1 < 8)
	{
#ifdef NP_DEBUG
		if(!NetPlay.JoypadsReady [NetPlay.JoypadReadInd][which1])
		{
            S9xNPSetWarning ("Missing input from server!");
		}
#endif
		NetPlay.JoypadsReady [NetPlay.JoypadReadInd][which1] = FALSE;

		return (NetPlay.Joypads [NetPlay.JoypadReadInd][which1]);
	}

    return (0);
}

void S9xNPStepJoypadHistory ()
{
    if ((NetPlay.JoypadReadInd + 1) % NP_JOYPAD_HIST_SIZE != NetPlay.JoypadWriteInd)
    {
        NetPlay.JoypadReadInd = (NetPlay.JoypadReadInd + 1) % NP_JOYPAD_HIST_SIZE;
        if (NetPlay.FrameCount != NetPlay.Frame [NetPlay.JoypadReadInd])
        {
            S9xNPSetWarning ("This Snes9X session may be out of sync with the server.");
#ifdef NP_DEBUG
            printf ("*** CLIENT: client out of sync with server (%d, %d) @%ld\n", NetPlay.FrameCount, NetPlay.Frame [NetPlay.JoypadReadInd], S9xGetMilliTime () - START);
#endif
        }
    }
    else
    {
#ifdef NP_DEBUG
        printf ("*** CLIENT: S9xNPStepJoypadHistory NOT OK@%ld\n", S9xGetMilliTime () - START);
#endif
    }
}


void S9xNPResetJoypadReadPos ()
{
#ifdef NP_DEBUG
    printf ("CLIENT: ResetJoyReadPos @%ld\n", S9xGetMilliTime () - START); fflush (stdout);
#endif
    NetPlay.JoypadWriteInd = 0;
    NetPlay.JoypadReadInd = NP_JOYPAD_HIST_SIZE - 1;
    for (int h = 0; h < NP_JOYPAD_HIST_SIZE; h++)
        memset ((void *) &NetPlay.Joypads [h], 0, sizeof (NetPlay.Joypads [0]));
    for (int h = 0; h < NP_JOYPAD_HIST_SIZE; h++)
        memset ((void *) &NetPlay.JoypadsReady [h], 0, sizeof (NetPlay.JoypadsReady [0]));
}

bool8 S9xNPSendJoypadUpdate (uint32 joypad)
{
    uint8 data [7];
    uint8 *ptr = data;

    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = NP_CLNT_JOYPAD;

    joypad |= 0x80000000;

    WRITE_LONG (ptr, joypad);
    if (!S9xNPSendData (NetPlay.Socket, data, 7))
    {
        S9xNPSetError ("Error while sending joypad data server.");
	S9xNPDisconnect ();
	return (FALSE);
    }
    return (TRUE);
}

void S9xNPDisconnect ()
{
    close (NetPlay.Socket);
    NetPlay.Socket = -1;
    NetPlay.Connected = FALSE;
    Settings.NetPlay = FALSE;
}

bool8 S9xNPSendData (int socket, const uint8 *data, int length)
{
    int len = length;
    const uint8 *ptr = data;

    NetPlay.PercentageComplete = 0;

    do
    {
        if (NetPlay.Abort)
            return (FALSE);

        int num_bytes = len;

        // Write the data in small chunks, allowing this thread to spot an
        // abort request from another thread.
        if (num_bytes > 512)
            num_bytes = 512;

	int sent = write (socket, (char *) ptr, num_bytes);
	if (sent < 0)
	{
	    if (errno == EINTR
#ifdef EAGAIN
		|| errno == EAGAIN
#endif
#ifdef EWOULDBLOCK
		|| errno == EWOULDBLOCK
#endif
		)
            {
#ifdef NP_DEBUG
                printf ("CLIENT: EINTR, EAGAIN or EWOULDBLOCK while sending data @%ld\n", S9xGetMilliTime () - START);
#endif
		continue;
            }
	    return (FALSE);
	}
	else
	if (sent == 0)
	    return (FALSE);
	len -= sent;
	ptr += sent;

        NetPlay.PercentageComplete = (uint8) (((length - len) * 100) / length);
    } while (len > 0);

    return (TRUE);
}

bool8 S9xNPGetData (int socket, uint8 *data, int length)
{
    int len = length;
    uint8 *ptr = data;
    int chunk = length / 50;

    if (chunk < 1024)
        chunk = 1024;

    NetPlay.PercentageComplete = 0;
    do
    {
        if (NetPlay.Abort)
            return (FALSE);

        int num_bytes = len;

        // Read the data in small chunks, allowing this thread to spot an
        // abort request from another thread.
        if (num_bytes > chunk)
            num_bytes = chunk;

        int got = read (socket, (char *) ptr, num_bytes);
        if (got < 0)
        {
	    if (errno == EINTR
#ifdef EAGAIN
		|| errno == EAGAIN
#endif
#ifdef EWOULDBLOCK
		|| errno == EWOULDBLOCK
#endif
#ifdef WSAEWOULDBLOCK
                || errno == WSAEWOULDBLOCK
#endif
		)
            {
#ifdef NP_DEBUG
                printf ("CLIENT: EINTR, EAGAIN or EWOULDBLOCK while receiving data @%ld\n", S9xGetMilliTime () - START);
#endif
		continue;
            }
#ifdef WSAEMSGSIZE
            if (errno != WSAEMSGSIZE)
                return (FALSE);
            else
            {
                got = num_bytes;
#ifdef NP_DEBUG
                printf ("CLIENT: WSAEMSGSIZE, actual bytes %d while receiving data @%ld\n", got, S9xGetMilliTime () - START);
#endif
            }
#else
            return (FALSE);
#endif
        }
        else
        if (got == 0)
            return (FALSE);

        len -= got;
        ptr += got;

        if (!Settings.NetPlayServer && length > 1024)
        {
            NetPlay.PercentageComplete = (uint8) (((length - len) * 100) / length);
#ifdef __WIN32__
            PostMessage (GUI.hWnd, WM_USER, NetPlay.PercentageComplete,
                         NetPlay.PercentageComplete);
            Sleep (0);
#endif
        }

    } while (len > 0);

    return (TRUE);
}

bool8 S9xNPInitialise ()
{
#ifdef __WIN32__
    static bool8 initialised = FALSE;

    if (!initialised)
    {
        initialised = TRUE;
        WSADATA data;

#ifdef NP_DEBUG
        START = S9xGetMilliTime ();

        printf ("CLIENT/SERVER: Initialising WinSock @%ld\n", S9xGetMilliTime () - START);
#endif
        S9xNPSetAction ("Initialising Windows sockets interface...");
        if (WSAStartup (MAKEWORD (1, 0), &data) != 0)
        {
            S9xNPSetError ("Call to init Windows sockets failed. Do you have WinSock2 installed?");
            return (FALSE);
        }
    }
#endif
    return (TRUE);
}

void S9xNPDiscardHeartbeats ()
{
    // Discard any pending heartbeats and wait for any frame that is currently
    // being emulated to complete.
#ifdef NP_DEBUG
    printf ("CLIENT: DiscardHeartbeats @%ld, finished @", S9xGetMilliTime () - START);
    fflush (stdout);
#endif

#ifdef __WIN32__
    while (WaitForSingleObject (GUI.ClientSemaphore, 200) == WAIT_OBJECT_0)
        ;
#endif

#ifdef NP_DEBUG
    printf ("%ld\n", S9xGetMilliTime () - START);
#endif
    NetPlay.Waiting4EmulationThread = FALSE;
}

void S9xNPSetAction (const char *action, bool8 force)
{
#ifdef NP_DEBUG
    printf ("NPSetAction: %s, forced = %d %ld\n", action, force, S9xGetMilliTime () - START);
#endif
    if (force || !Settings.NetPlayServer)
    {
        strncpy (NetPlay.ActionMsg, action, NP_MAX_ACTION_LEN - 1);
        NetPlay.ActionMsg [NP_MAX_ACTION_LEN - 1] = 0;
#ifdef __WIN32__
        PostMessage (GUI.hWnd, WM_USER, 0, 0);
        Sleep (0);
#endif
    }
}

void S9xNPSetError (const char *error)
{
#if defined(NP_DEBUG) && NP_DEBUG == 2
    printf("ERROR: %s\n", error);
    fflush (stdout);
#endif
    strncpy (NetPlay.ErrorMsg, error, NP_MAX_ACTION_LEN - 1);
    NetPlay.ErrorMsg [NP_MAX_ACTION_LEN - 1] = 0;
#ifdef __WIN32__
    PostMessage (GUI.hWnd, WM_USER + 1, 0, 0);
    Sleep (0);
#endif
}

void S9xNPSetWarning (const char *warning)
{
#if defined(NP_DEBUG) && NP_DEBUG == 3
    printf("Warning: %s\n", warning);
    fflush (stdout);
#endif
    strncpy (NetPlay.WarningMsg, warning, NP_MAX_ACTION_LEN - 1);
    NetPlay.WarningMsg [NP_MAX_ACTION_LEN - 1] = 0;
#ifdef __WIN32__
    PostMessage (GUI.hWnd, WM_USER + 2, 0, 0);
    Sleep (0);
#endif
}
#endif
