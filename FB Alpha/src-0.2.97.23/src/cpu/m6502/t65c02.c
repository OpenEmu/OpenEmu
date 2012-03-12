/*****************************************************************************
 *
 *   tbl65c02.c
 *   65c02 opcode functions and function pointer table
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
 * Not sure about the timing of all the extra (undocumented) NOP instructions.
 * Core may need to be split up into two 65c02 core. Not all versions supported
 * the bit operation RMB/SMB/etc.
 *
 *****************************************************************************/

#undef	OP
#define OP(nn) M6502_INLINE void m65c02_##nn(void)
#define RD_IMM_DISCARD		RDOPARG()
#define RD_ZPG_DISCARD		EA_ZPG; RDMEM(EAD)
#define RD_ZPX_DISCARD		EA_ZPX; RDMEM(EAD)

/*****************************************************************************
 *****************************************************************************
 *
 *  Implementations for 65C02 opcodes
 *
 *  There are a few slight differences between Rockwell and WDC 65C02 CPUs.
 *  The absolute indexed addressing mode RMW instructions take 6 cycles on
 *  WDC 65C02 CPU but 7 cycles on a regular 6502 and a Rockwell 65C02 CPU.
 *  TODO: Implement STP and WAI for wdc65c02.
 *
 *****************************************************************************
 * op    temp     cycles             rdmem   opc  wrmem   ********************/
OP(00) { BRK_C02;                                   } /* 7 BRK */
OP(20) { JSR;                                       } /* 6 JSR */
OP(40) { RTI;                                       } /* 6 RTI */
OP(60) { RTS;                                       } /* 6 RTS */
OP(80) { int tmp; BRA_C02( 1 );                     } /* 3-4 BRA REL */
OP(a0) { int tmp; RD_IMM; LDY;                      } /* 2 LDY IMM */
OP(c0) { int tmp; RD_IMM; CPY;                      } /* 2 CPY IMM */
OP(e0) { int tmp; RD_IMM; CPX;                      } /* 2 CPX IMM */

OP(10) { int tmp; BRA_C02( ! ( P & F_N ) );         } /* 2-4 BPL REL */
OP(30) { int tmp; BRA_C02(   ( P & F_N ) );         } /* 2-4 BMI REL */
OP(50) { int tmp; BRA_C02( ! ( P & F_V ) );         } /* 2-4 BVC REL */
OP(70) { int tmp; BRA_C02(   ( P & F_V ) );         } /* 2-4 BVS REL */
OP(90) { int tmp; BRA_C02( ! ( P & F_C ) );         } /* 2-4 BCC REL */
OP(b0) { int tmp; BRA_C02(   ( P & F_C ) );         } /* 2-4 BCS REL */
OP(d0) { int tmp; BRA_C02( ! ( P & F_Z ) );         } /* 2-4 BNE REL */
OP(f0) { int tmp; BRA_C02(   ( P & F_Z ) );         } /* 2-4 BEQ REL */

OP(01) { int tmp; RD_IDX; ORA;                      } /* 6 ORA IDX */
OP(21) { int tmp; RD_IDX; AND;                      } /* 6 AND IDX */
OP(41) { int tmp; RD_IDX; EOR;                      } /* 6 EOR IDX */
OP(61) { int tmp; RD_IDX; ADC_C02;                  } /* 6/7 ADC IDX */
OP(81) { int tmp; STA; WR_IDX;                      } /* 6 STA IDX */
OP(a1) { int tmp; RD_IDX; LDA;                      } /* 6 LDA IDX */
OP(c1) { int tmp; RD_IDX; CMP;                      } /* 6 CMP IDX */
OP(e1) { int tmp; RD_IDX; SBC_C02;                  } /* 6/7 SBC IDX */

OP(11) { int tmp; RD_IDY_C02_P; ORA;                } /* 5 ORA IDY page penalty */
OP(31) { int tmp; RD_IDY_C02_P; AND;                } /* 5 AND IDY page penalty */
OP(51) { int tmp; RD_IDY_C02_P; EOR;                } /* 5 EOR IDY page penalty */
OP(71) { int tmp; RD_IDY_C02_P; ADC_C02;            } /* 5/6 ADC IDY page penalty */
OP(91) { int tmp; STA; WR_IDY_C02_NP;               } /* 6 STA IDY */
OP(b1) { int tmp; RD_IDY_C02_P; LDA;                } /* 5 LDA IDY page penalty */
OP(d1) { int tmp; RD_IDY_C02_P; CMP;                } /* 5 CMP IDY page penalty */
OP(f1) { int tmp; RD_IDY_C02_P; SBC_C02;            } /* 5/6 SBC IDY page penalty */

OP(02) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */
OP(22) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */
OP(42) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */
OP(62) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */
OP(82) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */
OP(a2) { int tmp; RD_IMM; LDX;                      } /* 2 LDX IMM */
OP(c2) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */
OP(e2) { RD_IMM_DISCARD; NOP;                      } /* 2 NOP not sure for rockwell */

OP(12) { int tmp; RD_ZPI; ORA;                      } /* 5 ORA ZPI */
OP(32) { int tmp; RD_ZPI; AND;                      } /* 5 AND ZPI */
OP(52) { int tmp; RD_ZPI; EOR;                      } /* 5 EOR ZPI */
OP(72) { int tmp; RD_ZPI; ADC_C02;                  } /* 5/6 ADC ZPI */
OP(92) { int tmp; STA; WR_ZPI;                      } /* 5 STA ZPI */
OP(b2) { int tmp; RD_ZPI; LDA;                      } /* 5 LDA ZPI */
OP(d2) { int tmp; RD_ZPI; CMP;                      } /* 5 CMP ZPI */
OP(f2) { int tmp; RD_ZPI; SBC_C02;                  } /* 5/6 SBC ZPI */

OP(03) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(23) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(43) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(63) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(83) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(a3) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(c3) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(e3) { NOP;                                       } /* 1 NOP not sure for rockwell */

OP(13) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(33) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(53) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(73) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(93) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(b3) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(d3) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(f3) { NOP;                                       } /* 1 NOP not sure for rockwell */

OP(04) { int tmp; RD_ZPG; RD_EA; TSB; WB_EA;        } /* 5 TSB ZPG */
OP(24) { int tmp; RD_ZPG; BIT;                      } /* 3 BIT ZPG */
OP(44) { RD_ZPG_DISCARD; NOP;                      } /* 3 NOP not sure for rockwell */
OP(64) { int tmp; STZ; WR_ZPG;                      } /* 3 STZ ZPG */
OP(84) { int tmp; STY; WR_ZPG;                      } /* 3 STY ZPG */
OP(a4) { int tmp; RD_ZPG; LDY;                      } /* 3 LDY ZPG */
OP(c4) { int tmp; RD_ZPG; CPY;                      } /* 3 CPY ZPG */
OP(e4) { int tmp; RD_ZPG; CPX;                      } /* 3 CPX ZPG */

OP(14) { int tmp; RD_ZPG; RD_EA; TRB; WB_EA;        } /* 5 TRB ZPG */
OP(34) { int tmp; RD_ZPX; BIT;                      } /* 4 BIT ZPX */
OP(54) { RD_ZPX_DISCARD; NOP;                      } /* 4 NOP not sure for rockwell */
OP(74) { int tmp; STZ; WR_ZPX;                      } /* 4 STZ ZPX */
OP(94) { int tmp; STY; WR_ZPX;                      } /* 4 STY ZPX */
OP(b4) { int tmp; RD_ZPX; LDY;                      } /* 4 LDY ZPX */
OP(d4) { RD_ZPX_DISCARD; NOP;                      } /* 4 NOP not sure for rockwell */
OP(f4) { RD_ZPX_DISCARD; NOP;                      } /* 4 NOP not sure for rockwell */

OP(05) { int tmp; RD_ZPG; ORA;                      } /* 3 ORA ZPG */
OP(25) { int tmp; RD_ZPG; AND;                      } /* 3 AND ZPG */
OP(45) { int tmp; RD_ZPG; EOR;                      } /* 3 EOR ZPG */
OP(65) { int tmp; RD_ZPG; ADC_C02;                  } /* 3/4 ADC ZPG */
OP(85) { int tmp; STA; WR_ZPG;                      } /* 3 STA ZPG */
OP(a5) { int tmp; RD_ZPG; LDA;                      } /* 3 LDA ZPG */
OP(c5) { int tmp; RD_ZPG; CMP;                      } /* 3 CMP ZPG */
OP(e5) { int tmp; RD_ZPG; SBC_C02;                  } /* 3/4 SBC ZPG */

OP(15) { int tmp; RD_ZPX; ORA;                      } /* 4 ORA ZPX */
OP(35) { int tmp; RD_ZPX; AND;                      } /* 4 AND ZPX */
OP(55) { int tmp; RD_ZPX; EOR;                      } /* 4 EOR ZPX */
OP(75) { int tmp; RD_ZPX; ADC_C02;                  } /* 4/5 ADC ZPX */
OP(95) { int tmp; STA; WR_ZPX;                      } /* 4 STA ZPX */
OP(b5) { int tmp; RD_ZPX; LDA;                      } /* 4 LDA ZPX */
OP(d5) { int tmp; RD_ZPX; CMP;                      } /* 4 CMP ZPX */
OP(f5) { int tmp; RD_ZPX; SBC_C02;                  } /* 4/5 SBC ZPX */

OP(06) { int tmp; RD_ZPG, RD_EA; ASL; WB_EA;        } /* 5 ASL ZPG */
OP(26) { int tmp; RD_ZPG; RD_EA; ROL; WB_EA;        } /* 5 ROL ZPG */
OP(46) { int tmp; RD_ZPG; RD_EA; LSR; WB_EA;        } /* 5 LSR ZPG */
OP(66) { int tmp; RD_ZPG; RD_EA; ROR; WB_EA;        } /* 5 ROR ZPG */
OP(86) { int tmp; STX; WR_ZPG;                      } /* 3 STX ZPG */
OP(a6) { int tmp; RD_ZPG; LDX;                      } /* 3 LDX ZPG */
OP(c6) { int tmp; RD_ZPG; RD_EA; DEC; WB_EA;        } /* 5 DEC ZPG */
OP(e6) { int tmp; RD_ZPG; RD_EA; INC; WB_EA;        } /* 5 INC ZPG */

OP(16) { int tmp; RD_ZPX; RD_EA; ASL; WB_EA;        } /* 6 ASL ZPX */
OP(36) { int tmp; RD_ZPX; RD_EA; ROL; WB_EA;        } /* 6 ROL ZPX */
OP(56) { int tmp; RD_ZPX; RD_EA; LSR; WB_EA;        } /* 6 LSR ZPX */
OP(76) { int tmp; RD_ZPX; RD_EA; ROR; WB_EA;        } /* 6 ROR ZPX */
OP(96) { int tmp; STX; WR_ZPY;                      } /* 4 STX ZPY */
OP(b6) { int tmp; RD_ZPY; LDX;                      } /* 4 LDX ZPY */
OP(d6) { int tmp; RD_ZPX; RD_EA; DEC; WB_EA;        } /* 6 DEC ZPX */
OP(f6) { int tmp; RD_ZPX; RD_EA; INC; WB_EA;        } /* 6 INC ZPX */

OP(07) { int tmp; RD_ZPG; RD_EA; RMB(0);WB_EA;      } /* 5 RMB0 ZPG */
OP(27) { int tmp; RD_ZPG; RD_EA; RMB(2);WB_EA;      } /* 5 RMB2 ZPG */
OP(47) { int tmp; RD_ZPG; RD_EA; RMB(4);WB_EA;      } /* 5 RMB4 ZPG */
OP(67) { int tmp; RD_ZPG; RD_EA; RMB(6);WB_EA;      } /* 5 RMB6 ZPG */
OP(87) { int tmp; RD_ZPG; RD_EA; SMB(0);WB_EA;      } /* 5 SMB0 ZPG */
OP(a7) { int tmp; RD_ZPG; RD_EA; SMB(2);WB_EA;      } /* 5 SMB2 ZPG */
OP(c7) { int tmp; RD_ZPG; RD_EA; SMB(4);WB_EA;      } /* 5 SMB4 ZPG */
OP(e7) { int tmp; RD_ZPG; RD_EA; SMB(6);WB_EA;      } /* 5 SMB6 ZPG */

OP(17) { int tmp; RD_ZPG; RD_EA; RMB(1);WB_EA;      } /* 5 RMB1 ZPG */
OP(37) { int tmp; RD_ZPG; RD_EA; RMB(3);WB_EA;      } /* 5 RMB3 ZPG */
OP(57) { int tmp; RD_ZPG; RD_EA; RMB(5);WB_EA;      } /* 5 RMB5 ZPG */
OP(77) { int tmp; RD_ZPG; RD_EA; RMB(7);WB_EA;      } /* 5 RMB7 ZPG */
OP(97) { int tmp; RD_ZPG; RD_EA; SMB(1);WB_EA;      } /* 5 SMB1 ZPG */
OP(b7) { int tmp; RD_ZPG; RD_EA; SMB(3);WB_EA;      } /* 5 SMB3 ZPG */
OP(d7) { int tmp; RD_ZPG; RD_EA; SMB(5);WB_EA;      } /* 5 SMB5 ZPG */
OP(f7) { int tmp; RD_ZPG; RD_EA; SMB(7);WB_EA;      } /* 5 SMB7 ZPG */

OP(08) { RD_DUM; PHP;                               } /* 3 PHP */
OP(28) { RD_DUM; PLP;                               } /* 4 PLP */
OP(48) { RD_DUM; PHA;                               } /* 3 PHA */
OP(68) { RD_DUM; PLA;                               } /* 4 PLA */
OP(88) { RD_DUM; DEY;                               } /* 2 DEY */
OP(a8) { RD_DUM; TAY;                               } /* 2 TAY */
OP(c8) { RD_DUM; INY;                               } /* 2 INY */
OP(e8) { RD_DUM; INX;                               } /* 2 INX */

OP(18) { RD_DUM; CLC;                               } /* 2 CLC */
OP(38) { RD_DUM; SEC;                               } /* 2 SEC */
OP(58) { RD_DUM; CLI;                               } /* 2 CLI */
OP(78) { RD_DUM; SEI;                               } /* 2 SEI */
OP(98) { RD_DUM; TYA;                               } /* 2 TYA */
OP(b8) { RD_DUM; CLV;                               } /* 2 CLV */
OP(d8) { RD_DUM; CLD;                               } /* 2 CLD */
OP(f8) { RD_DUM; SED;                               } /* 2 SED */

OP(09) { int tmp; RD_IMM; ORA;                      } /* 2 ORA IMM */
OP(29) { int tmp; RD_IMM; AND;                      } /* 2 AND IMM */
OP(49) { int tmp; RD_IMM; EOR;                      } /* 2 EOR IMM */
OP(69) { int tmp; RD_IMM; ADC_C02;                  } /* 2/3 ADC IMM */
OP(89) { int tmp; RD_IMM; BIT_IMM_C02;              } /* 2 BIT IMM */
OP(a9) { int tmp; RD_IMM; LDA;                      } /* 2 LDA IMM */
OP(c9) { int tmp; RD_IMM; CMP;                      } /* 2 CMP IMM */
OP(e9) { int tmp; RD_IMM; SBC_C02;                  } /* 2/3 SBC IMM */

OP(19) { int tmp; RD_ABY_C02_P; ORA;                } /* 4 ORA ABY page penalty */
OP(39) { int tmp; RD_ABY_C02_P; AND;                } /* 4 AND ABY page penalty */
OP(59) { int tmp; RD_ABY_C02_P; EOR;                } /* 4 EOR ABY page penalty */
OP(79) { int tmp; RD_ABY_C02_P; ADC_C02;            } /* 4/5 ADC ABY page penalty */
OP(99) { int tmp; STA; WR_ABY_C02_NP;               } /* 5 STA ABY */
OP(b9) { int tmp; RD_ABY_C02_P; LDA;                } /* 4 LDA ABY page penalty */
OP(d9) { int tmp; RD_ABY_C02_P; CMP;                } /* 4 CMP ABY page penalty */
OP(f9) { int tmp; RD_ABY_C02_P; SBC_C02;            } /* 4/5 SBC ABY page penalty */

OP(0a) { int tmp; RD_DUM; RD_ACC; ASL; WB_ACC;      } /* 2 ASL A */
OP(2a) { int tmp; RD_DUM; RD_ACC; ROL; WB_ACC;      } /* 2 ROL A */
OP(4a) { int tmp; RD_DUM; RD_ACC; LSR; WB_ACC;      } /* 2 LSR A */
OP(6a) { int tmp; RD_DUM; RD_ACC; ROR; WB_ACC;      } /* 2 ROR A */
OP(8a) { RD_DUM; TXA;                               } /* 2 TXA */
OP(aa) { RD_DUM; TAX;                               } /* 2 TAX */
OP(ca) { RD_DUM; DEX;                               } /* 2 DEX */
OP(ea) { RD_DUM; NOP;                               } /* 2 NOP */

OP(1a) { RD_DUM;INA;                                } /* 2 INA */
OP(3a) { RD_DUM;DEA;                                } /* 2 DEA */
OP(5a) { RD_DUM;PHY;                                } /* 3 PHY */
OP(7a) { RD_DUM;PLY;                                } /* 4 PLY */
OP(9a) { RD_DUM; TXS;                               } /* 2 TXS */
OP(ba) { RD_DUM; TSX;                               } /* 2 TSX */
OP(da) { RD_DUM;PHX;                                } /* 3 PHX */
OP(fa) { RD_DUM;PLX;                                } /* 4 PLX */

OP(0b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(2b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(4b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(6b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(8b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(ab) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(cb) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(eb) { NOP;                                       } /* 1 NOP not sure for rockwell */

OP(1b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(3b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(5b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(7b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(9b) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(bb) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(db) { NOP;                                       } /* 1 NOP not sure for rockwell */
OP(fb) { NOP;                                       } /* 1 NOP not sure for rockwell */

OP(0c) { int tmp; RD_ABS; RD_EA; TSB; WB_EA;        } /* 6 TSB ABS */
OP(2c) { int tmp; RD_ABS; BIT;                      } /* 4 BIT ABS */
OP(4c) { EA_ABS; JMP;                               } /* 3 JMP ABS */
OP(6c) { int tmp; EA_IND_C02; JMP;                  } /* 6 JMP IND */
OP(8c) { int tmp; STY; WR_ABS;                      } /* 4 STY ABS */
OP(ac) { int tmp; RD_ABS; LDY;                      } /* 4 LDY ABS */
OP(cc) { int tmp; RD_ABS; CPY;                      } /* 4 CPY ABS */
OP(ec) { int tmp; RD_ABS; CPX;                      } /* 4 CPX ABS */

OP(1c) { int tmp; RD_ABS; RD_EA; TRB; WB_EA;        } /* 6 TRB ABS */
OP(3c) { int tmp; RD_ABX_C02_P; BIT;                } /* 4 BIT ABX page penalty */
OP(5c) { RD_ABX_C02_NP_DISCARD; RD_DUM; RD_DUM; RD_DUM; RD_DUM; } /* 8 NOP ABX not sure for rockwell. Page penalty not sure */
OP(7c) { int tmp; EA_IAX; JMP;                      } /* 6 JMP IAX page penalty */
OP(9c) { int tmp; STZ; WR_ABS;                      } /* 4 STZ ABS */
OP(bc) { int tmp; RD_ABX_C02_P; LDY;                } /* 4 LDY ABX page penalty */
OP(dc) { RD_ABX_C02_NP_DISCARD; NOP;               } /* 4 NOP ABX not sure for rockwell. Page penalty not sure  */
OP(fc) { RD_ABX_C02_NP_DISCARD; NOP;               } /* 4 NOP ABX not sure for rockwell. Page penalty not sure  */

OP(0d) { int tmp; RD_ABS; ORA;                      } /* 4 ORA ABS */
OP(2d) { int tmp; RD_ABS; AND;                      } /* 4 AND ABS */
OP(4d) { int tmp; RD_ABS; EOR;                      } /* 4 EOR ABS */
OP(6d) { int tmp; RD_ABS; ADC_C02;                  } /* 4/5 ADC ABS */
OP(8d) { int tmp; STA; WR_ABS;                      } /* 4 STA ABS */
OP(ad) { int tmp; RD_ABS; LDA;                      } /* 4 LDA ABS */
OP(cd) { int tmp; RD_ABS; CMP;                      } /* 4 CMP ABS */
OP(ed) { int tmp; RD_ABS; SBC_C02;                  } /* 4/5 SBC ABS */

OP(1d) { int tmp; RD_ABX_C02_P; ORA;                } /* 4 ORA ABX page penalty */
OP(3d) { int tmp; RD_ABX_C02_P; AND;                } /* 4 AND ABX page penalty */
OP(5d) { int tmp; RD_ABX_C02_P; EOR;                } /* 4 EOR ABX page penalty */
OP(7d) { int tmp; RD_ABX_C02_P; ADC_C02;            } /* 4/5 ADC ABX page penalty */
OP(9d) { int tmp; STA; WR_ABX_C02_NP;               } /* 5 STA ABX */
OP(bd) { int tmp; RD_ABX_C02_P; LDA;                } /* 4 LDA ABX page penalty */
OP(dd) { int tmp; RD_ABX_C02_P; CMP;                } /* 4 CMP ABX page penalty */
OP(fd) { int tmp; RD_ABX_C02_P; SBC_C02;            } /* 4/5 SBC ABX page penalty */

OP(0e) { int tmp; RD_ABS; RD_EA; ASL; WB_EA;        } /* 6 ASL ABS */
OP(2e) { int tmp; RD_ABS; RD_EA; ROL; WB_EA;        } /* 6 ROL ABS */
OP(4e) { int tmp; RD_ABS; RD_EA; LSR; WB_EA;        } /* 6 LSR ABS */
OP(6e) { int tmp; RD_ABS; RD_EA; ROR; WB_EA;        } /* 6 ROR ABS */
OP(8e) { int tmp; STX; WR_ABS;                      } /* 4 STX ABS */
OP(ae) { int tmp; RD_ABS; LDX;                      } /* 4 LDX ABS */
OP(ce) { int tmp; RD_ABS; RD_EA; DEC; WB_EA;        } /* 6 DEC ABS */
OP(ee) { int tmp; RD_ABS; RD_EA; INC; WB_EA;        } /* 6 INC ABS */

OP(1e) { int tmp; RD_ABX_C02_NP; RD_EA; ASL; WB_EA; } /* 7 ASL ABX */
OP(3e) { int tmp; RD_ABX_C02_NP; RD_EA; ROL; WB_EA; } /* 7 ROL ABX */
OP(5e) { int tmp; RD_ABX_C02_NP; RD_EA; LSR; WB_EA; } /* 7 LSR ABX */
OP(7e) { int tmp; RD_ABX_C02_NP; RD_EA; ROR; WB_EA; } /* 7 ROR ABX */
OP(9e) { int tmp; STZ; WR_ABX_C02_NP;               } /* 5 STZ ABX */
OP(be) { int tmp; RD_ABY_C02_P; LDX;                } /* 4 LDX ABY page penalty */
OP(de) { int tmp; RD_ABX_C02_NP; RD_EA; DEC; WB_EA; } /* 7 DEC ABX */
OP(fe) { int tmp; RD_ABX_C02_NP; RD_EA; INC; WB_EA; } /* 7 INC ABX */

OP(0f) { int tmp; RD_ZPG; BBR(0);                   } /* 5-7 BBR0 ZPG */
OP(2f) { int tmp; RD_ZPG; BBR(2);                   } /* 5-7 BBR2 ZPG */
OP(4f) { int tmp; RD_ZPG; BBR(4);                   } /* 5-7 BBR4 ZPG */
OP(6f) { int tmp; RD_ZPG; BBR(6);                   } /* 5-7 BBR6 ZPG */
OP(8f) { int tmp; RD_ZPG; BBS(0);                   } /* 5-7 BBS0 ZPG */
OP(af) { int tmp; RD_ZPG; BBS(2);                   } /* 5-7 BBS2 ZPG */
OP(cf) { int tmp; RD_ZPG; BBS(4);                   } /* 5-7 BBS4 ZPG */
OP(ef) { int tmp; RD_ZPG; BBS(6);                   } /* 5-7 BBS6 ZPG */

OP(1f) { int tmp; RD_ZPG; BBR(1);                   } /* 5-7 BBR1 ZPG */
OP(3f) { int tmp; RD_ZPG; BBR(3);                   } /* 5-7 BBR3 ZPG */
OP(5f) { int tmp; RD_ZPG; BBR(5);                   } /* 5-7 BBR5 ZPG */
OP(7f) { int tmp; RD_ZPG; BBR(7);                   } /* 5-7 BBR7 ZPG */
OP(9f) { int tmp; RD_ZPG; BBS(1);                   } /* 5-7 BBS1 ZPG */
OP(bf) { int tmp; RD_ZPG; BBS(3);                   } /* 5-7 BBS3 ZPG */
OP(df) { int tmp; RD_ZPG; BBS(5);                   } /* 5-7 BBS5 ZPG */
OP(ff) { int tmp; RD_ZPG; BBS(7);                   } /* 5-7 BBS7 ZPG */

static void (*const insn65c02[0x100])(void) = {
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

#ifdef WDC65C02
OP(cb_wdc) { RD_DUM; RD_DUM;                            } /* 3 WAI, TODO: Implement HALT mode */
OP(db_wdc) { RD_DUM; RD_DUM;                            } /* 3 STP, TODO: Implement STP mode */
OP(1e_wdc) { int tmp; RD_ABX_P; RD_EA; ASL; WB_EA;      } /* 6 ASL ABX page penalty */
OP(3e_wdc) { int tmp; RD_ABX_P; RD_EA; ROL; WB_EA;      } /* 6 ROL ABX page penalty */
OP(5e_wdc) { int tmp; RD_ABX_P; RD_EA; LSR; WB_EA;      } /* 6 LSR ABX page penalty */
OP(7e_wdc) { int tmp; RD_ABX_P; RD_EA; ROR; WB_EA;      } /* 6 ROR ABX page penalty */
OP(de_wdc) { int tmp; RD_ABX_P; RD_EA; DEC; WB_EA;      } /* 6 DEC ABX page penalty */
OP(fe_wdc) { int tmp; RD_ABX_P; RD_EA; INC; WB_EA;      } /* 6 INC ABX page penalty */

static void (*const insnwdc65c02[0x100])(m6502_Regs *cpustate) = {
	m65c02_00,m65c02_01,m65c02_02,m65c02_03,m65c02_04,m65c02_05,m65c02_06,m65c02_07,
	m65c02_08,m65c02_09,m65c02_0a,m65c02_0b,m65c02_0c,m65c02_0d,m65c02_0e,m65c02_0f,
	m65c02_10,m65c02_11,m65c02_12,m65c02_13,m65c02_14,m65c02_15,m65c02_16,m65c02_17,
	m65c02_18,m65c02_19,m65c02_1a,m65c02_1b,m65c02_1c,m65c02_1d,m65c02_1e_wdc,m65c02_1f,
	m65c02_20,m65c02_21,m65c02_22,m65c02_23,m65c02_24,m65c02_25,m65c02_26,m65c02_27,
	m65c02_28,m65c02_29,m65c02_2a,m65c02_2b,m65c02_2c,m65c02_2d,m65c02_2e,m65c02_2f,
	m65c02_30,m65c02_31,m65c02_32,m65c02_33,m65c02_34,m65c02_35,m65c02_36,m65c02_37,
	m65c02_38,m65c02_39,m65c02_3a,m65c02_3b,m65c02_3c,m65c02_3d,m65c02_3e_wdc,m65c02_3f,
	m65c02_40,m65c02_41,m65c02_42,m65c02_43,m65c02_44,m65c02_45,m65c02_46,m65c02_47,
	m65c02_48,m65c02_49,m65c02_4a,m65c02_4b,m65c02_4c,m65c02_4d,m65c02_4e,m65c02_4f,
	m65c02_50,m65c02_51,m65c02_52,m65c02_53,m65c02_54,m65c02_55,m65c02_56,m65c02_57,
	m65c02_58,m65c02_59,m65c02_5a,m65c02_5b,m65c02_5c,m65c02_5d,m65c02_5e_wdc,m65c02_5f,
	m65c02_60,m65c02_61,m65c02_62,m65c02_63,m65c02_64,m65c02_65,m65c02_66,m65c02_67,
	m65c02_68,m65c02_69,m65c02_6a,m65c02_6b,m65c02_6c,m65c02_6d,m65c02_6e,m65c02_6f,
	m65c02_70,m65c02_71,m65c02_72,m65c02_73,m65c02_74,m65c02_75,m65c02_76,m65c02_77,
	m65c02_78,m65c02_79,m65c02_7a,m65c02_7b,m65c02_7c,m65c02_7d,m65c02_7e_wdc,m65c02_7f,
	m65c02_80,m65c02_81,m65c02_82,m65c02_83,m65c02_84,m65c02_85,m65c02_86,m65c02_87,
	m65c02_88,m65c02_89,m65c02_8a,m65c02_8b,m65c02_8c,m65c02_8d,m65c02_8e,m65c02_8f,
	m65c02_90,m65c02_91,m65c02_92,m65c02_93,m65c02_94,m65c02_95,m65c02_96,m65c02_97,
	m65c02_98,m65c02_99,m65c02_9a,m65c02_9b,m65c02_9c,m65c02_9d,m65c02_9e,m65c02_9f,
	m65c02_a0,m65c02_a1,m65c02_a2,m65c02_a3,m65c02_a4,m65c02_a5,m65c02_a6,m65c02_a7,
	m65c02_a8,m65c02_a9,m65c02_aa,m65c02_ab,m65c02_ac,m65c02_ad,m65c02_ae,m65c02_af,
	m65c02_b0,m65c02_b1,m65c02_b2,m65c02_b3,m65c02_b4,m65c02_b5,m65c02_b6,m65c02_b7,
	m65c02_b8,m65c02_b9,m65c02_ba,m65c02_bb,m65c02_bc,m65c02_bd,m65c02_be,m65c02_bf,
	m65c02_c0,m65c02_c1,m65c02_c2,m65c02_c3,m65c02_c4,m65c02_c5,m65c02_c6,m65c02_c7,
	m65c02_c8,m65c02_c9,m65c02_ca,m65c02_cb_wdc,m65c02_cc,m65c02_cd,m65c02_ce,m65c02_cf,
	m65c02_d0,m65c02_d1,m65c02_d2,m65c02_d3,m65c02_d4,m65c02_d5,m65c02_d6,m65c02_d7,
	m65c02_d8,m65c02_d9,m65c02_da,m65c02_db_wdc,m65c02_dc,m65c02_dd,m65c02_de_wdc,m65c02_df,
	m65c02_e0,m65c02_e1,m65c02_e2,m65c02_e3,m65c02_e4,m65c02_e5,m65c02_e6,m65c02_e7,
	m65c02_e8,m65c02_e9,m65c02_ea,m65c02_eb,m65c02_ec,m65c02_ed,m65c02_ee,m65c02_ef,
	m65c02_f0,m65c02_f1,m65c02_f2,m65c02_f3,m65c02_f4,m65c02_f5,m65c02_f6,m65c02_f7,
	m65c02_f8,m65c02_f9,m65c02_fa,m65c02_fb,m65c02_fc,m65c02_fd,m65c02_fe_wdc,m65c02_ff
};
#endif


