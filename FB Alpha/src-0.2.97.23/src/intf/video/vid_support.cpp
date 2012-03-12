// Support functions for all blitters
#include "burner.h"
#include "vid_support.h"

// ---------------------------------------------------------------------------
// General

static UINT8* pVidSFullImage = NULL;

void VidSFreeVidImage()
{
	if (pVidSFullImage) {
		free(pVidSFullImage);
		pVidSFullImage = NULL;
	}
	pVidImage = NULL;
}

INT32 VidSAllocVidImage()
{
	INT32 nMemLen = 0;

	VidSFreeVidImage();

	// Allocate an extra line above and below the image to accomodate effects
	nVidImagePitch = nVidImageWidth * ((nVidImageDepth + 7) >> 3);
	nMemLen = (nVidImageHeight + 2) * nVidImagePitch;
	pVidSFullImage = (UINT8*)malloc(nMemLen);

	if (pVidSFullImage) {
		memset(pVidSFullImage, 0, nMemLen);
		pVidImage = pVidSFullImage + nVidImagePitch;
		return 0;
	} else {
		pVidImage = NULL;
		return 1;
	}
}

