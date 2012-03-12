// Video Output - (calls all the Vid Out plugins)
#include "burner.h"

#define DEFAULT_IMAGE_WIDTH (304)
#define DEFAULT_IMAGE_HEIGHT (224)

#define ENABLE_PREVIEW

#if defined (BUILD_WIN32)
	extern struct VidOut VidOutDDraw;
	extern struct VidOut VidOutD3D;
	extern struct VidOut VidOutDDrawFX;
	extern struct VidOut VidOutDX9;
	extern struct VidOut VidOutDX9Alt;
#elif defined (BUILD_SDL)
	extern struct VidOut VidOutSDLOpenGL;
	extern struct VidOut VidOutSDLFX;
#elif defined (_XBOX)
	extern struct VidOut VidOutD3D;
#endif

static struct VidOut *pVidOut[] = {
#if defined (BUILD_WIN32)
	&VidOutDDraw,
	&VidOutD3D,
	&VidOutDDrawFX,
	&VidOutDX9,
	&VidOutDX9Alt,
#elif defined (BUILD_SDL)
	&VidOutSDLOpenGL,
	&VidOutSDLFX,
#elif defined (_XBOX)
	&VidOutD3D,
#endif
};

#define VID_LEN (sizeof(pVidOut) / sizeof(pVidOut[0]))

INT64 nVidBlitterOpt[VID_LEN] = {0, };			// Options for the blitter module (meaning depens on module)

static InterfaceInfo VidInfo = { NULL, NULL, NULL };

UINT32 nVidSelect = 0;					// Which video output is selected
static UINT32 nVidActive = 0;

bool bVidOkay = false;

INT32 nVidWidth		= 640, nVidHeight		= 480, nVidDepth = 16, nVidRefresh = 0;

INT32 nVidHorWidth	= 640, nVidHorHeight	= 480;	// Default Horizontal oritated resolution
INT32 nVidVerWidth	= 640, nVidVerHeight	= 480;	// Default Vertical oriented resoultion

INT32 nVidFullscreen = 0;
INT32 bVidFullStretch = 0;						// 1 = stretch to fill the entire window/screen
INT32 bVidCorrectAspect = 1;						// 1 = stretch to fill the window/screen while maintaining the correct aspect ratio
INT32 bVidVSync = 0;								// 1 = sync blits/pageflips/presents to the screen
INT32 bVidTripleBuffer = 0;						// 1 = use triple buffering
INT32 bVidBilinear = 1;							// 1 = enable bi-linear filtering (D3D blitter)
INT32 bVidScanlines = 0;							// 1 = draw scanlines
INT32 bVidScanRotate = 1;							// 1 = rotate scanlines and RGB effects for rotated games
INT32 bVidScanBilinear = 1;						// 1 = use bi-linear filtering for scanlines (D3D blitter, debug variable)
INT32 nVidScanIntensity = 0x00BFBFBF;				// The maximum colour-value for the scanlines (D3D blitter)
INT32 bVidScanHalf = 1;							// Draw scanlines at half intensity instead of black (DDraw blitter)
INT32 bVidScanDelay = 0;							// Blend the previous image with the current one
INT32 nVidFeedbackIntensity = 0x40;				// Blend factor for previous frame (D3D blitter)
INT32 nVidFeedbackOverSaturation = 0x00;			// Add this to the current frame blend factor
INT32 bVidUseHardwareGamma = 1;					// Use the video hardware when correcting gamma
INT32 bVidAutoSwitchFull = 0;						// 1 = auto switch to fullscreen on loading driver
INT32 bVidArcaderes = 0;							// Use game resolution for fullscreen modes

INT32 bVidArcaderesHor = 0;
INT32 bVidArcaderesVer = 0;

INT32 nVidRotationAdjust = 0;						// & 1: do not rotate the graphics for vertical games,  & 2: Reverse flipping for vertical games
INT32 bVidForce16bit = 1;							// Emulate the game in 16-bit even when the screen is 32-bit (D3D blitter)
INT32 bVidForceFlip = 0;							// Force flipping (DDraw blitter, hardware detection seems to fail on all? graphics hardware)
INT32 nVidTransferMethod = -1;					// How to transfer the game image to video memory and/or a texture --
												//  0 = blit from system memory / use driver/DirectX texture management
												//  1 = copy to a video memory surface, then use bltfast()
												// -1 = autodetect for ddraw, equals 1 for d3d
float fVidScreenAngle = 0.174533f;				// The angle at which to tilt the screen backwards (in radians, D3D blitter)
float fVidScreenCurvature = 0.698132f;			// The angle of the maximum screen curvature (in radians, D3D blitter)
double dVidCubicB = 0.0;						// Paremeters for the cubic filter (default is the CAtmull-Rom spline, DX9 blitter)
double dVidCubicC = 0.5;						//

INT32 bVidDX9Bilinear = 1;							// 1 = enable bi-linear filtering (D3D9 Alt blitter)
INT32 bVidHardwareVertex = 0;			// 1 = use hardware vertex processing
INT32 bVidMotionBlur = 0;				// 1 = motion blur

#ifdef BUILD_WIN32
 HWND hVidWnd = NULL;							// Actual window used for video
#endif

#if defined (_XBOX)
  HWND hVidWnd = NULL;							// Actual window used for video
#endif

INT32 nVidScrnWidth = 0, nVidScrnHeight = 0;		// Actual Screen dimensions (0 if in windowed mode)
INT32 nVidScrnDepth = 0;							// Actual screen depth

INT32 nVidScrnAspectX = 4, nVidScrnAspectY = 3;	// Aspect ratio of the display screen

UINT8* pVidImage = NULL;				// Memory buffer
INT32 nVidImageWidth = DEFAULT_IMAGE_WIDTH;		// Memory buffer size
INT32 nVidImageHeight = DEFAULT_IMAGE_HEIGHT;		//
INT32 nVidImageLeft = 0, nVidImageTop = 0;		// Memory buffer visible area offsets
INT32 nVidImagePitch = 0, nVidImageBPP = 0;		// Memory buffer pitch and bytes per pixel
INT32 nVidImageDepth = 0;							// Memory buffer bits per pixel

UINT32 (__cdecl *VidHighCol) (INT32 r, INT32 g, INT32 b, INT32 i);
static bool bVidRecalcPalette;

static UINT8* pVidTransImage = NULL;
static UINT32* pVidTransPalette = NULL;

static UINT32 __cdecl HighCol15(INT32 r, INT32 g, INT32 b, INT32  /* i */)
{
	UINT32 t;

	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;

	return t;
}

INT32 VidSelect(UINT32 nPlugin)
{
	if (nPlugin < VID_LEN) {
		nVidSelect = nPlugin;
		return 0;
	} else {
		return 1;
	}
}

// Forward to VidOut functions
INT32 VidInit()
{
#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	HBITMAP hbitmap = NULL;
	BITMAP bitmap;
#endif

	INT32 nRet = 1;

	VidExit();

#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	if (!bDrvOkay) {
		//hbitmap = (HBITMAP)LoadImage(hAppInst, _T("BMP_SPLASH"), IMAGE_BITMAP, 304, 224, 0);
		hbitmap = (HBITMAP)LoadImage(hAppInst, MAKEINTRESOURCE(BMP_SPLASH), IMAGE_BITMAP, 304, 224, 0);
		
		GetObject(hbitmap, sizeof(BITMAP), &bitmap);

		nVidImageWidth = bitmap.bmWidth; nVidImageHeight = bitmap.bmHeight;
		nVidImageLeft = nVidImageTop = 0;
	}
#endif

#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	if ((nVidSelect < VID_LEN) && (bDrvOkay || hbitmap)) {
#else
	if ((nVidSelect < VID_LEN) && bDrvOkay) {
#endif
		nVidActive = nVidSelect;
		if ((nRet = pVidOut[nVidActive]->Init()) == 0) {
			nBurnBpp = nVidImageBPP;								// Set Burn library Bytes per pixel

			bVidOkay = true;

			if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nVidImageBPP > 2) {
				nBurnBpp = 2;

				pVidTransPalette = (UINT32*)malloc(32768 * sizeof(UINT32));
				pVidTransImage = (UINT8*)malloc(nVidImageWidth * nVidImageHeight * sizeof(INT16));

				BurnHighCol = HighCol15;

				if (pVidTransPalette == NULL || pVidTransImage == NULL) {
					VidExit();
					nRet = 1;
				}
			}
		}
	}

#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	if (bVidOkay && hbitmap) {
		BITMAPINFO bitmapinfo;
		UINT8* pLineBuffer = (UINT8*)malloc(bitmap.bmWidth * 4);
		HDC hDC = GetDC(hVidWnd);

		if (hDC && pLineBuffer) {

			memset(&bitmapinfo, 0, sizeof(BITMAPINFO));
			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFO);
			bitmapinfo.bmiHeader.biWidth = bitmap.bmWidth;
			bitmapinfo.bmiHeader.biHeight = bitmap.bmHeight;
			bitmapinfo.bmiHeader.biPlanes = 1;
			bitmapinfo.bmiHeader.biBitCount = 24;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;
			
			for (INT32 y = 0; y < nVidImageHeight; y++) {
				UINT8* pd = pVidImage + y * nVidImagePitch;
				UINT8* ps = pLineBuffer;

				GetDIBits(hDC, hbitmap, nVidImageHeight - 1 - y, 1, ps, &bitmapinfo, DIB_RGB_COLORS);

				for (INT32 x = 0; x < nVidImageWidth; x++, ps += 3) {
					UINT32 nColour = VidHighCol(ps[2], ps[1], ps[0], 0);
					switch (nVidImageBPP) {
						case 2:
							*((UINT16*)pd) = (UINT16)nColour;
							pd += 2;
							break;
						case 3:
							pd[0] = (nColour >> 16) & 0xFF;
							ps[1] = (nColour >>  8) & 0xFF;
							pd[2] = (nColour >>  0) & 0xFF;
							pd += 3;
							break;
						case 4:
							*((UINT32*)pd) = nColour;
							pd += 4;
							break;
					}
				}
			}
		}
		if (hDC) {
			ReleaseDC(hVidWnd, hDC);
		}
		if (pLineBuffer) {
			free(pLineBuffer);
			pLineBuffer = NULL;
		}
	}

	if (hbitmap) {
		DeleteObject(hbitmap);
	}
#endif

	return nRet;
}

INT32 VidExit()
{
	IntInfoFree(&VidInfo);

	if (bVidOkay) {
		INT32 nRet = pVidOut[nVidActive]->Exit();

#if defined (BUILD_WIN32)
		hVidWnd = NULL;
#endif

		bVidOkay = false;

		nVidImageWidth = DEFAULT_IMAGE_WIDTH; nVidImageHeight = DEFAULT_IMAGE_HEIGHT;

		nVidImageBPP = nVidImageDepth = 0;
		nBurnPitch = nBurnBpp = 0;

		if (pVidTransPalette) {
			free(pVidTransPalette);
			pVidTransPalette = NULL;
		}
		if (pVidTransImage) {
			free(pVidTransImage);
			pVidTransImage = NULL;
		}

		return nRet;
	} else {
		return 1;
	}
}

static INT32 VidDoFrame(bool bRedraw)
{
	INT32 nRet;
	
	if (pVidTransImage) {
		UINT16* pSrc = (UINT16*)pVidTransImage;
		UINT8* pDest = pVidImage;

		if (bVidRecalcPalette) {
			for (INT32 r = 0; r < 256; r += 8) {
				for (INT32 g = 0; g < 256; g += 8) {
					for (INT32 b = 0; b < 256; b += 8) {
						pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = VidHighCol(r | (r >> 5), g | (g >> 5), b | (b >> 5), 0);
					}
				}
			}

			bVidRecalcPalette = false;
		}

		pBurnDraw = pVidTransImage;
		nBurnPitch = nVidImageWidth * 2;

		nRet = pVidOut[nVidActive]->Frame(bRedraw);

		pBurnDraw = NULL;
		nBurnPitch = 0;

		switch (nVidImageBPP) {
			case 3: {
				for (INT32 y = 0; y < nVidImageHeight; y++, pSrc += nVidImageWidth, pDest += nVidImagePitch) {
					for (INT32 x = 0; x < nVidImageWidth; x++) {
						UINT32 c = pVidTransPalette[pSrc[x]];
						*(pDest + (x * 3) + 0) = c & 0xFF;
						*(pDest + (x * 3) + 1) = (c >> 8) & 0xFF;
						*(pDest + (x * 3) + 2) = c >> 16;
					}
				}
				break;
			}
			case 4: {
				for (INT32 y = 0; y < nVidImageHeight; y++, pSrc += nVidImageWidth, pDest += nVidImagePitch) {
					for (INT32 x = 0; x < nVidImageWidth; x++) {
						((UINT32*)pDest)[x] = pVidTransPalette[pSrc[x]];
					}
				}
				break;
			}
		}
	} else {
		pBurnDraw = pVidImage;
		nBurnPitch = nVidImagePitch;

		nRet = pVidOut[nVidActive]->Frame(bRedraw);

		pBurnDraw = NULL;
		nBurnPitch = 0;
	}

	return nRet;
}

INT32 VidReInitialise()
{
	if (pVidTransImage) {
		free(pVidTransImage);
		pVidTransImage = NULL;
	}
	pVidTransImage = (UINT8*)malloc(nVidImageWidth * nVidImageHeight * sizeof(INT16));
	
	return 0;
}

INT32 VidFrame()
{
	if (bVidOkay && bDrvOkay) {
		return VidDoFrame(0);
	} else {
		return 1;
	}
}

INT32 VidRedraw()
{
	if (bVidOkay /* && bDrvOkay */) {
		return VidDoFrame(1);
	} else {
		return 1;
	}
}

INT32 VidRecalcPal()
{
	bVidRecalcPalette = true;

	return BurnRecalcPal();
}

// If bValidate & 1, the video code should use ValidateRect() to validate the rectangle it draws.
INT32 VidPaint(INT32 bValidate)
{
	if (bVidOkay /* && bDrvOkay */) {
		return pVidOut[nVidActive]->Paint(bValidate);
	} else {
		return 1;
	}
}

INT32 VidImageSize(RECT* pRect, INT32 nGameWidth, INT32 nGameHeight)
{
 	if (bVidOkay) {
		return pVidOut[nVidActive]->ImageSize(pRect, nGameWidth, nGameHeight);
	} else {
		return pVidOut[nVidSelect]->ImageSize(pRect, nGameWidth, nGameHeight);
	}
}

const TCHAR* VidGetModuleName()
{
	const TCHAR* pszName = NULL;

	if (bVidOkay) {
		pszName = pVidOut[nVidActive]->szModuleName;
	} else {
		pszName = pVidOut[nVidSelect]->szModuleName;
	}

	if (pszName) {
		return pszName;
	}
#if defined (BUILD_WIN32)
	return FBALoadStringEx(hAppInst, IDS_ERR_UNKNOWN, true);
#else
	return "There was an error with the video";
#endif
}

InterfaceInfo* VidGetInfo()
{
	if (IntInfoInit(&VidInfo)) {
		IntInfoFree(&VidInfo);
		return NULL;
	}

	if (bVidOkay) {
		TCHAR szString[MAX_PATH] = _T("");
		RECT rect;

		VidInfo.pszModuleName = pVidOut[nVidActive]->szModuleName;

#if defined (BUILD_WIN32)
		GetClientScreenRect(hVidWnd, &rect);
		if (nVidFullscreen == 0) {
			rect.top += nMenuHeight;
			_sntprintf(szString, MAX_PATH, _T("Running in windowed mode, $ix%i, %ibpp"), rect.right - rect.left, rect.bottom - rect.top, nVidScrnDepth);
		} else {
			_sntprintf(szString, MAX_PATH, _T("Running fullscreen, $ix$i, %ibpp"), nVidScrnWidth, nVidScrnHeight, nVidScrnDepth);
		}
#elif defined (BUILD_SDL)
		_sntprintf(szString, MAX_PATH, _T("Filler for fullscreen/windowed mode & image size"));
#endif

		IntInfoAddStringInterface(&VidInfo, szString);

		_sntprintf(szString, MAX_PATH, _T("Source image %ix%i, %ibpp"), nVidImageWidth, nVidImageHeight, nVidImageDepth);
		IntInfoAddStringInterface(&VidInfo, szString);

		if (pVidTransImage) {
			_sntprintf(szString, MAX_PATH, _T("Using generic software 15->%ibpp wrapper"), nVidImageDepth);
			IntInfoAddStringInterface(&VidInfo, szString);
		}

	 	if (pVidOut[nVidActive]->GetPluginSettings) {
			pVidOut[nVidActive]->GetPluginSettings(&VidInfo);
		}
	} else {
		IntInfoAddStringInterface(&VidInfo, _T("Video plugin not initialised"));
	}

	return &VidInfo;
}
