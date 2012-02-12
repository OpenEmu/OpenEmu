/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 1998 BERO
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "cart.h"
#include "ppu.h"

#define INESPRIV
#include "ines.h"
#include "unif.h"
#include "state.h"
#include "file.h"
#include "utils/general.h"
#include "utils/memory.h"
#include "utils/crc32.h"
#include "utils/md5.h"
#include "cheat.h"
#include "vsuni.h"
#include "driver.h"

extern SFORMAT FCEUVSUNI_STATEINFO[];

//mbg merge 6/29/06 - these need to be global
uint8 *trainerpoo=0;
uint8 *ROM = NULL;
uint8 *VROM = NULL;
iNES_HEADER head ;

CartInfo iNESCart;

uint8 iNESMirroring=0;
uint16 iNESCHRBankList[8]={0,0,0,0,0,0,0,0};
int32 iNESIRQLatch=0,iNESIRQCount=0;
uint8 iNESIRQa=0;

uint32 ROM_size=0;
uint32 VROM_size=0;
char LoadedRomFName[2048]; //mbg merge 7/17/06 added

static void iNESPower(void);
static int NewiNES_Init(int num);

void (*MapClose)(void);
void (*MapperReset)(void);

static int MapperNo=0;

/*  MapperReset() is called when the NES is reset(with the reset button).
Mapperxxx_init is called when the NES has been powered on.
*/


static DECLFR(TrainerRead)
{
	return(trainerpoo[A&0x1FF]);
}


void iNESGI(GI h) //bbit edited: removed static keyword
{
	switch(h)
	{
	case GI_RESETSAVE:
		FCEU_ClearGameSave(&iNESCart);
		break;

	case GI_RESETM2:
		if(MapperReset)
			MapperReset();
		if(iNESCart.Reset)
			iNESCart.Reset();
		break;
	case GI_POWER:
		if(iNESCart.Power)
			iNESCart.Power();
		if(trainerpoo)
		{
			for(int x = 0; x < 512; x++)
			{
				X6502_DMW(0x7000+x,trainerpoo[x]);
				if(X6502_DMR(0x7000+x)!=trainerpoo[x])
				{
					SetReadHandler(0x7000,0x71FF,TrainerRead);
					break;
				}
			}
		}
		break;
	case GI_CLOSE:
		{
			#ifndef GEKKO
			FCEU_SaveGameSave(&iNESCart);
			#endif

			if(iNESCart.Close)
				iNESCart.Close();

			if(ROM)
			{
				free(ROM);
				ROM = NULL;
			}
			if(VROM)
			{
				free(VROM);
				VROM = NULL;
			}

			if(MapClose) MapClose();
			if(trainerpoo)
			{
				free(trainerpoo);
				trainerpoo=0;
			}
		}
		break;
	}
}

uint32 iNESGameCRC32=0;

struct CRCMATCH  {
	uint32 crc;
	char *name;
};

struct INPSEL {
	uint32 crc32;
	ESI input1;
	ESI input2;
	ESIFC inputfc;
};

static void SetInput(void)
{
	static struct INPSEL moo[]=
	{
		{0x3a1694f9,SI_GAMEPAD,SI_GAMEPAD,SIFC_4PLAYER},       // Nekketsu Kakutou Densetsu

		{0xc3c0811d,SI_GAMEPAD,SI_GAMEPAD,SIFC_OEKAKIDS},  // The two "Oeka Kids" games
		{0x9d048ea4,SI_GAMEPAD,SI_GAMEPAD,SIFC_OEKAKIDS},  //

		{0xaf4010ea,SI_GAMEPAD,SI_POWERPADB,SIFC_UNSET},  // World Class Track Meet
		{0xd74b2719,SI_GAMEPAD,SI_POWERPADB,SIFC_UNSET},  // Super Team Games
		{0x61d86167,SI_GAMEPAD,SI_POWERPADB,SIFC_UNSET},  // Street Cop
		{0x6435c095,SI_GAMEPAD,SI_POWERPADB,SIFC_UNSET},  // Short Order/Eggsplode


		{0x47232739,SI_GAMEPAD,SI_GAMEPAD,SIFC_TOPRIDER},  // Top Rider

		{0x48ca0ee1,SI_GAMEPAD,SI_GAMEPAD,SIFC_BWORLD},    // Barcode World
		{0x9f8f200a,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERA}, // Super Mogura Tataki!! - Pokkun Moguraa
		{0x9044550e,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERA}, // Rairai Kyonshizu
		{0x2f128512,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERA}, // Jogging Race
		{0x60ad090a,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERA}, // Athletic World

		{0x8a12a7d9,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB}, // Totsugeki Fuuun Takeshi Jou
		{0xea90f3e2,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB}, // Running Stadium
		{0x370ceb65,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB}, // Meiro Dai Sakusen
		// Bad dump? {0x69ffb014,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB}, // Fuun Takeshi Jou 2
		{0x6cca1c1f,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB}, // Dai Undoukai
		{0x29de87af,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB},  // Aerobics Studio
		{0xbba58be5,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB},  // Family Trainer: Manhattan Police
		{0xea90f3e2,SI_GAMEPAD,SI_GAMEPAD,SIFC_FTRAINERB},  // Family Trainer:  Running Stadium

		{0xd9f45be9,SI_GAMEPAD,SI_GAMEPAD,SIFC_QUIZKING},  // Gimme a Break ...
		{0x1545bd13,SI_GAMEPAD,SI_GAMEPAD,SIFC_QUIZKING},  // Gimme a Break ... 2

		{0x7b44fb2a,SI_GAMEPAD,SI_GAMEPAD,SIFC_MAHJONG},  // Ide Yousuke Meijin no Jissen Mahjong 2
		{0x9fae4d46,SI_GAMEPAD,SI_GAMEPAD,SIFC_MAHJONG},  // Ide Yousuke Meijin no Jissen Mahjong

		{0x980be936,SI_GAMEPAD,SI_GAMEPAD,SIFC_HYPERSHOT}, // Hyper Olympic
		{0x21f85681,SI_GAMEPAD,SI_GAMEPAD,SIFC_HYPERSHOT}, // Hyper Olympic (Gentei Ban)
		{0x915a53a7,SI_GAMEPAD,SI_GAMEPAD,SIFC_HYPERSHOT}, // Hyper Sports
		{0xad9c63e2,SI_GAMEPAD,SI_UNSET,SIFC_SHADOW},  // Space Shadow

		{0x24598791,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Duck Hunt
		{0xff24d794,SI_UNSET,SI_ZAPPER,SIFC_NONE},   // Hogan's Alley
		{0xbeb8ab01,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Gumshoe
		{0xde8fd935,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // To the Earth
		{0xedc3662b,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Operation Wolf
		{0x2a6559a1,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Operation Wolf (J)
	    {0x4e959173,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Gotcha! - The Sport!

		{0x23d17f5e,SI_GAMEPAD,SI_ZAPPER,SIFC_NONE},  // The Lone Ranger
		{0xb8b9aca3,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Wild Gunman
		{0x5112dc21,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Wild Gunman
		{0x4318a2f8,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Barker Bill's Trick Shooting
		{0x5ee6008e,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Mechanized Attack
		{0x3e58a87e,SI_UNSET,SI_ZAPPER,SIFC_NONE},  // Freedom Force
		{0xe9a7fe9e,SI_UNSET,SI_MOUSE,SIFC_NONE}, // Educational Computer 2000  //mbg merge 7/17/06 added -- appears to be from newer MM build
		{0x851eb9be,SI_GAMEPAD,SI_ZAPPER,SIFC_NONE},  // Shooting Range
		{0x74bea652,SI_GAMEPAD,SI_ZAPPER,SIFC_NONE},  // Supergun 3-in-1
		{0x32fb0583,SI_UNSET,SI_ARKANOID,SIFC_NONE}, // Arkanoid(NES)
		{0xd89e5a67,SI_UNSET,SI_UNSET,SIFC_ARKANOID}, // Arkanoid (J)
		{0x0f141525,SI_UNSET,SI_UNSET,SIFC_ARKANOID}, // Arkanoid 2(J)

		{0x912989dc,SI_UNSET,SI_UNSET,SIFC_FKB},  // Playbox BASIC
		{0xf7606810,SI_UNSET,SI_UNSET,SIFC_FKB},  // Family BASIC 2.0A
		{0x895037bc,SI_UNSET,SI_UNSET,SIFC_FKB},  // Family BASIC 2.1a
		{0xb2530afc,SI_UNSET,SI_UNSET,SIFC_FKB},  // Family BASIC 3.0
		{0x82f1fb96,SI_UNSET,SI_UNSET,SIFC_SUBORKB},  // Subor 1.0 Russian
		{0xabb2f974,SI_UNSET,SI_UNSET,SIFC_SUBORKB},  // Study and Game 32-in-1
		{0xd5d6eac4,SI_UNSET,SI_UNSET,SIFC_SUBORKB},  // Edu (As)
		{0x589b6b0d,SI_UNSET,SI_UNSET,SIFC_SUBORKB},  // SuporV20
		{0x5e073a1b,SI_UNSET,SI_UNSET,SIFC_SUBORKB},  // Supor English (Chinese)
		{0x8b265862,SI_UNSET,SI_UNSET,SIFC_SUBORKB},
		{0x41401c6d,SI_UNSET,SI_UNSET,SIFC_SUBORKB},  // SuporV40
		{0x41ef9ac4,SI_UNSET,SI_UNSET,SIFC_SUBORKB},
		{0x368c19a8,SI_UNSET,SI_UNSET,SIFC_SUBORKB},   // LIKO Study Cartridge
		{0x543ab532,SI_UNSET,SI_UNSET,SIFC_SUBORKB},   // LIKO Color Lines
		{0,SI_UNSET,SI_UNSET,SIFC_UNSET}
	};
	int x=0;

	while(moo[x].input1>=0 || moo[x].input2>=0 || moo[x].inputfc>=0)
	{
		if(moo[x].crc32==iNESGameCRC32)
		{
			GameInfo->input[0]=moo[x].input1;
			GameInfo->input[1]=moo[x].input2;
			GameInfo->inputfc=moo[x].inputfc;
			break;
		}
		x++;
	}
}

#define INESB_INCOMPLETE  1
#define INESB_CORRUPT     2
#define INESB_HACKED      4

struct BADINF {
	uint64 md5partial;
	char *name;
	uint32 type;
};


static struct BADINF BadROMImages[]=
{
#include "ines-bad.h"
};

void CheckBad(uint64 md5partial)
{
	int x;

	x=0;
	//printf("0x%llx\n",md5partial);
	while(BadROMImages[x].name)
	{
		if(BadROMImages[x].md5partial == md5partial)
		{
			FCEU_PrintError("The copy game you have loaded, \"%s\", is bad, and will not work properly on FCE Ultra.", BadROMImages[x].name);
			return;
		}
		x++;
	}

}


struct CHINF {
	uint32 crc32;
	int32 mapper;
	int32 mirror;
};

void MapperInit()
{
	if(NewiNES_Init(MapperNo))
	{

	}
	else
	{
		iNESCart.Power=iNESPower;
		if(head.ROM_type&2)
		{
			iNESCart.SaveGame[0]=WRAM;
			iNESCart.SaveGameLen[0]=8192;
		}
	}
}


static void CheckHInfo(void)
{
	/* ROM images that have the battery-backed bit set in the header that really
	don't have battery-backed RAM is not that big of a problem, so I'll
	treat this differently by only listing games that should have battery-backed RAM.

	Lower 64 bits of the MD5 hash.
	*/

	static uint64 savie[]=
	{
		0x498c10dc463cfe95LL,  /* Battle Fleet */
		0x6917ffcaca2d8466LL,  /* Famista '90 */

		0xd63dcc68c2b20adcLL,    /* Final Fantasy J */
		0x012df596e2b31174LL,    /* Final Fantasy 1+2 */
		0xf6b359a720549ecdLL,    /* Final Fantasy 2 */
		0x5a30da1d9b4af35dLL,    /* Final Fantasy 3 */

		0x2ee3417ba8b69706LL,  /* Hydlide 3*/

		0xebbce5a54cf3ecc0LL,  /* Justbreed */

		0x6a858da551ba239eLL,  /* Kaijuu Monogatari */
		0xa40666740b7d22feLL,  /* Mindseeker */

		0x77b811b2760104b9LL,    /* Mouryou Senki Madara */

		0x11b69122efe86e8cLL,  /* RPG Jinsei Game */

		0xa70b495314f4d075LL,  /* Ys 3 */


		0xc04361e499748382LL,  /* AD&D Heroes of the Lance */
		0xb72ee2337ced5792LL,  /* AD&D Hillsfar */
		0x2b7103b7a27bd72fLL,  /* AD&D Pool of Radiance */

		0x854d7947a3177f57LL,    /* Crystalis */

		0xb0bcc02c843c1b79LL,  /* DW */
		0x4a1f5336b86851b6LL,  /* DW */

		0x2dcf3a98c7937c22LL,  /* DW 2 */
		0x733026b6b72f2470LL,  /* Dw 3 */
		0x98e55e09dfcc7533LL,  /* DW 4*/
		0x8da46db592a1fcf4LL,  /* Faria */
		0x91a6846d3202e3d6LL,  /* Final Fantasy */
		0xedba17a2c4608d20LL,  /* ""    */

		0x94b9484862a26cbaLL,    /* Legend of Zelda */
		0x04a31647de80fdabLL,    /*      ""      */

		0x9aa1dc16c05e7de5LL,    /* Startropics */
		0x1b084107d0878bd0LL,    /* Startropics 2*/

		0x836c0ff4f3e06e45LL,    /* Zelda 2 */

		0x82000965f04a71bbLL,    /* Mirai Shinwa Jarvas */

		0      /* Abandon all hope if the game has 0 in the lower 64-bits of its MD5 hash */
	};

	static struct CHINF moo[]=
	{
#include "ines-correct.h"
	};
	int tofix=0;
	int x;
	uint64 partialmd5=0;

	for(x=0;x<8;x++)
	{
		partialmd5 |= (uint64)iNESCart.MD5[15-x] << (x*8);
		//printf("%16llx\n",partialmd5);
	}
	CheckBad(partialmd5);

	x=0;

	do
	{
		if(moo[x].crc32==iNESGameCRC32)
		{
			if(moo[x].mapper>=0)
			{
				if(moo[x].mapper&0x800 && VROM_size)
				{
					VROM_size=0;
					free(VROM);
					VROM = NULL;
					tofix|=8;
				}
				if(MapperNo!=(moo[x].mapper&0xFF))
				{
					tofix|=1;
					MapperNo=moo[x].mapper&0xFF;
				}
			}
			if(moo[x].mirror>=0)
			{
				if(moo[x].mirror==8)
				{
					if(Mirroring==2)  /* Anything but hard-wired(four screen). */
					{
						tofix|=2;
						Mirroring=0;
					}
				}
				else if(Mirroring!=moo[x].mirror)
				{
					if(Mirroring!=(moo[x].mirror&~4))
						if((moo[x].mirror&~4)<=2)  /* Don't complain if one-screen mirroring
												   needs to be set(the iNES header can't
												   hold this information).
												   */
												   tofix|=2;
					Mirroring=moo[x].mirror;
				}
			}
			break;
		}
		x++;
	} while(moo[x].mirror>=0 || moo[x].mapper>=0);

	x=0;
	while(savie[x] != 0)
	{
		if(savie[x] == partialmd5)
		{
			if(!(head.ROM_type&2))
			{
				tofix|=4;
				head.ROM_type|=2;
			}
		}
		x++;
	}

	/* Games that use these iNES mappers tend to have the four-screen bit set
	when it should not be.
	*/
	if((MapperNo==118 || MapperNo==24 || MapperNo==26) && (Mirroring==2))
	{
		Mirroring=0;
		tofix|=2;
	}

	/* Four-screen mirroring implicitly set. */
	if(MapperNo==99)
		Mirroring=2;

	if(tofix)
	{
		char gigastr[768];
		strcpy(gigastr,"The iNES header contains incorrect information.  For now, the information will be corrected in RAM.  ");
		if(tofix&1)
			sprintf(gigastr+strlen(gigastr),"The mapper number should be set to %d.  ",MapperNo);
		if(tofix&2)
		{
			char *mstr[3]={"Horizontal","Vertical","Four-screen"};
			sprintf(gigastr+strlen(gigastr),"Mirroring should be set to \"%s\".  ",mstr[Mirroring&3]);
		}
		if(tofix&4)
			strcat(gigastr,"The battery-backed bit should be set.  ");
		if(tofix&8)
			strcat(gigastr,"This game should not have any CHR ROM.  ");
		strcat(gigastr,"\n");
		FCEU_printf("%s",gigastr);
	}
}

typedef struct {
	int mapper;
	void (*init)(CartInfo *);
} NewMI;

//this is for games that is not the a power of 2
//mapper based for now...
//not really accurate but this works since games
//that are not in the power of 2 tends to come
//in obscure mappers themselves which supports such
//size
static int not_power2[] =
{
    228
};
typedef struct {
	char* name;
	int number;
	void (*init)(CartInfo *);
} BMAPPINGLocal;

static BMAPPINGLocal bmap[] = {
	{"NROM",				0,   NROM_Init},
	{"MMC1",    			1,	 Mapper1_Init},
	{"UNROM",				2,   UNROM_Init},
	{"CNROM",				3,   CNROM_Init},
	{"MMC3",				4,   Mapper4_Init},
	{"MMC5",				5,   Mapper5_Init},
	{"ANROM",				7,   ANROM_Init},
	{"Color Dreams",		11,  Mapper11_Init},
	{"",					12,  Mapper12_Init},
	{"CPROM",				13,  CPROM_Init},
	{"100-in1",				15,  Mapper15_Init},
	{"Bandai",				16,  Mapper16_Init},
	{"Namcot 106",			19,  Mapper19_Init},
	{"Konami VRC2 type B",	23,  Mapper23_Init},
	{"Wario Land 2", 		35,  UNLSC127_Init}, // Wario Land 2
	{"TXC Policeman",		36,  Mapper36_Init}, // TXC Policeman
	{"",					37,  Mapper37_Init},
	{"Bit Corp. Crime Busters",	38,  Mapper38_Init}, // Bit Corp. Crime Busters
	{"",					43,  Mapper43_Init},
	{"",					44,  Mapper44_Init},
	{"",					45,  Mapper45_Init},
	{"",					47,  Mapper47_Init},
	{"",					49,  Mapper49_Init},
	{"",					52,  Mapper52_Init},
	{"",					57,  Mapper57_Init},
	{"",					58,  BMCGK192_Init},
	{"",					60,  BMCD1038_Init},
	{"MHROM",				66,  MHROM_Init},
	{"Sunsoft Mapper #4",	68,  Mapper68_Init},
	{"",					70,  Mapper70_Init},
	{"",					74,  Mapper74_Init},
	{"Irem 74HC161/32",		78,  Mapper78_Init},
	{"",					87,  Mapper87_Init},
	{"",					88,  Mapper88_Init},
	{"",					90,  Mapper90_Init},
	{"Sunsoft UNROM",		93,  SUNSOFT_UNROM_Init},
	{"",					94,  Mapper94_Init},
	{"",					95,  Mapper95_Init},
	{"",					101, Mapper101_Init},
	{"",					103, Mapper103_Init},
	{"",					105, Mapper105_Init},
	{"",					106, Mapper106_Init},
	{"",					107, Mapper107_Init},
	{"",					108, Mapper108_Init},
	{"",					112, Mapper112_Init},
	{"",					113, Mapper113_Init},
	{"",					114, Mapper114_Init},
	{"",					115, Mapper115_Init},
	{"",					116, Mapper116_Init},
	//    {116, UNLSL1632_Init},
	{"",					117, Mapper117_Init},
	{"TSKROM",				118, TKSROM_Init},
	{"",					119, Mapper119_Init},
	{"",					120, Mapper120_Init},
	{"",					121, Mapper121_Init},
	{"UNLH2288",			123, UNLH2288_Init},
	{"UNL22211",			132, UNL22211_Init},
	{"SA72008",				133, SA72008_Init},
	{"",					134, Mapper134_Init},
	{"TCU02",				136, TCU02_Init},
	{"S8259D",				137, S8259D_Init},
	{"S8259B",				138, S8259B_Init},
	{"S8259C",				139, S8259C_Init},
	{"",					140, Mapper140_Init},
	{"S8259A",				141, S8259A_Init},
	{"UNLKS7032",			142, UNLKS7032_Init},
	{"TCA01",				143, TCA01_Init},
	{"",					144, Mapper144_Init},
	{"SA72007",				145, SA72007_Init},
	{"SA0161M",				146, SA0161M_Init},
	{"TCU01",				147, TCU01_Init},
	{"SA0037",				148, SA0037_Init},
	{"SA0036",				149, SA0036_Init},
	{"S74LS374N",			150, S74LS374N_Init},
	{"",					152, Mapper152_Init},
	{"",					153, Mapper153_Init},
	{"",					154, Mapper154_Init},
	{"",					155, Mapper155_Init},
	{"SA009",				160, SA009_Init},
	{"",					163, Mapper163_Init},
	{"",					164, Mapper164_Init},
	{"",					165, Mapper165_Init},
	//    {169, Mapper169_Init},
	{"",					171, Mapper171_Init},
	{"",					172, Mapper172_Init},
	{"",					173, Mapper173_Init},
	{"",					175, Mapper175_Init},
	{"BMCFK23C",			176, BMCFK23C_Init},
	{"",					177, Mapper177_Init},
	{"",					178, Mapper178_Init},
	{"",					180, Mapper180_Init},
	{"",					181, Mapper181_Init},
	{"",					182, Mapper182_Init},
	{"",					183, Mapper183_Init},
	{"",					184, Mapper184_Init},
	{"",					185, Mapper185_Init},
	{"",					186, Mapper186_Init},
	{"",					187, Mapper187_Init},
	{"",					188, Mapper188_Init},
	{"",					189, Mapper189_Init},
	{"",					191, Mapper191_Init},
	{"",					192, Mapper192_Init},
	{"",					194, Mapper194_Init},
	{"",					195, Mapper195_Init},
	{"",					196, Mapper196_Init},
	{"",					197, Mapper197_Init},
	{"",					198, Mapper198_Init},
	{"",					199, Mapper199_Init},
	{"",					200, Mapper200_Init},
	{"",					205, Mapper205_Init},
	{"DEIROM",				206, DEIROM_Init},
	{"",					208, Mapper208_Init},
	{"",					209, Mapper209_Init},
	{"",					210, Mapper210_Init},
	{"",					211, Mapper211_Init},
	{"",					215, Mapper215_Init},
	{"",					216, Mapper216_Init},
	{"",					217, Mapper217_Init},
	{"UNLA9746",			219, UNLA9746_Init},
	{"OneBus",	 		    220, UNLOneBus_Init},

	//    {220, BMCFK23C_Init},
	//    {220, UNL3DBlock_Init},
	//    {220, UNLTF1201_Init},
	//    {220, TCU02_Init},
	//    {220, UNLCN22M_Init},
	//    {220, BMCT2271_Init},
	//	  {220, UNLDANCE_Init},

	{"UNLN625092",			221, UNLN625092_Init},
	{"",					222, Mapper222_Init},
	{"",					226, Mapper226_Init},
	{"",					235, Mapper235_Init},
	{"UNL6035052",			238, UNL6035052_Init},
	{"",					240, Mapper240_Init},
	{"S74LS374NA",			243, S74LS374NA_Init},
	{"",					245, Mapper245_Init},
	{"",					249, Mapper249_Init},
	{"",					250, Mapper250_Init},
	{"",					253, Mapper253_Init},
	{"",					254, Mapper254_Init},
	{"",  0,        0}
};


int iNESLoad(const char *name, FCEUFILE *fp, int OverwriteVidMode)
{
	struct md5_context md5;

	if(fp->stream->fread((char*)&head,16) != 16)
		return 0;

	if(memcmp(&head,"NES\x1a",4))
		return 0;

	head.cleanup();

	memset(&iNESCart,0,sizeof(iNESCart));

	MapperNo = (head.ROM_type>>4);
	MapperNo|=(head.ROM_type2&0xF0);
	Mirroring = (head.ROM_type&1);


	//  int ROM_size=0;
	if(!head.ROM_size)
	{
		//   FCEU_PrintError("No PRG ROM!");
		//   return(0);
		ROM_size=256;
		//head.ROM_size++;
	}
	else
		ROM_size=uppow2(head.ROM_size);

	//    ROM_size = head.ROM_size;
	VROM_size = head.VROM_size;

	int round = true;
	for (int i = 0; i != sizeof(not_power2)/sizeof(not_power2[0]); ++i)
	{
		//for games not to the power of 2, so we just read enough
		//prg rom from it, but we have to keep ROM_size to the power of 2
		//since PRGCartMapping wants ROM_size to be to the power of 2
		//so instead if not to power of 2, we just use head.ROM_size when
		//we use FCEU_read
		if (not_power2[i] == MapperNo)
		{
			round = false;
			break;
		}
	}

	if(VROM_size)
		VROM_size=uppow2(VROM_size);


	if(head.ROM_type&8) Mirroring=2;

	ROM = (uint8 *)realloc(ROM, ROM_size << 14);
	if(ROM == NULL)
		return 0;

	if(VROM_size)
	{
		VROM = (uint8 *)realloc(VROM, VROM_size << 13);
		if(VROM == NULL)
		{
			free(ROM);
			ROM = NULL;
			return 0;
		}
	}

	memset(ROM,0xFF,ROM_size<<14);
	if(VROM_size) memset(VROM,0xFF,VROM_size<<13);
	if(head.ROM_type&4)   /* Trainer */
	{
		trainerpoo = (uint8 *)realloc(trainerpoo, 512);
		fp->stream->fread((char*)trainerpoo,512);
	}

	ResetCartMapping();
	ResetExState(0,0);

	SetupCartPRGMapping(0,ROM,ROM_size*0x4000,0);
	// SetupCartPRGMapping(1,WRAM,8192,1);

	fp->stream->fread((char*)ROM,(round) ? 0x4000 * ROM_size : 0x4000 * head.ROM_size);

	if(VROM_size)
		fp->stream->fread((char*)VROM, 0x2000 * head.VROM_size);

	md5_starts(&md5);
	md5_update(&md5,ROM,ROM_size<<14);

	iNESGameCRC32=CalcCRC32(0,ROM,ROM_size<<14);

	if(VROM_size)
	{
		iNESGameCRC32=CalcCRC32(iNESGameCRC32,VROM,VROM_size<<13);
		md5_update(&md5,VROM,VROM_size<<13);
	}
	md5_finish(&md5,iNESCart.MD5);
	memcpy(&GameInfo->MD5,&iNESCart.MD5,sizeof(iNESCart.MD5));

	iNESCart.CRC32=iNESGameCRC32;

	FCEU_printf(" PRG ROM:  %3d x 16KiB\n CHR ROM:  %3d x  8KiB\n ROM CRC32:  0x%08lx\n",
			(round) ? ROM_size : head.ROM_size, head.VROM_size,iNESGameCRC32);

	{
		int x;
		FCEU_printf(" ROM MD5:  0x");
		for(x=0;x<16;x++)
			FCEU_printf("%02x",iNESCart.MD5[x]);
		FCEU_printf("\n");
	}

	char* mappername = "Not Listed";

	for(int mappertest=0;mappertest< (sizeof bmap / sizeof bmap[0]) - 1;mappertest++)
	{
		if (bmap[mappertest].number == MapperNo) {
			mappername = bmap[mappertest].name;
			break;
		}
	}

	FCEU_printf(" Mapper #:  %d\n Mapper name: %s\n Mirroring: %s\n",
			MapperNo, mappername, Mirroring==2?"None (Four-screen)":Mirroring?"Vertical":"Horizontal");

	FCEU_printf(" Battery-backed: %s\n", (head.ROM_type&2)?"Yes":"No");
	FCEU_printf(" Trained: %s\n", (head.ROM_type&4)?"Yes":"No");
	// (head.ROM_type&8) = Mirroring: None(Four-screen)

	SetInput();
	CheckHInfo();
	{
		int x;
		uint64 partialmd5=0;

		for(x=0;x<8;x++)
		{
			partialmd5 |= (uint64)iNESCart.MD5[7-x] << (x*8);
		}

		FCEU_VSUniCheck(partialmd5, &MapperNo, &Mirroring);
	}
	/* Must remain here because above functions might change value of
	   VROM_size and free(VROM).
	 */
	if(VROM_size)
		SetupCartCHRMapping(0,VROM,VROM_size*0x2000,0);

	if(Mirroring==2)
		SetupCartMirroring(4,1,ExtraNTARAM);
	else if(Mirroring>=0x10)
		SetupCartMirroring(2+(Mirroring&1),1,0);
	else
		SetupCartMirroring(Mirroring&1,(Mirroring&4)>>2,0);

	iNESCart.battery=(head.ROM_type&2)?1:0;
	iNESCart.mirror=Mirroring;

	//if(MapperNo != 18) {
	//  if(ROM) free(ROM);
	//  if(VROM) free(VROM);
	//  ROM=VROM=0;
	//  return(0);
	// }


	GameInfo->mappernum = MapperNo;
	MapperInit();
#ifndef GEKKO
	FCEU_LoadGameSave(&iNESCart);
#endif

	strcpy(LoadedRomFName,name); //bbit edited: line added

	// Extract Filename only. Should account for Windows/Unix this way.
	if (strrchr(name, '/')) {
		name = strrchr(name, '/') + 1;
	} else if(strrchr(name, '\\')) {
		name = strrchr(name, '\\') + 1;
	}

	GameInterface=iNESGI;

	// since apparently the iNES format doesn't store this information,
	// guess if the settings should be PAL or NTSC from the ROM name
	// TODO: MD5 check against a list of all known PAL games instead?
	if(OverwriteVidMode)
	{
		if(strstr(name,"(E)") || strstr(name,"(e)")
				|| strstr(name,"(F)") || strstr(name,"(f)")
				|| strstr(name,"(G)") || strstr(name,"(g)")
				|| strstr(name,"(I)") || strstr(name,"(i)")
				|| strstr(name, "(Europe)") || strstr(name, "(Australia)")
				|| strstr(name, "(France)") || strstr(name, "(Germany)")
				|| strstr(name, "(Sweden)") || strstr(name, "(En, Fr, De)")
				|| strstr(name, "(Italy)"))
			FCEUI_SetVidSystem(1);
		else
			FCEUI_SetVidSystem(0);
	}
	return 1;
}


//bbit edited: the whole function below was added
int iNesSave(){
	FILE *fp;
	char name[2048];

	if(GameInfo->type != GIT_CART)return 0;
	if(GameInterface!=iNESGI)return 0;

	strcpy(name,LoadedRomFName);
	if (strcmp(name+strlen(name)-4,".nes") != 0) { //para edit
		strcat(name,".nes");
	}

	fp = fopen(name,"wb");

	if(fwrite(&head,1,16,fp)!=16)return 0;

	if(head.ROM_type&4) 	/* Trainer */
	{
		fwrite(trainerpoo,512,1,fp);
	}

	fwrite(ROM,0x4000,ROM_size,fp);

	if(head.VROM_size)fwrite(VROM,0x2000,head.VROM_size,fp);
	fclose(fp);

	return 1;
}

int iNesSaveAs(char* name)
{
	//adelikat: TODO: iNesSave() and this have pretty much the same code, outsource the common code to a single function
	FILE *fp;

	if(GameInfo->type != GIT_CART)return 0;
	if(GameInterface!=iNESGI)return 0;

	fp = fopen(name,"wb");
	int x = 0;
	if (!fp)
		int x = 1;
	if(fwrite(&head,1,16,fp)!=16)return 0;

	if(head.ROM_type&4) 	/* Trainer */
	{
		fwrite(trainerpoo,512,1,fp);
	}

	fwrite(ROM,0x4000,ROM_size,fp);

	if(head.VROM_size)fwrite(VROM,0x2000,head.VROM_size,fp);
	fclose(fp);

	return 1;
}

//para edit: added function below
char *iNesShortFName() {
	char *ret;

	if (!(ret = strrchr(LoadedRomFName,'\\'))) {
		if (!(ret = strrchr(LoadedRomFName,'/'))) return 0;
	}
	return ret+1;
}

void VRAM_BANK1(uint32 A, uint8 V)
{
	V&=7;
	PPUCHRRAM|=(1<<(A>>10));
	CHRBankList[(A)>>10]=V;
	VPage[(A)>>10]=&CHRRAM[V<<10]-(A);
}

void VRAM_BANK4(uint32 A, uint32 V)
{
	V&=1;
	PPUCHRRAM|=(0xF<<(A>>10));
	CHRBankList[(A)>>10]=(V<<2);
	CHRBankList[((A)>>10)+1]=(V<<2)+1;
	CHRBankList[((A)>>10)+2]=(V<<2)+2;
	CHRBankList[((A)>>10)+3]=(V<<2)+3;
	VPage[(A)>>10]=&CHRRAM[V<<10]-(A);
}
void VROM_BANK1(uint32 A,uint32 V)
{
	setchr1(A,V);
	CHRBankList[(A)>>10]=V;
}

void VROM_BANK2(uint32 A,uint32 V)
{
	setchr2(A,V);
	CHRBankList[(A)>>10]=(V<<1);
	CHRBankList[((A)>>10)+1]=(V<<1)+1;
}

void VROM_BANK4(uint32 A, uint32 V)
{
	setchr4(A,V);
	CHRBankList[(A)>>10]=(V<<2);
	CHRBankList[((A)>>10)+1]=(V<<2)+1;
	CHRBankList[((A)>>10)+2]=(V<<2)+2;
	CHRBankList[((A)>>10)+3]=(V<<2)+3;
}

void VROM_BANK8(uint32 V)
{
	setchr8(V);
	CHRBankList[0]=(V<<3);
	CHRBankList[1]=(V<<3)+1;
	CHRBankList[2]=(V<<3)+2;
	CHRBankList[3]=(V<<3)+3;
	CHRBankList[4]=(V<<3)+4;
	CHRBankList[5]=(V<<3)+5;
	CHRBankList[6]=(V<<3)+6;
	CHRBankList[7]=(V<<3)+7;
}

void ROM_BANK8(uint32 A, uint32 V)
{
	setprg8(A,V);
	if(A>=0x8000)
		PRGBankList[((A-0x8000)>>13)]=V;
}

void ROM_BANK16(uint32 A, uint32 V)
{
	setprg16(A,V);
	if(A>=0x8000)
	{
		PRGBankList[((A-0x8000)>>13)]=V<<1;
		PRGBankList[((A-0x8000)>>13)+1]=(V<<1)+1;
	}
}

void ROM_BANK32(uint32 V)
{
	setprg32(0x8000,V);
	PRGBankList[0]=V<<2;
	PRGBankList[1]=(V<<2)+1;
	PRGBankList[2]=(V<<2)+2;
	PRGBankList[3]=(V<<2)+3;
}

void onemir(uint8 V)
{
	if(Mirroring==2) return;
	if(V>1)
		V=1;
	Mirroring=0x10|V;
	setmirror(MI_0+V);
}

void MIRROR_SET2(uint8 V)
{
	if(Mirroring==2) return;
	Mirroring=V;
	setmirror(V);
}

void MIRROR_SET(uint8 V)
{
	if(Mirroring==2) return;
	V^=1;
	Mirroring=V;
	setmirror(V);
}

static void NONE_init(void)
{
	ROM_BANK16(0x8000,0);
	ROM_BANK16(0xC000,~0);

	if(VROM_size)
		VROM_BANK8(0);
	else
		setvram8(CHRRAM);
}
void (*MapInitTab[256])(void)=
{
	0,
	0,
	0, //Mapper2_init,
	0, //Mapper3_init,
	0,
	0,
	Mapper6_init,
	0,//Mapper7_init,
	Mapper8_init,
	Mapper9_init,
	Mapper10_init,
	0, //Mapper11_init,
	0,
	0, //Mapper13_init,
	0,
    0, //Mapper15_init,
    0, //Mapper16_init,
	Mapper17_init,
	Mapper18_init,
	0,
	0,
	Mapper21_init,
	Mapper22_init,
    0, //Mapper23_init,
	Mapper24_init,
	Mapper25_init,
	Mapper26_init,
	Mapper27_init,
	0,
	0,
	0,
	0,
	Mapper32_init,
	Mapper33_init,
	Mapper34_init,
	0,
	0,
	0,
	0,
	0,
	Mapper40_init,
	Mapper41_init,
	Mapper42_init,
	0, //Mapper43_init,
	0,
	0,
	Mapper46_init,
	0,
	Mapper48_init,
	0,
	Mapper50_init,
	Mapper51_init,
	0,
	0,
	0,
	0,
	0,
	0,//    Mapper57_init,
	0,//    Mapper58_init,
	Mapper59_init,
	0,// Mapper60_init,
	Mapper61_init,
	Mapper62_init,
	0,
	Mapper64_init,
	Mapper65_init,
	0,//Mapper66_init,
	Mapper67_init,
    0,//Mapper68_init,
	Mapper69_init,
	0,//Mapper70_init,
	Mapper71_init,
	Mapper72_init,
	Mapper73_init,
	0,
	Mapper75_init,
	Mapper76_init,
	Mapper77_init,
	0, //Mapper78_init,
	Mapper79_init,
	Mapper80_init,
	0,
	Mapper82_init,
	Mapper83_init,
	0,
	Mapper85_init,
	Mapper86_init,
	0, //Mapper87_init,
	0, //Mapper88_init,
	Mapper89_init,
	0,
	Mapper91_init,
	Mapper92_init,
	0, //Mapper93_init,
	0, //Mapper94_init,
	0,
	Mapper96_init,
	Mapper97_init,
	0,
	Mapper99_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, //Mapper107_init,
	0,
	0,
	0,
	0,
	0,
	0, // Mapper113_init,
	0,
	0,
	0, //Mapper116_init,
	0, //Mapper117_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, //Mapper140_init,
	0,
	0,
	0,
	0, //Mapper144_init,
	0,
	0,
	0,
	0,
	0,
	0,
	Mapper151_init,
	0, //Mapper152_init,
    0, //Mapper153_init,
	0, //Mapper154_init,
	0,
	Mapper156_init,
	Mapper157_init,
	0, //Mapper158_init, removed
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Mapper166_init,
	Mapper167_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, //Mapper180_init,
	0,
	0,
	0,
	0, //Mapper184_init,
	0, //Mapper185_init,
	0,
	0,
	0,
	0, //Mapper189_init,
	0,
	0, //Mapper191_init,
	0,
	Mapper193_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0, //Mapper200_init,
	Mapper201_init,
	Mapper202_init,
	Mapper203_init,
	Mapper204_init,
	0,
	0,
	Mapper207_init,
	0,
	0,
	0,
	0, //Mapper211_init,
	Mapper212_init,
	Mapper213_init,
	Mapper214_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Mapper225_init,
	0, //Mapper226_init,
	Mapper227_init,
	Mapper228_init,
	Mapper229_init,
	Mapper230_init,
	Mapper231_init,
	Mapper232_init,
	0,
	Mapper234_init,
	0, //Mapper235_init,
	0,
	0,
	0,
	0,
	0, //Mapper240_init,
	Mapper241_init,
	Mapper242_init,
	0,
	Mapper244_init,
	0,
	Mapper246_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Mapper255_init
};

static DECLFW(BWRAM)
{
	WRAM[A-0x6000]=V;
}

static DECLFR(AWRAM)
{
	return WRAM[A-0x6000];
}


void (*MapStateRestore)(int version);
void iNESStateRestore(int version)
{
	int x;

	if(!MapperNo) return;

	for(x=0;x<4;x++)
		setprg8(0x8000+x*8192,PRGBankList[x]);

	if(VROM_size)
		for(x=0;x<8;x++)
			setchr1(0x400*x,CHRBankList[x]);

	if(0) switch(Mirroring)
	{
case 0:setmirror(MI_H);break;
case 1:setmirror(MI_V);break;
case 0x12:
case 0x10:setmirror(MI_0);break;
case 0x13:
case 0x11:setmirror(MI_1);break;
	}
	if(MapStateRestore) MapStateRestore(version);
}

static void iNESPower(void)
{
	int x;
	int type=MapperNo;

	SetReadHandler(0x8000,0xFFFF,CartBR);
	GameStateRestore=iNESStateRestore;
	MapClose=0;
	MapperReset=0;
	MapStateRestore=0;

	setprg8r(1,0x6000,0);

	SetReadHandler(0x6000,0x7FFF,AWRAM);
	SetWriteHandler(0x6000,0x7FFF,BWRAM);
	FCEU_CheatAddRAM(8,0x6000,WRAM);

	/* This statement represents atrocious code.  I need to rewrite
	all of the iNES mapper code... */
	IRQCount=IRQLatch=IRQa=0;
	if(head.ROM_type&2)
      memset(GameMemBlock+8192,0,GAME_MEM_BLOCK_SIZE-8192);
	else
      memset(GameMemBlock,0,GAME_MEM_BLOCK_SIZE);

	NONE_init();
	ResetExState(0,0);

	if(GameInfo->type == GIT_VSUNI)
		AddExState(FCEUVSUNI_STATEINFO, ~0, 0, 0);

	AddExState(WRAM, 8192, 0, "WRAM");
	if(type==19 || type==6 || type==69 || type==85 || type==96)
		AddExState(MapperExRAM, 32768, 0, "MEXR");
	if((!VROM_size || type==6 || type==19) && (type!=13 && type!=96))
		AddExState(CHRRAM, 8192, 0, "CHRR");
	if(head.ROM_type&8)
		AddExState(ExtraNTARAM, 2048, 0, "EXNR");

	/* Exclude some mappers whose emulation code handle save state stuff
	themselves. */
	if(type && type!=13 && type!=96)
	{
		AddExState(mapbyte1, 32, 0, "MPBY");
		AddExState(&Mirroring, 1, 0, "MIRR");
		AddExState(&IRQCount, 4, 1, "IRQC");
		AddExState(&IRQLatch, 4, 1, "IQL1");
		AddExState(&IRQa, 1, 0, "IRQA");
		AddExState(PRGBankList, 4, 0, "PBL");
		for(x=0;x<8;x++)
		{
			char tak[8];
			sprintf(tak,"CBL%d",x);
			AddExState(&CHRBankList[x], 2, 1,tak);
		}
	}

	if(MapInitTab[type]) MapInitTab[type]();
	else if(type)
	{
		FCEU_PrintError("iNES mapper #%d is not supported at all.",type);
	}
}

static int NewiNES_Init(int num)
{
	BMAPPINGLocal *tmp=bmap;

	if(GameInfo->type == GIT_VSUNI)
		AddExState(FCEUVSUNI_STATEINFO, ~0, 0, 0);

	while(tmp->init)
	{
		if(num==tmp->number)
		{
			UNIFchrrama=0; // need here for compatibility with UNIF mapper code
			if(!VROM_size)
			{
				int CHRRAMSize;
				if(num==13)
				{
					CHRRAMSize=16384;
				}
				else
				{
					CHRRAMSize=8192;
				}
				VROM = (uint8 *)realloc(VROM, CHRRAMSize);
				if(VROM == NULL)
					return 0;

				UNIFchrrama=VROM;
				SetupCartCHRMapping(0,VROM,CHRRAMSize,1);
				AddExState(VROM,CHRRAMSize, 0, "CHRR");
			}
			if(head.ROM_type&8)
				AddExState(ExtraNTARAM, 2048, 0, "EXNR");
			tmp->init(&iNESCart);
			return 1;
		}
		tmp++;
	}
	return 0;
}
