////////////////////////////////////////////////////////////////
// Defines for the V810 CPU

#ifndef V810_CPU_H_
#define V810_CPU_H_

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

///////////////////////////////////////////////////////////////////
// Define CPU Globals


extern uint32 P_REG[32];  // Program registers pr0-pr31
extern uint32 S_REG[32];  // System registers sr0-sr31
//extern uint32 PC;         // Program Counter
extern const uint8 opcycle[0x50]; //clock cycles

extern uint32 v810_timestamp;

//////////////////////////////////////////////////////////////////////////
// Define CPU Functions

// Reset the registers
void v810_reset();  
void v810_kill(void);

extern void (*v810_run)(int32);

#ifdef WANT_DEBUGGER
void v810_setCPUHook(void (*newhook)(uint32 PC));
#endif

uint32 v810_getPC(void);
void v810_setPC(uint32);
void v810_setint(int level);
int v810_int(uint32 iNum);


#define V810_EVENT_PAD		1
#define V810_EVENT_TIMER	2
#define V810_EVENT_SCSI		3
#define V810_EVENT_ADPCM	4
#define V810_EVENT_KING		5
#define V810_EVENT_NONONO	0x7fffffff

void v810_setevent(int type, int32 thetime);
int V810_StateAction(StateMem *sm, int load, int data_only);

#ifdef WANT_PCFX_MMAP
void v810_set_mmap_base(uint8 *ptr);
#endif

#endif

