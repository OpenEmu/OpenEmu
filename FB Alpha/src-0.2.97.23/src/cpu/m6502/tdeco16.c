/*****************************************************************************
 *  Deco CPU16 instructions - still very work in progress!
 *
 *****************************************************************************/

#define DECO16_VERBOSE 0

#undef	OP
#define OP(nn) M6502_INLINE void deco16_##nn()

#define DECO16_BRK									\
	RDOPARG();										\
	PUSH(PCH);										\
	PUSH(PCL);										\
	PUSH(P | F_B);									\
	P = (P | F_I);									\
	PCL = RDMEM(DECO16_IRQ_VEC+1);					\
	PCH = RDMEM(DECO16_IRQ_VEC);


/*****************************************************************************
 *****************************************************************************
 *
 *   overrides for 6502 opcodes
 *
 *****************************************************************************
 * op    temp     cycles             rdmem   opc  wrmem   ********************/
OP(00) { DECO16_BRK; } /* 7 BRK */
#define deco16_20 m6502_20								/* 6 JSR ABS */
#define deco16_40 m6502_40								/* 6 RTI */
#define deco16_60 m6502_60								/* 6 RTS */
OP(80) { RD_DUM; ILL; }									/* 2 ILL */
#define deco16_a0 m6502_a0								/* 2 LDY IMM */
#define deco16_c0 m6502_c0								/* 2 CPY IMM */
#define deco16_e0 m6502_e0								/* 2 CPX IMM */

#define deco16_10 m6502_10								/* 2 BPL */
#define deco16_30 m6502_30								/* 2 BMI */
#define deco16_50 m6502_50								/* 2 BVC */
#define deco16_70 m6502_70								/* 2 BVS */
#define deco16_90 m6502_90								/* 2 BCC */
#define deco16_b0 m6502_b0								/* 2 BCS */
#define deco16_d0 m6502_d0								/* 2 BNE */
#define deco16_f0 m6502_f0								/* 2 BEQ */

#define deco16_01 m6502_01								/* 6 ORA IDX */
#define deco16_21 m6502_21								/* 6 AND IDX */
#define deco16_41 m6502_41								/* 6 EOR IDX */
#define deco16_61 m6502_61								/* 6 ADC IDX */
#define deco16_81 m6502_81								/* 6 STA IDX */
#define deco16_a1 m6502_a1								/* 6 LDA IDX */
#define deco16_c1 m6502_c1								/* 6 CMP IDX */
#define deco16_e1 m6502_e1								/* 6 SBC IDX */

#define deco16_11 m6502_11								/* 5 ORA IDY; */
#define deco16_31 m6502_31								/* 5 AND IDY; */
#define deco16_51 m6502_51								/* 5 EOR IDY; */
#define deco16_71 m6502_71								/* 5 ADC IDY; */
#define deco16_91 m6502_91								/* 6 STA IDY; */
#define deco16_b1 m6502_b1								/* 5 LDA IDY; */
#define deco16_d1 m6502_d1								/* 5 CMP IDY; */
#define deco16_f1 m6502_f1								/* 5 SBC IDY; */

OP(02) { RD_DUM; ILL; } 								/* 2 ILL */
OP(22) { RD_DUM; ILL; } 								/* 2 ILL */
OP(42) { RD_DUM; ILL; } 								/* 2 ILL */
OP(62) { RD_DUM; ILL; } 								/* 2 ILL */
OP(82) { RD_DUM; ILL; } 								/* 2 ILL */
#define deco16_a2 m6502_a2								/* 2 LDX IMM */
OP(c2) { RD_DUM; ILL; } 								/* 2 ILL */
OP(e2) { RD_DUM; ILL; } 								/* 2 ILL */

OP(12) { RD_DUM; ILL; }									/* 2 ILL / 3 ora zpi ?? */
OP(32) { RD_DUM; ILL; }									/* 2 ILL / 3 and zpi ?? */
OP(52) { RD_DUM; ILL; }									/* 2 ILL / 3 eor zpi ?? */
OP(72) { RD_DUM; ILL; }									/* 2 ILL / 3 adc zpi ?? */
OP(92) { RD_DUM; ILL; }									/* 2 ILL / 3 sta zpi ?? */
OP(b2) { RD_DUM; ILL; }									/* 2 ILL / 3 lda zpi ?? */
OP(d2) { RD_DUM; ILL; }									/* 2 ILL / 3 cmp zpi ?? */
OP(f2) { RD_DUM; ILL; }									/* 2 ILL / 3 sbc zpi ?? */

OP(03) { RD_DUM; ILL; } 								/* 2 ILL */
OP(23) {
	int tmp;

	m6502_ICount -= 1;
	RD_IMM;

	//if (DECO16_VERBOSE)
	//	logerror("%04x: OP23 %02x\n",PCW,tmp);
}
OP(43) { RD_DUM; ILL; } 								/* 2 ILL */
OP(63) {
	int tmp;

	m6502_ICount -= 1;
	RD_IMM;

	//if (DECO16_VERBOSE)
	//	logerror("%04x: OP63 %02x\n",PCW,tmp);
}
OP(83) { RD_DUM; ILL; } 								/* 2 ILL */
OP(a3) {
	int tmp;

	m6502_ICount -= 1;
	RD_IMM;

	//if (DECO16_VERBOSE)
	//	logerror("%04x: OPA3 %02x\n",PCW,tmp);
}
OP(c3) { RD_DUM; ILL; } 								/* 2 ILL */
OP(e3) { RD_DUM; ILL; } 								/* 2 ILL */

OP(13) { int tmp; m6502_ICount -= 1; RD_IMM;

	//if (DECO16_VERBOSE)
	//	logerror("%04x: OP13 %02x\n",PCW,tmp);

//bank select control?

				} /*  */
OP(33) { RD_DUM; ILL; } 								/* 2 ILL */
OP(53) { RD_DUM; ILL; } 								/* 2 ILL */
OP(73) { RD_DUM; ILL; } 								/* 2 ILL */
OP(93) { RD_DUM; ILL; } 								/* 2 ILL */
OP(b3) { RD_DUM; ILL; } 								/* 2 ILL */
OP(d3) { RD_DUM; ILL; } 								/* 2 ILL */
OP(f3) { RD_DUM; ILL; } 								/* 2 ILL */

OP(04) { RD_DUM; ILL; }									/* 2 ILL / 3 tsb zpg ?? */
#define deco16_24 m6502_24								/* 3 BIT ZPG */
OP(44) { RD_DUM; ILL; } 								/* 2 ILL */
OP(64) { RD_DUM; ILL; }									/* 2 ILL / 3 stz zpg ?? */
#define deco16_84 m6502_84								/* 3 STY ZPG */
#define deco16_a4 m6502_a4								/* 3 LDY ZPG */
#define deco16_c4 m6502_c4								/* 3 CPY ZPG */
#define deco16_e4 m6502_e4								/* 3 CPX ZPG */

OP(14) { RD_DUM; ILL; }									/* 2 ILL / 3 trb zpg ?? */
OP(34) { RD_DUM; ILL; }									/* 2 ILL / 4 bit zpx ?? */
OP(54) { RD_DUM; ILL; } 								/* 2 ILL */
OP(74) { RD_DUM; ILL; }									/* 2 ILL / 4 stz zpx ?? */
#define deco16_94 m6502_94								/* 4 sty zpx */
#define deco16_b4 m6502_b4								/* 4 ldy zpx */
OP(d4) { RD_DUM; ILL; } 								/* 2 ILL */
OP(f4) { RD_DUM; ILL; }									/* 2 ILL */

#define deco16_05 m6502_05								/* 3 ORA ZPG */
#define deco16_25 m6502_25								/* 3 AND ZPG */
#define deco16_45 m6502_45								/* 3 EOR ZPG */
#define deco16_65 m6502_65								/* 3 ADC ZPG */
#define deco16_85 m6502_85								/* 3 STA ZPG */
#define deco16_a5 m6502_a5								/* 3 LDA ZPG */
#define deco16_c5 m6502_c5								/* 3 CMP ZPG */
#define deco16_e5 m6502_e5								/* 3 SBC ZPG */

#define deco16_15 m6502_15								/* 4 ORA ZPX */
#define deco16_35 m6502_35								/* 4 AND ZPX */
#define deco16_55 m6502_55								/* 4 EOR ZPX */
#define deco16_75 m6502_75								/* 4 ADC ZPX */
#define deco16_95 m6502_95								/* 4 STA ZPX */
#define deco16_b5 m6502_b5								/* 4 LDA ZPX */
#define deco16_d5 m6502_d5								/* 4 CMP ZPX */
#define deco16_f5 m6502_f5								/* 4 SBC ZPX */

#define deco16_06 m6502_06								/* 5 ASL ZPG */
#define deco16_26 m6502_26								/* 5 ROL ZPG */
#define deco16_46 m6502_46								/* 5 LSR ZPG */
#define deco16_66 m6502_66								/* 5 ROR ZPG */
#define deco16_86 m6502_86								/* 3 STX ZPG */
#define deco16_a6 m6502_a6								/* 3 LDX ZPG */
#define deco16_c6 m6502_c6								/* 5 DEC ZPG */
#define deco16_e6 m6502_e6								/* 5 INC ZPG */

#define deco16_16 m6502_16								/* 6 ASL ZPX */
#define deco16_36 m6502_36								/* 6 ROL ZPX */
#define deco16_56 m6502_56								/* 6 LSR ZPX */
#define deco16_76 m6502_76								/* 6 ROR ZPX */
#define deco16_96 m6502_96								/* 4 STX ZPY */
#define deco16_b6 m6502_b6								/* 4 LDX ZPY */
#define deco16_d6 m6502_d6								/* 6 DEC ZPX */
#define deco16_f6 m6502_f6								/* 6 INC ZPX */

OP(07) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB0 ZPG ?? */
OP(27) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB2 ZPG ?? */
OP(47) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB4 ZPG ?? */
OP(67) {
	RD_IMM_DISCARD;
	m6502.a=M6502ReadPort(0);

//  logerror("%04x: VBL (0x67)\n",PCW);

// really - wait for status?

} /*  */
OP(87) { int tmp; m6502_ICount -= 1; RD_IMM;
	//logerror("%04x: OP87 %02x\n",PCW,tmp);

} /*  */
OP(a7) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB2 ZPG ?? */
OP(c7) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB4 ZPG ?? */
OP(e7) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB6 ZPG ?? */

OP(17) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB1 ZPG ?? */
OP(37) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB3 ZPG ?? */
OP(57) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB5 ZPG ?? */
OP(77) { RD_DUM; ILL; }									/* 2 ILL / 5 RMB7 ZPG ?? */
OP(97) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB1 ZPG ?? */
OP(b7) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB3 ZPG ?? */
OP(d7) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB5 ZPG ?? */
OP(f7) { RD_DUM; ILL; }									/* 2 ILL / 5 SMB7 ZPG ?? */

#define deco16_08 m6502_08								/* 3 PHP */
#define deco16_28 m6502_28								/* 4 PLP */
#define deco16_48 m6502_48								/* 3 PHA */
#define deco16_68 m6502_68								/* 4 PLA */
#define deco16_88 m6502_88								/* 2 DEY */
#define deco16_a8 m6502_a8								/* 2 TAY */
#define deco16_c8 m6502_c8								/* 2 INY */
#define deco16_e8 m6502_e8								/* 2 INX */

#define deco16_18 m6502_18								/* 2 CLC */
#define deco16_38 m6502_38								/* 2 SEC */
#define deco16_58 m6502_58								/* 2 CLI */
#define deco16_78 m6502_78								/* 2 SEI */
#define deco16_98 m6502_98								/* 2 TYA */
#define deco16_b8 m6502_b8								/* 2 CLV */
#define deco16_d8 m6502_d8								/* 2 CLD */
#define deco16_f8 m6502_f8								/* 2 SED */

#define deco16_09 m6502_09								/* 2 ORA IMM */
#define deco16_29 m6502_29								/* 2 AND IMM */
#define deco16_49 m6502_49								/* 2 EOR IMM */
#define deco16_69 m6502_69								/* 2 ADC IMM */
#define deco16_89 m65c02_89								/* 2 BIT IMM */
#define deco16_a9 m6502_a9								/* 2 LDA IMM */
#define deco16_c9 m6502_c9								/* 2 CMP IMM */
#define deco16_e9 m6502_e9								/* 2 SBC IMM */

#define deco16_19 m6502_19								/* 4 ORA ABY */
#define deco16_39 m6502_39								/* 4 AND ABY */
#define deco16_59 m6502_59								/* 4 EOR ABY */
#define deco16_79 m6502_79								/* 4 ADC ABY */
#define deco16_99 m6502_99								/* 5 STA ABY */
#define deco16_b9 m6502_b9								/* 4 LDA ABY */
#define deco16_d9 m6502_d9								/* 4 CMP ABY */
#define deco16_f9 m6502_f9								/* 4 SBC ABY */

#define deco16_0a m6502_0a								/* 2 ASL */
#define deco16_2a m6502_2a								/* 2 ROL */
#define deco16_4a m6502_4a								/* 2 LSR */
#define deco16_6a m6502_6a								/* 2 ROR */
#define deco16_8a m6502_8a								/* 2 TXA */
#define deco16_aa m6502_aa								/* 2 TAX */
#define deco16_ca m6502_ca								/* 2 DEX */
#define deco16_ea m6502_ea								/* 2 NOP */

#define deco16_1a m65c02_1a								/* 2 INA */
#define deco16_3a m65c02_3a								/* 2 DEA */
#define deco16_5a m65c02_5a								/* 3 PHY */
#define deco16_7a m65c02_7a								/* 4 PLY */
#define deco16_9a m6502_9a								/* 2 TXS */
#define deco16_ba m6502_ba								/* 2 TSX */
#define deco16_da m65c02_da								/* 3 PHX */
#define deco16_fa m65c02_fa								/* 4 PLX */

OP(0b) { int tmp; m6502_ICount -= 1; RD_IMM;
//	logerror("%04x: OP0B %02x\n",PCW,tmp);

				}
OP(2b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(4b) { m6502_ICount -= 1; RD_IMM_DISCARD;
	//logerror("%04x: OP4B %02x\n",PCW,tmp);
	/* TODO: Maybe it's just read I/O 0 and do a logic AND with bit 1? */
	m6502.a=M6502ReadPort(1);

//tilt??

//VBL on expr-raider
//VBL on boomrang (bit 2)

				}
OP(6b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(8b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(ab) { RD_DUM; ILL; } 								/* 2 ILL */
OP(cb) { RD_DUM; ILL; } 								/* 2 ILL */
OP(eb) { RD_DUM; ILL; } 								/* 2 ILL */

OP(1b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(3b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(5b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(7b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(9b) { RD_DUM; ILL; } 								/* 2 ILL */
OP(bb) {
	int tmp;

	m6502_ICount -= 1;
	RD_IMM;

	//if (DECO16_VERBOSE)
	//	logerror("%04x: OPBB %02x\n",PCW,tmp);
}
OP(db) { RD_DUM; ILL; } 								/* 2 ILL */
OP(fb) { RD_DUM; ILL; } 								/* 2 ILL */

#define deco16_0c m65c02_0c								/* 4 TSB ABS */
#define deco16_2c m6502_2c								/* 4 BIT ABS */
#define deco16_4c m6502_4c								/* 3 JMP ABS */
#define deco16_6c m65c02_6c 								/* 5 JMP IND */
#define deco16_8c m6502_8c								/* 4 STY ABS */
#define deco16_ac m6502_ac								/* 4 LDY ABS */
#define deco16_cc m6502_cc								/* 4 CPY ABS */
#define deco16_ec m6502_ec								/* 4 CPX ABS */

#define deco16_1c m65c02_1c								/* 4 TRB ABS */
#define deco16_3c m65c02_3c								/* 4 BIT ABX */
OP(5c) { RD_DUM; ILL; } 								/* 2 ILL */
#define deco16_7c m65c02_7c								/* 6 JMP IAX */
#define deco16_9c m65c02_9c								/* 4 STZ ABS */
#define deco16_bc m65c02_bc								/* 4 LDY ABX */
OP(dc) { RD_DUM; ILL; } 								/* 2 ILL */
OP(fc) { RD_DUM; ILL; } 								/* 2 ILL */

#define deco16_0d m6502_0d								/* 4 ORA ABS */
#define deco16_2d m6502_2d								/* 4 AND ABS */
#define deco16_4d m6502_4d								/* 4 EOR ABS */
#define deco16_6d m6502_6d								/* 4 ADC ABS */
#define deco16_8d m6502_8d								/* 4 STA ABS */
#define deco16_ad m6502_ad								/* 4 LDA ABS */
#define deco16_cd m6502_cd								/* 4 CMP ABS */
#define deco16_ed m6502_ed								/* 4 SBC ABS */

#define deco16_1d m6502_1d								/* 4 ORA ABX */
#define deco16_3d m6502_3d								/* 4 AND ABX */
#define deco16_5d m6502_5d								/* 4 EOR ABX */
#define deco16_7d m6502_7d								/* 4 ADC ABX */
#define deco16_9d m6502_9d								/* 5 STA ABX */
#define deco16_bd m6502_bd								/* 4 LDA ABX */
#define deco16_dd m6502_dd								/* 4 CMP ABX */
#define deco16_fd m6502_fd								/* 4 SBC ABX */

#define deco16_0e m6502_0e								/* 6 ASL ABS */
#define deco16_2e m6502_2e								/* 6 ROL ABS */
#define deco16_4e m6502_4e								/* 6 LSR ABS */
#define deco16_6e m6502_6e								/* 6 ROR ABS */
#define deco16_8e m6502_8e								/* 4 STX ABS */
#define deco16_ae m6502_ae								/* 4 LDX ABS */
#define deco16_ce m6502_ce								/* 6 DEC ABS */
#define deco16_ee m6502_ee								/* 6 INC ABS */

#define deco16_1e m6502_1e								/* 7 ASL ABX */
#define deco16_3e m6502_3e								/* 7 ROL ABX */
#define deco16_5e m6502_5e								/* 7 LSR ABX */
#define deco16_7e m6502_7e								/* 7 ROR ABX */
#define deco16_9e m65c02_9e								/* 5 STZ ABX */
#define deco16_be m6502_be								/* 4 LDX ABY */
#define deco16_de m6502_de								/* 7 DEC ABX */
#define deco16_fe m6502_fe								/* 7 INC ABX */

OP(0f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR0 ZPG ?? */
OP(2f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR2 ZPG ?? */
OP(4f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR4 ZPG ?? */
OP(6f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR6 ZPG ?? */
OP(8f) { int tmp; m6502_ICount -= 1; RD_IMM;
	//if (DECO16_VERBOSE)
	//	logerror("%04x: BANK (8F) %02x\n",PCW,tmp);

	M6502WritePort(0,tmp);

	//swap bank in/out
} /*  */
OP(af) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS2 ZPG ?? */
OP(cf) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS4 ZPG ?? */
OP(ef) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS6 ZPG ?? */

OP(1f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR1 ZPG ?? */
OP(3f) {
	int tmp;

	m6502_ICount -= 1;
	RD_IMM;

	//if (DECO16_VERBOSE)
	//	logerror("%04x: OP3F %02x\n",PCW,tmp);
}
OP(5f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR5 ZPG ?? */
OP(7f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBR7 ZPG ?? */
OP(9f) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS1 ZPG ?? */
OP(bf) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS3 ZPG ?? */
OP(df) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS5 ZPG ?? */
OP(ff) { RD_DUM; ILL; }									/* 2 ILL / 5 BBS7 ZPG ?? */

static void (*const insndeco16[0x100])() = {
	deco16_00,deco16_01,deco16_02,deco16_03,deco16_04,deco16_05,deco16_06,deco16_07,
	deco16_08,deco16_09,deco16_0a,deco16_0b,deco16_0c,deco16_0d,deco16_0e,deco16_0f,
	deco16_10,deco16_11,deco16_12,deco16_13,deco16_14,deco16_15,deco16_16,deco16_17,
	deco16_18,deco16_19,deco16_1a,deco16_1b,deco16_1c,deco16_1d,deco16_1e,deco16_1f,
	deco16_20,deco16_21,deco16_22,deco16_23,deco16_24,deco16_25,deco16_26,deco16_27,
	deco16_28,deco16_29,deco16_2a,deco16_2b,deco16_2c,deco16_2d,deco16_2e,deco16_2f,
	deco16_30,deco16_31,deco16_32,deco16_33,deco16_34,deco16_35,deco16_36,deco16_37,
	deco16_38,deco16_39,deco16_3a,deco16_3b,deco16_3c,deco16_3d,deco16_3e,deco16_3f,
	deco16_40,deco16_41,deco16_42,deco16_43,deco16_44,deco16_45,deco16_46,deco16_47,
	deco16_48,deco16_49,deco16_4a,deco16_4b,deco16_4c,deco16_4d,deco16_4e,deco16_4f,
	deco16_50,deco16_51,deco16_52,deco16_53,deco16_54,deco16_55,deco16_56,deco16_57,
	deco16_58,deco16_59,deco16_5a,deco16_5b,deco16_5c,deco16_5d,deco16_5e,deco16_5f,
	deco16_60,deco16_61,deco16_62,deco16_63,deco16_64,deco16_65,deco16_66,deco16_67,
	deco16_68,deco16_69,deco16_6a,deco16_6b,deco16_6c,deco16_6d,deco16_6e,deco16_6f,
	deco16_70,deco16_71,deco16_72,deco16_73,deco16_74,deco16_75,deco16_76,deco16_77,
	deco16_78,deco16_79,deco16_7a,deco16_7b,deco16_7c,deco16_7d,deco16_7e,deco16_7f,
	deco16_80,deco16_81,deco16_82,deco16_83,deco16_84,deco16_85,deco16_86,deco16_87,
	deco16_88,deco16_89,deco16_8a,deco16_8b,deco16_8c,deco16_8d,deco16_8e,deco16_8f,
	deco16_90,deco16_91,deco16_92,deco16_93,deco16_94,deco16_95,deco16_96,deco16_97,
	deco16_98,deco16_99,deco16_9a,deco16_9b,deco16_9c,deco16_9d,deco16_9e,deco16_9f,
	deco16_a0,deco16_a1,deco16_a2,deco16_a3,deco16_a4,deco16_a5,deco16_a6,deco16_a7,
	deco16_a8,deco16_a9,deco16_aa,deco16_ab,deco16_ac,deco16_ad,deco16_ae,deco16_af,
	deco16_b0,deco16_b1,deco16_b2,deco16_b3,deco16_b4,deco16_b5,deco16_b6,deco16_b7,
	deco16_b8,deco16_b9,deco16_ba,deco16_bb,deco16_bc,deco16_bd,deco16_be,deco16_bf,
	deco16_c0,deco16_c1,deco16_c2,deco16_c3,deco16_c4,deco16_c5,deco16_c6,deco16_c7,
	deco16_c8,deco16_c9,deco16_ca,deco16_cb,deco16_cc,deco16_cd,deco16_ce,deco16_cf,
	deco16_d0,deco16_d1,deco16_d2,deco16_d3,deco16_d4,deco16_d5,deco16_d6,deco16_d7,
	deco16_d8,deco16_d9,deco16_da,deco16_db,deco16_dc,deco16_dd,deco16_de,deco16_df,
	deco16_e0,deco16_e1,deco16_e2,deco16_e3,deco16_e4,deco16_e5,deco16_e6,deco16_e7,
	deco16_e8,deco16_e9,deco16_ea,deco16_eb,deco16_ec,deco16_ed,deco16_ee,deco16_ef,
	deco16_f0,deco16_f1,deco16_f2,deco16_f3,deco16_f4,deco16_f5,deco16_f6,deco16_f7,
	deco16_f8,deco16_f9,deco16_fa,deco16_fb,deco16_fc,deco16_fd,deco16_fe,deco16_ff
};
