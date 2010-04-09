#ifndef __MDFN_DRIVERS_MEMDEBUGGER_H
#define __MDFN_DRIVERS_MEMDEBUGGER_H

void MemDebugger_Draw(SDL_Surface *surface, const SDL_Rect *rect, const SDL_Rect *screen_rect);
int MemDebugger_Event(const SDL_Event *event);
void MemDebugger_SetActive(bool newia);

bool MemDebugger_Init(void);

#endif
