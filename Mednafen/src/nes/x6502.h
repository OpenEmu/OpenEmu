#ifndef _X6502H

#include "x6502struct.h"

#ifdef WANT_DEBUGGER
void X6502_Debug(void (*CPUHook)(uint32),
                uint8 (*ReadHook)(X6502 *, unsigned int),
                void (*WriteHook)(X6502 *, unsigned int, uint8));

extern void (*X6502_Run)(int32 cycles);
#else
void X6502_Run(int32 cycles);
#endif

extern uint32 timestamp;
extern X6502 X;

#define N_FLAG  0x80
#define V_FLAG  0x40
#define U_FLAG  0x20
#define B_FLAG  0x10
#define D_FLAG  0x08
#define I_FLAG  0x04
#define Z_FLAG  0x02
#define C_FLAG  0x01

extern void (*MapIRQHook)(int a);

#define NTSC_CPU 1789772.7272727272727272
#define PAL_CPU  1662607.125

#define MDFN_IQEXT      0x001
#define MDFN_IQEXT2     0x002
/* ... */
#define MDFN_IQRESET    0x020
#define MDFN_IQNMI2	0x040	// Delayed NMI, gets converted to *_IQNMI
#define MDFN_IQNMI	0x080
#define MDFN_IQDPCM     0x100
#define MDFN_IQFCOUNT   0x200
#define MDFN_IQTEMP     0x800

void X6502_Init(void);
void X6502_Reset(void);
void X6502_Power(void);

void TriggerNMI(void);
void TriggerNMI2(void);

uint8 X6502_DMR(uint32 A);
void X6502_DMW(uint32 A, uint8 V);

void X6502_IRQBegin(int w);
void X6502_IRQEnd(int w);

int X6502_StateAction(StateMem *sm, int load, int data_only);

#define _X6502H
#endif
