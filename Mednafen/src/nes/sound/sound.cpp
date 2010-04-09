/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "../nes.h"
#include "../x6502.h"

#include "../sound.h"
#include "filter.h"

#define SQ_SHIFT        8
#define TRINPCM_SHIFT   0


static FESTAFILT *ff = NULL;
static int SoundPAL;

static uint32 wlookup1[32];
static uint32 wlookup2[203];

static double wlookup1_d[32], wlookup2_d[203];

static int16 WaveHi[40000] __attribute__ ((aligned (16)));
int16 WaveHiEx[40000] __attribute__ ((aligned (16)));
int16 WaveFinal[2048+512] __attribute__ ((aligned (16)));

std::vector<EXPSOUND> GameExpSound;

static uint8 TriCount;
static uint8 TriMode;

static int32 tristep;

static int32 wlcount[4];	/* Wave length counters.	*/

static uint8 IRQFrameMode;	/* $4017 / xx000000 */
static uint8 PSG[0x10];
static uint8 RawDALatch;	/* $4011 0xxxxxxx */

uint8 EnabledChannels;		/* Byte written to $4015 */

typedef struct {
	uint8 Speed;
	uint8 Mode;	/* Fixed volume(1), and loop(2) */
	uint8 DecCountTo1;
	uint8 decvolume;
	int reloaddec;
} ENVUNIT;

static ENVUNIT EnvUnits[3];

static const int RectDuties[4]={1,2,4,6};

static int32 RectDutyCount[2];
static uint8 SweepOn[2];
static int32 curfreq[2];
static uint8 SweepCount[2];
static uint8 SweepReload[2];
static uint32 SweepPeriod[2];
static unsigned int SweepShift[2];

static uint16 nreg;  

static uint8 fcnt;
static int32 fhcnt;
static int32 fhinc;

uint32 soundtsoffs;

static int32 lengthcount[4]; 
static const uint8 lengthtable[0x20]=
{
 0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xa0, 0x08, 0x3c, 0x0a, 0x0e, 0x0c, 0x1a, 0x0e, 0x0c, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xc0, 0x18, 0x48, 0x1a, 0x10, 0x1c, 0x20, 0x1E
};

static const uint32 NTSCNoiseFreqTable[0x10]=
{
 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

static const uint32 PALNoiseFreqTable[0x10] =
{
 4, 7, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708, 944, 1890, 3778
};

static const uint32 NTSCDMCTable[0x10]=
{
 428,380,340,320,286,254,226,214,
 190,160,142,128,106, 84 ,72,54
};

static const uint32 PALDMCTable[0x10]=
{
 398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118, 98, 78, 66, 50
};

// $4010        -        Frequency
// $4011        -        Actual data outputted
// $4012        -        Address register: $c000 + V*64
// $4013        -        Size register:  Size in bytes = (V+1)*64

static int32 DMCacc=1;
static int32 DMCPeriod;
static uint8 DMCBitCount=0;

static uint8 DMCAddressLatch,DMCSizeLatch; /* writes to 4012 and 4013 */
static uint8 DMCFormat;	/* Write to $4010 */

static uint32 DMCAddress=0;
static int32 DMCSize=0;
static uint8 DMCShift=0; 
static uint8 SIRQStat=0;

static char DMCHaveDMA=0;
static uint8 DMCDMABuf=0;  
static char DMCHaveSample=0;

static void Dummyfunc(void) 
{
}

static void (*DoNoise)(void)=Dummyfunc;
static void (*DoTriangle)(void)=Dummyfunc;
static void (*DoPCM)(void)=Dummyfunc;
static void (*DoSQ1)(void)=Dummyfunc;
static void (*DoSQ2)(void)=Dummyfunc;

static uint32 ChannelBC[5];

static void LoadDMCPeriod(uint8 V)
{
 if(SoundPAL)
  DMCPeriod=PALDMCTable[V];
 else
  DMCPeriod=NTSCDMCTable[V];
}

static void PrepDPCM()
{
 DMCAddress=0x4000+(DMCAddressLatch<<6);
 DMCSize=(DMCSizeLatch<<4)+1;
}

/* Instantaneous?  Maybe the new freq value is being calculated all of the time... */

static int CheckFreq(uint32 cf, uint8 sr)
{
 uint32 mod;
 if(!(sr&0x8))
 {
  mod=cf>>(sr&7);
  if((mod+cf)&0x800)
   return(0);
 }
 return(1);
}

static void SQReload(int x, uint8 V)
{
           if(EnabledChannels&(1<<x))
           {           
            if(x)
             DoSQ2();
            else
             DoSQ1();
            lengthcount[x]=lengthtable[(V>>3)&0x1f];
	   }

           //SweepOn[x]=(PSG[(x<<2)|1]&0x80) && SweepShift[x];
	   curfreq[x] = (curfreq[x] & 0xFF) | ((V&7)<<8);
           //curfreq[x]=PSG[(x<<2)|0x2]|((V&7)<<8);
           //SweepCount[x]=((PSG[(x<<2)|0x1]>>4)&7)+1;           

           RectDutyCount[x]=7;
	   EnvUnits[x].reloaddec=1;
	   //reloadfreq[x]=1;
}

static DECLFW(Write_PSG)
{
 A&=0x1F;
 switch(A)
 {
  case 0x0:DoSQ1();
	   EnvUnits[0].Mode=(V&0x30)>>4;
	   EnvUnits[0].Speed=(V&0xF);
           break;
  case 0x1:
	   SweepReload[0] = 1;
	   SweepPeriod[0] = ((V >> 4) & 0x7) + 1;
	   SweepShift[0] = (V & 0x7);
	   SweepOn[0]=(V&0x80) && SweepShift[0];
           break;
  case 0x2:
           DoSQ1();
           curfreq[0]&=0xFF00;
           curfreq[0]|=V;
           break;
  case 0x3:
           SQReload(0,V);
           break;
  case 0x4:           
	   DoSQ2();
           EnvUnits[1].Mode=(V&0x30)>>4;
           EnvUnits[1].Speed=(V&0xF);
	   break;
  case 0x5:        
	  SweepReload[1] = 1;
	  SweepPeriod[1] = ((V >> 4) & 0x7) + 1;
	  SweepShift[1] = (V & 0x7);
	  SweepOn[1] = (V&0x80) && SweepShift[1];
          break;
  case 0x6:DoSQ2();
          curfreq[1]&=0xFF00;
          curfreq[1]|=V;
          break;
  case 0x7:          
          SQReload(1,V);
          break;
  case 0xa:DoTriangle();
	   break;
  case 0xb:
          DoTriangle();
	  if(EnabledChannels&0x4)
           lengthcount[2]=lengthtable[(V>>3)&0x1f];
	  TriMode=1;	// Load mode
          break;
  case 0xC:DoNoise();
           EnvUnits[2].Mode=(V&0x30)>>4;
           EnvUnits[2].Speed=(V&0xF);
           break;
  case 0xE:DoNoise();
           break;
  case 0xF:
	   DoNoise();
           if(EnabledChannels&0x8)
	    lengthcount[3]=lengthtable[(V>>3)&0x1f];
	   EnvUnits[2].reloaddec=1;
           break;
 case 0x10:DoPCM();
	   LoadDMCPeriod(V&0xF);

	   if(SIRQStat&0x80)
	   {
	    if(!(V&0x80))
	    {
	     X6502_IRQEnd(MDFN_IQDPCM);
 	     SIRQStat&=~0x80;
	    }
            else X6502_IRQBegin(MDFN_IQDPCM);
	   }
	   break;
 }
 PSG[A]=V;
}

static DECLFW(Write_DMCRegs)
{
 A&=0xF;

 switch(A)
 {
  case 0x00:DoPCM();
            LoadDMCPeriod(V&0xF);

            if(SIRQStat&0x80)
            {
             if(!(V&0x80))
             { 
              X6502_IRQEnd(MDFN_IQDPCM);
              SIRQStat&=~0x80;
             }
             else X6502_IRQBegin(MDFN_IQDPCM);
            }
	    DMCFormat=V;
	    break;
  case 0x01:DoPCM();
	    RawDALatch=V&0x7F;
	    break;
  case 0x02:DMCAddressLatch=V;break;
  case 0x03:DMCSizeLatch=V;break;
 }


}

static DECLFW(StatusWrite)
{
	int x;

        DoSQ1();
        DoSQ2();
        DoTriangle();
        DoNoise();
        DoPCM();
        for(x=0;x<4;x++)
         if(!(V&(1<<x))) lengthcount[x]=0;   /* Force length counters to 0. */

        if(V&0x10)
        {
         if(!DMCSize)
          PrepDPCM();
        }
	else
	{
	 DMCSize=0;
	}
	SIRQStat&=~0x80;
        X6502_IRQEnd(MDFN_IQDPCM);
	EnabledChannels=V&0x1F;
}

static DECLFR(StatusRead)
{
   int x;
   uint8 ret;

   ret=SIRQStat;

   for(x=0;x<4;x++) ret|=lengthcount[x]?(1<<x):0;
   if(DMCSize) ret|=0x10;

   if(!fceuindbg)
   {
    SIRQStat&=~0x40;
    X6502_IRQEnd(MDFN_IQFCOUNT);
   }
   return ret;
}

static void FrameSoundStuff(int V)
{
 int P;

 DoSQ1();
 DoSQ2();
 DoNoise();
 DoTriangle();

 if(V&1) /* Envelope decay, linear counter, length counter, freq sweep */
 {
  if(!(PSG[8]&0x80))
   if(lengthcount[2]>0)
    lengthcount[2]--;

  if(!(PSG[0xC]&0x20))	/* Make sure loop flag is not set. */
   if(lengthcount[3]>0)
    lengthcount[3]--;

  for(P=0;P<2;P++)
  {
   if(!(PSG[P<<2]&0x20))	/* Make sure loop flag is not set. */
    if(lengthcount[P]>0)
     lengthcount[P]--;            

   /* Frequency Sweep Code Here */
   /* xxxx 0000 */
   /* xxxx = hz.  120/(x+1)*/
   if(SweepCount[P]>0) 
    SweepCount[P]--; 
   if(SweepCount[P]<=0)
   {
    SweepCount[P]=SweepPeriod[P];
    if(SweepOn[P] && curfreq[P] >= 8)
    {
     int offset = curfreq[P] >> SweepShift[P];

     if(PSG[(P<<2)+0x1]&0x8)
     {
      curfreq[P] -= offset + (P^1);
     }
     else if(curfreq[P] + offset < 0x800)
     {
      curfreq[P] += offset;
     }     
    }
   }
   if(SweepReload[P])
   {
    SweepCount[P] = SweepPeriod[P];
    SweepReload[P] = 0;
   }

  }
 }

 /* Now do envelope decay + linear counter. */

  if(TriMode) // In load mode?
   TriCount=PSG[0x8]&0x7F;
  else if(TriCount)
   TriCount--;

  if(!(PSG[0x8]&0x80))
   TriMode=0;

  for(P=0;P<3;P++)
  {
   if(EnvUnits[P].reloaddec)
   {
    EnvUnits[P].decvolume=0xF;
    EnvUnits[P].DecCountTo1=EnvUnits[P].Speed+1;
    EnvUnits[P].reloaddec=0;
    continue;
   }

   if(EnvUnits[P].DecCountTo1>0) EnvUnits[P].DecCountTo1--;
   if(EnvUnits[P].DecCountTo1==0)
   {
    EnvUnits[P].DecCountTo1=EnvUnits[P].Speed+1;
    if(EnvUnits[P].decvolume || (EnvUnits[P].Mode&0x2))
    {
     EnvUnits[P].decvolume--;
     EnvUnits[P].decvolume&=0xF;
    }
   }
  }
}

static void FrameSoundUpdate(void)
{
 // Linear counter:  Bit 0-6 of $4008
 // Length counter:  Bit 4-7 of $4003, $4007, $400b, $400f

 if(fcnt==3)
 {
	if(IRQFrameMode&0x2)
	 fhcnt+=fhinc;
 }
 fcnt=(fcnt+1) & 3;
 FrameSoundStuff(fcnt);
 if(!fcnt && !(IRQFrameMode&0x3))
 {
         SIRQStat|=0x40;
         X6502_IRQBegin(MDFN_IQFCOUNT);
 }
}

static DECLFW(Write_IRQFM)
{
 //printf("%02x\n",V);
 V=(V&0xC0)>>6;
 fcnt=0;
 if(V&0x2)
  FrameSoundUpdate();
 fhcnt=fhinc;
 if(V & 1)
 {
  X6502_IRQEnd(MDFN_IQFCOUNT);
  SIRQStat&=~0x40;
 }
 IRQFrameMode=V;
}


static ALWAYS_INLINE void tester(void)
{
 if(DMCBitCount==0)
 {
  if(!DMCHaveDMA)
   DMCHaveSample=0;
  else
  {
   DMCHaveSample=1;
   DMCShift=DMCDMABuf;
   DMCHaveDMA=0;
  }
 }    
}

static ALWAYS_INLINE void DMCDMA(void)
{
  if(DMCSize && !DMCHaveDMA)
  {
   X6502_DMR(0x8000+DMCAddress);
   X6502_DMR(0x8000+DMCAddress);
   X6502_DMR(0x8000+DMCAddress);
   DMCDMABuf=X6502_DMR(0x8000+DMCAddress);
   DMCHaveDMA=1;
   DMCAddress=(DMCAddress+1)&0x7fff;
   DMCSize--;
   if(!DMCSize)
   {
    if(DMCFormat&0x40)
     PrepDPCM();
    else
    {
     SIRQStat|=0x80;
     if(DMCFormat&0x80)
      X6502_IRQBegin(MDFN_IQDPCM);
    }
   }
 }
}

void MDFN_SoundCPUHook(int cycles)
{
 DMCDMA();
 DMCacc-=cycles;

 while(DMCacc<=0)
 {
  if(DMCHaveSample)
  {
   uint8 bah=RawDALatch;
   int t=((DMCShift&1)<<2)-2;

   /* Unbelievably ugly hack */ 
   if(FSettings.SndRate)
   {
    soundtsoffs+=DMCacc;
    DoPCM();
    soundtsoffs-=DMCacc;
   }
   RawDALatch+=t;
   if(RawDALatch&0x80)
    RawDALatch=bah;
  }

  DMCacc+=DMCPeriod;
  DMCBitCount=(DMCBitCount+1)&7;
  DMCShift>>=1;  
  tester();
 }

 // This needs to come after the DMC code because of the "soundtsoffs" +=/-= hack
 fhcnt-=cycles*48;
 if(fhcnt<=0)
 {
  FrameSoundUpdate();
  fhcnt+=fhinc;
 }
}

/* This has the correct phase.  Don't mess with it. */
static INLINE void RDoSQ(int x)
{
   int32 V;
   int32 amp;
   int32 rthresh;
   int16 *D;
   int32 currdc;
   int32 cf;
   int32 rc;

   if(curfreq[x]<8 || curfreq[x]>0x7ff)
    goto endit;
   if(!CheckFreq(curfreq[x],PSG[(x<<2)|0x1]))
    goto endit;
   if(!lengthcount[x])
    goto endit;

   if(EnvUnits[x].Mode&0x1)
    amp=EnvUnits[x].Speed;
   else
    amp=EnvUnits[x].decvolume;
//   printf("%d\n",amp);
   amp<<=SQ_SHIFT;

   rthresh=RectDuties[(PSG[(x<<2)]&0xC0)>>6];

   D=&WaveHi[ChannelBC[x]];
   V=SOUNDTS-ChannelBC[x];
   
   currdc=RectDutyCount[x];
   cf=(curfreq[x]+1)*2;
   rc=wlcount[x];

   while(V>0)
   {
    if(currdc<rthresh)
     *D+=amp;
    rc--;
    if(!rc)
    {
     rc=cf;
     currdc=(currdc+1)&7;
    }
    V--;
    D++;
   }   
  
   RectDutyCount[x]=currdc;
   wlcount[x]=rc;

   endit:
   ChannelBC[x]=SOUNDTS;
}

static void RDoSQ1(void)
{
 RDoSQ(0);
}

static void RDoSQ2(void)
{
 RDoSQ(1);
}

static void RDoTriangle(void)
{
 uint32 V;
 int32 tcout;

 tcout=(tristep&0xF);
 if(!(tristep&0x10)) tcout^=0xF;
 tcout=(tcout*3) << TRINPCM_SHIFT;

 if(!lengthcount[2] || !TriCount)
 {   				/* Counter is halted, but we still need to output. */
  int16 *start = &WaveHi[ChannelBC[2]];
  int32 count = SOUNDTS - ChannelBC[2];
  while(count--)
  {
   *start += tcout;
   start++;
  }
 }
 else
  for(V=ChannelBC[2];V<SOUNDTS;V++)
  {
    WaveHi[V]+=tcout;
    wlcount[2]--;
    if(!wlcount[2])
    {
     wlcount[2]=(PSG[0xa]|((PSG[0xb]&7)<<8))+1;
     tristep++;
     tcout=(tristep&0xF);
     if(!(tristep&0x10)) tcout^=0xF;
     tcout=(tcout*3) << TRINPCM_SHIFT;
    }
  }

 ChannelBC[2]=SOUNDTS;
}

static void RDoNoise(void)
{
 uint32 V;
 int32 outo;
 uint32 amptab[2];

 if(EnvUnits[2].Mode&0x1)
  amptab[0]=EnvUnits[2].Speed;
 else
  amptab[0]=EnvUnits[2].decvolume;

 amptab[0] <<= TRINPCM_SHIFT;
 amptab[1] = 0;

 amptab[0]<<=1;

 if(!lengthcount[3])
  amptab[0]=0;

 amptab[0] += RawDALatch << TRINPCM_SHIFT;
 amptab[1] += RawDALatch << TRINPCM_SHIFT;

 outo=amptab[nreg & 1];


 int32 wl;

 if(SoundPAL)
  wl = PALNoiseFreqTable[PSG[0xE]&0xF];
 else
  wl = NTSCNoiseFreqTable[PSG[0xE]&0xF];

 if(PSG[0xE]&0x80)        // "short" noise
  for(V=ChannelBC[3];V<SOUNDTS;V++)
  {
   WaveHi[V]+=outo;
   wlcount[3]--;
   if(!wlcount[3])
   {
    uint8 feedback;
    wlcount[3] = wl;
    feedback=((nreg>>8)&1)^((nreg>>14)&1);
    nreg=(nreg<<1) | feedback;
    outo=amptab[nreg & 1];
   }
  }
 else
  for(V=ChannelBC[3];V<SOUNDTS;V++)
  {
   WaveHi[V]+=outo;
   wlcount[3]--;
   if(!wlcount[3])
   {
    uint8 feedback;
    wlcount[3] = wl;
    feedback=((nreg>>13)&1)^((nreg>>14)&1);
    nreg=(nreg<<1) | feedback;
    outo=amptab[nreg & 1];
   }
  }
 ChannelBC[3]=SOUNDTS;
}

static void SetNESSoundMap(void)
{ 
  SetWriteHandler(0x4000,0x400F,Write_PSG);
  SetWriteHandler(0x4010,0x4013,Write_DMCRegs);
  SetWriteHandler(0x4017,0x4017,Write_IRQFM);

  SetWriteHandler(0x4015,0x4015,StatusWrite);
  SetReadHandler(0x4015,0x4015,StatusRead);
}

static double capacimoo = 0;
static int32 inbuf=0;

int FlushEmulateSound(int reverse)
{
  int x;
  int32 end,left;

  if(!timestamp) return(0);

  bool HasHiFill = 0;

  for(std::vector<EXPSOUND>::iterator ep = GameExpSound.begin(); ep != GameExpSound.end(); ep++)
   if(ep->HiFill)
   {
    HasHiFill = 1;
    break;
   }

  if(!FSettings.SndRate)
  {
   left=0;
   end=0;
   goto nosoundo;
  }

  DoSQ1();
  DoSQ2();
  DoTriangle();
  DoNoise();
  DoPCM();
  
  {
   int16 *tmpo=&WaveHi[soundtsoffs];
   int16 *intmpo = &WaveHi[soundtsoffs];

   if(HasHiFill)
   {
    int16 *intmpoex = &WaveHiEx[soundtsoffs];
    const double mult = (double)94/1789772.72727272;

    for(std::vector<EXPSOUND>::iterator ep = GameExpSound.begin(); ep != GameExpSound.end(); ep++)
     if(ep->HiFill)
      ep->HiFill();

    for(x=timestamp;x;x--)
    {
	uint32 b=*intmpo;
	double sample;
        double delta;

	sample = wlookup2_d[(b>>TRINPCM_SHIFT) & 255]+wlookup1_d[b>>SQ_SHIFT];
	delta = (sample - capacimoo);

	*tmpo= (int16)(delta - *intmpoex);       
					// Invert expansion sound channels relative to main sound.
                                       // I think the VRC6 is like this, but the MMC5 is not(exception handled in mmc5.c)
                                        // Namco 106 *appears* to be like the MMC5.
                                        // Unsure about other chips.
	capacimoo += delta * mult;

	tmpo++;
	intmpo++;
	intmpoex++;
    }
   }
   else
   {
    for(x=timestamp;x;x--)
    {
     uint32 b=*intmpo;
     *tmpo = wlookup2[(b>>TRINPCM_SHIFT)&255]+wlookup1[b>>SQ_SHIFT] - 32768;
     tmpo++;
     intmpo++;
    }
   }
  }

  if(reverse)
  {
    int16 *neo1,*neo2;

    neo1 = &WaveHi[soundtsoffs];
    neo2 = &neo1[timestamp - 1];

    while(neo1 < neo2)
    {
     uint16 cha = *neo1;
     *neo1 = *neo2;
     *neo2 = cha;
     neo1++;
     neo2--;
    }
  }

  end=FESTAFILT_Do(ff, (int16*)WaveHi,WaveFinal,sizeof(WaveFinal) / sizeof(WaveFinal[0])/*WaveFinalLen*/, SOUNDTS, &left, HasHiFill ? 1 : 0);

  memmove(WaveHi,(int16 *)WaveHi+SOUNDTS-left,left*sizeof(uint16));
  memset((int16 *)WaveHi+left,0,sizeof(WaveHi)-left*sizeof(uint16));

  if(HasHiFill)
   memset((int16 *)WaveHiEx+left,0,sizeof(WaveHiEx)-left*sizeof(uint16));

  for(std::vector<EXPSOUND>::iterator ep = GameExpSound.begin(); ep != GameExpSound.end(); ep++)
   if(ep->HiSync)
    ep->HiSync(left);

  for(x=0;x<5;x++)
   ChannelBC[x]=left;

  nosoundo:

  soundtsoffs=left;
  inbuf=end;

  return(end);
}

/* FIXME:  Find out what sound registers get reset on reset.  I know $4001/$4005 don't,
due to that whole MegaMan 2 Game Genie thing.
*/

static double soundmultiplier = 1;

/* Called when a game is being closed. */
void MDFNSND_Close(void)
{
 if(ff)
 {
  FESTAFILT_Kill(ff);
  ff = NULL;
 }
}

void MDFNSND_Reset(void)
{
        int x;

	IRQFrameMode=0x0;
        fhcnt=fhinc;
        fcnt=0;

        nreg=1;
        for(x=0;x<2;x++)
	{
         wlcount[x]=2048;
	 SweepOn[x]=0;
	 curfreq[x]=0;
	}
        wlcount[2]=1;	//2048;
        wlcount[3]=2048;
	DMCHaveDMA=DMCHaveSample=0;
	SIRQStat=0x00;

	RawDALatch=0x00;
	TriCount=0;
	TriMode=0;
	tristep=0;
	EnabledChannels=0;
	for(x=0;x<4;x++)
	 lengthcount[x]=0;

	DMCAddressLatch=0;
	DMCSizeLatch=0;
	DMCFormat=0;
	DMCAddress=0;
	DMCSize=0;
	DMCShift=0;
}

void MDFNSND_Power(void)
{
        int x;
 
        SetNESSoundMap();
        memset(PSG,0x00,sizeof(PSG));
	MDFNSND_Reset();

        memset(WaveHi,0,sizeof(WaveHi));
	memset(WaveHiEx, 0, sizeof(WaveHiEx));
	memset(WaveFinal, 0, sizeof(WaveFinal));
	memset(&EnvUnits,0,sizeof(EnvUnits));

        for(x=0;x<5;x++)
         ChannelBC[x]=0;
        soundtsoffs=0;
        LoadDMCPeriod(DMCFormat&0xF);
}

void MDFNNES_Sound(int Rate)
{
 int x;

 capacimoo = 0;
 memset(ChannelBC,0,sizeof(ChannelBC));
 memset(WaveHi,0,sizeof(WaveHi));
 memset(WaveHiEx, 0, sizeof(WaveHiEx));
 memset(WaveFinal, 0, sizeof(WaveFinal));
 soundtsoffs = 0;
 LoadDMCPeriod(DMCFormat&0xF);

 /* Kill the filter if it was initialized. */
 if(ff)
 {
  FESTAFILT_Kill(ff);
  ff = NULL;
 }

 if(Rate)
 {
  DoNoise = RDoNoise;
  DoTriangle = RDoTriangle;
  DoPCM = RDoNoise;
  //DoPCM = RDoPCM;
  DoSQ1 = RDoSQ1;
  DoSQ2 = RDoSQ2;

  wlookup1[0]=0;
  for(x=1;x<32;x++)
  {
   double val = ( (double)16*16*16*4*95.52 / ( (double)8128 / (double)x+100) );

   wlookup1[x] = (uint32)val;
   wlookup1_d[x] = val;
  }
  wlookup2[0]=0;
  for(x=1;x<203;x++)
  {
   double val = ((double)16*16*16*4*163.67/((double)24329/(double)x+100));
   wlookup2[x] = (uint32)val;
   wlookup2_d[x] = val;
  }
  ff = FESTAFILT_Init(FSettings.SndRate, SoundPAL, soundmultiplier, FSettings.SoundVolume);
  ff->SoundVolume=FSettings.SoundVolume;
 }
 else
 {
  DoNoise=DoTriangle=DoPCM=DoSQ1=DoSQ2=Dummyfunc;
 }

 if(MDFNGameInfo)
 {
  for(int x = 0; x < GameExpSound.size(); x++)
   if(GameExpSound[x].RChange)
    GameExpSound[x].RChange(&GameExpSound[x]);
 }
}

void MDFNNES_SetSoundMultiplier(double multiplier)
{
 soundmultiplier = multiplier;

 if(ff)
  FESTAFILT_SetMultiplier(ff, multiplier);
}

void MDFNNES_SetSoundVolume(uint32 volume)
{
 if(ff)
  FESTAFILT_SetVolume(ff, volume);
}

/* Called when a game has been loaded. */
int MDFNSND_Init(int PAL)
{
 SoundPAL = PAL?1:0;

 ff = FESTAFILT_Init(FSettings.SndRate, SoundPAL, soundmultiplier, FSettings.SoundVolume);

 fhinc=SoundPAL?16626:14915;   // *2 CPU clock rate
 fhinc*=24;
 return(1);
}

int MDFNSND_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT MDFNSND_STATEINFO[]=
 {
  { &fhcnt, 4|MDFNSTATE_RLSB,"FHCN"},
  { &fcnt, 1, "FCNT"},  
  { PSG, 0x10, "PSG"},
  { &EnabledChannels, 1, "ENCH"},
  { &IRQFrameMode, 1, "IQFM"},
  { &nreg, 2|MDFNSTATE_RLSB, "NREG"},
  { &TriMode, 1, "TRIM"},
  { &TriCount, 1, "TRIC"},

  { &EnvUnits[0].Speed, 1, "E0SP"},
  { &EnvUnits[1].Speed, 1, "E1SP"},
  { &EnvUnits[2].Speed, 1, "E2SP"},

  { &EnvUnits[0].Mode, 1, "E0MO"},
  { &EnvUnits[1].Mode, 1, "E1MO"},
  { &EnvUnits[2].Mode, 1, "E2MO"},

  { &EnvUnits[0].DecCountTo1, 1, "E0D1"},
  { &EnvUnits[1].DecCountTo1, 1, "E1D1"},
  { &EnvUnits[2].DecCountTo1, 1, "E2D1"},

  { &EnvUnits[0].decvolume, 1, "E0DV"},
  { &EnvUnits[1].decvolume, 1, "E1DV"},
  { &EnvUnits[2].decvolume, 1, "E2DV"},

  { &lengthcount[0], 4|MDFNSTATE_RLSB, "LEN0"},
  { &lengthcount[1], 4|MDFNSTATE_RLSB, "LEN1"},
  { &lengthcount[2], 4|MDFNSTATE_RLSB, "LEN2"},
  { &lengthcount[3], 4|MDFNSTATE_RLSB, "LEN3"},
  { SweepOn, 2, "SWEE"},

  { &RectDutyCount[0], 4|MDFNSTATE_RLSB,"RDC0"},
  { &RectDutyCount[1], 4|MDFNSTATE_RLSB,"RDC1"},

  { &tristep, 4|MDFNSTATE_RLSB, "TRST"},
 
  { &wlcount[0], 4 | MDFNSTATE_RLSB, "WLC0"},
  { &wlcount[1], 4 | MDFNSTATE_RLSB, "WLC1"},
  { &wlcount[2], 4 | MDFNSTATE_RLSB, "WLC2"},
  { &wlcount[3], 4 | MDFNSTATE_RLSB, "WLC3"},


  { &curfreq[0], 4|MDFNSTATE_RLSB,"CRF1"},
  { &curfreq[1], 4|MDFNSTATE_RLSB,"CRF2"},
  { SweepCount, 2,"SWCT"},

  { &SIRQStat, 1, "SIRQ"},

  { &DMCacc, 4|MDFNSTATE_RLSB, "5ACC"},
  { &DMCBitCount, 1, "5BIT"},
  { &DMCAddress, 4|MDFNSTATE_RLSB, "5ADD"},
  { &DMCSize, 4|MDFNSTATE_RLSB, "5SIZ"},
  { &DMCShift, 1, "5SHF"},

  { &DMCHaveDMA, 1, "5HVDM"},
  { &DMCHaveSample, 1, "5HVSP"},

  { &DMCSizeLatch, 1, "5SZL"},
  { &DMCAddressLatch, 1, "5ADL"},
  { &DMCFormat, 1, "5FMT"},
  { &RawDALatch, 1, "RWDA"},
  { &capacimoo, sizeof(capacimoo) | MDFNSTATE_RLSB, "CMOO"},
  SFEND
 };

 std::vector <SSDescriptor> love;
 love.push_back(SSDescriptor(MDFNSND_STATEINFO, "SND"));

 int ret = MDFNSS_StateAction(sm, load, data_only, love);

 if(load)
 {
  LoadDMCPeriod(DMCFormat&0xF);
  RawDALatch&=0x7F;
  DMCAddress&=0x7FFF;
 }
 return(ret);
}

