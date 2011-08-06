#ifndef __MDFN_DRIVERS_DEBUGGER_H
#define __MDFN_DRIVERS_DEBUGGER_H

#ifdef WANT_DEBUGGER
void Debugger_Draw(SDL_Surface *surface, SDL_Rect *rect, const SDL_Rect *screen_rect);
void Debugger_ModOpacity(int deltalove);

void Debugger_ForceStepIfStepping(); // For synchronizations with save state loading and reset/power toggles.  Should be called from game thread only.

bool Debugger_IsActive(unsigned int *w = NULL, unsigned int *h = NULL);
void Debugger_Event(const SDL_Event *event);
bool Debugger_Toggle(void);
void Debugger_ForceSteppingMode(void);

extern volatile bool InSteppingMode;
#else

#define Debugger_Event(a)
#define Debugger_Draw(a, b, c)
#define Debugger_ForceStepIfStepping()
//#define Debugger_IsActive(a, b)	0
#define Debugger_IsActive(...) 0
#define Debugger_Toggle()
#define Debugger_ForceSteppingMode()
#define InSteppingMode 0

#endif

#endif
