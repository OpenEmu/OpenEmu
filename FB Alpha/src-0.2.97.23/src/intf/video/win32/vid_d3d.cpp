// DirectD3D blitter

// #define USE_D3D_REFERENCE_DEVICE

#include "burner.h"

#if !defined BUILD_X64_EXE

// #include "vid_directx_support.h"
#include <InitGuid.h>
#include "vid_softfx.h"

// #define ENABLE_PROFILING FBA_DEBUG

#define DIRECT3D_VERSION 0x0700							// Use this Direct3D version
#define D3D_OVERLOADS
#include <d3d.h>

#include "rgb_pattern.h"

#include "ddraw_core.h"

const float PI = 3.14159265358979323846f;

typedef struct _D3DTLVERTEX2 {
	union { D3DVALUE sx; D3DVALUE dvSX; };
	union { D3DVALUE sy; D3DVALUE dvSY; };
	union { D3DVALUE sz; D3DVALUE dvSZ; };
	union { D3DVALUE rhw; D3DVALUE dvRHW; };
	union { D3DCOLOR color; D3DCOLOR dcColor; };
	union { D3DCOLOR specular; D3DCOLOR dcSpecular; };
	union { D3DVALUE tu; D3DVALUE dvTU; };
	union { D3DVALUE tv; D3DVALUE dvTV; };
	union { D3DVALUE tu1; D3DVALUE dvTU1; };
	union { D3DVALUE tv1; D3DVALUE dvTV1; };
#if(DIRECT3D_VERSION >= 0x0500)
 #if (defined __cplusplus) && (defined D3D_OVERLOADS)
	_D3DTLVERTEX2() { }
	_D3DTLVERTEX2(const D3DVECTOR& v, float _rhw, D3DCOLOR _color, D3DCOLOR _specular, float _tu, float _tv, float _tu1, float _tv1)
	{
		sx = v.x; sy = v.y; sz = v.z; rhw = _rhw;
		color = _color; specular = _specular;
		tu = _tu; tv = _tv;
		tu1 = _tu1; tv1 = _tv1;
	}
 #endif
#endif
} D3DTLVERTEX2, *LPD3DTLVERTEX2;

#define D3DFVF_TLVERTEX2 ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) )

typedef struct _D3DLVERTEX2 {
	union { D3DVALUE x; D3DVALUE dvX; };
	union { D3DVALUE y; D3DVALUE dvY; };
	union { D3DVALUE z; D3DVALUE dvZ; };
    DWORD dwReserved;
	union { D3DCOLOR color; D3DCOLOR dcColor; };
	union { D3DCOLOR specular; D3DCOLOR dcSpecular; };
	union { D3DVALUE tu; D3DVALUE dvTU; };
	union { D3DVALUE tv; D3DVALUE dvTV; };
	union { D3DVALUE tu1; D3DVALUE dvTU1; };
	union { D3DVALUE tv1; D3DVALUE dvTV1; };
#if(DIRECT3D_VERSION >= 0x0500)
 #if (defined __cplusplus) && (defined D3D_OVERLOADS)
	_D3DLVERTEX2() { }
	_D3DLVERTEX2(const D3DVECTOR& v, D3DCOLOR _color, D3DCOLOR _specular, float _tu, float _tv, float _tu1, float _tv1)
	{
		x = v.x; y = v.y; z = v.z; dwReserved = 0;
		color = _color; specular = _specular;
		tu = _tu; tv = _tv;
		tu1 = _tu1; tv1 = _tv1;
	}
 #endif
#endif
} D3DLVERTEX2, *LPD3DLVERTEX2;

#define D3DFVF_LVERTEX2 ( D3DFVF_XYZ | D3DFVF_RESERVED1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) )

typedef struct _D3DVERTEX2 {
	union { D3DVALUE x; D3DVALUE dvX; };
	union { D3DVALUE y; D3DVALUE dvY; };
	union { D3DVALUE z; D3DVALUE dvZ; };
    union { D3DVALUE nx; D3DVALUE dvNX; };
    union { D3DVALUE ny; D3DVALUE dvNY; };
    union { D3DVALUE nz; D3DVALUE dvNZ; };
	union { D3DVALUE tu; D3DVALUE dvTU; };
	union { D3DVALUE tv; D3DVALUE dvTV; };
	union { D3DVALUE tu1; D3DVALUE dvTU1; };
	union { D3DVALUE tv1; D3DVALUE dvTV1; };
#if(DIRECT3D_VERSION >= 0x0500)
 #if (defined __cplusplus) && (defined D3D_OVERLOADS)
    _D3DVERTEX2() { }
    _D3DVERTEX2(const D3DVECTOR& v, const D3DVECTOR& n, float _tu, float _tv, float _tu1, float _tv1)
        {
			x = v.x; y = v.y; z = v.z;
			nx = n.x; ny = n.y; nz = n.z;
			tu = _tu; tv = _tv;
			tu1 = _tu1; tv1 = _tv1;
        }
 #endif
#endif
} D3DVERTEX2, *LPD3DVERTEX2;

#define D3DFVF_VERTEX2 ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) )

static bool	bLostControl;

static bool bUsePageflip;			// Use a flipping surface when in fullscreen mode
static bool bUseTriplebuffer;

static bool bMultiTexturing;
static bool bRenderToTexture;

struct EffectPreset {
	int nUseFilter;					// -1: disable, 1: enable, use specified settings, 0: don't change
	int nUseEffects;				//
	int nUseScanlines;				//
	int nUseFeedback;				//
	int nUsePrescale;				//

	int nIntensity;
	int nFloor;
	int nSaturation;
	int nPatternMode;
	TCHAR* pPatternName;
	int nAttenuation;
	int nBlendMode;

	int nScanIntensity;

	int nAmount;
	int nOverSaturation;
};

static const EffectPreset PresetInfo[] = {
	{  0, -1, -1,  0, -1, 0,	  0,	0,	  0, NULL,				 				0,	  0, 0,			 0, 0 },	// 1x zoom
	{  1,  1,  1,  0,  0, 0x0080, 0x20, 0x00, 0, _T("4x4_rgb_pattern.i.rgb"),		0x00, 0, 0x008F8F8F, 0, 0 },	// 2x zoom
	{  1,  1,  1,  0,  0, 0x00A0, 0x30, 0x00, 2, _T("4x6_rgb_pattern.i.rgb"),		0x00, 0, 0x005F5F5F, 0, 0 },	// 3x zoom
	{  1,  1,  1,  0,  1, 0x0080, 0x20, 0x00, 3, _T("6x8_rgb_pattern.i.rgb"),		0x20, 0, 0x004F4F4F, 0, 0 },	// 4x zoom
	{  1,  1,  1,  0,  1, 0x0060, 0x10, 0x48, 3, _T("9x10_ellipsoid.i.rgb"),		0x20, 0, 0x005F5F5F, 0, 0 },	// 5x zoom
	{  0,  0,  0,  0,  0, 0,	  0,	0,	  0, NULL,					 			0,	  0, 0,			 0, 0 },	// 6x zoom
	{  0,  0,  0,  0,  0, 0,	  0,	0,	  0, NULL,					 			0,	  0, 0,			 0, 0 },	// 7x zoom
	{  0,  0,  0,  0,  0, 0,	  0,	0,	  0, NULL,					 			0,	  0, 0,			 0, 0 },	// 8x zoom

	// Selectable presets
	{  0,  1, -1,  0,  0, 0x0100, 0x80, 0x40, 2, _T("4x6_rgb_pattern.i.rgb"),		0,	  1, 0,			 0, 0 },	// Small Faint RGB mask
	{  0,  1, -1,  0,  0, 0x0100, 0x80, 0x40, 2, _T("10x6_large_dot.i.rgb"),		0,	  1, 0,			 0, 0 },	// Large RGB Dot mask
	{  0,  1, -1,  0,  0, 0x0100, 0x90, 0x80, 2, _T("12x10_large_ellipsoid.i.rgb"),	0,	  1, 0,			 0, 0 },	// Large Faint RGB mask
	{  0,  1, -1,  0,  0, 0x0100, 0xC0, 0x00, 0, _T("4x4_mame_rgbtiny.i.rgb"),	    0,	  1, 0,			 0, 0 },	// MAME rgbtiny	small
	{  0,  1, -1,  0,  0, 0x0100, 0xD0, 0x00, 0, _T("8x8_mame_rgbtiny.i.rgb"),	    0,	  1, 0,			 0, 0 },	// MAME rgbtiny	large
	{  0,  1,  1,  0,  0, 0x0030, 0x00, 0x00, 0, _T("3x1_aperture_grille.i.rgb"),	0x20, 0, 0x00BFBFBF, 0, 0 },	// Aperture Grille
	{  0,  1, -1,  0,  0, 0x0040, 0x00, 0x00, 0, _T("10x6_large_dot.i.rgb"),		0x08, 0, 0,			 0, 0 },	// Large Oversaturated RGB Dot
	{  0,  1, -1,  0,  0, 0x00E0, 0x00, 0x40, 3, _T("18x10_large_round.i.rgb"),		0x10, 0, 0,			 0, 0 },	// Huge Oversaturated pattern
};

static bool bCreateImage = false;

static bool bUseRGBEffects = false;								// Use RGB effects

static int nRGBScanlineIntensity = 0;							// Scanline intensity when RGB effects are used

static int nRGBEffectIntensity = 0x0100;						// Multiply pattern RGB values with this
static int nRGBEffectFloor = 0x00;								// Minimum pattern colour grey-value
static int nRGBEffectSaturation = 0x00;							// Minimum saturation of the pattern colours
static int nRGBEffectPatternmode = 0;							// How to use the Floor/Saturation values
static int nRGBEffectAttenuation = 0;							// How much to attenuate the image before applying RGB effects
static TCHAR* pRGBEffectPatternName = _T("4x6 RGB Pattern.rgb");	// Pattern to use
static int nRGBEffectBlendmode = 1;								// Set of D3D blendmodes

static int nRotateGame;
static bool bRotateEffects;										// Use rotated scanlines/RGB patterns for rotated games

static bool bUseLighting;
static bool bUse3DProjection;

static float fPreviousScreenAngle;
static float fPreviousScreenCurvature;

static int nPreScale = 0;
static int nPreScaleZoom = 0;
static int nPreScaleEffect = 0;

static IDirectDraw7* pDD = NULL;								// DirectDraw interface
static IDirect3D7* pD3D = NULL;									// Direct3D interface
static IDirect3DDevice7* pD3DDevice = NULL;

static D3DDEVICEDESC7 d3dDeviceDesc;							// holds capabilities of the 3D hardware

static IDirectDrawSurface7* pPrimarySurf = NULL;				// Primary surface
static IDirectDrawSurface7* pBackbuffer = NULL;					// Back buffer surface

static D3DVIEWPORT7 Viewport = {0, 0, 0, 0, 0.0f, 10.0f};

static RECT Dest = { 0, 0, 0, 0 };
static RECT Render;

static int nZoomFactor;

static int nGameWidth = 0, nGameHeight = 0;						// Game screen size
static int nGameImageWidth, nGameImageHeight;

static IDirectDrawSurface7* pEmuImage[4] = {NULL, };			// The emulator screen surfaces/textures

static int nTextureWidth, nTextureHeight;
static int nPreScaleTextureWidth, nPreScaleTextureHeight;
static IDirectDrawSurface7* pScanlineTexture[2] = {NULL, };		// The scanline textures

static IDirectDrawSurface7* pRGBEffectTexture = NULL;			// The RGB effects texture

static DWORD nScanlineOp;

static int n3DScreenGridSize;

static D3DTLVERTEX2 vScreen[4];									// Vectors for emulator image, screen coordinates
static D3DVERTEX2* v3DScreen = NULL;							// Vectors for emulator image, 3D coordinates
static D3DTLVERTEX vFeedbackImage[4];							// Vectors for feedback

static D3DMATRIX matWorld;
static D3DMATRIX matView;
static D3DMATRIX matProjection;
// static D3DMATRIX matIdentity = D3DMATRIX(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

static void SetWorldMatrix()
{
/*
	matView._12 = matView._13 = matView._14 = 0.0f;
	matView._21 = matView._23 = matView._24 = 0.0f;
	matView._31 = matView._32 = matView._34 = 0.0f;
	matView._41 = 0.0f;
	matView._11 = matView._22 = matView._33 = matView._44 = 1.0f;

	matView._42 = -1.0f;
	matView._43 = fNearPlane;

	pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matView);
*/

	matWorld._12 = matWorld._13 = matWorld._14 = matWorld._21 = matWorld._24 = 0.0f;
	matWorld._31 = matWorld._34 = matWorld._41 = matWorld._42 = matWorld._43 = 0.0f;
	matWorld._11 = matWorld._44 = 1.0f;

	matWorld._22 = (float)cos(fVidScreenAngle);
	matWorld._23 = (float)sin(fVidScreenAngle);
	matWorld._32 = -(float)sin(fVidScreenAngle);
	matWorld._33 = (float)cos(fVidScreenAngle);

	pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

//	pD3DDevice->MultiplyTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);
}

static void SetViewMatrix(float fNearPlane)
{
	matView._12 = matView._13 = matView._14 = 0.0f;
	matView._21 = matView._23 = matView._24 = 0.0f;
	matView._31 = matView._32 = matView._34 = 0.0f;
	matView._41 = 0.0f;
	matView._11 = matView._22 = matView._33 = matView._44 = 1.0f;

	matView._42 = -1.0f;
	matView._43 = fNearPlane;

	pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
}

static void SetProjectionMatrix(float fNearPlane)
{
	// Set projection matrix for perspective
	memset(&matProjection, 0, sizeof(D3DMATRIX));

	float fFarPlane = 1000.0f;
	float Q = fFarPlane / (fFarPlane - fNearPlane);

	matProjection._11 = fNearPlane;
	matProjection._22 = fNearPlane;
	matProjection._33 = Q;
	matProjection._34 = 1.0f;
	matProjection._43 = -Q * fNearPlane;

	pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProjection);
}

static int GetTextureSize(int nSize)
{
	int nTextureSize = 128;

	while (nTextureSize < nSize) {
		nTextureSize <<= 1;
	}

	return nTextureSize;
}

static int Update3DScreen()
{
	if (bUse3DProjection) {

		if (v3DScreen == NULL) {
			v3DScreen = (D3DVERTEX2*)malloc(6 * n3DScreenGridSize * n3DScreenGridSize * sizeof(D3DVERTEX2));
			if (v3DScreen == NULL) {
				return 1;
			}
		}

		// Use a 3D world and tilt the image backward

		if (fVidScreenCurvature == 0.0f && !bUseLighting) {
			v3DScreen[nRotateGame & 2 ? 3 : 0] = D3DVERTEX2(D3DVECTOR(-1.0f, 2.0f, 0.0f), D3DVECTOR(0, 0, 1), vScreen[0].tu, vScreen[0].tv, vScreen[0].tu1, vScreen[0].tv1);
			v3DScreen[nRotateGame & 2 ? 2 : 1] = D3DVERTEX2(D3DVECTOR( 1.0f, 2.0f, 0.0f), D3DVECTOR(0, 0, 1), vScreen[1].tu, vScreen[1].tv, vScreen[1].tu1, vScreen[1].tv1);
			v3DScreen[nRotateGame & 2 ? 1 : 2] = D3DVERTEX2(D3DVECTOR(-1.0f, 0.0f, 0.0f), D3DVECTOR(0, 0, 1), vScreen[2].tu, vScreen[2].tv, vScreen[2].tu1, vScreen[2].tv1);
			v3DScreen[nRotateGame & 2 ? 0 : 3] = D3DVERTEX2(D3DVECTOR( 1.0f, 0.0f, 0.0f), D3DVECTOR(0, 0, 1), vScreen[3].tu, vScreen[3].tv, vScreen[3].tu1, vScreen[3].tv1);
		} else {
			int nGameAspectX = 4, nGameAspectY = 3;

			double dXAngle, dYAngle;

			if (bDrvOkay) {
				if ((BurnDrvGetFlags() & (BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED)) && nVidRotationAdjust) {
					BurnDrvGetAspect(&nGameAspectY, &nGameAspectX);
				} else {
					BurnDrvGetAspect(&nGameAspectX, &nGameAspectY);
				}
			}

			if (nGameAspectX > nGameAspectY) {
				dXAngle = (double)fVidScreenCurvature;
				dYAngle = (double)fVidScreenCurvature * nGameAspectY / nGameAspectX;
			} else {
				dXAngle = (double)fVidScreenCurvature * nGameAspectX / nGameAspectY;
				dYAngle = (double)fVidScreenCurvature;
			}

			double dXFactor = 1.0 / sin(dXAngle / 2);
			double dYFactor = 1.0 / sin(dYAngle / 2);

			for (int y = 0; y < n3DScreenGridSize; y++) {

				double dTop = dYFactor * sin(-dYAngle / 2 + dYAngle * y / n3DScreenGridSize);
				double dBottom = dYFactor * sin(-dYAngle / 2 + dYAngle * (y + 1) / n3DScreenGridSize);

				double dTopFactor = cos(-dYAngle / 2 + dYAngle * y / n3DScreenGridSize);
				double dBottomFactor = cos(-dYAngle / 2 + dYAngle * (y + 1) / n3DScreenGridSize);

				for (int x = 0; x < n3DScreenGridSize; x++) {

					double dLeft = dXFactor * sin(-dXAngle / 2 + dXAngle * x / n3DScreenGridSize);
					double dRight = dXFactor * sin(-dXAngle / 2 + dXAngle * (x + 1) / n3DScreenGridSize);

					double dLeftFactor = cos(-dXAngle / 2 + dXAngle * x / n3DScreenGridSize);
					double dRightFactor = cos(-dXAngle / 2 + dXAngle * (x + 1) / n3DScreenGridSize);

					if (nRotateGame & 1) {
						v3DScreen[6 * y + ((n3DScreenGridSize - 1 - x) * 6 * n3DScreenGridSize) + 0] = D3DVERTEX2(D3DVECTOR((float)(dTopFactor    * dLeft),  (float)(1.0 - dLeftFactor  * dTop),    (float)(1.0 - dTopFactor    * dLeftFactor) ), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * y + ((n3DScreenGridSize - 1 - x) * 6 * n3DScreenGridSize) + 1] = D3DVERTEX2(D3DVECTOR((float)(dTopFactor    * dRight), (float)(1.0 - dRightFactor * dTop),    (float)(1.0 - dTopFactor    * dRightFactor)), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * y + ((n3DScreenGridSize - 1 - x) * 6 * n3DScreenGridSize) + 2] = D3DVERTEX2(D3DVECTOR((float)(dBottomFactor * dLeft),  (float)(1.0 - dLeftFactor  * dBottom), (float)(1.0 - dBottomFactor * dLeftFactor) ), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);

						v3DScreen[6 * y + ((n3DScreenGridSize - 1 - x) * 6 * n3DScreenGridSize) + 3] = D3DVERTEX2(D3DVECTOR((float)(dTopFactor    * dRight), (float)(1.0 - dRightFactor * dTop),    (float)(1.0 - dTopFactor    * dRightFactor)), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * y + ((n3DScreenGridSize - 1 - x) * 6 * n3DScreenGridSize) + 4] = D3DVERTEX2(D3DVECTOR((float)(dBottomFactor * dLeft),  (float)(1.0 - dLeftFactor  * dBottom), (float)(1.0 - dBottomFactor * dLeftFactor) ), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * y + ((n3DScreenGridSize - 1 - x) * 6 * n3DScreenGridSize) + 5] = D3DVERTEX2(D3DVECTOR((float)(dBottomFactor * dRight), (float)(1.0 - dRightFactor * dBottom), (float)(1.0 - dBottomFactor * dRightFactor)), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
					} else {
						v3DScreen[6 * x + (y * 6 * n3DScreenGridSize) + 0] = D3DVERTEX2(D3DVECTOR((float)(dTopFactor    * dLeft),  (float)(1.0 - dLeftFactor  * dTop),    (float)(1.0 - dTopFactor    * dLeftFactor) ), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * x + (y * 6 * n3DScreenGridSize) + 1] = D3DVERTEX2(D3DVECTOR((float)(dTopFactor    * dRight), (float)(1.0 - dRightFactor * dTop),    (float)(1.0 - dTopFactor    * dRightFactor)), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * x + (y * 6 * n3DScreenGridSize) + 2] = D3DVERTEX2(D3DVECTOR((float)(dBottomFactor * dLeft),  (float)(1.0 - dLeftFactor  * dBottom), (float)(1.0 - dBottomFactor * dLeftFactor) ), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);

						v3DScreen[6 * x + (y * 6 * n3DScreenGridSize) + 3] = D3DVERTEX2(D3DVECTOR((float)(dTopFactor    * dRight), (float)(1.0 - dRightFactor * dTop),    (float)(1.0 - dTopFactor    * dRightFactor)), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * x + (y * 6 * n3DScreenGridSize) + 4] = D3DVERTEX2(D3DVECTOR((float)(dBottomFactor * dLeft),  (float)(1.0 - dLeftFactor  * dBottom), (float)(1.0 - dBottomFactor * dLeftFactor) ), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
						v3DScreen[6 * x + (y * 6 * n3DScreenGridSize) + 5] = D3DVERTEX2(D3DVECTOR((float)(dBottomFactor * dRight), (float)(1.0 - dRightFactor * dBottom), (float)(1.0 - dBottomFactor * dRightFactor)), D3DVECTOR(0, 0, 0), 0, 0, 0, 0);
					}
				}
			}

			// Compute the vertex normals
            for (int x = 0; x < n3DScreenGridSize; x++) {
				for (int y = 0; y < n3DScreenGridSize; y++) {
					int i = 6 * x + (y * 6 * n3DScreenGridSize);

					for (int j = 0; j < 6; j++) {
						v3DScreen[i + j].nx = v3DScreen[i + j].x; v3DScreen[i + j].ny = v3DScreen[i + j].y - 1.0f; v3DScreen[i + j].nz = (1.0f - v3DScreen[i + j].z) / fVidScreenCurvature;
					}
				}
			}

			// Texture mapping coordinates for the emulator image
			for (int y = 0; y < n3DScreenGridSize; y++) {
				float tuLeft, tuRight;
				float tvTop, tvBottom;

				int ry = nRotateGame & 2 ? n3DScreenGridSize - 1 - y : y;

				tvTop = ((float)nGameImageHeight / nTextureHeight) * y / n3DScreenGridSize;
				tvBottom = ((float)nGameImageHeight / nTextureHeight) * (y + 1) / n3DScreenGridSize;

				for (int x = 0; x < n3DScreenGridSize; x++) {

					int rx = nRotateGame & 2 ? n3DScreenGridSize - 1 - x : x;

					tuLeft = (float(nGameImageWidth) / nTextureWidth) * x / n3DScreenGridSize;
					tuRight = (float(nGameImageWidth) / nTextureWidth) * (x + 1) / n3DScreenGridSize;

					if (nRotateGame & 1) {
						int i = 6 * (n3DScreenGridSize - 1 - rx) + ((n3DScreenGridSize - 1 - ry) * 6 * n3DScreenGridSize);
						v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tu = tuRight; v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tv = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tu = tuRight; v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tv = tvBottom;
						v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tu = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tv = tvTop;

						v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tu = tuRight; v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tv = tvBottom;
						v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tu = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tv = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tu = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tv = tvBottom;
					} else {
						int i = 6 * rx + (ry * 6 * n3DScreenGridSize);
						v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tu = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tv = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tu = tuRight; v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tv = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tu = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tv = tvBottom;

						v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tu = tuRight; v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tv = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tu = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tv = tvBottom;
						v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tu = tuRight; v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tv = tvBottom;
					}
				}
			}

			// Texture mapping coordinates for the scanlines
			for (int y = 0; y < n3DScreenGridSize; y++) {
				float tuLeft, tuRight;
				float tvTop, tvBottom;

				int ry = nRotateGame & 2 ? n3DScreenGridSize - 1 - y : y;

				tvTop = float(nGameImageHeight) * y / n3DScreenGridSize;
				tvBottom = float(nGameImageHeight) * (y + 1) / n3DScreenGridSize;

				for (int x = 0; x < n3DScreenGridSize; x++) {

					int rx = nRotateGame & 2 ? n3DScreenGridSize - 1 - x : x;

					tuLeft = float(nGameImageWidth) * x / n3DScreenGridSize;
					tuRight = float(nGameImageWidth) * (x + 1) / n3DScreenGridSize;

					if ((!(nRotateGame & 1) && bRotateEffects) || ((nRotateGame & 1) && !bRotateEffects)) {
						int i = 6 * (n3DScreenGridSize - 1 - rx) + ((n3DScreenGridSize - 1 - ry) * 6 * n3DScreenGridSize);
						v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tu1 = tuRight; v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tv1 = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tu1 = tuRight; v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tv1 = tvBottom;
						v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tu1 = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tv1 = tvTop;

						v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tu1 = tuRight; v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tv1 = tvBottom;
						v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tu1 = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tv1 = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tu1 = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tv1 = tvBottom;
					} else {
						int i = 6 * rx + (ry * 6 * n3DScreenGridSize);
						v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tu1 = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 5 : 0)].tv1 = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tu1 = tuRight; v3DScreen[i + (nRotateGame & 2 ? 4 : 1)].tv1 = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tu1 = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 3 : 2)].tv1 = tvBottom;

						v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tu1 = tuRight; v3DScreen[i + (nRotateGame & 2 ? 2 : 3)].tv1 = tvTop;
						v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tu1 = tuLeft;  v3DScreen[i + (nRotateGame & 2 ? 1 : 4)].tv1 = tvBottom;
						v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tu1 = tuRight; v3DScreen[i + (nRotateGame & 2 ? 0 : 5)].tv1 = tvBottom;
					}
				}
			}
		}
	}

	return 0;
}

static void PutPixel(unsigned char** ppSurface, unsigned int nColour)
{
	switch (nVidScrnDepth) {
		case 15:
			*((unsigned short*)(*ppSurface)) = ((nColour >> 9) & 0x7C00) | ((nColour >> 6) & 0x03E0) | ((nColour >> 3) & 0x001F);
			*ppSurface += 2;
			break;
		case 16:
			*((unsigned short*)(*ppSurface)) = ((nColour >> 8) & 0xF800) | ((nColour >> 5) & 0x07E0) | ((nColour >> 3) & 0x001F);
			*ppSurface += 2;
			break;
		case 24:
			(*ppSurface)[0] = (nColour >> 16) & 0xFF;
			(*ppSurface)[1] = (nColour >>  8) & 0xFF;
			(*ppSurface)[2] = (nColour >>  0) & 0xFF;
			*ppSurface += 3;
			break;
		case 32:
			*((unsigned int*)(*ppSurface)) = nColour;
			*ppSurface += 4;
			break;
	}
}

static int ReleaseSurfaces()
{
	// Release effects surfaces
	RELEASE(pRGBEffectTexture);
	RELEASE(pScanlineTexture[0]);
	RELEASE(pScanlineTexture[1]);

	// Release game-image surfaces
	RELEASE(pEmuImage[0]);
	RELEASE(pEmuImage[1]);
	RELEASE(pEmuImage[2]);
	RELEASE(pEmuImage[3]);

	VidSFreeVidImage();

	return 0;
}

static int vidExit()
{
	VidSExitOSD();

	if (v3DScreen) {
		free(v3DScreen);
		v3DScreen = NULL;
	}

	VidSoftFXExit();

	ReleaseSurfaces();

	VidSRestoreGamma();

	RELEASE(pD3DDevice);
	RELEASE(pD3D);

	VidSRestoreScreenMode();

	if (bUsePageflip == false) {
		RELEASE(pBackbuffer);				// explicitly free the off-screen surface when not usign pageflipping
	}
	RELEASE(pPrimarySurf);
	pBackbuffer = NULL;

	VidSExit();

	RELEASE(pDD);

	return 0;
}

static HRESULT CALLBACK myEnumTexturesCallback(DDPIXELFORMAT* pddpf, void* pPreferredddpf)
{
    if (pddpf->dwFlags == DDPF_RGB && pddpf->dwRGBBitCount == 16) {

		if (VidSoftFXCheckDepth(nPreScaleEffect, 15) == 15) {
			// Use 15-bit format if supported by the 3D hardware and the effects blitter
			if(pddpf->dwRBitMask == 0x7C00 && pddpf->dwGBitMask == 0x03E0 && pddpf->dwBBitMask == 0x001F) {
				memcpy(pPreferredddpf, pddpf, sizeof(DDPIXELFORMAT) );

				return D3DENUMRET_CANCEL;
			}
		}

		// Use 16-bit format otherwise
		if(pddpf->dwRBitMask == 0xF800 && pddpf->dwGBitMask == 0x07E0 && pddpf->dwBBitMask == 0x001F) {
            memcpy(pPreferredddpf, pddpf, sizeof(DDPIXELFORMAT) );
		}
    }
    return D3DENUMRET_OK;
}

static int vidCreateGameSurfaces()
{
	bool bForceTextureFormat;

	DDSURFACEDESC2 ddsd;
	DDPIXELFORMAT ddpf;

	// Determine if we should use a texture format different from the screen format
	bForceTextureFormat = false;
	if (VidSoftFXCheckDepth(nPreScaleEffect, 16) != 32 && nVidScrnDepth > 16 &&
		((bDrvOkay || VidSoftFXCheckDepth(nPreScaleEffect, 32) != 32) ||
		(bDrvOkay && (bVidForce16bit || !bDoGamma || (nVidFullscreen && bVidUseHardwareGamma) || (!nVidFullscreen && bHardwareGammaOnly)))))
	{
		memset(&ddpf, 0, sizeof(DDPIXELFORMAT));

		pD3DDevice->EnumTextureFormats(&myEnumTexturesCallback, (void*)&ddpf);

		if (ddpf.dwSize) {
			bForceTextureFormat = true;
		}
	}

	// Create a secondary surface to render the game image onto for the feedback effect
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;

	ddsd.dwWidth = (nPreScale & 1) ? nPreScaleTextureWidth : nTextureWidth;
	ddsd.dwHeight = (nPreScale & 2) ? nPreScaleTextureHeight : nTextureHeight;

	memset(&ddsd.ddpfPixelFormat, 0, sizeof(DDPIXELFORMAT));

	if (FAILED(pDD->CreateSurface(&ddsd, &pEmuImage[1], NULL))) {
		bRenderToTexture = false;
	} else {

		IDirectDrawSurface7* pPreviousTarget;
		if (FAILED(pD3DDevice->GetRenderTarget(&pPreviousTarget))) {
			bRenderToTexture = false;
		} else {
			if (FAILED(pD3DDevice->SetRenderTarget(pEmuImage[1], 0))) {
				 bRenderToTexture = false;
			 } else {
				 bRenderToTexture = true;

				 VidSClearSurface(pEmuImage[1], 0, NULL);
			 }
			 pD3DDevice->SetRenderTarget(pPreviousTarget, 0);
			 pPreviousTarget->Release();
		}
	}

	if (!bRenderToTexture) {

#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Warning: Device can't render to textures. Using fall-back method for pre-scale/feedback.\n"));
#endif

		RELEASE(pEmuImage[1]);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

		ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;

		ddsd.dwWidth = (nPreScale & 1) ? nPreScaleTextureWidth : nTextureWidth;
		ddsd.dwHeight = (nPreScale & 2) ? nPreScaleTextureHeight : nTextureHeight;

		memset(&ddsd.ddpfPixelFormat, 0, sizeof(DDPIXELFORMAT));

		if (FAILED(pDD->CreateSurface(&ddsd, &pEmuImage[1], NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create secondary game surface(s) for pre-scale/feedback.\n"));
#endif
			return 1;
		}

		VidSClearSurface(pEmuImage[1], 0, NULL);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

		ddsd.dwWidth = (nPreScale & 1) ? nPreScaleTextureWidth : nTextureWidth;
		ddsd.dwHeight = (nPreScale & 2) ? nPreScaleTextureHeight : nTextureHeight;

		if (FAILED(pDD->CreateSurface(&ddsd, &pEmuImage[3], NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create secondary game surface(s) for pre-scale/feedback.\n"));
#endif
			return 1;
		}

		VidSClearSurface(pEmuImage[3], 0, NULL);
	}

	if (nVidTransferMethod <= 0) {

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

		ddsd.dwWidth = nTextureWidth;
		ddsd.dwHeight = nTextureHeight;

		if (nPreScaleEffect) {
			ddsd.dwWidth = nPreScaleTextureWidth;
			ddsd.dwHeight = nPreScaleTextureHeight;
		}

		if (bForceTextureFormat) {
			ddsd.dwFlags |= DDSD_PIXELFORMAT;

			memcpy(&ddsd.ddpfPixelFormat, &ddpf, sizeof(DDPIXELFORMAT));
		}

		if (FAILED(pDD->CreateSurface(&ddsd, &pEmuImage[2], NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create secondary game surface for transfer.\n"));
#endif
			return 1;
		}

		VidSClearSurface(pEmuImage[2], 0, NULL);
	}

	// Create a surface for the texture that will contain the game image
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	if (nVidTransferMethod > 0) {
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTDYNAMIC | DDSCAPS2_TEXTUREMANAGE;
	}

	ddsd.dwWidth = nTextureWidth;
	ddsd.dwHeight = nTextureHeight;

	if (nPreScaleEffect) {
		ddsd.dwWidth = nPreScaleTextureWidth;
		ddsd.dwHeight = nPreScaleTextureHeight;
	}

	if (bForceTextureFormat) {
		ddsd.dwFlags |= DDSD_PIXELFORMAT;

		memcpy(&ddsd.ddpfPixelFormat, &ddpf, sizeof(DDPIXELFORMAT));
	}

	if (FAILED(pDD->CreateSurface(&ddsd, &pEmuImage[0], NULL))) {
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't create game texture surface.\n"));
#endif
		return 1;
	}

	VidSClearSurface(pEmuImage[0], 0, NULL);

	return 0;
}

static int InitEffectsSurfaces()
{
	DDSURFACEDESC2 ddsd;

	for (int i = 0, nSize = 2; i < 2; i++, nSize <<= 1) {
		int Scanlines[][8] = { { 0xFFFFFF, 0x000000 }, { 0x9F9F9F, 0xFFFFFF, 0x9F9F9F, 0x000000 } };

		if (pScanlineTexture[i]) {

			if (FAILED(pScanlineTexture[i]->IsLost())) {
				if (FAILED(pScanlineTexture[i]->Restore())) {
					return 1;
				}
			}

			memset(&ddsd, 0, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			if (FAILED(pScanlineTexture[i]->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_WAIT, NULL))) {
				return 1;
			}

			for (int y = 0; y < nSize; y++) {
				unsigned char* pSurface = (unsigned char*)ddsd.lpSurface + y * ddsd.lPitch;
				int nColour = Scanlines[i][y];
				for (int x = 0; x < nSize; x++) {
					PutPixel(&pSurface, nColour);
				}
			}

			pScanlineTexture[i]->Unlock(NULL);
		}
	}

	if (pRGBEffectTexture) {
		TCHAR pFilename[MAX_PATH] = _T("");
		int nPatternSize, nPatternXSize, nPatternYSize;
		unsigned char* RGBPattern = NULL;

		if (FAILED(pRGBEffectTexture->IsLost())) {
			if (FAILED(pRGBEffectTexture->Restore())) {
				return 1;
			}
		}

		// Get the filename and size of a pattern
		_tcscpy(pFilename, _T("pattern/"));
		_tcscat(pFilename, pRGBEffectPatternName);
		_stscanf(pRGBEffectPatternName, _T("%i x %i"), &nPatternXSize, &nPatternYSize);
		nPatternSize = nPatternXSize * nPatternYSize * 4;

		for (int i = 0; BuiltinPatternInfo[i].szName; i++) {
			if (!_tcscmp(BuiltinPatternInfo[i].szName, pRGBEffectPatternName)) {
				RGBPattern = (unsigned char*)malloc(nPatternSize);
				memcpy(RGBPattern, BuiltinPatternInfo[i].pData, nPatternSize);
				break;
			}
		}

		if (RGBPattern == NULL) {
			FILE* fp = _tfopen(pFilename, _T("rb"));
			if (fp) {

				// Read the file containing the pattern data
				fseek(fp, 0, SEEK_END);

				// Check if the file is the correct size.
				if (ftell(fp) != nPatternSize) {

					fclose(fp);
					FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("RGB pattern \'%s\' has an incorrect size.\nIt should be %i bytes long."), pRGBEffectPatternName, nPatternSize);
					FBAPopupDisplay(PUF_TYPE_WARNING);
					return 1;
				}

				RGBPattern = (unsigned char*)malloc(nPatternSize);
				fseek(fp, 0, SEEK_SET);
				fread(RGBPattern, 1, nPatternSize, fp);
				fclose(fp);
			}
		}

		if (RGBPattern) {
			// Pre-process the pattern
			for (int y = 0; y < nPatternYSize; y++) {
				unsigned char* pPattern = &RGBPattern[y * (nPatternXSize << 2)];
				for (int x = 0; x < nPatternXSize; x++) {
					switch (nRGBEffectPatternmode) {
						case 0:
						case 1: {
							if (nRGBEffectPatternmode == 0 || pPattern[(x << 2) + 3]) {
								int n;
								for (int z = 0; z < 3; z++) {
									n = nRGBEffectIntensity * pPattern[(x << 2) + z] / 256;
									pPattern[(x << 2) + z] = (n > 255) ? 255 : nRGBEffectFloor + (n * (256 - nRGBEffectFloor) / 256);
								}
							}
							break;
						}
						case 2:
						case 3: {
							int nSaturate = 0;
							int nMax = 0;
							int n;

							if (pPattern[(x << 2) + 0] != pPattern[(x << 2) + 1] || pPattern[(x << 2) + 0] != pPattern[(x << 2) + 2] || pPattern[(x << 2) + 1] != pPattern[(x << 2) + 2]) {

								if (pPattern[(x << 2) + 0] >= pPattern[(x << 2) + 1] && pPattern[(x << 2) + 0] >= pPattern[(x << 2) + 2]) {
									nMax = pPattern[(x << 2) + 0];
									nSaturate |= 1;
								}
								if (pPattern[(x << 2) + 1] >= pPattern[(x << 2) + 0] && pPattern[(x << 2) + 1] >= pPattern[(x << 2) + 2]) {
									nMax = pPattern[(x << 2) + 1];
									nSaturate |= 2;
								}
								if (pPattern[(x << 2) + 2] >= pPattern[(x << 2) + 0] && pPattern[(x << 2) + 2] >= pPattern[(x << 2) + 1]) {
									nMax = pPattern[(x << 2) + 2];
									nSaturate |= 4;
								}

								nSaturate ^= 7;
							}

							for (int z = 0; z < 3; z++) {
								if (nRGBEffectPatternmode == 2 || pPattern[(x << 2) + 3]) {
									n = pPattern[(x << 2) + z];
									if (nSaturate & (1 << z)) {
										n += (nMax - n) * nRGBEffectSaturation / 256;
									}
									n = n * nRGBEffectIntensity / 256;
									pPattern[(x << 2) + z] = (n > 255) ? 255 : nRGBEffectFloor + (n * (256 - nRGBEffectFloor) / 256);
								}
							}
							break;
						}
					}
				}
			}

			// Replicate the pattern over the needed size
			memset(&ddsd, 0, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			if (FAILED(pRGBEffectTexture->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_WAIT, NULL))) {
				return 1;
			} else {
				if ((!(nRotateGame & 1) && bRotateEffects) || ((nRotateGame & 1) && !bRotateEffects)) {
					for (unsigned int y = 0; y < ddsd.dwHeight; y++) {
						int ry = nRotateGame & 2 ? ddsd.dwHeight - 1 - y : y;
						unsigned char* pSurface = (unsigned char*)ddsd.lpSurface + ddsd.lPitch * y;
						unsigned char* pPattern = &RGBPattern[(ry % nPatternXSize) << 2];
						for (unsigned int x = 0; x < ddsd.dwWidth; x++) {
							int rx = nRotateGame & 2 ? ddsd.dwWidth - 1 - x : x;
							int nColour = pPattern[((rx % nPatternYSize) * (nPatternXSize << 2)) + 0] << 16;
							nColour |= pPattern[((rx % nPatternYSize) * (nPatternXSize << 2)) + 1] << 8;
							nColour |= pPattern[((rx % nPatternYSize) * (nPatternXSize << 2)) + 2];

							PutPixel(&pSurface, nColour);
						}
					}
				} else {
					for (unsigned int y = 0; y < ddsd.dwHeight; y++) {
						int ry = nRotateGame & 2 ? ddsd.dwHeight - 1 - y : y;
						unsigned char* pSurface = (unsigned char*)ddsd.lpSurface + ddsd.lPitch * y;
						unsigned char* pPattern = &RGBPattern[(ry % nPatternYSize) * nPatternXSize << 2];
						for (unsigned int x = 0; x < ddsd.dwWidth; x++) {
							int rx = nRotateGame & 2 ? ddsd.dwWidth - 1 - x : x;
							int nColour = pPattern[((rx % nPatternXSize) << 2) + 0] << 16;
							nColour |= pPattern[((rx % nPatternXSize) << 2) + 1] << 8;
							nColour |= pPattern[((rx % nPatternXSize) << 2) + 2];

							PutPixel(&pSurface, nColour);
						}
					}
				}
				pRGBEffectTexture->Unlock(NULL);
			}

			if (RGBPattern) {
				free(RGBPattern);
				RGBPattern = NULL;
			}
		} else {
			FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Couldn't find RGB pattern."));
			FBAPopupDisplay(PUF_TYPE_WARNING);
			return 1;
		}
	}

	return 0;
}

// Create extra surfaces for textures and effects
static int vidCreateEffectsSurfaces()
{
	DDSURFACEDESC2 ddsd;

	// Create the scanline textures
	for (int i = 0, nSize = 2; i < 2; i++, nSize <<= 1) {
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;

		ddsd.dwWidth = nSize;
		ddsd.dwHeight = nSize;

		if (FAILED(pDD->CreateSurface(&ddsd, &pScanlineTexture[i], NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create scanline texture.\n"));
#endif
			return 1;
		}
	}

	if (bUseRGBEffects) {
		// Create the RGB effect texture
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
//		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

		if (!nVidFullscreen || (nVidFullscreen && bVidFullStretch)) {
			ddsd.dwWidth = nVidScrnWidth;
			ddsd.dwHeight = nVidScrnHeight;
		} else {
			if (bVidArcaderes) {
				ddsd.dwWidth = nGameWidth;
				ddsd.dwHeight = nGameHeight;
			} else {
				RECT rect = { 0, 0, nVidScrnWidth, nVidScrnHeight };

				VidImageSize(&rect, nGameWidth, nGameHeight);

				ddsd.dwWidth = rect.right - rect.left;
				ddsd.dwHeight = rect.bottom - rect.top;
			}
		}

		if (FAILED(pDD->CreateSurface(&ddsd, &pRGBEffectTexture, NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create RGB effects surface.\n"));
#endif
			return 1;
		}
	}

	InitEffectsSurfaces();

	return 0;
}

static int vidAllocSurfaces()
{
	if (pDD == NULL || pD3DDevice == NULL) {
		return 1;
	}

	if (bDrvOkay) {
		BurnDrvGetFullSize(&nVidImageWidth, &nVidImageHeight);
		BurnDrvGetVisibleOffs(&nVidImageLeft, &nVidImageTop);
	}

	if (nRotateGame & 1) {
		nGameImageWidth = nGameHeight;
		nGameImageHeight = nGameWidth;
	} else {
		nGameImageWidth = nGameWidth;
		nGameImageHeight = nGameHeight;
	}

	nTextureWidth = GetTextureSize(nGameImageWidth);
	nTextureHeight = GetTextureSize(nGameImageHeight);
	nPreScaleTextureWidth = GetTextureSize(nGameImageWidth * nPreScaleZoom);
	nPreScaleTextureHeight = GetTextureSize(nGameImageHeight * nPreScaleZoom);

	// 2xSaI etc. needs an extra line below the image
	if (nPreScaleEffect >= FILTER_SUPEREAGLE && nPreScaleEffect <= FILTER_SUPER_2XSAI && nGameImageHeight == nTextureHeight) {
		nTextureHeight <<= 1;
	}

	if (d3dDeviceDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
		if (nTextureWidth < nTextureHeight) {
			nTextureWidth = nTextureHeight;
		}
		if (nTextureHeight < nTextureWidth) {
			nTextureHeight = nTextureWidth;
		}
	}

	if ((int)d3dDeviceDesc.dwMaxTextureWidth < nTextureWidth || (int)d3dDeviceDesc.dwMaxTextureHeight < nTextureHeight) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE,_T("Required texture size not supported by 3D hardware."));
		FBAPopupDisplay(PUF_TYPE_ERROR);
		return 1;
	}

	// Allocate all DirectX surfaces
	if (vidCreateGameSurfaces()) {
		ReleaseSurfaces();
		return 1;
	}
	if (vidCreateEffectsSurfaces()) {
		ReleaseSurfaces();
		return 1;
	}

	nVidImageDepth = VidSGetSurfaceDepth(pEmuImage[0]);		// Get colourdepth of game (texture) surface

	for (int i = 0; i < 2; i++) {
		int nDepth[] = { 15, 16, 32 };
		if (VidSoftFXCheckDepth(nPreScaleEffect, nDepth[i]) == nVidImageDepth) {
			nVidImageDepth = nDepth[i];
			break;
		}
	}

	nVidImageBPP = (nVidImageDepth + 7) >> 3;

	VidSAllocVidImage();									// Set up memory buffer for Burn library

	SetBurnHighCol(nVidImageDepth);

	return 0;
}

static int vidClear()
{
	if (pPrimarySurf == NULL) {
		return 1;
	}

	//if(bVidVSync && !nVidFullscreen) { pDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL); }

	if (nVidFullscreen) {
		VidSClearSurface(pPrimarySurf, 0, NULL);			// Clear front buffer
		if (bUsePageflip) {
			pPrimarySurf->Flip(NULL, DDFLIP_WAIT);
			VidSClearSurface(pPrimarySurf, 0, NULL);		// Clear 1st page of backbuffer

			if (bUseTriplebuffer) {
				pPrimarySurf->Flip(NULL, DDFLIP_WAIT);
				VidSClearSurface(pPrimarySurf, 0, NULL);	// Clear 2nd page of backbuffer
			}

			pPrimarySurf->Flip(NULL, DDFLIP_WAIT);
		} else {
			VidSClearSurface(pBackbuffer, 0, NULL);			// Clear backbuffer
		}
	} else {
		VidSClearSurface(pBackbuffer, 0, NULL);				// Clear backbuffer
	}

	return 0;
}

static int vidInitBuffers(bool bTriple)
{
	DDSURFACEDESC2 ddsd;
    DDSCAPS2 ddscaps;

	bUsePageflip = false;

	if (bDrvOkay && nVidFullscreen) {
		if (bTriple) {
			bUsePageflip = true;
		} else {
			DDCAPS ddcaps;

			memset(&ddcaps, 0, sizeof(ddcaps));
			ddcaps.dwSize = sizeof(ddcaps);

			pDD->GetCaps(&ddcaps, NULL);
			//if (ddcaps.dwCaps2 & DDCAPS2_FLIPNOVSYNC) {
				bUsePageflip = true;
			//}
		}
	}

	// Create the primary surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	if (nVidFullscreen && bUsePageflip) {
		// Allocate a complex surface that supports page flipping.
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = bVidTripleBuffer ? 2 : 1;
	} else {
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
	}

	if (FAILED(pDD->CreateSurface(&ddsd, &pPrimarySurf, NULL))) {
		pPrimarySurf = NULL;
		return 1;
	}

	// Get the back buffer
	if (nVidFullscreen && bUsePageflip) {
		memset(&ddscaps, 0, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(pPrimarySurf->GetAttachedSurface(&ddscaps, &pBackbuffer))) {
			RELEASE(pPrimarySurf);
			pBackbuffer = NULL;
			return 1;
		}

		bUseTriplebuffer = bTriple;

	} else {
		// Running in windowed mode or using blt() transfer the final image. Allocate an off-screen surface to render to.
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth = nVidScrnWidth;
		ddsd.dwHeight = nVidScrnHeight;
		if (FAILED(pDD->CreateSurface(&ddsd, &pBackbuffer, NULL))) {
			RELEASE(pPrimarySurf);
			pBackbuffer = NULL;
			return 1;
		}
	}

	nVidScrnDepth = VidSGetSurfaceDepth(pPrimarySurf);	// Get colourdepth of primary surface

	vidClear();											// Clear the surfaces we've just allocated

	return 0;
}

static GUID MyGuid;
static int nWantDriver;

#if 1 && defined(PRINT_DEBUG_INFO)
static int nCurrentDriver;
#ifdef UNICODE
static BOOL PASCAL MyEnumDisplayDrivers(GUID FAR* pGuid, LPWSTR pszDesc, LPWSTR /*pszName*/, LPVOID /*pContext*/, HMONITOR hMonitor)
#else
static BOOL PASCAL MyEnumDisplayDrivers(GUID FAR* pGuid, LPSTR pszDesc, LPSTR /*pszName*/, LPVOID /*pContext*/, HMONITOR hMonitor)
#endif
{
	if (nCurrentDriver == nWantDriver && pGuid) {
		memcpy(&MyGuid, pGuid, sizeof(GUID));
	}

	if (nCurrentDriver == 0) {
		dprintf(_T("    %s\n"), pszDesc);
	} else {
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);

		GetMonitorInfo(hMonitor, (MONITORINFO*)&mi);

		dprintf(_T("    Display %d (%s on %s"), nCurrentDriver, mi.szDevice, pszDesc);
		if (mi.dwFlags & MONITORINFOF_PRIMARY) {
			dprintf(_T(", primary"));
		}
		dprintf(_T(")\n"));
	}

	nCurrentDriver++;

	return DDENUMRET_OK;
}
#endif

#if 0
static HRESULT CALLBACK MyEnumDevicesCallback(LPSTR lpDeviceDescription, LPSTR /*lpDeviceName*/, LPD3DDEVICEDESC7, LPVOID)
{
	dprintf(_T("    %s\n"), lpDeviceDescription);

	return DDENUMRET_OK;
}
#endif

static int vidInit()
{
#ifdef ENABLE_PROFILING
	ProfileInit();
#endif

	bCreateImage = true;

	bUseTriplebuffer = false;

	hVidWnd = nVidFullscreen ? hScrnWnd : hVideoWindow;								// Use Screen window for video

	nWantDriver = 0;
#if 1 && defined(PRINT_DEBUG_INFO)
	nCurrentDriver = 0;
	memset(&MyGuid, 0, sizeof(GUID));
	dprintf(_T(" ** Enumerating available DirectDraw drivers:\n"));
	_DirectDrawEnumerateEx(MyEnumDisplayDrivers, NULL, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
#endif

	// Get pointer to DirectDraw device
	_DirectDrawCreateEx(nWantDriver ? &MyGuid : NULL, (void**)&pDD, IID_IDirectDraw7, NULL);

	VidSInit(pDD);

	// Get pointer to D3D device
	if (FAILED(pDD->QueryInterface(IID_IDirect3D7, (void**)&pD3D))) {
#ifdef PRINT_DEBUG_INFO
	   	dprintf(_T("  * Error: Couldn't access Direct3D.\n"));
#endif
		vidExit();
		return 1;
	}

#if 0 && defined(PRINT_DEBUG_INFO)
	dprintf(_T(" ** Enumerating available 3D devices:\n"));
	pD3D->EnumDevices(MyEnumDevicesCallback, NULL);
#endif

	pPrimarySurf = NULL;							// No primary surface yet
	pBackbuffer = NULL;

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

	nPreScale = 0;
	nPreScaleZoom = 2;
	nPreScaleEffect = 0;
	if (nVidBlitterOpt[nVidSelect] & 0x01000000) {
		nPreScale = 3;

		if (nVidBlitterOpt[nVidSelect] & 0x02000000) {
			nPreScaleEffect = (unsigned long long)(nVidBlitterOpt[nVidSelect] >> 32);
			nPreScaleZoom = VidSoftFXGetZoom(nPreScaleEffect);
		} else {
			if (bVidScanlines) {
				nPreScale &= 2;
			}
		}
	}

	// Set up the display mode
	if (nVidFullscreen) {
		int nZoomlevel;
		if (nVidBlitterOpt[nVidSelect] & 0x04000000) {
			nZoomlevel = nPreScaleZoom;
		} else {
			nZoomlevel = nScreenSize;
		}
		if (VidSEnterFullscreenMode(nZoomlevel, 0)) {
			vidExit();
			return 1;
		}
	} else {
		RECT rect;

		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		nVidScrnWidth = rect.right - rect.left;
		nVidScrnHeight = rect.bottom - rect.top;

		pDD->SetCooperativeLevel(hVidWnd, DDSCL_NORMAL);
	}

#ifdef PRINT_DEBUG_INFO
	{
		// Display amount of free video memory
		DDSCAPS2 ddsCaps2;
		DWORD dwTotal;
		DWORD dwFree;

		memset(&ddsCaps2, 0, sizeof(ddsCaps2));
		ddsCaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;

		dprintf(_T(" ** Starting Direct3D7 blitter.\n"));

		if (SUCCEEDED(pDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree))) {
			dprintf(_T("  * Initialising video: Total video memory minus display surface: %.2fMB.\n"), (double)dwTotal / (1024 * 1024));
		}
	}
#endif

	if (bVidTripleBuffer) {
		if (vidInitBuffers(1)) {				// Try to make triple buffer
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Warning: Couldn't allocate a triple-buffering surface.\n"));
#endif
		}
	}

	if (pPrimarySurf == NULL) {					// No primary surface yet, so try normal
		vidInitBuffers(0);
	}

	if (pPrimarySurf == NULL) {					// No primary surface, fail
#ifdef PRINT_DEBUG_INFO
	   	dprintf(_T("  * Error: Couldn't create primary surface.\n"));
#endif
		vidExit();
		return 1;
	}

	if (nVidFullscreen) {
		pDD->Compact();
	}

	VidSClipperInit(pPrimarySurf);				// Initialise clipper

	VidSSetupGamma(pPrimarySurf);				// Set up gamma controls

	if (bVidScanRotate && nGameWidth < nGameHeight) {
		bRotateEffects = true;
	} else {
		bRotateEffects = false;
	}

	if (nRotateGame & 1) {
		bRotateEffects = !bRotateEffects;
	}

	bUse3DProjection = false;
	if (nVidBlitterOpt[nVidSelect] & 0x00100000) {
		bUse3DProjection = true;
	}
	bUseLighting = false;
	n3DScreenGridSize = 16;
	if (nVidBlitterOpt[nVidSelect] & 0x00200000) {
		bUseLighting = true;
		n3DScreenGridSize *= 2;
	}

	bUseRGBEffects = false;
	if (nVidBlitterOpt[nVidSelect] & 0x00010000) {
		int nPreset = -1;

		if (nVidBlitterOpt[nVidSelect] & 0x00020000) {
			RECT rect = { 0, 0, 0, 0 };
			int nScaledSize, nOriginalSize;

			GetClientScreenRect(hVidWnd, &rect);
			if (!nVidFullscreen) {
				rect.top += 0 /*nMenuHeight*/;
			}
			VidImageSize(&rect, nGameWidth, nGameHeight);

			if (bVidScanRotate && nGameWidth < nGameHeight) {
				nScaledSize = rect.right - rect.left;
				nOriginalSize = nGameWidth;
			} else {
				nScaledSize = rect.bottom - rect.top;
				nOriginalSize = nGameHeight;
			}

			for (int n = 7; n >= 0; n--) {
				if (nScaledSize >= (n + 1) * nOriginalSize) {
					nPreset = n;
					break;
				}
			}
		} else {
			nPreset = nVidBlitterOpt[nVidSelect] & 0xFF;
		}

		if (nPreset != -1) {
			if (PresetInfo[nPreset].nUseFilter != 0) {
				bVidBilinear = PresetInfo[nPreset].nUseFilter == 1 ? true : false;
			}

			if (PresetInfo[nPreset].nUseEffects != 0) {
				bUseRGBEffects = PresetInfo[nPreset].nUseEffects == 1 ? true : false;
				if (PresetInfo[nPreset].nUseEffects == 1) {
					nRGBEffectIntensity = PresetInfo[nPreset].nIntensity;
					nRGBEffectFloor = PresetInfo[nPreset].nFloor;
					nRGBEffectSaturation = PresetInfo[nPreset].nSaturation;
					nRGBEffectPatternmode = PresetInfo[nPreset].nPatternMode;
					pRGBEffectPatternName = PresetInfo[nPreset].pPatternName;
					nRGBEffectAttenuation = PresetInfo[nPreset].nAttenuation;
					nRGBEffectBlendmode = PresetInfo[nPreset].nBlendMode;
				}
			}

			if (PresetInfo[nPreset].nUseScanlines != 0) {
				bVidScanlines = PresetInfo[nPreset].nUseScanlines == 1 ? true : false;
				if (PresetInfo[nPreset].nUseScanlines == 1) {
					nRGBScanlineIntensity = PresetInfo[nPreset].nScanIntensity;
				}
			}

			if (PresetInfo[nPreset].nUseFeedback != 0) {
				bVidScanDelay = PresetInfo[nPreset].nUseFeedback == 1 ? true : false;
				if (PresetInfo[nPreset].nUseFeedback == 1) {
					nVidFeedbackIntensity = PresetInfo[nPreset].nAmount;
					nVidFeedbackOverSaturation = PresetInfo[nPreset].nOverSaturation;
				}
			}

			if (PresetInfo[nPreset].nUsePrescale != 0) {
				if (PresetInfo[nPreset].nUsePrescale == 1) {
					nVidBlitterOpt[nVidSelect] |= 0x01000000;
				} else {
					nVidBlitterOpt[nVidSelect] &= ~0x01000000;
				}
			}

			if (nVidBlitterOpt[nVidSelect] & 0x00020000) {
				// Modify the effect a bit if it's rotated
				if ((!(nRotateGame & 1) && bRotateEffects) || ((nRotateGame & 1) && !bRotateEffects)) {
					nRGBEffectIntensity = nRGBEffectIntensity * 0xF0 / 0x0100;
					nRGBEffectSaturation += (0x0100 - nRGBEffectSaturation) * 0x70 / 0x0100;
					int nScan = (((nRGBScanlineIntensity >> 16) & 0xFF) * 0xE0 / 0x0100) << 16;
					nScan |= (((nRGBScanlineIntensity >> 8) & 0xFF) * 0xE0 / 0x0100) << 8;
					nScan |= (((nRGBScanlineIntensity >> 0) & 0xFF) * 0xE0 / 0x0100) << 0;
					nRGBScanlineIntensity = nScan;
				}
			}
		}
	}

	if (bRotateEffects) {
		nPreScale = ((nPreScale >> 1) | (nPreScale << 1)) & 3;
	}

	pD3DDevice = NULL;

#ifdef USE_D3D_REFERENCE_DEVICE
	if (FAILED(pD3D->CreateDevice(IID_IDirect3DRefDevice, pBackbuffer, &pD3DDevice))) {
 #ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't access Direct3D Reference device.\n"));
 #endif
		vidExit();
		return 1;
	} else {
		dprintf(_T("  * Using Direct3D Reference device.\n"));
	}
#else
	if (bUse3DProjection) {
		if (FAILED(pD3D->CreateDevice(IID_IDirect3DTnLHalDevice, pBackbuffer, &pD3DDevice))) {
			pD3DDevice = NULL;
 #ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Warning: Couldn't get TnL 3D hardware, falling back to software TnL.\n"));
 #endif
		}
	}
	if (pD3DDevice == NULL) {
		if (FAILED(pD3D->CreateDevice(IID_IDirect3DHALDevice, pBackbuffer, &pD3DDevice))) {
 #ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't access 3D hardware.\n"));
 #endif
			vidExit();
			return 1;
		}
	}
#endif

	pD3DDevice->GetCaps(&d3dDeviceDesc);									// Get the capabilities of the 3D hardware

	bMultiTexturing = false;
	if (d3dDeviceDesc.wMaxSimultaneousTextures >= 2 && d3dDeviceDesc.wMaxTextureBlendStages >= 2) {
		bMultiTexturing = true;
	}

#ifdef PRINT_DEBUG_INFO
	if (!bMultiTexturing) {
		dprintf(_T("  * Warning: Using fall-back method for rendering scanlines.\n"));
	}
#endif

	nScanlineOp = D3DTOP_ADD;
	if (d3dDeviceDesc.dwTextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH) {
		nScanlineOp = D3DTOP_ADDSMOOTH;
	}

#ifdef PRINT_DEBUG_INFO
	if (nScanlineOp != D3DTOP_ADDSMOOTH) {
		dprintf(_T("  * Warning: Using fall-back method for blending scanlines.\n"));
	}
#endif

	// Init the buffer surfaces
	if (vidAllocSurfaces()) {
		ReleaseSurfaces();
		return 1;
	}

	if (nPreScaleEffect) {
		if (VidSoftFXInit(nPreScaleEffect, 0)) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't initialise software SoftFX.\n"));
#endif
			vidExit();
			return 1;
		}
	}

	// Add a viewport
	Viewport.dwX = 0;
	Viewport.dwY = 0;
	Viewport.dwWidth = nVidScrnWidth;
	Viewport.dwHeight = nVidScrnHeight;

	if (FAILED(pD3DDevice->SetViewport(&Viewport))) {
#ifdef PRINT_DEBUG_INFO
	   	dprintf(_T("  * Error: Couldn't set initial viewport.\n"));
#endif
		vidExit();
		return 1;
	}

	pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	if (bUse3DProjection) {
		pD3DDevice->SetRenderState(D3DRENDERSTATE_COLORVERTEX, FALSE);
	}

	pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

	// The polygons for the emulator image on the screen
	{
		int nWidth = nGameImageWidth;
		int nHeight = nGameImageHeight;
		int nTexWidth = nTextureWidth;
		int nTexHeight = nTextureHeight;

		if (nPreScaleEffect) {
			if (nPreScale & 1) {
				nWidth *= nPreScaleZoom;
				nTexWidth = nPreScaleTextureWidth;
			}
			if (nPreScale & 2) {
				nHeight *= nPreScaleZoom;
				nTexHeight = nPreScaleTextureHeight;
			}
		}

		if (nRotateGame & 1) {
			vScreen[nRotateGame & 2 ? 3 : 0] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, (float)nWidth / nTexWidth, 0, 0, 0);
			vScreen[nRotateGame & 2 ? 2 : 1] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, (float)nWidth / nTexWidth, (float)nHeight / nTexHeight, 0, 0);
			vScreen[nRotateGame & 2 ? 1 : 2] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, 0, 0, 0, 0);
			vScreen[nRotateGame & 2 ? 0 : 3] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, 0, (float)nHeight / nTexHeight, 0, 0);
		} else {
			vScreen[nRotateGame & 2 ? 3 : 0] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, 0, 0, 0, 0);
			vScreen[nRotateGame & 2 ? 2 : 1] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, (float)nWidth / nTexWidth, 0, 0, 0);
			vScreen[nRotateGame & 2 ? 1 : 2] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, 0, (float)nHeight / nTexHeight, 0, 0);
			vScreen[nRotateGame & 2 ? 0 : 3] = D3DTLVERTEX2(D3DVECTOR(0, 0, 0), 1, 0xFFFFFFFF, 0, (float)nWidth / nTexWidth, (float)nHeight / nTexHeight, 0, 0);
		}
	}

	// Set the texture coordinates for the scanlines
	if ((!(nRotateGame & 1) && bRotateEffects) || ((nRotateGame & 1) && !bRotateEffects)) {
		vScreen[nRotateGame & 2 ? 3 : 0].tu1 = float(nGameHeight);	vScreen[nRotateGame & 2 ? 3 : 0].tv1 = 0;
		vScreen[nRotateGame & 2 ? 2 : 1].tu1 = float(nGameHeight);	vScreen[nRotateGame & 2 ? 2 : 1].tv1 = (float)nGameWidth;
		vScreen[nRotateGame & 2 ? 1 : 2].tu1 = 0;					vScreen[nRotateGame & 2 ? 1 : 2].tv1 = 0;
		vScreen[nRotateGame & 2 ? 0 : 3].tu1 = 0;					vScreen[nRotateGame & 2 ? 0 : 3].tv1 = (float)nGameWidth;
	} else {
		vScreen[nRotateGame & 2 ? 3 : 0].tu1 = 0;					vScreen[nRotateGame & 2 ? 3 : 0].tv1 = 0;
		vScreen[nRotateGame & 2 ? 2 : 1].tu1 = float(nGameWidth);	vScreen[nRotateGame & 2 ? 2 : 1].tv1 = 0;
		vScreen[nRotateGame & 2 ? 1 : 2].tu1 = 0;					vScreen[nRotateGame & 2 ? 1 : 2].tv1 = (float)nGameHeight;
		vScreen[nRotateGame & 2 ? 0 : 3].tu1 = float(nGameWidth);	vScreen[nRotateGame & 2 ? 0 : 3].tv1 = (float)nGameHeight;
	}

	{
		int nWidth = nGameImageWidth;
		int nHeight = nGameImageHeight;

		if (nPreScale & 1) {
			nWidth *= nPreScaleZoom;
		}
		if (nPreScale & 2) {
			nHeight *= nPreScaleZoom;
		}

		vFeedbackImage[nRotateGame & 2 ? 3 : 0] = D3DTLVERTEX(D3DVECTOR(-0.5, -0.5, 2.0f), 1, 0, 0, 0, 0);
		vFeedbackImage[nRotateGame & 2 ? 2 : 1] = D3DTLVERTEX(D3DVECTOR(-0.5 + nWidth, -0.5, 2.0f), 1, 0, 0, float(nGameImageWidth) / nTextureWidth, 0);
		vFeedbackImage[nRotateGame & 2 ? 1 : 2] = D3DTLVERTEX(D3DVECTOR(-0.5, -0.5 + nHeight, 2.0f), 1, 0, 0, 0, float(nGameImageHeight) / nTextureHeight);
		vFeedbackImage[nRotateGame & 2 ? 0 : 3] = D3DTLVERTEX(D3DVECTOR(-0.5 + nWidth, -0.5 + nHeight, 2.0f), 1, 0, 0, float(nGameImageWidth) / nTextureWidth, float(nGameImageHeight) / nTextureHeight);
	}

	// Force an update of the matrices
	fPreviousScreenAngle = 9999.0f;
	fPreviousScreenCurvature = 9999.0f;

	if (Update3DScreen()) {
#ifdef PRINT_DEBUG_INFO
	   	dprintf(_T("  * Error: Couldn't initialise 3D geometry.\n"));
#endif
		vidExit();
		return 1;
	}

	int nFlags = 0;
	if (nVidScrnWidth < 1280) {
		nFlags++;
		if (nVidScrnWidth < 1024) {
			nFlags++;
			if (nVidScrnWidth < 640) {
				nFlags++;
			}
		}
	}
	VidSInitOSD(nFlags);

#ifdef PRINT_DEBUG_INFO
	{
		DDSCAPS2 ddsCaps2;
		DWORD dwTotal;
		DWORD dwFree;

		memset(&ddsCaps2, 0, sizeof(ddsCaps2));
		ddsCaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if (SUCCEEDED(pDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree))) {
			dprintf(_T("  * Initialisation complete: %.2fMB video memory free.\n"), (double)dwFree / (1024 * 1024));
			dprintf(_T("    Displaying and rendering in %i-bit mode, emulation running in %i-bit mode.\n"), nVidScrnDepth, nVidImageDepth);
			if (nVidFullscreen) {
				dprintf(_T("    Running in fullscreen mode (%i x %i), "), nVidScrnWidth, nVidScrnHeight);
				if (bUseTriplebuffer) {
					dprintf(_T("using a triple buffer.\n"));
				} else {
					if (bUsePageflip) {
						dprintf(_T("using a double buffer.\n"));
					} else {
						dprintf(_T("using BltFast() to transfer the final image.\n"));
					}
				}
			} else {
				dprintf(_T("    Running in windowed mode, using Blt() to transfer the final image.\n"));
			}
		}
	}
#endif

	bLostControl = false;

	return 0;
}

// Scale the image to fit the screen/window
static int vidScale(RECT* pRect, int nWidth, int nHeight)
{
	if ((nVidBlitterOpt[nVidSelect] & 0x07000000) == 0x07000000) {
		return VidSoftFXScale(pRect, nWidth, nHeight);
	}

	return VidSScaleImage(pRect, nWidth, nHeight, bVidScanRotate);
}

// Render the polygons with the emulator image
static inline void RenderPolygons()
{
	if (bUse3DProjection) {
		if (fVidScreenCurvature == 0.0f && !bUseLighting) {
			pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, (void*)v3DScreen, 4, 0);
		} else {
			pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX2, (void*)v3DScreen, 6 * n3DScreenGridSize * n3DScreenGridSize, 0);
		}
	} else {
		pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX2, (void*)vScreen, 4, 0);
	}
}

static int vidRenderImageA()
{
	if (bLostControl) {						// We've lost control of the video hardware
		return 1;
	}

	if (FAILED(pBackbuffer->IsLost())) {	// We've lost control of the video hardware
		return 1;
	}

	GetClientScreenRect(hVidWnd, &Dest);

	if (nVidFullscreen == 0) {
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
		vidScale(&Dest, nGameWidth, nGameHeight);
	}

	if (nVidFullscreen) {
		Render.left = Dest.left;
		Render.right = Dest.right;
		Render.top = Dest.top;
		Render.bottom = Dest.bottom;
	} else {
		Render.left = 0;
		Render.right = Dest.right - Dest.left;
		Render.top = 0;
		Render.bottom = Dest.bottom - Dest.top;
	}

	if (((nRotateGame & 1) && !bRotateEffects) || (!(nRotateGame & 1) && bRotateEffects)) {
		nZoomFactor = (Render.right - Render.left) / nGameWidth;
	} else {
		nZoomFactor = (Render.bottom - Render.top) / nGameHeight;
	}

	if (bUseRGBEffects) {
		RECT rect = {0, 0, Render.right - Render.left, Render.bottom - Render.top};
		pBackbuffer->BltFast(Render.left, Render.top, pRGBEffectTexture, &rect, DDBLTFAST_WAIT);
	} else {
		if (bUse3DProjection) {
			RECT rect = { Render.left, Render.top, Render.right, Render.top + 52 };
			VidSClearSurface(pBackbuffer, 0, &rect);
		}
	}

	return 0;
}

static int vidRenderImageB()
{
	int bScanlines = 0, nScanlineSize = 0;
	unsigned int nColour;
	int nPage;

	if (bLostControl) {						// We've lost control of the video hardware
		return 1;
	}

	if (FAILED(pBackbuffer->IsLost())) {	// We've lost control of the video hardware
		return 1;
	}

	if ((bDrvOkay && bVidScanDelay) || (nPreScale && nPreScaleEffect == 0)) {

		if (FAILED(pD3DDevice->BeginScene())) {
			return 1;
		}

		if (bRenderToTexture) {
			pD3DDevice->SetRenderTarget(pEmuImage[1], 0);
		} else {
			pD3DDevice->SetRenderTarget(pEmuImage[3], 0);
		}

		Viewport.dwX = 0;
		Viewport.dwY = 0;
		Viewport.dwWidth = nGameImageWidth;
		Viewport.dwHeight = nGameImageHeight;

		if (nPreScale & 1) {
			Viewport.dwWidth *= nPreScaleZoom;
		}

		if (nPreScale & 2) {
			Viewport.dwHeight *= nPreScaleZoom;
		}

		pD3DDevice->SetViewport(&Viewport);

		nColour = (0xFF - nVidFeedbackIntensity) + nVidFeedbackOverSaturation;
		nColour = (nColour > 0x0FF) ? 0xFF : nColour;
		nColour = (nVidFeedbackIntensity << 24) | (nColour << 16) | (nColour << 8) | nColour;

		vFeedbackImage[0].color = nColour;
		vFeedbackImage[1].color = nColour;
		vFeedbackImage[2].color = nColour;
		vFeedbackImage[3].color = nColour;

		pD3DDevice->SetTexture(0, pEmuImage[0]);

		pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
		pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		if (bDrvOkay && bVidScanDelay) {
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA);
		} else {
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
		}

		pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, (void*)vFeedbackImage, 4, 0);

		if (FAILED(pD3DDevice->EndScene())) {
			return 1;
		}

		pD3DDevice->SetTexture(0, NULL);

		pD3DDevice->SetRenderTarget(pBackbuffer, 0);

		if (!bRenderToTexture) {
			RECT rect = {0, 0, nGameImageWidth, nGameImageHeight};

			if (nPreScale & 1) {
				rect.right = nPreScaleTextureWidth;
			}
			if (nPreScale & 2) {
				rect.bottom = nPreScaleTextureHeight;
			}

			pEmuImage[1]->BltFast(0, 0, pEmuImage[3], &rect, DDBLTFAST_WAIT);
		}

		nPage = 1;
	} else {
		nPage = 0;
	}

	// Set up for the emulator image
	if (FAILED(pD3DDevice->BeginScene())) {
		return 1;
	}

	if (bUse3DProjection) {

		// It should only do this when the window size/pos has changed
		if (nVidFullscreen == 0) {
			static RECT prevDest = { 0, 0, 0, 0 };
			if (Render.left != prevDest.left || Render.top != prevDest.top || Render.right != prevDest.right || Render.bottom != prevDest.bottom) {
				prevDest.left = Render.left;
				prevDest.top = Render.top;
				prevDest.right = Render.right;
				prevDest.bottom = Render.bottom;

				VidSClearSurface(pBackbuffer, 0, &Render);
			}
		}

		if (fVidScreenAngle != fPreviousScreenAngle || fVidScreenCurvature != fPreviousScreenCurvature) {
			Update3DScreen();
			VidSClearSurface(pBackbuffer, 0, NULL);
			SetWorldMatrix();
			SetViewMatrix(2.0f);
			SetProjectionMatrix(2.0f);

			fPreviousScreenAngle = fVidScreenAngle;
			fPreviousScreenCurvature = fVidScreenCurvature;
		}

		Viewport.dwX = Render.left;
		Viewport.dwY = Render.top;
		Viewport.dwWidth = Render.right - Render.left;
		Viewport.dwHeight = Render.bottom - Render.top;
	} else {
		Viewport.dwX = 0;
		Viewport.dwY = 0;
		Viewport.dwWidth = nVidScrnWidth;
		Viewport.dwHeight = nVidScrnHeight;

		vScreen[0].sx = -0.5f + (float)Render.left;
		vScreen[0].sy = -0.5f + (float)Render.top;
		vScreen[1].sx = -0.5f + (float)Render.right;
		vScreen[1].sy = -0.5f + (float)Render.top;
		vScreen[2].sx = -0.5f + (float)Render.left;
		vScreen[2].sy = -0.5f + (float)Render.bottom;
		vScreen[3].sx = -0.5f + (float)Render.right;
		vScreen[3].sy = -0.5f + (float)Render.bottom;
	}

	pD3DDevice->SetViewport(&Viewport);

	if (bUseRGBEffects) {
		nColour = ((0xFF - nRGBEffectAttenuation) << 24) | nRGBScanlineIntensity;
		if (nRGBEffectBlendmode == 0) {
			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
		} else {
			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
		}
	} else {
		nColour = ((0xFF - nRGBEffectAttenuation) << 24) | nVidScanIntensity;

		if (bUse3DProjection && bUseLighting) {

			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

			pD3DDevice->LightEnable(0, TRUE);
			pD3DDevice->LightEnable(1, TRUE);

			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			RenderPolygons();

			pD3DDevice->LightEnable(1, FALSE);
			pD3DDevice->LightEnable(0, FALSE);

			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVDESTCOLOR);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		} else {
			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
		}
	}

	if (bVidScanlines) {
		if (nZoomFactor >= 2) {
			bScanlines = 1;
		}
		if (nZoomFactor >= 4 || bVidFullStretch) {
			nScanlineSize++;
		}
	}

	pD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, nColour);

	if (bMultiTexturing) {

		if (bScanlines) {
			if (bVidScanBilinear) {
				pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
			} else {
				pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
				pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
			}
			if (bVidBilinear) {
				pD3DDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				pD3DDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
			} else {
				pD3DDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_POINT);
				pD3DDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
			}

			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, nScanlineOp);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);

			pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			pD3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);

			pD3DDevice->SetTexture(0, pScanlineTexture[nScanlineSize]);
			pD3DDevice->SetTexture(1, pEmuImage[nPage]);

		} else {
			if (bVidBilinear) {
				pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
			} else {
				pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
				pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
			}

			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

			pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			pD3DDevice->SetTexture(0, pEmuImage[nPage]);
		}

		RenderPolygons();

		pD3DDevice->SetTexture(0, NULL);
		pD3DDevice->SetTexture(1, NULL);

	} else {						// multi-texturing not supported by hardware

		if (bVidBilinear) {
			pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
			pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
		} else {
			pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
			pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
		}

		pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

		pD3DDevice->SetTexture(0, pEmuImage[nPage]);

		pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		RenderPolygons();

		pD3DDevice->SetTexture(0, NULL);

		if (bScanlines) {

			if (bVidScanBilinear) {
				pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
			} else {
				pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
				pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
			}

			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, nScanlineOp);
			pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);

			pD3DDevice->SetTexture(0, pScanlineTexture[nScanlineSize]);

			pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
			pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

			RenderPolygons();

			pD3DDevice->SetTexture(0, NULL);
		}
	}

	if (FAILED(pD3DDevice->EndScene())) {
		return 1;
	}

	return 0;
}

// Copy the game image to the texture and render the image using 3D hardware
static int vidBurnToSurf()
{
	if (bLostControl) {						// We've lost control of the video hardware
		return 1;
	}

	RECT rect = { 0, 0, nGameImageWidth, nGameImageHeight };

	DDSURFACEDESC2 ddsd;

	unsigned char *pd, *ps;
	int nSize = nGameImageWidth * nVidImageBPP;
	int nPitch;

	// Lock the surface so we can write to it
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	if (nVidTransferMethod <= 0) {
		if (nPreScaleEffect) {

			rect.right *= nPreScaleZoom;
			rect.bottom *= nPreScaleZoom;

			VidSoftFXApplyEffectDirectX(pEmuImage[2], NULL);
		} else {
			// Copy the image to a surface (located in video memory), then use bltfast() to blit it to the texture
			if (FAILED(pEmuImage[2]->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))) {
				return 1;
			}
			ps = pVidImage + nVidImageLeft * nVidImageBPP;
			pd = (unsigned char*)ddsd.lpSurface;
			nPitch = ddsd.lPitch;

			for (int y = 0; y < nGameImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
				memcpy(pd, ps, nSize);
			}

			pEmuImage[2]->Unlock(NULL);
		}

		pEmuImage[0]->BltFast(0, 0, pEmuImage[2], &rect, DDBLTFAST_WAIT);
	} else {
		if (nPreScaleEffect) {

			rect.right *= nPreScaleZoom;
			rect.bottom *= nPreScaleZoom;

			VidSoftFXApplyEffectDirectX(pEmuImage[0], &rect);
		} else {
			// Use the surface supplied by DirectX texture management and let it perform the blit
			if (FAILED(pEmuImage[0]->Lock(&rect, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_WAIT, NULL))) {
				return 1;
			}

			ps = pVidImage + nVidImageLeft * nVidImageBPP;
			pd = (unsigned char*)ddsd.lpSurface;
			nPitch = ddsd.lPitch;

			for (int y = 0; y < nGameImageHeight; y++, pd += nPitch, ps += nVidImagePitch) {
				memcpy(pd, ps, nSize);
			}

			pEmuImage[0]->Unlock(NULL);
		}
	}

	return 0;
}

// Run one frame and render the screen
static int vidFrame(bool bRedraw)			// bRedraw = 0
{
	// Check if we've lost control of the video hardware
	if (FAILED(pPrimarySurf->IsLost()) || FAILED(pDD->TestCooperativeLevel())) {
		bLostControl = true;
	}

	// If we've lost control, see if we've regained it and attempt to restore surfaces
	if (bLostControl) {
		if (FAILED(pDD->TestCooperativeLevel())) {
			return 1;
		}

		if (nVidFullscreen) {
			pDD->Compact();
		}

		if (FAILED(pPrimarySurf->IsLost())) {
			if (FAILED(pPrimarySurf->Restore())) {
				return 1;
			}
		}

		if (!bUsePageflip) {
			if (FAILED(pBackbuffer->IsLost())) {
				if (FAILED(pBackbuffer->Restore())) {
					return 1;
				}
			}
		}

		for (int i = 0; i < 4; i++) {

			// Skip texture 0 if we're using DirectX texture management
			if (nVidTransferMethod > 0 && i == 0) {
				continue;
			}

			if (pEmuImage[i]) {
				if (FAILED(pEmuImage[i]->IsLost())) {
					if (FAILED(pEmuImage[i]->Restore())) {
						return 1;
					}
				}
			}
		}

		if (InitEffectsSurfaces()) {		// Restore RGB effects and/or scanline surfaces
			return 1;
		}

		if (VidSRestoreOSD()) {
			return 1;
		}

		vidClear();							// Clear screen

		bLostControl = false;
	}

#ifdef ENABLE_PROFILING
	ProfileProfileStart(0);
#else
	vidRenderImageA();
#endif

	if (bDrvOkay) {
		if (bRedraw) {						// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();				// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();					// Run one frame and draw the screen
		}
	}
#ifdef ENABLE_PROFILING
	ProfileProfileEnd(0);

	ProfileProfileStart(1);
#endif

	vidBurnToSurf();						// Copy the memory buffer

#ifdef ENABLE_PROFILING
	ProfileProfileStart(2);
	vidRenderImageA();
#endif

	vidRenderImageB();						// Use 3D hardware to render the image

#ifdef ENABLE_PROFILING
	{
		// Force the D3D pipeline to be flushed

		DDSURFACEDESC2 ddsd;

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		if (SUCCEEDED(pBackbuffer->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))) {
			unsigned char c  = *((unsigned char*)ddsd.lpSurface);

			pBackbuffer->Unlock(NULL);
		}
	}

	ProfileProfileEnd(2);
	ProfileProfileEnd(1);

	dprintf(_T("burn %.2lfms; blit %.2lf; effect %.2lf\n"), ProfileProfileReadAverage(0), ProfileProfileReadAverage(1), ProfileProfileReadAverage(2));
#endif

	return 0;
}

// Paint the Dtos surface onto the primary surface
int vidPaint(int bValidate)
{
	RECT rect;

	if (bLostControl) {						// We've lost control of the video hardware
		return 1;
	}

	if (FAILED(pPrimarySurf->IsLost())) {	// We've lost control of the video hardware
		return 1;
	}

	if (!bUsePageflip) {
		GetClientScreenRect(hVidWnd, &rect);
		rect.top += 0 /*nMenuHeight*/;

		vidScale(&rect, nGameWidth, nGameHeight);

		if ((rect.right - rect.left) != (Dest.right - Dest.left) || (rect.bottom - rect.top ) != (Dest.bottom - Dest.top)) {
			bValidate |= 2;
		}
	}

	if (bValidate & 2) {
		vidRenderImageA();
		vidRenderImageB();
	}

	// Display OSD text message
	VidSDisplayOSD(pBackbuffer, &Render, 0);

	if(bVidVSync && !nVidFullscreen) { pDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL); }

	// Display final image
	if (bUsePageflip) {
		if (bUseTriplebuffer) {
			if (FAILED(pPrimarySurf->Flip(NULL, DDFLIP_WAIT))) {
				return 1;
			}
		} else {
			if (FAILED(pPrimarySurf->Flip(NULL, DDFLIP_NOVSYNC | DDFLIP_WAIT))) {
				return 1;
			}
		}
	} else {

		if (nVidFullscreen) {
			if (FAILED(pPrimarySurf->BltFast(Dest.left, Dest.top, pBackbuffer, &Dest, DDBLTFAST_WAIT))) {
				return 1;
			}
		} else {
			RECT RGBDest = {0, 0, Dest.right - Dest.left, Dest.bottom - Dest.top};

			if (FAILED(pPrimarySurf->Blt(&rect, pBackbuffer, &RGBDest, DDBLT_ASYNC, NULL))) {
				if (FAILED(pPrimarySurf->Blt(&rect, pBackbuffer, &RGBDest, DDBLT_WAIT, NULL))) {
					return 1;
				}
			}
			//if(bVidVSync && !nVidFullscreen) { pDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL); }
			
		}

		if (bValidate & 1) {
			// Validate the rectangle we just drew
			POINT c = {0, 0};
			ClientToScreen(hVidWnd, &c);
			rect.left -= c.x; rect.right -= c.x;
			rect.top -= c.y; rect.bottom -= c.y;
			ValidateRect(hVidWnd, &rect);
		}
	}

	return 0;
}

static int vidGetSettings(InterfaceInfo* pInfo)
{
	if (nVidFullscreen) {
		if (bUseTriplebuffer) {
			IntInfoAddStringModule(pInfo, _T("Using a triple buffer"));
		} else {
			if (bUsePageflip) {
				IntInfoAddStringModule(pInfo, _T("Using a double buffer"));
			} else {
				IntInfoAddStringModule(pInfo, _T("Using Bltfast() to transfer the image"));
			}
		}
	} else {
		IntInfoAddStringModule(pInfo, _T("Using Blt() to transfer the image"));
	}

	if (nPreScale) {
		TCHAR szString[MAX_PATH] = _T("");

		if (nPreScaleEffect) {
			_sntprintf(szString, MAX_PATH, _T("Prescaling using %s (%ix zoom)"), VidSoftFXGetEffect(nPreScaleEffect), nPreScaleZoom);
		} else {
			_sntprintf(szString, MAX_PATH, _T("Prescaling using 3D hardware (%ix zoom)"), nPreScaleZoom);
		}
		IntInfoAddStringModule(pInfo, szString);
	}

	if (bUse3DProjection || bUseRGBEffects || bVidScanDelay || bVidScanlines) {
		TCHAR* pszEffect[8] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
		TCHAR szString[MAX_PATH] = _T("");

		if (bUse3DProjection) {
			pszEffect[0] = _T("3D projection");
		}
		if (bUseRGBEffects) {
			pszEffect[0] = _T("RGB effects");
		}

		if (bVidScanDelay || bVidScanlines) {
			pszEffect[1] = _T(",");
			if (bVidScanDelay && bVidScanlines) {
				pszEffect[2] = _T(" feedback");
				pszEffect[3] = _T(",");
				pszEffect[4] = _T(" scanlines");
			} else {
				if (bVidScanDelay) {
					pszEffect[2] = _T(" feedback");
				}
				if (bVidScanlines) {
					pszEffect[2] = _T(" scanlines");
				}
			}
		}

		_sntprintf(szString, MAX_PATH, _T("Applying %s%s%s%s%s%s%s%s"), pszEffect[0], pszEffect[1], pszEffect[2], pszEffect[3], pszEffect[4], pszEffect[5], pszEffect[6], pszEffect[7]);
		IntInfoAddStringModule(pInfo, szString);
	}

	if (nVidTransferMethod > 0) {
		TCHAR szString[MAX_PATH] = _T("");

		_sntprintf(szString, MAX_PATH, _T("Using Direct3D texture management"), VidSoftFXGetEffect(nPreScaleEffect), nPreScaleZoom);
		IntInfoAddStringModule(pInfo, szString);
	}

	return 0;
}

// The video output plugin:
struct VidOut VidOutD3D = { vidInit, vidExit, vidFrame, vidPaint, vidScale, vidGetSettings, _T("DirectDraw7 / Direct3D7 Enhanced video output") };
#else
struct VidOut VidOutD3D = { NULL, NULL, NULL, NULL, NULL, NULL, _T("DirectDraw7 / Direct3D7 Enhanced video output") };
#endif

