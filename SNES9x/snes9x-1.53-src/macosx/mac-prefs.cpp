/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#include "snes9x.h"
#include "memmap.h"
#include "blit.h"

#include <OpenGL/OpenGL.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "mac-prefix.h"
#include "mac-audio.h"
#include "mac-cart.h"
#include "mac-coreimage.h"
#include "mac-dialog.h"
#include "mac-file.h"
#include "mac-keyboard.h"
#include "mac-os.h"
#include "mac-render.h"
#include "mac-snes9x.h"
#include "mac-stringtools.h"
#include "mac-prefs.h"

enum
{
	iNibGFullScreen = 1,
	iNibGSwitchResolution,
	iNibGShowFrameRate,
	iNibGCIFilterEnable,
	iNibGTransparency,
	iNibGGL32bit,
	iNibGGLStretch,
	iNibGCoreImageFilter,
	iNibGVideoMode,
	iNibGDirectMP,
	iNibGGLVSync,
	iNibGDrawOverscan,
	iNibGScreenCurvature,
	iNibGCurvatureWarp,
	iNibGAspectRatio,
	iNibGAspectRatioText,

	iNibSSynchronize = 201,
	iNibS16BitPlayback,
	iNibSStereo,
	iNibSReverseStereo,
	iNibSPlaybackRate,
	iNibSBufferSize,
	iNibSVolume,
	iNibSInputRate,
	iNibSInputRateText,
	iNibSAllowLag,
	iNibSInterval,

	iNibOSaveFolder = 401,
	iNibOAutoSaveInterval,

	iNibMCPUCycles = 601,
	iNibMReserved1,
	iNibMTurboSkipArrows,
	iNibMTurboSkipText,
	iNibMFrameSkip,
	iNibMAllowInvalidVRAMAccess,
	iNibMAllowSpecificGameHacks,

	iNibXStartOpenDialog = 801,
	iNibXShowTimeInFrz,
	iNibXMusicBoxMode,
	iNibXEnableToggle,
	iNibXBSXBootup,
	iNibXSaveWindowPos,
	iNibXUseIPSPatch,
	iNibXOnScreenInfo,
	iNibXInactiveMode
};

enum
{
	iOpenGLBlocky = 1,
	iOpenGLTVMode,
	iOpenGLSmoothMode,
	iOpenGLBlendMode,
	iOpenGLEagleMode,
	iOpenGL2xSAIMode,
	iOpenGLSuper2xSAIMode,
	iOpenGLEPXMode,
	iOpenGLHQ2xMode,
	iOpenGLHQ3xMode,
	iOpenGLHQ4xMode,
	iOpenGLNTSC_CMode,
	iOpenGLNTSC_SMode,
	iOpenGLNTSC_RMode,
	iOpenGLNTSC_MMode,
	iOpenGLNTSC_TV_CMode,
	iOpenGLNTSC_TV_SMode,
	iOpenGLNTSC_TV_RMode,
	iOpenGLNTSC_TV_MMode
};

enum
{
	iNibSaveFolderNameMenuItem  = 5
};

static int	lastTabIndex = 1;
static int	tabList[]    = { 5, 129, 130, 131, 132, 133 };

struct PrefList
{
	OSType	itemName;
	void	*itemPointer;
	int		size;
};

static PrefList	prefList[] =
{
	{ 'flsc', &fullscreen,									sizeof(bool8      ) },
	{ 'reso', &autoRes,										sizeof(bool8      ) },
	{ 'dfps', &Settings.DisplayFrameRate,					sizeof(bool8      ) },
	{ 'tran', &Settings.Transparency,						sizeof(bool8      ) },
	{ 'gl32', &gl32bit,										sizeof(bool8      ) },
	{ 'glst', &glstretch,									sizeof(bool8      ) },
	{ 'draw', &drawingMethod,								sizeof(long       ) },
	{ 'Vmod', &videoMode,									sizeof(int        ) },
	{ 'MPmt', &multiprocessor,								sizeof(bool8      ) },
	{ 'VSNC', &vsync,										sizeof(bool8      ) },
	{ 'H239', &drawoverscan,								sizeof(bool8      ) },
	{ 'SCur', &screencurvature,								sizeof(bool8      ) },
	{ 'SCuW', &macCurvatureWarp,							sizeof(int        ) },
	{ 'ASPe', &macAspectRatio,								sizeof(int        ) },
	{ 'CIFl', &ciFilterEnable,							    sizeof(bool8      ) },

	{ 'sSyn', &Settings.SoundSync,					        sizeof(bool8      ) },
	{ 'so16', &Settings.SixteenBitSound,					sizeof(bool8      ) },
	{ 'ster', &Settings.Stereo,								sizeof(bool8      ) },
	{ 'rbst', &Settings.ReverseStereo,						sizeof(bool8      ) },
	{ 'srat', &Settings.SoundPlaybackRate,					sizeof(uint32     ) },
	{ 'InRt', &Settings.SoundInputRate,						sizeof(uint32     ) },
	{ 'MxIv', &macSoundInterval_ms,					        sizeof(uint32     ) },
	{ 'SBuf', &macSoundBuffer_ms,					        sizeof(uint32     ) },
	{ 'SLag', &macSoundLagEnable,					        sizeof(bool8      ) },
	{ 'Volm', &macSoundVolume,								sizeof(SInt32     ) },
	{ 'AUef', &aueffect,									sizeof(uint16     ) },
	{ 'AUce', &cureffect,									sizeof(int        ) },

	{ 'romf', &saveInROMFolder,								sizeof(uint8      ) },
	{ 'atsv', &Settings.AutoSaveDelay,						sizeof(int32      ) },
	{ 'RFlg', &macRecordFlag,								sizeof(uint16     ) },
	{ 'PFlg', &macPlayFlag,									sizeof(uint16     ) },
	{ 'QTfg', &macQTMovFlag,								sizeof(uint16     ) },

	{ 'HHck', &Settings.HDMATimingHack,					    sizeof(int32      ) },
	{ 'TbRt', &macFastForwardRate,							sizeof(int        ) },
	{ 'FSkp', &macFrameSkip,							    sizeof(int        ) },
	{ 'IvVR', &Settings.BlockInvalidVRAMAccessMaster,       sizeof(bool8      ) },
	{ 'GSHk', &Settings.DisableGameSpecificHacks,           sizeof(bool8      ) },

	{ 'StOp', &startopendlog,								sizeof(bool8      ) },
	{ 'STiF', &showtimeinfrz,								sizeof(bool8      ) },
	{ 'Togl', &enabletoggle,								sizeof(bool8      ) },
	{ 'SvWp', &savewindowpos,								sizeof(bool8      ) },
	{ 'OnSc', &onscreeninfo,								sizeof(bool8      ) },
	{ 'NIPS', &Settings.NoPatch,							sizeof(bool8      ) },
	{ 'BSX_', &Settings.BSXBootup,							sizeof(bool8      ) },
	{ 'MbxM', &musicboxmode,								sizeof(int        ) },
	{ 'InAc', &inactiveMode,								sizeof(int        ) },

	{ 'tab ', &lastTabIndex,								sizeof(int        ) },
	{ 'Ftab', &autofireLastTabIndex,						sizeof(int        ) },
	{ 'keyb', keyCode,										sizeof(keyCode    ) },
	{ 'pset', &padSetting,									sizeof(int        ) },
	{ 'dset', &deviceSetting,								sizeof(int        ) },
	{ 'chea', &applycheat,									sizeof(bool8      ) },

	{ 'ARec', autofireRec,									sizeof(autofireRec) },
	{ 'wPos', windowPos,									sizeof(windowPos  ) },
	{ 'wSiz', windowSize,									sizeof(windowSize ) },
	{ 'wExt', (void *) &windowExtend,						sizeof(bool8      ) },

	{ 'NPIP', npServerIP,									sizeof(npServerIP ) },
	{ 'NPNM', npName,										sizeof(npName     ) },

	{ 'L239', &lastoverscan,								sizeof(bool8      ) },

	{ 'ExT1', &(extraOptions.benchmark),					sizeof(bool8      ) },
	{ 'ExT2', &(extraOptions.glForceNoTextureRectangle),	sizeof(bool8      ) },
	{ 'ExT3', &(extraOptions.glUseClientStrageApple),		sizeof(bool8      ) },
	{ 'ExT4', &(extraOptions.glUseTexturePriority),			sizeof(bool8      ) },
	{ 'ExT5', &(extraOptions.glStorageHint),				sizeof(int        ) }
};

#define	kPrefListSize	(sizeof(prefList) / sizeof(prefList[0]))

static void SelectTabPane (HIViewRef, SInt16);
static pascal void InputRateSliderActionProc (HIViewRef, HIViewPartCode);
static pascal void LittleArrowsActionProc (HIViewRef, HIViewPartCode);
static pascal OSStatus InputRateTextEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus AspectRatioTextEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus TabEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus PreferencesEventHandler (EventHandlerCallRef, EventRef, void *);


void SavePrefs (void)
{
	CFMutableStringRef	mref;
	CFStringRef			sref;
	CFDataRef			data;

	for (unsigned int i = 0; i < kPrefListSize; i++)
	{
		mref = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("Preferences_"));
		if (mref)
		{
			sref = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *) &(prefList[i].itemName), sizeof(OSType), kCFStringEncodingMacRoman, false);
			if (sref)
			{
				CFStringAppend(mref, sref);

				data = CFDataCreate(kCFAllocatorDefault, (UInt8 *) prefList[i].itemPointer, prefList[i].size);
				if (data)
				{
					CFPreferencesSetAppValue(mref, data, kCFPreferencesCurrentApplication);
					CFRelease(data);
				}

				CFRelease(sref);
			}

			CFRelease(mref);
		}
	}

	mref = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("Preferences_SaveFolder"));
	if (mref)
	{
		if (saveFolderPath)
		{
			CFPreferencesSetAppValue(mref, saveFolderPath, kCFPreferencesCurrentApplication);
			CFRelease(saveFolderPath);
		}
		else
			CFPreferencesSetAppValue(mref, NULL, kCFPreferencesCurrentApplication);

		CFRelease(mref);
	}

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

void LoadPrefs (void)
{
	CFMutableStringRef	mref;
	CFStringRef			sref;
	CFDataRef			data;

	for (unsigned int i = 0; i < kPrefListSize; i++)
	{
		mref = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("Preferences_"));
		if (mref)
		{
			sref = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *) &(prefList[i].itemName), sizeof(OSType), kCFStringEncodingMacRoman, false);
			if (sref)
			{
				CFStringAppend(mref, sref);

				data = (CFDataRef) CFPreferencesCopyAppValue(mref, kCFPreferencesCurrentApplication);
				if (data)
				{
					if (CFDataGetLength(data) == prefList[i].size)
						CFDataGetBytes(data, CFRangeMake(0, prefList[i].size), (UInt8 *) prefList[i].itemPointer);
					CFRelease(data);
				}

				CFRelease(sref);
			}

			CFRelease(mref);
		}
	}

	mref = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("Preferences_SaveFolder"));
	if (mref)
	{
		sref = (CFStringRef) CFPreferencesCopyAppValue(mref, kCFPreferencesCurrentApplication);
		if (sref)
			saveFolderPath = sref;

		CFRelease(mref);
	}
}

void ConfigurePreferences (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;
		SInt32		oldVolume;
		uint32		oldPlaybackRate, oldInputRate, oldInterval, oldBufferSize;
		bool8		oldSynchronize, old16BitPlayback, oldStereo, oldReverseStereo, oldLagEnable;

		oldSynchronize   = Settings.SoundSync;
		old16BitPlayback = Settings.SixteenBitSound;
		oldStereo        = Settings.Stereo;
		oldReverseStereo = Settings.ReverseStereo;
		oldPlaybackRate  = Settings.SoundPlaybackRate;
		oldInputRate     = Settings.SoundInputRate;
		oldInterval      = macSoundInterval_ms;
		oldBufferSize    = macSoundBuffer_ms;
		oldLagEnable     = macSoundLagEnable;
		oldVolume        = macSoundVolume;

		if (cartOpen)
			DeinitGameWindow();

		S9xGraphicsDeinit();

		err = CreateWindowFromNib(nibRef, CFSTR("Preferences"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerUPP		tUPP, iUPP, aUPP, pUPP;
			EventHandlerRef		tRef, iRef, aRef, pRef;
			EventTypeSpec		tEvents[] = { { kEventClassControl, kEventControlHit          } },
								iEvents[] = { { kEventClassControl, kEventControlClick        } },
								aEvents[] = { { kEventClassControl, kEventControlClick        } },
								pEvents[] = { { kEventClassWindow,  kEventWindowClose         },
											  { kEventClassCommand, kEventCommandProcess      },
											  { kEventClassCommand, kEventCommandUpdateStatus } };
			ControlActionUPP	arrowsUPP, sliderUPP;
			HIViewRef			ctl, root;
			HIViewID			cid;
			MenuRef				menu;
			char				num[16];

			root = HIViewGetRoot(tWindowRef);

			cid.signature = 'tabs';
			cid.id = 128;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, lastTabIndex);
			SelectTabPane(ctl, lastTabIndex);
			tUPP = NewEventHandlerUPP(TabEventHandler);
			err = InstallControlEventHandler(ctl, tUPP, GetEventTypeCount(tEvents), tEvents, 0, &tRef);

			cid.signature = 'snd_';
			cid.id = iNibSInputRateText;
			HIViewFindByID(root, cid, &ctl);
			iUPP = NewEventHandlerUPP(InputRateTextEventHandler);
			err = InstallControlEventHandler(ctl, iUPP, GetEventTypeCount(iEvents), iEvents, 0, &iRef);

			cid.signature = 'grap';
			cid.id = iNibGAspectRatioText;
			HIViewFindByID(root, cid, &ctl);
			aUPP = NewEventHandlerUPP(AspectRatioTextEventHandler);
			err = InstallControlEventHandler(ctl, aUPP, GetEventTypeCount(aEvents), aEvents, 0, &aRef);

			pUPP = NewEventHandlerUPP(PreferencesEventHandler);
			err = InstallWindowEventHandler(tWindowRef, pUPP, GetEventTypeCount(pEvents), pEvents, (void *) tWindowRef, &pRef);

			sliderUPP = NewControlActionUPP(InputRateSliderActionProc);
			arrowsUPP = NewControlActionUPP(LittleArrowsActionProc);

			cid.signature = 'grap';

			cid.id = iNibGFullScreen;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, fullscreen);

			cid.id = iNibGSwitchResolution;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, autoRes);

			cid.id = iNibGShowFrameRate;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.DisplayFrameRate);

			cid.id = iNibGTransparency;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.Transparency);

			cid.id = iNibGGL32bit;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, gl32bit);

			cid.id = iNibGGLStretch;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, glstretch);

			cid.id = iNibGVideoMode;
			HIViewFindByID(root, cid, &ctl);
			switch (videoMode)
			{
				case VIDEOMODE_BLOCKY:
					SetControl32BitValue(ctl, iOpenGLBlocky);
					break;

				case VIDEOMODE_TV:
					SetControl32BitValue(ctl, iOpenGLTVMode);
					break;

				case VIDEOMODE_SMOOTH:
					SetControl32BitValue(ctl, iOpenGLSmoothMode);
					break;

				case VIDEOMODE_BLEND:
					SetControl32BitValue(ctl, iOpenGLBlendMode);
					break;

				case VIDEOMODE_SUPEREAGLE:
					SetControl32BitValue(ctl, iOpenGLEagleMode);
					break;

				case VIDEOMODE_2XSAI:
					SetControl32BitValue(ctl, iOpenGL2xSAIMode);
					break;

				case VIDEOMODE_SUPER2XSAI:
					SetControl32BitValue(ctl, iOpenGLSuper2xSAIMode);
					break;

				case VIDEOMODE_EPX:
					SetControl32BitValue(ctl, iOpenGLEPXMode);
					break;

				case VIDEOMODE_HQ2X:
					SetControl32BitValue(ctl, iOpenGLHQ2xMode);
					break;

				case VIDEOMODE_HQ3X:
					SetControl32BitValue(ctl, iOpenGLHQ3xMode);
					break;

				case VIDEOMODE_HQ4X:
					SetControl32BitValue(ctl, iOpenGLHQ4xMode);
					break;

				case VIDEOMODE_NTSC_C:
					SetControl32BitValue(ctl, iOpenGLNTSC_CMode);
					break;

				case VIDEOMODE_NTSC_S:
					SetControl32BitValue(ctl, iOpenGLNTSC_SMode);
					break;

				case VIDEOMODE_NTSC_R:
					SetControl32BitValue(ctl, iOpenGLNTSC_RMode);
					break;

				case VIDEOMODE_NTSC_M:
					SetControl32BitValue(ctl, iOpenGLNTSC_MMode);
					break;

				case VIDEOMODE_NTSC_TV_C:
					SetControl32BitValue(ctl, iOpenGLNTSC_TV_CMode);
					break;

				case VIDEOMODE_NTSC_TV_S:
					SetControl32BitValue(ctl, iOpenGLNTSC_TV_SMode);
					break;

				case VIDEOMODE_NTSC_TV_R:
					SetControl32BitValue(ctl, iOpenGLNTSC_TV_RMode);
					break;

				case VIDEOMODE_NTSC_TV_M:
					SetControl32BitValue(ctl, iOpenGLNTSC_TV_MMode);
					break;
			}

			cid.id = iNibGDirectMP;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, multiprocessor);

			cid.id = iNibGGLVSync;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, vsync);

			cid.id = iNibGDrawOverscan;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, drawoverscan);

			cid.id = iNibGScreenCurvature;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, screencurvature);

			cid.id = iNibGCurvatureWarp;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macCurvatureWarp);
			if (!screencurvature)
				DeactivateControl(ctl);

			cid.id = iNibGAspectRatio;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macAspectRatio);
			if (!glstretch)
				DeactivateControl(ctl);

			cid.id = iNibGCIFilterEnable;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, ciFilterEnable);
			if (systemVersion < 0x1040)
				DisableControl(ctl);

			if (systemVersion < 0x1040)
			{
				cid.id = iNibGCoreImageFilter;
				HIViewFindByID(root, cid, &ctl);
				DisableControl(ctl);
			}

			cid.signature = 'snd_';

			cid.id = iNibSSynchronize;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.SoundSync);

			cid.id = iNibS16BitPlayback;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.SixteenBitSound);

			cid.id = iNibSStereo;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.Stereo);

			cid.id = iNibSReverseStereo;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.ReverseStereo);
			if (!Settings.Stereo)
				DeactivateControl(ctl);

			cid.id = iNibSPlaybackRate;
			HIViewFindByID(root, cid, &ctl);
			switch (Settings.SoundPlaybackRate)
			{
				case 48000:
					SetControl32BitValue(ctl, 1);
					break;

				case 44100:
					SetControl32BitValue(ctl, 2);
					break;

				case 35000:
					SetControl32BitValue(ctl, 3);
					break;

				case 32000:
					SetControl32BitValue(ctl, 4);
					break;

				case 30000:
					SetControl32BitValue(ctl, 5);
					break;

				case 22050:
					SetControl32BitValue(ctl, 6);
					break;

				case 16000:
					SetControl32BitValue(ctl, 7);
					break;

				case 11025:
					SetControl32BitValue(ctl, 8);
					break;

				case 8000:
					SetControl32BitValue(ctl, 9);
					break;
			}

			cid.id = iNibSInputRate;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.SoundInputRate);
			SetControlAction(ctl, sliderUPP);

			cid.id = iNibSInputRateText;
			HIViewFindByID(root, cid, &ctl);
			sprintf(num, "%d", Settings.SoundInputRate);
			SetStaticTextCStr(ctl, num, false);

			cid.id = iNibSInterval;
			HIViewFindByID(root, cid, &ctl);
			switch (macSoundInterval_ms)
			{
				case 8:
					SetControl32BitValue(ctl, 1);
					break;

				case 16:
					SetControl32BitValue(ctl, 2);
					break;

				case 32:
					SetControl32BitValue(ctl, 3);
					break;

				case 64:
					SetControl32BitValue(ctl, 4);
					break;

				case 0:
				default:
					SetControl32BitValue(ctl, 6);
					break;
			}

			cid.id = iNibSBufferSize;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macSoundBuffer_ms / 20);

			cid.id = iNibSAllowLag;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macSoundLagEnable);

			cid.id = iNibSVolume;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macSoundVolume);

			cid.signature = 'othe';

			cid.id = iNibOSaveFolder;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, saveInROMFolder + 1);
			err = GetControlData(ctl, kControlMenuPart, kControlPopupButtonMenuRefTag, sizeof(MenuRef), &menu, NULL);
			if (saveFolderPath)
			{
				CFURLRef	url;
				CFStringRef	ref;

				url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, saveFolderPath, kCFURLPOSIXPathStyle, true);
				ref = CFURLCopyLastPathComponent(url);
				err = SetMenuItemTextWithCFString(menu, iNibSaveFolderNameMenuItem, ref);
				CFRelease(ref);
				CFRelease(url);
				EnableMenuItem(menu, iNibSaveFolderNameMenuItem);
			}
			else
			{
				CFStringRef	ref;

				ref = CFCopyLocalizedString(CFSTR("NoneSelected"), "NoneSelected");
				err = SetMenuItemTextWithCFString(menu, iNibSaveFolderNameMenuItem, ref);
				CFRelease(ref);
				DisableMenuItem(menu, iNibSaveFolderNameMenuItem);
			}

			cid.id = iNibOAutoSaveInterval;
			HIViewFindByID(root, cid, &ctl);
			sprintf(num, "%d", Settings.AutoSaveDelay);
			SetEditTextCStr(ctl, num, false);

			cid.signature = 'msc2';

			cid.id = iNibMCPUCycles;
			HIViewFindByID(root, cid, &ctl);
			sprintf(num, "%" PRIi32, Settings.HDMATimingHack);
			SetEditTextCStr(ctl, num, false);

			cid.id = iNibMTurboSkipArrows;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macFastForwardRate);
			SetControlAction(ctl, arrowsUPP);

			cid.id = iNibMTurboSkipText;
			HIViewFindByID(root, cid, &ctl);
			sprintf(num, "%d", macFastForwardRate);
			SetStaticTextCStr(ctl, num, false);

			cid.id = iNibMFrameSkip;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, macFrameSkip + 2);

			cid.id = iNibMAllowInvalidVRAMAccess;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, !Settings.BlockInvalidVRAMAccessMaster);

			cid.id = iNibMAllowSpecificGameHacks;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, !Settings.DisableGameSpecificHacks);

			cid.signature = 'osx_';

			cid.id = iNibXStartOpenDialog;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, startopendlog);

			cid.id = iNibXShowTimeInFrz;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, showtimeinfrz);

			cid.id = iNibXMusicBoxMode;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, musicboxmode);

			cid.id = iNibXEnableToggle;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, enabletoggle);

			cid.id = iNibXSaveWindowPos;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, savewindowpos);

			cid.id = iNibXUseIPSPatch;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, !Settings.NoPatch);

			cid.id = iNibXOnScreenInfo;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, onscreeninfo);

			cid.id = iNibXInactiveMode;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, inactiveMode);

			cid.id = iNibXBSXBootup;
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, Settings.BSXBootup);

			MoveWindowPosition(tWindowRef, kWindowPreferences, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowPreferences);

			cid.signature = 'grap';

			cid.id = iNibGFullScreen;
			HIViewFindByID(root, cid, &ctl);
			fullscreen = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGSwitchResolution;
			HIViewFindByID(root, cid, &ctl);
			autoRes = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGShowFrameRate;
			HIViewFindByID(root, cid, &ctl);
			Settings.DisplayFrameRate = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGTransparency;
			HIViewFindByID(root, cid, &ctl);
			Settings.Transparency = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGGL32bit;
			HIViewFindByID(root, cid, &ctl);
			gl32bit = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGGLStretch;
			HIViewFindByID(root, cid, &ctl);
			glstretch = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGVideoMode;
			HIViewFindByID(root, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case iOpenGLBlocky:
					drawingMethod = kDrawingOpenGL;
					videoMode = VIDEOMODE_BLOCKY;
					break;

				case iOpenGLTVMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_TV;
					break;

				case iOpenGLSmoothMode:
					drawingMethod = kDrawingOpenGL;
					videoMode = VIDEOMODE_SMOOTH;
					break;

				case iOpenGLBlendMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_BLEND;
					break;

				case iOpenGLEagleMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_SUPEREAGLE;
					break;

				case iOpenGL2xSAIMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_2XSAI;
					break;

				case iOpenGLSuper2xSAIMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_SUPER2XSAI;
					break;

				case iOpenGLEPXMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_EPX;
					break;

				case iOpenGLHQ2xMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_HQ2X;
					break;

				case iOpenGLHQ3xMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_HQ3X;
					break;

				case iOpenGLHQ4xMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_HQ4X;
					break;

				case iOpenGLNTSC_CMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_C;
					S9xBlitNTSCFilterSet(&snes_ntsc_composite);
					break;

				case iOpenGLNTSC_SMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_S;
					S9xBlitNTSCFilterSet(&snes_ntsc_svideo);
					break;

				case iOpenGLNTSC_RMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_R;
					S9xBlitNTSCFilterSet(&snes_ntsc_rgb);
					break;

				case iOpenGLNTSC_MMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_M;
					S9xBlitNTSCFilterSet(&snes_ntsc_monochrome);
					break;

				case iOpenGLNTSC_TV_CMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_TV_C;
					S9xBlitNTSCFilterSet(&snes_ntsc_composite);
					break;

				case iOpenGLNTSC_TV_SMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_TV_S;
					S9xBlitNTSCFilterSet(&snes_ntsc_svideo);
					break;

				case iOpenGLNTSC_TV_RMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_TV_R;
					S9xBlitNTSCFilterSet(&snes_ntsc_rgb);
					break;

				case iOpenGLNTSC_TV_MMode:
					drawingMethod = kDrawingBlitGL;
					videoMode = VIDEOMODE_NTSC_TV_M;
					S9xBlitNTSCFilterSet(&snes_ntsc_monochrome);
					break;
			}

			cid.id = iNibGDirectMP;
			HIViewFindByID(root, cid, &ctl);
			multiprocessor = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGGLVSync;
			HIViewFindByID(root, cid, &ctl);
			vsync = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGDrawOverscan;
			HIViewFindByID(root, cid, &ctl);
			drawoverscan = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGScreenCurvature;
			HIViewFindByID(root, cid, &ctl);
			screencurvature = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibGCurvatureWarp;
			HIViewFindByID(root, cid, &ctl);
			macCurvatureWarp = GetControl32BitValue(ctl);

			cid.id = iNibGAspectRatio;
			HIViewFindByID(root, cid, &ctl);
			macAspectRatio = GetControl32BitValue(ctl);

			cid.id = iNibGCIFilterEnable;
			HIViewFindByID(root, cid, &ctl);
			ciFilterEnable = GetControl32BitValue(ctl) ? true : false;

			cid.signature = 'snd_';

			cid.id = iNibSSynchronize;
			HIViewFindByID(root, cid, &ctl);
			Settings.SoundSync = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibS16BitPlayback;
			HIViewFindByID(root, cid, &ctl);
			Settings.SixteenBitSound = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibSStereo;
			HIViewFindByID(root, cid, &ctl);
			Settings.Stereo = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibSReverseStereo;
			HIViewFindByID(root, cid, &ctl);
			Settings.ReverseStereo = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibSPlaybackRate;
			HIViewFindByID(root, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case 1:
					Settings.SoundPlaybackRate = 48000;
					break;

				case 2:
					Settings.SoundPlaybackRate = 44100;
					break;

				case 3:
					Settings.SoundPlaybackRate = 35000;
					break;

				case 4:
					Settings.SoundPlaybackRate = 32000;
					break;

				case 5:
					Settings.SoundPlaybackRate = 30000;
					break;

				case 6:
					Settings.SoundPlaybackRate = 22050;
					break;

				case 7:
					Settings.SoundPlaybackRate = 16000;
					break;

				case 8:
					Settings.SoundPlaybackRate = 11025;
					break;

				case 9:
					Settings.SoundPlaybackRate = 8000;
					break;
			}

			cid.id = iNibSInputRate;
			HIViewFindByID(root, cid, &ctl);
			Settings.SoundInputRate = GetControl32BitValue(ctl);

			cid.id = iNibSInterval;
			HIViewFindByID(root, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case 1:
					macSoundInterval_ms = 8;
					break;

				case 2:
					macSoundInterval_ms = 16;
					break;

				case 3:
					macSoundInterval_ms = 32;
					break;

				case 4:
					macSoundInterval_ms = 64;
					break;

				case 6:
				default:
					macSoundInterval_ms = 0;
					break;
			}

			cid.id = iNibSBufferSize;
			HIViewFindByID(root, cid, &ctl);
			macSoundBuffer_ms = GetControl32BitValue(ctl) * 20;

			cid.id = iNibSAllowLag;
			HIViewFindByID(root, cid, &ctl);
			macSoundLagEnable = GetControl32BitValue(ctl);

			cid.id = iNibSVolume;
			HIViewFindByID(root, cid, &ctl);
			macSoundVolume = GetControl32BitValue(ctl);

			cid.signature = 'othe';

			cid.id = iNibOSaveFolder;
			HIViewFindByID(root, cid, &ctl);
			saveInROMFolder = GetControl32BitValue(ctl) - 1;

			cid.id = iNibOAutoSaveInterval;
			HIViewFindByID(root, cid, &ctl);
			GetEditTextCStr(ctl, num);
			Settings.AutoSaveDelay = atoi(num);

			cid.signature = 'msc2';

			cid.id = iNibMCPUCycles;
			HIViewFindByID(root, cid, &ctl);
			GetEditTextCStr(ctl, num);
			Settings.HDMATimingHack = atoi(num);
			if ((Settings.HDMATimingHack <= 0) || (Settings.HDMATimingHack >= 200))
				Settings.HDMATimingHack = 100;

			cid.id = iNibMTurboSkipArrows;
			HIViewFindByID(root, cid, &ctl);
			macFastForwardRate = GetControl32BitValue(ctl);

			cid.id = iNibMFrameSkip;
			HIViewFindByID(root, cid, &ctl);
			macFrameSkip = GetControl32BitValue(ctl) - 2;

			cid.id = iNibMAllowInvalidVRAMAccess;
			HIViewFindByID(root, cid, &ctl);
			Settings.BlockInvalidVRAMAccessMaster = GetControl32BitValue(ctl) ? false : true;

			cid.id = iNibMAllowSpecificGameHacks;
			HIViewFindByID(root, cid, &ctl);
			Settings.DisableGameSpecificHacks = GetControl32BitValue(ctl) ? false : true;

			cid.signature = 'osx_';

			cid.id = iNibXStartOpenDialog;
			HIViewFindByID(root, cid, &ctl);
			startopendlog = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibXShowTimeInFrz;
			HIViewFindByID(root, cid, &ctl);
			showtimeinfrz = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibXMusicBoxMode;
			HIViewFindByID(root, cid, &ctl);
			musicboxmode = GetControl32BitValue(ctl);

			cid.id = iNibXEnableToggle;
			HIViewFindByID(root, cid, &ctl);
			enabletoggle = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibXSaveWindowPos;
			HIViewFindByID(root, cid, &ctl);
			savewindowpos = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibXUseIPSPatch;
			HIViewFindByID(root, cid, &ctl);
			Settings.NoPatch = GetControl32BitValue(ctl) ? false : true;

			cid.id = iNibXOnScreenInfo;
			HIViewFindByID(root, cid, &ctl);
			onscreeninfo = GetControl32BitValue(ctl) ? true : false;

			cid.id = iNibXInactiveMode;
			HIViewFindByID(root, cid, &ctl);
			inactiveMode = GetControl32BitValue(ctl);

			cid.id = iNibXBSXBootup;
			HIViewFindByID(root, cid, &ctl);
			Settings.BSXBootup = GetControl32BitValue(ctl) ? true : false;

			DisposeControlActionUPP(arrowsUPP);
			DisposeControlActionUPP(sliderUPP);

			err = RemoveEventHandler(pRef);
			DisposeEventHandlerUPP(pUPP);

			err = RemoveEventHandler(aRef);
			DisposeEventHandlerUPP(aUPP);

			err = RemoveEventHandler(iRef);
			DisposeEventHandlerUPP(iUPP);

			err = RemoveEventHandler(tRef);
			DisposeEventHandlerUPP(tUPP);

			CFRelease(tWindowRef);
		}

		DisposeNibReference(nibRef);

		S9xGraphicsInit();

		if (((oldSynchronize   != Settings.SoundSync        ) ||
			 (old16BitPlayback != Settings.SixteenBitSound  ) ||
			 (oldStereo        != Settings.Stereo           ) ||
			 (oldReverseStereo != Settings.ReverseStereo    ) ||
			 (oldPlaybackRate  != Settings.SoundPlaybackRate) ||
			 (oldInputRate     != Settings.SoundInputRate   ) ||
			 (oldInterval      != macSoundInterval_ms       ) ||
			 (oldBufferSize    != macSoundBuffer_ms         ) ||
			 (oldLagEnable     != macSoundLagEnable         ) ||
			 (oldVolume        != macSoundVolume            )) && cartOpen)
			SNES9X_InitSound();

		if (!fullscreen && cartOpen)
		{
			InitGameWindow();
			ShowWindow(gWindow);
		}

		if (cartOpen)
		{
			FSRef	ref;

			err = FSPathMakeRef((unsigned char *) Memory.ROMFilename, &ref, NULL);
			if (err == noErr)
				CheckSaveFolder(&ref);
		}
	}
}

static void SelectTabPane (HIViewRef tabControl, SInt16 index)
{
	HIViewRef	sup, userPane, selectedPane = NULL;
	HIViewID	cid;

	lastTabIndex = index;

	sup = HIViewGetSuperview(tabControl);
	cid.signature = 'tabs';

	for (int i = 1; i < tabList[0] + 1; i++)
	{
		cid.id = tabList[i];
		HIViewFindByID(sup, cid, &userPane);

		if (i == index)
			selectedPane = userPane;
		else
			HIViewSetVisible(userPane, false);
	}

	if (selectedPane != NULL)
		HIViewSetVisible(selectedPane, true);

	HIViewSetNeedsDisplay(tabControl, true);
}

static pascal void InputRateSliderActionProc (HIViewRef slider, HIViewPartCode partCode)
{
	HIViewRef	ctl;
	HIViewID	cid;
	char		num[10];

	cid.signature = 'snd_';
	cid.id = iNibSInputRateText;
	HIViewFindByID(HIViewGetSuperview(slider), cid, &ctl);

	sprintf(num, "%ld", GetControl32BitValue(slider));
	SetStaticTextCStr(ctl, num, true);
}

static pascal void LittleArrowsActionProc (HIViewRef arrows, HIViewPartCode partCode)
{
	HIViewRef	ctl;
	HIViewID	cid = { 'msc2', iNibMTurboSkipText };
	char		num[8];

	if (partCode == kControlUpButtonPart)
		SetControl32BitValue(arrows, GetControl32BitValue(arrows) + 1);
	else
	if (partCode == kControlDownButtonPart)
		SetControl32BitValue(arrows, GetControl32BitValue(arrows) - 1);

	HIViewFindByID(HIViewGetSuperview(arrows), cid, &ctl);
	sprintf(num, "%ld", GetControl32BitValue(arrows));
	SetStaticTextCStr(ctl, num, true);
}

static pascal OSStatus InputRateTextEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	HIViewRef	ctl, slider;
	HIViewID	cid;
	SInt32		value;
	char		num[10];

	err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ctl);
	if (err == noErr)
	{
		cid.signature = 'snd_';
		cid.id = iNibSInputRate;
		HIViewFindByID(HIViewGetSuperview(ctl), cid, &slider);
		value = GetControl32BitValue(slider);

		value /= 50;
		value *= 50;
		if (value > 33000)
			value = 33000;
		if (value < 31000)
			value = 31000;

		SetControl32BitValue(slider, value);
		sprintf(num, "%ld", value);
		SetEditTextCStr(ctl, num, true);

		result = noErr;
	}

	return (result);
}

static pascal OSStatus AspectRatioTextEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	HIViewRef	ctl, slider;
	HIViewID	cid;
	float		w, h, v;
	int			iw, ih;

	err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ctl);
	if (err == noErr)
	{
		cid.signature = 'grap';
		cid.id = iNibGAspectRatio;
		HIViewFindByID(HIViewGetSuperview(ctl), cid, &slider);

		GetGameDisplay(&iw, &ih);
		w = (float) iw;
		h = (float) ih;

		v = (float) SNES_WIDTH / (float) SNES_HEIGHT * h;
		macAspectRatio = (int) (((4.0f / 3.0f) * h - v) / (w - v) * 10000.0f);

		SetControl32BitValue(slider, macAspectRatio);

		result = noErr;
	}

	return (result);
}

static pascal OSStatus TabEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	HIViewRef	ctl;
	HIViewID	cid;
	SInt32		value;

	err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ctl);
	if (err == noErr)
	{
		GetControlID(ctl, &cid);
		value = GetControl32BitValue(ctl);

		if ((cid.id == 128) && (value != lastTabIndex))
		{
			SelectTabPane(ctl, value);
			result = noErr;
		}
	}

	return (result);
}

static pascal OSStatus PreferencesEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef = (WindowRef) inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
		{
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
				{
					QuitAppModalLoopForWindow(tWindowRef);
					result = noErr;
					break;
				}
			}

			break;
		}

		case kEventClassCommand:
		{
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(true);
						result = noErr;
					}

					break;
				}

				case kEventCommandProcess:
				{
					HIViewRef	ctl, root;
					HIViewID	cid;
					SInt32		value;
					FSRef		ref;
					bool8		r;

					root = HIViewGetRoot(tWindowRef);

					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						switch (tHICommand.commandID)
						{
							case 'S_EF':
							{
								HideWindow(tWindowRef);
								ConfigureSoundEffects();
								ShowWindow(tWindowRef);

								result = noErr;
								break;
							}

							case 'G_FL':
							{
								if (systemVersion >= 0x1040)
								{
									HideWindow(tWindowRef);
									ConfigureCoreImageFilter();
									ShowWindow(tWindowRef);
								}

								result = noErr;
								break;
							}

							case 'G__7':
							{
								cid.signature = 'grap';
								cid.id = iNibGGLStretch;
								HIViewFindByID(root, cid, &ctl);
								value = GetControl32BitValue(ctl);

								cid.id = iNibGAspectRatio;
								HIViewFindByID(root, cid, &ctl);
								if (value)
									ActivateControl(ctl);
								else
									DeactivateControl(ctl);

								result = noErr;
								break;
							}

							case 'G_13':
							{
								cid.signature = 'grap';
								cid.id = iNibGScreenCurvature;
								HIViewFindByID(root, cid, &ctl);
								value = GetControl32BitValue(ctl);

								cid.id = iNibGCurvatureWarp;
								HIViewFindByID(root, cid, &ctl);
								if (value)
									ActivateControl(ctl);
								else
									DeactivateControl(ctl);

								result = noErr;
								break;
							}

							case 'S__3':
							{
								cid.signature = 'snd_';
								cid.id = iNibSStereo;
								HIViewFindByID(root, cid, &ctl);
								value = GetControl32BitValue(ctl);

								cid.id = iNibSReverseStereo;
								HIViewFindByID(root, cid, &ctl);
								if (value)
									ActivateControl(ctl);
								else
									DeactivateControl(ctl);

								result = noErr;
								break;
							}

							case 'F_FL':
							{
								UInt32	modifierkey;

								err = GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifierkey);
								if (err == noErr)
								{
									if (modifierkey & optionKey)
									{
										CFStringRef	str;
										MenuRef		menu;

										str = CFCopyLocalizedString(CFSTR("NoneSelected"), "NoneSelected");

										cid.signature = 'othe';
										cid.id = iNibOSaveFolder;
										HIViewFindByID(root, cid, &ctl);
										SetControl32BitValue(ctl, 3);
										err = GetControlData(ctl, kControlMenuPart, kControlPopupButtonMenuRefTag, sizeof(MenuRef), &menu, NULL);
										err = SetMenuItemTextWithCFString(menu, iNibSaveFolderNameMenuItem, str);
										DisableMenuItem(menu, iNibSaveFolderNameMenuItem);
										HIViewSetNeedsDisplay(ctl, true);

										CFRelease(str);

										if (saveFolderPath)
											CFRelease(saveFolderPath);
										saveFolderPath = NULL;
									}
									else
										r = NavBeginChooseFolderSheet(tWindowRef);
								}

								result = noErr;
								break;
							}

							case 'NvDn':
							{
								r = NavEndChooseFolderSheet(&ref);
								if (r)
								{
									CFStringRef	str;
									CFURLRef	url;
									MenuRef		menu;

									url = CFURLCreateFromFSRef(kCFAllocatorDefault, &ref);
									str = CFURLCopyLastPathComponent(url);

									cid.signature = 'othe';
									cid.id = iNibOSaveFolder;
									HIViewFindByID(root, cid, &ctl);
									SetControl32BitValue(ctl, iNibSaveFolderNameMenuItem);
									err = GetControlData(ctl, kControlMenuPart, kControlPopupButtonMenuRefTag, sizeof(MenuRef), &menu, NULL);
									err = SetMenuItemTextWithCFString(menu, iNibSaveFolderNameMenuItem, str);
									EnableMenuItem(menu, iNibSaveFolderNameMenuItem);
									HIViewSetNeedsDisplay(ctl, true);

									CFRelease(str);

									str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
									if (saveFolderPath)
										CFRelease(saveFolderPath);
									saveFolderPath = str;

									CFRelease(url);
								}

								result = noErr;
								break;
							}
						}
					}

					break;
				}
			}

			break;
		}
	}

	return (result);
}

void ConfigureExtraOptions (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	tWindowRef;

		err = CreateWindowFromNib(nibRef, CFSTR("ExtraOptions"), &tWindowRef);
		if (err == noErr)
		{
			EventHandlerRef	eref;
			EventHandlerUPP	eventUPP;
			EventTypeSpec	windowEvents[] = { { kEventClassWindow,  kEventWindowClose         },
											   { kEventClassCommand, kEventCommandUpdateStatus } };
			HIViewRef		ctl, root;
			HIViewID		cid;

			root = HIViewGetRoot(tWindowRef);
			cid.id = 0;

			cid.signature = 'BMrk';
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, extraOptions.benchmark);

			cid.signature = 'NoTR';
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, extraOptions.glForceNoTextureRectangle);

			cid.signature = 'CSAp';
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, extraOptions.glUseClientStrageApple);

			cid.signature = 'AGPT';
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, extraOptions.glUseTexturePriority);

			cid.signature = 'Hint';
			HIViewFindByID(root, cid, &ctl);
			SetControl32BitValue(ctl, extraOptions.glStorageHint);

			eventUPP = NewEventHandlerUPP(DefaultEventHandler);
			err = InstallWindowEventHandler(tWindowRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) tWindowRef, &eref);

			MoveWindowPosition(tWindowRef, kWindowExtra, false);
			ShowWindow(tWindowRef);
			err = RunAppModalLoopForWindow(tWindowRef);
			HideWindow(tWindowRef);
			SaveWindowPosition(tWindowRef, kWindowExtra);

			cid.signature = 'BMrk';
			HIViewFindByID(root, cid, &ctl);
			extraOptions.benchmark = GetControl32BitValue(ctl) ? true : false;

			cid.signature = 'NoTR';
			HIViewFindByID(root, cid, &ctl);
			extraOptions.glForceNoTextureRectangle = GetControl32BitValue(ctl) ? true : false;

			cid.signature = 'CSAp';
			HIViewFindByID(root, cid, &ctl);
			extraOptions.glUseClientStrageApple = GetControl32BitValue(ctl) ? true : false;

			cid.signature = 'AGPT';
			HIViewFindByID(root, cid, &ctl);
			extraOptions.glUseTexturePriority = GetControl32BitValue(ctl) ? true : false;

			cid.signature = 'Hint';
			HIViewFindByID(root, cid, &ctl);
			extraOptions.glStorageHint = GetControl32BitValue(ctl);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eventUPP);

			CFRelease(tWindowRef);
		}

		DisposeNibReference(nibRef);
	}
}
