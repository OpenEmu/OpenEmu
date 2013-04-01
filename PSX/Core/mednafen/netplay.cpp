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
//  3 -

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
#include "md5.h"

#include "driver.h"

int MDFNnetplay=0;

static char *OurNick = NULL;

static bool Joined = 0;
static uint32 LocalPlayersMask = 0;
static uint32 TotalInputStateSize = 0;
static uint32 LocalInputStateSize = 0;

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

static void NetPrintText(const char *format, ...)
{
 char *temp = NULL;
 va_list ap;

 va_start(ap, format);
 temp = trio_vaprintf(format, ap);
 va_end(ap);

 MDFND_NetplayText((UTF8 *)temp, FALSE);
 free(temp);
}


void MDFNI_NetplayStop(void)
{
	if(MDFNnetplay)
	{
	 Joined = false;
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

int NetplayStart(const char *PortDeviceCache[16], const uint32 PortDataLenCache[16])
{
 uint8 *sendbuf;
 uint32 sblen;
 const char *emu_id = PACKAGE " " MEDNAFEN_VERSION;
 const uint32 local_players = MDFN_GetSettingUI("netplay.localplayers");
 const std::string nickname = MDFN_GetSettingS("netplay.nick");
 const std::string game_key = MDFN_GetSettingS("netplay.gamekey");
 const std::string connect_password = MDFN_GetSettingS("netplay.password");

 try
 {
  sblen = 4 + 16 + 16 + 64 + 1 + nickname.size() + strlen(emu_id);
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

  extra[0] = 3; // Protocol version

  // Set input device number thingies here.
  extra[1] = MDFNGameInfo->InputInfo->InputPorts; // Total number of ports

  // The size of the giganto string with the controller types
  MDFN_en32lsb(&extra[4], strlen(emu_id));

  // 16-32, controller data sizes
  for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
   extra[16 + x] = PortDataLenCache[x];


  sendbuf[4 + 16 + 16 + 64] = local_players;

  if(nickname != "")
   memcpy(sendbuf + 4 + 16 + 16 + 64 + 1, nickname.c_str(), nickname.size());

  memcpy(sendbuf + 4 + 16 + 16 + 64 + 1 + nickname.size(), emu_id, strlen(emu_id));

  MDFND_SendData(sendbuf, sblen);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
  return(false);
 }

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

static void SendCommand(uint8 cmd, uint32 len)
{
 uint8 buf[1 + LocalInputStateSize + 4]; // Command, unused, command length

 memset(buf, 0, sizeof(buf));

 buf[0] = cmd;
 MDFN_en32lsb(&buf[1 + LocalInputStateSize], len);
 MDFND_SendData(buf,LocalInputStateSize + 1 + 4);
}

bool NetplaySendCommand(uint8 cmd, uint32 len)
{
 try
 {
  SendCommand(cmd, len);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
  return(false);
 }
 return(true);
}

void MDFNI_NetplaySwap(uint8 a, uint8 b)
{
 try
 {
  SendCommand(MDFNNPCMD_CTRLR_SWAP, (a << 0) | (b << 8));
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayTake(uint32 mask)
{
 try
 {
  SendCommand(MDFNNPCMD_CTRLR_TAKE, mask);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayDrop(uint32 mask)
{
 try
 {
  SendCommand(MDFNNPCMD_CTRLR_DROP, mask);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayDupe(uint32 mask)
{
 try
 {
  SendCommand(MDFNNPCMD_CTRLR_DUPE, mask);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayList(void)
{
 try
 {
  SendCommand(MDFNNPCMD_REQUEST_LIST, 0);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}


void MDFNI_NetplayPing(void)
{
 try
 {
  uint64 now_time;

  now_time = MDFND_GetTime();

  SendCommand(MDFNNPCMD_ECHO, sizeof(now_time));

  // Endianness doesn't matter, since it will be echoed back only to us.
  MDFND_SendData(&now_time, sizeof(now_time));
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayIntegrity(void)
{
 try
 {
  SendCommand(MDFNNPCMD_INTEGRITY, 0);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayText(const uint8 *text)
{
 try
 {
  uint32 len;

  if(!Joined) return;

  len = strlen((char *)text);

  SendCommand(MDFNNPCMD_TEXT, len);

  MDFND_SendData(text,len);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayChangeNick(UTF8 *newnick)
{
 try
 {
  uint32 len;

  if(!Joined) return;

  len = strlen((char *)newnick);

  SendCommand(MDFNNPCMD_SETNICK, len);

  MDFND_SendData(newnick, len);
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void MDFNI_NetplayQuit(const char *quit_message)
{
 try
 {
  SendCommand(MDFNNPCMD_QUIT, strlen(quit_message));
  MDFND_SendData(quit_message, strlen(quit_message));
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}


// Integrity checking is experimental, and needs work to function properly(in the emulator cores).
static int SendIntegrity(void)
{
 StateMem sm;
 md5_context md5;
 uint8 digest[16];

 memset(&sm, 0, sizeof(StateMem));

 // Do not do a raw/data-only state for speed, due to lack of endian and bool conversion.
 if(!MDFNSS_SaveSM(&sm, 0, false))
 {
  throw MDFN_Error(0, _("Error during save state generation."));
 }

 md5.starts();
 md5.update(sm.data, sm.len);
 md5.finish(digest);

 free(sm.data);

 //for(int i = 15; i >= 0; i--)
 // printf("%02x", digest[i]);
 //puts("");

 SendCommand(MDFNNPCMD_INTEGRITY_RES, 16);
 MDFND_SendData(digest, 16);

 return(1);
}


static void SendState(void)
{
 StateMem sm;
 uLongf clen;
 std::vector<uint8> cbuf;

 memset(&sm, 0, sizeof(StateMem));

 if(!MDFNSS_SaveSM(&sm, 0, 0))
 {
  throw MDFN_Error(0, _("Error during save state generation."));
 }

 clen = sm.len + sm.len / 1000 + 12;
 cbuf.resize(4 + clen);
 MDFN_en32lsb(&cbuf[0], sm.len);
 compress2((Bytef *)&cbuf[0] + 4, &clen, (Bytef *)sm.data, sm.len, 7);
 free(sm.data);

 SendCommand(MDFNNPCMD_LOADSTATE, clen + 4);
 MDFND_SendData(&cbuf[0], clen + 4);
}

static void RecvState(const uint32 clen)
{
 StateMem sm;
 std::vector<uint8> cbuf;
 std::vector<uint8> buf;

 memset(&sm, 0, sizeof(StateMem));

 if(clen < 4)
 {
  throw MDFN_Error(0, _("Compressed save state data is too small: %u"), clen);
 }

 if(clen > 8 * 1024 * 1024) // Compressed length sanity check - 8 MiB max.
 {
  throw MDFN_Error(0, _("Compressed save state data is too large: %u"), clen);
 }

 cbuf.resize(clen);

 MDFND_RecvData(&cbuf[0], clen);

 uLongf len = MDFN_de32lsb(&cbuf[0]);
 if(len > 12 * 1024 * 1024) // Uncompressed length sanity check - 12 MiB max.
 {
  throw MDFN_Error(0, _("Uncompressed save state data is too large: %u"), len);
 }

 buf.resize(len);

 uncompress((Bytef *)&buf[0], &len, (Bytef *)&cbuf[0] + 4, clen - 4);

 sm.data = &buf[0];
 sm.len = len;

 if(!MDFNSS_LoadSM(&sm, 0, 0))
 {
  throw MDFN_Error(0, _("Error during save state loading."));
 }

 if(MDFNMOV_IsRecording())
  MDFNMOV_RecordState();
}

static std::string GenerateMPSString(uint32 mps, bool ctlr_string = false)
{
 char tmpbuf[256];

 tmpbuf[0] = 0;

 if(!mps)
 {
  if(!ctlr_string)
   trio_snprintf(tmpbuf, sizeof(tmpbuf), _("a lurker"));
 }
 else
  trio_snprintf(tmpbuf, sizeof(tmpbuf), ("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"), ctlr_string ? ((mps == round_up_pow2(mps)) ? _("controller") : _("controllers")) : ((mps == round_up_pow2(mps)) ? _("player") : _("players")),
				       (mps & 0x0001) ? " 1" : "",
				       (mps & 0x0002) ? " 2" : "",
				       (mps & 0x0004) ? " 3" : "",
				       (mps & 0x0008) ? " 4" : "",
                                       (mps & 0x0010) ? " 5" : "",
                                       (mps & 0x0020) ? " 6" : "",
                                       (mps & 0x0040) ? " 7" : "",
                                       (mps & 0x0080) ? " 8" : "",
                                       (mps & 0x0100) ? " 9" : "",
                                       (mps & 0x0200) ? " 10" : "",
                                       (mps & 0x0400) ? " 11" : "",
                                       (mps & 0x0800) ? " 12" : "",
                                       (mps & 0x1000) ? " 13" : "",
                                       (mps & 0x2000) ? " 14" : "",
                                       (mps & 0x4000) ? " 15" : "",
                                       (mps & 0x8000) ? " 16" : "");


 return(std::string(tmpbuf));
}

void NetplaySendState(void)
{
 try
 {
  SendState();
 }
 catch(std::exception &e)
 {
  NetError("%s", e.what());
 }
}

void NetplayUpdate(const char **PortDNames, void *PortData[], uint32 PortLen[], int NumPorts)
{
 uint8 buf[TotalInputStateSize + 1];

//
//
try
{
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
  MDFND_SendData(outgoing_buffer, 1 + LocalInputStateSize);
 }

 do
 {
  MDFND_RecvData(buf, TotalInputStateSize + 1);

  switch(buf[TotalInputStateSize])
  {
   case 0: break; // No command

   default: MDFN_DoSimpleCommand(buf[TotalInputStateSize]);break;

   case MDFNNPCMD_INTEGRITY:
			SendIntegrity();
			break;

   case MDFNNPCMD_REQUEST_STATE:
			SendState();
	  	 	break;

   case MDFNNPCMD_LOADSTATE:
			RecvState(MDFN_de32lsb(buf));
			MDFN_DispMessage(_("Remote state loaded."));
			break;

   case MDFNNPCMD_SERVERTEXT:
			{
			 static const uint32 MaxLength = 2000;
                         uint8 neobuf[MaxLength + 1];
                         char *textbuf = NULL;
                         const uint32 totallen = MDFN_de32lsb(buf);

                         if(totallen > MaxLength) // Sanity check
                         {
                          throw MDFN_Error(0, _("Text length is too long: %u"), totallen);
                         }

                         MDFND_RecvData(neobuf, totallen);

			 neobuf[totallen] = 0;
			 trio_asprintf(&textbuf, "** %s", neobuf);
                         MDFND_NetplayText((UTF8*)textbuf, FALSE);
                         free(textbuf);
			}
			break;

   case MDFNNPCMD_ECHO:
			{
                         uint32 totallen = MDFN_de32lsb(buf);
			 uint64 then_time;
			 uint64 now_time;

			 if(totallen != sizeof(then_time))
			 {
                          throw MDFN_Error(0, _("Echo response length is incorrect size: %u"), totallen);
			 }

                         MDFND_RecvData(&then_time, sizeof(then_time));

			 now_time = MDFND_GetTime();

                         char *textbuf = NULL;
			 trio_asprintf(&textbuf, _("*** Round-trip time: %llu ms"), now_time - then_time);
                         MDFND_NetplayText((UTF8*)textbuf, FALSE);
                         free(textbuf);
			}
			break;

   case MDFNNPCMD_TEXT:
			{
			 static const uint32 MaxLength = 2000;
			 uint8 neobuf[MaxLength + 1];
			 const uint32 totallen = MDFN_de32lsb(buf);
                         uint32 nicklen;
                         bool NetEcho = false;
                         char *textbuf = NULL;

			 if(totallen < 4)
			 {
			  throw MDFN_Error(0, _("Text command length is too short: %u"), totallen);
	  		 }

			 if(totallen > MaxLength) // Sanity check
			 {
                          throw MDFN_Error(0, _("Text command length is too long: %u"), totallen);
			 }

			 MDFND_RecvData(neobuf, totallen);

			 nicklen = MDFN_de32lsb(neobuf);
			 if(nicklen > (totallen - 4)) // Sanity check
			 {
			  throw MDFN_Error(0, _("Received nickname length is too long: %u"), nicklen);
			 }

                         neobuf[totallen] = 0;

			 if(nicklen)
			 {
			  uint8 nickbuf[nicklen + 1];
			  memcpy(nickbuf, neobuf + 4, nicklen);
			  nickbuf[nicklen] = 0;
			  if(OurNick && !strcasecmp(OurNick, (char *)nickbuf))
			  {
                           trio_asprintf(&textbuf, "> %s", &neobuf[4 + nicklen]);
			   NetEcho = true;
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
			 static const uint32 MaxLength = 2000;
                         uint8 neobuf[MaxLength + 1];
                         uint8 *newnick;
                         char *textbuf = NULL;
			 const uint32 len = MDFN_de32lsb(buf);

                         if(len > MaxLength) // Sanity check
                         {
                          throw MDFN_Error(0, _("Nickname change length is too long: %u"), len);
                         }

                         MDFND_RecvData(neobuf, len);

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

    case MDFNNPCMD_CTRL_CHANGE:
			{
			 const uint32 len = MDFN_de32lsb(buf);

			 //
                         // Joined = true;
                         SendCommand(MDFNNPCMD_CTRL_CHANGE_ACK, len);
			 //
			 //
                         LocalInputStateSize = 0;
                         LocalPlayersMask = len;

                         for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
                         {
                          if(LocalPlayersMask & (1 << x))
                           LocalInputStateSize += PortLen[x];
                         }
			}
			break;

   case MDFNNPCMD_CTRLR_SWAP_NOTIF:
			{
			 const uint32 cm = MDFN_de32lsb(buf);
			 char textbuf[512];

			 trio_snprintf(textbuf, sizeof(textbuf), _("* All instances of controllers %u and %u have been swapped."), ((cm & 0xFF) + 1), ((cm >> 8) & 0xFF) + 1);
			 MDFND_NetplayText((UTF8*)textbuf, false);
			}
			break;

   case MDFNNPCMD_CTRLR_TAKE_NOTIF:
   case MDFNNPCMD_CTRLR_DROP_NOTIF:
   case MDFNNPCMD_CTRLR_DUPE_NOTIF:
			{
			 static const uint32 MaxNicknameLength = 1000;
			 static const uint32 MaxLength = 12 + MaxNicknameLength;
			 const char *fstr = NULL;
			 const uint32 len = MDFN_de32lsb(buf);
			 uint8 ntf_buf[MaxLength + 1];
			 char *textbuf = NULL;

			 if(len < 12)
			  throw MDFN_Error(0, _("Take/drop/dupe notification is too short: %u"), len);

			 if(len > MaxLength)
			  throw MDFN_Error(0, _("Take/drop/dupe notification is too long: %u"), len);

			 MDFND_RecvData(ntf_buf, len);
			 ntf_buf[len] = 0;

 	 	 	 switch(buf[TotalInputStateSize])
			 {
			  case MDFNNPCMD_CTRLR_TAKE_NOTIF:
			  	fstr = _("* <%s> took all instances of %s, and is now %s.");
				break;

			  case MDFNNPCMD_CTRLR_DUPE_NOTIF:
			 	fstr = _("* <%s> took copies of %s, and is now %s.");
				break;

			  case MDFNNPCMD_CTRLR_DROP_NOTIF:
				fstr = _("* <%s> dropped %s, and is now %s.");
				break;
			 }
                         trio_asprintf(&textbuf, fstr, ntf_buf + 12, GenerateMPSString(MDFN_de32lsb(&ntf_buf[0]), true).c_str(), GenerateMPSString(MDFN_de32lsb(&ntf_buf[4]), false).c_str());
	                 MDFND_NetplayText((UTF8*)textbuf, false);
			 free(textbuf);
			}
			break;

   case MDFNNPCMD_YOUJOINED:
   case MDFNNPCMD_YOULEFT:
   case MDFNNPCMD_PLAYERLEFT:
   case MDFNNPCMD_PLAYERJOINED:
			{
			 static const uint32 MaxLength = 2000;
                         uint8 neobuf[MaxLength + 1];
                         char *textbuf = NULL;
			 uint32 mps;
                         std::string mps_string;
	                 const uint32 len = MDFN_de32lsb(buf);

			 if(len < 8)
			 {
                          throw MDFN_Error(0, _("Join/Left length is too short: %u"), len);
		         }

                         if(len > MaxLength) // Sanity check
                         {
                          throw MDFN_Error(0, _("Join/Left length is too long: %u"), len);
                         }

                         MDFND_RecvData(neobuf, len);
			 neobuf[len] = 0; // NULL-terminate the string

			 mps = MDFN_de32lsb(&neobuf[0]);

			 mps_string = GenerateMPSString(mps);

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
			  OurNick = strdup((char*)neobuf + 8);

                          trio_asprintf(&textbuf, _("* You, %s, have connected as: %s"), neobuf + 8, mps_string.c_str());

			  LocalInputStateSize = 0;
			  LocalPlayersMask = mps;

			  for(int x = 0; x < MDFNGameInfo->InputInfo->InputPorts; x++)
			  {
			   if(LocalPlayersMask & (1 << x))
			    LocalInputStateSize += PortLen[x];
			  }
			  Joined = TRUE;
			  SendCommand(MDFNNPCMD_SETFPS, MDFNGameInfo->fps);
			 }
			 else if(buf[TotalInputStateSize] == MDFNNPCMD_PLAYERLEFT)
			 {
                                  trio_asprintf(&textbuf, _("* %s(%s) has left"), neobuf + 8, mps_string.c_str());
			 }
			 else
			 {
                                  trio_asprintf(&textbuf, _("* %s has connected as: %s"), neobuf + 8, mps_string.c_str());
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
catch(std::exception &e)
{
 NetError("%s", e.what());
}
//
//

}


//
//
//
//

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
static bool CC_swap(const UTF8 *arg);
static bool CC_dupe(const UTF8 *arg);
static bool CC_drop(const UTF8 *arg);
static bool CC_take(const UTF8 *arg);
static bool CC_list(const UTF8 *arg);

static CommandEntry ConsoleCommands[]   =
{
 { "/server", CC_server,	gettext_noop("[REMOTE_HOST] [PORT]"), "Connects to REMOTE_HOST(IP address or FQDN), on PORT." },

 { "/connect", CC_server,	NULL, NULL },

 //{ "/gamekey", CC_gamekey,	gettext_noop("GAMEKEY"), gettext_noop("Changes the game key to the specified GAMEKEY.") },

 { "/quit", CC_quit,		gettext_noop("[MESSAGE]"), gettext_noop("Disconnects from the netplay server.") },

 { "/help", CC_help,		"", gettext_noop("Help, I'm drowning in a sea of cliche metaphors!") },

 { "/nick", CC_nick,		gettext_noop("NICKNAME"), gettext_noop("Changes your nickname to the specified NICKNAME.") },

 { "/swap", CC_swap,		gettext_noop("A B"), gettext_noop("Swap/Exchange all instances of controllers A and B(numbered from 1).") },

 { "/dupe", CC_dupe,            gettext_noop("[A] [...]"), gettext_noop("Duplicate and take instances of specified controller(s).") },
 { "/drop", CC_drop,            gettext_noop("[A] [...]"), gettext_noop("Drop all instances of specified controller(s).") },
 { "/take", CC_take,            gettext_noop("[A] [...]"), gettext_noop("Take all instances of specified controller(s).") },

 //{ "/list", CC_list,		"", "List players in game." },

 { "/ping", CC_ping,		"", "Pings the server." },

 //{ "/integrity", CC_integrity,	"", "Starts netplay integrity check sequence." },

 { NULL, NULL },
};


static bool CC_server(const UTF8 *arg)
{
 char server[300];
 unsigned int port = 0;

 server[0] = 0;

 switch(trio_sscanf((char*)arg, "%.299s %u", server, port))
 {
  default:
  case 0:
	break;

  case 1:
	MDFNI_SetSetting("netplay.host", (char*)server);
	break;

  case 2:
	MDFNI_SetSetting("netplay.host", (char*)server);
	MDFNI_SetSettingUI("netplay.port", port);
	break;
 }

 MDFND_NetworkConnect();

 return(false);
}

static bool CC_gamekey(const UTF8 *arg)
{
// SendCEvent(CEVT_NP_SETGAMEKEY, strdup(arg), NULL);
 return(true);
}

static bool CC_quit(const UTF8 *arg)
{
 if(MDFNnetplay)
 {
  MDFNI_NetplayQuit((const char *)arg);
  MDFND_NetworkClose();
 }
 else
 {
  NetPrintText(_("*** Not connected!"));
  return(true);
 }

 return(false);
}

static bool CC_list(const UTF8 *arg)
{
 if(MDFNnetplay)
  MDFNI_NetplayList();
 else
 {
  NetPrintText(_("*** Not connected!"));
  return(true);
 }

 return(true);
}

static bool CC_swap(const UTF8 *arg)
{
 int a = 0, b = 0;

 if(sscanf((const char *)arg, "%u %u", &a, &b) == 2 && a && b)
 {
  uint32 sc = ((a - 1) & 0xFF) | (((b - 1) & 0xFF) << 8);

  if(MDFNnetplay)
   MDFNI_NetplaySwap((sc >> 0) & 0xFF, (sc >> 8) & 0xFF);
  else
  {
   NetPrintText(_("*** Not connected!"));
   return(true);
  }
 }
 else
 {
  NetPrintText(_("*** %s command requires at least %u non-zero integer argument(s)."), "SWAP", 2);
  return(true);
 }

 return(false);
}

static bool CC_dupe(const UTF8 *arg)
{
 int tmp[32];
 int count;


 memset(tmp, 0, sizeof(tmp));
 count = sscanf((const char *)arg, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
			&tmp[0x00], &tmp[0x01], &tmp[0x02], &tmp[0x03], &tmp[0x04], &tmp[0x05], &tmp[0x06], &tmp[0x07],
			&tmp[0x08], &tmp[0x09], &tmp[0x0A], &tmp[0x0B], &tmp[0x0C], &tmp[0x0D], &tmp[0x0E], &tmp[0x0F],
                        &tmp[0x00], &tmp[0x01], &tmp[0x02], &tmp[0x03], &tmp[0x04], &tmp[0x05], &tmp[0x06], &tmp[0x07],
                        &tmp[0x08], &tmp[0x09], &tmp[0x0A], &tmp[0x0B], &tmp[0x0C], &tmp[0x0D], &tmp[0x0E], &tmp[0x0F]);

 if(count > 0)
 {
  uint32 mask = 0;

  for(int i = 0; i < 32; i++)
  {
   if(tmp[i] > 0)
    mask |= 1U << (unsigned)(tmp[i] - 1);
  }

  if(MDFNnetplay)
   MDFNI_NetplayDupe(mask);
  else
  {
   NetPrintText(_("*** Not connected!"));
   return(true);
  }
 }
 else
 {
  NetPrintText(_("*** %s command requires at least %u non-zero integer argument(s)."), "DUPE", 1);
  return(true);
 }

 return(false);
}

static bool CC_drop(const UTF8 *arg)
{
 int tmp[32];
 int count;


 memset(tmp, 0, sizeof(tmp));
 count = sscanf((const char *)arg, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
                        &tmp[0x00], &tmp[0x01], &tmp[0x02], &tmp[0x03], &tmp[0x04], &tmp[0x05], &tmp[0x06], &tmp[0x07],
                        &tmp[0x08], &tmp[0x09], &tmp[0x0A], &tmp[0x0B], &tmp[0x0C], &tmp[0x0D], &tmp[0x0E], &tmp[0x0F],
                        &tmp[0x00], &tmp[0x01], &tmp[0x02], &tmp[0x03], &tmp[0x04], &tmp[0x05], &tmp[0x06], &tmp[0x07],
                        &tmp[0x08], &tmp[0x09], &tmp[0x0A], &tmp[0x0B], &tmp[0x0C], &tmp[0x0D], &tmp[0x0E], &tmp[0x0F]);

 if(count > 0)
 {
  uint32 mask = 0;

  for(int i = 0; i < 32; i++)
  {
   if(tmp[i] > 0)
    mask |= 1U << (unsigned)(tmp[i] - 1);
  }

  if(MDFNnetplay)
   MDFNI_NetplayDrop(mask);
  else
  {
   NetPrintText(_("*** Not connected!"));
   return(true);
  }
 }
 else
 {
  NetPrintText(_("*** %s command requires at least %u non-zero integer argument(s)."), "DROP", 1);
  return(true);
 }

 return(false);
}

static bool CC_take(const UTF8 *arg)
{
 int tmp[32];
 int count;


 memset(tmp, 0, sizeof(tmp));
 count = sscanf((const char *)arg, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
                        &tmp[0x00], &tmp[0x01], &tmp[0x02], &tmp[0x03], &tmp[0x04], &tmp[0x05], &tmp[0x06], &tmp[0x07],
                        &tmp[0x08], &tmp[0x09], &tmp[0x0A], &tmp[0x0B], &tmp[0x0C], &tmp[0x0D], &tmp[0x0E], &tmp[0x0F],
                        &tmp[0x00], &tmp[0x01], &tmp[0x02], &tmp[0x03], &tmp[0x04], &tmp[0x05], &tmp[0x06], &tmp[0x07],
                        &tmp[0x08], &tmp[0x09], &tmp[0x0A], &tmp[0x0B], &tmp[0x0C], &tmp[0x0D], &tmp[0x0E], &tmp[0x0F]);

 if(count > 0)
 {
  uint32 mask = 0;

  for(int i = 0; i < 32; i++)
  {
   if(tmp[i] > 0)
    mask |= 1U << (unsigned)(tmp[i] - 1);
  }

  if(MDFNnetplay)
   MDFNI_NetplayTake(mask);
  else
  {
   NetPrintText(_("*** Not connected!"));
   return(true);
  }
 }
 else
 {
  NetPrintText(_("*** %s command requires at least %u non-zero integer argument(s)."), "TAKE", 1);
  return(true);
 }

 return(false);
}

static bool CC_ping(const UTF8 *arg)
{
 if(MDFNnetplay)
  MDFNI_NetplayPing();
 else
 {
  NetPrintText(_("*** Not connected!"));
  return(true);
 }

 return(false);
}

static bool CC_integrity(const UTF8 *arg)
{
 if(MDFNnetplay)
  MDFNI_NetplayIntegrity();
 else
 {
  NetPrintText(_("*** Not connected!"));
  return(true);
 }

 return(FALSE);
}

static bool CC_help(const UTF8 *arg)
{
 for(unsigned int x = 0; ConsoleCommands[x].name; x++)
 {
  if(ConsoleCommands[x].help_desc)
  {
   char help_buf[512];
   trio_snprintf(help_buf, 512, "%s %s  -  %s", ConsoleCommands[x].name, _(ConsoleCommands[x].help_args), _(ConsoleCommands[x].help_desc));
   MDFND_NetplayText((UTF8*)help_buf, false);
  }
 }
 return(true);
}

static bool CC_nick(const UTF8 *arg)
{
 MDFNI_SetSetting("netplay.nick", (char*)arg);

 if(MDFNnetplay)
  MDFNI_NetplayChangeNick((UTF8*)arg);

 return(true);
}

void MDFNI_NetplayLine(const char *text, bool &inputable, bool &viewable)
{
	 inputable = viewable = false;

         for(unsigned int x = 0; ConsoleCommands[x].name; x++)
	 {
          if(!strncasecmp(ConsoleCommands[x].name, (char*)text, strlen(ConsoleCommands[x].name)) && text[strlen(ConsoleCommands[x].name)] <= 0x20)
          {
	   char *trim_text = strdup((char*)&text[strlen(ConsoleCommands[x].name)]);

	   MDFN_trim(trim_text);

           inputable = viewable = ConsoleCommands[x].func((UTF8*)trim_text);

           free(trim_text);
           return;
          }
	 }

         if(text[0] != 0)	// Is non-empty line?
	 {
	  MDFNI_NetplayText((UTF8*)text);
	  viewable = true;
         }
}
