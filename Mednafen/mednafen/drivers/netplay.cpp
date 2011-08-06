/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "main.h"
#include <stdarg.h>
#include <SDL_net.h>
#include <string.h>
#include <math.h>
#include "netplay.h"
#include "console.h"
#include "../md5.h"

#include <trio/trio.h>

class NetplayConsole : public MDFNConsole
{
        public:
        NetplayConsole(void);

        private:
        virtual bool TextHook(UTF8 *text);
};

static NetplayConsole NetConsole;

// All command functions are called in the main(video blitting) thread.
typedef struct
{
 const char *name;
 bool (*func)(const UTF8 *arg);
 const char *help_args;
 const char *help_desc;
} CommandEntry;

static bool CC_server(const UTF8 *arg);
static bool CC_quit(const UTF8 *arg);
static bool CC_help(const UTF8 *arg);
static bool CC_nick(const UTF8 *arg);
static bool CC_ping(const UTF8 *arg);
static bool CC_integrity(const UTF8 *arg);
static bool CC_gamekey(const UTF8 *arg);

static CommandEntry ConsoleCommands[]   =
{
 { "/server", CC_server,	"REMOTE_HOST [PORT]", "Connects to REMOTE_HOST(IP address or FQDN), optionally on a custom PORT." },

 { "/connect", CC_server,	NULL, NULL },

// { "/gamekey", CC_gamekey,	"GAMEKEY", "Changes the game key to the specified GAMEKEY." },

 { "/quit", CC_quit,		"", "Disconnects from the netplay server."  },

 { "/help", CC_help,		"", "Help, I'm drowning in a sea of cliche metaphors!" },

 { "/nick", CC_nick,		"NICKNAME", "Changes your nickname to the specified NICKNAME." },

 { "/ping", CC_ping,		"", "Pings the server." },

 { "/integrity", CC_integrity,	"", "Starts netplay integrity check sequence." },

 { NULL, NULL },
};

static const int PopupTime = 3500;
static volatile int inputable = 0;
static volatile int viewable = 0;
static int64 LastTextTime = -1;

int MDFNDnetplay = 0;  // Only write/read this global variable in the game thread.

static bool CC_server(const UTF8 *arg)
{
 UTF8 server[300];
 unsigned int *port;

 server[0] = 0;

 port = (unsigned int *)malloc(sizeof(unsigned int));

 if(trio_sscanf((char*)arg, "%*[ ]%.299[^ :]%*[ :]%u", (char*)server, port) == 2)
 {

 }
 else
 {
  free(port);
  port = NULL;
 }

 SendCEvent(CEVT_NP_CONNECT, strdup((char*)server), port);
 return(FALSE);
}

static bool CC_gamekey(const UTF8 *arg)
{
// SendCEvent(CEVT_NP_SETGAMEKEY, strdup(arg), NULL);
 return(FALSE);
}

static bool CC_quit(const UTF8 *arg)
{
 SendCEvent(CEVT_NP_CONNECT, NULL, NULL);
 return(FALSE);
}

static bool CC_ping(const UTF8 *arg)
{
 SendCEvent(CEVT_NP_PING, NULL, NULL);
 return(FALSE);
}

static bool CC_integrity(const UTF8 *arg)
{
 SendCEvent(CEVT_NP_INTEGRITY, NULL, NULL);
 return(FALSE);
}

static bool CC_help(const UTF8 *arg)
{
 for(unsigned int x = 0; ConsoleCommands[x].name; x++)
 {
  if(ConsoleCommands[x].help_desc)
  {
   char help_buf[256];
   trio_snprintf(help_buf, 256, "%s %s  -  %s", ConsoleCommands[x].name, ConsoleCommands[x].help_args, ConsoleCommands[x].help_desc);
   NetConsole.WriteLine((UTF8*)help_buf);
  }
 }
 return(TRUE);
}

static bool CC_nick(const UTF8 *arg)
{
 char newnick[512];

 if(trio_sscanf((char*)arg, "%*[ ]%.511[^\n]", newnick))
 {
  SendCEvent(CEVT_NP_SETNICK, strdup(newnick), NULL);
 }
 return(FALSE);
}


NetplayConsole::NetplayConsole(void)
{
 //SetSmallFont(1);
}

// Called from main thread
bool NetplayConsole::TextHook(UTF8 *text)
{
         bool found = 0;
	 bool cc_retval = 0;

         for(unsigned int x = 0; ConsoleCommands[x].name; x++)
          if(!strncasecmp(ConsoleCommands[x].name, (char*)text, strlen(ConsoleCommands[x].name)) && text[strlen(ConsoleCommands[x].name)] <= 0x20)
          {
           cc_retval = ConsoleCommands[x].func(&text[strlen(ConsoleCommands[x].name)]);
           found = TRUE;
           free(text);
           break;
          }

         if(!found)
         {
	  MDFNI_NetplayText(text);
         }

	 if(cc_retval)
	  inputable = TRUE;
	 else
          inputable = FALSE;

	 if(text[0] || cc_retval)
	 {
          LastTextTime = SDL_GetTicks();
          viewable = TRUE;
	 }
	 else
	 {
	  viewable = FALSE;
	  LastTextTime = -1;
	 }
         return(1);
}

static TCPsocket Socket = NULL;

static void PrintNetStatus(const char *s)
{
 MDFND_NetplayText((uint8 *)s, FALSE);
}

static void PrintNetError(const char *format, ...)
{
 char *temp;

 va_list ap;

 va_start(ap, format);

 temp = trio_vaprintf(format, ap);
 MDFND_NetplayText((uint8 *)temp, FALSE);
 free(temp);

 va_end(ap);
}

// Called from game thread
int MDFND_NetworkConnect(void)
{
 IPaddress IPa;

 if(Socket) // Disconnect if we're already connected.  TODO:  Refactor this.
 {
  MDFND_NetworkClose();
 }

 if(SDLNet_Init() == -1)
 {
  PrintNetStatus(_("*** Error intializing SDL_net!"));
  return(0);
 }

 std::string nickname = MDFN_GetSettingS("netplay.nick");
 std::string remote_host = MDFN_GetSettingS("netplay.host");
 unsigned int remote_port = MDFN_GetSettingUI("netplay.port");
 std::string game_key = MDFN_GetSettingS("netplay.gamekey");

 if(SDLNet_ResolveHost(&IPa, remote_host.c_str(), remote_port) == -1)
 {
  PrintNetError(_("*** Error resolving host \"%s\"!"), remote_host.c_str());
  return(0);
 }

 Socket = SDLNet_TCP_Open(&IPa);
 if(!Socket)
 {
  PrintNetError(_("*** Error connecting to remote host \"%s\" on port %u!"), remote_host.c_str(), remote_port);
  return(0);
 }
 
 PrintNetStatus(_("*** Sending initialization data to server."));

 MDFNDnetplay = 1;
 if(!MDFNI_NetplayStart(MDFN_GetSettingUI("netplay.localplayers"), MDFN_GetSettingUI("netplay.merge"), nickname, game_key, MDFN_GetSettingS("netplay.password")))
 {
  MDFNDnetplay = 0;
  return(0);
 }
 PrintNetStatus(_("*** Connection established."));

 return(1);
}

// Called from game thread
int MDFND_SendData(const void *data, uint32 len)
{
 SDLNet_TCP_Send(Socket, (void *)data, len); // Stupid non-constness!
 return(1);
}

// Called from game thread
int MDFND_RecvData(void *data, uint32 len)
{
  NoWaiting&=~2;
   
  SDLNet_SocketSet funfun;

  funfun = SDLNet_AllocSocketSet(1);
  SDLNet_TCP_AddSocket(funfun, Socket);

  for(;;)
  {
   switch(SDLNet_CheckSockets(funfun, 100000))
   {
    case 0: SDLNet_FreeSocketSet(funfun);continue;
    case -1: SDLNet_FreeSocketSet(funfun); printf("RecvData Failed on select(): %d\n", len); return(0);
   }

   if(SDLNet_SocketReady(Socket))
   {
    while(len)
    {
     int32 boop = SDLNet_TCP_Recv(Socket, data, len);
     if(boop <= 0)
     {
      puts(SDLNet_GetError());
      return(0);
     }
     data = (uint8 *)data + boop;
     len -= boop;
    }
    SDLNet_FreeSocketSet(funfun);
    funfun = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(funfun, Socket);
    if(SDLNet_CheckSockets(funfun, 0) == 1)
      NoWaiting|=2;
    SDLNet_FreeSocketSet(funfun);
    return(1);
   }

   SDLNet_FreeSocketSet(funfun);
  }
  printf("RecvData Failed: %d\n", len);
  return 0;
}

// Called from the game thread
void MDFND_NetworkClose(void)
{
 if(Socket)
  SDLNet_TCP_Close(Socket);
 Socket = NULL;

 if(MDFNDnetplay)
  MDFNI_NetplayStop();
 MDFNDnetplay = 0;
 NoWaiting&=~2;
}

// Called from the game thread
void MDFND_NetplayText(const uint8 *text, bool NetEcho)
{
 uint8 *tot = (uint8 *)strdup((char *)text);
 uint8 *tmp;
 tmp = tot;

 while(*tmp)
 {
  if(*tmp < 0x20) *tmp = ' ';
  tmp++;
 }

 SendCEvent(CEVT_NP_DISPLAY_TEXT, strdup((char*)text), (void*)NetEcho);
}

// Called from the game thread
void Netplay_ToggleTextView(void)
{
 SendCEvent(CEVT_NP_TOGGLE_TT, NULL, NULL);
}

// Called from main thread
int Netplay_GetTextView(void)
{
 return(viewable);
}

// Called from main thread and game thread
bool Netplay_IsTextInput(void)
{
 return(inputable);
}

// Called from the main thread
bool Netplay_TryTextExit(void)
{
 if(viewable || inputable)
 {
  viewable = FALSE;
  inputable = FALSE;
  LastTextTime = -1;
  return(TRUE);
 }
 else if(LastTextTime > 0 && (int64)SDL_GetTicks() < (LastTextTime + PopupTime + 500)) // Allow some extra time if a user tries to escape away an auto popup box but misses
 {
  return(TRUE);
 }
 else
 {
  return(FALSE);
 }
}

// Called from main thread
void DrawNetplayTextBuffer(SDL_Surface *surface, const SDL_Rect *src_rect)
{
 if(!viewable) 
 {
  return;
 }
 if(!inputable)
 {
  if((int64)SDL_GetTicks() >= (LastTextTime + PopupTime))
  {
   viewable = 0;
   return;
  }
 }
 NetConsole.ShowPrompt(inputable);
 NetConsole.Draw(surface, src_rect);
}

// Called from main thread
int NetplayEventHook(const SDL_Event *event)
{
 if(event->type == SDL_USEREVENT)
  switch(event->user.code)
  {
   case CEVT_NP_TOGGLE_TT:
	NetConsole.SetSmallFont(MDFN_GetSettingB("netplay.smallfont"));
	if(viewable && !inputable)
	{
	 inputable = TRUE;
	}
	else
	{
	 viewable = !viewable;
	 inputable = viewable;
	}
	break;

   case CEVT_NP_DISPLAY_TEXT:
	NetConsole.WriteLine((UTF8*)event->user.data1);
	free(event->user.data1);

	if(!(bool)event->user.data2)
	{
	 viewable = 1;
	 LastTextTime = SDL_GetTicks();
	}
	break;
  }

 if(!inputable)
  return(1);

 return(NetConsole.Event(event));
}

// Called from game thread
int NetplayEventHook_GT(const SDL_Event *event)
{
 if(event->type == SDL_USEREVENT)
  switch(event->user.code)
  {
   case CEVT_NP_INTEGRITY:
	MDFNI_NetplayIntegrity();
	break;

   case CEVT_NP_PING:
	MDFNI_NetplayPing();
	break;

   case CEVT_NP_SETNICK:
	MDFNI_SetSetting("netplay.nick", (char*)event->user.data1);
	if(MDFNDnetplay)
	 MDFNI_NetplayChangeNick((UTF8*)event->user.data1);
	free(event->user.data1);
	break;

   case CEVT_NP_CONNECT:
	if(event->user.data1) // Connect!
	{
	 MDFNI_SetSetting("netplay.host", (char*)event->user.data1);
	 if(event->user.data2)
	 {
	  MDFNI_SetSettingUI("netplay.port", *(unsigned int *)event->user.data2);
	  free(event->user.data2);
	 }
	 free(event->user.data1);
	 MDFND_NetworkConnect();
	}
	else
	{
	 MDFNI_NetplayStop();
	}
	break;
  }

 return(1);
}


