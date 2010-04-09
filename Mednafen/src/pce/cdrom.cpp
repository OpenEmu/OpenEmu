/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2004 Ki
 *  Copyright (C) 2007 Mednafen Team
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

#include <string.h>
#include <vector>

#include "pce.h"
#include "psg.h"
#include "cdrom.h"
#include "adpcm.h"
#include "huc.h"
#include "../cdrom/cdromif.h"
#include "../cdrom/scsicd.h"

//#define PCECD_DEBUG

static double CDDAVolumeSetting; // User setting!

static bool8	bBRAMEnabled;
static uint8	_Port[15];
static uint8 	ACKStatus;
static uint8 	SubChannelData;

extern Blip_Buffer sbuf[2];
static int16 RawPCMVolumeCache[2];

static int32 CurrentCDVolume;
static int32 InitialCdVolume;
static int32 VolumeStep;
static bool8 bFadeIn;
static bool8 bFadeOut;

static int32 FadeClockCount;
static int32 FadeCycle;
static int32 ClearACKDelay;

static int32 lastts;


#ifdef WANT_DEBUGGER

static RegType PCECDRegs[] = 
{
 { "BSY", "SCSI BSY", 1 },
 { "REQ", "SCSI REQ", 1 },
 { "MSG", "SCSI MSG", 1 },
 { "IO", "SCSI IO", 1 },
 { "CD", "SCSI CD", 1 },
 { "SEL", "SCSI SEL", 1 },

 { "ADFREQ", "ADPCM Frequency", 1 },
 { "ADCUR", "ADPCM Current 12-bit Value", 2 },
 { "ADWrAddr", "ADPCM Write Address", 2 },
 { "ADWrNibble", "ADPCM Write Nibble Select", 1 },
 { "ADRdAddr", "ADPCM Read AddresS", 2 },
 { "ADRdNibble", "ADPCM Read Nibble Select", 1 },
 { "", "", 0 },
};

static uint32 PCECD_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name == "BSY")
  value = SCSICD_GetBSY();
 else if(name == "REQ")
  value = SCSICD_GetREQ();
 else if(name == "MSG")
  value = SCSICD_GetMSG();
 else if(name == "CD")
  value = SCSICD_GetCD();
 else if(name == "IO")
  value = SCSICD_GetIO();
 else if(name == "SEL")
  value = SCSICD_GetSEL();
 else
  value = ADPCM_GetRegister(name, special);
 return(value);
}

static RegGroupType PCECDRegsGroup =
{
 PCECDRegs,
 PCECD_GetRegister,
 NULL
};
#endif


static void SyncCDVolume(void)
{
	SCSICD_SetCDDAVolume(0.50f * CurrentCDVolume * CDDAVolumeSetting / 65536, 0.50f * CurrentCDVolume * CDDAVolumeSetting / 65536);
}

static ALWAYS_INLINE void CDFADER_AdvanceClock(int32 cycles) // 21477270 / 3 = 7159090 [Hz] ^ﾃの偲ｼ^ﾃ波^ﾃ真ﾃ膿ﾃで与ﾃ�^ﾃづｩ^ﾃ。
{
        if (bFadeOut || bFadeIn)
        {
                FadeClockCount += cycles;

                while (FadeClockCount >= FadeCycle * pce_overclocked)
                {
                        FadeClockCount -= FadeCycle * pce_overclocked;

                        if (bFadeOut)
                        {
                                if (CurrentCDVolume > 0)
                                {
                                        CurrentCDVolume -= VolumeStep;
                                        if (CurrentCDVolume < 0)
                                        {
                                                CurrentCDVolume = 0;
                                                bFadeOut = FALSE;
                                        }
					SyncCDVolume();
                                }
                        }
                        else if (bFadeIn)
                        {
                                if (CurrentCDVolume < InitialCdVolume)
                                {
                                        CurrentCDVolume += VolumeStep;
                                        if (CurrentCDVolume > InitialCdVolume)
                                        {
                                                CurrentCDVolume = InitialCdVolume;
                                                bFadeIn = FALSE;
                                        }
					SyncCDVolume();
                                }
                        }
                }
        }
}

static void update_irq_state()
{
        uint8           irq = _Port[2] & _Port[0x3] & (0x4|0x8|0x10|0x20|0x40);
        if (irq != 0)
                HuC6280_IRQBegin(MDFN_IQIRQ2);
        else
                HuC6280_IRQEnd(MDFN_IQIRQ2);
}

static void StuffSubchannel(uint8 meow, int subindex)
{
 if(subindex == -2)
  SubChannelData = 0x00;      
 else if(subindex == -1)
  SubChannelData = 0x80;        
 else
  SubChannelData = meow & 0x7F;

 _Port[0x3] |= 0x10;
 update_irq_state();
}

static void CDIRQ(int type)
{
 #ifdef PCECD_DEBUG
 if(type != 0x8000 || _Port[0x3] & 0x60)
  printf("CDIRQ: %d\n", type);
 #endif
 if(type & 0x8000)
 {
  type &= 0x7FFF;
  if(type == SCSICD_IRQ_DATA_TRANSFER_DONE)
   _Port[0x3] &= ~0x20;
  else if(type == SCSICD_IRQ_DATA_TRANSFER_READY)
   _Port[0x3] &= ~0x40;
 }
 else if(type == SCSICD_IRQ_DATA_TRANSFER_DONE)
 {
  _Port[0x3] |= 0x20;
 }
 else if(type == SCSICD_IRQ_DATA_TRANSFER_READY)
 {
  _Port[0x3] |= 0x40;
 }
 update_irq_state();
}

void adpcm_state_notification_callback_function(uint32 adpcmState)
{
	switch (adpcmState)
	{
		case ADPCM_STATE_NORMAL:
			_Port[0x3] &= ~(4 | 8);
			break;

		case ADPCM_STATE_HALF_PLAYED:
			_Port[0x3] |= 4;
			_Port[0x3] &= ~8;
			break;

		case ADPCM_STATE_FULL_PLAYED:
			_Port[0x3] &= ~4;
			_Port[0x3] |= 8;
			break;

		case ADPCM_STATE_STOPPED:
			_Port[0x3] &= ~4;
			_Port[0x3] |= 8;
			break;
	}
	update_irq_state();
}

static uint8 read_1808(void)
{
 uint8 ret = SCSICD_GetDB();

 if(!PCE_InDebug)
 {
  if(SCSICD_GetREQ() && !SCSICD_GetACK() && !SCSICD_GetCD())
  {
   if(SCSICD_GetIO())
   {
    SCSICD_SetACK(TRUE);
    ACKStatus = TRUE;
    SCSICD_Run(HuCPU.timestamp);
    ClearACKDelay = 15;
   }
  }
 }

 return(ret);
}

int32 PCECD_Init()
{
	if (!CDIF_Init())
		return -1;

	CDDAVolumeSetting = (double)MDFN_GetSettingUI("pce.cddavolume");

	if(CDDAVolumeSetting != 100)
	{
	 MDFN_printf(_("CD-DA Volume: %d%%\n"), (int)CDDAVolumeSetting);
	}

	CDDAVolumeSetting /= 100;


        CurrentCDVolume = InitialCdVolume = 65536;
        VolumeStep      = InitialCdVolume / 100;

        bFadeOut = FALSE;
        bFadeIn  = FALSE;

	ADPCM_SetNotificationFunction(adpcm_state_notification_callback_function);

	SCSICD_Init(SCSICD_PCE, 1 * pce_overclocked, &sbuf[0], &sbuf[1], 126000 * MDFN_GetSettingUI("pce.cdspeed"), 7159091 * pce_overclocked, CDIRQ, StuffSubchannel);

	SyncCDVolume();


	#ifdef WANT_DEBUGGER
	MDFNDBG_AddRegGroup(&PCECDRegsGroup);
	#endif
	return 0;
}


void PCECD_Close(void)
{
	ADPCM_Close();
	CDIF_Deinit();
}


void PCECD_Power(void)
{
        HuC6280_IRQEnd(MDFN_IQIRQ2);
	SCSICD_Power();

        bBRAMEnabled = FALSE;
        memset(_Port, 0, sizeof(_Port));
	ACKStatus = 0;
	ClearACKDelay = 0;

	bFadeIn = 0;
	bFadeOut = 0;

	ADPCM_Power();
}

bool PCECD_IsBRAMEnabled()
{
	return bBRAMEnabled;
}

uint8 PCECD_Read(uint32 A)
{
 uint8 ret = 0;

 if((A & 0x18c0) == 0x18c0)
 {
  switch (A & 0x18cf)
  {
   case 0x18c1: ret = 0xaa; break;
   case 0x18c2:	ret = 0x55; break;
   case 0x18c3: ret = 0x00; break;
   case 0x18c5:	ret = 0xaa; break;
   case 0x18c6: ret = 0x55; break;
   case 0x18c7:	ret = 0x03; break;
  }
 }
 else switch(A & 0xf)
 {
  case 0x0:
   ret = 0;
   ret |= SCSICD_GetBSY() ? 0x80 : 0x00;
   ret |= SCSICD_GetREQ() ? 0x40 : 0x00;
   ret |= SCSICD_GetMSG() ? 0x20 : 0x00;
   ret |= SCSICD_GetCD() ? 0x10 : 0x00;
   ret |= SCSICD_GetIO() ? 0x08 : 0x00;
   break;

  case 0x1: ret = SCSICD_GetDB(); break;

  case 0x2: ret = _Port[2]; break; //ret = _Port[2] & 0x7f; ret |= SCSICD_GetACK() ? 0x80 : 0x00; break;

  case 0x3:
   bBRAMEnabled = FALSE;

   /* switch left/right of digitized cd playback */
   ret = _Port[0x3];
   if(!PCE_InDebug)
    _Port[0x3] ^= 2;
   break;

  case 0x4: ret = _Port[4]; break;

  case 0x5:
   if(_Port[0x3] & 0x2)
    ret = RawPCMVolumeCache[1]&0xff;	// Right
   else
    ret = RawPCMVolumeCache[0]&0xff;	// Left
   break;

  case 0x6:
   if(_Port[0x3] & 0x2)
    ret = ((uint16)RawPCMVolumeCache[1]) >> 8;	// Right
   else
    ret = ((uint16)RawPCMVolumeCache[0]) >> 8;	// Left
   break;

  case 0x7: 
   if(!PCE_InDebug)
   {
    _Port[0x3] &= ~0x10;
    update_irq_state();
   }
   ret = SubChannelData;
   break;

  case 0x8:
   ret = read_1808();
   break;

  case 0xa: 
   ret = ADPCM_ReadBuffer();
   break;

  case 0xb: 
   ret = _Port[0xb];
   break;

  case 0xc:
   ret = 0x00;

   ret |= ADPCM_IsPlaying() ? 0x8 : 0x1;
   ret |= ADPCM_IsWritePending() ? 0x04 : 0x00;
   ret |= ADPCM_IsBusyReading() ? 0x80 : 0x00;
   break;   

  case 0xd: 
   ret = ADPCM_Read180D();
   break;
 }

 #ifdef PCECD_DEBUG
 printf("Read: %04x %02x, %d\n", A, ret, HuCPU.timestamp);
 #endif

 return(ret);
}


bool PCECD_DoADFun(uint8 *data)
{
 if(_Port[0xb] & 0x3)
 {
  if(!SCSICD_GetCD() && SCSICD_GetIO() && SCSICD_GetREQ() && !SCSICD_GetACK())
  {
   *data = read_1808(); //read_1801();
   return(1);
  }
 }
 return(0);
}

static void CDFADER_FadeOut(int32 ms)
{
        if (ms == 0)
        {
                CurrentCDVolume = 0;
                bFadeOut = FALSE;
                bFadeIn  = FALSE;
                FadeCycle = 0;
		SyncCDVolume();
        }
        else if (CurrentCDVolume > 0)
        {
                FadeCycle = (int32)(((7159090.0 / ((double)CurrentCDVolume / (double)VolumeStep)) * (double)ms) / 1000.0);
                bFadeOut       = TRUE;
                bFadeIn        = FALSE;
        }
        else
        {
                bFadeOut = FALSE;
                bFadeIn  = FALSE;
                FadeCycle = 0;
        }
}

static void CDFADER_FadeIn(int32 ms)
{
        if (ms == 0)
        {
                CurrentCDVolume = InitialCdVolume;
                bFadeOut = FALSE;
                bFadeIn  = FALSE;
                FadeCycle = 0;
		SyncCDVolume();
        }
        else if (InitialCdVolume - CurrentCDVolume > 0)
        {
                FadeCycle = (int32)(((7159090.0 / (((double)InitialCdVolume - (double)CurrentCDVolume) / (double)VolumeStep)) * (double)ms) / 1000.0);
                bFadeOut = FALSE;
                bFadeIn  = TRUE;
        }
        else
        {
                bFadeOut = FALSE;
                bFadeIn  = FALSE;
                FadeCycle = 0;
        }
}

void PCECD_Write(uint32	physAddr, uint8 data)
{
	#ifdef PCECD_DEBUG
	printf("Write: (PC=%04x, t=%6d) %04x %02x; MSG: %d, REQ: %d, ACK: %d, CD: %d, IO: %d, BSY: %d, SEL: %d\n", HuCPU.PC, HuCPU.timestamp, physAddr, data, SCSICD_GetMSG(), SCSICD_GetREQ(), SCSICD_GetACK(), SCSICD_GetCD(), SCSICD_GetIO(), SCSICD_GetBSY(), SCSICD_GetSEL());
	#endif

	switch (physAddr & 0xf)
	{
		case 0x0:
			SCSICD_SetSEL(1);
			SCSICD_Run(HuCPU.timestamp);
			SCSICD_SetSEL(0);
			SCSICD_Run(HuCPU.timestamp);

			/* reset irq status */
			_Port[0x3] &= ~0x70;
			update_irq_state();
			return;

		case 0x1:		// $1801
			_Port[1] = data;
			SCSICD_SetDB(data);
			SCSICD_Run(HuCPU.timestamp);
			return;

		case 0x2:		// $1802
			#ifdef PCECD_DEBUG
			if(!(_Port[0x3] & _Port[2] & 0x40) && (_Port[0x3] & data & 0x40))
			 puts("IRQ on waah 0x40");
			if(!(_Port[0x3] & _Port[2] & 0x20) && (_Port[0x3] & data & 0x20))
			 puts("IRQ on waah 0x20");
			#endif

			SCSICD_SetACK(data & 0x80);
			SCSICD_Run(HuCPU.timestamp);
			_Port[2] = data;
			ACKStatus = (bool)(data & 0x80);
			update_irq_state();
			return;

		case 0x3:		// read only
			puts("Zoom1");
			return;

		case 0x4:
			SCSICD_SetRST(data & 0x2);
			SCSICD_Run(HuCPU.timestamp);
			if(data & 0x2)
			{
				_Port[0x3] &= ~0x70;
				update_irq_state();
			}
			_Port[4] = data;
			return;

		case 0x5:
		case 0x6:
			 {
			  int16 left, right;
 			  SCSICD_GetCDDAValues(left, right);
			  RawPCMVolumeCache[0] = (abs(left) * CurrentCDVolume) >> 16;
			  RawPCMVolumeCache[1] = (abs(right) * CurrentCDVolume) >> 16;
			 }
			 break;

		case 0x7:	// $1807: D7=1 enables backup ram 
			if (data & 0x80)
			{
				bBRAMEnabled = TRUE;
			}
			return;
	
		case 0x8:
			ADPCM_SetAddrLo(data);
			return;

		case 0x9:
			ADPCM_SetAddrHi(data);
			return;

		case 0xa:
			ADPCM_WriteBuffer(data);
			return;

		case 0xb:	// adpcm dma
                        _Port[0xb] = data;
			return;

		case 0xc:		// read-only
			puts("Zoom2");
			return;

		case 0xd:
			ADPCM_Write180D(data);
			return;

		case 0xe:		// Set ADPCM playback rate
			ADPCM_SetFreq(data & 0xF);
			return;

		case 0xf:
			//printf("Fade: %02x\n", data & 0xF);
			switch (data & 0xf)
			{
				case 0:	// フェードアウト解除 
					CDFADER_FadeIn(0);
					ADPCM_FadeIn(0);
					break;

				case 8:	// fade out CD (6[s])
				case 9:
					CDFADER_FadeOut(6000);
					break;

				case 0xa: // fade out ADPCM (6[s])
					//PRINTF("ADPCM fade (6[s])");
					ADPCM_FadeOut(6000);
					break;

				case 0xc:
				case 0xd:
					CDFADER_FadeOut(2500);
					ADPCM_FadeIn(0);
					break;

				case 0xe: // fade out ADPCM (2.5[s])
					//PRINTF("ADPCM fade (2.5[s])");
					ADPCM_FadeOut(2500);
					break;
			}
			return;
	}
}

void PCECD_Run(void)
{
 int32 clocks = HuCPU.timestamp - lastts;

 CDFADER_AdvanceClock(clocks);

 if(ClearACKDelay > 0)
 {
  ClearACKDelay -= clocks;
  if(ClearACKDelay <= 0)
  {
   ACKStatus = FALSE;
   SCSICD_SetACK(FALSE);
   SCSICD_Run(HuCPU.timestamp);
   if(SCSICD_GetCD())
   {
    _Port[0xb] &= ~1;
    #ifdef PCECD_DEBUG
    puts("DMA End");
    #endif
   }
  }
 }

 ADPCM_Run(clocks);
 SCSICD_Run(HuCPU.timestamp);

 lastts = HuCPU.timestamp;
}

void PCECD_ResetTS(void)
{
 SCSICD_ResetTS();
 lastts = 0;
}

int PCECD_StateAction(StateMem *sm, int load, int data_only)
{
	SFORMAT StateRegs[] =
	{
	 SFVAR(bBRAMEnabled),
	 SFVAR(CurrentCDVolume),
	 SFVAR(bFadeIn),
	 SFVAR(bFadeOut),
	 SFVAR(FadeClockCount),
	 SFVAR(FadeCycle),
	 SFVAR(ACKStatus),
	 SFVAR(ClearACKDelay),
	 SFARRAY16(RawPCMVolumeCache, 2),
	 SFARRAY(_Port, sizeof(_Port)),
	 SFEND
	};

        int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "PECD");
	ret &= SCSICD_StateAction(sm, load, data_only, "CDRM");

	if(load)
	{
	 SyncCDVolume();
	 //SCSICD_SetDB(_Port[1]);
	 SCSICD_SetACK(ACKStatus);
         SCSICD_SetRST(_Port[4] & 0x2);
	}
	return(ret);
}
