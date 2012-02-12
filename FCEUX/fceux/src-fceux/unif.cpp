/* FCE Ultra - NES/Famicom Emulator
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

/* TODO:  Battery backup file saving, mirror force    */
/* **INCOMPLETE**             */
/* Override stuff: CHR RAM instead of CHR ROM,   mirroring. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "types.h"
#include "fceu.h"
#include "cart.h"
#include "unif.h"
#include "ines.h"
#include "utils/endian.h"
#include "utils/memory.h"
#include "utils/md5.h"
#include "state.h"
#include "file.h"
#include "input.h"
#include "driver.h"

typedef struct {
	char ID[4];
	uint32 info;
} UNIF_HEADER;

typedef struct {
	const char *name;
	void (*init)(CartInfo *);
	int flags;
} BMAPPING;

typedef struct {
	const char *name;
	int (*init)(FCEUFILE *fp);
} BFMAPPING;

CartInfo UNIFCart;

static int vramo;
static int mirrortodo;
static uint8 *boardname;
static uint8 *sboardname;

static uint32 CHRRAMSize;
uint8 *UNIFchrrama=0;

static UNIF_HEADER unhead;
static UNIF_HEADER uchead;


static uint8 *malloced[32];
static uint32 mallocedsizes[32];

static int FixRomSize(uint32 size, uint32 minimum)
{
	uint32 x=1; //mbg merge 7/17/06 made uint

	if(size<minimum)
		return minimum;
	while(x<size)
		x<<=1;
	return x;
}

static void FreeUNIF(void)
{
	int x;
	if(UNIFchrrama)
	{free(UNIFchrrama);UNIFchrrama=0;}
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

static uint8 exntar[2048];

static void MooMirroring(void)
{
	if(mirrortodo<0x4)
		SetupCartMirroring(mirrortodo,1,0);
	else if(mirrortodo==0x4)
	{
		SetupCartMirroring(4,1,exntar);
		AddExState(exntar, 2048, 0,"EXNR");
	}
	else
		SetupCartMirroring(0,0,0);
}

static int DoMirroring(FCEUFILE *fp)
{
	uint8 t;
	t= fp->stream->fgetc();
	mirrortodo=t;

	{
		static const char *stuffo[6]={"Horizontal","Vertical","$2000","$2400","\"Four-screen\"","Controlled by Mapper Hardware"};
		if(t<6)
			FCEU_printf(" Name/Attribute Table Mirroring: %s\n",stuffo[t]);
	}
	return(1);
}

static int NAME(FCEUFILE *fp)
{
	char namebuf[100];
	int index;
	int t;

	FCEU_printf(" Name: ");
	index=0;

	while((t= fp->stream->fgetc()) > 0)
		if(index<99)
			namebuf[index++]=t;

	namebuf[index]=0;
	FCEU_printf("%s\n",namebuf);

	if(!GameInfo->name)
	{
		GameInfo->name=(uint8*)malloc(strlen(namebuf)+1); //mbg merge 7/17/06 added cast
		strcpy((char*)GameInfo->name,namebuf); //mbg merge 7/17/06 added cast
	}
	return(1);
}
static int DINF(FCEUFILE *fp)
{
	char name[100], method[100];
	uint8 d, m;
	uint16 y;
	int t;

	if(fp->stream->fread((char*)name,100) != 100)
		return(0);
	if((t=fp->stream->fgetc()) == EOF)
		return(0);
	d=t;
	if((t=fp->stream->fgetc())==EOF)
		return(0);
	m=t;
	if((t=fp->stream->fgetc())==EOF)
		return(0);
	y=t;
	if((t=fp->stream->fgetc())==EOF)
		return(0);
	y|=t<<8;
	if(fp->stream->fread((char*)method,100) != 100)
		return(0);
	name[99]=method[99]=0;
	FCEU_printf(" Dumped by: %s\n",name);
	FCEU_printf(" Dumped with: %s\n",method);
	{
		const char *months[12]={"January","February","March","April","May","June","July",
			"August","September","October","November","December"};
		FCEU_printf(" Dumped on: %s %d, %d\n",months[(m-1)%12],d,y);
	}
	return(1);
}

static int CTRL(FCEUFILE *fp)
{
	int t;

	if((t=fp->stream->fgetc())==EOF)
		return(0);
	/* The information stored in this byte isn't very helpful, but it's
	better than nothing...maybe.
	*/

	if(t&1) GameInfo->input[0]=GameInfo->input[1]=SI_GAMEPAD;
	else GameInfo->input[0]=GameInfo->input[1]=SI_NONE;

	if(t&2) GameInfo->input[1]=SI_ZAPPER;
	//else if(t&0x10) GameInfo->input[1]=SI_POWERPAD;

	return(1);
}

static int TVCI(FCEUFILE *fp)
{
	int t;
	if( (t=fp->stream->fgetc()) ==EOF)
		return(0);
	if(t<=2)
	{
		char *stuffo[3]={"NTSC","PAL","NTSC and PAL"};
		if(t==0)
		{
			GameInfo->vidsys=GIV_NTSC;
			FCEUI_SetVidSystem(0);
		}
		else if(t==1)
		{
			GameInfo->vidsys=GIV_PAL;
			FCEUI_SetVidSystem(1);
		}
		FCEU_printf(" TV Standard Compatibility: %s\n",stuffo[t]);
	}
	return(1);
}

static int EnableBattery(FCEUFILE *fp)
{
	FCEU_printf(" Battery-backed.\n");
	if(fp->stream->fgetc() == EOF)
		return(0);
	UNIFCart.battery=1;
	return(1);
}

static int LoadPRG(FCEUFILE *fp)
{
	int z,t;
	z=uchead.ID[3]-'0';

	if(z<0 || z>15)
		return(0);
	FCEU_printf(" PRG ROM %d size: %d",z,(int) uchead.info);
	if(malloced[z])
		free(malloced[z]);
	t=FixRomSize(uchead.info,2048);
	if(!(malloced[z]=(uint8 *)malloc(t)))
		return(0);
	mallocedsizes[z]=t;
	memset(malloced[z]+uchead.info,0xFF,t-uchead.info);
	if(fp->stream->fread((char*)malloced[z], uchead.info) != uchead.info)
	{
		FCEU_printf("Read Error!\n");
		return(0);
	}
	else
		FCEU_printf("\n");

	SetupCartPRGMapping(z,malloced[z],t,0);
	return(1);
}

static int SetBoardName(FCEUFILE *fp)
{
	if(!(boardname=(uint8 *)malloc(uchead.info+1)))
		return(0);
	fp->stream->fread((char*)boardname,uchead.info);
	boardname[uchead.info]=0;
	FCEU_printf(" Board name: %s\n",boardname);
	sboardname=boardname;
	if(!memcmp(boardname,"NES-",4) || !memcmp(boardname,"UNL-",4) || !memcmp(boardname,"HVC-",4) || !memcmp(boardname,"BTL-",4) || !memcmp(boardname,"BMC-",4))
		sboardname+=4;
	return(1);
}

static int LoadCHR(FCEUFILE *fp)
{
	int z,t;
	z=uchead.ID[3]-'0';
	if(z<0 || z>15)
		return(0);
	FCEU_printf(" CHR ROM %d size: %d",z,(int) uchead.info);
	if(malloced[16+z])
		free(malloced[16+z]);
	t=FixRomSize(uchead.info,8192);
	if(!(malloced[16+z]=(uint8 *)malloc(t)))
		return(0);
	mallocedsizes[16+z]=t;
	memset(malloced[16+z]+uchead.info,0xFF,t-uchead.info);
	if(fp->stream->fread((char*)malloced[16+z],uchead.info) != uchead.info)
	{
		FCEU_printf("Read Error!\n");
		return(0);
	}
	else
		FCEU_printf("\n");

	SetupCartCHRMapping(z,malloced[16+z],t,0);
	return(1);
}


#define BMCFLAG_FORCE4 1
#define BMCFLAG_16KCHRR  2
#define BMCFLAG_32KCHRR  4

static BMAPPING bmap[] = {

	/* Sachen Carts */
	{ "TC-U01-1.5M", TCU01_Init,0},
	{ "Sachen-8259A", S8259A_Init,0},
	{ "Sachen-8259B", S8259B_Init,0},
	{ "Sachen-8259C", S8259C_Init,0},
	{ "Sachen-8259D", S8259D_Init,0},
	{ "Sachen-74LS374N", S74LS374N_Init,0},
	{ "Sachen-74LS374NA", S74LS374NA_Init,0}, //seems to be custom mapper
	{ "SA-002", TCU02_Init, 0},
	{ "SA-016-1M", SA0161M_Init,0},
	{ "SA-72007", SA72007_Init,0},
	{ "SA-72008", SA72008_Init,0},
	{ "SA-009", SA009_Init,0},
	{ "SA-0036", SA0036_Init,0},
	{ "SA-0037", SA0037_Init,0},
	{ "SA-NROM", TCA01_Init,0},

	// /* AVE carts. */
	// { "MB-91", MB91_Init,0},  // DeathBots
	// { "NINA-06", NINA06_Init,0},  // F-15 City War
	// { "NINA-03", NINA03_Init,0},  // Tiles of Fate
	// { "NINA-001", NINA001_Init,0}, // Impossible Mission 2

	{ "ANROM", ANROM_Init,0},

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

	{ "TBROM", TBROM_Init,0},
	{ "TEROM", TEROM_Init,0},
	{ "TFROM", TFROM_Init,0},
	{ "TLROM", TLROM_Init,0},
	{ "TKROM", TKROM_Init,0},
	{ "TSROM", TSROM_Init,0},

	{ "TLSROM", TLSROM_Init,0},
	{ "TKSROM", TKSROM_Init,0},
	{ "TQROM", TQROM_Init,0},
	{ "TVROM", TLROM_Init,BMCFLAG_FORCE4},

	{ "NTBROM", Mapper68_Init,0},

	{ "CPROM", CPROM_Init,BMCFLAG_16KCHRR},
	{ "CNROM", CNROM_Init,0},
	{ "NROM", NROM_Init,0 }, //NROM256_Init,0 },
	{ "NROM-128", NROM_Init,0 }, //NROM128_Init,0 },
	{ "NROM-256", NROM_Init,0 }, //NROM256_Init,0 },
	{ "RROM", NROM_Init,0 }, //NROM128_Init,0 },
	{ "RROM-128", NROM_Init,0 }, //NROM128_Init,0 },
	{ "MHROM", MHROM_Init,0},
	{ "UNROM", UNROM_Init,0},
	{ "UOROM", UNROM_Init,0},
	{ "SUNSOFT_UNROM", SUNSOFT_UNROM_Init,0},
	{ "MARIO1-MALEE2", MALEE_Init,0},
	{ "3D-BLOCK", UNL3DBlock_Init, 0},
	{ "SMB2J", UNLSMB2J_Init, 0},
	{ "AX5705", UNLAX5705_Init, 0},
	{ "CC-21", UNLCC21_Init,0},

	{ "H2288", UNLH2288_Init,0},
	{ "KOF97", UNLKOF97_Init,0},
	{ "SL1632", UNLSL1632_Init,0},
	{ "SHERO", UNLSHeroes_Init,0},
	{ "8237", UNL8237_Init,0},
	{ "8157", UNL8157_Init,0},
	{ "T-262", BMCT262_Init,0},
	{ "FK23C", BMCFK23C_Init,0},
	{ "A65AS", BMCA65AS_Init,0},
	{ "C-N22M", UNLCN22M_Init,0},
	{ "EDU2000", UNLEDU2000_Init,0},
	{ "603-5052", UNL6035052_Init,0},
	{ "N625092", UNLN625092_Init,0},
	{ "Supervision16in1", Supervision16_Init,0},
	{ "NovelDiamond9999999in1", Novel_Init,0},
	{ "Super24in1SC03", Super24_Init,0},
	{ "64in1NoRepeat", BMC64in1nr_Init, 0},
	{ "13in1JY110", BMC13in1JY110_Init, 0},
	{ "70in1", BMC70in1_Init, 0},
	{ "70in1B", BMC70in1B_Init, 0},
	{ "D1038", BMCD1038_Init, 0},
	{ "GK-192", BMCGK192_Init, 0},
	{ "SuperHIK8in1", Mapper45_Init,0},
	{ "22211", UNL22211_Init,0},
	{ "TF1201", UNLTF1201_Init, 0},
	{ "GS-2004", BMCGS2004_Init, 0},
	{ "GS-2013", BMCGS2013_Init, 0},
	{ "KS7032", UNLKS7032_Init, 0},
	{ "T-230", UNLT230_Init, 0},
	{ "190in1", BMC190in1_Init, 0},
	{ "Ghostbusters63in1", BMCGhostbusters63in1_Init, 0},
	{ "BS-5",BMCBS5_Init, 0},
	{ "411120-C",BMC411120C_Init, 0},
	{ "830118C",BMC830118C_Init, 0},
	{ "T-227-1",BMCT2271_Init,0},

	{ "DREAMTECH01", DreamTech01_Init,0},
	{ "KONAMI-QTAI", Mapper190_Init,0},

	//{ "DANCE", UNLDANCE_Init,0},
	{ "DANCE", NULL,0},
	{ "OneBus", UNLOneBus_Init,0},
	{ "SC-127", UNLSC127_Init,0},

	{ "TEK90", Mapper90_Init,0},

	{ "COPYFAMI_MMC3", MapperCopyFamiMMC3_Init,0},

	{0,0,0}
};

static BFMAPPING bfunc[] = {
	{ "CTRL", CTRL },
	{ "TVCI", TVCI },
	{ "BATR", EnableBattery },
	{ "MIRR", DoMirroring },
	{ "PRG",  LoadPRG },
	{ "CHR",  LoadCHR },
	{ "NAME", NAME  },
	{ "MAPR", SetBoardName },
	{ "DINF", DINF },
	{ 0, 0 }
};

int LoadUNIFChunks(FCEUFILE *fp)
{
	int x;
	int t;
	for(;;)
	{
		t=fp->stream->fread((char*)&uchead,4);
		if(t<4)
		{
			if(t>0)
				return 0;
			return 1;
		}
		if(!(read32le(&uchead.info,fp->stream)))
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
			if(FCEU_fseek(fp,uchead.info,SEEK_CUR))
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
				if(bmap[x].flags & BMCFLAG_16KCHRR)
					CHRRAMSize = 16384;
				else
					CHRRAMSize = 8192;
				if((UNIFchrrama=(uint8 *)malloc(CHRRAMSize)))
				{
					SetupCartCHRMapping(0,UNIFchrrama,CHRRAMSize,1);
					AddExState(UNIFchrrama, CHRRAMSize, 0,"CHRR");
				}
				else
					return(-1);
			}
			if(bmap[x].flags&BMCFLAG_FORCE4)
				mirrortodo=4;
			MooMirroring();
			bmap[x].init(&UNIFCart);
			return(1);
		}
		x++;
	}
	FCEU_PrintError("Board type not supported.");
	return(0);
}

static void UNIFGI(GI h)
{
	switch(h)
	{
	case GI_RESETSAVE:
		FCEU_ClearGameSave(&UNIFCart);
		break;

	case GI_RESETM2:
		if(UNIFCart.Reset)
			UNIFCart.Reset();
		break;
	case GI_POWER:
		if(UNIFCart.Power)
			UNIFCart.Power();
		if(UNIFchrrama) memset(UNIFchrrama,0,8192);
		break;
	case GI_CLOSE:
		#ifndef GEKKO
		FCEU_SaveGameSave(&UNIFCart);
		#endif
		if(UNIFCart.Close)
			UNIFCart.Close();
		FreeUNIF();
		break;
	}
}

int UNIFLoad(const char *name, FCEUFILE *fp)
{
	FCEU_fseek(fp,0,SEEK_SET);
	fp->stream->fread((char*)&unhead,4);
	if(memcmp(&unhead,"UNIF",4))
		return 0;

	ResetCartMapping();

	ResetExState(0,0);
	ResetUNIF();
	if(!read32le(&unhead.info,fp->stream))
		goto aborto;
	if(FCEU_fseek(fp,0x20,SEEK_SET)<0)
		goto aborto;
	if(!LoadUNIFChunks(fp))
		goto aborto;
	{
		int x;
		struct md5_context md5;

		md5_starts(&md5);

		for(x=0;x<32;x++)
			if(malloced[x])
			{
				md5_update(&md5,malloced[x],mallocedsizes[x]);
			}
			md5_finish(&md5,UNIFCart.MD5);
			FCEU_printf(" ROM MD5:  0x");
			for(x=0;x<16;x++)
				FCEU_printf("%02x",UNIFCart.MD5[x]);
			FCEU_printf("\n");
			memcpy(&GameInfo->MD5,&UNIFCart.MD5,sizeof(UNIFCart.MD5));
	}

	if(!InitializeBoard())
		goto aborto;

	#ifndef GEKKO
	FCEU_LoadGameSave(&UNIFCart);
	#endif

	strcpy(LoadedRomFName,name); //For the debugger list
	GameInterface=UNIFGI;
	return 1;

aborto:

	FreeUNIF();
	ResetUNIF();


	return 0;
}
