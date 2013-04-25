#ifndef VOTRAX_H_
#define VOTRAX_H_

#include "atari.h"

typedef void (*Votrax_BusyCallBack)(int);

struct Votrax_interface
{
        int num;	/* total number of chips */
	Votrax_BusyCallBack BusyCallback;	/* callback function when busy signal changes */
};

int Votrax_Start(void *sound_interface);
void Votrax_Stop(void);

void Votrax_PutByte(UBYTE data);
UBYTE Votrax_GetStatus(void);

void Votrax_Update(int num, SWORD *buffer, int length);
int Votrax_Samples(int currentP, int nextP, int cursamples);

#endif /* VOTRAX_H_ */
