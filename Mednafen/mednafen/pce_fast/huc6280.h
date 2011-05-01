#ifndef _HuC6280H

#define HUC6280_CRAZY_VERSION
//#define HUC6280_EXTRA_CRAZY

#define HUC6280_LAZY_FLAGS

namespace PCE_Fast
{


typedef struct __HuC6280
{
	#ifdef HUC6280_CRAZY_VERSION
        uint8 *PC, *PC_base;
	#else
	uint16 PC;
	#endif

        uint8 A,X,Y,S,P,mooPI;
	#ifdef HUC6280_LAZY_FLAGS
 	 uint32 ZNFlags;
	#endif
	uint8 MPR[9];		// 8, + 1 for PC overflow from $ffff to $10000
	uint8 *FastPageR[9];
	uint8 *Page1;
	//uint8 *PAGE1_W;
	//const uint8 *PAGE1_R;

        uint32 IRQlow;          /* Simulated IRQ pin held low(or is it high?).
                                   And other junk hooked on for speed reasons.*/
	int32 timestamp;

	uint8 IRQMask, IRQMaskDelay;
	uint8 timer_status;
	int32 timer_value, timer_load;
        int32 timer_next_timestamp;

	uint32 in_block_move;
	uint16 bmt_src, bmt_dest, bmt_length;
	uint32 bmt_alternate;
	#define IBM_TIA	1
	#define IBM_TAI	2
	#define IBM_TDD 3
	#define IBM_TII 4
	#define IBM_TIN 5

	int32 previous_next_user_event;
} HuC6280;

void HuC6280_Run(int32 cycles);
void HuC6280_ResetTS(void);

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

int HuC6280_StateAction(StateMem *sm, int load, int data_only);

static INLINE void HuC6280_StealCycle(void)
{
 HuCPU.timestamp++;
}

static INLINE uint8 HuC6280_TimerRead(unsigned int A)
{
 #if 0
 return(HuCPU.timer_value | (PCEIODataBuffer & 0x80));
 #endif

 uint8 tvr = HuCPU.timer_value;

 if(HuCPU.timer_next_timestamp == HuCPU.timestamp)
  tvr = (tvr - 1) & 0x7F;

 return(tvr | (PCEIODataBuffer & 0x80));
}

static INLINE void HuC6280_TimerWrite(unsigned int A, uint8 V)
{
 switch(A & 1)
 {
  case 0: HuCPU.timer_load = (V & 0x7F); break;
  case 1: if(V & 1) // Enable counter
          {
                if(HuCPU.timer_status == 0)
                {
                        HuCPU.timer_next_timestamp = HuCPU.timestamp + 1024;
                        HuCPU.timer_value = HuCPU.timer_load;
                }
          }
          HuCPU.timer_status = V & 1;
          break;
 }
}

static INLINE uint8 HuC6280_IRQStatusRead(unsigned int A)
{
 if(!(A & 2))
  return(PCEIODataBuffer);

 switch(A & 1)
 {
  case 0:
         HuC6280_IRQEnd(MDFN_IQTIMER);
         return(HuCPU.IRQMask ^ 0x7);
  case 1:
        {
         int status = 0;
         if(HuCPU.IRQlow & MDFN_IQIRQ1) status |= 2;
         if(HuCPU.IRQlow & MDFN_IQIRQ2) status |= 1;
         if(HuCPU.IRQlow & MDFN_IQTIMER) status |= 4;
         return(status | (PCEIODataBuffer & ~(1 | 2 | 4)));
        }
 }
 return(PCEIODataBuffer);
}

static INLINE void HuC6280_IRQStatusWrite(unsigned int A, uint8 V)
{
 if(!(A & 2)) return;
 switch(A & 1)
 {
  case 0: HuCPU.IRQMask = (V & 0x7) ^ 0x7; break;
  case 1: HuC6280_IRQEnd(MDFN_IQTIMER); break;
 }
}


};

#define _HuC6280H
#endif
