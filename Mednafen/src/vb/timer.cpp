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

#include "vb.h"
#include "timer.h"

namespace MDFN_IEN_VB
{

#define TC_TENABLE	0x01
#define TC_ZSTAT	0x02
#define TC_ZSTATCLR	0x04
#define TC_TIMZINT	0x08
#define TC_TCLKSEL	0x10

static uint8 TimerControl;
static uint16 TimerReloadValue;
static uint16 TimerCounter;
static int32 TimerDivider;
static v810_timestamp_t TimerLastTS;
static bool TimerStatus, TimerStatusShadow;
static bool ReloadPending;

v810_timestamp_t TIMER_Update(v810_timestamp_t timestamp)
{
 int32 run_time = timestamp - TimerLastTS;

 if(TimerControl & TC_TENABLE)
 {
  TimerDivider -= run_time;
  while(TimerDivider <= 0)
  {
   if(!TimerCounter || ReloadPending)
   {
    TimerCounter = TimerReloadValue;
    ReloadPending = false;
   }
   
   if(TimerCounter)
    TimerCounter--;

   if(!TimerCounter || TimerStatus)
   {
    TimerStatusShadow = TimerStatus = true;
   }

   VBIRQ_Assert(VBIRQ_SOURCE_TIMER, TimerStatusShadow && (TimerControl & TC_TIMZINT));
   TimerDivider += (TimerControl & TC_TCLKSEL) ? 500 : 2000;
  }
 }

 TimerLastTS = timestamp;

 return((TimerControl & TC_TENABLE) ? (timestamp + TimerDivider) : VB_EVENT_NONONO);
}

void TIMER_ResetTS(void)
{
 TimerLastTS = 0;
}

uint8 TIMER_Read(const v810_timestamp_t &timestamp, uint32 A)
{
 uint8 ret = 0;

 //if(A <= 0x1C)
 //printf("Read: %d, %08x\n", timestamp, A);
 TIMER_Update(timestamp);

 switch(A & 0xFF)
 {
  case 0x18: ret = TimerCounter;
	     break;

  case 0x1C: ret = TimerCounter >> 8;
	     break;

  case 0x20: ret = TimerControl | (0xE0 | TC_ZSTATCLR) | (TimerStatus ? TC_ZSTAT : 0);
	     break;

 }

 return(ret);
}

void TIMER_Write(const v810_timestamp_t &timestamp, uint32 A, uint8 V)
{
 if(A & 0x3)
 {
  puts("HWCtrl Bogus Write?");
  return;
 }

 TIMER_Update(timestamp);

 //if((A & 0xFF) <= 0x1C)
   //printf("Write: %d, %08x %02x\n", timestamp, A, V);

 switch(A & 0xFF)
 {
  case 0x18: TimerReloadValue &= 0xFF00;
	     TimerReloadValue |= V;
	     ReloadPending = true;
	     break;

  case 0x1C: TimerReloadValue &= 0x00FF;
	     TimerReloadValue |= V << 8;
	     ReloadPending = true;
	     break;

  case 0x20: if(V & TC_ZSTATCLR)
	     {
	      if((TimerControl & TC_TENABLE) && TimerCounter == 0)
	      {
	       //puts("Faulty Z-Stat-Clr");
	      }
	      else
	      {
	       TimerStatus = false;
	      }
	      TimerStatusShadow = false;
	     }
	     if((V & TC_TENABLE) && !(TimerControl & TC_TENABLE))
	     {
	      //TimerCounter = TimerReloadValue;
	      TimerDivider = (V & TC_TCLKSEL) ? 500 : 2000;
	     }
	     TimerControl = V & (0x10 | 0x08 | 0x01);

	     if(!(TimerControl & TC_TIMZINT))
	      TimerStatus = TimerStatusShadow = false;

             VBIRQ_Assert(VBIRQ_SOURCE_TIMER, TimerStatusShadow && (TimerControl & TC_TIMZINT));

	     if(TimerControl & TC_TENABLE)
	      VB_SetEvent(VB_EVENT_TIMER, timestamp + TimerDivider);
	     break;
 }
}

void TIMER_Power(void)
{
 TimerLastTS = 0;

 TimerCounter = 0xFFFF;
 TimerReloadValue = 0xFFFF;
 TimerDivider = 2000;	//2150;	//2000;

 TimerStatus = false;
 TimerStatusShadow = false;
 TimerControl = 0;

 ReloadPending = false;

 VBIRQ_Assert(VBIRQ_SOURCE_TIMER, false);
}

int TIMER_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(TimerCounter),
  SFVAR(TimerReloadValue),
  SFVAR(TimerDivider),
  SFVAR(TimerStatus),
  SFVAR(TimerStatusShadow),
  SFVAR(TimerControl),
  SFVAR(ReloadPending),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "TIMER");

 if(load)
 {

 }

 return(ret);
}

uint32 TIMER_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 ret = 0xDEADBEEF;

 switch(id)
 {
  case TIMER_GSREG_TCR:
	ret = TimerControl;
	if(special)
	 trio_snprintf(special, special_len, "TEnable: %d, TimZInt: %d, TClkSel: %d(%.3f KHz)",
		(int)(bool)(ret & TC_TENABLE),
		(int)(bool)(ret & TC_TIMZINT),
		(int)(bool)(ret & TC_TCLKSEL),
		(double)VB_MASTER_CLOCK / ((ret & TC_TCLKSEL) ? 500 : 2000) / 1000 );
	break;

  case TIMER_GSREG_DIVCOUNTER:
	ret = TimerDivider;
	break;
	
  case TIMER_GSREG_RELOAD_VALUE:
	ret = TimerReloadValue;
	break;

  case TIMER_GSREG_COUNTER:
	ret = TimerCounter;
	break;

 }
 return(ret);
}

void TIMER_SetRegister(const unsigned int id, const uint32 value)
{
 switch(id)
 {
  case TIMER_GSREG_TCR:
	TimerControl = value & (TC_TENABLE | TC_TIMZINT | TC_TCLKSEL);
	break;

  case TIMER_GSREG_DIVCOUNTER:
        TimerDivider = value % ((TimerControl & TC_TCLKSEL) ? 500 : 2000);
        break;

  case TIMER_GSREG_RELOAD_VALUE:
        TimerReloadValue = value;
        break;

  case TIMER_GSREG_COUNTER:
        TimerCounter = value;
        break;

 }


}


}
