// blitter effects via PSP GE
#include "burner.h"
#include "vid_support.h"
#include "vid_psp.h"

static int clipx		= 0;
static int clipy		= 0;
static int sizex		=0;
static int sizey		=0;

static int VidMemLen	= 0;
static int VidMemPitch	= 0;
static int VidBpp		= 0;
static unsigned char* VidMem = NULL;


static int bRotateGame=0;
static int bFlipGame=0;

static unsigned int HighCol16(int r, int g, int b, int /* i */)
{
	unsigned int t;

	t  = (b << 8) & 0xf800; // rrrr r000 0000 0000
	t |= (g << 3) & 0x07e0; // 0000 0ggg ggg0 0000
	t |= (r >> 3) & 0x001f; // 0000 0000 000b bbbb
	return t;
}


static int Exit()
{
	free(VidMem);
	return 0;
}

static int Init()
{
	bRotateGame = (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL);
	bFlipGame = BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED;

	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&clipy, &clipx);
		BurnDrvGetFullSize(&sizex, &sizey);
	} else {
		BurnDrvGetVisibleSize(&clipx, &clipy);
		BurnDrvGetFullSize(&sizex, &sizey);
	}
   	VidBpp		= nBurnBpp = 2;
	VidMemPitch	= sizex * VidBpp;
	VidMemLen	= sizey * VidMemPitch;
	VidMem = (unsigned char*)malloc(VidMemLen);
	SetBurnHighCol(16);
	bVidScanlines = 0;								// !!!
	nVidFullscreen=0;
	vidgu_init();
	return 0;
}

static int vidScale(RECT* , int, int)
{
	return 0;
}

// Run one frame and render the screen
static int Frame(bool bRedraw)						// bRedraw = 0
{
	nBurnBpp=2;

	nBurnPitch=VidMemPitch;
	pBurnDraw=VidMem;

	if (bDrvOkay) {
		BurnDrvFrame();							// Run one frame and draw the screen
	}

	unsigned char* ps = VidMem;
	unsigned char* pd = (unsigned char*)g_pBlitBuff;
	int p = 512*2;
	int s = sizex * nBurnBpp;

	for (int y = 0; y < sizey; y++, pd += p, ps += nBurnPitch) 
	{
		memcpy(pd, ps, s);
	}

	pBurnDraw = NULL;
	nBurnPitch = 0;
	return 0;
}

// Paint the BlitFX surface onto the primary surface
static int Paint(int bValidate)
{
	vidgu_render(0,0,sizex,sizey,0,0,480,272);
	return 0;
}

static int GetSettings(InterfaceInfo* pInfo)
{
	return 0;
}

// The Video Output plugin:
struct VidOut VidOutPSPFX = { Init, Exit, Frame, Paint, vidScale, GetSettings, _T("PSP video output") };
