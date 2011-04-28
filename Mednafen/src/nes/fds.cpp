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

#include "nes.h"
#include "x6502.h"
#include "fds.h"
#include "sound.h"
#include "cart.h"
#include "nsf.h"
#include "fds-sound.h"

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

static DECLFR(FDSBIOSRead);
static DECLFR(FDSRAMRead);
static DECLFW(FDSRAMWrite);
static void FDSPower(void);
static void FDSInit(void);
static void FDSFix(int a);


static uint8 *FDSRAM = NULL;
static uint8 *CHRRAM = NULL;

static uint8 V4023;
static uint8 ExLatch;

static uint8 Control;

static int32 IRQCounter, IRQReload, IRQControl;

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
 if(IRQCounter)
 {
  IRQCounter -= a;
  if(IRQCounter <= 0)
  {
   //extern int scanline;
   //printf("FDS IRQ: %d\n", scanline);

   if(IRQControl & 1)
    IRQCounter = IRQReload;
   else
    IRQCounter = 0;
   X6502_IRQBegin(MDFN_IQEXT);
  }
 }

 if(DiskSeekIRQ>0) 
 {
  DiskSeekIRQ-=a;
  if(DiskSeekIRQ<=0)
  {
   if(Control & 0x80)
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
	//printf("Read 4030: %02x\n", ret);

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

        if(InDisk==255 || !(Control & 1) || (Control & 2))        
         ret|=2;
        return ret;
}

static DECLFR(FDSRead4033)
{
	return(0x80 & ExLatch); // battery
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

static DECLFW(FDSWrite)
{
// extern int scanline;
// if(A == 0x4020 || A == 0x4021 || A == 0x4022 || A == 0x4023)
//  MDFN_printf("IRQ Write : $%04x:$%02x, %d %08x %08x %02x\n",A,V,scanline, IRQReload, IRQCounter, V4023);
// else
//  MDFN_printf("$%04x:$%02x, %d %08x %08x\n",A,V,scanline, IRQReload, IRQCounter);

 switch(A)
 {
  case 0x4020:
	if(V4023 & 1)
	{
	 IRQReload &= 0xFF00;
	 IRQReload |= V;
	}
        break;

  case 0x4021:
	if(V4023 & 1)
	{
	 IRQReload &= 0x00FF;
	 IRQReload |= V << 8;
	}
        break;

  case 0x4022:
	X6502_IRQEnd(MDFN_IQEXT);

	if(V4023 & 1)
	{
         if(V & 2)
          IRQCounter = IRQReload;

	 IRQControl = V & 1;
	}
        break;

  case 0x4023:
	V4023 = V;
	break;

  case 0x4024:
        if(InDisk!=255 && !(Control & 0x4) && (V4023 & 0x1))
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
          if((Control & 0x40) && !(V & 0x10))
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

	Control = V;
        break;

  case 0x4026:
	ExLatch = V;
	break;

 }
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
 uint8 header[16];
 unsigned int x;

 fp->fread(header, 16, 1);
 
 if(memcmp(header,"FDS\x1a",4))
 {
  if(!(memcmp(header+1,"*NINTENDO-HVC*",14)))
  {
   long t;

   t = fp->Size();

   if(t < 65500)
    t = 65500;

   TotalSides=t/65500;

   fp->rewind();
  }
  else
   return(FALSE);
 } 
 else
  TotalSides=header[4];

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
  fp->fread(diskdata[x], 1, 65500);
 }

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
  SFARRAY(CHRRAM, 8192),

  SFVAR(V4023),
  SFVAR(ExLatch),
  SFVAR(Control),
  SFVAR(IRQCounter),
  SFVAR(IRQReload),
  SFVAR(IRQControl),
  SFVAR(writeskip),
  SFVAR(DiskPtr),
  SFVAR(DiskSeekIRQ),
  SFVAR(SelectDisk),
  SFVAR(InDisk),
  SFVAR(DiskWritten),
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
  setmirror(((Control & 8)>>3)^1);

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
 
 ret &= FDSSound_StateAction(sm, load, data_only);

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
 MDFNFILE bios_fp;
 int32 romoff = 0;

 fp->rewind();

 if(!SubLoad(fp))
  return(FALSE);
   
 md5_context md5;
 md5.starts();

 for(int x = 0; x < TotalSides; x++)
 {
  md5.update(diskdata[x],65500);

  diskdatao[x] = (uint8 *)MDFN_malloc(65500, _("FDS Disk Data"));
  memcpy(diskdatao[x],diskdata[x],65500);
 }
 md5.finish(MDFNGameInfo->MD5);

 if(!bios_fp.Open(MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, "disksys.rom").c_str(), NULL, _("FDS BIOS")))
 {
  FreeFDSMemory();
  return 0;
 }

 if(bios_fp.Size() < 8192)
 {
  MDFN_PrintError(_("FDS BIOS ROM image is too small."));

  bios_fp.Close();
  FreeFDSMemory();
  return(0);
 }

 if(!memcmp(bios_fp.Data(), "NES\x1a", 4)) // Encapsulated in iNES format?
 {
  romoff = 16 + 8192;

  if(bios_fp.Data()[4] == 2)
   romoff += 16384;

  if((bios_fp.Size() - romoff) < 8192)
  {
   MDFN_PrintError(_("FDS BIOS ROM image is too small."));
   bios_fp.Close();
   FreeFDSMemory();
   return(0);
  }
 }

 memcpy(FDSBIOS, bios_fp.Data() + romoff, 8192);

 bios_fp.Close();

 if(!(FDSRAM = (uint8*)MDFN_malloc(32768, _("FDS RAM"))))
 {
  return(0);
 }

 if(!(CHRRAM = (uint8*)MDFN_malloc(8192, _("CHR RAM"))))
 {
  return(0);
 }


 {
  MDFNFILE tp;

  if(tp.Open(MDFN_MakeFName(MDFNMKF_SAV, 0, "fds").c_str(), NULL, _("auxillary FDS data")))
  {
   FreeFDSMemory();
   if(!SubLoad(&tp))
   {
    MDFN_PrintError("Error reading auxillary FDS file.");
    return(0);
   }
   tp.Close();
   DiskWritten = 1;	/* For save state handling. */
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

 FDSInit();

 gt->Close = FDSClose;
 gt->Power = FDSPower;
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
 setprg8r(0, 0xe000, 0);          // BIOS
 setprg32r(1, 0x6000, 0);         // 32KB RAM
 setchr8(0);			  // 8KB CHR RAM

 MapIRQHook = FDSFix;

 SetReadHandler(0x4030, 0x4030, FDSRead4030);
 SetReadHandler(0x4031, 0x4031, FDSRead4031);
 SetReadHandler(0x4032, 0x4032, FDSRead4032);
 SetReadHandler(0x4033, 0x4033, FDSRead4033);

 SetWriteHandler(0x4020, 0x402F, FDSWrite);

 SetWriteHandler(0x6000, 0xdfff, FDSRAMWrite);
 SetReadHandler(0x6000, 0xdfff, FDSRAMRead);
 SetReadHandler(0xE000, 0xFFFF, FDSBIOSRead);

 FDSSound_Init();
}

static void FDSPower(void)
{
 writeskip=DiskPtr=DiskSeekIRQ=0;

 Control = 0;
 setmirror(1);

 V4023 = 0;
 ExLatch = 0;
 IRQCounter = 0;
 IRQReload = 0;
 IRQControl = 0;



 InDisk = 0;
 SelectDisk = 0;

 FDSSound_Power();
}
