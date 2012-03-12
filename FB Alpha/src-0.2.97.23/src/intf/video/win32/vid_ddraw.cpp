// DirectDraw blitter

#include "burner.h"

#if !defined BUILD_X64_EXE
 #include "vid_directx_support.h" 
#endif

#include <InitGuid.h>
#define DIRECT3D_VERSION 0x0700							// Use this Direct3D version

#if defined BUILD_X64_EXE
 #include "vid_directx_support.h" 
#endif

#include "ddraw_core.h"

static IDirectDraw7* DtoDD = NULL;				// DirectDraw interface
static IDirectDrawSurface7* DtoPrim = NULL;		// Primary surface
static IDirectDrawSurface7* DtoBack = NULL;		// Back buffer surface

static int nRotateGame;
static bool bRotateScanlines;
static DDBLTFX* DtoBltFx = NULL;				// We use mirrored blits for flipped games

static IDirectDrawSurface7* pddsDtos = NULL;	// The screen surface
static int nGameWidth = 0, nGameHeight = 0;		// screen size

static bool bDtosScan;

static RECT Src = { 0, 0, 0, 0 };
static RECT Dest = { 0, 0, 0, 0 };

static int nHalfMask = 0;

static int nUseSys;								// Use System or Video memory

static int DtoPrimClear()
{
	if (DtoPrim == NULL) {
		return 1;
	}

	VidSClearSurface(DtoPrim, 0, NULL);			// Clear 1st page

	if (DtoBack) {								// We're using a triple buffer
		VidSClearSurface(DtoBack, 0, NULL);		// Clear 2nd page
		DtoPrim->Flip(NULL, DDFLIP_WAIT);
		VidSClearSurface(DtoBack, 0, NULL);		// Clear 3rd page
	}

	return 0;
}

static int DtoPrimInit(int bTriple)
{
	DDSURFACEDESC2 ddsd;

	// Create the primary surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	if (bTriple) {															// Make a primary surface capable of triple bufferring
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 2;
	} else {
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	if (FAILED(DtoDD->CreateSurface(&ddsd, &DtoPrim, NULL))) {
		return 1;
	}

	if (bTriple) {
		// Get the back buffer
		memset(&ddsd.ddsCaps, 0, sizeof(ddsd.ddsCaps));
		ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

		if (FAILED(DtoPrim->GetAttachedSurface(&ddsd.ddsCaps, &DtoBack))) {	// Failed to make triple buffer
			RELEASE(DtoPrim)
			return 1;
		}

		DtoPrimClear();														// Clear surfaces
	}

	return 0;
}

// Try to autodetect the best secondary buffer type to use, based on the hardware capabilities
static int AutodetectUseSys()
{
	DDCAPS ddc;

	memset(&ddc, 0, sizeof(ddc));
	ddc.dwSize = sizeof(ddc);
	DtoDD->GetCaps(&ddc, NULL);

	if (ddc.dwCaps & DDCAPS_BLTSTRETCH) {					// If it can do a hardware stretch use video memory
		return 0;
	} else {												// Otherwise use system memory:
		return 1;
	}
}

// Create a secondary DirectDraw surface for the game image
static int DtosMakeSurf()
{
	int nRet;

	nUseSys = 0;
	DDSURFACEDESC2 ddsd;

	if (DtoDD == NULL) {
		return 1;
	}

	nUseSys = nVidTransferMethod;
	if (nUseSys < 0) {
		nUseSys = AutodetectUseSys();
	}

	// Try to allocate buffer in Video memory first, if that fails use System memory
	do {
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (nUseSys == 0) {
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		} else {
			if (nUseSys == 1) {
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			}
		}

		ddsd.dwWidth = nGameWidth << 1;						// Make the surface large enough to add scanlines
		ddsd.dwHeight = nGameHeight << 1;					//

		nRet = DtoDD->CreateSurface(&ddsd, &pddsDtos, NULL);

		if (SUCCEEDED(nRet)) {								// Break early, so nUseSys will keep its value
			break;
		}

	} while (++nUseSys <= 1);

	if (FAILED(nRet)) {
		return 1;
	}

	nVidScrnDepth = VidSGetSurfaceDepth(pddsDtos);			// Get colourdepth of primary surface

	VidSClearSurface(pddsDtos, 0, NULL);

	return 0;
}

static int DtosExit()
{
	RELEASE(pddsDtos);

	VidSFreeVidImage();

	VidSExitOSD();

	return 0;
}

static int DtosInit()
{
	if (DtoDD == NULL) {
		return 1;
	}

	if (nRotateGame & 1) {
		nVidImageWidth = nGameHeight;
		nVidImageHeight = nGameWidth;
	} else {
		nVidImageWidth = nGameWidth;
		nVidImageHeight = nGameHeight;
	}

	if (bVidScanRotate && nGameWidth < nGameHeight) {
		bRotateScanlines = true;
	} else {
		bRotateScanlines = false;
	}

	nVidImageDepth = VidSGetSurfaceDepth(DtoPrim);	// Get color depth of primary surface
	nVidImageBPP = (nVidImageDepth + 7) >> 3;

	// Make the mask to mask out all but the lowest intensity bit
	if (nVidImageDepth == 15) {
		nHalfMask = 0xFBDEFBDE;
	} else {
		if (nVidImageDepth == 16) {
			nHalfMask = 0xF7DEF7DE;
		} else {
			nHalfMask = 0xFEFEFEFE;
		}
	}

	// Make the normal memory buffer
	if (VidSAllocVidImage()) {
		DtosExit();
		return 1;
	}

	// Make the DirectDraw secondary surface
	if (DtosMakeSurf()) {
		DtosExit();
		return 1;
	}

	// Use our callback to get colors:
	SetBurnHighCol(nVidImageDepth);

	Dest.left = 0; Dest.right = -1;

	RECT rect = { 0, 0, 0, 0 };
	GetClientScreenRect(hVidWnd, &rect);
	rect.top += 0 /*nMenuHeight*/;

	VidSScaleImage(&rect, nGameWidth, nGameHeight, bVidScanRotate);

	VidSInitOSD(4);

	return 0;
}

static int vidExit()
{
	VidSRestoreGamma();

	VidSRestoreScreenMode();

	DtosExit();

	RELEASE(DtoPrim)					// a single call releases all surfaces
	DtoBack = NULL;

	VidSExit();

	if (DtoBltFx) {
		free(DtoBltFx);
		DtoBltFx = NULL;
	}

	RELEASE(DtoDD)

	return 0;
}

static GUID MyGuid;
static int nWantDriver;

#ifdef PRINT_DEBUG_INFO
static int nCurrentDriver;
#ifdef UNICODE
static BOOL PASCAL MyEnumDisplayDrivers(GUID FAR* pGuid, LPWSTR pszDesc, LPWSTR /*pszName*/, LPVOID /*pContext*/, HMONITOR /*hMonitor*/)
#else
static BOOL PASCAL MyEnumDisplayDrivers(GUID FAR* pGuid, LPSTR pszDesc, LPSTR /*pszName*/, LPVOID /*pContext*/, HMONITOR /*hMonitor*/)
#endif
{
	if (nCurrentDriver == nWantDriver) {
		memcpy(&MyGuid, pGuid, sizeof(GUID));
	}

	if (nCurrentDriver == 0) {
		dprintf(_T("    %s\n"), pszDesc);
	} else {
		dprintf(_T("    Display %d (on %s)\n"), nCurrentDriver, pszDesc);
	}

	nCurrentDriver++;

	return DDENUMRET_OK;
}
#endif

static int vidInit()
{
	hVidWnd = nVidFullscreen ? hScrnWnd : hVideoWindow;						// Use Screen window for video

#ifdef PRINT_DEBUG_INFO
	dprintf(_T("  * Enumerating available drivers:\n"));
	nWantDriver = 0;
	nCurrentDriver = 0;
	_DirectDrawEnumerateEx(MyEnumDisplayDrivers, NULL, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
#endif

	// Get pointer to DirectDraw device
	_DirectDrawCreateEx(nWantDriver ? &MyGuid : NULL, (void**)&DtoDD, IID_IDirectDraw7, NULL);

	VidSInit(DtoDD);

	nGameWidth = nVidImageWidth; nGameHeight = nVidImageHeight;

	nRotateGame = 0;
	if (bDrvOkay) {
		DtoBltFx = NULL;

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

		if (nRotateGame & 2) {
			DDCAPS ddcaps;

			// Disable flipping until we've checked the hardware supports it
			nRotateGame &= ~2;

			memset(&ddcaps, 0, sizeof(ddcaps));
			ddcaps.dwSize = sizeof(ddcaps);

			DtoDD->GetCaps(&ddcaps, NULL);
			if (((ddcaps.dwFXCaps & DDFXCAPS_BLTMIRRORLEFTRIGHT) && (ddcaps.dwFXCaps & DDFXCAPS_BLTMIRRORUPDOWN)) || bVidForceFlip) {

				DtoBltFx = (DDBLTFX*)malloc(sizeof(DDBLTFX));
				if (DtoBltFx == NULL) {
					vidExit();
					return 1;
				}

				memset(DtoBltFx, 0, sizeof(DDBLTFX));

				DtoBltFx->dwSize = sizeof(DDBLTFX);
				DtoBltFx->dwDDFX = DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN;

				// Enable flipping now
				nRotateGame |= 2;
			}
		}
	}

	DtoPrim = NULL;							// No primary surface yet
	DtoBack = NULL;

	// Remember the changes to the display
	if (nVidFullscreen) {
		if (VidSEnterFullscreenMode(nScreenSize, 0)) {
			vidExit();
			return 1;
		}
	} else {
		DtoDD->SetCooperativeLevel(hVidWnd, DDSCL_NORMAL);
	}

#ifdef PRINT_DEBUG_INFO
	{
		// Display amount of free video memory
		DDSCAPS2 ddsCaps2;
		DWORD dwTotal;
		DWORD dwFree;

		memset(&ddsCaps2, 0, sizeof(ddsCaps2));
		ddsCaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;

		dprintf(_T(" ** Starting DirectDraw7 blitter.\n"));

		if (SUCCEEDED(DtoDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree))) {
			dprintf(_T("  * Initialising video: Total video memory minus display surface: %.2fMB.\n"), (double)dwTotal / (1024 * 1024));
		}

		if (bDrvOkay) {
			if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
				if (nRotateGame & 2) {
					dprintf(_T("  * Using graphics hardware to rotate the image 180 degrees.\n"));
				} else {
					dprintf(_T("  * Warning: Graphics hardware does not support mirroring blits.\n    Image orientation will be incorrect.\n"));
				}
			}
		}
	}
#endif

	if (bVidTripleBuffer && nVidFullscreen) {
		if (DtoPrimInit(1)) {			// Try to make triple buffer

#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Warning: Couldn't allocate a triple-buffering surface.\n"));
#endif

			// If we fail, fail entirely and make a normal buffer
			RELEASE(DtoPrim)
		}
	}

	if (DtoPrim == NULL) {
		// No primary surface yet, so try normal
		if (DtoPrimInit(0)) {
#ifdef PRINT_DEBUG_INFO
	   	dprintf(_T("  * Error: Couldn't create primary surface.\n"));
#endif

			vidExit();
			return 1;
		}
	}

	if (nVidFullscreen) {
		DtoDD->Compact();
	}

	VidSClipperInit(DtoPrim);

	VidSSetupGamma(DtoPrim);

	// Init the buffer surfaces
	if (DtosInit()) {
		vidExit();
		return 1;
	}

#ifdef PRINT_DEBUG_INFO
	{
		DDSCAPS2 ddsCaps2;
		DWORD dwTotal;
		DWORD dwFree;

		memset(&ddsCaps2, 0, sizeof(ddsCaps2));
		ddsCaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if (SUCCEEDED(DtoDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree))) {
			dprintf(_T("  * Initialisation complete: %.2fMB video memory free.\n"), (double)dwFree / (1024 * 1024));
			dprintf(_T("    Displaying and rendering in %i-bit mode, emulation running in %i-bit mode.\n"), nVidScrnDepth, nVidImageDepth);
			if (nUseSys) {
				dprintf(_T("    Blitting directly from system memory.\n"));
			} else {
				dprintf(_T("    Transferring the image to video memory before blitting.\n"));
			}
			if (nVidFullscreen) {
				dprintf(_T("    Running in fullscreen mode (%i x %i)"), nVidScrnWidth, nVidScrnHeight);
				if (DtoBack != NULL) {
					dprintf(_T(", using a triple buffer"));
				}
				dprintf(_T(".\n"));
			} else {
				dprintf(_T("    Running in windowed mode.\n"));
			}
		}
	}
#endif

	return 0;
}

// Copy pVidImage to pddsDtos, rotate
static int vidRenderRotate(DDSURFACEDESC2* ddsd)
{
	unsigned char *pd, *ps, *pdd;
	unsigned char *Surf;
	int nPitch;

	Surf = (unsigned char*)ddsd->lpSurface;
	nPitch = ddsd->lPitch;

	pd = Surf;

	if (bDtosScan) {
		if (bVidScanHalf) {
			if (bVidScanRotate) {
				switch (nVidImageBPP) {
					case 4: {
						int t;
						for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
							ps = pVidImage + (nGameHeight - 1 - y) * 4;
							pdd = pd;
							for (int x = 0; x < nGameWidth; x++) {
								t = *(int*)ps;
								ps += nVidImagePitch;
								*(int*)pdd = t;
								pdd += 4;
								*(int*)pdd = (t & 0xFEFEFE) >> 1;
								pdd += 4;
							}
						}
						break;
					}
					case 3: {
						for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
							ps = pVidImage + (nGameHeight - 1 - y) * 3;
							pdd = pd;
							for (int x = 0; x < nGameWidth; x++) {
								pdd[0] = ps[0];
								pdd[1] = ps[1];
								pdd[2] = ps[2];
								pdd[3] = ps[0] >> 1;
								pdd[4] = ps[1] >> 1;
								pdd[5] = ps[2] >> 1;
								ps += nVidImagePitch;
								pdd +=6;
							}
						}
						break;
					}
					case 2: {
						short t;
						for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
							ps = pVidImage + (nGameHeight - 1 - y) * 2;
							pdd = pd;
							for (int x = 0; x < nGameWidth; x++) {
								t = *(short*)ps;
								ps += nVidImagePitch;
								*(short*)pdd = t;
								pdd += 2;
								*(short*)pdd = (t & nHalfMask) >> 1;
								pdd += 2;
							}
						}
						break;
					}
				}

			} else {
				unsigned char* pBuffer = (unsigned char*)malloc(nPitch);
				unsigned char* pd2;

				switch (nVidImageBPP) {
					case 4: {
						int t;
						for (int y = 0; y < nGameHeight; y++, pd += nPitch * 2) {
							ps = pVidImage + (nGameHeight - 1 - y) * 4;
							pdd = pd;
							pd2 = pBuffer;
							for (int x = 0; x < nGameWidth; x++) {
								t = *(int*)ps;
								ps += nVidImagePitch;
								*(int*)pdd = t;
								*(int*)pd2 = t;
								pdd += 4;
								pd2 += 4;
							}
							pdd = pd + nPitch;
							for (ps = pBuffer; ps < (pBuffer + nGameWidth * 4); ps += 4) {
								*(int*)pdd = (*(int*)ps & 0xFEFEFE) >> 1;
								pdd += 4;
							}
						}
						break;
					}
					case 3: {
						for (int y = 0; y < nGameHeight; y++, pd += nPitch * 2) {
							ps = pVidImage + (nGameHeight - 1 - y) * 3;
							pdd = pd;
							pd2 = pBuffer;
							for (int x = 0; x < nGameWidth; x++) {
								pdd[0] = ps[0];
								pd2[0] = ps[0];
								pdd[1] = ps[1];
								pd2[1] = ps[1];
								pdd[2] = ps[2];
								pd2[2] = ps[2];
								ps += nVidImagePitch;
								pdd += 3;
								pd2 += 3;
							}
							pdd = pd + nPitch;
							for (ps = pBuffer; ps < (pBuffer + nGameWidth * 3); ps += 3) {
								pdd[0] = ps[0] >> 1;
								pdd[1] = ps[1] >> 1;
								pdd[2] = ps[2] >> 1;
								pdd += 3;
							}
						}
						break;
					}
					case 2: {
						short t;
						for (int y = 0; y < nGameHeight; y++, pd += nPitch * 2) {
							ps = pVidImage + (nGameHeight - 1 - y) * 2;
							pdd = pd;
							pd2 = pBuffer;
							for (int x = 0; x < nGameWidth; x++) {
								t = *(short*)ps;
								ps += nVidImagePitch;
								*(short*)pdd = t;
								*(short*)pd2 = t;
								pdd += 2;
								pd2 += 2;
							}
							pdd = pd + nPitch;
							for (ps = pBuffer; ps < (pBuffer + nGameWidth * 2); ps += 4) {
								*(unsigned int*)pdd = (*(unsigned int*)ps & nHalfMask) >> 1;
								pdd += 4;
							}
						}
						break;
					}
				}
				if (pBuffer) {
					free (pBuffer);
					pBuffer = NULL;
				}
			}
		} else {
			int nPixelSize = nVidImageBPP;
			if (bVidScanRotate) {
				nPixelSize <<= 1;
			} else {
				nPitch <<= 1;
			}

			switch (nVidImageBPP) {
				case 4: {
					for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
						ps = pVidImage + (nGameHeight - 1 - y) * 4;
						pdd = pd;
						for (int x = 0; x < nGameWidth; x++) {
							*(int*)pdd = *(int*)ps;
							ps += nVidImagePitch;
							pdd += nPixelSize;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
						ps = pVidImage + (nGameHeight - 1 - y) * 3;
						pdd = pd;
						for (int x = 0; x < nGameWidth; x++) {
							pdd[0] = ps[0];
							pdd[1] = ps[1];
							pdd[2] = ps[2];
							ps += nVidImagePitch;
							pdd += nPixelSize;
						}
					}
					break;
				}
				case 2: {
					for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
						ps = pVidImage + (nGameHeight - 1 - y) * 2;
						pdd = pd;
						for (int x = 0; x < nGameWidth; x++) {
							*(short*)pdd = *(short*)ps;
							ps += nVidImagePitch;
							pdd += nPixelSize;
						}
					}
					break;
				}
			}
		}
	} else {
		switch (nVidImageBPP) {
			case 4: {
				for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
					ps = pVidImage + (nGameHeight - 1 - y) * 4;
					pdd = pd;
					for (int x = 0; x < nGameWidth; x++) {
						*(int*)pdd = *(int*)ps;
						ps += nVidImagePitch;
						pdd += 4;
					}
				}
				break;
			}
			case 3: {
				for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
					ps = pVidImage + (nGameHeight - 1 - y) * 3;
					pdd = pd;
					for (int x = 0; x < nGameWidth; x++) {
						pdd[0] = ps[0];
						pdd[1] = ps[1];
						pdd[2] = ps[2];
						ps += nVidImagePitch;
						pdd +=3;
					}
				}
				break;
			}
			case 2:	{
				for (int y = 0; y < nGameHeight; y++, pd += nPitch) {
					ps = pVidImage + (nGameHeight - 1 - y) * 2;
					pdd = pd;
					for (int x = 0; x < nGameWidth; x++) {
						*(short*)pdd = *(short*)ps;
						ps += nVidImagePitch;
						pdd += 2;
					}
				}
				break;
			}
		}
	}

	return 0;
}

// Copy pVidImage to pddsDtos, don't rotate, add scanlines in odd lines if needed
static int vidRenderNoRotateHorScanlines(DDSURFACEDESC2* ddsd, int nField, int nHalf)
{
	unsigned char *pd, *ps;

	unsigned char* Surf = (unsigned char*)ddsd->lpSurface;
	int nPitch = ddsd->lPitch;
	if (bDtosScan) {
		if (nField) {			// copy to odd fields
			Surf += nPitch;
		}
		nPitch <<= 1;
	}

	pd = Surf; ps = pVidImage;
	for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
		if (nHalf == 0) {
			memcpy(pd, ps, nVidImagePitch);
		} else {
			unsigned char* psEnd = ps + nVidImagePitch;
			unsigned char* pdp = pd;
			unsigned char* psp = ps;

			do {
				unsigned int t;
				t = *((unsigned int *)(psp));
				t = (t & nHalfMask) >> 1;
				*((unsigned int *)(pdp)) = t;

				psp += 4;
				pdp += 4;
			} while (psp < psEnd);
		}
	}

	return 0;
}

// Copy pVidImage to pddsDtos, don't rotate, add rotated scanlines
static int vidRenderNoRotateVertScanlines(DDSURFACEDESC2* ddsd)
{
	unsigned char *pd, *ps;
	unsigned char* Surf = (unsigned char*)ddsd->lpSurface;
	int nPitch = ddsd->lPitch;

	if (bVidScanHalf) {
		pd = Surf; ps = pVidImage;
		switch (nVidImageBPP) {
			case 4: {
				unsigned char* pdp;
				unsigned char* psp;
				unsigned char* psEnd;
				for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
					pdp = pd;
					psp = ps;
					psEnd = ps + nVidImagePitch;
					do {
						unsigned int t = *(unsigned int*)psp;
						*(unsigned int*)pdp = t;
						psp += 4;
						pdp += 4;
						*(unsigned int*)pdp = (t & 0xFEFEFE) >> 1;
						pdp += 4;
					} while (psp < psEnd);
				}
				break;
			}
			case 3: {
				unsigned char* pdp;
				unsigned char* psp;
				unsigned char* psEnd;
				for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
					pdp = pd;
					psp = ps;
					psEnd = ps + nVidImagePitch;
					do {
						pdp[0] = psp[0];
						pdp[1] = psp[1];
						pdp[2] = psp[2];
						pdp[3] = psp[0] >> 1;
						pdp[4] = psp[1] >> 1;
						pdp[5] = psp[2] >> 1;
						psp += 3;
						pdp += 6;
					} while (psp < psEnd);
				}
				break;
			}
			case 2: {
				unsigned char* pdp;
				unsigned char* psp;
				unsigned char* psEnd;
				unsigned short t;
				for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
					pdp = pd;
					psp = ps;
					psEnd = ps + nVidImagePitch;
					do {
						t = *(unsigned short*)psp;
						*(unsigned short*)pdp = t;
						psp += 2;
						pdp += 2;
						*(unsigned short*)pdp = (t & nHalfMask) >> 1;
						pdp += 2;
					} while (psp < psEnd);
				}
				break;
			}
		}

	} else {
		pd = Surf; ps = pVidImage;
		switch (nVidImageBPP) {
			case 4: {
				unsigned char* pdp;
				unsigned char* psp;
				unsigned char* psEnd;
				for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
					pdp = pd;
					psp = ps;
					psEnd = ps + nVidImagePitch;
					do {
						*(unsigned int*)pdp = *(unsigned int*)psp;

						psp += 4;
						pdp += 8;
					} while (psp < psEnd);
				}
				break;
			}
			case 3: {
				unsigned char* pdp;
				unsigned char* psp;
				unsigned char* psEnd;
				for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
					pdp = pd;
					psp = ps;
					psEnd = ps + nVidImagePitch;
					do {
						pdp[0] = psp[0];
						pdp[1] = psp[1];
						pdp[2] = psp[2];

						psp += 3;
						pdp += 6;
					} while (psp < psEnd);
				}
				break;
			}
			case 2: {
				unsigned char* pdp;
				unsigned char* psp;
				unsigned char* psEnd;
				for (int y = 0; y < nVidImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
					pdp = pd;
					psp = ps;
					psEnd = ps + nVidImagePitch;
					do {
						*(unsigned short*)pdp = *(unsigned short*)psp;

						psp += 2;
						pdp += 4;
					} while (psp < psEnd);
				}
				break;
			}
		}
	}

	return 0;
}

static int vidBurnToSurf()
{
	DDSURFACEDESC2 ddsd;

	if (pddsDtos == NULL) {
		return 1;
	}

	if (DtoPrim->IsLost()) {														// We've lost control of the screen
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
		VidSScaleImage(&Dest, nGameWidth, nGameHeight, bVidScanRotate);
	}

	Src.right = nGameWidth;
	Src.bottom = nGameHeight;
	bool bScan = false;

	if (bVidScanlines) {															// See if the display window is large enough to add scanlines if needed
		if (bRotateScanlines) {
			if (Dest.right - Dest.left >= (nGameWidth << 1)) {						// We need to add vertical scanlines
				Src.right <<= 1;
				bScan = true;
			}
		} else {
			if (Dest.bottom - Dest.top >= (nGameHeight << 1)) {						// We need to add horizontal scanlines
				Src.bottom <<= 1;
				bScan = true;
			}
		}
	}

	// Lock the surface so we can write to it
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	if (FAILED(pddsDtos->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))) {
		return 1;
	}

	if (bScan && !bDtosScan) {														// Scanlines were just enabled. We need to clear the screen
		for (int y = 0; y < (nVidImageHeight << 1); y++) {
			memset(((unsigned char*)ddsd.lpSurface) + y * ddsd.lPitch, 0, ddsd.lPitch);
		}
	}

	bDtosScan = bScan;

	if (nRotateGame & 1) {
		vidRenderRotate(&ddsd);
	} else {
		if (bDtosScan && bRotateScanlines) {
			vidRenderNoRotateVertScanlines(&ddsd);
		} else {
			vidRenderNoRotateHorScanlines(&ddsd, 0, 0);
			if (bDtosScan && bVidScanHalf) {
				vidRenderNoRotateHorScanlines(&ddsd, 1, 1);
			}
		}
	}

	pddsDtos->Unlock(NULL);

	return 0;
}

// Run one frame and render the screen
int vidFrame(bool bRedraw)			// bRedraw = 0
{
	if (pVidImage == NULL) {
		return 1;
	}

	if (DtoPrim->IsLost()) {		// We've lost control of the screen
		if (VidSRestoreOSD()) {
			return 1;
		}

		if (FAILED(DtoDD->RestoreAllSurfaces())) {
			return 1;
		}

		DtoPrimClear();
	}

	if (bDrvOkay) {
		if (bRedraw) {				// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();		// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();			// Run one frame and draw the screen
		}
	}

	vidBurnToSurf();

	return 0;
}

// Paint the Dtos surface onto the primary surface
static int vidPaint(int bValidate)
{
	if (DtoPrim == NULL || pddsDtos == NULL) {
		return 1;
	}

	if (DtoPrim->IsLost()) {																	// We've lost control of the screen
		return 1;
	}

	if (!nVidFullscreen) {																		// Check if the window has changed since we prepared the image
		RECT rect = { 0, 0, 0, 0 };

		GetClientScreenRect(hVidWnd, &rect);
		rect.top += 0 /*nMenuHeight*/;

		VidSScaleImage(&rect, nGameWidth, nGameHeight, bVidScanRotate);

		if (Dest.left != rect.left || Dest.right != rect.right || Dest.top != rect.top || Dest.bottom != rect.bottom) {
			bValidate |= 2;
		}
	}

	if (bValidate & 2) {
		vidBurnToSurf();
	}

	DWORD dwBltFlags = 0;																		// See if we need to use blit effects
	if (DtoBltFx) {
		dwBltFlags |= DDBLT_DDFX;
	}

	if (bVidVSync && !nVidFullscreen) { DtoDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL); }

	if (DtoBack != NULL) {																		// Triple bufferring
		if (FAILED(DtoBack->Blt(&Dest, pddsDtos, &Src, DDBLT_ASYNC | dwBltFlags, DtoBltFx))) {
			if (FAILED(DtoBack->Blt(&Dest, pddsDtos, &Src, DDBLT_WAIT | dwBltFlags, DtoBltFx))) {
				return 1;
			}
		}
		VidSDisplayOSD(DtoBack, &Dest, 0);

		DtoPrim->Flip(NULL, DDFLIP_WAIT);
	} else {																					// Normal
		RECT rect = { 0, 0, nGameWidth, nGameHeight };
		int nFlags = 0;

		if (bDtosScan) {
			if (bRotateScanlines) {
				nFlags |= 0x01;
			} else {
				nFlags |= 0x02;
			}
		}

		// Display OSD text message
		VidSDisplayOSD(pddsDtos, &rect, nFlags);

		if (FAILED(DtoPrim->Blt(&Dest, pddsDtos, &Src, DDBLT_ASYNC | dwBltFlags, DtoBltFx))) {
			if (FAILED(DtoPrim->Blt(&Dest, pddsDtos, &Src, DDBLT_WAIT | dwBltFlags, DtoBltFx))) {
				return 1;
			}
		}
/*
		DWORD lpdwScanLine;
		RECT window;
		GetWindowRect(hVidWnd, &window);

		while (1) 
		{
			DtoDD->GetScanLine(&lpdwScanLine);
			if (lpdwScanLine >= (unsigned int)window.bottom) {
				break;
			}
			//Sleep(1);
		}
*/
	}

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

static int vidScale(RECT* pRect, int nWidth, int nHeight)
{
	return VidSScaleImage(pRect, nWidth, nHeight, bVidScanRotate);
}

static int vidGetSettings(InterfaceInfo* pInfo)
{
	if (nVidFullscreen && DtoBack) {
		IntInfoAddStringModule(pInfo, _T("Using a triple buffer"));
	} else {
		IntInfoAddStringModule(pInfo, _T("Using Blt() to transfer the image"));
	}

	if (nUseSys) {
		IntInfoAddStringModule(pInfo, _T("Using system memory"));
	} else {
		IntInfoAddStringModule(pInfo, _T("Using video memory for the final blit"));
	}

	if (nRotateGame) {
		TCHAR* pszEffect[8] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
		TCHAR szString[MAX_PATH] = _T("");

		pszEffect[0] = _T("Using ");
		if (nRotateGame & 1) {
			pszEffect[1] = _T("software rotation");
		}
		if (nRotateGame & 2) {
			if (nRotateGame & 1) {
				pszEffect[2] = _T(" and");
			}
			pszEffect[3] = _T(" hardware mirroring");
		}
		pszEffect[4] = _T(", ");

		_sntprintf(szString, MAX_PATH, _T("%s%s%s%s%s%s%s%s"), pszEffect[0], pszEffect[1], pszEffect[2], pszEffect[3], pszEffect[4], pszEffect[5], pszEffect[6], pszEffect[7]);
		IntInfoAddStringModule(pInfo, szString);
	}

	if (bDtosScan) {
		TCHAR* pszEffect[8] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
		TCHAR szString[MAX_PATH] = _T("");

		pszEffect[0] = _T("Applying ");
		if (bVidScanHalf) {
			pszEffect[1] = _T("50% ");
		}
		if (bVidScanRotate) {
			pszEffect[2] = _T("rotated ");
		}
		pszEffect[3] = _T("scanlines");

		_sntprintf(szString, MAX_PATH, _T("%s%s%s%s%s%s%s%s"), pszEffect[0], pszEffect[1], pszEffect[2], pszEffect[3], pszEffect[4], pszEffect[5], pszEffect[6], pszEffect[7]);
		IntInfoAddStringModule(pInfo, szString);
	}

	return 0;
}

// The video output plugin:
struct VidOut VidOutDDraw = { vidInit, vidExit, vidFrame, vidPaint, vidScale, vidGetSettings, _T("DirectDraw7 video output") };
