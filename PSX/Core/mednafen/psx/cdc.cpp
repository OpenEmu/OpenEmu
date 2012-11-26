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

// TODO: async command counter and async command phase?
/*

 TODO:
	Implement missing commands.

	SPU CD-DA and CD-XA streaming semantics.
*/

/*
 After eject(doesn't appear to occur when drive is in STOP state):
	* Does not appear to occur in STOP state.
	* Does not appear to occur in PAUSE state.
	* DOES appear to occur in STANDBY state. (TODO: retest)

% Result 0: 16
% Result 1: 08
% IRQ Result: e5
% 19 e0

 Command abortion tests(NOP tested):
	Does not appear to occur when in STOP or PAUSE states(STOP or PAUSE command just executed).

	DOES occur after a ReadTOC completes, if ReadTOC is not followed by a STOP or PAUSE.  Odd.
*/

#include "psx.h"
#include "cdc.h"
#include "spu.h"

using namespace CDUtility;

namespace MDFN_IEN_PSX
{

PS_CDC::PS_CDC() : DMABuffer(4096)
{
 IsPSXDisc = false;
 Cur_CDIF = NULL;
}

PS_CDC::~PS_CDC()
{

}

void PS_CDC::SetDisc(bool tray_open, CDIF *cdif, const char *disc_id)
{
 if(tray_open)
  cdif = NULL;

 Cur_CDIF = cdif;
 IsPSXDisc = false;
 memset(DiscID, 0, sizeof(DiscID));

 if(!Cur_CDIF)
 {
  PSRCounter = 0;

  if((DriveStatus != DS_PAUSED && DriveStatus != DS_STOPPED) || PendingCommandPhase > 0)
  {
   PendingCommand = 0x00;
   PendingCommandCounter = 0;
   PendingCommandPhase = 0;
  }

  HeaderBufValid = false;
  DriveStatus = DS_STOPPED;
  ClearAIP();
 }
 else
 {
  HeaderBufValid = false;
  DiscStartupDelay = (int64)1000 * 33868800 / 1000;
  DiscChanged = true;

  Cur_CDIF->ReadTOC(&toc);

  if(disc_id)
  {
   strncpy((char *)DiscID, disc_id, 4);
   IsPSXDisc = true;
  }
 }
}

int32 PS_CDC::CalcNextEvent(void)
{
 int32 next_event = SPUCounter;

 if(PSRCounter > 0 && next_event > PSRCounter)
  next_event = PSRCounter;

 if(PendingCommandCounter > 0 && next_event > PendingCommandCounter)
  next_event = PendingCommandCounter;

 if(!IRQBuffer)
 {
  if(CDCReadyReceiveCounter > 0 && next_event > CDCReadyReceiveCounter)
   next_event = CDCReadyReceiveCounter;
 }

 if(DiscStartupDelay > 0 && next_event > DiscStartupDelay)
  next_event = DiscStartupDelay;

 //fprintf(stderr, "%d %d %d %d --- %d\n", PSRCounter, PendingCommandCounter, CDCReadyReceiveCounter, DiscStartupDelay, next_event);

 return(next_event);
}

void PS_CDC::SoftReset(void)
{
 ClearAudioBuffers();

 // Not sure about initial volume state
 Pending_DecodeVolume[0][0] = 0x80;
 Pending_DecodeVolume[0][1] = 0x00;
 Pending_DecodeVolume[1][0] = 0x00;
 Pending_DecodeVolume[1][1] = 0x80;
 memcpy(DecodeVolume, Pending_DecodeVolume, sizeof(DecodeVolume));

 RegSelector = 0;
 memset(ArgsBuf, 0, sizeof(ArgsBuf));
 ArgsIn = 0;

 memset(ResultsBuffer, 0, sizeof(ResultsBuffer));
 ResultsWP = 0;
 ResultsRP = 0;
 ResultsIn = 0;

 CDCReadyReceiveCounter = 0;

 IRQBuffer = 0;
 IRQOutTestMask = 0;
 RecalcIRQ();

 DMABuffer.Flush();
 SB_In = 0;

 memset(SubQBuf, 0, sizeof(SubQBuf));
 memset(SubQBuf_Safe, 0, sizeof(SubQBuf_Safe));
 SubQChecksumOK = false;

 memset(HeaderBuf, 0, sizeof(HeaderBuf));


 FilterFile = 0;
 FilterChan = 0;

 PendingCommand = 0;
 PendingCommandPhase = 0;
 PendingCommandCounter = 0;

 Mode = 0;

 HeaderBufValid = false;
 DriveStatus = DS_STOPPED;
 ClearAIP();
 StatusAfterSeek = DS_STOPPED;

 Forward = false;
 Backward = false;
 Muted = false;

 PlayTrackMatch = 0;

 PSRCounter = 0;

 CurSector = 0;

 ClearAIP();

 SeekTarget = 0;

 CommandLoc = 0;
 CommandLoc_Dirty = true;

 DiscChanged = true;
}

void PS_CDC::Power(void)
{
 SPU->Power();

 SoftReset();

 DiscStartupDelay = 0;

 SPUCounter = SPU->UpdateFromCDC(0);
 lastts = 0;
}

int PS_CDC::StateAction(StateMem *sm, int load, int data_only)
{
 assert(AudioBuffer_Count == 4);

 SFORMAT StateRegs[] =
 {
 SFVAR(DiscChanged),
 SFVAR(DiscStartupDelay),

#define SFAB(n)	SFARRAY16N(&AudioBuffer[n].Samples[0][0], sizeof(AudioBuffer[n].Samples) / sizeof(AudioBuffer[n].Samples[0][0]), #n "Samples"),	\
		SFVARN(AudioBuffer[n].Size, #n "Size"),											\
		SFVARN(AudioBuffer[n].Freq, #n "Freq")

 SFAB(0),
 SFAB(1),
 SFAB(2),
 SFAB(3),

 SFVAR(AudioBuffer_ReadPos),
 SFVAR(AudioBuffer_WritePos),
 SFVAR(AudioBuffer_UsedCount),
 SFVAR(AudioBuffer_InPrebuffer),

 SFARRAY(&Pending_DecodeVolume[0][0], 2 * 2),
 SFARRAY(&DecodeVolume[0][0], 2 * 2),

 SFVAR(RegSelector),
 SFARRAY(ArgsBuf, 16),
 SFVAR(ArgsIn),

 SFARRAY(ResultsBuffer, 16),
 SFVAR(ResultsIn),
 SFVAR(ResultsWP),
 SFVAR(ResultsRP),

//
//
//
 SFARRAY(&DMABuffer.data[0], DMABuffer.data.size()),
 SFVAR(DMABuffer.read_pos),
 SFVAR(DMABuffer.write_pos),
 SFVAR(DMABuffer.in_count),
//
//
//

 SFARRAY(SB, sizeof(SB) / sizeof(SB[0])),
 SFVAR(SB_In),
 SFARRAY(SubQBuf, sizeof(SubQBuf) / sizeof(SubQBuf[0])),
 SFARRAY(SubQBuf_Safe, sizeof(SubQBuf_Safe) / sizeof(SubQBuf_Safe[0])),

 SFVAR(SubQChecksumOK),
 SFARRAY(HeaderBuf, sizeof(HeaderBuf) / sizeof(HeaderBuf[0])),

 SFVAR(IRQBuffer),
 SFVAR(IRQOutTestMask),
 SFVAR(CDCReadyReceiveCounter),



 SFVAR(FilterFile),
 SFVAR(FilterChan),


 SFVAR(PendingCommand),
 SFVAR(PendingCommandPhase),
 SFVAR(PendingCommandCounter),

 SFVAR(SPUCounter),

 SFVAR(Mode),
 SFVAR(DriveStatus),
 SFVAR(StatusAfterSeek),
 SFVAR(Forward),
 SFVAR(Backward),
 SFVAR(Muted),

 SFVAR(PlayTrackMatch),

 SFVAR(PSRCounter),

 SFVAR(CurSector),

 SFVAR(AsyncIRQPending),
 SFARRAY(AsyncResultsPending, sizeof(AsyncResultsPending) / sizeof(AsyncResultsPending[0])),
 SFVAR(AsyncResultsPendingCount),


 SFVAR(SeekTarget),

 SFVAR(lastts),

#if 0
 CDUtility::TOC toc;
 bool IsPSXDisc;
 uint8 DiscID[4];
#endif
  SFVAR(CommandLoc),
  SFVAR(CommandLoc_Dirty),
  SFARRAY16(&xa_previous[0][0], sizeof(xa_previous) / sizeof(xa_previous[0][0])),

  SFVAR(xa_cur_set),
  SFVAR(xa_cur_file),
  SFVAR(xa_cur_chan),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "CDC");

 if(load)
 {

 }
 return(ret);
}

void PS_CDC::ResetTS(void)
{
 lastts = 0;
}

void PS_CDC::RecalcIRQ(void)
{
 IRQ_Assert(IRQ_CD, (bool)(IRQBuffer & (IRQOutTestMask & 0x1F)));
}

void PS_CDC::WriteIRQ(uint8 V)
{
 assert(CDCReadyReceiveCounter <= 0);
 assert(!IRQBuffer);

 CDCReadyReceiveCounter = 1024;

 IRQBuffer = V;
 RecalcIRQ();
}

void PS_CDC::BeginResults(void)
{
 //if(ResultsIn)
 // {
 // printf("Cleared %d results. IRQBuffer=0x%02x\n", ResultsIn, IRQBuffer);
 //}

 // TODO: test semantics on real thing.
 ResultsIn = 0;
 ResultsWP = 0;
 ResultsRP = 0;
}

void PS_CDC::WriteResult(uint8 V)
{
 ResultsBuffer[ResultsWP] = V;
 ResultsWP = (ResultsWP + 1) & 0xF;
 ResultsIn = (ResultsIn + 1) & 0x1F;

 if(!ResultsIn)
  PSX_WARNING("[CDC] Results buffer overflow!");
}

uint8 PS_CDC::ReadResult(void)
{
 uint8 ret = ResultsBuffer[ResultsRP];

 ResultsRP = (ResultsRP + 1) & 0xF;
 ResultsIn = (ResultsIn - 1) & 0x1F;

 return ret;
}

uint8 PS_CDC::MakeStatus(bool cmd_error)
{
 uint8 ret = 0;

 // Are these bit positions right?

 if(DriveStatus == DS_PLAYING)
  ret |= 0x80;

 if(DriveStatus == DS_SEEKING || DriveStatus == DS_SEEKING_LOGICAL)
  ret |= 0x40;

 if(DriveStatus == DS_READING)
  ret |= 0x20;

 // TODO: shell open and seek error
 if(!Cur_CDIF || DiscChanged)
  ret |= 0x10;

 if(DriveStatus != DS_STOPPED)
  ret |= 0x02;

 if(cmd_error)
  ret |= 0x01;

 DiscChanged = false;

 return(ret);
}

bool PS_CDC::DecodeSubQ(uint8 *subpw)
{
 uint8 tmp_q[0xC];

 memset(tmp_q, 0, 0xC);

 for(int i = 0; i < 96; i++)
  tmp_q[i >> 3] |= ((subpw[i] & 0x40) >> 6) << (7 - (i & 7));

 if((tmp_q[0] & 0xF) == 1)
 {
  memcpy(SubQBuf, tmp_q, 0xC);
  SubQChecksumOK = subq_check_checksum(tmp_q);

  if(SubQChecksumOK)
  {
   memcpy(SubQBuf_Safe, tmp_q, 0xC);
   return(true);
  }
 }

 return(false);
}

struct XA_Subheader
{
 uint8 file;
 uint8 channel;
 uint8 submode;
 uint8 coding;

 uint8 file_dup;
 uint8 channel_dup;
 uint8 submode_dup;
 uint8 coding_dup;
} __attribute__((__packed__));

struct XA_SoundGroup
{
 uint8 params[16];
 uint8 samples[112];
} __attribute__((__packed__));

#define XA_SUBMODE_EOF		0x80
#define XA_SUBMODE_REALTIME	0x40
#define XA_SUBMODE_FORM		0x20
#define XA_SUBMODE_TRIGGER	0x10
#define XA_SUBMODE_DATA		0x08
#define XA_SUBMODE_AUDIO	0x04
#define XA_SUBMODE_VIDEO	0x02
#define XA_SUBMODE_EOR		0x01

#define XA_CODING_EMPHASIS	0x40

//#define XA_CODING_BPS_MASK	0x30
//#define XA_CODING_BPS_4BIT	0x00
//#define XA_CODING_BPS_8BIT	0x10
//#define XA_CODING_SR_MASK	0x0C
//#define XA_CODING_SR_378	0x00
//#define XA_CODING_SR_

#define XA_CODING_8BIT		0x10
#define XA_CODING_189		0x04
#define XA_CODING_STEREO	0x01

// Special regression prevention test cases:
//	Um Jammer Lammy (start doing poorly)
//	Yarudora Series Vol.1 - Double Cast (non-FMV speech)

bool PS_CDC::XA_Test(const uint8 *sdata)
{
 const XA_Subheader *sh = (const XA_Subheader *)&sdata[12 + 4];

 if(!(Mode & MODE_STRSND))
  return false;

 if(!(sh->submode & XA_SUBMODE_AUDIO))
  return false;

 //printf("Test File: 0x%02x 0x%02x - Channel: 0x%02x 0x%02x - Submode: 0x%02x 0x%02x - Coding: 0x%02x 0x%02x - \n", sh->file, sh->file_dup, sh->channel, sh->channel_dup, sh->submode, sh->submode_dup, sh->coding, sh->coding_dup);

 if((Mode & MODE_SF) && (sh->file != FilterFile || sh->channel != FilterChan))
  return false;

 if(!xa_cur_set || (Mode & MODE_SF))
 {
  xa_cur_set = true;
  xa_cur_file = sh->file;
  xa_cur_chan = sh->channel;
 }
 else if(sh->file != xa_cur_file || sh->channel != xa_cur_chan)
  return false;

 if(sh->submode & XA_SUBMODE_EOF)
 {
  //puts("YAY");
  xa_cur_set = false;
  xa_cur_file = 0;
  xa_cur_chan = 0;
 }

 return true;
}

void PS_CDC::ClearAudioBuffers(void)
{
 memset(AudioBuffer, 0, sizeof(AudioBuffer));
 memset(xa_previous, 0, sizeof(xa_previous));

 xa_cur_set = false;
 xa_cur_file = 0;
 xa_cur_chan = 0;

 AudioBuffer_ReadPos = 0;
 AudioBuffer_WritePos = 0;
 AudioBuffer_UsedCount = 0;

 AudioBuffer_InPrebuffer = true;
}

void PS_CDC::XA_ProcessSector(const uint8 *sdata, CD_Audio_Buffer *ab)
{
 const XA_Subheader *sh = (const XA_Subheader *)&sdata[12 + 4];
 const unsigned unit_index_shift = (sh->coding & XA_CODING_8BIT) ? 0 : 1;

 //printf("File: 0x%02x 0x%02x - Channel: 0x%02x 0x%02x - Submode: 0x%02x 0x%02x - Coding: 0x%02x 0x%02x - \n", sh->file, sh->file_dup, sh->channel, sh->channel_dup, sh->submode, sh->submode_dup, sh->coding, sh->coding_dup);

 ab->Size = 18 * (4 << unit_index_shift) * 28;

 if(sh->coding & XA_CODING_STEREO)
  ab->Size >>= 1;

 ab->Freq = (sh->coding & XA_CODING_189) ? 3 : 6;

 //fprintf(stderr, "Coding: %02x %02x\n", sh->coding, sh->coding_dup);

 for(unsigned group = 0; group < 18; group++)
 {
  const XA_SoundGroup *sg = (const XA_SoundGroup *)&sdata[12 + 4 + 8 + group * 128];

  for(unsigned unit = 0; unit < (4U << unit_index_shift); unit++)
  {
   const uint8 param = sg->params[(unit & 3) | ((unit & 4) << 1)];
   const uint8 param_copy = sg->params[4 | (unit & 3) | ((unit & 4) << 1)];
   uint8 ibuffer[28];
   int16 obuffer[2 + 28];

   if(param != param_copy)
    printf("%d %02x %02x\n", unit, param, param_copy);

   for(unsigned i = 0; i < 28; i++)
   {
    uint8 tmp = sg->samples[i * 4 + (unit >> unit_index_shift)];

    if(unit_index_shift)
    {
     tmp <<= (unit & 1) ? 0 : 4;
     tmp &= 0xf0;
    }

    ibuffer[i] = tmp;
   }

   const bool ocn = (bool)(unit & 1) && (sh->coding & XA_CODING_STEREO);

   obuffer[0] = xa_previous[ocn][0];
   obuffer[1] = xa_previous[ocn][1];

   PS_SPU::DecodeADPCM(ibuffer, &obuffer[2], param & 0x0F, param >> 4);

   xa_previous[ocn][0] = obuffer[28];
   xa_previous[ocn][1] = obuffer[29];

   if(param != param_copy)
    memset(obuffer, 0, sizeof(obuffer));

   if(sh->coding & XA_CODING_STEREO)
   {
    for(unsigned s = 0; s < 28; s++)
    {
     ab->Samples[ocn][group * (2 << unit_index_shift) * 28 + (unit >> 1) * 28 + s] = obuffer[2 + s];
    }
   }
   else
   {
    for(unsigned s = 0; s < 28; s++)
    {
     ab->Samples[0][group * (4 << unit_index_shift) * 28 + unit * 28 + s] = obuffer[2 + s];
     ab->Samples[1][group * (4 << unit_index_shift) * 28 + unit * 28 + s] = obuffer[2 + s];
    }
   }
  }
 }
}

void PS_CDC::ClearAIP(void)
{
 AsyncResultsPendingCount = 0;
 AsyncIRQPending = 0;
}

void PS_CDC::CheckAIP(void)
{
 if(AsyncIRQPending && CDCReadyReceiveCounter <= 0)
 {
  BeginResults();

  for(unsigned i = 0; i < AsyncResultsPendingCount; i++)
   WriteResult(AsyncResultsPending[i]);

  WriteIRQ(AsyncIRQPending);

  ClearAIP();
 }
}

void PS_CDC::SetAIP(unsigned irq, unsigned result_count, uint8 *r)
{
 if(AsyncIRQPending)
 {
  PSX_WARNING("***WARNING*** Previous notification skipped: CurSector=%d, old_notification=0x%02x", CurSector, AsyncIRQPending);
 }
 ClearAIP();

 AsyncResultsPendingCount = result_count;

 for(unsigned i = 0; i < result_count; i++)
  AsyncResultsPending[i] = r[i];

 AsyncIRQPending = irq;

 CheckAIP();
}

void PS_CDC::SetAIP(unsigned irq, uint8 result0)
{
 uint8 tr[1] = { result0 };
 SetAIP(irq, 1, tr);
}

void PS_CDC::SetAIP(unsigned irq, uint8 result0, uint8 result1)
{
 uint8 tr[2] = { result0, result1 };
 SetAIP(irq, 2, tr);
}


pscpu_timestamp_t PS_CDC::Update(const pscpu_timestamp_t timestamp)
{
 int32 clocks = timestamp - lastts;

 while(clocks > 0)
 {
  int32 chunk_clocks = clocks;

  if(PSRCounter > 0 && chunk_clocks > PSRCounter)
   chunk_clocks = PSRCounter;

  if(PendingCommandCounter > 0 && chunk_clocks > PendingCommandCounter)
   chunk_clocks = PendingCommandCounter;

  if(chunk_clocks > SPUCounter)
   chunk_clocks = SPUCounter;

  if(DiscStartupDelay > 0)
  {
   if(chunk_clocks > DiscStartupDelay)
    chunk_clocks = DiscStartupDelay;

   DiscStartupDelay -= chunk_clocks;

   if(DiscStartupDelay <= 0)
   {
    DriveStatus = DS_PAUSED;	// or is it supposed to be DS_STANDBY?
   }
  }

  //MDFN_DispMessage("%02x %d -- %d %d -- %02x", IRQBuffer, CDCReadyReceiveCounter, PSRCounter, PendingCommandCounter, PendingCommand);

  if(!IRQBuffer)
  {
   if(CDCReadyReceiveCounter > 0 && chunk_clocks > CDCReadyReceiveCounter)
    chunk_clocks = CDCReadyReceiveCounter;

   if(CDCReadyReceiveCounter > 0)
    CDCReadyReceiveCounter -= chunk_clocks;
  }

  CheckAIP();

  if(PSRCounter > 0)
  {
   uint8 buf[2352 + 96];

   PSRCounter -= chunk_clocks;

   if(PSRCounter <= 0) 
   {
    if(DriveStatus == DS_RESETTING)
    {
     SetAIP(CDCIRQ_COMPLETE, MakeStatus());

     Muted = false; // Does it get reset here?
     ClearAudioBuffers();

     SB_In = 0;

     Mode = 0;
     CurSector = 0;
     CommandLoc = 0;

     DriveStatus = DS_PAUSED;	// or DS_STANDBY?
     ClearAIP();
    }
    else if(DriveStatus == DS_SEEKING)
    {
     CurSector = SeekTarget;
     Cur_CDIF->ReadRawSector(buf, CurSector);
     DecodeSubQ(buf + 2352);

     DriveStatus = StatusAfterSeek;

     if(DriveStatus != DS_PAUSED && DriveStatus != DS_STANDBY)
     {
      PSRCounter = 33868800 / (75 * ((Mode & MODE_SPEED) ? 2 : 1));
     }
    }
    else if(DriveStatus == DS_SEEKING_LOGICAL)
    {
     CurSector = SeekTarget;
     Cur_CDIF->ReadRawSector(buf, CurSector);
     DecodeSubQ(buf + 2352);
     memcpy(HeaderBuf, buf + 12, 12);

     DriveStatus = StatusAfterSeek;

     if(DriveStatus != DS_PAUSED && DriveStatus != DS_STANDBY)
     {
      PSRCounter = 33868800 / (75 * ((Mode & MODE_SPEED) ? 2 : 1));
     }
    }
    else if(DriveStatus == DS_READING)
    {
     if(CurSector >= (int32)toc.tracks[100].lba)
     {
      PSX_WARNING("[CDC] Beyond end!");
      DriveStatus = DS_STOPPED;

      SetAIP(CDCIRQ_DISC_ERROR, MakeStatus() | 0x04, 0x04);
     }
     else if((Mode & MODE_STRSND) && AudioBuffer_UsedCount == AudioBuffer_Count)
     {
      PSRCounter += 33868800 / (75 * ((Mode & MODE_SPEED) ? 2 : 1)) / 4;
     }
     else
     {
      //PSX_WARNING("Read sector: %d", CurSector);

      Cur_CDIF->ReadRawSector(buf, CurSector);	// FIXME: error out on error.
      DecodeSubQ(buf + 2352);

      memcpy(HeaderBuf, buf + 12, 12);
      HeaderBufValid = true;

      if((Mode & MODE_STRSND) && (buf[12 + 3] == 0x2) && (buf[12 + 6] & 0x20) && (buf[12 + 6] & 0x04))
      {
       if(XA_Test(buf))
       {
	if(AudioBuffer_ReadPos & 0xFFF)
	 printf("readpos=%04x(rabl=%04x) writepos=%04x\n", AudioBuffer_ReadPos, AudioBuffer[AudioBuffer_ReadPos >> 12].Size, AudioBuffer_WritePos);

	//if(AudioBuffer_UsedCount == 0)
	// AudioBuffer_InPrebuffer = true;

        XA_ProcessSector(buf, &AudioBuffer[AudioBuffer_WritePos >> 12]);
	AudioBuffer_UsedCount++;

	if(AudioBuffer_UsedCount == AudioBuffer_PreBufferCount)
	 AudioBuffer_InPrebuffer = false;

	AudioBuffer_WritePos = (AudioBuffer_WritePos & 0xFFF) | ((((AudioBuffer_WritePos >> 12) + 1) % AudioBuffer_Count) << 12);
       }
      }
      else
      {
       // maybe if(!(Mode & 0x30)) too?
       if(!(buf[12 + 6] & 0x20))
       {
	if(!edc_lec_check_correct(buf, true))
	{
	 MDFN_DispMessage("Bad sector? - %d", CurSector);
	}
       }

       if(!(Mode & 0x30) && (buf[12 + 6] & 0x20))
	PSX_WARNING("BORK: %d", CurSector);

       {
	int32 offs = (Mode & 0x20) ? 0 : 12;
	int32 size = (Mode & 0x20) ? 2340 : 2048;

	if(Mode & 0x10)
	{
	 offs = 12;
	 size = 2328;
	}

 	memcpy(SB, buf + 12 + offs, size);
 	SB_In = size;
       }

       SetAIP(CDCIRQ_DATA_READY, MakeStatus());
      }

      PSRCounter += 33868800 / (75 * ((Mode & MODE_SPEED) ? 2 : 1));
      CurSector++;
     }
    }
    else if(DriveStatus == DS_PLAYING)
    {
     if(CurSector >= (int32)toc.tracks[100].lba)
     {
      HeaderBufValid = false;
      DriveStatus = DS_STOPPED;
      SetAIP(CDCIRQ_DISC_ERROR, MakeStatus() | 0x04, 0x04);	// TODO: Verify
     }
     else
     {
      if(AudioBuffer_UsedCount < AudioBuffer_Count)
      {
       Cur_CDIF->ReadRawSector(buf, CurSector);	// FIXME: error out on error.
       DecodeSubQ(buf + 2352);

       // Note: Some game(s) start playing in the pregap of a track(so don't replace this with a simple subq index == 0 check for autopause).
       if(PlayTrackMatch == -1 && SubQChecksumOK)
	PlayTrackMatch = SubQBuf_Safe[0x1];

       if(1)	//if(Mode & MODE_CDDA)  MODE_CDDA doesn't seem to be required for CD-DA playback, so then what is it for?
       {
        CD_Audio_Buffer *ab = &AudioBuffer[AudioBuffer_WritePos >> 12];

        ab->Freq = 7 * ((Mode & MODE_SPEED) ? 2 : 1);
        ab->Size = 588;

	if(SubQBuf_Safe[0] & 0x40)
	{
	 for(int i = 0; i < 588; i++)
	 {
	  ab->Samples[0][i] = 0;
	  ab->Samples[1][i] = 0;
	 }
	}
	else
	{
	 for(int i = 0; i < 588; i++)
	 {
	  ab->Samples[0][i] = (int16)MDFN_de16lsb(&buf[i * sizeof(int16) * 2 + 0]);
	  ab->Samples[1][i] = (int16)MDFN_de16lsb(&buf[i * sizeof(int16) * 2 + 2]);
	 }
	}

	//if(AudioBuffer_UsedCount == 0)
 	// AudioBuffer_InPrebuffer = true;

	AudioBuffer_UsedCount++;

	if(AudioBuffer_UsedCount == AudioBuffer_PreBufferCount)
	 AudioBuffer_InPrebuffer = false;

	AudioBuffer_WritePos = (AudioBuffer_WritePos & 0xFFF) | ((((AudioBuffer_WritePos >> 12) + 1) % AudioBuffer_Count) << 12);
       }

       PSRCounter += 33868800 / (75 * ((Mode & MODE_SPEED) ? 2 : 1));

       if((Mode & MODE_AUTOPAUSE) && PlayTrackMatch != -1 && SubQBuf_Safe[0x1] != PlayTrackMatch)
       {
	// Status needs to be taken before we're paused(IE it should still report playing).
	SetAIP(CDCIRQ_DATA_END, MakeStatus());

	DriveStatus = DS_PAUSED;
	PSRCounter = 0;
       }
       else if((Mode & MODE_REPORT) && (!(SubQBuf_Safe[0x9] & 0xF) || Forward || Backward) && SubQChecksumOK)	// Not sure about accurate notification behavior for corrupt SubQ data
       {
	uint8 tr[8];

	tr[0] = MakeStatus();
	tr[1] = SubQBuf_Safe[0x1];	// Track
	tr[2] = SubQBuf_Safe[0x2];	// Index

	if(SubQBuf_Safe[0x9] & 0x10)
	{
	 tr[3] = SubQBuf_Safe[0x3];		// R M
	 tr[4] = SubQBuf_Safe[0x4] | 0x80;	// R S
 	 tr[5] = SubQBuf_Safe[0x5];		// R F
	}
	else	
	{
	 tr[3] = SubQBuf_Safe[0x7];	// A M
	 tr[4] = SubQBuf_Safe[0x8];	// A S
	 tr[5] = SubQBuf_Safe[0x9];	// A F
	}

	tr[6] = 0;	// ??
	tr[7] = 0;	// ??

	SetAIP(CDCIRQ_DATA_READY, 8, tr);
       }

	// FIXME: What's the real fast-forward and backward speed?
       if(Forward)
        CurSector += 12;
       else if(Backward)
       {
        CurSector -= 12;

        if(CurSector < 0)	// FIXME: How does a real PS handle this condition?
         CurSector = 0;
       }
       else
        CurSector++;
      }
      else
       PSX_WARNING("[CDC] BUG CDDA buffer full");

     }
    } // end if playing
   }
  }

  if(PendingCommandCounter > 0)
  {
   PendingCommandCounter -= chunk_clocks;

   if(PendingCommandCounter <= 0 && CDCReadyReceiveCounter > 0)
   {
    PendingCommandCounter = CDCReadyReceiveCounter; //256;
   }
   //else if(PendingCommandCounter <= 0 && PSRCounter > 0 && PSRCounter < 2000)
   //{
   // PendingCommandCounter = PSRCounter + 1;
   //}
   else if(PendingCommandCounter <= 0)
   {
    int32 next_time = 0;

    BeginResults();

    if(PendingCommandPhase)
    {
     const CDC_CTEntry *command = &Commands[PendingCommand];

     next_time = (this->*(command->func2))();
    }
    else if(PendingCommand >= 0x20 || !Commands[PendingCommand].func)
    {
     PSX_WARNING("[CDC] Unknown command: 0x%02x", PendingCommand);

     WriteResult(MakeStatus(true));
     WriteResult(ERRCODE_BAD_COMMAND);
     WriteIRQ(CDCIRQ_DISC_ERROR);

     ArgsIn = 0;
    }
    else if(ArgsIn < Commands[PendingCommand].args_min || ArgsIn > Commands[PendingCommand].args_max)
    {
     PSX_WARNING("[CDC] Bad number(%d) of args(first check) for command 0x%02x", ArgsIn, PendingCommand);

     WriteResult(MakeStatus(true));
     WriteResult(ERRCODE_BAD_NUMARGS);
     WriteIRQ(CDCIRQ_DISC_ERROR);

     ArgsIn = 0;
    }
    else
    {
     const CDC_CTEntry *command = &Commands[PendingCommand];
     //PSX_WARNING("[CDC] Command: %s --- %d", command->name, Results.CanRead());

#if 1
     printf("[CDC] Command: %s --- ", command->name);
     for(unsigned int i = 0; i < ArgsIn; i++)
      printf(" 0x%02x", ArgsBuf[i]);
     printf("\n");
#endif
     next_time = (this->*(command->func))(ArgsIn, ArgsBuf);
     PendingCommandPhase = 1;
     ArgsIn = 0;
    }

    if(!next_time)
     PendingCommandCounter = 0;
    else
     PendingCommandCounter += next_time;
   }
  }

  SPUCounter = SPU->UpdateFromCDC(chunk_clocks);

  clocks -= chunk_clocks;
 } // end while(clocks > 0)

 lastts = timestamp;

 return(timestamp + CalcNextEvent());
}

void PS_CDC::Write(const pscpu_timestamp_t timestamp, uint32 A, uint8 V)
{
 A &= 0x3;

 //printf("Write: %08x %02x\n", A, V);

 if(A == 0x00)
 {
  RegSelector = V & 0x3;
 }
 else
 {
  const unsigned reg_index = ((RegSelector & 0x3) * 3) + (A - 1);

  Update(timestamp);
  //PSX_WARNING("[CDC] Write to register 0x%02x: 0x%02x @ %d --- 0x%02x\n", reg_index, V, timestamp, DMABuffer.CanRead());

  switch(reg_index)
  {
	default:
		PSX_WARNING("[CDC] Unknown write to register 0x%02x: 0x%02x\n", reg_index, V);
		break;

	 case 0x00:
		if(PendingCommandCounter > 0)
		{
		 PSX_WARNING("[CDC] WARNING: Interrupting command 0x%02x, phase=%d, timeleft=%d with command=0x%02x", PendingCommand, PendingCommandPhase,
			PendingCommandCounter, V);
		}

		if(IRQBuffer)
		{
		 PSX_WARNING("[CDC] Attempting to start command(0x%02x) while IRQBuffer is not clear.", V);
		}

		if(ResultsIn > 0)
		{
		 PSX_WARNING("[CDC] Attempting to start command(0x%02x) while command results(count=%d) still in buffer.", V, ResultsIn);
		}

         	PendingCommandCounter = 8192; //1024; //128; //256; //16; //1024;
	 	PendingCommand = V;
         	PendingCommandPhase = 0;
		break;

	 case 0x01:
		ArgsBuf[ArgsIn & 0xF] = V;
		ArgsIn = (ArgsIn + 1) & 0x1F;

		if(!(ArgsIn & 0x0F))
		{
		 PSX_WARNING("[CDC] Argument buffer overflow");
		}
		break;

	 case 0x02:
 	 	if(V & 0x80)
	 	{
	  	 if(!DMABuffer.CanRead())
	  	 {
		  if(!SB_In)
		  {
		   PSX_WARNING("[CDC] Data read begin when no data to read!");

		   DMABuffer.Write(SB, 2340);

		   while(DMABuffer.CanWrite())
		    DMABuffer.WriteByte(0x00);
		  }
		  else
		  {
	   	   DMABuffer.Write(SB, SB_In);
		   SB_In = 0;
		  }
	  	 }
		 else
		 {
		  //PSX_WARNING("[CDC] Attempt to start data transfer via 0x80->1803 when %d bytes still in buffer", DMABuffer.CanRead());
		 }
	 	}
	 	else if(V & 0x40)	// Something CD-DA related(along with & 0x20 ???)?
	 	{
		 for(unsigned i = 0; i < 4 && DMABuffer.CanRead(); i++)
		  DMABuffer.ReadByte();
		}
		else
		{
		 DMABuffer.Flush();
		}

		if(V & 0x20)
		{
		 PSX_WARNING("[CDC] Mystery IRQ trigger bit set.");
		 IRQBuffer |= 0x10;
		}
		break;

	 case 0x04:
	 	IRQOutTestMask = V;
		RecalcIRQ();
		break;

	 case 0x05:
	 	IRQBuffer &= ~V;
	 	RecalcIRQ();

		if(V & 0x80)	// Forced CD hardware reset of some kind(interface, controller, and drive?)  Seems to take a while(relatively speaking) to complete.
		{
		 PSX_WARNING("[CDC] Soft Reset");
		 SoftReset();
		}

		if(V & 0x40)	// Does it clear more than arguments buffer?  Doesn't appear to clear results buffer.
		{
		 ArgsIn = 0;
		}
		break;

	 case 0x07:
		Pending_DecodeVolume[0][0] = V;
		break;

	 case 0x08:
		Pending_DecodeVolume[0][1] = V;
		break;

	 case 0x09:
		Pending_DecodeVolume[1][1] = V;
		break;

	 case 0x0A:
		Pending_DecodeVolume[1][0] = V;
		break;

	 case 0x0B:
		if(V & 0x20)
		{
		 memcpy(DecodeVolume, Pending_DecodeVolume, sizeof(DecodeVolume));

		 for(int i = 0; i < 2; i++)
		 {
		  for(int o = 0; o < 2; o++)
		  {
		   //fprintf(stderr, "Input Channel %d, Output Channel %d -- Volume=%d\n", i, o, DecodeVolume[i][o]);
		  }
		 }
		}
		break;
  }
  PSX_SetEventNT(PSX_EVENT_CDC, timestamp + CalcNextEvent());
 }
}

uint8 PS_CDC::Read(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint8 ret = 0;

 A &= 0x03;

 //printf("Read %08x\n", A);

 if(A == 0x00)
 {
	ret = RegSelector & 0x3;

	ret |= 0x18;	// Unknown purpose

	if(ResultsIn)
	 ret |= 0x20;

	if(DMABuffer.CanRead())
	 ret |= 0x40;

        if(PendingCommandCounter > 0 && PendingCommandPhase == 0)
	 ret |= 0x80;
 }
 else
 {
  switch(A & 0x3)
  {
   case 0x01:
	ret = ReadResult();
	break;

   case 0x02:
	if(DMABuffer.CanRead())
	 ret = DMABuffer.ReadByte();
	else
	{
	 PSX_WARNING("[CDC] CD data transfer port read, but no data present!");
	}
	break;

   case 0x03:
	if(RegSelector & 0x1)
	{
	 ret = 0xE0 | IRQBuffer;
	}
	else
	{
	 ret = 0xFF;
	}
	break;
  }
 }

 return(ret);
}


bool PS_CDC::DMACanRead(void)
{
 return(DMABuffer.CanRead());
}

uint32 PS_CDC::DMARead(void)
{
 uint32 data = 0;

 for(int i = 0; i < 4; i++)
 {
  if(DMABuffer.CanRead())
   data |= DMABuffer.ReadByte() << (i * 8);
  else
  {
   //assert(0);
  }
 }

 return(data);
}

bool PS_CDC::CommandCheckDiscPresent(void)
{
 if(!Cur_CDIF || DiscStartupDelay > 0)
 {
  WriteResult(MakeStatus(true));
  WriteResult(ERRCODE_NOT_READY);

  WriteIRQ(CDCIRQ_DISC_ERROR);

  return(false);
 }

 return(true);
}

int32 PS_CDC::Command_Sync(const int arg_count, const uint8 *args)
{
 PSX_WARNING("[CDC] Unimplemented command: 0x%02x", PendingCommand);
 return(0);
}

int32 PS_CDC::Command_Nop(const int arg_count, const uint8 *args)
{
 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_Setloc(const int arg_count, const uint8 *args)
{
 uint8 m, s, f;

 m = BCD_to_U8(args[0] & 0x7F);
 s = BCD_to_U8(args[1]);
 f = BCD_to_U8(args[2]);

 CommandLoc = f + 75 * s + 75 * 60 * m - 150;
 CommandLoc_Dirty = true;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::CalcSeekTime(int32 initial, int32 target, bool motor_on, bool paused)
{
 int32 ret = 0;

 if(!motor_on)
 {
  initial = 0;
  ret += 33868800;
 }

 ret += std::max<int64>((int64)abs(initial - target) * 33868800 * 1000 / (72 * 60 * 75) / 1000, 10000);

 if(abs(initial - target) >= 2250)
  ret += (int64)33868800 * 300 / 1000;
 else if(paused)
 {
  // The delay to restart from a Pause state is...very....WEIRD.  The time it takes is related to the amount of time that has passed since the pause, and
  // where on the disc the laser head is, with generally more time passed = longer to resume, except that there's a window of time where it takes a
  // ridiculous amount of time when not much time has passed.
  // 
  // What we have here will be EXTREMELY simplified.

  //
  //

  //if(time_passed >= 67737)
  //{
  //}
  //else
  {
   // Take twice as long for 1x mode.
   ret += 1247952 * ((Mode & MODE_SPEED) ? 1 : 2);
  }
 }

 printf("%d\n", ret);

 return(ret);
}

#if 0
void PS_CDC::BeginSeek(uint32 target, int after_seek)
{
 SeekTarget = target;
 StatusAfterSeek = after_seek;

 PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
}
#endif

// Remove this function when we have better seek emulation; it's here because the Rockman complete works games(at least 2 and 4) apparently have finicky fubared CD
// access code.
void PS_CDC::PreSeekHack(uint32 target)
{
 uint8 buf[2352 + 96];
 int max_try = 32;

 CurSector = target;

 if(target < toc.tracks[100].lba)
 {
  do
  {
   Cur_CDIF->ReadRawSector(buf, target++);
  } while(!DecodeSubQ(buf + 2352) && --max_try > 0 && target < toc.tracks[100].lba);
 }
}

int32 PS_CDC::Command_Play(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 ClearAIP();

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 Forward = Backward = false;

 if(arg_count && args[0])
 {
  int track = BCD_to_U8(args[0]);

  if(track < toc.first_track)
  {
   PSX_WARNING("[CDC] Attempt to play track before first track.");
   track = toc.first_track;
  }
  else if(track > toc.last_track)
  {
   PSX_WARNING("[CDC] Attempt to play track before first track.");
   track = toc.last_track;
  }

  ClearAudioBuffers();

  PlayTrackMatch = track;

  printf("[CDC] Play track: %d\n", track);
  SeekTarget = toc.tracks[track].lba;
  PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
  HeaderBufValid = false;
  PreSeekHack(SeekTarget);

  DriveStatus = DS_SEEKING;
  StatusAfterSeek = DS_PLAYING;
 }
 else
 {
  if(CommandLoc_Dirty || (DriveStatus != DS_PLAYING && DriveStatus != DS_PAUSED && DriveStatus != DS_STANDBY))
  {
   ClearAudioBuffers();
   SeekTarget = CommandLoc;
   PlayTrackMatch = -1;

   PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
   HeaderBufValid = false;
   PreSeekHack(SeekTarget);

   DriveStatus = DS_SEEKING;
   StatusAfterSeek = DS_PLAYING;
  }
  else if(DriveStatus != DS_PLAYING)
  {
   ClearAudioBuffers();
   SeekTarget = CurSector;
   PlayTrackMatch = -1;

   PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
   HeaderBufValid = false;
   PreSeekHack(SeekTarget);

   DriveStatus = DS_SEEKING;
   StatusAfterSeek = DS_PLAYING;
  }
 }

 CommandLoc_Dirty = false;
 return(0);
}

int32 PS_CDC::Command_Forward(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 Backward = false;
 Forward = true;

 return(0);
}

int32 PS_CDC::Command_Backward(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 Backward = true;
 Forward = false;

 return(0);
}


void PS_CDC::ReadBase(void)
{
 if(!CommandCheckDiscPresent())
  return;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 if(DriveStatus == DS_SEEKING_LOGICAL && SeekTarget == CommandLoc && StatusAfterSeek == DS_READING)
 {
  CommandLoc_Dirty = false;
  return;
 }

 if(CommandLoc_Dirty || DriveStatus != DS_READING)
 {
  CommandLoc_Dirty = false;


  ClearAIP();
  ClearAudioBuffers();
  DMABuffer.Flush();
  SB_In = 0;

  // TODO: separate motor start from seek phase?

  SeekTarget = CommandLoc;

  PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
  HeaderBufValid = false;
  PreSeekHack(SeekTarget);

  DriveStatus = DS_SEEKING_LOGICAL;
  StatusAfterSeek = DS_READING;
 }
}

int32 PS_CDC::Command_ReadN(const int arg_count, const uint8 *args)
{
 ReadBase();
 return 0;
}

int32 PS_CDC::Command_ReadS(const int arg_count, const uint8 *args)
{
 ReadBase();
 return 0;
}

int32 PS_CDC::Command_Stop(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 if(DriveStatus == DS_STOPPED)
 {
  return(5000);
 }
 else
 {
  ClearAudioBuffers();
  ClearAIP();
  DriveStatus = DS_STOPPED;
  HeaderBufValid = false;

  return(33868);	// FIXME, should be much higher.
 }
}

int32 PS_CDC::Command_Stop_Part2(void)
{
 PSRCounter = 0;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_COMPLETE);

 return(0);
}

int32 PS_CDC::Command_Standby(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 if(DriveStatus != DS_STOPPED)
 {
  WriteResult(MakeStatus(true));
  WriteResult(0x20);
  WriteIRQ(CDCIRQ_DISC_ERROR);
  return(0);
 }

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 ClearAudioBuffers();
 ClearAIP();
 DriveStatus = DS_STANDBY;

 return((int64)33868800 * 100 / 1000);	// No idea, FIXME.
}

int32 PS_CDC::Command_Standby_Part2(void)
{
 PSRCounter = 0;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_COMPLETE);

 return(0);
}

int32 PS_CDC::Command_Pause(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 if(DriveStatus == DS_PAUSED || DriveStatus == DS_STOPPED)
 {
  return(5000);
 }
 else
 {
  // "Viewpoint" flips out and crashes if reading isn't stopped (almost?) immediately.
  ClearAudioBuffers();
  ClearAIP();
  DriveStatus = DS_PAUSED;

  // An approximation.
  return((1124584 + ((int64)CurSector * 42596 / (75 * 60))) * ((Mode & MODE_SPEED) ? 1 : 2));
 }
}

int32 PS_CDC::Command_Pause_Part2(void)
{
 PSRCounter = 0;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_COMPLETE);

 return(0);
}

int32 PS_CDC::Command_Reset(const int arg_count, const uint8 *args)
{
 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 if(DriveStatus != DS_RESETTING)
 {
  HeaderBufValid = false;
  DriveStatus = DS_RESETTING;
  PSRCounter = 1136000;
 }

 return(0);
}

int32 PS_CDC::Command_Mute(const int arg_count, const uint8 *args)
{
 Muted = true;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_Demute(const int arg_count, const uint8 *args)
{
 Muted = false;

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_Setfilter(const int arg_count, const uint8 *args)
{
 FilterFile = args[0];
 FilterChan = args[1];

 //PSX_WARNING("[CDC] Setfilter: %02x %02x", args[0], args[1]);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_Setmode(const int arg_count, const uint8 *args)
{
 PSX_DBGINFO("[CDC] Set mode 0x%02x", args[0]);
 Mode = args[0];

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_Getparam(const int arg_count, const uint8 *args)
{
 WriteResult(MakeStatus());
 WriteResult(Mode);
 WriteResult(0x00);
 WriteResult(FilterFile);
 WriteResult(FilterChan);

 WriteIRQ(CDCIRQ_ACKNOWLEDGE);


 return(0);
}

int32 PS_CDC::Command_GetlocL(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 if(!HeaderBufValid)
 {
  WriteResult(MakeStatus(true));
  WriteResult(0x80);
  WriteIRQ(CDCIRQ_DISC_ERROR);
  return(0);
 }

 for(unsigned i = 0; i < 8; i++)
 {
  //printf("%d %d: %02x\n", DriveStatus, i, HeaderBuf[i]);
  WriteResult(HeaderBuf[i]);
 }

 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_GetlocP(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(SubQBuf_Safe[0x1]);	// Track
 WriteResult(SubQBuf_Safe[0x2]);	// Index
 WriteResult(SubQBuf_Safe[0x3]);	// R M
 WriteResult(SubQBuf_Safe[0x4]);	// R S
 WriteResult(SubQBuf_Safe[0x5]);	// R F
 WriteResult(SubQBuf_Safe[0x7]);	// A M
 WriteResult(SubQBuf_Safe[0x8]);	// A S
 WriteResult(SubQBuf_Safe[0x9]);	// A F

 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_ReadT(const int arg_count, const uint8 *args)
{
 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(44100 * 768 / 1000);
}

int32 PS_CDC::Command_ReadT_Part2(void)
{
 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_COMPLETE);

 return(0);
}

int32 PS_CDC::Command_GetTN(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteResult(U8_to_BCD(toc.first_track));
 WriteResult(U8_to_BCD(toc.last_track));

 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_GetTD(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 int track;
 uint8 m, s, f;

 if(!args[0] || args[0] == 0xAA)
  track = 100;
 else
 {
  track= BCD_to_U8(args[0]);

  if(track < toc.first_track || track > toc.last_track)	// Error
  {
   WriteResult(MakeStatus(true));
   WriteIRQ(CDCIRQ_ACKNOWLEDGE);
   return(0);
  }
 }

 LBA_to_AMSF(toc.tracks[track].lba, &m, &s, &f);

 WriteResult(MakeStatus());
 WriteResult(U8_to_BCD(m));
 WriteResult(U8_to_BCD(s));
 //WriteResult(U8_to_BCD(f));

 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 return(0);
}

int32 PS_CDC::Command_SeekL(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 SeekTarget = CommandLoc;

 PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
 HeaderBufValid = false;
 PreSeekHack(SeekTarget);
 DriveStatus = DS_SEEKING_LOGICAL;
 StatusAfterSeek = DS_STANDBY;
 ClearAIP();

 return(PSRCounter);
}

int32 PS_CDC::Command_SeekP(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 SeekTarget = CommandLoc;

 PSRCounter = CalcSeekTime(CurSector, SeekTarget, DriveStatus != DS_STOPPED, DriveStatus == DS_PAUSED);
 HeaderBufValid = false;
 PreSeekHack(SeekTarget);
 DriveStatus = DS_SEEKING;
 StatusAfterSeek = DS_STANDBY;
 ClearAIP();

 return(PSRCounter);
}

int32 PS_CDC::Command_Seek_PartN(void)
{
 if(DriveStatus == DS_STANDBY)
 {
  BeginResults();
  WriteResult(MakeStatus());
  WriteIRQ(CDCIRQ_COMPLETE);

  return(0);
 }
 else
 {
  return(std::max<int32>(PSRCounter, 256));
 }
}

int32 PS_CDC::Command_Test(const int arg_count, const uint8 *args)
{
 //PSX_WARNING("[CDC] Test command sub-operation: 0x%02x", args[0]);

 switch(args[0])
 {
  default:
	PSX_WARNING("[CDC] Unknown Test command sub-operation: 0x%02x", args[0]);
	WriteResult(MakeStatus(true));
	WriteResult(0x10);
	WriteIRQ(CDCIRQ_DISC_ERROR);
	break;

  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
  case 0x16:
  case 0x17:
  case 0x18:
  case 0x19:
  case 0x1A:
	PSX_WARNING("[CDC] Unknown Test command sub-operation: 0x%02x", args[0]);
	WriteResult(MakeStatus());
	WriteIRQ(CDCIRQ_ACKNOWLEDGE);
  	break;

#if 0
  case 0x50:	// *Need to retest this test command, it takes additional arguments??? Or in any case, it generates a different error code(0x20) than most other Test
		// sub-commands that generate an error code(0x10).
	break;

 // Same with 0x60, 0x71-0x76

#endif

  case 0x51:	// *Need to retest this test command
	PSX_WARNING("[CDC] Unknown Test command sub-operation: 0x%02x", args[0]);
	WriteResult(0x01);
	WriteResult(0x00);
	WriteResult(0x00);
	break;

  case 0x75:	// *Need to retest this test command
	PSX_WARNING("[CDC] Unknown Test command sub-operation: 0x%02x", args[0]);
	WriteResult(0x00);
	WriteResult(0xC0);
	WriteResult(0x00);
	WriteResult(0x00);
	break;

  // 
  // SCEx counters not reset by command 0x0A.
  //

  case 0x04:	// Reset SCEx counters
	WriteResult(MakeStatus());
	WriteIRQ(CDCIRQ_ACKNOWLEDGE);
  	break;

  case 0x05:	// Read SCEx counters
	WriteResult(0x00);	// Number of TOC/leadin reads? (apparently increases by 1 or 2 per ReadTOC, even on non-PSX music CD)
	WriteResult(0x00);	// Number of SCEx strings received? (Stays at zero on music CD)
	WriteIRQ(CDCIRQ_ACKNOWLEDGE);
  	break;

  case 0x20:
	{
	 WriteResult(0x97);
	 WriteResult(0x01);
	 WriteResult(0x10);
	 WriteResult(0xC2);

	 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
	}
	break;

  case 0x21:	// *Need to retest this test command.
	{
	 WriteResult(0x01);
	 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
	}
	break;

  case 0x22:
	{
	 static const uint8 td[7] = { 0x66, 0x6f, 0x72, 0x20, 0x55, 0x2f, 0x43 };

	 for(unsigned i = 0; i < 7; i++)
	  WriteResult(td[i]);

	 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
	}
	break;

  case 0x23:
  case 0x24:
	{
	 static const uint8 td[8] = { 0x43, 0x58, 0x44, 0x32, 0x35, 0x34, 0x35, 0x51 };

	 for(unsigned i = 0; i < 8; i++)
	  WriteResult(td[i]);

	 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
	}
	break;

  case 0x25:
	{
	 static const uint8 td[8] = { 0x43, 0x58, 0x44, 0x31, 0x38, 0x31, 0x35, 0x51 };

	 for(unsigned i = 0; i < 8; i++)
	  WriteResult(td[i]);

	 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
	}
	break;
 }
 return(0);
}

int32 PS_CDC::Command_ID(const int arg_count, const uint8 *args)
{
 if(!CommandCheckDiscPresent())
  return(0);

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);

 HeaderBufValid = false;
 PSRCounter = 0;
 DriveStatus = DS_PAUSED;	// or DS_STANDBY?
 ClearAIP();

 return(33868);
}

int32 PS_CDC::Command_ID_Part2(void)
{
 if(IsPSXDisc)
 {
  WriteResult(MakeStatus());
  WriteResult(0x00);
  WriteResult(0x20);
  WriteResult(0x00);
 }
 else
 {
  WriteResult(MakeStatus() | 0x08);
  WriteResult(0x90);
  WriteResult(0x00);
  WriteResult(0x00);
 }

 if(IsPSXDisc)
 {
  WriteResult(DiscID[0]);
  WriteResult(DiscID[1]);
  WriteResult(DiscID[2]);
  WriteResult(DiscID[3]);
 }
 else
 {
  WriteResult(0xff);
  WriteResult(0);
  WriteResult(0);
  WriteResult(0);
 }

 WriteIRQ(CDCIRQ_COMPLETE);

 return(0);
}

int32 PS_CDC::Command_Init(const int arg_count, const uint8 *args)
{
 return(0);
}

int32 PS_CDC::Command_ReadTOC(const int arg_count, const uint8 *args)
{
 // Tested; ReadTOC doesn't error out if the tray is open, and it completes rather quickly in that case.
 //
 //if(!CommandCheckDiscPresent())
 // return(0);

 HeaderBufValid = false;
 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
 
 return((int64)33868800 * 500 / 1000);
}

int32 PS_CDC::Command_ReadTOC_Part2(void)
{
 DriveStatus = DS_PAUSED;	// or DS_STANDBY?
 ClearAIP();

 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_COMPLETE);

 return(0);
}

int32 PS_CDC::Command_0x1d(const int arg_count, const uint8 *args)
{
 WriteResult(MakeStatus());
 WriteIRQ(CDCIRQ_ACKNOWLEDGE);
 return(0);
}

PS_CDC::CDC_CTEntry PS_CDC::Commands[0x20] =
{
 { /* 0x00, */ 0, 0, "Sync", &PS_CDC::Command_Sync, NULL },
 { /* 0x01, */ 0, 0, "Nop", &PS_CDC::Command_Nop, NULL },
 { /* 0x02, */ 3, 3, "Setloc", &PS_CDC::Command_Setloc, NULL },
 { /* 0x03, */ 0, 1, "Play", &PS_CDC::Command_Play, NULL },
 { /* 0x04, */ 0, 0, "Forward", &PS_CDC::Command_Forward, NULL },
 { /* 0x05, */ 0, 0, "Backward", &PS_CDC::Command_Backward, NULL },
 { /* 0x06, */ 0, 0, "ReadN", &PS_CDC::Command_ReadN, NULL },
 { /* 0x07, */ 0, 0, "Standby", &PS_CDC::Command_Standby, &PS_CDC::Command_Standby_Part2 },
 { /* 0x08, */ 0, 0, "Stop", &PS_CDC::Command_Stop, &PS_CDC::Command_Stop_Part2 },
 { /* 0x09, */ 0, 0, "Pause", &PS_CDC::Command_Pause, &PS_CDC::Command_Pause_Part2 },
 { /* 0x0A, */ 0, 0, "Reset", &PS_CDC::Command_Reset, NULL },
 { /* 0x0B, */ 0, 0, "Mute", &PS_CDC::Command_Mute, NULL },
 { /* 0x0C, */ 0, 0, "Demute", &PS_CDC::Command_Demute, NULL },
 { /* 0x0D, */ 2, 2, "Setfilter", &PS_CDC::Command_Setfilter, NULL },
 { /* 0x0E, */ 1, 1, "Setmode", &PS_CDC::Command_Setmode, NULL },
 { /* 0x0F, */ 0, 0, "Getparam", &PS_CDC::Command_Getparam, NULL },
 { /* 0x10, */ 0, 0, "GetlocL", &PS_CDC::Command_GetlocL, NULL },
 { /* 0x11, */ 0, 0, "GetlocP", &PS_CDC::Command_GetlocP, NULL },
 { /* 0x12, */ 1, 1, "ReadT", &PS_CDC::Command_ReadT, &PS_CDC::Command_ReadT_Part2 },
 { /* 0x13, */ 0, 0, "GetTN", &PS_CDC::Command_GetTN, NULL },
 { /* 0x14, */ 1, 1, "GetTD", &PS_CDC::Command_GetTD, NULL },
 { /* 0x15, */ 0, 0, "SeekL", &PS_CDC::Command_SeekL, &PS_CDC::Command_Seek_PartN },
 { /* 0x16, */ 0, 0, "SeekP", &PS_CDC::Command_SeekP, &PS_CDC::Command_Seek_PartN },

 { /* 0x17, */ 0, 0, NULL, NULL, NULL },
 { /* 0x18, */ 0, 0, NULL, NULL, NULL },

 { /* 0x19, */ 1, 1/* ??? */, "Test", &PS_CDC::Command_Test, NULL },
 { /* 0x1A, */ 0, 0, "ID", &PS_CDC::Command_ID, &PS_CDC::Command_ID_Part2 },
 { /* 0x1B, */ 0, 0, "ReadS", &PS_CDC::Command_ReadS, NULL },
 { /* 0x1C, */ 0, 0, "Init", &PS_CDC::Command_Init, NULL },
 { /* 0x1D, */ 2, 2, "Unknown 0x1D", &PS_CDC::Command_0x1d, NULL },
 { /* 0x1E, */ 0, 0, "ReadTOC", &PS_CDC::Command_ReadTOC, &PS_CDC::Command_ReadTOC_Part2 },
 { /* 0x1F, */ 0, 0, NULL, NULL, NULL },
};


}
