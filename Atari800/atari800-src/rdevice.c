/*
 * rdevice.c - Atari850 emulation
 *
 * Copyright (c) ???? Tom Hunt, Chris Martin
 * Copyright (c) 2003-2005,2007-2008 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* Originally From Closer To Home (Tom Hunt)
 *
 * 2003.04.03 cmartin@ti.com - Added connecting Host name lookup and port
 *                             changing as well as CONNECT_STRING changing...
 * 2003.04.03 cmartin@ti.com - Added setsockopt SO_REUSEADDR for socket to
 *                             allow reconnections
 * 2003.04.05 cmartin@ti.com - Added sighandler for catching SIGPIPE error...
 *                             can get this and cause the emu to crash if
 *                             the other end disconnects prematurely...
 * 2003.04.06 cmartin@ti.com - Added translation on/off and line feeds
 *                           - Fixed IP address lookup (print ip address if
 *                             lookup fails...)
 * 2003.04.07 cmartin@ti.com - Added Local echo when not connected.
 * 2003.04.17 cmartin@ti.com - Added Telnet escape sequence parsing for
 *                             connecting to telnet
 * 2003.09.02 cmartin@ti.com - Cleaned up Serial port (/dev/ttyS*) Handling
 * 2003.09.05 cmartin@ti.com - Included minor bug fixes submitted from
 *                             Christian Groessler (thanks Chris!)
 *                           - Removed erroneous return in RREAD().
 *                           - Fixed typo for 9600 baud in xio_36()
 *                           - Modified serial port dev_name generation in
 *                             open_connection_serial()
 *                           - Now we only modify the serial port options if
 *                             we can open a port (in open_connection_serial())
 *                           - Removed Telnet IAC escape sequence handling in
 *                             RWRIT() and now we only
 *                             write to the port if we are connected.
 * 2003.09.08 cmartin@ti.com - Added missing baud rates and data word
 *                             lengths (5,6,7,8)
 * 2003.09.11 cmartin@ti.com - Fixed Address lookup in open_connection()
 *                           - Reformatted and cleaned whole file.
 *                           - Removed all memory writes to addresses
 *                             746 - 749 except within the RSTAT function.
 *
 * TODO:
 *       create a new socket and a buffer for each port number....
 *       non-concurrent mode error reporting to 747
 *       error bits in location 746
 *       implement xio 32?
 *       error reporting to CPU_regY
 *       heavy ASCII translation
 *       controlling DTR, RTS, & XMT - xio 34
 *
 * Dialing out via telnet:
 * Type "ATDL" to toggle Linefeeds on/off (normally off)
 * Type "ATDI <hostname> <port>" example: "ATDI localhost 23"
 * if the port is omitted, then 23 is assumed.
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef DEBUG
#define DBG_APRINT(x) Log_print(x)
#else
#define DBG_APRINT(x)
#endif

#if 0  /* debug stuff */
#ifdef R_NETWORK
#warning R_NETWORK defined
#endif
#ifdef R_SERIAL
#warning R_SERIAL defined
#endif
#endif

#ifdef HAVE_WINDOWS_H
#ifdef R_SERIAL
#error The Windows version does not support the serial R: device
#endif
#ifndef R_NETWORK
#error R_NETWORK must be defined to use the Windows R: device
#endif
#include <winsock2.h>
   #define F_SETFL 0
   #define O_NONBLOCK 0
   #define WM_SOCKET (WM_USER+20)
   #define inet_pton(fam,ip,addr) ((addr)->s_addr = inet_addr(ip), 0)
static u_long ioctlsocket_non_block = 1; /* for ioctlsocket */
#define perror(a) printf("%s:WSA error code:%d\n",a,WSAGetLastError())
#define close(a) closesocket(a)
typedef char *caddr_t;
static void catch_disconnect(int sig);
static int rdevice_win32_read(SOCKET s, char *buf, int len) {
  int r;
  r = recv(s, buf, len, 0);
  if (r <= 0 ) {
    switch (WSAGetLastError()) {
      case WSAECONNRESET:
      case WSAECONNABORTED:
      case WSAESHUTDOWN:
        catch_disconnect(0);
        break;
    }
  }
  return r;
}

static int rdevice_win32_write(SOCKET s, char *buf, int len) {
  int r;
  r = send(s, buf, len, 0);
  if (r <= 0 ) {
    switch (WSAGetLastError()) {
      case WSAECONNRESET:
      case WSAECONNABORTED:
      case WSAESHUTDOWN:
        catch_disconnect(0);
        break;
    }
  }
  return r;
}

#define read(a,b,c) rdevice_win32_read(a,b,c)
#define write(a,b,c) rdevice_win32_write(a,b,c)

#elif defined(DREAMCAST) /* above WIN32, below DREAMCAST */

/* these functions reside in dc/atari_dc.c */
extern void dc_init_serial(void);
extern int dc_write_serial(unsigned char byte);
extern int dc_read_serial(unsigned char *byte);
extern void dc_set_baud(int baud);

#else /* above DREAMCAST, below not WIN32 and not DREAMCAST */

#if !defined(R_SERIAL) && !defined(R_NETWORK)
#error R: device needs serial or network or both
#endif
#ifdef R_NETWORK
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif /* R_NETWORK */
#include <sys/types.h>
#include <fcntl.h>
#endif /* not WIN32 and not DREAMCAST */

#if defined(R_SERIAL) && !defined(DREAMCAST)
#include <termios.h>
#endif /* defined(R_SERIAL) && !defined(DREAMCAST) */

#include "atari.h"
#include "rdevice.h"
#include "cpu.h"
#include "devices.h"
#include "log.h"
#include "memory.h"
#include "util.h"

#define Peek(a)    MEMORY_dGetByte(a)
#define DPeek(a)   MEMORY_dGetWord(a)
#define Poke(x,y)  MEMORY_dPutByte(x, y)

/*---------------------------------------------------------------------------
  Global Variables
---------------------------------------------------------------------------*/
static int connected;
static int do_once;
static int rdev_fd;

#ifdef R_NETWORK
static struct sockaddr_in in;
static struct sockaddr_in peer_in;
static int sock;
static int portnum = 9000;
static char inetaddress[256];
static char CONNECT_STRING[40] = "\r\n_CONNECT 2400\r\n";
static int retval;
#endif /* R_NETWORK */

static char MESSAGE[256];
static char command_buf[256];
static char bufout[256];
static int concurrent;

static int command_end = 0;
static int translation = 1;
static int trans_cr = 0;
static int linefeeds = 1;
static int bufend = 0;

#ifndef R_NETWORK
int RDevice_serial_enabled = 1;
#else
int RDevice_serial_enabled = 0;  /* Default to network, if enabled. Use parameter to -rdevice command line switch to enable serial mode. */
#endif
char RDevice_serial_device[FILENAME_MAX];

/*---------------------------------------------------------------------------
   Host Support Function - If Disconnect signal is found, then close socket
   and clean up.
---------------------------------------------------------------------------*/
#ifdef R_NETWORK
static void catch_disconnect(int sig)
{
  DBG_APRINT("R*: Disconnected....");
  close(rdev_fd);
  connected = 0;
  do_once = 0;
  bufout[0] = 0;
  strcat(bufout, "\r\nNO CARRIER\r\n");
  bufend = 13;
#if 0
  Poke(747,bufend);
  Poke(748,0);
#endif
}
#endif /* R_NETWORK */

/*---------------------------------------------------------------------------
   Host Support Function - XIO 34 - Called from RDevice_SPEC
   Controls handshake lines DTR, RTS, SD
---------------------------------------------------------------------------*/
static void xio_34(void)
{
#ifndef DREAMCAST  /* Dreamcast port doesn't currently support handshake lines */
  int temp;
  /*int fid;*/
#ifdef R_SERIAL
  struct termios options;
#endif /* R_SERIAL */
  /*int status;*/

  /*fid = MEMORY_dGetByte(0x2e) >> 4;*/
  temp = MEMORY_dGetByte(Devices_ICAX1Z);

#ifdef R_SERIAL
  if(RDevice_serial_enabled)
  {
    tcgetattr(rdev_fd, &options);
    /*ioctl(rdev_fd, TIOCMGET, &status);*/
  }
#endif /* R_SERIAL */

  if(temp & 0x80)
  {
    if(temp & 0x40)
    {
      /* turn DTR on */
#ifdef R_SERIAL
      if(RDevice_serial_enabled)
      {
        /*status |= TIOCM_DTR;*/
      }
#endif /* R_SERIAL */
    }
    else
    {
      /*Drop DTR*/
#ifdef R_SERIAL
      if(RDevice_serial_enabled)
      {
        cfsetispeed(&options, B0);
        cfsetospeed(&options, B0);
      }
#endif /* R_SERIAL */

      if(connected != 0)
      {
        close ( rdev_fd );
        connected = 0;
        do_once = 0;
        /*bufend = 0;*/
      }
    }
  }

#ifdef R_SERIAL
  if(RDevice_serial_enabled)
  {
    /* RTS Set/Clear*/
    if(temp & 0x20)
    {
      if(temp & 0x10)
      {
        /*status |= TIOCM_RTS;*/
      }
      else
      {
        /*status &= ~TIOCM_RTS;*/
      }
    }

    if(temp & 0x02)
    {
      if(temp & 0x01)
      {
        /*status |= TIOCM_RTS;*/
      }
      else
      {
        /*status &= ~TIOCM_RTS;*/
      }
    }
  }
#endif /* R_SERIAL */

#ifdef R_SERIAL
  if(RDevice_serial_enabled)
  {
    tcsetattr(rdev_fd, TCSANOW, &options);
    /*ioctl(rdev_fd, TIOCMSET, &status);*/
  }
#endif /* R_SERIAL */
#endif /* not defined DREAMCAST */

  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;

}

/*---------------------------------------------------------------------------
   Host Support Function - XIO 36 - Called from RDevice_SPEC
   Sets baud, stop bits, and ready monitoring.
---------------------------------------------------------------------------*/
static void xio_36(void)
{
  int aux1, aux2;
#if defined(R_SERIAL) && !defined(DREAMCAST)
  struct termios options;
#endif /* defined(R_SERIAL) && !defined(DREAMCAST) */

  aux1 = MEMORY_dGetByte(Devices_ICAX1Z);
  aux2 = MEMORY_dGetByte(Devices_ICAX2Z);

#ifdef R_SERIAL
  if(RDevice_serial_enabled)
  {
#ifndef DREAMCAST
    tcgetattr(rdev_fd, &options);

    /*Set Stop bits*/
    if(aux1 & 0x80)
    { /*2 Stop bits*/
      options.c_cflag |= CSTOPB;
    }
    else
    { /*1 Stop bit*/
      options.c_cflag &= ~CSTOPB;
    }

    /*Set word size*/
    if((aux1 & 0x30) == 0)
    { /*8 bit word size*/
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS8;
    }
    else if((aux1 & 0x30) == 0x10)
    { /*7 bit word size*/
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS7;
    }
    else if((aux1 & 0x30) == 0x20)
    { /*6 bit word size*/
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS6;
    }
    else if((aux1 & 0x30) == 0x30)
    { /*5 bit word size*/
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS5;
    }
    else
    { /*8 bit word size*/
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS8;
    }

    /*Set Baud Rate*/
    if((aux1 & 0x0f) == 0)
    { /*300 Baud*/
      cfsetispeed(&options, B300);
      cfsetospeed(&options, B300);
    }
    else if((aux1 & 0x0f) == 1)
    { /* 45.5 Baud (unsupported) - now 57600 */
#ifdef B57600
      cfsetispeed(&options, B57600);
      cfsetospeed(&options, B57600);
#else
      cfsetispeed(&options, B50);
      cfsetospeed(&options, B50);
#endif
    }
    else if ((aux1 & 0x0f) == 2)
    { /* 50 Baud */
      cfsetispeed(&options, B50);
      cfsetospeed(&options, B50);
    }
    else if ((aux1 & 0x0f) == 3)
    { /* 56.875 Baud (unsupported) - now 115200 */
#ifdef B115200
      cfsetispeed(&options, B115200);
      cfsetospeed(&options, B115200);
#else
      cfsetispeed(&options, B50);
      cfsetospeed(&options, B50);
#endif
    }
    else if((aux1 & 0x0f) == 4)
    { /* 75 Baud */
      cfsetispeed(&options, B75);
      cfsetospeed(&options, B75);
    }
    else if((aux1 & 0x0f) == 5)
    { /* 110 Baud */
      cfsetispeed(&options, B110);
      cfsetospeed(&options, B110);
    }
    else if((aux1 & 0x0f) == 6)
    { /* 134.5 Baud */
      cfsetispeed(&options, B134);
      cfsetospeed(&options, B134);
    }
    else if((aux1 & 0x0f) == 7)
    { /* 150 Baud */
      cfsetispeed(&options, B150);
      cfsetospeed(&options, B150);
    }
    else if((aux1 & 0x0f) == 8)
    { /* 300 Baud */
      cfsetispeed(&options, B300);
      cfsetospeed(&options, B300);
    }
    else if((aux1 & 0x0f) == 9)
    { /* 600 Baud */
      cfsetispeed(&options, B600);
      cfsetospeed(&options, B600);
    }
    else if((aux1 & 0x0f) == 10)
    { /* 1200 Baud */
      cfsetispeed(&options, B1200);
      cfsetospeed(&options, B1200);
    }
    else if((aux1 & 0x0f) == 12)
    { /* 2400 Baud */
      cfsetispeed(&options, B2400);
      cfsetospeed(&options, B2400);
    }
    else if((aux1 & 0x0f) == 13)
    { /* 4800 Baud */
      cfsetispeed(&options, B4800);
      cfsetospeed(&options, B4800);
    }
    else if((aux1 & 0x0f) == 14)
    { /* 9600 Baud */
      cfsetispeed(&options, B9600);
      cfsetospeed(&options, B9600);
    }
    else if((aux1 & 0x0f) == 15)
    { /* 19200 Baud */
#ifdef B19200
      cfsetispeed(&options, B19200);
      cfsetospeed(&options, B19200);
#else
      cfsetispeed(&options, B9600);
      cfsetospeed(&options, B9600);
#endif
    }
    else
    { /* 115200 Baud (can add 38400, 76800 if wanted) */
#ifdef B115200
      cfsetispeed(&options, B115200);
      cfsetospeed(&options, B115200);
#else
      cfsetispeed(&options, B9600);
      cfsetospeed(&options, B9600);
#endif
    }

    if(aux1 & 0x40)
    { /* 230400 Baud */
#ifdef B230400
      cfsetispeed(&options, B230400);
      cfsetospeed(&options, B230400);
#endif
    }

    tcsetattr(rdev_fd, TCSANOW, &options);
#else /* below DREAMCAST, above not; both R_SERIAL */
    /* Set Stop bits */
    if(aux1 & 0x80)
    { /* 2 Stop bits */
      /* currently not supported on DC */
    }
    else
    { /* 1 Stop bit */
      /* currently not supported on DC */
    }

    /* Set word size */
    if((aux1 & 0x30) == 0)
    { /* 8 bit word size */
      /* currently not supported on DC */
    }
    else if((aux1 & 0x30) == 0x10)
    { /* 7 bit word size */
      /* currently not supported on DC */
    }
    else if((aux1 & 0x30) == 0x20)
    { /* 6 bit word size */
      /* currently not supported on DC */
    }
    else if((aux1 & 0x30) == 0x30)
    { /* 5 bit word size */
      /* currently not supported on DC */
    }
    else
    { /* 8 bit word size */
      /* currently not supported on DC */
    }

    /* Set Baud Rate */
    if((aux1 & 0x0f) == 0)
    { /* 300 Baud */
      dc_set_baud(300);
    }
    else if((aux1 & 0x0f) == 1)
    { /* 45.5 Baud (unsupported) - now 57600 */
      dc_set_baud(57600);
    }
    else if ((aux1 & 0x0f) == 2)
    { /* 50 Baud */
      dc_set_baud(50);
    }
    else if ((aux1 & 0x0f) == 3)
    { /* 56.875 Baud (unsupported) - now 115200 */
      dc_set_baud(115200);
    }
    else if((aux1 & 0x0f) == 4)
    { /* 75 Baud */
      dc_set_baud(75);
    }
    else if((aux1 & 0x0f) == 5)
    { /* 110 Baud */
      dc_set_baud(110);
    }
    else if((aux1 & 0x0f) == 6)
    { /* 134.5 Baud */
      dc_set_baud(134);
    }
    else if((aux1 & 0x0f) == 7)
    { /* 150 Baud */
      dc_set_baud(150);
    }
    else if((aux1 & 0x0f) == 8)
    { /* 300 Baud */
      dc_set_baud(300);
    }
    else if((aux1 & 0x0f) == 9)
    { /* 600 Baud */
      dc_set_baud(600);
    }
    else if((aux1 & 0x0f) == 10)
    { /* 1200 Baud */
      dc_set_baud(1200);
    }
    else if((aux1 & 0x0f) == 12)
    { /* 2400 Baud */
      dc_set_baud(2400);
    }
    else if((aux1 & 0x0f) == 13)
    { /* 4800 Baud */
      dc_set_baud(4800);
    }
    else if((aux1 & 0x0f) == 14)
    { /* 9600 Baud */
      dc_set_baud(9600);
    }
    else if((aux1 & 0x0f) == 15)
    { /* 19200 Baud */
      dc_set_baud(19200);
    }
    else
    { /* 115200 Baud (can add 38400, 76800 if wanted) */
      dc_set_baud(115200);
    }

    if(aux1 & 0x40)
    { /*  230400 Baud */
      dc_set_baud(230400);
    }
#endif /* DREAMCAST */
  }
#endif /* R_SERIAL */

  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;
}

/*---------------------------------------------------------------------------
   Host Support Function - XIO 38 - Called from RDevice_SPEC
   Sets Translation and parity
---------------------------------------------------------------------------*/
static void xio_38(void)
{
  int aux1;
#if defined(R_SERIAL) && !defined(DREAMCAST)
  struct termios options;
#endif /* defined(R_SERIAL) && !defined(DREAMCAST) */

  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;

  aux1 = Peek(Devices_ICAX1Z);
#if defined(R_SERIAL) && !defined(DREAMCAST)
  if(RDevice_serial_enabled)
  {
    if(aux1 & 0x04)
    { /*Odd Parity*/
      tcgetattr(rdev_fd, &options);
      options.c_cflag |= PARENB;
      options.c_cflag |= PARODD;
      tcsetattr(rdev_fd, TCSANOW, &options);
    }
    else if(aux1 & 0x08)
    { /*Even Parity*/
      tcgetattr(rdev_fd, &options);
      options.c_cflag |= PARENB;
      options.c_cflag &= ~PARODD;
      tcsetattr(rdev_fd, TCSANOW, &options);

    }
    else
    { /*No Parity*/
      tcgetattr(rdev_fd, &options);
      options.c_cflag &= ~PARENB;
      tcsetattr(rdev_fd, TCSANOW, &options);
    }
  }
#endif /* defined(R_SERIAL) && !defined(DREAMCAST) */

  if(aux1 & 0x20)
  { /* No Translation */
    DBG_APRINT("R*: No ATASCII/ASCII TRANSLATION");
    translation = 0;
  }
  else
  {
    translation = 1;
  }

  if(aux1 & 0x40)
  { /* Append line feed */
    DBG_APRINT("R*: Append Line Feeds");
    linefeeds = 1;
  }
  else
  {
    linefeeds = 0;
  }

}

/*---------------------------------------------------------------------------
   Host Support Function - XIO 40 - Called from RDevice_SPEC
   Sets concurrent mode.  Also checks for dropped carrier.
---------------------------------------------------------------------------*/
static void xio_40(void)
{

  int aux1;
/*
    if(connected == 0)
      Poke(747,0);
*/
  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;

  aux1 = Peek(Devices_ICAX1Z);

  if(aux1 >= 12)
  {
    concurrent = 1;
    DBG_APRINT("R*: Entering concurrent IO mode...");
  }
  else
  {
    concurrent = 0;
    snprintf(MESSAGE, sizeof(MESSAGE), "R*: XIO 40, %d", aux1);
    DBG_APRINT(MESSAGE);
  }

}

/*---------------------------------------------------------------------------
   Host Support Function - Internet Socket Open Connection
---------------------------------------------------------------------------*/
#ifdef R_NETWORK
static void open_connection(char * address, int port)
{
  struct hostent *host;
#ifdef HAVE_WINDOWS_H
  static int winsock_started;
  WSADATA wdata;
  WORD ver = MAKEWORD(1,1);
  int ret;
  if (!winsock_started) {
    ret = WSAStartup(ver, &wdata);
    if (ret) {
      printf("Error in WSAStartup,return code:%d\n", ret);
    }
    winsock_started = 1;
  }
#endif /* HAVE_WINDOWS_H */
  if((address != NULL) && (strlen(address) > 0))
  {
    close(rdev_fd);
    close(sock);
    do_once = 1;
    connected = 1;
    memset ( &peer_in, 0, sizeof ( struct sockaddr_in ) );
    /*rdev_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);*/
    rdev_fd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef HAVE_WINDOWS_H
    ioctlsocket(rdev_fd, FIONBIO, &ioctlsocket_non_block);
#else
    fcntl(rdev_fd, F_SETFL, O_NONBLOCK);
#endif
    peer_in.sin_family = AF_INET;
    if(inet_pton(AF_INET, address, &peer_in.sin_addr) == 0)
    { /* invalid address if zero */
      if((peer_in.sin_addr.s_addr == -1) || (peer_in.sin_addr.s_addr == 0))
      {
        host = gethostbyname(address);
        if(host != NULL)
        {
          snprintf(MESSAGE, sizeof(MESSAGE), "R*: Host = '%s'.",  host->h_name);
          DBG_APRINT(MESSAGE);
          memcpy((caddr_t)&peer_in.sin_addr, host->h_addr_list[0], host->h_length);
        }
        else
        {
          perror("gethostbyname");
        }
      }
    }

    if(port > 0)
    {
      peer_in.sin_port = htons (port);
    }
    else
    {  /* telnet port */
      peer_in.sin_port = htons (23);
    }
    if(connect(rdev_fd, (struct sockaddr *)&peer_in, sizeof(peer_in)) < 0)
    {
#ifdef DEBUG
      snprintf(MESSAGE, sizeof(MESSAGE), "R*: connect: '%s'", strerror(errno));
      DBG_APRINT(MESSAGE);
#endif
    }
#ifndef HAVE_WINDOWS_H
    signal(SIGPIPE, catch_disconnect); /*Need to see if the other end disconnects...*/
    signal(SIGHUP, catch_disconnect); /*Need to see if the other end disconnects...*/
#endif /* HAVE_WINDOWS_H */
    snprintf(MESSAGE, sizeof(MESSAGE), "R*: Connecting to %s", address);
    DBG_APRINT(MESSAGE);
#ifdef HAVE_WINDOWS_H
    ioctlsocket(rdev_fd, FIONBIO, &ioctlsocket_non_block);
#else
    fcntl(rdev_fd, F_SETFL, O_NONBLOCK);
#endif /* HAVE_WINDOWS_H */

    /* Telnet negotiation */
    snprintf(MESSAGE, sizeof(MESSAGE), "%c%c%c%c%c%c%c%c%c", 0xff, 0xfb, 0x01, 0xff, 0xfb, 0x03, 0xff, 0xfd, 0x0f3);
    write(rdev_fd, MESSAGE, 9);
    DBG_APRINT("R*: Negotiating Terminal Options...");
  }
}
#endif /* R_NETWORK */


#if defined(R_SERIAL) && !defined(DREAMCAST)
#ifdef __linux__
#define TTY_DEV_NAME "/dev/ttyS0"   /* Linux */
#elif defined (__NetBSD__) && defined(__i386__)
#define TTY_DEV_NAME "/dev/tty00"   /* NetBSD/x86 */
#elif defined (__FreeBSD__) && defined(__i386__)
#define TTY_DEV_NAME "/dev/ttyd1"   /* FreeBSD/x86 */
#elif defined (__sun__)
#define TTY_DEV_NAME "/dev/ttya"    /* Solaris */
#else
#error tty name unknown!
#endif
#endif /* defined(R_SERIAL) && !defined(DREAMCAST) */

/*---------------------------------------------------------------------------
   Host Support Function - Serial Open Connection
---------------------------------------------------------------------------*/
#ifdef R_SERIAL
static void open_connection_serial(int port)
{
#ifdef DREAMCAST
  dc_init_serial();
  connected = 1;
#else /* above DREAMCAST, below not */
  char dev_name[FILENAME_MAX] = TTY_DEV_NAME; /* reinitialize each time */
  struct termios options;

  if(connected)
    close(rdev_fd);
  do_once = 1;

  if (*RDevice_serial_device)  /* got a device name from command line */
  {
    strcpy(dev_name, RDevice_serial_device);
  }

  dev_name[strlen(dev_name) - 1] += port - 1;

  snprintf(MESSAGE, sizeof(MESSAGE), "R*: using serial device %s", dev_name);
  DBG_APRINT(MESSAGE);

  rdev_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
  if(rdev_fd == -1)
  {
    connected = 0;
    perror("R*: open_port: Unable to open serial Port - ");
  }
  else
  {
    connected = 1;
#if 0
    fcntl(rdev_fd, F_SETFL, O_NONBLOCK);
#endif
    /*Set 8N1 by default on open*/
    /*Set Baud to 115200 by default;*/
    tcgetattr(rdev_fd, &options);
    options.c_lflag = 0;
    options.c_iflag = 0;
    options.c_oflag = 0;
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    options.c_cflag = CS8 | CREAD | CLOCAL;
#if 0
    cfmakeraw(&options);
/*#else*/
    /* from glibc 2.3.2 */
    options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    options.c_cflag &= ~(CSIZE|PARENB);
    options.c_cflag |= CS8;
    options.c_cc[VMIN] = 1;            /* read returns when one char is available.  */
    options.c_cc[VTIME] = 0;
#endif

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    tcsetattr(rdev_fd, TCSANOW, &options);
  }
#endif /* not DREAMCAST */
}
#endif /* R_SERIAL */

/*---------------------------------------------------------------------------
   Host Support Function - Retrieve address from open command:
   From Basic: OPEN #1,8,23,"R:JYBOLAC.HOMELINUX.COM"
   Returns:    "jybolac.homelinux.com"
---------------------------------------------------------------------------*/
#ifdef R_NETWORK
static void RDevice_GetInetAddress(void)
{
  UWORD bufadr = Devices_SkipDeviceName();
  char *p;

  if (bufadr == 0) {
    /* XXX: signal error */
    inetaddress[0] = '\0';
    return;
  }

  p = inetaddress;
  while (p < inetaddress + sizeof(inetaddress) - 1) {
    char c = (char) MEMORY_dGetByte(bufadr);
    if (c < '!' || c > '\x7e')
      break;
    if (c >= 'A' && c <= 'Z')
      c += 'a' - 'A';
    *p++ = c;
    bufadr++;
  }
  *p = '\0';
}
#endif /* R_NETWORK */


/*---------------------------------------------------------------------------
   R Device OPEN vector - called from Atari OS Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_OPEN(void)
{
  int  port;
  int  direction;
  int  devnum;

  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;

  bufend = 0;

  port = Peek(Devices_ICAX2Z);
  direction = Peek(Devices_ICAX1Z);
  devnum = MEMORY_dGetByte(Devices_ICDNOZ);
  if(direction & 0x04)
  {
    DBG_APRINT("R*: Open for Reading...");
  }

  if(direction & 0x08)
  {
    DBG_APRINT("R*: Open for Writing...");
#ifdef R_SERIAL
    if(RDevice_serial_enabled)
    {
      DBG_APRINT("R*: serial mode.");
      open_connection_serial(devnum);
    }
#endif /* R_SERIAL */
#if defined(R_SERIAL) && defined(R_NETWORK)
    else
#endif
#ifdef R_NETWORK
    {
      DBG_APRINT("R*: Socket mode.");
      RDevice_GetInetAddress();
      open_connection(inetaddress, port);
    }
#endif /* R_NETWORK */
  }
  if(direction & 0x01)
  {
    /* Open for concurrent mode */
  }

}

/*---------------------------------------------------------------------------
   R Device CLOSE vector - called from Atari OS Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_CLOS(void)
{
  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;
  concurrent = 0;
  bufend = 0;
  close(rdev_fd);
}

/*---------------------------------------------------------------------------
   R Device READ vector - called from Atari OS Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_READ(void)
{
  int j;

  /*bufend = Peek(747);*/
  /*printf("Bufend = %d.\n", bufend);*/

/*  if(bufend >= 0) */
/*  { */
    if(translation)
    {
      if(bufout[0] == 0x0d)
      {
        CPU_regA = 0x9b;
      }
    }
    else
    {
      CPU_regA = bufout[0];
    }

    bufend--;

    for(j = 0; j <= bufend; j++)
    {
      bufout[j] = bufout[j+1];
    }

    /*Cycle the buffer again to skip over linefeed....*/
    if(translation && linefeeds && (bufout[0] == 0x0a))
    {
      for(j = 0; j <= bufend; j++)
      {
        bufout[j] = bufout[j+1];
      }
    }
    /*return; ???*/
/*  } */

  CPU_regY = 1;
  CPU_ClrN;
}



/*---------------------------------------------------------------------------
   R Device WRITE vector - called from Atari OS Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_WRIT(void)
{
  unsigned char out_char;
#ifdef R_NETWORK
  int port;
#endif

  CPU_regY = 1;
  CPU_ClrN;

  /*bufend = Peek(747);*/

  /* Translation mode */
  if(translation)
  {
    if(CPU_regA == 0x9b)
    {
      out_char = 0x0d;
      if(linefeeds)
      {
        if((RDevice_serial_enabled == 0) && (connected == 0))
        { /* local echo */
          bufend++;
          bufout[bufend-1] = out_char;
          bufout[bufend] = 0;

          command_end = 0;
          command_buf[command_end] = 0;
          strcat(bufout, "OK\r\n");
          bufend += 4;

        }
        else
        {
#ifndef DREAMCAST
          write(rdev_fd, (char *)&out_char, 1); /* Write return */
#else
          dc_write_serial(out_char);
#endif
        }
        out_char = 0x0a;  /*set char for line feed to be output later....*/
      }
    }
  }
  else
  {
    out_char = CPU_regA;
  }

  /* Translate the CR to a LF for telnet, ftp, etc */
  if(connected && trans_cr && (out_char == 0x0d))
  {
    out_char = 0x0a;
  }

  /*if((RDevice_serial_enabled == 0) && (out_char == 255))*/
  /*{*/
  /*  DBG_APRINT("R: Writing IAC..."); */
  /*  retval = write(rdev_fd, &out_char, 1);*/ /* IAC escape sequence */ 
  /*}*/
  /*if(retval == -1)*/

#ifdef R_NETWORK
  if((RDevice_serial_enabled == 0) && (connected == 0))
  { /* Local echo - only do if in socket mode */
    bufend++;
    bufout[bufend-1] = out_char;
    bufout[bufend] = 0;

    /* Grab Command */
    if((out_char == 0x9b) || (out_char == 0x0d))
    { /*process command with a return/enter*/
      command_end = 0;

      /*Make out going connection command 'ATDI'*/
      if((command_buf[0] == 'A') && (command_buf[1] == 'T') && (command_buf[2] == 'D') && (command_buf[3] == 'I'))
      {
        /*DBG_APRINT(command_buf);*/
        if(strchr(command_buf, ' ') != NULL)
        {
          if(strrchr(command_buf, ' ') != strchr(command_buf, ' '))
          {
            port = atoi((char *)(strrchr(command_buf, ' ')+1));
            * strrchr(command_buf, ' ') = '\0'; /*zero last space in line*/
          }
          else
          {
            port = 23;
          }
          open_connection((char *)(strchr(command_buf, ' ')+1), port); /*send string after first space in line*/
        }
        command_buf[command_end] = 0;
        strcat(bufout, "OK\r\n");
        bufend += 4;
      /*Change translation command 'ATDL'*/
      }
      else if((command_buf[0] == 'A') && (command_buf[1] == 'T') && (command_buf[2] == 'D') && (command_buf[3] == 'L'))
      {
        trans_cr = (trans_cr + 1) % 2;

        command_buf[command_end] = 0;
        strcat(bufout, "OK\r\n");
        bufend += 4;
      }
    }
    else
    {
      if(((out_char == 0x7f) || (out_char == 0x08)) && (command_end > 0))
      {
        command_end--; /* backspace */
        command_buf[command_end] = 0;
      }
      else
      {
        command_buf[command_end] = out_char;
        command_buf[command_end+1] = 0;
        command_end = (command_end + 1) % 256;
      }
    }
  }
  else
#endif /* R_NETWORK */
#ifndef DREAMCAST
    if((connected) && (write(rdev_fd, (char *)&out_char, 1) < 1))
    { /* returns -1 if disconnected or 0 if could not send */
      perror("write");
      DBG_APRINT("R*: ERROR on write.");
      CPU_SetN;
      CPU_regY = 135;
      /*bufend = 13;*/ /* To catch NO CARRIER message */
    }
#else
  if (connected)
  {
    if (dc_write_serial(out_char) != 1)
    {
      Log_print("R*: ERROR on write.");
      CPU_SetN;
      CPU_regY = 135;
    }
  }
#endif

  CPU_regA = 1;
}

/*---------------------------------------------------------------------------
   R Device GET STATUS vector - called from Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_STAT(void)
{
#ifdef HAVE_WINDOWS_H
  int len;
#else
#ifdef R_NETWORK
  unsigned int len;
#endif
#endif
  int bytesread;
  unsigned char one;
  int devnum;
  int on;
  unsigned char telnet_command[2];
  /*static char IACctr = 0;*/
  on = 1;

  if(Peek(764) == 1)
  { /* Hack for Ice-T Terminal program to work! */
    Poke(764, 255);
  }
  devnum = MEMORY_dGetByte(Devices_ICDNOZ);

#ifdef R_NETWORK
  if(connected == 0)
  {
    if(RDevice_serial_enabled == 0)
    {
      if(do_once == 0)
      {

        /*strcpy(PORT,"23\n");*/
        /*strcpy(PORT,"8000\n");*/
        /*sprintf(PORT, "%d", 8000 + devnum);*/
        portnum = portnum + devnum - 1;

        /* Set up the listening port. */
        do_once = 1;
        memset ( &in, 0, sizeof ( struct sockaddr_in ) );
        sock = socket ( AF_INET, SOCK_STREAM, 0 );
        in.sin_family = AF_INET;
        in.sin_addr.s_addr = INADDR_ANY;
        /*in.sin_port = htons ( atoi ( PORT ) );*/
        in.sin_port = htons (portnum);
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on) ); /* cmartin */
        if(bind ( sock, (struct sockaddr *)&in, sizeof ( struct sockaddr_in ) ) < 0) perror("bind");
        listen ( sock, 5 );
        /* sethostent(1); */
#ifdef HAVE_WINDOWS_H
        retval = ioctlsocket(sock, FIONBIO, &ioctlsocket_non_block);
#else
        retval = fcntl( sock, F_SETFL, O_NONBLOCK);
#endif /* HAVE_WINDOWS_H */
        len = sizeof ( struct sockaddr_in );
        /*bufend = 0;*/
        snprintf(MESSAGE, sizeof(MESSAGE), "R%d: Listening on port %d...", devnum, portnum);
        DBG_APRINT(MESSAGE);
      }

      rdev_fd = accept ( sock, (struct sockaddr *)&peer_in, &len );
      if(rdev_fd != -1)
      {
        struct hostent *host;
        if (getpeername(rdev_fd, (struct sockaddr *) &peer_in, &len) < 0)
        {
          perror("getpeername");
        }
        else
        {
          snprintf(MESSAGE, sizeof(MESSAGE), "R%d: Serving Connection from %s...", devnum, inet_ntoa(peer_in.sin_addr));
          if ((host = gethostbyaddr((char *) &peer_in.sin_addr, sizeof peer_in.sin_addr, AF_INET)) == NULL)
          {
            /*perror("gethostbyaddr");*/
            /*DBG_APRINT("Connected.");*/
          } else {
            snprintf(MESSAGE, sizeof(MESSAGE), "R%d: Serving Connection from %s.", devnum, host->h_name);
          }
        }
        DBG_APRINT(MESSAGE);
#ifndef HAVE_WINDOWS_H
        signal(SIGPIPE, catch_disconnect); /*Need to see if the other end disconnects...*/
        signal(SIGHUP, catch_disconnect); /*Need to see if the other end disconnects...*/
#endif /* HAVE_WINDOWS_H */
#ifdef HAVE_WINDOWS_H
        retval = ioctlsocket(rdev_fd, FIONBIO, &ioctlsocket_non_block);
#else
        retval = fcntl( rdev_fd, F_SETFL, O_NONBLOCK);
#endif /* HAVE_WINDOWS_H */

        /* Telnet negotiation */
        snprintf(MESSAGE, sizeof(MESSAGE), "%c%c%c%c%c%c%c%c%c", 0xff, 0xfb, 0x01, 0xff, 0xfb, 0x03, 0xff, 0xfd, 0x0f3);
        write(rdev_fd, MESSAGE, 9);
        DBG_APRINT("R*: Negotiating Terminal Options...");

        connected = 1;
  /*
        retval = write(rdev_fd, &IACdoBinary, 3);
        retval = write(rdev_fd, &IACwillBinary, 3);
        retval = write(rdev_fd, &IACdontLinemode, 3);
        retval = write(rdev_fd, &IACwontLinemode, 3);
  */
        bufout[0] = 0;
        strcat(bufout, CONNECT_STRING);
        bufend = strlen(CONNECT_STRING);
        close(sock);
      }
    }
  }
  else
#endif /* R_NETWORK */
  {
    /* Actually reading and setting the Atari input buffer here */
    if(concurrent)
    {
#ifndef DREAMCAST
      bytesread = read(rdev_fd, (char *)&one, 1);
#else
      bytesread = dc_read_serial(&one);
#endif
      if(bytesread > 0)
      {
        if((RDevice_serial_enabled == 0) && (one == 0xff))
        {
          /* Start Telnet escape seq processing... */
          while(read(rdev_fd, (char *)telnet_command,2) != 2) {};

          /*sprintf(MESSAGE, "Telnet Command = 0x%x 0x%x", telnet_command[0], telnet_command[1]);*/
          /*DBG_APRINT(MESSAGE);*/
          if(telnet_command[0] ==  0xfd)
          { /*DO*/
            if((telnet_command[1] == 0x01) || (telnet_command[1] == 0x03))
            { /* WILL ECHO and GO AHEAD (char mode) */
              telnet_command[0] = 0xfb; /* WILL */
            }
            else
            {
              telnet_command[0] = 0xfc; /* WONT */
            }
          }
          else if(telnet_command[0] == 0xfb)
          { /*WILL*/
            /*telnet_command[0] = 0xfd;*/ /*DO*/
            telnet_command[0] = 0xfe; /*DONT*/
          }
          else if(telnet_command[0] == 0xfe)
          { /*DONT*/
            telnet_command[0] = 0xfc;
          }
          else if(telnet_command[0] == 0xfc)
          { /*WONT*/
            telnet_command[0] = 0xfe;
          }

          if(telnet_command[0] == 0xfa)
          { /* subnegotiation */
            while(read(rdev_fd, (char *)&one, 1) != 1) {};

            while(one != 0xf0)
            { /* wait for end of sub negotiation */
              while(read(rdev_fd, (char *)&one, 1) != 1) {};
            }
          }
          else
          {
            write(rdev_fd, (char *)&one, 1);
            write(rdev_fd, (char *)telnet_command, 2);
          }
        }
        else
        {
          bufend++;
          bufout[bufend-1] = one;
          bufout[bufend] = 0;
          /*return;*/
        }
      }
    }
  }

  /* Set all values at all memory locations we modify on exit */
  Poke(746,0);
  Poke(748,0);
  Poke(749,0);
  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;

  if(concurrent)
  {
    Poke(747,bufend);
  }
  else
  {
    DBG_APRINT("R*: Not in concurrent mode....");
    /*Poke(747,8);*/
    Poke(747,(12+48+192)); /* Write 0xfc to address 747 */
  }
}

/*---------------------------------------------------------------------------
   R Device SPECIAL vector - called from Atari OS Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_SPEC(void)
{
  int iccom;

  iccom = Peek(Devices_ICCOMZ);
  snprintf(MESSAGE, sizeof(MESSAGE), "R*: XIO %d", iccom);
  DBG_APRINT(MESSAGE);

/*
  DBG_APRINT("Devices_ICCOMZ =");
  DBG_APRINT("%d",iccom);
  DBG_APRINT("^^ in Devices_ICCOMZ");
*/

  switch (iccom)
  {
    case 32: /*Force Short Block*/
      break;
    case 34:
      xio_34();
      break;
    case 36:
      xio_36();
      break;
    case 38:
      xio_38();
      break;
    case 40:
      xio_40();
      break;
    default:
      DBG_APRINT("R*: Unsupported XIO #.");
      break;
  }
/*
  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;
*/

}

/*---------------------------------------------------------------------------
   R Device INIT vector - called from Atari OS Device Handler Address Table
---------------------------------------------------------------------------*/
void RDevice_INIT(void)
{
  DBG_APRINT("R*: INIT");
  CPU_regA = 1;
  CPU_regY = 1;
  CPU_ClrN;
}

void RDevice_Exit(void)
{
#ifdef HAVE_WINDOWS_H
  WSACleanup();
#endif /* HAVE_WINDOWS_H */
}
