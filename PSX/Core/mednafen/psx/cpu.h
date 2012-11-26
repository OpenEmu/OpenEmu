#ifndef __MDFN_PSX_CPU_H
#define __MDFN_PSX_CPU_H

#include "gte.h"

namespace MDFN_IEN_PSX
{

class PS_CPU
{
 public:

 PS_CPU();
 ~PS_CPU();

 // FAST_MAP_* enums are in BYTES(8-bit), not in 32-bit units("words" in MIPS context), but the sizes
 // will always be multiples of 4.
 enum { FAST_MAP_SHIFT = 16 };
 enum { FAST_MAP_PSIZE = 1 << FAST_MAP_SHIFT };

 void SetFastMap(void *region_mem, uint32 region_address, uint32 region_size);

 INLINE void SetEventNT(const pscpu_timestamp_t next_event_ts_arg)
 {
  next_event_ts = next_event_ts_arg;
 }

 pscpu_timestamp_t Run(pscpu_timestamp_t timestamp_in, bool ILHMode);

 void Power(void);

 // which ranges 0-5, inclusive
 void AssertIRQ(int which, bool asserted);

 void SetHalt(bool status);

 int StateAction(StateMem *sm, int load, int data_only);

 private:

 struct
 {
  uint32 GPR[32];
  uint32 GPR_dummy;	// Used in load delay simulation(indexing past the end of GPR)
 };
 uint32 LO;
 uint32 HI;


 uint32 BACKED_PC;
 uint32 BACKED_new_PC;
 uint32 BACKED_new_PC_mask;

 uint32 IPCache;
 void RecalcIPCache(void);
 bool Halted;

 uint32 BACKED_LDWhich;
 uint32 BACKED_LDValue;

 pscpu_timestamp_t next_event_ts;
 pscpu_timestamp_t gte_ts_done;

 uint8 *FastMap[1 << (32 - FAST_MAP_SHIFT)];
 uint8 DummyPage[FAST_MAP_PSIZE];

 enum
 {
  CP0REG_BPC = 3,		// PC breakpoint address.
  CP0REG_BDA = 5,		// Data load/store breakpoint address.
  CP0REG_TAR = 6,		// Target address(???)
  CP0REG_DCIC = 7,		// Cache control
  CP0REG_BDAM = 9,		// Data load/store address mask.
  CP0REG_BPCM = 11,		// PC breakpoint address mask.
  CP0REG_SR = 12,
  CP0REG_CAUSE = 13,
  CP0REG_EPC = 14,
  CP0REG_PRID = 15,		// Product ID
  CP0REG_ERREG = 16
 };

 struct
 {
  union
  {
   uint32 Regs[32];
   struct
   {
    uint32 Unused00;
    uint32 Unused01;
    uint32 Unused02;
    uint32 BPC;		// RW
    uint32 Unused04;
    uint32 BDA;		// RW
    uint32 TAR;
    uint32 DCIC;	// RW
    uint32 Unused08;	
    uint32 BDAM;	// R/W
    uint32 Unused0A;
    uint32 BPCM;	// R/W
    uint32 SR;		// R/W
    uint32 CAUSE;	// R/W(partial)
    uint32 EPC;		// R
    uint32 PRID;	// R
    uint32 ERREG;	// ?(may not exist, test)
   };
  };
 } CP0;

 //PS_GTE GTE;

 enum
 {
  EXCEPTION_INT = 0,
  EXCEPTION_MOD = 1,
  EXCEPTION_TLBL = 2,
  EXCEPTION_TLBS = 3,
  EXCEPTION_ADEL = 4, // Address error on load
  EXCEPTION_ADES = 5, // Address error on store
  EXCEPTION_IBE = 6, // Instruction bus error
  EXCEPTION_DBE = 7, // Data bus error
  EXCEPTION_SYSCALL = 8, // System call
  EXCEPTION_BP = 9, // Breakpoint
  EXCEPTION_RI = 10, // Reserved instruction
  EXCEPTION_COPU = 11,  // Coprocessor unusable
  EXCEPTION_OV = 12	// Arithmetic overflow
 };

 uint32 Exception(uint32 code, uint32 PC, const uint32 NPM) MDFN_WARN_UNUSED_RESULT;

 template<bool DebugMode, bool ILHMode> pscpu_timestamp_t RunReal(pscpu_timestamp_t timestamp_in);

 template<typename T> T ReadMemory(pscpu_timestamp_t &timestamp, uint32 address, bool DS24 = false);
 template<typename T> void WriteMemory(pscpu_timestamp_t &timestamp, uint32 address, uint32 value, bool DS24 = false);


 //
 // Mednafen debugger stuff follows:
 //
 public:
 void SetCPUHook(void (*cpuh)(uint32 pc), void (*addbt)(uint32 from, uint32 to, bool exception));
 void CheckBreakpoints(void (*callback)(bool write, uint32 address, unsigned int len), uint32 instr);

 enum
 {
  GSREG_GPR = 0,
  GSREG_PC = 32,
  GSREG_PC_NEXT,
  GSREG_IN_BD_SLOT,
  GSREG_LO,
  GSREG_HI,
  GSREG_SR,
  GSREG_CAUSE,
  GSREG_EPC,
 };

 uint32 GetRegister(unsigned int which, char *special, const uint32 special_len);
 void SetRegister(unsigned int which, uint32 value);

 private:
 void (*CPUHook)(uint32 pc);
 void (*ADDBT)(uint32 from, uint32 to, bool exception);
};

}

#endif
