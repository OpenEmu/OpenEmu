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
#include "screenshot.h"

#include <QuickTime/QuickTime.h>

#include "mac-prefix.h"
#include "mac-file.h"
#include "mac-gworld.h"
#include "mac-os.h"
#include "mac-render.h"
#include "mac-screenshot.h"

static Handle GetScreenAsRawHandle (int, int);
static void ExportCGImageToPNGFile (CGImageRef, const char *);


static Handle GetScreenAsRawHandle (int destWidth, int destHeight)
{
	CGContextRef	ctx;
	CGColorSpaceRef	color;
	CGImageRef		image;
	Handle			data = NULL;

	image = CreateGameScreenCGImage();
	if (image)
	{
		data = NewHandleClear(destWidth * destHeight * 2);
		if (data)
		{
			HLock(data);

			color = CGColorSpaceCreateDeviceRGB();
			if (color)
			{
				ctx = CGBitmapContextCreate(*data, destWidth, destHeight, 5, destWidth * 2, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Big : 0));
				if (ctx)
				{
					CGContextDrawImage(ctx, CGRectMake(0.0f, 0.0f, (float) destWidth, (float) destHeight), image);
					CGContextRelease(ctx);
				}

				CGColorSpaceRelease(color);
			}

			HUnlock(data);
		}

		CGImageRelease(image);
	}

	return (data);
}

void WriteThumbnailToResourceFork (FSRef *ref, int destWidth, int destHeight)
{
	OSStatus		err;
	HFSUniStr255	fork;
	SInt16			resf;

	err = FSGetResourceForkName(&fork);
	if (err == noErr)
	{
		err = FSCreateResourceFork(ref, fork.length, fork.unicode, 0);
		if ((err == noErr) || (err == errFSForkExists))
		{
			err = FSOpenResourceFile(ref, fork.length, fork.unicode, fsWrPerm, &resf);
			if (err == noErr)
			{
				Handle	pict;

				pict = GetScreenAsRawHandle(destWidth, destHeight);
				if (pict)
				{
					AddResource(pict, 'Thum', 128, "\p");
					WriteResource(pict);
					ReleaseResource(pict);
				}

				CloseResFile(resf);
			}
		}
	}
}

static void ExportCGImageToPNGFile (CGImageRef image, const char *path)
{
	OSStatus				err;
	GraphicsExportComponent	exporter;
	CFStringRef				str;
	CFURLRef				url;
	Handle					dataRef;
	OSType					dataRefType;

	str = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
	if (str)
	{
		url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str, kCFURLPOSIXPathStyle, false);
		if (url)
		{
			err = QTNewDataReferenceFromCFURL(url, 0, &dataRef, &dataRefType);
			if (err == noErr)
			{
				err = OpenADefaultComponent(GraphicsExporterComponentType, kQTFileTypePNG, &exporter);
				if (err == noErr)
				{
					err = GraphicsExportSetInputCGImage(exporter, image);
					if (err == noErr)
					{
						err = GraphicsExportSetOutputDataReference(exporter, dataRef, dataRefType);
						if (err == noErr)
							err = GraphicsExportDoExport(exporter, NULL);
					}

					CloseComponent(exporter);
				}

				DisposeHandle(dataRef);
			}

			CFRelease(url);
		}

		CFRelease(str);
	}
}

CGImageRef CreateGameScreenCGImage (void)
{
	CGDataProviderRef	prov;
	CGColorSpaceRef		color;
	CGImageRef			image = NULL;
	int					rowbytes;

	rowbytes = IPPU.RenderedScreenWidth * 2;

	prov = CGDataProviderCreateWithData(NULL, GFX.Screen, 512 * 2 * 478, NULL);
	if (prov)
	{
		color = CGColorSpaceCreateDeviceRGB();
		if (color)
		{
			image = CGImageCreate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 5, 16, rowbytes, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Host : 0), prov, NULL, 1, kCGRenderingIntentDefault);
			CGColorSpaceRelease(color);
		}

		CGDataProviderRelease(prov);
	}

	return (image);
}

CGImageRef CreateBlitScreenCGImage (int width, int height, int rowbytes, uint8 *buffer)
{
	CGDataProviderRef	prov;
	CGColorSpaceRef		color;
	CGImageRef			image = NULL;

	prov = CGDataProviderCreateWithData(NULL, buffer, rowbytes * height, NULL);
	if (prov)
	{
		color = CGColorSpaceCreateDeviceRGB();
		if (color)
		{
			image = CGImageCreate(width, height, 5, 16, rowbytes, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Host : 0), prov, NULL, 1, kCGRenderingIntentDefault);
			CGColorSpaceRelease(color);
		}

		CGDataProviderRelease(prov);
	}

	return (image);
}

void DrawThumbnailResource (FSRef *ref, CGContextRef ctx, CGRect bounds)
{
	OSStatus			err;
	CGDataProviderRef	prov;
	CGColorSpaceRef		color;
	CGImageRef			image;
	QDPictRef			qdpr;
	Handle				pict;
	HFSUniStr255		fork;
	SInt16				resf;
	Size				size;

	CGContextSaveGState(ctx);

	CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
	CGContextFillRect(ctx, bounds);

	err = FSGetResourceForkName(&fork);
	if (err == noErr)
	{
		err = FSOpenResourceFile(ref, fork.length, fork.unicode, fsRdPerm, &resf);
		if (err == noErr)
		{
			pict = Get1Resource('PICT', 128);
			if (pict)
			{
				HLock(pict);

				size = GetHandleSize(pict);
				prov = CGDataProviderCreateWithData(NULL, (void *) *pict, size, NULL);
				if (prov)
				{
					qdpr = QDPictCreateWithProvider(prov);
					if (qdpr)
					{
						QDPictDrawToCGContext(ctx, bounds, qdpr);
						QDPictRelease(qdpr);
					}

					CGDataProviderRelease(prov);
				}

				HUnlock(pict);
				ReleaseResource(pict);
			}
			else
			{
				pict = Get1Resource('Thum', 128);
				if (pict)
				{
					HLock(pict);

					size = GetHandleSize(pict);
					prov = CGDataProviderCreateWithData(NULL, (void *) *pict, size, NULL);
					if (prov)
					{
						color = CGColorSpaceCreateDeviceRGB();
						if (color)
						{
							image = CGImageCreate(128, 120, 5, 16, 256, color, kCGImageAlphaNoneSkipFirst | ((systemVersion >= 0x1040) ? kCGBitmapByteOrder16Big : 0), prov, NULL, 0, kCGRenderingIntentDefault);
							if (image)
							{
								CGContextDrawImage(ctx, bounds, image);
								CGImageRelease(image);
							}

							CGColorSpaceRelease(color);
						}

						CGDataProviderRelease(prov);
					}

					HUnlock(pict);
					ReleaseResource(pict);
				}
			}

			CloseResFile(resf);
		}
	}

	CGContextRestoreGState(ctx);
}

bool8 S9xDoScreenshot (int width, int height)
{
	Settings.TakeScreenshot = false;

	uint16	*data;

	data = (uint16 *) malloc(512 * 478 * 2);
	if (data)
	{
		uint16	*sp, *dp;

		if (width > 256 && height > 239)
		{
			for (int y = 0; y < height; y++)
			{
				sp = GFX.Screen + y * GFX.RealPPL;
				dp = data + y * 512;

				for (int x = 0; x < width; x++)
					*dp++ = *sp++;
			}
		}
		else
		if (width > 256)
		{
			for (int y = 0; y < height; y++)
			{
				sp = GFX.Screen + y * GFX.RealPPL;
				dp = data + y * 2 * 512;

				for (int x = 0; x < width; x++)
				{
					*dp = *(dp + 512) = *sp++;
					dp++;
				}
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				sp = GFX.Screen + y * GFX.RealPPL;
				dp = data + y * 2 * 512;

				for (int x = 0; x < width; x++)
				{
					*dp = *(dp + 1) = *(dp + 512) = *(dp + 512 + 1) = *sp++;
					dp += 2;
				}
			}
		}

		CGImageRef	image;

		image = CreateBlitScreenCGImage(512, (height > 239) ? height : (height * 2), 1024, (uint8 *) data);
		if (image)
		{
			ExportCGImageToPNGFile(image, S9xGetPNGFilename());
			CGImageRelease(image);
		}

		free(data);
	}

	return (true);
}
