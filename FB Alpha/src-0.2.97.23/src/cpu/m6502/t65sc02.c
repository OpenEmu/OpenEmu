/*****************************************************************************
 *
 *   tbl65sc02.c
 *   65sc02 opcode functions and function pointer table
 *
 *   Copyright Peter Trauner, all rights reserved.
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
/* 4. February 2000 PeT fixed relative word operand */

/* opcode already in 65c02
   although docu says they were introduced with this cpu

 bbr
 bbs
 rmb
 smb
 trb
 tsb
*/

#undef	OP
#define OP(nn) M6502_INLINE void m65sc02_##nn()

/*****************************************************************************
 *****************************************************************************
 *
 *   overrides for 65SC02 opcodes
 *
 *****************************************************************************
 * op    temp     cycles             rdmem   opc  wrmem   ********************/
OP(63) { BSR;                                             } /* 5? BSR */

static void (*const insn65sc02[0x100])() = {
	m65c02_00,m65c02_01,m65c02_02,m65c02_03,m65c02_04,m65c02_05,m65c02_06,m65c02_07,
	m65c02_08,m65c02_09,m65c02_0a,m65c02_0b,m65c02_0c,m65c02_0d,m65c02_0e,m65c02_0f,
	m65c02_10,m65c02_11,m65c02_12,m65c02_13,m65c02_14,m65c02_15,m65c02_16,m65c02_17,
	m65c02_18,m65c02_19,m65c02_1a,m65c02_1b,m65c02_1c,m65c02_1d,m65c02_1e,m65c02_1f,
	m65c02_20,m65c02_21,m65c02_22,m65c02_23,m65c02_24,m65c02_25,m65c02_26,m65c02_27,
	m65c02_28,m65c02_29,m65c02_2a,m65c02_2b,m65c02_2c,m65c02_2d,m65c02_2e,m65c02_2f,
	m65c02_30,m65c02_31,m65c02_32,m65c02_33,m65c02_34,m65c02_35,m65c02_36,m65c02_37,
	m65c02_38,m65c02_39,m65c02_3a,m65c02_3b,m65c02_3c,m65c02_3d,m65c02_3e,m65c02_3f,
	m65c02_40,m65c02_41,m65c02_42,m65c02_43,m65c02_44,m65c02_45,m65c02_46,m65c02_47,
	m65c02_48,m65c02_49,m65c02_4a,m65c02_4b,m65c02_4c,m65c02_4d,m65c02_4e,m65c02_4f,
	m65c02_50,m65c02_51,m65c02_52,m65c02_53,m65c02_54,m65c02_55,m65c02_56,m65c02_57,
	m65c02_58,m65c02_59,m65c02_5a,m65c02_5b,m65c02_5c,m65c02_5d,m65c02_5e,m65c02_5f,
	m65c02_60,m65c02_61,m65c02_62,m65c02_63,m65c02_64,m65c02_65,m65c02_66,m65c02_67,
	m65c02_68,m65c02_69,m65c02_6a,m65c02_6b,m65c02_6c,m65c02_6d,m65c02_6e,m65c02_6f,
	m65c02_70,m65c02_71,m65c02_72,m65c02_73,m65c02_74,m65c02_75,m65c02_76,m65c02_77,
	m65c02_78,m65c02_79,m65c02_7a,m65c02_7b,m65c02_7c,m65c02_7d,m65c02_7e,m65c02_7f,
	m65c02_80,m65c02_81,m65c02_82,m65c02_83,m65c02_84,m65c02_85,m65c02_86,m65c02_87,
	m65c02_88,m65c02_89,m65c02_8a,m65c02_8b,m65c02_8c,m65c02_8d,m65c02_8e,m65c02_8f,
	m65c02_90,m65c02_91,m65c02_92,m65c02_93,m65c02_94,m65c02_95,m65c02_96,m65c02_97,
	m65c02_98,m65c02_99,m65c02_9a,m65c02_9b,m65c02_9c,m65c02_9d,m65c02_9e,m65c02_9f,
	m65c02_a0,m65c02_a1,m65c02_a2,m65c02_a3,m65c02_a4,m65c02_a5,m65c02_a6,m65c02_a7,
	m65c02_a8,m65c02_a9,m65c02_aa,m65c02_ab,m65c02_ac,m65c02_ad,m65c02_ae,m65c02_af,
	m65c02_b0,m65c02_b1,m65c02_b2,m65c02_b3,m65c02_b4,m65c02_b5,m65c02_b6,m65c02_b7,
	m65c02_b8,m65c02_b9,m65c02_ba,m65c02_bb,m65c02_bc,m65c02_bd,m65c02_be,m65c02_bf,
	m65c02_c0,m65c02_c1,m65c02_c2,m65c02_c3,m65c02_c4,m65c02_c5,m65c02_c6,m65c02_c7,
	m65c02_c8,m65c02_c9,m65c02_ca,m65c02_cb,m65c02_cc,m65c02_cd,m65c02_ce,m65c02_cf,
	m65c02_d0,m65c02_d1,m65c02_d2,m65c02_d3,m65c02_d4,m65c02_d5,m65c02_d6,m65c02_d7,
	m65c02_d8,m65c02_d9,m65c02_da,m65c02_db,m65c02_dc,m65c02_dd,m65c02_de,m65c02_df,
	m65c02_e0,m65c02_e1,m65c02_e2,m65c02_e3,m65c02_e4,m65c02_e5,m65c02_e6,m65c02_e7,
	m65c02_e8,m65c02_e9,m65c02_ea,m65c02_eb,m65c02_ec,m65c02_ed,m65c02_ee,m65c02_ef,
	m65c02_f0,m65c02_f1,m65c02_f2,m65c02_f3,m65c02_f4,m65c02_f5,m65c02_f6,m65c02_f7,
	m65c02_f8,m65c02_f9,m65c02_fa,m65c02_fb,m65c02_fc,m65c02_fd,m65c02_fe,m65c02_ff
};

