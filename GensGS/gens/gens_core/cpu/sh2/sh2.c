/**********************************************************/
/*                                                        */
/* SH2 emulator 1.60 (C source part)                      */
/* This part define only the initialisation stuff...      */
/* Emulation core itself is defined in the SH2a.asm file. */
/* Copyright 2002 Stéphane Dallongeville                  */
/*                                                        */
/**********************************************************/

#define SH2_VERSION "1.60"
#define SH2_DEBUG   0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "sh2.h"

#if SH2_DEBUG > 0
FILE *SH2_Debug = NULL;

#define SH2_LOG0(a) fprintf(SH2_Debug, a);
#define SH2_LOG1(a, b) fprintf(SH2_Debug, a, b);
#define SH2_LOG2(a, b, c) fprintf(SH2_Debug, a, b, c);
#define SH2_LOG3(a, b, c, d) fprintf(SH2_Debug, a, b, c, d);
#define SH2_LOG4(a, b, c, d, e) fprintf(SH2_Debug, a, b, c, d, e);
#else
#define SH2_LOG0(a)
#define SH2_LOG1(a, b)
#define SH2_LOG2(a, b, c)
#define SH2_LOG3(a, b, c, d)
#define SH2_LOG4(a, b, c, d, e)
#endif



/************************************************************/
/*                                                          */
/* Opcode Table :                                           */
/* Almost of the following table was taken from the         */
/* Tim's SH2 core (Retrodrives).                            */
/*                                                          */
/************************************************************/


typedef enum __ifmt
{
  IFMT_0,			// xxxx xxxx xxxx xxxx
  IFMT_n,			// xxxx nnnn xxxx xxxx
  IFMT_m,			// xxxx mmmm xxxx xxxx
  IFMT_nm,			// xxxx nnnn mmmm xxxx
  IFMT_md,			// xxxx xxxx mmmm dddd
  IFMT_nd4,			// xxxx xxxx nnnn dddd
  IFMT_nmd,			// xxxx nnnn mmmm dddd
  IFMT_d,			// xxxx xxxx dddd dddd
  IFMT_d12,			// xxxx dddd dddd dddd
  IFMT_nd8,			// xxxx nnnn dddd dddd
  IFMT_i,			// xxxx xxxx iiii iiii
  IFMT_ni,			// xxxx nnnn iiii iiii
} _IFMT;


typedef void SH2_INST_CALL ();


typedef struct
{
  _IFMT IFMT;
  UINT32 Mask;
  SH2_INST_CALL *Func;

} PACKED_OP_TAB;

PACKED_OP_TAB InitOp[] = {
  {IFMT_nm, 0x300C, SH2I_ADD},
  {IFMT_ni, 0x7000, SH2I_ADDI},
  {IFMT_nm, 0x300E, SH2I_ADDC},
  {IFMT_nm, 0x300F, SH2I_ADDV},
  {IFMT_nm, 0x2009, SH2I_AND},
  {IFMT_i, 0xC900, SH2I_ANDI},
  {IFMT_i, 0xCD00, SH2I_ANDM},
  {IFMT_d, 0x8B00, SH2I_BF},
  {IFMT_0, 0x8BFD, SH2I_BFfast},
  {IFMT_d, 0x8F00, SH2I_BFS},
  {IFMT_0, 0x8FFD, SH2I_BFSfast},
  {IFMT_d12, 0xA000, SH2I_BRA},
  {IFMT_0, 0xAFFE, SH2I_BRAfast1},
  {IFMT_0, 0xAFFD, SH2I_BRAfast2},
  {IFMT_m, 0x0023, SH2I_BRAF},
  {IFMT_d12, 0xB000, SH2I_BSR},
  {IFMT_n, 0x0003, SH2I_BSRF},
  {IFMT_d, 0x8900, SH2I_BT},
  {IFMT_d, 0x8D00, SH2I_BTS},
  {IFMT_0, 0x0028, SH2I_CLRMAC},
  {IFMT_0, 0x0008, SH2I_CLRT},
  {IFMT_nm, 0x3000, SH2I_CMPEQ},
  {IFMT_nm, 0x3003, SH2I_CMPGE},
  {IFMT_nm, 0x3007, SH2I_CMPGT},
  {IFMT_nm, 0x3006, SH2I_CMPHI},
  {IFMT_nm, 0x3002, SH2I_CMPHS},
  {IFMT_n, 0x4015, SH2I_CMPPL},
  {IFMT_n, 0x4011, SH2I_CMPPZ},
  {IFMT_nm, 0x200C, SH2I_CMPSTR},
  {IFMT_i, 0x8800, SH2I_CMPIM},
  {IFMT_nm, 0x2007, SH2I_DIV0S},
  {IFMT_0, 0x0019, SH2I_DIV0U},
  {IFMT_nm, 0x3004, SH2I_DIV1},
  {IFMT_nm, 0x300D, SH2I_DMULS},
  {IFMT_nm, 0x3005, SH2I_DMULU},
  {IFMT_n, 0x4010, SH2I_DT},
  {IFMT_nm, 0x600E, SH2I_EXTSB},
  {IFMT_nm, 0x600F, SH2I_EXTSW},
  {IFMT_nm, 0x600C, SH2I_EXTUB},
  {IFMT_nm, 0x600D, SH2I_EXTUW},
  {IFMT_n, 0x402B, SH2I_JMP},
  {IFMT_n, 0x400B, SH2I_JSR},
  {IFMT_m, 0x400E, SH2I_LDCSR},
  {IFMT_m, 0x401E, SH2I_LDCGBR},
  {IFMT_m, 0x402E, SH2I_LDCVBR},
  {IFMT_m, 0x4007, SH2I_LDCMSR},
  {IFMT_m, 0x4017, SH2I_LDCMGBR},
  {IFMT_m, 0x4027, SH2I_LDCMVBR},
  {IFMT_m, 0x400A, SH2I_LDSMACH},
  {IFMT_m, 0x401A, SH2I_LDSMACL},
  {IFMT_m, 0x402A, SH2I_LDSPR},
  {IFMT_m, 0x4006, SH2I_LDSMMACH},
  {IFMT_m, 0x4016, SH2I_LDSMMACL},
  {IFMT_m, 0x4026, SH2I_LDSMPR},
  {IFMT_nm, 0x000F, SH2I_MACL},
  {IFMT_nm, 0x400F, SH2I_MACW},
  {IFMT_nm, 0x6003, SH2I_MOV},
  {IFMT_nm, 0x2000, SH2I_MOVBS},
  {IFMT_nm, 0x2001, SH2I_MOVWS},
  {IFMT_nm, 0x2002, SH2I_MOVLS},
  {IFMT_nm, 0x6000, SH2I_MOVBL},
  {IFMT_nm, 0x6001, SH2I_MOVWL},
  {IFMT_nm, 0x6002, SH2I_MOVLL},
  {IFMT_nm, 0x2004, SH2I_MOVBM},
  {IFMT_nm, 0x2005, SH2I_MOVWM},
  {IFMT_nm, 0x2006, SH2I_MOVLM},
  {IFMT_nm, 0x6004, SH2I_MOVBP},
  {IFMT_nm, 0x6005, SH2I_MOVWP},
  {IFMT_nm, 0x6006, SH2I_MOVLP},
  {IFMT_nm, 0x0004, SH2I_MOVBS0},
  {IFMT_nm, 0x0005, SH2I_MOVWS0},
  {IFMT_nm, 0x0006, SH2I_MOVLS0},
  {IFMT_nm, 0x000C, SH2I_MOVBL0},
  {IFMT_nm, 0x000D, SH2I_MOVWL0},
  {IFMT_nm, 0x000E, SH2I_MOVLL0},
  {IFMT_ni, 0xE000, SH2I_MOVI},
  {IFMT_nd8, 0x9000, SH2I_MOVWI},
  {IFMT_nd8, 0xD000, SH2I_MOVLI},
  {IFMT_d, 0xC400, SH2I_MOVBLG},
  {IFMT_d, 0xC500, SH2I_MOVWLG},
  {IFMT_d, 0xC600, SH2I_MOVLLG},
  {IFMT_d, 0xC000, SH2I_MOVBSG},
  {IFMT_d, 0xC100, SH2I_MOVWSG},
  {IFMT_d, 0xC200, SH2I_MOVLSG},
  {IFMT_nd4, 0x8000, SH2I_MOVBS4},
  {IFMT_nd4, 0x8100, SH2I_MOVWS4},
  {IFMT_nmd, 0x1000, SH2I_MOVLS4},
  {IFMT_md, 0x8400, SH2I_MOVBL4},
  {IFMT_md, 0x8500, SH2I_MOVWL4},
  {IFMT_nmd, 0x5000, SH2I_MOVLL4},
  {IFMT_d, 0xC700, SH2I_MOVA},
  {IFMT_n, 0x0029, SH2I_MOVT},
  {IFMT_nm, 0x0007, SH2I_MULL},
  {IFMT_nm, 0x200F, SH2I_MULS},
  {IFMT_nm, 0x200E, SH2I_MULU},
  {IFMT_nm, 0x600B, SH2I_NEG},
  {IFMT_nm, 0x600A, SH2I_NEGC},
  {IFMT_0, 0x0009, SH2I_NOP},
  {IFMT_nm, 0x6007, SH2I_NOT},
  {IFMT_nm, 0x200B, SH2I_OR},
  {IFMT_i, 0xCB00, SH2I_ORI},
  {IFMT_i, 0xCF00, SH2I_ORM},
  {IFMT_n, 0x4024, SH2I_ROTCL},
  {IFMT_n, 0x4025, SH2I_ROTCR},
  {IFMT_n, 0x4004, SH2I_ROTL},
  {IFMT_n, 0x4005, SH2I_ROTR},
  {IFMT_0, 0x002B, SH2I_RTE},
  {IFMT_0, 0x000B, SH2I_RTS},
  {IFMT_0, 0x0018, SH2I_SETT},
  {IFMT_n, 0x4020, SH2I_SHAL},
  {IFMT_n, 0x4021, SH2I_SHAR},
  {IFMT_n, 0x4000, SH2I_SHLL},
  {IFMT_n, 0x4008, SH2I_SHLL2},
  {IFMT_n, 0x4018, SH2I_SHLL8},
  {IFMT_n, 0x4028, SH2I_SHLL16},
  {IFMT_n, 0x4001, SH2I_SHLR},
  {IFMT_n, 0x4009, SH2I_SHLR2},
  {IFMT_n, 0x4019, SH2I_SHLR8},
  {IFMT_n, 0x4029, SH2I_SHLR16},
  {IFMT_0, 0x001B, SH2I_SLEEP},
  {IFMT_n, 0x0002, SH2I_STCSR},
  {IFMT_n, 0x0012, SH2I_STCGBR},
  {IFMT_n, 0x0022, SH2I_STCVBR},
  {IFMT_n, 0x4003, SH2I_STCMSR},
  {IFMT_n, 0x4013, SH2I_STCMGBR},
  {IFMT_n, 0x4023, SH2I_STCMVBR},
  {IFMT_n, 0x000A, SH2I_STSMACH},
  {IFMT_n, 0x001A, SH2I_STSMACL},
  {IFMT_n, 0x002A, SH2I_STSPR},
  {IFMT_n, 0x4002, SH2I_STSMMACH},
  {IFMT_n, 0x4012, SH2I_STSMMACL},
  {IFMT_n, 0x4022, SH2I_STSMPR},
  {IFMT_nm, 0x3008, SH2I_SUB},
  {IFMT_nm, 0x300A, SH2I_SUBC},
  {IFMT_nm, 0x300B, SH2I_SUBV},
  {IFMT_nm, 0x6008, SH2I_SWAPB},
  {IFMT_nm, 0x6009, SH2I_SWAPW},
  {IFMT_n, 0x401B, SH2I_TAS},
  {IFMT_i, 0xC300, SH2I_TRAPA},
  {IFMT_nm, 0x2008, SH2I_TST},
  {IFMT_i, 0xC800, SH2I_TSTI},
  {IFMT_i, 0xCC00, SH2I_TSTM},
  {IFMT_nm, 0x200A, SH2I_XOR},
  {IFMT_i, 0xCA00, SH2I_XORI},
  {IFMT_i, 0xCE00, SH2I_XORM},
  {IFMT_nm, 0x200D, SH2I_XTRCT},
  {0, 0x0000, NULL},
};

PACKED_OP_TAB InitOpDS[] = {
  {IFMT_nm, 0x300C, SH2I_ADD_DS},
  {IFMT_ni, 0x7000, SH2I_ADDI_DS},
  {IFMT_nm, 0x300E, SH2I_ADDC_DS},
  {IFMT_nm, 0x300F, SH2I_ADDV_DS},
  {IFMT_nm, 0x2009, SH2I_AND_DS},
  {IFMT_i, 0xC900, SH2I_ANDI_DS},
  {IFMT_i, 0xCD00, SH2I_ANDM_DS},
  {IFMT_0, 0x0028, SH2I_CLRMAC_DS},
  {IFMT_0, 0x0008, SH2I_CLRT_DS},
  {IFMT_nm, 0x3000, SH2I_CMPEQ_DS},
  {IFMT_nm, 0x3003, SH2I_CMPGE_DS},
  {IFMT_nm, 0x3007, SH2I_CMPGT_DS},
  {IFMT_nm, 0x3006, SH2I_CMPHI_DS},
  {IFMT_nm, 0x3002, SH2I_CMPHS_DS},
  {IFMT_n, 0x4015, SH2I_CMPPL_DS},
  {IFMT_n, 0x4011, SH2I_CMPPZ_DS},
  {IFMT_nm, 0x200C, SH2I_CMPSTR_DS},
  {IFMT_i, 0x8800, SH2I_CMPIM_DS},
  {IFMT_nm, 0x2007, SH2I_DIV0S_DS},
  {IFMT_0, 0x0019, SH2I_DIV0U_DS},
  {IFMT_nm, 0x3004, SH2I_DIV1_DS},
  {IFMT_nm, 0x300D, SH2I_DMULS_DS},
  {IFMT_nm, 0x3005, SH2I_DMULU_DS},
  {IFMT_n, 0x4010, SH2I_DT_DS},
  {IFMT_nm, 0x600E, SH2I_EXTSB_DS},
  {IFMT_nm, 0x600F, SH2I_EXTSW_DS},
  {IFMT_nm, 0x600C, SH2I_EXTUB_DS},
  {IFMT_nm, 0x600D, SH2I_EXTUW_DS},
  {IFMT_m, 0x400E, SH2I_LDCSR_DS},
  {IFMT_m, 0x401E, SH2I_LDCGBR_DS},
  {IFMT_m, 0x402E, SH2I_LDCVBR_DS},
  {IFMT_m, 0x4007, SH2I_LDCMSR_DS},
  {IFMT_m, 0x4017, SH2I_LDCMGBR_DS},
  {IFMT_m, 0x4027, SH2I_LDCMVBR_DS},
  {IFMT_m, 0x400A, SH2I_LDSMACH_DS},
  {IFMT_m, 0x401A, SH2I_LDSMACL_DS},
  {IFMT_m, 0x402A, SH2I_LDSPR_DS},
  {IFMT_m, 0x4006, SH2I_LDSMMACH_DS},
  {IFMT_m, 0x4016, SH2I_LDSMMACL_DS},
  {IFMT_m, 0x4026, SH2I_LDSMPR_DS},
  {IFMT_nm, 0x000F, SH2I_MACL_DS},
  {IFMT_nm, 0x400F, SH2I_MACW_DS},
  {IFMT_nm, 0x6003, SH2I_MOV_DS},
  {IFMT_nm, 0x2000, SH2I_MOVBS_DS},
  {IFMT_nm, 0x2001, SH2I_MOVWS_DS},
  {IFMT_nm, 0x2002, SH2I_MOVLS_DS},
  {IFMT_nm, 0x6000, SH2I_MOVBL_DS},
  {IFMT_nm, 0x6001, SH2I_MOVWL_DS},
  {IFMT_nm, 0x6002, SH2I_MOVLL_DS},
  {IFMT_nm, 0x2004, SH2I_MOVBM_DS},
  {IFMT_nm, 0x2005, SH2I_MOVWM_DS},
  {IFMT_nm, 0x2006, SH2I_MOVLM_DS},
  {IFMT_nm, 0x6004, SH2I_MOVBP_DS},
  {IFMT_nm, 0x6005, SH2I_MOVWP_DS},
  {IFMT_nm, 0x6006, SH2I_MOVLP_DS},
  {IFMT_nm, 0x0004, SH2I_MOVBS0_DS},
  {IFMT_nm, 0x0005, SH2I_MOVWS0_DS},
  {IFMT_nm, 0x0006, SH2I_MOVLS0_DS},
  {IFMT_nm, 0x000C, SH2I_MOVBL0_DS},
  {IFMT_nm, 0x000D, SH2I_MOVWL0_DS},
  {IFMT_nm, 0x000E, SH2I_MOVLL0_DS},
  {IFMT_ni, 0xE000, SH2I_MOVI_DS},
  {IFMT_nd8, 0x9000, SH2I_MOVWI_DS},
  {IFMT_nd8, 0xD000, SH2I_MOVLI_DS},
  {IFMT_d, 0xC400, SH2I_MOVBLG_DS},
  {IFMT_d, 0xC500, SH2I_MOVWLG_DS},
  {IFMT_d, 0xC600, SH2I_MOVLLG_DS},
  {IFMT_d, 0xC000, SH2I_MOVBSG_DS},
  {IFMT_d, 0xC100, SH2I_MOVWSG_DS},
  {IFMT_d, 0xC200, SH2I_MOVLSG_DS},
  {IFMT_nd4, 0x8000, SH2I_MOVBS4_DS},
  {IFMT_nd4, 0x8100, SH2I_MOVWS4_DS},
  {IFMT_nmd, 0x1000, SH2I_MOVLS4_DS},
  {IFMT_md, 0x8400, SH2I_MOVBL4_DS},
  {IFMT_md, 0x8500, SH2I_MOVWL4_DS},
  {IFMT_nmd, 0x5000, SH2I_MOVLL4_DS},
  {IFMT_d, 0xC700, SH2I_MOVA_DS},
  {IFMT_n, 0x0029, SH2I_MOVT_DS},
  {IFMT_nm, 0x0007, SH2I_MULL_DS},
  {IFMT_nm, 0x200F, SH2I_MULS_DS},
  {IFMT_nm, 0x200E, SH2I_MULU_DS},
  {IFMT_nm, 0x600B, SH2I_NEG_DS},
  {IFMT_nm, 0x600A, SH2I_NEGC_DS},
  {IFMT_0, 0x0009, SH2I_NOP_DS},
  {IFMT_nm, 0x6007, SH2I_NOT_DS},
  {IFMT_nm, 0x200B, SH2I_OR_DS},
  {IFMT_i, 0xCB00, SH2I_ORI_DS},
  {IFMT_i, 0xCF00, SH2I_ORM_DS},
  {IFMT_n, 0x4024, SH2I_ROTCL_DS},
  {IFMT_n, 0x4025, SH2I_ROTCR_DS},
  {IFMT_n, 0x4004, SH2I_ROTL_DS},
  {IFMT_n, 0x4005, SH2I_ROTR_DS},
  {IFMT_0, 0x0018, SH2I_SETT_DS},
  {IFMT_n, 0x4020, SH2I_SHAL_DS},
  {IFMT_n, 0x4021, SH2I_SHAR_DS},
  {IFMT_n, 0x4000, SH2I_SHLL_DS},
  {IFMT_n, 0x4008, SH2I_SHLL2_DS},
  {IFMT_n, 0x4018, SH2I_SHLL8_DS},
  {IFMT_n, 0x4028, SH2I_SHLL16_DS},
  {IFMT_n, 0x4001, SH2I_SHLR_DS},
  {IFMT_n, 0x4009, SH2I_SHLR2_DS},
  {IFMT_n, 0x4019, SH2I_SHLR8_DS},
  {IFMT_n, 0x4029, SH2I_SHLR16_DS},
  {IFMT_0, 0x001B, SH2I_SLEEP_DS},
  {IFMT_n, 0x0002, SH2I_STCSR_DS},
  {IFMT_n, 0x0012, SH2I_STCGBR_DS},
  {IFMT_n, 0x0022, SH2I_STCVBR_DS},
  {IFMT_n, 0x4003, SH2I_STCMSR_DS},
  {IFMT_n, 0x4013, SH2I_STCMGBR_DS},
  {IFMT_n, 0x4023, SH2I_STCMVBR_DS},
  {IFMT_n, 0x000A, SH2I_STSMACH_DS},
  {IFMT_n, 0x001A, SH2I_STSMACL_DS},
  {IFMT_n, 0x002A, SH2I_STSPR_DS},
  {IFMT_n, 0x4002, SH2I_STSMMACH_DS},
  {IFMT_n, 0x4012, SH2I_STSMMACL_DS},
  {IFMT_n, 0x4022, SH2I_STSMPR_DS},
  {IFMT_nm, 0x3008, SH2I_SUB_DS},
  {IFMT_nm, 0x300A, SH2I_SUBC_DS},
  {IFMT_nm, 0x300B, SH2I_SUBV_DS},
  {IFMT_nm, 0x6008, SH2I_SWAPB_DS},
  {IFMT_nm, 0x6009, SH2I_SWAPW_DS},
  {IFMT_n, 0x401B, SH2I_TAS_DS},
  {IFMT_nm, 0x2008, SH2I_TST_DS},
  {IFMT_i, 0xC800, SH2I_TSTI_DS},
  {IFMT_i, 0xCC00, SH2I_TSTM_DS},
  {IFMT_nm, 0x200A, SH2I_XOR_DS},
  {IFMT_i, 0xCA00, SH2I_XORI_DS},
  {IFMT_i, 0xCE00, SH2I_XORM_DS},
  {IFMT_nm, 0x200D, SH2I_XTRCT_DS},
  {0, 0x0000, NULL},
};


/* Default READ/WRITE callback */
/*******************************/


UINT8 FASTCALL
Def_READB (UINT32 adr)
{
  SH2_LOG1 ("SH2 read byte at %.8X\n", adr) return 0;
}

UINT16 FASTCALL
Def_READW (UINT32 adr)
{
  SH2_LOG1 ("SH2 read word at %.8X\n", adr) return 0;
}

UINT32 FASTCALL
Def_READL (UINT32 adr)
{
  SH2_LOG1 ("SH2 read long at %.8X\n", adr) return 0;
}

void FASTCALL
Def_WRITEB (UINT32 adr, UINT8 data)
{
SH2_LOG2 ("SH2 write byte %.2X at %.8X\n", data, adr)}

void FASTCALL
Def_WRITEW (UINT32 adr, UINT16 data)
{
SH2_LOG2 ("SH2 write word %.4X at %.8X\n", data, adr)}

void FASTCALL
Def_WRITEL (UINT32 adr, UINT32 data)
{
SH2_LOG2 ("SH2 write long %.8X at %.8X\n", data, adr)}



/* SH2 Initialisation C functions */
/**********************************/


// IMPORTANT : *Fetch have to be DWORD aligned !!!!

void
SH2_Add_Fetch (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
	       UINT16 * Fetch)
{
  UINT32 i;

  for (i = 0; i < 0x100; i++)
    {
      if (SH2->Fetch_Region[i].Fetch_Reg == NULL)
	{
	  SH2->Fetch_Region[i].Low_Adr = low_adr;
	  SH2->Fetch_Region[i].High_Adr = high_adr;
	  SH2->Fetch_Region[i].Fetch_Reg = &Fetch[0 - (low_adr >> 1)];
	  break;
	}
    }
}


// IMPORTANT : *Fetch have to be DWORD aligned !!!!

void
SH2_Set_Fetch_Reg (SH2_CONTEXT * SH2, UINT32 reg, UINT32 low_adr,
		   UINT32 high_adr, UINT16 * Fetch)
{
  reg &= 0xFF;

  SH2->Fetch_Region[reg].Low_Adr = low_adr;
  SH2->Fetch_Region[reg].High_Adr = high_adr;
  SH2->Fetch_Region[reg].Fetch_Reg = &Fetch[0 - (low_adr >> 1)];
}


void
SH2_Add_ReadB (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
	       SH2_RB * Func)
{
  UINT32 i;

  for (i = (low_adr & 0xFF); i <= (high_adr & 0xFF); i++)
    SH2->Read_Byte[i] = Func;
}


void
SH2_Add_ReadW (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
	       SH2_RW * Func)
{
  UINT32 i;

  for (i = (low_adr & 0xFF); i <= (high_adr & 0xFF); i++)
    SH2->Read_Word[i] = Func;
}


void
SH2_Add_ReadL (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
	       SH2_RL * Func)
{
  UINT32 i;

  for (i = (low_adr & 0xFF); i <= (high_adr & 0xFF); i++)
    SH2->Read_Long[i] = Func;
}


void
SH2_Add_WriteB (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
		SH2_WB * Func)
{
  UINT32 i;

  for (i = (low_adr & 0xFF); i <= (high_adr & 0xFF); i++)
    SH2->Write_Byte[i] = Func;
}


void
SH2_Add_WriteW (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
		SH2_WW * Func)
{
  UINT32 i;

  for (i = (low_adr & 0xFF); i <= (high_adr & 0xFF); i++)
    SH2->Write_Word[i] = Func;
}


void
SH2_Add_WriteL (SH2_CONTEXT * SH2, UINT32 low_adr, UINT32 high_adr,
		SH2_WL * Func)
{
  UINT32 i;

  for (i = (low_adr & 0xFF); i <= (high_adr & 0xFF); i++)
    SH2->Write_Long[i] = Func;
}


void
SH2_Map_Cache_Trough (SH2_CONTEXT * SH2)
{
  UINT32 i;

  for (i = 0; i < 0x10; i++)
    {
      SH2->Read_Byte[i + 0x20] = SH2->Read_Byte[i];
      SH2->Read_Word[i + 0x20] = SH2->Read_Word[i];
      SH2->Read_Long[i + 0x20] = SH2->Read_Long[i];
      SH2->Write_Byte[i + 0x20] = SH2->Write_Byte[i];
      SH2->Write_Word[i + 0x20] = SH2->Write_Word[i];
      SH2->Write_Long[i + 0x20] = SH2->Write_Long[i];
    }
}


void
SH2_Init (SH2_CONTEXT * SH2, UINT32 slave)
{
  static UINT32 SH2_Initialised = 0;
  INT32 i, j, m, q, s, t, il;
  void *OP_Table_Temp[0x10000];

  if (!SH2_Initialised)
    {

#if SH2_DEBUG > 0
      SH2_Debug = fopen ("sh2.log", "wb");
#endif

      SH2_LOG0 ("Starting to log :\n\n");

      // Unpacking the 'normal' opcode jump table

      for (i = 0; i < 0x10000; i++)
	OP_Table_Temp[i] = SH2I_ILLEGAL;

      i = -1;

      while (InitOp[++i].Func != NULL)
	{
	  switch (InitOp[i].IFMT)
	    {
	    case IFMT_0:
	      OP_Table_Temp[InitOp[i].Mask] = InitOp[i].Func;
	      break;

	    case IFMT_n:
	    case IFMT_m:
	      for (j = 0; j < 0x10; j++)
		{
		  OP_Table_Temp[InitOp[i].Mask | (j << 8)] = InitOp[i].Func;
		}
	      break;

	    case IFMT_nm:
	      for (j = 0; j < 0x100; j++)
		{
		  OP_Table_Temp[InitOp[i].Mask | (j << 4)] = InitOp[i].Func;
		}
	      break;

	    case IFMT_md:
	    case IFMT_nd4:
	    case IFMT_d:
	    case IFMT_i:
	      for (j = 0; j < 0x100; j++)
		{
		  OP_Table_Temp[InitOp[i].Mask | j] = InitOp[i].Func;
		}
	      break;

	    case IFMT_nmd:
	    case IFMT_d12:
	    case IFMT_nd8:
	    case IFMT_ni:
	      for (j = 0; j < 0x1000; j++)
		{
		  OP_Table_Temp[InitOp[i].Mask | j] = InitOp[i].Func;
		}
	      break;
	    }
	}

      // Big Indian Opcode, we need to "byte-swap" the opcode table pointer

      for (i = 0; i < 0x10000; i++)
	{
	  j = (i >> 8) & 0xFF;
	  j |= (i & 0xFF) << 8;

	  OP_Table[j] = OP_Table_Temp[i];
	}

      // Now we unpack the 'Delay Slot' opcode jump table

      for (i = 0; i < 0x10000; i++)
	OP_Table_Temp[i] = SH2I_ILLEGAL_DS;

      i = -1;

      while (InitOpDS[++i].Func != NULL)
	{
	  switch (InitOpDS[i].IFMT)
	    {
	    case IFMT_0:
	      OP_Table_Temp[InitOpDS[i].Mask] = InitOpDS[i].Func;
	      break;

	    case IFMT_n:
	    case IFMT_m:
	      for (j = 0; j < 0x10; j++)
		{
		  OP_Table_Temp[InitOpDS[i].Mask | (j << 8)] =
		    InitOpDS[i].Func;
		}
	      break;

	    case IFMT_nm:
	      for (j = 0; j < 0x100; j++)
		{
		  OP_Table_Temp[InitOpDS[i].Mask | (j << 4)] =
		    InitOpDS[i].Func;
		}
	      break;

	    case IFMT_md:
	    case IFMT_nd4:
	    case IFMT_d:
	    case IFMT_i:
	      for (j = 0; j < 0x100; j++)
		{
		  OP_Table_Temp[InitOpDS[i].Mask | j] = InitOpDS[i].Func;
		}
	      break;

	    case IFMT_nmd:
	    case IFMT_d12:
	    case IFMT_nd8:
	    case IFMT_ni:
	      for (j = 0; j < 0x1000; j++)
		{
		  OP_Table_Temp[InitOpDS[i].Mask | j] = InitOpDS[i].Func;
		}
	      break;
	    }
	}

      // Big Indian Opcode, we need to "byte-swap" the opcode table pointer

      for (i = 0; i < 0x10000; i++)
	{
	  j = (i >> 8) & 0xFF;
	  j |= (i & 0xFF) << 8;

	  OPDS_Table[j] = OP_Table_Temp[i];
	}

      // Fast SR Set table initialisation

      for (i = 0; i < 0x400; i++)
	{
	  t = (i & 0x001) >> 0;
	  s = (i & 0x002) >> 1;
	  q = (i & 0x100) >> 8;
	  m = (i & 0x200) >> 9;
	  il = (i & 0x0F0) >> 4;

	  Set_SR_Table[i] =
	    (t << 0) | (s << 8) | (il << 16) | (m << 25) | (q << 24);

	  SH2_LOG2 ("SR Table %.3X = %.8X\n", i, Set_SR_Table[i]);
	}

      SH2_Initialised = 1;
    }

  memset (SH2, 0, sizeof (SH2_CONTEXT));

  // Default initialisation memory...

  SH2_Add_ReadB (SH2, 0x00, 0xFF, Def_READB);
  SH2_Add_ReadW (SH2, 0x00, 0xFF, Def_READW);
  SH2_Add_ReadL (SH2, 0x00, 0xFF, Def_READL);
  SH2_Add_WriteB (SH2, 0x00, 0xFF, Def_WRITEB);
  SH2_Add_WriteW (SH2, 0x00, 0xFF, Def_WRITEW);
  SH2_Add_WriteL (SH2, 0x00, 0xFF, Def_WRITEL);

  SH2_Add_ReadB (SH2, 0xC0, 0xC0, SH2_Read_Byte_C0);
  SH2_Add_ReadW (SH2, 0xC0, 0xC0, SH2_Read_Word_C0);
  SH2_Add_ReadL (SH2, 0xC0, 0xC0, SH2_Read_Long_C0);
  SH2_Add_WriteB (SH2, 0xC0, 0xC0, SH2_Write_Byte_C0);
  SH2_Add_WriteW (SH2, 0xC0, 0xC0, SH2_Write_Word_C0);
  SH2_Add_WriteL (SH2, 0xC0, 0xC0, SH2_Write_Long_C0);

  SH2_Add_ReadB (SH2, 0xFF, 0xFF, SH2_Read_Byte_FF);
  SH2_Add_ReadW (SH2, 0xFF, 0xFF, SH2_Read_Word_FF);
  SH2_Add_ReadL (SH2, 0xFF, 0xFF, SH2_Read_Long_FF);
  SH2_Add_WriteB (SH2, 0xFF, 0xFF, SH2_Write_Byte_FF);
  SH2_Add_WriteW (SH2, 0xFF, 0xFF, SH2_Write_Word_FF);
  SH2_Add_WriteL (SH2, 0xFF, 0xFF, SH2_Write_Long_FF);

  SH2->WDT_Tab[0] = 16 - 1;
  SH2->WDT_Tab[1] = 16 - 6;
  SH2->WDT_Tab[2] = 16 - 7;
  SH2->WDT_Tab[3] = 16 - 8;
  SH2->WDT_Tab[4] = 16 - 9;
  SH2->WDT_Tab[5] = 16 - 10;
  SH2->WDT_Tab[6] = 16 - 12;
  SH2->WDT_Tab[7] = 16 - 13;

  SH2->FRT_Tab[0] = 8 - 3;
  SH2->FRT_Tab[1] = 8 - 5;
  SH2->FRT_Tab[2] = 8 - 7;
  SH2->FRT_Tab[3] = 0;		// not supported !

  SH2->BCR1 = 0x03F0;
  if (slave)
    SH2->BCR1 |= 0x8000;
}
