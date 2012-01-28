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


#include "port.h"

#include "mac-prefix.h"
#include "mac-os.h"
#include "mac-gworld.h"

#define kIconSize	16

static void SetIconImage (CGImageRef, CGRect, int);
#ifdef MAC_PANTHER_SUPPORT
static IconRef CreateIconRefFromImage (CGImageRef, CGRect);
#endif


void DrawSubCGImage (CGContextRef ctx, CGImageRef image, CGRect src, CGRect dst)
{
    float	w = (float) CGImageGetWidth(image);
    float	h = (float) CGImageGetHeight(image);

	CGRect	drawRect = CGRectMake(0.0f, 0.0f, w, h);

	if (!CGRectEqualToRect(src, dst))
	{
		float	sx = CGRectGetWidth(dst)  / CGRectGetWidth(src);
		float	sy = CGRectGetHeight(dst) / CGRectGetHeight(src);
		float	dx = CGRectGetMinX(dst) - (CGRectGetMinX(src) * sx);
		float	dy = CGRectGetMinY(dst) - (CGRectGetMinY(src) * sy);

		drawRect = CGRectMake(dx, dy, w * sx, h * sy);
	}

	CGContextSaveGState(ctx);
	CGContextClipToRect(ctx, dst);
	CGContextDrawImage(ctx, drawRect, image);
	CGContextRestoreGState(ctx);
}

static void SetIconImage (CGImageRef image, CGRect rct, int n)
{
	if (systemVersion >= 0x1040)
		macIconImage[n] = CGImageCreateWithImageInRect(image, rct);
#ifdef MAC_PANTHER_SUPPORT
	else
		macIconRef[n] = CreateIconRefFromImage(image, rct);
#endif
}

void CreateIconImages (void)
{
	CGDataProviderRef	prov;
	CGImageRef			image;
	CFURLRef			url;

	image = NULL;
	memset(macIconImage, 0, sizeof(macIconImage));
#ifdef MAC_PANTHER_SUPPORT
	if (systemVersion < 0x1040)
		memset(macIconRef, 0, sizeof(macIconRef));
#endif

	url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("icons"),  CFSTR("png"), NULL);
	if (url)
	{
		prov = CGDataProviderCreateWithURL(url);
		if (prov)
		{
			image = CGImageCreateWithPNGDataProvider(prov, NULL, true, kCGRenderingIntentDefault);
			CGDataProviderRelease(prov);
		}

		CFRelease(url);
	}

	if (image)
	{
		int	x, y, v = 0, n = 0;

		macPadIconIndex = n;
		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 12; x++)
				SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);
			v += kIconSize;
		}

		macLegendIconIndex = n;
		for (x = 0; x < 2; x++)
			SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);
		v += kIconSize;

		macMusicBoxIconIndex = n;
		for (x = 0; x < 3; x++)
			SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);
		v += kIconSize;

		macFunctionIconIndex = n;
		for (x = 0; x < 17; x++)
			SetIconImage(image, CGRectMake(x * kIconSize, v, kIconSize, kIconSize), n++);

		CGImageRelease(image);
		
	#ifdef MAC_PANTHER_SUPPORT
		if (systemVersion < 0x1040)
		{
			CGColorSpaceRef		color;
			CGContextRef		ctx;
			CGRect				rct;
			static UInt32		data[2][kIconSize * kIconSize];

			rct = CGRectMake(0, 0, kIconSize, kIconSize);

			color = CGColorSpaceCreateDeviceRGB();
			if (color)
			{
				for (int i = 0; i < 2; i++)
				{
					ctx = CGBitmapContextCreate(data[i], kIconSize, kIconSize, 8, kIconSize * 4, color, kCGImageAlphaNoneSkipFirst);
					if (ctx)
					{
						PlotIconRefInContext(ctx, &rct, kAlignNone, kTransformNone, NULL, kPlotIconRefNormalFlags, macIconRef[macLegendIconIndex + i]);
						CGContextRelease(ctx);

						prov = CGDataProviderCreateWithData(NULL, data[i], kIconSize * kIconSize * 4, NULL);
						if (prov)
						{
							macIconImage[macLegendIconIndex + i] = CGImageCreate(kIconSize, kIconSize, 8, 32, kIconSize * 4, color, kCGImageAlphaNoneSkipFirst, prov, NULL, 1, kCGRenderingIntentDefault);
							CGDataProviderRelease(prov);
						}
					}
				}

				CGColorSpaceRelease(color);
			}
		}
	#endif
	}
}

void ReleaseIconImages (void)
{
	for (int i = 0; i < 118; i++)
	{
		if (systemVersion >= 0x1040)
		{
			if (macIconImage[i])
				CGImageRelease(macIconImage[i]);
		}
	#ifdef MAC_PANTHER_SUPPORT
		else
		{
			if (macIconRef[i])
				ReleaseIconRef(macIconRef[i]);
		}
	#endif
	}

#ifdef MAC_PANTHER_SUPPORT
	if (systemVersion < 0x1040)
	{
		if (macIconImage[macLegendIconIndex])
			CGImageRelease(macIconImage[macLegendIconIndex]);
		if (macIconImage[macLegendIconIndex + 1])
			CGImageRelease(macIconImage[macLegendIconIndex + 1]);
	}
#endif
}

#ifdef MAC_PANTHER_SUPPORT
static IconRef CreateIconRefFromImage (CGImageRef srcImage, CGRect srcRect)
{
	OSStatus			err;
	CGContextRef		cctx, actx;
	CGColorSpaceRef		color;
	CGRect				dstRect;
	IconRef				iconRef;
	IconFamilyHandle	icns;
	Handle				hdl;
	SInt32				size;
	UInt32				rgb[kIconSize * kIconSize];
	UInt8				alp[kIconSize * kIconSize];

	srcRect.origin.y = CGImageGetHeight(srcImage) - srcRect.origin.y - kIconSize;

	color = CGColorSpaceCreateDeviceRGB();
	if (color)
	{
		cctx = CGBitmapContextCreate(rgb, kIconSize, kIconSize, 8, kIconSize * 4, color, kCGImageAlphaNoneSkipFirst);
		if (cctx)
		{
			dstRect = CGRectMake(0, 0, kIconSize, kIconSize);
			DrawSubCGImage(cctx, srcImage, srcRect, dstRect);

			actx = CGBitmapContextCreate(alp, kIconSize, kIconSize, 8, kIconSize, NULL, kCGImageAlphaOnly);
			if (actx)
			{
				DrawSubCGImage(actx, srcImage, srcRect, dstRect);
				CGContextRelease(actx);
			}

			CGContextRelease(cctx);
		}

		CGColorSpaceRelease(color);
	}

	iconRef = NULL;

	size = sizeof(OSType) + sizeof(SInt32);
	icns = (IconFamilyHandle) NewHandle(size);
	if (icns)
	{
		// Big-endian: Panther is for PowerPC only
		(*icns)->resourceType = kIconFamilyType;
		(*icns)->resourceSize = size;

		err = PtrToHand(rgb, &hdl, sizeof(rgb));
		if (err == noErr)
		{
			err = SetIconFamilyData(icns, kSmall32BitData, hdl);
			DisposeHandle(hdl);

			if (err == noErr)
			{
				err = PtrToHand(alp, &hdl, sizeof(alp));
				if (err == noErr)
				{
					err = SetIconFamilyData(icns, kSmall8BitMask, hdl);
					DisposeHandle(hdl);
				}
			}
		}

		if (err == noErr)
			err = GetIconRefFromIconFamilyPtr(*icns, GetHandleSize((Handle) icns), &iconRef);

		DisposeHandle((Handle) icns);
	}

	return (iconRef);
}
#endif
