#ifndef __MDFN_DRIVERS_GFXDEBUGGER_H
#define __MDFN_DRIVERS_GFXDEBUGGER_H

void GfxDebugger_Draw(SDL_Surface *surface, const SDL_Rect *rect, const SDL_Rect *screen_rect);
int GfxDebugger_Event(const SDL_Event *event);
void GfxDebugger_SetActive(bool newia);

#endif
