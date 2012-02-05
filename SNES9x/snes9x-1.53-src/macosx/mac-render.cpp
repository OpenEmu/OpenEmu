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
#include "display.h"
#include "blit.h"

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <AGL/agl.h>
#include <sys/time.h>

#include "mac-prefix.h"
#include "mac-cheatfinder.h"
#include "mac-coreimage.h"
#include "mac-os.h"
#include "mac-quicktime.h"
#include "mac-screenshot.h"
#include "mac-render.h"

typedef void (* Blitter) (uint8 *, int, uint8 *, int, int, int);

static OSStatus BlitMPGLTask (void *);
static OSStatus PrepareMPBlitGL (void);
static void S9xInitFullScreen (void);
static void S9xDeinitFullScreen (void);
static void S9xInitWindowMode (void);
static void S9xDeinitWindowMode (void);
static void S9xInitOpenGLFullScreen (void);
static void S9xDeinitOpenGLFullScreen (void);
static void S9xInitOpenGLWindowMode (void);
static void S9xDeinitOpenGLWindowMode (void);
static void S9xInitBlitGL (void);
static void S9xDeinitBlitGL (void);
static void S9xInitOpenGLContext (void);
static void S9xDeinitOpenGLContext (void);
static void S9xInitCoreImage (void);
static void S9xDeinitCoreImage (void);
static void S9xPutImageOpenGL (int, int);
static void S9xPutImageBlitGL (int, int);
static void S9xPutImageBlitGL2 (int, int);
static void GLMakeScreenMesh (GLfloat *, int, int);
static void GLMakeTextureMesh (GLfloat *, int, int, float, float);
static void GLPrepareTexture (bool8, int, int, int, int, int, int);
static inline void RenderBlitScreen (Blitter, int, int, int, int, int, uint16 *);
#ifndef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
static void SetBestDisplayMode (int, int);
#endif

enum
{
	kMPBlitFrame = 1,
	kMPBlitDone,
	kMPBlitNone
};

enum
{
	kGL256256 = 0,
	kGL256512,
	kGL512256,
	kGL512512,
	kGLBlit2x,
	kGLBlit3x,
	kGLBlit4x,
	kGLNTS256,
	kGLNTS512,
	kGLNumTextures
};

enum
{
	kSC2xNormal = 0,
	kSC2xExtend,
	kSC2xNHiRes,
	kSC2xEHiRes,
	kSC2xNInter,
	kSC2xEInter,
	kSC3xNormal,
	kSC3xExtend,
	kSC3xNHiRes,
	kSC3xEHiRes,
	kSCNTNormal,
	kSCNTExtend,
	kSCNumTextures
};

enum
{
	kSCMeshX = 10,
	kSCMeshY = 9
};

typedef struct
{
	Blitter		blitFn;
	int			nx;
	int			srcWidth;
	int			srcHeight;
	int			copyWidth;
	int			copyHeight;
	uint16		*gfxBuffer;
}	MPData;

typedef struct
{
	GLint		internal_format;
	GLint		format;
	GLint		type;
	GLenum		target;
	GLuint		textures[kGLNumTextures];
	GLfloat		vertex[kGLNumTextures][8];
	GLint		texW[kGLNumTextures];
	GLint		texH[kGLNumTextures];
	GLboolean   rangeExt;
	GLint		storage_hint;
	GLint		storage_apple;
	GLfloat		agp_texturing;
}	OpenGLData;

static uint16				*gfxScreen[2],
							*snesScreenA,
							*snesScreenB;
static uint8				*blitGLBuffer;

static CGDirectDisplayID	gGameDisplayID;

static MPTaskID				taskID            = NULL;
static MPQueueID			notificationQueue = NULL,
							taskQueue         = NULL;
static MPSemaphoreID		readySemaphore    = NULL;
static MPData				*mpBlit           = NULL;

static OpenGLData			OpenGL;
static CGLContextObj		glContext;
static AGLContext			agContext;
static CGLPixelFormatObj	cglpix;
static AGLPixelFormat		aglpix;
static GLint				glSwapInterval    = 0;
static GLint				agSwapInterval    = 0;
#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
static CFDictionaryRef		oldDisplayMode;
#else
static CGDisplayModeRef		oldDisplayModeRef;
#endif
static CGImageRef			cgGameImage       = NULL,
							cgBlitImage       = NULL;

static int					whichBuf          = 0;
static int					textureNum        = 0;
static int					prevBlitWidth, prevBlitHeight;
static int					imageWidth[2], imageHeight[2];
static int					nx                = 2;

static GLfloat				*scTexArray[kSCNumTextures];
static GLfloat				*scScnArray;

static struct timeval		bencht1, bencht2;

static const int			ntsc_width = SNES_NTSC_OUT_WIDTH(SNES_WIDTH); // 602


void InitGraphics (void)
{
	int	safemarginbytes = (520 * 520 - 512 * 512) * 2;

	snesScreenA  = (uint16 *) calloc( 520 *  520 * 2, 1);
	snesScreenB  = (uint16 *) calloc( 520 *  520 * 2, 1);
	blitGLBuffer = (uint8  *) calloc(1024 * 1024 * 2, 1);

	gfxScreen[0] = snesScreenA + (safemarginbytes >> 2);
	gfxScreen[1] = snesScreenB + (safemarginbytes >> 2);

	GFX.Pitch    = 512 * 2;
	GFX.Screen   = gfxScreen[0];

	if (!snesScreenA || !snesScreenB || !blitGLBuffer)
		QuitWithFatalError(0, "render 01");

#ifdef GFX_MULTI_FORMAT
	S9xSetRenderPixelFormat(RGB555);
	printf("GFX_MULTI_FORMAT is #defined.\n");
#endif

	if (!S9xBlitFilterInit()      |
		!S9xBlit2xSaIFilterInit() |
		!S9xBlitHQ2xFilterInit()  |
		!S9xBlitNTSCFilterInit())
		QuitWithFatalError(0, "render 02");

	switch (videoMode)
	{
		default:
		case VIDEOMODE_NTSC_C:
		case VIDEOMODE_NTSC_TV_C:
			S9xBlitNTSCFilterSet(&snes_ntsc_composite);
			break; 

		case VIDEOMODE_NTSC_S:
		case VIDEOMODE_NTSC_TV_S:
			S9xBlitNTSCFilterSet(&snes_ntsc_svideo);
			break; 

		case VIDEOMODE_NTSC_R:
		case VIDEOMODE_NTSC_TV_R:
			S9xBlitNTSCFilterSet(&snes_ntsc_rgb);
			break; 

		case VIDEOMODE_NTSC_M:
		case VIDEOMODE_NTSC_TV_M:
			S9xBlitNTSCFilterSet(&snes_ntsc_monochrome);
			break;
	}
}

void DeinitGraphics (void)
{
	S9xBlitNTSCFilterDeinit();
	S9xBlitHQ2xFilterDeinit();
	S9xBlit2xSaIFilterDeinit();
	S9xBlitFilterDeinit();

	if (snesScreenA)
	{
		free(snesScreenA);
		snesScreenA  = NULL;
	}

	if (snesScreenB)
	{
		free(snesScreenB);
		snesScreenB  = NULL;
	}

	if (blitGLBuffer)
	{
		free(blitGLBuffer);
		blitGLBuffer = NULL;
	}
}

void DrawPauseScreen (CGContextRef ctx, HIRect bounds)
{
	CGImageRef	image;
	CGRect		rct;
	float		sh, mh, rofs, ry;

	if ((IPPU.RenderedScreenWidth == 0) || (IPPU.RenderedScreenHeight == 0))
		return;

	sh = (float) ((IPPU.RenderedScreenHeight > 256) ? IPPU.RenderedScreenHeight : IPPU.RenderedScreenHeight * 2);
	mh = (float) (SNES_HEIGHT_EXTENDED * 2);

	if (drawoverscan)
	{
		rofs = (mh - sh) / mh;
		ry   = sh / mh;
	}
	else
	if (windowExtend)
	{
		rofs = (mh - sh) / mh / 2.0f;
		ry   = sh / mh;
	}
	else
	{
		rofs = 0.0f;
		ry   = 1.0f;
	}

	image = CreateGameScreenCGImage();
	if (image)
	{
		CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
		CGContextFillRect(ctx, bounds);

		rct = CGRectMake(0.0f, bounds.size.height * rofs, bounds.size.width, bounds.size.height * ry);
		CGContextDrawImage(ctx, rct, image);

		CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 0.5f);
		CGContextFillRect(ctx, bounds);

		CGImageRelease(image);
	}
}

void DrawFreezeDefrostScreen (uint8 *draw)
{
	const int	w = SNES_WIDTH << 1, h = kMacWindowHeight;

	imageWidth[0] = imageHeight[0] = 0;
	imageWidth[1] = imageHeight[1] = 0;
	prevBlitWidth = prevBlitHeight = 0;

	if ((drawingMethod == kDrawingBlitGL) && multiprocessor)
	{
		MPWaitOnSemaphore(readySemaphore, kDurationForever);
		printf("MP: Send dummy signal.\n");
		MPNotifyQueue(taskQueue, (void *) kMPBlitNone, 0, 0);
	}

	if (nx < 0 && !ciFilterEnable)
	{
		for (int y = 0; y < h; y++)
			memcpy(blitGLBuffer + y * 1024 * 2, draw + y * w * 2, w * 2);
	}
	else
		memcpy(blitGLBuffer, draw, w * h * 2);

	S9xPutImageBlitGL2(512, kMacWindowHeight);
}

void ClearGFXScreen (void)
{
	memset(gfxScreen[0], 0,  512 *  512 * 2);
	memset(gfxScreen[1], 0,  512 *  512 * 2);
	memset(blitGLBuffer, 0, 1024 * 1024 * 2);

	S9xBlitClearDelta();

	imageWidth[0] = imageHeight[0] = 0;
	imageWidth[1] = imageHeight[1] = 0;
	prevBlitWidth = prevBlitHeight = 0;

	if (fullscreen)
	{
		CGLSetCurrentContext(glContext);
		glViewport(0, 0, glScreenW, glScreenH);
	}
	else
	{
		aglSetCurrentContext(agContext);
		aglUpdateContext(agContext);
		glViewport(0, 0, (GLsizei) gWindowRect.size.width, (GLsizei) gWindowRect.size.height);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 2; i++)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		if (fullscreen)
			CGLFlushDrawable(glContext);
		else
			aglSwapBuffers(agContext);
	}
}

#ifndef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
static void SetBestDisplayMode (int width, int height)
{
	if (autoRes || !gl32bit)
	{
		CGError				err;
		CGDisplayModeRef	mode;
		CFArrayRef			array;
		CFStringRef			pixenc, pix;
		CFIndex				n, i;
		size_t				w, h;
		bool				r;

		pixenc = gl32bit ? CFSTR(IO32BitDirectPixels) : CFSTR(IO16BitDirectPixels);

		array = CGDisplayCopyAllDisplayModes(gGameDisplayID, NULL);
		n = CFArrayGetCount(array);

		for (i = 0; i < n; i++)
		{
			mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(array, i);

			w   = CGDisplayModeGetWidth(mode);
			h   = CGDisplayModeGetHeight(mode);
			pix = CGDisplayModeCopyPixelEncoding(mode);
			r   = CFStringCompare(pix, pixenc, 0) == kCFCompareEqualTo;
			CFRelease(pix);

			if (w == (size_t) width && h == (size_t) height && r)
				break;
		}

		if (i < n)
			err = CGDisplaySetDisplayMode(gGameDisplayID, mode, NULL);

		CFRelease(array);
	}
}
#endif

static void S9xInitFullScreen (void)
{
	DeinitGameWindow();

	size_t	width, height;

	width  = autoRes ? 640 : CGDisplayPixelsWide(gGameDisplayID);
	height = autoRes ? 480 : CGDisplayPixelsHigh(gGameDisplayID);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	CFDictionaryRef	mode;
	boolean_t		exactMatch;
	size_t			depth = gl32bit ? 32 : 16;

	oldDisplayMode = CGDisplayCurrentMode(gGameDisplayID);
	mode = CGDisplayBestModeForParameters(gGameDisplayID, depth, width, height, &exactMatch);
	CGDisplayCapture(gGameDisplayID);
	CGDisplaySwitchToMode(gGameDisplayID, mode);
#else
	oldDisplayModeRef = CGDisplayCopyDisplayMode(gGameDisplayID);
	CGDisplayCapture(gGameDisplayID);
	SetBestDisplayMode(width, height);
#endif

	CGDisplayErr		cgErr;
	CGDisplayCount		numDisplays, maxDisplays = 32;
	CGDirectDisplayID	activeDisplays[32];

	cgErr = CGGetActiveDisplayList(maxDisplays, activeDisplays, &numDisplays);
	if (cgErr == noErr)
	{
		if ((macControllerOption == SNES_MOUSE) || (macControllerOption == SNES_MOUSE_SWAPPED) || (numDisplays == 1))
			CGDisplayHideCursor(gGameDisplayID);

		if ((macControllerOption == SNES_MOUSE) || (macControllerOption == SNES_MOUSE_SWAPPED))
		{
			CGDisplayMoveCursorToPoint(gGameDisplayID, CGPointMake((float) (width >> 1), (float) (height >> 1)));
			CGAssociateMouseAndMouseCursorPosition(false);
		}
	}
}

static void S9xDeinitFullScreen (void)
{
	CGAssociateMouseAndMouseCursorPosition(true);
	CGDisplayShowCursor(gGameDisplayID);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	CGDisplaySwitchToMode(gGameDisplayID, oldDisplayMode);
#else
	CGError	err;

	err = CGDisplaySetDisplayMode(gGameDisplayID, oldDisplayModeRef, NULL);
	CGDisplayModeRelease(oldDisplayModeRef);
#endif

	CGDisplayRelease(gGameDisplayID);
}

static void S9xInitWindowMode (void)
{
	Rect	rct;
	size_t	width, height;

	width  = CGDisplayPixelsWide(gGameDisplayID);
	height = CGDisplayPixelsHigh(gGameDisplayID);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	CFDictionaryRef	mode;
	boolean_t		exactMatch;
	size_t			depth = gl32bit ? 32 : 16;

	oldDisplayMode = CGDisplayCurrentMode(gGameDisplayID);
	mode = CGDisplayBestModeForParameters(gGameDisplayID, depth, width, height, &exactMatch);
	if (exactMatch)
		CGDisplaySwitchToMode(gGameDisplayID, mode);
#else
	oldDisplayModeRef = CGDisplayCopyDisplayMode(gGameDisplayID);
	SetBestDisplayMode(width, height);
#endif

	InitGameWindow();
	ShowWindow(gWindow);

	GetWindowBounds(gWindow, kWindowContentRgn, &rct);
	gWindowRect = CGRectMake((float) rct.left, (float) rct.top, (float) (rct.right - rct.left), (float) (rct.bottom - rct.top));

	UpdateGameWindow();
}

static void S9xDeinitWindowMode (void)
{
#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	CGDisplaySwitchToMode(gGameDisplayID, oldDisplayMode);
#else
	CGError	err;

	err = CGDisplaySetDisplayMode(gGameDisplayID, oldDisplayModeRef, NULL);
	CGDisplayModeRelease(oldDisplayModeRef);
#endif

	UpdateGameWindow();
}

static void S9xInitOpenGLFullScreen (void)
{
	CGOpenGLDisplayMask	displayMask;
	GLint 				numPixelFormats;

	displayMask = CGDisplayIDToOpenGLDisplayMask(gGameDisplayID);
	CGLPixelFormatAttribute	attribs[] = { (CGLPixelFormatAttribute) kCGLPFAFullScreen,
										  (CGLPixelFormatAttribute) kCGLPFADoubleBuffer,
										  (CGLPixelFormatAttribute) kCGLPFAAccelerated,
										  (CGLPixelFormatAttribute) kCGLPFANoRecovery,
										  (CGLPixelFormatAttribute) kCGLPFAColorSize,
										  (CGLPixelFormatAttribute) (gl32bit ? 32 : 16),
										  (CGLPixelFormatAttribute) kCGLPFADisplayMask,
										  (CGLPixelFormatAttribute) displayMask,
										  (CGLPixelFormatAttribute) 0 };

	CGLChoosePixelFormat(attribs, &cglpix, &numPixelFormats);
	CGLCreateContext(cglpix, NULL, &glContext);
	glSwapInterval = vsync ? 1 : 0;
	if (extraOptions.benchmark)
		glSwapInterval = 0;
	CGLSetParameter(glContext, kCGLCPSwapInterval, &glSwapInterval);
	CGLSetCurrentContext(glContext);

#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	CGLSetFullScreen(glContext);
#else
	CGLSetFullScreenOnDisplay(glContext, CGDisplayIDToOpenGLDisplayMask(gGameDisplayID));
#endif

	glScreenW = CGDisplayPixelsWide(gGameDisplayID);
	glScreenH = CGDisplayPixelsHigh(gGameDisplayID);
}

static void S9xDeinitOpenGLFullScreen (void)
{
	if (glContext)
	{
		CGLSetCurrentContext(NULL);
		CGLClearDrawable(glContext);
		CGLDestroyContext(glContext);
		CGLDestroyPixelFormat(cglpix);
	}
}

static void S9xInitOpenGLWindowMode (void)
{
	GLint	attribs[] = { AGL_RGBA,
						  AGL_DOUBLEBUFFER,
						  AGL_ACCELERATED,
						  AGL_NO_RECOVERY,
						  AGL_PIXEL_SIZE, gl32bit ? 32 : 16,
						  AGL_NONE };

	aglpix = aglChoosePixelFormat(NULL, 0, attribs);
	agContext = aglCreateContext(aglpix, NULL);

	if (systemVersion >= 0x1050)
		aglSetWindowRef(agContext, gWindow);
#ifdef MAC_TIGER_PANTHER_SUPPORT
	else
		aglSetDrawable(agContext, GetWindowPort(gWindow));
#endif

	agSwapInterval = vsync ? 1 : 0;
	if (extraOptions.benchmark)
		agSwapInterval = 0;
	aglSetInteger(agContext, AGL_SWAP_INTERVAL, &agSwapInterval);
	aglSetCurrentContext(agContext);

	if (systemVersion >= 0x1040)
	{
		aglGetCGLPixelFormat(aglpix, (void **) &cglpix);
		aglGetCGLContext(agContext, (void **) &glContext);
	}
}

static void S9xDeinitOpenGLWindowMode (void)
{
	if (agContext)
	{
		if (systemVersion >= 0x1050)
			aglSetWindowRef(agContext, NULL);
	#ifdef MAC_TIGER_PANTHER_SUPPORT
		else
			aglSetDrawable(agContext, NULL);
	#endif

		aglSetCurrentContext(NULL);
		aglDestroyContext(agContext);
		aglDestroyPixelFormat(aglpix);
	}
}

static void S9xInitBlitGL (void)
{
	if (multiprocessor)
	{
		printf("MP: Creating BlitGL thread.\n");

		if (noErr != PrepareMPBlitGL())
			multiprocessor = false;
	}
}

static void S9xDeinitBlitGL (void)
{
	if (multiprocessor)
	{
		MPNotifyQueue(taskQueue, (void *) kMPBlitDone, 0, 0);
		MPWaitOnQueue(notificationQueue, NULL, NULL, NULL, kDurationForever);
		MPDeleteQueue(notificationQueue);
		notificationQueue = NULL;

		printf("MP: Successfully received terminate signal from BlitGL thread.\n");
	}
}

static void GLPrepareTexture (bool8 useRange, int texNo, int rangeOnW, int rangeOnH, int rangeOffW, int rangeOffH, int filter)
{
	bool8	rangeAvailable = OpenGL.rangeExt & useRange;

	OpenGL.texW[texNo] = rangeAvailable ? rangeOnW : rangeOffW;
	OpenGL.texH[texNo] = rangeAvailable ? rangeOnH : rangeOffH;

	OpenGL.vertex[texNo][0] = 0;
	OpenGL.vertex[texNo][1] = 0;
	OpenGL.vertex[texNo][2] = rangeAvailable ? rangeOnW : 1;
	OpenGL.vertex[texNo][3] = 0;
	OpenGL.vertex[texNo][4] = rangeAvailable ? rangeOnW : 1;
	OpenGL.vertex[texNo][5] = rangeAvailable ? rangeOnH : 1;
	OpenGL.vertex[texNo][6] = 0;
	OpenGL.vertex[texNo][7] = rangeAvailable ? rangeOnH : 1;

	glBindTexture(OpenGL.target, OpenGL.textures[texNo]);

	if (rangeAvailable)
	{
		glTextureRangeAPPLE(OpenGL.target, OpenGL.texW[texNo] * OpenGL.texH[texNo] * 2, GFX.Screen);
		glTexParameteri(OpenGL.target, GL_TEXTURE_STORAGE_HINT_APPLE, OpenGL.storage_hint);
	}

	glTexParameterf(OpenGL.target, GL_TEXTURE_PRIORITY, OpenGL.agp_texturing);
	glTexParameteri(OpenGL.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(OpenGL.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(OpenGL.target, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(OpenGL.target, GL_TEXTURE_MIN_FILTER, filter);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(OpenGL.target, 0, OpenGL.internal_format, OpenGL.texW[texNo], OpenGL.texH[texNo], 0, OpenGL.format, OpenGL.type, GFX.Screen);
}

static void GLMakeScreenMesh (GLfloat *vertex3D, int meshx, int meshy)
{
	GLfloat *v;
	float   warp;

	v = vertex3D;
	warp = macCurvatureWarp * 0.001f;

	for (int y = 0; y < meshy; y++)
	{
		for (int x = 0; x <= meshx; x++)
		{
			float	u1, v1, v2;

			u1 = -1.0f + 2.0f / (float) meshx * (float)  x;
			v1 = -1.0f + 2.0f / (float) meshy * (float)  y;
			v2 = -1.0f + 2.0f / (float) meshy * (float) (y + 1);

			*v++ = u1;
			*v++ = v2;
			*v++ = -1.0f - (u1 * u1 + v2 * v2) * warp;

			*v++ = u1;
			*v++ = v1;
			*v++ = -1.0f - (u1 * u1 + v1 * v1) * warp;
		}
	}
}

static void GLMakeTextureMesh (GLfloat *vertex2D, int meshx, int meshy, float lx, float ly)
{
	GLfloat *v;

	v = vertex2D;

	for (int y = meshy; y > 0; y--)
	{
		for (int x = 0; x <= meshx; x++)
		{
			float	u1, v1, v2;

			u1 = lx / (float) meshx * (float)  x;
			v1 = ly / (float) meshy * (float)  y;
			v2 = ly / (float) meshy * (float) (y - 1);

			*v++ = u1;
			*v++ = v2;

			*v++ = u1;
			*v++ = v1;
		}
	}
}

static void S9xInitOpenGLContext (void)
{
	OpenGL.internal_format = GL_RGB5_A1;
	OpenGL.format          = GL_BGRA;
	OpenGL.type            = GL_UNSIGNED_SHORT_1_5_5_5_REV;
	OpenGL.rangeExt        = gluCheckExtension((const GLubyte *) "GL_APPLE_texture_range", glGetString(GL_EXTENSIONS));
	OpenGL.target          = OpenGL.rangeExt ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D;

	OpenGL.storage_apple   = extraOptions.glUseClientStrageApple ? 1    : 0;
	OpenGL.agp_texturing   = extraOptions.glUseTexturePriority   ? 0.0f : 1.0f;
	switch (extraOptions.glStorageHint)
	{
		case 1:	OpenGL.storage_hint = GL_STORAGE_PRIVATE_APPLE;	break;
		case 2:	OpenGL.storage_hint = GL_STORAGE_CACHED_APPLE;	break;
		case 3:	OpenGL.storage_hint = GL_STORAGE_SHARED_APPLE;	break;
	}

	if (screencurvature || videoMode >= VIDEOMODE_NTSC_C || extraOptions.glForceNoTextureRectangle)
	{
		OpenGL.rangeExt = false;
		OpenGL.target   = GL_TEXTURE_2D;
	}

	printf("TextureRange: %s\n", OpenGL.rangeExt ? "enable" : "disable");

	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glCullFace(GL_BACK);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_RECTANGLE_EXT);
	glEnable(OpenGL.target);

	glGenTextures(kGLNumTextures, OpenGL.textures);

	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, OpenGL.storage_apple);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);

	int	filter = (videoMode == VIDEOMODE_SMOOTH) ? GL_LINEAR : GL_NEAREST;
	GLPrepareTexture(true,  kGL256256, SNES_WIDTH,     SNES_HEIGHT_EXTENDED,      256,  256, filter);
	GLPrepareTexture(true,  kGL256512, SNES_WIDTH,     SNES_HEIGHT_EXTENDED * 2,  256,  512, filter);
	GLPrepareTexture(true,  kGL512256, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED,      512,  256, filter);
	GLPrepareTexture(true,  kGL512512, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2,  512,  512, filter);
	GLPrepareTexture(true,  kGLBlit2x, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2,  512,  512, GL_LINEAR);
	GLPrepareTexture(true,  kGLBlit3x, SNES_WIDTH * 3, SNES_HEIGHT_EXTENDED * 3, 1024, 1024, GL_LINEAR);
	GLPrepareTexture(true,  kGLBlit4x, SNES_WIDTH * 4, SNES_HEIGHT_EXTENDED * 4, 1024, 1024, GL_LINEAR);
	GLPrepareTexture(false, kGLNTS256, 1024,           256,                      1024,  256, GL_LINEAR);
	GLPrepareTexture(false, kGLNTS512, 1024,           512,                      1024,  512, GL_LINEAR);

	if (!screencurvature)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-1.0, 1.0, -1.0, 1.0, 0.95, 5.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		int	mesh = (kSCMeshX + 1) * 2 * kSCMeshY * 2;
		scTexArray[kSC2xNormal] = new GLfloat [mesh];
		scTexArray[kSC2xExtend] = new GLfloat [mesh];
		scTexArray[kSC2xNHiRes] = new GLfloat [mesh];
		scTexArray[kSC2xEHiRes] = new GLfloat [mesh];
		scTexArray[kSC2xNInter] = new GLfloat [mesh];
		scTexArray[kSC2xEInter] = new GLfloat [mesh];
		scTexArray[kSC3xNormal] = new GLfloat [mesh];
		scTexArray[kSC3xExtend] = new GLfloat [mesh];
		scTexArray[kSC3xNHiRes] = new GLfloat [mesh];
		scTexArray[kSC3xEHiRes] = new GLfloat [mesh];
		scTexArray[kSCNTNormal] = new GLfloat [mesh];
		scTexArray[kSCNTExtend] = new GLfloat [mesh];

		GLMakeTextureMesh(scTexArray[kSC2xNormal], kSCMeshX, kSCMeshY,                         1.0f, 224.0f /  256.0f);
		GLMakeTextureMesh(scTexArray[kSC2xExtend], kSCMeshX, kSCMeshY,                         1.0f, 239.0f /  256.0f);
		GLMakeTextureMesh(scTexArray[kSC2xNHiRes], kSCMeshX, kSCMeshY,                         1.0f, 224.0f /  512.0f);
		GLMakeTextureMesh(scTexArray[kSC2xEHiRes], kSCMeshX, kSCMeshY,                         1.0f, 239.0f /  512.0f);
		GLMakeTextureMesh(scTexArray[kSC2xNInter], kSCMeshX, kSCMeshY,             256.0f /  512.0f, 224.0f /  256.0f);
		GLMakeTextureMesh(scTexArray[kSC2xEInter], kSCMeshX, kSCMeshY,             256.0f /  512.0f, 239.0f /  256.0f);
		GLMakeTextureMesh(scTexArray[kSC3xNormal], kSCMeshX, kSCMeshY,             768.0f / 1024.0f, 672.0f / 1024.0f);
		GLMakeTextureMesh(scTexArray[kSC3xExtend], kSCMeshX, kSCMeshY,             768.0f / 1024.0f, 717.0f / 1024.0f);
		GLMakeTextureMesh(scTexArray[kSC3xNHiRes], kSCMeshX, kSCMeshY,             768.0f / 1024.0f, 672.0f / 2048.0f);
		GLMakeTextureMesh(scTexArray[kSC3xEHiRes], kSCMeshX, kSCMeshY,             768.0f / 1024.0f, 717.0f / 2048.0f);
		GLMakeTextureMesh(scTexArray[kSCNTNormal], kSCMeshX, kSCMeshY, (float) ntsc_width / 1024.0f, 224.0f /  256.0f);
		GLMakeTextureMesh(scTexArray[kSCNTExtend], kSCMeshX, kSCMeshY, (float) ntsc_width / 1024.0f, 239.0f /  256.0f);

		scScnArray = new GLfloat [(kSCMeshX + 1) * 2 * kSCMeshY * 3];
		GLMakeScreenMesh(scScnArray, kSCMeshX, kSCMeshY);
	}

	if (fullscreen)
	{
		CGLSetCurrentContext(glContext);
		glViewport(0, 0, glScreenW, glScreenH);
	}
	else
	{
		aglSetCurrentContext(agContext);
		aglUpdateContext(agContext);
		glViewport(0, 0, (GLsizei) gWindowRect.size.width, (GLsizei) gWindowRect.size.height);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 2; i++)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		if (fullscreen)
			CGLFlushDrawable(glContext);
		else
			aglSwapBuffers(agContext);
	}
}

static void S9xDeinitOpenGLContext (void)
{
	if (screencurvature)
	{
		delete [] scTexArray[kSC2xNormal];
		delete [] scTexArray[kSC2xExtend];
		delete [] scTexArray[kSC2xNHiRes];
		delete [] scTexArray[kSC2xEHiRes];
		delete [] scTexArray[kSC2xNInter];
		delete [] scTexArray[kSC2xEInter];
		delete [] scTexArray[kSC3xNormal];
		delete [] scTexArray[kSC3xExtend];
		delete [] scTexArray[kSC3xNHiRes];
		delete [] scTexArray[kSC3xEHiRes];
		delete [] scTexArray[kSCNTNormal];
		delete [] scTexArray[kSCNTExtend];
		delete [] scScnArray;

		scTexArray[kSC2xNormal] = NULL;
		scTexArray[kSC2xExtend] = NULL;
		scTexArray[kSC2xNHiRes] = NULL;
		scTexArray[kSC2xEHiRes] = NULL;
		scTexArray[kSC2xNInter] = NULL;
		scTexArray[kSC2xEInter] = NULL;
		scTexArray[kSC3xNormal] = NULL;
		scTexArray[kSC3xExtend] = NULL;
		scTexArray[kSC3xNHiRes] = NULL;
		scTexArray[kSC3xEHiRes] = NULL;
		scTexArray[kSCNTNormal] = NULL;
		scTexArray[kSCNTExtend] = NULL;
		scScnArray              = NULL;
	}

	glDeleteTextures(kGLNumTextures, OpenGL.textures);
}

static void S9xInitCoreImage (void)
{
	cgGameImage = NULL;
	cgBlitImage = NULL;

	InitCoreImageContext(glContext, cglpix);
}

static void S9xDeinitCoreImage (void)
{
	DeinitCoreImageContext();

	if (cgGameImage)
	{
		CGImageRelease(cgGameImage);
		cgGameImage = NULL;
	}

	if (cgBlitImage)
	{
		CGImageRelease(cgBlitImage);
		cgBlitImage = NULL;
	}
}

void GetGameDisplay (int *w, int *h)
{
	CGDisplayErr		cgErr;
	CGDisplayCount		numDisplays, maxDisplays = 32;
	CGDirectDisplayID	activeDisplays[32];
	CGPoint				windowAt;

	gGameDisplayID = CGMainDisplayID();

	windowAt = CGPointMake((float) windowPos[kWindowScreen].h, (float) windowPos[kWindowScreen].v);

	cgErr = CGGetDisplaysWithPoint(windowAt, maxDisplays, activeDisplays, &numDisplays);
	if ((cgErr == noErr) && (numDisplays > 0))
	{
		for (unsigned int i = 0; i < numDisplays; i++)
		{
			if (activeDisplays[i] != CGMainDisplayID())
				gGameDisplayID = activeDisplays[i];
		}
	}

	if (w != NULL && h != NULL)
	{
		*w = CGDisplayPixelsWide(gGameDisplayID);
		*h = CGDisplayPixelsHigh(gGameDisplayID);
	}
}

void S9xInitDisplay (int argc, char **argv)
{
	if (directDisplay)
		return;

	GetGameDisplay(NULL, NULL);

	glScreenBounds = CGDisplayBounds(gGameDisplayID);

	unlimitedCursor = CGPointMake(0.0f, 0.0f);

	imageWidth[0] = imageHeight[0] = 0;
	imageWidth[1] = imageHeight[1] = 0;
	prevBlitWidth = prevBlitHeight = 0;
	GFX.Screen    = gfxScreen[0];
	whichBuf      = 0;
	textureNum    = 0;

	switch (videoMode)
	{
		case VIDEOMODE_HQ4X:
			nx =  4;
			break;

		case VIDEOMODE_HQ3X:
			nx =  3;
			break;

		case VIDEOMODE_NTSC_C:
		case VIDEOMODE_NTSC_S:
		case VIDEOMODE_NTSC_R:
		case VIDEOMODE_NTSC_M:
			nx = -1;
			break;

		case VIDEOMODE_NTSC_TV_C:
		case VIDEOMODE_NTSC_TV_S:
		case VIDEOMODE_NTSC_TV_R:
		case VIDEOMODE_NTSC_TV_M:
			nx = -2;
			break;

		default:
			nx =  2;
			break;
	}

	if (fullscreen)
	{
		S9xInitFullScreen();
		S9xInitOpenGLFullScreen();
	}
	else
	{
		S9xInitWindowMode();
		S9xInitOpenGLWindowMode();
	}

	S9xInitOpenGLContext();
	if (ciFilterEnable)
		S9xInitCoreImage();
	if (drawingMethod == kDrawingBlitGL)
		S9xInitBlitGL();

	S9xSetSoundMute(false);
	Microseconds((UnsignedWide *) &lastFrame);

	windowResizeCount = 1;

	gettimeofday(&bencht1, NULL);

	directDisplay = true;
}

void S9xDeinitDisplay (void)
{
	if (!directDisplay)
		return;

	S9xSetSoundMute(true);

	if (drawingMethod == kDrawingBlitGL)
		S9xDeinitBlitGL();
	if (ciFilterEnable)
		S9xDeinitCoreImage();
	S9xDeinitOpenGLContext();

	if (fullscreen)
	{
		S9xDeinitOpenGLFullScreen();
		S9xDeinitFullScreen();
	}
	else
	{
		S9xDeinitOpenGLWindowMode();
		S9xDeinitWindowMode();
	}

	directDisplay = false;
}

bool8 S9xInitUpdate (void)
{
	return (true);
}

bool8 S9xDeinitUpdate (int width, int height)
{
	if (directDisplay)
		S9xPutImage(width, height);

	return (true);
}

bool8 S9xContinueUpdate (int width, int height)
{
	return (true);
}

static inline void RenderBlitScreen (Blitter Fn, int x, int sW, int sH, int cW, int cH, uint16 *buf)
{
	switch (x)
	{
		case -1:
			(Fn) ((uint8 *) buf, sW * 2, blitGLBuffer, 1024 * 2, sW, sH);
			break;

		case -2:
			if (sH > SNES_HEIGHT_EXTENDED)
				(Fn) ((uint8 *) buf, sW * 2, blitGLBuffer, 1024 * 2, sW, sH);
			else
			{
				uint8	*tmpBuffer = blitGLBuffer + (1024 * 512 * 2);
				int		aligned    = ((ntsc_width + 2) >> 1) << 1;
				(Fn) ((uint8 *) buf, sW * 2, tmpBuffer, 1024 * 2, sW, sH);
				S9xBlitPixMixedTV1x2(tmpBuffer, 1024 * 2, blitGLBuffer, 1024 * 2, aligned, cH);
				cH *= 2;
			}

			break;

		default:
			int	dstbytes = (OpenGL.rangeExt ? cW : ((cW > 512) ? 1024 : ((cW > 256) ? 512 : 256))) * 2;
			(Fn) ((uint8 *) buf, sW * 2, blitGLBuffer, dstbytes, sW, sH);
			break;
	}

	S9xPutImageBlitGL2(cW, cH);
}

static OSStatus PrepareMPBlitGL (void)
{
	OSStatus	err;

	mpBlit = (MPData *) MPAllocateAligned(sizeof(MPData), kMPAllocateDefaultAligned, kMPAllocateClearMask);
	if (!mpBlit)
		return (memFullErr);

	err = MPCreateQueue(&notificationQueue);
	if (err == noErr)
	{
		err = MPCreateQueue(&taskQueue);
		if (err == noErr)
		{
			err = MPCreateBinarySemaphore(&readySemaphore);
			if (err == noErr)
			{
				MPSignalSemaphore(readySemaphore);
				err = MPCreateTask(BlitMPGLTask, NULL, 0, notificationQueue, NULL, NULL, 0, &taskID);
			}
		}
	}

	return (err);
}

static OSStatus BlitMPGLTask (void *parameter)
{
	OSStatus	err = noErr;
	int32		theCommand, param1, param2;

	printf("MP: Entered BlitGL thread.\n");

	for (;;)
	{
		err = MPWaitOnQueue(taskQueue, (void **) &theCommand, (void **) &param1, (void **) &param2, kDurationForever);
		if (err)
			break;

		if (theCommand == kMPBlitFrame)
		{
			RenderBlitScreen(mpBlit->blitFn, mpBlit->nx, mpBlit->srcWidth, mpBlit->srcHeight, mpBlit->copyWidth, mpBlit->copyHeight, mpBlit->gfxBuffer);
			MPSignalSemaphore(readySemaphore);
		}
		else
		if (theCommand == kMPBlitNone)
			MPSignalSemaphore(readySemaphore);
		else
		if (theCommand == kMPBlitDone)
			break;
		else
		{
			err = userCanceledErr;
			break;
		}
	}

	MPFree(mpBlit);
	MPDeleteSemaphore(readySemaphore);
	MPDeleteQueue(taskQueue);
	mpBlit         = NULL;
	readySemaphore = NULL;
	taskQueue      = NULL;

	printf("MP: Exited BlitGL thread.\n");

	return (err);
}

void S9xPutImage (int width, int height)
{
	static float	fps   = 0.0f;
	static long		count = 0;
	static char		text[32];

	if (extraOptions.benchmark)
	{
		uint16	*basePtr;
		long	delta;
		size_t	len;

		count++;

		gettimeofday(&bencht2, NULL);

		delta = 1000000 * (bencht2.tv_sec - bencht1.tv_sec) + (bencht2.tv_usec - bencht1.tv_usec);
		if (delta > 1000000)
		{
			fps = (1000000.0f * (float) count) / (float) delta;
			count = 0;

			gettimeofday(&bencht1, NULL);
		}

		sprintf(text, "%.1f", fps);

		basePtr = GFX.Screen + 1;
		len = strlen(text);

		for (unsigned int i = 0; i < len; i++)
		{
			S9xDisplayChar(basePtr, text[i]);
			basePtr += (8 - 1);
		}
	}
	else
	{
		if (cfIsWatching)
			CheatFinderDrawWatchAddr();

		if (Settings.DisplayFrameRate)
		{
			static int	drawnFrames[60] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
											1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			static int	tableIndex = 0;
			int			frameCalc  = 0;

			drawnFrames[tableIndex] = skipFrames;

			if (Settings.TurboMode)
			{
				drawnFrames[tableIndex] = (drawnFrames[tableIndex] + (macFastForwardRate / 2)) / macFastForwardRate;
				if (drawnFrames[tableIndex] == 0)
					drawnFrames[tableIndex] = 1;
			}

			tableIndex = (tableIndex + 1) % 60;

			for (int i = 0; i < 60; i++)
				frameCalc += drawnFrames[i];

			IPPU.DisplayedRenderedFrameCount = (Memory.ROMFramesPerSecond * 60) / frameCalc;
		}
	}

	switch (drawingMethod)
	{
		case kDrawingOpenGL:
			S9xPutImageOpenGL(width, height);
			break;

		case kDrawingBlitGL:
			S9xPutImageBlitGL(width, height);
			break;
	}
}

static void S9xPutImageOpenGL (int width, int height)
{
	int	orig_height = height;

	if ((imageWidth[0] != width) || (imageHeight[0] != height))
		windowResizeCount += 2;

	if (windowResizeCount > 0)
	{
		if (drawoverscan && (height % SNES_HEIGHT == 0))
		{
			int		pitch    = width << 1;
			int		extbtm   = (height > 256) ? (SNES_HEIGHT_EXTENDED << 1) : SNES_HEIGHT_EXTENDED;
			uint32	*extarea = (uint32 *) ((uint8 *) GFX.Screen + height * pitch);

			for (int i = 0; i < (((extbtm - height) * pitch) >> 2); i++)
				extarea[i] = 0;

			height = extbtm;
		}

		int	vh = (height > 256) ? height : (height << 1);

		if (fullscreen)
		{
			CGLSetCurrentContext(glContext);

			glViewport(0, 0, glScreenW, glScreenH);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (glstretch)
			{
				float   fpw = (float) glScreenH / vh * 512.0f;
				int		pw  = (int) (fpw + ((float) glScreenW - fpw) * (float) macAspectRatio / 10000.0);

				glViewport((glScreenW - pw) >> 1, 0, pw, glScreenH);
			}
			else
				glViewport((glScreenW - 512) >> 1, (glScreenH - vh) >> 1, 512, vh);
		}
		else
		{
			int	ww = (int) gWindowRect.size.width,
				wh = (int) gWindowRect.size.height;

			aglSetCurrentContext(agContext);
			aglUpdateContext(agContext);

			glViewport(0, 0, ww, wh);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (windowExtend)
				glViewport(0, ((kMacWindowHeight - vh) >> 1) * wh / kMacWindowHeight, ww, vh * wh / kMacWindowHeight);
		}

		glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

		if (!ciFilterEnable)
		{
			textureNum = (width <= 256) ? ((height <= 256) ? kGL256256 : kGL256512) : ((height <= 256) ? kGL512256 : kGL512512);
			OpenGL.vertex[textureNum][5] = OpenGL.vertex[textureNum][7] = OpenGL.rangeExt ? height : (vh / 512.0f);
			glBindTexture(OpenGL.target, OpenGL.textures[textureNum]);
		}
		else
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, width, orig_height - height, orig_height, -1, 1);

			if (cgGameImage)
				CGImageRelease(cgGameImage);
			cgGameImage = CreateGameScreenCGImage();
		}

		imageWidth[0]  = width;
		imageHeight[0] = height;

		windowResizeCount--;
	}
	else
	{
		if (drawoverscan)
			height = (height > 256) ? (SNES_HEIGHT_EXTENDED << 1) : SNES_HEIGHT_EXTENDED;
	}

	if (!ciFilterEnable)
	{
		glTexSubImage2D(OpenGL.target, 0, 0, 0, OpenGL.texW[textureNum], OpenGL.texH[textureNum], OpenGL.format, OpenGL.type, GFX.Screen);

		if (!screencurvature)
		{
			glBegin(GL_QUADS);

			glTexCoord2fv(&OpenGL.vertex[textureNum][6]);
			glVertex2f(-1.0f, -1.0f);
			glTexCoord2fv(&OpenGL.vertex[textureNum][4]);
			glVertex2f( 1.0f, -1.0f);
			glTexCoord2fv(&OpenGL.vertex[textureNum][2]);
			glVertex2f( 1.0f,  1.0f);
			glTexCoord2fv(&OpenGL.vertex[textureNum][0]);
			glVertex2f(-1.0f,  1.0f);

			glEnd();
		}
		else
		{
			GLfloat *t, *s;

			t = scTexArray[(height % SNES_HEIGHT) ? kSC2xExtend : kSC2xNormal];
			s = scScnArray;

			for (int i = 0; i < kSCMeshY; i++)
			{
				glTexCoordPointer(2, GL_FLOAT, 0, t);
				glVertexPointer(3, GL_FLOAT, 0, s);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, (kSCMeshX + 1) * 2);

				t += (kSCMeshX + 1) * 2 * 2;
				s += (kSCMeshX + 1) * 2 * 3;
			}
		}

		glFinishObjectAPPLE(GL_TEXTURE, OpenGL.textures[textureNum]);
	}
	else
	{
		CGRect	src;

		src = CGRectMake(0, 0, width, orig_height);
		DrawWithCoreImageFilter(src, cgGameImage);
	}

	if (fullscreen)
		CGLFlushDrawable(glContext);
	else
		aglSwapBuffers(agContext);
}

static void S9xPutImageBlitGL (int width, int height)
{
	Blitter	blitFn;
	int		copyWidth, copyHeight;

	if ((imageWidth[whichBuf] != width) || (imageHeight[whichBuf] != height))
	{
		if ((videoMode == VIDEOMODE_TV) && (width <= 256))
			S9xBlitClearDelta();

		if (drawoverscan && (height % SNES_HEIGHT == 0))
		{
			memset(blitGLBuffer, 0, 1024 * 1024 * 2);

			int		pitch    = width << 1;
			int		extbtm   = (height > 256) ? (SNES_HEIGHT_EXTENDED << 1) : SNES_HEIGHT_EXTENDED;
			uint32	*extarea = (uint32 *) ((uint8 *) GFX.Screen + height * pitch);

			for (int i = 0; i < (((extbtm - height) * pitch) >> 2); i++)
				extarea[i] = 0;

			height = extbtm;
		}
	}
	else
	{
		if (drawoverscan)
			height = (height > 256) ? (SNES_HEIGHT_EXTENDED << 1) : SNES_HEIGHT_EXTENDED;
	}

	switch (nx)
	{
		default:
		case 2:
			if (videoMode == VIDEOMODE_BLEND)
			{
				if (width <= 256)
				{
					copyWidth  = width  * 2;
					copyHeight = height;
					blitFn = S9xBlitPixBlend2x1;
				}
				else
				{
					copyWidth  = width;
					copyHeight = height;
					blitFn = S9xBlitPixBlend1x1;
				}
			}
			else
			if (height <= 256)
			{
				if (width <= 256)
				{
					copyWidth  = width  * 2;
					copyHeight = height * 2;

					switch (videoMode)
					{
						default:
						case VIDEOMODE_TV:			blitFn = S9xBlitPixTV2x2;			break;
						case VIDEOMODE_SUPEREAGLE:	blitFn = S9xBlitPixSuperEagle16;	break;
						case VIDEOMODE_2XSAI:		blitFn = S9xBlitPix2xSaI16;			break;
						case VIDEOMODE_SUPER2XSAI:	blitFn = S9xBlitPixSuper2xSaI16;	break;
						case VIDEOMODE_EPX:			blitFn = S9xBlitPixEPX16;			break;
						case VIDEOMODE_HQ2X:		blitFn = S9xBlitPixHQ2x16;			break;
					}
				}
				else
				{
					if (videoMode == VIDEOMODE_TV)
					{
						copyWidth  = width;
						copyHeight = height * 2;
						blitFn = S9xBlitPixTV1x2;
					}
					else
					{
						copyWidth  = width;
						copyHeight = height;
						blitFn = S9xBlitPixSimple1x1;
					}
				}
			}
			else
			{
				copyWidth  = width;
				copyHeight = height;
				blitFn = S9xBlitPixSimple1x1;
			}

			break;

		case 3:
			if (width <= 256 && height <= 256)
			{
				copyWidth  = width  * 3;
				copyHeight = height * 3;
				blitFn = S9xBlitPixHQ3x16;
			}
			else
			{
				copyWidth  = width;
				copyHeight = height;
				blitFn = S9xBlitPixSimple1x1;
			}

			break;

		case 4:
			if (width <= 256 && height <= 256)
			{
				copyWidth  = width  * 4;
				copyHeight = height * 4;
				blitFn = S9xBlitPixHQ4x16;
			}
			else
			if (width >  256 && height  > 256)
			{
				copyWidth  = width  * 2;
				copyHeight = height * 2;
				blitFn = S9xBlitPixHQ2x16;
			}
			else
			{
				copyWidth  = width;
				copyHeight = height;
				blitFn = S9xBlitPixSimple1x1;
			}

			break;

		case -1:
		case -2:
			copyWidth  = ntsc_width;
			copyHeight = height;

			if (width <= 256)
				blitFn = S9xBlitPixNTSC16;
			else
				blitFn = S9xBlitPixHiResNTSC16;

			break;
	}

	imageWidth[whichBuf]  = width;
	imageHeight[whichBuf] = height;

	if (multiprocessor)
	{
		MPWaitOnSemaphore(readySemaphore, kDurationForever);

		mpBlit->nx          = nx;
		mpBlit->blitFn      = blitFn;
		mpBlit->srcWidth    = width;
		mpBlit->srcHeight   = height;
		mpBlit->copyWidth   = copyWidth;
		mpBlit->copyHeight  = copyHeight;
		mpBlit->gfxBuffer   = GFX.Screen;

		MPNotifyQueue(taskQueue, (void *) kMPBlitFrame, 0, 0);

		whichBuf = 1 - whichBuf;
		GFX.Screen = gfxScreen[whichBuf];
	}
	else
		RenderBlitScreen(blitFn, nx, width, height, copyWidth, copyHeight, GFX.Screen);
}

static void S9xPutImageBlitGL2 (int blit_width, int blit_height)
{
	if ((prevBlitWidth != blit_width) || (prevBlitHeight != blit_height))
		windowResizeCount += 2;

	if (windowResizeCount > 0)
	{
		if (fullscreen)
		{
			CGLSetCurrentContext(glContext);

			glViewport(0, 0, glScreenW, glScreenH);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (glstretch)
			{
				int		sh  = (blit_width < blit_height) ? (blit_height >> 1) : ((blit_width > blit_height * 2) ? (blit_height << 1) : blit_height);
				float	fpw = (float) glScreenH / (float) sh * (float) blit_width;
				int		pw  = (int) (fpw + ((float) glScreenW - fpw) * (float) macAspectRatio / 10000.0);

				glViewport((glScreenW - pw) >> 1, 0, pw, glScreenH);
			}
			else
			{
				int		sw, sh;

				if (nx < 0)
				{
					sw = ntsc_width;
					sh = ((blit_height % SNES_HEIGHT) ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT) * 2;
				}
				else
				{
					sw = SNES_WIDTH * nx;
					sh = ((blit_height % SNES_HEIGHT) ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT) * nx;
				}

				glViewport((glScreenW - sw) >> 1, (glScreenH - sh) >> 1, sw, sh);
			}
		}
		else
		{
			int	ww = (int) gWindowRect.size.width,
				wh = (int) gWindowRect.size.height;

			aglSetCurrentContext(agContext);
			aglUpdateContext(agContext);

			glViewport(0, 0, ww, wh);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (windowExtend)
			{
				int	bh = (blit_height % SNES_HEIGHT) ? (SNES_HEIGHT_EXTENDED << 1) : (SNES_HEIGHT << 1);
				glViewport(0, ((kMacWindowHeight - bh) >> 1) * wh / kMacWindowHeight, ww, bh * wh / kMacWindowHeight);
			}
		}

		if (!ciFilterEnable)
		{
			if (nx < 0)
				textureNum = (blit_height > 256) ? kGLNTS512 : kGLNTS256;
			else
			{
				switch (blit_width / SNES_WIDTH)
				{
					default:
					case 1:
					case 2:	textureNum = kGLBlit2x;	break;
					case 3:	textureNum = kGLBlit3x;	break;
					case 4:	textureNum = kGLBlit4x;	break;
				}
			}

			if (nx < 0)
			{
				int	sh = (blit_height > 256) ?  512 : 256;
				OpenGL.vertex[textureNum][2] = OpenGL.vertex[textureNum][4] = blit_width  / 1024.0f;
				OpenGL.vertex[textureNum][5] = OpenGL.vertex[textureNum][7] = blit_height / (float) sh;
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 1024);
			}
			else
			{
				if (OpenGL.rangeExt)
				{
					OpenGL.vertex[textureNum][2] = OpenGL.vertex[textureNum][4] = blit_width;
					OpenGL.vertex[textureNum][5] = OpenGL.vertex[textureNum][7] = blit_height;
					glPixelStorei(GL_UNPACK_ROW_LENGTH, blit_width);
				}
				else
				{
					int	sl = (blit_width  > 512) ? 1024 : 512;
					int	sh = (blit_height > 512) ? 1024 : 512;
					OpenGL.vertex[textureNum][2] = OpenGL.vertex[textureNum][4] = blit_width  / (float) sl;
					OpenGL.vertex[textureNum][5] = OpenGL.vertex[textureNum][7] = blit_height / (float) sh;
					glPixelStorei(GL_UNPACK_ROW_LENGTH, (blit_width > 512) ? 1024 : ((blit_width > 256) ? 512 : 256));
				}
			}

			glBindTexture(OpenGL.target, OpenGL.textures[textureNum]);
		}
		else
		{
			int	sl = OpenGL.rangeExt ? blit_width : ((blit_width > 512) ? 1024 : ((blit_width > 256) ? 512 : 256));

			glPixelStorei(GL_UNPACK_ROW_LENGTH, sl);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, blit_width, 0, blit_height, -1, 1);

			if (cgBlitImage)
				CGImageRelease(cgBlitImage);
			cgBlitImage = CreateBlitScreenCGImage(blit_width, blit_height, sl << 1, blitGLBuffer);
		}

		prevBlitWidth  = blit_width;
		prevBlitHeight = blit_height;

		windowResizeCount--;
	}

	if (!ciFilterEnable)
	{
		glTexSubImage2D(OpenGL.target, 0, 0, 0, OpenGL.texW[textureNum], OpenGL.texH[textureNum], OpenGL.format, OpenGL.type, blitGLBuffer);

		if (!screencurvature)
		{
			glBegin(GL_QUADS);

			glTexCoord2fv(&OpenGL.vertex[textureNum][6]);
			glVertex2f(-1.0f, -1.0f);
			glTexCoord2fv(&OpenGL.vertex[textureNum][4]);
			glVertex2f( 1.0f, -1.0f);
			glTexCoord2fv(&OpenGL.vertex[textureNum][2]);
			glVertex2f( 1.0f,  1.0f);
			glTexCoord2fv(&OpenGL.vertex[textureNum][0]);
			glVertex2f(-1.0f,  1.0f);

			glEnd();
		}
		else
		{
			GLfloat *t, *s;
			int		tex;

			if (nx < 0)
				tex = (blit_height % SNES_HEIGHT) ? kSCNTExtend : kSCNTNormal;
			else
			if (blit_width > blit_height * 2)
			{
				if (blit_width / SNES_WIDTH != 3)
					tex = (blit_height % SNES_HEIGHT) ? kSC2xEHiRes : kSC2xNHiRes;
				else
					tex = (blit_height % SNES_HEIGHT) ? kSC3xEHiRes : kSC3xNHiRes;
			}
			else
			if (blit_width > blit_height)
			{
				if (blit_width / SNES_WIDTH != 3)
					tex = (blit_height % SNES_HEIGHT) ? kSC2xExtend : kSC2xNormal;
				else
					tex = (blit_height % SNES_HEIGHT) ? kSC3xExtend : kSC3xNormal;
			}
			else
				tex = (blit_height % SNES_HEIGHT) ? kSC2xEInter : kSC2xNInter;

			t = scTexArray[tex];
			s = scScnArray;

			for (int i = 0; i < kSCMeshY; i++)
			{
				glTexCoordPointer(2, GL_FLOAT, 0, t);
				glVertexPointer(3, GL_FLOAT, 0, s);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, (kSCMeshX + 1) * 2);

				t += (kSCMeshX + 1) * 2 * 2;
				s += (kSCMeshX + 1) * 2 * 3;
			}
		}

		glFinishObjectAPPLE(GL_TEXTURE, OpenGL.textures[textureNum]);
	}
	else
	{
		CGRect	src;

		src = CGRectMake(0, 0, blit_width, blit_height);
		DrawWithCoreImageFilter(src, cgBlitImage);
	}

	if (fullscreen)
		CGLFlushDrawable(glContext);
	else
		aglSwapBuffers(agContext);
}

void S9xTextMode (void)
{
	return;
}

void S9xGraphicsMode (void)
{
	return;
}

void S9xSetPalette (void)
{
	return;
}
