/*****************************************************************************
 *
 *   arm7.c
 *   Portable ARM7TDMI CPU Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *  #3) Thumb support by Ryan Holtz
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.c file itself regarding issues/limitations of the arm7 core.
    **
*****************************************************************************/
#include "burnint.h"
#include "arm7core.h"
#include "arm7_intf.h"

#if defined __GNUC__
__extension__ typedef unsigned long long	UINT64;
__extension__ typedef signed long long		INT64;
#endif

#if defined _MSC_VER
typedef unsigned long long	UINT64;
typedef signed long long	INT64;
#endif

/* Example for showing how Co-Proc functions work */
#define TEST_COPROC_FUNCS 0

/* prototypes */
#if TEST_COPROC_FUNCS
static WRITE32_HANDLER(test_do_callback);
static READ32_HANDLER(test_rt_r_callback);
static WRITE32_HANDLER(test_rt_w_callback);
static void test_dt_r_callback(UINT32 insn, UINT32 *prn, UINT32 (*read32)(UINT32 addr));
static void test_dt_w_callback(UINT32 insn, UINT32 *prn, void (*write32)(UINT32 addr, UINT32 data));
static char *Spec_RT(char *pBuf, UINT32 opcode, char *pConditionCode, char *pBuf0);
static char *Spec_DT(char *pBuf, UINT32 opcode, char *pConditionCode, char *pBuf0);
static char *Spec_DO(char *pBuf, UINT32 opcode, char *pConditionCode, char *pBuf0);
#endif

/* Macros that can be re-defined for custom cpu implementations - The core expects these to be defined */
/* In this case, we are using the default arm7 handlers (supplied by the core)
   - but simply changes these and define your own if needed for cpu implementation specific needs */
#define READ8(addr)         arm7_cpu_read8(addr)
#define WRITE8(addr,data)   arm7_cpu_write8(addr,data)
#define READ16(addr)        arm7_cpu_read16(addr)
#define WRITE16(addr,data)  arm7_cpu_write16(addr,data)
#define READ32(addr)        arm7_cpu_read32(addr)
#define WRITE32(addr,data)  arm7_cpu_write32(addr,data)
#define PTR_READ32          &arm7_cpu_read32
#define PTR_WRITE32         &arm7_cpu_write32

/* Macros that need to be defined according to the cpu implementation specific need */
#define ARM7REG(reg)        arm7.sArmRegister[reg]
#define ARM7                arm7
#define ARM7_ICOUNT         arm7_icount

/* CPU Registers */
typedef struct
{
    ARM7CORE_REGS               // these must be included in your cpu specific register implementation
} ARM7_REGS;

static ARM7_REGS arm7;
static int ARM7_ICOUNT;
static int total_cycles = 0;
static int curr_cycles = 0;

void Arm7Open(int ) 
{

}

void Arm7Close()
{

}

int Arm7TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7TotalCycles called without init\n"));
#endif

	return total_cycles;
}

void Arm7RunEnd()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7RunEnd called without init\n"));
#endif

	arm7_icount = 0;
}

void Arm7BurnCycles(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7BurnCycles called without init\n"));
#endif

	arm7_icount -= cycles;
}

void Arm7NewFrame()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7NewFrame called without init\n"));
#endif

	total_cycles = 0;
}

/* include the arm7 core */
#include "arm7core.c"

/***************************************************************************
 * CPU SPECIFIC IMPLEMENTATIONS
 **************************************************************************/
/*
static void arm7_init(int )
{

}*/

void Arm7Reset()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7Reset called without init\n"));
#endif

    // must call core reset
    arm7_core_reset();
}

/*
static void arm7_exit()
{
    
}*/

int Arm7Run(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7Run called without init\n"));
#endif

/* include the arm7 core execute code */
#include "arm7exec.c"
}

void arm7_set_irq_line(int irqline, int state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("arm7_set_irq_line called without init\n"));
#endif

	// must call core
	arm7_core_set_irq_line(irqline,state);
}

int Arm7Scan(int nAction)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARM7Initted) bprintf(PRINT_ERROR, _T("Arm7Scan called without init\n"));
#endif

	struct BurnArea ba;
	
	if (nAction & ACB_DRIVER_DATA) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = (unsigned char*)&ARM7;
		ba.nLen	  = sizeof(ARM7);
		ba.szName = "All  Registers";
		BurnAcb(&ba);

		SCAN_VAR(ARM7_ICOUNT);
		SCAN_VAR(total_cycles);
		SCAN_VAR(curr_cycles);
	}

	return 0;
}
