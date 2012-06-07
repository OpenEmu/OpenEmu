#ifndef __V30MZ_H_
#define __V30MZ_H_

namespace MDFN_IEN_WSWAN
{

enum {
	NEC_PC=1, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_DS1, NEC_PS, NEC_SS, NEC_DS0
};

/* Public variables */
extern int v30mz_ICount;
extern uint32 v30mz_timestamp;


/* Public functions */
void v30mz_execute(int cycles);
void v30mz_set_reg(int, unsigned);
unsigned v30mz_get_reg(int regnum);
void v30mz_reset(void);
void v30mz_init(uint8 (*readmem20)(uint32), void (*writemem20)(uint32,uint8), uint8 (*readport)(uint32), void (*writeport)(uint32, uint8));

void v30mz_int(uint32 vector, bool IgnoreIF = FALSE);

int v30mz_StateAction(StateMem *sm, int load, int data_only);


#ifdef WANT_DEBUGGER
void v30mz_debug(void (*CPUHook)(uint32), uint8 (*ReadHook)(uint32), void (*WriteHook)(uint32, uint8), uint8 (*PortReadHook)(uint32), void (*PortWriteHook)(uint32, uint8),
			void (*BranchTraceHook)(uint16 from_CS, uint16 from_IP, uint16 to_CS, uint16 to_IP, bool interrupt) );
#endif
#endif

}
