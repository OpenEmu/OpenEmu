#ifndef __MDFN_DRIVERS_HELP_H
#define __MDFN_DRIVERS_HELP_H

void Help_Draw(SDL_Surface *surface, const SDL_Rect *rect);
bool Help_IsActive(void);
bool Help_Toggle(void);
void Help_Init(void);
void Help_Close(void);

#endif
