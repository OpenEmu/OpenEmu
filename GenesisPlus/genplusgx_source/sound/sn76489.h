/* 
    SN76489 emulation
    by Maxim in 2001 and 2002
*/

#ifndef _SN76489_H_
#define _SN76489_H_

/* Function prototypes */

extern void SN76489_Init(double PSGClockValue, int SamplingRate);
extern void SN76489_Reset(void);
extern void SN76489_Shutdown(void);
extern void SN76489_SetContext(uint8 *data);
extern void SN76489_GetContext(uint8 *data);
extern uint8 *SN76489_GetContextPtr(void);
extern int SN76489_GetContextSize(void);
extern void SN76489_Write(int data);
extern void SN76489_Update(INT16 *buffer, int length);
extern void SN76489_BoostNoise(int boost);

#endif /* _SN76489_H_ */

