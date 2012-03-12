//#define DIRECTDRAW_VERSION 7				// Use this DirectDraw version
#include <ddraw.h>
#include "vid_support.h"

// General
void VidSExit();
int VidSInit(IDirectDraw7* pDD7);

// Miscellaneous
int VidSGetSurfaceDepth(IDirectDrawSurface7* pSurf);
int VidSClearSurface(IDirectDrawSurface7* pSurf, unsigned int nColour, RECT* pRect);

// Clipper
int VidSClipperInit(IDirectDrawSurface7* pSurf);

// Gamma controls
void VidSRestoreGamma();
int VidSUpdateGamma();
int VidSSetupGamma(IDirectDrawSurface7* pSurf);

// Select optimal full-screen resolution
struct VidSDisplayScoreInfo {
	// Filled in advance by the caller
	unsigned int nRequestedWidth; unsigned int nRequestedHeight; unsigned int nRequestedDepth; unsigned int nRequestedZoom;
	unsigned int nModeWidth; unsigned int nModeHeight; unsigned int nModeDepth;
	// Filled by the scoring routine
	unsigned int nBestWidth; unsigned int nBestHeight; unsigned int nBestDepth;
	unsigned int nBestScore;
};

int VidSScoreDisplayMode(VidSDisplayScoreInfo* pScoreInfo);
int VidSInitScoreInfo(VidSDisplayScoreInfo* pScoreInfo);

void VidSRestoreScreenMode();
int VidSEnterFullscreenMode(int nZoom, int nDepth);

// Image size
int VidSScaleImage(RECT* pRect, int nGameWidth, int nGameHeight, bool bVertScanlines);
bool VidSGetArcaderes(int* pWidth, int* pHeight);

// Text display
void VidSExitOSD();
int VidSInitOSD(int nFlags);
int VidSRestoreOSD();
void VidSDisplayOSD(IDirectDrawSurface7* pSurf, RECT* pRect, int nFlags);

