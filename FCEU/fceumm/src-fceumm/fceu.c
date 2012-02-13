/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
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
 
#include  <string.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <stdarg.h>

#include  "types.h"
#include  "x6502.h"
#include  "fceu.h"
#include  "ppu.h"
#include  "sound.h"
#include  "general.h"
#include  "myendian.h"
#include  "memory.h"

#include  "cart.h"
#include  "fds.h"
#include  "ines.h"
#include  "unif.h"
#include  "cheat.h"
#include  "palette.h"
#include  "state.h"
#include  "input.h"
#include  "file.h"
#include  "crc32.h"
#include  "vsuni.h"

uint64 timestampbase;


FCEUGI *FCEUGameInfo = NULL;
void (*GameInterface)(int h);

void (*GameStateRestore)(int version);

readfunc ARead[0x10000];
writefunc BWrite[0x10000];
static readfunc *AReadG;
static writefunc *BWriteG;
static int RWWrap=0;

static DECLFW(BNull)
{

}

static DECLFR(ANull)
{
	return(X.DB);
}

int AllocGenieRW(void)
{
	if(!(AReadG=(readfunc *)FCEU_malloc(0x8000*sizeof(readfunc))))
		return 0;
	if(!(BWriteG=(writefunc *)FCEU_malloc(0x8000*sizeof(writefunc))))
		return 0;
	RWWrap=1;
	return 1;
}

void FlushGenieRW(void)
{
	int32 x;

	if(RWWrap)
	{
		for(x=0;x<0x8000;x++)
		{
			ARead[x+0x8000]=AReadG[x];
			BWrite[x+0x8000]=BWriteG[x];
		}
		free(AReadG);
		free(BWriteG);
		AReadG=0;
		BWriteG=0;
		RWWrap=0;
	}
}

readfunc GetReadHandler(int32 a)
{
	if(a>=0x8000 && RWWrap)
		return AReadG[a-0x8000];
	else
		return ARead[a];
}

void SetReadHandler(int32 start, int32 end, readfunc func)
{
	int32 x;

	if(!func)
		func=ANull;

	if(RWWrap)
		for(x=end;x>=start;x--)
		{
			if(x>=0x8000)
				AReadG[x-0x8000]=func;
			else
				ARead[x]=func;
		}
	else

		for(x=end;x>=start;x--)
			ARead[x]=func;
}

writefunc GetWriteHandler(int32 a)
{
  if(RWWrap && a>=0x8000)
   return BWriteG[a-0x8000];
  else
   return BWrite[a];
}

void SetWriteHandler(int32 start, int32 end, writefunc func)
{
	int32 x;

	if(!func)
		func=BNull;

	if(RWWrap)
		for(x=end;x>=start;x--)
		{
			if(x>=0x8000)
				BWriteG[x-0x8000]=func;
			else
				BWrite[x]=func;
		}
	else
		for(x=end;x>=start;x--)
			BWrite[x]=func;
}

uint8 GameMemBlock[131072];

#ifdef COPYFAMI
uint8 RAM[0x4000];
#else
uint8 RAM[0x800];
#endif

uint8 PAL=0;

static DECLFW(BRAML)
{
  RAM[A]=V;
}

static DECLFR(ARAML)
{
  return RAM[A];
}

#ifndef COPYFAMI
static DECLFW(BRAMH)
{
  RAM[A&0x7FF]=V;
}

static DECLFR(ARAMH)
{
  return RAM[A&0x7FF];
}
#endif

static void CloseGame(void)
{
	if(FCEUGameInfo)
	{
		if(FCEUGameInfo->name)
		{
			free(FCEUGameInfo->name);
			FCEUGameInfo->name=0;
		}
		FCEU_FlushGameCheats(0,0);
		GameInterface(GI_CLOSE);
		ResetExState(0,0);
		CloseGenie();
		free(FCEUGameInfo);
		FCEUGameInfo = 0;
	}
}

extern uint8 pale;

void ResetGameLoaded(void)
{
	if(FCEUGameInfo)
		CloseGame();
	GameStateRestore=0;
	PPU_hook=0;
	GameHBIRQHook=0;
	if(GameExpSound.Kill)
		GameExpSound.Kill();
	memset(&GameExpSound,0,sizeof(GameExpSound));
	MapIRQHook=0;
	MMC5Hack=0;
	PAL&=1;
	pale=0;
}

int UNIFLoad(const char *name, FCEUFILE *fp);
int iNESLoad(const char *name, FCEUFILE *fp);
int FDSLoad(const char *name, FCEUFILE *fp);

FCEUGI *FCEUI_LoadGame(const char *name)
{
	char *ipsfn;
	FCEUFILE *fp;

	ResetGameLoaded();

	FCEUGameInfo = malloc(sizeof(FCEUGI));
	memset(FCEUGameInfo, 0, sizeof(FCEUGI));

	FCEUGameInfo->soundchan = 0;
	FCEUGameInfo->soundrate = 0;
	FCEUGameInfo->name=0;
	FCEUGameInfo->type=GIT_CART;
	FCEUGameInfo->vidsys=GIV_USER;
	FCEUGameInfo->input[0]=FCEUGameInfo->input[1]=-1;
	FCEUGameInfo->inputfc=-1;
	FCEUGameInfo->cspecial=0;

#ifdef FCEU_LOG
	FCEU_printf("Loading %s...\n\n",name);
#endif

	GetFileBase(name);

	ipsfn=FCEU_MakeFName(FCEUMKF_IPS,0,0);
	fp=FCEU_fopen(name,ipsfn,"rb",0);
	free(ipsfn);

	if(!fp)
	{
		return 0;	/*Error opening ROM*/
	}

	if(iNESLoad(name,fp))
		goto endlseq;
	if(UNIFLoad(name,fp))
		goto endlseq;
	if(FDSLoad(name,fp))
		goto endlseq;

	FCEU_fclose(fp);	/*An error occcurred while loading the file*/
	return 0;

endlseq:
	FCEU_fclose(fp);

	FCEU_ResetVidSys();
	if(FSettings.GameGenie)
		OpenGenie();

	PowerNES();

	FCEU_LoadGamePalette();
	/*FCEU_LoadGameCheats(0);*/

	FCEU_ResetPalette();

	return(FCEUGameInfo);
}

int CopyFamiLoad(void);

FCEUGI *FCEUI_CopyFamiStart(void)
{
	ResetGameLoaded();

	FCEUGameInfo = malloc(sizeof(FCEUGI));
	memset(FCEUGameInfo, 0, sizeof(FCEUGI));

	FCEUGameInfo->soundchan = 0;
	FCEUGameInfo->soundrate = 0;
	FCEUGameInfo->name="copyfami";
	FCEUGameInfo->type=GIT_CART;
	FCEUGameInfo->vidsys=GIV_USER;
	FCEUGameInfo->input[0]=FCEUGameInfo->input[1]=-1;
	FCEUGameInfo->inputfc=-1;
	FCEUGameInfo->cspecial=0;

#ifdef FCEU_LOG
	FCEU_printf("Starting CopyFamicom...\n\n");
#endif

	if(!CopyFamiLoad())
		return 0;	/*An error occurred while starting CopyFamicom*/

	FCEU_ResetVidSys();
	if(FSettings.GameGenie)
		OpenGenie();

	PowerNES();

	FCEU_LoadGamePalette();
	FCEU_LoadGameCheats(0);

	FCEU_ResetPalette();

	return(FCEUGameInfo);
}

int FCEUI_Initialize(void)
{
	if(!FCEU_InitVirtualVideo())
		return 0;
	memset(&FSettings,0,sizeof(FSettings));
	FSettings.UsrFirstSLine[0]=8;
	FSettings.UsrFirstSLine[1]=0;
	FSettings.UsrLastSLine[0]=231;
	FSettings.UsrLastSLine[1]=239;
	FSettings.SoundVolume=100;
	FCEUPPU_Init();
	X6502_Init();
	return 1;
}

void FCEUI_Emulate(uint8 **pXBuf, int32 **SoundBuf, int32 *SoundBufSize)
{
	int ssize;

	FCEU_UpdateInput();

	if(geniestage!=1)
		FCEU_ApplyPeriodicCheats();

	FCEUPPU_Loop();

	if(timestamp)
		ssize = FlushEmulateSound();
	else
		ssize = 0;

	timestampbase += timestamp;

	timestamp = 0;

	*pXBuf= XBuf;
	*SoundBuf=WaveFinal;
	*SoundBufSize=ssize;
}

void FCEUI_CloseGame(void)
{
	CloseGame();
}

void ResetNES(void)
{
	if(!FCEUGameInfo)
		return;
	GameInterface(GI_RESETM2);
	FCEUSND_Reset();
	FCEUPPU_Reset();
	X6502_Reset();
}

static void FCEU_MemoryRand(uint8 *ptr, uint32 size)
{
	int x=0;
	while(size)
	{
		*ptr=(x&4)?0x7F:0x00;
		x++;
		size--;
		ptr++;
	}
}

void PowerNES(void)
{
	if(!FCEUGameInfo)
		return;

	FCEU_CheatResetRAM();
	FCEU_CheatAddRAM(2,0,RAM);

	GeniePower();

#ifndef COPYFAMI
	FCEU_MemoryRand(RAM,0x800);
#endif

	SetReadHandler(0x0000,0xFFFF,ANull);
	SetWriteHandler(0x0000,0xFFFF,BNull);

#ifdef COPYFAMI
	SetReadHandler(0,0x3FFF,ARAML);
	SetWriteHandler(0,0x3FFF,BRAML);
#else
	SetReadHandler(0,0x7FF,ARAML);
	SetWriteHandler(0,0x7FF,BRAML);

	SetReadHandler(0x800,0x1FFF,ARAMH);  /* Part of a little */
	SetWriteHandler(0x800,0x1FFF,BRAMH); /* hack for a small speed boost. */
#endif
	InitializeInput();
	FCEUSND_Power();
	FCEUPPU_Power();

	/* Have the external game hardware "powered" after the internal NES stuff.
	   Needed for the NSF code and VS System code.
	 */
	GameInterface(GI_POWER);
	if(FCEUGameInfo->type==GIT_VSUNI)
		FCEU_VSUniPower();


	timestampbase=0;
	X6502_Power();
	FCEU_PowerCheats();
}

void FCEU_ResetVidSys(void)
{
	int w;

	if(FCEUGameInfo->vidsys==GIV_NTSC)
		w=0;
	else if(FCEUGameInfo->vidsys==GIV_PAL)
		w=1;
	else
		w=FSettings.PAL;

	PAL=w?1:0;
	FCEUPPU_SetVideoSystem(w);
	SetSoundVariables();
}

FCEUS FSettings;

void FCEU_printf(char *format, ...)
{
	FILE *ofile;
	char temp[2048];

	va_list ap;

	va_start(ap,format);
	vsprintf(temp,format,ap);
	FCEUD_Message(temp);

	ofile=fopen("stdout.txt","ab");
	fwrite(temp,1,strlen(temp),ofile);
	fclose(ofile);

	va_end(ap);
}

void FCEU_PrintError(char *format, ...)
{
	char temp[2048];

	va_list ap;

	va_start(ap,format);
	vsprintf(temp,format,ap);
	FCEUD_PrintError(temp);

	va_end(ap);
}

void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall)
{
	FSettings.UsrFirstSLine[0]=ntscf;
	FSettings.UsrLastSLine[0]=ntscl;
	FSettings.UsrFirstSLine[1]=palf;
	FSettings.UsrLastSLine[1]=pall;
	if(PAL)
	{
		FSettings.FirstSLine=FSettings.UsrFirstSLine[1];
		FSettings.LastSLine=FSettings.UsrLastSLine[1];
	}
	else
	{
		FSettings.FirstSLine=FSettings.UsrFirstSLine[0];
		FSettings.LastSLine=FSettings.UsrLastSLine[0];
	}

}

void FCEUI_SetVidSystem(int a)
{
	FSettings.PAL=a?1:0;
	if(FCEUGameInfo)
	{
		FCEU_ResetVidSys();
		FCEU_ResetPalette();
	}
}

int FCEUI_GetCurrentVidSystem(int *slstart, int *slend)
{
	if(slstart)
		*slstart=FSettings.FirstSLine;
	if(slend)
		*slend=FSettings.LastSLine;
	return(PAL);
}

void FCEUI_SetGameGenie(int a)
{
	FSettings.GameGenie=a?1:0;
}
