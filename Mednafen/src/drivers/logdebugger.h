#ifndef __MDFN_DRIVERS_LOGDEBUGGER_H
#define __MDFN_DRIVERS_LOGDEBUGGER_H

void LogDebugger_Draw(SDL_Surface *surface, const SDL_Rect *rect, const SDL_Rect *screen_rect);
int LogDebugger_Event(const SDL_Event *event);
void LogDebugger_SetActive(bool newia);

#endif

