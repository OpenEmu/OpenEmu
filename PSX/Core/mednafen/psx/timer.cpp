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

#include "psx.h"
#include "timer.h"

/*
 Notes(some of it may be incomplete or wrong in subtle ways)

 Control bits:
	Lower 3 bits of mode, for timer1:
		0x1 = don't count during vblank
		0x3 = vblank going inactive triggers timer reset
		0x5 = vblank going inactive triggers timer reset, and only count within vblank.
		0x7 = Wait until vblank goes active then inactive, then start counting?
	For timer2:
		0x1 = timer stopped(TODO: confirm on real system)

	Target mode enabled		0x008
	IRQ enable			0x010
	--?Affects 0x400 status flag?-- 0x020
	IRQ evaluation auto-reset	0x040
	--unknown--			0x080
	Clock selection			0x100
	Divide by 8(timer 2 only?)	0x200

 Counter:
	Reset to 0 on writes to the mode/status register.

 Status flags:
	Unknown flag 		      0x0400
	Compare flag		      0x0800
		Cleared on mode/status read.
		Set when:	//ever Counter == 0(proooobably, need to investigate lower 3 bits in relation to this).
			

	Overflow/Carry flag	      0x1000
		Cleared on mode/status read.
		Set when counter overflows from 0xFFFF->0.

 Hidden flags:
	IRQ done
		Cleared on writes to the mode/status register, on writes to the count register, and apparently automatically when the counter
		increments if (Mode & 0x40) [Note: If target mode is enabled, and target is 0, IRQ done flag won't be automatically reset]

 There seems to be a brief period(edge condition?) where, if count to target is enabled, you can (sometimes?) read the target value in the count
 register before it's reset to 0.  I doubt any games rely on this, but who knows.  Maybe a PSX equivalent of the PC Engine "Battle Royale"? ;)

 When the counter == 0, the compare flag is set.  An IRQ will be generated if (Mode & 0x10), and the hidden IRQ done flag will be set.
*/

/*
 Dec. 26, 2011 Note
	Due to problems I've had with my GPU timing test program, timer2 appears to be unreliable(clocks are skipped?) when target mode is enabled and the full
	33MHz clock is used(rather than 33MHz / 8).  TODO: Investigate further and confirm(or not).
*/

/*
 FIXME: Clock appropriately(and update events) when using SetRegister() via the debugger.

 TODO: If we ever return randomish values to "simulate" open bus, remember to change the return type and such of the TIMER_Read() function to full 32-bit too.
*/

namespace MDFN_IEN_PSX
{

struct Timer
{
 uint32 Mode;
 int32 Counter;	// Only 16-bit, but 32-bit here for detecting counting past target.
 int32 Target;

 int32 Div8Counter;

 bool IRQDone;
 int32 DoZeCounting;
};

static bool vblank;
static bool hretrace;
static Timer Timers[3];
static pscpu_timestamp_t lastts;

static int32 CalcNextEvent(int32 next_event)
{
 for(int i = 0; i < 3; i++)
 {
  int32 target;
  int32 count_delta;

  if((i == 0 || i == 1) && (Timers[i].Mode & 0x100))	// If clocked by GPU, abort for this timer(will result in poor granularity for pixel-clock-derived timer IRQs, but whatever).
   continue;

  if(!(Timers[i].Mode & 0x10))	// If IRQ is disabled, abort for this timer.
   continue;

  if((Timers[i].Mode & 0x8) && (Timers[i].Counter == 0) && (Timers[i].Target == 0) && !Timers[i].IRQDone)
  {
   next_event = 1;
   continue;
  }

  target = ((Timers[i].Mode & 0x8) && (Timers[i].Counter < Timers[i].Target)) ? Timers[i].Target : 0x10000;

  count_delta = target - Timers[i].Counter;
  if(count_delta <= 0)
  {
   fprintf(stderr, "timer %d count_delta <= 0!!! %d %d\n", i, target, Timers[i].Counter);
   continue;
  }

  {
   int32 tmp_clocks;

   if(Timers[i].DoZeCounting <= 0)
    continue;

   if((i == 0x2) && (Timers[i].Mode & 0x1))
    continue;

   if((i == 0x2) && (Timers[i].Mode & 0x200))
   {
    assert(Timers[i].Div8Counter >= 0 && Timers[i].Div8Counter < 8);
    tmp_clocks = ((count_delta - 1) * 8) + (8 - Timers[i].Div8Counter);
   }
   else
    tmp_clocks = count_delta;

   assert(tmp_clocks > 0);

   if(next_event > tmp_clocks)
    next_event = tmp_clocks;
  }
 }

 return(next_event);
}

static void ClockTimer(int i, uint32 clocks)
{
 int32 before = Timers[i].Counter;
 int32 target = 0x10000;
 bool zero_tm = false;

 if(Timers[i].DoZeCounting <= 0)
  clocks = 0;

 if(i == 0x2)
 {
  uint32 d8_clocks;

  Timers[i].Div8Counter += clocks;
  d8_clocks = Timers[i].Div8Counter >> 3;
  Timers[i].Div8Counter -= d8_clocks << 3;

  if(Timers[i].Mode & 0x200)	// Divide by 8, at least for timer 0x2
   clocks = d8_clocks;

  if(Timers[i].Mode & 1)
   clocks = 0;
 }

 if(Timers[i].Mode & 0x008)
  target = Timers[i].Target;

 if(target == 0 && Timers[i].Counter == 0)
  zero_tm = true;
 else
  Timers[i].Counter += clocks;

 if(clocks && (Timers[i].Mode & 0x40))
  Timers[i].IRQDone = false;

 if((before < target && Timers[i].Counter >= target) || zero_tm || Timers[i].Counter > 0xFFFF)
 {
#if 1
  if(Timers[i].Mode & 0x10)
  {
   if((Timers[i].Counter - target) > 3)
    printf("Timer %d IRQ trigger error: %d\n", i, Timers[i].Counter - target);
  }

#endif


  Timers[i].Mode |= 0x0800;

  if(Timers[i].Counter > 0xFFFF)
  {
   Timers[i].Counter -= 0x10000;

   if(target == 0x10000)
    Timers[i].Mode |= 0x1000;

   if(!target)
    Timers[i].Counter = 0;
  }

  if(target)
   Timers[i].Counter -= (Timers[i].Counter / target) * target;

  if((Timers[i].Mode & 0x10) && !Timers[i].IRQDone)
  {
   Timers[i].IRQDone = true;

   IRQ_Assert(IRQ_TIMER_0 + i, true);
   IRQ_Assert(IRQ_TIMER_0 + i, false);
  }

  if(Timers[i].Counter && (Timers[i].Mode & 0x40))
   Timers[i].IRQDone = false;
 }

}

void TIMER_SetVBlank(bool status)
{
 switch(Timers[1].Mode & 0x7)
 {
  case 0x1:
	Timers[1].DoZeCounting = !status;
	break;

  case 0x3:
	if(vblank && !status)
	 Timers[1].Counter = 0;
	break;

  case 0x5:
	Timers[1].DoZeCounting = status;
	if(vblank && !status)
	 Timers[1].Counter = 0;
	break;

  case 0x7:
	if(Timers[1].DoZeCounting == -1)
	{
	 if(!vblank && status)
	  Timers[1].DoZeCounting = 0;
	}
	else if(Timers[1].DoZeCounting == 0)
	{
	 if(vblank && !status)
	  Timers[1].DoZeCounting = 1;
	}
	break;
 }
 vblank = status;
}

void TIMER_SetHRetrace(bool status)
{
 if(hretrace && !status)
 {
  if((Timers[0].Mode & 0x7) == 0x3)
   Timers[0].Counter = 0;
 }

 hretrace = status;
}

void TIMER_AddDotClocks(uint32 count)
{
 if(Timers[0].Mode & 0x100)
  ClockTimer(0, count);
}

void TIMER_ClockHRetrace(void)
{
 if(Timers[1].Mode & 0x100)
  ClockTimer(1, 1);
}

pscpu_timestamp_t TIMER_Update(const pscpu_timestamp_t timestamp)
{
 int32 cpu_clocks = timestamp - lastts;

 for(int i = 0; i < 3; i++)
 {
  uint32 timer_clocks = cpu_clocks;

  if(Timers[i].Mode & 0x100)
   continue;

  ClockTimer(i, timer_clocks);
 }

 lastts = timestamp;

 return(timestamp + CalcNextEvent(1024));
}

static void CalcCountingStart(unsigned which)
{
 Timers[which].DoZeCounting = true;

 switch(which)
 {
  case 1:
	switch(Timers[which].Mode & 0x07)
	{
	 case 0x1:
		Timers[which].DoZeCounting = !vblank;
		break;

	 case 0x5:
		Timers[which].DoZeCounting = vblank;
		break;

	 case 0x7:
		Timers[which].DoZeCounting = -1;
		break;
	}
	break;


 }
}

void TIMER_Write(const pscpu_timestamp_t timestamp, uint32 A, uint16 V)
{
 TIMER_Update(timestamp);

 int which = (A >> 4) & 0x3;

 V <<= (A & 3) * 8;

 PSX_DBGINFO("[TIMER] Write: %08x %04x\n", A, V);

 if(which >= 3)
  return;

 // TODO: See if the "Timers[which].Counter" part of the IRQ if() statements below is what a real PSX does.
 switch(A & 0xC)
 {
  case 0x0: Timers[which].IRQDone = false;
#if 1
	    if(Timers[which].Counter && (V & 0xFFFF) == 0)
	    {
	     Timers[which].Mode |= 0x0800;
	     if((Timers[which].Mode & 0x10) && !Timers[which].IRQDone)
	     {
	      Timers[which].IRQDone = true;
	      IRQ_Assert(IRQ_TIMER_0 + which, true);
	      IRQ_Assert(IRQ_TIMER_0 + which, false);
	     }
	    }
#endif
	    Timers[which].Counter = V & 0xFFFF;
	    break;

  case 0x4: Timers[which].Mode = (V & 0x3FF) | (Timers[which].Mode & 0x1C00);
	    Timers[which].IRQDone = false;
#if 1
	    if(Timers[which].Counter)
	    {
	     Timers[which].Mode |= 0x0800;
	     if((Timers[which].Mode & 0x10) && !Timers[which].IRQDone)
	     {
	      Timers[which].IRQDone = true;
	      IRQ_Assert(IRQ_TIMER_0 + which, true);
	      IRQ_Assert(IRQ_TIMER_0 + which, false);
	     }
	    }
	    Timers[which].Counter = 0;
#endif
	    CalcCountingStart(which);	// Call after setting .Mode
	    break;

  case 0x8: Timers[which].Target = V & 0xFFFF;
	    break;

  case 0xC: // Open bus
	    break;
 }

 // TIMER_Update(timestamp);

 PSX_SetEventNT(PSX_EVENT_TIMER, timestamp + CalcNextEvent(1024));
}

uint16 TIMER_Read(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint16 ret = 0;
 int which = (A >> 4) & 0x3;

 if(which >= 3)
 {
  PSX_WARNING("[TIMER] Open Bus Read: 0x%08x", A);

  return(ret >> ((A & 3) * 8));
 }

 TIMER_Update(timestamp);

 switch(A & 0xC)
 {
  case 0x0: ret = Timers[which].Counter;
	    break;

  case 0x4: ret = Timers[which].Mode;
	    Timers[which].Mode &= ~0x1800;
	    break;

  case 0x8: ret = Timers[which].Target;
	    break;

  case 0xC: PSX_WARNING("[TIMER] Open Bus Read: 0x%08x", A);
	    break;
 }

 return(ret >> ((A & 3) * 8));
}


void TIMER_ResetTS(void)
{
 lastts = 0;
}


void TIMER_Power(void)
{
 lastts = 0;

 hretrace = false;
 vblank = false;
 memset(Timers, 0, sizeof(Timers));
}

int TIMER_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
#define SFTIMER(n)	SFVARN(Timers[n].Mode, #n "Mode"),			\
			SFVARN(Timers[n].Counter, #n "Counter"),		\
			SFVARN(Timers[n].Target, #n "Target"),			\
			SFVARN(Timers[n].Div8Counter, #n "Div8Counter"),	\
			SFVARN(Timers[n].IRQDone, #n "IRQDone")
  SFTIMER(0),
  SFTIMER(1),
  SFTIMER(2),
#undef SFTIMER
  SFVAR(lastts),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "TIMER");

 if(load)
 {

 }

 return(ret);
}

uint32 TIMER_GetRegister(unsigned int which, char *special, const uint32 special_len)
{
 int tw = (which >> 4) & 0x3;
 uint32 ret = 0;

 switch(which & 0xF)
 {
  case TIMER_GSREG_COUNTER0:
	ret = Timers[tw].Counter;
	break;

  case TIMER_GSREG_MODE0:
	ret = Timers[tw].Mode;
	break;

  case TIMER_GSREG_TARGET0:
	ret = Timers[tw].Target;
	break;
 }

 return(ret);
}

void TIMER_SetRegister(unsigned int which, uint32 value)
{
 int tw = (which >> 4) & 0x3;

 switch(which & 0xF)
 {
  case TIMER_GSREG_COUNTER0:
	Timers[tw].Counter = value & 0xFFFF;
	break;

  case TIMER_GSREG_MODE0:
	Timers[tw].Mode = value & 0xFFFF;
	break;

  case TIMER_GSREG_TARGET0:
	Timers[tw].Target = value & 0xFFFF;
	break;
 }

}


}
