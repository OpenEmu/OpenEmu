#ifndef _MOVIE_H
#define _MOVIE_H

void MDFNI_SelectMovie(int);

#include "movie-driver.h"
#include "state.h"
void MDFNMOV_AddJoy(void *donutdata, uint32 donutlen);
void MDFNMOV_CheckMovies(void);
void MDFNMOV_Stop(void);
void MDFNMOV_AddCommand(int cmd);
bool MDFNMOV_IsPlaying(void);
bool MDFNMOV_IsRecording(void);
void MDFNMOV_ForceRecord(StateMem *sm);
StateMem MDFNMOV_GrabRewindJoy(void);
void MDFNMOV_RecordState();
#endif
