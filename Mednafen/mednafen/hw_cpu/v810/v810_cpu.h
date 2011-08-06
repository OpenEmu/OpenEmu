////////////////////////////////////////////////////////////////
// Defines for the V810 CPU

#ifndef V810_CPU_H_
#define V810_CPU_H_

#include <vector>

#include "fpu-new/softfloat.h"

typedef int32 v810_timestamp_t;

#define V810_FAST_MAP_SHIFT	16
#define V810_FAST_MAP_PSIZE     (1 << V810_FAST_MAP_SHIFT)
#define V810_FAST_MAP_TRAMPOLINE_SIZE	1024

// Exception codes
enum
{
 ECODE_TRAP_BASE =	0xFFA0,
 ECODE_INVALID_OP = 	0xFF90,
 ECODE_ZERO_DIV =	0xFF80, // Integer divide by 0
 ECODE_FIV =		0xFF70, // Floating point invalid operation
 ECODE_FZD =		0xFF68, // Floating point zero division
 ECODE_FOV = 		0xFF64, // Floating point overflow
//#define ECODE_FUD	0xFF62 // Floating point underflow(unused on V810)
//#define ECODE_FPR	0xFF61 // Floating point precision degradation(unused on V810)
 ECODE_FRO = 		0xFF60 // Floating point reserved operand
};

enum
{
 INVALID_OP_HANDLER_ADDR = 0xFFFFFF90, // Invalid opcode/instruction code!
 ZERO_DIV_HANDLER_ADDR = 0xFFFFFF80, // Integer divide by 0 exception
 FPU_HANDLER_ADDR = 0xFFFFFF60, // FPU exception
 TRAP_HANDLER_BASE = 0xFFFFFFA0 // TRAP instruction
};

//System Register Defines (these are the only valid system registers!)
#define EIPC     0       //Exeption/Interupt PC
#define EIPSW    1       //Exeption/Interupt PSW

#define FEPC     2       //Fatal Error PC
#define FEPSW    3       //Fatal Error PSW

#define ECR      4       //Exception Cause Register
#define PSW      5       //Program Status Word
#define PIR      6       //Processor ID Register
#define TKCW     7       //Task Controll Word
#define CHCW     24      //Cashe Controll Word
#define ADDTRE   25      //ADDTRE

//PSW Specifics
#define PSW_IA  0xF0000 // All Interupt bits...
#define PSW_I3  0x80000
#define PSW_I2  0x40000
#define PSW_I1  0x20000
#define PSW_I0  0x10000

#define PSW_NP  0x08000
#define PSW_EP  0x04000

#define PSW_AE	0x02000

#define PSW_ID  0x01000

#define PSW_FRO 0x00200 // Floating point reserved operand(set on denormal, NaN, or indefinite)
#define PSW_FIV 0x00100 // Floating point invalid operation(set when trying to convert a number too large to an (un)signed integer)

#define PSW_FZD 0x00080 // Floating point divide by zero
#define PSW_FOV 0x00040 // Floating point overflow
#define PSW_FUD 0x00020 // Floating point underflow
#define PSW_FPR	0x00010 // Floating point precision degradation

#define PSW_CY  0x00008
#define PSW_OV  0x00004
#define PSW_S   0x00002
#define PSW_Z   0x00001

//condition codes
#define COND_V  0
#define COND_C  1
#define COND_Z  2
#define COND_NH 3
#define COND_S  4
#define COND_T  5
#define COND_LT 6
#define COND_LE 7
#define COND_NV 8
#define COND_NC 9
#define COND_NZ 10
#define COND_H  11
#define COND_NS 12
#define COND_F  13
#define COND_GE 14
#define COND_GT 15

#define TESTCOND_V                      (S_REG[PSW]&PSW_OV)

#define TESTCOND_L                      (S_REG[PSW]&PSW_CY)
#define TESTCOND_C	TESTCOND_L

#define TESTCOND_E                      (S_REG[PSW]&PSW_Z)
#define TESTCOND_Z	TESTCOND_E

#define TESTCOND_NH             ( (S_REG[PSW]&PSW_Z) || (S_REG[PSW]&PSW_CY) )
#define TESTCOND_N                      (S_REG[PSW]&PSW_S)
#define TESTCOND_S	TESTCOND_N

#define TESTCOND_LT             ( (!!(S_REG[PSW]&PSW_S)) ^ (!!(S_REG[PSW]&PSW_OV)) )
#define TESTCOND_LE             ( ((!!(S_REG[PSW]&PSW_S)) ^ (!!(S_REG[PSW]&PSW_OV))) || (S_REG[PSW]&PSW_Z) )
#define TESTCOND_NV             (!(S_REG[PSW]&PSW_OV))

#define TESTCOND_NL             (!(S_REG[PSW]&PSW_CY))
#define TESTCOND_NC	TESTCOND_NL

#define TESTCOND_NE             (!(S_REG[PSW]&PSW_Z))
#define TESTCOND_NZ	TESTCOND_NE

#define TESTCOND_H                      ( !((S_REG[PSW]&PSW_Z) || (S_REG[PSW]&PSW_CY)) )
#define TESTCOND_P                      (!(S_REG[PSW] & PSW_S))
#define TESTCOND_NS	TESTCOND_P

#define TESTCOND_GE             (!((!!(S_REG[PSW]&PSW_S))^(!!(S_REG[PSW]&PSW_OV))))
#define TESTCOND_GT             (! (((!!(S_REG[PSW]&PSW_S))^(!!(S_REG[PSW]&PSW_OV))) || (S_REG[PSW]&PSW_Z)) )

// Tag layout
//  Bit 0-21: TAG31-TAG10
//  Bit 22-23: Validity bits(one for each 4-byte subblock)
//  Bit 24-27: NECRV("Reserved")
//  Bit 28-31: 0

typedef enum
{
 V810_EMU_MODE_FAST = 0,
 V810_EMU_MODE_ACCURATE = 1,
 _V810_EMU_MODE_COUNT
} V810_Emu_Mode;

//
// WARNING: Do NOT instantiate this class in multiple threads in such a way that both threads can be inside a method of this class at the same time.
// To fix this, you'll need to put locks or something(re-engineer it to use state passed in through pointers) around the SoftFloat code.
//


class V810
{
 public:

 V810();
 ~V810();

 // Pass TRUE for vb_mode if we're emulating a VB-specific enhanced V810 CPU core
 bool Init(V810_Emu_Mode mode, bool vb_mode);
 void Kill(void);

 void SetInt(int level);

 void SetMemWriteBus32(uint8 A, bool value);
 void SetMemReadBus32(uint8 A, bool value);

 void SetMemReadHandlers(uint8 MDFN_FASTCALL (*read8)(v810_timestamp_t &, uint32), uint16 MDFN_FASTCALL (*read16)(v810_timestamp_t &, uint32), uint32 MDFN_FASTCALL (*read32)(v810_timestamp_t &, uint32));
 void SetMemWriteHandlers(void MDFN_FASTCALL (*write8)(v810_timestamp_t &, uint32, uint8), void MDFN_FASTCALL (*write16)(v810_timestamp_t &, uint32, uint16), void MDFN_FASTCALL (*write32)(v810_timestamp_t &, uint32, uint32));

 void SetIOReadHandlers(uint8 MDFN_FASTCALL (*read8)(v810_timestamp_t &, uint32), uint16 MDFN_FASTCALL (*read16)(v810_timestamp_t &, uint32), uint32 MDFN_FASTCALL (*read32)(v810_timestamp_t &, uint32));
 void SetIOWriteHandlers(void MDFN_FASTCALL (*write8)(v810_timestamp_t &, uint32, uint8), void MDFN_FASTCALL (*write16)(v810_timestamp_t &, uint32, uint16), void MDFN_FASTCALL (*write32)(v810_timestamp_t &, uint32, uint32));

 // Length specifies the number of bytes to map in, at each location specified by addresses[] (for mirroring)
 uint8 *SetFastMap(uint32 addresses[], uint32 length, unsigned int num_addresses, const char *name);

 INLINE void ResetTS(void)
 {
  assert(next_event_ts > v810_timestamp);

  next_event_ts -= v810_timestamp;
  v810_timestamp = 0;
 }

 INLINE void SetEventNT(const v810_timestamp_t timestamp)
 {
  next_event_ts = timestamp;
 }

 INLINE v810_timestamp_t GetEventNT(void)
 {
  return(next_event_ts);
 }

 v810_timestamp_t Run(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp));
 void Exit(void);

 void Reset(void);

 int StateAction(StateMem *sm, int load, int data_only);

 #ifdef WANT_DEBUGGER
 void CheckBreakpoints(void (*callback)(int type, uint32 address, unsigned int len), uint16 MDFN_FASTCALL (*peek16)(const v810_timestamp_t, uint32), uint32 MDFN_FASTCALL (*peek32)(const v810_timestamp_t, uint32));
 void SetCPUHook(void (*newhook)(uint32 PC), void (*new_ADDBT)(uint32));
 #endif
 uint32 GetPC(void);
 void SetPC(uint32);

 uint32 GetPR(const unsigned int which);
 void SetPR(const unsigned int which, uint32 value);

 uint32 GetSR(const unsigned int which);
 void SetSR(const unsigned int which, uint32 value);


 private:

 // Make sure P_REG[] is the first variable/array in this class, so non-zerfo offset encoding(at assembly level) isn't necessary to access it.
 uint32 P_REG[32];  // Program registers pr0-pr31
 uint32 S_REG[32];  // System registers sr0-sr31
 uint32 PC;
 uint8 *PC_ptr;
 uint8 *PC_base;

 public:
 v810_timestamp_t v810_timestamp;	// Will never be less than 0.

 private:
 v810_timestamp_t next_event_ts;

 enum
 {
  LASTOP_NORMAL = 0,
  LASTOP_LOAD = 1,
  LASTOP_STORE = 2,
  LASTOP_IN = 3,
  LASTOP_OUT = 4,
  LASTOP_HEAVY_MATH = 5
 };

 V810_Emu_Mode EmuMode;
 bool VBMode;

 void Run_Fast(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp)) NO_INLINE;
 void Run_Accurate(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp)) NO_INLINE;

 #ifdef WANT_DEBUGGER
 void Run_Fast_Debug(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp)) NO_INLINE;
 void Run_Accurate_Debug(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp)) NO_INLINE;
 #endif

 uint8 MDFN_FASTCALL (*MemRead8)(v810_timestamp_t &timestamp, uint32 A);
 uint16 MDFN_FASTCALL (*MemRead16)(v810_timestamp_t &timestamp, uint32 A);
 uint32 MDFN_FASTCALL (*MemRead32)(v810_timestamp_t &timestamp, uint32 A);

 void MDFN_FASTCALL (*MemWrite8)(v810_timestamp_t &timestamp, uint32 A, uint8 V);
 void MDFN_FASTCALL (*MemWrite16)(v810_timestamp_t &timestamp, uint32 A, uint16 V);
 void MDFN_FASTCALL (*MemWrite32)(v810_timestamp_t &timestamp, uint32 A, uint32 V);

 uint8 MDFN_FASTCALL (*IORead8)(v810_timestamp_t &timestamp, uint32 A);
 uint16 MDFN_FASTCALL (*IORead16)(v810_timestamp_t &timestamp, uint32 A);
 uint32 MDFN_FASTCALL (*IORead32)(v810_timestamp_t &timestamp, uint32 A);

 void MDFN_FASTCALL (*IOWrite8)(v810_timestamp_t &timestamp, uint32 A, uint8 V);
 void MDFN_FASTCALL (*IOWrite16)(v810_timestamp_t &timestamp, uint32 A, uint16 V);
 void MDFN_FASTCALL (*IOWrite32)(v810_timestamp_t &timestamp, uint32 A, uint32 V);

 bool MemReadBus32[256];      // Corresponding to the upper 8 bits of the memory address map.
 bool MemWriteBus32[256];

 int32 lastop;    // Set to -1 on FP/MUL/DIV, 0x100 on LD, 0x200 on ST, 0x400 on in, 0x800 on out, and the actual opcode * 2(or >= 0) on everything else.

 #define LASTOP_LD       0x100
 #define LASTOP_ST       0x200
 #define LASTOP_IN       0x400
 #define LASTOP_OUT      0x800

 enum
 {
  HALT_NONE = 0,
  HALT_HALT = 1,
  HALT_FATAL_EXCEPTION = 2
 };

 uint8 Halted;

 bool Running;

 int ilevel;

 bool in_bstr;
 uint16 in_bstr_to;

 bool bstr_subop(v810_timestamp_t &timestamp, int sub_op, int arg1);
 void fpu_subop(v810_timestamp_t &timestamp, int sub_op, int arg1, int arg2);

 void Exception(uint32 handler, uint16 eCode);
 bool WillInterruptOccur(void);
 int Int(uint32 iNum);

 // Caching-related:
 typedef struct
 {
  uint32 tag;
  uint32 data[2];
  bool data_valid[2];
 } V810_CacheEntry_t;

 V810_CacheEntry_t Cache[128];

 // Bitstring variables.
 uint32 src_cache;
 uint32 dst_cache;
 bool have_src_cache, have_dst_cache;

 uint8 *FastMap[(1ULL << 32) / V810_FAST_MAP_PSIZE];
 std::vector<void *> FastMapAllocList;


 #ifdef WANT_DEBUGGER
 void (*CPUHook)(uint32 PC);
 void (*ADDBT)(uint32 PC);
 #endif


 // For CacheDump and CacheRestore
 void CacheOpMemStore(v810_timestamp_t &timestamp, uint32 A, uint32 V);
 uint32 CacheOpMemLoad(v810_timestamp_t &timestamp, uint32 A);

 void CacheClear(v810_timestamp_t &timestamp, uint32 start, uint32 count);
 void CacheDump(v810_timestamp_t &timestamp, const uint32 SA);
 void CacheRestore(v810_timestamp_t &timestamp, const uint32 SA);

 uint32 RDCACHE(v810_timestamp_t &timestamp, uint32 addr);
 //
 // End caching related
 //

 uint16 RDOP(v810_timestamp_t &timestamp, uint32 addr, uint32 meow = 2);
 void SetFlag(uint32 n, bool condition);
 void SetSZ(uint32 value);

 void SetSREG(v810_timestamp_t &timestamp, unsigned int which, uint32 value);
 uint32 GetSREG(unsigned int which);


 bool IsSubnormal(uint32 fpval);
 void FPU_Math_Template(float32 (*func)(float32, float32), uint32 arg1, uint32 arg2);
 void FPU_DoException(void);
 bool CheckFPInputException(uint32 fpval);
 bool FPU_DoesExceptionKillResult(void);
 void SetFPUOPNonFPUFlags(uint32 result);


 uint32 BSTR_RWORD(v810_timestamp_t &timestamp, uint32 A);
 void BSTR_WWORD(v810_timestamp_t &timestamp, uint32 A, uint32 V);
 bool Do_BSTR_Search(v810_timestamp_t &timestamp, const int inc_mul, unsigned int bit_test);


 uint8 DummyRegion[V810_FAST_MAP_PSIZE + V810_FAST_MAP_TRAMPOLINE_SIZE];
};

#endif

