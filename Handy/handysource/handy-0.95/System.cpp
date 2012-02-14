//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// System object class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides the glue to bind of of the emulation objects         //
// together via peek/poke handlers and pass thru interfaces to lower        //
// objects, all control of the emulator is done via this class. Update()    //
// does most of the work and each call emulates one CPU instruction and     //
// updates all of the relevant hardware if required. It must be remembered  //
// that if that instruction involves setting SPRGO then, it will cause a    //
// sprite painting operation and then a corresponding update of all of the  //
// hardware which will usually involve recursive calls to Update, see       //
// Mikey SPRGO code for more details.                                       //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define SYSTEM_CPP

//#include <crtdbg.h>
//#define	TRACE_SYSTEM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "error.h"
#include "./zlib-113/zlib.h"
#include "./zlib-113/unzip.h"

#ifdef WII
#include "wii_handy.h"
#include "wii_hash.h"
#endif

// The following is needed since GCC doesn't seem to understand the _splitpath function.
// Because of this "problem", an additional header file is created which resides the
// _splitpath function as well as other dependencies.
//
// Source : Usenetposting done by Martin Schmidt
#ifdef SDL_PATCH
	#include "stdlib_gcc_extra.h"
#endif

int lss_read(void* dest,int varsize, int varcount,LSS_FILE *fp)
{
	ULONG copysize;
	copysize=varsize*varcount;
	if((fp->index + copysize) > fp->index_limit) copysize=fp->index_limit - fp->index;
	memcpy(dest,fp->memptr+fp->index,copysize);
	fp->index+=copysize;
	return copysize;
}

CSystem::CSystem(char* gamefile,char* romfile)
	:mCart(NULL),
	mRom(NULL),
	mMemMap(NULL),
	mRam(NULL),
	mCpu(NULL),
	mMikie(NULL),
	mSusie(NULL)
{

#ifdef _LYNXDBG
	mpDebugCallback=NULL;
	mDebugCallbackObject=0;
#endif

	// Select the default filetype
	UBYTE *filememory=NULL;
	UBYTE *howardmemory=NULL;
	ULONG filesize=0;
	ULONG howardsize=0;

	mFileType=HANDY_FILETYPE_LNX;
	if(strcmp(gamefile,"")==0)
	{
		// No file
		filesize=0;
		filememory=NULL;
	}
	else if(IsZip(gamefile))
	{
		// Try and find a file in the zip
		unzFile *fp;
		unz_file_info info;
		char filename_buf[0x100], *ptr;
		bool gotIt;
		
		if((fp=(unzFile*)unzOpen(gamefile))!=NULL)
		{
			if(unzGoToFirstFile(fp)!=UNZ_OK)
			{
				unzClose(fp);
				CLynxException lynxerr;
				lynxerr.Message() << "Handy Error: ZIP File select problems" ;
				lynxerr.Description()
					<< "The file you selected could not be read." << endl
					<< "(The ZIP file may be corrupted)." << endl ;
				throw(lynxerr);
			}
			
			gotIt = FALSE;
			for (;;)
			{
				// Get file descriptor and analyse
				if(unzGetCurrentFileInfo(fp, &info, filename_buf, 0x100, NULL, 0, NULL, 0) != UNZ_OK)
				{
					break;
				}
				else
				{
					ptr = strchr(filename_buf, '.');
					if (ptr != NULL)
					{
						char buf[4];
					
						ptr++; buf[0] = tolower(*ptr);
						ptr++; buf[1] = tolower(*ptr);
						ptr++; buf[2] = tolower(*ptr);
						buf[3] = 0;
						if (!strcmp(buf, "lnx") || !strcmp(buf, "com") || !strcmp(buf, "o"))
						{
							// Found a likely file so signal
							gotIt = TRUE;
							break;
						}
					}

					// No match so lets try the next file
					if(unzGoToNextFile(fp)!=UNZ_OK)	break;
				}
			}
			
			// Did we strike gold ?
			if(gotIt)
			{
				if(unzOpenCurrentFile(fp)==UNZ_OK)
				{
					// Allocate memory for the rom
					filesize=info.uncompressed_size;
					filememory=(UBYTE*) new UBYTE[filesize];

					// Read it into memory					
					if(unzReadCurrentFile(fp,filememory,filesize)!=(int)info.uncompressed_size)
					{
						unzCloseCurrentFile(fp);
						unzClose(fp);
						delete filememory;
						// Throw a wobbly
						CLynxException lynxerr;
						lynxerr.Message() << "Handy Error: ZIP File load problems" ;
						lynxerr.Description()
							<< "The zip file you selected could not be loaded." << endl
							<< "(The ZIP file may be corrupted)." << endl ;
						throw(lynxerr);
					}

					// Got it!
					unzCloseCurrentFile(fp);
					unzClose(fp);
				}
				
			}
			else
			{
				CLynxException lynxerr;
				lynxerr.Message() << "Handy Error: ZIP File load problems" ;
				lynxerr.Description()
					<< "The file you selected could not be loaded." << endl
					<< "Could not find a Lynx file in the ZIP archive." << endl ;
				throw(lynxerr);
			}
		}
		else
		{
			CLynxException lynxerr;
			lynxerr.Message() << "Handy Error: ZIP File open problems" ;
			lynxerr.Description()
				<< "The file you selected could not be opened." << endl
				<< "(The ZIP file may be corrupted)." << endl ;
			throw(lynxerr);
		}
	}
	else
	{
		// Open the file and load the file
		FILE	*fp;

		// Open the cartridge file for reading
		if((fp=fopen(gamefile,"rb"))==NULL)
		{
			CLynxException lynxerr;

			lynxerr.Message() << "Handy Error: File Open Error";
			lynxerr.Description()
				<< "The lynx emulator will not run without a cartridge image." << endl
				<< "\"" << gamefile << "\" was not found in the place you " << endl
				<< "specified. (see the Handy User Guide for more information).";
			throw(lynxerr);
		}

		// How big is the file ??
		fseek(fp,0,SEEK_END);
		filesize=ftell(fp);
		fseek(fp,0,SEEK_SET);
		filememory=(UBYTE*) new UBYTE[filesize];

		if(fread(filememory,sizeof(char),filesize,fp)!=filesize)
		{
			CLynxException lynxerr;
			delete filememory;

			lynxerr.Message() << "Handy Error: Unspecified Load error (Header)";
			lynxerr.Description()
				<< "The lynx emulator will not run without a cartridge image." << endl
				<< "It appears that your cartridge image may be corrupted or there is" << endl
				<< "some other error.(see the Handy User Guide for more information)";
			throw(lynxerr);
		}

		fclose(fp);
	}

	// Now try and determine the filetype we have opened
	if(filesize)
	{
#ifdef WII
    wii_hash_compute( filememory, filesize, wii_cartridge_hash );
    memcpy( 
      wii_cartridge_hash_with_header, 
      wii_cartridge_hash, 
      sizeof(wii_cartridge_hash_with_header) );
#endif

		char clip[11];
		memcpy(clip,filememory,11);
		clip[4]=0;
		clip[10]=0;

		if(!strcmp(&clip[6],"BS93")) mFileType=HANDY_FILETYPE_HOMEBREW;
		else if(!strcmp(&clip[0],"LYNX")) mFileType=HANDY_FILETYPE_LNX;
		else if(!strcmp(&clip[0],LSS_VERSION_OLD)) mFileType=HANDY_FILETYPE_SNAPSHOT;
		else
		{
			CLynxException lynxerr;
			delete filememory;
			mFileType=HANDY_FILETYPE_ILLEGAL;
			lynxerr.Message() << "Handy Error: File format invalid!";
			lynxerr.Description()
				<< "The image you selected was not a recognised game cartridge format." << endl
				<< "(see the Handy User Guide for more information).";
			throw(lynxerr);
		}
	}
	
	mCycleCountBreakpoint=0xffffffff;

// Create the system objects that we'll use

	// Attempt to load the cartridge errors caught above here...

	mRom = new CRom(romfile);

	// An exception from this will be caught by the level above

	switch(mFileType)
	{
		case HANDY_FILETYPE_LNX:
			mCart = new CCart(filememory,filesize);
			if(mCart->CartHeaderLess())
			{
				FILE	*fp;
				char drive[3],dir[256],cartgo[256];
				mFileType=HANDY_FILETYPE_HOMEBREW;
				_splitpath(romfile,drive,dir,NULL,NULL);
				strcpy(cartgo,drive);
				strcat(cartgo,dir);
				strcat(cartgo,"howard.o");

				// Open the howard file for reading
				if((fp=fopen(cartgo,"rb"))==NULL)
				{
					CLynxException lynxerr;
					delete filememory;
					lynxerr.Message() << "Handy Error: Howard.o File Open Error";
					lynxerr.Description()
						<< "Headerless cartridges need howard.o bootfile to ." << endl
						<< "be able to run correctly, could not open file. " << endl;
					throw(lynxerr);
				}

				// How big is the file ??
				fseek(fp,0,SEEK_END);
				howardsize=ftell(fp);
				fseek(fp,0,SEEK_SET);
				howardmemory=(UBYTE*) new UBYTE[filesize];

				if(fread(howardmemory,sizeof(char),howardsize,fp)!=howardsize)
				{
					CLynxException lynxerr;
					delete filememory;
					delete howardmemory;
					lynxerr.Message() << "Handy Error: Howard.o load error (Header)";
					lynxerr.Description()
						<< "Howard.o could not be read????." << endl;
					throw(lynxerr);
				}

				fclose(fp);

				// Pass it to RAM to load
				mRam = new CRam(howardmemory,howardsize);
			}
			else
			{
				mRam = new CRam(0,0);
			}
			break;
		case HANDY_FILETYPE_HOMEBREW:
			mCart = new CCart(0,0);
			mRam = new CRam(filememory,filesize);
			break;
		case HANDY_FILETYPE_SNAPSHOT:
		case HANDY_FILETYPE_ILLEGAL:
		default:
			mCart = new CCart(0,0);
			mRam = new CRam(0,0);
			break;
	}

	// These can generate exceptions

	mMikie = new CMikie(*this);
	mSusie = new CSusie(*this);

// Instantiate the memory map handler

	mMemMap = new CMemMap(*this);

// Now the handlers are set we can instantiate the CPU as is will use handlers on reset

	mCpu = new C65C02(*this);

// Now init is complete do a reset, this will cause many things to be reset twice
// but what the hell, who cares, I don't.....

	Reset();

// If this is a snapshot type then restore the context

	if(mFileType==HANDY_FILETYPE_SNAPSHOT)
	{
		if(!ContextLoad(gamefile))
		{
			Reset();
			CLynxException lynxerr;
			lynxerr.Message() << "Handy Error: Snapshot load error" ;
			lynxerr.Description()
				<< "The snapshot you selected could not be loaded." << endl
				<< "(The file format was not recognised by Handy)." << endl ;
			throw(lynxerr);
		}
	}
	if(filesize) delete filememory;
	if(howardsize) delete howardmemory;
}

CSystem::~CSystem()
{
	// Cleanup all our objects

	if(mCart!=NULL) delete mCart;
	if(mRom!=NULL) delete mRom;
	if(mRam!=NULL) delete mRam;
	if(mCpu!=NULL) delete mCpu;
	if(mMikie!=NULL) delete mMikie;
	if(mSusie!=NULL) delete mSusie;
	if(mMemMap!=NULL) delete mMemMap;
}

bool CSystem::IsZip(const char *filename)
{
	UBYTE buf[2];
	FILE *fp;

	if((fp=fopen(filename,"rb"))!=NULL)
	{
		fread(buf, 2, 1, fp);
		fclose(fp);
		return(memcmp(buf,"PK",2)==0);
	}
	if(fp)fclose(fp);
	return FALSE;
}

void CSystem::Reset(void)
{
	gSystemCycleCount=0;
	gNextTimerEvent=0;
	gCPUBootAddress=0;
	gBreakpointHit=FALSE;
	gSingleStepMode=FALSE;
	gSingleStepModeSprites=FALSE;
	gSystemIRQ=FALSE;
	gSystemNMI=FALSE;
	gSystemCPUSleep=FALSE;
	gSystemHalt=FALSE;

	gThrottleLastTimerCount=0;
	gThrottleNextCycleCheckpoint=0;

	gTimerCount=0;

	gAudioBufferPointer=0;
	gAudioLastUpdateCycle=0;
	memset(gAudioBuffer,128,HANDY_AUDIO_BUFFER_SIZE);

#ifdef _LYNXDBG
	gSystemHalt=TRUE;
#endif

	mMemMap->Reset();
	mCart->Reset();
	mRom->Reset();
	mRam->Reset();
	mMikie->Reset();
	mSusie->Reset();
	mCpu->Reset();

	// Homebrew hashup

	if(mFileType==HANDY_FILETYPE_HOMEBREW)
	{
		mMikie->PresetForHomebrew();

		C6502_REGS regs;
		mCpu->GetRegs(regs);
		regs.PC=(UWORD)gCPUBootAddress;
		mCpu->SetRegs(regs);
	}
}

bool CSystem::ContextSave(const char *context)
{
	FILE *fp;
	bool status=1;

	if((fp=fopen(context,"wb"))==NULL) return false;

	if(!fprintf(fp,LSS_VERSION)) status=0;

	// Save ROM CRC
	ULONG checksum=mCart->CRC32();
	if(!fwrite(&checksum,sizeof(ULONG),1,fp)) status=0;

	if(!fprintf(fp,"CSystem::ContextSave")) status=0;

	if(!fwrite(&mCycleCountBreakpoint,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSystemCycleCount,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gNextTimerEvent,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gCPUWakeupTime,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gCPUBootAddress,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gIRQEntryCycle,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gBreakpointHit,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSingleStepMode,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSystemIRQ,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSystemNMI,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSystemCPUSleep,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSystemCPUSleep_Saved,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gSystemHalt,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gThrottleMaxPercentage,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gThrottleLastTimerCount,sizeof(ULONG),1,fp)) status=0;
	if(!fwrite(&gThrottleNextCycleCheckpoint,sizeof(ULONG),1,fp)) status=0;

	ULONG tmp=gTimerCount;
	if(!fwrite(&tmp,sizeof(ULONG),1,fp)) status=0;

#ifdef WII
  // This is necessary for backward compatibility with previous WiiHandy releases
  UBYTE	oldAudioBuffer[OLD_HANDY_AUDIO_BUFFER_SIZE];
  memset( oldAudioBuffer, 128, OLD_HANDY_AUDIO_BUFFER_SIZE );
  if(!fwrite(oldAudioBuffer,sizeof(UBYTE),OLD_HANDY_AUDIO_BUFFER_SIZE,fp)) status=0;
  ULONG	oldAudioBufferPointer=0;
  if(!fwrite(&oldAudioBufferPointer,sizeof(ULONG),1,fp)) status=0;
#else
	if(!fwrite(gAudioBuffer,sizeof(UBYTE),HANDY_AUDIO_BUFFER_SIZE,fp)) status=0;
  if(!fwrite(&gAudioBufferPointer,sizeof(ULONG),1,fp)) status=0;
#endif	
	if(!fwrite(&gAudioLastUpdateCycle,sizeof(ULONG),1,fp)) status=0;

	// Save other device contexts
	if(!mMemMap->ContextSave(fp)) status=0;
	if(!mCart->ContextSave(fp)) status=0; 
	//if(!mRom->ContextSave(fp)) status=0; // We no longer save the system ROM
	if(!mRam->ContextSave(fp)) status=0;
	if(!mMikie->ContextSave(fp)) status=0;
	if(!mSusie->ContextSave(fp)) status=0;
	if(!mCpu->ContextSave(fp)) status=0;

	fclose(fp); 
	return status;
}

			 
bool CSystem::ContextLoad(const char *context)
{
	LSS_FILE *fp;
	bool status=1;
	UBYTE *filememory=NULL;
	ULONG filesize=0;

	// First check for ZIP file
	if(IsZip(context))
	{
		// Find the file and read into memory
		// Try and find a file in the zip
		unzFile *fp;
		unz_file_info info;
		char filename_buf[0x100], *ptr;
		bool gotIt;
		
		if((fp=(unzFile*)unzOpen(context))!=NULL)
		{
			if(unzGoToFirstFile(fp)!=UNZ_OK)
			{
				unzClose(fp);
				gError->Warning("ContextLoad(): ZIP File select problems, could not read zip file");
				return 1;
			}
			
			gotIt = FALSE;
			for (;;)
			{
				// Get file descriptor and analyse
				if(unzGetCurrentFileInfo(fp, &info, filename_buf, 0x100, NULL, 0, NULL, 0) != UNZ_OK)
				{
					break;
				}
				else
				{
					ptr = strchr(filename_buf, '.');
					if (ptr != NULL)
					{
						char buf[4];
					
						ptr++; buf[0] = tolower(*ptr);
						ptr++; buf[1] = tolower(*ptr);
						ptr++; buf[2] = tolower(*ptr);
						buf[3] = 0;
						if (!strcmp(buf, "lss"))
						{
							// Found a likely file so signal
							gotIt = TRUE;
							break;
						}
					}

					// No match so lets try the next file
					if(unzGoToNextFile(fp)!=UNZ_OK)	break;
				}
			}
			
			// Did we strike gold ?
			if(gotIt)
			{
				if(unzOpenCurrentFile(fp)==UNZ_OK)
				{
					// Allocate memory for the rom
					filesize=info.uncompressed_size;
					filememory=(UBYTE*) new UBYTE[filesize];

					// Read it into memory					
					if(unzReadCurrentFile(fp,filememory,filesize)!=(int)info.uncompressed_size)
					{
						unzCloseCurrentFile(fp);
						unzClose(fp);
						delete filememory;
						// Throw a wobbly
						gError->Warning("ContextLoad(): ZIP File load problems, could not read data from the zip file");
						return 1;
					}

					// Got it!
					unzCloseCurrentFile(fp);
					unzClose(fp);
				}
				
			}
			else
			{
				gError->Warning("ContextLoad(): ZIP File load problems, could not find an LSS file in the zip archive");
				return 1;
			}
		}
		else
		{
			gError->Warning("ContextLoad(): ZIP File load problems, could not open the zip archive");
			return 1;
		}

	}
	else
	{
		FILE *fp;
		// Just open an read into memory
		if((fp=fopen(context,"rb"))==NULL) status=0;

		fseek(fp,0,SEEK_END);
		filesize=ftell(fp);
		fseek(fp,0,SEEK_SET);
		filememory=(UBYTE*) new UBYTE[filesize];

		if(fread(filememory,sizeof(char),filesize,fp)!=filesize)
		{
			fclose(fp);
			return 1;
		}
		fclose(fp);
	}

	// Setup our read structure
	fp = new LSS_FILE;
	fp->memptr=filememory;
	fp->index=0;
	fp->index_limit=filesize;

	char teststr[100];
	// Check identifier
	if(!lss_read(teststr,sizeof(char),4,fp)) status=0;
	teststr[4]=0;

	if(strcmp(teststr,LSS_VERSION)==0 || strcmp(teststr,LSS_VERSION_OLD)==0)
	{
		bool legacy=FALSE;
		if(strcmp(teststr,LSS_VERSION_OLD)==0)
		{
			legacy=TRUE;
		}
		else
		{
			ULONG checksum;
			// Read CRC32 and check against the CART for a match
			lss_read(&checksum,sizeof(ULONG),1,fp);
			if(mCart->CRC32()!=checksum)
			{
				delete fp;
				delete filememory;
				gError->Warning("LSS Snapshot CRC does not match the loaded cartridge image, aborting load");
				return 0;
			}
		}

		// Check our block header
		if(!lss_read(teststr,sizeof(char),20,fp)) status=0;
		teststr[20]=0;
		if(strcmp(teststr,"CSystem::ContextSave")!=0) status=0;

		if(!lss_read(&mCycleCountBreakpoint,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSystemCycleCount,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gNextTimerEvent,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gCPUWakeupTime,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gCPUBootAddress,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gIRQEntryCycle,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gBreakpointHit,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSingleStepMode,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSystemIRQ,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSystemNMI,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSystemCPUSleep,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSystemCPUSleep_Saved,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gSystemHalt,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gThrottleMaxPercentage,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gThrottleLastTimerCount,sizeof(ULONG),1,fp)) status=0;
		if(!lss_read(&gThrottleNextCycleCheckpoint,sizeof(ULONG),1,fp)) status=0;

		ULONG tmp;
		if(!lss_read(&tmp,sizeof(ULONG),1,fp)) status=0;
		gTimerCount=tmp;

#ifdef WII
    UBYTE	oldAudioBuffer[OLD_HANDY_AUDIO_BUFFER_SIZE];
    if(!lss_read(oldAudioBuffer,sizeof(UBYTE),OLD_HANDY_AUDIO_BUFFER_SIZE,fp)) status=0;
		if(!lss_read(&gAudioBufferPointer,sizeof(ULONG),1,fp)) status=0;
    gAudioBufferPointer = 0;
#else
		if(!lss_read(gAudioBuffer,sizeof(UBYTE),HANDY_AUDIO_BUFFER_SIZE,fp)) status=0;
		if(!lss_read(&gAudioBufferPointer,sizeof(ULONG),1,fp)) status=0;
#endif
		if(!lss_read(&gAudioLastUpdateCycle,sizeof(ULONG),1,fp)) status=0;

		if(!mMemMap->ContextLoad(fp)) status=0;
		// Legacy support
		if(legacy)
		{
			if(!mCart->ContextLoadLegacy(fp)) status=0;
			if(!mRom->ContextLoad(fp)) status=0;
		}
		else
		{
			if(!mCart->ContextLoad(fp)) status=0;
		}
		if(!mRam->ContextLoad(fp)) status=0;
		if(!mMikie->ContextLoad(fp)) status=0;
		if(!mSusie->ContextLoad(fp)) status=0;
		if(!mCpu->ContextLoad(fp)) status=0;
	}
	else
	{
		gError->Warning("Not a recognised LSS file");
	}

	delete fp;
	delete filememory;

	return status;
}

#ifdef _LYNXDBG

void CSystem::DebugTrace(int address)
{
	char message[1024+1];
	int count=0;

	sprintf(message,"%08x - DebugTrace(): ",gSystemCycleCount);
	count=strlen(message);

	if(address)
	{
		if(address==0xffff)
		{
			C6502_REGS regs;
			char linetext[1024];
			// Register dump
			GetRegs(regs);
			sprintf(linetext,"PC=$%04x SP=$%02x PS=0x%02x A=0x%02x X=0x%02x Y=0x%02x",regs.PC,regs.SP, regs.PS,regs.A,regs.X,regs.Y);
			strcat(message,linetext);
			count=strlen(message);
		}
		else
		{
			// The RAM address contents should be dumped to an open debug file in this function
			do
			{
				message[count++]=Peek_RAM(address);
			}
			while(count<1024 && Peek_RAM(address++)!=0);
		}
	}
	else
	{
		strcat(message,"CPU Breakpoint");
		count=strlen(message);
	}
	message[count]=0;

	// Callback to dump the message
	if(mpDebugCallback)
	{
		(*mpDebugCallback)(mDebugCallbackObject,message);
	}
}

void CSystem::DebugSetCallback(void (*function)(ULONG objref,char *message),ULONG objref)
{
	mDebugCallbackObject=objref;
	mpDebugCallback=function;
}


#endif
