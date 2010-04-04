#ifndef __MDFN_DRIVERS_VIDEO_H
#define __MDFN_DRIVERS_VIDEO_H

void PtoV(int *x, int *y);
int InitVideo(MDFNGI *gi);
int VideoCanBlit(void);
void KillVideo(void);
void BlitScreen(uint32 *XBuf, MDFN_Rect *DisplayRect, MDFN_Rect *LineWidths);
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

bool MDFND_ValidateVideoSetting(const char *name, const char *value);
bool MDFND_ValidateSpecialScalerSetting(const char *name, const char *value);
int VideoResize(int nw, int nh);

void VideoShowMessage(UTF8 *text);

void BlitRaw(SDL_Surface *src, SDL_Rect *src_rect, SDL_Rect *dest_rect);

#endif
