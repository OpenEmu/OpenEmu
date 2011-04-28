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
#include "../../cdrom/cdromif.h"
#include "../../general.h"
#include <errno.h>
#include "cd.h"
#include "pcm.h"
#include "cdc_cdd.h"
#include "interrupt.h"
#include "timer.h"

#define READ8of16(value, a) ((value >> ((((a) & 1) ^ 1) << 3)) & 0xFF)

c68k_struc Sub68K;
static int64 Sub68KCycleCounter;

static uint8 *BIOS = NULL; // BIOS ROM, 128KiB
static uint8 *PRAM = NULL; // Program RAM, 512KiB
static uint8 *WordRAM = NULL; // Word RAM, 256KiB
static uint8 *BRAM = NULL; // 8KiB, Battery-backed RAM, accessible by sub CPU.
static bool SRES;	// Sub CPU Reset.  0 = reset, 1 = run
static bool SBRQ;	// Sub CPU Bus Req.
			// Write: 0 = cancel, 1 = request
			// Read: 0 = Sub CPU "operating"(?), 1 = acknowledge
static bool SACK;

static uint8 LEDControl;
static bool InPeripheralReset;

static uint8 MM_WP;	// Write protect
static uint8 MM_BK;	// R/W only on main.  PRAM bank select for the main CPU 
static uint8 MM_PM;	// R/W only on sub.  Priority mode.
static bool MM_MODE;	// R/W on main and sub.  0 = 2M mode, 1 = 1M mode
enum
{
 MODE_2M = 0,
 MODE_1M = 1,
};

static bool MM_DMNA;	/* R/W on main, R only on sub.  Dependent on MODE:
				2M Mode(0): If set, the main CPU can't access sub CPU word RAM at all.
				1M Mode(1): If set, sends a swap request to sub CPU.  Returns 0 when the swap request is done.
			*/
static bool MM_RET;	/* R on main, R/W on sub. */

static uint16 StopWatchCounter;
static int32 StopWatchDiv;

static uint16 COMM_FLAGS;

// Both COMM_*[8] arrays are readable by both CPUs.  CMD is only writeable by main, STATUS is only writeable by sub.
// Changed to uint8 arrays to simplify handling.
static uint8 COMM_CMD[8 * 2];
static uint8 COMM_STATUS[8 * 2];

#define DEBUGMAIN(format, ...) { printf("MAIN "format, ## __VA_ARGS__); }
#define DEBUGSUB(format, ...) { printf("SUB "format, ## __VA_ARGS__); }

#define UNDEFMAIN(format, ...) { printf("MAIN Undefined "format, ## __VA_ARGS__); }
#define UNDEFSUB(format, ...) { printf("SUB Undefined "format, ## __VA_ARGS__); }

//#if 0
#define DEBUGMAIN(format, ...) { }
//#define DEBUGSUB(format, ...)

#define UNDEFMAIN(format, ...) { }
//#define UNDEFSUB(format, ...)
//#endif

static void MDCD_MainWrite8(uint32 A, uint8 V)
{
 A &= 0xFFFFFF;

 if(A >= 0x40000)
  DEBUGMAIN("Write8: %08x %02x\n", A, V);

 if(A >= 0x20000 && A <= 0x3FFFF) // Sub68k program RAM(banked)
 {
  if(!SBRQ)
  {
   UNDEFMAIN("Write8 to PRAM when BUSREQ is not held: %08x:%02x\n",A,V);
  }
  else
  {
   uint32 offset = (A & 0x1FFFF) | (MM_BK << 17);

   WRITE_BYTE_MSB(PRAM, offset, V);
  }
 }
 else if(A >= 0x200000 && A <= 0x23FFFF) // Sub68k word RAM(switchable banking/mirroring)
 {
  if(MM_MODE == MODE_2M)
  {
   if(MM_DMNA)
   {
    UNDEFMAIN("Write to WordRAM when DMNA is set to 1: %08x %02x", A, V);
   }
   else
   {
    WRITE_BYTE_MSB(WordRAM, A & 0x3FFFF, V);
   }
  }
  else
  {
   if(A <= 0x21FFFF)
   {
    uint32 offset = (A & 0x1FFFF) | (MM_RET << 17);
    WRITE_BYTE_MSB(WordRAM, offset, V);
   }
  }
 }
 else if(A >= 0xa12000 && A <= 0xa120ff)
 {
  switch(A & 0xFF)
  {
   default: UNDEFMAIN("Unknown write: %08x %02x\n", A, V);
	    break;
   case 0x00: if(V & 0x1)
	       MDCD_InterruptAssert(2, TRUE);
	      break;
   case 0x01: 
	      if(SRES != (V & 0x1))
	      {
	       C68k_Reset(&Sub68K);
	      }

	      SRES = V & 0x1;
              SBRQ = V & 0x2;
	      SACK = SBRQ | (!SRES);
	      break;

   case 0x02: MM_WP = V;
	      break;

   case 0x03: MM_BK = (V >> 6) & 0x3;
              MM_DMNA = (V >> 1) & 0x1;
	      printf("DMNA: %02x\n", MM_DMNA);
              //if(MM_MODE == MODE_1M)
              //{
              // if(!MM_DMNA) // Writing 0 to DMNA triggers it
              // {
              //  MM_RET = !MM_RET;
	      //MM_DMNA = FALSE;
              // }
              //}
	      break;

   // CDC registers:
   case 0x04 ... 0x0B:
              MDCD_CDC_MainWrite8(A & 0xFF, V);
	      break;

   case 0x0E: COMM_FLAGS &= 0x00FF;
	      COMM_FLAGS |= V << 8;
              break;
   case 0x0F: // Unused on main CPU writes
	      break;

   case 0x10 ... 0x1F: WRITE_BYTE_MSB(COMM_CMD, A & 0xF, V);
		       break;
   case 0x20 ... 0x2F: break; // Unused on main cpu writes

  }  
 }
}

static void MDCD_MainWrite16(uint32 A, uint16 V)
{
 A &= 0xFFFFFF;

 if(A >= 0x40000)
  DEBUGMAIN("Write16: %08x %04x\n", A, V);

 if(A >= 0x20000 && A <= 0x3FFFF) // Sub68k program RAM(banked)
 {
  if(!SBRQ)
  {
   UNDEFMAIN("Write16 to PRAM when BUSREQ is not held: %08x:%04x\n",A,V);
  }
  else
  {
   uint32 offset = (A & 0x1FFFF) | (MM_BK << 17);

   WRITE_WORD_MSB(PRAM, offset, V);
  }
 }
 else if(A >= 0x200000 && A <= 0x23FFFF) // Sub68k word RAM(switchable banking/mirroring)
 {
  if(MM_MODE == MODE_2M)
  {
   if(MM_DMNA)
   {
    UNDEFMAIN("Write to WordRAM when DMNA is set to 1: %08x %02x", A, V);
   }
   else
   {
    WRITE_WORD_MSB(WordRAM, A & 0x3FFFF, V);  
   }
  }
  else
  {
   if(A <= 0x21FFFF)
   {
    uint32 offset = (A & 0x1FFFF) | (MM_RET << 17);
    WRITE_WORD_MSB(WordRAM, offset, V);
   }
  }
 }
 else if(A >= 0xa12000 && A <= 0xa120ff)
 {
  switch(A & 0xFF)
  {
   default: UNDEFMAIN("Unknown write: %08x %04x\n", A, V);
            break;

   case 0x00: if(V & 0x0100)
               MDCD_InterruptAssert(2, TRUE);

              if(SRES != (V & 0x1))
              {
               C68k_Reset(&Sub68K);
	      }
	      SRES = V & 0x1;
	      SBRQ = V & 0x2;
              SACK = SBRQ | (!SRES);

	      break;

   case 0x02: MM_WP = V >> 8;
	      MM_DMNA = (V >> 1) & 0x1;
	      MM_BK = (V >> 6) & 0x3;
              printf("DMNA: %02x\n", MM_DMNA);
              //if(MM_MODE == MODE_1M)
              //{
              // if(MM_DMNA)
              // {
              //  MM_RET = !MM_RET;
	      //MM_DMNA = FALSE;
              // }
              // }
              break;

   // CDC registers:
   case 0x04 ... 0x0B:
              MDCD_CDC_MainWrite16(A & 0xFF, V);
              break;

   case 0x0E: COMM_FLAGS &= 0x00FF;
              COMM_FLAGS |= V & 0xFF00;
	      break;
   case 0x10 ... 0x1F: WRITE_WORD_MSB(COMM_CMD, A & 0xF, V);
		       break;
   case 0x20 ... 0x2F: // Unused on main CPU writes
		       break;
  }
 }
}

static uint8 MDCD_MainRead8(uint32 A)
{
 A &= 0xFFFFFF;

 if(A >= 0x40000)
  DEBUGMAIN("Read8: %08x\n", A);

 if(A <= 0x1FFFF)	// BIOS ROM
 {
  return(READ_BYTE_MSB(BIOS, A));
 }
 else if(A >= 0x20000 && A <= 0x3FFFF) // Sub68k program RAM(banked)
 {
  if(!SBRQ)
  {
   UNDEFMAIN("Read8 from PRAM when BUSREQ is not held: %08x\n",A);
  }
  else
  {
   uint32 offset = (A & 0x1FFFF) | (MM_BK << 17);
   return(READ_BYTE_MSB(PRAM, offset));
  }
 }
 else if(A >= 0x200000 && A <= 0x23FFFF) // Sub68k word RAM(switchable banking/mirroring)
 {
  if(MM_MODE == MODE_2M)
  {
   if(MM_DMNA)
   {
    UNDEFMAIN("Read8 from WordRAM when DMNA is set to 1: %08x", A);
   }
   else
   {
    return(READ_BYTE_MSB(WordRAM, A & 0x3FFFF));
   }
  }
  else
  {
   if(A <= 0x21FFFF)
   {
    uint32 offset = (A & 0x1FFFF) | (MM_RET << 17);
    return(READ_BYTE_MSB(WordRAM, offset));
   }
  }
 }
 else if(A >= 0xa12000 && A <= 0xa120ff) 
 {
  uint8 ret = 0;
  switch(A & 0xFF)
  {
   case 0x00: ret |= MDCD_InterruptGetAsserted(2) ? 0x01 : 0x00;
	      ret |= MDCD_InterruptGetMask(2) ? 0x80 : 0x00;
	      return(ret);

   case 0x01: ret |= SRES ? 0x1 : 0x0;
	      ret |= SACK ? 0x2 : 0x0;
	      return(ret);

   case 0x02: ret = MM_WP;
	      return(ret);

   case 0x03: ret |= MM_BK << 6;
	      ret |= MM_MODE ? 0x4 : 0x0;
	      ret |= MM_DMNA ? 0x2 : 0x0;
	      ret |= MM_RET ? 0x1 : 0x0;
	      return(ret);

   // CDC registers:
   case 0x04 ... 0x0B:
	      return(MDCD_CDC_MainRead8(A & 0xFF));

   // Communications:
   case 0x0C:
   case 0x0D: return(READ8of16(StopWatchCounter, A & 1));

   case 0x0E: return(COMM_FLAGS >> 8);
   case 0x0F: return(COMM_FLAGS >> 0);
   case 0x10 ... 0x1F: return(READ_BYTE_MSB(COMM_CMD, (A & 0xF)));
   case 0x20 ... 0x2F: return(READ_BYTE_MSB(COMM_STATUS, (A & 0xF)));
  }
 }
 printf("Unknown main read8: %08x\n", A);
 return(0x0);
}

static uint16 MDCD_MainRead16(uint32 A)
{
 A &= 0xFFFFFF;

 if(A >= 0x40000)
  DEBUGMAIN("Read16: %08x\n", A);

 if(A <= 0x1FFFF)       // BIOS ROM
 {
  return(READ_WORD_MSB(BIOS, A));
 }
 else if(A >= 0x20000 && A <= 0x3FFFF) // Sub68k program RAM(banked)
 {
  if(!SBRQ)
  {
   UNDEFMAIN("Read16 from PRAM when BUSREQ is not held: %08x\n",A);
  }
  else
  {
   uint32 offset = (A & 0x1FFFF) | (MM_BK << 17);
   return(READ_WORD_MSB(PRAM, offset));
  }
 }
 else if(A >= 0x200000 && A <= 0x23FFFF) // Sub68k word RAM(switchable banking/mirroring)
 {
  if(MM_MODE == MODE_2M)
  {
   if(MM_DMNA)
   {
    UNDEFMAIN("Read16 from WordRAM when DMNA is set to 1: %08x", A);
   }
   else
   {
    return(READ_WORD_MSB(WordRAM, A & 0x3FFFF));
   }
  }
  else
  {
   if(A <= 0x21FFFF)
   {
    uint32 offset = (A & 0x1FFFF) | (MM_RET << 17);
    return(READ_WORD_MSB(WordRAM, offset));
   }
  }
 }
 else if(A >= 0xa12000 && A <= 0xa120ff)
 {
  uint16 ret = 0;

  switch(A & 0xFF)
  {
   case 0x00:
	     ret |= MDCD_InterruptGetAsserted(2) ? 0x0100 : 0x0000;
             ret |= MDCD_InterruptGetMask(2) ? 0x8000 : 0x0000;
	     ret |= SRES ? 0x1 : 0x0;
             ret |= SACK ? 0x2 : 0x0;
	     return(ret);

   case 0x02: ret |= MM_WP << 8;
	      ret |= MM_BK << 6;
              ret |= MM_MODE ? 0x4 : 0x0;
              ret |= MM_DMNA ? 0x2 : 0x0;
              ret |= MM_RET ? 0x1 : 0x0;
	      return(ret);

   // CDC registers:
   case 0x04 ... 0x0B:
              return(MDCD_CDC_MainRead16(A & 0xFF));

   case 0x0C: return(StopWatchCounter);
   case 0x0E: return(COMM_FLAGS);
   case 0x10 ... 0x1F: return(READ_WORD_MSB(COMM_CMD, (A & 0xF)));
   case 0x20 ... 0x2F: return(READ_WORD_MSB(COMM_STATUS, (A & 0xF)));
  }
 }

 printf("Unknown main Read16: %08x\n", A);
}

//////////////////////////////////
//
// Sub 68K memory map handling:
//
//////////////////////////////////
static void MDCD_SubWrite8(uint32 A, uint8 V)
{
 A &= 0xFFFFFF;

 if(A >= 0x80000)
  DEBUGSUB("Write8: %08x %02x\n", A, V);

 if(A <= 0x7FFFF)               // Program RAM
 {
  if((A >> 8) < MM_WP)
  {
   printf("Oops8: %08x:%02x\n", A, V);
  }
  else
  {
   WRITE_BYTE_MSB(PRAM, A, V);
  }
 }
 else if(A >= 0x80000 && A <= 0xBFFFF)  // Word RAM
 {                                      // 2M mode: full 256KiB
                                        // 1M mode:
  if(MM_MODE == MODE_2M)
  {
   WRITE_BYTE_MSB(WordRAM, A & 0x3FFFF, V);
  }
  else
  {
   uint32 base = ((MM_RET ^ 1) << 17);

  }
 }
 else if(A >= 0xFE0000 && A <= 0xFE3FFF) //8KiB BRAM mapped into a 16KiB space
 {
  // Are even bytes open bus, 0, or the same as odd bytes?
  if(A & 1)
   BRAM[(A & 0x3FFF) >> 1] = V;
  else
   printf("Invalid write8: %08x:%02x\n", A, V);
 }
 else if(A >= 0xFE4000 && A <= 0xFEFFFF) //BRAM mirror(read only?  unstable?  reserved?)
 {
  printf("Invalid write8: %08x:%02x\n", A, V);
 }
 else if(A >= 0xFF0000 && A <= 0xFF3FFF) // PCM sound source
 {
  MDCD_PCM_Write((A >> 1) & 0x1FFF, V);
 }
 else if(A >= 0xFF4000 && A <= 0xFF7FFF) // PCM sound source mirror(read only?  unstable?  reserved?)
 {
  printf("Invalid write8: %08x:%02x\n", A, V);
 }
 else if(A >= 0xFF8000 && A <= 0xFF81FF) // Registers(as if everything else isn't ;))
 {
  //
  // Sub cpu registers!
  //
  switch(A & 0x1FF)
  {
   default: UNDEFSUB("Unknown write: %08x %02x\n", A, V);
            break;

   case 0x00: LEDControl = (V & 0x3);
	      break;
   case 0x01: if(!(V & 0x1))
	       InPeripheralReset = TRUE;
	      break;

   case 0x02: break; // WP, unused on sub cpu writes
   case 0x03: MM_PM = (V >> 3) & 0x3;
	      MM_MODE = (V >> 2) & 0x1;
	      MM_RET = (V >> 0) & 0x1;
	      printf("Cha8: %02x\n", V);
	      break;

   case 0x04 ... 0x0B:
	      MDCD_CDC_SubWrite8(A & 0x1FF, V);
	      break;

   case 0x0C:
   case 0x0D: StopWatchCounter = 0;	
	      // Should we reset the divider too?
	      break;

   //case 0x0E: break;			// Unused on sub cpu writes.  TODO:  Investigate Dragon's Lair
   case 0x0F: COMM_FLAGS &= 0xFF00;
              COMM_FLAGS |= V << 0;
	      break;

   case 0x10 ... 0x1F: break; // Unused on sub cpu writes
   case 0x20 ... 0x2F: WRITE_BYTE_MSB(COMM_STATUS, (A & 0xF), V);
		       break;

   case 0x30: break;
   case 0x31: MDCD_Timer_Write(V);
              break;

   case 0x32: break;
   case 0x33: MDCD_InterruptWrite(V);
   
   case 0x34 ... 0x4B:
	      MDCD_CDD_Write8(A & 0x1FF, V);
	      break;

  }
 }
}

static void MDCD_SubWrite16(uint32 A, uint16 V)
{
 A &= 0xFFFFFF;

 assert(!(A & 1));

 if(A >= 0x80000)
  DEBUGSUB("Write16: %08x %04x\n", A, V);

 if(A <= 0x7FFFF)               // Program RAM
 {
  if((A >> 8) < MM_WP)
  {
   printf("Oops16: %08x:%04x\n", A, V);
  }
  else
  {
   WRITE_WORD_MSB(PRAM, A, V);
  }
 }
 else if(A >= 0x80000 && A <= 0xBFFFF)  // Word RAM
 {                                      // 2M mode: full 256KiB
                                        // 1M mode:
  if(MM_MODE == MODE_2M)
  {
   WRITE_WORD_MSB(WordRAM, A & 0x3FFFF, V);
  }
  else
  {
   uint32 base = ((MM_RET ^ 1) << 17);

  }
 }
 else if(A >= 0xFE0000 && A <= 0xFE3FFF) //8KiB BRAM mapped into a 16KiB space
 {
  BRAM[(A & 0x3FFF) >> 1] = V & 0xFF;
 }
 else if(A >= 0xFE4000 && A <= 0xFEFFFF) //BRAM mirror(read only?  unstable?  reserved?)
 {
  printf("Invalid write16: %08x:%04x\n", A, V);
 }
 else if(A >= 0xFF0000 && A <= 0xFF3FFF) // PCM sound source
 {
  MDCD_PCM_Write((A >> 1) & 0x1FFF, V & 0xFF);
 }
 else if(A >= 0xFF4000 && A <= 0xFF7FFF) // PCM sound source mirror(read only?  unstable?  reserved?)
 {
  printf("Invalid write16: %08x:%04x\n", A, V);
 }
 else if(A >= 0xFF8000 && A <= 0xFF81FF) // Registers(as if everything else isn't ;))
 {
  //
  // Sub cpu registers!
  //
  switch(A & 0x1FF)
  {
   default: UNDEFSUB("Unknown write: %08x %04x\n", A, V);
            break;

   case 0x02:
	      MM_PM = (V >> 3) & 0x3;
              MM_MODE = (V >> 2) & 0x1;
              MM_RET = (V >> 0) & 0x1;
              printf("Cha16: %04x\n", V);
	      break;

   case 0x04 ... 0x0B:
              MDCD_CDC_SubWrite16(A & 0x1FF, V);
              break;

   case 0x0C: StopWatchCounter = 0;                             
              // Should we reset the divider too?
              break;

   case 0x0E: 
	      COMM_FLAGS &= 0xFF00;
              COMM_FLAGS |= V & 0x00FF;
              break;

   case 0x10 ... 0x1F: break; // Unused on sub cpu writes
   case 0x20 ... 0x2F: WRITE_WORD_MSB(COMM_STATUS, (A & 0xF), V);
                       break;

   case 0x30: MDCD_Timer_Write(V & 0xFF);
	      break;

   case 0x32: MDCD_InterruptWrite(V);
	      break;

   case 0x34 ... 0x4B:
              MDCD_CDD_Write16(A & 0x1FF, V);
              break;
  }
 }
}

static uint8 MDCD_SubRead8(uint32 A)
{
 A &= 0xFFFFFF;

 if(A >= 0x80000)
  DEBUGSUB("Read8: %08x\n", A);

 if(A <= 0x7FFFF)		// Program RAM
 {
  return(READ_BYTE_MSB(PRAM, A));
 }
 else if(A >= 0x80000 && A <= 0xBFFFF)	// Word RAM
 {					// 2M mode: full 256KiB
					// 1M mode: 
  if(MM_MODE == MODE_2M)
  {
   return(READ_BYTE_MSB(WordRAM, A & 0x3FFFF));
  }
  else
  {
   uint32 base = ((MM_RET ^ 1) << 17);

  }
 }
 else if(A >= 0xC0000 && A <= 0xDFFFF)	// Word RAM.
 {					// 2M mode: Unmapped
					// 1M mode: Selectable
  if(MM_MODE == MODE_1M)
  {
   uint32 offset = (A & 0x1FFFF) | ((MM_RET ^ 1) << 17);
   return(READ_BYTE_MSB(WordRAM, offset));
  }
 }
 else if(A >= 0xFE0000 && A <= 0xFE3FFF) //8KiB BRAM mapped into a 16KiB space
 {
  // Are even bytes open bus, 0, or the same as odd bytes?
  return(BRAM[(A & 0x3FFF) >> 1]);
 }
 else if(A >= 0xFE4000 && A <= 0xFEFFFF) //BRAM mirror(read only?  unstable?  reserved?)
 {
  return(BRAM[(A & 0x3FFF) >> 1]);
 }
 else if(A >= 0xFF0000 && A <= 0xFF3FFF) // PCM sound source
 {

 }
 else if(A >= 0xFF4000 && A <= 0xFF7FFF) // PCM sound source mirror(read only?  unstable?  reserved?)
 {

 }
 else if(A >= 0xFF8000 && A <= 0xFF81FF) // Registers(as if everything else isn't ;))
 {
  //
  // Sub cpu registers!
  //
  uint8 ret = 0;

  switch(A & 0x1FF)
  {
   case 0x00: ret |= LEDControl << 0;
	      return(ret);
   case 0x01: ret |= 1 << 0;
	      ret |= 0 << 4; // Version
	      return(ret);

   case 0x02: return(MM_WP);
   case 0x03: ret |= MM_RET << 0;
	      ret |= MM_DMNA << 1;
	      ret |= MM_MODE << 2;
	      ret |= MM_PM << 3;
	      return(ret);

   case 0x0C:
   case 0x0D: return(READ8of16(StopWatchCounter, A & 1));

   case 0x0E: return(COMM_FLAGS >> 8);
   case 0x0F: return(COMM_FLAGS >> 0);

   case 0x10 ... 0x1F: return(READ_BYTE_MSB(COMM_CMD, (A & 0xF)));
   case 0x20 ... 0x2F: return(READ_BYTE_MSB(COMM_STATUS, (A & 0xF)));

   case 0x30: break;
   case 0x31: return(MDCD_Timer_Read());

   case 0x32: break;
   case 0x33: return(MDCD_InterruptRead());
  }
 }
 else if(A >= 0xFF8200 && A <= 0xFFFFFF) // Reserved
 {

 }

 printf("Unknown Sub read8: %08x\n", A);

 return(0);
}

static uint16 MDCD_SubRead16(uint32 A)
{
 A &= 0xFFFFFF;

 assert(!(A & 1));

 if(A >= 0x80000)
  DEBUGSUB("Read16: %08x\n", A);

 if(A <= 0x7FFFF)               // Program RAM
 {
  return(READ_WORD_MSB(PRAM, A));
 }
 else if(A >= 0x80000 && A <= 0xBFFFF)  // Word RAM
 {                                      // 2M mode: full 256KiB
                                        // 1M mode:
  if(MM_MODE == MODE_2M)
  {
   return(READ_WORD_MSB(WordRAM, A & 0x3FFFF));
  }
  else
  {
   uint32 base = ((MM_RET ^ 1) << 17);

  }
 }
 else if(A >= 0xFE0000 && A <= 0xFE3FFF) //8KiB BRAM mapped into a 16KiB space
 {
  return(BRAM[(A & 0x3FFF) >> 1]);
 }
 else if(A >= 0xFE4000 && A <= 0xFEFFFF) //BRAM mirror(read only?  unstable?  reserved?)
 {
  return(BRAM[(A & 0x3FFF) >> 1]);
 }
 else if(A >= 0xFF0000 && A <= 0xFF3FFF) // PCM sound source
 {

 }
 else if(A >= 0xFF4000 && A <= 0xFF7FFF) // PCM sound source mirror(read only?  unstable?  reserved?)
 {

 }
 else if(A >= 0xFF8000 && A <= 0xFF81FF) // Registers(as if everything else isn't ;))
 {
  //
  // Sub cpu registers! 
  //
  uint16 ret = 0;

  switch(A & 0x1FF)
  {
   case 0x02: ret = MM_WP << 8;
   	      ret |= MM_RET << 0;
              ret |= MM_DMNA << 1;
              ret |= MM_MODE << 2;
              ret |= MM_PM << 3;
              return(ret);

   case 0x0C: return(StopWatchCounter);
   case 0x0E: return(COMM_FLAGS);
   case 0x10 ... 0x1F: return(READ_WORD_MSB(COMM_CMD, (A & 0xF)));
   case 0x20 ... 0x2F: return(READ_WORD_MSB(COMM_STATUS, (A & 0xF)));
   case 0x30: return(MDCD_Timer_Read());
   case 0x32: return(MDCD_InterruptRead());
  }
 }
 else if(A >= 0xFF8200 && A <= 0xFFFFFF) // Reserved
 {

 }

 printf("Unknown Sub read16: %08x\n", A);
 return(0);
}

#define SUBCPU_CLOCK 12500000

void MDCD_Run(int32 md_master_cycles)
{
 const uint32 md_master_clock = CLOCK_NTSC;

 Sub68KCycleCounter += (int64)md_master_cycles * SUBCPU_CLOCK;

 while(Sub68KCycleCounter > 0)
 {
  int32 temp_cycles;
 
  if(MM_MODE)
	exit(1);

  if(!SRES || SBRQ)
   temp_cycles = 8;
  else
  {
   //printf("Yay: %08x\n", C68k_Get_PC(&Sub68K));
   temp_cycles = C68k_Exec(&Sub68K);
  }
  MDCD_Timer_Run(temp_cycles);
  MDCD_PCM_Run(temp_cycles);

  // Run stop watch
  StopWatchDiv -= temp_cycles; 
  while(StopWatchDiv <= 0)
  {
   StopWatchCounter = (StopWatchCounter + 1) & 0xFFF;
   StopWatchDiv += 384;
  }
  Sub68KCycleCounter -= (int64)temp_cycles * md_master_clock;
 }
}

void MDCD_Close(void)
{
 if(BIOS)
 {
  MDFN_free(BIOS);
  BIOS = NULL;
 }

 if(PRAM)
 {
  MDFN_free(PRAM);
  PRAM = NULL;
 }

 if(WordRAM)
 {
  MDFN_free(WordRAM);
  WordRAM = NULL;
 }
}

bool MDCD_Init(void)
{
 if(!(BIOS = (uint8 *)MDFN_calloc(1, 0x20000, _("BIOS ROM"))))
 {
  return(FALSE);
 }

 if(!(BRAM = (uint8 *)MDFN_calloc(1, 0x2000, _("Battery-backed RAM"))))
 {
  return(FALSE);
 }
 memset(BRAM, 0xFF, 8192);

 if(!(PRAM = (uint8 *)MDFN_calloc(1, 0x80000, _("Program RAM"))))
 {
  return(FALSE);
 }

 if(!(WordRAM = (uint8 *)MDFN_calloc(1, 0x40000, _("Word RAM"))))
 {
  return(FALSE);
 }

 // Load the BIOS
 static const FileExtensionSpecStruct KnownBIOSExtensions[] =
 {
  { ".bin", gettext_noop("BIOS Image") },
  { ".bios", gettext_noop("BIOS Image") },
  { NULL, NULL }
 };
 std::string bios_path = MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, MDFN_GetSettingS("md.cdbios").c_str());
 MDFNFILE bios_fp;

 if(!bios_fp.Open(bios_path.c_str(), KnownBIOSExtensions))
 {
  MDFN_PrintError(_("Could not open CD BIOS file \"%s\": %s\n"), bios_path.c_str(), strerror(errno));
  return(0);
 }

 if(bios_fp.Size() != 0x20000)
 {
  MDFN_PrintError(_("BIOS is incorrect size."));
  return(0);
 }

 memcpy(BIOS, bios_fp.Data(), 0x20000);

 bios_fp.Close();

 MD_ExtRead8 = MDCD_MainRead8;
 MD_ExtRead16 = MDCD_MainRead16;
 MD_ExtWrite8 = MDCD_MainWrite8;
 MD_ExtWrite16 = MDCD_MainWrite16;

 C68k_Init(&Sub68K, MDCD_InterruptAck);
 C68k_Set_ReadB(&Sub68K, MDCD_SubRead8);
 C68k_Set_ReadW(&Sub68K, MDCD_SubRead16);
 C68k_Set_WriteB(&Sub68K, MDCD_SubWrite8);
 C68k_Set_WriteW(&Sub68K, MDCD_SubWrite16);

 return(TRUE);
}

static int32 CheckValidTrack(uint8 *sector_buffer)
{
 CD_TOC toc;
 bool DTFound = 0;
 int32 track;

 CDIF_ReadTOC(&toc);

 for(track = toc.first_track; track <= toc.last_track; track++)
 {
  if(toc.tracks[track].control & 0x4)
  {
   DTFound = 1;
   break;
  }
 }

 if(DTFound) // Only add the MD5 hash if we were able to find a data track.
 {
  if(CDIF_ReadSector(sector_buffer, toc.tracks[track].lba, 1))
  {
   if(!memcmp(sector_buffer + 0x100, "SEGA MEGA DRIVE", 15) || !memcmp(sector_buffer + 0x100, "SEGA GENESIS", 12))
    return(track);
  }
 }

 return(0);
}

bool MDCD_TestMagic(void)
{
 uint8 sector_buffer[2048];

 return((bool)CheckValidTrack(sector_buffer));
}

bool MDCD_Load(md_game_info *ginfo)
{
 uint8 sector_buffer[2048];

 if(!CheckValidTrack(sector_buffer))
  return(FALSE);

 MD_ReadSegaHeader(sector_buffer + 0x100, ginfo);

 //uint8 sector_buffer[2048];
 //for(int x = 0; x < 128; x++)
 //{
 // memset(sector_buffer, 0, 2048);
 // CDIF_ReadSector(sector_buffer, NULL, start_sector + x, 1);
 // md5.update(sector_buffer, 2048);
 //}
 //}

 return(MDCD_Init()); 
}

 
void MDCD_Reset(void)
{
 memset(PRAM, 0, 0x80000);
 memset(WordRAM, 0, 0x40000);

 C68k_Reset(&Sub68K);
 MDCD_InterruptReset();
 MDCD_PCM_Reset();

 SRES = 1;
 SBRQ = 1;
 SACK = 1;

 MM_WP = 0;
 MM_BK = 0;

 MM_RET = 1;
}
