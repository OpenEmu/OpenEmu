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
#include <errno.h>

#include	"nes.h"
#include	"cart.h"
#include        "unif.h"
#include	"memory.h"
#include	"input.h"
#include	"vsuni.h"

typedef struct {
           char ID[4];
           uint32 info;
} UNIF_HEADER;

typedef struct {
           const char *name;
           int (*init)(CartInfo *);
	   int flags;
} BMAPPING;

typedef struct {
           const char *name;
           int (*init)(MDFNFILE *fp);
} BFMAPPING;

static CartInfo UNIFCart;

static int vramo;
static int mirrortodo;
static uint8 *boardname;
static uint8 *sboardname;

static uint32 CHRRAMSize;
uint8 *UNIFchrrama = NULL;
static uint8 *exntar = NULL;

static UNIF_HEADER unhead;
static UNIF_HEADER uchead;


static uint8 *malloced[32];
static uint32 mallocedsizes[32];

//static uint32 checksums[32];

static int FixRomSize(uint32 size, uint32 minimum)
{
  uint32 x=1;

  if(size<minimum)
   return minimum;
  while(x<size)
   x<<=1;
  return x;
}

static int UNIF_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(exntar, 2048),
  SFARRAY(UNIFchrrama, CHRRAMSize),
  SFEND
 };
 if(NESIsVSUni)
  if(!MDFNNES_VSUNIStateAction(sm,load, data_only))
   return(0);
 if(exntar || UNIFchrrama)
  if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "UNIF"))
   return(0);
 if(UNIFCart.StateAction)
  return(UNIFCart.StateAction(sm, load, data_only));
 return(1);
}

static void FreeUNIF(void)
{
 int x;
 if(UNIFchrrama)
  {free(UNIFchrrama);UNIFchrrama=0;}
 if(exntar)
 { free(exntar); exntar = 0; }
 if(boardname)
  {free(boardname);boardname=0;}
 for(x=0;x<32;x++)
 {
  if(malloced[x])
   {free(malloced[x]);malloced[x]=0;}
 }
}

static void ResetUNIF(void)
{
 int x;
 for(x=0;x<32;x++)
  malloced[x]=0;
 vramo=0;
 boardname=0;
 mirrortodo=0;
 memset(&UNIFCart,0,sizeof(UNIFCart));
 UNIFchrrama=0;
}

static void InitBoardMirroring(void)
{
 if(mirrortodo<0x4)
  SetupCartMirroring(mirrortodo,1,0);
 else if(mirrortodo==0x4)
 {
  exntar = (uint8 *)malloc(2048);
  SetupCartMirroring(4,1,exntar);
 }
 else
  SetupCartMirroring(0,0,0);
}

static int DoMirroring(MDFNFILE *fp)
{
 uint8 t;
 t=MDFN_fgetc(fp);
 mirrortodo=t; 

 {
  static const char *stuffo[6]={"Horizontal","Vertical","$2000","$2400","\"Four-screen\"","Controlled by Mapper Hardware"};
  if(t<6)
   MDFN_printf(_("Name/Attribute Table Mirroring: %s\n"),stuffo[t]);
 }
 return(1);
}

static int NAME(MDFNFILE *fp)
{
 char namebuf[100];
 int index;
 int t;

 MDFN_printf(_("Name: "));
 index=0;

 while((t=MDFN_fgetc(fp))>0)
  if(index<99)
   namebuf[index++]=t;

 namebuf[index]=0;
 MDFN_printf("%s\n",namebuf);

 if(!MDFNGameInfo->name)
 {
  MDFNGameInfo->name=(uint8 *)malloc(strlen(namebuf)+1);
  strcpy((char *)MDFNGameInfo->name,namebuf);
 }
 return(1);
}
static int DINF(MDFNFILE *fp)
{
 char name[100], method[100];
 uint8 d, m;
 uint16 y;
 int t;

 if(MDFN_fread(name,1,100,fp)!=100)
  return(0);
 if((t=MDFN_fgetc(fp))==EOF) return(0);
 d=t;
 if((t=MDFN_fgetc(fp))==EOF) return(0);
 m=t;
 if((t=MDFN_fgetc(fp))==EOF) return(0);
 y=t;
 if((t=MDFN_fgetc(fp))==EOF) return(0);
 y|=t<<8;
 if(MDFN_fread(method,1,100,fp)!=100)
  return(0);
 name[99]=method[99]=0;
 MDFN_printf(_("Dumped by: %s\n"),name);
 MDFN_printf(_("Dumped with: %s\n"),method);
 {
  char *months[12]={_("January"),_("February"),_("March"),_("April"),_("May"),_("June"),_("July"),
		    _("August"),_("September"),_("October"),_("November"),_("December")};
  MDFN_printf(_("Dumped on: %s %d, %d\n"),months[(m-1)%12],d,y);
 }
 return(1);
}

static const char *WantInput[3];

static int CTRL(MDFNFILE *fp)
{
 int t;

 if((t=MDFN_fgetc(fp))==EOF)
  return(0);
 /* The information stored in this byte isn't very helpful, but it's
    better than nothing...maybe.
 */

 if(t&1) 
 {
  WantInput[0] = WantInput[1] = "gamepad";
 }
 else 
 {
  WantInput[0] = WantInput[1] = "none";
 }

 if(t&2) 
 {
  WantInput[1] = "zapper";
 }

 return(1);
}

static int TVCI(MDFNFILE *fp)
{
 int t;
 if( (t=MDFN_fgetc(fp)) ==EOF)
  return(0);
 if(t<=2)
 {
  const char *stuffo[3]={"NTSC","PAL","NTSC and PAL"};

  if(t==0)
   MDFNGameInfo->VideoSystem = VIDSYS_NTSC;
  else if(t==1)
   MDFNGameInfo->VideoSystem = VIDSYS_PAL;

  MDFN_printf(_("TV Standard Compatibility: %s\n"),stuffo[t]);
 }
 return(1);
}

static int EnableBattery(MDFNFILE *fp)
{
 MDFN_printf(_("Battery-backed.\n"));
 if(MDFN_fgetc(fp)==EOF)
  return(0);
 UNIFCart.battery=1;
 return(1);
}

#if 0
static int PCK(MDFNFILE *fp)
{
 int z;
 z=uchead.ID[3]-'0';
 if(z<0 || z>15) return(0);
}
#endif

static int LoadPRG(MDFNFILE *fp)
{
 int z,t;
 z=uchead.ID[3]-'0';

 if(z<0 || z>15)
  return(0);
 MDFN_printf(_("PRG ROM %d size: %d"),z,(int) uchead.info);
 if(malloced[z])
  free(malloced[z]);
 t=FixRomSize(uchead.info,2048);
 if(!(malloced[z]=(uint8 *)malloc(t)))
  return(0);
 mallocedsizes[z]=t;
 memset(malloced[z]+uchead.info,0xFF,t-uchead.info);
 if(MDFN_fread(malloced[z],1,uchead.info,fp)!=uchead.info)
 {
  MDFN_printf("%m\n", errno);
  return(0);
 }
 else
  MDFN_printf("\n");

 SetupCartPRGMapping(z,malloced[z],t,0); 
 return(1);
}

static int SetBoardName(MDFNFILE *fp)
{
 if(!(boardname=(uint8 *)malloc(uchead.info+1)))
  return(0);
 MDFN_fread(boardname,1,uchead.info,fp);
 boardname[uchead.info]=0;
 MDFN_printf(_("Board name: %s\n"),boardname);
 sboardname=boardname;
 if(!memcmp(boardname,"NES-",4) || !memcmp(boardname,"UNL-",4) || !memcmp(boardname,"HVC-",4) || !memcmp(boardname,"BTL-",4) || !memcmp(boardname,"BMC-",4))
  sboardname+=4;
 return(1);
}

static int LoadCHR(MDFNFILE *fp)
{
 int z,t;
 z=uchead.ID[3]-'0';
 if(z<0 || z>15)
  return(0);
 MDFN_printf(_("CHR ROM %d size: %d"),z,(int) uchead.info);
 if(malloced[16+z])
  free(malloced[16+z]);
 t=FixRomSize(uchead.info,8192);
 if(!(malloced[16+z]=(uint8 *)malloc(t)))
  return(0);
 mallocedsizes[16+z]=t;
 memset(malloced[16+z]+uchead.info,0xFF,t-uchead.info);
 if(MDFN_fread(malloced[16+z],1,uchead.info,fp)!=uchead.info)
 {
  MDFN_printf("%m\n", errno);
  return(0);
 }
 else
  MDFN_printf("\n");

 SetupCartCHRMapping(z,malloced[16+z],t,0);
 return(1);
}


#define BMCFLAG_FORCE4	1
#define BMCFLAG_32KCHRR	2	// Generic UNIF code should make available 32K CHR RAM if no VROM is present(else just 8KB CHR RAM).

static BMAPPING bmap[] = 
{
 { "BTR", BTR_Init, 0 },

 /* MMC2 */
 { "PNROM", PNROM_Init, 0 },
 { "PEEOROM", PNROM_Init, 0},

/* Sachen Carts */
 { "TC-U01-1.5M", TCU01_Init,0},
 { "Sachen-8259B", S8259B_Init, 0},
 { "Sachen-8259A", S8259A_Init,0},
 { "Sachen-74LS374N", S74LS374N_Init,0},
 { "SA-016-1M", SA0161M_Init,0},
 { "SA-72007", SA72007_Init,0},
 { "SA-72008", SA72008_Init,0},
 { "SA-0036", SA0036_Init,0},
 { "SA-0037", SA0037_Init,0},

 { "H2288", H2288_Init,0},
 { "8237", UNL8237_Init,0},

// /* AVE carts. */
// { "MB-91", MB91_Init,0},	// DeathBots
 { "NINA-06", NINA06_Init,0},	// F-15 City War
// { "NINA-03", NINA03_Init,0},	// Tiles of Fate
// { "NINA-001", NINA001_Init,0}, // Impossible Mission 2

 { "HKROM", HKROM_Init,0},

 { "EWROM", EWROM_Init,0},
 { "EKROM", EKROM_Init,0},
 { "ELROM", ELROM_Init,0},
 { "ETROM", ETROM_Init,0},

 { "SAROM", SAROM_Init,0},
 { "SBROM", SBROM_Init,0},
 { "SCROM", SCROM_Init,0},
 { "SEROM", SEROM_Init,0},
 { "SGROM", SGROM_Init,0},
 { "SKROM", SKROM_Init,0},
 { "SLROM", SLROM_Init,0},
 { "SL1ROM", SL1ROM_Init,0},
 { "SNROM", SNROM_Init,0},
 { "SOROM", SOROM_Init,0},

 { "TGROM", TGROM_Init,0},
 { "TR1ROM", TFROM_Init,BMCFLAG_FORCE4},

 { "TEROM", TEROM_Init,0},
 { "TFROM", TFROM_Init,0},
 { "TLROM", TLROM_Init,0},
 { "TKROM", TKROM_Init,0},
 { "TSROM", TSROM_Init,0},

 { "TLSROM", TLSROM_Init,0},
 { "TKSROM", TKSROM_Init,0},
 { "TQROM", TQROM_Init,0},
 { "TVROM", TLROM_Init,BMCFLAG_FORCE4},

 { "AOROM", AOROM_Init, 0},
 { "CPROM", CPROM_Init, BMCFLAG_32KCHRR},
 { "CNROM", CNROM_Init,0},
 { "GNROM", GNROM_Init,0},
 { "NROM", NROM256_Init,0 },
 { "RROM", NROM128_Init,0 },
 { "RROM-128", NROM128_Init,0 },
 { "NROM-128", NROM128_Init,0 },
 { "NROM-256", NROM256_Init,0 },
 { "MHROM", MHROM_Init,0},
 { "UNROM", UNROM_Init, 0},

 { "MARIO1-MALEE2", MALEE_Init, 0},
 { "Supervision16in1", Supervision16_Init,0},
 { "NovelDiamond9999999in1", Novel_Init,0},
 { "Super24in1SC03", Super24_Init,0},
 { "BioMiracleA", BioMiracleA_Init, 0},

 { "603-5052", UNL6035052_Init, 0},
 {0,0,0}
};

static BFMAPPING bfunc[] = {
 { "CTRL", CTRL },
 { "TVCI", TVCI },
 { "BATR", EnableBattery },
 { "MIRR", DoMirroring },
 { "PRG",  LoadPRG },
 { "CHR",  LoadCHR },
 //{ "CCK",  CCK	   },
 //{ "PCK",  PCK	   },
 { "NAME", NAME	},
 { "MAPR", SetBoardName },
 { "DINF", DINF },
 { 0, 0 }
};

int LoadUNIFChunks(MDFNFILE *fp)
{
   int x;
   int t;
   for(;;)
   {
    t=MDFN_fread(&uchead,1,4,fp);
    if(t<4) 
    {
     if(t>0)
      return 0; 
     return 1;
    }
    if(!(MDFN_read32le(&uchead.info,fp))) 
     return 0;
    t=0;
    x=0;
	//printf("Funky: %s\n",((uint8 *)&uchead));
    while(bfunc[x].name)
    {
     if(!memcmp(&uchead,bfunc[x].name,strlen(bfunc[x].name)))
     {
      if(!bfunc[x].init(fp))
       return 0;
      t=1;
      break;     
     }
     x++;
    }
    if(!t)
     if(MDFN_fseek(fp,uchead.info,SEEK_CUR))
      return(0);
   }
}

static int InitializeBoard(void)
{
   int x=0;

   if(!sboardname) return(0);

   while(bmap[x].name)
   {
    if(!strcmp((char *)sboardname,(char *)bmap[x].name))
    {
     if(!malloced[16])
     {
      if(bmap[x].flags & BMCFLAG_32KCHRR)
	CHRRAMSize = 32768;
      else
	CHRRAMSize = 8192;
      if((UNIFchrrama=(uint8 *)malloc(CHRRAMSize)))
      {
       SetupCartCHRMapping(0,UNIFchrrama,CHRRAMSize,1);
      }
      else
       return(-1);
     }

     if(bmap[x].flags&BMCFLAG_FORCE4)
      mirrortodo=4;
     InitBoardMirroring();
     bmap[x].init(&UNIFCart);
     return(1);
    }
    x++;
   }
   MDFN_PrintError(_("Board type not supported."));
   return(0);
}

static void UNIF_Reset(void)
{
 if(UNIFCart.Reset)
  UNIFCart.Reset(&UNIFCart);
}

static void UNIF_Power(void)
{
 if(UNIFCart.Power)
  UNIFCart.Power(&UNIFCart);
 if(UNIFchrrama) memset(UNIFchrrama, 0xFF, CHRRAMSize);
}

static void UNIF_Close(void)
{
 MDFN_SaveGameSave(&UNIFCart);
 if(UNIFCart.Close)
  UNIFCart.Close();
 FreeUNIF();
}

bool UNIF_TestMagic(const char *name, MDFNFILE *fp)
{
 if(fp->size < 4)
  return(FALSE);

 if(memcmp(fp->data, "UNIF", 4))
  return(FALSE);

 return(TRUE);
}


bool UNIFLoad(const char *name, MDFNFILE *fp, NESGameType *gt)
{
	if(!UNIF_TestMagic(name, fp))
	 return(FALSE);

	MDFN_fseek(fp, 4, SEEK_SET);

	ResetCartMapping();

        ResetUNIF();
	memset(WantInput, 0, sizeof(WantInput));

        if(!MDFN_read32le(&unhead.info,fp))
	 goto aborto;
        if(MDFN_fseek(fp,0x20,SEEK_SET)<0)
	 goto aborto;
        if(!LoadUNIFChunks(fp))
	 goto aborto;
	{
	 int x;
	 md5_context md5;
	 
	 md5.starts();

	 for(x=0;x<32;x++)
	  if(malloced[x])
	  {
	   md5.update(malloced[x],mallocedsizes[x]);
	  }
	  md5.finish(UNIFCart.MD5);
          MDFN_printf(_("ROM MD5:  0x%s\n"), md5_context::asciistr(UNIFCart.MD5, 0).c_str());
	  memcpy(MDFNGameInfo->MD5,UNIFCart.MD5,sizeof(UNIFCart.MD5));
	  MDFN_printf("\n");
	}

        if(!InitializeBoard())
	 goto aborto;

	MDFN_LoadGameSave(&UNIFCart);

	gt->Power = UNIF_Power;
	gt->Reset = UNIF_Reset;
	gt->Close = UNIF_Close;
	gt->StateAction = UNIF_StateAction;

        if(UNIFCart.CartExpSound.HiFill)
         GameExpSound.push_back(UNIFCart.CartExpSound);

	MDFNGameInfo->DesiredInput.push_back(WantInput[0]);
        MDFNGameInfo->DesiredInput.push_back(WantInput[1]);
        MDFNGameInfo->DesiredInput.push_back("gamepad");
        MDFNGameInfo->DesiredInput.push_back("gamepad");

        MDFNGameInfo->DesiredInput.push_back(WantInput[2]);


        return 1;
	
	aborto:

	FreeUNIF();
	ResetUNIF();
	return 0;
}
