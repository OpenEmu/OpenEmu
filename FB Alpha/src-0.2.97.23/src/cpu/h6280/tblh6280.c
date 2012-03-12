/*****************************************************************************

    tblh6280.c

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

#undef	OP
#define OP(nnn) H6280_INLINE void h6280##nnn(void)

/*****************************************************************************
 *****************************************************************************
 *
 *   Hu6280 opcodes
 *
 *****************************************************************************
 * op     temp     cycles             rdmem   opc   wrmem   ******************/
OP(_000) {		   H6280_CYCLES(8);		  BRK;		   } // 8 BRK
OP(_020) {		   H6280_CYCLES(7); EA_ABS; JSR;		   } // 7 JSR  ABS
OP(_040) {		   H6280_CYCLES(7);		  RTI;		   } // 7 RTI
OP(_060) {		   H6280_CYCLES(7);		  RTS;		   } // 7 RTS
OP(_080) { int tmp;  				          BRA(1);	   } // 4 BRA  REL
OP(_0a0) { int tmp; H6280_CYCLES(2); RD_IMM; LDY;		   } // 2 LDY  IMM
OP(_0c0) { int tmp; H6280_CYCLES(2); RD_IMM; CPY;		   } // 2 CPY  IMM
OP(_0e0) { int tmp; H6280_CYCLES(2); RD_IMM; CPX;		   } // 2 CPX  IMM

OP(_010) { int tmp;							  BPL;		   } // 2/4 BPL  REL
OP(_030) { int tmp;							  BMI;		   } // 2/4 BMI  REL
OP(_050) { int tmp;							  BVC;		   } // 2/4 BVC  REL
OP(_070) { int tmp;							  BVS;		   } // 2/4 BVS  REL
OP(_090) { int tmp;							  BCC;		   } // 2/4 BCC  REL
OP(_0b0) { int tmp;							  BCS;		   } // 2/4 BCS  REL
OP(_0d0) { int tmp;							  BNE;		   } // 2/4 BNE  REL
OP(_0f0) { int tmp;							  BEQ;		   } // 2/4 BEQ  REL

OP(_001) { int tmp; H6280_CYCLES(7); RD_IDX; ORA;		   } // 7 ORA  IDX
OP(_021) { int tmp; H6280_CYCLES(7); RD_IDX; AND;		   } // 7 AND  IDX
OP(_041) { int tmp; H6280_CYCLES(7); RD_IDX; EOR;		   } // 7 EOR  IDX
OP(_061) { int tmp; H6280_CYCLES(7); RD_IDX; ADC;		   } // 7 ADC  IDX
OP(_081) { int tmp; H6280_CYCLES(7);         STA; WR_IDX; } // 7 STA  IDX
OP(_0a1) { int tmp; H6280_CYCLES(7); RD_IDX; LDA;		   } // 7 LDA  IDX
OP(_0c1) { int tmp; H6280_CYCLES(7); RD_IDX; CMP;		   } // 7 CMP  IDX
OP(_0e1) { int tmp; H6280_CYCLES(7); RD_IDX; SBC;		   } // 7 SBC  IDX

OP(_011) { int tmp; H6280_CYCLES(7); RD_IDY; ORA;		   } // 7 ORA  IDY
OP(_031) { int tmp; H6280_CYCLES(7); RD_IDY; AND;		   } // 7 AND  IDY
OP(_051) { int tmp; H6280_CYCLES(7); RD_IDY; EOR;		   } // 7 EOR  IDY
OP(_071) { int tmp; H6280_CYCLES(7); RD_IDY; ADC;		   } // 7 ADC  AZP
OP(_091) { int tmp; H6280_CYCLES(7);		  STA; WR_IDY; } // 7 STA  IDY
OP(_0b1) { int tmp; H6280_CYCLES(7); RD_IDY; LDA;		   } // 7 LDA  IDY
OP(_0d1) { int tmp; H6280_CYCLES(7); RD_IDY; CMP;		   } // 7 CMP  IDY
OP(_0f1) { int tmp; H6280_CYCLES(7); RD_IDY; SBC;		   } // 7 SBC  IDY

OP(_002) { int tmp; H6280_CYCLES(3);		  SXY;		   } // 3 SXY
OP(_022) { int tmp; H6280_CYCLES(3);		  SAX;		   } // 3 SAX
OP(_042) { int tmp; H6280_CYCLES(3);		  SAY;		   } // 3 SAY
OP(_062) {		   H6280_CYCLES(2);		  CLA;		   } // 2 CLA
OP(_082) {		   H6280_CYCLES(2);		  CLX;		   } // 2 CLX
OP(_0a2) { int tmp; H6280_CYCLES(2); RD_IMM; LDX;		   } // 2 LDX  IMM
OP(_0c2) {		   H6280_CYCLES(2);		  CLY;		   } // 2 CLY
OP(_0e2) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP

OP(_012) { int tmp; H6280_CYCLES(7); RD_ZPI; ORA;		   } // 7 ORA  ZPI
OP(_032) { int tmp; H6280_CYCLES(7); RD_ZPI; AND;		   } // 7 AND  ZPI
OP(_052) { int tmp; H6280_CYCLES(7); RD_ZPI; EOR;		   } // 7 EOR  ZPI
OP(_072) { int tmp; H6280_CYCLES(7); RD_ZPI; ADC;		   } // 7 ADC  ZPI
OP(_092) { int tmp; H6280_CYCLES(7);		  STA; WR_ZPI; } // 7 STA  ZPI
OP(_0b2) { int tmp; H6280_CYCLES(7); RD_ZPI; LDA;		   } // 7 LDA  ZPI
OP(_0d2) { int tmp; H6280_CYCLES(7); RD_ZPI; CMP;		   } // 7 CMP  ZPI
OP(_0f2) { int tmp; H6280_CYCLES(7); RD_ZPI; SBC;		   } // 7 SBC  ZPI

OP(_003) { int tmp; H6280_CYCLES(5); RD_IMM; ST0;		   } // 4 + 1 penalty cycle ST0  IMM
OP(_023) { int tmp; H6280_CYCLES(5); RD_IMM; ST2;		   } // 4 + 1 penalty cycle ST2  IMM
OP(_043) { int tmp; H6280_CYCLES(4); RD_IMM; TMA;		   } // 4 TMA
OP(_063) {			H6280_CYCLES(4);		   NOP;		   } // 2 NOP
OP(_083) { int tmp,tmp2; H6280_CYCLES(7); RD_IMM2; RD_ZPG; TST; } // 7 TST  IMM,ZPG
OP(_0a3) { int tmp,tmp2; H6280_CYCLES(7); RD_IMM2; RD_ZPX; TST; } // 7 TST  IMM,ZPX
OP(_0c3) { int to,from,length;			      TDD;		   } // 6*l+17 TDD  XFER
OP(_0e3) { int to,from,length,alternate;       TIA;		   } // 6*l+17 TIA  XFER

OP(_013) { int tmp; H6280_CYCLES(5); RD_IMM; ST1;		   } // 4 + 1 penalty cycle ST1
OP(_033) {       	H6280_CYCLES(2);		   NOP;		   } // 2 NOP
OP(_053) { int tmp; H6280_CYCLES(5); RD_IMM; TAM;		   } // 5 TAM  IMM
OP(_073) { int to,from,length;    			  TII;		   } // 6*l+17 TII  XFER
OP(_093) { int tmp,tmp2; H6280_CYCLES(8); RD_IMM2; RD_ABS; TST; } // 8 TST  IMM,ABS
OP(_0b3) { int tmp,tmp2; H6280_CYCLES(8); RD_IMM2; RD_ABX; TST; } // 8 TST  IMM,ABX
OP(_0d3) { int to,from,length;			      TIN;		   } // 6*l+17 TIN  XFER
OP(_0f3) { int to,from,length,alternate;       TAI;		   } // 6*l+17 TAI  XFER

OP(_004) { int tmp; H6280_CYCLES(6); RD_ZPG; TSB; WB_EAZ; } // 6 TSB  ZPG
OP(_024) { int tmp; H6280_CYCLES(4); RD_ZPG; BIT;		   } // 4 BIT  ZPG
OP(_044) { int tmp;							  BSR;		   } // 8 BSR  REL
OP(_064) { int tmp; H6280_CYCLES(4);		  STZ; WR_ZPG; } // 4 STZ  ZPG
OP(_084) { int tmp; H6280_CYCLES(4);		  STY; WR_ZPG; } // 4 STY  ZPG
OP(_0a4) { int tmp; H6280_CYCLES(4); RD_ZPG; LDY;		   } // 4 LDY  ZPG
OP(_0c4) { int tmp; H6280_CYCLES(4); RD_ZPG; CPY;		   } // 4 CPY  ZPG
OP(_0e4) { int tmp; H6280_CYCLES(4); RD_ZPG; CPX;		   } // 4 CPX  ZPG

OP(_014) { int tmp; H6280_CYCLES(6); RD_ZPG; TRB; WB_EAZ; } // 6 TRB  ZPG
OP(_034) { int tmp; H6280_CYCLES(4); RD_ZPX; BIT;		   } // 4 BIT  ZPX
OP(_054) {		   H6280_CYCLES(3);			CSL;		   } // 3 CSL
OP(_074) { int tmp; H6280_CYCLES(4);		  STZ; WR_ZPX; } // 4 STZ  ZPX
OP(_094) { int tmp; H6280_CYCLES(4);		  STY; WR_ZPX; } // 4 STY  ZPX
OP(_0b4) { int tmp; H6280_CYCLES(4); RD_ZPX; LDY;		   } // 4 LDY  ZPX
OP(_0d4) {		   H6280_CYCLES(3);			CSH;		   } // 3 CSH
OP(_0f4) {		   H6280_CYCLES(2);		  SET;		   } // 2 SET

OP(_005) { int tmp; H6280_CYCLES(4); RD_ZPG; ORA;		   } // 4 ORA  ZPG
OP(_025) { int tmp; H6280_CYCLES(4); RD_ZPG; AND;		   } // 4 AND  ZPG
OP(_045) { int tmp; H6280_CYCLES(4); RD_ZPG; EOR;		   } // 4 EOR  ZPG
OP(_065) { int tmp; H6280_CYCLES(4); RD_ZPG; ADC;		   } // 4 ADC  ZPG
OP(_085) { int tmp; H6280_CYCLES(4);		  STA; WR_ZPG; } // 4 STA  ZPG
OP(_0a5) { int tmp; H6280_CYCLES(4); RD_ZPG; LDA;		   } // 4 LDA  ZPG
OP(_0c5) { int tmp; H6280_CYCLES(4); RD_ZPG; CMP;		   } // 4 CMP  ZPG
OP(_0e5) { int tmp; H6280_CYCLES(4); RD_ZPG; SBC;		   } // 4 SBC  ZPG

OP(_015) { int tmp; H6280_CYCLES(4); RD_ZPX; ORA;		   } // 4 ORA  ZPX
OP(_035) { int tmp; H6280_CYCLES(4); RD_ZPX; AND;		   } // 4 AND  ZPX
OP(_055) { int tmp; H6280_CYCLES(4); RD_ZPX; EOR;		   } // 4 EOR  ZPX
OP(_075) { int tmp; H6280_CYCLES(4); RD_ZPX; ADC;		   } // 4 ADC  ZPX
OP(_095) { int tmp; H6280_CYCLES(4);		  STA; WR_ZPX; } // 4 STA  ZPX
OP(_0b5) { int tmp; H6280_CYCLES(4); RD_ZPX; LDA;		   } // 4 LDA  ZPX
OP(_0d5) { int tmp; H6280_CYCLES(4); RD_ZPX; CMP;		   } // 4 CMP  ZPX
OP(_0f5) { int tmp; H6280_CYCLES(4); RD_ZPX; SBC;		   } // 4 SBC  ZPX

OP(_006) { int tmp; H6280_CYCLES(6); RD_ZPG; ASL; WB_EAZ; } // 6 ASL  ZPG
OP(_026) { int tmp; H6280_CYCLES(6); RD_ZPG; ROL; WB_EAZ; } // 6 ROL  ZPG
OP(_046) { int tmp; H6280_CYCLES(6); RD_ZPG; LSR; WB_EAZ; } // 6 LSR  ZPG
OP(_066) { int tmp; H6280_CYCLES(6); RD_ZPG; ROR; WB_EAZ; } // 6 ROR  ZPG
OP(_086) { int tmp; H6280_CYCLES(4);		  STX; WR_ZPG; } // 4 STX  ZPG
OP(_0a6) { int tmp; H6280_CYCLES(4); RD_ZPG; LDX;		   } // 4 LDX  ZPG
OP(_0c6) { int tmp; H6280_CYCLES(6); RD_ZPG; DEC; WB_EAZ; } // 6 DEC  ZPG
OP(_0e6) { int tmp; H6280_CYCLES(6); RD_ZPG; INC; WB_EAZ; } // 6 INC  ZPG

OP(_016) { int tmp; H6280_CYCLES(6); RD_ZPX; ASL; WB_EAZ  } // 6 ASL  ZPX
OP(_036) { int tmp; H6280_CYCLES(6); RD_ZPX; ROL; WB_EAZ  } // 6 ROL  ZPX
OP(_056) { int tmp; H6280_CYCLES(6); RD_ZPX; LSR; WB_EAZ  } // 6 LSR  ZPX
OP(_076) { int tmp; H6280_CYCLES(6); RD_ZPX; ROR; WB_EAZ  } // 6 ROR  ZPX
OP(_096) { int tmp; H6280_CYCLES(4);		  STX; WR_ZPY; } // 4 STX  ZPY
OP(_0b6) { int tmp; H6280_CYCLES(4); RD_ZPY; LDX;		   } // 4 LDX  ZPY
OP(_0d6) { int tmp; H6280_CYCLES(6); RD_ZPX; DEC; WB_EAZ; } // 6 DEC  ZPX
OP(_0f6) { int tmp; H6280_CYCLES(6); RD_ZPX; INC; WB_EAZ; } // 6 INC  ZPX

OP(_007) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(0);WB_EAZ;} // 7 RMB0 ZPG
OP(_027) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(2);WB_EAZ;} // 7 RMB2 ZPG
OP(_047) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(4);WB_EAZ;} // 7 RMB4 ZPG
OP(_067) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(6);WB_EAZ;} // 7 RMB6 ZPG
OP(_087) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(0);WB_EAZ;} // 7 SMB0 ZPG
OP(_0a7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(2);WB_EAZ;} // 7 SMB2 ZPG
OP(_0c7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(4);WB_EAZ;} // 7 SMB4 ZPG
OP(_0e7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(6);WB_EAZ;} // 7 SMB6 ZPG

OP(_017) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(1);WB_EAZ;} // 7 RMB1 ZPG
OP(_037) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(3);WB_EAZ;} // 7 RMB3 ZPG
OP(_057) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(5);WB_EAZ;} // 7 RMB5 ZPG
OP(_077) { int tmp; H6280_CYCLES(7); RD_ZPG; RMB(7);WB_EAZ;} // 7 RMB7 ZPG
OP(_097) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(1);WB_EAZ;} // 7 SMB1 ZPG
OP(_0b7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(3);WB_EAZ;} // 7 SMB3 ZPG
OP(_0d7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(5);WB_EAZ;} // 7 SMB5 ZPG
OP(_0f7) { int tmp; H6280_CYCLES(7); RD_ZPG; SMB(7);WB_EAZ;} // 7 SMB7 ZPG

OP(_008) {		   H6280_CYCLES(3);		  PHP;		   } // 3 PHP
OP(_028) {		   H6280_CYCLES(4);		  PLP;		   } // 4 PLP
OP(_048) {		   H6280_CYCLES(3);		  PHA;		   } // 3 PHA
OP(_068) {		   H6280_CYCLES(4);		  PLA;		   } // 4 PLA
OP(_088) {		   H6280_CYCLES(2);		  DEY;		   } // 2 DEY
OP(_0a8) {		   H6280_CYCLES(2);		  TAY;		   } // 2 TAY
OP(_0c8) {		   H6280_CYCLES(2);		  INY;		   } // 2 INY
OP(_0e8) {		   H6280_CYCLES(2);		  INX;		   } // 2 INX

OP(_018) {		   H6280_CYCLES(2);		  CLC;		   } // 2 CLC
OP(_038) {		   H6280_CYCLES(2);		  SEC;		   } // 2 SEC
OP(_058) {		   H6280_CYCLES(2);		  CLI;		   } // 2 CLI
OP(_078) {		   H6280_CYCLES(2);		  SEI;		   } // 2 SEI
OP(_098) {		   H6280_CYCLES(2);		  TYA;		   } // 2 TYA
OP(_0b8) {		   H6280_CYCLES(2);		  CLV;		   } // 2 CLV
OP(_0d8) {		   H6280_CYCLES(2);		  CLD;		   } // 2 CLD
OP(_0f8) {		   H6280_CYCLES(2);		  SED;		   } // 2 SED

OP(_009) { int tmp; H6280_CYCLES(2); RD_IMM; ORA;		   } // 2 ORA  IMM
OP(_029) { int tmp; H6280_CYCLES(2); RD_IMM; AND;		   } // 2 AND  IMM
OP(_049) { int tmp; H6280_CYCLES(2); RD_IMM; EOR;		   } // 2 EOR  IMM
OP(_069) { int tmp; H6280_CYCLES(2); RD_IMM; ADC;		   } // 2 ADC  IMM
OP(_089) { int tmp; H6280_CYCLES(2); RD_IMM; BIT;		   } // 2 BIT  IMM
OP(_0a9) { int tmp; H6280_CYCLES(2); RD_IMM; LDA;		   } // 2 LDA  IMM
OP(_0c9) { int tmp; H6280_CYCLES(2); RD_IMM; CMP;		   } // 2 CMP  IMM
OP(_0e9) { int tmp; H6280_CYCLES(2); RD_IMM; SBC;		   } // 2 SBC  IMM

OP(_019) { int tmp; H6280_CYCLES(5); RD_ABY; ORA;		   } // 5 ORA  ABY
OP(_039) { int tmp; H6280_CYCLES(5); RD_ABY; AND;		   } // 5 AND  ABY
OP(_059) { int tmp; H6280_CYCLES(5); RD_ABY; EOR;		   } // 5 EOR  ABY
OP(_079) { int tmp; H6280_CYCLES(5); RD_ABY; ADC;		   } // 5 ADC  ABY
OP(_099) { int tmp; H6280_CYCLES(5);		  STA; WR_ABY; } // 5 STA  ABY
OP(_0b9) { int tmp; H6280_CYCLES(5); RD_ABY; LDA;		   } // 5 LDA  ABY
OP(_0d9) { int tmp; H6280_CYCLES(5); RD_ABY; CMP;		   } // 5 CMP  ABY
OP(_0f9) { int tmp; H6280_CYCLES(5); RD_ABY; SBC;		   } // 5 SBC  ABY

OP(_00a) { int tmp; H6280_CYCLES(2); RD_ACC; ASL; WB_ACC; } // 2 ASL  A
OP(_02a) { int tmp; H6280_CYCLES(2); RD_ACC; ROL; WB_ACC; } // 2 ROL  A
OP(_04a) { int tmp; H6280_CYCLES(2); RD_ACC; LSR; WB_ACC; } // 2 LSR  A
OP(_06a) { int tmp; H6280_CYCLES(2); RD_ACC; ROR; WB_ACC; } // 2 ROR  A
OP(_08a) {		   H6280_CYCLES(2);		  TXA;		   } // 2 TXA
OP(_0aa) {		   H6280_CYCLES(2);		  TAX;		   } // 2 TAX
OP(_0ca) {		   H6280_CYCLES(2);		  DEX;		   } // 2 DEX
OP(_0ea) {		   H6280_CYCLES(2);		  NOP;		   } // 2 NOP

OP(_01a) {		   H6280_CYCLES(2);		  INA;		   } // 2 INC  A
OP(_03a) {		   H6280_CYCLES(2);		  DEA;		   } // 2 DEC  A
OP(_05a) {		   H6280_CYCLES(3);		  PHY;		   } // 3 PHY
OP(_07a) {		   H6280_CYCLES(4);		  PLY;		   } // 4 PLY
OP(_09a) {		   H6280_CYCLES(2);		  TXS;		   } // 2 TXS
OP(_0ba) {		   H6280_CYCLES(2);		  TSX;		   } // 2 TSX
OP(_0da) {		   H6280_CYCLES(3);		  PHX;		   } // 3 PHX
OP(_0fa) {		   H6280_CYCLES(4);		  PLX;		   } // 4 PLX

OP(_00b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_02b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_04b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_06b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_08b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0ab) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0cb) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0eb) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP

OP(_01b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_03b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_05b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_07b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_09b) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0bb) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0db) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0fb) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP

OP(_00c) { int tmp; H6280_CYCLES(7); RD_ABS; TSB; WB_EA;  } // 7 TSB  ABS
OP(_02c) { int tmp; H6280_CYCLES(5); RD_ABS; BIT;		   } // 5 BIT  ABS
OP(_04c) {		   H6280_CYCLES(4); EA_ABS; JMP;		   } // 4 JMP  ABS
OP(_06c) { int tmp; H6280_CYCLES(7); EA_IND; JMP;		   } // 7 JMP  IND
OP(_08c) { int tmp; H6280_CYCLES(5);		  STY; WR_ABS; } // 5 STY  ABS
OP(_0ac) { int tmp; H6280_CYCLES(5); RD_ABS; LDY;		   } // 5 LDY  ABS
OP(_0cc) { int tmp; H6280_CYCLES(5); RD_ABS; CPY;		   } // 5 CPY  ABS
OP(_0ec) { int tmp; H6280_CYCLES(5); RD_ABS; CPX;		   } // 5 CPX  ABS

OP(_01c) { int tmp; H6280_CYCLES(7); RD_ABS; TRB; WB_EA;  } // 7 TRB  ABS
OP(_03c) { int tmp; H6280_CYCLES(5); RD_ABX; BIT;		   } // 5 BIT  ABX
OP(_05c) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_07c) { int tmp; H6280_CYCLES(7); EA_IAX; JMP;		   } // 7 JMP  IAX
OP(_09c) { int tmp; H6280_CYCLES(5);		  STZ; WR_ABS; } // 5 STZ  ABS
OP(_0bc) { int tmp; H6280_CYCLES(5); RD_ABX; LDY;		   } // 5 LDY  ABX
OP(_0dc) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP
OP(_0fc) {			H6280_CYCLES(2);		  NOP;		   } // 2 NOP

OP(_00d) { int tmp; H6280_CYCLES(5); RD_ABS; ORA;		   } // 5 ORA  ABS
OP(_02d) { int tmp; H6280_CYCLES(5); RD_ABS; AND;		   } // 5 AND  ABS
OP(_04d) { int tmp; H6280_CYCLES(5); RD_ABS; EOR;		   } // 5 EOR  ABS
OP(_06d) { int tmp; H6280_CYCLES(5); RD_ABS; ADC;		   } // 5 ADC  ABS
OP(_08d) { int tmp; H6280_CYCLES(5);		  STA; WR_ABS; } // 5 STA  ABS
OP(_0ad) { int tmp; H6280_CYCLES(5); RD_ABS; LDA;		   } // 5 LDA  ABS
OP(_0cd) { int tmp; H6280_CYCLES(5); RD_ABS; CMP;		   } // 5 CMP  ABS
OP(_0ed) { int tmp; H6280_CYCLES(5); RD_ABS; SBC;		   } // 5 SBC  ABS

OP(_01d) { int tmp; H6280_CYCLES(5); RD_ABX; ORA;		   } // 5 ORA  ABX
OP(_03d) { int tmp; H6280_CYCLES(5); RD_ABX; AND;		   } // 5 AND  ABX
OP(_05d) { int tmp; H6280_CYCLES(5); RD_ABX; EOR;		   } // 5 EOR  ABX
OP(_07d) { int tmp; H6280_CYCLES(5); RD_ABX; ADC;		   } // 5 ADC  ABX
OP(_09d) { int tmp; H6280_CYCLES(5);		  STA; WR_ABX; } // 5 STA  ABX
OP(_0bd) { int tmp; H6280_CYCLES(5); RD_ABX; LDA;		   } // 5 LDA  ABX
OP(_0dd) { int tmp; H6280_CYCLES(5); RD_ABX; CMP;		   } // 5 CMP  ABX
OP(_0fd) { int tmp; H6280_CYCLES(5); RD_ABX; SBC;		   } // 5 SBC  ABX

OP(_00e) { int tmp; H6280_CYCLES(7); RD_ABS; ASL; WB_EA;  } // 7 ASL  ABS
OP(_02e) { int tmp; H6280_CYCLES(7); RD_ABS; ROL; WB_EA;  } // 7 ROL  ABS
OP(_04e) { int tmp; H6280_CYCLES(7); RD_ABS; LSR; WB_EA;  } // 7 LSR  ABS
OP(_06e) { int tmp; H6280_CYCLES(7); RD_ABS; ROR; WB_EA;  } // 7 ROR  ABS
OP(_08e) { int tmp; H6280_CYCLES(5);		  STX; WR_ABS; } // 5 STX  ABS
OP(_0ae) { int tmp; H6280_CYCLES(5); RD_ABS; LDX;		   } // 5 LDX  ABS
OP(_0ce) { int tmp; H6280_CYCLES(7); RD_ABS; DEC; WB_EA;  } // 7 DEC  ABS
OP(_0ee) { int tmp; H6280_CYCLES(7); RD_ABS; INC; WB_EA;  } // 7 INC  ABS

OP(_01e) { int tmp; H6280_CYCLES(7); RD_ABX; ASL; WB_EA;  } // 7 ASL  ABX
OP(_03e) { int tmp; H6280_CYCLES(7); RD_ABX; ROL; WB_EA;  } // 7 ROL  ABX
OP(_05e) { int tmp; H6280_CYCLES(7); RD_ABX; LSR; WB_EA;  } // 7 LSR  ABX
OP(_07e) { int tmp; H6280_CYCLES(7); RD_ABX; ROR; WB_EA;  } // 7 ROR  ABX
OP(_09e) { int tmp; H6280_CYCLES(5);		  STZ; WR_ABX; } // 5 STZ  ABX
OP(_0be) { int tmp; H6280_CYCLES(5); RD_ABY; LDX;		   } // 5 LDX  ABY
OP(_0de) { int tmp; H6280_CYCLES(7); RD_ABX; DEC; WB_EA;  } // 7 DEC  ABX
OP(_0fe) { int tmp; H6280_CYCLES(7); RD_ABX; INC; WB_EA;  } // 7 INC  ABX

OP(_00f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(0);	   } // 6/8 BBR0 ZPG,REL
OP(_02f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(2);	   } // 6/8 BBR2 ZPG,REL
OP(_04f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(4);	   } // 6/8 BBR4 ZPG,REL
OP(_06f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(6);	   } // 6/8 BBR6 ZPG,REL
OP(_08f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(0);	   } // 6/8 BBS0 ZPG,REL
OP(_0af) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(2);	   } // 6/8 BBS2 ZPG,REL
OP(_0cf) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(4);	   } // 6/8 BBS4 ZPG,REL
OP(_0ef) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(6);	   } // 6/8 BBS6 ZPG,REL

OP(_01f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(1);	   } // 6/8 BBR1 ZPG,REL
OP(_03f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(3);	   } // 6/8 BBR3 ZPG,REL
OP(_05f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(5);	   } // 6/8 BBR5 ZPG,REL
OP(_07f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBR(7);	   } // 6/8 BBR7 ZPG,REL
OP(_09f) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(1);	   } // 6/8 BBS1 ZPG,REL
OP(_0bf) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(3);	   } // 6/8 BBS3 ZPG,REL
OP(_0df) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(5);	   } // 6/8 BBS5 ZPG,REL
OP(_0ff) { int tmp; H6280_CYCLES(4); RD_ZPG; BBS(7);	   } // 6/8 BBS7 ZPG,REL

static void (*const insnh6280[0x100])(void) = {
	h6280_000,h6280_001,h6280_002,h6280_003,h6280_004,h6280_005,h6280_006,h6280_007,
	h6280_008,h6280_009,h6280_00a,h6280_00b,h6280_00c,h6280_00d,h6280_00e,h6280_00f,
	h6280_010,h6280_011,h6280_012,h6280_013,h6280_014,h6280_015,h6280_016,h6280_017,
	h6280_018,h6280_019,h6280_01a,h6280_01b,h6280_01c,h6280_01d,h6280_01e,h6280_01f,
	h6280_020,h6280_021,h6280_022,h6280_023,h6280_024,h6280_025,h6280_026,h6280_027,
	h6280_028,h6280_029,h6280_02a,h6280_02b,h6280_02c,h6280_02d,h6280_02e,h6280_02f,
	h6280_030,h6280_031,h6280_032,h6280_033,h6280_034,h6280_035,h6280_036,h6280_037,
	h6280_038,h6280_039,h6280_03a,h6280_03b,h6280_03c,h6280_03d,h6280_03e,h6280_03f,
	h6280_040,h6280_041,h6280_042,h6280_043,h6280_044,h6280_045,h6280_046,h6280_047,
	h6280_048,h6280_049,h6280_04a,h6280_04b,h6280_04c,h6280_04d,h6280_04e,h6280_04f,
	h6280_050,h6280_051,h6280_052,h6280_053,h6280_054,h6280_055,h6280_056,h6280_057,
	h6280_058,h6280_059,h6280_05a,h6280_05b,h6280_05c,h6280_05d,h6280_05e,h6280_05f,
	h6280_060,h6280_061,h6280_062,h6280_063,h6280_064,h6280_065,h6280_066,h6280_067,
	h6280_068,h6280_069,h6280_06a,h6280_06b,h6280_06c,h6280_06d,h6280_06e,h6280_06f,
	h6280_070,h6280_071,h6280_072,h6280_073,h6280_074,h6280_075,h6280_076,h6280_077,
	h6280_078,h6280_079,h6280_07a,h6280_07b,h6280_07c,h6280_07d,h6280_07e,h6280_07f,
	h6280_080,h6280_081,h6280_082,h6280_083,h6280_084,h6280_085,h6280_086,h6280_087,
	h6280_088,h6280_089,h6280_08a,h6280_08b,h6280_08c,h6280_08d,h6280_08e,h6280_08f,
	h6280_090,h6280_091,h6280_092,h6280_093,h6280_094,h6280_095,h6280_096,h6280_097,
	h6280_098,h6280_099,h6280_09a,h6280_09b,h6280_09c,h6280_09d,h6280_09e,h6280_09f,
	h6280_0a0,h6280_0a1,h6280_0a2,h6280_0a3,h6280_0a4,h6280_0a5,h6280_0a6,h6280_0a7,
	h6280_0a8,h6280_0a9,h6280_0aa,h6280_0ab,h6280_0ac,h6280_0ad,h6280_0ae,h6280_0af,
	h6280_0b0,h6280_0b1,h6280_0b2,h6280_0b3,h6280_0b4,h6280_0b5,h6280_0b6,h6280_0b7,
	h6280_0b8,h6280_0b9,h6280_0ba,h6280_0bb,h6280_0bc,h6280_0bd,h6280_0be,h6280_0bf,
	h6280_0c0,h6280_0c1,h6280_0c2,h6280_0c3,h6280_0c4,h6280_0c5,h6280_0c6,h6280_0c7,
	h6280_0c8,h6280_0c9,h6280_0ca,h6280_0cb,h6280_0cc,h6280_0cd,h6280_0ce,h6280_0cf,
	h6280_0d0,h6280_0d1,h6280_0d2,h6280_0d3,h6280_0d4,h6280_0d5,h6280_0d6,h6280_0d7,
	h6280_0d8,h6280_0d9,h6280_0da,h6280_0db,h6280_0dc,h6280_0dd,h6280_0de,h6280_0df,
	h6280_0e0,h6280_0e1,h6280_0e2,h6280_0e3,h6280_0e4,h6280_0e5,h6280_0e6,h6280_0e7,
	h6280_0e8,h6280_0e9,h6280_0ea,h6280_0eb,h6280_0ec,h6280_0ed,h6280_0ee,h6280_0ef,
	h6280_0f0,h6280_0f1,h6280_0f2,h6280_0f3,h6280_0f4,h6280_0f5,h6280_0f6,h6280_0f7,
	h6280_0f8,h6280_0f9,h6280_0fa,h6280_0fb,h6280_0fc,h6280_0fd,h6280_0fe,h6280_0ff
};
