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

// Notes: DMA tested to abort when 

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
 CH_FIVE = 5,
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
 uint16 WordCounter; 

 //
 //
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
 unsigned ch = 0;

 for(ch = 0; ch < 7; ch++)
 {
  if(DMACH[ch].ChanControl & (1U << 24))
  {
   if(!(DMACH[ch].ChanControl & (7U << 8)))
   {
    if(DMACH[ch].WordCounter > 0)
    {
     Halt = true;
     break;
    }
   }

#if 0
   if(DMACH[ch].ChanControl & 0x100)	// DMA doesn't hog the bus when this bit is set, though the DMA takes longer.
    continue;

   if(ch == 4 || ch == 5)	// Not sure if these channels will typically hog the bus or not...investigate.
    continue;

   if(!(DMACH[ch].ChanControl & (1U << 10)))	// Not sure about HOGGERYNESS with linked-list mode, and it likely wouldn't work well either in regards
						// to GPU commands due to the rather large DMA update granularity.
   {
    if((DMACH[ch].WordCounter > 0) || ChCan(ch, DMACH[ch].ChanControl & 0x1))
    {
     Halt = true;
     break;
    }
   }
#endif
  }
 }

#if 0
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
#endif

 //printf("Halt: %d\n", Halt);

 CPU->SetHalt(Halt);
}


static INLINE void ChRW(const unsigned ch, const uint32 CRModeCache, uint32 *V)
{
 unsigned extra_cyc_overhead = 0;

 switch(ch)
 {
  default:
	abort();
	break;

  case CH_MDEC_IN:
	  if(CRModeCache & 0x1)
	   MDEC_DMAWrite(*V);
	  else
	   *V = 0;
	  break;

  case CH_MDEC_OUT:
	  if(CRModeCache & 0x1)
	  {
	  }
	  else
	   *V = MDEC_DMARead();
	  break;

  case CH_GPU:
	  if(CRModeCache & 0x1)
	   GPU->WriteDMA(*V);
	  else
	   *V = GPU->ReadDMA();
	  break;

  case CH_CDC:
	  // 0x1f801018 affects CDC DMA timing.
#if 0
	  if(CRModeCache & 0x100)		// For CDC DMA(at least): When this bit is set, DMA controller doesn't appear to hog the (RAM?) bus.
	  {
	   if(CRModeCache & 0x00400000)	// For CDC DMA(at least): When this bit is set, DMA controller appears to get even less bus time(or has a lower priority??)
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
	  if(CRModeCache & 0x1)
	  {
	  }
	  else
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

	  extra_cyc_overhead = 47;	// Should be closer to 69, average, but actual timing is...complicated.

	  if(CRModeCache & 0x1)
	   SPU->WriteDMA(*V);
	  else
	   *V = SPU->ReadDMA();
	  break;

  case CH_FIVE:
	  if(CRModeCache & 0x1)
	  {
	  }
	  else
	  {
	   *V = 0;
	  }
	  break;

  case CH_OT:
	  if(DMACH[ch].WordCounter == 1)
	   *V = 0xFFFFFF;
	  else
	   *V = (DMACH[ch].CurAddr - 4) & 0x1FFFFF;
	  break;
 }

 // GROSS APPROXIMATION, shoehorning multiple effects together, TODO separate(especially SPU and CDC)
 DMACH[ch].ClockCounter -= std::max<int>(extra_cyc_overhead, (CRModeCache & 0x100) ? 7 : 0);
}

static INLINE bool ChCan(const unsigned ch, const uint32 CRModeCache)
{
 switch(ch)
 {
  default:
	abort();

  case CH_MDEC_IN:
	return(MDEC_DMACanWrite());

  case CH_MDEC_OUT:
	return(MDEC_DMACanRead());
  
  case CH_GPU: 
	if(CRModeCache & 0x1)
	 return(GPU->DMACanWrite());
	else
	 return(true);

  case CH_CDC:
	return(true);

  case CH_SPU:
	return(true);

  case CH_FIVE:
	return(false);

  case CH_OT:
	 return((bool)(DMACH[ch].ChanControl & (1U << 28)));
 }
}

//
// Remember to handle an end condition on the same iteration of the while(DMACH[ch].ClockCounter > 0) loop that caused it,
// otherwise RecalcHalt() might take the CPU out of a halted state before the end-of-DMA is signaled(especially a problem considering our largeish
// DMA update timing granularity).
//
static INLINE void RunChannelI(const unsigned ch, const uint32 CRModeCache, int32 clocks)
{
 //const uint32 dc = (DMAControl >> (ch * 4)) & 0xF;

 DMACH[ch].ClockCounter += clocks;

 while(MDFN_LIKELY(DMACH[ch].ClockCounter > 0))
 {
  if(DMACH[ch].WordCounter == 0)	// Begin WordCounter reload.
  {
   if(!(DMACH[ch].ChanControl & (1 << 24)))	// Needed for the forced-DMA-stop kludge(see DMA_Write()).
    break;

   if(!ChCan(ch, CRModeCache))
    break;

   DMACH[ch].CurAddr = DMACH[ch].BaseAddr;

   if(CRModeCache & (1U << 10))
   {
    uint32 header;

    if(MDFN_UNLIKELY(DMACH[ch].CurAddr & 0x800000))
    {
     DMACH[ch].ChanControl &= ~(0x11 << 24);
     DMAIntControl |= 0x8000;
     RecalcIRQOut();
     break;
    }

    header = MainRAM.ReadU32(DMACH[ch].CurAddr & 0x1FFFFC);
    DMACH[ch].CurAddr = (DMACH[ch].CurAddr + 4) & 0xFFFFFF;

    DMACH[ch].WordCounter = header >> 24;
    DMACH[ch].BaseAddr = header & 0xFFFFFF;

    // printf to debug Soul Reaver ;)
    //if(DMACH[ch].WordCounter > 0x10) 
    // printf("What the lala?  0x%02x @ 0x%08x\n", DMACH[ch].WordCounter, DMACH[ch].CurAddr - 4);

    if(DMACH[ch].WordCounter)
     DMACH[ch].ClockCounter -= 15;
    else
     DMACH[ch].ClockCounter -= 10;

    goto SkipPayloadStuff;	// 3 cheers for gluten-free spaghetticode(necessary because the newly-loaded WordCounter might be 0, and we actually
				// want 0 to mean 0 and not 65536 in this context)!
   }
   else
   {
    DMACH[ch].WordCounter = DMACH[ch].BlockControl & 0xFFFF;

    if(CRModeCache & (1U << 9))
     DMACH[ch].BlockControl = (DMACH[ch].BlockControl & 0xFFFF) | ((DMACH[ch].BlockControl - (1U << 16)) & 0xFFFF0000);
   }
  }	// End WordCounter reload.
  else if(CRModeCache & 0x100) // BLARGH BLARGH FISHWHALE
  {
   //printf("LoadWC: %u(oldWC=%u)\n", DMACH[ch].BlockControl & 0xFFFF, DMACH[ch].WordCounter);
   //MDFN_DispMessage("SPOOOON\n");
   DMACH[ch].CurAddr = DMACH[ch].BaseAddr;
   DMACH[ch].WordCounter = DMACH[ch].BlockControl & 0xFFFF;
  }

  //
  // Do the payload read/write
  //
  {
   uint32 vtmp;

   if(MDFN_UNLIKELY(DMACH[ch].CurAddr & 0x800000))
   {
    DMACH[ch].ChanControl &= ~(0x11 << 24);
    DMAIntControl |= 0x8000;
    RecalcIRQOut();
    break;
   }

   if(CRModeCache & 0x1)
    vtmp = MainRAM.ReadU32(DMACH[ch].CurAddr & 0x1FFFFC);

   ChRW(ch, CRModeCache, &vtmp);

   if(!(CRModeCache & 0x1))
    MainRAM.WriteU32(DMACH[ch].CurAddr & 0x1FFFFC, vtmp);
  }

  if(CRModeCache & 0x2)
   DMACH[ch].CurAddr = (DMACH[ch].CurAddr - 4) & 0xFFFFFF;
  else
   DMACH[ch].CurAddr = (DMACH[ch].CurAddr + 4) & 0xFFFFFF;

  DMACH[ch].WordCounter--;
  DMACH[ch].ClockCounter--;

  SkipPayloadStuff: ;

  if(CRModeCache & 0x100) // BLARGH BLARGH WHALEFISH
  {
   DMACH[ch].BaseAddr = DMACH[ch].CurAddr;
   DMACH[ch].BlockControl = (DMACH[ch].BlockControl & 0xFFFF0000) | DMACH[ch].WordCounter;
   //printf("SaveWC: %u\n", DMACH[ch].WordCounter);
  }

  //
  // Handle channel end condition:
  //
  if(DMACH[ch].WordCounter == 0)
  {
   bool ChannelEndTC = false;

   if(!(DMACH[ch].ChanControl & (1 << 24)))	// Needed for the forced-DMA-stop kludge(see DMA_Write()).
    break;

   switch((CRModeCache >> 9) & 0x3)
   {
    case 0x0:
	ChannelEndTC = true;
	break;

    case 0x1:
	DMACH[ch].BaseAddr = DMACH[ch].CurAddr;
	if((DMACH[ch].BlockControl >> 16) == 0)
   	 ChannelEndTC = true;
	break;

    case 0x2:
    case 0x3:	// Not sure about 0x3.
	if(DMACH[ch].BaseAddr == 0xFFFFFF)
	 ChannelEndTC = true;
	break;
   }

   if(ChannelEndTC)
   {
    DMACH[ch].ChanControl &= ~(0x11 << 24);
    if(DMAIntControl & (1U << (16 + ch)))
    {
     DMAIntStatus |= 1U << ch;
     RecalcIRQOut();
    }
    break;
   }
  }
 }

 if(DMACH[ch].ClockCounter > 0)
  DMACH[ch].ClockCounter = 0;
}

static INLINE void RunChannel(pscpu_timestamp_t timestamp, int32 clocks, int ch)
{
 // Mask out the bits that the DMA controller will modify during the course of operation.
 const uint32 CRModeCache = DMACH[ch].ChanControl &~(0x11 << 24);

 switch(ch)
 {
  default: abort();

  case 0:
	if(MDFN_LIKELY(CRModeCache == 0x00000201))
	 RunChannelI(0, 0x00000201, clocks);
	else
	 RunChannelI(0, CRModeCache, clocks);
	break;

  case 1:
	if(MDFN_LIKELY(CRModeCache == 0x00000200))
	 RunChannelI(1, 0x00000200, clocks);
	else
	 RunChannelI(1, CRModeCache, clocks);
	break;

  case 2:
	if(MDFN_LIKELY(CRModeCache == 0x00000401))
	 RunChannelI(2, 0x00000401, clocks);
	else if(MDFN_LIKELY(CRModeCache == 0x00000201))
	 RunChannelI(2, 0x00000201, clocks);
	else if(MDFN_LIKELY(CRModeCache == 0x00000200))
	 RunChannelI(2, 0x00000200, clocks);
	else
	 RunChannelI(2, CRModeCache, clocks);
	break;

  case 3:
	if(MDFN_LIKELY(CRModeCache == 0x00000000))
	 RunChannelI(3, 0x00000000, clocks);
	else if(MDFN_LIKELY(CRModeCache == 0x00000100))
	 RunChannelI(3, 0x00000100, clocks);
	else
	 RunChannelI(3, CRModeCache, clocks);
	break;

  case 4:
	if(MDFN_LIKELY(CRModeCache == 0x00000201))
	 RunChannelI(4, 0x00000201, clocks);
	else if(MDFN_LIKELY(CRModeCache == 0x00000200))
	 RunChannelI(4, 0x00000200, clocks);
	else
	 RunChannelI(4, CRModeCache, clocks);
	break;

  case 5:
	RunChannelI(5, CRModeCache, clocks);
	break;

  case 6:
	if(MDFN_LIKELY(CRModeCache == 0x00000002))
	 RunChannelI(6, 0x00000002, clocks);
	else
	 RunChannelI(6, CRModeCache, clocks);
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

	    //printf("CHCR: %u, %08x --- 0x%08x\n", ch, V, DMACH[ch].BlockControl);
	    //
            // Kludge for DMA timing granularity and other issues.  Needs to occur before setting all bits of ChanControl to the new value, to accommodate the
	    // case of a game cancelling DMA and changing the type of DMA(read/write, etc.) at the same time.
            //
	    if((DMACH[ch].ChanControl & (1 << 24)) && !(V & (1 << 24)))
	    {
	     DMACH[ch].ChanControl &= ~(1 << 24);	// Clear bit before RunChannel(), so it will only finish the block it's on at most.
	     RunChannel(timestamp, 128 * 16, ch);
	     DMACH[ch].WordCounter = 0;

#if 0	// TODO(maybe, need to work out worst-case performance for abnormally/brokenly large block sizes)
	     DMACH[ch].ClockCounter = (1 << 30);
	     RunChannel(timestamp, 1, ch);
	     DMACH[ch].ClockCounter = 0;
#endif
	     PSX_WARNING("[DMA] Forced stop for channel %d -- scanline=%d", ch, GPU->GetScanlineNum());
	     //MDFN_DispMessage("[DMA] Forced stop for channel %d", ch);
	    }

	    if(ch == 6)
	     DMACH[ch].ChanControl = (V & 0x51000000) | 0x2;
	    else
	     DMACH[ch].ChanControl = V & 0x71770703;

	    if(!(OldCC & (1 << 24)) && (V & (1 << 24)))
	    {
	     //PSX_WARNING("[DMA] Started DMA for channel=%d --- CHCR=0x%08x --- BCR=0x%08x --- scanline=%d", ch, DMACH[ch].ChanControl, DMACH[ch].BlockControl, GPU->GetScanlineNum());

	     DMACH[ch].WordCounter = 0;
	     DMACH[ch].ClockCounter = 0;

	     //
	     // Viewpoint starts a short MEM->GPU LL DMA and apparently has race conditions that can cause a crash if it doesn't finish almost immediately(
	     // or at least very quickly, which the current DMA granularity has issues with, so run the channel ahead a bit to take of this issue and potentially
	     // games with similar issues).
	     //
	     // Though, Viewpoint isn't exactly a good game, so maybe we shouldn't bother? ;)
	     //
	     // Also, it's needed for RecalcHalt() to work with some semblance of workiness.
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
