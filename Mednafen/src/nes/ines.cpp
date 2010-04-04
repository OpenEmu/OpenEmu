/* Mednafen - Multi-system Emulator
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

#include <string.h>
#include <string>
#include <zlib.h>
#include <math.h>

#include "nes.h"
#include "x6502.h"
#include "cart.h"
#include "ppu/ppu.h"
#include "memory.h"

#define INESPRIV
#include "ines.h"
#include "unif.h"
#include "vsuni.h"
#include "input.h"


static uint8 ExtraNTARAM[0x800];
static uint32 CHRRAMSize;

static uint8 trainerpoo[512], TrainerRAM[512];
static uint8 *WRAM = NULL; // External RAM used for ROM images using mappers that nominally don't support external RAM
static uint8 *ROM = NULL;
static uint8 *VROM = NULL;
static CartInfo iNESCart;

static int Mirroring;

static uint32 ROM_size;
static uint32 VROM_size;

static int NewiNES_Init(int num);

static int MapperNo;

static iNES_HEADER head;

static DECLFR(TrainerRead)
{
 return(TrainerRAM[A&0x1FF]);
}

static DECLFW(TrainerWrite)
{
 TrainerRAM[A & 0x1FF] = V;
}

static int iNES_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(ExtraNTARAM, head.ROM_type & 8 ? 2048 : 0),
  SFARRAY(VROM, VROM_size ? 0 : CHRRAMSize),
  SFARRAY(TrainerRAM, (head.ROM_type & 4) ? 0x200 : 0),
  SFARRAY(WRAM, WRAM ? 8192 : 0),
  SFEND
 };

 if(NESIsVSUni)
  if(!MDFNNES_VSUNIStateAction(sm,load, data_only))
   return(0);

 if(head.ROM_type & 8 || !VROM_size)
  if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "iNES"))
   return(0);

 if(iNESCart.StateAction)
  return(iNESCart.StateAction(sm, load, data_only));
 return(1);
}
static void iNESFree(void)
{
	if(ROM)
	{
	 MDFN_free(ROM);
	 ROM = NULL;
	}

	if(VROM)
	{
	 MDFN_free(VROM);
	 VROM = NULL;
	}

	if(WRAM)
	{
	 MDFN_free(WRAM);
	 WRAM = NULL;
	}
}

static void iNES_Reset(void)
{
        if(head.ROM_type & 4)
         memcpy(TrainerRAM, trainerpoo, 512);

	if(iNESCart.Reset)
	 iNESCart.Reset(&iNESCart);
}

static void iNES_Power(void)
{
	if(!VROM_size)
	 memset(VROM, 0xFF, CHRRAMSize);

	if(head.ROM_type & 8)
	 memset(ExtraNTARAM, 0xFF, 2048);

        if(head.ROM_type & 4)
         memcpy(TrainerRAM, trainerpoo, 512);

	if(WRAM)
	 setprg8r(0x10, 0x6000, 0);

	if(iNESCart.Power)
	 iNESCart.Power(&iNESCart);
}

static void iNES_Close(void)
{
	MDFN_SaveGameSave(&iNESCart);

	if(iNESCart.Close) iNESCart.Close();

	iNESFree();
}

#ifdef WANT_DEBUGGER
static void iNES_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "prgrom"))
 {
  while(Length--)
  {
   Address &= (ROM_size << 14) - 1;
   *Buffer = ROM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "chrrom") && VROM_size)
 {
  while(Length--)
  {
   Address &= (VROM_size << 13) - 1;
   *Buffer = VROM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "chrram") && !VROM_size && CHRRAMSize)
 {
  while(Length--)
  {
   Address &= CHRRAMSize - 1;
   *Buffer = VROM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "wram") && WRAM)
 {
  while(Length--)
  {
   Address &= 0x1FFF;
   *Buffer = WRAM[Address];
   Address++;
   Buffer++;
  }
 }
}

static void iNES_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "prgrom"))
 {
  while(Length--)
  {
   Address &= (ROM_size << 14) - 1;
   ROM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "chrrom") && VROM_size)
 {
  while(Length--)
  {
   Address &= (VROM_size << 13) - 1;
   VROM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "chrram") && !VROM_size && CHRRAMSize)
 {
  while(Length--)
  {
   Address &= CHRRAMSize - 1;
   VROM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "wram") && WRAM)
 {
  while(Length--)
  {
   Address &= 0x1FFF;
   WRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
}
#endif


uint32 iNESGameCRC32;

struct CRCMATCH	{
	uint32 crc;
	const char *name;
};

struct INPSEL {
	const uint32 crc32;
	const char* input1;
	const char *input2;
	const char *inputfc;
};

/* This is mostly for my personal use.  So HA. */
static void SetInput(void)
{
 static struct INPSEL moo[]=
	{
         {0x3a1694f9,"gamepad","gamepad","4player"},       /* Nekketsu Kakutou Densetsu */

	 {0xc3c0811d,"gamepad","gamepad","oekakids"},	/* The two "Oeka Kids" games */
	 {0x9d048ea4,"gamepad","gamepad","oekakids"},	/*			     */

	 {0xaf4010ea,"gamepad","powerpadb", NULL},	/* World Class Track Meet */
	 {0xd74b2719,"gamepad","powerpadb", NULL},	/* Super Team Games */
	 {0x61d86167,"gamepad","powerpadb", NULL},	/* Street Cop */
	 {0x6435c095,"gamepad","powerpadb", NULL},	    /* Short Order/Eggsplode */


	 {0x48ca0ee1,"gamepad","gamepad", "bworld"},    /* Barcode World */
	 {0x9f8f200a,"gamepad","gamepad","ftrainera"}, /* Super Mogura Tataki!! - Pokkun Moguraa */
	 {0x9044550e,"gamepad","gamepad","ftrainera"}, /* Rairai Kyonshizu */
	 {0x2f128512,"gamepad","gamepad","ftrainera"}, /* Jogging Race */
	 {0x60ad090a,"gamepad","gamepad","ftrainera"}, /* Athletic World */

	 {0x8a12a7d9,"gamepad","gamepad","ftrainerb"}, /* Totsugeki Fuuun Takeshi Jou */
	 {0xea90f3e2,"gamepad","gamepad","ftrainerb"}, /* Running Stadium */
	 {0x370ceb65,"gamepad","gamepad","ftrainerb"}, /* Meiro Dai Sakusen */
	 // Bad dump? {0x69ffb014,"gamepad","gamepad","ftrainerb"}, /* Fuun Takeshi Jou 2 */
	 {0x6cca1c1f,"gamepad","gamepad","ftrainerb"}, /* Dai Undoukai */
	 {0x29de87af,"gamepad","gamepad","ftrainerb"},  /* Aerobics Studio */
         {0xbba58be5,"gamepad","gamepad","ftrainerb"},  /* Family Trainer: Manhattan Police */
	 {0xea90f3e2,"gamepad","gamepad","ftrainerb"},  /* Family Trainer:  Running Stadium */

	 {0xd9f45be9,"gamepad","gamepad","quizking"},  /* Gimme a Break ... */
	 {0x1545bd13,"gamepad","gamepad","quizking"},  /* Gimme a Break ... 2 */

	 {0x7b44fb2a,"gamepad","gamepad","mahjong"},  /* Ide Yousuke Meijin no Jissen Mahjong 2 */
	 {0x9fae4d46,"gamepad","gamepad","mahjong"},  /* Ide Yousuke Meijin no Jissen Mahjong */

	 {0x980be936,"gamepad","gamepad","hypershot"}, /* Hyper Olympic */
	 {0x21f85681,"gamepad","gamepad","hypershot"}, /* Hyper Olympic (Gentei Ban) */
	 {0x915a53a7,"gamepad","gamepad","hypershot"}, /* Hyper Sports */
	 {0xad9c63e2,"gamepad",NULL,"shadow"},	/* Space Shadow */

	 {0x4d68cfb1, NULL, "zapper", "none"},	/* Crime Busters */
	 {0xbbe40dc4,NULL,"zapper", "none"},	/* Baby Boomer */
	 {0x24598791,NULL,"zapper", "none"},	/* Duck Hunt */
	 {0xff24d794,NULL,"zapper", "none"},   /* Hogan's Alley */
	 {0xbeb8ab01,NULL,"zapper", "none"},	/* Gumshoe */
	 {0xde8fd935,NULL,"zapper", "none"},	/* To the Earth */
	 {0xedc3662b,NULL,"zapper", "none"},	/* Operation Wolf */
	 {0x2a6559a1,NULL,"zapper", "none"},	/* Operation Wolf (J) */

	 {0x23d17f5e,"gamepad","zapper", "none"},	/* The Lone Ranger */
	 {0xb8b9aca3,NULL,"zapper", "none"},  /* Wild Gunman */
	 {0x5112dc21,NULL,"zapper", "none"},  /* Wild Gunman */
	 {0x4318a2f8,NULL,"zapper", "none"},  /* Barker Bill's Trick Shooting */
	 {0x5ee6008e,NULL,"zapper", "none"},  /* Mechanized Attack */
	 {0x3e58a87e,NULL,"zapper", "none"},  /* Freedom Force */
	 {0x851eb9be,"gamepad","zapper", "none"},	/* Shooting Range */
	 {0x74bea652,"gamepad","zapper", "none"},	/* Supergun 3-in-1 */
	 {0x32fb0583,NULL,"arkanoid","none"}, /* Arkanoid(NES) */
	 {0xd89e5a67,NULL,NULL,"arkanoid"}, /* Arkanoid (J) */
	 {0x0f141525,NULL,NULL,"arkanoid"}, /* Arkanoid 2(J) */

	 {0x912989dc,NULL,NULL,"fkb"},	/* Playbox BASIC */
	 {0xf7606810,NULL,NULL,"fkb"},	/* Family BASIC 2.0A */
	 {0x895037bc,NULL,NULL,"fkb"},	/* Family BASIC 2.1a */
	 {0xb2530afc,NULL,NULL,"fkb"},	/* Family BASIC 3.0 */
	 {0, NULL, NULL, NULL },
	};
 int x=0;

 while(moo[x].input1 > 0 || moo[x].input2 > 0 || moo[x].inputfc > 0)
 {
  if(moo[x].crc32==iNESGameCRC32)
  {
   MDFNGameInfo->DesiredInput.clear();
   MDFNGameInfo->DesiredInput.push_back(moo[x].input1);
   MDFNGameInfo->DesiredInput.push_back(moo[x].input2);
   MDFNGameInfo->DesiredInput.push_back("gamepad");
   MDFNGameInfo->DesiredInput.push_back("gamepad");
   MDFNGameInfo->DesiredInput.push_back(moo[x].inputfc);
   break;
  }
  x++;
 }
}

#define INESB_INCOMPLETE        1
#define INESB_CORRUPT           2
#define INESB_HACKED            4

struct BADINF {
        uint64 md5partial;
        const char *name;
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
   MDFN_PrintError(_("The copy of the game you have loaded, \"%s\", is bad, and will not work properly on Mednafen."), BadROMImages[x].name);
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
         0xedba17a2c4608d20LL,  /* ""           */

         0x94b9484862a26cbaLL,    /* Legend of Zelda */
         0x04a31647de80fdabLL,    /*      ""      */

         0x9aa1dc16c05e7de5LL,    /* Startropics */
         0x1b084107d0878bd0LL,    /* Startropics 2*/

         0x836c0ff4f3e06e45LL,    /* Zelda 2 */

         0                      /* Abandon all hope if the game has 0 in the lower 64-bits of its MD5 hash */
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
  if(moo[x].crc32 == iNESGameCRC32)
  {
   if(moo[x].mapper >= 0)
   {
    if((moo[x].mapper & 0x800) && VROM_size)
    {
     VROM_size=0;
     MDFN_free(VROM);
     VROM = 0;
     tofix|=8;
    }
    if(MapperNo != (moo[x].mapper & 0xFF))
    {
     tofix|=1;
     MapperNo=moo[x].mapper&0xFF;
    }
   }
   if(moo[x].mirror>=0)
   {
    if(moo[x].mirror==8)
    {
     if(Mirroring==2)   /* Anything but hard-wired(four screen). */
     {
      tofix|=2;
      Mirroring=0;
     }
    }
    else if(Mirroring!=moo[x].mirror)
    {
     if(Mirroring!=(moo[x].mirror&~4))
      if((moo[x].mirror&~4)<=2) /* Don't complain if one-screen mirroring
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
  MDFN_printf(_("The iNES header contains incorrect information.  For now, the information will be corrected in RAM:\n"));
  MDFN_indent(1);
  if(tofix&1)
   MDFN_printf(_("The mapper number should be set to %d.\n"), MapperNo);

  if(tofix&2)
  {
   const char *mstr[3] = {
			  _("Horizontal"), _("Vertical"), _("Four-screen")
		 	 };
   MDFN_printf(_("Mirroring should be set to \"%s\".\n"), mstr[Mirroring & 3]);
  }
  if(tofix&4)
  {
   MDFN_printf(_("The battery-backed bit should be set.\n"));
  }
  if(tofix&8)
  {
   MDFN_printf(_("This game should not have any CHR ROM.  "));
  }
  MDFN_indent(-1);
 }
}



bool iNES_TestMagic(const char *name, MDFNFILE *fp)
{
 if(fp->size < 16)
  return(FALSE);

 if(memcmp(fp->data, "NES\x1a", 4))
  return(FALSE);

 return(TRUE);
}

bool iNESLoad(const char *name, MDFNFILE *fp, NESGameType *gt)
{
        md5_context md5;

	if(!iNES_TestMagic(name, fp))
	 return(FALSE);

	/* File size is too small to be an iNES file */
        memcpy(&head, fp->data, 16);
	MDFN_fseek(fp, 16, SEEK_SET);

	memset(&iNESCart,0,sizeof(iNESCart));

	/* Do some fixes to common strings found in
	   old iNES-format headers.
	*/
        if(!memcmp((char *)(&head)+0x7,"DiskDude",8))
        {
         memset((char *)(&head)+0x7,0,0x9);
        }

        if(!memcmp((char *)(&head)+0x7,"demiforce",9))
        {
         memset((char *)(&head)+0x7,0,0x9);
        }

        if(!memcmp((char *)(&head)+0xA,"Ni03",4))
        {
         if(!memcmp((char *)(&head)+0x7,"Dis",3))
          memset((char *)(&head)+0x7,0,0x9);
         else
          memset((char *)(&head)+0xA,0,0x6);
        }

        if(!head.ROM_size)
	{
	 MDFN_PrintError(_("No PRG ROM present!"));
	 return(0);

	}
        ROM_size = head.ROM_size;
        VROM_size = head.VROM_size;
	ROM_size = uppow2(ROM_size);

        if(VROM_size)
	 VROM_size = uppow2(VROM_size);

        MapperNo = (head.ROM_type>>4);
        MapperNo|=(head.ROM_type2&0xF0);
        Mirroring = (head.ROM_type&1);

	if(head.ROM_type&8) Mirroring=2;

        if(!(ROM = (uint8 *)MDFN_malloc(ROM_size<<14, _("PRG ROM data"))))
	{
	 iNESFree();
	 return 0;
	}

	#ifdef WANT_DEBUGGER
	MDFNDBG_AddASpace(iNES_GetAddressSpaceBytes, iNES_PutAddressSpaceBytes, "prgrom", "PRG ROM", (unsigned int)(log(ROM_size << 14) / log(2)));
	#endif

        if (VROM_size) 
	{
         if(!(VROM = (uint8 *)MDFN_malloc(VROM_size<<13, _("CHR ROM data"))))
	 {
	  iNESFree();
	  return 0;
	 }
	 #ifdef WANT_DEBUGGER
	 MDFNDBG_AddASpace(iNES_GetAddressSpaceBytes, iNES_PutAddressSpaceBytes, "chrrom", "CHR ROM", (unsigned int)(log(VROM_size << 13) / log(2)));
	 #endif
	}
        memset(ROM,0xFF,ROM_size<<14);
        if(VROM_size) memset(VROM,0xFF,VROM_size<<13);
        if(head.ROM_type&4)     /* Trainer */
        {
         if(MDFN_fread(trainerpoo,512,1,fp) != 1)
	 {
	  MDFN_PrintError(_("Error reading trainer."));
	  return(0);
	 }
        }

	ResetCartMapping();

	SetupCartPRGMapping(0,ROM,ROM_size*0x4000,0);

        if(MDFN_fread(ROM,0x4000,head.ROM_size,fp) != head.ROM_size)
	{
	 MDFN_PrintError(_("Error reading PRG ROM data"));
	 iNESFree();
	 return(0);
	}

	if(VROM_size)
	{
	 if(MDFN_fread(VROM,0x2000,head.VROM_size,fp) != head.VROM_size)
	 {
	  MDFN_PrintError(_("Error reading CHR ROM data"));
	  iNESFree();
	  return(0);
	 }
	}
        md5.starts();
        md5.update(ROM,ROM_size<<14);

	iNESGameCRC32 = crc32(0,ROM,ROM_size<<14);

	if(VROM_size)
	{
	 iNESGameCRC32 = crc32(iNESGameCRC32,VROM,VROM_size<<13);
         md5.update(VROM,VROM_size<<13);
	}
	md5.finish(iNESCart.MD5);
	memcpy(MDFNGameInfo->MD5,iNESCart.MD5,sizeof(iNESCart.MD5));

	iNESCart.CRC32 = iNESGameCRC32;

        MDFN_printf(_("PRG ROM:  %3d x 16KiB\n"), head.ROM_size);
	MDFN_printf(_("CHR ROM:  %3d x  8KiB\n"), head.VROM_size);
	MDFN_printf(_("ROM CRC32:  0x%08x\n"), iNESGameCRC32);

        MDFN_printf(_("ROM MD5:  0x%s\n"), md5_context::asciistr(iNESCart.MD5, 0).c_str());
	MDFN_printf(_("Mapper:  %d\n"), MapperNo);
	MDFN_printf(_("Mirroring: %s\n"), Mirroring==2?_("None(Four-screen)"):Mirroring?_("Vertical"):_("Horizontal"));

        if(head.ROM_type&2) MDFN_printf(_("Battery-backed.\n"));
        if(head.ROM_type&4) MDFN_printf(_("Trained.\n"));

	SetInput();
	CheckHInfo();
	{
	 int x;
	 uint64 partialmd5=0;

	 for(x=0;x<8;x++)
	 {
	  partialmd5 |= (uint64)iNESCart.MD5[7-x] << (x*8);
	 } 

	 MDFN_VSUniCheck(partialmd5, &MapperNo, &Mirroring);
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
	
	iNESCart.battery = (head.ROM_type&2)?1:0;
	iNESCart.mirror = Mirroring;

	if(!NewiNES_Init(MapperNo))
	{
	 iNESFree();
	 return(0);
	}

	MDFN_LoadGameSave(&iNESCart);

	gt->Power = iNES_Power;
	gt->Reset = iNES_Reset;
	gt->Close = iNES_Close;
	gt->StateAction = iNES_StateAction;

	if(iNESCart.CartExpSound.HiFill)
	 GameExpSound.push_back(iNESCart.CartExpSound);

	if(head.ROM_type & 4)
	{
         SetReadHandler(0x7000, 0x71FF, TrainerRead);
	 SetWriteHandler(0x7000, 0x71FF, TrainerWrite);
	}
        return 1;
}


#define BMAPF_32KCHRR		1	// 32KB CHR RAM
#define BMAPF_INESWRAMOK	2	// Ok to emulate WRAM here(ines.cpp) if the battery-backed bit is set.

typedef struct {
           int number;   
           int (*init)(CartInfo *);
	   uint32 flags;
} BMAPPING;

static const BMAPPING bmap[] = {
	{ 0, NROM256_Init, BMAPF_INESWRAMOK },
	{ 1, Mapper1_Init, 0 },
	{ 2, UNROM_Init, BMAPF_INESWRAMOK },
	{ 3, CNROM_Init, BMAPF_INESWRAMOK },
	{ 4, Mapper4_Init, 0},
	{ 5, Mapper5_Init, 0},
	{ 6, Mapper6_Init,  BMAPF_32KCHRR},
	{ 7, AOROM_Init, 0},
	{ 8, Mapper8_Init, 0},
	{ 9, PNROM_Init, 0},
	{ 10, MMC4_Init, 0},
	{ 11, Mapper11_Init, 0},
	{ 12, Mapper12_Init, 0},
	{ 13, CPROM_Init, BMAPF_32KCHRR},
	{ 15, Mapper15_Init, 0 },
	{ 16, Mapper16_Init, 0},
	{ 17, Mapper17_Init, 0},
	{ 18, Mapper18_Init, 0},
	{ 19, Mapper19_Init, 0},
	{ 21, Mapper21_Init, 0},
	{ 22, Mapper22_Init, 0},
	{ 23, Mapper23_Init, 0},
	{ 24, Mapper24_Init, 0},
	{ 25, Mapper25_Init, 0},
	{ 26, Mapper26_Init, 0},
	{ 32, Mapper32_Init, 0},
	{ 33, Mapper33_Init, 0},
	{ 34, Mapper34_Init, 0},
	{ 38, Mapper38_Init, 0},
        { 41, Mapper41_Init, 0},
	{ 42, BioMiracleA_Init, 0},
        { 44, Mapper44_Init, 0},
        { 45, Mapper45_Init, 0},
        { 46, Mapper46_Init, 0},
        { 47, Mapper47_Init, 0},
	{ 48, Mapper48_Init, 0},
        { 49, Mapper49_Init, 0},
	{ 51, Mapper51_Init, 0},
        { 52, Mapper52_Init, 0},
	{ 64, Mapper64_Init, 0},
	{ 65, Mapper65_Init, 0},
	{ 66, GNROM_Init, 0},
	{ 67, Mapper67_Init, 0},
	{ 68, Mapper68_Init, 0},
	{ 69, BTR_Init, 0},
	{ 70, Mapper70_Init, 0},
	{ 71, Mapper71_Init, 0},
	{ 72, Mapper72_Init, 0},
	{ 73, Mapper73_Init, 0},
        { 74, Mapper74_Init, 0},
	{ 75, Mapper75_Init, 0},
	{ 76, Mapper76_Init, 0},
	{ 77, Mapper77_Init, 0},
	{ 78, Mapper78_Init, 0},
	{ 79, NINA06_Init, 0},
        { 80, Mapper80_Init, 0},
        { 82, Mapper82_Init, 0},
	{ 85, Mapper85_Init, 0},
        { 86, Mapper86_Init, 0},
	{ 87, Mapper87_Init, 0},
        { 88, Mapper88_Init, 0},
        { 89, Mapper89_Init, 0},
	{ 90, Mapper90_Init, 0},
	{ 92, Mapper92_Init, 0},
	{ 93, Mapper93_Init, 0},
	{ 94, Mapper94_Init, 0},
        { 95, Mapper95_Init, 0},
	{ 96, Mapper96_Init, 0},
	{ 97, Mapper97_Init, 0},
	{ 99, Mapper99_Init, 0},
	{ 105, Mapper105_Init, 0},
	{ 107, Mapper107_Init, 0},
	{ 112, Mapper112_Init, 0},
	{ 113, Mapper113_Init, 0},
	{ 114, Mapper114_Init, 0},
        { 115, Mapper115_Init, 0},
        { 116, Mapper116_Init, 0},
	{ 117, Mapper117_Init, 0},
	{ 118, Mapper118_Init, 0},
	{ 119, Mapper119_Init, 0},	/* Has CHR ROM and CHR RAM by default.  Hmm. */
	{ 133, SA72008_Init, 0},
	{ 134, S74LS374N_Init, 0},
	{ 135, S8259A_Init, 0},
	{ 140, Mapper140_Init, 0},
	{ 144, AGCI50282_Init, 0},
	{ 150, BIC62_Init, 0},
	{ 151, Mapper151_Init, 0},
	{ 152, Mapper152_Init, 0},
	{ 153, Mapper153_Init, 0},
	{ 154, Mapper154_Init, 0},
	{ 155, Mapper155_Init, 0},
	{ 156, Mapper156_Init, 0},
	{ 157, Mapper157_Init, 0},
	{ 158, Mapper158_Init, 0},
        { 160, Mapper90_Init, 0},
	{ 163, Mapper163_Init, 0 },
	{ 180, Mapper180_Init, 0},
	{ 182, Mapper182_Init, 0},
	{ 184, Mapper184_Init, 0},
	{ 185, Mapper185_Init, 0},
        { 187, Mapper187_Init, 0},
	{ 189, Mapper189_Init, 0},
	{ 193, Mapper193_Init, 0},
	{ 206, Mapper206_Init, 0},
	{ 207, Mapper207_Init, 0},
        { 208, Mapper208_Init, 0},
        { 209, Mapper209_Init, 0},
	{ 210, Mapper210_Init, 0},
	{ 215, Mapper215_Init, 0},
	{ 217, Mapper217_Init, 0},
	{ 222, Mapper222_Init, 0},
	{ 228, Mapper228_Init, 0},
	{ 232, BIC48_Init, 0},
	{ 234, Mapper234_Init, 0},
        { 240, Mapper240_Init, 0},
	{ 241, Mapper241_Init, 0},
	{ 242, Mapper242_Init, 0},
	{ 243, S74LS374N_Init, 0},
        { 244, Mapper244_Init, 0},
        { 245, Mapper245_Init, 0},
	{ 246, Mapper246_Init, 0},
	{ 248, Mapper248_Init, 0},
        { 249, Mapper249_Init, 0},
        { 250, Mapper250_Init, 0},
	{ 0, NULL, 0}
};

static int NewiNES_Init(int num)
{
 const BMAPPING *tmp=bmap;

 while(tmp->init)
 {
  if(num == tmp->number)
  {
   if(!VROM_size)
   {
    if(tmp->flags & BMAPF_32KCHRR)
     CHRRAMSize = 32768;
    else
     CHRRAMSize = 8192;

    if(!(VROM = (uint8 *)MDFN_malloc(CHRRAMSize, _("CHR RAM"))))
     return(0);

    SetupCartCHRMapping(0x0,VROM,CHRRAMSize,1);

    #ifdef WANT_DEBUGGER
    MDFNDBG_AddASpace(iNES_GetAddressSpaceBytes, iNES_PutAddressSpaceBytes, "chrram", "CHR RAM", (unsigned int)(log(CHRRAMSize) / log(2)));
    #endif
   }

   if(iNESCart.battery && (tmp->flags & BMAPF_INESWRAMOK))
   {
    if(!(WRAM = (uint8 *)MDFN_malloc(8192, _("WRAM"))))
     return(0);

    memset(WRAM, 0x00, 8192);

    SetupCartPRGMapping(0x10, WRAM, 8192, 1);
    SetReadHandler(0x6000, 0x7FFF, CartBR);
    SetWriteHandler(0x6000, 0x7FFF, CartBW);
    iNESCart.SaveGame[0] = WRAM;
    iNESCart.SaveGameLen[0] = 8192;

    #ifdef WANT_DEBUGGER
    MDFNDBG_AddASpace(iNES_GetAddressSpaceBytes, iNES_PutAddressSpaceBytes, "wram", "WRAM", 13);
    #endif
   }

   tmp->init(&iNESCart);

   return(1);
  }
  tmp++;
 }

 MDFN_PrintError(_("iNES mapper %d is not supported!"), num);
 return(0);
}
