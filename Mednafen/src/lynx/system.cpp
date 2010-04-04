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

#include <string.h>
#include "system.h"

#include "../movie.h"
#include "../general.h"
#include "../netplay.h"
#include "../mempatcher.h"
#include "../md5.h"

CSystem::CSystem(uint8 *filememory, int32 filesize)
	:mCart(NULL),
	mRom(NULL),
	mMemMap(NULL),
	mRam(NULL),
	mCpu(NULL),
	mMikie(NULL),
	mSusie(NULL)
{
	mFileType=HANDY_FILETYPE_LNX;

	if(filesize < 11) throw(-1);

	char clip[11];
	memcpy(clip,filememory,11);
	clip[4]=0;
	clip[10]=0;

	if(!strcmp(&clip[6],"BS93")) mFileType=HANDY_FILETYPE_HOMEBREW;
	else if(!strcmp(&clip[0],"LYNX")) mFileType=HANDY_FILETYPE_LNX;
	else
	{
		throw(-1);
		//CLynxException lynxerr;
		//delete filememory;
		//mFileType=HANDY_FILETYPE_ILLEGAL;
		//lynxerr.Message() << "Handy Error: File format invalid!";
		//lynxerr.Description()
		//	<< "The image you selected was not a recognised game cartridge format." << endl
		//	<< "(see the Handy User Guide for more information).";
		//throw(lynxerr);
	}

	MDFNMP_Init(65536, 1);

	// Create the system objects that we'll use

	// Attempt to load the cartridge errors caught above here...

	mRom = new CRom(MDFN_MakeFName(MDFNMKF_LYNXROM,0,0).c_str());

	// An exception from this will be caught by the level above

	switch(mFileType)
	{
		case HANDY_FILETYPE_LNX:
			mCart = new CCart(filememory,filesize);
			mRam = new CRam(0,0);
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

void CSystem::Reset(void)
{
	gSystemCycleCount=0;
	gNextTimerEvent=0;
	gCPUBootAddress=0;
	gSystemIRQ=FALSE;
	gSystemNMI=FALSE;
	gSystemCPUSleep=FALSE;
	gSystemHalt=FALSE;
	gSuzieDoneTime = 0;

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
		regs.PC=(uint16)gCPUBootAddress;
		mCpu->SetRegs(regs);
	}
}

static CSystem *lynxie = NULL;
extern MDFNGI EmulatedLynx;

static int Load(const char *name, MDFNFILE *fp)
{
 try
 {
  lynxie = new CSystem(fp->data, fp->size);
 }
 catch(int i)
 {
  // FIXME:  erhm, free memory here? 
  return(i);
 }

 lynxie->DisplaySetAttributes(FSettings.rshift, FSettings.gshift, FSettings.bshift, 256 * sizeof(uint32));

 lynxie->mMikie->mikbuf.set_sample_rate(FSettings.SndRate? FSettings.SndRate : 44100, 60);
 lynxie->mMikie->mikbuf.clock_rate(16000000 / 4);
 lynxie->mMikie->mikbuf.bass_freq(60);

 lynxie->mMikie->miksynth.volume(0.50);

 MDFNGameInfo->soundchan = 1;

 int rot = lynxie->CartGetRotate();
 if(rot == CART_ROTATE_LEFT) MDFNGameInfo->rotated = MDFN_ROTATE270;
 else if(rot == CART_ROTATE_RIGHT) MDFNGameInfo->rotated = MDFN_ROTATE90;

 gAudioEnabled = 1;

 memcpy(MDFNGameInfo->MD5, lynxie->mCart->MD5, 16);
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 MDFN_printf(_("ROM:       %dKiB\n"), (lynxie->mCart->InfoROMSize + 1023) / 1024);
 MDFN_printf(_("ROM CRC32: 0x%08x\n"), lynxie->mCart->CRC32());
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 MDFN_LoadGameCheats(NULL);

 MDFNGameInfo->fps = (uint32)(59.8 * 65536 * 256);

 if(MDFN_GetSettingB("lynx.lowpass"))
 {
  lynxie->mMikie->miksynth.treble_eq(-35);
 }
 else
 {
  lynxie->mMikie->miksynth.treble_eq(0);
 }
 return(1);
}

static void CloseGame(void)
{
 MDFN_FlushGameCheats(0);
 if(lynxie)
  delete lynxie;
}

static uint8 *chee;
static void Emulate(EmulateSpecStruct *espec)
{
 MDFNGameInfo->fb = espec->pixels;

 lynxie->SetButtonData(chee[0] | (chee[1] << 8));

 MDFNMP_ApplyPeriodicCheats();

 lynxie->mMikie->mpSkipFrame = espec->skip;
 lynxie->mMikie->mpDisplayCurrent = (uint8 *)espec->pixels;
 lynxie->mMikie->startTS = gSystemCycleCount;

 while(lynxie->mMikie->mpDisplayCurrent && (gSystemCycleCount - lynxie->mMikie->startTS) < 700000)
 {
  lynxie->Update();
//  printf("%d ", gSystemCycleCount - lynxie->mMikie->startTS);
 }

 if(FSettings.SndRate)
 {
  static int16 yaybuf[8000];
  int love;

  lynxie->mMikie->mikbuf.end_frame((gSystemCycleCount - lynxie->mMikie->startTS) >> 2);
  love = lynxie->mMikie->mikbuf.read_samples(yaybuf, 8000);

  //printf("%d %d\n",love, gSystemCycleCount - lynxie->mMikie->startTS);
  *(espec->SoundBufSize) = love;
  *(espec->SoundBuf) = yaybuf;
 }
 else
 {
  *(espec->SoundBufSize) = 0;
  *(espec->SoundBuf) = NULL;
 }
}

static void SetPixelFormat(int rs, int gs, int bs)
{
 lynxie->DisplaySetAttributes(rs, gs, bs, 256 * sizeof(uint32));
}

static void SetInput(int port, const char *type, void *ptr)
{
 chee = (uint8 *)ptr;
}

static void SetSoundMultiplier(double mult)
{
 lynxie->mMikie->mikbuf.set_sample_rate(FSettings.SndRate? FSettings.SndRate : 44100, 60);
 lynxie->mMikie->mikbuf.clock_rate((long int)(16000000 * mult / 4));
}

static void SetSoundVolume(uint32 volume)
{

}

static void Sound(int32 rate)
{
 lynxie->mMikie->mikbuf.set_sample_rate(rate? rate : 44100, 60);
 lynxie->mMikie->mikbuf.clock_rate((long int)(16000000 * FSettings.soundmultiplier / 4));
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT SystemRegs[] =
 {
	SFVAR(gSuzieDoneTime),
        SFVAR(gSystemCycleCount),
        SFVAR(gNextTimerEvent),
        SFVAR(gCPUBootAddress),
        SFVAR(gSystemIRQ),
        SFVAR(gSystemNMI),
        SFVAR(gSystemCPUSleep),
        SFVAR(gSystemHalt),
	{lynxie->GetRamPointer(), RAM_SIZE, "RAM"},
	SFEND
 };
 std::vector <SSDescriptor> love;

 love.push_back(SSDescriptor(SystemRegs, "SYST"));
 MDFNSS_StateAction(sm, load, data_only, love);

 if(!lynxie->mSusie->StateAction(sm, load, data_only))
  return(0);
 if(!lynxie->mMemMap->StateAction(sm, load, data_only))
  return(0);

 if(!lynxie->mCart->StateAction(sm, load, data_only))
  return(0);

 if(!lynxie->mMikie->StateAction(sm, load, data_only))
  return(0);

 if(!lynxie->mCpu->StateAction(sm, load, data_only))
  return(0);

 return(1);
}

static bool ToggleLayer(int which)
{

 return(1);
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFNNPCMD_POWER:
  case MDFNNPCMD_RESET: lynxie->Reset(); break;
 }
}

static MDFNSetting LynxSettings[] =
{
 { "lynx.rotateinput", gettext_noop("Virtually rotate D-pad along with screen."), MDFNST_BOOL, "1" },
 { "lynx.lowpass", gettext_noop("Enable sound output lowpass filter."), MDFNST_BOOL, "1" },
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "a", "A (outer)", 8, IDIT_BUTTON_CAN_RAPID, NULL },
 { "b", "B (inner)", 7, IDIT_BUTTON_CAN_RAPID, NULL },
 { "option_2", "Option 2 (lower)", 5, IDIT_BUTTON_CAN_RAPID, NULL },
 { "option_1", "Option 1 (upper)", 4, IDIT_BUTTON_CAN_RAPID, NULL },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "pause", "PAUSE", 6, IDIT_BUTTON, NULL },
};

static InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


MDFNGI EmulatedLynx =
{
 "lynx",
 #ifdef WANT_DEBUGGER
 NULL,
 #endif
 &InputInfo,
 Load,
 NULL,
 CloseGame,
 ToggleLayer,
 "",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetPixelFormat,
 SetInput,
 NULL,
 NULL,
 NULL,
 SetSoundMultiplier,
 SetSoundVolume,
 Sound,
 DoSimpleCommand,
 LynxSettings,
 0,
 NULL,
 160,
 102,
 160, // Save state preview width
 256 * sizeof(uint32),
 {0, 0, 160, 102},
};

