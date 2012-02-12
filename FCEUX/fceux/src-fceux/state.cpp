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

//  TODO: Add (better) file io error checking

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <fstream>

#include "version.h"
#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "sound.h"
#include "utils/endian.h"
#include "utils/memory.h"
#include "utils/xstring.h"
#include "file.h"
#include "fds.h"
#include "state.h"
#include "ppu.h"
#include "video.h"
#include "input.h"
#include "zlib.h"
#include "driver.h"

using namespace std;

static void (*SPreSave)(void);
static void (*SPostSave)(void);

static int SaveStateStatus[10];

bool FCEU_state_loading_old_format;	//tells the save system innards that we're loading the old format

#define SFMDATA_SIZE (64)
#define RLSB 		FCEUSTATE_RLSB	//0x80000000

static SFORMAT SFMDATA[SFMDATA_SIZE];
static int SFEXINDEX;

extern SFORMAT FCEUPPU_STATEINFO[];
extern SFORMAT FCEU_NEWPPU_STATEINFO[];
extern SFORMAT FCEUSND_STATEINFO[];
extern SFORMAT FCEUCTRL_STATEINFO[];
extern SFORMAT FCEUMOV_STATEINFO[];

SFORMAT SFCPU[]={
	{ &X.PC, 2|RLSB, "PC\0"},
	{ &X.A, 1, "A\0\0"},
	{ &X.P, 1, "P\0\0"},
	{ &X.X, 1, "X\0\0"},
	{ &X.Y, 1, "Y\0\0"},
	{ &X.S, 1, "S\0\0"},
	{ &RAM, 0x800 | FCEUSTATE_INDIRECT, "RAM", },
	{ 0 }
};

SFORMAT SFCPUC[]={
	{ &X.jammed, 1, "JAMM"},
	{ &X.IRQlow, 4|RLSB, "IQLB"},
	{ &X.tcount, 4|RLSB, "ICoa"},
	{ &X.count,  4|RLSB, "ICou"},
	{ &timestampbase, sizeof(timestampbase) | RLSB, "TSBS"},
	{ &X.mooPI, 1, "MooP"}, // alternative to the "quick and dirty hack"
	{ 0 }
};

static int SubWrite(EMUFILE* os, SFORMAT *sf)
{
	uint32 acc=0;

	while(sf->v)
	{
		if(sf->s==~0)		//Link to another struct
		{
			uint32 tmp;

			if(!(tmp=SubWrite(os,(SFORMAT *)sf->v)))
				return(0);
			acc+=tmp;
			sf++;
			continue;
		}

		acc+=8;			//Description + size
		acc+=sf->s&(~FCEUSTATE_FLAGS);

		if(os)			//Are we writing or calculating the size of this block?
		{
			os->fwrite(sf->desc,4);
			write32le(sf->s&(~FCEUSTATE_FLAGS),os);

#ifndef LSB_FIRST
			if(sf->s&RLSB)
				FlipByteOrder((uint8*)sf->v,sf->s&(~FCEUSTATE_FLAGS));
#endif

			if(sf->s&FCEUSTATE_INDIRECT)
				os->fwrite(*(char **)sf->v,sf->s&(~FCEUSTATE_FLAGS));
			else
				os->fwrite((char*)sf->v,sf->s&(~FCEUSTATE_FLAGS));

			//Now restore the original byte order.
#ifndef LSB_FIRST
			if(sf->s&RLSB)
				FlipByteOrder((uint8*)sf->v,sf->s&(~FCEUSTATE_FLAGS));
#endif
		}
		sf++;
	}

	return(acc);
}

static int WriteStateChunk(EMUFILE* os, int type, SFORMAT *sf)
{
	os->fputc(type);
	int bsize = SubWrite((EMUFILE*)0,sf);
	write32le(bsize,os);

	if(!SubWrite(os,sf))
	{
		return 5;
	}
	return (bsize+5);
}

static SFORMAT *CheckS(SFORMAT *sf, uint32 tsize, char *desc)
{
	while(sf->v)
	{
		if(sf->s==~0)		// Link to another SFORMAT structure.
		{
			SFORMAT *tmp;
			if((tmp= CheckS((SFORMAT *)sf->v, tsize, desc) ))
				return(tmp);
			sf++;
			continue;
		}
		if(!memcmp(desc,sf->desc,4))
		{
			if(tsize!=(sf->s&(~FCEUSTATE_FLAGS)))
				return(0);
			return(sf);
		}
		sf++;
	}
	return(0);
}

static bool ReadStateChunk(EMUFILE* is, SFORMAT *sf, int size)
{
	SFORMAT *tmp;
	int temp = is->ftell();

	while(is->ftell() < temp+size)
	{
		uint32 tsize;
		char toa[4];
		if(is->fread(toa,4)<4)
			return false;

		read32le(&tsize,is);

		if((tmp=CheckS(sf,tsize,toa)))
		{
			if(tmp->s&FCEUSTATE_INDIRECT)
				is->fread(*(char **)tmp->v,tmp->s&(~FCEUSTATE_FLAGS));
			else
				is->fread((char *)tmp->v,tmp->s&(~FCEUSTATE_FLAGS));

#ifndef LSB_FIRST
			if(tmp->s&RLSB)
				FlipByteOrder((uint8*)tmp->v,tmp->s&(~FCEUSTATE_FLAGS));
#endif
		}
		else
			is->fseek(tsize,SEEK_CUR);
	}

	return true;
}

static int read_sfcpuc=0;

static bool ReadStateChunks(EMUFILE* is, int32 totalsize)
{
	int t;
	uint32 size;
	bool ret=true;
	bool warned=false;

	read_sfcpuc=0;

	while(totalsize > 0)
	{
		t=is->fgetc();

		if(t==EOF)
			break;

		if(!read32le(&size,is))
			break;

		totalsize -= size + 5;

		switch(t)
		{
			case 1:
				if(!ReadStateChunk(is,SFCPU,size))
					ret=false;
				break;
			case 3:
				if(!ReadStateChunk(is,FCEUPPU_STATEINFO,size))
					ret=false;
				break;
			case 31:
				if(!ReadStateChunk(is,FCEU_NEWPPU_STATEINFO,size))
					ret=false;
				break;
			case 4:
				if(!ReadStateChunk(is,FCEUCTRL_STATEINFO,size))
					ret=false;
				break;
			case 7:
				//allow this to fail in old-format savestates.
				if(!FCEU_state_loading_old_format)
					ret=false;
				break;
			case 0x10:
				if(!ReadStateChunk(is,SFMDATA,size))
					ret=false;
				break;

				// now it gets hackier:
			case 5:
				if(!ReadStateChunk(is,FCEUSND_STATEINFO,size))
					ret=false;
				break;
			case 6:
				is->fseek(size,SEEK_CUR);
				break;
			case 8:
				// load back buffer
				{
					extern uint8 *XBackBuf;
					if(is->fread((char*)XBackBuf,size) != size)
						ret = false;

					//MBG TODO - can this be moved to a better place?
					//does it even make sense, displaying XBuf when its XBackBuf we just loaded?

				}
				break;
			case 2:
				{
					if(!ReadStateChunk(is,SFCPUC,size))
						ret=false;
					else
						read_sfcpuc=1;
				}  break;
			default:
				// for somebody's sanity's sake, at least warn about it:
				if(!warned)
				{
					char str [256];
					sprintf(str, "Warning: Found unknown save chunk of type %d.\nThis could indicate the save state is corrupted\nor made with a different (incompatible) emulator version.", t);
					FCEUD_PrintError(str);
					warned=true;
				}
				//if(fseek(st,size,SEEK_CUR)<0) goto endo;break;
				is->fseek(size,SEEK_CUR);
		}
	}

	return ret;
}

int CurrentState=0;
extern int geniestage;


static bool FCEUSS_SaveMS(EMUFILE* outstream)
{
	//a temp memory stream. we'll dump some data here and then compress
	//TODO - support dumping directly without compressing to save a buffer copy

	EMUFILE_MEMORY ms;
	EMUFILE* os = &ms;

	uint32 totalsize = 0;

	FCEUPPU_SaveState();
	FCEUSND_SaveState();
	totalsize=WriteStateChunk(os,1,SFCPU);
	totalsize+=WriteStateChunk(os,2,SFCPUC);
	totalsize+=WriteStateChunk(os,3,FCEUPPU_STATEINFO);
	totalsize+=WriteStateChunk(os,31,FCEU_NEWPPU_STATEINFO);
	totalsize+=WriteStateChunk(os,4,FCEUCTRL_STATEINFO);
	totalsize+=WriteStateChunk(os,5,FCEUSND_STATEINFO);

	// save back buffer
	{
		extern uint8 *XBackBuf;
		uint32 size = 256 * 256 + 8;
		os->fputc(8);
		write32le(size, os);
		os->fwrite((char*)XBackBuf,size);
		totalsize += 5 + size;
	}

	if(SPreSave)
		SPreSave();

	totalsize += WriteStateChunk(os,0x10,SFMDATA);

	if(SPreSave)
		SPostSave();

	//save the length of the file
	int len = ms.size();

	//sanity check: len and totalsize should be the same
	if(len != totalsize)
	{
		FCEUD_PrintError("sanity violation: len != totalsize");
		return false;
	}

	int error = Z_OK;
	uint8* cbuf = (uint8*)ms.buf();
	uLongf comprlen = -1;

	//dump the header
	uint8 header[16]="FCSX";
	FCEU_en32lsb(header+4, totalsize);
	FCEU_en32lsb(header+8, FCEU_VERSION_NUMERIC);
	FCEU_en32lsb(header+12, comprlen);

	//dump it to the destination file
	outstream->fwrite((char*)header,16);
	outstream->fwrite((char*)cbuf, comprlen == -1 ? totalsize : comprlen);

	if(cbuf != (uint8*)ms.buf())
		delete[] cbuf;

	return error == Z_OK;
}

#ifdef __LIBSNES__
void FCEUSS_SaveMemory(uint8_t *data, unsigned size)
{
   EMUFILE_MEMORY mem(size);
   FCEUSS_SaveMS(&mem);
   std::copy(mem.buf(), mem.buf() + size, data);

#if 0
   const char *tmp = tmpnam(NULL);
   if (!tmp)
      return;
   FCEUSS_Save(tmp);
   FILE *file = fopen(tmp, "rb");
   if (!file)
      return;

   fseek(file, 0, SEEK_END);
   unsigned len = ftell(file);
   if (len != size)
   {
      fclose(file);
      return;
   }

   rewind(file);
   fread(data, 1, len, file);
   fclose(file);
   unlink(tmp);
#endif
}

static bool FCEUSS_LoadFP(EMUFILE* is);
void FCEUSS_LoadMemory(const uint8_t *data, unsigned size)
{
   EMUFILE_MEMORY mem(data, size);
   FCEUSS_LoadFP(&mem);

#if 0
   const char *tmp = tmpnam(NULL);
   FILE *file = fopen(tmp, "wb");
   if (!file)
      return;

   fwrite(data, 1, size, file);
   fclose(file);
   FCEUSS_Load(tmp);
   unlink(tmp);
#endif
}

unsigned FCEUSS_SizeMemory()
{
   EMUFILE_MEMORY mem;
   FCEUSS_SaveMS(&mem);
   return mem.size();

#if 0
   const char *tmp = tmpnam(NULL);
   FCEUSS_Save(tmp);
   FILE *file = fopen(tmp, "rb");
   if (!file)
   {
      unlink(tmp);
      return 0;
   }

   fseek(file, 0, SEEK_END);
   unsigned len = ftell(file);
   fclose(file);
   unlink(tmp);
   return len;
#endif
}
#endif

void FCEUSS_Save(const char *fname)
{
	EMUFILE* st = 0;
	char fn[2048];

	if(geniestage==1)
	{
		//FCEU_DispMessage("Cannot save FCS in GG screen.",0);
		return;
	}

	if(fname)	//If filename is given use it.
	{
		st = new EMUFILE_FILE(fname, "wb");
		strcpy(fn, fname);
	}
	else		//Else, generate one
	{
		//FCEU_PrintError("daCurrentState=%d",CurrentState);
		strcpy(fn, FCEU_MakeFName(FCEUMKF_STATE,CurrentState,0).c_str());

		st = new EMUFILE_FILE(fn,"wb");
	}

	if(st == NULL || st->get_fp() == NULL)
	{
		//FCEU_DispMessage("State %d save error.",0,CurrentState);
		return;
	}

	FCEUSS_SaveMS(st);

	delete st;

	if(!fname)
	{
		SaveStateStatus[CurrentState]=1;
		//FCEU_DispMessage("State %d saved.",0,CurrentState);
	}
}

static int FCEUSS_LoadFP_old(EMUFILE* is)
{
	int x;
	uint8 header[16];
	int stateversion;
	char* fn=0;

	//if(params!=SSLOADPARAM_DUMMY)
	is->fread((char*)&header,16);

	if(memcmp(header,"FCS",3))
		return 0;

	if(header[3] == 0xFF)
		stateversion = FCEU_de32lsb(header + 8);
	else
		stateversion = header[3] * 100;

	//if(params == SSLOADPARAM_DUMMY)
	//{
	//	scan_chunks=1;
	//}
	x=ReadStateChunks(is,*(uint32*)(header+4));
	//if(params == SSLOADPARAM_DUMMY)
	//{
	//	scan_chunks=0;
	//	return 1;
	//}
	if(read_sfcpuc && stateversion<9500)
		X.IRQlow=0;

	if(GameStateRestore)
		GameStateRestore(stateversion);

	if(x)
	{
		FCEUPPU_LoadState(stateversion);
		FCEUSND_LoadState(stateversion);  
		x= true;
	}
	if(fn)
	{
		// Previously, temporary savestate would be loaded here in case things went wrong -
		// was commented-out code anyway
		free(fn);
	}

	return(x);
}


static bool FCEUSS_LoadFP(EMUFILE* is)
{
	if(!is)
		return false;

	uint8 header[16];

	//read and analyze the header
	is->fread((char*)&header,16);
	if(memcmp(header,"FCSX",4)) {
		//its not an fceux save file.. perhaps it is an fceu savefile
		is->fseek(0,SEEK_SET);
		FCEU_state_loading_old_format = true;
		bool ret = FCEUSS_LoadFP_old(is) != 0;
		FCEU_state_loading_old_format = false;
		return ret;
	}

	int totalsize = FCEU_de32lsb(header + 4);
	int stateversion = FCEU_de32lsb(header + 8);
	int comprlen = FCEU_de32lsb(header + 12);

	std::vector<uint8> buf(totalsize);

	//not compressed:
	if(comprlen != -1)
	{
		//load the compressed chunk and decompress
		std::vector<char> cbuf(comprlen);
		is->fread((char*)&cbuf[0],comprlen);

		uLongf uncomprlen = totalsize;
		int error = uncompress((uint8*)&buf[0],&uncomprlen,(uint8*)&cbuf[0],comprlen);
		if(error != Z_OK || uncomprlen != totalsize)
			return false;
		//we dont need to restore the backup here because we havent messed with the emulator state yet
	}
	else
		is->fread((char*)&buf[0],totalsize);

	EMUFILE_MEMORY mstemp(&buf);
	bool x = ReadStateChunks(&mstemp,totalsize)!=0;

	if(GameStateRestore)
		GameStateRestore(stateversion);

	if(x)
	{
		FCEUPPU_LoadState(stateversion);
		FCEUSND_LoadState(stateversion);
	}

	return x;
}


bool FCEUSS_Load(const char *fname)
{
	EMUFILE* st;
	char fn[2048];

	if(geniestage == 1)
		return false;	// Cannot load FCS in GG screen.

	if(fname)
	{
		st= new EMUFILE_FILE(fname, "rb");
		strcpy(fn, fname);
	}
	else
	{
		strcpy(fn, FCEU_MakeFName(FCEUMKF_STATE,CurrentState,fname).c_str());
		st= new EMUFILE_FILE(fn,"rb");
	}

	if(st == NULL || (st->get_fp() == NULL))
	{
		//FCEU_DispMessage("State %d load error. Filename: %s",0,CurrentState, fn);
		SaveStateStatus[CurrentState]=0;
		return false;
	}
    
	//If in bot mode, don't do a backup when loading.
	//Otherwise you eat at the hard disk, since so many
	//states are being loaded.
	if(FCEUSS_LoadFP(st))
	{
		if(fname)
		{
			char szFilename[260]={0};
			splitpath(fname, 0, 0, szFilename, 0);
			//FCEU_DispMessage("State %s loaded. Filename: %s",0,szFilename, fn);
		}
		else
		{
			//FCEU_DispMessage("State %d loaded. Filename: %s",0,CurrentState, fn);
			SaveStateStatus[CurrentState]=1;
		}
		delete st;

		return true;
	}
	else
	{
		if(!fname)
			SaveStateStatus[CurrentState]=1;

		// Error reading state!
		delete st;
		return 0;
	}
}

void FCEUSS_CheckStates(void)
{
	FILE *st=NULL;

	for(int ssel = 0; ssel < 10; ssel++)
	{
		std::string soot = FCEU_MakeFName(FCEUMKF_STATE,ssel,0);
		st= fopen(soot.c_str(),"rb");
		if(st)
		{
			SaveStateStatus[ssel]=1;
			fclose(st);
		}
		else
			SaveStateStatus[ssel]=0;
	}

	CurrentState=1;
}

void ResetExState(void (*PreSave)(void), void (*PostSave)(void))
{
	for(int x = 0; x < SFEXINDEX; x++)
	{
		if(SFMDATA[x].desc)
			free(SFMDATA[x].desc);
	}

	// adelikat, 3/14/09:  had to add this to clear out the size parameter.  NROM(mapper 0) games were having savestate crashes if loaded after a non NROM game	because the size variable was carrying over and causing savestates to save too much data
	SFMDATA[0].s = 0;		
	
	SPreSave = PreSave;
	SPostSave = PostSave;
	SFEXINDEX=0;
}

void AddExState(void *v, uint32 s, int type, char *desc)
{
	if(desc)
	{
		SFMDATA[SFEXINDEX].desc=(char *)malloc(5);
		strcpy(SFMDATA[SFEXINDEX].desc,desc);
	}
	else
		SFMDATA[SFEXINDEX].desc=0;

	SFMDATA[SFEXINDEX].v=v;
	SFMDATA[SFEXINDEX].s=s;
	if(type)
		SFMDATA[SFEXINDEX].s |= RLSB;

	if(SFEXINDEX<SFMDATA_SIZE-1)
		SFEXINDEX++;
	else
	{
		static int once=1;
		if(once)
		{
			once=0;
			FCEU_PrintError("Error in AddExState: SFEXINDEX overflow.\nSomebody made SFMDATA_SIZE too small.");
		}
	}
	SFMDATA[SFEXINDEX].v=0;		// End marker.
}

void FCEUI_SelectStateNext(int n)
{
	if(n>0)
		CurrentState=(CurrentState+1)%10;
	else
		CurrentState=(CurrentState+9)%10;

	FCEUI_SelectState(CurrentState, 1);
}

int FCEUI_SelectState(int w, int show)
{
	FCEUSS_CheckStates();
	int oldstate=CurrentState;

	if(w == -1)
		return 0;	//mbg merge 7/17/06 had to make return a value

	CurrentState=w;

	return oldstate;
}

void FCEUI_SaveState(const char *fname)
{
	if(!FCEU_IsValidUI(FCEUI_SAVESTATE))
		return;

	FCEUSS_Save(fname);
}

bool file_exists(const char * filename)
{
	if (FILE * file = fopen(filename, "r")) //I'm sure, you meant for READING =)
	{
		fclose(file);
		return true;
	}

	return false;
}
uint32_t FCEUI_LoadState(const char *fname)
{
	if(!FCEU_IsValidUI(FCEUI_LOADSTATE))
		return false;

	if (fname != NULL && !file_exists(fname))
		return false; // state doesn't exist; exit cleanly

	if(FCEUSS_Load(fname))
		  return true;
	else
		return false;
}
