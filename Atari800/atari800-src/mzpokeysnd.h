#ifndef MZPOKEYSND_H_
#define MZPOKEYSND_H_

#include "config.h"
#include "pokeysnd.h"
#include "atari.h"

int MZPOKEYSND_Init(ULONG freq17,
                        int playback_freq,
                        UBYTE num_pokeys,
                        int flags,
                        int quality
#ifdef __PLUS
                        , int clear_regs
#endif
                       );

#ifdef SYNCHRONIZED_SOUND
#endif /* SYNCHRONIZED_SOUND */
int MZPOKEYSND_UpdateProcessBuffer(void);
extern UBYTE *MZPOKEYSND_process_buffer;
#endif /* MZPOKEYSND_H_ */
