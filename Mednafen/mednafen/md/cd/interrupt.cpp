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
#include "interrupt.h"

/* SegaCD interrupt levels:
	6 - Sub-channel buffering completed
	5 - CDC, error correction or buffer completed
	4 - CDD, command reception is completed?
	3 - Timer
	2 - Software int from main CPU
	1 - Graphics operation is completed
*/


static uint32 asserted;
static uint32 InterruptMask;

static void Rebuild(void)
{
 int pending_level = 0;
 uint32 awm = asserted & InterruptMask;

 // Level 0 doesn't exist per-se.
 // Level 7 doesn't seem to be used on the SegaCD
 for(int i = 6; i > 0; i--)
 {
  if(awm & (1 << i))
  {
   pending_level = i;
   break;
  }
 }

 C68k_Set_IRQ(&Sub68K, pending_level);
}

uint8 MDCD_InterruptRead(void)
{
 return(InterruptMask);
}

bool MDCD_InterruptGetMask(int level)
{
 return((InterruptMask >> level) & 1);
}

bool MDCD_InterruptGetAsserted(int level)
{
 return((asserted >> level) & 1);
}

void MDCD_InterruptWrite(uint8 V)
{
 InterruptMask = V & 0x7E;
 Rebuild();
}

int MDCD_InterruptAck(int level)
{
 printf("Interrupt: %d\n", level);

 asserted &= ~(1 << level);
 Rebuild();

 return(C68K_INT_ACK_AUTOVECTOR);
}

void MDCD_InterruptAssert(int level, bool status)
{
 assert(status == 0 || status == 1);

 asserted &= ~(1 << level);
 asserted |= status << level;
 Rebuild();
}

void MDCD_InterruptReset(void)
{
 asserted = 0;
 Rebuild();
}

void MDCD_InterruptInit(void)
{


}
