#ifndef SOUND_H_
#define SOUND_H_

#include "config.h"

void Sound_Initialise(int *argc, char *argv[]);
void Sound_Exit(void);
void Sound_Update(void);
void Sound_Pause(void);
void Sound_Continue(void);
#ifdef SUPPORTS_SOUND_REINIT
void Sound_Reinit(void);
#endif

#endif /* SOUND_H_ */
