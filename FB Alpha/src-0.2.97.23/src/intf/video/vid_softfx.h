// Sotfware effects

#ifdef BUILD_WIN32
 #define SOFTFX_ENABLE_DIRECTX
#endif
#ifdef BUILD_SDL
 #define SOFTFX_ENABLE_SDL
#endif

#ifdef SOFTFX_ENABLE_DIRECTX
 #include "vid_directx_support.h"
#endif
#ifdef SOFTFX_ENABLE_SDL
 #include <SDL.h>
#endif

#define FILTER_PLAIN				0
#define FILTER_ADVMAME_SCALE_2X			1
#define FILTER_ADVMAME_SCALE_3X			2
#define FILTER_2XPM_LQ				3
#define FILTER_2XPM_HQ				4
#define FILTER_EAGLE				5
#define FILTER_SUPEREAGLE			6
#define FILTER_2XSAI				7
#define FILTER_SUPER_2XSAI			8
#define FILTER_SUPEREAGLE_VBA			9
#define FILTER_2XSAI_VBA			10
#define FILTER_SUPER_2XSAI_VBA			11
#define FILTER_SUPERSCALE			12
#define FILTER_SUPERSCALE_75SCAN		13
#define FILTER_HQ2X				14
#define FILTER_HQ3X				15
#define FILTER_HQ4X				16
#define FILTER_HQ2XS_VBA			17
#define FILTER_HQ3XS_VBA			18
#define FILTER_HQ2XS_SNES9X			19
#define FILTER_HQ3XS_SNES9X			20
#define FILTER_HQ2XBOLD				21
#define FILTER_HQ3XBOLD				22
#define FILTER_EPXB				23
#define FILTER_EPXC				24
#define FILTER_2XBR_A			25
#define FILTER_2XBR_B			26
#define FILTER_2XBR_C			27
#define FILTER_3XBR_A			28
#define FILTER_3XBR_B			29
#define FILTER_3XBR_C			30
#define FILTER_4XBR_A			31
#define FILTER_4XBR_B			32
#define FILTER_4XBR_C			33
#define FILTER_DDT3X            34

TCHAR* VidSoftFXGetEffect(int nEffect);
int VidSoftFXGetZoom(int nEffect);
int VidSoftFXCheckDepth(int nEffect, int nDepth);

void VidSoftFXExit();
int VidSoftFXInit(int nBlitter, int nRotate);

int VidSoftFXScale(RECT* pRect, int nGameWidth, int nGameHeight);

#ifdef SOFTFX_ENABLE_DIRECTX
int VidSoftFXApplyEffectDirectX(IDirectDrawSurface7* pSurf, RECT* pRect);
#endif

#ifdef SOFTFX_ENABLE_SDL
int VidSoftFXApplyEffectSDL(SDL_Surface* pSurf);
#endif

int VidFilterApplyEffect(unsigned char* pd, int pitch);
