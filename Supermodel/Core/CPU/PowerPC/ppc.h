/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * ppc.h
 *
 * Header file for PowerPC emulator.
 */

#ifndef INCLUDED_PPC_H
#define INCLUDED_PPC_H


/******************************************************************************
 Definitions
******************************************************************************/
 
#define SPR_XER				1		/* Fixed Point Exception Register               Read / Write */
#define SPR_LR				8		/* Link Register                                Read / Write */
#define SPR_CTR				9		/* Count Register                               Read / Write */
#define SPR_SRR0			26		/* Save/Restore Register 0                      Read / Write */
#define SPR_SRR1			27		/* Save/Restore Register 1                      Read / Write */
#define SPR_SPRG0			272		/* SPR General 0                                Read / Write */
#define SPR_SPRG1			273		/* SPR General 1                                Read / Write */
#define SPR_SPRG2			274		/* SPR General 2                                Read / Write */
#define SPR_SPRG3			275		/* SPR General 3                                Read / Write */
#define SPR_PVR				287		/* Processor Version Number                     Read Only */

#define SPR403_ICDBDR		0x3D3	/* 406GA Instruction Cache Debug Data Register  Rad Only */
#define SPR403_ESR			0x3D4	/* 406GA Exception Syndrome Register            Read / Write */
#define SPR403_DEAR			0x3D5	/* 406GA Data Exception Address Register        Read Only */
#define SPR403_EVPR			0x3D6	/* 406GA Exception Vector Prefix Register       Read / Write */
#define SPR403_CDBCR		0x3D7	/* 406GA Cache Debug Control Register           Read/Write */
#define SPR403_TSR			0x3D8	/* 406GA Timer Status Register                  Read / Clear */
#define SPR403_TCR			0x3DA	/* 406GA Timer Control Register                 Read / Write */
#define SPR403_PIT			0x3DB	/* 406GA Programmable Interval Timer            Read / Write */
#define SPR403_TBHI			988		/* 406GA Time Base High                         Read / Write */
#define SPR403_TBLO			989		/* 406GA Time Base Low                          Read / Write */
#define SPR403_SRR2			0x3DE	/* 406GA Save/Restore Register 2                Read / Write */
#define SPR403_SRR3			0x3DF	/* 406GA Save/Restore Register 3                Read / Write */
#define SPR403_DBSR			0x3F0	/* 406GA Debug Status Register                  Read / Clear */
#define SPR403_DBCR			0x3F2	/* 406GA Debug Control Register                 Read / Write */
#define SPR403_IAC1			0x3F4	/* 406GA Instruction Address Compare 1          Read / Write */
#define SPR403_IAC2			0x3F5	/* 406GA Instruction Address Compare 2          Read / Write */
#define SPR403_DAC1			0x3F6	/* 406GA Data Address Compare 1                 Read / Write */
#define SPR403_DAC2			0x3F7	/* 406GA Data Address Compare 2                 Read / Write */
#define SPR403_DCCR			0x3FA	/* 406GA Data Cache Cacheability Register       Read / Write */
#define SPR403_ICCR			0x3FB	/* 406GA I Cache Cacheability Registe           Read / Write */
#define SPR403_PBL1			0x3FC	/* 406GA Protection Bound Lower 1               Read / Write */
#define SPR403_PBU1			0x3FD	/* 406GA Protection Bound Upper 1               Read / Write */
#define SPR403_PBL2			0x3FE	/* 406GA Protection Bound Lower 2               Read / Write */
#define SPR403_PBU2			0x3FF	/* 406GA Protection Bound Upper 2               Read / Write */

#define SPR403_SGR			0x3b9	/* 403GCX Storage Guarded Register */
#define SPR403_DCWR			0x3ba	/* 403GCX Data Cache Write Through */
#define SPR403_PID			0x3b1	/* 403GCX Process ID */
#define SPR403_TBHU			0x3cc	/* 403GCX Time Base High User-mode */
#define SPR403_TBLU			0x3cd	/* 403GCX Time Base Low User-mode */

#define SPR603E_DSISR		18		/* 603E */
#define SPR603E_DAR			19		/* 603E */
#define SPR603E_DEC			22		/* 603E */
#define SPR603E_SDR1		25		/* 603E */
#define SPR603E_TBL_R		268		/* 603E Time Base Low (Read-only) */
#define SPR603E_TBU_R		269		/* 603E Time Base High (Read-only) */
#define SPR603E_TBL_W		284		/* 603E Time Base Low (Write-only) */
#define SPR603E_TBU_W		285		/* 603E Time Base Hight (Write-only) */
#define SPR603E_EAR			282		/* 603E */
#define SPR603E_IBAT0U		528		/* 603E */
#define SPR603E_IBAT0L		529		/* 603E */
#define SPR603E_IBAT1U		530		/* 603E */
#define SPR603E_IBAT1L		531		/* 603E */
#define SPR603E_IBAT2U		532		/* 603E */
#define SPR603E_IBAT2L		533		/* 603E */
#define SPR603E_IBAT3U		534		/* 603E */
#define SPR603E_IBAT3L		535		/* 603E */
#define SPR603E_DBAT0U		536		/* 603E */
#define SPR603E_DBAT0L		537		/* 603E */
#define SPR603E_DBAT1U		538		/* 603E */
#define SPR603E_DBAT1L		539		/* 603E */
#define SPR603E_DBAT2U		540		/* 603E */
#define SPR603E_DBAT2L		541		/* 603E */
#define SPR603E_DBAT3U		542		/* 603E */
#define SPR603E_DBAT3L		543		/* 603E */
#define SPR603E_DABR		1013	/* 603E */
#define SPR603E_DMISS		0x3d0	/* 603E */
#define SPR603E_DCMP		0x3d1	/* 603E */
#define SPR603E_HASH1		0x3d2	/* 603E */
#define SPR603E_HASH2		0x3d3	/* 603E */
#define SPR603E_IMISS		0x3d4	/* 603E */
#define SPR603E_ICMP		0x3d5	/* 603E */
#define SPR603E_RPA			0x3d6	/* 603E */
#define SPR603E_HID0		1008	/* 603E */
#define SPR603E_HID1		1009	/* 603E */
#define SPR603E_IABR		1010	/* 603E */
#define SPR603E_HID2		1011	/* 603E */

#define SPR602_TCR			984		/* 602 */
#define SPR602_IBR			986		/* 602 */
#define SPR602_SP			1021	/* 602 */
#define SPR602_LT			1022	/* 602 */


#define DCR_BEAR		0x90	/* bus error address */
#define DCR_BESR		0x91	/* bus error syndrome */
#define DCR_BR0			0x80	/* bank */
#define DCR_BR1			0x81	/* bank */
#define DCR_BR2			0x82	/* bank */
#define DCR_BR3			0x83	/* bank */
#define DCR_BR4			0x84	/* bank */
#define DCR_BR5			0x85	/* bank */
#define DCR_BR6			0x86	/* bank */
#define DCR_BR7			0x87	/* bank */
#define DCR_DMACC0		0xc4	/* dma chained count */
#define DCR_DMACC1		0xcc	/* dma chained count */
#define DCR_DMACC2		0xd4	/* dma chained count */
#define DCR_DMACC3		0xdc	/* dma chained count */
#define DCR_DMACR0		0xc0	/* dma channel control */
#define DCR_DMACR1		0xc8	/* dma channel control */
#define DCR_DMACR2		0xd0	/* dma channel control */
#define DCR_DMACR3		0xd8	/* dma channel control */
#define DCR_DMACT0		0xc1	/* dma destination address */
#define DCR_DMACT1		0xc9	/* dma destination address */
#define DCR_DMACT2		0xd1	/* dma destination address */
#define DCR_DMACT3		0xd9	/* dma destination address */
#define DCR_DMADA0		0xc2	/* dma destination address */
#define DCR_DMADA1		0xca	/* dma destination address */
#define DCR_DMADA2		0xd2	/* dma source address */
#define DCR_DMADA3		0xda	/* dma source address */
#define DCR_DMASA0		0xc3	/* dma source address */
#define DCR_DMASA1		0xcb	/* dma source address */
#define DCR_DMASA2		0xd3	/* dma source address */
#define DCR_DMASA3		0xdb	/* dma source address */
#define DCR_DMASR		0xe0	/* dma status */
#define DCR_EXIER		0x42	/* external interrupt enable */
#define DCR_EXISR		0x40	/* external interrupt status */
#define DCR_IOCR		0xa0	/* io configuration */

enum {
	EXCEPTION_IRQ						= 1,
	EXCEPTION_DECREMENTER				= 2,
	EXCEPTION_TRAP						= 3,
	EXCEPTION_SYSTEM_CALL				= 4,
	EXCEPTION_SMI						= 5,
	EXCEPTION_DSI						= 6,
	EXCEPTION_ISI						= 7,
	EXCEPTION_PROGRAMMABLE_INTERVAL_TIMER   = 20,
	EXCEPTION_FIXED_INTERVAL_TIMER		= 21,
	EXCEPTION_WATCHDOG_TIMER			= 22,
	EXCEPTION_CRITICAL_INTERRUPT			= 23,
};

enum {
	PPC_INPUT_LINE_SMI = 10,
	PPC_INPUT_LINE_TLBISYNC
};

enum {
	PPC_PC=1,
	PPC_MSR,
	PPC_CR,
	PPC_LR,
	PPC_CTR,
	PPC_XER,
	PPC_DEC,
	PPC_SRR0,
	PPC_SRR1,
	PPC_R0,
	PPC_R1,
	PPC_R2,
	PPC_R3,
	PPC_R4,
	PPC_R5,
	PPC_R6,
	PPC_R7,
	PPC_R8,
	PPC_R9,
	PPC_R10,
	PPC_R11,
	PPC_R12,
	PPC_R13,
	PPC_R14,
	PPC_R15,
	PPC_R16,
	PPC_R17,
	PPC_R18,
	PPC_R19,
	PPC_R20,
	PPC_R21,
	PPC_R22,
	PPC_R23,
	PPC_R24,
	PPC_R25,
	PPC_R26,
	PPC_R27,
	PPC_R28,
	PPC_R29,
	PPC_R30,
	PPC_R31
};

typedef enum {
	PPC_MODEL_403GA				= 0x00200000,
	PPC_MODEL_403GB				= 0x00200100,
	PPC_MODEL_403GC				= 0x00200200,
	PPC_MODEL_403GCX			= 0x00201400,
	PPC_MODEL_405GP				= 0x40110000,
	PPC_MODEL_601				= 0x00010000,
	PPC_MODEL_603				= 0x00030000,	/* "Wart" */
	PPC_MODEL_604				= 0x00040000,	/* "Zephyr" */
	PPC_MODEL_602				= 0x00050000,	/* "Galahad" */
	PPC_MODEL_603E				= 0x00060103,	/* "Stretch", version 1.3 */
	PPC_MODEL_603EV				= 0x00070000,	/* "Valiant" */
	PPC_MODEL_603R				= 0x00071202,	/* "Goldeneye", version 2.1 */
	PPC_MODEL_740				= 0x00080301,	/* "Arthur", version 3.1 */
	PPC_MODEL_750				= PPC_MODEL_740,
	PPC_MODEL_740P				= 0x00080202,	/* "Conan Doyle", version 1.2 */
	PPC_MODEL_750P				= PPC_MODEL_740P,
	PPC_MODEL_755				= 0x00083203	/* "Goldfinger", version 2.3 */
} PPC_MODEL;

typedef enum {
	BUS_FREQUENCY_16MHZ = 0,
	BUS_FREQUENCY_20MHZ,
	BUS_FREQUENCY_25MHZ,
	BUS_FREQUENCY_33MHZ,
	BUS_FREQUENCY_40MHZ,
	BUS_FREQUENCY_50MHZ,
	BUS_FREQUENCY_60MHZ,
	BUS_FREQUENCY_66MHZ,
	BUS_FREQUENCY_75MHZ,
} PPC_BUS_FREQUENCY;

// PLL Configuration based on the table in MPC603EUM page 7-31
static const int mpc603e_pll_config[12][9] =
{
	// 16,  20,  25,  33,  40,  50,  60,  66,  75
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{ 0x2, 0x2, 0x2, 0x1, 0x1, 0x1,  -1, 0x0,  -1 },
	{  -1,  -1,  -1,  -1,  -1, 0xc,  -1, 0xc,  -1 },
	{ 0x5, 0x5, 0x5, 0x4, 0x4, 0x4,  -1,  -1,  -1 },
	{  -1,	-1,  -1, 0x6, 0x6,  -1,  -1,  -1,  -1 },
	{  -1,  -1, 0x8, 0x8,  -1,  -1,  -1,  -1,  -1 },
	{  -1, 0xe, 0xe,  -1,  -1,  -1,  -1,  -1,  -1 },
	{ 0xa, 0xa, 0xa,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
};

// PLL Configuration based on the table in MPC603E7VEC page 29
static const int mpc603ev_pll_config[12][9] =
{
	// 16,  20,  25,  33,  40,  50,  60,  66,  75
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	// 2:1
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1, 0x4, 0x4 },
	// 2.5:1
	{  -1,  -1,  -1,  -1,  -1, 0x6, 0x6, 0x6, 0x6 },
	// 3:1
	{  -1,  -1,  -1,  -1, 0x8, 0x8, 0x8, 0x8, 0x8 },
	// 3.5:1
	{  -1,  -1,  -1,  -1, 0xe, 0xe, 0xe, 0xe,  -1 },
	// 4:1
	{  -1,  -1,  -1, 0xa, 0xa, 0xa, 0xa,  -1,  -1 },
	// 4.5:1
	{  -1,  -1,  -1, 0x7, 0x7, 0x7,  -1,  -1,  -1 },
	// 5:1
	{  -1,  -1, 0xb, 0xb, 0xb,  -1,  -1,  -1,  -1 },
	// 5.5:1
	{  -1,  -1, 0x9, 0x9, 0x9,  -1,  -1,  -1,  -1 },
	// 6:1
	{  -1,  -1, 0xd, 0xd, 0xd,  -1,  -1,  -1,  -1 }
};

// PLL Configuration based on the table in MPC603E7TEC page 23
static const int mpc603r_pll_config[12][9] =
{
	// 16,  20,  25,  33,  40,  50,  60,  66,  75
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	// 2:1
	{  -1,  -1,  -1,  -1, 0x5, 0x5, 0x5, 0x5, 0x5 },
	// 2.5:1
	{  -1,  -1,  -1,  -1,  -1,  -1, 0x6, 0x6, 0x6 },
	// 3:1
	{  -1,  -1,  -1,  -1,  -1, 0x8, 0x8, 0x8, 0x8 },
	// 3.5:1
	{  -1,  -1,  -1,  -1,  -1, 0xe, 0xe, 0xe, 0xe },
	// 4:1
	{  -1,  -1,  -1,  -1, 0xa, 0xa, 0xa, 0xa, 0xa },
	// 4.5:1
	{  -1,  -1,  -1, 0x7, 0x7, 0x7, 0x7, 0x7,  -1 },
	// 5:1
	{  -1,  -1,  -1, 0xb, 0xb, 0xb, 0xb,  -1,  -1 },
	// 5.5:1
	{  -1,  -1,  -1, 0x9, 0x9, 0x9,  -1,  -1,  -1 },
	// 6:1
	{  -1,  -1, 0xd, 0xd, 0xd, 0xd,  -1,  -1,  -1 },
};


/******************************************************************************
 Configuration Data Structures
******************************************************************************/

typedef struct {
	PPC_MODEL pvr;
	int bus_frequency_multiplier;
	PPC_BUS_FREQUENCY bus_frequency;
} PPC_CONFIG;

typedef struct
{
	UINT32	start;
	UINT32	end;
	UINT32	* ptr;

} PPC_FETCH_REGION;


/******************************************************************************
 Functions
******************************************************************************/

extern UINT32 ppc_get_pc(void);
extern void ppc_set_irq_line(int irqline);
extern int ppc_execute(int cycles);
extern void ppc_reset(void);
extern void ppc_shutdown(void);
extern void ppc_init(const PPC_CONFIG *config);		// must be called second!
extern void ppc_set_fetch(PPC_FETCH_REGION * fetch);

// These have been added to support the new Supermodel
extern void ppc_attach_bus(class CBus *BusPtr);		// must be called first!
extern void ppc_save_state(class CBlockFile *SaveState);
extern void ppc_load_state(class CBlockFile *SaveState);
extern UINT32 ppc_get_gpr(unsigned num);
extern double ppc_get_fpr(unsigned num);
extern UINT32 ppc_get_lr(void);
extern UINT32 ppc_read_spr(unsigned spr);
extern UINT32 ppc_read_sr(unsigned num);

#ifdef SUPERMODEL_DEBUGGER
// These have been added to support the Supermodel debugger
extern void ppc_attach_debugger(class Debugger::CPPCDebug *PPCDebugPtr);
extern void ppc_detach_debugger();
extern void ppc_set_pc(UINT32 pc);
extern UINT8 ppc_get_cr(unsigned num);
extern void ppc_set_cr(unsigned num, UINT8 val);
extern void ppc_set_gpr(unsigned num, UINT32 val);
extern void ppc_set_fpr(unsigned num, double val);
extern void ppc_write_spr(unsigned spr, UINT32 val);
extern void ppc_write_sr(unsigned num, UINT32 val);
extern UINT32 ppc_read_msr();
#endif  // SUPERMODEL_DEBUGGER

#endif	// INCLUDED_PPC_H
