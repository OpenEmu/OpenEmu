#ifndef VOTRAXSND_H_
#define VOTRAXSND_H_

#include "atari.h"
#include "votrax.h"

void VOTRAXSND_PutByte(UBYTE byte);
void VOTRAXSND_Init(int playback_freq, int n_pokeys, int b16);
void VOTRAXSND_Frame(void);
void VOTRAXSND_Process(void *sndbuffer, int sndn);
extern int VOTRAXSND_busy;
void VOTRAXSND_Reinit(void);
void VOTRAXSND_ModifyRatio(double factor);

#endif /* VOTRAXSND_H_ */
