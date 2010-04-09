#ifndef __MDFN_STATE_H
#define __MDFN_STATE_H

typedef struct
{
 int status[10];
 int current;
 int current_movie; // For movies only, status(recording/playback)

 // The most recently-saved-to slot
 int recently_saved;

 uint32 *gfx;
 uint32 w, h, pitch;
} StateStatusStruct;

void MDFNI_SelectState(int);

/* "fname" overrides the default save state filename code if non-NULL. */
/* If suffix is set, just override the default suffix(nc0-nc9) */
void MDFNI_SaveState(const char *fname, const char *suffix, uint32 *fb, MDFN_Rect *LineWidths);
void MDFNI_LoadState(const char *fname, const char *suffix);
void MDFNI_EnableStateRewind(int enable);


void MDFND_SetStateStatus(StateStatusStruct *status);
void MDFND_SetMovieStatus(StateStatusStruct *status);

#endif
