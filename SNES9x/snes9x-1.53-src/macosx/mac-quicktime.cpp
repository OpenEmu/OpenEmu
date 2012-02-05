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
#include "apu.h"

#include <QuickTime/QuickTime.h>

#include "mac-prefix.h"
#include "mac-gworld.h"
#include "mac-os.h"
#include "mac-screenshot.h"
#include "mac-quicktime.h"

#define	kMovDoubleSize		(1 << 0)
#define	kMovExtendedHeight	(1 << 1)

static void CheckError (OSStatus, int);
static void MacQTOpenVideoComponent (ComponentInstance *);
static void MacQTCloseVideoComponent (ComponentInstance);
static OSStatus WriteFrameCallBack (void *, ICMCompressionSessionRef, OSStatus, ICMEncodedFrameRef, void *);

typedef struct
{
	Movie							movie;
	Track							vTrack, sTrack;
	Media							vMedia, sMedia;
	ComponentInstance				vci;
	SoundDescriptionHandle			soundDesc;
	DataHandler						dataHandler;
	Handle							soundBuffer;
	Handle							dataRef;
	OSType							dataRefType;
	CVPixelBufferPoolRef			pool;
	ICMCompressionSessionRef		session;
	ICMCompressionSessionOptionsRef	option;
	CGImageRef						srcImage;
	TimeValue64						timeStamp;
	long							keyFrame, keyFrameCount;
	long							frameSkip, frameSkipCount;
	int								width, height;
	int								soundBufferSize;
	int								samplesPerSec;
}	MacQTState;

static MacQTState	sqt;


static void CheckError (OSStatus err, int n)
{
	if (err != noErr)
	{
		char	mes[32];

		sprintf(mes, "quicktime %02d", n);
		QuitWithFatalError(err, mes);
	}
}

static void MacQTOpenVideoComponent (ComponentInstance *rci)
{
	OSStatus			err;
	ComponentInstance	ci;
	CFDataRef			data;

	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);

	data = (CFDataRef) CFPreferencesCopyAppValue(CFSTR("QTVideoSetting"), kCFPreferencesCurrentApplication);
	if (data)
	{
		CFIndex	len;
		Handle	hdl;

		len = CFDataGetLength(data);
		hdl = NewHandleClear((Size) len);
		if (MemError() == noErr)
		{
			HLock(hdl);
			CFDataGetBytes(data, CFRangeMake(0, len), (unsigned char *) *hdl);
			err = SCSetInfo(ci, scSettingsStateType, &hdl);

			DisposeHandle(hdl);
		}

		CFRelease(data);
	}
	else
	{
		SCSpatialSettings	ss;
		SCTemporalSettings	ts;

		ss.codecType       = kAnimationCodecType;
		ss.codec           = 0;
		ss.depth           = 16;
		ss.spatialQuality  = codecMaxQuality;
		err = SCSetInfo(ci, scSpatialSettingsType, &ss);

		ts.frameRate       = FixRatio(Memory.ROMFramesPerSecond, 1);
		ts.keyFrameRate    = Memory.ROMFramesPerSecond;
		ts.temporalQuality = codecMaxQuality;
		err = SCSetInfo(ci, scTemporalSettingsType, &ts);
	}

	*rci = ci;
}

static void MacQTCloseVideoComponent (ComponentInstance ci)
{
	OSStatus	err;

	err = CloseComponent(ci);
}

void MacQTVideoConfig (void)
{
	OSStatus			err;
	ComponentInstance	ci;

	MacQTOpenVideoComponent(&ci);

	long	flag;
	flag = scListEveryCodec | scAllowZeroKeyFrameRate | scDisableFrameRateItem | scAllowEncodingWithCompressionSession;
	err = SCSetInfo(ci, scPreferenceFlagsType, &flag);

	SCWindowSettings	ws;
	ws.size          = sizeof(SCWindowSettings);
	ws.windowRefKind = scWindowRefKindCarbon;
	ws.parentWindow  = NULL;
	err = SCSetInfo(ci, scWindowOptionsType, &ws);

	err = SCRequestSequenceSettings(ci);
	if (err == noErr)
	{
		CFDataRef	data;
		Handle		hdl;

		err = SCGetInfo(ci, scSettingsStateType, &hdl);
		if (err == noErr)
		{
			HLock(hdl);
			data = CFDataCreate(kCFAllocatorDefault, (unsigned char *) *hdl, GetHandleSize(hdl));
			if (data)
			{
				CFPreferencesSetAppValue(CFSTR("QTVideoSetting"), data, kCFPreferencesCurrentApplication);
				CFRelease(data);
			}

			DisposeHandle(hdl);
		}
	}

	MacQTCloseVideoComponent(ci);
}

void MacQTStartRecording (char *path)
{
	OSStatus	err;
	CFStringRef str;
	CFURLRef	url;

	memset(&sqt, 0, sizeof(sqt));

	// storage

	str = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
	url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str, kCFURLPOSIXPathStyle, false);
	err = QTNewDataReferenceFromCFURL(url, 0, &(sqt.dataRef), &(sqt.dataRefType));
	CheckError(err, 21);
	CFRelease(url);
	CFRelease(str);

	err = CreateMovieStorage(sqt.dataRef, sqt.dataRefType, 'TVOD', smSystemScript, createMovieFileDeleteCurFile | newMovieActive, &(sqt.dataHandler), &(sqt.movie));
	CheckError(err, 22);

	// video

	MacQTOpenVideoComponent(&(sqt.vci));

	long				flag;
	SCTemporalSettings	ts;

	flag = scAllowEncodingWithCompressionSession;
	err = SCSetInfo(sqt.vci, scPreferenceFlagsType, &flag);

	err = SCGetInfo(sqt.vci, scTemporalSettingsType, &ts);
	ts.frameRate = FixRatio(Memory.ROMFramesPerSecond, 1);
	if (ts.keyFrameRate < 1)
		ts.keyFrameRate = Memory.ROMFramesPerSecond;
	sqt.keyFrame  = sqt.keyFrameCount  = ts.keyFrameRate;
	sqt.frameSkip = sqt.frameSkipCount = (macQTMovFlag & 0xFF00) >> 8;
	err = SCSetInfo(sqt.vci, scTemporalSettingsType, &ts);

	sqt.width  = ((macQTMovFlag & kMovDoubleSize) ? 2 : 1) * SNES_WIDTH;
	sqt.height = ((macQTMovFlag & kMovDoubleSize) ? 2 : 1) * ((macQTMovFlag & kMovExtendedHeight) ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT);

	sqt.srcImage = NULL;
	sqt.timeStamp = 0;

	SCSpatialSettings			ss;
	ICMEncodedFrameOutputRecord	record;
	ICMMultiPassStorageRef		nullStorage = NULL;

	err = SCCopyCompressionSessionOptions(sqt.vci, &(sqt.option));
	CheckError(err, 61);
	err = ICMCompressionSessionOptionsSetProperty(sqt.option, kQTPropertyClass_ICMCompressionSessionOptions, kICMCompressionSessionOptionsPropertyID_MultiPassStorage, sizeof(ICMMultiPassStorageRef), &nullStorage);

	record.encodedFrameOutputCallback = WriteFrameCallBack;
	record.encodedFrameOutputRefCon   = NULL;
	record.frameDataAllocator         = NULL;
	err = SCGetInfo(sqt.vci, scSpatialSettingsType, &ss);
	err = ICMCompressionSessionCreate(kCFAllocatorDefault, sqt.width, sqt.height, ss.codecType, Memory.ROMFramesPerSecond, sqt.option, NULL, &record, &(sqt.session));
	CheckError(err, 62);

	CFMutableDictionaryRef	dic;
	CFNumberRef				val;
	OSType					pix = k16BE555PixelFormat;
	int						row = sqt.width * 2;

	dic = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	val = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pix);
	CFDictionaryAddValue(dic, kCVPixelBufferPixelFormatTypeKey, val);
	CFRelease(val);

	val = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &(sqt.width));
	CFDictionaryAddValue(dic, kCVPixelBufferWidthKey, val);
	CFRelease(val);

	val = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &(sqt.height));
	CFDictionaryAddValue(dic, kCVPixelBufferHeightKey, val);
	CFRelease(val);

	val = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &row);
	CFDictionaryAddValue(dic, kCVPixelBufferBytesPerRowAlignmentKey, val);
	CFRelease(val);

	CFDictionaryAddValue(dic, kCVPixelBufferCGImageCompatibilityKey, kCFBooleanTrue);
	CFDictionaryAddValue(dic, kCVPixelBufferCGBitmapContextCompatibilityKey, kCFBooleanTrue);

	err = CVPixelBufferPoolCreate(kCFAllocatorDefault, NULL, dic, &(sqt.pool));
	CheckError(err, 63);

	CFRelease(dic);

	sqt.vTrack = NewMovieTrack(sqt.movie, FixRatio(sqt.width, 1), FixRatio(sqt.height, 1), kNoVolume);
	CheckError(GetMoviesError(), 23);

	sqt.vMedia = NewTrackMedia(sqt.vTrack, VideoMediaType, Memory.ROMFramesPerSecond, NULL, 0);
	CheckError(GetMoviesError(), 24);

	err = BeginMediaEdits(sqt.vMedia);
	CheckError(err, 25);

	// sound

	sqt.soundDesc = (SoundDescriptionHandle) NewHandleClear(sizeof(SoundDescription));
	CheckError(MemError(), 26);

	(**sqt.soundDesc).descSize    = sizeof(SoundDescription);
#ifdef __BIG_ENDIAN__
	(**sqt.soundDesc).dataFormat  = Settings.SixteenBitSound ? k16BitBigEndianFormat    : k8BitOffsetBinaryFormat;
#else
	(**sqt.soundDesc).dataFormat  = Settings.SixteenBitSound ? k16BitLittleEndianFormat : k8BitOffsetBinaryFormat;
#endif
	(**sqt.soundDesc).numChannels = Settings.Stereo ? 2 : 1;
	(**sqt.soundDesc).sampleSize  = Settings.SixteenBitSound ? 16 : 8;
	(**sqt.soundDesc).sampleRate  = (UnsignedFixed) FixRatio(Settings.SoundPlaybackRate, 1);

	sqt.samplesPerSec = Settings.SoundPlaybackRate / Memory.ROMFramesPerSecond;

	sqt.soundBufferSize = sqt.samplesPerSec;
	if (Settings.SixteenBitSound)
		sqt.soundBufferSize <<= 1;
	if (Settings.Stereo)
		sqt.soundBufferSize <<= 1;

	sqt.soundBuffer = NewHandleClear(sqt.soundBufferSize);
	CheckError(MemError(), 27);
	HLock(sqt.soundBuffer);

	sqt.sTrack = NewMovieTrack(sqt.movie, 0, 0, kFullVolume);
	CheckError(GetMoviesError(), 28);

	sqt.sMedia = NewTrackMedia(sqt.sTrack, SoundMediaType, Settings.SoundPlaybackRate, NULL, 0);
	CheckError(GetMoviesError(), 29);

	err = BeginMediaEdits(sqt.sMedia);
	CheckError(err, 30);
}

void MacQTRecordFrame (int width, int height)
{
	OSStatus	err;

	// video

	if (sqt.frameSkipCount == sqt.frameSkip)
	{
		CVPixelBufferRef	buf;

		err = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, sqt.pool, &buf);
		if (err == noErr)
		{
			CGColorSpaceRef		color;
			CGContextRef		ctx;
			uint16				*p;

			err = CVPixelBufferLockBaseAddress(buf, 0);
			p = (uint16 *) CVPixelBufferGetBaseAddress(buf);

			color = CGColorSpaceCreateDeviceRGB();
			ctx = CGBitmapContextCreate((void *) p, sqt.width, sqt.height, 5, sqt.width * 2, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Host : 0));
			CGContextSetShouldAntialias(ctx, false);

			if (sqt.srcImage)
				CGImageRelease(sqt.srcImage);
			sqt.srcImage = CreateGameScreenCGImage();

			CGRect	dst = CGRectMake(0.0f, 0.0f, (float) sqt.width, (float) sqt.height);

			if ((!(height % SNES_HEIGHT_EXTENDED)) && (!(macQTMovFlag & kMovExtendedHeight)))
			{
				CGRect	src;

				src.size.width  = (float) width;
				src.size.height = (float) ((height > 256) ? (SNES_HEIGHT << 1) : SNES_HEIGHT);
				src.origin.x    = (float) 0;
				src.origin.y    = (float) height - src.size.height;
				DrawSubCGImage(ctx, sqt.srcImage, src, dst);
			}
			else
			if ((sqt.height << 1) % height)
			{
				CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
				CGContextFillRect(ctx, dst);

				float	dh  = (float) ((sqt.height > 256) ? (SNES_HEIGHT << 1) : SNES_HEIGHT);
				float	ofs = (float) ((int) ((drawoverscan ? 1.0 : 0.5) * ((float) sqt.height - dh) + 0.5));
				dst = CGRectMake(0.0f, ofs, (float) sqt.width, dh);
				CGContextDrawImage(ctx, dst, sqt.srcImage);
			}
			else
				CGContextDrawImage(ctx, dst, sqt.srcImage);

			CGContextRelease(ctx);
			CGColorSpaceRelease(color);

		#ifndef __BIG_ENDIAN__
			for (int i = 0; i < sqt.width * sqt.height; i++)
				SWAP_WORD(p[i]);
		#endif

			err = CVPixelBufferUnlockBaseAddress(buf, 0);

			err = ICMCompressionSessionEncodeFrame(sqt.session, buf, sqt.timeStamp, 0, kICMValidTime_DisplayTimeStampIsValid, NULL, NULL, NULL);

			CVPixelBufferRelease(buf);
		}

		sqt.keyFrameCount--;
		if (sqt.keyFrameCount <= 0)
			sqt.keyFrameCount = sqt.keyFrame;
	}

	sqt.frameSkipCount--;
	if (sqt.frameSkipCount < 0)
		sqt.frameSkipCount = sqt.frameSkip;

	sqt.timeStamp++;

	// sound

	int	sample_count = sqt.soundBufferSize;
	if (Settings.SixteenBitSound)
		sample_count >>= 1;

	S9xMixSamples((uint8 *) *(sqt.soundBuffer), sample_count);

	err = AddMediaSample(sqt.sMedia, sqt.soundBuffer, 0, sqt.soundBufferSize, 1, (SampleDescriptionHandle) sqt.soundDesc, sqt.samplesPerSec, mediaSampleNotSync, NULL);
}

static OSStatus WriteFrameCallBack (void *refCon, ICMCompressionSessionRef session, OSStatus r, ICMEncodedFrameRef frame, void *reserved)
{
	OSStatus	err;

	err = AddMediaSampleFromEncodedFrame(sqt.vMedia, frame, NULL);
	return (err);
}

void MacQTStopRecording (void)
{
	OSStatus   err;

	// video

	err = ICMCompressionSessionCompleteFrames(sqt.session, true, 0, sqt.timeStamp);
	err = ExtendMediaDecodeDurationToDisplayEndTime(sqt.vMedia, NULL);

	err = EndMediaEdits(sqt.vMedia);
	CheckError(err, 52);

	err = InsertMediaIntoTrack(sqt.vTrack, 0, 0, (TimeValue) GetMediaDisplayDuration(sqt.vMedia), fixed1);
	CheckError(err, 58);

	CGImageRelease(sqt.srcImage);
	CVPixelBufferPoolRelease(sqt.pool);
	ICMCompressionSessionRelease(sqt.session);
	ICMCompressionSessionOptionsRelease(sqt.option);

	// sound

	err = EndMediaEdits(sqt.sMedia);
	CheckError(err, 54);

	err = InsertMediaIntoTrack(sqt.sTrack, 0, 0, GetMediaDuration(sqt.sMedia), fixed1);
	CheckError(err, 55);

	DisposeHandle(sqt.soundBuffer);
	DisposeHandle((Handle) sqt.soundDesc);

	// storage

	err = AddMovieToStorage(sqt.movie, sqt.dataHandler);
	CheckError(err, 56);

	MacQTCloseVideoComponent(sqt.vci);

	err = CloseMovieStorage(sqt.dataHandler);
	CheckError(err, 57);

	DisposeHandle(sqt.dataRef);
	DisposeMovie(sqt.movie);
}
