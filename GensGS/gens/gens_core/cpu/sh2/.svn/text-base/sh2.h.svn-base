/**********************************************************/
/*                                                        */
/* SH2 emulator 1.40 (Hearder part)                       */
/* Copyright 2002 Stéphane Dallongeville                  */
/* Used for the 32X emulation in Gens                     */
/*                                                        */
/**********************************************************/

/* 20021106 sthief: slightly changed for GCC (FASTCALL, DECL_FASTCALL) */

#ifndef GENS_SH2_H
#define GENS_SH2_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************/
/* Compiler dependent defines */
/******************************/

#ifndef UINT8
#define UINT8   unsigned char
#endif

#ifndef INT8
#define INT8    signed char
#endif

#ifndef UINT16
#define UINT16  unsigned short
#endif

#ifndef INT16
#define INT16   signed short
#endif

#ifndef UINT32
#define UINT32  unsigned long
#endif

#ifndef INT32
#define INT32   signed long
#endif


#ifndef __WIN32__

#ifndef __GNUC__ 

#define FASTCALL				__fastcall
#define DECL_FASTCALL(type, name)	type FASTCALL name

#else //__GNUC__
#define __fastcall __attribute__ ((fastcall))
#define FASTCALL __attribute__ ((fastcall))
#define DECL_FASTCALL(type, name)	type name __attribute__ ((fastcall))

#endif //!__GNUC__

#else /* __WIN32__ */

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif /* FASTCALL */

#ifndef DECL_FASTCALL
#define DECL_FASTCALL(type, name)	type FASTCALL name
#endif /* DECL_FASTCALL */

#endif /* __WIN32__ */


/****************************/
/* Structures & definitions */
/****************************/

struct __status_reg
{
	UINT8 T;
	UINT8 S;
	UINT8 IMask;
	UINT8 MQ;
};

typedef struct __status_reg STATREG;

struct __interrupt_struct
{
	UINT8 Vect;
	UINT8 Prio;
	UINT8 res1;
	UINT8 res2;
};

typedef struct __interrupt_struct INTSTR;


struct __fetch_region
{
	UINT32 Low_Adr;
	UINT32 High_Adr;
	UINT16 *Fetch_Reg;
};

typedef struct __fetch_region FETCHREG;


typedef UINT8 FASTCALL SH2_RB(UINT32 adr);
typedef UINT16 FASTCALL SH2_RW(UINT32 adr);
typedef UINT32 FASTCALL SH2_RL(UINT32 adr);
typedef void FASTCALL SH2_WB(UINT32 adr, UINT8 data);
typedef void FASTCALL SH2_WW(UINT32 adr, UINT16 data);
typedef void FASTCALL SH2_WL(UINT32 adr, UINT32 data);


struct __SH2_context
{
	UINT8      Cache[0x1000];

	UINT32     R[0x10];

	STATREG    SR;
	INTSTR     INT;
	UINT32     GBR;
	UINT32     VBR;

	UINT8      INT_QUEUE[0x20];

	UINT32     MACH;
	UINT32     MACL;
	UINT32     PR;
	UINT32     PC;

	UINT32     Status;
	UINT32     Base_PC;
	UINT32     Fetch_Start;
	UINT32     Fetch_End;

	UINT32     DS_Inst;
	UINT32     DS_PC;
	UINT32     Unused1;
	UINT32     Unused2;

	UINT32     Odometer;
	UINT32     Cycle_TD;
	UINT32     Cycle_IO;
	UINT32     Cycle_Sup;
	
	SH2_RB     *Read_Byte[0x100];
	SH2_RW     *Read_Word[0x100];
	SH2_RL     *Read_Long[0x100];

	SH2_WB     *Write_Byte[0x100];
	SH2_WW     *Write_Word[0x100];
	SH2_WL     *Write_Long[0x100];

	FETCHREG   Fetch_Region[0x100];

	UINT8      IO_Reg[0x200];

	UINT32     DVCR;
	UINT32     DVSR;
	UINT32     DVDNTH;
	UINT32     DVDNTL;

	UINT8      DRCR0;
	UINT8      DRCR1;
	UINT8      DREQ0;
	UINT8      DREQ1;

	UINT32     DMAOR;

	UINT32     SAR0;
	UINT32     DAR0;
	UINT32     TCR0;
	UINT32     CHCR0;

	UINT32     SAR1;
	UINT32     DAR1;
	UINT32     TCR1;
	UINT32     CHCR1;

	UINT32     VCRDIV;
	UINT32     VCRDMA0;
	UINT32     VCRDMA1;
	UINT32     VCRWDT;

	UINT32     IPDIV;
	UINT32     IPDMA;
	UINT32     IPWDT;
	UINT32     IPBSC;

	UINT32     BARA;
	UINT32     BAMRA;

	UINT8      WDT_Tab[8];
	UINT32     WDTCNT;
	UINT8      WDT_Sft;
	UINT8      WDTSR;
	UINT8      WDTRST;
	UINT8      Unused3;

	UINT8      FRT_Tab[4];
	UINT32     FRTCNT;
	UINT32     FRTOCRA;
	UINT32     FRTOCRB;

	UINT8      FRTTIER;
	UINT8      FRTCSR;
	UINT8      FRTTCR;
	UINT8      FRTTOCR;
	UINT32     FRTICR;
	UINT32     FRT_Sft;
	UINT32     BCR1;
};

typedef struct __SH2_context SH2_CONTEXT;


/*************************/
/* Publics SH2 variables */
/*************************/

// 2 SH2 contexts offered :)

extern SH2_CONTEXT M_SH2;
extern SH2_CONTEXT S_SH2;


/*************************/
/* Publics SH2 functions */
/*************************/

// C functions
;
void SH2_Init(SH2_CONTEXT *SH2, UINT32 slave);
			 
// VERY IMPORTANT : Fetch have to be a DWORD aligned pointer !

void SH2_Set_Fetch_Reg(SH2_CONTEXT *SH2, UINT32 reg, UINT32 low_adr, UINT32 high_adr, UINT16 *Fetch);
void SH2_Add_Fetch(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, UINT16 *Fetch);

void SH2_Add_ReadB(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, SH2_RB *Func);
void SH2_Add_ReadW(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, SH2_RW *Func);
void SH2_Add_ReadL(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, SH2_RL *Func);
void SH2_Add_WriteB(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, SH2_WB *Func);
void SH2_Add_WriteW(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, SH2_WW *Func);
void SH2_Add_WriteL(SH2_CONTEXT *SH2, UINT32 low_adr, UINT32 high_adr, SH2_WL *Func);

void SH2_Map_Cache_Trough(SH2_CONTEXT *SH2);

DECL_FASTCALL(UINT8,	Def_READB(UINT32));
DECL_FASTCALL(UINT16,	Def_READW(UINT32));
DECL_FASTCALL(UINT32,	Def_READL(UINT32));

DECL_FASTCALL(void,	Def_WRITEB(UINT32, UINT8));
DECL_FASTCALL(void,	Def_WRITEW(UINT32, UINT16));
DECL_FASTCALL(void,	Def_WRITEL(UINT32, UINT32));

// ASM emulation functions

DECL_FASTCALL(UINT32,	SH2_Reset(SH2_CONTEXT *, UINT32));			/* SH2, manual */
DECL_FASTCALL(void,	SH2_Enable(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(void,	SH2_Disable(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Exec(SH2_CONTEXT *, UINT32));			/* SH2, odo */
DECL_FASTCALL(void,	SH2_Interrupt(SH2_CONTEXT *, UINT32));		/* SH2, level */
DECL_FASTCALL(void,	SH2_Interrupt_Internal(SH2_CONTEXT *, UINT32));	/* SH2, level_vector */
DECL_FASTCALL(void,	SH2_NMI(SH2_CONTEXT *));					/* SH2 */

DECL_FASTCALL(UINT32,	SH2_Read_Odo(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(void,	SH2_Write_Odo(SH2_CONTEXT *, UINT32));		/* SH2 */
DECL_FASTCALL(void,	SH2_Clear_Odo(SH2_CONTEXT *));			/* SH2 */
DECL_FASTCALL(void,	SH2_Add_Cycles(SH2_CONTEXT *, UINT32));		/* SH2, cycles */

DECL_FASTCALL(void,	SH2_DMA0_Request(SH2_CONTEXT *, UINT8));		/* SH2, state */
DECL_FASTCALL(void,	SH2_DMA1_Request(SH2_CONTEXT *, UINT8));		/* SH2, state */
DECL_FASTCALL(void,	SH2_FRT_Signal(SH2_CONTEXT *));			/* SH2 */

DECL_FASTCALL(UINT32,	SH2_Get_R(SH2_CONTEXT *, UINT32));			/* SH2, num */
DECL_FASTCALL(UINT32,	SH2_Get_PC(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Get_SR(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Get_GBR(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Get_VBR(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Get_PR(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Get_MACH(SH2_CONTEXT *));				/* SH2 */
DECL_FASTCALL(UINT32,	SH2_Get_MACL(SH2_CONTEXT *));				/* SH2 */

DECL_FASTCALL(void,	SH2_Set_PC(SH2_CONTEXT *, UINT32));			/* SH2, val */
DECL_FASTCALL(void,	SH2_Set_SR(SH2_CONTEXT *, UINT32));			/* SH2, val */
DECL_FASTCALL(void,	SH2_Set_GBR(SH2_CONTEXT *, UINT32));		/* SH2, val */
DECL_FASTCALL(void,	SH2_Set_VBR(SH2_CONTEXT *, UINT32));		/* SH2, val */
DECL_FASTCALL(void,	SH2_Set_PR(SH2_CONTEXT *, UINT32));			/* SH2, val */
DECL_FASTCALL(void,	SH2_Set_MACH(SH2_CONTEXT *, UINT32));		/* SH2, val */
DECL_FASTCALL(void,	SH2_Set_MACL(SH2_CONTEXT *, UINT32));		/* SH2, val */

UINT8 SH2_Read_Byte(SH2_CONTEXT *SH2, UINT32 adr);
UINT16 SH2_Read_Word(SH2_CONTEXT *SH2, UINT32 adr);
UINT32 SH2_Read_Long(SH2_CONTEXT *SH2, UINT32 adr);

void SH2_Write_Byte(SH2_CONTEXT *SH2, UINT32 adr, UINT8 data);
void SH2_Write_Word(SH2_CONTEXT *SH2, UINT32 adr, UINT16 data);
void SH2_Write_Long(SH2_CONTEXT *SH2, UINT32 adr, UINT32 data);


/****************************/
/* No-Publics SH2 variables */
/****************************/

extern void *OP_Table[0x10000];
extern void *OPDS_Table[0x10000];
extern UINT32 Set_SR_Table[0x400];


/****************************/
/* No-Publics SH2 functions */
/****************************/

// ASM Memories handlers

DECL_FASTCALL(UINT8,	SH2_Read_Byte_C0(UINT32));
DECL_FASTCALL(UINT16,	SH2_Read_Word_C0(UINT32));
DECL_FASTCALL(UINT32,	SH2_Read_Long_C0(UINT32));
DECL_FASTCALL(UINT8,	SH2_Read_Byte_FF(UINT32));
DECL_FASTCALL(UINT16,	SH2_Read_Word_FF(UINT32));
DECL_FASTCALL(UINT32,	SH2_Read_Long_FF(UINT32));

DECL_FASTCALL(void,	SH2_Write_Byte_C0(UINT32, UINT8));
DECL_FASTCALL(void,	SH2_Write_Word_C0(UINT32, UINT16));
DECL_FASTCALL(void,	SH2_Write_Long_C0(UINT32, UINT32));
DECL_FASTCALL(void,	SH2_Write_Byte_FF(UINT32, UINT8));
DECL_FASTCALL(void,	SH2_Write_Word_FF(UINT32, UINT16));
DECL_FASTCALL(void,	SH2_Write_Long_FF(UINT32, UINT32));


// Instruction function definition

void SH2I_ADD();
void SH2I_ADDI();
void SH2I_ADDC();
void SH2I_ADDV();
void SH2I_AND();
void SH2I_ANDI();
void SH2I_ANDM();
void SH2I_BF();
void SH2I_BFfast();
void SH2I_BFS();
void SH2I_BFSfast();
void SH2I_BRA();
void SH2I_BRAfast1();
void SH2I_BRAfast2();
void SH2I_BRAF();
void SH2I_BSR();
void SH2I_BSRF();
void SH2I_BT();
void SH2I_BTS();
void SH2I_CLRMAC();
void SH2I_CLRT();
void SH2I_CMPEQ();
void SH2I_CMPGE();
void SH2I_CMPGT();
void SH2I_CMPHI();
void SH2I_CMPHS();
void SH2I_CMPPL();
void SH2I_CMPPZ();
void SH2I_CMPSTR();
void SH2I_CMPIM();
void SH2I_DIV0S();
void SH2I_DIV0U();
void SH2I_DIV1();
void SH2I_DMULS();
void SH2I_DMULU();
void SH2I_DT();
void SH2I_EXTSB();
void SH2I_EXTSW();
void SH2I_EXTUB();
void SH2I_EXTUW();
void SH2I_ILLEGAL();
void SH2I_JMP();
void SH2I_JSR();
void SH2I_LDCSR();
void SH2I_LDCGBR();
void SH2I_LDCVBR();
void SH2I_LDCMSR();
void SH2I_LDCMGBR();
void SH2I_LDCMVBR();
void SH2I_LDSMACH();
void SH2I_LDSMACL();
void SH2I_LDSPR();
void SH2I_LDSMMACH();
void SH2I_LDSMMACL();
void SH2I_LDSMPR();
void SH2I_MACL();
void SH2I_MACW();
void SH2I_MOV();
void SH2I_MOVBS();
void SH2I_MOVWS();
void SH2I_MOVLS();
void SH2I_MOVBL();
void SH2I_MOVWL();
void SH2I_MOVLL();
void SH2I_MOVBM();
void SH2I_MOVWM();
void SH2I_MOVLM();
void SH2I_MOVBP();
void SH2I_MOVWP();
void SH2I_MOVLP();
void SH2I_MOVBS0();
void SH2I_MOVWS0();
void SH2I_MOVLS0();
void SH2I_MOVBL0();
void SH2I_MOVWL0();
void SH2I_MOVLL0();
void SH2I_MOVI();
void SH2I_MOVWI();
void SH2I_MOVLI();
void SH2I_MOVBLG();
void SH2I_MOVWLG();
void SH2I_MOVLLG();
void SH2I_MOVBSG();
void SH2I_MOVWSG();
void SH2I_MOVLSG();
void SH2I_MOVBS4();
void SH2I_MOVWS4();
void SH2I_MOVLS4();
void SH2I_MOVBL4();
void SH2I_MOVWL4();
void SH2I_MOVLL4();
void SH2I_MOVA();
void SH2I_MOVT();
void SH2I_MULL();
void SH2I_MULS();
void SH2I_MULU();
void SH2I_NEG();
void SH2I_NEGC();
void SH2I_NOP();
void SH2I_NOT();
void SH2I_OR();
void SH2I_ORI();
void SH2I_ORM();
void SH2I_ROTCL();
void SH2I_ROTCR();
void SH2I_ROTL();
void SH2I_ROTR();
void SH2I_RTE();
void SH2I_RTS();
void SH2I_SETT();
void SH2I_SHAL();
void SH2I_SHAR();
void SH2I_SHLL();
void SH2I_SHLL2();
void SH2I_SHLL8();
void SH2I_SHLL16();
void SH2I_SHLR();
void SH2I_SHLR2();
void SH2I_SHLR8();
void SH2I_SHLR16();
void SH2I_SLEEP();
void SH2I_STCSR();
void SH2I_STCGBR();
void SH2I_STCVBR();
void SH2I_STCMSR();
void SH2I_STCMGBR();
void SH2I_STCMVBR();
void SH2I_STSMACH();
void SH2I_STSMACL();
void SH2I_STSPR();
void SH2I_STSMMACH();
void SH2I_STSMMACL();
void SH2I_STSMPR();
void SH2I_SUB();
void SH2I_SUBC();
void SH2I_SUBV();
void SH2I_SWAPB();
void SH2I_SWAPW();
void SH2I_TAS();
void SH2I_TRAPA();
void SH2I_TST();
void SH2I_TSTI();
void SH2I_TSTM();
void SH2I_XOR();
void SH2I_XORI();
void SH2I_XORM();
void SH2I_XTRCT();


// Delay-slot instruction function definition

void SH2I_ADD_DS();
void SH2I_ADDI_DS();
void SH2I_ADDC_DS();
void SH2I_ADDV_DS();
void SH2I_AND_DS();
void SH2I_ANDI_DS();
void SH2I_ANDM_DS();
void SH2I_CLRMAC_DS();
void SH2I_CLRT_DS();
void SH2I_CMPEQ_DS();
void SH2I_CMPGE_DS();
void SH2I_CMPGT_DS();
void SH2I_CMPHI_DS();
void SH2I_CMPHS_DS();
void SH2I_CMPPL_DS();
void SH2I_CMPPZ_DS();
void SH2I_CMPSTR_DS();
void SH2I_CMPIM_DS();
void SH2I_DIV0S_DS();
void SH2I_DIV0U_DS();
void SH2I_DIV1_DS();
void SH2I_DMULS_DS();
void SH2I_DMULU_DS();
void SH2I_DT_DS();
void SH2I_EXTSB_DS();
void SH2I_EXTSW_DS();
void SH2I_EXTUB_DS();
void SH2I_EXTUW_DS();
void SH2I_ILLEGAL_DS();
void SH2I_LDCSR_DS();
void SH2I_LDCGBR_DS();
void SH2I_LDCVBR_DS();
void SH2I_LDCMSR_DS();
void SH2I_LDCMGBR_DS();
void SH2I_LDCMVBR_DS();
void SH2I_LDSMACH_DS();
void SH2I_LDSMACL_DS();
void SH2I_LDSPR_DS();
void SH2I_LDSMMACH_DS();
void SH2I_LDSMMACL_DS();
void SH2I_LDSMPR_DS();
void SH2I_MACL_DS();
void SH2I_MACW_DS();
void SH2I_MOV_DS();
void SH2I_MOVBS_DS();
void SH2I_MOVWS_DS();
void SH2I_MOVLS_DS();
void SH2I_MOVBL_DS();
void SH2I_MOVWL_DS();
void SH2I_MOVLL_DS();
void SH2I_MOVBM_DS();
void SH2I_MOVWM_DS();
void SH2I_MOVLM_DS();
void SH2I_MOVBP_DS();
void SH2I_MOVWP_DS();
void SH2I_MOVLP_DS();
void SH2I_MOVBS0_DS();
void SH2I_MOVWS0_DS();
void SH2I_MOVLS0_DS();
void SH2I_MOVBL0_DS();
void SH2I_MOVWL0_DS();
void SH2I_MOVLL0_DS();
void SH2I_MOVI_DS();
void SH2I_MOVWI_DS();
void SH2I_MOVLI_DS();
void SH2I_MOVBLG_DS();
void SH2I_MOVWLG_DS();
void SH2I_MOVLLG_DS();
void SH2I_MOVBSG_DS();
void SH2I_MOVWSG_DS();
void SH2I_MOVLSG_DS();
void SH2I_MOVBS4_DS();
void SH2I_MOVWS4_DS();
void SH2I_MOVLS4_DS();
void SH2I_MOVBL4_DS();
void SH2I_MOVWL4_DS();
void SH2I_MOVLL4_DS();
void SH2I_MOVA_DS();
void SH2I_MOVT_DS();
void SH2I_MULL_DS();
void SH2I_MULS_DS();
void SH2I_MULU_DS();
void SH2I_NEG_DS();
void SH2I_NEGC_DS();
void SH2I_NOP_DS();
void SH2I_NOT_DS();
void SH2I_OR_DS();
void SH2I_ORI_DS();
void SH2I_ORM_DS();
void SH2I_ROTCL_DS();
void SH2I_ROTCR_DS();
void SH2I_ROTL_DS();
void SH2I_ROTR_DS();
void SH2I_SETT_DS();
void SH2I_SHAL_DS();
void SH2I_SHAR_DS();
void SH2I_SHLL_DS();
void SH2I_SHLL2_DS();
void SH2I_SHLL8_DS();
void SH2I_SHLL16_DS();
void SH2I_SHLR_DS();
void SH2I_SHLR2_DS();
void SH2I_SHLR8_DS();
void SH2I_SHLR16_DS();
void SH2I_SLEEP_DS();
void SH2I_STCSR_DS();
void SH2I_STCGBR_DS();
void SH2I_STCVBR_DS();
void SH2I_STCMSR_DS();
void SH2I_STCMGBR_DS();
void SH2I_STCMVBR_DS();
void SH2I_STSMACH_DS();
void SH2I_STSMACL_DS();
void SH2I_STSPR_DS();
void SH2I_STSMMACH_DS();
void SH2I_STSMMACL_DS();
void SH2I_STSMPR_DS();
void SH2I_SUB_DS();
void SH2I_SUBC_DS();
void SH2I_SUBV_DS();
void SH2I_SWAPB_DS();
void SH2I_SWAPW_DS();
void SH2I_TAS_DS();
void SH2I_TST_DS();
void SH2I_TSTI_DS();
void SH2I_TSTM_DS();
void SH2I_XOR_DS();
void SH2I_XORI_DS();
void SH2I_XORM_DS();
void SH2I_XTRCT_DS();

#ifdef __cplusplus
}
#endif

#endif /* GENS_SH2_H */
