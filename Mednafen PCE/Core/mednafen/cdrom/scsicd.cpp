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
#include "../clamp.h"
#include <math.h>
#include <trio/trio.h>
#include "scsicd.h"
#include "cdromif.h"
#include "SimpleFIFO.h"

#define SCSIDBG(format, ...) { printf("SCSI: " format "\n",  ## __VA_ARGS__); }
//#define SCSIDBG(format, ...) { }

using namespace CDUtility;


static uint32 CD_DATA_TRANSFER_RATE;
static uint32 System_Clock;
static void (*CDIRQCallback)(int);
static void (*CDStuffSubchannels)(uint8, int);
static Blip_Buffer *sbuf[2];
static int WhichSystem;
static CDIF *Cur_CDIF;

// Internal operation to the SCSI CD unit.  Only pass 1 or 0 to these macros!
#define SetIOP(mask, set)	{ cd_bus.signals &= ~mask; if(set) cd_bus.signals |= mask; }

#define SetBSY(set)		SetIOP(SCSICD_BSY_mask, set)
#define SetIO(set)              SetIOP(SCSICD_IO_mask, set)
#define SetCD(set)              SetIOP(SCSICD_CD_mask, set)
#define SetMSG(set)             SetIOP(SCSICD_MSG_mask, set)

static INLINE void SetREQ(bool set)
{
 if(set && !REQ_signal)
  CDIRQCallback(SCSICD_IRQ_MAGICAL_REQ);

 SetIOP(SCSICD_REQ_mask, set);
}

#define SetkingACK(set)		SetIOP(SCSICD_kingACK_mask, set)
#define SetkingRST(set)         SetIOP(SCSICD_kingRST_mask, set)
#define SetkingSEL(set)         SetIOP(SCSICD_kingSEL_mask, set)
#define SetkingATN(set)         SetIOP(SCSICD_kingATN_mask, set)


enum
{
 QMode_Zero = 0,
 QMode_Time = 1,
 QMode_MCN = 2, // Media Catalog Number
 QMode_ISRC = 3 // International Standard Recording Code
};

typedef struct
{
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

 bool TrayOpen;
 bool DiscChanged;

 uint8 SubQBuf[4][0xC];		// One for each of the 4 most recent q-Modes.
 uint8 SubQBuf_Last[0xC];	// The most recent q subchannel data, regardless of q-mode.

 uint8 SubPWBuf[96];

} scsicd_t;

typedef Blip_Synth < /*64*/blip_good_quality, 1 > CDSynth;

enum
{
 CDDASTATUS_PAUSED = -1,
 CDDASTATUS_STOPPED = 0,
 CDDASTATUS_PLAYING = 1,
 CDDASTATUS_SCANNING = 2,
};

enum
{
 PLAYMODE_SILENT = 0x00,
 PLAYMODE_NORMAL,
 PLAYMODE_INTERRUPT,
 PLAYMODE_LOOP,
};

typedef struct
{
 int32 CDDADivAcc;
 uint32 scan_sec_end;

 uint8 PlayMode;
 CDSynth CDDASynth[2];
 int32 CDDAVolume[2];
 int16 last_sample[2];
 int16 CDDASectorBuffer[1176];
 uint32 CDDAReadPos;

 int8 CDDAStatus;
 uint8 ScanMode;
 int32 CDDADiv;
 int CDDATimeDiv;

 uint8 OutPortChSelect[2];
 uint32 OutPortChSelectCache[2];
 int32 OutPortVolumeCache[2];
} cdda_t;

void MakeSense(uint8 * target, uint8 key, uint8 asc, uint8 ascq, uint8 fru)
{
 memset(target, 0, 18);

 target[0] = 0x70;		// Current errors and sense data is not SCSI compliant
 target[2] = key;
 target[7] = 0x0A;
 target[12] = asc;		// Additional Sense Code
 target[13] = ascq;		// Additional Sense Code Qualifier
 target[14] = fru;		// Field Replaceable Unit code
}

static void (*SCSILog)(const char *, const char *format, ...);
static void InitModePages(void);

static scsicd_timestamp_t lastts;
static int64 monotonic_timestamp;
static int64 pce_lastsapsp_timestamp;

scsicd_t cd;
scsicd_bus_t cd_bus;
static cdda_t cdda;

static SimpleFIFO<uint8> *din = NULL;

static CDUtility::TOC toc;

static uint32 read_sec_start;
static uint32 read_sec;
static uint32 read_sec_end;

static int32 CDReadTimer;
static uint32 SectorAddr;
static uint32 SectorCount;


enum
{
 PHASE_BUS_FREE = 0,
 PHASE_COMMAND,
 PHASE_DATA_IN,
 PHASE_DATA_OUT,
 PHASE_STATUS,
 PHASE_MESSAGE_IN,
 PHASE_MESSAGE_OUT
};
static unsigned int CurrentPhase;
static void ChangePhase(const unsigned int new_phase);


static void FixOPV(void)
{
 for(int port = 0; port < 2; port++)
 {
  cdda.OutPortVolumeCache[port] = cdda.CDDAVolume[port];

  if(cdda.OutPortChSelect[port] & 0x01)
   cdda.OutPortChSelectCache[port] = 0;
  else if(cdda.OutPortChSelect[port] & 0x02)
   cdda.OutPortChSelectCache[port] = 1;
  else
  {
   cdda.OutPortChSelectCache[port] = 0;
   cdda.OutPortVolumeCache[port] = 0;
  }
 }
}

static void VirtualReset(void)
{
 InitModePages();

 din->Flush();

 cdda.CDDADivAcc = (int64)System_Clock * 65536 / 44100;
 CDReadTimer = 0;

 pce_lastsapsp_timestamp = monotonic_timestamp;

 SectorAddr = SectorCount = 0;
 read_sec_start = read_sec = 0;
 read_sec_end = ~0;

 cdda.PlayMode = PLAYMODE_SILENT;
 cdda.CDDAReadPos = 0;
 cdda.CDDAStatus = CDDASTATUS_STOPPED;
 cdda.CDDADiv = 0;

 cdda.ScanMode = 0;
 cdda.scan_sec_end = 0;


 cd.data_out_pos = cd.data_out_size = 0;

 FixOPV();

 ChangePhase(PHASE_BUS_FREE);
}

void SCSICD_Power(scsicd_timestamp_t system_timestamp)
{
 memset(&cd, 0, sizeof(scsicd_t));
 memset(&cd_bus, 0, sizeof(scsicd_bus_t));

 monotonic_timestamp = system_timestamp;

 cd.DiscChanged = false;

 if(Cur_CDIF && !cd.TrayOpen)
  Cur_CDIF->ReadTOC(&toc);

 CurrentPhase = PHASE_BUS_FREE;

 VirtualReset();
}


void SCSICD_SetDB(uint8 data)
{
 cd_bus.DB = data;
 //printf("Set DB: %02x\n", data);
}

void SCSICD_SetACK(bool set)
{
 SetkingACK(set);
 //printf("Set ACK: %d\n", set);
}

void SCSICD_SetSEL(bool set)
{
 SetkingSEL(set);
 //printf("Set SEL: %d\n", set);
}

void SCSICD_SetRST(bool set)
{
 SetkingRST(set);
 //printf("Set RST: %d\n", set);
}

void SCSICD_SetATN(bool set)
{
 SetkingATN(set);
 //printf("Set ATN: %d\n", set);
}

static void GenSubQFromSubPW(void)
{
 uint8 SubQBuf[0xC];

 memset(SubQBuf, 0, 0xC);

 for(int i = 0; i < 96; i++)
  SubQBuf[i >> 3] |= ((cd.SubPWBuf[i] & 0x40) >> 6) << (7 - (i & 7));

 //printf("Real %d/ SubQ %d - ", read_sec, BCD_to_U8(SubQBuf[7]) * 75 * 60 + BCD_to_U8(SubQBuf[8]) * 75 + BCD_to_U8(SubQBuf[9]) - 150);
 // Debug code, remove me.
 //for(int i = 0; i < 0xC; i++)
 // printf("%02x ", SubQBuf[i]);
 //printf("\n");

 if(!subq_check_checksum(SubQBuf))
 {
  SCSIDBG("SubQ checksum error!");
 }
 else
 {
  memcpy(cd.SubQBuf_Last, SubQBuf, 0xC);

  uint8 adr = SubQBuf[0] & 0xF;

  if(adr <= 0x3)
   memcpy(cd.SubQBuf[adr], SubQBuf, 0xC);

  //if(adr == 0x02)
  //for(int i = 0; i < 12; i++)
  // printf("%02x\n", cd.SubQBuf[0x2][i]);
 }
}


#define STATUS_GOOD		0
#define STATUS_CHECK_CONDITION	1
#define STATUS_CONDITION_MET	2
#define STATUS_BUSY		4
#define STATUS_INTERMEDIATE	8

#define SENSEKEY_NO_SENSE		0x0
#define SENSEKEY_NOT_READY		0x2
#define SENSEKEY_MEDIUM_ERROR		0x3
#define SENSEKEY_HARDWARE_ERROR		0x4
#define SENSEKEY_ILLEGAL_REQUEST	0x5
#define SENSEKEY_UNIT_ATTENTION		0x6
#define SENSEKEY_ABORTED_COMMAND	0xB

#define ASC_MEDIUM_NOT_PRESENT		0x3A


// NEC sub-errors(ASC), no ASCQ.
#define NSE_NO_DISC			0x0B		// Used with SENSEKEY_NOT_READY	- This condition occurs when tray is closed with no disc present.
#define NSE_TRAY_OPEN			0x0D		// Used with SENSEKEY_NOT_READY 
#define NSE_SEEK_ERROR			0x15
#define NSE_HEADER_READ_ERROR		0x16		// Used with SENSEKEY_MEDIUM_ERROR
#define NSE_NOT_AUDIO_TRACK		0x1C		// Used with SENSEKEY_MEDIUM_ERROR
#define NSE_NOT_DATA_TRACK		0x1D		// Used with SENSEKEY_MEDIUM_ERROR
#define NSE_INVALID_COMMAND		0x20
#define NSE_INVALID_ADDRESS		0x21
#define NSE_INVALID_PARAMETER		0x22
#define NSE_END_OF_VOLUME		0x25
#define NSE_INVALID_REQUEST_IN_CDB	0x27
#define NSE_DISC_CHANGED		0x28		// Used with SENSEKEY_UNIT_ATTENTION
#define NSE_AUDIO_NOT_PLAYING		0x2C

// ASC, ASCQ pair
#define AP_UNRECOVERED_READ_ERROR	0x11, 0x00
#define AP_LEC_UNCORRECTABLE_ERROR	0x11, 0x05
#define AP_CIRC_UNRECOVERED_ERROR	0x11, 0x06

#define AP_UNKNOWN_MEDIUM_FORMAT	0x30, 0x01
#define AP_INCOMPAT_MEDIUM_FORMAT	0x30, 0x02

static void ChangePhase(const unsigned int new_phase)
{
 //printf("New phase: %d %lld\n", new_phase, monotonic_timestamp);
 switch(new_phase)
 {
  case PHASE_BUS_FREE:
		SetBSY(false);
		SetMSG(false);
		SetCD(false);
		SetIO(false);
		SetREQ(false);

	        CDIRQCallback(0x8000 | SCSICD_IRQ_DATA_TRANSFER_DONE);
		break;

  case PHASE_DATA_IN:		// Us to them
		SetBSY(true);
	        SetMSG(false);
	        SetCD(false);
	        SetIO(true);
	        //SetREQ(true);
		SetREQ(false);
		break;

  case PHASE_STATUS:		// Us to them
		SetBSY(true);
		SetMSG(false);
		SetCD(true);
		SetIO(true);
		SetREQ(true);
		break;

  case PHASE_MESSAGE_IN:	// Us to them
		SetBSY(true);
		SetMSG(true);
		SetCD(true);
		SetIO(true);
		SetREQ(true);
		break;


  case PHASE_DATA_OUT:		// Them to us
		SetBSY(true);
	        SetMSG(false);
	        SetCD(false);
	        SetIO(false);
	        SetREQ(true);
		break;

  case PHASE_COMMAND:		// Them to us
		SetBSY(true);
	        SetMSG(false);
	        SetCD(true);
	        SetIO(false);
	        SetREQ(true);
		break;

  case PHASE_MESSAGE_OUT:	// Them to us
		SetBSY(true);
  		SetMSG(true);
		SetCD(true);
		SetIO(false);
		SetREQ(true);
		break;
 }
 CurrentPhase = new_phase;
}

static void SendStatusAndMessage(uint8 status, uint8 message)
{
 // This should never ever happen, but that doesn't mean it won't. ;)
 if(din->CanRead())
 {
  printf("BUG: %d bytes still in SCSI CD FIFO\n", din->CanRead());
  din->Flush();
 }

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

 ChangePhase(PHASE_STATUS);
}

static void DoSimpleDataIn(const uint8 *data_in, uint32 len)
{
 din->Write(data_in, len);

 cd.data_transfer_done = true;

 ChangePhase(PHASE_DATA_IN);
}

void SCSICD_SetDisc(bool tray_open, CDIF *cdif, bool no_emu_side_effects)
{
 Cur_CDIF = cdif;

 // Closing the tray.
 if(cd.TrayOpen && !tray_open)
 {
  cd.TrayOpen = false;

  if(cdif)
  {
   cdif->ReadTOC(&toc);

   if(!no_emu_side_effects)
   {
    memset(cd.SubQBuf, 0, sizeof(cd.SubQBuf));
    memset(cd.SubQBuf_Last, 0, sizeof(cd.SubQBuf_Last));
    cd.DiscChanged = true;
   }
  }
 }
 else if(!cd.TrayOpen && tray_open)	// Opening the tray
 {
  cd.TrayOpen = true;
 }
}

static void CommandCCError(int key, int asc = 0, int ascq = 0)
{
 printf("CC Error: %02x %02x %02x\n", key, asc, ascq);

 cd.key_pending = key;
 cd.asc_pending = asc;
 cd.ascq_pending = ascq;
 cd.fru_pending = 0x00;

 SendStatusAndMessage(STATUS_CHECK_CONDITION, 0x00);
}

static bool ValidateRawDataSector(uint8 *data, const uint32 lba)
{
 if(!Cur_CDIF->ValidateRawSector(data))
 {
  MDFN_DispMessage(_("Uncorrectable data at sector %d"), lba);
  MDFN_PrintError(_("Uncorrectable data at sector %d"), lba);

  din->Flush();
  cd.data_transfer_done = false;

  CommandCCError(SENSEKEY_MEDIUM_ERROR, AP_LEC_UNCORRECTABLE_ERROR);
  return(false);
 }

 return(true);
}

static void DoMODESELECT6(const uint8 *cdb)
{
 if(cdb[4])
 {
  cd.data_out_pos = 0;
  cd.data_out_size = cdb[4];
  //printf("Switch to DATA OUT phase, len: %d\n", cd.data_out_size);

  ChangePhase(PHASE_DATA_OUT);
 }
 else
  SendStatusAndMessage(STATUS_GOOD, 0x00);
}

/*
 All Japan Female Pro Wrestle:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0a

 Kokuu Hyouryuu Nirgends:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0f
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0f

 Last Imperial Prince:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0f 
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0f 

 Megami Paradise II:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0a

 Miraculum:
	Datumama: 7, 00 00 00 00 29 01 00 
	Datumama: 10, 00 00 00 00 00 00 00 00 00 0f 
	Datumama: 7, 00 00 00 00 29 01 00 
	Datumama: 10, 00 00 00 00 00 00 00 00 00 00 
	Datumama: 7, 00 00 00 00 29 01 00 

 Pachio Kun FX:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 14

 Return to Zork:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 00

 Sotsugyou II:
	Datumama: 10, 00 00 00 00 01 00 00 00 00 01

 Tokimeki Card Paradise:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 14 
	Datumama: 10, 00 00 00 00 00 00 00 00 00 07 

 Tonari no Princess Rolfee:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 00

 Zoku Hakutoi Monogatari:
	Datumama: 10, 00 00 00 00 00 00 00 00 00 14
*/

      // Page 151: MODE SENSE(6)
	// PC = 0 current
	// PC = 1 Changeable
	// PC = 2 Default
	// PC = 3 Saved
      // Page 183: Mode parameter header.
      // Page 363: CD-ROM density codes.
      // Page 364: CD-ROM mode page codes.
      // Page 469: ASC and ASCQ table


struct ModePageParam
{
 const uint8 default_value;
 const uint8 alterable_mask;	// Alterable mask reported when PC == 1
 const uint8 real_mask;		// Real alterable mask.
};

struct ModePage
{
 const uint8 code;
 const uint8 param_length;
 const ModePageParam params[64];	// 64 should be more than enough
 uint8 current_value[64];
};

/*
 Mode pages present:
	0x00:
	0x0E:
	0x28:
	0x29:
	0x2A:
	0x2B:
	0x3F(Yes, not really a mode page but a fetch method)
*/
static const int NumModePages = 5;
static ModePage ModePages[] =
{
 // Unknown
 { 0x28,
   0x04,
   {
        { 0x00, 0x00, 0xFF },
        { 0x00, 0x00, 0xFF },
        { 0x00, 0x00, 0xFF },
        { 0x00, 0x00, 0xFF },
   }
 },

 // Unknown
 { 0x29,
   0x01,
   {
	{ 0x00, 0x00, 0xFF },
   }
 },

 // Unknown
 { 0x2a,
   0x02,
   {
        { 0x00, 0x00, 0xFF },
        { 0x11, 0x00, 0xFF },
   }
 },

 // CD-DA playback speed modifier
 { 0x2B,
   0x01,
   {
	{ 0x00, 0x00, 0xFF },
   }
 },

 // 0x0E goes last, for correct order of return data when page code == 0x3F
 // Real mask values are probably not right; some functionality not emulated yet.
 // CD-ROM audio control parameters
 { 0x0E,
   0x0E,
   {
        { 0x04, 0x04, 0x04 },   // Immed
        { 0x00, 0x00, 0x00 },   // Reserved
        { 0x00, 0x00, 0x00 }, // Reserved
        { 0x00, 0x01, 0x01 }, // Reserved?
        { 0x00, 0x00, 0x00 },   // MSB of LBA per second.
        { 0x00, 0x00, 0x00 }, // LSB of LBA per second.
        { 0x01, 0x01, 0x03 }, // Outport port 0 channel selection.
        { 0xFF, 0x00, 0x00 }, // Outport port 0 volume.
        { 0x02, 0x02, 0x03 }, // Outport port 1 channel selection.
        { 0xFF, 0x00, 0x00 }, // Outport port 1 volume.
        { 0x00, 0x00, 0x00 }, // Outport port 2 channel selection.
        { 0x00, 0x00, 0x00 }, // Outport port 2 volume.
        { 0x00, 0x00, 0x00 }, // Outport port 3 channel selection.
        { 0x00, 0x00, 0x00 }, // Outport port 3 volume.
   }
 },
};

static void UpdateMPCache(uint8 code)
{
 for(int pi = 0; pi < NumModePages; pi++)
 { 
  const ModePage *mp = &ModePages[pi];
  //const ModePageParam *params = &ModePages[pi].params[0];

  if(mp->code != code)
   continue;

  switch(mp->code)
  {
   case 0x0E:
	     {
              const uint8 *pd = &mp->current_value[0];

              for(int i = 0; i < 2; i++)
               cdda.OutPortChSelect[i] = pd[6 + i * 2];
              FixOPV();
	     }
	     break;

   case 0x28:
	     break;

   case 0x29:
	     break;

   case 0x2A:
	     break;

   case 0x2B:
	    {
             int8 speed = mp->current_value[0];
             double rate = 44100 + (double)44100 * speed / 100;
             //printf("Speed: %d %f\n", speed, rate);
             cdda.CDDADivAcc = (int32)((int64)System_Clock * 65536 / rate);
	    }
	    break;
  }

  break;
 }
}

static void InitModePages(void)
{
 for(int pi = 0; pi < NumModePages; pi++)
 {
  ModePage *mp = &ModePages[pi];
  const ModePageParam *params = &ModePages[pi].params[0];

  for(int parami = 0; parami < mp->param_length; parami++)
   mp->current_value[parami] = params[parami].default_value;

  UpdateMPCache(mp->code);
 }
}

static void FinishMODESELECT6(const uint8 *data, const uint32 data_len)
{
	uint8 mode_data_length, medium_type, device_specific, block_descriptor_length;
	uint32 offset = 0;

        printf("Mode Select (6) Data: 0x%08x, ", data_len);
        for(unsigned int i = 0; i < cd.data_out_size; i++)
         printf("0x%02x ", cd.data_out[i]);
        printf("\n");

        if(data_len < 4)
        {
         CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
         return;
        }

	mode_data_length = data[offset++];
	medium_type = data[offset++];
	device_specific = data[offset++];
	block_descriptor_length = data[offset++];

	if(block_descriptor_length & 0x7)
	{
	 CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	 return;
	}

	if((offset + block_descriptor_length) > data_len)
	{
	 CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	 return;
	}

	// TODO: block descriptors.
	offset += block_descriptor_length;

	// Now handle mode pages
	while(offset < data_len)
	{
	 const uint8 code = data[offset++];
	 uint8 param_len = 0;
	 bool page_found = false;

	 if(code == 0x00)
	 {
	  if((offset + 0x5) > data_len)
	  {
           CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	   return;
	  }

	  UpdateMPCache(0x00);

	  offset += 0x5;
 	  continue;
	 }

	 if(offset >= data_len)
	 {
          CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	  return;
	 }

         param_len = data[offset++];

	 for(int pi = 0; pi < NumModePages; pi++)
	 {
	  ModePage *mp = &ModePages[pi];

	  if(code == mp->code)
	  {
	   page_found = true;
	
	   if(param_len != mp->param_length)
	   {
            CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
            return;
	   }

	   if((param_len + offset) > data_len)
	   {
            CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
            return;
	   }

	   for(int parami = 0; parami < mp->param_length; parami++)
	   {
	    mp->current_value[parami] &= ~mp->params[parami].real_mask;
	    mp->current_value[parami] |= (data[offset++]) & mp->params[parami].real_mask;
	   }

	   UpdateMPCache(mp->code);
	   break;
	  }
	 }

	 if(!page_found)
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	  return;
	 }
	}

	SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoMODESENSE6(const uint8 *cdb)
{
 unsigned int PC = (cdb[2] >> 6) & 0x3;
 unsigned int PageCode = cdb[2] & 0x3F;
 bool DBD = cdb[1] & 0x08;
 int AllocSize = cdb[4];
 int index = 0;
 uint8 data_in[8192];
 uint8 PageMatchOR = 0x00;
 bool AnyPageMatch = false;

 SCSIDBG("Mode sense 6: %02x %d %d %d\n", PageCode, PC, DBD, AllocSize);

 if(!AllocSize)
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }

 if(PC == 3)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(PageCode == 0x00)	// Special weird case.
 {
  if(DBD || PC)
  {
   CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
   return;
  }

  memset(data_in, 0, 0xA);
  data_in[0] = 0x09;
  data_in[2] = 0x80;
  data_in[9] = 0x0F;

  if(AllocSize > 0xA)
   AllocSize = 0xA;

  DoSimpleDataIn(data_in, AllocSize);
  return;
 }

 data_in[0] = 0x00;	// Fill this in later.
 data_in[1] = 0x00;	// Medium type
 data_in[2] = 0x00;	// Device-specific parameter.
 data_in[3] = DBD ? 0x00 : 0x08;	// Block descriptor length.
 index += 4;

 if(!DBD)
 {
  data_in[index++] = 0x00;	// Density code.
  MDFN_en24msb(&data_in[index], 0x6E); // Number of blocks?
  index += 3;
 
  data_in[index++] = 0x00;	// Reserved
  MDFN_en24msb(&data_in[index], 0x800); // Block length;
  index += 3;
 }

 PageMatchOR = 0x00;
 if(PageCode == 0x3F)
  PageMatchOR = 0x3F;

 for(int pi = 0; pi < NumModePages; pi++)
 {
  const ModePage *mp = &ModePages[pi];
  const ModePageParam *params = &ModePages[pi].params[0];

  if((mp->code | PageMatchOR) != PageCode)
   continue;

  AnyPageMatch = true;

  data_in[index++] = mp->code;
  data_in[index++] = mp->param_length;
  
  for(int parami = 0; parami < mp->param_length; parami++)
  {
   uint8 data;

   if(PC == 0x02)
    data = params[parami].default_value;
   else if(PC == 0x01)
    data = params[parami].alterable_mask;
   else
    data = mp->current_value[parami];

   data_in[index++] = data;
  }
 }

 if(!AnyPageMatch)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(AllocSize > index)
  AllocSize = index;

 data_in[0] = AllocSize - 1;

 DoSimpleDataIn(data_in, AllocSize);
}

static void DoSTARTSTOPUNIT6(const uint8 *cdb)
{
 bool Immed = cdb[1] & 0x01;
 bool LoEj = cdb[4] & 0x02;
 bool Start = cdb[4] & 0x01;

 SCSIDBG("Do start stop unit 6: %d %d %d\n", Immed, LoEj, Start);

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoREZEROUNIT(const uint8 *cdb)
{
 SCSIDBG("Rezero Unit: %02x\n", cdb[5]);
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

// This data was originally taken from a PC-FXGA software loader, but
// while it was mostly correct(maybe it is correct for the FXGA, but not for the PC-FX?),
// it was 3 bytes too long, and the last real byte was 0x45 instead of 0x20.
// TODO:  Investigate this discrepancy by testing an FXGA with the official loader software.
#if 0
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
#endif

// Miraculum behaves differently if the last byte(offset 0x23) of the inquiry data is 0x45(ASCII character 'E').  Relavent code is at PC=0x3E382
// If it's = 0x45, it will run MODE SELECT, and transfer this data to the CD unit: 00 00 00 00 29 01 00
static const uint8 InqData[0x24] =
{
 // Peripheral device-type: CD-ROM/read-only direct access device
 0x05,

 // Removable media: yes
 // Device-type qualifier: 0
 0x80,

 // ISO version: 0
 // ECMA version: 0
 // ANSI version: 2	(SCSI-2? ORLY?)
 0x02,

 // Supports asynchronous event notification: no
 // Supports the terminate I/O process message: no
 // Response data format: 0 (not exactly correct, not exactly incorrect, meh. :b)
 0x00,

 // Additional Length
 0x1F,

 // Reserved
 0x00, 0x00,

 // Yay, no special funky features.
 0x00,

 // 8-15, vendor ID
 // NEC     
 0x4E, 0x45, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20,

 // 16-31, product ID
 // CD-ROM DRIVE:FX 
 0x43, 0x44, 0x2D, 0x52, 0x4F, 0x4D, 0x20, 0x44, 0x52, 0x49, 0x56, 0x45, 0x3A, 0x46, 0x58, 0x20,

 // 32-35, product revision level
 // 1.0 
 0x31, 0x2E, 0x30, 0x20
};

static void DoINQUIRY(const uint8 *cdb)
{
 unsigned int AllocSize = (cdb[4] < sizeof(InqData)) ? cdb[4] : sizeof(InqData);

 if(AllocSize)
  DoSimpleDataIn(InqData, AllocSize);
 else
  SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoNEC_NOP(const uint8 *cdb)
{
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



/********************************************************
*							*
*	PC-FX CD Command 0xDC - EJECT			*
*				  			*
********************************************************/
static void DoNEC_EJECT(const uint8 *cdb)
{
 CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_REQUEST_IN_CDB);
}

static void DoREQUESTSENSE(const uint8 *cdb)
{
 uint8 data_in[8192];

 MakeSense(data_in, cd.key_pending, cd.asc_pending, cd.ascq_pending, cd.fru_pending);

 DoSimpleDataIn(data_in, 18);

 cd.key_pending = 0;
 cd.asc_pending = 0;
 cd.ascq_pending = 0;
 cd.fru_pending = 0;
}

static void EncodeM3TOC(uint8 *buf, uint8 POINTER_RAW, int32 LBA, uint32 PLBA, uint8 control)
{
 uint8 MIN, SEC, FRAC;
 uint8 PMIN, PSEC, PFRAC;

 LBA_to_AMSF(LBA, &MIN, &SEC, &FRAC);
 LBA_to_AMSF(PLBA, &PMIN, &PSEC, &PFRAC);

 buf[0x0] = control << 4;
 buf[0x1] = 0x00;	// TNO
 buf[0x2] = POINTER_RAW;
 buf[0x3] = U8_to_BCD(MIN);
 buf[0x4] = U8_to_BCD(SEC);
 buf[0x5] = U8_to_BCD(FRAC);
 buf[0x6] = 0x00;	// Zero
 buf[0x7] = U8_to_BCD(PMIN);
 buf[0x8] = U8_to_BCD(PSEC);
 buf[0x9] = U8_to_BCD(PFRAC);
}

/********************************************************
*							*
*	PC-FX CD Command 0xDE - Get Directory Info	*
*							*
********************************************************/
static void DoNEC_GETDIRINFO(const uint8 *cdb)
{
 // Problems:
 //	Mode 0x03 has a few semi-indeterminate(but within a range, and they only change when the disc is reloaded) fields on a real PC-FX, that correspond to where in the lead-in area the data
 //	was read, that we don't bother to handle here.
 //	Mode 0x03 returns weird/wrong control field data for the "last track" and "leadout" entries in the "Blue Breaker" TOC.
 //		A bug in the PC-FX CD firmware, or an oddity of the disc(maybe other PC-FX discs are similar)?  Or maybe it's an undefined field in that context?
 //	"Match" value of 0xB0 is probably not handled properly.  Is it to return the catalog number, or something else?

 uint8 data_in[2048];
 uint32 data_in_size = 0;

 memset(data_in, 0, sizeof(data_in));

 switch(cdb[1] & 0x03)
 {
  // This commands returns relevant raw TOC data as encoded in the Q subchannel(sans the CRC bytes).
  case 0x3:
   {
    int offset = 0;
    int32 lilba = -150;
    uint8 match = cdb[2];

    if(match != 0x00 && match != 0xA0 && match != 0xA1 && match != 0xA2 && match != 0xB0)
    {
     CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_ADDRESS);
     return;
    }
    memset(data_in, 0, sizeof(data_in));

    data_in[0] = 0x00;	// Size MSB???
    data_in[1] = 0x00;	// Total Size - 2(we'll fill it in later).
    offset = 2;

    if(!match || match == 0xA0)
    {
     EncodeM3TOC(&data_in[offset], 0xA0, lilba, toc.first_track * 75 * 60 - 150, toc.tracks[toc.first_track].control);
     lilba++;
     offset += 0xA;
    }

    if(!match || match == 0xA1)
    {
     EncodeM3TOC(&data_in[offset], 0xA1, lilba, toc.last_track * 75 * 60 - 150, toc.tracks[toc.last_track].control);
     lilba++;
     offset += 0xA;
    }
  
    if(!match || match == 0xA2)
    {
     EncodeM3TOC(&data_in[offset], 0xA2, lilba, toc.tracks[100].lba, toc.tracks[100].control);
     lilba++;
     offset += 0xA;
    }

    if(!match)
     for(int track = toc.first_track; track <= toc.last_track; track++)
     {
      EncodeM3TOC(&data_in[offset], U8_to_BCD(track), lilba, toc.tracks[track].lba, toc.tracks[track].control);
      lilba++;
      offset += 0xA;
     }

    if(match == 0xB0)
    {
     memset(&data_in[offset], 0, 0x14);
     offset += 0x14;
    }

    assert((unsigned int)offset <= sizeof(data_in));
    data_in_size = offset;
    MDFN_en16msb(&data_in[0], offset - 2);
   }
   break;

  case 0x0:
   data_in[0] = U8_to_BCD(toc.first_track);
   data_in[1] = U8_to_BCD(toc.last_track);

   data_in_size = 4;
   break;

  case 0x1:
   {
    uint8 m, s, f;

    LBA_to_AMSF(toc.tracks[100].lba, &m, &s, &f);

    data_in[0] = U8_to_BCD(m);
    data_in[1] = U8_to_BCD(s);
    data_in[2] = U8_to_BCD(f);

    data_in_size = 4;
   }
   break;

  case 0x2:
   {
    uint8 m, s, f;
    int track = BCD_to_U8(cdb[2]);

    if(track < toc.first_track || track > toc.last_track)
    {
     CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_ADDRESS);
     return;
    }

    LBA_to_AMSF(toc.tracks[track].lba, &m, &s, &f);

    data_in[0] = U8_to_BCD(m);
    data_in[1] = U8_to_BCD(s);
    data_in[2] = U8_to_BCD(f);
    data_in[3] = toc.tracks[track].control;
    data_in_size = 4;
   }
   break;
 }

 DoSimpleDataIn(data_in, data_in_size);
}

static void DoREADTOC(const uint8 *cdb)
{
 uint8 data_in[8192];
 int FirstTrack = toc.first_track;
 int LastTrack = toc.last_track;
 int StartingTrack = cdb[6];
 unsigned int AllocSize = (cdb[7] << 8) | cdb[8];
 unsigned int RealSize = 0;
 const bool WantInMSF = cdb[1] & 0x2;

 if(!AllocSize)
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }

 if((cdb[1] & ~0x2) || cdb[2] || cdb[3] || cdb[4] || cdb[5] || cdb[9])
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(!StartingTrack)
  StartingTrack = 1;
 else if(StartingTrack == 0xAA)
 {
  StartingTrack = LastTrack + 1;
 }
 else if(StartingTrack > toc.last_track)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 data_in[2] = FirstTrack;
 data_in[3] = LastTrack;

 RealSize += 4;

 // Read leadout track too LastTrack + 1 ???
 for(int track = StartingTrack; track <= (LastTrack + 1); track++)
 {
  uint8 *subptr = &data_in[RealSize];
  uint32 lba;
  uint8 m, s, f;
  uint32 eff_track;

  if(track == (LastTrack + 1))
   eff_track = 100;
  else
   eff_track = track;

  lba = toc.tracks[eff_track].lba;
  LBA_to_AMSF(lba, &m, &s, &f);

  subptr[0] = 0;
  subptr[1] = toc.tracks[eff_track].control | (toc.tracks[eff_track].adr << 4);

  if(eff_track == 100)
   subptr[2] = 0xAA;
  else
   subptr[2] = track;

  subptr[3] = 0;

  if(WantInMSF)
  {
   subptr[4] = 0;
   subptr[5] = m;		// Min
   subptr[6] = s;		// Sec
   subptr[7] = f;		// Frames
  }
  else
  {
   subptr[4] = lba >> 24;
   subptr[5] = lba >> 16;
   subptr[6] = lba >> 8;
   subptr[7] = lba >> 0;
  }
  RealSize += 8;
 }

 // PC-FX: AllocSize too small doesn't reflect in this.
 data_in[0] = (RealSize - 2) >> 8;
 data_in[1] = (RealSize - 2) >> 0;

 DoSimpleDataIn(data_in, (AllocSize < RealSize) ? AllocSize : RealSize);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x25 - READ CD-ROM CAPACITY	*
*							*
********************************************************/
static void DoREADCDCAP10(const uint8 *cdb)
{
 bool pmi = cdb[8] & 0x1;
 uint32 lba = MDFN_de32msb(cdb + 0x2);
 uint32 ret_lba;
 uint32 ret_bl;
 uint8 data_in[8];

 memset(data_in, 0, sizeof(data_in));

 if(lba > 0x05FF69)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
  return;
 }

 ret_lba = toc.tracks[100].lba - 1;

 if(pmi)
 {
  // Look for the track containing the LBA specified, then search for the first track afterwards that has a different track type(audio, data),
  // and set the returned LBA to the sector preceding that track.
  //
  // If the specified LBA is >= leadout track, return the LBA of the sector immediately before the leadout track.
  //
  // If the specified LBA is < than the LBA of the first track, then return the LBA of sector preceding the first track.  (I don't know if PC-FX can even handle discs like this, though)
  if(lba >= toc.tracks[100].lba)
   ret_lba = toc.tracks[100].lba - 1;
  else if(lba < toc.tracks[toc.first_track].lba)
   ret_lba = toc.tracks[toc.first_track].lba - 1;
  else
  {
   const int track = toc.FindTrackByLBA(lba);

   for(int st = track + 1; st <= toc.last_track; st++)
   {
    if((toc.tracks[st].control ^ toc.tracks[track].control) & 0x4)
    {
     ret_lba = toc.tracks[st].lba - 1;
     break;
    }
   }
  }
 }

 ret_bl = 2048;

 MDFN_en32msb(&data_in[0], ret_lba);
 MDFN_en32msb(&data_in[4], ret_bl);

 cdda.CDDAStatus = CDDASTATUS_STOPPED;

 DoSimpleDataIn(data_in, 8);
}

static void DoREADHEADER10(const uint8 *cdb)
{
 uint8 data_in[8192];
 bool WantInMSF = cdb[1] & 0x2;
 uint32 HeaderLBA = MDFN_de32msb(cdb + 0x2);
 int AllocSize = MDFN_de16msb(cdb + 0x7);
 uint8 raw_buf[2352 + 96];
 uint8 mode;
 int m, s, f;
 uint32 lba;

 // Don't run command at all if AllocSize == 0(FIXME: On a real PC-FX, this command will return success
 // if there's no CD when AllocSize == 0, implement this here, might require refactoring).
 if(!AllocSize)
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }

 if(HeaderLBA >= toc.tracks[100].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(HeaderLBA < toc.tracks[toc.first_track].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 Cur_CDIF->ReadRawSector(raw_buf, HeaderLBA);	//, HeaderLBA + 1);
 if(!ValidateRawDataSector(raw_buf, HeaderLBA))
  return;

 m = BCD_to_U8(raw_buf[12 + 0]);
 s = BCD_to_U8(raw_buf[12 + 1]);
 f = BCD_to_U8(raw_buf[12 + 2]);
 mode = raw_buf[12 + 3];
 lba = AMSF_to_LBA(m, s, f);

 //printf("%d:%d:%d(LBA=%08x) %02x\n", m, s, f, lba, mode);

 data_in[0] = mode;
 data_in[1] = 0;
 data_in[2] = 0;
 data_in[3] = 0;

 if(WantInMSF)
 {
  data_in[4] = 0;
  data_in[5] = m;	// Min
  data_in[6] = s;	// Sec
  data_in[7] = f;	// Frames
 }
 else
 {
  data_in[4] = lba >> 24;
  data_in[5] = lba >> 16;
  data_in[6] = lba >> 8;
  data_in[7] = lba >> 0;
 }

 cdda.CDDAStatus = CDDASTATUS_STOPPED;

 DoSimpleDataIn(data_in, 8);
}

static void DoNEC_SST(const uint8 *cdb)		// Command 0xDB, Set Stop Time
{
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoPABase(const uint32 lba, const uint32 length, unsigned int status = CDDASTATUS_PLAYING, unsigned int mode = PLAYMODE_NORMAL)
{
 if(lba > toc.tracks[100].lba) // > is not a typo, it's a PC-FX bug apparently.
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }
  
 if(lba < toc.tracks[toc.first_track].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(!length)	// FIXME to return good status in this case even if no CD is present
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }
 else
 {
  if(toc.tracks[toc.FindTrackByLBA(lba)].control & 0x04)
  {
   CommandCCError(SENSEKEY_MEDIUM_ERROR, NSE_NOT_AUDIO_TRACK);
   return;
  }

  cdda.CDDAReadPos = 588;
  read_sec = read_sec_start = lba;
  read_sec_end = read_sec_start + length;

  cdda.CDDAStatus = status;
  cdda.PlayMode = mode;

  if(read_sec < toc.tracks[100].lba)
  {
   Cur_CDIF->HintReadSector(read_sec);	//, read_sec_end, read_sec_start);
  }
 }

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



/********************************************************
*							*
*	PC-FX CD Command 0xD8 - SAPSP			*
*							*
********************************************************/
static void DoNEC_SAPSP(const uint8 *cdb)
{
 uint32 lba;

 switch (cdb[9] & 0xc0)
 {
  default: 
	CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	return;
	break;

  case 0x00:
	lba = MDFN_de24msb(&cdb[3]);
	break;

  case 0x40:
	{
	 uint8 m, s, f;

	 if(!BCD_to_U8_check(cdb[2], &m) || !BCD_to_U8_check(cdb[3], &s) || !BCD_to_U8_check(cdb[4], &f))
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	  return;
	 }

	 lba = AMSF_to_LBA(m, s, f);
	}
	break;

  case 0x80:
	{
	 uint8 track;

	 if(!cdb[2] || !BCD_to_U8_check(cdb[2], &track))
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	  return;
	 }

	 if(track == toc.last_track + 1)
	  track = 100;
	 else if(track > toc.last_track)
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
	  return;
	 }
	 lba = toc.tracks[track].lba;
	}
	break;
 }

 if(cdb[1] & 0x01)
  DoPABase(lba, toc.tracks[100].lba - lba, CDDASTATUS_PLAYING, PLAYMODE_NORMAL);
 else
  DoPABase(lba, toc.tracks[100].lba - lba, CDDASTATUS_PAUSED, PLAYMODE_SILENT);
}



/********************************************************
*							*
*	PC-FX CD Command 0xD9 - SAPEP			*
*							*
********************************************************/
static void DoNEC_SAPEP(const uint8 *cdb)
{
 uint32 lba;

 if(cdda.CDDAStatus == CDDASTATUS_STOPPED)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_AUDIO_NOT_PLAYING);
  return;
 }

 switch (cdb[9] & 0xc0)
 {
  default: 
	CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	return;
	break;

  case 0x00:
	lba = MDFN_de24msb(&cdb[3]);
	break;

  case 0x40:
	{
	 uint8 m, s, f;

	 if(!BCD_to_U8_check(cdb[2], &m) || !BCD_to_U8_check(cdb[3], &s) || !BCD_to_U8_check(cdb[4], &f))
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	  return;
	 }

	 lba = AMSF_to_LBA(m, s, f);
	}
	break;

  case 0x80:
	{
	 uint8 track;

	 if(!cdb[2] || !BCD_to_U8_check(cdb[2], &track))
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
	  return;
	 }

	 if(track == toc.last_track + 1)
	  track = 100;
	 else if(track > toc.last_track)
	 {
	  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
	  return;
	 }
	 lba = toc.tracks[track].lba;
	}
	break;
 }

 switch(cdb[1] & 0x7)
 {
   case 0x00: cdda.PlayMode = PLAYMODE_SILENT;
	      break;

   case 0x04: cdda.PlayMode = PLAYMODE_LOOP;
	      break;

   default:   cdda.PlayMode = PLAYMODE_NORMAL;
	      break;
 }
 cdda.CDDAStatus = CDDASTATUS_PLAYING;

 read_sec_end = lba;

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x45 - PLAY AUDIO(10) 	*
*				  			*
********************************************************/
static void DoPA10(const uint8 *cdb)
{
 // Real PC-FX Bug: Error out on LBA >(not >=) leadout sector number
 const uint32 lba = MDFN_de32msb(cdb + 0x2);
 const uint16 length = MDFN_de16msb(cdb + 0x7);

 DoPABase(lba, length);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0xA5 - PLAY AUDIO(12) 	*
*				  			*
********************************************************/
static void DoPA12(const uint8 *cdb)
{
 // Real PC-FX Bug: Error out on LBA >(not >=) leadout sector number
 const uint32 lba = MDFN_de32msb(cdb + 0x2);
 const uint32 length = MDFN_de32msb(cdb + 0x6);

 DoPABase(lba, length);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x47 - PLAY AUDIO MSF 	*
*				  			*
********************************************************/
static void DoPAMSF(const uint8 *cdb)
{
 int32 lba_start, lba_end;

 lba_start = AMSF_to_LBA(cdb[3], cdb[4], cdb[5]);
 lba_end = AMSF_to_LBA(cdb[6], cdb[7], cdb[8]);

 if(lba_start < 0 || lba_end < 0 || lba_start >= (int32)toc.tracks[100].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
  return;
 }

 if(lba_start == lba_end)
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }
 else if(lba_start > lba_end)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_ADDRESS);
  return;
 }

 cdda.CDDAReadPos = 588;
 read_sec = read_sec_start = lba_start;
 read_sec_end = lba_end;

 cdda.CDDAStatus = CDDASTATUS_PLAYING;
 cdda.PlayMode = PLAYMODE_NORMAL;

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



static void DoPATI(const uint8 *cdb)
{
 // "Boundary Gate" uses this command.
 // Problems:
 //  The index fields aren't handled.  The ending index wouldn't be too bad, but the starting index would require a bit of work and code uglyfying(to scan for the index), and may be highly
 //  problematic when Mednafen is used with a physical CD.
 int StartTrack = cdb[4];
 int EndTrack = cdb[7];
 int StartIndex = cdb[5];
 int EndIndex = cdb[8];

 if(!StartTrack || StartTrack < toc.first_track || StartTrack > toc.last_track)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 //printf("PATI: %d %d %d  SI: %d, EI: %d\n", StartTrack, EndTrack, Cur_CDIF->GetTrackStartPositionLBA(StartTrack), StartIndex, EndIndex);

 DoPABase(toc.tracks[StartTrack].lba, toc.tracks[EndTrack].lba - toc.tracks[StartTrack].lba);
}


static void DoPATRBase(const uint32 lba, const uint32 length)
{
 if(lba >= toc.tracks[100].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(lba < toc.tracks[toc.first_track].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(!length)	// FIXME to return good status in this case even if no CD is present
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }
 else
 {  
  if(toc.tracks[toc.FindTrackByLBA(lba)].control & 0x04)
  {
   CommandCCError(SENSEKEY_MEDIUM_ERROR, NSE_NOT_AUDIO_TRACK);
   return;
  }

  cdda.CDDAReadPos = 588;
  read_sec = read_sec_start = lba;
  read_sec_end = read_sec_start + length;

  cdda.CDDAStatus = CDDASTATUS_PLAYING;
  cdda.PlayMode = PLAYMODE_NORMAL;
 }

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}


/********************************************************
*							*
*	SCSI-2 CD Command 0x49 - PLAY AUDIO TRACK 	*
*				  RELATIVE(10)		*
********************************************************/
static void DoPATR10(const uint8 *cdb)
{
 const int32 rel_lba = MDFN_de32msb(cdb + 0x2);
 const int StartTrack = cdb[6];
 const uint16 length = MDFN_de16msb(cdb + 0x7);

 if(!StartTrack || StartTrack < toc.first_track || StartTrack > toc.last_track)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 DoPATRBase(toc.tracks[StartTrack].lba + rel_lba, length);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0xA9 - PLAY AUDIO TRACK 	*
*				  RELATIVE(12)		*
********************************************************/
static void DoPATR12(const uint8 *cdb)
{
 const int32 rel_lba = MDFN_de32msb(cdb + 0x2);
 const int StartTrack = cdb[10];
 const uint32 length = MDFN_de32msb(cdb + 0x6);

 if(!StartTrack || StartTrack < toc.first_track || StartTrack > toc.last_track)
 { 
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 DoPATRBase(toc.tracks[StartTrack].lba + rel_lba, length);
}

static void DoPAUSERESUME(const uint8 *cdb)
{
 // Pause/resume
 // "It shall not be considered an error to request a pause when a pause is already in effect, 
 // or to request a resume when a play operation is in progress."

 if(cdda.CDDAStatus == CDDASTATUS_STOPPED)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_AUDIO_NOT_PLAYING);
  return;
 }

 if(cdb[8] & 1)	// Resume
  cdda.CDDAStatus = CDDASTATUS_PLAYING;
 else
  cdda.CDDAStatus = CDDASTATUS_PAUSED;

 SendStatusAndMessage(STATUS_GOOD, 0x00);
}





static void DoREADBase(uint32 sa, uint32 sc)
{
 int track;

 if(sa > toc.tracks[100].lba) // Another one of those off-by-one PC-FX CD bugs.
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
  return;
 }

 if((track = toc.FindTrackByLBA(sa)) == 0)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
  return;
 }

 if(!(toc.tracks[track].control) & 0x4)
 {
  CommandCCError(SENSEKEY_MEDIUM_ERROR, NSE_NOT_DATA_TRACK);
  return;
 }

 // Case for READ(10) and READ(12) where sc == 0, and sa == toc.tracks[100].lba
 if(!sc && sa == toc.tracks[100].lba)
 {
  CommandCCError(SENSEKEY_MEDIUM_ERROR, NSE_HEADER_READ_ERROR);
  return;
 }

 if(SCSILog)
 {
  int Track = toc.FindTrackByLBA(sa);
  uint32 Offset = sa - toc.tracks[Track].lba; //Cur_CDIF->GetTrackStartPositionLBA(Track);
  SCSILog("SCSI", "Read: start=0x%08x(track=%d, offs=0x%08x), cnt=0x%08x", sa, Track, Offset, sc);
 }

 SectorAddr = sa;
 SectorCount = sc;
 if(SectorCount)
 {
  Cur_CDIF->HintReadSector(sa);	//, sa + sc);

  CDReadTimer = (uint64)1 * 2048 * System_Clock / CD_DATA_TRANSFER_RATE;
 }
 else
 {
  CDReadTimer = 0;
  SendStatusAndMessage(STATUS_GOOD, 0x00);
 }
 cdda.CDDAStatus = CDDASTATUS_STOPPED;
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x08 - READ(6)		*
*							*
********************************************************/
static void DoREAD6(const uint8 *cdb)
{
 uint32 sa = ((cdb[1] & 0x1F) << 16) | (cdb[2] << 8) | (cdb[3] << 0);
 uint32 sc = cdb[4];

 // TODO: confirm real PCE does this(PC-FX does at least).
 if(!sc)
 {
  SCSIDBG("READ(6) with count == 0.\n");
  sc = 256;
 }

 DoREADBase(sa, sc);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x28 - READ(10)		*
*							*
********************************************************/
static void DoREAD10(const uint8 *cdb)
{
 uint32 sa = MDFN_de32msb(cdb + 0x2);
 uint32 sc = MDFN_de16msb(cdb + 0x7);

 DoREADBase(sa, sc);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0xA8 - READ(12)		*
*							*
********************************************************/
static void DoREAD12(const uint8 *cdb)
{
 uint32 sa = MDFN_de32msb(cdb + 0x2);
 uint32 sc = MDFN_de32msb(cdb + 0x6);

 DoREADBase(sa, sc);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x34 - PREFETCH(10)		*
*							*
********************************************************/
static void DoPREFETCH(const uint8 *cdb)
{
 uint32 lba = MDFN_de32msb(cdb + 0x2);
 //uint32 len = MDFN_de16msb(cdb + 0x7);
 //bool reladdr = cdb[1] & 0x1;
 //bool immed = cdb[1] & 0x2;

 // Note: This command appears to lock up the CD unit to some degree on a real PC-FX if the (lba + len) >= leadout_track_lba,
 // more testing is needed if we ever try to fully emulate this command.
 if(lba >= (int32)toc.tracks[100].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
  return;
 }

 //printf("Prefetch: %08x %08x %d %d %d %d\n", lba, len, link, flag, reladdr, immed);
 //SendStatusAndMessage(STATUS_GOOD, 0x00);
 SendStatusAndMessage(STATUS_CONDITION_MET, 0x00);
}




// SEEK functions are mostly just stubs for now, until(if) we emulate seek delays.
static void DoSEEKBase(uint32 lba)
{
 if(lba >= toc.tracks[100].lba)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
  return;
 } 

 cdda.CDDAStatus = CDDASTATUS_STOPPED;
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x0B - SEEK(6)		*
*							*
********************************************************/
static void DoSEEK6(const uint8 *cdb)
{
 uint32 lba = ((cdb[1] & 0x1F) << 16) | (cdb[2] << 8) | cdb[3];

 DoSEEKBase(lba);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x2B - SEEK(10)		*
*							*
********************************************************/
static void DoSEEK10(const uint8 *cdb)
{
 uint32 lba = MDFN_de32msb(cdb + 0x2);

 DoSEEKBase(lba);
}

// 353
/********************************************************
*							*
*	SCSI-2 CD Command 0x42 - READ SUB-CHANNEL(10)	*
*							*
********************************************************/
static void DoREADSUBCHANNEL(const uint8 *cdb)
{
 uint8 data_in[8192];
 int DataFormat = cdb[3];
 int TrackNum = cdb[6];
 int AllocSize = (cdb[7] << 8) | cdb[8];
 bool WantQ = cdb[2] & 0x40;
 bool WantMSF = cdb[1] & 0x02;
 uint32 offset = 0;

 if(!AllocSize)
 {
  SendStatusAndMessage(STATUS_GOOD, 0x00);
  return;
 }

 if(DataFormat > 0x3)
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 if(DataFormat == 0x3 && (TrackNum < toc.first_track || TrackNum > toc.last_track))
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_PARAMETER);
  return;
 }

 data_in[offset++] = 0;

 // FIXME:  Is this audio status code correct for scanning playback??
 if(cdda.CDDAStatus == CDDASTATUS_PLAYING || cdda.CDDAStatus == CDDASTATUS_SCANNING)
  data_in[offset++] = 0x11;	// Audio play operation in progress
 else if(cdda.CDDAStatus == CDDASTATUS_PAUSED)
  data_in[offset++] = 0x12;	// Audio play operation paused
 else
  data_in[offset++] = 0x13;	// 0x13(audio play operation completed successfully) or 0x15(no current audio status to return)? :(


 // Subchannel data length(at data_in[0x2], filled out at the end of the function)
 data_in[offset++] = 0x00;
 data_in[offset++] = 0x00;

 //printf("42Read SubChannel: %02x %02x %d %d %d\n", DataFormat, TrackNum, AllocSize, WantQ, WantMSF);
 if(WantQ)
 {
  // Sub-channel format code
  data_in[offset++] = DataFormat;
  if(!DataFormat || DataFormat == 0x01)
  {
   uint8 *SubQBuf = cd.SubQBuf[QMode_Time];

   data_in[offset++] = ((SubQBuf[0] & 0x0F) << 4) | ((SubQBuf[0] & 0xF0) >> 4); // Control/adr
   data_in[offset++] = SubQBuf[1]; // Track
   data_in[offset++] = SubQBuf[2]; // Index

   // Absolute CD-ROM address
   if(WantMSF)
   {
    data_in[offset++] = 0;
    data_in[offset++] = BCD_to_U8(SubQBuf[7]); // M
    data_in[offset++] = BCD_to_U8(SubQBuf[8]); // S
    data_in[offset++] = BCD_to_U8(SubQBuf[9]); // F
   }
   else
   {
    uint32 tmp_lba = BCD_to_U8(SubQBuf[7]) * 60 * 75 + BCD_to_U8(SubQBuf[8]) * 75 + BCD_to_U8(SubQBuf[9]) - 150;

    data_in[offset++] = tmp_lba >> 24;
    data_in[offset++] = tmp_lba >> 16;
    data_in[offset++] = tmp_lba >> 8;
    data_in[offset++] = tmp_lba >> 0;
   }

   // Relative CD-ROM address
   if(WantMSF)
   {
    data_in[offset++] = 0;
    data_in[offset++] = BCD_to_U8(SubQBuf[3]); // M
    data_in[offset++] = BCD_to_U8(SubQBuf[4]); // S
    data_in[offset++] = BCD_to_U8(SubQBuf[5]); // F
   }
   else
   {
    uint32 tmp_lba = BCD_to_U8(SubQBuf[3]) * 60 * 75 + BCD_to_U8(SubQBuf[4]) * 75 + BCD_to_U8(SubQBuf[5]);	// Don't subtract 150 in the conversion!

    data_in[offset++] = tmp_lba >> 24;
    data_in[offset++] = tmp_lba >> 16;
    data_in[offset++] = tmp_lba >> 8;
    data_in[offset++] = tmp_lba >> 0;
   }
  }  

  if(!DataFormat || DataFormat == 0x02)
  {
   if(DataFormat == 0x02)
   {
    data_in[offset++] = 0x00;
    data_in[offset++] = 0x00;
    data_in[offset++] = 0x00;
   }
   data_in[offset++] = 0x00;	// MCVal and reserved.
   for(int i = 0; i < 15; i++)
    data_in[offset++] = 0x00;
  }

  // Track ISRC
  if(!DataFormat || DataFormat == 0x03)
  {
   if(DataFormat == 0x03)
   {
    uint8 *SubQBuf = cd.SubQBuf[QMode_Time];	// FIXME
    data_in[offset++] = ((SubQBuf[0] & 0x0F) << 4) | ((SubQBuf[0] & 0xF0) >> 4); // Control/adr
    data_in[offset++] = TrackNum;	// From sub Q or from parameter?
    data_in[offset++] = 0x00;		// Reserved.
   }
   data_in[offset++] = 0x00; // TCVal and reserved
   for(int i = 0; i < 15; i++)
    data_in[offset++] = 0x00;
  }
 }

 MDFN_en16msb(&data_in[0x2], offset - 0x4);

 DoSimpleDataIn(data_in, (AllocSize > offset) ? offset : AllocSize);
}



/********************************************************
*							*
*	PC-FX CD Command 0xDD - READ SUB Q		*
*				  			*
********************************************************/
static void DoNEC_READSUBQ(const uint8 *cdb)
{
 uint8 *SubQBuf = cd.SubQBuf[QMode_Time];
 uint8 data_in[10];
 const uint8 alloc_size = (cdb[1] < 10) ? cdb[1] : 10;

 memset(data_in, 0x00, 10);

 if(cdda.CDDAStatus == CDDASTATUS_PAUSED)
  data_in[0] = 2;		// Pause
 else if(cdda.CDDAStatus == CDDASTATUS_PLAYING || cdda.CDDAStatus == CDDASTATUS_SCANNING) // FIXME:  Is this the correct status code for scanning playback?
  data_in[0] = 0;		// Playing
 else
  data_in[0] = 3;		// Stopped

 data_in[1] = SubQBuf[0];	// Control/adr
 data_in[2] = SubQBuf[1];	// Track
 data_in[3] = SubQBuf[2];	// Index
 data_in[4] = SubQBuf[3];	// M(rel)
 data_in[5] = SubQBuf[4];	// S(rel)
 data_in[6] = SubQBuf[5];	// F(rel)
 data_in[7] = SubQBuf[7];	// M(abs)
 data_in[8] = SubQBuf[8];	// S(abs)
 data_in[9] = SubQBuf[9];	// F(abs)

 DoSimpleDataIn(data_in, alloc_size);
}

static void DoTESTUNITREADY(const uint8 *cdb)
{
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}

static void DoNEC_PAUSE(const uint8 *cdb)
{
 if(cdda.CDDAStatus != CDDASTATUS_STOPPED) // Hmm, should we give an error if it tries to pause and it's already paused?
 {
  cdda.CDDAStatus = CDDASTATUS_PAUSED;
  SendStatusAndMessage(STATUS_GOOD, 0x00);
 }
 else // Definitely give an error if it tries to pause when no track is playing!
 {
  CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_AUDIO_NOT_PLAYING);
 }
}

static void DoNEC_SCAN(const uint8 *cdb)
{
 uint32 sector_tmp = 0;

 // 0: 0xD2
 // 1: 0x03 = reverse scan, 0x02 = forward scan
 // 2: End M
 // 3: End S
 // 4: End F

 switch (cdb[9] & 0xc0)
 {
  default:
   SCSIDBG("Unknown NECSCAN format");
   break;

  case 0x00:
   sector_tmp = (cdb[3] << 16) | (cdb[4] << 8) | cdb[5];
   break;

  case 0x40:
   sector_tmp = AMSF_to_LBA(BCD_to_U8(cdb[2]), BCD_to_U8(cdb[3]), BCD_to_U8(cdb[4]));
   break;

  case 0x80:	// FIXME: error on invalid track number???
   sector_tmp = toc.tracks[BCD_to_U8(cdb[2])].lba;
   break;
 }

 cdda.ScanMode = cdb[1] & 0x3;
 cdda.scan_sec_end = sector_tmp;

 if(cdda.CDDAStatus != CDDASTATUS_STOPPED)
 {
  if(cdda.ScanMode)
  {
   cdda.CDDAStatus = CDDASTATUS_SCANNING;
  }
 }
 SendStatusAndMessage(STATUS_GOOD, 0x00);
}



/********************************************************
*							*
*	SCSI-2 CD Command 0x1E - PREVENT/ALLOW MEDIUM 	*
*				  REMOVAL		*
********************************************************/
static void DoPREVENTALLOWREMOVAL(const uint8 *cdb)
{
 //bool prevent = cdb[4] & 0x01;
 //const int logical_unit = cdb[1] >> 5;
 //SCSIDBG("PREVENT ALLOW MEDIUM REMOVAL: %d for %d\n", cdb[4] & 0x1, logical_unit);
 //SendStatusAndMessage(STATUS_GOOD, 0x00);

 CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_REQUEST_IN_CDB);
}

//
//
//
#include "scsicd-pce-commands.inc"


#define SCF_REQUIRES_MEDIUM	0x0001
#define SCF_INCOMPLETE		0x4000
#define SCF_UNTESTED		0x8000

typedef struct
{
 uint8 cmd;
 uint32 flags;
 void (*func)(const uint8 *cdb);
 const char *pretty_name;
 const char *format_string;
} SCSICH;

static const int32 RequiredCDBLen[16] =
{
 6, // 0x0n
 6, // 0x1n
 10, // 0x2n
 10, // 0x3n
 10, // 0x4n
 10, // 0x5n
 10, // 0x6n
 10, // 0x7n
 10, // 0x8n
 10, // 0x9n
 12, // 0xAn
 12, // 0xBn
 10, // 0xCn
 10, // 0xDn
 10, // 0xEn
 10, // 0xFn
};

static SCSICH PCFXCommandDefs[] =
{
 { 0x00, SCF_REQUIRES_MEDIUM, DoTESTUNITREADY, "Test Unit Ready" },
 { 0x01, 0/* ? */, DoREZEROUNIT, "Rezero Unit" },
 { 0x03, 0, DoREQUESTSENSE, "Request Sense" },
 { 0x08, SCF_REQUIRES_MEDIUM, DoREAD6, "Read(6)" },
 { 0x0B, SCF_REQUIRES_MEDIUM, DoSEEK6, "Seek(6)" },
 { 0x0D, 0, DoNEC_NOP, "No Operation" },
 { 0x12, 0, DoINQUIRY, "Inquiry" },
 { 0x15, 0, DoMODESELECT6, "Mode Select(6)" },
 // TODO: { 0x16, 0 /* ? */, DoRESERVE, "Reserve" },			// 9.2.12
 // TODO: { 0x17, 0 /* ? */, DoRELEASE, "Release" },			// 9.2.11
 { 0x1A, 0, DoMODESENSE6, "Mode Sense(6)" },
 { 0x1B, SCF_REQUIRES_MEDIUM, DoSTARTSTOPUNIT6, "Start/Stop Unit" },		// 9.2.17
 // TODO: { 0x1D, , DoSENDDIAG, "Send Diagnostic" },		// 8.2.15
 { 0x1E, 0, DoPREVENTALLOWREMOVAL, "Prevent/Allow Media Removal" },

 { 0x25, SCF_REQUIRES_MEDIUM, DoREADCDCAP10, "Read CD-ROM Capacity" },	// 14.2.8
 { 0x28, SCF_REQUIRES_MEDIUM, DoREAD10, "Read(10)" },
 { 0x2B, SCF_REQUIRES_MEDIUM, DoSEEK10, "Seek(10)" },

 // TODO: { 0x2F, SCF_REQUIRES_MEDIUM, DoVERIFY10, "Verify(10)" },		// 16.2.11

 { 0x34, SCF_REQUIRES_MEDIUM, DoPREFETCH, "Prefetch" },
 // TODO: { 0x3B, 0, 10, DoWRITEBUFFER, "Write Buffer" },		// 8.2.17
 // TODO: { 0x3C, 0, 10, DoREADBUFFER, "Read Buffer" },		// 8.2.12

 { 0x42, SCF_REQUIRES_MEDIUM, DoREADSUBCHANNEL, "Read Subchannel" },
 { 0x43, SCF_REQUIRES_MEDIUM, DoREADTOC, "Read TOC" },
 { 0x44, SCF_REQUIRES_MEDIUM, DoREADHEADER10, "Read Header" },

 { 0x45, SCF_REQUIRES_MEDIUM, DoPA10, "Play Audio(10)" },
 { 0x47, SCF_REQUIRES_MEDIUM, DoPAMSF, "Play Audio MSF" },
 { 0x48, SCF_REQUIRES_MEDIUM, DoPATI, "Play Audio Track Index" },
 { 0x49, SCF_REQUIRES_MEDIUM, DoPATR10, "Play Audio Track Relative(10)" },
 { 0x4B, SCF_REQUIRES_MEDIUM, DoPAUSERESUME, "Pause/Resume" },

 { 0xA5, SCF_REQUIRES_MEDIUM, DoPA12, "Play Audio(12)" },
 { 0xA8, SCF_REQUIRES_MEDIUM, DoREAD12, "Read(12)" },
 { 0xA9, SCF_REQUIRES_MEDIUM, DoPATR12, "Play Audio Track Relative(12)" },

 // TODO: { 0xAF, SCF_REQUIRES_MEDIUM, DoVERIFY12, "Verify(12)" },		// 16.2.12

 { 0xD2, SCF_REQUIRES_MEDIUM, DoNEC_SCAN, "Scan" },
 { 0xD8, SCF_REQUIRES_MEDIUM, DoNEC_SAPSP, "Set Audio Playback Start Position" }, // "Audio track search"
 { 0xD9, SCF_REQUIRES_MEDIUM, DoNEC_SAPEP, "Set Audio Playback End Position" },   // "Play"
 { 0xDA, SCF_REQUIRES_MEDIUM, DoNEC_PAUSE, "Pause" },			     // "Still"
 { 0xDB, SCF_REQUIRES_MEDIUM | SCF_UNTESTED, DoNEC_SST, "Set Stop Time" },
 { 0xDC, SCF_REQUIRES_MEDIUM, DoNEC_EJECT, "Eject" },
 { 0xDD, SCF_REQUIRES_MEDIUM, DoNEC_READSUBQ, "Read Subchannel Q" },
 { 0xDE, SCF_REQUIRES_MEDIUM, DoNEC_GETDIRINFO, "Get Dir Info" },

 { 0xFF, 0, 0, NULL, NULL },
};

static SCSICH PCECommandDefs[] = 
{
 { 0x00, SCF_REQUIRES_MEDIUM, DoTESTUNITREADY, "Test Unit Ready" },
 { 0x03, 0, DoREQUESTSENSE, "Request Sense" },
 { 0x08, SCF_REQUIRES_MEDIUM, DoREAD6, "Read(6)" },
 //{ 0x15, DoMODESELECT6, "Mode Select(6)" },
 { 0xD8, SCF_REQUIRES_MEDIUM, DoNEC_PCE_SAPSP, "Set Audio Playback Start Position" },
 { 0xD9, SCF_REQUIRES_MEDIUM, DoNEC_PCE_SAPEP, "Set Audio Playback End Position" },
 { 0xDA, SCF_REQUIRES_MEDIUM, DoNEC_PCE_PAUSE, "Pause" },
 { 0xDD, SCF_REQUIRES_MEDIUM, DoNEC_PCE_READSUBQ, "Read Subchannel Q" },
 { 0xDE, SCF_REQUIRES_MEDIUM, DoNEC_PCE_GETDIRINFO, "Get Dir Info" },

 { 0xFF, 0, 0, NULL, NULL },
};

void SCSICD_ResetTS(void)
{
 lastts = 0;
}

void SCSICD_GetCDDAValues(int16 &left, int16 &right)
{
 if(cdda.CDDAStatus)
 {
  left = cdda.CDDASectorBuffer[cdda.CDDAReadPos * 2];
  right = cdda.CDDASectorBuffer[cdda.CDDAReadPos * 2 + 1];
 }
 else
  left = right = 0;
}

static INLINE void RunCDDA(uint32 system_timestamp, int32 run_time)
{
 if(cdda.CDDAStatus == CDDASTATUS_PLAYING || cdda.CDDAStatus == CDDASTATUS_SCANNING)
 {
  int32 sample[2];

  cdda.CDDADiv -= run_time << 16;

  while(cdda.CDDADiv <= 0)
  {
   cdda.CDDADiv += cdda.CDDADivAcc;

   //MDFN_DispMessage("%d %d %d\n", read_sec_start, read_sec, read_sec_end);

   if(cdda.CDDAReadPos == 588)
   {
    if(read_sec >= read_sec_end || (cdda.CDDAStatus == CDDASTATUS_SCANNING && read_sec == cdda.scan_sec_end))
    {
     switch(cdda.PlayMode)
     {
      case PLAYMODE_SILENT:
      case PLAYMODE_NORMAL:
       cdda.CDDAStatus = CDDASTATUS_STOPPED;
       break;

      case PLAYMODE_INTERRUPT:
       cdda.CDDAStatus = CDDASTATUS_STOPPED;
       CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
       break;

      case PLAYMODE_LOOP:
       read_sec = read_sec_start;
       break;
     }

     // If CDDA playback is stopped, break out of our while(CDDADiv ...) loop and don't play any more sound!
     if(cdda.CDDAStatus == CDDASTATUS_STOPPED)
      break;
    }

    // Don't play past the user area of the disc.
    if(read_sec >= toc.tracks[100].lba)
    {
     cdda.CDDAStatus = CDDASTATUS_STOPPED;
     break;
    }

    if(cd.TrayOpen)
    {
     cdda.CDDAStatus = CDDASTATUS_STOPPED;

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


    cdda.CDDAReadPos = 0;

    {
     uint8 tmpbuf[2352 + 96];

     Cur_CDIF->ReadRawSector(tmpbuf, read_sec);	//, read_sec_end, read_sec_start);

     for(int i = 0; i < 588 * 2; i++)
      cdda.CDDASectorBuffer[i] = MDFN_de16lsb(&tmpbuf[i * 2]);

     memcpy(cd.SubPWBuf, tmpbuf + 2352, 96);
    }
    GenSubQFromSubPW();

    if(cdda.CDDAStatus == CDDASTATUS_SCANNING)
    {
     int64 tmp_read_sec = read_sec;

     if(cdda.ScanMode & 1)
     {
      tmp_read_sec -= 24;
      if(tmp_read_sec < cdda.scan_sec_end)
       tmp_read_sec = cdda.scan_sec_end;
     }
     else
     {
      tmp_read_sec += 24;
      if(tmp_read_sec > cdda.scan_sec_end)
       tmp_read_sec = cdda.scan_sec_end;
     }
     read_sec = tmp_read_sec;
    }
    else
     read_sec++;
   } // End    if(CDDAReadPos == 588)

   // If the last valid sub-Q data decoded indicate that the corresponding sector is a data sector, don't output the
   // current sector as audio.
   sample[0] = sample[1] = 0;

   if(!(cd.SubQBuf_Last[0] & 0x40) && cdda.PlayMode != PLAYMODE_SILENT)
   {
    sample[0] += (cdda.CDDASectorBuffer[cdda.CDDAReadPos * 2 + cdda.OutPortChSelectCache[0]] * cdda.OutPortVolumeCache[0]) >> 16;
    sample[1] += (cdda.CDDASectorBuffer[cdda.CDDAReadPos * 2 + cdda.OutPortChSelectCache[1]] * cdda.OutPortVolumeCache[1]) >> 16;
   }

   uint32 synthtime = ((system_timestamp + (cdda.CDDADiv >> 16))) / cdda.CDDATimeDiv;
   if(!(cdda.CDDAReadPos % 6))
   {
    int subindex = cdda.CDDAReadPos / 6 - 2;

    if(subindex >= 0)
     CDStuffSubchannels(cd.SubPWBuf[subindex], subindex);
    else // The system-specific emulation code should handle what value the sync bytes are.
     CDStuffSubchannels(0x00, subindex);
   }

   if(sbuf[0] && sbuf[1])
   {
    cdda.CDDASynth[0].offset_inline(synthtime, sample[0] - cdda.last_sample[0], sbuf[0]);
    cdda.CDDASynth[1].offset_inline(synthtime, sample[1] - cdda.last_sample[1], sbuf[1]);
   }

   cdda.last_sample[0] = sample[0];
   cdda.last_sample[1] = sample[1];

   cdda.CDDAReadPos++;
  }
 }
}

static INLINE void RunCDRead(uint32 system_timestamp, int32 run_time)
{
 if(CDReadTimer > 0)
 {
  CDReadTimer -= run_time;

  if(CDReadTimer <= 0)
  {
   if(din->CanWrite() < ((WhichSystem == SCSICD_PCFX) ? 2352 : 2048))	// +96 if we find out the PC-FX can read subchannel data along with raw data too. ;)
   {
    //printf("Carp: %d %d %d\n", din->CanWrite(), SectorCount, CDReadTimer);
    //CDReadTimer = (cd.data_in_size - cd.data_in_pos) * 10;
    
    CDReadTimer += (uint64) 1 * 2048 * System_Clock / CD_DATA_TRANSFER_RATE;

    //CDReadTimer += (uint64) 1 * 128 * System_Clock / CD_DATA_TRANSFER_RATE;
   }
   else
   {
    uint8 tmp_read_buf[2352 + 96];

    if(cd.TrayOpen)
    {
     din->Flush();
     cd.data_transfer_done = FALSE;

     CommandCCError(SENSEKEY_NOT_READY, NSE_TRAY_OPEN);
    }
    else if(SectorAddr >= toc.tracks[100].lba)
    {
     CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_END_OF_VOLUME);
    }
    else if(!Cur_CDIF->ReadRawSector(tmp_read_buf, SectorAddr))	//, SectorAddr + SectorCount))
    {
     cd.data_transfer_done = FALSE;

     CommandCCError(SENSEKEY_ILLEGAL_REQUEST);
    }
    else if(ValidateRawDataSector(tmp_read_buf, SectorAddr))
    {
     memcpy(cd.SubPWBuf, tmp_read_buf + 2352, 96);

     if(tmp_read_buf[12 + 3] == 0x2)
      din->Write(tmp_read_buf + 24, 2048);
     else
      din->Write(tmp_read_buf + 16, 2048);

     GenSubQFromSubPW();

     CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_READY);

     SectorAddr++;
     SectorCount--;

     if(CurrentPhase != PHASE_DATA_IN)
      ChangePhase(PHASE_DATA_IN);

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
   }				// end else to if(!Cur_CDIF->ReadSector

  }
 }
}


uint32 SCSICD_Run(scsicd_timestamp_t system_timestamp)
{
 int32 run_time = system_timestamp - lastts;

 if(system_timestamp < lastts)
 {
  fprintf(stderr, "Meow: %d %d\n", system_timestamp, lastts);
  assert(system_timestamp >= lastts);
 }

 monotonic_timestamp += run_time;

 lastts = system_timestamp;

 RunCDRead(system_timestamp, run_time);
 RunCDDA(system_timestamp, run_time);

 bool ResetNeeded = false;

 if(RST_signal && !cd.last_RST_signal)
  ResetNeeded = true;

 cd.last_RST_signal = RST_signal;

 if(ResetNeeded)
 {
  //puts("RST");
  VirtualReset();
 }
 else if(CurrentPhase == PHASE_BUS_FREE)
 {
  if(SEL_signal)
  {
   if(WhichSystem == SCSICD_PCFX)
   {
    //if(cd_bus.DB == 0x84)
    {
     ChangePhase(PHASE_COMMAND);
    }
   }
   else // PCE
   {
    ChangePhase(PHASE_COMMAND);
   }
  }
 }
 else if(ATN_signal && !REQ_signal && !ACK_signal)
 {
  //printf("Yay: %d %d\n", REQ_signal, ACK_signal);
  ChangePhase(PHASE_MESSAGE_OUT);
 }
 else switch(CurrentPhase)
 {
  case PHASE_COMMAND:
    if(REQ_signal && ACK_signal)	// Data bus is valid nowww
    {
     //printf("Command Phase Byte I->T: %02x, %d\n", cd_bus.DB, cd.command_buffer_pos);
     cd.command_buffer[cd.command_buffer_pos++] = cd_bus.DB;
     SetREQ(FALSE);
    }

    if(!REQ_signal && !ACK_signal && cd.command_buffer_pos)	// Received at least one byte, what should we do?
    {
     if(cd.command_buffer_pos == RequiredCDBLen[cd.command_buffer[0] >> 4])
     {
      const SCSICH *cmd_info_ptr;

      if(WhichSystem == SCSICD_PCFX)
       cmd_info_ptr = PCFXCommandDefs;
      else
       cmd_info_ptr = PCECommandDefs;

      while(cmd_info_ptr->pretty_name && cmd_info_ptr->cmd != cd.command_buffer[0])
       cmd_info_ptr++;
  
      if(SCSILog)
      {
       char log_buffer[1024];
       int lb_pos;

       log_buffer[0] = 0;
       
       lb_pos = trio_snprintf(log_buffer, 1024, "Command: %02x, %s%s  ", cd.command_buffer[0], cmd_info_ptr->pretty_name ? cmd_info_ptr->pretty_name : "!!BAD COMMAND!!",
			(cmd_info_ptr->flags & SCF_UNTESTED) ? "(UNTESTED)" : "");

       for(int i = 0; i < RequiredCDBLen[cd.command_buffer[0] >> 4]; i++)
        lb_pos += trio_snprintf(log_buffer + lb_pos, 1024 - lb_pos, "%02x ", cd.command_buffer[i]);

       SCSILog("SCSI", "%s", log_buffer);
       //puts(log_buffer);
      }


      if(cmd_info_ptr->pretty_name == NULL)	// Command not found!
      {
       CommandCCError(SENSEKEY_ILLEGAL_REQUEST, NSE_INVALID_COMMAND);

       SCSIDBG("Bad Command: %02x\n", cd.command_buffer[0]);

       if(SCSILog)
        SCSILog("SCSI", "Bad Command: %02x", cd.command_buffer[0]);

       cd.command_buffer_pos = 0;
      }
      else
      {
       if(cmd_info_ptr->flags & SCF_UNTESTED)
       {
        SCSIDBG("Untested SCSI command: %02x, %s", cd.command_buffer[0], cmd_info_ptr->pretty_name);
       }

       if(cd.TrayOpen && (cmd_info_ptr->flags & SCF_REQUIRES_MEDIUM))
       {
	CommandCCError(SENSEKEY_NOT_READY, NSE_TRAY_OPEN);
       }
       else if(!Cur_CDIF && (cmd_info_ptr->flags & SCF_REQUIRES_MEDIUM))
       {
	CommandCCError(SENSEKEY_NOT_READY, NSE_NO_DISC);
       }
       else if(cd.DiscChanged && (cmd_info_ptr->flags & SCF_REQUIRES_MEDIUM))
       {
	CommandCCError(SENSEKEY_UNIT_ATTENTION, NSE_DISC_CHANGED);
	cd.DiscChanged = false;
       }
       else
       {
        cmd_info_ptr->func(cd.command_buffer);
       }

       cd.command_buffer_pos = 0;
      }
     } // end if(cd.command_buffer_pos == RequiredCDBLen[cd.command_buffer[0] >> 4])
     else			// Otherwise, get more data for the command!
      SetREQ(TRUE);
    }
    break;

  case PHASE_DATA_OUT:
    if(REQ_signal && ACK_signal)	// Data bus is valid nowww
    {
     //printf("DATAOUT-SCSIIN: %d %02x\n", cd.data_out_pos, cd_bus.DB);
     cd.data_out[cd.data_out_pos++] = cd_bus.DB;
     SetREQ(FALSE);
    }
    else if(!REQ_signal && !ACK_signal && cd.data_out_pos)
    {
     if(cd.data_out_pos == cd.data_out_size)
     {
      cd.data_out_pos = 0;

      if(cd.command_buffer[0] == 0x15)
	FinishMODESELECT6(cd.data_out, cd.data_out_size);
      else	// Error out here?  It shouldn't be reached:
       SendStatusAndMessage(STATUS_GOOD, 0x00);
     }
     else
      SetREQ(TRUE);
    }
    break;

  
  case PHASE_MESSAGE_OUT:
   //printf("%d %d, %02x\n", REQ_signal, ACK_signal, cd_bus.DB);
   if(REQ_signal && ACK_signal)
   {
    SetREQ(FALSE);

    // ABORT message is 0x06, but the code isn't set up to be able to recover from a MESSAGE OUT phase back to the previous phase, so we treat any message as an ABORT.
    // Real tests are needed on the PC-FX to determine its behavior.
    //  (Previously, ATN emulation was a bit broken, which resulted in the wrong data on the data bus in this code path in at least "Battle Heat", but it's fixed now and 0x06 is on the data bus).
    //if(cd_bus.DB == 0x6)		// ABORT message!
    if(1)
    {
     puts("ABORT");
     din->Flush();
     cd.data_out_pos = cd.data_out_size = 0;

     CDReadTimer = 0;
     cdda.CDDAStatus = CDDASTATUS_STOPPED;
     ChangePhase(PHASE_BUS_FREE);
    }
    else
     printf("Message to target: %02x\n", cd_bus.DB);
   }
   break;


  case PHASE_STATUS:
    if(REQ_signal && ACK_signal)
    {
     SetREQ(FALSE);
     cd.status_sent = TRUE;
    }

    if(!REQ_signal && !ACK_signal && cd.status_sent)
    {
     // Status sent, so get ready to send the message!
     cd.status_sent = FALSE;
     cd_bus.DB = cd.message_pending;

     ChangePhase(PHASE_MESSAGE_IN);
    }
    break;

  case PHASE_DATA_IN:
    if(!REQ_signal && !ACK_signal)
    {
     //puts("REQ and ACK false");
     if(din->CanRead() == 0)	// aaand we're done!
     {
      CDIRQCallback(0x8000 | SCSICD_IRQ_DATA_TRANSFER_READY);

      if(cd.data_transfer_done)
      {
       SendStatusAndMessage(STATUS_GOOD, 0x00);
       cd.data_transfer_done = FALSE;
       CDIRQCallback(SCSICD_IRQ_DATA_TRANSFER_DONE);
      }
     }
     else
     {
      cd_bus.DB = din->ReadByte();
      SetREQ(TRUE);
     }
    }
    if(REQ_signal && ACK_signal)
    {
     //puts("REQ and ACK true");
     SetREQ(FALSE);
    }
    break;

  case PHASE_MESSAGE_IN:
   if(REQ_signal && ACK_signal)
   {
    SetREQ(FALSE);
    cd.message_sent = TRUE;
   }

   if(!REQ_signal && !ACK_signal && cd.message_sent)
   {
    cd.message_sent = FALSE;
    ChangePhase(PHASE_BUS_FREE);
   }
   break;
 }

 int32 next_time = 0x7fffffff;

 if(CDReadTimer > 0 && CDReadTimer < next_time)
  next_time = CDReadTimer;

 if(cdda.CDDAStatus == CDDASTATUS_PLAYING || cdda.CDDAStatus == CDDASTATUS_SCANNING)
 {
  int32 cdda_div_sexytime = (cdda.CDDADiv + 0xFFFF) >> 16;
  if(cdda_div_sexytime > 0 && cdda_div_sexytime < next_time)
   next_time = cdda_div_sexytime;
 }

 assert(next_time >= 0);

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


void SCSICD_SetTransferRate(uint32 TransferRate)
{
 CD_DATA_TRANSFER_RATE = TransferRate;
}

void SCSICD_Close(void)
{
 if(din)
 {
  delete din;
  din = NULL;
 }
}

void SCSICD_Init(int type, int cdda_time_div, Blip_Buffer *leftbuf, Blip_Buffer *rightbuf, uint32 TransferRate, uint32 SystemClock, void (*IRQFunc)(int), void (*SSCFunc)(uint8, int))
{
 Cur_CDIF = NULL;
 cd.TrayOpen = false;

 monotonic_timestamp = 0;
 lastts = 0;

 SCSILog = NULL;

 if(type == SCSICD_PCFX)
  din = new SimpleFIFO<uint8>(65536);	//4096);
 else
  din = new SimpleFIFO<uint8>(2048); //8192); //1024); /2048);

 WhichSystem = type;
 cdda.CDDATimeDiv = cdda_time_div;

 for(int i = 0; i < 2; i++)
 {
  //double meow = (((double)22050 / 48000) - (6.4 / 64 / 2));
  //my_kernel eq(meow);

  cdda.CDDASynth[i].volume(1.0f / 65536);

  cdda.CDDAVolume[i] = 65536; //1.0;
  //CDDASynth[i].treble_eq(eq);
 }

 FixOPV();

 sbuf[0] = leftbuf;
 sbuf[1] = rightbuf;

 CD_DATA_TRANSFER_RATE = TransferRate;
 System_Clock = SystemClock;
 CDIRQCallback = IRQFunc;
 CDStuffSubchannels = SSCFunc;
}

void SCSICD_SetCDDAVolume(double left, double right)
{
 cdda.CDDAVolume[0] = 65536 * left;
 cdda.CDDAVolume[1] = 65536 * right;

 for(int i = 0; i < 2; i++)
 {
  if(cdda.CDDAVolume[i] > 65536)
  {
   printf("Debug Warning: CD-DA volume %d too large: %d\n", i, cdda.CDDAVolume[i]);
   cdda.CDDAVolume[i] = 65536;
  }
 }

 FixOPV();
}

int SCSICD_StateAction(StateMem * sm, int load, int data_only, const char *sname)
{
 SFORMAT StateRegs[] = 
 {
  SFVARN(cd_bus.DB, "DB"),
  SFVARN(cd_bus.signals, "Signals"),
  SFVAR(CurrentPhase),

  //SFVARN(cd_bus.BSY, "BSY"),
  //SFVARN(cd_bus.MSG, "MSG"),
  //SFVARN(cd_bus.CD, "CD"),
  //SFVARN(cd_bus.REQ, "REQ"),
  //SFVARN(cd_bus.IO, "IO"),
  //SFVARN(cd_bus.kingACK, "kingACK"),
  //SFVARN(cd_bus.kingRST, "kingRST"),
  //SFVARN(cd_bus.kingSEL, "kingSEL"),
  //SFVARN(cd_bus.kingATN, "kingATN"),

  SFVARN(cd.last_RST_signal, "last_RST"),
  SFVARN(cd.message_pending, "message_pending"),
  SFVARN(cd.status_sent, "status_sent"),
  SFVARN(cd.message_sent, "message_sent"),
  SFVARN(cd.key_pending, "key_pending"),
  SFVARN(cd.asc_pending, "asc_pending"),
  SFVARN(cd.ascq_pending, "ascq_pending"),
  SFVARN(cd.fru_pending, "fru_pending"),

  SFARRAYN(cd.command_buffer, 256, "command_buffer"),
  SFVARN(cd.command_buffer_pos, "command_buffer_pos"),
  SFVARN(cd.command_size_left, "command_size_left"),

  // Don't save the FIFO's write position, it will be reconstructed from read_pos and in_count
  SFARRAYN(&din->data[0], din->data.size(), "din_fifo"),
  SFVARN(din->read_pos, "din_read_pos"),
  SFVARN(din->in_count, "din_in_count"),
  SFVARN(cd.data_transfer_done, "data_transfer_done"),

  SFARRAYN(cd.data_out, 8192, "data_out"),
  SFVARN(cd.data_out_pos, "data_out_pos"),
  SFVARN(cd.data_out_size, "data_out_size"),

  SFVARN(cd.TrayOpen, "TrayOpen"),
  SFVARN(cd.DiscChanged, "DiscChanged"),

  SFVAR(cdda.PlayMode),
  SFARRAY16(cdda.CDDASectorBuffer, 1176),
  SFVAR(cdda.CDDAReadPos),
  SFVAR(cdda.CDDAStatus),
  SFVAR(cdda.CDDADiv),
  SFVAR(read_sec_start),
  SFVAR(read_sec),
  SFVAR(read_sec_end),

  SFVAR(CDReadTimer),
  SFVAR(SectorAddr),
  SFVAR(SectorCount),

  SFVAR(cdda.ScanMode),
  SFVAR(cdda.scan_sec_end),

  SFARRAYN(&cd.SubQBuf[0][0], sizeof(cd.SubQBuf), "SubQBufs"),
  SFARRAYN(cd.SubQBuf_Last, sizeof(cd.SubQBuf_Last), "SubQBufLast"),
  SFARRAYN(cd.SubPWBuf, sizeof(cd.SubPWBuf), "SubPWBuf"),

  SFVAR(monotonic_timestamp),
  SFVAR(pce_lastsapsp_timestamp),

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
