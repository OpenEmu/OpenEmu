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

#include <string.h>
#include <math.h>

#include "nes.h"
#include "x6502.h"
#include "sound.h"
#include "nsf.h"
#include "nsfe.h"
#include "memory.h"
#include "fds.h"
#include "cart.h"
#include "input.h"
#include "../player.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint8 SongReload;

static DECLFW(NSF_write);
static DECLFR(NSF_read);

static NSFINFO *NSFInfo;
typedef std::vector<writefunc> NSFWriteEntry;
static NSFWriteEntry *WriteHandlers = NULL; //[0x10000];

static uint8 NSFROM[0x30+6]=
{
/* 0x00 - NMI */
0x8D,0xF4,0x3F,                         /* Stop play routine NMIs. */
0xA2,0xFF,0x9A,                         /* Initialize the stack pointer. */
0xAD,0xF0,0x3F,                         /* See if we need to init. */
0xF0,0x09,                              /* If 0, go to play routine playing. */

0xAD,0xF1,0x3F,                         /* Confirm and load A      */
0xAE,0xF3,0x3F,                         /* Load X with PAL/NTSC byte */

0x20,0x00,0x00,                         /* JSR to init routine     */

0xA9,0x00,
0xAA,
0xA8,
0x20,0x00,0x00,                         /* JSR to play routine  */
0x8D,0xF5,0x3F,				/* Start play routine NMIs. */
0x90,0xFE,                               /* Loopie time. */

/* 0x20 */
0x8D,0xF3,0x3F,				/* Init init NMIs */
0x18,
0x90,0xFE				/* Loopie time. */
};

static DECLFR(NSFROMRead)
{
 return (NSFROM-0x3800)[A];
}

static int doreset=0;
static int NSFNMIFlags;
static uint8 BSon;

static uint8 *ExWRAM=0;

static void FreeNSF(void)
{
 if(NSFInfo)
 {
  if(NSFInfo->GameName) free(NSFInfo->GameName);
  if(NSFInfo->Artist) free(NSFInfo->Artist);
  if(NSFInfo->Copyright) free(NSFInfo->Copyright);
  if(NSFInfo->Ripper) free(NSFInfo->Ripper);
  if(NSFInfo->NSFDATA) free(NSFInfo->NSFDATA);
  if(ExWRAM) { free(ExWRAM); ExWRAM = NULL; }
  free(NSFInfo);
  NSFInfo = NULL;
 }

 if(WriteHandlers)
 {
  delete[] WriteHandlers;
  WriteHandlers = NULL;
 }
}

static void NSF_Close(void)
{
 FreeNSF();
}

static void NSF_Reset(void)
{
 NSF_init();
}

static void NSF_Power(void)
{
 NSF_init();
}

// First 32KB is reserved for sound chip emulation in the iNES mapper code.

static INLINE void BANKSET(uint32 A, uint32 bank)
{
 bank &= NSFInfo->NSFMaxBank;
 if(NSFInfo->SoundChip&4)
  memcpy(ExWRAM+(A-0x6000),NSFInfo->NSFDATA+(bank<<12),4096);
 else 
  setprg4(A,bank);
}

int LoadNSF(MDFNFILE *fp)
{
 NSF_HEADER NSFHeader;
 MDFN_fseek(fp, 0, SEEK_SET);
 MDFN_fread(&NSFHeader, 1, 0x80, fp);

 // NULL-terminate strings just in case.
 NSFHeader.GameName[31] = NSFHeader.Artist[31] = NSFHeader.Copyright[31] = 0;

 NSFInfo->GameName = (UTF8*)MDFN_FixString(strdup((char *)NSFHeader.GameName));
 NSFInfo->Artist = (UTF8 *)MDFN_FixString(strdup((char *)NSFHeader.Artist));
 NSFInfo->Copyright = (UTF8 *)MDFN_FixString(strdup((char *)NSFHeader.Copyright));

 MDFN_trim((char*)NSFInfo->GameName);
 MDFN_trim((char*)NSFInfo->Artist);
 MDFN_trim((char*)NSFInfo->Copyright);

 NSFInfo->LoadAddr = NSFHeader.LoadAddressLow | (NSFHeader.LoadAddressHigh << 8);
 NSFInfo->InitAddr = NSFHeader.InitAddressLow | (NSFHeader.InitAddressHigh << 8);
 NSFInfo->PlayAddr = NSFHeader.PlayAddressLow | (NSFHeader.PlayAddressHigh << 8);

 NSFInfo->NSFSize = MDFN_fgetsize(fp)-0x80;

 NSFInfo->NSFMaxBank = ((NSFInfo->NSFSize+(NSFInfo->LoadAddr&0xfff)+4095)/4096);
 NSFInfo->NSFMaxBank = uppow2(NSFInfo->NSFMaxBank);

 if(!(NSFInfo->NSFDATA=(uint8 *)MDFN_malloc(NSFInfo->NSFMaxBank*4096, _("NSF data"))))
  return 0;

 MDFN_fseek(fp,0x80,SEEK_SET);
 memset(NSFInfo->NSFDATA, 0x00, NSFInfo->NSFMaxBank*4096);
 MDFN_fread(NSFInfo->NSFDATA+(NSFInfo->LoadAddr&0xfff), 1, NSFInfo->NSFSize, fp);
 
 NSFInfo->NSFMaxBank--;

 NSFInfo->VideoSystem = NSFHeader.VideoSystem;
 NSFInfo->SoundChip = NSFHeader.SoundChip;
 NSFInfo->TotalSongs = NSFHeader.TotalSongs;

 if(NSFHeader.StartingSong == 0)
  NSFHeader.StartingSong = 1;

 NSFInfo->StartingSong = NSFHeader.StartingSong - 1;
 memcpy(NSFInfo->BankSwitch, NSFHeader.BankSwitch, 8);

 return(1);
}


bool NSF_TestMagic(const char *name, MDFNFILE *fp)
{
 if(fp->size < 5)
  return(FALSE);

 if(memcmp(fp->data, "NESM\x1a", 5) && memcmp(fp->data, "NSFE", 4))
  return(FALSE);

 return(TRUE);
} 

bool NSFLoad(const char *name, MDFNFILE *fp, NESGameType *gt)
{
 char magic[5];
 int x;

 if(!(NSFInfo = (NSFINFO *)MDFN_malloc(sizeof(NSFINFO), _("NSF header"))))
 {
  return(0);
 }
 memset(NSFInfo, 0, sizeof(NSFINFO));

 MDFN_fseek(fp,0,SEEK_SET);
 MDFN_fread(magic, 1, 5, fp);

 if(!memcmp(magic, "NESM\x1a", 5))
 {
  if(!LoadNSF(fp))
  {
   FreeNSF();
   return(0);
  }
 }
 else if(!memcmp(magic, "NSFE", 4))
 {
  if(!LoadNSFE(NSFInfo, fp->data, MDFN_fgetsize(fp),0))
  {
   FreeNSF();
   return(0);
  }
 }
 else
 {
  FreeNSF();
  return(FALSE);
 }

 if(NSFInfo->LoadAddr < 0x6000)
 {
  MDFNI_printf(_("Load address is invalid!"));
  FreeNSF();
  return(0);
 }

 if(NSFInfo->TotalSongs < 1)
 {
  MDFNI_printf(_("Total number of songs is less than 1!"));
  FreeNSF();
  return(0);
 }

 BSon = 0;
 for(x=0;x<8;x++)
  BSon |= NSFInfo->BankSwitch[x];

 MDFNGameInfo->GameType = GMT_PLAYER;

 if(NSFInfo->GameName)
  MDFNGameInfo->name = (UTF8*)strdup((char*)NSFInfo->GameName);

 for(x=0;;x++)
 {
  if(NSFROM[x]==0x20)
  {
   NSFROM[x+1]=NSFInfo->InitAddr&0xFF;
   NSFROM[x+2]=NSFInfo->InitAddr>>8;
   NSFROM[x+8]=NSFInfo->PlayAddr&0xFF;
   NSFROM[x+9]=NSFInfo->PlayAddr>>8;
   break;
  }
 }

 if(NSFInfo->VideoSystem == 0)
  MDFNGameInfo->VideoSystem = VIDSYS_NTSC;
 else if(NSFInfo->VideoSystem == 1)
  MDFNGameInfo->VideoSystem = VIDSYS_PAL;

 MDFN_printf(_("NSF Loaded.  File information:\n\n"));
 MDFN_indent(1);
 if(NSFInfo->GameName)
  MDFN_printf(_("Game/Album Name:\t%s\n"), NSFInfo->GameName);
 if(NSFInfo->Artist)
  MDFN_printf(_("Music Artist:\t%s\n"), NSFInfo->Artist);
 if(NSFInfo->Copyright)
  MDFN_printf(_("Copyright:\t\t%s\n"), NSFInfo->Copyright);
 if(NSFInfo->Ripper)
  MDFN_printf(_("Ripper:\t\t%s\n"), NSFInfo->Ripper);

 if(NSFInfo->SoundChip)
 {
  static const char *tab[6]={"Konami VRCVI","Konami VRCVII","Nintendo FDS","Nintendo MMC5","Namco 106","Sunsoft FME-07"};

  for(x=0;x<6;x++)
   if(NSFInfo->SoundChip&(1<<x))
   {
    MDFN_printf(_("Expansion hardware:  %s\n"), tab[x]);
    //NSFInfo->SoundChip=1<<x;	/* Prevent confusing weirdness if more than one bit is set. */
    //break;
   }
 }
 if(BSon)
  MDFN_printf(_("Bank-switched\n"));
 MDFN_printf(_("Load address:  $%04x\nInit address:  $%04x\nPlay address:  $%04x\n"),NSFInfo->LoadAddr,NSFInfo->InitAddr,NSFInfo->PlayAddr);
 MDFN_printf("%s\n",(NSFInfo->VideoSystem&1)?"PAL":"NTSC");
 MDFN_printf(_("Starting song:  %d / %d\n\n"),NSFInfo->StartingSong + 1,NSFInfo->TotalSongs);

 if(NSFInfo->SoundChip&4)
  ExWRAM=(uint8 *)MDFN_malloc(32768+8192, _("NSF expansion RAM"));
 else
  ExWRAM=(uint8 *)MDFN_malloc(8192, _("NSF expansion RAM"));

 MDFN_indent(-1);

 if(!ExWRAM)
 {
  return(0);
 }


 gt->Power = NSF_Power;
 gt->Reset = NSF_Reset;
 gt->Close = NSF_Close;

 Player_Init(NSFInfo->TotalSongs, NSFInfo->GameName, NSFInfo->Artist, NSFInfo->Copyright, NSFInfo->SongNames);
 return 1;
}

static DECLFR(NSFVectorRead)
{
 if(((NSFNMIFlags&1) && SongReload) || (NSFNMIFlags&2) || doreset)
 {
  if(A==0xFFFA) return(0x00);
  else if(A==0xFFFB) return(0x38);
  else if(A==0xFFFC) return(0x20);
  else if(A==0xFFFD) {doreset=0;return(0x38);}
  return(X.DB);
 }
 else
  return(CartBR(A));
}

void NSFECSetWriteHandler(int32 start, int32 end, writefunc func)
{
  int32 x;

  if(!func) return;

  for(x=end;x>=start;x--)
   WriteHandlers[x].push_back(func);
}

static DECLFW(NSFECWriteHandler)
{
 for(unsigned int x = 0; x < WriteHandlers[A].size(); x++)
  WriteHandlers[A][x](A, V);

}

int NSFFDS_Init(EXPSOUND *, bool MultiChip);
int NSFVRC6_Init(EXPSOUND *, bool MultiChip);
int NSFMMC5_Init(EXPSOUND *, bool MultiChip);
int NSFAY_Init(EXPSOUND *, bool MultiChip);
int NSFN106_Init(EXPSOUND *, bool MultiChip);
int NSFVRC7_Init(EXPSOUND *, bool MultiChip);

void NSF_init(void)
{
  doreset = 1;

  WriteHandlers = new NSFWriteEntry[0x10000];

  ResetCartMapping();

  SetWriteHandler(0x2000, 0x3FFF, NSFECWriteHandler);
  SetWriteHandler(0x4020, 0xFFFF, NSFECWriteHandler);

  if(NSFInfo->SoundChip&4)
  {
   SetupCartPRGMapping(0,ExWRAM,32768+8192,1);
   setprg32(0x6000,0);
   setprg8(0xE000,4);
   memset(ExWRAM,0x00,32768+8192);
   NSFECSetWriteHandler(0x6000,0xDFFF,CartBW);
   SetReadHandler(0x6000,0xFFFF,CartBR);
  }
  else
  {
   memset(ExWRAM,0x00,8192);
   SetReadHandler(0x6000,0x7FFF,CartBR);
   NSFECSetWriteHandler(0x6000,0x7FFF,CartBW);
   SetupCartPRGMapping(0,NSFInfo->NSFDATA,((NSFInfo->NSFMaxBank+1)*4096),0);
   SetupCartPRGMapping(1,ExWRAM,8192,1);
   setprg8r(1,0x6000,0);
   SetReadHandler(0x8000,0xFFFF,CartBR);
  }

  if(BSon)
  {
   int32 x;
   for(x=0;x<8;x++)
   {
    if(NSFInfo->SoundChip&4 && x>=6)
     BANKSET(0x6000+(x-6)*4096,NSFInfo->BankSwitch[x]);
    BANKSET(0x8000+x*4096,NSFInfo->BankSwitch[x]);
   }
  }
  else
  {
   int32 x;
   for(x=(NSFInfo->LoadAddr&0xF000);x<0x10000;x+=0x1000)
    BANKSET(x,((x-(NSFInfo->LoadAddr&0xf000))>>12));
  }

  SetReadHandler(0xFFFA,0xFFFD,NSFVectorRead);

  NSFECSetWriteHandler(0x2000,0x3fff,0);
  SetReadHandler(0x2000,0x37ff,0);
  SetReadHandler(0x3836,0x3FFF,0);
  SetReadHandler(0x3800,0x3835,NSFROMRead);

  NSFECSetWriteHandler(0x5ff6,0x5fff,NSF_write);

  NSFECSetWriteHandler(0x3ff0,0x3fff,NSF_write);
  SetReadHandler(0x3ff0,0x3fff,NSF_read);

  int (*InitPointers[8])(EXPSOUND *, bool MultiChip) = { NSFVRC6_Init, NSFVRC7_Init, NSFFDS_Init, NSFMMC5_Init, NSFN106_Init, NSFAY_Init, NULL, NULL };

  for(int x = 0; x < 8; x++)
   if((NSFInfo->SoundChip & (1 << x)) && InitPointers[x])
   {
    EXPSOUND TmpExpSound;
    memset(&TmpExpSound, 0, sizeof(TmpExpSound));

    InitPointers[x](&TmpExpSound, NSFInfo->SoundChip != (1 << x));
    GameExpSound.push_back(TmpExpSound);
   }

  NSFInfo->CurrentSong=NSFInfo->StartingSong;
  SongReload=0xFF;
  NSFNMIFlags=0;
}

static DECLFW(NSF_write)
{
 switch(A)
 {
  case 0x3FF3:NSFNMIFlags|=1;break;
  case 0x3FF4:NSFNMIFlags&=~2;break;
  case 0x3FF5:NSFNMIFlags|=2;break;

  case 0x5FF6:
  case 0x5FF7:if(!(NSFInfo->SoundChip&4)) return;
  case 0x5FF8:
  case 0x5FF9:
  case 0x5FFA:
  case 0x5FFB:
  case 0x5FFC:
  case 0x5FFD:
  case 0x5FFE:
  case 0x5FFF:if(!BSon) return;
              A&=0xF;
              BANKSET((A*4096),V);
  	      break;
 } 
}

static DECLFR(NSF_read)
{
 int x;

 switch(A)
 {
 case 0x3ff0:x=SongReload;
	     if(!fceuindbg)
	      SongReload=0;
	     return x;
 case 0x3ff1:
	    if(!fceuindbg)
	    {
	     extern uint8 RAM[0x800]; // from nes.cpp

             memset(RAM,0x00,0x800);

             BWrite[0x4015](0x4015,0x0);
             for(x=0;x<0x14;x++)
              BWrite[0x4000+x](0x4000+x,0);
             BWrite[0x4015](0x4015,0xF);

	     if(NSFInfo->SoundChip&4) 
	     {
	      BWrite[0x4017](0x4017,0xC0);	/* FDS BIOS writes $C0 */
	      BWrite[0x4089](0x4089,0x80);
	      BWrite[0x408A](0x408A,0xE8);
	     }
	     else 
	     {
	      memset(ExWRAM,0x00,8192);
	      BWrite[0x4017](0x4017,0xC0);
              BWrite[0x4017](0x4017,0xC0);
              BWrite[0x4017](0x4017,0x40);
	     }

             if(BSon)
             {
              for(x=0;x<8;x++)
	       BANKSET(0x8000+x*4096,NSFInfo->BankSwitch[x]);
             }
             return (NSFInfo->CurrentSong);
 	     }
 case 0x3FF3:return PAL;
 }
 return 0;
}

uint8 MDFN_GetJoyJoy(void);

void DoNSFFrame(void)
{
 if(((NSFNMIFlags&1) && SongReload) || (NSFNMIFlags&2))
  TriggerNMI();

 {
  static uint8 last=0;
  uint8 tmp;
  tmp=MDFN_GetJoyJoy();
  if((tmp&JOY_RIGHT) && !(last&JOY_RIGHT))
  {
   if(NSFInfo->CurrentSong<(NSFInfo->TotalSongs - 1))
   {
    NSFInfo->CurrentSong++;
    SongReload=0xFF;
   }
  }
  else if((tmp&JOY_LEFT) && !(last&JOY_LEFT))
  {
   if(NSFInfo->CurrentSong>0)
   {
    NSFInfo->CurrentSong--;
    SongReload=0xFF;
   }
  }
  else if((tmp&JOY_UP) && !(last&JOY_UP))
  {
   NSFInfo->CurrentSong+=10;
   if(NSFInfo->CurrentSong>=NSFInfo->TotalSongs) NSFInfo->CurrentSong=NSFInfo->TotalSongs - 1;
   SongReload=0xFF;
  }
  else if((tmp&JOY_DOWN) && !(last&JOY_DOWN))
  {
   NSFInfo->CurrentSong-=10;
   if(NSFInfo->CurrentSong<0) NSFInfo->CurrentSong=0;
   SongReload=0xFF;
  }
  else if((tmp&JOY_START) && !(last&JOY_START))
   SongReload=0xFF;
  else if((tmp&JOY_A) && !(last&JOY_A))
  {

  }
  last=tmp;
 }
}

void MDFNNES_DrawNSF(uint32 *XBuf, int16 *samples, int32 scount)
{
 Player_Draw(XBuf, NSFInfo->CurrentSong, samples, scount);
}

