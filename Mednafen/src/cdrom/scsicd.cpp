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

#include "../mednafen.h"
#include <math.h>
#include <trio/trio.h>
#include "scsicd.h"
#include "cdromif.h"

static uint32 CD_DATA_TRANSFER_RATE;
static uint32 System_Clock;
static int32 CDDADivAcc;
static void (*CDIRQCallback)(int);
static void (*CDStuffSubchannels)(uint8, int);
static Blip_Buffer *sbuf[2];
static int WhichSystem;

enum
{
 QMode_Zero = 0,
 QMode_Time = 1,
 QMode_MCN = 2, // Media Catalog Number
 QMode_ISRC = 3 // International Standard Recording Code
};

class SimpleFIFO
{
 public:

 // Constructor
 SimpleFIFO(uint32 the_size) // Size should be a power of 2!
 {
  ptr = new uint8[the_size];
  size = the_size;
 }

 // Copy constructor
 SimpleFIFO(const SimpleFIFO &cfifo)
 {
  size = cfifo.size;
  read_pos = cfifo.read_pos;
  write_pos = cfifo.write_pos;
  in_count = cfifo.in_count;  

  ptr = new uint8[cfifo.size];
  memcpy(ptr, cfifo.ptr, cfifo.size);
 }

 // Destructor
 ~SimpleFIFO()
 {
  delete[] ptr;
 }

 ALWAYS_INLINE uint32 CanRead(void)
 {
  return(in_count);
 }

 uint32 CanWrite(void)
 {
  return(size - in_count);
 }

 ALWAYS_INLINE uint8 ReadByte(void)
 {
  uint8 ret;

  assert(in_count > 0);

  ret = ptr[read_pos];
  read_pos = (read_pos + 1) & (size - 1);
  in_count--;

  return(ret);
 }

 // This could probably be optimized.
 void Write(const uint8 *happy_data, uint32 happy_count)
 {
  assert(CanWrite() >= happy_count);

  while(happy_count)
  {
   ptr[write_pos] = *happy_data;

   write_pos = (write_pos + 1) & (size - 1);
   in_count++;
   happy_data++;
   happy_count--;
  }  
 }

 void Flush(void)
 {
  read_pos = 0;
  write_pos = 0;
  in_count = 0;
 }

 //private:
 uint8 *ptr;
 uint32 size;
 uint32 read_pos; // Read position
 uint32 write_pos; // Write position
 uint32 in_count; // Number of bytes in the FIFO
};

typedef struct
{
 // If we're not selected, we don't do anything...
 // Except, on the PC-FX at least, the SCSI CDROM drive responds to RST even if it isn't selected.
 bool device_selected;

 bool last_RST_signal;

 // The pending message to send(in the message phase)
 uint8 message_pending;

 bool status_sent, message_sent;

 // Pending error codes
 uint8 key_pending, asc_pending, ascq_pending, fru_pending;

 uint8 command_buffer[256];
 uint8 command_buffer_pos;
 uint8 command_size_left;

 // FALSE if not all pending data is in the FIFO, TRUE if it is.
 // Used for multiple sector CD reads.
 bool data_transfer_done;

 // To target(the cd unit), for "MODE SELECT" mainly
 uint8 data_out[8192];
 uint32 data_out_pos;
 uint32 data_out_size;

 uint32 lastts;

 bool IsInserted;

 uint8 SubQBuf[4][0xC];		// One for each of the 4 most recent q-Modes.
 uint8 SubQBuf_Last[0xC];	// The most recent q subchannel data, regardless of q-mode.

 uint8 SubPWBuf[96];

} scsicd_t;

void MakeSense(uint8 * target, uint8 key, uint8 asc, uint8 ascq, uint8 fru)
{
 memset(target, 0, 18);

 target[0] = 0x70;		// Current errors and sense data is not SCSI compliant
 target[2] = key;
 target[12] = asc;		// Additional Sense Code
 target[13] = ascq;		// Additional Sense Code Qualifier
 target[14] = fru;		// Field Replaceable Unit code
}

scsicd_t cd;
scsicd_bus_t cd_bus;

static SimpleFIFO *din = NULL;

typedef Blip_Synth < /*64*/blip_good_quality, 1 > CDSynth;
static void (*SCSILog)(const char *, const char *format, ...);

static uint8 PlayMode;
static CDSynth CDDASynth[2];
static int16 last_sample[2];
static int16 CDDASectorBuffer[1176];
static uint32 CDDAReadPos;

enum
{
 CDDASTATUS_PAUSED = -1,
 CDDASTATUS_STOPPED = 0,
 CDDASTATUS_PLAYING = 1,
 CDDASTATUS_SCANNING = 2,
};

static int8 CDDAStatus;
static uint8 ScanMode;
static int32 CDDADiv;
static int CDDATimeDiv;

static uint32 read_sec_start;
static uint32 read_sec;
static uint32 read_sec_end;

static uint32 scan_sec_end;


static int32 CDReadTimer;
static uint32 SectorAddr;
static uint32 SectorCount;

static bool PhaseChange = 0;
static uint32 next_time;

void SCSICD_Power(void)
{
 memset(&cd, 0, sizeof(scsicd_t));
 memset(&cd_bus, 0, sizeof(scsicd_bus_t));

 din->Flush();

 cd.IsInserted = TRUE;

 CDDADivAcc = (int64)System_Clock * 65536 / 44100;
 CDReadTimer = 0;

 SectorAddr = SectorCount = 0;
 read_sec_start = read_sec = 0;
 read_sec_end = ~0;

 PlayMode = 0;
 CDDAReadPos = 0;
 CDDAStatus = CDDASTATUS_STOPPED;
 CDDADiv = 0;

 ScanMode = 0;
 scan_sec_end = 0;
}


void SCSICD_SetDB(uint8 data)
{
 cd_bus.DB = data;
}

void SCSICD_SetACK(bool set)
{
 cd_bus.kingACK = set;
}

void SCSICD_SetSEL(bool set)
{
 cd_bus.kingSEL = set;
}

void SCSICD_SetRST(bool set)
{
 cd_bus.kingRST = set;
}

void SCSICD_SetATN(bool set)
{
 cd_bus.kingATN = set;
}

static void GenSubQFromSubPW(void)
{
 uint8 SubQBuf[0xC];

 memset(SubQBuf, 0, 0xC);

 for(int i = 0; i < 96; i++)
  SubQBuf[i >> 3] |= ((cd.SubPWBuf[i] & 0x40) >> 6) << (7 - (i & 7));

 if(!CDIF_CheckSubQChecksum(SubQBuf))
 {
  puts("SubQ checksum error!");
 }
 else
 {
  memcpy(cd.SubQBuf_Last, SubQBuf, 0xC);

  uint8 adr = SubQBuf[0] & 0xF;

  if(adr != 1) return;

  if(adr <= 0x3)
   memcpy(cd.SubQBuf[adr], SubQBuf, 0xC);
 }
}


#define STATUS_GOOD		0
#define STATUS_CHECK_CONDITION	1
#define STATUS_CONDITION_MET	2
#define STATUS_BUSY		4
#define STATUS_INTERMEDIATE	8

#define SENSEKEY_NO_SENSE		0
#define SENSEKEY_RECOVERED_ERROR	1
#define SENSEKEY_NOT_READY		2
#define SENSEKEY_MEDIUM_ERROR		3
#define SENSEKEY_HARDWARE_ERROR		4
#define SENSEKEY_ILLEGAL_REQUEST	5
#define SENSEKEY_UNIT_ATTENTION		6
#define SENSEKEY_DATA_PROTECT		7

#define ASC_MEDIUM_NOT_PRESENT		0x3A
#define ASC_INVALID_command_OPERATION_CODE   0x20
#define ASC_INVALID_FIELD_IN_CDB	0x24

static uint8 CDIFFormat_To_DMF(CDIF_Track_Format format)
{
 switch (format)
 {
  default:
   return (0xFF);
  case CDIF_FORMAT_AUDIO:
   return (0x00);
  case CDIF_FORMAT_MODE1:
   return (0x01);
  case CDIF_FORMAT_MODE2:
   return (0x02);
 }
}

static uint8 CDIFFORMAT_To_QSCF(CDIF_Track_Format format)	// Q sub-channel control field
{
 switch (format)
 {
  case CDIF_FORMAT_AUDIO:
   return (0x00);
  default:
   return (0x04);
 }
}

static void SendStatusAndMessage(uint8 status, uint8 message)
{
 // This should never ever happen, but that doesn't mean it won't. ;)
 if(din->CanRead())
 {
  printf("BUG: %d bytes still in SCSI CD FIFO\n", din->CanRead());
  din->Flush();
 }

 cd_bus.CD = TRUE;
 cd_bus.MSG = FALSE;
 cd_bus.IO = TRUE;
 cd_bus.REQ = TRUE;

 cd.message_pending = message;

 cd.status_sent = FALSE;
 cd.message_sent = FALSE;

 if(WhichSystem == SCSICD_PCE)
 {
  if(status == STATUS_GOOD || status == STATUS_CONDITION_MET)
   cd_bus.DB = 0x00;
  else
   cd_bus.DB = 0x01;
 }
 else
  cd_bus.DB = status << 1;

 PhaseChange = TRUE;
}

bool SCSICD_IsInserted(void)
{
 return (cd.IsInserted);
}

bool SCSICD_EjectVirtual(void)
{
 if(cd.IsInserted)
 {
  cd.IsInserted = 0;
  return (1);
 }
 return (0);
}

bool SCSICD_InsertVirtual(void)
{
 if(!cd.IsInserted)
 {
  cd.IsInserted = 1;
  return (1);
 }
 return (0);
}


static void lba2msf(uint32 lba, uint8 * m, uint8 * s, uint8 * f)
{
 *m = lba / 75 / 60;
 *s = (lba - *m * 75 * 60) / 75;
 *f = lba - (*m * 75 * 60) - (*s * 75);
}

static void DoMODESELECT6(void)
{
 if(cd.command_buffer[4])
 {
  cd.data_out_pos = 0;
  cd.data_out_size = cd.command_buffer[4];
  //printf("Switch to DATA OUT phase, len: %d\n", cd.data_out_size);
  cd_bus.CD = FALSE;
  cd_bus.IO = FALSE;
  cd_bus.MSG = FALSE;
  cd_bus.REQ = TRUE;
  PhaseChange = TRUE;
 }
 else
  SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoMODESENSE6(void)
{
 unsigned int PC = (cd.command_buffer[2] >> 6) & 0x3;
 unsigned int PageCode = cd.command_buffer[2] & 0x3F;
 bool DBD = cd.command_buffer[1] & 0x08;
 int AllocSize = cd.command_buffer[4];
 uint8 data_in[8192];


 printf("Mode sense 6: %02x %d %d %d\n", PageCode, PC, DBD, AllocSize);

 switch (PageCode)
 {
  default:
   exit(1);
  case 0x0E:			// Audio control page
   if(AllocSize > 16)
    AllocSize = 16;
   memset(data_in, 0, 16);

   data_in[0] = 0x0E;	// Page code
   data_in[1] = 0x0E;	// Page length(kinky coincidence!)
   data_in[2] = 0x00;

   //data_in[5] = 0x80
   data_in[8] = 0x01;	// Output port 0 channel selection
   data_in[9] = 0xFF;	// Outport port 0 volume
   data_in[10] = 0x02;	// Output port 1 channel selection
   data_in[11] = 0xFF;	// Outport port 1 volume
   break;
 }

 din->Write(data_in, AllocSize);

 cd.data_transfer_done = TRUE;
 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 PhaseChange = TRUE;

 //exit(1);
}

static void DoSTARTSTOPUNIT6(void)
{
 bool Immed = cd.command_buffer[1] & 0x01;
 bool LoEj = cd.command_buffer[4] & 0x02;
 bool Start = cd.command_buffer[4] & 0x01;

 //printf("Do start stop unit 6: %d %d %d\n", Immed, LoEj, Start);
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoREZEROUNIT(void)
{
 printf("Rezero Unit: %02x\n", cd.command_buffer[5]);
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

// This data was originally taken from a PC-FXGA software loader, but
// while it was mostly correct(maybe it is correct for the FXGA, but not for the PC-FX?),
// it was 3 bytes too long, and the last real byte was 0x45 instead of 0x20.
// TODO:  Investigate this discrepancy by testing an FXGA with the official loader software.
static const uint8 InqData[0x24] = 
{
 // Standard
 0x05, 0x80, 0x02, 0x00,

 // Additional Length
 0x1F,

 // Vendor Specific
 0x00, 0x00, 0x00, 0x4E, 0x45, 0x43, 0x20, 0x20, 
 0x20, 0x20, 0x20, 0x43, 0x44, 0x2D, 0x52, 0x4F, 
 0x4D, 0x20, 0x44, 0x52, 0x49, 0x56, 0x45, 0x3A, 
 0x46, 0x58, 0x20, 0x31, 0x2E, 0x30, 0x20
};

static void DoINQUIRY(void)
{
 unsigned int AllocSize = cd.command_buffer[4];

 if(!AllocSize)
 {
  cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
  cd.asc_pending = 0x20;
  cd.ascq_pending = 0x00;
  cd.fru_pending = 0x00;
  SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
  return;
 }

 //printf("Inquiry: %02x %02x %02x %02x %02x\n", cd.command_buffer[1], cd.command_buffer[2], cd.command_buffer[3], cd.command_buffer[4], cd.command_buffer[5]);

 din->Write(InqData, (AllocSize > sizeof(InqData)) ? sizeof(InqData) : AllocSize);

 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 cd.data_transfer_done = TRUE;
 PhaseChange = TRUE;
}

static void DoEJECT(void)
{
 puts("Eject!");
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoREQUESTSENSE(void)
{
 uint8 data_in[8192];

 MakeSense(data_in, cd.key_pending, cd.asc_pending, cd.ascq_pending, cd.fru_pending);

 din->Write(data_in, 18);

 cd.key_pending = 0;
 cd.asc_pending = 0;
 cd.ascq_pending = 0;
 cd.fru_pending = 0;

 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 cd.data_transfer_done = TRUE;
 PhaseChange = TRUE;
}

static void DoGETDIRINFO(void)
{
 uint8 data_in[8192];
 uint32 data_in_size = 0;

 switch (cd.command_buffer[1])
 {
  default:
   printf("DOHDOH: %02x\n", cd.command_buffer[1]);
  case 0x0:
   data_in[0] = INT_TO_BCD(CDIF_GetFirstTrack());
   data_in[1] = INT_TO_BCD(CDIF_GetLastTrack());
   data_in_size = 2;
   break;
  case 0x1:
   {
    uint8 m, s, f;

    int32 sector_count = CDIF_GetSectorCountLBA();

    lba2msf(sector_count + 150, &m, &s, &f);

    data_in[0] = INT_TO_BCD(m);
    data_in[1] = INT_TO_BCD(s);
    data_in[2] = INT_TO_BCD(f);
    data_in_size = 3;
   }
   break;
  case 0x2:
   {
    int m, s, f;
    int track = BCD_TO_INT(cd.command_buffer[2]);

    if(!track)
     track = 1;
    else if(cd.command_buffer[2] == 0xAA)
    {
     track = CDIF_GetLastTrack() + 1;
    }
    else if(track > 99)
    {
     cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
     cd.asc_pending = ASC_INVALID_FIELD_IN_CDB;
     cd.ascq_pending = 0x00;
     cd.fru_pending = 0x00;
     SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
     return;
    }

    CDIF_Track_Format format;
    CDIF_GetTrackStartPositionMSF(track, m, s, f);
    CDIF_GetTrackFormat(track, format);

    //printf(" Track: %d %02x %02x %02x\n", track, m, s, f);

    data_in[0] = INT_TO_BCD(m);
    data_in[1] = INT_TO_BCD(s);
    data_in[2] = INT_TO_BCD(f);
    data_in[3] = (format == CDIF_FORMAT_AUDIO) ? 0x00 : 0x04;
    data_in_size = 4;
   }
   break;
 }

 din->Write(data_in, data_in_size);

 cd.data_transfer_done = TRUE;
 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 PhaseChange = TRUE;
}

static void DoREADTOC(void)
{
 uint8 data_in[8192];
 int FirstTrack = CDIF_GetFirstTrack();
 int LastTrack = CDIF_GetLastTrack();
 int StartingTrack = cd.command_buffer[6];
 unsigned int AllocSize = (cd.command_buffer[7] << 8) | cd.command_buffer[8];
 unsigned int RealSize = 0;
 bool WantInMSF = cd.command_buffer[1] & 0x2;

 //printf("READ TOC: %d %02x %d %d\n", StartingTrack, cd.command_buffer[6], AllocSize, WantInMSF);

 if(!StartingTrack)
  StartingTrack = 1;
 else if(cd.command_buffer[6] == 0xAA || StartingTrack > 99)
 {
  StartingTrack = LastTrack + 1;
 }
 //else if(StartingTrack > 100) // 100 is the hidden leadout track, so test for >100, not >99!
 //{
 // puts("ERROR");
 // cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
 // cd.asc_pending = ASC_INVALID_FIELD_IN_CDB;
 // cd.ascq_pending = 0x00;
 // cd.fru_pending = 0x00;
 // SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
 // return;
 // }

 data_in[2] = INT_TO_BCD(FirstTrack);
 data_in[3] = INT_TO_BCD(LastTrack);

 RealSize += 4;

 // Read leadout track too LastTrack + 1 ???
 for(int track = StartingTrack; track <= (LastTrack + 1) && (RealSize + 8) <= AllocSize; track++)
 {
  uint8 *subptr = &data_in[RealSize];

  int minutes;
  int seconds;
  int frames;
  CDIF_Track_Format format;
  CDIF_GetTrackStartPositionMSF(track, minutes, seconds, frames);
  CDIF_GetTrackFormat(track, format);

  uint32 lba = frames + 75 * seconds + 75 * 60 * minutes;

  //printf("%d, %d, %02x\n", track, WantInMSF, CDIFFORMAT_To_QSCF(format) | 0x10);
  subptr[0] = 0;
  subptr[1] = CDIFFORMAT_To_QSCF(format) | 0x10;	// | 0x10 = Q sub channel encodes current position info
  if(track == (LastTrack + 1))
   subptr[2] = 0xAA;
  else
   subptr[2] = INT_TO_BCD(track);
  subptr[3] = 0;

  //printf("LA: %d\n", lba);
  if(WantInMSF)
  {
   subptr[4] = 0;
   subptr[5] = minutes;		// Min
   subptr[6] = seconds;		// Sec
   subptr[7] = frames;		// Frames
  }
  else
  {
   lba -= 150;
   subptr[4] = lba >> 24;
   subptr[5] = lba >> 16;
   subptr[6] = lba >> 8;
   subptr[7] = lba >> 0;
  }
  RealSize += 8;
 }

 if(RealSize > AllocSize)
  RealSize = AllocSize;

 // Record the length of the response, minus the size of the TOC data length field(2 bytes)
 data_in[0] = (RealSize - 2) >> 8;
 data_in[1] = (RealSize - 2) >> 0;

 din->Write(data_in, RealSize);

 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 cd.data_transfer_done = TRUE;
 PhaseChange = TRUE;
}

static void DoREADHEADER(void)
{
 uint8 data_in[8192];
 bool WantInMSF = cd.command_buffer[1] & 0x2;
 uint32 HeaderLBA = (cd.command_buffer[2] << 24) | (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
 int AllocSize = (cd.command_buffer[7] << 8) | cd.command_buffer[8];
 int Track = CDIF_FindTrackByLBA(HeaderLBA);

 int minutes;
 int seconds;
 int frames;
 CDIF_Track_Format format;

 CDIF_GetTrackStartPositionMSF(Track, minutes, seconds, frames);
 CDIF_GetTrackFormat(Track, format);

 uint32 lba = frames + 75 * seconds + 75 * 60 * minutes;

 //printf("LBA: %d, Track: %d\n", HeaderLBA, Track);

 data_in[0] = CDIFFormat_To_DMF(format);
 data_in[1] = 0;
 data_in[2] = 0;
 data_in[3] = 0;

 if(WantInMSF)
 {
  data_in[4] = 0;
  data_in[5] = minutes;	// Min
  data_in[6] = seconds;	// Sec
  data_in[7] = frames;	// Frames
 }
 else
 {
  lba -= 150;
  data_in[4] = lba >> 24;
  data_in[5] = lba >> 16;
  data_in[6] = lba >> 8;
  data_in[7] = lba >> 0;
 }

 din->Write(data_in, 8);

 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 cd.data_transfer_done = TRUE;

 CDDAStatus = CDDASTATUS_STOPPED;
 PhaseChange = TRUE;
}

static void DoSAPSP(void)
{
 uint32 new_read_sec_start;

 //            printf("Set audio start: %02x %02x %02x %02x %02x %02x %02x\n", cd.command_buffer[9], cd.command_buffer[1], cd.command_buffer[2], cd.command_buffer[3], cd.command_buffer[4], cd.command_buffer[5], cd.command_buffer[6]);
 switch (cd.command_buffer[9] & 0xc0)
 {
  default:  printf("Unknown SAPSP 9: %02x\n", cd.command_buffer[9]);
  case 0x00:
   new_read_sec_start = (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
   break;
  case 0x40:
   new_read_sec_start = BCD_TO_INT(cd.command_buffer[4]) + 75 * (BCD_TO_INT(cd.command_buffer[3]) + 60 * BCD_TO_INT(cd.command_buffer[2]));
   new_read_sec_start -= 150;
   break;
  case 0x80:
   new_read_sec_start = CDIF_GetTrackStartPositionLBA(BCD_TO_INT(cd.command_buffer[2]));
   break;
 }

 read_sec = read_sec_start = new_read_sec_start;
 CDDAReadPos = 588;
 CDDAStatus = CDDASTATUS_PAUSED;
 PlayMode = cd.command_buffer[1];

 if(PlayMode)
 {
  CDDAStatus = CDDASTATUS_PLAYING;
 }

 SendStatusAndMessage(STATUS_GOOD, 0x00);
 CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
}

static void DoSAPEP(void)
{
 //                printf("Set audio end: %02x %02x %02x %02x %02x\n", cd.command_buffer[9], cd.command_buffer[1], cd.command_buffer[2], cd.command_buffer[3], cd.command_buffer[4]);
 uint32 new_read_sec_end;

 switch (cd.command_buffer[9] & 0xc0)
 {
  default: printf("Unknown SAPEP 9: %02x\n", cd.command_buffer[9]);

  case 0x00:
   new_read_sec_end = (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
   break;
  case 0x40:
   new_read_sec_end = BCD_TO_INT(cd.command_buffer[4]) + 75 * (BCD_TO_INT(cd.command_buffer[3]) + 60 * BCD_TO_INT(cd.command_buffer[2]));
   new_read_sec_end -= 150;
   break;
  case 0x80:
   new_read_sec_end = CDIF_GetTrackStartPositionLBA(BCD_TO_INT(cd.command_buffer[2]));
   break;
 }

 PlayMode = cd.command_buffer[1];
 if(PlayMode)
  CDDAStatus = CDDASTATUS_PLAYING;
 else
  CDDAStatus = CDDASTATUS_PAUSED;	//	paused or stopped?

 read_sec_end = new_read_sec_end;

 SendStatusAndMessage(STATUS_GOOD, 0x00);
 //CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
}

static void DoSST(void)		// Command 0xDB, Set Stop Time
{
 printf("Set stop time: ");
 for(int i = 0; i < 10; i++)
  printf("%02x ", cd.command_buffer[i]);
 printf("\n");
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoPATI(void)
{
 // "Boundary Gate" uses this command.
 // Why was this command removed from the MMC draft(I think around version 4)?
 // Are StartIndex and EndIndex used on the PC-FX?
 // Is the playback end point when track==EndTrack && index==EndIndex?
 // That would seem to contradict a literal interpretation of this description in the MMC drafts
 // that still have this command.
 int StartTrack = cd.command_buffer[4];
 int EndTrack = cd.command_buffer[7];
 int StartIndex = cd.command_buffer[5];
 int EndIndex = cd.command_buffer[8];

 CDIF_Track_Format start_format;

 if(!CDIF_GetTrackFormat(StartTrack, start_format) || start_format != CDIF_FORMAT_AUDIO)
 {
  cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
  cd.asc_pending = 0x00;
  cd.ascq_pending = 0x00;
  cd.fru_pending = 0x00;

  SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
  return;
 }

 read_sec = read_sec_start = CDIF_GetTrackStartPositionLBA(StartTrack);
 read_sec_end = CDIF_GetTrackStartPositionLBA(EndTrack);
 CDDAStatus = CDDASTATUS_PLAYING;
 PlayMode = 3;			// No repeat

 //printf("PATI: %d %d %d  SI: %d, EI: %d\n", StartTrack, EndTrack, CDIF_GetTrackStartPositionLBA(StartTrack), StartIndex, EndIndex);
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoPAUSERESUME(void)
{
 // Pause/resume
 // "It shall not be considered an error to request a pause when a pause is already in effect, 
 // or to request a resume when a play operation is in progress."
 if(!CDDAStatus)		// If CDDA isn't playing/paused, make an error!
 {
  cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
  cd.asc_pending = 0x2c;
  cd.ascq_pending = 0x00;
  cd.fru_pending = 0x00;
  SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
  return;
 }

 if(cd.command_buffer[8] & 1)	// Resume
  CDDAStatus = CDDASTATUS_PLAYING;
 else
  CDDAStatus = CDDASTATUS_PAUSED;

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoREADBase(uint32 sa, uint32 sc)
{
 if(SCSILog)
 {
  int Track = CDIF_FindTrackByLBA(sa);
  uint32 Offset = sa - CDIF_GetTrackStartPositionLBA(Track);
  SCSILog("SCSI", "Read: start=0x%08x(track=%d, offs=0x%08x), cnt=0x%08x", sa, Track, Offset, sc);
 }

 SectorAddr = sa;
 SectorCount = sc;
 if(SectorCount)
 {
  CDReadTimer = (uint64)1 * 2048 * System_Clock / CD_DATA_TRANSFER_RATE;
  PhaseChange = TRUE;
 }
 else
 {
  CDReadTimer = 0;
  SendStatusAndMessage(STATUS_GOOD, 0x00);
 }
 CDDAStatus = CDDASTATUS_STOPPED;
}

static void DoREAD6(void)
{
 uint32 sa = ((cd.command_buffer[1] & 0x1F) << 16) | (cd.command_buffer[2] << 8) | (cd.command_buffer[3] << 0);
 uint32 sc = cd.command_buffer[4];

 DoREADBase(sa, sc);
}

static void DoREAD10(void)
{
 uint32 sa = (cd.command_buffer[2] << 24) | (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
 uint32 sc = (cd.command_buffer[7] << 8) | (cd.command_buffer[8]);

 DoREADBase(sa, sc);
}

static void DoREAD12(void)
{
 uint32 sa = (cd.command_buffer[2] << 24) | (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
 uint32 sc = (cd.command_buffer[6] << 24) | (cd.command_buffer[7] << 16) | (cd.command_buffer[8] << 8) | cd.command_buffer[9];

 DoREADBase(sa, sc);
}

static void DoPREFETCH(void)
{
 uint32 lba = (cd.command_buffer[2] << 24) | (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
 uint32 len = (cd.command_buffer[7] << 8) | cd.command_buffer[8];
 bool link = cd.command_buffer[9] & 0x1;
 bool flag = cd.command_buffer[9] & 0x2;
 bool reladdr = cd.command_buffer[1] & 0x1;
 bool immed = cd.command_buffer[1] & 0x2;

 //printf("Prefetch: %08x %08x %d %d %d %d\n", lba, len, link, flag, reladdr, immed);
 //SendStatusAndMessage(STATUS_GOOD, 0x00);
 SendStatusAndMessage(STATUS_CONDITION_MET, 0x00);
 //exit(1);
}

// SEEK functions are mostly just stubs for now, until(if) we emulate seek delays.
static void DoSEEKBase(uint32 lba)
{
 CDDAStatus = CDDASTATUS_STOPPED;
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoSEEK6(void)
{
 uint32 lba = ((cd.command_buffer[1] & 0x1F) << 16) | (cd.command_buffer[2] << 8) | cd.command_buffer[3];
 //printf("Seek 6(stub): %08x\n", lba);
 DoSEEKBase(lba);
}

static void DoSEEK10(void)
{
 uint32 lba = (cd.command_buffer[2] << 24) | (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
 //printf("Seek 10(stub): %08x\n", lba);
 DoSEEKBase(lba);
}

static void DoREADSUBCHANNEL(void)
{
 uint8 data_in[8192];
 int DataFormat = cd.command_buffer[3];
 int TrackNum = cd.command_buffer[6];
 int AllocSize = (cd.command_buffer[7] << 8) | cd.command_buffer[8];
 bool WantQ = cd.command_buffer[2] & 0x40;
 bool WantMSF = cd.command_buffer[1] & 0x02;

 //printf("42Read SubChannel: %02x %02x %d %d %d\n", DataFormat, TrackNum, AllocSize, WantQ, WantMSF);
 if(!WantQ)
  SendStatusAndMessage(STATUS_GOOD, 0x00);
 else
 {
  switch (DataFormat)
  {
   default:
    printf("42Read SubChannel: %02x %02x %d %d %d\n", DataFormat, TrackNum, AllocSize, WantQ, WantMSF);

    exit(1);

   case 0x01:			// CD-ROM current position
    {
     uint8 *SubQBuf = cd.SubQBuf[QMode_Time];
     data_in[0] = 0;

     // FIXME:  Is this audio status code correct for scanning playback??
     if(CDDAStatus == CDDASTATUS_PLAYING || CDDAStatus == CDDASTATUS_SCANNING)
      data_in[1] = 0x11;	// Audio play operation in progress
     else if(CDDAStatus == CDDASTATUS_PAUSED)
      data_in[1] = 0x12;	// Audio play operation paused
     else
      data_in[1] = 0x13;	// 0x13(audio play operation completed successfully) or 0x15(no current audio status to return)? :(

     // Sub-channel data length
     data_in[2] = 0x00;
     data_in[3] = 0x0C;

     // Sub-channel format code
     data_in[4] = 0x01;

     data_in[5] = ((SubQBuf[0] & 0x0F) << 4) | ((SubQBuf[0] & 0xF0) >> 4); // Control/adr
     data_in[6] = SubQBuf[1]; // Track
     data_in[7] = SubQBuf[2]; // Index

     // Absolute CD-ROM address
     if(WantMSF)
     {
      data_in[8] = 0;
      data_in[9] = BCD_TO_INT(SubQBuf[7]); // M
      data_in[10] = BCD_TO_INT(SubQBuf[8]); // S
      data_in[11] = BCD_TO_INT(SubQBuf[9]); // F
     }
     else
     {
      uint32 tmp_lba = BCD_TO_INT(SubQBuf[7]) * 60 * 75 + BCD_TO_INT(SubQBuf[8]) * 75 + BCD_TO_INT(SubQBuf[9]) - 150;

      data_in[8] = tmp_lba >> 24;
      data_in[9] = tmp_lba >> 16;
      data_in[10] = tmp_lba >> 8;
      data_in[11] = tmp_lba >> 0;
     }

     // Relative CD-ROM address
     if(WantMSF)
     {
      data_in[12] = 0;
      data_in[13] = BCD_TO_INT(SubQBuf[3]); // M
      data_in[14] = BCD_TO_INT(SubQBuf[4]); // S
      data_in[15] = BCD_TO_INT(SubQBuf[5]); // F
     }
     else
     {
      uint32 tmp_lba = BCD_TO_INT(SubQBuf[3]) * 60 * 75 + BCD_TO_INT(SubQBuf[4]) * 75 + BCD_TO_INT(SubQBuf[5]) - 150;

      data_in[12] = tmp_lba >> 24;
      data_in[13] = tmp_lba >> 16;
      data_in[14] = tmp_lba >> 8;
      data_in[15] = tmp_lba >> 0;
     }
     din->Write(data_in, (AllocSize > 16) ? 16 : AllocSize);

     cd.data_transfer_done = TRUE;
     cd_bus.IO = TRUE;
     cd_bus.CD = FALSE;
     PhaseChange = TRUE;
    }
    break;
  }
 }
 //printf("%02x %02x %04x %d\n", DataFormat, TrackNum, AllocSize, WantQ);
 //exit(1);
}

static void DoNECREADSUBQ(void)
{
 uint8 *SubQBuf = cd.SubQBuf[QMode_Time];
 uint8 data_in[8192];

 memset(data_in, 0x00, 10);

 data_in[2] = SubQBuf[1];     // Track
 data_in[3] = SubQBuf[2];     // Index
 data_in[4] = SubQBuf[3];     // M(rel)
 data_in[5] = SubQBuf[4];     // S(rel)
 data_in[6] = SubQBuf[5];     // F(rel)
 data_in[7] = SubQBuf[7];     // M(abs)
 data_in[8] = SubQBuf[8];     // S(abs)
 data_in[9] = SubQBuf[9];     // F(abs)

 if(CDDAStatus == CDDASTATUS_PAUSED)
  data_in[0] = 2;		// Pause
 else if(CDDAStatus == CDDASTATUS_PLAYING || CDDAStatus == CDDASTATUS_SCANNING) // FIXME:  Is this the correct status code for scanning playback?
  data_in[0] = 0;		// Playing
 else
  data_in[0] = 3;		// Stopped

 din->Write(data_in, 10);

 cd.data_transfer_done = TRUE;
 cd_bus.IO = TRUE;
 cd_bus.CD = FALSE;
 PhaseChange = TRUE;
}

static void DoTESTUNITREADY(void)
{
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoNECPAUSE(void)
{
 if(CDDAStatus != CDDASTATUS_STOPPED) // Hmm, should we give an error if it tries to pause and it's already paused?
 {
  CDDAStatus = CDDASTATUS_PAUSED;
  SendStatusAndMessage(STATUS_GOOD, 0x00);
 }
 else // Definitely give an error if it tries to pause when no track is playing!
 {
  cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
  cd.asc_pending = 0x2c;
  cd.ascq_pending = 0x00;
  cd.fru_pending = 0x00;
  SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
 }
}

static void DoNECSCAN(void)
{
 uint32 sector_tmp = 0;

 // 0: 0xD2
 // 1: 0x03 = reverse scan, 0x02 = forward scan
 // 2: End M
 // 3: End S
 // 4: End F

 switch (cd.command_buffer[9] & 0xc0)
 {
  default: puts("Unknown NECSCAN format"); break;
  case 0x00:
   sector_tmp = (cd.command_buffer[3] << 16) | (cd.command_buffer[4] << 8) | cd.command_buffer[5];
   break;
  case 0x40:
   sector_tmp = BCD_TO_INT(cd.command_buffer[4]) + 75 * (BCD_TO_INT(cd.command_buffer[3]) + 60 * BCD_TO_INT(cd.command_buffer[2]));
   sector_tmp -= 150;
   break;
  case 0x80:
   sector_tmp = CDIF_GetTrackStartPositionLBA(BCD_TO_INT(cd.command_buffer[2]));
   break;
 }

 ScanMode = cd.command_buffer[1] & 0x3;

 scan_sec_end = sector_tmp;

 if(CDDAStatus != CDDASTATUS_STOPPED)
 {
  if(ScanMode)
  {
   CDDAStatus = CDDASTATUS_SCANNING;
  }
 }
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoPREVENTALLOWREMOVAL(void)
{
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}


typedef struct
{
 uint8 cmd;
 unsigned int cdb_length;
 void (*func)(void);
 const char *pretty_name;
 const char *format_string;
} SCSICH;

static SCSICH PCFXCommandDefs[] =
{
 { 0x00, 6, DoTESTUNITREADY, "Test Unit Ready" },
 { 0x01, 6, DoREZEROUNIT, "Rezero Unit" },
 { 0x03, 6, DoREQUESTSENSE, "Request Sense" },
 { 0x08, 6, DoREAD6, "Read(6)" },
 { 0x0B, 6, DoSEEK6, "Seek(6)" },
 { 0x12, 6, DoINQUIRY, "Inquiry" },
 { 0x15, 6, DoMODESELECT6, "Mode Select(6)" },
 { 0x1A, 6, DoMODESENSE6, "Mode Sense(6)" },
 { 0x1B, 6, DoSTARTSTOPUNIT6, "Start/Stop Unit" },
 { 0x1E, 16, DoPREVENTALLOWREMOVAL, "Prevent/Allow Media Removal" },
 { 0x28, 10, DoREAD10, "Read(10)" },
 { 0x2B, 10, DoSEEK10, "Seek(10)" },
 { 0x34, 10, DoPREFETCH, "Prefetch" },
 { 0x42, 10, DoREADSUBCHANNEL, "Read Subchannel" },
 { 0x43, 10, DoREADTOC, "Read TOC" },
 { 0x44, 10, DoREADHEADER, "Read Header" },
 { 0x48, 10, DoPATI, "Play Audio Track Index" },
 { 0x4B, 10, DoPAUSERESUME, "Pause/Resume" },

 { 0xA8, 12, DoREAD12, "Read(12)" },

 { 0xD2, 10, DoNECSCAN, "Scan" },
 { 0xD8, 10, DoSAPSP, "Set Audio Playback Start Position" }, // "Audio track search"
 { 0xD9, 10, DoSAPEP, "Set Audio Playback End Position" },   // "Play"
 { 0xDA, 10, DoNECPAUSE, "Pause" },			     // "Still"
 { 0xDB, 10, DoSST, "Set Stop Time" },
 { 0xDC, 10, DoEJECT, "Eject" },
 { 0xDD, 10, DoNECREADSUBQ, "Read Subchannel Q" },
 { 0xDE, 10, DoGETDIRINFO, "Get Dir Info" },

 { 0xFF, 0, NULL, NULL },
};

static SCSICH PCECommandDefs[] = 
{
 { 0x00, 1, DoTESTUNITREADY, "Test Unit Ready" }, // 1 or 6?
 { 0x03, 6, DoREQUESTSENSE, "Request Sense" },
 { 0x08, 6, DoREAD6, "Read(6)" },
 //{ 0x15, 6, DoMODESELECT6, "Mode Select(6)" },
 { 0xD8, 10, DoSAPSP, "Set Audio Playback Start Position" },
 { 0xD9, 10, DoSAPEP, "Set Audio Playback End Position" },
 { 0xDA, 10, DoNECPAUSE, "Pause" },
 { 0xDD, 10, DoNECREADSUBQ, "Read Subchannel Q" },
 { 0xDE, 10, DoGETDIRINFO, "Get Dir Info" },

 { 0xFF, 0, NULL, NULL },
};


static int32 lastts;
void SCSICD_ResetTS(void)
{
 lastts = 0;
}

void SCSICD_GetCDDAValues(int16 &left, int16 &right)
{
 if(CDDAStatus)
 {
  left = CDDASectorBuffer[CDDAReadPos * 2];
  right = CDDASectorBuffer[CDDAReadPos * 2 + 1];
 }
 else
  left = right = 0;
}

static uint8 last_ATN = 0;

uint32 SCSICD_Run(uint32 system_timestamp)
{
 bool before_req = REQ_signal;
 int32 run_time = system_timestamp - lastts;

 lastts = system_timestamp;

 if(CDReadTimer > 0)
 {
  CDReadTimer -= run_time;

  if(CDReadTimer <= 0)
  {
   if(din->CanWrite() < 2048)
   {
    //printf("Carp: %d %d %d\n", din->CanWrite(), SectorCount, CDReadTimer);
    //CDReadTimer = (cd.data_in_size - cd.data_in_pos) * 10;
    
    CDReadTimer += (uint64) 1 * 2048 * System_Clock / CD_DATA_TRANSFER_RATE;

    //CDReadTimer += (uint64) 1 * 128 * System_Clock / CD_DATA_TRANSFER_RATE;
   }
   else
   {
    uint8 tmp_read_buf[2048];

    if(!cd.IsInserted)
    {
     din->Flush();
     cd.data_transfer_done = FALSE;
     cd.key_pending = SENSEKEY_NOT_READY;
     cd.asc_pending = ASC_MEDIUM_NOT_PRESENT;
     cd.ascq_pending = 0x00;
     cd.fru_pending = 0x00;
     SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
    }
    else if(!CDIF_ReadSector(tmp_read_buf, cd.SubPWBuf, SectorAddr, 1))
    {
     cd.data_transfer_done = FALSE;
     cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
     cd.asc_pending = 0x00;
     cd.ascq_pending = 0x00;
     cd.fru_pending = 0x00;
     SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
     puts("Error");
    }
    else
    {
     din->Write(tmp_read_buf, 2048);

     GenSubQFromSubPW();

     CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_READY);

     SectorAddr++;
     SectorCount--;

     cd_bus.IO = TRUE;
     cd_bus.CD = FALSE;

     if(SectorCount)
     {
      cd.data_transfer_done = FALSE;
      CDReadTimer += (uint64) 1 * 2048 * System_Clock / CD_DATA_TRANSFER_RATE;
     }
     else
     {
      cd.data_transfer_done = TRUE;
     }
    }
   }				// end else to if(!CDIF_ReadSector

  }
 }

 if(CDDAStatus == CDDASTATUS_PLAYING || CDDAStatus == CDDASTATUS_SCANNING)
 {
  int16 sample[2];

  CDDADiv -= run_time << 16;

  while(CDDADiv <= 0)
  {
   CDDADiv += CDDADivAcc;

   //MDFN_DispMessage("%d %d %d\n", read_sec_start, read_sec, read_sec_end);

   if(CDDAReadPos == 588)
   {
    if(read_sec == read_sec_end || (CDDAStatus == CDDASTATUS_SCANNING && read_sec == scan_sec_end))
    {
     switch (PlayMode)
     {
      default:
      case 0x03:		// No repeat
       CDDAStatus = CDDASTATUS_STOPPED;
       break;
      case 0x02:		// Interrupt when finished?
       CDDAStatus = CDDASTATUS_STOPPED;
       CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
       break;
      case 0x04:
      case 0x01:		// Repeat play!
       read_sec = read_sec_start;
       break;
     }

     // If CDDA playback is stopped, break out of our while(CDDADiv ...) loop and don't play any more sound!
     if(CDDAStatus == CDDASTATUS_STOPPED)
      break;
    }

    if(!cd.IsInserted)
    {
     CDDAStatus = CDDASTATUS_STOPPED;

     #if 0
     cd.data_transfer_done = FALSE;
     cd.key_pending = SENSEKEY_NOT_READY;
     cd.asc_pending = ASC_MEDIUM_NOT_PRESENT;
     cd.ascq_pending = 0x00;
     cd.fru_pending = 0x00;
     SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
     #endif

     break;
    }


    CDDAReadPos = 0;
    CDIF_ReadAudioSector(CDDASectorBuffer, cd.SubPWBuf, read_sec);
    GenSubQFromSubPW();

    if(CDDAStatus == CDDASTATUS_SCANNING)
    {
     int64 tmp_read_sec = read_sec;

     if(ScanMode & 1)
     {
      tmp_read_sec -= 24;
      if(tmp_read_sec < scan_sec_end)
       tmp_read_sec = scan_sec_end;
     }
     else
     {
      tmp_read_sec += 24;
      if(tmp_read_sec > scan_sec_end)
       tmp_read_sec = scan_sec_end;
     }
     read_sec = tmp_read_sec;
    }
    else
     read_sec++;
   } // End    if(CDDAReadPos == 588)

   sample[0] = CDDASectorBuffer[CDDAReadPos * 2];
   sample[1] = CDDASectorBuffer[CDDAReadPos * 2 + 1];

//   sample[0] = ((rand() & 0xFFFF) - 0x8000) * 3 / 4; // * 2 / 3;
//   sample[1] = ((rand() & 0xFFFF) - 0x8000) * 3 / 4; // * 2 / 3;

   uint32 synthtime = ((system_timestamp + (CDDADiv >> 16))) / CDDATimeDiv;
   if(FSettings.SndRate)
   {
    if(!(CDDAReadPos % 6))
    {
     int subindex = CDDAReadPos / 6 - 2;

     if(subindex >= 0)
      CDStuffSubchannels(cd.SubPWBuf[subindex], subindex);
     else // The system-specific emulation code should handle what value the sync bytes are.
      CDStuffSubchannels(0x00, subindex);
    }

    CDDASynth[0].offset(synthtime, sample[0] - last_sample[0], sbuf[0]);
    CDDASynth[1].offset(synthtime, sample[1] - last_sample[1], sbuf[1]);

    last_sample[0] = sample[0];
    last_sample[1] = sample[1];
   }
   CDDAReadPos++;
  }
 }

 do
 {
  PhaseChange = FALSE;

  if(!SEL_signal && !BSY_signal && cd.device_selected)	// BUS-free mode, release our signals
  {
   //puts("Bus free mode!");
   CDIRQCallback(0x8000 | SCSICD_IRQ_DATA_TRANSFER_DONE);
   cd.device_selected = FALSE;
   cd_bus.CD = FALSE;
   cd_bus.MSG = FALSE;
   cd_bus.IO = FALSE;
   cd_bus.REQ = FALSE;
  }

  if(RST_signal)
  {
   if(!cd.last_RST_signal)
   {
    din->Flush();

    cd.data_out_pos = cd.data_out_size = 0;

    // Reset CDDA playback to normal speed.
    CDDADivAcc = (int64)System_Clock * 65536 / 44100;

    CDDAStatus = CDDASTATUS_STOPPED;
    CDReadTimer = 0;

    cd_bus.BSY = FALSE;
    cd_bus.CD = FALSE;
    cd_bus.MSG = FALSE;
    cd_bus.IO = FALSE;
    cd_bus.REQ = FALSE;
    PhaseChange = TRUE;
   }
   cd.last_RST_signal = RST_signal;
   goto SpoonyBard;
  }
  cd.last_RST_signal = RST_signal;

 if(SEL_signal)
 {
  if(!cd.device_selected)
  {
   if(WhichSystem == SCSICD_PCFX)
   {
    //if(cd_bus.DB == 0x84)
    {
     cd.device_selected = TRUE;

     cd_bus.CD = TRUE;		// Ask for message!
     cd_bus.BSY = TRUE;
     cd_bus.MSG = FALSE;
     cd_bus.IO = FALSE;
     cd_bus.REQ = TRUE;
    }
   }
   else // PCE
   {
    cd.device_selected = TRUE;
    cd_bus.CD = TRUE;               // Ask for message!
    cd_bus.BSY = TRUE;
    cd_bus.MSG = FALSE;
    cd_bus.IO = FALSE;
    cd_bus.REQ = TRUE;
   }
  }
 }
 else if(!cd.device_selected)
 {

 }
 else if(ATN_signal && !last_ATN)
 {
  // If any bits are changing, set PhaseChange
  if(!cd_bus.BSY || !cd_bus.MSG || !cd_bus.REQ || !cd_bus.CD || cd_bus.IO)
   PhaseChange = TRUE;

  cd_bus.BSY = TRUE;
  cd_bus.MSG = TRUE;
  cd_bus.REQ = TRUE;
  cd_bus.CD = TRUE;
  cd_bus.IO = FALSE;

  last_ATN = TRUE;
 }
 else if(!MSG_signal && BSY_signal)	// Data phaseseses
 {
  if(!IO_signal)
  {
   if(CD_signal)		// COMMAND phase
   {
    if(REQ_signal && ACK_signal)	// Data bus is valid nowww
    {
     //printf("Command Phase Byte I->T: %02x, %d\n", cd_bus.DB, cd.command_buffer_pos);
     cd.command_buffer[cd.command_buffer_pos++] = cd_bus.DB;
     cd_bus.REQ = FALSE;
    }

    if(!REQ_signal && !ACK_signal && cd.command_buffer_pos)	// Received at least one byte, what should we do?
    {
     const SCSICH *cmd_info_ptr;

     if(WhichSystem == SCSICD_PCFX)
      cmd_info_ptr = PCFXCommandDefs;
     else
      cmd_info_ptr = PCECommandDefs;

     while(cmd_info_ptr->pretty_name && cmd_info_ptr->cmd != cd.command_buffer[0])
      cmd_info_ptr++;
  
     if(cmd_info_ptr->pretty_name == NULL)	// Command not found!
     {
      cd.key_pending = SENSEKEY_ILLEGAL_REQUEST;
      cd.asc_pending = 0x20;
      cd.ascq_pending = 0x00;
      cd.fru_pending = 0x00;
      SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);

      printf("Bad Command: %02x\n", cd.command_buffer[0]);

      if(SCSILog)
       SCSILog("SCSI", "Bad Command: %02x", cd.command_buffer[0]);

      cd.command_buffer_pos = 0;
     }
     else if(cd.command_buffer_pos == cmd_info_ptr->cdb_length)	// We've received the command, and it's done, sooooo...
     {
      uint8 cmd = cd.command_buffer[0];

      if(SCSILog)
      {
       char log_buffer[1024];
       int lb_pos;

       log_buffer[0] = 0;
       
       lb_pos = trio_snprintf(log_buffer, 1024, "Command: %02x, %s  ", cd.command_buffer[0], cmd_info_ptr->pretty_name);

       for(int i = 0; i < cmd_info_ptr->cdb_length; i++)
        lb_pos += trio_snprintf(log_buffer + lb_pos, 1024 - lb_pos, "%02x ", cd.command_buffer[i]);

       SCSILog("SCSI", "%s", log_buffer);
       puts(log_buffer);
      }

      if(!cd.IsInserted && cmd != 0x03 && cmd != 0x1E)	// Hmm, user shouldn't be able to eject a physical CD while it's open with libcdio, so
       // hopefully there won't be a race condition! >_>
      {
       cd.key_pending = SENSEKEY_NOT_READY;
       cd.asc_pending = ASC_MEDIUM_NOT_PRESENT;
       cd.ascq_pending = 0x00;
       cd.fru_pending = 0x00;
       SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
      }
      else
      {
       cmd_info_ptr->func();
      }

      cd.command_buffer_pos = 0;
     }
     else			// Otherwise, get more data for the command!
      cd_bus.REQ = TRUE;
    }
   }
   else				// DATA OUT Phase, neato!?
   {
    if(REQ_signal && ACK_signal)	// Data bus is valid nowww
    {
     //printf("DATAOUT-SCSIIN: %d %02x\n", cd.data_out_pos, cd_bus.DB);
     cd.data_out[cd.data_out_pos++] = cd_bus.DB;
     cd_bus.REQ = FALSE;
    }
    else if(!REQ_signal && !ACK_signal && cd.data_out_pos)
    {
     if(cd.data_out_pos == cd.data_out_size)
     {
      //printf("Datumama: %02x %02x %02x %02x %02x %02x %02x   %d\n", cd.data_out[0], cd.data_out[1], cd.data_out[2], cd.data_out[3], cd.data_out[4], cd.data_out[5], cd.data_out[6], cd.data_out_size);
      //for(int i = 0; i < cd.data_out_size; i++) printf("%02x\n", cd.data_out[i]);
      //printf("%d\n", cd.data_out_size);
      if(cd.command_buffer[0] == 0x15)
      {
       switch (cd.data_out[4])
       {
	case 0x29:
	 break;			// ??? Miraculum tries to set this mode page
	case 0x2B:
	 {
	  int8 speed = cd.data_out[6];
	  double rate = 44100 + (double)44100 * speed / 100;
	  //printf("Speed: %d %f\n", speed, rate);
	  CDDADivAcc = (int32)((int64)System_Clock * 65536 / rate);
	 }
	 break;
       }
      }
      cd.data_out_pos = 0;
      SendStatusAndMessage(STATUS_GOOD, 0x00);
     }
     else
      cd_bus.REQ = TRUE;
    }
   }
  }
  else				// to if(!IO_signal)
  {
   if(CD_signal)		// STATUS
   {
    if(REQ_signal && ACK_signal)
    {
     cd_bus.REQ = FALSE;
     cd.status_sent = TRUE;
    }

    if(!REQ_signal && !ACK_signal && cd.status_sent)
    {
     // Status sent, so get ready to send the message!
     cd.status_sent = FALSE;
     cd_bus.MSG = TRUE;
     cd_bus.REQ = TRUE;
     cd_bus.DB = cd.message_pending;
    }
   }				// END STATUS
   else
   {				// BEGIN "DATA IN"(us to them!)
    //puts("Choo");

    if(!REQ_signal && !ACK_signal)
    {
     if(din->CanRead() == 0)	// aaand we're done!
     {
      CDIRQCallback(0x8000 | SCSICD_IRQ_DATA_TRANSFER_READY);

      if(cd.data_transfer_done)
      {
       SendStatusAndMessage(STATUS_GOOD, 0x00);
       cd.data_transfer_done = FALSE;
       CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
      }
      else
      {
       //SendStatus(STATUS_GOOD);
      }
     }
     else
     {
      cd_bus.DB = din->ReadByte();
      cd_bus.REQ = TRUE;
     }
    }
    if(REQ_signal && ACK_signal)
    {
     cd_bus.REQ = FALSE;
    }
   }

  }
 }
 else if(MSG_signal && BSY_signal)	// Message phases
 {
  if(!IO_signal)		// MESSAGE OUT
  {
   if(REQ_signal && ACK_signal)
   {
    cd_bus.REQ = FALSE;

    // Battle Heat doesn't bother to set the data bus when it wants an abort,
    // so it's somewhat random? o_O
    //if(cd_bus.DB == 0x6)		// ABORT message!
    if(1)
    {
     din->Flush();
     cd.data_out_pos = cd.data_out_size = 0;

     cd_bus.BSY = FALSE;
     cd_bus.CD = FALSE;
     cd_bus.MSG = FALSE;
     cd_bus.IO = FALSE;
     CDReadTimer = 0;
     CDDAStatus = CDDASTATUS_STOPPED;
     PhaseChange = TRUE;
    }
    else
     printf("Message to target: %02x\n", cd_bus.DB);

   }
  }
  else				// MESSAGE IN
  {
   if(REQ_signal && ACK_signal)
   {
    cd_bus.REQ = FALSE;
    cd.message_sent = TRUE;
   }

   if(!REQ_signal && !ACK_signal && cd.message_sent)
   {
    cd.message_sent = FALSE;
    cd_bus.BSY = FALSE;
    PhaseChange = TRUE;
   }
  }
 }
 } while(PhaseChange);

 SpoonyBard:

 next_time = 0x7fffffff;

 if(CDReadTimer > 0 && CDReadTimer < next_time)
  next_time = CDReadTimer;

 if(CDDAStatus == CDDASTATUS_PLAYING || CDDAStatus == CDDASTATUS_SCANNING)
 {
  int32 cdda_div_sexytime = (CDDADiv + 0xFFFF) >> 16;
  if(cdda_div_sexytime > 0 && cdda_div_sexytime < next_time)
   next_time = cdda_div_sexytime;
 }

 if(REQ_signal && !before_req)
  CDIRQCallback(SCSICD_IRQ_MAGICAL_REQ);

 last_ATN = ATN_signal;

 return(next_time);
}

void SCSICD_SetLog(void (*logfunc)(const char *, const char *, ...))
{
 SCSILog = logfunc;
}
#if 0
static void kaiser_window( double* io, int count, double beta )
{
        int const accuracy = 20; //12;

        double* end = io + count;

        double beta2    = beta * beta * (double) -0.25;
        double to_fract = beta2 / ((double) count * count);
        double i        = 0;
        double rescale;
        for ( ; io < end; ++io, i += 1 )
        {
                double x = i * i * to_fract - beta2;
                double u = x;
                double k = x + 1;

                double n = 2;
                do
                {
                        u *= x / (n * n);
                        n += 1;
                        k += u;
                }
                while ( k <= u * (1 << accuracy) );

                if ( !i )
                        rescale = 1 / k; // otherwise values get large

                *io *= k * rescale;
        }
}

static void gen_sinc( double* out, int size, double cutoff, double kaiser )
{
        assert( size % 2 == 0 ); // size must be enev

        int const half_size = size / 2;
        double* const mid = &out [half_size];

        // Generate right half of sinc
        for ( int i = 0; i < half_size; i++ )
        {
                double angle = (i * 2 + 1) * (M_PI / 2);
                mid [i] = sin( angle * cutoff ) / angle;
        }

        kaiser_window( mid, half_size, kaiser );

        // Mirror for left half
        for ( int i = 0; i < half_size; i++ )
                out [i] = mid [half_size - 1 - i];
}

static void normalize( double* io, int size, double gain = 1.0 )
{
        double sum = 0;
        for ( int i = 0; i < size; i++ )
                sum += io [i];

        double scale = gain / sum;
        for ( int i = 0; i < size; i++ )
                io [i] *= scale;
}

struct my_kernel : blip_eq_t
{
        double cutoff;

        my_kernel( double cutoff ) : cutoff( cutoff )
        {
        }

        void generate( double* out, int count ) const
        {
                double tmp_buf[count * 2];
                double moo = 0;

		printf("%f %f\n", (double)cutoff, (double)oversample);

                gen_sinc(tmp_buf, count * 2, cutoff / oversample * 2, 10);
                normalize(tmp_buf, count * 2, 1); //189216615);

                for(int i = 0; i < count; i++)
                {
                 out[i] = tmp_buf[count + i];
                 moo += out[i];
                }
                printf("Moo: %f\n", moo);
        }
};
#endif


void SCSICD_Init(int type, int cdda_time_div, Blip_Buffer *leftbuf, Blip_Buffer *rightbuf, uint32 TransferRate, uint32 SystemClock, void (*IRQFunc)(int), void (*SSCFunc)(uint8, int))
{
 SCSILog = NULL;

 if(type == SCSICD_PCFX)
  din = new SimpleFIFO(4096);
 else
  din = new SimpleFIFO(2048); //8192); //1024); /2048);

 WhichSystem = type;
 CDDATimeDiv = cdda_time_div;

 for(int i = 0; i < 2; i++)
 {
  //double meow = (((double)22050 / 48000) - (6.4 / 64 / 2));
  //my_kernel eq(meow);

  CDDASynth[i].volume(1.0f / 65536);
  //CDDASynth[i].treble_eq(eq);
 }

 sbuf[0] = leftbuf;
 sbuf[1] = rightbuf;

 CD_DATA_TRANSFER_RATE = TransferRate;
 System_Clock = SystemClock;
 CDIRQCallback = IRQFunc;
 CDStuffSubchannels = SSCFunc;
}

void SCSICD_SetCDDAVolume(double left, double right)
{
 // CDDASynth[*].volume will generate an exception if a too small number is passed(less than 1/8192), but we'll check for < 1024
 // to be on the safe side.
 //printf("Volume: %f %f\n", left, right);

 if(left < (1.0f / 1024))
  left = 0;
 if(right < (1.0f / 1024))
  right = 0;

 CDDASynth[0].volume(left / 65536);
 CDDASynth[1].volume(right / 65536);
}

int SCSICD_StateAction(StateMem * sm, int load, int data_only, const char *sname)
{
 SFORMAT StateRegs[] = 
 {
  SFVARN(cd_bus.DB, "DB"),
  SFVARN(cd_bus.BSY, "BSY"),
  SFVARN(cd_bus.MSG, "MSG"),
  SFVARN(cd_bus.CD, "CD"),
  SFVARN(cd_bus.REQ, "REQ"),
  SFVARN(cd_bus.IO, "IO"),

  SFVARN(cd_bus.kingACK, "kingACK"),
  SFVARN(cd_bus.kingRST, "kingRST"),
  SFVARN(cd_bus.kingSEL, "kingSEL"),
  SFVARN(cd_bus.kingATN, "kingATN"),

  SFVARN(cd.last_RST_signal, "last_RST"),
  SFVARN(cd.message_pending, "message_pending"),
  SFVARN(cd.status_sent, "status_sent"),
  SFVARN(cd.message_sent, "message_sent"),
  SFVARN(cd.key_pending, "key_pending"),
  SFVARN(cd.asc_pending, "asc_pending"),
  SFVARN(cd.ascq_pending, "ascq_pending"),
  SFVARN(cd.fru_pending, "fru_pending"),
  SFVARN(cd.device_selected, "device_selected"),

  SFARRAYN(cd.command_buffer, 256, "command_buffer"),
  SFVARN(cd.command_buffer_pos, "command_buffer_pos"),
  SFVARN(cd.command_size_left, "command_size_left"),

  // Don't save the FIFO's write position, it will be reconstructed from read_pos and in_count
  SFARRAYN(din->ptr, din->size, "din_fifo"),
  SFVARN(din->read_pos, "din_read_pos"),
  SFVARN(din->in_count, "din_in_count"),
  SFVARN(cd.data_transfer_done, "data_transfer_done"),

  SFARRAYN(cd.data_out, 8192, "data_out"),
  SFVARN(cd.data_out_pos, "data_out_pos"),
  SFVARN(cd.data_out_size, "data_out_size"),

  SFVARN(cd.IsInserted, "IsInserted"),

  SFVAR(PlayMode),
  SFARRAY16(CDDASectorBuffer, 1176),
  SFVAR(CDDAReadPos),
  SFVAR(CDDAStatus),
  SFVAR(CDDADiv),
  SFVAR(read_sec_start),
  SFVAR(read_sec),
  SFVAR(read_sec_end),

  SFVAR(CDReadTimer),
  SFVAR(SectorAddr),
  SFVAR(SectorCount),

  SFVAR(ScanMode),
  SFVAR(scan_sec_end),

  SFVAR(next_time),

  SFARRAYN(&cd.SubQBuf[0][0], sizeof(cd.SubQBuf), "SubQBufs"),
  SFARRAYN(cd.SubQBuf_Last, sizeof(cd.SubQBuf_Last), "SubQBufLast"),
  SFARRAYN(cd.SubPWBuf, sizeof(cd.SubPWBuf), "SubPWBuf"),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, sname);

 if(load)
 {
  din->in_count &= din->size - 1;
  din->read_pos &= din->size - 1;
  din->write_pos = (din->read_pos + din->in_count) & (din->size - 1);
  //printf("%d %d %d\n", din->in_count, din->read_pos, din->write_pos);
 }

 return (ret);
}
