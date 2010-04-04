#ifndef _HuC6280H

typedef struct __HuC6280
{
        int32 tcount;           /* Temporary cycle counter */
        uint32 PC;
        uint8 A,X,Y,S,P,mooPI;
	uint8 MPR[9];		// 8, + 1 for PC overflow from $ffff to $10000
	uint8 *FastPageR[9];

        int32 count;
        uint32 IRQlow;          /* Simulated IRQ pin held low(or is it high?).
                                   And other junk hooked on for speed reasons.*/
	uint8 speed;
	uint8 speed_shift_cache;
	uint32 timestamp;

	uint8 IRQMask, IRQMaskDelay;
	uint8 timer_status;
	int32 timer_value, timer_load;
	int32 timer_div;
	uint32 in_block_move;
	uint16 bmt_src, bmt_dest, bmt_length;
	uint32 bmt_alternate;
	bool isopread;
	#define IBM_TIA	1
	#define IBM_TAI	2
	#define IBM_TDD 3
	#define IBM_TII 4
	#define IBM_TIN 5

	bool preexec;
	bool cpoint;

	#ifdef WANT_DEBUGGER
	void (*CPUHook)(uint32, uint8);
	uint8 (*ReadHook)(struct __HuC6280 *, unsigned int);
	void (*WriteHook)(struct __HuC6280 *, uint32, uint8);
	#endif
} HuC6280;

#ifdef WANT_DEBUGGER
extern void (*HuC6280_Run)(int32 cycles);
void HuC6280_Debug(void (*CPUHook)(uint32, uint8),
                uint8 (*ReadHook)(HuC6280 *, unsigned int),
                void (*WriteHook)(HuC6280 *, uint32, uint8));
#else
void HuC6280_Run(int32 cycles);
#endif

extern HuC6280 HuCPU;
extern uint8 *HuCPUFastMap[0x100];

#define N_FLAG  0x80
#define V_FLAG  0x40
#define T_FLAG  0x20
#define B_FLAG  0x10
#define D_FLAG  0x08
#define I_FLAG  0x04
#define Z_FLAG  0x02
#define C_FLAG  0x01

#define NTSC_CPU 1789772.7272727272727272
#define PAL_CPU  1662607.125

#define MDFN_IQIRQ1	0x002
#define MDFN_IQIRQ2	0x001
#define MDFN_IQTIMER	0x004
#define MDFN_IQRESET    0x020

void HuC6280_Init(void);
void HuC6280_Reset(void);
void HuC6280_Power(void);

void HuC6280_IRQBegin(int w);
void HuC6280_IRQEnd(int w);

uint8 HuC6280_TimerRead(unsigned int A);
void HuC6280_TimerWrite(unsigned int A, uint8 V);

uint8 HuC6280_IRQStatusRead(unsigned int A);
void HuC6280_IRQStatusWrite(unsigned int A, uint8 V);

int HuC6280_StateAction(StateMem *sm, int load, int data_only);

void HuC6280_StealCycles(int count);


void HuC6280_DumpMem(char *filename, uint32 start, uint32 end); // For debugging


// Two functions used by HES code
void HuC6280_FlushMPRCache(void);
void HuC6280_SetMPR(int i, int v);

#define _HuC6280H
#endif
