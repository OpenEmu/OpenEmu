#ifndef _H_YM2413_
#define _H_YM2413_

/* select output bits size of output : 8 or 16 */
#define SAMPLE_BITS 16

#if (SAMPLE_BITS==16)
typedef INT16 SAMP;
#endif
#if (SAMPLE_BITS==8)
typedef INT8 SAMP;
#endif




int  YM2413Init(int num, int clock, int rate);
void YM2413Shutdown(void);
void YM2413ResetChip(int which);
void YM2413Write(int which, int a, int v);
void YM2413WriteReg(int which, int r, int v);
unsigned char YM2413Read(int which, int a);
void YM2413UpdateOne(int which, INT16 **buffers, int length);

typedef void (*OPLL_UPDATEHANDLER)(int param,int min_interval_us);

void YM2413SetUpdateHandler(int which, OPLL_UPDATEHANDLER UpdateHandler, int param);

void YM2413Scan(INT32 which, INT32 nAction);

#endif /*_H_YM2413_*/
