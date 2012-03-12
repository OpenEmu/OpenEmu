/*****************************************************************************
 *
 *   tbl2a03.c
 *   2a03 opcode functions and function pointer table
 *
 *   The 2a03 is a 6502 CPU that does not support the decimal mode
 *   of the ADC and SBC instructions, so all opcodes except ADC/SBC
 *   are simply mapped to the m6502 ones.
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

/*
  based on the nmos 6502

  b flag handling might be changed,
  although only nmos series b-flag handling is quite sure
*/


#undef	OP
#define OP(nn) M6502_INLINE void n2a03_##nn(void)

/*****************************************************************************
 *****************************************************************************
 *
 *   overrides for 2a03 opcodes
 *
 *****************************************************************************
 ********** insn   temp     cycles             rdmem   opc  wrmem   **********/
OP(61) { int tmp; RD_IDX; ADC_NES;	  } /* 6 ADC IDX */
OP(e1) { int tmp; RD_IDX; SBC_NES;	  } /* 6 SBC IDX */
OP(71) { int tmp; RD_IDY_P; ADC_NES;	  } /* 5 ADC IDY page penalty */
OP(f1) { int tmp; RD_IDY_P; SBC_NES;	  } /* 5 SBC IDY page penalty */
OP(63) { int tmp; RD_IDX; WB_EA; RRA_NES; WB_EA;   } /* 7 RRA IDX */
OP(73) { int tmp; RD_IDY_NP; WB_EA; RRA_NES; WB_EA; } /* 7 RRA IDY */
OP(e3) { int tmp; RD_IDX; WB_EA; ISB_NES; WB_EA;   } /* 7 ISB IDX */
OP(f3) { int tmp; RD_IDY_NP; WB_EA; ISB_NES; WB_EA; } /* 7 ISB IDY */
OP(65) { int tmp; RD_ZPG; ADC_NES;	  } /* 3 ADC ZPG */
OP(e5) { int tmp; RD_ZPG; SBC_NES;	  } /* 3 SBC ZPG */
OP(75) { int tmp; RD_ZPX; ADC_NES;	  } /* 4 ADC ZPX */
OP(f5) { int tmp; RD_ZPX; SBC_NES;	  } /* 4 SBC ZPX */
OP(67) { int tmp; RD_ZPG; WB_EA; RRA_NES; WB_EA;    } /* 5 RRA ZPG */
OP(77) { int tmp; RD_ZPX; WB_EA; RRA_NES; WB_EA;    } /* 6 RRA ZPX */
OP(e7) { int tmp; RD_ZPG; WB_EA; ISB_NES; WB_EA;    } /* 5 ISB ZPG */
OP(f7) { int tmp; RD_ZPX; WB_EA; ISB_NES; WB_EA;    } /* 6 ISB ZPX */
OP(69) { int tmp; RD_IMM; ADC_NES;	  } /* 2 ADC IMM */
OP(e9) { int tmp; RD_IMM; SBC_NES;	  } /* 2 SBC IMM */
OP(79) { int tmp; RD_ABY_P; ADC_NES;	  } /* 4 ADC ABY page penalty */
OP(f9) { int tmp; RD_ABY_P; SBC_NES;	  } /* 4 SBC ABY page penalty */
OP(6b) { int tmp; RD_IMM; ARR_NES; WB_ACC; } /* 2 ARR IMM */
OP(7b) { int tmp; RD_ABY_NP; WB_EA; RRA_NES; WB_EA; } /* 7 RRA ABY */
OP(ab) { int tmp; RD_IMM; OAL_NES;                  } /* 2 OAL IMM */
OP(eb) { int tmp; RD_IMM; SBC_NES;		  } /* 2 SBC IMM */
OP(fb) { int tmp; RD_ABY_NP; WB_EA; ISB_NES; WB_EA; } /* 7 ISB ABY */
OP(9c) { int tmp; EA_ABX_NP; SYH_NES; WB_EA;     } /* 5 SYH ABX */
OP(6d) { int tmp; RD_ABS; ADC_NES;	  } /* 4 ADC ABS */
OP(ed) { int tmp; RD_ABS; SBC_NES;	  } /* 4 SBC ABS */
OP(7d) { int tmp; RD_ABX_P; ADC_NES;	  } /* 4 ADC ABX page penalty */
OP(fd) { int tmp; RD_ABX_P; SBC_NES;	  } /* 4 SBC ABX page penalty */
OP(9e) { int tmp; EA_ABY_NP; SXH_NES; WB_EA;    } /* 5 SXH ABY */
OP(6f) { int tmp; RD_ABS; WB_EA; RRA_NES; WB_EA;    } /* 6 RRA ABS */
OP(7f) { int tmp; RD_ABX_NP; WB_EA; RRA_NES; WB_EA; } /* 7 RRA ABX */
OP(ef) { int tmp; RD_ABS; WB_EA; ISB_NES; WB_EA;    } /* 6 ISB ABS */
OP(ff) { int tmp; RD_ABX_NP; WB_EA; ISB_NES; WB_EA; } /* 7 ISB ABX */


static void (*const insn2a03[0x100])(void) = {
	m6502_00,m6502_01,m6502_02,m6502_03,m6502_04,m6502_05,m6502_06,m6502_07,
	m6502_08,m6502_09,m6502_0a,m6502_0b,m6502_0c,m6502_0d,m6502_0e,m6502_0f,
	m6502_10,m6502_11,m6502_12,m6502_13,m6502_14,m6502_15,m6502_16,m6502_17,
	m6502_18,m6502_19,m6502_1a,m6502_1b,m6502_1c,m6502_1d,m6502_1e,m6502_1f,
	m6502_20,m6502_21,m6502_22,m6502_23,m6502_24,m6502_25,m6502_26,m6502_27,
	m6502_28,m6502_29,m6502_2a,m6502_2b,m6502_2c,m6502_2d,m6502_2e,m6502_2f,
	m6502_30,m6502_31,m6502_32,m6502_33,m6502_34,m6502_35,m6502_36,m6502_37,
	m6502_38,m6502_39,m6502_3a,m6502_3b,m6502_3c,m6502_3d,m6502_3e,m6502_3f,
	m6502_40,m6502_41,m6502_42,m6502_43,m6502_44,m6502_45,m6502_46,m6502_47,
	m6502_48,m6502_49,m6502_4a,m6502_4b,m6502_4c,m6502_4d,m6502_4e,m6502_4f,
	m6502_50,m6502_51,m6502_52,m6502_53,m6502_54,m6502_55,m6502_56,m6502_57,
	m6502_58,m6502_59,m6502_5a,m6502_5b,m6502_5c,m6502_5d,m6502_5e,m6502_5f,
	m6502_60,n2a03_61,m6502_62,n2a03_63,m6502_64,n2a03_65,m6502_66,n2a03_67,
	m6502_68,n2a03_69,m6502_6a,n2a03_6b,m6502_6c,n2a03_6d,m6502_6e,n2a03_6f,
	m6502_70,n2a03_71,m6502_72,n2a03_73,m6502_74,n2a03_75,m6502_76,n2a03_77,
	m6502_78,n2a03_79,m6502_7a,n2a03_7b,m6502_7c,n2a03_7d,m6502_7e,n2a03_7f,
	m6502_80,m6502_81,m6502_82,m6502_83,m6502_84,m6502_85,m6502_86,m6502_87,
	m6502_88,m6502_89,m6502_8a,m6502_8b,m6502_8c,m6502_8d,m6502_8e,m6502_8f,
	m6502_90,m6502_91,m6502_92,m6502_93,m6502_94,m6502_95,m6502_96,m6502_97,
	m6502_98,m6502_99,m6502_9a,m6502_9b,n2a03_9c,m6502_9d,n2a03_9e,m6502_9f,
	m6502_a0,m6502_a1,m6502_a2,m6502_a3,m6502_a4,m6502_a5,m6502_a6,m6502_a7,
	m6502_a8,m6502_a9,m6502_aa,n2a03_ab,m6502_ac,m6502_ad,m6502_ae,m6502_af,
	m6502_b0,m6502_b1,m6502_b2,m6502_b3,m6502_b4,m6502_b5,m6502_b6,m6502_b7,
	m6502_b8,m6502_b9,m6502_ba,m6502_bb,m6502_bc,m6502_bd,m6502_be,m6502_bf,
	m6502_c0,m6502_c1,m6502_c2,m6502_c3,m6502_c4,m6502_c5,m6502_c6,m6502_c7,
	m6502_c8,m6502_c9,m6502_ca,m6502_cb,m6502_cc,m6502_cd,m6502_ce,m6502_cf,
	m6502_d0,m6502_d1,m6502_d2,m6502_d3,m6502_d4,m6502_d5,m6502_d6,m6502_d7,
	m6502_d8,m6502_d9,m6502_da,m6502_db,m6502_dc,m6502_dd,m6502_de,m6502_df,
	m6502_e0,n2a03_e1,m6502_e2,n2a03_e3,m6502_e4,n2a03_e5,m6502_e6,n2a03_e7,
	m6502_e8,n2a03_e9,m6502_ea,n2a03_eb,m6502_ec,n2a03_ed,m6502_ee,n2a03_ef,
	m6502_f0,n2a03_f1,m6502_f2,n2a03_f3,m6502_f4,n2a03_f5,m6502_f6,n2a03_f7,
	m6502_f8,n2a03_f9,m6502_fa,n2a03_fb,m6502_fc,n2a03_fd,m6502_fe,n2a03_ff
};
