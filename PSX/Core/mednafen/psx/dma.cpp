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
#include "mdec.h"
#include "cdc.h"
#include "spu.h"

// FIXME: 0-length block count?


/* Notes:

 Channel 4(SPU):
	Write:
		Doesn't seem to work properly with CHCR=0x01000001
		Hung when CHCR=0x11000601

 Channel 6:
	DMA hangs if D28 of CHCR is 0?
	D1 did not have an apparent effect.

*/

enum
{
 CH_MDEC_IN = 0,
 CH_MDEC_OUT = 1,
 CH_GPU = 2,
 CH_CDC = 3,
 CH_SPU = 4,
 CH_OT = 6,
};


// RunChannels(128 - whatevercounter);
//
// GPU next event, std::max<128, wait_time>, or something similar, for handling FIFO.

namespace MDFN_IEN_PSX
{

static int32 DMACycleCounter;

static uint32 DMAControl;
static uint32 DMAIntControl;
static uint8 DMAIntStatus;
static bool IRQOut;

struct Channel
{
 uint32 BaseAddr;
 uint32 BlockControl;
 uint32 ChanControl;

 //
 //
 //
 uint32 CurAddr;
 uint32 NextAddr;

 uint16 BlockCounter;
 uint16 WordCounter; 

 int32 ClockCounter;
};

static Channel DMACH[7];
static pscpu_timestamp_t lastts;


static const char *PrettyChannelNames[7] = { "MDEC IN", "MDEC OUT", "GPU", "CDC", "SPU", "PIO", "OTC" };

void DMA_Init(void)
{

}

void DMA_Kill(void)
{

}

static INLINE void RecalcIRQOut(void)
{
 bool irqo;

 irqo = (bool)(DMAIntStatus & ((DMAIntControl >> 16) & 0x7F));
 irqo &= (DMAIntControl >> 23) & 1;

 // I think it's logical OR, not XOR/invert.  Still kind of weird, maybe it actually does something more complicated?
 //irqo ^= (DMAIntControl >> 15) & 1;
 irqo |= (DMAIntControl >> 15) & 1;

 IRQOut = irqo;
 IRQ_Assert(IRQ_DMA, irqo);
}

void DMA_ResetTS(void)
{
 lastts = 0;
}

void DMA_Power(void)
{
 lastts = 0;

 memset(DMACH, 0, sizeof(DMACH));

 DMACycleCounter = 128;

 DMAControl = 0;
 DMAIntControl = 0;
 DMAIntStatus = 0;
 RecalcIRQOut();
}

static void RecalcHalt(void)
{
 bool Halt = false;

 if((DMACH[0].WordCounter || (DMACH[0].ChanControl & (1 << 24))) && (DMACH[0].ChanControl & 0x200) /*&& MDEC_DMACanWrite()*/)
  Halt = true;

 if((DMACH[1].WordCounter || (DMACH[1].ChanControl & (1 << 24))) && (DMACH[1].ChanControl & 0x200) && (DMACH[1].WordCounter || MDEC_DMACanRead()))
  Halt = true;

 if((DMACH[2].WordCounter || (DMACH[2].ChanControl & (1 << 24))) && (DMACH[2].ChanControl & 0x200) && ((DMACH[2].ChanControl & 0x1) && (DMACH[2].WordCounter || GPU->DMACanWrite())))
  Halt = true;

 if((DMACH[3].WordCounter || (DMACH[3].ChanControl & (1 << 24))) && !(DMACH[3].ChanControl & 0x100))
  Halt = true;

 if(DMACH[6].WordCounter || (DMACH[6].ChanControl & (1 << 24)))
  Halt = true;

 //printf("Halt: %d\n", Halt);

 CPU->SetHalt(Halt);
}

template<int ch, bool write_mode>
static INLINE bool ChCan(void)
{
#if 0
 if(ch != 3)
 {
  if((DMACH[3].WordCounter || (DMACH[3].ChanControl & (1 << 24))) && !(DMACH[3].ChanControl & 0x100))
  {
   return(false);
  }
 }
#endif

 switch(ch)
 {
  case 0: return(true);	// MDEC IN
  case 1: return(MDEC_DMACanRead());	// MDEC out
  case 2: 
	  if(write_mode)
	   return(GPU->DMACanWrite());
	  else
	   return(true);	// GPU
  case 3: return(true);	// CDC
  case 4: return(true);	// SPU
  case 5: return(true);	// ??
  case 6: return(true);	// OT
 }
 abort();
}

template<int ch, bool write_mode>
static INLINE void ChRW(int32 timestamp, uint32 *V)
{
 switch(ch)
 {
  default:
	abort();

  case CH_MDEC_IN:
	  MDEC_DMAWrite(*V);
	  break;

  case CH_MDEC_OUT:
	  *V = MDEC_DMARead();
	  break;

  case CH_GPU:
	  if(write_mode)
	   GPU->WriteDMA(*V);
	  else
	   *V = GPU->Read(timestamp, 0);
	  break;

  case CH_CDC:
	  // 0x1f801018 affects CDC DMA timing.
#if 0
	  if(DMACH[ch].ChanControl & 0x100)		// For CDC DMA(at least): When this bit is set, DMA controller doesn't appear to hog the (RAM?) bus.
	  {
	   if(DMACH[ch].ChanControl & 0x00400000)	// For CDC DMA(at least): When this bit is set, DMA controller appears to get even less bus time(or has a lower priority??)
	   {
	    DMACH[ch].ClockCounter -= 44 * 20 / 12;
	   }
	   else
	   {
	    DMACH[ch].ClockCounter -= 29 * 20 / 12;
	   }
	  }
	  else
	  {
	   DMACH[ch].ClockCounter -= 23 * 20 / 12; // (23 + 1) = 24.  (Though closer to 24.5 or 24.4 on average per tests on a PS1)
	  }
#endif
	  *V = CDC->DMARead();
	  break;

  case CH_SPU:
	  // 0x1f801014 affects SPU DMA timing.
	  // Wild conjecture about 0x1f801014:
	  //
	  //  & 0x0000000F
	  //  & 0x000001E0  --- Used if (& 0x20000000) == 0?
	  //  & 0x00001000  --- Double total bus cycle time if value == 0?
	  //  & 0x0f000000  --- (value << 1) 33MHz cycles, bus cycle extension(added to 4?)?
	  //  & 0x20000000  --- 
	  //
	  //
	  // TODO?: SPU DMA will "complete" much faster if there's a mismatch between the CHCR read/write mode bit and the SPU control register DMA mode.
	  //
	  //
	  // Investigate: SPU DMA doesn't seem to work right if the value written to 0x1F801DAA doesn't have the upper bit set to 1(0x8000) on a PS1.

	  DMACH[ch].ClockCounter -= 47; // Should be closer to 69, average, but actual timing is...complicated.
	  if(write_mode)
	   SPU->WriteDMA(*V);
	  else
	   *V = SPU->ReadDMA();
	  break;

  case CH_OT:
	  if(DMACH[ch].WordCounter == 1)
	   *V = 0xFFFFFF;
	  else
	   *V = (DMACH[ch].CurAddr - 4) & 0x1FFFFF;
	  break;
 }
}

template<int ch, bool write_mode>
static INLINE void RunChannelT(pscpu_timestamp_t timestamp, int32 clocks)
{
 //const uint32 dc = (DMAControl >> (ch * 4)) & 0xF;

 DMACH[ch].ClockCounter += clocks;

 while(DMACH[ch].ClockCounter > 0)
 {
  if(!DMACH[ch].WordCounter)
  {
   if(!(DMACH[ch].ChanControl & (1 << 24)))
   {
    break;
   }

   if(DMACH[ch].NextAddr & 0x800000)
   {
    //if(ch == 2)
    // PSX_WARNING("[DMA] LL Channel 2 ended normally: %d\n", GPU->GetScanlineNum());
    DMACH[ch].ChanControl &= ~(0x11 << 24);
    if(DMAIntControl & (1 << (16 + ch)))
    {
     DMAIntStatus |= 1 << ch;
     RecalcIRQOut();
    }
    break;
   }

   if(!ChCan<ch, write_mode>())
    break;

   if((DMACH[ch].ChanControl & (1 << 10)) && write_mode)
   {
    uint32 header;

    DMACH[ch].CurAddr = DMACH[ch].NextAddr & 0x1FFFFC;
    header = MainRAM.ReadU32(DMACH[ch].CurAddr);
    DMACH[ch].CurAddr = (DMACH[ch].CurAddr + 4) & 0x1FFFFF;

    DMACH[ch].WordCounter = header >> 24;
    DMACH[ch].NextAddr = header & 0xFFFFFF;

    if(DMACH[ch].WordCounter > 0x10) 
     printf("What the lala?  0x%02x @ 0x%08x\n", DMACH[ch].WordCounter, DMACH[ch].CurAddr - 4);

    if(DMACH[ch].WordCounter)
     DMACH[ch].ClockCounter -= 15;
    else
     DMACH[ch].ClockCounter -= 10;

    continue;
   }
   else
   {
    DMACH[ch].CurAddr = DMACH[ch].NextAddr & 0x1FFFFC;
    DMACH[ch].WordCounter = DMACH[ch].BlockControl & 0xFFFF;
    DMACH[ch].BlockCounter--;

    if(!DMACH[ch].BlockCounter || ch == 6 || ch == 3)
     DMACH[ch].NextAddr = 0xFFFFFF;
    else
     DMACH[ch].NextAddr = (DMACH[ch].CurAddr + ((DMACH[ch].BlockControl & 0xFFFF) << 2)) & 0x1FFFFF;
   }
  }

  if(ch != 2 && ch != 1)
  {
   if(!ChCan<ch, write_mode>())
    break;
  }


  {
   uint32 vtmp;

   if(write_mode)
    vtmp = MainRAM.ReadU32(DMACH[ch].CurAddr);

   ChRW<ch, write_mode>(timestamp, &vtmp);

   if(!write_mode)
    MainRAM.WriteU32(DMACH[ch].CurAddr, vtmp);
  }

  if(ch == 6)
   DMACH[ch].CurAddr = (DMACH[ch].CurAddr - 4) & 0x1FFFFF;
  else
   DMACH[ch].CurAddr = (DMACH[ch].CurAddr + 4) & 0x1FFFFF;

  DMACH[ch].WordCounter--;
  DMACH[ch].ClockCounter--;
 }


 if(DMACH[ch].ClockCounter > 0)
  DMACH[ch].ClockCounter = 0;
}

static INLINE void RunChannel(pscpu_timestamp_t timestamp, int32 clocks, int ch)
{
 switch(ch)
 {
  default: abort();

  case 0:
	RunChannelT<0, true>(timestamp, clocks);
	break;

  case 1:
	RunChannelT<1, false>(timestamp, clocks);
	break;

  case 2:
	if(DMACH[2].ChanControl & 0x1)
  	 RunChannelT<2, true>(timestamp, clocks);
	else
	 RunChannelT<2, false>(timestamp, clocks);
	break;

  case 3:
	RunChannelT<3, false>(timestamp, clocks);
	break;

  case 4:
	if(DMACH[4].ChanControl & 0x1)
 	RunChannelT<4, true>(timestamp, clocks);
	else
	 RunChannelT<4, false>(timestamp, clocks);
	break;

  case 6:
	RunChannelT<6, false>(timestamp, clocks);
	break;
 }
}

static INLINE int32 CalcNextEvent(int32 next_event)
{
 if(DMACycleCounter < next_event)
  next_event = DMACycleCounter;

 return(next_event);
}

pscpu_timestamp_t DMA_Update(const pscpu_timestamp_t timestamp)
{
//   uint32 dc = (DMAControl >> (ch * 4)) & 0xF;
 int32 clocks = timestamp - lastts;
 lastts = timestamp;

 GPU->Update(timestamp);
 MDEC_Run(clocks);

 RunChannel(timestamp, clocks, 0);
 RunChannel(timestamp, clocks, 1);
 RunChannel(timestamp, clocks, 2);
 RunChannel(timestamp, clocks, 3);
 RunChannel(timestamp, clocks, 4);
 RunChannel(timestamp, clocks, 6);

 DMACycleCounter -= clocks;
 while(DMACycleCounter <= 0)
  DMACycleCounter += 128;

 RecalcHalt();

 return(timestamp + CalcNextEvent(0x10000000));
}

void DMA_Write(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 int ch = (A & 0x7F) >> 4;

 //if(ch == 2 || ch == 7)
 //PSX_WARNING("[DMA] Write: %08x %08x, DMAIntStatus=%08x", A, V, DMAIntStatus);

 // FIXME if we ever have "accurate" bus emulation
 V <<= (A & 3) * 8;

 DMA_Update(timestamp);

 if(ch == 7)
 {
  switch(A & 0xC)
  {
   case 0x0: //fprintf(stderr, "Global DMA control: 0x%08x\n", V);
	     DMAControl = V;
	     RecalcHalt();
	     break;

   case 0x4: 
	     //for(int x = 0; x < 7; x++)
	     //{
             // if(DMACH[x].WordCounter || (DMACH[x].ChanControl & (1 << 24)))
	     // {
	     //  fprintf(stderr, "Write DMAIntControl while channel %d active: 0x%08x\n", x, V);
	     // }
	     //}
	     DMAIntControl = V & 0x00ff803f;
	     DMAIntStatus &= ~(V >> 24);

	     //if(DMAIntStatus ^ (DMAIntStatus & (V >> 16)))
	     // fprintf(stderr, "DMAINT Fudge: %02x\n", DMAIntStatus ^ (DMAIntStatus & (V >> 16)));
	     DMAIntStatus &= (V >> 16);	// THIS IS ALMOST CERTAINLY WRONG AND A HACK.  Remove when CDC emulation is better.
     	     RecalcIRQOut();
	     break;

   default: PSX_WARNING("[DMA] Unknown write: %08x %08x", A, V);
		assert(0);
	    break;
  }
  return;
 }
 switch(A & 0xC)
 {
  case 0x0: DMACH[ch].BaseAddr = V & 0xFFFFFF;
	    break;

  case 0x4: DMACH[ch].BlockControl = V;
	    break;

  case 0xC:
  case 0x8: 
	   {
	    uint32 OldCC = DMACH[ch].ChanControl;

	    //
            // Kludge for DMA timing granularity and other issues.  Needs to occur before setting all bits of ChanControl to the new value, to accommodate the
	    // case of a game cancelling DMA and changing the type of DMA(read/write, etc.) at the same time.
            //
	    if((DMACH[ch].ChanControl & (1 << 24)) && !(V & (1 << 24)))
	    {
	     DMACH[ch].ChanControl &= ~(1 << 24);	// Clear bit before RunChannel(), so it will only finish the block it's on at most.
	     RunChannel(timestamp, 128 * 16, ch);
	     DMACH[ch].BlockCounter = 0;
	     DMACH[ch].WordCounter = 0;

#if 0	// TODO(maybe, need to work out worst-case performance for abnormally/brokenly large block sizes)
	     DMACH[ch].ClockCounter = (1 << 30);
	     RunChannel(timestamp, 1, ch);
	     DMACH[ch].ClockCounter = 0;
#endif

	     if(ch == 2)
	     {
	      GPU->AbortDMA();
	     }

	     PSX_WARNING("[DMA] Forced stop for channel %d -- scanline=%d", ch, GPU->GetScanlineNum());
	     //MDFN_DispMessage("[DMA] Forced stop for channel %d", ch);
	    }

	    if(ch == 6)
	     DMACH[ch].ChanControl = V & 0x51000002; 	// Not 100% sure, but close.
	    else
	     DMACH[ch].ChanControl = V & 0x71770703;

	    if(!(OldCC & (1 << 24)) && (V & (1 << 24)))
	    {
	     //if(ch == 2)
		//if(ch == 4)
	     //PSX_WARNING("[DMA] Started DMA for channel=%d --- CHCR=0x%08x --- BCR=0x%08x --- scanline=%d", ch, DMACH[ch].ChanControl, DMACH[ch].BlockControl, GPU->GetScanlineNum());

	     DMACH[ch].ClockCounter = 0;
	     DMACH[ch].NextAddr = DMACH[ch].BaseAddr & 0x1FFFFC;
	     DMACH[ch].BlockCounter = DMACH[ch].BlockControl >> 16;

	     //
	     // Viewpoint starts a short MEM->GPU LL DMA and apparently has race conditions that can cause a crash if it doesn't finish almost immediately(
	     // or at least very quickly, which the current DMA granularity has issues with, so run the channel ahead a bit to take of this issue and potentially
	     // games with similar issues).
	     //
	     // Though, Viewpoint isn't exactly a good game, so maybe we shouldn't bother? ;)
	     //
	     RunChannel(timestamp, 64, ch);	//std::max<int>(128 - DMACycleCounter, 1)); //64); //1); //128 - DMACycleCounter);
	    }

	    RecalcHalt();
	   }
	   break;
 }
 PSX_SetEventNT(PSX_EVENT_DMA, timestamp + CalcNextEvent(0x10000000));
}

uint32 DMA_Read(const pscpu_timestamp_t timestamp, uint32 A)
{
 int ch = (A & 0x7F) >> 4;
 uint32 ret = 0;

 if(ch == 7)
 {
  switch(A & 0xC)
  {
   default: PSX_WARNING("[DMA] Unknown read: %08x", A);
		assert(0);
	    break;

   case 0x0: ret = DMAControl;
	     break;

   case 0x4: ret = DMAIntControl | (DMAIntStatus << 24) | (IRQOut << 31);
	     break;
  }
 }
 else switch(A & 0xC)
 {
  case 0x0: ret = DMACH[ch].BaseAddr;
  	    break;

  case 0x4: ret = DMACH[ch].BlockControl;
	    break;

  case 0xC:
  case 0x8: ret = DMACH[ch].ChanControl;
            break;

 }

 ret >>= (A & 3) * 8;

 //PSX_WARNING("[DMA] Read: %08x %08x", A, ret);

 return(ret);
}


int DMA_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(DMACycleCounter),
  SFVAR(DMAControl),
  SFVAR(DMAIntControl),
  SFVAR(DMAIntStatus),
  SFVAR(IRQOut),

#define SFDMACH(n)	SFVARN(DMACH[n].BaseAddr, #n "BaseAddr"),		\
			SFVARN(DMACH[n].BlockControl, #n "BlockControl"),	\
			SFVARN(DMACH[n].ChanControl, #n "ChanControl"),		\
			SFVARN(DMACH[n].CurAddr, #n "CurAddr"),			\
			SFVARN(DMACH[n].NextAddr, #n "NextAddr"),		\
			SFVARN(DMACH[n].BlockCounter, #n "BlockCounter"),	\
			SFVARN(DMACH[n].WordCounter, #n "WordCounter"),		\
			SFVARN(DMACH[n].ClockCounter, #n "ClockCounter")

  SFDMACH(0),
  SFDMACH(1),
  SFDMACH(2),
  SFDMACH(3),
  SFDMACH(4),
  SFDMACH(5),
  SFDMACH(6),

#undef SFDMACH

  SFVAR(lastts),

  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "DMA");

 if(load)
 {

 }

 return(ret);
}


}
