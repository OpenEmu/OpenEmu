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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#ifdef HAVE_STRINGS_H
	#include <strings.h>
#endif

#ifdef __WIN32__

	#include <winsock.h>
	#include <process.h>
	#include "win32/wsnes9x.h"
	#define ioctl ioctlsocket
	#define close closesocket
	#define read(a,b,c) recv(a, b, c, 0)
	#define write(a,b,c) send(a, b, c, 0)
	#define gettimeofday(a,b) S9xGetTimeOfDay (a)
	#define exit(a) _endthread()
	void S9xGetTimeOfDay (struct timeval *n);
#else
	#include <unistd.h>
	#include <sys/time.h>

	#include <netdb.h>
	#include <sys/socket.h>
	#include <sys/param.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <signal.h>

	#ifdef __SVR4
		#include <sys/stropts.h>
	#endif

#endif // !__WIN32__

#include "snes9x.h"
#include "memmap.h"
#include "snapshot.h"
#include "netplay.h"

#ifdef __WIN32__
#define NP_ONE_CLIENT 1
#else
#define NP_ONE_CLIENT 0
#endif

struct SNPServer NPServer;

extern unsigned long START;

void S9xNPSendToAllClients (uint8 *data, int len);
bool8 S9xNPLoadFreezeFile (const char *fname, uint8 *&data, uint32 &len);
void S9xNPSendFreezeFile (int c, uint8 *data, uint32 len);
void S9xNPNoClientReady (int start_index = NP_ONE_CLIENT);
void S9xNPRecomputePause ();
void S9xNPWaitForEmulationToComplete ();
void S9xNPSendROMImageToAllClients ();
bool8 S9xNPSendROMImageToClient (int client);
void S9xNPSendSRAMToClient (int c);
void S9xNPSendSRAMToAllClients ();
void S9xNPSyncClient (int);
void S9xNPSendROMLoadRequest (const char *filename);
void S9xNPSendFreezeFileToAllClients (const char *filename);
void S9xNPStopServer ();

void S9xNPShutdownClient (int c, bool8 report_error = FALSE)
{
    if (NPServer.Clients [c].Connected)
    {
        NPServer.Clients [c].Connected = FALSE;
        NPServer.Clients [c].SaidHello = FALSE;

        close (NPServer.Clients [c].Socket);
#ifdef NP_DEBUG
        printf ("SERVER: Player %d disconnecting @%ld\n", c + 1, S9xGetMilliTime () - START);
#endif
        if (report_error)
        {
            sprintf (NetPlay.ErrorMsg,
                     "Player %d on '%s' has disconnected.", c + 1,
                     NPServer.Clients [c].HostName);
            S9xNPSetWarning  (NetPlay.ErrorMsg);
        }

        if (NPServer.Clients [c].HostName)
        {
            free ((char *) NPServer.Clients [c].HostName);
            NPServer.Clients [c].HostName = NULL;
        }
        if (NPServer.Clients [c].ROMName)
        {
            free ((char *) NPServer.Clients [c].ROMName);
            NPServer.Clients [c].ROMName = NULL;
        }
        if (NPServer.Clients [c].Who)
        {
            free ((char *) NPServer.Clients [c].Who);
            NPServer.Clients [c].Who = NULL;
        }
        NPServer.Joypads [c] = 0;
        NPServer.NumClients--;
        S9xNPRecomputePause ();
    }
}

static bool8 S9xNPSGetData (int socket, uint8 *data, int length)
{
    int len = length;
    uint8 *ptr = data;

    do
    {
        int num_bytes = len;

        // Read the data in small chunks, allowing this thread to spot an
        // abort request from another thread.
        if (num_bytes > 512)
            num_bytes = 512;

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
		continue;
#ifdef WSAEMSGSIZE
            if (errno != WSAEMSGSIZE)
                return (FALSE);
            else
            {
                got = num_bytes;
#ifdef NP_DEBUG
                printf ("SERVER: WSAEMSGSIZE, actual bytes %d while receiving data @%d\n", got, S9xGetMilliTime () - START);
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
    } while (len > 0);

    return (TRUE);
}

static bool8 S9xNPSSendData (int fd, const uint8 *data, int length)
{
    int Percent = 0;
    int len = length;
    int chunk = length / 50;

    if (chunk < 1024)
        chunk = 1024;

    do
    {
        int num_bytes = len;

        // Write the data in small chunks, allowing this thread to spot an
        // abort request from another thread.
        if (num_bytes > chunk)
            num_bytes = chunk;

	int sent;
	sent = write (fd, (char *) data, len);

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
                printf ("SERVER: EINTR, EAGAIN or EWOULDBLOCK while sending data @%ld\n", S9xGetMilliTime () - START);
#endif
		continue;
            }
	    return (FALSE);
	}
	else
	if (sent == 0)
	    return (FALSE);
	len -= sent;
	data += sent;
        if (length > 1024)
        {
            Percent = (uint8) (((length - len) * 100) / length);
#ifdef __WIN32__
            PostMessage (GUI.hWnd, WM_USER, Percent, Percent);
            Sleep (0);
#endif
        }
    } while (len > 0);

    return (TRUE);
}

void S9xNPSendHeartBeat ()
{
    int len = 3;
    uint8 data [3 + 4 * 5];
    uint8 *ptr = data;
    int n;

    for (n = NP_MAX_CLIENTS - 1; n >= 0; n--)
    {
        if (NPServer.Clients [n].SaidHello)
            break;
    }

    if (n >= 0)
    {
        bool8 Paused = NPServer.Paused != 0;

        NPServer.FrameCount++;
        *ptr++ = NP_SERV_MAGIC;
        *ptr++ = 0; // Individual client sequence number will get placed here
        *ptr++ = NP_SERV_JOYPAD | (n << 6) | ((Paused != 0) << 5);

        WRITE_LONG (ptr, NPServer.FrameCount);
        len += 4;
        ptr += 4;

        int i;

        for (i = 0; i <= n; i++)
        {
            WRITE_LONG (ptr, NPServer.Joypads [i]);
            len += 4;
            ptr += 4;
        }

        S9xNPSendToAllClients (data, len);
    }
}

void S9xNPSendToAllClients (uint8 *data, int len)
{
    int i;

    for (i = 0; i < NP_MAX_CLIENTS; i++)
    {
	if (NPServer.Clients [i].SaidHello)
	{
            data [1] = NPServer.Clients [i].SendSequenceNum++;
	    if (!S9xNPSSendData (NPServer.Clients [i].Socket, data, len))
		S9xNPShutdownClient (i, TRUE);
	}
    }
}

void S9xNPProcessClient (int c)
{
    uint8 header [7];
    uint8 *data;
    uint32 len;
    uint8 *ptr;

    if (!S9xNPSGetData (NPServer.Clients [c].Socket, header, 7))
    {
        S9xNPSetWarning ("SERVER: Failed to get message header from client.\n");
        S9xNPShutdownClient (c, TRUE);
        return;
    }
    if (header [0] != NP_CLNT_MAGIC)
    {
        S9xNPSetWarning ("SERVER: Bad header magic value received from client.\n");
        S9xNPShutdownClient (c, TRUE);
        return;
    }

    if (header [1] != NPServer.Clients [c].ReceiveSequenceNum)
    {
#ifdef NP_DEBUG
        printf ("SERVER: Messages lost from '%s', expected %d, got %d\n",
                NPServer.Clients [c].HostName ?
                NPServer.Clients [c].HostName : "Unknown",
                NPServer.Clients [c].ReceiveSequenceNum,
                header [1]);
#endif
        sprintf (NetPlay.WarningMsg,
                 "SERVER: Messages lost from '%s', expected %d, got %d\n",
                NPServer.Clients [c].HostName ?
                NPServer.Clients [c].HostName : "Unknown",
                NPServer.Clients [c].ReceiveSequenceNum,
                header [1]);
        NPServer.Clients [c].ReceiveSequenceNum = header [1] + 1;
        S9xNPSetWarning (NetPlay.WarningMsg);
    }
    else
        NPServer.Clients [c].ReceiveSequenceNum++;

    len = READ_LONG (&header [3]);

    switch (header [2] & 0x3f)
    {
        case NP_CLNT_HELLO:
#ifdef NP_DEBUG
            printf ("SERVER: Got HELLO from client @%ld\n", S9xGetMilliTime () - START);
#endif
            S9xNPSetAction ("Got HELLO from client...", TRUE);
            if (len > 0x10000)
            {
                S9xNPSetWarning ("SERVER: Client HELLO message length error.");
                S9xNPShutdownClient (c, TRUE);
                return;
            }
            data = new uint8 [len - 7];
            if (!S9xNPSGetData (NPServer.Clients [c].Socket, data, len - 7))
            {
                S9xNPSetWarning ("SERVER: Failed to get HELLO message content from client.");
                S9xNPShutdownClient (c, TRUE);
                return;
            }

            if (NPServer.NumClients <= NP_ONE_CLIENT)
            {
		NPServer.FrameTime = READ_LONG (data);
		strncpy (NPServer.ROMName, (char *) &data [4], 29);
		NPServer.ROMName [29] = 0;
            }

            NPServer.Clients [c].ROMName = strdup ((char *) &data [4]);
#ifdef NP_DEBUG
            printf ("SERVER: Client is playing: %s, Frame Time: %d @%ld\n", data + 4, READ_LONG (data), S9xGetMilliTime () - START);
#endif

            NPServer.Clients [c].SendSequenceNum = 0;

            len = 7 + 1 + 1 + 4 + strlen (NPServer.ROMName) + 1;

            delete data;
            ptr = data = new uint8 [len];
            *ptr++ = NP_SERV_MAGIC;
            *ptr++ = NPServer.Clients [c].SendSequenceNum++;

            if (NPServer.SendROMImageOnConnect &&
                NPServer.NumClients > NP_ONE_CLIENT)
                *ptr++ = NP_SERV_HELLO | 0x80;
            else
                *ptr++ = NP_SERV_HELLO;
            WRITE_LONG (ptr, len);
            ptr += 4;
            *ptr++ = NP_VERSION;
            *ptr++ = c + 1;
            WRITE_LONG (ptr, NPServer.FrameCount);
            ptr += 4;
            strcpy ((char *) ptr, NPServer.ROMName);

#ifdef NP_DEBUG
            printf ("SERVER: Sending welcome information to client @%ld...\n", S9xGetMilliTime () - START);
#endif
            S9xNPSetAction ("SERVER: Sending welcome information to new client...", TRUE);
            if (!S9xNPSSendData (NPServer.Clients [c].Socket, data, len))
            {
                S9xNPSetWarning ("SERVER: Failed to send welcome message to client.");
                S9xNPShutdownClient (c, TRUE);
                return;
            }
            delete data;
#ifdef NP_DEBUG
            printf ("SERVER: Waiting for a response from the client @%ld...\n", S9xGetMilliTime () - START);
#endif
            S9xNPSetAction ("SERVER: Waiting for a response from the client...", TRUE);
            break;

        case NP_CLNT_LOADED_ROM:
#ifdef NP_DEBUG
            printf ("SERVER: Client %d loaded requested ROM @%ld...\n", c, S9xGetMilliTime () - START);
#endif
            NPServer.Clients [c].SaidHello = TRUE;
            NPServer.Clients [c].Ready = FALSE;
            NPServer.Clients [c].Paused = FALSE;
            S9xNPRecomputePause ();
            S9xNPWaitForEmulationToComplete ();

            if (NPServer.SyncByReset)
            {
                S9xNPServerAddTask (NP_SERVER_SEND_SRAM, (void *) c);
                S9xNPServerAddTask (NP_SERVER_RESET_ALL, 0);
            }
            else
                S9xNPServerAddTask (NP_SERVER_SYNC_CLIENT, (void *) c);
            break;

        case NP_CLNT_RECEIVED_ROM_IMAGE:
#ifdef NP_DEBUG
            printf ("SERVER: Client %d received ROM image @%ld...\n", c, S9xGetMilliTime () - START);
#endif
            NPServer.Clients [c].SaidHello = TRUE;
            NPServer.Clients [c].Ready = FALSE;
            NPServer.Clients [c].Paused = FALSE;
            S9xNPRecomputePause ();
            S9xNPWaitForEmulationToComplete ();

            if (NPServer.SyncByReset)
            {
                S9xNPServerAddTask (NP_SERVER_SEND_SRAM, (void *) c);
                S9xNPServerAddTask (NP_SERVER_RESET_ALL, 0);
            }
            else
                S9xNPServerAddTask (NP_SERVER_SYNC_CLIENT, (void *) c);

            break;

        case NP_CLNT_WAITING_FOR_ROM_IMAGE:
#ifdef NP_DEBUG
            printf ("SERVER: Client %d waiting for ROM image @%ld...\n", c, S9xGetMilliTime () - START);
#endif
            NPServer.Clients [c].SaidHello = TRUE;
            NPServer.Clients [c].Ready = FALSE;
            NPServer.Clients [c].Paused = FALSE;
            S9xNPRecomputePause ();
            S9xNPSendROMImageToClient (c);
            break;

        case NP_CLNT_READY:
#ifdef NP_DEBUG
            printf ("SERVER: Client %d ready @%ld...\n", c, S9xGetMilliTime () - START);
#endif
            if (NPServer.Clients [c].SaidHello)
            {
                NPServer.Clients [c].Paused = FALSE;
                NPServer.Clients [c].Ready = TRUE;

                S9xNPRecomputePause ();
                break;
            }
            NPServer.Clients [c].SaidHello = TRUE;
            NPServer.Clients [c].Ready = TRUE;
            NPServer.Clients [c].Paused = FALSE;
            S9xNPRecomputePause ();

//printf ("SERVER: SaidHello = TRUE, SeqNum = %d @%d\n", NPServer.Clients [c].SendSequenceNum, S9xGetMilliTime () - START);
            if (NPServer.NumClients > NP_ONE_CLIENT)
            {
                if (!NPServer.SendROMImageOnConnect)
                {
                    S9xNPWaitForEmulationToComplete ();

                    if (NPServer.SyncByReset)
                    {
                        S9xNPServerAddTask (NP_SERVER_SEND_SRAM, (void *) c);
                        S9xNPServerAddTask (NP_SERVER_RESET_ALL, 0);
                    }
                    else
#ifdef __WIN32__
                        S9xNPServerAddTask (NP_SERVER_SYNC_CLIENT, (void *) c);
#else
                        /* We need to resync all clients on new player connect as we don't have a 'reference game' */
                        S9xNPServerAddTask (NP_SERVER_SYNC_ALL, (void *) c);
#endif
                }
            }
            else
            {
                NPServer.Clients [c].Ready = TRUE;
                S9xNPRecomputePause ();
            }
            break;
        case NP_CLNT_JOYPAD:
            NPServer.Joypads [c] = len;
            break;
        case NP_CLNT_PAUSE:
#ifdef NP_DEBUG
            printf ("SERVER: Client %d Paused: %s @%ld\n", c, (header [2] & 0x80) ? "YES" : "NO", S9xGetMilliTime () - START);
#endif
            NPServer.Clients [c].Paused = (header [2] & 0x80) != 0;
            if (NPServer.Clients [c].Paused)
                sprintf (NetPlay.WarningMsg, "SERVER: Client %d has paused.", c + 1);
            else
                sprintf (NetPlay.WarningMsg, "SERVER: Client %d has resumed.", c + 1);
            S9xNPSetWarning (NetPlay.WarningMsg);
            S9xNPRecomputePause ();
            break;
    }
}

void S9xNPAcceptClient (int Listen, bool8 block)
{
    struct sockaddr_in remote_address;
    struct linger val2;
    struct hostent *host;
    int new_fd;
    int i;

#ifdef NP_DEBUG
    printf ("SERVER: attempting to accept new client connection @%ld\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("SERVER: Attempting to accept client connection...", TRUE);
    memset (&remote_address, 0, sizeof (remote_address));
    socklen_t len = sizeof (remote_address);

    new_fd = accept (Listen, (struct sockaddr *)&remote_address, &len);

    S9xNPSetAction ("Setting socket options...", TRUE);
    val2.l_onoff = 1;
    val2.l_linger = 0;
    if (setsockopt (new_fd, SOL_SOCKET, SO_LINGER,
		    (char *) &val2, sizeof (val2)) < 0)
    {
        S9xNPSetError ("Setting socket options failed.");
	close (new_fd);
        return;
    }

    for (i = 0; i < NP_MAX_CLIENTS; i++)
    {
	if (!NPServer.Clients [i].Connected)
	{
            NPServer.NumClients++;
	    NPServer.Clients [i].Socket = new_fd;
            NPServer.Clients [i].SendSequenceNum = 0;
            NPServer.Clients [i].ReceiveSequenceNum = 0;
            NPServer.Clients [i].Connected = TRUE;
            NPServer.Clients [i].SaidHello = FALSE;
            NPServer.Clients [i].Paused = FALSE;
            NPServer.Clients [i].Ready = FALSE;
            NPServer.Clients [i].ROMName = NULL;
            NPServer.Clients [i].HostName = NULL;
            NPServer.Clients [i].Who = NULL;
	    break;
	}
    }

    if (i >= NP_MAX_CLIENTS)
    {
        S9xNPSetError ("SERVER: Maximum number of NetPlay Clients have already connected.");
	close (new_fd);
	return;
    }

    if (remote_address.sin_family == AF_INET)
    {
#ifdef NP_DEBUG
        printf ("SERVER: Looking up new client's hostname @%ld\n", S9xGetMilliTime () - START);
#endif
        S9xNPSetAction ("SERVER: Looking up new client's hostname...", TRUE);
	host = gethostbyaddr ((char *) &remote_address.sin_addr,
			      sizeof (remote_address.sin_addr), AF_INET);

	if (host)
	{
#ifdef NP_DEBUG
            printf ("SERVER: resolved new client's hostname (%s) @%ld\n", host->h_name, S9xGetMilliTime () - START);
#endif
	    sprintf (NetPlay.WarningMsg, "SERVER: Player %d on %s has connected.", i + 1, host->h_name);
	    NPServer.Clients [i].HostName = strdup (host->h_name);
	}
        else
        {
            char *ip = inet_ntoa (remote_address.sin_addr);
            if (ip)
                NPServer.Clients [i].HostName = strdup (ip);
#ifdef NP_DEBUG
            printf ("SERVER: couldn't resolve new client's hostname (%s) @%ld\n", ip ? ip : "Unknown", S9xGetMilliTime () - START);
#endif
	    sprintf (NetPlay.WarningMsg, "SERVER: Player %d on %s has connected.", i + 1, ip ? ip : "Unknown");
        }
        S9xNPSetWarning (NetPlay.WarningMsg);
    }
#ifdef NP_DEBUG
    printf ("SERVER: waiting for HELLO message from new client @%ld\n", S9xGetMilliTime () - START);
#endif
    S9xNPSetAction ("SERVER: Waiting for HELLO message from new client...");
}

static bool8 server_continue = TRUE;

static bool8 S9xNPServerInit (int port)
{
    struct sockaddr_in address;
    int i;
    int val;

    if (!S9xNPInitialise ())
        return (FALSE);

    for (i = 0; i < NP_MAX_CLIENTS; i++)
    {
        NPServer.Clients [i].SendSequenceNum = 0;
        NPServer.Clients [i].ReceiveSequenceNum = 0;
        NPServer.Clients [i].Connected = FALSE;
        NPServer.Clients [i].SaidHello = FALSE;
        NPServer.Clients [i].Paused = FALSE;
        NPServer.Clients [i].Ready = FALSE;
        NPServer.Clients [i].Socket = 0;
        NPServer.Clients [i].ROMName = NULL;
        NPServer.Clients [i].HostName = NULL;
        NPServer.Clients [i].Who = NULL;
        NPServer.Joypads [i] = 0;
    }

    NPServer.NumClients = 0;
    NPServer.FrameCount = 0;

#ifdef NP_DEBUG
    printf ("SERVER: Creating socket @%ld\n", S9xGetMilliTime () - START);
#endif
    if ((NPServer.Socket = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
	S9xNPSetError ("NetPlay Server: Can't create listening socket.");
	return (FALSE);
    }

    val = 1;
    setsockopt (NPServer.Socket, SOL_SOCKET, SO_REUSEADDR,
                (char *)&val, sizeof (val));

    memset (&address, 0, sizeof (address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl (INADDR_ANY);
    address.sin_port = htons (port);

#ifdef NP_DEBUG
    printf ("SERVER: Binding socket to address and port @%ld\n", S9xGetMilliTime () - START);
#endif
    if (bind (NPServer.Socket, (struct sockaddr *) &address, sizeof (address)) < 0)
    {
	S9xNPSetError ("NetPlay Server: Can't bind socket to port number.\nPort already in use?");
	return (FALSE);
    }

#ifdef NP_DEBUG
    printf ("SERVER: Getting socket to listen @%ld\n", S9xGetMilliTime () - START);
#endif
    if (listen (NPServer.Socket, NP_MAX_CLIENTS) < 0)
    {
	S9xNPSetError ("NetPlay Server: Can't get new socket to listen.");
	return (FALSE);
    }

#ifdef NP_DEBUG
    printf ("SERVER: Init complete @%ld\n", S9xGetMilliTime () - START);
#endif
    return (TRUE);
}

void S9xNPSendServerPause (bool8 paused)
{
#ifdef NP_DEBUG
    printf ("SERVER: Pause - %s @%ld\n", paused ? "YES" : "NO", S9xGetMilliTime () - START);
#endif
    uint8 pause [7];
    uint8 *ptr = pause;
    *ptr++ = NP_SERV_MAGIC;
    *ptr++ = 0;
    *ptr++ = NP_SERV_PAUSE | (paused ? 0x20 : 0);
    WRITE_LONG (ptr, NPServer.FrameCount);
    S9xNPSendToAllClients (pause, 7);
}

void S9xNPServerLoop (void *)
{
#ifdef __WIN32__
    BOOL success = FALSE;
#else
    bool8 success = FALSE;
    static struct timeval next1 = {0, 0};
    struct timeval now;
#endif

	int pausedState = -1, newPausedState = -1;

    while (server_continue)
    {
        fd_set read_fds;
        struct timeval timeout;
        int res;
        int i;

        int max_fd = NPServer.Socket;

#ifdef __WIN32__
        Sleep (0);
#endif

        if (success && !(Settings.Paused && !Settings.FrameAdvance) && !Settings.StopEmulation &&
            !Settings.ForcedPause && !NPServer.Paused)
        {
            S9xNPSendHeartBeat ();
			newPausedState = 0;
        }
		else
		{
			newPausedState = 1;
		}

		if(pausedState != newPausedState)
		{
			pausedState = newPausedState;
//			S9xNPSendServerPause(pausedState); // XXX: doesn't seem to work yet...
		}

        do
        {
            FD_ZERO (&read_fds);
            FD_SET (NPServer.Socket, &read_fds);
            for (i = 0; i < NP_MAX_CLIENTS; i++)
            {
                if (NPServer.Clients [i].Connected)
                {
                    FD_SET (NPServer.Clients [i].Socket, &read_fds);
                    if (NPServer.Clients [i].Socket > max_fd)
                        max_fd = NPServer.Clients [i].Socket;
                }
            }

            timeout.tv_sec = 0;
            timeout.tv_usec = 1000;
            res = select (max_fd + 1, &read_fds, NULL, NULL, &timeout);

            if (res > 0)
            {
                if (FD_ISSET (NPServer.Socket, &read_fds))
                    S9xNPAcceptClient (NPServer.Socket, FALSE);

                for (i = 0; i < NP_MAX_CLIENTS; i++)
                {
                    if (NPServer.Clients [i].Connected &&
                        FD_ISSET (NPServer.Clients [i].Socket, &read_fds))
                    {
                        S9xNPProcessClient (i);
                    }
                }
            }
        } while (res > 0);

#ifdef __WIN32__
        success = WaitForSingleObject (GUI.ServerTimerSemaphore, 200) == WAIT_OBJECT_0;
#else
        while (gettimeofday (&now, NULL) < 0) ;

        /* If there is no known "next" frame, initialize it now */
        if (next1.tv_sec == 0) { next1 = now; ++next1.tv_usec; }

	success=FALSE;

	if (timercmp(&next1, &now, >))
        {
            /* If we're ahead of time, sleep a while */
            unsigned timeleft =
                (next1.tv_sec - now.tv_sec) * 1000000
                + next1.tv_usec - now.tv_usec;
	    usleep(timeleft<(200*1000)?timeleft:(200*1000));
        }

        if (!timercmp(&next1, &now, >))
        {

            /* Calculate the timestamp of the next frame. */
            next1.tv_usec += Settings.FrameTime;
            if (next1.tv_usec >= 1000000)
            {
                next1.tv_sec += next1.tv_usec / 1000000;
                next1.tv_usec %= 1000000;
            }
            success=TRUE;
         }
#endif

        while (NPServer.TaskHead != NPServer.TaskTail)
        {
            void *task_data = NPServer.TaskQueue [NPServer.TaskHead].Data;

#if defined(NP_DEBUG) && NP_DEBUG == 2
            printf ("SERVER: task %d @%ld\n", NPServer.TaskQueue [NPServer.TaskHead].Task, S9xGetMilliTime () - START);
#endif

            switch (NPServer.TaskQueue [NPServer.TaskHead].Task)
            {
                case NP_SERVER_SEND_ROM_IMAGE:
                    S9xNPSendROMImageToAllClients ();
                    break;
                case NP_SERVER_SYNC_CLIENT:
                    NPServer.Clients [(pint) task_data].Ready = FALSE;
                    S9xNPRecomputePause ();
                    S9xNPSyncClient ((pint) task_data);
                    break;
                case NP_SERVER_SYNC_ALL:
                    S9xNPSyncClients ();
                    break;
                case NP_SERVER_SEND_FREEZE_FILE_ALL:
                    S9xNPSendFreezeFileToAllClients ((char *) task_data);
                    free ((char *) task_data);
                    break;
                case NP_SERVER_SEND_ROM_LOAD_REQUEST_ALL:
                    S9xNPSendROMLoadRequest ((char *) task_data);
                    free ((char *) task_data);
                    break;
                case NP_SERVER_RESET_ALL:
                    S9xNPNoClientReady (0);
                    S9xNPWaitForEmulationToComplete ();
                    S9xNPSetAction ("SERVER: Sending RESET to all clients...", TRUE);
#ifdef NP_DEBUG
                    printf ("SERVER: Sending RESET to all clients @%ld\n", S9xGetMilliTime () - START);
#endif
                    {
                        uint8 reset [7];
                        uint8 *ptr;

                        ptr = reset;
                        *ptr++ = NP_SERV_MAGIC;
                        *ptr++ = 0;
                        *ptr++ = NP_SERV_RESET;
                        WRITE_LONG (ptr, NPServer.FrameCount);
                        S9xNPSendToAllClients (reset, 7);
                    }
                    S9xNPSetAction ("", TRUE);
                    break;
                case NP_SERVER_SEND_SRAM:
                    NPServer.Clients [(pint) task_data].Ready = FALSE;
                    S9xNPRecomputePause ();
                    S9xNPWaitForEmulationToComplete ();
                    S9xNPSendSRAMToClient ((pint) task_data);
                    break;

                case NP_SERVER_SEND_SRAM_ALL:
                    S9xNPNoClientReady ();
                    S9xNPWaitForEmulationToComplete ();
                    S9xNPSendSRAMToAllClients ();
                    break;

                default:
                    S9xNPSetError ("SERVER: *** Unknown task ***\n");
                    break;
            }
            NPServer.TaskHead = (NPServer.TaskHead + 1) % NP_MAX_TASKS;
        }
    }
#ifdef NP_DEBUG
    printf ("SERVER: Server thread exiting @%ld\n", S9xGetMilliTime () - START);
#endif
    // OV2: S9xNPStopServer has already been called if we get here
    // S9xNPStopServer ();
}

bool8 S9xNPStartServer (int port)
{
    static int p;

#ifdef NP_DEBUG
    printf ("SERVER: Starting server on port %d @%ld\n", port, S9xGetMilliTime () - START);
#endif
    p = port;
    server_continue = TRUE;
    if (S9xNPServerInit (port))
#ifdef __WIN32__
        return (_beginthread (S9xNPServerLoop, 0, &p) != (uintptr_t)(~0));
#else
    	S9xNPServerLoop(NULL);
	return (TRUE);
#endif

    return (FALSE);
}

void S9xNPStopServer ()
{
#ifdef NP_DEBUG
    printf ("SERVER: Stopping server @%ld\n", S9xGetMilliTime () - START);
#endif
    server_continue = FALSE;
    close (NPServer.Socket);

    for (int i = 0; i < NP_MAX_CLIENTS; i++)
    {
        if (NPServer.Clients [i].Connected)
	    S9xNPShutdownClient(i, FALSE);
    }
}

#ifdef __WIN32__
void S9xGetTimeOfDay (struct timeval *n)
{
    unsigned long t = S9xGetMilliTime ();

    n->tv_sec = t / 1000;
    n->tv_usec = (t % 1000) * 1000;
}
#endif

void S9xNPSendROMImageToAllClients ()
{
    S9xNPNoClientReady ();
    S9xNPWaitForEmulationToComplete ();

    int c;

    for (c = NP_ONE_CLIENT; c < NP_MAX_CLIENTS; c++)
    {
        if (NPServer.Clients [c].SaidHello)
            S9xNPSendROMImageToClient (c);
    }

    if (NPServer.SyncByReset)
    {
        S9xNPServerAddTask (NP_SERVER_SEND_SRAM_ALL, 0);
        S9xNPServerAddTask (NP_SERVER_RESET_ALL, 0);
    }
    else
        S9xNPSyncClient (-1);
}

bool8 S9xNPSendROMImageToClient (int c)
{
#ifdef NP_DEBUG
    printf ("SERVER: Sending ROM image to player %d @%ld\n", c + 1, S9xGetMilliTime () - START);
#endif
    sprintf (NetPlay.ActionMsg, "Sending ROM image to player %d...", c + 1);
    S9xNPSetAction (NetPlay.ActionMsg, TRUE);

    uint8 header [7 + 1 + 4];
    uint8 *ptr = header;
    int len = sizeof (header) + Memory.CalculatedSize +
              strlen (Memory.ROMFilename) + 1;
    *ptr++ = NP_SERV_MAGIC;
    *ptr++ = NPServer.Clients [c].SendSequenceNum++;
    *ptr++ = NP_SERV_ROM_IMAGE;
    WRITE_LONG (ptr, len);
    ptr += 4;
    *ptr++ = Memory.HiROM;
    WRITE_LONG (ptr, Memory.CalculatedSize);

    if (!S9xNPSSendData (NPServer.Clients [c].Socket, header, sizeof (header)) ||
        !S9xNPSSendData (NPServer.Clients [c].Socket, Memory.ROM,
                        Memory.CalculatedSize) ||
        !S9xNPSSendData (NPServer.Clients [c].Socket, (uint8 *) Memory.ROMFilename,
                        strlen (Memory.ROMFilename) + 1))
    {
        S9xNPShutdownClient (c, TRUE);
        return (FALSE);
    }
    return (TRUE);
}

void S9xNPSyncClients ()
{
    S9xNPNoClientReady ();
    S9xNPSyncClient (-1);
}

void S9xNPSyncClient (int client)
{
#ifdef HAVE_MKSTEMP
    char fname[] = "/tmp/snes9x_fztmpXXXXXX";
    int fd=-1;
#else
    char fname [L_tmpnam];
#endif

    S9xNPWaitForEmulationToComplete ();

    S9xNPSetAction ("SERVER: Freezing game...", TRUE);
#ifdef HAVE_MKSTEMP
    if ( ((fd=mkstemp(fname)) >= 0) && S9xFreezeGame(fname) )
#else
    if ( tmpnam(fname) && S9xFreezeGame(fname) )
#endif
    {
        uint8 *data;
        uint32 len;

        S9xNPSetAction ("SERVER: Loading freeze file...", TRUE);
        if (S9xNPLoadFreezeFile (fname, data, len))
        {
            int c;

            if (client < 0)
            {
                for (c = NP_ONE_CLIENT; c < NP_MAX_CLIENTS; c++)
                {
                    if (NPServer.Clients [c].SaidHello)
                    {
                        NPServer.Clients [c].Ready = FALSE;
                        S9xNPRecomputePause ();
                        S9xNPSendFreezeFile (c, data, len);
                    }
                }
            }
            else
            {
                NPServer.Clients [client].Ready = FALSE;
                S9xNPRecomputePause ();
                S9xNPSendFreezeFile (client, data, len);
            }
            delete data;
        }
        remove (fname);
    }
#ifdef HAVE_MKSTEMP
    if (fd != -1)
        close(fd);
#endif
}

bool8 S9xNPLoadFreezeFile (const char *fname, uint8 *&data, uint32 &len)
{
    FILE *ff;

    if ((ff = fopen (fname, "rb")))
    {
        fseek (ff, 0, SEEK_END);
        len = ftell (ff);
        fseek (ff, 0, SEEK_SET);

        data = new uint8 [len];
        bool8 ok = (fread (data, 1, len, ff) == len);
        fclose (ff);

        return (ok);
    }
    return (FALSE);
}

void S9xNPSendFreezeFile (int c, uint8 *data, uint32 len)
{
#ifdef NP_DEBUG
    printf ("SERVER: Sending freeze file to player %d @%ld\n", c + 1, S9xGetMilliTime () - START);
#endif

    sprintf (NetPlay.ActionMsg, "SERVER: Sending freeze-file to player %d...", c + 1);
    S9xNPSetAction (NetPlay.ActionMsg, TRUE);
    uint8 header [7 + 4];
    uint8 *ptr = header;

    *ptr++ = NP_SERV_MAGIC;
    *ptr++ = NPServer.Clients [c].SendSequenceNum++;
    *ptr++ = NP_SERV_FREEZE_FILE;
    WRITE_LONG (ptr, len + 7 + 4);
    ptr += 4;
    WRITE_LONG (ptr, NPServer.FrameCount);

    if (!S9xNPSSendData (NPServer.Clients [c].Socket, header, 7 + 4) ||
        !S9xNPSSendData (NPServer.Clients [c].Socket, data, len))
    {
       S9xNPShutdownClient (c, TRUE);
    }
    S9xNPSetAction ("", TRUE);
}

void S9xNPRecomputePause ()
{
    int c;

    for (c = 0; c < NP_MAX_CLIENTS; c++)
    {
        if (NPServer.Clients [c].SaidHello &&
            (!NPServer.Clients [c].Ready || NPServer.Clients [c].Paused))
        {
#if defined(NP_DEBUG) && NP_DEBUG == 2
            printf ("SERVER: Paused because of client %d (%d,%d) @%ld\n", c, NPServer.Clients [c].Ready, NPServer.Clients [c].Paused, S9xGetMilliTime () - START);
#endif
            NPServer.Paused = TRUE;
            return;
        }
    }
#if defined(NP_DEBUG) && NP_DEBUG == 2
    printf ("SERVER: not paused @%ld\n", S9xGetMilliTime () - START);
#endif
    NPServer.Paused = FALSE;
}

void S9xNPNoClientReady (int start_index)
{
    int c;

    for (c = start_index; c < NP_MAX_CLIENTS; c++)
        NPServer.Clients [c].Ready = FALSE;
    S9xNPRecomputePause ();
}

void S9xNPSendROMLoadRequest (const char *filename)
{
    S9xNPNoClientReady ();

    int len = 7 + strlen (filename) + 1;
    uint8 *data = new uint8 [len];
    uint8 *ptr = data;
    *ptr++ = NP_SERV_MAGIC;
    *ptr++ = 0;
    *ptr++ = NP_SERV_LOAD_ROM;
    WRITE_LONG (ptr, len);
    ptr += 4;
    strcpy ((char *) ptr, filename);

    for (int i = NP_ONE_CLIENT; i < NP_MAX_CLIENTS; i++)
    {
	if (NPServer.Clients [i].SaidHello)
	{
#ifdef NP_DEBUG
            printf ("SERVER: Sending load ROM requesting to player %d @%ld\n", i + 1, S9xGetMilliTime () - START);
#endif
            sprintf (NetPlay.WarningMsg, "SERVER: sending ROM load request to player %d...", i + 1);
            S9xNPSetAction (NetPlay.WarningMsg, TRUE);
            data [1] = NPServer.Clients [i].SendSequenceNum++;
	    if (!S9xNPSSendData (NPServer.Clients [i].Socket, data, len))
            {
		S9xNPShutdownClient (i, TRUE);
            }
        }
    }
    delete data;
}

void S9xNPSendSRAMToAllClients ()
{
    int i;

    for (i = NP_ONE_CLIENT; i < NP_MAX_CLIENTS; i++)
    {
        if (NPServer.Clients [i].SaidHello)
            S9xNPSendSRAMToClient (i);
    }
}

void S9xNPSendSRAMToClient (int c)
{
#ifdef NP_DEBUG
    printf ("SERVER: Sending S-RAM data to player %d @%ld\n", c + 1, S9xGetMilliTime () - START);
#endif
    uint8 sram [7];
    int SRAMSize = Memory.SRAMSize ?
                   (1 << (Memory.SRAMSize + 3)) * 128 : 0;
    if (SRAMSize > 0x10000)
        SRAMSize = 0x10000;
    int len = 7 + SRAMSize;

    sprintf (NetPlay.ActionMsg, "SERVER: Sending S-RAM to player %d...", c + 1);
    S9xNPSetAction (NetPlay.ActionMsg, TRUE);

    uint8 *ptr = sram;
    *ptr++ = NP_SERV_MAGIC;
    *ptr++ = NPServer.Clients [c].SendSequenceNum++;
    *ptr++ = NP_SERV_SRAM_DATA;
    WRITE_LONG (ptr, len);
    if (!S9xNPSSendData (NPServer.Clients [c].Socket,
                        sram, sizeof (sram)) ||
        (len > 7 &&
         !S9xNPSSendData (NPServer.Clients [c].Socket,
                         Memory.SRAM, len - 7)))
    {
        S9xNPShutdownClient (c, TRUE);
    }
}

void S9xNPSendFreezeFileToAllClients (const char *filename)
{
    uint8 *data;
    uint32 len;

    if (NPServer.NumClients > NP_ONE_CLIENT && S9xNPLoadFreezeFile (filename, data, len))
    {
        S9xNPNoClientReady ();

        for (int c = NP_ONE_CLIENT; c < NP_MAX_CLIENTS; c++)
        {
            if (NPServer.Clients [c].SaidHello)
                S9xNPSendFreezeFile (c, data, len);
        }
        delete data;
    }
}

void S9xNPServerAddTask (uint32 task, void *data)
{
    NPServer.TaskQueue [NPServer.TaskTail].Task = task;
    NPServer.TaskQueue [NPServer.TaskTail].Data = data;

    NPServer.TaskTail = (NPServer.TaskTail + 1) % NP_MAX_TASKS;
}

void S9xNPReset ()
{
    S9xNPNoClientReady (0);
    S9xNPServerAddTask (NP_SERVER_RESET_ALL, 0);
}

void S9xNPWaitForEmulationToComplete ()
{
#ifdef NP_DEBUG
    printf ("SERVER: WaitForEmulationToComplete start @%ld\n", S9xGetMilliTime () - START);
#endif

    while (!NetPlay.PendingWait4Sync && NetPlay.Connected &&
           !Settings.ForcedPause && !Settings.StopEmulation &&
           !(Settings.Paused && !Settings.FrameAdvance))
    {
#ifdef __WIN32__
        Sleep (40);
#endif
    }
#ifdef NP_DEBUG
    printf ("SERVER: WaitForEmulationToComplete end @%ld\n", S9xGetMilliTime () - START);
#endif
}

void S9xNPServerQueueSyncAll ()
{
    if (Settings.NetPlay && Settings.NetPlayServer &&
        NPServer.NumClients > NP_ONE_CLIENT)
    {
        S9xNPNoClientReady ();
        S9xNPDiscardHeartbeats ();
        S9xNPServerAddTask (NP_SERVER_SYNC_ALL, 0);
    }
}

void S9xNPServerQueueSendingROMImage ()
{
    if (Settings.NetPlay && Settings.NetPlayServer &&
        NPServer.NumClients > NP_ONE_CLIENT)
    {
        S9xNPNoClientReady ();
        S9xNPDiscardHeartbeats ();
        S9xNPServerAddTask (NP_SERVER_SEND_ROM_IMAGE, 0);
    }
}

void S9xNPServerQueueSendingFreezeFile (const char *filename)
{
    if (Settings.NetPlay && Settings.NetPlayServer &&
        NPServer.NumClients > NP_ONE_CLIENT)
    {
        S9xNPNoClientReady ();
        S9xNPDiscardHeartbeats ();
        S9xNPServerAddTask (NP_SERVER_SEND_FREEZE_FILE_ALL,
                            (void *) strdup (filename));
    }
}

void S9xNPServerQueueSendingLoadROMRequest (const char *filename)
{
    if (Settings.NetPlay && Settings.NetPlayServer &&
        NPServer.NumClients > NP_ONE_CLIENT)
    {
        S9xNPNoClientReady ();
        S9xNPDiscardHeartbeats ();
        S9xNPServerAddTask (NP_SERVER_SEND_ROM_LOAD_REQUEST_ALL,
                            (void *) strdup (filename));
    }
}

#ifndef __WIN32__
uint32 S9xGetMilliTime ()
{
    static bool8 first = TRUE;
    static long start_sec;
    struct timeval tv;

    gettimeofday (&tv, NULL);
    if (first)
    {
	start_sec = tv.tv_sec;
	first = FALSE;
    }
    return ((uint32) ((tv.tv_sec - start_sec) * 1000 + tv.tv_usec / 1000));
}
#endif
#endif
