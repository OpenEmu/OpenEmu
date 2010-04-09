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

#include <string.h>

#include "nes.h"
#include "x6502.h"
#include "fds.h"
#include "sound.h"
#include "memory.h"
#include "cart.h"
#include "nsf.h"

/*	TODO:  Add code to put a delay in between the time a disk is inserted
	and the when it can be successfully read/written to.  This should
	prevent writes to wrong places OR add code to prevent disk ejects
	when the virtual motor is on(mmm...virtual motor).
*/

static DECLFR(FDSRead4030);
static DECLFR(FDSRead4031);
static DECLFR(FDSRead4032);
static DECLFR(FDSRead4033);

static DECLFW(FDSWrite);

static DECLFW(FDSWaveWrite);
static DECLFR(FDSWaveRead);

static DECLFR(FDSSRead);
static DECLFW(FDSSWrite);
static DECLFR(FDSBIOSRead);
static DECLFR(FDSRAMRead);
static DECLFW(FDSRAMWrite);
static void FDSInit(void);
static void FDSFix(int a);


static uint8 *FDSRAM = NULL;
static uint8 *CHRRAM = NULL;

static uint8 FDSRegs[6];
static int32 IRQLatch,IRQCount;
static uint8 IRQa;
static void FDSClose(void);

static uint8 FDSBIOS[8192];

/* Original disk data backup, to help in creating save states. */
static uint8 *diskdatao[8]={0,0,0,0,0,0,0,0};

static uint8 *diskdata[8]={0,0,0,0,0,0,0,0};

static unsigned int TotalSides;
static uint8 DiskWritten=0;		/* Set to 1 if disk was written to. */
static uint8 writeskip;
static uint32 DiskPtr;
static int32 DiskSeekIRQ;
static uint8 SelectDisk,InDisk;

static int FDS_StateAction(StateMem *sm, int load, int data_only);

#define DC_INC		1

static void RenderSoundHQ(void);

int FDS_DiskInsert(int oride)
{
	if(InDisk==255)
        {
         MDFN_DispMessage(_("Disk %1$d Side %2$s Inserted"),SelectDisk>>1,(SelectDisk&1)?"B":"A");  
         InDisk=SelectDisk;
        }
        else   
        {
         MDFN_DispMessage(_("Disk %1$d Side %2$s Ejected"),SelectDisk>>1,(SelectDisk&1)?"B":"A");
         InDisk=255;
        }
	return(TRUE);
}

int FDS_DiskEject(void)
{
	InDisk=255;
	return(TRUE);
}

int FDS_DiskSelect(void)
{
	if(InDisk!=255)
        {
         MDFN_DispMessage(_("Eject disk before selecting."));
	 return(FALSE);
        }
        SelectDisk=((SelectDisk+1)%TotalSides)&3;
        MDFN_DispMessage(_("Disk %1$d Side %2$s Selected"),SelectDisk>>1,(SelectDisk&1)?"B":"A");
	return(TRUE);
}

static void FDSFix(int a)
{
 if((IRQa&2) && IRQCount)
 {
  IRQCount-=a;
  if(IRQCount<=0)
  {
   if(!(IRQa&1))
   {
    IRQa&=~2;
    IRQCount=IRQLatch=0;
   }
   else
    IRQCount=IRQLatch; 
   //IRQCount=IRQLatch; //0xFFFF;
   X6502_IRQBegin(MDFN_IQEXT);
   //printf("IRQ: %d\n",timestamp);
//   printf("IRQ: %d\n",scanline);
  }
 }
 if(DiskSeekIRQ>0) 
 {
  DiskSeekIRQ-=a;
  if(DiskSeekIRQ<=0)
  {
   if(FDSRegs[5]&0x80)
   {
    X6502_IRQBegin(MDFN_IQEXT2);
   }
  }
 }
}

static DECLFR(FDSRead4030)
{
	uint8 ret=0;

	/* Cheap hack. */
	if(X.IRQlow&MDFN_IQEXT) ret|=1;
	if(X.IRQlow&MDFN_IQEXT2) ret|=2;

	if(!fceuindbg)
	{
	 X6502_IRQEnd(MDFN_IQEXT);
	 X6502_IRQEnd(MDFN_IQEXT2);
	}
	return ret;
}

static DECLFR(FDSRead4031)
{
	static uint8 z=0;
	if(InDisk!=255)
	{
         z=diskdata[InDisk][DiskPtr];
	 if(!fceuindbg)
	 {
          if(DiskPtr<64999) DiskPtr++;
          DiskSeekIRQ=150;
          X6502_IRQEnd(MDFN_IQEXT2);
	 }
	}
        return z;
}
static DECLFR(FDSRead4032)
{       
        uint8 ret;

        ret=X.DB&~7;
        if(InDisk==255)
         ret|=5;

        if(InDisk==255 || !(FDSRegs[5]&1) || (FDSRegs[5]&2))        
         ret|=2;
        return ret;
}

static DECLFR(FDSRead4033)
{
	return 0x80; // battery
}

static DECLFW(FDSRAMWrite)
{
 (FDSRAM-0x6000)[A]=V;
}

static DECLFR(FDSBIOSRead)
{
 return (FDSBIOS-0xE000)[A];
}

static DECLFR(FDSRAMRead)
{
 return (FDSRAM-0x6000)[A];
}

/* Begin FDS sound */

#define FDSClock (1789772.7272727272727272/2)

typedef struct {
        int64 cycles;           // Cycles per PCM sample
        int64 count;		// Cycle counter
	int64 envcount;		// Envelope cycle counter
	uint32 b19shiftreg60;
	uint32 b24adder66;
	uint32 b24latch68;
	uint32 b17latch76;
	uint8 amplitude[2];	// Current amplitudes.
	uint8 speedo[2];
	uint8 mwcount;
	uint8 mwstart;
        uint8 mwave[0x20];      // Modulation waveform
        uint8 cwave[0x40];      // Game-defined waveform(carrier)
        uint8 SPSG[0xB];
} FDSSOUND;

static FDSSOUND fdso;

#define	SPSG	fdso.SPSG
#define b19shiftreg60	fdso.b19shiftreg60
#define b24adder66	fdso.b24adder66
#define b24latch68	fdso.b24latch68
#define b17latch76	fdso.b17latch76
#define amplitude	fdso.amplitude
#define speedo		fdso.speedo

static int sweep_bias;

static DECLFR(FDSSRead)
{
 switch(A&0xF)
 {
  case 0x0:return(amplitude[0]|(X.DB&0xC0));
  case 0x2:return(amplitude[1]|(X.DB&0xC0));
 }
 return(X.DB);
}


static DECLFW(FDSSWrite)
{
 if(FSettings.SndRate)
 {
  RenderSoundHQ();
 }
 A-=0x4080;
 switch(A)
 {
  case 0x0: 
  case 0x4: if(V&0x80)
	     amplitude[(A&0xF)>>2]=V&0x3F; //)>0x20?0x20:(V&0x3F);
	    break;
  case 0x5://printf("$%04x:$%02x\n",A,V);
		b17latch76 = 0;
		break;
  case 0x7:	sweep_bias = (V & 0x3F) - (V & 0x40);
		b17latch76 = 0;
		SPSG[0x5]=0;//printf("$%04x:$%02x\n",A,V);
		break;
  case 0x8:
	   b17latch76=0;
	   fdso.mwave[SPSG[0x5]&0x1F]=V&0x7;
           SPSG[0x5]=(SPSG[0x5]+1)&0x1F;
	   break;
 }
 //if(A>=0x7 && A!=0x8 && A<=0xF)
 //if(A==0xA || A==0x9) 
 //printf("$%04x:$%02x\n",A,V);
 SPSG[A]=V;
}

// $4080 - Fundamental wave amplitude data register 92
// $4082 - Fundamental wave frequency data register 58
// $4083 - Same as $4082($4083 is the upper 4 bits).

// $4084 - Modulation amplitude data register 78
// $4086 - Modulation frequency data register 72
// $4087 - Same as $4086($4087 is the upper 4 bits)


static void DoEnv()
{
 int x;

 for(x=0;x<2;x++)
  if(!(SPSG[x<<2]&0x80) && !(SPSG[0x3]&0x40))
  {
   static int counto[2]={0,0};

   if(counto[x]<=0)
   {
    if(!(SPSG[x<<2]&0x80))
    {
     if(SPSG[x<<2]&0x40)
     {
      if(amplitude[x]<0x3F)
       amplitude[x]++;
     }
     else
     {
      if(amplitude[x]>0)
       amplitude[x]--;
     }
    }
    counto[x]=(SPSG[x<<2]&0x3F);
   }
   else
    counto[x]--;
  }
}

static DECLFR(FDSWaveRead)
{
 return(fdso.cwave[A&0x3f]|(X.DB&0xC0));
}

static DECLFW(FDSWaveWrite)
{
 //printf("$%04x:$%02x, %d\n",A,V,SPSG[0x9]&0x80);
 if(SPSG[0x9]&0x80)
  fdso.cwave[A&0x3f]=V&0x3F;
}

static int32 cheep;
static ALWAYS_INLINE void ClockRise(void)
{
 b17latch76=(SPSG[0x6]|((SPSG[0x07]&0xF)<<8))+b17latch76;

 if(!(SPSG[0x7]&0x80)) 
 {
   static const int bias_tab[8] = { 0, 1, 2, 4, 0, 128 - 4, 128 - 2 , 128 - 1};
   int mw = fdso.mwave[((b17latch76>>16)&0x1F)]&7;
   int amp = amplitude[1];
   if(amp > 0x20) amp = 0x20;

   cheep = (bias_tab[mw] * amp);
   cheep = (cheep & 0x3F) - (cheep & 0x40);
   //if((bias_tab[mw] * amp) & 0x40) cheep = 0 - cheep;
  }

 if(SPSG[0x7] & 0x80)
  b24latch68 += ((SPSG[0x2]|((SPSG[0x3]&0xF)<<8)) << 11);
 else
 {
  int32 freq = ((SPSG[0x2]|((SPSG[0x3]&0xF)<<8)));
  int32 mod;

  mod = freq * cheep / 64;
  freq += mod;
  if(freq < 0) freq = 0;
  b24latch68 += freq << 11;
 }
}

static INLINE int32 FDSDoSound(void)
{
 fdso.count+=fdso.cycles;
 if(fdso.count>=((int64)1<<40))
 {
  dogk:
  fdso.count-=(int64)1<<40;
  ClockRise();
  fdso.envcount--;
  if(fdso.envcount<=0)
  {
   fdso.envcount+=SPSG[0xA]*3;
   DoEnv(); 
  }
 }
 if(fdso.count>=32768) goto dogk;

 // Might need to emulate applying the amplitude to the waveform a bit better...
 {
  int k=amplitude[0];
  if(k>0x20) k=0x20;
  return (fdso.cwave[b24latch68>>26]*k)*4/((SPSG[0x9]&0x3)+2);
 }
}

static int32 FBC=0;

static void RenderSoundHQ(void)
{
 uint32 x;
  
 if(!(SPSG[0x9]&0x80))
  for(x=FBC;x<SOUNDTS;x++)
  {
   uint32 t=FDSDoSound();
   WaveHiEx[x] += t; //(t<<2)-(t<<1);
  }
 FBC=SOUNDTS;
}

static void HQSync(int32 ts)
{
 FBC=ts;
}

static void FDS_ESI(EXPSOUND *ep)
{
 if(FSettings.SndRate)
  fdso.cycles=(int64)1<<39;
}

int NSFFDS_Init(EXPSOUND *ep, bool MultiChip)
{
 memset(&fdso,0,sizeof(fdso));

 FDS_ESI(ep);

 ep->HiSync=HQSync;
 ep->HiFill=RenderSoundHQ;
 ep->RChange=FDS_ESI;

 SetReadHandler(0x4040,0x407f,FDSWaveRead);
 NSFECSetWriteHandler(0x4040,0x407f,FDSWaveWrite);
 NSFECSetWriteHandler(0x4080,0x408A,FDSSWrite);
 SetReadHandler(0x4090,0x4092,FDSSRead);

 return(1);
}


static DECLFW(FDSWrite)
{
 //extern int scanline;
 //MDFN_printf("$%04x:$%02x, %d\n",A,V,scanline);
 switch(A)
 {
  case 0x4020:
	X6502_IRQEnd(MDFN_IQEXT);
	IRQLatch&=0xFF00;
	IRQLatch|=V;
//	printf("$%04x:$%02x\n",A,V);
        break;
  case 0x4021:
        X6502_IRQEnd(MDFN_IQEXT);
	IRQLatch&=0xFF;
	IRQLatch|=V<<8;
//	printf("$%04x:$%02x\n",A,V);
        break;
  case 0x4022:
	X6502_IRQEnd(MDFN_IQEXT);
	IRQCount=IRQLatch;
	IRQa=V&3;
//	printf("$%04x:$%02x\n",A,V);
        break;
  case 0x4023:break;
  case 0x4024:
        if(InDisk!=255 && !(FDSRegs[5]&0x4) && (FDSRegs[3]&0x1))
        {
         if(DiskPtr>=0 && DiskPtr<65500)
         {
          if(writeskip) writeskip--;
          else if(DiskPtr>=2)
          {
	   DiskWritten=1;
           diskdata[InDisk][DiskPtr-2]=V;
          }
         }
        }
        break;
  case 0x4025:
	X6502_IRQEnd(MDFN_IQEXT2);
	if(InDisk!=255)
	{
         if(!(V&0x40))
         {
          if(FDSRegs[5]&0x40 && !(V&0x10))
          {
           DiskSeekIRQ=200;
           DiskPtr-=2;
          }
          if(DiskPtr<0) DiskPtr=0;
         }
         if(!(V&0x4)) writeskip=2;
         if(V&2) {DiskPtr=0;DiskSeekIRQ=200;}
         if(V&0x40) DiskSeekIRQ=200;
	}
        setmirror(((V>>3)&1)^1);
        break;
 }
 FDSRegs[A&7]=V;
}

static void FreeFDSMemory(void)
{
 unsigned int x;

 for(x=0;x<TotalSides;x++)
  if(diskdata[x])
  {
   free(diskdata[x]);
   diskdata[x]=0;
  }
}

static int SubLoad(MDFNFILE *fp)
{
 md5_context md5;
 uint8 header[16];
 unsigned int x;

 MDFN_fread(header,16,1,fp);
 
 if(memcmp(header,"FDS\x1a",4))
 {
  if(!(memcmp(header+1,"*NINTENDO-HVC*",14)))
  {
   long t;
   t=MDFN_fgetsize(fp);
   if(t<65500)
    t=65500;
   TotalSides=t/65500;
   MDFN_fseek(fp,0,SEEK_SET);
  }
  else
   return(FALSE);
 } 
 else
  TotalSides=header[4];

 md5.starts();

 if(TotalSides>8) TotalSides=8;
 if(TotalSides<1) TotalSides=1;

 for(x=0;x<TotalSides;x++)
 {
  diskdata[x]=(uint8 *)MDFN_malloc(65500, _("FDS Disk Data"));
  if(!diskdata[x])
  {
   unsigned int zol;
   for(zol=0;zol<x;zol++)
    free(diskdata[zol]);
   return 0;
  }
  MDFN_fread(diskdata[x],1,65500,fp);
  md5.update(diskdata[x],65500);
 }
 md5.finish(MDFNGameInfo->MD5);
 return(1);
}

static int FDS_StateAction(StateMem *sm, int load, int data_only)
{
 unsigned int x;
 SFORMAT StateRegs[] =
 {
  SFARRAY(diskdata[0], 65500),
  SFARRAY(diskdata[1], 65500),
  SFARRAY(diskdata[2], 65500),
  SFARRAY(diskdata[3], 65500),
  SFARRAY(diskdata[4], 65500),
  SFARRAY(diskdata[5], 65500),
  SFARRAY(diskdata[6], 65500),
  SFARRAY(diskdata[7], 65500),
  SFARRAY(FDSRAM, 32768),
  {FDSRegs,sizeof(FDSRegs),"FREG"},
  SFARRAY(CHRRAM, 8192),
  SFVAR(IRQCount),
  SFVAR(IRQLatch),
  SFVAR(IRQa),
  SFVAR(writeskip),
  SFVAR(DiskPtr),
  SFVAR(DiskSeekIRQ),
  SFVAR(SelectDisk),
  SFVAR(InDisk),
  SFVAR(DiskWritten),
  {fdso.cwave,64,"WAVE"},
  {fdso.mwave,32,"MWAV"},
  {amplitude,2,"AMPL"},
  {SPSG,0xB,"SPSG"},

  {&b19shiftreg60,4|MDFNSTATE_RLSB,"B60"},
  {&b24adder66,4|MDFNSTATE_RLSB,"B66"},
  {&b24latch68,4|MDFNSTATE_RLSB,"B68"},
  {&b17latch76,4|MDFNSTATE_RLSB,"B76"},
  SFEND
 };

 if(!load)
 {
  for(x=0;x<TotalSides;x++)
  {
   int b;
   for(b=0; b<65500; b++)
    diskdata[x][b] ^= diskdatao[x][b];
  }
 }
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "FDS");
 if(load)
 {
  setmirror(((FDSRegs[5]&8)>>3)^1);

  for(x=0;x<TotalSides;x++)
  {
   int b;
   for(b=0; b<65500; b++)
    diskdata[x][b] ^= diskdatao[x][b];
  }
 }
 else
 {
  for(x=0;x<TotalSides;x++)
  {
   int b;

   for(b=0; b<65500; b++)
    diskdata[x][b] ^= diskdatao[x][b];
  }
 }
 return(ret);
}

typedef struct
{
 uint8 code;
 const char *name;
} CodeNameCore;

static const char* GetManName(uint8 code)
{
 CodeNameCore Names[] = 
 {
  { 0x01, "Nintendo" },
  { 0x08, "Capcom" },
  { 0x09, "Hot-B" },
  { 0x0A, "Jaleco" },
  { 0x0B, "Coconuts" },
  { 0x0C, "Elite Systems" },
  { 0x13, "Electronic Arts" },
  { 0x18, "Hudsonsoft" },
  { 0x19, "ITC Entertainment" },
  { 0x1A, "Yanoman" },
  { 0x1D, "Clary" },
  { 0x1F, "Virgin" },
  { 0x24, "PCM Complete" },
  { 0x25, "San-X" },
  { 0x28, "Kotobuki Systems "},
  { 0x29, "SETA" },
  { 0x30, "Infogrames" },
  { 0x31, "Nintendo" },
  { 0x32, "Bandai" },             
  { 0x34, "Konami" },             
  { 0x35, "Hector" },             
  { 0x38, "Capcom" },
  { 0x39, "Banpresto" },          
  { 0x3C, "Entertainment i" },   
  { 0x3E, "Gremlin" },
  { 0x41, "Ubi Soft" },           
  { 0x42, "Atlus" },              
  { 0x44, "Malibu" },
  { 0x46, "Angel" },              
  { 0x47, "Spectrum Holobyte" },
  { 0x49, "IREM" },
  { 0x4A, "Virgin" },             
  { 0x4D, "Malibu" },             
  { 0x4F, "U.S. Gold" },
  { 0x50, "Absolute" },           
  { 0x51, "Acclaim" },            
  { 0x52, "Activision" },
  { 0x53, "American Sammy" },
  { 0x54, "Gametek" },
  { 0x55, "Park Place" },
  { 0x56, "LJN" },
  { 0x57, "Matchbox" },           
  { 0x59, "Milton Bradley" },
  { 0x5A, "Mindscape" },          
  { 0x5B, "Romstar" },            
  { 0x5C, "Naxat Soft" },
  { 0x5D, "Tradewest" },          
  { 0x60, "Titus" },              
  { 0x61, "Virgin" },
  { 0x67, "Ocean" },              
  { 0x69, "Electronic Arts" },    
  { 0x6E, "Elite Systems" },
  { 0x6F, "Electro Brain" },      
  { 0x70, "Infogrames" },         
  { 0x71, "Interplay" },
  { 0x72, "Broderbund" },         
  { 0x73, "Sculptered Soft" },
  { 0x75, "The Sales Curve" },
  { 0x78, "T*HQ" },               
  { 0x79, "Accolade" },           
  { 0x7A, "Triffix Entertainment" },
  { 0x7C, "Microprose" },         
  { 0x7F, "Kemco" },
  { 0x80, "Misawa Entertainment" },
  { 0x83, "lozc" },
  { 0x86, "Tokuma Shote" },
  { 0x8B, "Bullet-Proof Software" },
  { 0x8C, "Vic Tokai" },
  { 0x8E, "Ape" },                
  { 0x8F, "i'max" },
  { 0x91, "Chun Soft" },          
  { 0x92, "Video System" },       
  { 0x93, "Tsuburava" },
  { 0x95, "Varie" },
  { 0x96, "Yonezawa/s'pal" },     
  { 0x97, "Kaneko" },
  { 0x99, "Arc" },
  { 0x9A, "Nihon Bussan" },
  { 0x9B, "Tecmo" },
  { 0x9C, "Imagineer" },
  { 0x9D, "Banpresto" },
  { 0x9F, "Nova" },
  { 0xA1, "Hori Electric" },
  { 0xA2, "Bandai" },
  { 0xA4, "Konami" },
  { 0xA6, "Kawada" },
  { 0xA7, "Takara" },
  { 0xA9, "Technos Japan" },
  { 0xAA, "Broderbund" },
  { 0xAC, "Toei Animation" },
  { 0xAD, "Toho" },
  { 0xAF, "Namco" },
  { 0xB0, "Acclaim" },
  { 0xB1, "ASCII or Nexoft" },
  { 0xB2, "Bandai" },
  { 0xB4, "Enix" },
  { 0xB6, "HAL" },
  { 0xB7, "SNK" },
  { 0xB9, "Pony Canyon" },
  { 0xBA, "Culture Brain" },
  { 0xBB, "Sunsoft" },
  { 0xBD, "Sony Imagesoft" },
  { 0xBF, "Sammy" },
  { 0xC0, "Taito" },              
  { 0xC2, "Kemco" },              
  { 0xC3, "Squaresoft" },
  { 0xC4, "Tokuma Shoten" },
  { 0xC5, "Data East" },          
  { 0xC6, "Tonkin House" },
  { 0xC8, "Koei" },               
  { 0xC9, "UFL" },                
  { 0xCA, "Ultra" },
  { 0xCB, "Vap" },
  { 0xCC, "Use" },                
  { 0xCD, "Meldac" },
  { 0xCE, "Pony Canyon" },
  { 0xCF, "Angel" },
  { 0xD0, "Taito" },
  { 0xD1, "Sofel" },              
  { 0xD2, "Quest" },              
  { 0xD3, "Sigma Enterprises" },
  { 0xD4, "Ask Kodansha" },       
  { 0xD6, "Naxat Soft" },         
  { 0xD7, "Copya Systems" },
  { 0xD9, "Banpresto" },
  { 0xDA, "Tomy" },
  { 0xDB, "LJN" },
  { 0xDD, "NCS" },                
  { 0xDE, "Human" },              
  { 0xDF, "Altron" },
  { 0xE0, "Jaleco" },             
  { 0xE1, "Towachiki" },          
  { 0xE2, "Uutaka" },
  { 0xE3, "Varie" },              
  { 0xE5, "Epoch" },              
  { 0xE7, "Athena" },
  { 0xE8, "Asmik" },              
  { 0xE9, "Natsume" },            
  { 0xEA, "King Records" },
  { 0xEB, "Atlus" },              
  { 0xEC, "Epic/Sony Records" },
  { 0xEE, "IGS" },
  { 0xF0, "A Wave" },         
  { 0xF3, "Extreme Entertainment" },
  { 0xFF, "LJN" },
  { 0, NULL },
 };

 for(int x = 0; Names[x].name; x++)
  if(Names[x].code == code)
   return(Names[x].name);


 return(_("Unknown"));
}

bool FDS_TestMagic(const char *name, MDFNFILE *fp)
{
 if(fp->size < 16)
  return(FALSE);

 if(memcmp(fp->data, "FDS\x1a",4) && memcmp(fp->data+1,"*NINTENDO-HVC*", 14))
  return(FALSE);

 return(TRUE);
}


bool FDSLoad(const char *name, MDFNFILE *fp, NESGameType *gt)
{
 MDFNFILE *zp;

 MDFN_fseek(fp,0,SEEK_SET);

 if(!SubLoad(fp))
  return(FALSE);
   
 if(!(zp=MDFN_fopen(MDFN_MakeFName(MDFNMKF_FDSROM,0,0).c_str(),NULL, "rb", NULL)))  
 {
  MDFN_PrintError("FDS BIOS ROM image missing!");
  FreeFDSMemory();
  return 0;
 }

 if(MDFN_fread(FDSBIOS,1,8192,zp) != 8192)
 {
  MDFN_fclose(zp);
  FreeFDSMemory();
  MDFN_PrintError("Error reading FDS BIOS ROM image.");
  return 0;
 }

 if(!memcmp(FDSBIOS, "NES\x1a", 4)) // Encapsulated in iNES format?
 {
  uint32 romoff = 16 + 8192;

  if(FDSBIOS[4] == 2)
   romoff += 16384;
  MDFN_fseek(zp, romoff, SEEK_SET);
  if(MDFN_fread(FDSBIOS, 1, 8192, zp) != 8192)
  {
   MDFN_fclose(zp);
   FreeFDSMemory();
   MDFN_PrintError("Error reading FDS BIOS ROM image.");
   return(0);
  }
 }

 MDFN_fclose(zp);

 if(!(FDSRAM = (uint8*)MDFN_malloc(32768, _("FDS RAM"))))
 {
  return(0);
 }

 if(!(CHRRAM = (uint8*)MDFN_malloc(8192, _("CHR RAM"))))
 {
  return(0);
 }


 {
  MDFNFILE *tp;

  unsigned int x;
  for(x=0;x<TotalSides;x++)
  {
   diskdatao[x]=(uint8 *)MDFN_malloc(65500, _("FDS Disk Data"));
   memcpy(diskdatao[x],diskdata[x],65500);
  }

  if((tp=MDFN_fopen(MDFN_MakeFName(MDFNMKF_SAV,0, "fds").c_str(),0,"rb",0)))
  {
   FreeFDSMemory();
   if(!SubLoad(tp))
   {
    MDFN_PrintError("Error reading auxillary FDS file.");
    return(0);
   }
   MDFN_fclose(tp);
   DiskWritten=1;	/* For save state handling. */
  }
 }

 MDFNGameInfo->GameType = GMT_DISK;

 SelectDisk=0;
 InDisk=255;

 ResetCartMapping();
 SetupCartCHRMapping(0,CHRRAM,8192,1);
 SetupCartMirroring(0,0,0);
 memset(CHRRAM,0,8192);
 memset(FDSRAM,0,32768);

 MDFN_printf(_("Sides: %d\n"),TotalSides);
 MDFN_printf(_("Manufacturer Code: %02x (%s)\n"), diskdata[0][0xF], GetManName(diskdata[0][0xF]));
 MDFN_printf(_("MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 gt->Close = FDSClose;
 gt->Power = FDSInit;
 gt->StateAction = FDS_StateAction;
 return 1;
}

void FDSClose(void)
{
 FILE *fp;
 unsigned int x;

 if(!DiskWritten) return;

 if(!(fp=fopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "fds").c_str(),"wb")))
  return;

 for(x=0;x<TotalSides;x++)
 {
  if(fwrite(diskdata[x],1,65500,fp)!=65500) 
  {
   MDFN_PrintError("Error saving FDS image!");
   fclose(fp);
   return;
  }
 }
 FreeFDSMemory();
 fclose(fp);
}


static void FDSInit(void)
{
 memset(FDSRegs,0,sizeof(FDSRegs));
 writeskip=DiskPtr=DiskSeekIRQ=0;
 setmirror(1);

 setprg8r(0,0xe000,0);          // BIOS
 setprg32r(1,0x6000,0);         // 32KB RAM
 setchr8(0);           // 8KB CHR RAM

 MapIRQHook=FDSFix;

 SetReadHandler(0x4030,0x4030,FDSRead4030);
 SetReadHandler(0x4031,0x4031,FDSRead4031);
 SetReadHandler(0x4032,0x4032,FDSRead4032);
 SetReadHandler(0x4033,0x4033,FDSRead4033);

 SetWriteHandler(0x4020,0x4025,FDSWrite);

 SetWriteHandler(0x6000,0xdfff,FDSRAMWrite);
 SetReadHandler(0x6000,0xdfff,FDSRAMRead);
 SetReadHandler(0xE000,0xFFFF,FDSBIOSRead);
 IRQCount=IRQLatch=IRQa=0;

 memset(&fdso,0,sizeof(fdso));

 EXPSOUND TmpExpSound;
 memset(&TmpExpSound, 0, sizeof(TmpExpSound));

 SetReadHandler(0x4040,0x407f,FDSWaveRead);
 SetWriteHandler(0x4040,0x407f,FDSWaveWrite);
 SetWriteHandler(0x4080,0x408A,FDSSWrite);
 SetReadHandler(0x4090,0x4092,FDSSRead);

 FDS_ESI(&TmpExpSound);

 TmpExpSound.HiSync=HQSync;
 TmpExpSound.HiFill=RenderSoundHQ;
 TmpExpSound.RChange=FDS_ESI;

 GameExpSound.push_back(TmpExpSound);

 InDisk=0;
 SelectDisk=0;
}

