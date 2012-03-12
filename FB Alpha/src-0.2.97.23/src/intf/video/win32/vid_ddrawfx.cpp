// Software blitter effects via DirectDraw
#include "burner.h"

#if !defined BUILD_X64_EXE
// #include "vid_directx_support.h"
 #include "vid_softfx.h"
#endif

#include <InitGuid.h>
#define DIRECT3D_VERSION 0x0700							// Use this Direct3D version

#if defined BUILD_X64_EXE
// #include "vid_directx_support.h"
 #include "vid_softfx.h"
#endif

#include "ddraw_core.h"

static IDirectDraw7* BlitFXDD = NULL;				// DirectDraw interface
static IDirectDrawSurface7* BlitFXPrim = NULL;		// Primary surface
static IDirectDrawSurface7* BlitFXBack = NULL;		// Back buffer surface

static int nGameWidth = 0, nGameHeight = 0;			// screen size
IDirectDrawSurface7* pddsBlitFX[2] = {NULL, };		// The image surfaces

static int nSize;
static int nUseBlitter;

static int nUseSys;
static int nDirectAccess = 1;
static int nRotateGame = 0;

static int PrimClear()
{
	if (BlitFXPrim == NULL) {
		return 1;
	}
	VidSClearSurface(BlitFXPrim, 0, NULL);			// Clear 1st page

	if (BlitFXBack == NULL) {
		return 0;
	}

	VidSClearSurface(BlitFXBack, 0, NULL);			// Clear 2nd page
	BlitFXPrim->Flip(NULL, DDFLIP_WAIT);			// wait till the flip actually occurs
	VidSClearSurface(BlitFXBack, 0, NULL);			// Clear 3rd page

	return 0;
}

static int AutodetectUseSys()
{
	// Try to autodetect the best secondary buffer type to use, based on the cards capabilities
	DDCAPS ddc;
	if (BlitFXDD == NULL) {
		return 1;
	}
	memset(&ddc, 0, sizeof(ddc));
	ddc.dwSize = sizeof(ddc);
	BlitFXDD->GetCaps(&ddc, NULL);

	if (ddc.dwCaps & DDCAPS_BLTSTRETCH) {	// If it can do a hardware stretch use video memory
		return 0;
	} else {								// Otherwise use system memory:
		return 1;
	}
}

// Create a secondary DD surface for the screen
static int BlitFXMakeSurf()
{
	int nRet;
	DDSURFACEDESC2 ddsd;

	if (BlitFXDD == NULL) {
		return 1;
	}

	nUseSys = nVidTransferMethod;
	if (nUseSys < 0) {
		nUseSys = AutodetectUseSys();
	}

	nDirectAccess = 0;
	if (nUseSys == 0 && nUseBlitter != 0) {
		nDirectAccess = ((nVidBlitterOpt[nVidSelect] >> 9) & 1) ^ 1;
	}
	
	pddsBlitFX[0] = NULL;
	pddsBlitFX[1] = NULL;

	// Try to allocate buffer in Video memory if required, always allocate one in System memory
	if (nUseSys == 0) {
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

		ddsd.dwWidth = nGameWidth * nSize;
		ddsd.dwHeight = nGameHeight * nSize;

		nRet = BlitFXDD->CreateSurface(&ddsd, &pddsBlitFX[0], NULL);

		if (nRet < 0 || pddsBlitFX[0] == NULL) {									// Allocation in Video meory has failed, so use System memory only
			pddsBlitFX[0] = NULL;
			nDirectAccess = 0;
			nUseSys = 1;
		}
	}

	if (nDirectAccess == 0) {
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		ddsd.dwWidth = nGameWidth * nSize;
		ddsd.dwHeight = nGameHeight * nSize;

		nRet = BlitFXDD->CreateSurface(&ddsd, &pddsBlitFX[1], NULL);

		if (nRet < 0 || pddsBlitFX[1] == NULL) {
			return 1;
		}
	}

	VidSClearSurface(pddsBlitFX[1 ^ nDirectAccess], 0, NULL);

	return 0;
}

static int PrimInit(int bTriple)
{
	DDSURFACEDESC2 ddsd;
	int nRet = 0;

	// Create the primary surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	if (bTriple) {
		// Make a primary surface capable of triple bufferring
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 2;
	} else {
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	BlitFXPrim = NULL;
	nRet = BlitFXDD->CreateSurface(&ddsd, &BlitFXPrim, NULL);
	if (nRet < 0 || BlitFXPrim == NULL) {
		return 1;
	}

	if (bTriple == 0) {								// We are done
		return 0;
	}

	// Get the back buffer
	memset(&ddsd.ddsCaps, 0, sizeof(ddsd.ddsCaps));
	ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

	BlitFXBack = NULL;
	nRet = BlitFXPrim->GetAttachedSurface(&ddsd.ddsCaps, &BlitFXBack);
	if (nRet < 0 || BlitFXBack == NULL) {			// Failed to make triple buffer
		RELEASE(BlitFXPrim)
		return 1;
	}

	PrimClear();									// Clear all three pages
	return 0;
}

static int BlitFXExit()
{
	RELEASE(pddsBlitFX[0]);
	RELEASE(pddsBlitFX[1]);

	VidSFreeVidImage();

	nRotateGame = 0;

	return 0;
}

static int BlitFXInit()
{
	if (BlitFXDD == NULL) {
		return 1;
	}

	if (nRotateGame & 1) {
		nVidImageWidth = nGameHeight;
		nVidImageHeight = nGameWidth;
	} else {
		nVidImageWidth = nGameWidth;
		nVidImageHeight = nGameHeight;
	}

	RECT rect = { 0, 0, 0, 0 };
	GetClientScreenRect(hVidWnd, &rect);
	if (!nVidFullscreen) {
		rect.top += 0 /*nMenuHeight*/;
	}

	if (nUseBlitter >= FILTER_SUPEREAGLE && nUseBlitter <= FILTER_SUPER_2XSAI) {
		nVidImageDepth = 16;								// Use 565 format
	} else {
		nVidImageDepth = VidSGetSurfaceDepth(BlitFXPrim);	// Use color depth of primary surface
	}
	nVidImageBPP = (nVidImageDepth + 7) >> 3;
	nBurnBpp = nVidImageBPP;								// Set Burn library Bytes per pixel

	// Use our callback to get colors:
	SetBurnHighCol(nVidImageDepth);

	// Make the normal memory buffer
	if (VidSAllocVidImage()) {
		BlitFXExit();
		return 1;
	}

	// Make the DirectDraw secondary surface
	if (BlitFXMakeSurf()) {
		BlitFXExit();
		return 1;
	}

	return 0;
}

static int Exit()
{
	VidSExitOSD();

	BlitFXExit();

	VidSRestoreGamma();

	RELEASE(BlitFXPrim);							// a single call releases all surfaces
	BlitFXBack = NULL;

	VidSRestoreScreenMode();

	VidSExit();

	RELEASE(BlitFXDD)

	return 0;
}

static int Init()
{
	if (hScrnWnd == NULL) {
		return 1;
	}

	hVidWnd = nVidFullscreen ? hScrnWnd : hVideoWindow;								// Use Screen window for video

	nUseBlitter = nVidBlitterOpt[nVidSelect] & 0xFF;

	// Get pointer to DirectDraw device
	_DirectDrawCreateEx(NULL, (void**)&BlitFXDD, IID_IDirectDraw7, NULL);

	VidSInit(BlitFXDD);

	nGameWidth = nVidImageWidth; nGameHeight = nVidImageHeight;

	nRotateGame = 0;
	if (bDrvOkay) {
		// Get the game screen size
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);

	    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
			if (nVidRotationAdjust & 1) {
				int n = nGameWidth;
				nGameWidth = nGameHeight;
				nGameHeight = n;
				nRotateGame |= (nVidRotationAdjust & 2);
			} else {
				nRotateGame |= 1;
			}
		}

		if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
			nRotateGame ^= 2;
		}
	}

	BlitFXPrim = NULL;								// No primary surface yet
	BlitFXBack = NULL;

	bVidScanlines = 0;								// !!!
	nSize = VidSoftFXGetZoom(nUseBlitter);

	// Remember the changes to the display
	if (nVidFullscreen) {
		int nZoom = nSize;
		if ((nVidBlitterOpt[nVidSelect] & 0x0100) == 0) {
			nZoom = nScreenSize;
		}

		if (VidSEnterFullscreenMode(nZoom, VidSoftFXCheckDepth(nUseBlitter, 16))) {
			Exit();
			return 1;
		}

		if (bVidTripleBuffer) {
			if (PrimInit(1)) {						// Try to make triple buffer
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_TRIPLE));
				FBAPopupDisplay(PUF_TYPE_WARNING);

				// If we fail, fail entirely and make a normal buffer
				RELEASE(BlitFXBack);
				RELEASE(BlitFXPrim);
			}
		}
	} else {
		BlitFXDD->SetCooperativeLevel(hVidWnd, DDSCL_NORMAL);
	}

	if (BlitFXPrim == NULL) {
		// No primary surface yet, so try normal
		if (PrimInit(0)) {
			RELEASE(BlitFXBack);
			RELEASE(BlitFXPrim);
		}
	}

	if (BlitFXPrim == NULL) {						// No primary surface
		Exit();
		return 1;
	}

	if (nVidFullscreen) {
		BlitFXDD->Compact();
	}

	VidSClipperInit(BlitFXPrim);

	VidSSetupGamma(BlitFXPrim);

	// Initialize the buffer surfaces
	BlitFXInit();
	
	if (VidSoftFXInit(nUseBlitter, nRotateGame)) {
		if (VidSoftFXInit(0, nRotateGame)) {
			Exit();
			return 1;
		}
	}

	VidSInitOSD(4 - nSize);

	return 0;
}

static int vidScale(RECT* pRect, int nWidth, int nHeight)
{
	if (nVidBlitterOpt[nVidSelect] & 0x0100) {
		return VidSoftFXScale(pRect, nWidth, nHeight);
	}

	return VidSScaleImage(pRect, nWidth, nHeight, false);
}

// Copy BlitFXsMem to pddsBlitFX
static int MemToSurf()
{
	if (pddsBlitFX == NULL) {
		return 1;
	}

	if (BlitFXPrim->IsLost()) {						// We've lost control of the screen
		return 1;
	}

	VidSoftFXApplyEffectDirectX(pddsBlitFX[1 ^ nDirectAccess], NULL);

	if (nUseSys == 0 && nDirectAccess == 0) {
		DDSURFACEDESC2 ddsd;
		DDSURFACEDESC2 ddsdVid;

		// Lock the surface so we can write to it
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		pddsBlitFX[1]->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (ddsd.lpSurface == NULL) {
			return 1;
		}
		unsigned char* Surf = (unsigned char*)ddsd.lpSurface;
		int nPitch = ddsd.lPitch;

		// Lock the surface so we can write to it
		memset(&ddsdVid, 0, sizeof(ddsdVid));
		ddsdVid.dwSize = sizeof(ddsdVid);
		pddsBlitFX[0]->Lock(NULL, &ddsdVid, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (ddsdVid.lpSurface == NULL) {
			return 1;
		}

		unsigned char *pd, *ps;

		unsigned char* VidSurf = (unsigned char*)ddsdVid.lpSurface;
		int nVidPitch = ddsdVid.lPitch;
		int nHeight = nGameHeight * nSize;

		pd = VidSurf; ps = Surf;
		for (int y = 0; y < nHeight; y++, pd += nVidPitch, ps += nPitch) {
			memcpy(pd, ps, nPitch);
		}

		pddsBlitFX[0]->Unlock(NULL);
		pddsBlitFX[1]->Unlock(NULL);
	}

	return 0;
}

// Run one frame and render the screen
static int Frame(bool bRedraw)								// bRedraw = 0
{
	if (pVidImage == NULL) {
		return 1;
	}

	if (BlitFXPrim->IsLost()) {						// We've lost control of the screen

		if (VidSRestoreOSD()) {
			return 1;
		}

		if (FAILED(BlitFXDD->RestoreAllSurfaces())) {
			return 1;
		}

		PrimClear();
	}

	if (bDrvOkay) {
		if (bRedraw) {								// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();						// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();							// Run one frame and draw the screen
		}
	}

	MemToSurf();									// Copy the memory buffer to the directdraw buffer for later blitting

	return 0;
}

// Paint the BlitFX surface onto the primary surface
static int Paint(int bValidate)
{
	RECT Dest = {0, 0, 0, 0};

	if (BlitFXPrim == NULL || pddsBlitFX[nUseSys] == NULL) {
		return 1;
	}

	if (BlitFXPrim->IsLost()) {						// We've lost control of the screen
		return 1;
	}

	GetClientScreenRect(hVidWnd, &Dest);
	if (!nVidFullscreen) {
		Dest.top += 0 /*nMenuHeight*/;
	}

	if (bVidArcaderes && nVidFullscreen) {
		Dest.left = (Dest.right + Dest.left) / 2;
		Dest.left -= nGameWidth / 2;
		Dest.right = Dest.left + nGameWidth;

		Dest.top = (Dest.top + Dest.bottom) / 2;
		Dest.top -= nGameHeight / 2;
		Dest.bottom = Dest.top + nGameHeight;
	} else {
		if (!bVidFullStretch) {
			vidScale(&Dest, nGameWidth, nGameHeight);
		}
	}

	if (bValidate & 2) {
		MemToSurf();									// Copy the memory buffer to the directdraw buffer for later blitting
	}

	RECT rect = { 0, 0, nGameWidth, nGameHeight };
	rect.right *= nSize;
	rect.bottom *= nSize;

	// Display OSD text message
	VidSDisplayOSD(pddsBlitFX[nUseSys], &rect, 0);

	if (bVidVSync && !nVidFullscreen) { BlitFXDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL); }

	if (BlitFXBack != NULL) {
		// Triple bufferring
		if (BlitFXBack->Blt(&Dest, pddsBlitFX[nUseSys], NULL, DDBLT_WAIT, NULL) < 0) {
			return 1;
		}
		BlitFXPrim->Flip(NULL, DDFLIP_WAIT);
	} else {
		// Normal
		if (BlitFXPrim->Blt(&Dest, pddsBlitFX[nUseSys], NULL, DDBLT_WAIT, NULL) < 0) {
			return 1;
		}
	}

	

/*
	DWORD lpdwScanLine;	
	RECT window;
	GetWindowRect(hVidWnd, &window);

	while(1) {
		BlitFXDD->GetScanLine(&lpdwScanLine);
		if (lpdwScanLine >= (unsigned int)window.bottom) {
			break;
		}
		//Sleep(1);
	}
*/

	if (bValidate & 1) {
		// Validate the rectangle we just drew
		POINT c = {0, 0};
		ClientToScreen(hVidWnd, &c);
		Dest.left -= c.x; Dest.right -= c.x;
		Dest.top -= c.y; Dest.bottom -= c.y;
		ValidateRect(hVidWnd, &Dest);
	}

	return 0;
}

static int GetSettings(InterfaceInfo* pInfo)
{
	TCHAR szString[MAX_PATH] = _T("");

	if (nVidFullscreen && BlitFXBack) {
		IntInfoAddStringModule(pInfo, _T("Using a triple buffer"));
	} else {
		IntInfoAddStringModule(pInfo, _T("Using Blt() to transfer the image"));
	}
	
	_sntprintf(szString, MAX_PATH, _T("Prescaling using %s (%ix zoom)"), VidSoftFXGetEffect(nUseBlitter), nSize);
	IntInfoAddStringModule(pInfo, szString);

	if (nUseSys) {
		IntInfoAddStringModule(pInfo, _T("Using system memory"));
	} else {
		if (nDirectAccess) {
			IntInfoAddStringModule(pInfo, _T("Using video memory for effect buffers"));
		} else {
			IntInfoAddStringModule(pInfo, _T("Using video memory for the final blit"));
		}
	}

	if (nRotateGame) {
		IntInfoAddStringModule(pInfo, _T("Using software rotation"));
	}

	return 0;
}

// The Video Output plugin:
struct VidOut VidOutDDrawFX = { Init, Exit, Frame, Paint, vidScale, GetSettings, _T("DirectDraw7 Software Effects video output") };
