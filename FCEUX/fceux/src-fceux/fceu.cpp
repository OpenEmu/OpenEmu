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

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <fstream>
#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "ppu.h"
#include "sound.h"
#include "file.h"
#include "utils/endian.h"
#include "utils/memory.h"
#include "utils/crc32.h"

#include "cart.h"
#include "fds.h"
#include "ines.h"
#include "unif.h"
#include "cheat.h"
#include "palette.h"
#include "state.h"
#include "video.h"
#include "input.h"
#include "file.h"
#include "vsuni.h"
#include "ines.h"

#include "driver.h"

using namespace std;

FCEUGI::FCEUGI() : filename(0), archiveFilename(0)
{
	//printf("%08x",opsize);
}

FCEUGI::~FCEUGI()
{
	if(filename)
		delete filename;
	if(archiveFilename)
		delete archiveFilename;
}

bool CheckFileExists(const char* filename)
{
	//This function simply checks to see if the given filename exists
	if (!filename)
		return false;
	fstream test;
	test.open(filename,fstream::in);
		
	if (test.fail())
	{
		test.close();
		return false; 
	}
	else
	{
		test.close();
		return true; 
	}
}

void FCEU_TogglePPU(void)
{
	newppu ^= 1;
	if (newppu)
	{
		FCEU_DispMessage("New PPU loaded", 0);
		FCEUI_printf("New PPU loaded");
	}
	else
	{
		FCEU_DispMessage("Old PPU loaded", 0);
		FCEUI_printf("Old PPU loaded");
	}
}

static void FCEU_CloseGame(void)
{
	if(GameInfo)
	{

		if(GameInfo->name)
		{
			free(GameInfo->name);
			GameInfo->name=0;
		}

		FCEU_FlushGameCheats(0,0);

		GameInterface(GI_CLOSE);

		ResetExState(0,0);

		//clear screen when game is closed
		extern uint8 *XBuf;
		if(XBuf)
			memset(XBuf,0,256*256);

		CloseGenie();

		delete GameInfo;
		GameInfo = 0;
	}
}


uint64 timestampbase;


FCEUGI *GameInfo = 0;

void (*GameInterface)(GI h);
void (*GameStateRestore)(int version);

readfunc ARead[0x10000];
writefunc BWrite[0x10000];
static readfunc *AReadG;
static writefunc *BWriteG;
static int RWWrap=0;


static int *AutosaveStatus; //is it safe to load Auto-savestate
static int AutosaveIndex = 0; //which Auto-savestate we're on
int AutosaveQty = 4; // Number of Autosaves to store

static DECLFW(BNull)
{

}

static DECLFR(ANull)
{
	return(X.DB);
}

int AllocGenieRW(void)
{
	if(!(AReadG=(readfunc *)malloc(0x8000*sizeof(readfunc))))
		return 0;
	if(!(BWriteG=(writefunc *)malloc(0x8000*sizeof(writefunc))))
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

uint8 *GameMemBlock;
uint8 *RAM;

//---------
//windows might need to allocate these differently, so we have some special code

static void AllocBuffers()
{
	GameMemBlock = (uint8*)realloc(GameMemBlock, GAME_MEM_BLOCK_SIZE);
	RAM = (uint8*)realloc(RAM, 0x800);
}

static void FreeBuffers()
{
	free(GameMemBlock);
	free(RAM);
}
//------

uint8 PAL=0;

static DECLFW(BRAML)
{
	RAM[A]=V;
}

static DECLFW(BRAMH)
{
	RAM[A&0x7FF]=V;
}

static DECLFR(ARAML)
{
	return RAM[A];
}

static DECLFR(ARAMH)
{
	return RAM[A&0x7FF];
}


void ResetGameLoaded(void)
{
	if(GameInfo)
		FCEU_CloseGame();
	GameStateRestore=0;
	PPU_hook=0;
	GameHBIRQHook=0;
	FFCEUX_PPURead = 0;
	FFCEUX_PPUWrite = 0;
	if(GameExpSound.Kill)
		GameExpSound.Kill();
	memset(&GameExpSound,0,sizeof(GameExpSound));
	MapIRQHook=0;
	MMC5Hack=0;
	PAL&=1;
	pale=0;
}

int UNIFLoad(const char *name, FCEUFILE *fp);
int iNESLoad(const char *name, FCEUFILE *fp, int OverwriteVidMode);
int FDSLoad(const char *name, FCEUFILE *fp);

FCEUGI *FCEUI_LoadGameVirtual(const char *name, int OverwriteVidMode)
{
	//----------
	//attempt to open the files
	FCEUFILE *fp;

	//FCEU_printf("Loading %s...\n\n",name);

	const char* romextensions[] = {"nes","fds",0};
	fp=FCEU_fopen(name,0,"rb",0,-1,romextensions);
	if(!fp)
	{
		return 0;
	}

	GetFileBase(fp->filename.c_str());

	if(!fp)
	{
		FCEU_PrintError("Error opening \"%s\"!",name);
		return 0;
	}

	//file opened ok. start loading.

	ResetGameLoaded();

	if (!AutosaveStatus)
		AutosaveStatus = (int*)realloc(AutosaveStatus, sizeof(int)*AutosaveQty);
	for (AutosaveIndex=0; AutosaveIndex<AutosaveQty; ++AutosaveIndex)
		AutosaveStatus[AutosaveIndex] = 0;

	FCEU_CloseGame();
	GameInfo = new FCEUGI();
	memset(GameInfo, 0, sizeof(FCEUGI));

	GameInfo->filename = strdup(fp->filename.c_str());
	if(fp->archiveFilename != "") GameInfo->archiveFilename = strdup(fp->archiveFilename.c_str());
	GameInfo->archiveCount = fp->archiveCount;

#ifdef __LIBSNES__
	GameInfo->soundrate = 32050;
#else
	GameInfo->soundrate = 48200;
#endif
	GameInfo->name=0;
	GameInfo->type=GIT_CART;
	GameInfo->vidsys=GIV_USER;
	GameInfo->input[0]=GameInfo->input[1]=SI_UNSET;
	GameInfo->inputfc=SIFC_UNSET;
	GameInfo->cspecial=SIS_NONE;

	FCEUI_Sound(GameInfo->soundrate);

	//try to load each different format
	if(iNESLoad(name,fp,OverwriteVidMode))
		goto endlseq;
	if(UNIFLoad(name,fp))
		goto endlseq;
	if(FDSLoad(name,fp))
		goto endlseq;

	FCEU_PrintError("An error occurred while loading the file.");
	delete fp;

	delete GameInfo;
	GameInfo = 0;

	return 0;

endlseq:

	delete fp;

	FCEU_ResetVidSys();

	if(FSettings.GameGenie)
		OpenGenie();

	PowerNES();

	FCEU_LoadGamePalette();

	FCEU_ResetPalette();

	FCEU_LoadGameCheats(0);

	return GameInfo;
}

FCEUGI *FCEUI_LoadGame(const char *name, int OverwriteVidMode)
{
	return FCEUI_LoadGameVirtual(name,OverwriteVidMode);
}


//Return: Flag that indicates whether the function was succesful or not.
bool FCEUI_Initialize()
{
	srand(time(0));

	if(!FCEU_InitVirtualVideo())
		return false;

	AllocBuffers();

	// Initialize some parts of the settings structure
	//mbg 5/7/08 - I changed the ntsc settings to match pal.
	//this is more for precision emulation, instead of entertainment, which is what fceux is all about nowadays
	memset(&FSettings,0,sizeof(FSettings));
	//FSettings.UsrFirstSLine[0]=8;
	FSettings.UsrFirstSLine[0]=0;
	FSettings.UsrFirstSLine[1]=0;
	//FSettings.UsrLastSLine[0]=231;
	FSettings.UsrLastSLine[0]=239;
	FSettings.UsrLastSLine[1]=239;
	FSettings.SoundVolume=150;	//0-150 scale
	FSettings.TriangleVolume=256;	//0-256 scale (256 is max volume)
	FSettings.Square1Volume=256;	//0-256 scale (256 is max volume)
	FSettings.Square2Volume=256;	//0-256 scale (256 is max volume)
	FSettings.NoiseVolume=256;	//0-256 scale (256 is max volume)
	FSettings.PCMVolume=256;	//0-256 scale (256 is max volume)

	FCEUPPU_Init();

	X6502_Init();

	return true;
}

void FCEUI_Kill(void)
{
	FCEU_KillGenie();
	FreeBuffers();
}

void FCEUI_CloseGame(void)
{
	FCEU_CloseGame();
}

void ResetNES(void)
{
	if(!GameInfo)
		return;
	GameInterface(GI_RESETM2);
	FCEUSND_Reset();
	FCEUPPU_Reset();
	X6502_Reset();

	// clear back buffer
	extern uint8 *XBackBuf;
	memset(XBackBuf,0,256*256);
}

static void FCEU_MemoryRand(uint8 *ptr, uint32 size)
{
	int x=0;
	do
	{
		*ptr=(x&4)?0xFF:0x00;
		x++;
		size--;
		ptr++;
	}while(size);
}

void PowerNES(void)
{
	//void MapperInit();
	//MapperInit();

	if(!GameInfo)
		return;

	FCEU_CheatResetRAM();
	FCEU_CheatAddRAM(2,0,RAM);

	GeniePower();

	FCEU_MemoryRand(RAM,0x800);
	//memset(RAM,0xFF,0x800);

	SetReadHandler(0x0000,0xFFFF,ANull);
	SetWriteHandler(0x0000,0xFFFF,BNull);

	SetReadHandler(0,0x7FF,ARAML);
	SetWriteHandler(0,0x7FF,BRAML);

	SetReadHandler(0x800,0x1FFF,ARAMH); // Part of a little
	SetWriteHandler(0x800,0x1FFF,BRAMH); //hack for a small speed boost.

	InitializeInput();
	FCEUSND_Power();
	FCEUPPU_Power();

	//Have the external game hardware "powered" after the internal NES stuff.  Needed for the VS System code.
	GameInterface(GI_POWER);
	if(GameInfo->type==GIT_VSUNI)
		FCEU_VSUniPower();

	timestampbase=0;

	X6502_Power();
	FCEU_PowerCheats();
	// clear back buffer
	extern uint8 *XBackBuf;
	memset(XBackBuf,0,256*256);
}

void FCEU_ResetVidSys(void)
{
	int w;

	if(GameInfo->vidsys==GIV_NTSC)
		w=0;
	else if(GameInfo->vidsys==GIV_PAL)
		w=1;
	else
		w=FSettings.PAL;

	PAL=w?1:0;
	FCEUPPU_SetVideoSystem(w);
	SetSoundVariables();
}

FCEUS FSettings;

void FCEU_printf(const char *format, ...)
{
#if 0
	char temp[2048];

	va_list ap;

	va_start(ap,format);
	vsnprintf(temp,sizeof(temp),format,ap);
	FCEUD_Message(temp);

	va_end(ap);
#endif
}

void FCEU_PrintError(const char *format, ...)
{
	char temp[2048];

	va_list ap;

	va_start(ap,format);
	vsnprintf(temp,sizeof(temp),format,ap);
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
	if(FSettings.PAL)
		GameInfo->vidsys=GIV_PAL;
	else
		GameInfo->vidsys=GIV_NTSC;

	if(GameInfo)
	{
		FCEU_ResetVidSys();
		FCEU_ResetPalette();
		FCEUD_VideoChanged();
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

//Enable or disable Game Genie option.
void FCEUI_SetGameGenie(bool a)
{
	FSettings.GameGenie = a;
}

int32 FCEUI_GetDesiredFPS(void)
{
	if(PAL)
		return(838977920); // ~50.007
	else
		return(1008307711);	// ~60.1
}

void FCEUI_Autosave(void)
{
	if(AutosaveStatus[AutosaveIndex] == 1)
	{
		char * f;
		f = strdup(FCEU_MakeFName(FCEUMKF_AUTOSTATE,AutosaveIndex,0).c_str());
		FCEUSS_Load(f);
		free(f);

		//Set pointer to previous available slot
		if(AutosaveStatus[(AutosaveIndex + AutosaveQty-1)%AutosaveQty] == 1)
		{
			AutosaveIndex = (AutosaveIndex + AutosaveQty-1)%AutosaveQty;
		}
	}
}

int FCEU_TextScanlineOffset(int y)
{
	return FSettings.FirstSLine*256;
}

int FCEU_TextScanlineOffsetFromBottom(int y)
{
	return (FSettings.LastSLine-y)*256;
}

bool FCEU_IsValidUI(EFCEUI ui)
{
	switch(ui)
	{
		case FCEUI_QUICKSAVE:
		case FCEUI_QUICKLOAD:
		case FCEUI_SAVESTATE:
		case FCEUI_LOADSTATE:
		case FCEUI_NEXTSAVESTATE:
		case FCEUI_PREVIOUSSAVESTATE:
		case FCEUI_VIEWSLOTS:
		case FCEUI_RESET:
		case FCEUI_POWER:
			if(!GameInfo)
				return false;
			break;
	}
	return true;
}
