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

// Protocol versions:
//  1 - I forgot!
//  2 - Added support for more versatile input configurations...somewhat

#include "mednafen.h"

#include <stdarg.h>
#include <string.h>
#include <zlib.h>
#include <string>
#include <math.h>
#include <trio/trio.h>

#include "netplay.h"
#include "netplay-driver.h"
#include "general.h"
#include "state.h"
#include "movie.h"
#include "endian.h"
#include "md5.h"

int MDFNnetplay=0;

#ifdef NETWORK
static char *OurNick = NULL;

static bool Joined = 0;
static uint32 LocalPlayersMask = 0;
static uint32 TotalInputStateSize = 0;
static uint32 LocalInputStateSize = 0;

/* NetError should only be called after a MDFND_*Data function returned 0, in the function
   that called MDFND_*Data, to prevent it from being called twice.
*/

static void NetError(const char *format, ...)
{
 char *temp = NULL;
 va_list ap;

 va_start(ap, format);
 temp = trio_vaprintf(format, ap);
 va_end(ap);

 MDFND_NetplayText((UTF8 *)temp, FALSE);
 MDFND_NetworkClose();
 free(temp);
}

void MDFNI_NetplayStop(void)
{
	if(MDFNnetplay)
	{
	 MDFNnetplay = 0;
 	 MDFN_FlushGameCheats(1);	/* Don't save netplay cheats. */
 	 MDFN_LoadGameCheats(0);		/* Reload our original cheats. */
	 if(OurNick)
	 {
	  free(OurNick);
	  OurNick = NULL;
	 }
	}
	else puts("Check your code!");
}

int NetplayStart(const char *PortDeviceCache[16], const uint32 PortDataLenCache[16], uint32 local_players, uint32 netmerge, const std::string &nickname, const std::string &game_key, const std::string &connect_password)
{
 uint8 *sendbuf;
 uint32 sblen;
 std::string controller_types;

 // Concatenate!
 for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
  controller_types += std::string(PortDeviceCache[x]) + "\n";

 sblen = 4 + 16 + 16 + 64 + 1 + nickname.size() + controller_types.size();
 sendbuf = (uint8 *)malloc(sblen);
 memset(sendbuf, 0, sblen);

 MDFN_en32lsb(sendbuf, sblen - 4);
 if(game_key != "")
 {
  md5_context md5;
  uint8 md5out[16];

  md5.starts();
  md5.update(MDFNGameInfo->MD5, 16);
  md5.update((uint8 *)game_key.c_str(), game_key.size());
  md5.finish(md5out);
  memcpy(sendbuf + 4, md5out, 16);
 }
 else
  memcpy(sendbuf + 4, MDFNGameInfo->MD5, 16);

 if(connect_password != "")
 {
  md5_context md5;
  uint8 md5out[16];

  md5.starts();
  md5.update((uint8*)connect_password.c_str(), connect_password.size());
  md5.finish(md5out);
  memcpy(sendbuf + 4 + 16, md5out, 16);
 }

 uint8 *extra = sendbuf + 4 + 16 + 16;

 extra[0] = 2; // Protocol version

 // Set input device number thingies here.
 extra[1] = MDFNGameInfo->InputInfo->InputPorts; // Total number of ports

 // The size of the giganto string with the controller types
 MDFN_en32lsb(&extra[4], controller_types.size());

 // 16-32, controller data sizes
 for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
  extra[16 + x] = PortDataLenCache[x];


 sendbuf[4 + 16 + 16 + 32] = MDFN_GetSettingUI("netmerge");

 sendbuf[4 + 16 + 16 + 64] = local_players;

 if(nickname != "")
  memcpy(sendbuf + 4 + 16 + 16 + 64 + 1, nickname.c_str(), nickname.size());

 memcpy(sendbuf + 4 + 16 + 16 + 64 + 1 + nickname.size(), controller_types.c_str(), controller_types.size());

 MDFND_SendData(sendbuf, sblen);

 TotalInputStateSize = 0;
 for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
   TotalInputStateSize += PortDataLenCache[x];

 // Hack so the server can always encode its command data length properly(a matching "hack" exists in the server).
 if(TotalInputStateSize < 4)
  TotalInputStateSize = 4;

 LocalPlayersMask = 0;
 LocalInputStateSize = 0; 
 Joined = 0;

 MDFN_FlushGameCheats(0);	/* Save our pre-netplay cheats. */
 MDFNnetplay = 1;

 if(MDFNMOV_IsPlaying())		/* Recording's ok during netplay, playback is not. */
  MDFNMOV_Stop();

 //printf("%d\n", TotalInputStateSize);

 return(1);
}

int MDFNNET_SendCommand(uint8 cmd, uint32 len)
{
 uint8 buf[1 + LocalInputStateSize + 4]; // Command, unused, command length

 memset(buf, 0, sizeof(buf));

 buf[0] = cmd;
 MDFN_en32lsb(&buf[1 + LocalInputStateSize], len);
 if(!MDFND_SendData(buf,LocalInputStateSize + 1 + 4)) 
 {
  NetError("Could not send command.");
  return(0);
 }
 return(1);
}

void MDFNI_NetplayText(const uint8 *text)
{
 uint32 len;

 if(!Joined) return;

 len = strlen((char *)text);

 if(!MDFNNET_SendCommand(MDFNNPCMD_TEXT,len)) return;

 if(!MDFND_SendData(text,len))
  NetError("Could not send text data.");
}

void MDFNI_NetplayChangeNick(UTF8 *newnick)
{
 uint32 len;

 if(!Joined) return;

 len = strlen((char *)newnick);

 if(!MDFNNET_SendCommand(MDFNNPCMD_SETNICK,len)) return;

 if(!MDFND_SendData(newnick, len))
  NetError("Could not send new nick data.");
}

int MDFNNET_SendState(void)
{
 StateMem sm;
 uLongf clen;
 uint8 *cbuf;

 memset(&sm, 0, sizeof(StateMem));

 if(!MDFNSS_SaveSM(&sm, 0, 0))
  return(0);

 clen = sm.len + sm.len / 1000 + 12;
 cbuf = (uint8 *)malloc(4 + clen);
 MDFN_en32lsb(cbuf, sm.len);
 compress2((Bytef *)cbuf + 4, &clen, (Bytef *)sm.data, sm.len, 7);

 if(!MDFNNET_SendCommand(MDFNNPCMD_LOADSTATE,clen + 4))
 {
  free(cbuf);
  NetError(_("Could not send the save state command to the netplay server."));
  return(0);
 }

 if(!MDFND_SendData(cbuf, clen + 4))
 {
  NetError(_("Could not send the save state data to the netplay server."));
  free(cbuf);
  return(0);
 }

 free(sm.data);
 free(cbuf);

 return(1);
}

int MDFNNET_RecvState(uint32 clen)
{
 StateMem sm;
 uint8 *cbuf;

 memset(&sm, 0, sizeof(StateMem));

 if(clen > 4000000) // Sanity check
 {
  NetError("Compressed save state data is too large: %d", clen);
  return(0);
 }
 cbuf = (uint8*)malloc(clen);
 if(!MDFND_RecvData(cbuf, clen))
 {
  NetError("Could not receive compressed save state data.");
  free(cbuf);
  return(0);
 }
 uLongf len = MDFN_de32lsb((uint8 *)cbuf);
 if(len > 4000000)
 {
  NetError("Decompressed save state data is too large: %d", clen);
  free(cbuf);
  return(0);
 }
 uint8 *buf = (uint8 *)malloc(len);
 uncompress((Bytef *)buf, &len, (Bytef *)cbuf + 4, clen - 4);

 sm.data = buf;
 sm.len = len;
 if(!MDFNSS_LoadSM(&sm, 0, 0))
 {
  NetError("Error during save state loading.");
  return(0);
 }
 if(MDFNMOV_IsRecording())
  MDFNMOV_RecordState();
 return(1);
}

void NetplayUpdate(const char **PortDNames, void *PortData[], uint32 PortLen[], int NumPorts)
{
 uint8 buf[TotalInputStateSize + 1];

 if(Joined)
 {
  uint8 outgoing_buffer[1 + LocalInputStateSize];
  bool Taken[NumPorts];

  memset(Taken, 0, sizeof(Taken));

  outgoing_buffer[0] = 0; // This is not a command, duh!

  int wpos = 1;

  for(int x = 0; x < NumPorts; x++)  
  {
   if(LocalPlayersMask & (1 << x))
   {

    for(int n = 0; n <= x; n++)
    {
     if(!Taken[n] && !strcmp(PortDNames[n], PortDNames[x]))
     {
      memcpy(outgoing_buffer + wpos, PortData[n], PortLen[n]);
      Taken[n] = TRUE;
      wpos += PortLen[n];
      break;
     }
    }

   }
  }
  if(!MDFND_SendData(outgoing_buffer, 1 + LocalInputStateSize))
  {
   NetError("Sending joystick update data failed.");
   return;
  }
 }

 do
 {
  if(!MDFND_RecvData(buf, TotalInputStateSize + 1))
  {
   NetError("Could not receive joystick update data.");
   return;
  }

  switch(buf[TotalInputStateSize])
  {
   case 0: break; // No command

   default: MDFN_DoSimpleCommand(buf[TotalInputStateSize]);break;

   case MDFNNPCMD_SAVESTATE:	
			if(!MDFNNET_SendState())
			{
			 return;
			}
	  	 	break;
   case MDFNNPCMD_LOADSTATE:  
			if(!MDFNNET_RecvState(MDFN_de32lsb(buf)))
			{
			 return;
			}
			MDFN_DispMessage(_("Remote state loaded."));
			break;
   case MDFNNPCMD_TEXT:
			{
			 uint32 totallen = MDFN_de32lsb(buf);
			 if(totallen > 2000) // Sanity check
			 {
                          NetError("Text length is too long: %d", totallen);
			  return;
			 }
			 uint32 nicklen;
			 uint8 neobuf[totallen + 1];
			 char *textbuf = NULL;
			 if(!MDFND_RecvData(neobuf, totallen))
			 {
			  NetError("Could not receive text data.");
			  return;
			 }
			 nicklen = MDFN_de32lsb(neobuf);
			 if(nicklen > totallen) // Sanity check
			 {
			  NetError("Received nickname length is too long: %d", nicklen);
			  return;
			 }
                         neobuf[totallen] = 0;
			 bool NetEcho = 0;

			 if(nicklen)
			 {
			  uint8 nickbuf[nicklen + 1];
			  memcpy(nickbuf, neobuf + 4, nicklen);
			  nickbuf[nicklen] = 0;
			  if(OurNick && !strcasecmp(OurNick, (char *)nickbuf))
			  {
                           trio_asprintf(&textbuf, "> %s", &neobuf[4 + nicklen]);
			   NetEcho = TRUE;
			  }
			  else
			   trio_asprintf(&textbuf, "<%s> %s", nickbuf, &neobuf[4 + nicklen]);
			 }
		         else
			 {
			  trio_asprintf(&textbuf, "* %s", &neobuf[4]);
			 }
                         MDFND_NetplayText((UTF8*)textbuf, NetEcho);
			 free(textbuf);			
			}
			break;
   case MDFNNPCMD_NICKCHANGED:
			{
			 uint32 len = MDFN_de32lsb(buf);
			 uint8 neobuf[len + 1];
			 uint8 *newnick;
			 char *textbuf = NULL;

                         if(!MDFND_RecvData(neobuf, len))
                         {
                          NetError("Unable to receive data for nickname change.");
                          return;
                         }

			 neobuf[len] = 0;

			 newnick = (uint8*)strchr((char*)neobuf, '\n');

			 if(newnick)
			 {
			  bool IsMeow = FALSE;
			  *newnick = 0;
			  newnick++;
			  if(OurNick)
			  {
			   if(!strcasecmp((char*)neobuf, (char*)OurNick))
			   {
			    free(OurNick);
			    OurNick = strdup((char*)newnick);
			    textbuf = trio_aprintf(_("* You are now known as <%s>."), newnick);
			    IsMeow = TRUE;
			   }
			  }
			  if(!textbuf)
			   textbuf = trio_aprintf(_("* <%s> is now known as <%s>"), neobuf, newnick);
                          MDFND_NetplayText((UTF8*)textbuf, IsMeow);
			  free(textbuf);
			 }
			}
			break;

   case MDFNNPCMD_YOUJOINED:
   case MDFNNPCMD_YOULEFT:
   case MDFNNPCMD_PLAYERLEFT:
   case MDFNNPCMD_PLAYERJOINED:
			{
	                 uint32 len = MDFN_de32lsb(buf);
			 uint8 neobuf[len + 1];
			 char *textbuf = NULL;
			 char mergedstr[] = " merged into:  ";

                         if(!MDFND_RecvData(neobuf, len))
			 {				  
			  NetError("Unable to receive data for join/part message");
			  return;
			 }
			 neobuf[len] = 0; // NULL-terminate the string

			 if(neobuf[1]) // Player is merged?
			 {
			  mergedstr[strlen(mergedstr) - 1] = '1' + (int)rint(log(neobuf[1]) / log(2));
			 }
			 else
			  mergedstr[0] = 0;

			 if(buf[TotalInputStateSize] == MDFNNPCMD_YOULEFT)
			 {
			  // Uhm, not supported yet!
			  LocalPlayersMask = 0;
			  LocalInputStateSize = 0;
			  Joined = FALSE;
			 }
			 else if(buf[TotalInputStateSize] == MDFNNPCMD_YOUJOINED)
			 {
			  if(OurNick) // This shouldn't happen, really...
			  {
			   free(OurNick);
			   OurNick = NULL;
			  }
			  OurNick = strdup((char*)neobuf + 2);
                          trio_asprintf(&textbuf, _("* You, %s, have connected as player: %s%s%s%s%s%s"), neobuf + 2, (neobuf[0] & 1) ? "1 " : "",
                                       (neobuf[0] & 2) ? "2 " : "", (neobuf[0] & 4) ? "3 " : "", (neobuf[0] & 8) ? "4 " : "", 
				       (neobuf[0] & 0x10) ? "5 " : "", mergedstr);
			  LocalInputStateSize = 0;
			  LocalPlayersMask = neobuf[0];
			  for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
			  {
			   if(LocalPlayersMask & (1 << x))
			    LocalInputStateSize += PortLen[x];
			  }
			  Joined = TRUE;
			  MDFNNET_SendCommand(MDFNNPCMD_SETFPS, MDFNGameInfo->fps);
			 }
			 else if(buf[TotalInputStateSize] == MDFNNPCMD_PLAYERLEFT)
			 {
                                  trio_asprintf(&textbuf, _("* %s has left(player: %s%s%s%s%s%s)"), neobuf + 2, (neobuf[0] & 1) ? "1 " : "",
                                        (neobuf[0] & 2) ? "2 " : "", (neobuf[0] & 4) ? "3 " : "", (neobuf[0] & 8) ? "4 " : "", 
					(neobuf[0] & 0x10) ? "5 " : "", mergedstr);
			 }
			 else
			 {
                                  trio_asprintf(&textbuf, _("* %s has connected as player: %s%s%s%s%s%s"), neobuf + 2, (neobuf[0] & 1) ? "1 " : "",
					(neobuf[0] & 2) ? "2 " : "", (neobuf[0] & 4) ? "3 " : "", (neobuf[0] & 8) ? "4 " : "", 
					(neobuf[0] & 0x10) ? "5 " : "", mergedstr);
			 }
	                 MDFND_NetplayText((UTF8*)textbuf, FALSE);
			 free(textbuf);
			}
			break;
  }
 } while(buf[TotalInputStateSize]);

 int rpos = 0;
 for(int x = 0; x < NumPorts; x++)
 { 
  //printf("%d %d\n", x, PortLen[x]);
  //memset(PortData[x], 0, PortLen[x]);
  memcpy(PortData[x], buf + rpos, PortLen[x]);
  rpos += PortLen[x];
 }

}

#endif
