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
#include "frontio.h"
#include "timer.h"
#include "sio.h"
#include "cdc.h"
#include "spu.h"
#include "../mempatcher.h"
#include "../PSFLoader.h"
#include "../player.h"
#include "../cputest/cputest.h"

extern MDFNGI EmulatedPSX;

namespace MDFN_IEN_PSX
{

class PSF1Loader : public PSFLoader
{
 public:

 PSF1Loader(MDFNFILE *fp);
 virtual ~PSF1Loader();

 virtual void HandleEXE(const uint8 *data, uint32 len, bool ignore_pcsp = false);

 PSFTags tags;
};

enum
{
 REGION_JP = 0,
 REGION_NA = 1,
 REGION_EU = 2,
};

#if 0
static uint32 PortReadCounter[0x4000] = { 0 };	// Debugging(performance)
static uint32 ReadCounter = 0;
static uint32 WriteCounter = 0;
#endif

static PSF1Loader *psf_loader = NULL;
static std::vector<CDIF*> *cdifs = NULL;
static std::vector<const char *> cdifs_scex_ids;
static bool CD_TrayOpen;
static int CD_SelectedDisc;     // -1 for no disc

static uint64 Memcard_PrevDC[8];
static int64 Memcard_SaveDelay[8];

PS_CPU *CPU = NULL;
PS_SPU *SPU = NULL;
PS_GPU *GPU = NULL;
PS_CDC *CDC = NULL;
FrontIO *FIO = NULL;

static MultiAccessSizeMem<512 * 1024, uint32, false, 0, 0> *BIOSROM = NULL;
static MultiAccessSizeMem<65536, uint32, false, 0, 0> *PIOMem = NULL;

MultiAccessSizeMem<2048 * 1024, uint32, false, 0, 0> MainRAM;

static uint32 TextMem_Start;
static std::vector<uint8> TextMem;

static MultiAccessSizeMem<1024, uint32, false, 0, 0> ScratchRAM;

static const uint32 SysControl_Mask[9] = { 0x00ffffff, 0x00ffffff, 0xffffffff, 0x2f1fffff,
					   0xffffffff, 0x2f1fffff, 0x2f1fffff, 0xffffffff,
					   0x0003ffff };

static const uint32 SysControl_OR[9] = { 0x1f000000, 0x1f000000, 0x00000000, 0x00000000,
					 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					 0x00000000 };

static struct
{
 union
 {
  struct
  {
   uint32 PIO_Base;	// 0x1f801000	// BIOS Init: 0x1f000000, Writeable bits: 0x00ffffff(assumed, verify), FixedOR = 0x1f000000
   uint32 Unknown0;	// 0x1f801004	// BIOS Init: 0x1f802000, Writeable bits: 0x00ffffff, FixedOR = 0x1f000000
   uint32 Unknown1;	// 0x1f801008	// BIOS Init: 0x0013243f, ????
   uint32 Unknown2;	// 0x1f80100c	// BIOS Init: 0x00003022, Writeable bits: 0x2f1fffff, FixedOR = 0x00000000
   
   uint32 BIOS_Mapping;	// 0x1f801010	// BIOS Init: 0x0013243f, ????
   uint32 SPU_Delay;	// 0x1f801014	// BIOS Init: 0x200931e1, Writeable bits: 0x2f1fffff, FixedOR = 0x00000000 - Affects bus timing on access to SPU
   uint32 CDC_Delay;	// 0x1f801018	// BIOS Init: 0x00020843, Writeable bits: 0x2f1fffff, FixedOR = 0x00000000
   uint32 Unknown4;	// 0x1f80101c	// BIOS Init: 0x00070777, ????
   uint32 Unknown5;	// 0x1f801020	// BIOS Init: 0x00031125(but rewritten with other values often), Writeable bits: 0x0003ffff, FixedOR = 0x00000000 -- Possibly CDC related
  };
  uint32 Regs[9];
 };
} SysControl;

//
// Event stuff
//
static pscpu_timestamp_t next_timestamps[6];

static void EventReset(void)
{
 next_timestamps[PSX_EVENT_GPU] = PSX_EVENT_MAXTS;
 next_timestamps[PSX_EVENT_CDC] = PSX_EVENT_MAXTS;
 //next_timestamps[PSX_EVENT_SPU] = PSX_EVENT_MAXTS;
 next_timestamps[PSX_EVENT_TIMER] = PSX_EVENT_MAXTS;
 next_timestamps[PSX_EVENT_DMA] = PSX_EVENT_MAXTS;
 next_timestamps[PSX_EVENT_FIO] = PSX_EVENT_MAXTS;
}

static INLINE pscpu_timestamp_t CalcNextTS(void)
{
 pscpu_timestamp_t next_timestamp = next_timestamps[PSX_EVENT_GPU];

 if(next_timestamp > next_timestamps[PSX_EVENT_CDC])
  next_timestamp  = next_timestamps[PSX_EVENT_CDC];

 //if(next_timestamp > next_timestamps[PSX_EVENT_SPU])
 // next_timestamp  = next_timestamps[PSX_EVENT_SPU];

 if(next_timestamp > next_timestamps[PSX_EVENT_TIMER])
  next_timestamp  = next_timestamps[PSX_EVENT_TIMER];

 if(next_timestamp > next_timestamps[PSX_EVENT_DMA])
  next_timestamp  = next_timestamps[PSX_EVENT_DMA];

 if(next_timestamp > next_timestamps[PSX_EVENT_FIO])
  next_timestamp  = next_timestamps[PSX_EVENT_FIO];

 //printf("%d %d %d %d %d %d -- %08x\n", next_timestamps[PSX_EVENT_GPU], next_timestamps[PSX_EVENT_CDC], next_timestamps[PSX_EVENT_SPU], next_timestamps[PSX_EVENT_TIMER], next_timestamps[PSX_EVENT_DMA], next_timestamps[PSX_EVENT_FIO], next_timestamp);

 return(next_timestamp);
}

static void RebaseTS(const pscpu_timestamp_t timestamp)
{
 //printf("Rebase: %08x %08x %08x\n", timestamp, next_vip_ts, next_timestamps[PSX_EVENT_TIMER]);
 assert(next_timestamps[PSX_EVENT_GPU] > timestamp);
 assert(next_timestamps[PSX_EVENT_CDC] > timestamp);
 //assert(next_timestamps[PSX_EVENT_SPU] > timestamp);
 assert(next_timestamps[PSX_EVENT_TIMER] > timestamp);
 assert(next_timestamps[PSX_EVENT_DMA] > timestamp);
 assert(next_timestamps[PSX_EVENT_FIO] > timestamp);

 next_timestamps[PSX_EVENT_GPU] -= timestamp;
 next_timestamps[PSX_EVENT_CDC] -= timestamp;
 //next_timestamps[PSX_EVENT_SPU] -= timestamp;
 next_timestamps[PSX_EVENT_TIMER] -= timestamp;
 next_timestamps[PSX_EVENT_DMA] -= timestamp;
 next_timestamps[PSX_EVENT_FIO] -= timestamp;

 CPU->SetEventNT(CalcNextTS());
}

void PSX_SetEventNT(const int type, const pscpu_timestamp_t next_timestamp)
{
 //assert(next_timestamp > VB_V810->v810_timestamp);

 assert(type >= 0 && type < 6);

 next_timestamps[type] = next_timestamp;

 if(next_timestamp < CPU->GetEventNT())
  CPU->SetEventNT(next_timestamp);
}

static void ForceEventUpdates(const pscpu_timestamp_t timestamp)
{
 next_timestamps[PSX_EVENT_GPU] = GPU->Update(timestamp);
 next_timestamps[PSX_EVENT_CDC] = CDC->Update(timestamp);
 //next_timestamps[PSX_EVENT_SPU] = SPU->Update(timestamp);

 next_timestamps[PSX_EVENT_TIMER] = TIMER_Update(timestamp);

 next_timestamps[PSX_EVENT_DMA] = DMA_Update(timestamp);

 next_timestamps[PSX_EVENT_FIO] = FIO->Update(timestamp);

 CPU->SetEventNT(CalcNextTS());
}


pscpu_timestamp_t MDFN_FASTCALL PSX_EventHandler(const pscpu_timestamp_t timestamp)
{
 // FIXME: This seems rather inefficient.

 if(timestamp >= next_timestamps[PSX_EVENT_GPU])
  next_timestamps[PSX_EVENT_GPU] = GPU->Update(timestamp);

 if(timestamp >= next_timestamps[PSX_EVENT_CDC])
  next_timestamps[PSX_EVENT_CDC] = CDC->Update(timestamp);

 //if(timestamp >= next_timestamps[PSX_EVENT_SPU])
 // next_timestamps[PSX_EVENT_SPU] = SPU->Update(timestamp);

 if(timestamp >= next_timestamps[PSX_EVENT_TIMER])
  next_timestamps[PSX_EVENT_TIMER] = TIMER_Update(timestamp);

 if(timestamp >= next_timestamps[PSX_EVENT_DMA])
  next_timestamps[PSX_EVENT_DMA] = DMA_Update(timestamp);

 if(timestamp >= next_timestamps[PSX_EVENT_FIO])
  next_timestamps[PSX_EVENT_FIO] = FIO->Update(timestamp);

 return(CalcNextTS());
}


//
// End event stuff
//

void DMA_CheckReadDebug(uint32 A);

template<typename T, bool IsWrite, bool Access24, bool Peek> static INLINE void MemRW(const pscpu_timestamp_t timestamp, uint32 A, uint32 &V)
{
 static const uint32 mask[8] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
				 0x7FFFFFFF, 0x1FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

 //if(IsWrite)
 // V = (T)V;

 if(!Peek)
 {
  #if 0
  if(IsWrite)
   printf("Write%d: %08x(orig=%08x), %08x\n", (int)(sizeof(T) * 8), A & mask[A >> 29], A, V);
  else
   printf("Read%d: %08x(orig=%08x)\n", (int)(sizeof(T) * 8), A & mask[A >> 29], A);
  #endif
 }

 A &= mask[A >> 29];

 //if(A == 0xa0 && IsWrite)
 // DBG_Break();

 if(A < 0x00800000)
 //if(A <= 0x1FFFFF)
 {
  //DMA_CheckReadDebug(A);
  //assert(A <= 0x1FFFFF);
  if(Access24)
  {
   if(IsWrite)
    MainRAM.WriteU24(A & 0x1FFFFF, V);
   else
    V = MainRAM.ReadU24(A & 0x1FFFFF);
  }
  else
  {
   if(IsWrite)
    MainRAM.Write<T>(A & 0x1FFFFF, V);
   else
    V = MainRAM.Read<T>(A & 0x1FFFFF);
  }

  return;
 }

 if(A >= 0x1F800000 && A <= 0x1F8003FF)
 {
  if(Access24)
  {
   if(IsWrite)
    ScratchRAM.WriteU24(A & 0x3FF, V);
   else
    V = ScratchRAM.ReadU24(A & 0x3FF);
  }
  else
  {
   if(IsWrite)
    ScratchRAM.Write<T>(A & 0x3FF, V);
   else
    V = ScratchRAM.Read<T>(A & 0x3FF);
  }
  return;
 }

 if(A >= 0x1FC00000 && A <= 0x1FC7FFFF)
 {
  if(!IsWrite)
  {
   if(Access24)
    V = BIOSROM->ReadU24(A & 0x7FFFF);
   else
    V = BIOSROM->Read<T>(A & 0x7FFFF);
  }

  return;
 }

 if(A >= 0x1F801000 && A <= 0x1F802FFF && !Peek)	// Hardware register region. (TODO: Implement proper peek suppor)
 {
#if 0
  if(!IsWrite)
  {
   ReadCounter++;
   PortReadCounter[A & 0x3FFF]++;
  }
  else
   WriteCounter++;
#endif

  //if(IsWrite)
  // printf("HW Write%d: %08x %08x\n", (unsigned int)(sizeof(T)*8), (unsigned int)A, (unsigned int)V);
  //else
  // printf("HW Read%d: %08x\n", (unsigned int)(sizeof(T)*8), (unsigned int)A);

  if(A >= 0x1F801C00 && A <= 0x1F801FFF) // SPU
  {
   if(sizeof(T) == 4 && !Access24)
   {
    if(IsWrite)
    {
     SPU->Write(timestamp, A | 0, V);
     SPU->Write(timestamp, A | 2, V >> 16);
    }
    else
    {
     V = SPU->Read(timestamp, A) | (SPU->Read(timestamp, A | 2) << 16);
    }
   }
   else
   {
    if(IsWrite)
     SPU->Write(timestamp, A & ~1, V);
    else
     V = SPU->Read(timestamp, A & ~1);
   }
   return;
  }		// End SPU

  if(A >= 0x1f801800 && A <= 0x1f80180F)
  {
   if(IsWrite)
    CDC->Write(timestamp, A & 0x3, V);
   else
    V = CDC->Read(timestamp, A & 0x3);

   return;
  }

  if(A >= 0x1F801810 && A <= 0x1F801817)
  {
   if(IsWrite)
    GPU->Write(timestamp, A, V);
   else
    V = GPU->Read(timestamp, A);

   return;
  }

  if(A >= 0x1F801820 && A <= 0x1F801827)
  {
   if(IsWrite)
    MDEC_Write(timestamp, A, V);
   else
    V = MDEC_Read(timestamp, A);

   return;
  }

  if(A >= 0x1F801000 && A <= 0x1F801023)
  {
   unsigned index = (A & 0x1F) >> 2;

   //if(A == 0x1F801014 && IsWrite)
   // fprintf(stderr, "%08x %08x\n",A,V);

   if(IsWrite)
   {
    V <<= (A & 3) * 8;
    SysControl.Regs[index] = V & SysControl_Mask[index];
   }
   else
   {
    V = SysControl.Regs[index] | SysControl_OR[index];
    V >>= (A & 3) * 8;
   }
   return;
  }

  if(A >= 0x1F801040 && A <= 0x1F80104F)
  {
   if(IsWrite)
    FIO->Write(timestamp, A, V);
   else
    V = FIO->Read(timestamp, A);
   return;
  }

  if(A >= 0x1F801050 && A <= 0x1F80105F)
  {
   if(IsWrite)
    SIO_Write(timestamp, A, V);
   else
    V = SIO_Read(timestamp, A);
   return;
  }

#if 0
  if(A >= 0x1F801060 && A <= 0x1F801063)
  {
   if(IsWrite)
   {

   }
   else
   {

   }

   return;
  }
#endif

  if(A >= 0x1F801070 && A <= 0x1F801077)	// IRQ
  {
   if(IsWrite)
    IRQ_Write(A, V);
   else
    V = IRQ_Read(A);
   return;
  }

  if(A >= 0x1F801080 && A <= 0x1F8010FF) 	// DMA
  {
   if(IsWrite)
    DMA_Write(timestamp, A, V);
   else
    V = DMA_Read(timestamp, A);

   return;
  }

  if(A >= 0x1F801100 && A <= 0x1F80112F)	// Root counters
  {
   if(IsWrite)
    TIMER_Write(timestamp, A, V);
   else
    V = TIMER_Read(timestamp, A);

   return;
  }
 }


 if(A >= 0x1F000000 && A <= 0x1F7FFFFF)
 {
  if(!IsWrite)
  {
   //if((A & 0x7FFFFF) <= 0x84)
   // PSX_WARNING("[PIO] Read%d from %08x at time %d", (int)(sizeof(T) * 8), A, timestamp);

   V = 0;

   if((A & 0x7FFFFF) < 65536)
   {
    if(Access24)
     V = PIOMem->ReadU24(A & 0x7FFFFF);
    else
     V = PIOMem->Read<T>(A & 0x7FFFFF);
   }
   else if((A & 0x7FFFFF) < (65536 + TextMem.size()))
   {
    if(Access24)
     V = MDFN_de24lsb(&TextMem[(A & 0x7FFFFF) - 65536]);
    else switch(sizeof(T))
    {
     case 1: V = TextMem[(A & 0x7FFFFF) - 65536]; break;
     case 2: V = MDFN_de16lsb(&TextMem[(A & 0x7FFFFF) - 65536]); break;
     case 4: V = MDFN_de32lsb(&TextMem[(A & 0x7FFFFF) - 65536]); break;
    }
   }
  }
  return;
 }

 if(!Peek)
 {
  if(IsWrite)
  {
   PSX_WARNING("[MEM] Unknown write%d to %08x at time %d, =%08x(%d)", (int)(sizeof(T) * 8), A, timestamp, V, V);
  }
  else
  {
   V = 0;
   PSX_WARNING("[MEM] Unknown read%d from %08x at time %d", (int)(sizeof(T) * 8), A, timestamp);
  }
 }
 else
  V = 0;

}

void MDFN_FASTCALL PSX_MemWrite8(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 MemRW<uint8, true, false, false>(timestamp, A, V);
}

void MDFN_FASTCALL PSX_MemWrite16(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 MemRW<uint16, true, false, false>(timestamp, A, V);
}

void MDFN_FASTCALL PSX_MemWrite24(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 //assert(0);
 MemRW<uint32, true, true, false>(timestamp, A, V);
}

void MDFN_FASTCALL PSX_MemWrite32(const pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 MemRW<uint32, true, false, false>(timestamp, A, V);
}

uint8 MDFN_FASTCALL PSX_MemRead8(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 V;

 MemRW<uint8, false, false, false>(timestamp, A, V);

 return(V);
}

uint16 MDFN_FASTCALL PSX_MemRead16(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 V;

 MemRW<uint16, false, false, false>(timestamp, A, V);

 return(V);
}

uint32 MDFN_FASTCALL PSX_MemRead24(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 V;

 //assert(0);
 MemRW<uint32, false, true, false>(timestamp, A, V);

 return(V);
}

uint32 MDFN_FASTCALL PSX_MemRead32(const pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 V;

 MemRW<uint32, false, false, false>(timestamp, A, V);

 return(V);
}


uint8 PSX_MemPeek8(uint32 A)
{
 uint32 V;

 MemRW<uint8, false, false, true>(0, A, V);

 return(V);
}

uint16 PSX_MemPeek16(uint32 A)
{
 uint32 V;

 MemRW<uint16, false, false, true>(0, A, V);

 return(V);
}

uint32 PSX_MemPeek32(uint32 A)
{
 uint32 V;

 MemRW<uint32, false, false, true>(0, A, V);

 return(V);
}

void PSX_Power(void)
{
 memset(MainRAM.data32, 0, 2048 * 1024);
 memset(ScratchRAM.data32, 0, 1024);

 for(unsigned i = 0; i < 9; i++)
  SysControl.Regs[i] = 0;

 CPU->Power();

 EventReset();

 TIMER_Power();

 DMA_Power();

 FIO->Power();
 SIO_Power();

 MDEC_Power();
 CDC->Power();
 GPU->Power();
 //SPU->Power();	// Called from CDC->Power()
 IRQ_Power();

 ForceEventUpdates(0);
}

void PSX_RequestMLExit(void)
{
 CPU->Exit();
}

}

using namespace MDFN_IEN_PSX;


static void Emulate(EmulateSpecStruct *espec)
{
 pscpu_timestamp_t timestamp = 0;

 MDFNGameInfo->mouse_sensitivity = MDFN_GetSettingF("psx.input.mouse_sensitivity");

 MDFNMP_ApplyPeriodicCheats();


 espec->MasterCycles = 0;
 espec->SoundBufSize = 0;

 FIO->UpdateInput();
 GPU->StartFrame(psf_loader ? NULL : espec);
 SPU->StartFrame(espec->SoundRate, MDFN_GetSettingUI("psx.spu.resamp_quality"));

 timestamp = CPU->Run(timestamp, psf_loader != NULL);

 //printf("Timestamp: %d\n", timestamp);

 assert(timestamp);

 ForceEventUpdates(timestamp);

 espec->SoundBufSize = SPU->EndFrame(espec->SoundBuf);

 CDC->ResetTS();
 TIMER_ResetTS();
 DMA_ResetTS();
 GPU->ResetTS();
 FIO->ResetTS();

 RebaseTS(timestamp);

 espec->MasterCycles = timestamp;

 if(psf_loader)
 {
  if(!espec->skip)
  {
   espec->LineWidths[0].w = ~0;
   Player_Draw(espec->surface, &espec->DisplayRect, 0, espec->SoundBuf, espec->SoundBufSize);
  }
 }

 // Save memcards if dirty.
 for(int i = 0; i < 8; i++)
 {
  uint64 new_dc = FIO->GetMemcardDirtyCount(i);

  if(new_dc > Memcard_PrevDC[i])
  {
   Memcard_PrevDC[i] = new_dc;
   Memcard_SaveDelay[i] = 0;
  }

  if(Memcard_SaveDelay[i] >= 0)
  {
   Memcard_SaveDelay[i] += timestamp;
   if(Memcard_SaveDelay[i] >= (33868800 * 2))	// Wait until about 2 seconds of no new writes.
   {
    fprintf(stderr, "Saving memcard %d...\n", i);
    try
    {
     char ext[64];
     trio_snprintf(ext, sizeof(ext), "%d.mcr", i);
     FIO->SaveMemcard(i, MDFN_MakeFName(MDFNMKF_SAV, 0, ext).c_str());
     Memcard_SaveDelay[i] = -1;
     Memcard_PrevDC[i] = 0;
    }
    catch(std::exception &e)
    {
     MDFN_PrintError("Memcard %d save error: %s", i, e.what());
     MDFN_DispMessage("Memcard %d save error: %s", i, e.what());
    }
    MDFN_DispMessage("Memcard %d saved.", i);
   }
  }
 }

 #if 0
 printf("read=%6d, write=%6d\n", ReadCounter, WriteCounter);
 ReadCounter = 0;
 WriteCounter = 0;
 printf("HW Port reads for this frame:\n");
 for(unsigned i = 0; i < 0x4000; i++)
 {
  if(PortReadCounter[i] > 100)
   printf("0x%08x: %d\n", 0x1f800000 + i, PortReadCounter[i]);
 }
 memset(PortReadCounter, 0, sizeof(PortReadCounter));
 printf("\n");
 #endif
}

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(PSFLoader::TestMagic(0x01, fp))
  return(true);

 if(fp->size < 0x800)
  return(false);

 if(memcmp(fp->data, "PS-X EXE", 8))
  return(false);

 return(true);
}

static bool TestMagicCD(std::vector<CDIF *> *CDInterfaces)
{
 uint8 buf[2048];

 if((*CDInterfaces)[0]->ReadSector(buf, 4, 1) != 0x2)
  return(false);

 if(strncmp((char *)buf + 10, "Licensed  by", strlen("Licensed  by")))
  return(false);

 //if(strncmp((char *)buf + 32, "Sony", 4))
 // return(false);

 //for(int i = 0; i < 2048; i++)
 // printf("%d, %02x %c\n", i, buf[i], buf[i]);
 //exit(1);

 return(true);
}

static unsigned CalcDiscSCEx(void)
{
 const char *prev_valid_id = NULL;
 unsigned ret_region = MDFN_GetSettingI("psx.region_default");

 cdifs_scex_ids.clear();

if(cdifs)
 for(unsigned i = 0; i < cdifs->size(); i++)
 {
  const char *id = NULL;
  uint8 buf[2048];
  uint8 fbuf[2048 + 1];
  unsigned ipos, opos;

  memset(fbuf, 0, sizeof(fbuf));

  if((*cdifs)[i]->ReadSector(buf, 4, 1) == 0x2)
  {
   for(ipos = 0, opos = 0; ipos < 0x48; ipos++)
   {
    if(buf[ipos] > 0x20 && buf[ipos] < 0x80)
    {
     fbuf[opos++] = tolower(buf[ipos]);
    }
   }

   fbuf[opos++] = 0;

   puts((char *)fbuf);

   if(strstr((char *)fbuf, "licensedby") != NULL)
   {
    if(strstr((char *)fbuf, "america") != NULL)
    {
     id = "SCEA";
     if(!i)
      ret_region = REGION_NA;
    }
    else if(strstr((char *)fbuf, "europe") != NULL)
    {
     id = "SCEE";
     if(!i)
      ret_region = REGION_EU;
    }
    else if(strstr((char *)fbuf, "japan") != NULL)
    {
     id = "SCEI";	// ?
     if(!i)
      ret_region = REGION_JP;
    }
    else if(strstr((char *)fbuf, "sonycomputerentertainmentinc.") != NULL)
    {
     id = "SCEI";
     if(!i)
      ret_region = REGION_JP;
    }
    else	// Failure case
    {
     if(prev_valid_id != NULL)
      id = prev_valid_id;
     else
     {
      switch(ret_region)	// Less than correct, but meh, what can we do.
      {
       case REGION_JP:
	id = "SCEI";
	break;

       case REGION_NA:
	id = "SCEA";
	break;

       case REGION_EU:
	id = "SCEE";
	break;
      }
     }
    }
   }
  }

  if(id != NULL)
   prev_valid_id = id;

  cdifs_scex_ids.push_back(id);
 }

 return ret_region;
}

static bool InitCommon(std::vector<CDIF *> *CDInterfaces, const bool EmulateMemcards = true)
{
 unsigned region;
 bool emulate_memcard[8];
 bool emulate_multitap[2];

 for(unsigned i = 0; i < 8; i++)
 {
  char buf[64];
  trio_snprintf(buf, sizeof(buf), "psx.input.port%d.memcard", i + 1);
  emulate_memcard[i] = EmulateMemcards && MDFN_GetSettingB(buf);
 }

 for(unsigned i = 0; i < 2; i++)
 {
  char buf[64];
  trio_snprintf(buf, sizeof(buf), "psx.input.port%d.multitap", i + 1);
  emulate_multitap[i] = MDFN_GetSettingB(buf);
 }


 cdifs = CDInterfaces;
 region = CalcDiscSCEx();

 if(!MDFN_GetSettingB("psx.region_autodetect"))
  region = MDFN_GetSettingI("psx.region_default");

 CPU = new PS_CPU();
 SPU = new PS_SPU();
 GPU = new PS_GPU(region == REGION_EU);
 CDC = new PS_CDC();
 FIO = new FrontIO(emulate_memcard, emulate_multitap);
 DMA_Init();

 if(region == REGION_EU)
 {
  EmulatedPSX.nominal_width = 367;	// Dunno. :(
  EmulatedPSX.nominal_height = 288;

  EmulatedPSX.fb_width = 1024;
  EmulatedPSX.fb_height = 576;
 }
 else
 {
  EmulatedPSX.lcm_width = 2720;
  EmulatedPSX.lcm_height = 480;

  EmulatedPSX.nominal_width = 306;
  EmulatedPSX.nominal_height = 240;

  EmulatedPSX.fb_width = 1024;
  EmulatedPSX.fb_height = 480;
 }

 if(cdifs)
 {
  CD_TrayOpen = false;
  CD_SelectedDisc = 0;
 }
 else
 {
  CD_TrayOpen = true;
  CD_SelectedDisc = -1;
 }

 CDC->SetDisc(true, NULL, NULL);
 CDC->SetDisc(CD_TrayOpen, (CD_SelectedDisc >= 0 && !CD_TrayOpen) ? (*cdifs)[CD_SelectedDisc] : NULL,
	(CD_SelectedDisc >= 0 && !CD_TrayOpen) ? cdifs_scex_ids[CD_SelectedDisc] : NULL);


 BIOSROM = new MultiAccessSizeMem<512 * 1024, uint32, false, 0, 0>();
 PIOMem = new MultiAccessSizeMem<65536, uint32, false, 0, 0>();

 for(uint32 ma = 0x00000000; ma < 0x00800000; ma += 2048 * 1024)
 {
  CPU->SetFastMap(MainRAM.data32, 0x00000000 + ma, 2048 * 1024);
  CPU->SetFastMap(MainRAM.data32, 0x80000000 + ma, 2048 * 1024);
  CPU->SetFastMap(MainRAM.data32, 0xA0000000 + ma, 2048 * 1024);
 }

 CPU->SetFastMap(BIOSROM->data32, 0x1FC00000, 512 * 1024);
 CPU->SetFastMap(BIOSROM->data32, 0x9FC00000, 512 * 1024);
 CPU->SetFastMap(BIOSROM->data32, 0xBFC00000, 512 * 1024);

 CPU->SetFastMap(PIOMem->data32, 0x1F000000, 65536);
 CPU->SetFastMap(PIOMem->data32, 0x9F000000, 65536);
 CPU->SetFastMap(PIOMem->data32, 0xBF000000, 65536);


 MDFNMP_Init(1024, ((uint64)1 << 29) / 1024);
 MDFNMP_AddRAM(2048 * 1024, 0x00000000, MainRAM.data8);
 MDFNMP_AddRAM(1024, 0x1F800000, ScratchRAM.data8);

 try
 {
  const char *biospath_sname;
  std::string biospath;

  if(region == REGION_JP)
   biospath_sname = "psx.bios_jp";
  else if(region == REGION_EU)
   biospath_sname = "psx.bios_eu";
  else if(region == REGION_NA)
   biospath_sname = "psx.bios_na";
  else
   abort();

  biospath = MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, MDFN_GetSettingS(biospath_sname).c_str());

  FileWrapper BIOSFile(biospath.c_str(), FileWrapper::MODE_READ);

  BIOSFile.read(BIOSROM->data8, 512 * 1024);

  for(int i = 0; i < 8; i++)
  {
   char ext[64];
   trio_snprintf(ext, sizeof(ext), "%d.mcr", i);
   FIO->LoadMemcard(i, MDFN_MakeFName(MDFNMKF_SAV, 0, ext).c_str());
  }
 }
 catch(std::exception &e)
 {
  MDFN_PrintError("%s", e.what());
  return(false);
 }

 for(int i = 0; i < 8; i++)
 {
  Memcard_PrevDC[i] = FIO->GetMemcardDirtyCount(i);
  Memcard_SaveDelay[i] = -1;
 }


 #ifdef WANT_DEBUGGER
 DBG_Init();
 #endif

 PSX_Power();

 return(true);
}

static void LoadEXE(const uint8 *data, const uint32 size, bool ignore_pcsp = false)
{
 uint32 PC;
 uint32 SP;
 uint32 TextStart;
 uint32 TextSize;

 if(size < 0x800)
  throw(MDFN_Error(0, "PS-EXE is too small."));

 PC = MDFN_de32lsb(&data[0x10]);
 SP = MDFN_de32lsb(&data[0x30]);
 TextStart = MDFN_de32lsb(&data[0x18]);
 TextSize = MDFN_de32lsb(&data[0x1C]);

 printf("PC=0x%08x\nTextStart=0x%08x\nTextSize=0x%08x\nSP=0x%08x\n", PC, TextStart, TextSize, SP);

 TextStart &= 0x1FFFFF;

 if(TextSize > 2048 * 1024)
 {
  throw(MDFN_Error(0, "Text section too large"));
 }

 if(TextSize > (size - 0x800))
  throw(MDFN_Error(0, "Text section recorded size is larger than data available in file.  Header=0x%08x, Available=0x%08x", TextSize, size - 0x800));

 if(TextSize < (size - 0x800))
  throw(MDFN_Error(0, "Text section recorded size is smaller than data available in file.  Header=0x%08x, Available=0x%08x", TextSize, size - 0x800));

 if(!TextMem.size())
 {
  TextMem_Start = TextStart;
  TextMem.resize(TextSize);
 }

 if(TextStart < TextMem_Start)
 {
  uint32 old_size = TextMem.size();

  printf("RESIZE: 0x%08x\n", TextMem_Start - TextStart);

  TextMem.resize(old_size + TextMem_Start - TextStart);
  memmove(&TextMem[TextMem_Start - TextStart], &TextMem[0], old_size);

  TextMem_Start = TextStart;
 }

 if(TextMem.size() < (TextStart - TextMem_Start + TextSize))
  TextMem.resize(TextStart - TextMem_Start + TextSize);

 memcpy(&TextMem[TextStart - TextMem_Start], data + 0x800, TextSize);


 //
 //
 //

 // BIOS patch
 BIOSROM->WriteU32(0x6990, (3 << 26) | ((0xBF001000 >> 2) & ((1 << 26) - 1)));
// BIOSROM->WriteU32(0x691C, (3 << 26) | ((0xBF001000 >> 2) & ((1 << 26) - 1)));

// printf("INSN: 0x%08x\n", BIOSROM->ReadU32(0x6990));
// exit(1);
 uint8 *po;

 po = &PIOMem->data8[0x0800];

 MDFN_en32lsb(po, (0x0 << 26) | (31 << 21) | (0x8 << 0));	// JR
 po += 4;
 MDFN_en32lsb(po, 0);	// NOP(kinda)
 po += 4;

 po = &PIOMem->data8[0x1000];
 // Load source address into r8
 uint32 sa = 0x9F000000 + 65536;
 MDFN_en32lsb(po, (0xF << 26) | (0 << 21) | (1 << 16) | (sa >> 16));	// LUI
 po += 4;
 MDFN_en32lsb(po, (0xD << 26) | (1 << 21) | (8 << 16) | (sa & 0xFFFF)); 	// ORI
 po += 4;

 // Load dest address into r9
 MDFN_en32lsb(po, (0xF << 26) | (0 << 21) | (1 << 16)  | (TextMem_Start >> 16));	// LUI
 po += 4;
 MDFN_en32lsb(po, (0xD << 26) | (1 << 21) | (9 << 16) | (TextMem_Start & 0xFFFF)); 	// ORI
 po += 4;

 // Load size into r10
 MDFN_en32lsb(po, (0xF << 26) | (0 << 21) | (1 << 16)  | (TextMem.size() >> 16));	// LUI
 po += 4;
 MDFN_en32lsb(po, (0xD << 26) | (1 << 21) | (10 << 16) | (TextMem.size() & 0xFFFF)); 	// ORI
 po += 4;

 //
 // Loop begin
 //
 
 MDFN_en32lsb(po, (0x24 << 26) | (8 << 21) | (1 << 16));	// LBU to r1
 po += 4;
 MDFN_en32lsb(po, 0); po += 4;			      	        // NOP

 MDFN_en32lsb(po, (0x28 << 26) | (9 << 21) | (1 << 16));	// SB from r1
 po += 4;
 MDFN_en32lsb(po, 0); po += 4;			      	        // NOP

 MDFN_en32lsb(po, (0x08 << 26) | (10 << 21) | (10 << 16) | 0xFFFF);	// Decrement size
 po += 4;

 MDFN_en32lsb(po, (0x08 << 26) | (8 << 21) | (8 << 16) | 0x0001);	// Increment source addr
 po += 4;

 MDFN_en32lsb(po, (0x08 << 26) | (9 << 21) | (9 << 16) | 0x0001);	// Increment dest addr
 po += 4;

 MDFN_en32lsb(po, (0x05 << 26) | (0 << 21) | (10 << 16) | (-8 & 0xFFFF));
 po += 4;
 MDFN_en32lsb(po, 0); po += 4;			      	        // NOP

 //
 // Loop end
 //

 // Load SP into r29
 if(ignore_pcsp)
 {
  po += 16;
 }
 else
 {
  printf("MEOWPC: %08x\n", PC);
  MDFN_en32lsb(po, (0xF << 26) | (0 << 21) | (1 << 16)  | (SP >> 16));	// LUI
  po += 4;
  MDFN_en32lsb(po, (0xD << 26) | (1 << 21) | (29 << 16) | (SP & 0xFFFF)); 	// ORI
  po += 4;

  // Load PC into r2
  MDFN_en32lsb(po, (0xF << 26) | (0 << 21) | (1 << 16)  | ((PC >> 16) | 0x8000));      // LUI
  po += 4;
  MDFN_en32lsb(po, (0xD << 26) | (1 << 21) | (2 << 16) | (PC & 0xFFFF));   // ORI
  po += 4;
 }

 // Jump to r2
 MDFN_en32lsb(po, (0x0 << 26) | (2 << 21) | (0x8 << 0));	// JR
 po += 4;
 MDFN_en32lsb(po, 0);	// NOP(kinda)
 po += 4;

}

PSF1Loader::PSF1Loader(MDFNFILE *fp)
{
 tags = Load(0x01, 2033664, fp);
}

PSF1Loader::~PSF1Loader()
{

}

void PSF1Loader::HandleEXE(const uint8 *data, uint32 size, bool ignore_pcsp)
{
 LoadEXE(data, size, ignore_pcsp);
}

static void Cleanup(void);
static int Load(const char *name, MDFNFILE *fp)
{
 const bool IsPSF = PSFLoader::TestMagic(0x01, fp);

 if(!TestMagic(name, fp))
  return(0);

// For testing.
#if 0
 static std::vector<CDIF *> CDInterfaces;

 CDInterfaces.push_back(new CDIF("/extra/games/PSX/Tony Hawk's Pro Skater 2 (USA)/Tony Hawk's Pro Skater 2 (USA).cue"));

 if(!InitCommon(&CDInterfaces, !IsPSF))
  return(0);
#else
 if(!InitCommon(NULL, !IsPSF))
  return(0);
#endif

 TextMem.resize(0);

 try
 {
  if(IsPSF)
  {
   psf_loader = new PSF1Loader(fp);

   std::vector<std::string> SongNames;

   SongNames.push_back(psf_loader->tags.GetTag("title"));

   Player_Init(1, psf_loader->tags.GetTag("game"), psf_loader->tags.GetTag("artist"), psf_loader->tags.GetTag("copyright"), SongNames);
  }
  else
   LoadEXE(fp->data, fp->size);
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
  Cleanup();
  return 0;
 }

 return(1);
}

static int LoadCD(std::vector<CDIF *> *CDInterfaces)
{
 int ret = InitCommon(CDInterfaces);

 // TODO: fastboot setting
 //if(MDFN_GetSettingB("psx.fastboot"))
 // BIOSROM->WriteU32(0x6990, 0);

 MDFNGameInfo->GameType = GMT_CDROM;

 return(ret);
}

static void Cleanup(void)
{
 TextMem.resize(0);

 if(psf_loader)
 {
  delete psf_loader;
  psf_loader = NULL;
 }

 if(CDC)
 {
  delete CDC;
  CDC = NULL;
 }

 if(SPU)
 {
  delete SPU;
  SPU = NULL;
 }

 if(GPU)
 {
  delete GPU;
  GPU = NULL;
 }

 if(CPU)
 {
  delete CPU;
  CPU = NULL;
 }

 if(FIO)
 {
  delete FIO;
  FIO = NULL;
 }

 DMA_Kill();

 if(BIOSROM)
 {
  delete BIOSROM;
  BIOSROM = NULL;
 }

 if(PIOMem)
 {
  delete PIOMem;
  PIOMem = NULL;
 }

 cdifs = NULL;
}

static void CloseGame(void)
{
 if(!psf_loader)
 {
  for(int i = 0; i < 8; i++)
  {
   // If there's an error saving one memcard, don't skip trying to save the other, since it might succeed and
   // we can reduce potential data loss!
   try
   {
    char ext[64];
    trio_snprintf(ext, sizeof(ext), "%d.mcr", i);

    FIO->SaveMemcard(i, MDFN_MakeFName(MDFNMKF_SAV, 0, ext).c_str());
   }
   catch(std::exception &e)
   {
    MDFN_PrintError("%s", e.what());
   }
  }
 }

 Cleanup();
}


static void SetInput(int port, const char *type, void *ptr)
{
 FIO->SetInput(port, type, ptr);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 return(0);

 SFORMAT StateRegs[] =
 {
  SFVAR(CD_TrayOpen),
  SFVAR(CD_SelectedDisc),
  SFARRAY(MainRAM.data8, 1024 * 2048),
  SFARRAY(ScratchRAM.data8, 1024),
  SFARRAY32(SysControl.Regs, 9),
  SFARRAY32(next_timestamps, sizeof(next_timestamps) / sizeof(next_timestamps[0])),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 // Call SetDisc() BEFORE we load CDC state, since SetDisc() has emulation side effects.  We might want to clean this up in the future.
 if(load)
 {
  CDC->SetDisc(CD_TrayOpen, (CD_SelectedDisc >= 0 && !CD_TrayOpen) ? (*cdifs)[CD_SelectedDisc] : NULL,
	(CD_SelectedDisc >= 0 && !CD_TrayOpen) ? cdifs_scex_ids[CD_SelectedDisc] : NULL);
 }

 // TODO: Remember to increment dirty count in memory card state loading routine.

 ret &= CPU->StateAction(sm, load, data_only);
 ret &= DMA_StateAction(sm, load, data_only);
 ret &= TIMER_StateAction(sm, load, data_only);
 ret &= CDC->StateAction(sm, load, data_only);
 ret &= MDEC_StateAction(sm, load, data_only);
 ret &= IRQ_StateAction(sm, load, data_only);

 if(load)
 {

 }

 return(ret);
}

static void CDInsertEject(void)
{
 CD_TrayOpen = !CD_TrayOpen;

 for(unsigned disc = 0; disc < cdifs->size(); disc++)
 {
  if(!(*cdifs)[disc]->Eject(CD_TrayOpen))
  {
   MDFN_DispMessage(_("Eject error."));
   CD_TrayOpen = !CD_TrayOpen;
  }
 }

 if(CD_TrayOpen)
  MDFN_DispMessage(_("Virtual CD Drive Tray Open"));
 else
  MDFN_DispMessage(_("Virtual CD Drive Tray Closed"));

 CDC->SetDisc(CD_TrayOpen, (CD_SelectedDisc >= 0 && !CD_TrayOpen) ? (*cdifs)[CD_SelectedDisc] : NULL,
	(CD_SelectedDisc >= 0 && !CD_TrayOpen) ? cdifs_scex_ids[CD_SelectedDisc] : NULL);
}

static void CDEject(void)
{
 if(!CD_TrayOpen)
  CDInsertEject();
}

static void CDSelect(void)
{
 if(cdifs && CD_TrayOpen)
 {
  CD_SelectedDisc = (CD_SelectedDisc + 1) % (cdifs->size() + 1);

  if((unsigned)CD_SelectedDisc == cdifs->size())
   CD_SelectedDisc = -1;

  if(CD_SelectedDisc == -1)
   MDFN_DispMessage(_("Disc absence selected."));
  else
   MDFN_DispMessage(_("Disc %d of %d selected."), CD_SelectedDisc + 1, (int)cdifs->size());
 }
}


static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFN_MSC_RESET: PSX_Power(); break;
  case MDFN_MSC_POWER: PSX_Power(); break;

  case MDFN_MSC_INSERT_DISK:
                CDInsertEject();
                break;

  case MDFN_MSC_SELECT_DISK:
                CDSelect();
                break;

  case MDFN_MSC_EJECT_DISK:
                CDEject();
                break;
 }
}


static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".psf", gettext_noop("PSF1 Rip") },
 { ".minipsf", gettext_noop("MiniPSF1 Rip") },
 { ".psx", gettext_noop("PS-X Executable") },
 { ".exe", gettext_noop("PS-X Executable") },
 { NULL, NULL }
};

static const MDFNSetting_EnumList Region_List[] =
{
 { "jp", REGION_JP, gettext_noop("Japan") },
 { "na", REGION_NA, gettext_noop("North America") },
 { "eu", REGION_EU, gettext_noop("Europe") },
 { NULL, 0 },
};

static MDFNSetting PSXSettings[] =
{
 { "psx.input.mouse_sensitivity", MDFNSF_NOFLAGS, gettext_noop("Emulated mouse sensitivity."), NULL, MDFNST_FLOAT, "1.00", NULL, NULL },

 { "psx.input.port1.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap on PSX port 1."), gettext_noop("Makes ports 1B-1D available."), MDFNST_BOOL, "0", NULL, NULL },
 { "psx.input.port2.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap on PSX port 2."), gettext_noop("Makes ports 2B-2D available."), MDFNST_BOOL, "0", NULL, NULL },

 { "psx.input.port1.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 1/1A."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port2.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 2/2A."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port3.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 1B."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port4.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 1C."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port5.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 1D."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port6.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 2B."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port7.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 2C."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
 { "psx.input.port8.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memcard on port 2D."), NULL, MDFNST_BOOL, "1", NULL, NULL, },


 //{ "psx.fastboot", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Skip BIOS intro sequence."), gettext_noop("MAY BREAK GAMES."), MDFNST_BOOL, "0" },
 { "psx.region_autodetect", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Attempt to auto-detect region of game."), NULL, MDFNST_BOOL, "1" },
 { "psx.region_default", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Default region to use."), gettext_noop("Used if region autodetection fails or is disabled."), MDFNST_ENUM, "jp", NULL, NULL, NULL, NULL, Region_List },

 { "psx.bios_jp", MDFNSF_EMU_STATE, gettext_noop("Path to the Japan SCPH-5500 ROM BIOS"), NULL, MDFNST_STRING, "scph5500.bin" },
 { "psx.bios_na", MDFNSF_EMU_STATE, gettext_noop("Path to the North America SCPH-5501 ROM BIOS"), gettext_noop("SHA1 0555c6fae8906f3f09baf5988f00e55f88e9f30b"), MDFNST_STRING, "scph5501.bin" },
 { "psx.bios_eu", MDFNSF_EMU_STATE, gettext_noop("Path to the Europe SCPH-5502 ROM BIOS"), NULL, MDFNST_STRING, "scph5502.bin" },

 { "psx.spu.resamp_quality", MDFNSF_NOFLAGS, gettext_noop("SPU output resampler quality."),
	gettext_noop("0 is lowest quality and CPU usage, 10 is highest quality and CPU usage.  The resampler that this setting refers to is used for converting from 44.1KHz to the sampling rate of the host audio device Mednafen is using.  Changing Mednafen's output rate, via the \"sound.rate\" setting, to \"44100\" will bypass the resampler, which will decrease CPU usage by Mednafen, and can increase or decrease audio quality, depending on various operating system and hardware factors."), MDFNST_UINT, "5", "0", "10" },
 { NULL },
};


MDFNGI EmulatedPSX =
{
 "psx",
 "Sony PlayStation",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &PSX_DBGInfo,
 #else
 NULL,
 #endif
 &FIO_InputInfo,
 Load,
 TestMagic,
 LoadCD,
 TestMagicCD,
 CloseGame,
 NULL,	//ToggleLayer,
 NULL,	//"Background Scroll\0Foreground Scroll\0Sprites\0",
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 PSXSettings,
 MDFN_MASTERCLOCK_FIXED(33868800),
 0,

 true, // Multires possible?

 //
 // Note: Following video settings will be overwritten during game load.
 //
 0,	// lcm_width
 0,	// lcm_height
 NULL,  // Dummy

 306,   // Nominal width
 240,   // Nominal height

 0,   // Framebuffer width
 0,   // Framebuffer height
 //
 //
 //

 2,     // Number of output sound channels

};
