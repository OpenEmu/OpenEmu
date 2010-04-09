#ifndef __MDFN_DRIVERS_VIDEO_STATE_H
#define __MDFN_DRIVERS_VIDEO_STATE_H

void DrawSaveStates(SDL_Surface *screen, double, double, int, int, int, int);

void MT_SetStateStatus(StateStatusStruct *status);
void MT_SetMovieStatus(StateStatusStruct *status);

#endif
