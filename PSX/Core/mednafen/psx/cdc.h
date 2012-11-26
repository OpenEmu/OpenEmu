#ifndef __MDFN_PSX_CDC_H
#define __MDFN_PSX_CDC_H

#include "../cdrom/cdromif.h"
#include "../cdrom/SimpleFIFO.h"
#include "../clamp.h"

namespace MDFN_IEN_PSX
{

struct CD_Audio_Buffer
{
 int16 Samples[2][0x1000];	// [0][...] = l, [1][...] = r
 int32 Size;
 uint32 Freq;
};

class PS_CDC
{
 public:

 PS_CDC();
 ~PS_CDC();

 void SetDisc(bool tray_open, CDIF *cdif, const char disc_id[4]);

 void Power(void);
 int StateAction(StateMem *sm, int load, int data_only);
 void ResetTS(void);

 int32 CalcNextEvent(void);	// Returns in master cycles to next event.

 pscpu_timestamp_t Update(const pscpu_timestamp_t timestamp);

 void Write(const pscpu_timestamp_t timestamp, uint32 A, uint8 V);
 uint8 Read(const pscpu_timestamp_t timestamp, uint32 A);

 bool DMACanRead(void);
 uint32 DMARead(void);

 INLINE uint32 GetCDAudioFreq(void)
 {
  if(AudioBuffer_UsedCount && !AudioBuffer_InPrebuffer)
  {
   const unsigned wb = AudioBuffer_ReadPos >> 12;
   return AudioBuffer[wb].Freq;
  }
  return 0;
 }

 private:
 inline void ApplyVolume(int32 samples[2])
 {
  int32 left_source = samples[0]; //(int16)MDFN_de16lsb(&buf[i * sizeof(int16) * 2 + 0]);
  int32 right_source = samples[1]; //(int16)MDFN_de16lsb(&buf[i * sizeof(int16) * 2 + 2]);
  int32 left_out = ((left_source * DecodeVolume[0][0]) >> 7) + ((right_source * DecodeVolume[1][0]) >> 7);
  int32 right_out = ((left_source * DecodeVolume[0][1]) >> 7) + ((right_source * DecodeVolume[1][1]) >> 7);

  clamp(&left_out, -32768, 32767);
  clamp(&right_out, -32768, 32767);

  if(Muted)
  {
   left_out = right_out = 0;
  }

  samples[0] = left_out;
  samples[1] = right_out;
 }
 public:

 INLINE void GetCDAudio(int32 &l, int32 &r)
 {
  if(AudioBuffer_UsedCount && !AudioBuffer_InPrebuffer)
  {
   const unsigned wb = AudioBuffer_ReadPos >> 12;
   int32 samples[2] = { AudioBuffer[wb].Samples[0][AudioBuffer_ReadPos & 0xFFF], AudioBuffer[wb].Samples[1][AudioBuffer_ReadPos & 0xFFF] };

   ApplyVolume(samples);

   l = samples[0];
   r = samples[1];

   AudioBuffer_ReadPos = ((AudioBuffer_ReadPos + 1) & 0xFFF) | (AudioBuffer_ReadPos & ~0xFFF);

   if((AudioBuffer_ReadPos & 0xFFF) == (AudioBuffer[wb].Size & 0xFFF))
   {
    //printf("RP == size; usedcount(predec)= %d, PSRCounter=%d\n", AudioBuffer_UsedCount, PSRCounter);
    AudioBuffer_ReadPos = ((((AudioBuffer_ReadPos >> 12) + 1) % AudioBuffer_Count) << 12);
    AudioBuffer_UsedCount--;
   }
  }
 }

 private:

 void SoftReset(void);

 CDIF *Cur_CDIF;
 bool DiscChanged;
 int32 DiscStartupDelay;


 enum { AudioBuffer_PreBufferCount = 2 };
 enum { AudioBuffer_Count = 4 };

 CD_Audio_Buffer AudioBuffer[AudioBuffer_Count];
 uint32 AudioBuffer_ReadPos;
 uint32 AudioBuffer_WritePos;
 uint32 AudioBuffer_UsedCount;
 bool AudioBuffer_InPrebuffer;

 uint8 Pending_DecodeVolume[2][2], DecodeVolume[2][2];		// [data_source][output_port]

 void ClearAudioBuffers(void);

 uint8 RegSelector;
 uint8 ArgsBuf[16];
 uint32 ArgsIn;		// 5-bit(0 ... 31)

 uint8 ResultsBuffer[16];
 uint8 ResultsIn;	// 5-bit(0 ... 31)
 uint8 ResultsWP;	// Write position, 4 bit(0 ... 15).
 uint8 ResultsRP;	// Read position, 4 bit(0 ... 15).

 SimpleFIFO<uint8> DMABuffer;
 uint8 SB[2340];
 uint32 SB_In;


 uint8 SubQBuf[0xC];
 uint8 SubQBuf_Safe[0xC];
 bool SubQChecksumOK;

 bool HeaderBufValid;
 uint8 HeaderBuf[12];

 void RecalcIRQ(void);
 enum
 {
  CDCIRQ_NONE = 0,
  CDCIRQ_DATA_READY = 1,
  CDCIRQ_COMPLETE = 2,
  CDCIRQ_ACKNOWLEDGE = 3,
  CDCIRQ_DATA_END = 4,
  CDCIRQ_DISC_ERROR = 5
 };

 // Names are just guessed for these based on what conditions cause them:
 enum
 {
  ERRCODE_BAD_ARGVAL  = 0x10,
  ERRCODE_BAD_NUMARGS = 0x20,
  ERRCODE_BAD_COMMAND = 0x40,
  ERRCODE_NOT_READY = 0x80,	// 0x80 (happens with getlocl when drive isn't reading, pause when tray is open, and MAYBE when trying to run an async
				//	 command while another async command is currently in its asynch phase being executed[pause when in readtoc, todo test more])
 };

 uint8 IRQBuffer;
 uint8 IRQOutTestMask;
 int32 CDCReadyReceiveCounter;	// IRQBuffer being non-zero prevents new results and new IRQ from coming in and erasing the current results,
				// but apparently at least one CONFOUNDED game is clearing the IRQ state BEFORE reading the results, so we need to have a delay
				// between IRQBuffer being cleared to when we allow new results to come in.  (The real thing should be like this too,
				// but the mechanism is probably more nuanced and complex and ugly and I like anchovy pizza)

 void BeginResults(void);
 void WriteIRQ(uint8);
 void WriteResult(uint8);
 uint8 ReadResult(void);

 uint8 FilterFile;
 uint8 FilterChan;


 uint8 PendingCommand;
 bool PendingCommandPhase;
 int32 PendingCommandCounter;

 int32 SPUCounter;

 enum { MODE_SPEED = 0x80 };
 enum { MODE_STRSND = 0x40 };
 enum { MODE_SIZE = 0x20 };
 enum { MODE_SIZE2 = 0x10 };
 enum { MODE_SF = 0x08 };
 enum { MODE_REPORT = 0x04 };
 enum { MODE_AUTOPAUSE = 0x02 };
 enum { MODE_CDDA = 0x01 };
 uint8 Mode;

 enum
 {
  DS_STANDBY = -2,
  DS_PAUSED = -1,
  DS_STOPPED = 0,
  DS_SEEKING,
  DS_SEEKING_LOGICAL,
  DS_PLAY_SEEKING,
  DS_PLAYING,
  DS_READING,
  DS_RESETTING
 };
 int DriveStatus;
 int StatusAfterSeek;
 bool Forward;
 bool Backward;
 bool Muted;

 int32 PlayTrackMatch;

 int32 PSRCounter;

 int32 CurSector;

 unsigned AsyncIRQPending;
 uint8 AsyncResultsPending[16];
 uint8 AsyncResultsPendingCount;

 int32 CalcSeekTime(int32 initial, int32 target, bool motor_on, bool paused);

 void ClearAIP(void);
 void CheckAIP(void);
 void SetAIP(unsigned irq, unsigned result_count, uint8 *r);
 void SetAIP(unsigned irq, uint8 result0);
 void SetAIP(unsigned irq, uint8 result0, uint8 result1);

 int32 SeekTarget;

 pscpu_timestamp_t lastts;

 CDUtility::TOC toc;
 bool IsPSXDisc;
 uint8 DiscID[4];

 int32 CommandLoc;
 bool CommandLoc_Dirty;

 uint8 MakeStatus(bool cmd_error = false);
 bool DecodeSubQ(uint8 *subpw);
 bool CommandCheckDiscPresent(void);

 bool XA_Test(const uint8 *sdata);
 void XA_ProcessSector(const uint8 *sdata, CD_Audio_Buffer *ab);
 int16 xa_previous[2][2];
 bool xa_cur_set;
 uint8 xa_cur_file;
 uint8 xa_cur_chan;

 struct CDC_CTEntry
 {
  uint8 args_min;
  uint8 args_max;
  const char *name;
  int32 (PS_CDC::*func)(const int arg_count, const uint8 *args);
  int32 (PS_CDC::*func2)(void);
 };

 void BeginSeek(uint32 target);
 void PreSeekHack(uint32 target);
 void ReadBase(void);

 static CDC_CTEntry Commands[0x20];

 int32 Command_Sync(const int arg_count, const uint8 *args);
 int32 Command_Nop(const int arg_count, const uint8 *args);
 int32 Command_Setloc(const int arg_count, const uint8 *args);
 int32 Command_Play(const int arg_count, const uint8 *args);
 int32 Command_Forward(const int arg_count, const uint8 *args);
 int32 Command_Backward(const int arg_count, const uint8 *args);
 int32 Command_ReadN(const int arg_count, const uint8 *args);
 int32 Command_Standby(const int arg_count, const uint8 *args);
 int32 Command_Standby_Part2(void);
 int32 Command_Stop(const int arg_count, const uint8 *args);
 int32 Command_Stop_Part2(void); 
 int32 Command_Pause(const int arg_count, const uint8 *args);
 int32 Command_Pause_Part2(void);
 int32 Command_Reset(const int arg_count, const uint8 *args);
 int32 Command_Mute(const int arg_count, const uint8 *args);
 int32 Command_Demute(const int arg_count, const uint8 *args);
 int32 Command_Setfilter(const int arg_count, const uint8 *args);
 int32 Command_Setmode(const int arg_count, const uint8 *args);
 int32 Command_Getparam(const int arg_count, const uint8 *args);
 int32 Command_GetlocL(const int arg_count, const uint8 *args);
 int32 Command_GetlocP(const int arg_count, const uint8 *args);

 int32 Command_ReadT(const int arg_count, const uint8 *args);
 int32 Command_ReadT_Part2(void);

 int32 Command_GetTN(const int arg_count, const uint8 *args);
 int32 Command_GetTD(const int arg_count, const uint8 *args);
 int32 Command_SeekL(const int arg_count, const uint8 *args);
 int32 Command_SeekP(const int arg_count, const uint8 *args);
 int32 Command_Seek_PartN(void);

 int32 Command_Test(const int arg_count, const uint8 *args);

 int32 Command_ID(const int arg_count, const uint8 *args);
 int32 Command_ID_Part2(void);

 int32 Command_ReadS(const int arg_count, const uint8 *args);
 int32 Command_Init(const int arg_count, const uint8 *args);

 int32 Command_ReadTOC(const int arg_count, const uint8 *args);
 int32 Command_ReadTOC_Part2(void);

 int32 Command_0x1d(const int arg_count, const uint8 *args);
};

}

#endif
