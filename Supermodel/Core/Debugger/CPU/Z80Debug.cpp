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
 * Z80Debug.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Z80Debug.h"
#include "CPU/Z80/Z80.h"

#include <string>

namespace Debugger
{
	// Instruction templates
	static char *templates[5][256] = {
		{
			// Table 0: single byte instructions
			"NOP","LD BC,@a","LD (BC),A","INC BC","INC B","DEC B","LD B,@d","RLCA",
			"EX AF,AF'","ADD HL,BC","LD A,(BC)","DEC BC","INC C","DEC C","LD C,@d","RRCA",
			"DJNZ @b","LD DE,@a","LD (DE),A","INC DE","INC D","DEC D","LD D,@d","RLA",
			"JR @b","ADD HL,DE","LD A,(DE)","DEC DE","INC E","DEC E","LD E,@d","RRA",
			"JR NZ,@b","LD HL,@a","LD (@a),HL","INC HL","INC H","DEC H","LD H,@d","DAA",
			"JR Z,@b","ADD HL,HL","LD HL,(@a)","DEC HL","INC L","DEC L","LD L,@d","CPL",
			"JR NC,@b","LD SP,@a","LD (@a),A","INC SP","INC (HL)","DEC (HL)","LD (HL),@d","SCF",
			"JR C,@b","ADD HL,SP","LD A,(@a)","DEC SP","INC A","DEC A","LD A,@d","CCF",
			"LD B,B","LD B,C","LD B,D","LD B,E","LD B,H","LD B,L","LD B,(HL)","LD B,A",
			"LD C,B","LD C,C","LD C,D","LD C,E","LD C,H","LD C,L","LD C,(HL)","LD C,A",
			"LD D,B","LD D,C","LD D,D","LD D,E","LD D,H","LD D,L","LD D,(HL)","LD D,A",
			"LD E,B","LD E,C","LD E,D","LD E,E","LD E,H","LD E,L","LD E,(HL)","LD E,A",
			"LD H,B","LD H,C","LD H,D","LD H,E","LD H,H","LD H,L","LD H,(HL)","LD H,A",
			"LD L,B","LD L,C","LD L,D","LD L,E","LD L,H","LD L,L","LD L,(HL)","LD L,A",
			"LD (HL),B","LD (HL),C","LD (HL),D","LD (HL),E","LD (HL),H","LD (HL),L","HALT","LD (HL),A",
			"LD A,B","LD A,C","LD A,D","LD A,E","LD A,H","LD A,L","LD A,(HL)","LD A,A",
			"ADD B","ADD C","ADD D","ADD E","ADD H","ADD L","ADD (HL)","ADD A",
			"ADC B","ADC C","ADC D","ADC E","ADC H","ADC L","ADC (HL)","ADC A",
			"SUB B","SUB C","SUB D","SUB E","SUB H","SUB L","SUB (HL)","SUB A",
			"SBC B","SBC C","SBC D","SBC E","SBC H","SBC L","SBC (HL)","SBC A",
			"AND B","AND C","AND D","AND E","AND H","AND L","AND (HL)","AND A",
			"XOR B","XOR C","XOR D","XOR E","XOR H","XOR L","XOR (HL)","XOR A",
			"OR B","OR C","OR D","OR E","OR H","OR L","OR (HL)","OR A",
			"CP B","CP C","CP D","CP E","CP H","CP L","CP (HL)","CP A",
			"RET NZ","POP BC","JP NZ,@j","JP @j","CALL NZ,@j","PUSH BC","ADD @d","RST 00h",
			"RET Z","RET","JP Z,@j","PFX_CB","CALL Z,@j","CALL @j","ADC @d","RST 08h",
			"RET NC","POP DE","JP NC,@j","OUTA (@p)","CALL NC,@j","PUSH DE","SUB @d","RST 10h",
			"RET C","EXX","JP C,@j","INA (@p)","CALL C,@j","PFX_DD","SBC @d","RST 18h",
			"RET PO","POP HL","JP PO,@j","EX HL,(SP)","CALL PO,@j","PUSH HL","AND @d","RST 20h",
			"RET PE","LD PC,HL","JP PE,@j","EX DE,HL","CALL PE,@j","PFX_ED","XOR @d","RST 28h",
			"RET P","POP AF","JP P,@j","DI","CALL P,@j","PUSH AF","OR @d","RST 30h",
			"RET M","LD SP,HL","JP M,@j","EI","CALL M,@j","PFX_FD","CP @d","RST 38h"
		}, {
			// Table 1: two byte instructions of form CB-XX
			"RLC B","RLC C","RLC D","RLC E","RLC H","RLC L","RLC (HL)","RLC A",
			"RRC B","RRC C","RRC D","RRC E","RRC H","RRC L","RRC (HL)","RRC A",
			"RL B","RL C","RL D","RL E","RL H","RL L","RL (HL)","RL A",
			"RR B","RR C","RR D","RR E","RR H","RR L","RR (HL)","RR A",
			"SLA B","SLA C","SLA D","SLA E","SLA H","SLA L","SLA (HL)","SLA A",
			"SRA B","SRA C","SRA D","SRA E","SRA H","SRA L","SRA (HL)","SRA A",
			"SLL B","SLL C","SLL D","SLL E","SLL H","SLL L","SLL (HL)","SLL A",
			"SRL B","SRL C","SRL D","SRL E","SRL H","SRL L","SRL (HL)","SRL A",
			"BIT 0,B","BIT 0,C","BIT 0,D","BIT 0,E","BIT 0,H","BIT 0,L","BIT 0,(HL)","BIT 0,A",
			"BIT 1,B","BIT 1,C","BIT 1,D","BIT 1,E","BIT 1,H","BIT 1,L","BIT 1,(HL)","BIT 1,A",
			"BIT 2,B","BIT 2,C","BIT 2,D","BIT 2,E","BIT 2,H","BIT 2,L","BIT 2,(HL)","BIT 2,A",
			"BIT 3,B","BIT 3,C","BIT 3,D","BIT 3,E","BIT 3,H","BIT 3,L","BIT 3,(HL)","BIT 3,A",
			"BIT 4,B","BIT 4,C","BIT 4,D","BIT 4,E","BIT 4,H","BIT 4,L","BIT 4,(HL)","BIT 4,A",
			"BIT 5,B","BIT 5,C","BIT 5,D","BIT 5,E","BIT 5,H","BIT 5,L","BIT 5,(HL)","BIT 5,A",
			"BIT 6,B","BIT 6,C","BIT 6,D","BIT 6,E","BIT 6,H","BIT 6,L","BIT 6,(HL)","BIT 6,A",
			"BIT 7,B","BIT 7,C","BIT 7,D","BIT 7,E","BIT 7,H","BIT 7,L","BIT 7,(HL)","BIT 7,A",
			"RES 0,B","RES 0,C","RES 0,D","RES 0,E","RES 0,H","RES 0,L","RES 0,(HL)","RES 0,A",
			"RES 1,B","RES 1,C","RES 1,D","RES 1,E","RES 1,H","RES 1,L","RES 1,(HL)","RES 1,A",
			"RES 2,B","RES 2,C","RES 2,D","RES 2,E","RES 2,H","RES 2,L","RES 2,(HL)","RES 2,A",
			"RES 3,B","RES 3,C","RES 3,D","RES 3,E","RES 3,H","RES 3,L","RES 3,(HL)","RES 3,A",
			"RES 4,B","RES 4,C","RES 4,D","RES 4,E","RES 4,H","RES 4,L","RES 4,(HL)","RES 4,A",
			"RES 5,B","RES 5,C","RES 5,D","RES 5,E","RES 5,H","RES 5,L","RES 5,(HL)","RES 5,A",
			"RES 6,B","RES 6,C","RES 6,D","RES 6,E","RES 6,H","RES 6,L","RES 6,(HL)","RES 6,A",
			"RES 7,B","RES 7,C","RES 7,D","RES 7,E","RES 7,H","RES 7,L","RES 7,(HL)","RES 7,A",
			"SET 0,B","SET 0,C","SET 0,D","SET 0,E","SET 0,H","SET 0,L","SET 0,(HL)","SET 0,A",
			"SET 1,B","SET 1,C","SET 1,D","SET 1,E","SET 1,H","SET 1,L","SET 1,(HL)","SET 1,A",
			"SET 2,B","SET 2,C","SET 2,D","SET 2,E","SET 2,H","SET 2,L","SET 2,(HL)","SET 2,A",
			"SET 3,B","SET 3,C","SET 3,D","SET 3,E","SET 3,H","SET 3,L","SET 3,(HL)","SET 3,A",
			"SET 4,B","SET 4,C","SET 4,D","SET 4,E","SET 4,H","SET 4,L","SET 4,(HL)","SET 4,A",
			"SET 5,B","SET 5,C","SET 5,D","SET 5,E","SET 5,H","SET 5,L","SET 5,(HL)","SET 5,A",
			"SET 6,B","SET 6,C","SET 6,D","SET 6,E","SET 6,H","SET 6,L","SET 6,(HL)","SET 6,A",
			"SET 7,B","SET 7,C","SET 7,D","SET 7,E","SET 7,H","SET 7,L","SET 7,(HL)","SET 7,A"
		}, {
			// Table 2: two byte instructions of form ED-XX
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			"IN B,(C)","OUT (C),B","SBC HL,BC","LD (@a),BC","NEG","RETN","IM 0","LD I,A",
			"IN C,(C)","OUT (C),C","ADC HL,BC","LD BC,(@a)",NULL,"RETI",NULL,"LD R,A",
			"IN D,(C)","OUT (C),D","SBC HL,DE","LD (@a),DE",NULL,NULL,"IM 1","LD A,I",
			"IN E,(C)","OUT (C),E","ADC HL,DE","LD DE,(@a)",NULL,NULL,"IM 2","LD A,R",
			"IN H,(C)","OUT (C),H","SBC HL,HL","LD (@a),HL",NULL,NULL,NULL,"RRD",
			"IN L,(C)","OUT (C),L","ADC HL,HL","LD HL,(@a)",NULL,NULL,NULL,"RLD",
			"IN F,(C)",NULL,"SBC HL,SP","LD (@a),SP",NULL,NULL,NULL,NULL,
			"IN A,(C)","OUT (C),A","ADC HL,SP","LD SP,(@a)",NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			"LDI","CPI","INI","OUTI",NULL,NULL,NULL,NULL,
			"LDD","CPD","IND","OUTD",NULL,NULL,NULL,NULL,
			"LDIR","CPIR","INIR","OTIR",NULL,NULL,NULL,NULL,
			"LDDR","CPDR","INDR","OTDR",NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
		}, {	
			// Table 3: two byte instructions of form DD-XX or FD-XX
			"NOP","LD BC,@a","LD (BC),A","INC BC","INC B","DEC B","LD B,@d","RLCA",
			"EX AF,AF'","ADD I?,BC","LD A,(BC)","DEC BC","INC C","DEC C","LD C,@d","RRCA",
			"DJNZ @b","LD DE,@a","LD (DE),A","INC DE","INC D","DEC D","LD D,@d","RLA",
			"JR @b","ADD I?,DE","LD A,(DE)","DEC DE","INC E","DEC E","LD E,@d","RRA",
			"JR NZ,@b","LD I?,@a","LD (@a),I?","INC I?","INC I?h","DEC I?h","LD I?h,@d","DAA",
			"JR Z,@b","ADD I?,I?","LD I?,(@a)","DEC I?","INC I?l","DEC I?l","LD I?l,@d","CPL",
			"JR NC,@b","LD SP,@a","LD (@a),A","INC SP","INC (I?+@d)","DEC (I?+@d)","LD (I?+@d),@d","SCF",
			"JR C,@b","ADD I?,SP","LD A,(@a)","DEC SP","INC A","DEC A","LD A,@d","CCF",
			"LD B,B","LD B,C","LD B,D","LD B,E","LD B,I?h","LD B,I?l","LD B,(I?+@d)","LD B,A",
			"LD C,B","LD C,C","LD C,D","LD C,E","LD C,I?h","LD C,I?l","LD C,(I?+@d)","LD C,A",
			"LD D,B","LD D,C","LD D,D","LD D,E","LD D,I?h","LD D,I?l","LD D,(I?+@d)","LD D,A",
			"LD E,B","LD E,C","LD E,D","LD E,E","LD E,I?h","LD E,I?l","LD E,(I?+@d)","LD E,A",
			"LD I?h,B","LD I?h,C","LD I?h,D","LD I?h,E","LD I?h,I?h","LD I?h,I?l","LD H,(I?+@d)","LD I?h,A",
			"LD I?l,B","LD I?l,C","LD I?l,D","LD I?l,E","LD I?l,I?h","LD I?l,I?l","LD L,(I?+@d)","LD I?l,A",
			"LD (I?+@d),B","LD (I?+@d),C","LD (I?+@d),D","LD (I?+@d),E","LD (I?+@d),H","LD (I?+@d),L","HALT","LD (I?+@d),A",
			"LD A,B","LD A,C","LD A,D","LD A,E","LD A,I?h","LD A,I?l","LD A,(I?+@d)","LD A,A",
			"ADD B","ADD C","ADD D","ADD E","ADD I?h","ADD I?l","ADD (I?+@d)","ADD A",
			"ADC B","ADC C","ADC D","ADC E","ADC I?h","ADC I?l","ADC (I?+@d)","ADC,A",
			"SUB B","SUB C","SUB D","SUB E","SUB I?h","SUB I?l","SUB (I?+@d)","SUB A",
			"SBC B","SBC C","SBC D","SBC E","SBC I?h","SBC I?l","SBC (I?+@d)","SBC A",
			"AND B","AND C","AND D","AND E","AND I?h","AND I?l","AND (I?+@d)","AND A",
			"XOR B","XOR C","XOR D","XOR E","XOR I?h","XOR I?l","XOR (I?+@d)","XOR A",
			"OR B","OR C","OR D","OR E","OR I?h","OR I?l","OR (I?+@d)","OR A",
			"CP B","CP C","CP D","CP E","CP I?h","CP I?l","CP (I?+@d)","CP A",
			"RET NZ","POP BC","JP NZ,@a","JP @a","CALL NZ,@a","PUSH BC","ADD @d","RST 00h",
			"RET Z","RET","JP Z,@a","PFX_CB","CALL Z,@a","CALL @a","ADC @d","RST 08h",
			"RET NC","POP DE","JP NC,@a","OUTA (@p)","CALL NC,@a","PUSH DE","SUB @d","RST 10h",
			"RET C","EXX","JP C,@a","INA (@p)","CALL C,@a","PFX_DD","SBC @d","RST 18h",
			"RET PO","POP I?","JP PO,@a","EX I?,(SP)","CALL PO,@a","PUSH I?","AND @d","RST 20h",
			"RET PE","LD PC,I?","JP PE,@a","EX DE,I?","CALL PE,@a","PFX_ED","XOR @d","RST 28h",
			"RET P","POP AF","JP P,@a","DI","CALL P,@a","PUSH AF","OR @d","RST 30h",
			"RET M","LD SP,I?","JP M,@a","EI","CALL M,@a","PFX_FD","CP @d","RST 38h"
		}, {
			// Table 4: three byte instructions of form DD-CB-XX or FD-CB-XX
			"RLC B","RLC C","RLC D","RLC E","RLC H","RLC L","RLC (I?@o)","RLC A",
			"RRC B","RRC C","RRC D","RRC E","RRC H","RRC L","RRC (I?@o)","RRC A",
			"RL B","RL C","RL D","RL E","RL H","RL L","RL (I?@o)","RL A",
			"RR B","RR C","RR D","RR E","RR H","RR L","RR (I?@o)","RR A",
			"SLA B","SLA C","SLA D","SLA E","SLA H","SLA L","SLA (I?@o)","SLA A",
			"SRA B","SRA C","SRA D","SRA E","SRA H","SRA L","SRA (I?@o)","SRA A",
			"SLL B","SLL C","SLL D","SLL E","SLL H","SLL L","SLL (I?@o)","SLL A",
			"SRL B","SRL C","SRL D","SRL E","SRL H","SRL L","SRL (I?@o)","SRL A",
			"BIT 0,B","BIT 0,C","BIT 0,D","BIT 0,E","BIT 0,H","BIT 0,L","BIT 0,(I?@o)","BIT 0,A",
			"BIT 1,B","BIT 1,C","BIT 1,D","BIT 1,E","BIT 1,H","BIT 1,L","BIT 1,(I?@o)","BIT 1,A",
			"BIT 2,B","BIT 2,C","BIT 2,D","BIT 2,E","BIT 2,H","BIT 2,L","BIT 2,(I?@o)","BIT 2,A",
			"BIT 3,B","BIT 3,C","BIT 3,D","BIT 3,E","BIT 3,H","BIT 3,L","BIT 3,(I?@o)","BIT 3,A",
			"BIT 4,B","BIT 4,C","BIT 4,D","BIT 4,E","BIT 4,H","BIT 4,L","BIT 4,(I?@o)","BIT 4,A",
			"BIT 5,B","BIT 5,C","BIT 5,D","BIT 5,E","BIT 5,H","BIT 5,L","BIT 5,(I?@o)","BIT 5,A",
			"BIT 6,B","BIT 6,C","BIT 6,D","BIT 6,E","BIT 6,H","BIT 6,L","BIT 6,(I?@o)","BIT 6,A",
			"BIT 7,B","BIT 7,C","BIT 7,D","BIT 7,E","BIT 7,H","BIT 7,L","BIT 7,(I?@o)","BIT 7,A",
			"RES 0,B","RES 0,C","RES 0,D","RES 0,E","RES 0,H","RES 0,L","RES 0,(I?@o)","RES 0,A",
			"RES 1,B","RES 1,C","RES 1,D","RES 1,E","RES 1,H","RES 1,L","RES 1,(I?@o)","RES 1,A",
			"RES 2,B","RES 2,C","RES 2,D","RES 2,E","RES 2,H","RES 2,L","RES 2,(I?@o)","RES 2,A",
			"RES 3,B","RES 3,C","RES 3,D","RES 3,E","RES 3,H","RES 3,L","RES 3,(I?@o)","RES 3,A",
			"RES 4,B","RES 4,C","RES 4,D","RES 4,E","RES 4,H","RES 4,L","RES 4,(I?@o)","RES 4,A",
			"RES 5,B","RES 5,C","RES 5,D","RES 5,E","RES 5,H","RES 5,L","RES 5,(I?@o)","RES 5,A",
			"RES 6,B","RES 6,C","RES 6,D","RES 6,E","RES 6,H","RES 6,L","RES 6,(I?@o)","RES 6,A",
			"RES 7,B","RES 7,C","RES 7,D","RES 7,E","RES 7,H","RES 7,L","RES 7,(I?@o)","RES 7,A",
			"SET 0,B","SET 0,C","SET 0,D","SET 0,E","SET 0,H","SET 0,L","SET 0,(I?@o)","SET 0,A",
			"SET 1,B","SET 1,C","SET 1,D","SET 1,E","SET 1,H","SET 1,L","SET 1,(I?@o)","SET 1,A",
			"SET 2,B","SET 2,C","SET 2,D","SET 2,E","SET 2,H","SET 2,L","SET 2,(I?@o)","SET 2,A",
			"SET 3,B","SET 3,C","SET 3,D","SET 3,E","SET 3,H","SET 3,L","SET 3,(I?@o)","SET 3,A",
			"SET 4,B","SET 4,C","SET 4,D","SET 4,E","SET 4,H","SET 4,L","SET 4,(I?@o)","SET 4,A",
			"SET 5,B","SET 5,C","SET 5,D","SET 5,E","SET 5,H","SET 5,L","SET 5,(I?@o)","SET 5,A",
			"SET 6,B","SET 6,C","SET 6,D","SET 6,E","SET 6,H","SET 6,L","SET 6,(I?@o)","SET 6,A",
			"SET 7,B","SET 7,C","SET 7,D","SET 7,E","SET 7,H","SET 7,L","SET 7,(I?@o)","SET 7,A"
		}
	};

	UINT8 CZ80Debug::ReadReg8(CCPUDebug *cpu, unsigned reg8)
	{
		return ((CZ80Debug*)cpu)->m_z80->GetReg8(reg8);
	}

	bool CZ80Debug::WriteReg8(CCPUDebug *cpu, unsigned reg8, UINT8 value)
	{
		return ((CZ80Debug*)cpu)->m_z80->SetReg8(reg8, value);
	}

	UINT16 CZ80Debug::ReadReg16(CCPUDebug *cpu, unsigned reg16)
	{
		return ((CZ80Debug*)cpu)->m_z80->GetReg16(reg16);
	}

	bool CZ80Debug::WriteReg16(CCPUDebug *cpu, unsigned reg16, UINT16 value)
	{
		return ((CZ80Debug*)cpu)->m_z80->SetReg16(reg16, value);
	}

	static const char *crGroup  = "Control Regs";
	static const char *irGroup  = "Int/Refresh Regs";
	static const char *r8Group  = "8-Bit Regs";
	static const char *r16Group = "16-Bit Regs";
	static const char *srGroup  = "Shadow Regs";

	CZ80Debug::CZ80Debug(const char *name, CZ80 *z80) : CCPUDebug("Z80", name, 1, 4, false, 16, 4), m_z80(z80)
	{	
		// Main registers
		AddPCRegister     ("PC", crGroup);
		AddInt16Register  ("SP", crGroup, Z80_REG16_SP, ReadReg16, WriteReg16);
		AddStatus8Register("F",  crGroup, Z80_REG8_F, "SZ.HPVNC", ReadReg8, WriteReg8);

		AddStatus8Register("IFF", irGroup, Z80_REG8_IFF, "H.E.G21F", ReadReg8, WriteReg8);
		AddInt8Register   ("I",   irGroup, Z80_REG8_I,   ReadReg8, WriteReg8);
		AddInt8Register   ("R",   irGroup, Z80_REG8_R,   ReadReg8, WriteReg8);

		AddInt8Register("A", r8Group, Z80_REG8_A, ReadReg8, WriteReg8);
		AddInt8Register("B", r8Group, Z80_REG8_B, ReadReg8, WriteReg8);
		AddInt8Register("C", r8Group, Z80_REG8_C, ReadReg8, WriteReg8);
		AddInt8Register("D", r8Group, Z80_REG8_D, ReadReg8, WriteReg8);
		AddInt8Register("E", r8Group, Z80_REG8_E, ReadReg8, WriteReg8);

		AddInt16Register("AF", r16Group, Z80_REG16_AF, ReadReg16, WriteReg16);
		AddInt16Register("BC", r16Group, Z80_REG16_BC, ReadReg16, WriteReg16);
		AddInt16Register("DE", r16Group, Z80_REG16_DE, ReadReg16, WriteReg16);
		AddInt16Register("HL", r16Group, Z80_REG16_HL, ReadReg16, WriteReg16);
		AddInt16Register("IX", r16Group, Z80_REG16_IX, ReadReg16, WriteReg16);
		AddInt16Register("IY", r16Group, Z80_REG16_IY, ReadReg16, WriteReg16);
		
		AddInt16Register("AF'", srGroup, Z80_REG16_AF_, ReadReg16, WriteReg16);
		AddInt16Register("BC'", srGroup, Z80_REG16_BC_, ReadReg16, WriteReg16);
		AddInt16Register("DE'", srGroup, Z80_REG16_DE_, ReadReg16, WriteReg16);
		AddInt16Register("HL'", srGroup, Z80_REG16_HL_, ReadReg16, WriteReg16);

		// Exceptions
		AddException("NMI",    Z80_EX_NMI,     "Non-Maskable Interrupt");
		AddException("RST00",  Z80_INT_RST_00, "RST 00h (IM0)");
		AddException("RST08",  Z80_INT_RST_08, "RST 08h (IM0)");
		AddException("RST10",  Z80_INT_RST_10, "RST 10h (IM0)");
		AddException("RST18",  Z80_INT_RST_18, "RST 18h (IM0)");
		AddException("RST20",  Z80_INT_RST_20, "RST 20h (IM0)");
		AddException("RST28",  Z80_INT_RST_28, "RST 28h (IM0)");
		AddException("RST30",  Z80_INT_RST_30, "RST 30h (IM0)");
		AddException("RST38",  Z80_INT_RST_38, "RST 38h (IM0)");
		AddException("IRQ",    Z80_IM1_IRQ,    "Hardwired IRQ (IM1)");
		AddException("VECTOR", Z80_IM2_VECTOR, "Interrupt Vectors (IM2)");

		// Interrupts
		for (unsigned vecNum = 0; vecNum < 256; vecNum++)
		{
			sprintf(m_vecIds[vecNum], "V%u", vecNum + 1);
			sprintf(m_vecNames[vecNum], "Interrupt Vector #%u", vecNum + 1);
			AddInterrupt(m_vecIds[vecNum], vecNum, m_vecNames[vecNum]);
		}

		// I/O ports
		for (unsigned portNum = 0; portNum < 256; portNum++)
		{
			sprintf(m_portNames[portNum], "Port #%u", portNum);
			AddPortIO(portNum, 1, m_portNames[portNum], "I/O Ports");
		}
	}

	CZ80Debug::~CZ80Debug()
	{
		DetachFromCPU();	
	}

	void CZ80Debug::AttachToCPU()
	{
		m_z80->AttachDebugger(this);
	}

	::CBus *CZ80Debug::AttachBus(::CBus *bus)
	{
		m_bus = bus;
		return this;
	}

	void CZ80Debug::DetachFromCPU()
	{
		m_z80->DetachDebugger();
	}

	::CBus *CZ80Debug::DetachBus()
	{
		::CBus *bus = m_bus;
		m_bus = NULL;
		return bus;
	}

	UINT32 CZ80Debug::GetResetAddr()
	{
		return 0x0000;
	}

	bool CZ80Debug::UpdatePC(UINT32 pc)
	{
		return m_z80->SetReg16(Z80_REG16_PC, pc);
	}

	bool CZ80Debug::ForceException(CException *ex)
	{
		if (ex->code != Z80_EX_NMI)
		{
			// TODO
			return false;
		}
		m_z80->TriggerNMI();
		return true;
	}

	bool CZ80Debug::ForceInterrupt(CInterrupt *in)
	{
		// TODO
		return false;
	}

	UINT64 CZ80Debug::ReadMem(UINT32 addr, unsigned dataSize)
	{
		if (dataSize == 1)
			return m_bus->Read8(addr);
		// TODO - byte swapping
		return CCPUDebug::ReadMem(addr, dataSize);
	}

	bool CZ80Debug::WriteMem(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		if (dataSize == 1)
		{
			m_bus->Write8(addr, (UINT8)data);
			return true;
		}
		// TODO - byte swapping
		return CCPUDebug::WriteMem(addr, dataSize, data);
	}

	UINT64 CZ80Debug::ReadPort(UINT16 portNum)
	{
		return m_bus->IORead8(portNum);
	}

	bool CZ80Debug::WritePort(UINT16 portNum, UINT64 data)
	{
		m_bus->IOWrite8(portNum, (UINT8)data);
		return true;
	}

	int CZ80Debug::Disassemble(UINT32 addr, char *mnemonic, char *operands)
	{
		char tmpStr[128], valStr[50], *templ, *pos;
		INT8 offs;
		EOpFlags opFlags;

		UINT16 dAddr = addr;
		UINT16 tAddr;
		char xyChr = '\0';
		bool notJump = false;

		// Get instruction template from opcode
		UINT8 opcode = m_bus->Read8(dAddr++);
		UINT8 nextCode;
		switch (opcode)
		{
			case 0xCB: 
				templ = templates[1][m_bus->Read8(dAddr++)];
				break;
			case 0xED:
				templ = templates[2][m_bus->Read8(dAddr++)];
				break;
			case 0xDD: 
				xyChr = 'X';
				nextCode = m_bus->Read8(dAddr++);
				if (nextCode == 0xCB) 
				{
					offs = m_bus->Read8(dAddr++);
					notJump = true;
					templ = templates[4][m_bus->Read8(dAddr++)]; 
				}
				else
					templ = templates[3][nextCode];
				break;
			case 0xFD: 
				xyChr = 'Y';
				nextCode = m_bus->Read8(dAddr++);
				if (nextCode == 0xCB)
				{
					offs = m_bus->Read8(dAddr++);
					notJump = true;
					templ = templates[4][m_bus->Read8(dAddr++)]; 
				}
				else
					templ = templates[3][nextCode];
				break;
			default:
				templ = templates[0][opcode];
				break;
		}

		// If no template found, then instruction is invalid
		if (templ == NULL)
			return -1;

		// See if instruction has any operands
		if (pos = strchr(templ, ' '))
		{
			// If so, substitute all @ parameters in template operand
			strncpy(mnemonic, templ, pos - templ);
			mnemonic[pos - templ] = '\0';
			operands[0] = '\0';
			char *opPos = operands;
			templ = pos + 1;
			for (;;)
			{
				if (pos = strchr(templ, '@'))
				{
					strncpy(opPos, templ, pos - templ);
					opPos[pos - templ] = '\0';
					// Format data for parameter
					switch (*(pos + 1))
					{
						case 'd':    // ^h or *h
							FormatData(valStr, 1, m_bus->Read8(dAddr++));
							break; 
						case 'p':    // *p  
							FormatPortNum(valStr, m_bus->Read8(dAddr++), true);
							break;
						case 'b':    // @H
							offs = m_bus->Read8(dAddr++);
							tAddr = dAddr + offs;
							opFlags = GetOpFlags(addr, opcode);
							FormatJumpAddress(valStr, tAddr, opFlags);
							break;
						case 'o':    // @h
							if (notJump)
							{
								// Keep argument as +/-offs
								strcat(operands, (offs&0x80 ? "-" : "+"));
								FormatData(valStr, 1, (offs&0x80 ? 256 - offs : offs));
							}
							else
							{
								offs = m_bus->Read8(dAddr++);
								tAddr = dAddr + offs;
								FormatAddress(valStr, tAddr, true);
							}
							break;
						case 'j':    // #H
							tAddr = m_bus->Read8(dAddr++) + (m_bus->Read8(dAddr++)<<8);
							opFlags = GetOpFlags(addr, opcode);
							FormatJumpAddress(valStr, tAddr, opFlags);
							break;
						case 'a':    // #h
							tAddr = m_bus->Read8(dAddr++) + (m_bus->Read8(dAddr++)<<8);
							FormatAddress(valStr, tAddr, true);
							break;
					}
					// Append formatted data and loop again to process any more @ parameters
					strcat(opPos, valStr);
					templ = pos + 2;
					opPos += strlen(opPos);
				}
				else
				{
					// No more @ parameters found so append remainder of template operand and exit loop
					strcat(opPos, templ);
					break;
				}
			}

			// Finally substitute ? parameter (X or Y)
			if (pos = strchr(operands, '?'))
				*pos = xyChr;
		}
		else
		{
			// Instruction has no operands
			strcpy(mnemonic, templ);
			operands[0] = '\0';
		}

		// Return instruction size
		return dAddr - addr;
	}

	//int CZ80Debug::GetOpLength(UINT32 addr)
	//{
	//	// Get instruction template from opcode
	//	UINT32 dAddr = addr;
	//	UINT8 opcode = m_bus->Read8(dAddr++);
	//	const char *templ;
	//	switch (opcode)
	//	{
	//		case 0xCB: 
	//			templ = templates[1][m_bus->Read8(dAddr++)];
	//			break;
	//		case 0xED:
	//			templ = templates[2][m_bus->Read8(dAddr++)];
	//			break;
	//		case 0xDD: 
	//			opcode = m_bus->Read8(dAddr++);
	//			if (opcode == 0xCB)
	//			{
	//				dAddr++;
	//				templ = templates[4][m_bus->Read8(dAddr++)]; 
	//			}
	//			else
	//				templ = templates[3][opcode];
	//			break;
	//		case 0xFD: 
	//			opcode = m_bus->Read8(dAddr++);
	//			if (opcode == 0xCB)
	//			{
	//				dAddr++;
	//				templ = templates[4][m_bus->Read8(dAddr++)]; 
	//			}
	//			else
	//				templ = templates[3][opcode];
	//			break;
	//		default:
	//			templ = templates[0][opcode];
	//			break;
	//	}
	//
	//	// If no template found, then instruction is invalid
	//	if (templ == NULL)
	//		return -1;
	//	
	//	// Return instruction size
	//	return dAddr - addr;
	//}

	EOpFlags CZ80Debug::GetOpFlags(UINT32 addr, UINT32 opcode)
	{
		if (opcode == 0xC3)
			return JumpSimple; // JP
		else if (opcode == 0xC2 || opcode == 0xCA || opcode == 0xD2 || opcode == 0xDA ||
				 opcode == 0xE2 || opcode == 0xEA || opcode == 0xF2 || opcode == 0xFA)
			return (EOpFlags)(JumpSimple | Conditional); // JP NZ, JP Z, JP NC, JP C, JP PO, JP PE, JP P, JP M
		else if (opcode == 0x18)
			return (EOpFlags)(JumpSimple | Relative); // JR 
		else if (opcode == 0x20 || opcode == 0x28 || opcode == 0x30 || opcode == 0x38)
			return (EOpFlags)(JumpSimple | Relative | Conditional); // JR NZ, JR Z, JR NC, JR C
		else if (opcode == 0x10)
			return (EOpFlags)(JumpLoop | Relative | Conditional); // DJNZ
		else if (opcode == 0xCD)
			return JumpSub; // CALL
		else if (opcode == 0xC4 || opcode == 0xCC || opcode == 0xD4 || opcode == 0xDC || 
				 opcode == 0xE4 || opcode == 0xEC || opcode == 0xF4 || opcode == 0xFC)
			return (EOpFlags)(JumpSub | Conditional); // CALL NZ, CALL Z, CALL NC, CALL C, CALL PO, CALL PE, CALL P, CALL M
		else if (opcode == 0xC9)
			return ReturnSub; // RET
		else if (opcode == 0xC0 || opcode == 0xC8 || opcode == 0xD0 || opcode == 0xD8 ||
				 opcode == 0xE0 || opcode == 0xE8 || opcode == 0xF0 || opcode == 0xF8)
			return (EOpFlags)(ReturnSub | Conditional); // RET NZ, RET Z, RET NC, RET C, RET PO, RET PE, RET P, RET M
		else if (opcode == 0xC7 || opcode == 0xCF || opcode == 0xD7 || opcode == 0xD7 || 
				 opcode == 0xE7 || opcode == 0xEF || opcode == 0xF7 || opcode == 0xFF)
			return (EOpFlags)(JumpEx | NotFixed); // RST 0, RST 8, RST 10H, RST 18H, RST 20H, RST 28H, RST 30H, RST 38H
		else if (opcode == 0xED)
		{
			UINT8 nextCode = m_bus->Read8(addr + 1);
			if (nextCode == 0x4D || nextCode == 0x45)
				return ReturnEx; // RETI, RETN
		}
		else if (opcode == 0xE9 ||
			opcode == 0xDD && m_bus->Read8(addr + 1) == 0xE9 ||
			opcode == 0xFD && m_bus->Read8(addr + 1) == 0xE9)
			return (EOpFlags)(JumpSimple | NotFixed); // JR (HL), JR (IX) or JR (IY)
		return NormalOp;
	}

	bool CZ80Debug::GetJumpAddr(UINT32 addr, UINT32 opcode, UINT32 &jumpAddr)
	{
		if (opcode == 0xC3 || 
			opcode == 0xC2 || opcode == 0xCA || opcode == 0xD2 || opcode == 0xDA ||
			opcode == 0xE2 || opcode == 0xEA || opcode == 0xF2 || opcode == 0xFA ||
			opcode == 0xCD || 
			opcode == 0xC4 || opcode == 0xCC || opcode == 0xD4 || opcode == 0xDC || 
			opcode == 0xE4 || opcode == 0xEC || opcode == 0xF4 || opcode == 0xFC)
		{
			// JP, JP NZ, JP Z, JP NC, JP C, JP PO, JP PE, JP P, JP M,
			// CALL, CALL NZ, CALL Z, CALL NC, CALL C, CALL PO, CALL PE, CALL P, CALL M
			jumpAddr = (UINT32)(m_bus->Read8(addr + 1) + (m_bus->Read8(addr + 2)<<8));
			return true;
		}
		else if (opcode == 0x18 || opcode == 0x20 || opcode == 0x28 || opcode == 0x30 || opcode == 0x38 || opcode == 0x10)
		{
			// JR, JR NZ, JR NC, JR C, DJNZ
			INT8 offs = m_bus->Read8(addr + 1);
			jumpAddr = addr + 2 + offs;
			return true;
		}
		return false;
	}
		
	bool CZ80Debug::GetJumpRetAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr)
	{
		if (opcode == 0xCD || 
			opcode == 0xC4 || opcode == 0xCC || opcode == 0xD4 || opcode == 0xDC || 
			opcode == 0xE4 || opcode == 0xEC || opcode == 0xF4 || opcode == 0xFC)
		{
			// CALL, CALL NZ, CALL Z, CALL NC, CALL C, CALL PO, CALL PE, CALL P, CALL M
			retAddr = addr + 3;
			return true;
		}
		else if (opcode == 0xC7 || opcode == 0xCF || opcode == 0xD7 || opcode == 0xD7 || 
				 opcode == 0xE7 || opcode == 0xEF || opcode == 0xF7 || opcode == 0xFF)
		{
			// RST 0, RST 8, RST 10H, RST 18H, RST 20H, RST 28H, RST 30H, RST 38H
			retAddr = addr + 1;
		}
		return false;
	}

	bool CZ80Debug::GetReturnAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr)
	{
		if (opcode == 0xC9 ||
			opcode == 0xC0 || opcode == 0xC8 || opcode == 0xD0 || opcode == 0xD8 ||
			opcode == 0xE0 || opcode == 0xE8 || opcode == 0xF0 || opcode == 0xF8)
		{
			// RET, RET NZ, RET Z, RET NC, RET C, RET PO, RET PE, RET P, RET M
			UINT16 sp = m_z80->GetReg16(Z80_REG16_SP);
			retAddr = (UINT32)m_bus->Read8(sp) + (((UINT32)m_bus->Read8(sp + 1))<<8);
			return true;
		}
		else if (opcode == 0xED)
		{
			UINT8 nextCode = m_bus->Read8(addr + 1);
			if (nextCode == 0x4D || nextCode == 0x45)
			{
				// RETI, RETN
				UINT16 sp = m_z80->GetReg16(Z80_REG16_SP);
				retAddr = (UINT32)m_bus->Read8(sp) + (((UINT32)m_bus->Read8(sp + 1))<<8);
				return true;
			}
		}
		return false;
	}

	bool CZ80Debug::GetHandlerAddr(CException *ex, UINT32 &handlerAddr)
	{
		switch (ex->code)
		{
			case Z80_EX_NMI:	 handlerAddr = 0x0066; return true; // Hardwired NMI vector
			case Z80_INT_RST_00: handlerAddr = 0x0000; return true; // RST 0h
			case Z80_INT_RST_08: handlerAddr = 0x0008; return true; // RST 8h
			case Z80_INT_RST_10: handlerAddr = 0x0010; return true; // RST 10h
			case Z80_INT_RST_18: handlerAddr = 0x0018; return true; // RST 18h
			case Z80_INT_RST_20: handlerAddr = 0x0020; return true; // RST 20h
			case Z80_INT_RST_28: handlerAddr = 0x0028; return true; // RST 28h
			case Z80_INT_RST_30: handlerAddr = 0x0030; return true; // RST 30h
			case Z80_INT_RST_38: handlerAddr = 0x0038; return true; // RST 38h
			case Z80_IM1_IRQ:    handlerAddr = 0x0038; return true; // Hardwired IRQ vector
			default:             return false;
		}
	}

	bool CZ80Debug::GetHandlerAddr(CInterrupt *in, UINT32 &handlerAddr)
	{
		UINT8 iff = m_z80->GetReg8(Z80_REG8_IFF);
		if (!(iff&0x04)) // IM2 mode
			return false;
		UINT8 i = m_z80->GetReg8(Z80_REG8_I);
		handlerAddr = (((UINT32)i)<<8)|(((UINT32)in->code)&0xFF);
		return true;
	}

	// CBus methods

	UINT8 CZ80Debug::Read8(UINT32 addr)
	{
		UINT8 data = m_bus->Read8(addr);
		CheckRead8(addr, data);
		return data;
	}

	void CZ80Debug::Write8(UINT32 addr, UINT8 data)
	{
		m_bus->Write8(addr, data);
		CheckWrite8(addr, data);
	}

	UINT8 CZ80Debug::IORead8(UINT32 portNum)
	{
		UINT8 data = m_bus->IORead8(portNum);
		CheckPortInput(portNum, data);
		return data;
	}

	void CZ80Debug::IOWrite8(UINT32 portNum, UINT8 data)
	{
		m_bus->IOWrite8(portNum, data);
		CheckPortOutput(portNum, data);
	}
}

#endif  // SUPERMODEL_DEBUGGER