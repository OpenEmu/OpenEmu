/* 
    SN76489 emulation
    by Maxim in 2001 and 2002
*/

#ifndef _SN76489_H_
#define _SN76489_H_

#include "blip_buf.h"

#define SN_DISCRETE    0
#define SN_INTEGRATED  1

/* Function prototypes */
extern void SN76489_Init(blip_t* left, blip_t* right, int type);
extern void SN76489_Reset(void);
extern void SN76489_Config(unsigned int clocks, int preAmp, int boostNoise, int stereo);
extern void SN76489_Write(unsigned int clocks, unsigned int data);
extern void SN76489_Update(unsigned int cycles);
extern void *SN76489_GetContextPtr(void);
extern int SN76489_GetContextSize(void);

#endif /* _SN76489_H_ */
