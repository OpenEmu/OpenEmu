#ifndef __MDFN_DRIVERS_VIDEO_H
#define __MDFN_DRIVERS_VIDEO_H

enum
{
 VDRIVER_OPENGL = 0,
 VDRIVER_SOFTSDL = 1,
 VDRIVER_OVERLAY = 2
};


void PtoV(int *x, int *y);
int InitVideo(MDFNGI *gi);
void KillVideo(void);
void BlitScreen(MDFN_Surface *, const MDFN_Rect *DisplayRect, const MDFN_Rect *LineWidths);
void ToggleFS();
void ClearVideoSurfaces(void);

#define NTVB_HQ2X       1
#define NTVB_HQ3X       2
#define NTVB_HQ4X	3

#define NTVB_SCALE2X    4
#define NTVB_SCALE3X    5
#define NTVB_SCALE4X	6

#define NTVB_NN2X	7
#define NTVB_NN3X	8
#define NTVB_NN4X	9

#define NTVB_NNY2X       10
#define NTVB_NNY3X       11
#define NTVB_NNY4X       12

#define NTVB_2XSAI       13
#define NTVB_SUPER2XSAI  14
#define NTVB_SUPEREAGLE  15

int VideoResize(int nw, int nh);

void VideoShowMessage(UTF8 *text);

void BlitRaw(SDL_Surface *src, const SDL_Rect *src_rect, const SDL_Rect *dest_rect);

// Called from the main thread
bool Video_Init(MDFNGI *gi);

// Called from the main thread
void Video_Kill(void);

// Called from the game thread
void Video_GetInternalBB(uint32 **XBuf, MDFN_Rect **LineWidths);

// Called from the game thread
void Video_SetInternalBBReady(const MDFN_Rect &DisplayRect);

// Called from the main thread.
bool Video_ScreenBlitReady(void);

// Called from the main thread.
void Video_BlitToScreen(void);

#endif
