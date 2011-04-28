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

#include "system.h"

#include "../movie.h"
#include "../general.h"
#include "../mempatcher.h"
#include "../md5.h"

CSystem::CSystem(const uint8 *filememory, int32 filesize)
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

	mRom = new CRom(MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, "lynxboot.img").c_str());

	// An exception from this will be caught by the level above

	switch(mFileType)
	{
		case HANDY_FILETYPE_LNX:
			mCart = new CCart(filememory,filesize);
			mRam = new CRam(0,0);
			break;
		case HANDY_FILETYPE_HOMEBREW:
			{
			 #if 0
			 static uint8 dummy_cart[sizeof(LYNX_HEADER) + 65536] = 
			 {
				'L', 'Y', 'N', 'X', 0x00, 0x01, 0x00, 0x00,
				0x01, 0x00,
			 };
			 mCart = new CCart(dummy_cart, sizeof(dummy_cart));
			 #else
			 mCart = new CCart(NULL, 0);
			 #endif
			 mRam = new CRam(filememory,filesize);
			}
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

// Somewhat of a hack to make sure undrawn lines are black.
bool LynxLineDrawn[256];

static CSystem *lynxie = NULL;
extern MDFNGI EmulatedLynx;

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 return(CCart::TestMagic(fp->data, fp->size));
}

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

 int rot = lynxie->CartGetRotate();
 if(rot == CART_ROTATE_LEFT) MDFNGameInfo->rotated = MDFN_ROTATE270;
 else if(rot == CART_ROTATE_RIGHT) MDFNGameInfo->rotated = MDFN_ROTATE90;

 gAudioEnabled = 1;

 memcpy(MDFNGameInfo->MD5, lynxie->mCart->MD5, 16);
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 MDFN_printf(_("ROM:       %dKiB\n"), (lynxie->mCart->InfoROMSize + 1023) / 1024);
 MDFN_printf(_("ROM CRC32: 0x%08x\n"), lynxie->mCart->CRC32());
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

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
 if(lynxie)
 {
  delete lynxie;
  lynxie = NULL;
 }
}

static uint8 *chee;
static void Emulate(EmulateSpecStruct *espec)
{
 espec->DisplayRect.x = 0;
 espec->DisplayRect.y = 0;
 espec->DisplayRect.w = 160;
 espec->DisplayRect.h = 102;

 if(espec->VideoFormatChanged)
  lynxie->DisplaySetAttributes(espec->surface->format, espec->surface->pitch32); // FIXME, pitch

 if(espec->SoundFormatChanged)
 {
  lynxie->mMikie->mikbuf.set_sample_rate(espec->SoundRate ? espec->SoundRate : 44100, 60);
  lynxie->mMikie->mikbuf.clock_rate((long int)(16000000 / 4));
  lynxie->mMikie->mikbuf.bass_freq(60);
  lynxie->mMikie->miksynth.volume(0.50);
 }

 uint16 butt_data = chee[0] | (chee[1] << 8);

 lynxie->SetButtonData(butt_data);

 MDFNMP_ApplyPeriodicCheats();

 memset(LynxLineDrawn, 0, sizeof(LynxLineDrawn[0]) * 102);

 lynxie->mMikie->mpSkipFrame = espec->skip;
 lynxie->mMikie->mpDisplayCurrent = espec->surface->pixels;
 lynxie->mMikie->mpDisplayCurrentLine = 0;
 lynxie->mMikie->startTS = gSystemCycleCount;

 while(lynxie->mMikie->mpDisplayCurrent && (gSystemCycleCount - lynxie->mMikie->startTS) < 700000)
 {
  lynxie->Update();
//  printf("%d ", gSystemCycleCount - lynxie->mMikie->startTS);
 }

 {
  // FIXME, we should integrate this into mikie.*
  uint32 color_black = espec->surface->MakeColor(30, 30, 30);

  for(int y = 0; y < 102; y++)
  {
   uint32 *row = espec->surface->pixels + y * espec->surface->pitch32;

   if(!LynxLineDrawn[y])
   {
    for(int x = 0; x < 160; x++)
     row[x] = color_black;
   }
  }
 }

 espec->MasterCycles = gSystemCycleCount - lynxie->mMikie->startTS;

 if(espec->SoundBuf)
 {
  lynxie->mMikie->mikbuf.end_frame((gSystemCycleCount - lynxie->mMikie->startTS) >> 2);
  espec->SoundBufSize = lynxie->mMikie->mikbuf.read_samples(espec->SoundBuf, espec->SoundBufMaxSize);
 }
 else
  espec->SoundBufSize = 0;
}

static void SetInput(int port, const char *type, void *ptr)
{
 chee = (uint8 *)ptr;
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
	SFARRAYN(lynxie->GetRamPointer(), RAM_SIZE, "RAM"),
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
  case MDFN_MSC_POWER:
  case MDFN_MSC_RESET: lynxie->Reset(); break;
 }
}

static MDFNSetting LynxSettings[] =
{
 { "lynx.rotateinput", MDFNSF_NOFLAGS,	gettext_noop("Virtually rotate D-pad along with screen."), NULL, MDFNST_BOOL, "1" },
 { "lynx.lowpass", MDFNSF_CAT_SOUND,	gettext_noop("Enable sound output lowpass filter."), NULL, MDFNST_BOOL, "1" },
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "a", "A (outer)", 8, IDIT_BUTTON_CAN_RAPID, NULL },

 { "b", "B (inner)", 7, IDIT_BUTTON_CAN_RAPID, NULL },

 { "option_2", "Option 2 (lower)", 5, IDIT_BUTTON_CAN_RAPID, NULL },

 { "option_1", "Option 1 (upper)", 4, IDIT_BUTTON_CAN_RAPID, NULL },


 { "left", "LEFT ←", 	/*VIRTB_DPAD0_L,*/ 2, IDIT_BUTTON, "right",		{ "up", "right", "down" } },

 { "right", "RIGHT →", 	/*VIRTB_DPAD0_R,*/ 3, IDIT_BUTTON, "left", 		{ "down", "left", "up" } },

 { "up", "UP ↑", 	/*VIRTB_DPAD0_U,*/ 0, IDIT_BUTTON, "down",		{ "right", "down", "left" } },

 { "down", "DOWN ↓", 	/*VIRTB_DPAD0_D,*/ 1, IDIT_BUTTON, "up", 		{ "left", "up", "right" } },

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
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, 0 }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".lnx", gettext_noop("Atari Lynx ROM Image") },
 { NULL, NULL }
};

MDFNGI EmulatedLynx =
{
 "lynx",
 "Atari Lynx",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 NULL,
 &InputInfo,
 Load,
 TestMagic,
 NULL,
 NULL,
 CloseGame,
 ToggleLayer,
 "",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 LynxSettings,
 MDFN_MASTERCLOCK_FIXED(16000000),
 0,

 false, // Multires possible?

 160,   // lcm_width
 102,   // lcm_height
 NULL,  // Dummy


 160,	// Nominal width
 102,	// Nominal height

 160,	// Framebuffer width
 102,	// Framebuffer height

 1,     // Number of output sound channels
};

