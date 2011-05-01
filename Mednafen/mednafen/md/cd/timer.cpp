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

#include "../shared.h"
#include "cd.h"
#include "timer.h"
#include "interrupt.h"

static uint8 TimerLatch;
static int32 TimerDiv;

void MDCD_Timer_Reset(void)
{
 TimerLatch = 0;
 TimerDiv = 0;
 MDCD_InterruptAssert(3, FALSE);
}

void MDCD_Timer_Run(int32 clocks)
{
 TimerDiv -= clocks;

 while(TimerDiv <= 0)
 {
  if(TimerLatch)
  {
   TimerLatch--;
   if(!TimerLatch)
   {
    MDCD_InterruptAssert(3, TRUE);
   }
  }
  TimerDiv += 384;
 }
}


void MDCD_Timer_Write(uint8 V)
{
 TimerLatch = V;
 // Should we cancel a pending interrupt on writes(or writes of 0)?
}

uint8 MDCD_Timer_Read(void)
{
 return(TimerLatch);
}
