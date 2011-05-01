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

#include "pcfx.h"
#include "interrupt.h"
#include <trio/trio.h>

static uint16 InterruptAsserted;
static uint16 InterruptMask;
static uint16 InterruptPriority[2];

static void BuildInterruptCache(void)
{
 uint32 iwithmask = InterruptAsserted &~ InterruptMask;
 int InterruptCache = -1;
 int last_prio = -1;

 for(int level = 8; level < 16; level++)
  if(iwithmask & (1 << (15 - level)))
  {
   int tmp_prio;

   if(level >= 12)
    tmp_prio = (InterruptPriority[0] >> ((15 - level) * 3)) & 0x7;
   else
    tmp_prio = (InterruptPriority[1] >> ((11 - level) * 3)) & 0x7;

   if(tmp_prio >= last_prio)
   {
    if(tmp_prio == last_prio)
    {
     FXDBG("Undefined IRQ behavior: %d, %d\n", level, tmp_prio);
    }
    InterruptCache = 8 + tmp_prio;
    last_prio = tmp_prio;
   }
  }

 PCFX_V810.SetInt(InterruptCache);
}

void PCFXIRQ_Assert(int source, bool assert)
{
 assert(source >= 0 && source <= 7);

 InterruptAsserted &= ~(1 << (7 - source));
 
 if(assert)
  InterruptAsserted |= (1 << (7 - source));

 BuildInterruptCache();
}


uint16 PCFXIRQ_Read16(uint32 A)
{
 uint32 ret = 0x00;

 switch(A & 0xC0)
 {
  case 0x00: ret = InterruptAsserted; break;
  case 0x40: ret = InterruptMask; break;
  case 0x80: ret = InterruptPriority[0]; break;
  case 0xC0: ret = InterruptPriority[1]; break;
 }

 return(ret);
}

uint8 PCFXIRQ_Read8(uint32 A)
{
 return(PCFXIRQ_Read16(A&~1) >> ((A & 1) * 8));
}

void PCFXIRQ_Write16(uint32 A, uint16 V)
{
// printf("IRQ Controller Write: %08x %04x\n", A, V);
 switch(A & 0xC0)
 {
  case 0x00: puts("Address error clear");
	     break;

  case 0x40: InterruptMask = V & 0x7F;
	     BuildInterruptCache();
	     break;

  case 0x80: if(InterruptMask == 0x7F)
	     {
	      InterruptPriority[0] = V & 0xFFF;
	      BuildInterruptCache();
	     }
	     break;

  case 0xC0: if(InterruptMask == 0x7F)
	     {
	      InterruptPriority[1] = V & 0x1FF;
	      BuildInterruptCache();
	     }
	     break;
 }
}

int PCFXIRQ_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(InterruptAsserted),
  SFVAR(InterruptMask),
  SFARRAY16(InterruptPriority, 2),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "IRQ");

 if(load)
  BuildInterruptCache();

 return(ret);
}

bool PCFXIRQ_SetRegister(const std::string &name, uint32 value)
{
 if(name == "IMASK")
 {
  InterruptMask = value & 0x7F;
  BuildInterruptCache();
 }
 else if(name == "IPRIO0")
 {
  InterruptPriority[0] = value & 0xFFF;
  BuildInterruptCache();
 }
 else if(name == "IPRIO1")
 {
  InterruptPriority[1] = value & 0x1FF;
  BuildInterruptCache();
 }
 else if(name == "IPEND")
 {
  InterruptAsserted = value;
  BuildInterruptCache();
 }
 return(FALSE);
}

bool PCFXIRQ_GetRegister(const std::string &name, uint32 &value, std::string *special)
{
 if(name == "IMASK")
 {
  value = InterruptMask;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "IRQ Allowed; HuC6273: %s, HuC6270-B: %s, HuC6272: %s, HuC6270-A: %s, Pad: %s, Timer: %s, Reset: %s",
	(InterruptMask & (1 << 0)) ? "No" : "Yes", (InterruptMask & (1 << 1)) ? "No" : "Yes",
	(InterruptMask & (1 << 2)) ? "No" : "Yes", (InterruptMask & (1 << 3)) ? "No" : "Yes",
	(InterruptMask & (1 << 4)) ? "No" : "Yes", (InterruptMask & (1 << 6)) ? "No" : "Yes",
	(InterruptMask & (1 << 7)) ? "No" : "Yes");
   *special = std::string(buf);
  }
  return(TRUE);
 }
 else if(name == "IPRIO0")
 {
  value =  InterruptPriority[0];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "HuC6273: %d, HuC6270-B: %d, HuC6272: %d, HuC6270-A: %d",
 	 (InterruptPriority[0] >> 0) & 0x7, (InterruptPriority[0] >> 3) & 0x7,
	 (InterruptPriority[0] >> 6) & 0x7, (InterruptPriority[0] >> 9) & 0x7);
   *special = std::string(buf);
  }
  return(TRUE);
 }
 else if(name == "IPRIO1")
 {
  value = InterruptPriority[1];
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "Pad: %d, ??: %d, Timer: %d, Reset: %d",
         (InterruptPriority[1] >> 0) & 0x7, (InterruptPriority[1] >> 3) & 0x7,
         (InterruptPriority[1] >> 6) & 0x7, (InterruptPriority[1] >> 9) & 0x7);
   *special = std::string(buf);
  }
  return(TRUE);
 }
 else if(name == "IPEND")
 {
  value = InterruptAsserted;
  if(special)
  {
   char buf[256];
   trio_snprintf(buf, 256, "HuC6273: %d, HuC6270-B: %d, HuC6272: %d, HuC6270-A: %d, Pad: %d, ??: %d, Timer: %d, Reset: %d", (int)(bool)(value & 0x01), (int)(bool)(value & 0x02),
	(int)(bool)(value & 0x04), (int)(bool)(value & 0x08), (int)(bool)(value & 0x10), (int)(bool)(value & 0x20),
	(int)(bool)(value & 0x40), (int)(bool)(value & 0x80));
   *special = std::string(buf);
  }
  return(TRUE);
 }
 else
  return(FALSE);
}

void PCFXIRQ_Reset(void)
{
 InterruptAsserted = 0;
 InterruptMask = 0xFFFF;

 InterruptPriority[0] = 0;
 InterruptPriority[1] = 0;

 BuildInterruptCache();
}

