#ifndef __MDFN_DRIVERS_FPS_H
#define __MDFN_DRIVERS_FPS_H

void FPS_IncVirtual(void);
void FPS_IncDrawn(void);
void FPS_IncBlitted(void);
void FPS_Init(void);
void FPS_Draw(MDFN_Surface *target, const int xpos, const int ypos);
void FPS_DrawToScreen(SDL_Surface *screen, int rs, int gs, int bs, int as);
bool FPS_IsActive(int *w, int *h);
void FPS_ToggleView(void);

#endif
