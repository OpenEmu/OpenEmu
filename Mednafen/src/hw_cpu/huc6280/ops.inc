/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

case 0x00:  /* BRK */
            PC++;
	    P &= ~T_FLAG;
            PUSH(PC >> 8);
            PUSH(PC);
            PUSH(P | B_FLAG);

	    P |= I_FLAG;
	    REDOPIMCACHE();

	    P &= ~D_FLAG;

            PC = RdOp(0xFFF6);
            PC |= RdOp(0xFFF7) << 8;

	    if(ADDBT)
	     ADDBT(PC);

            ADDCYC(7);
            LASTCYCLE;
            break;

case 0x40:  /* RTI */
            P = POP();
	    REDOPIMCACHE();
            PC = POP();
            PC |= POP() << 8;

	    if(ADDBT)
	     ADDBT(PC);

            ADDCYC(6);
            LASTCYCLE;

	    goto skip_T_flag_clear;

            break;
            
case 0x60:  /* RTS */
            PC = POP();
            PC |= POP() << 8;
	    PC++;

	    if(ADDBT)
	     ADDBT(PC);

            ADDCYC(6);
            LASTCYCLE;
            break;

case 0x48: /* PHA */
           ADDCYC(2);
           LASTCYCLE;
           PUSH(A);
           break;

case 0x08: /* PHP */
           ADDCYC(2);
           LASTCYCLE;
	   P &= ~T_FLAG;
           PUSH(P | B_FLAG);
           break;

case 0xDA: // PHX	65C02
           ADDCYC(2);
           LASTCYCLE;
           PUSH(X);
	   break;

case 0x5A: // PHY	65C02
           ADDCYC(2);
           LASTCYCLE;
	   PUSH(Y);
	   break;

case 0x68: /* PLA */
           ADDCYC(3);
           LASTCYCLE;
           A = POP();
           X_ZN(A);
           break;

case 0xFA: // PLX	65C02
           ADDCYC(3);
           LASTCYCLE;
	   X = POP();
	   X_ZN(X);
	   break;

case 0x7A: // PLY	65C02
           ADDCYC(3);
           LASTCYCLE;
	   Y = POP();
	   X_ZN(Y);
	   break;

case 0x28: /* PLP */
	   ADDCYC(3);
	   LASTCYCLE;
           P = POP();
	   REDOPIMCACHE();

	   goto skip_T_flag_clear;

           break;

case 0x4C: /* JMP ABSOLUTE */
	  {
	   uint16 ptmp = PC;
	   unsigned int npc;

	   npc = RdOp(ptmp);
	   ptmp++;
	   npc |= RdOp(ptmp)<<8;
	   PC = npc;

	   if(ADDBT)
	    ADDBT(PC);

           ADDCYC(3);
           LASTCYCLE;
	  }
	  break;

case 0x6C: /* JMP Indirect */
	   {
	    uint32 tmp;
	    GetAB(tmp);
	    PC = RdMem(tmp);
	    PC |= RdMem(tmp + 1) << 8;

	    if(ADDBT)
	     ADDBT(PC);

            ADDCYC(6);
            LASTCYCLE;
	   }
	   break;

case 0x7C: // JMP Indirect X - 65C02
           {
            uint32 tmp;
            GetAB(tmp);
	    tmp += X;

            PC = RdMem(tmp);
            PC |= RdMem(tmp + 1) << 8;

	    if(ADDBT)
	     ADDBT(PC);

            ADDCYC(6);
            LASTCYCLE;
           }
           break;

case 0x20: /* JSR */
	   {
	    uint8 npc;
	    npc=RdOp(PC);
	    PC++;
            PUSH(PC >> 8);
            PUSH(PC);
            PC = RdOp(PC) << 8;
	    PC |= npc;

	    if(ADDBT)
	     ADDBT(PC);

	    ADDCYC(6);
	    LASTCYCLE;
	   }
           break;

case 0xAA: IMP(TAX);

case 0x8A: IMP(TXA);

case 0xA8: IMP(TAY);

case 0x98: IMP(TYA);

case 0xBA: IMP(TSX);

case 0x9A: IMP(TXS);

case 0xCA: IMP(DEX);

case 0x88: IMP(DEY);

case 0xE8: IMP(INX);

case 0xC8: IMP(INY);

case 0x54: IMP(CSL);
case 0xD4: IMP(CSH);

#define OP_CLEARR(r)	{ ADDCYC(1); LASTCYCLE; r = 0; break; }

case 0x62: OP_CLEARR(A); // CLA
case 0x82: OP_CLEARR(X); // CLX
case 0xC2: OP_CLEARR(Y); // CLY

// The optional argument(s) will run at the end, immediately before the break.
#define OP_CLEARF(f, ...)	{ ADDCYC(1); LASTCYCLE; P &= ~f; __VA_ARGS__ break; }
#define OP_SETF(f, ...)	{ ADDCYC(1); LASTCYCLE; P |= f; __VA_ARGS__ break; }

case 0x18: /* CLC */
	   OP_CLEARF(C_FLAG);

case 0xD8: /* CLD */
	   OP_CLEARF(D_FLAG);

case 0x58: /* CLI */
	   OP_CLEARF(I_FLAG, REDOPIMCACHE(););

case 0xB8: /* CLV */
	   OP_CLEARF(V_FLAG);

case 0x38: /* SEC */
	   OP_SETF(C_FLAG);

case 0xF8: /* SED */
	   OP_SETF(D_FLAG);

case 0x78: /* SEI */
	   OP_SETF(I_FLAG, REDOPIMCACHE(););

case 0xF4: /* SET */
	   //puts("SET");
	   ADDCYC(1);
	   LASTCYCLE;
	   P |= T_FLAG;

	   goto skip_T_flag_clear;

	   break;


case 0xEA: /* NOP */
	   ADDCYC(1);
	   LASTCYCLE;
           break;

case 0x0A: RMW_A(ASL);
case 0x06: RMW_ZP(ASL);
case 0x16: RMW_ZPX(ASL);
case 0x0E: RMW_AB(ASL);
case 0x1E: RMW_ABX(ASL);

case 0x3A: RMW_A(DEC);
case 0xC6: RMW_ZP(DEC);
case 0xD6: RMW_ZPX(DEC);
case 0xCE: RMW_AB(DEC);
case 0xDE: RMW_ABX(DEC);

case 0x1A: RMW_A(INC);		// 65C02
case 0xE6: RMW_ZP(INC);
case 0xF6: RMW_ZPX(INC);
case 0xEE: RMW_AB(INC);
case 0xFE: RMW_ABX(INC);

case 0x4A: RMW_A(LSR);
case 0x46: RMW_ZP(LSR);
case 0x56: RMW_ZPX(LSR);
case 0x4E: RMW_AB(LSR);
case 0x5E: RMW_ABX(LSR);

case 0x2A: RMW_A(ROL);
case 0x26: RMW_ZP(ROL);
case 0x36: RMW_ZPX(ROL);
case 0x2E: RMW_AB(ROL);
case 0x3E: RMW_ABX(ROL);

case 0x6A: RMW_A(ROR);
case 0x66: RMW_ZP(ROR);
case 0x76: RMW_ZPX(ROR);
case 0x6E: RMW_AB(ROR);
case 0x7E: RMW_ABX(ROR);

case 0x69: LD_IM(ADC);
case 0x65: LD_ZP(ADC);
case 0x75: LD_ZPX(ADC);
case 0x6D: LD_AB(ADC);
case 0x7D: LD_ABX(ADC);
case 0x79: LD_ABY(ADC);
case 0x72: LD_IND(ADC);
case 0x61: LD_IX(ADC);
case 0x71: LD_IY(ADC);

case 0x29: LD_IM(AND);
case 0x25: LD_ZP(AND);
case 0x35: LD_ZPX(AND);
case 0x2D: LD_AB(AND);
case 0x3D: LD_ABX(AND);
case 0x39: LD_ABY(AND);
case 0x32: LD_IND(AND);
case 0x21: LD_IX(AND);
case 0x31: LD_IY(AND);

case 0x89: LD_IM(BIT);
case 0x24: LD_ZP(BIT);
case 0x34: LD_ZPX(BIT);
case 0x2C: LD_AB(BIT);
case 0x3C: LD_ABX(BIT);

case 0xC9: LD_IM(CMP);
case 0xC5: LD_ZP(CMP);
case 0xD5: LD_ZPX(CMP);
case 0xCD: LD_AB(CMP);
case 0xDD: LD_ABX(CMP);
case 0xD9: LD_ABY(CMP);
case 0xD2: LD_IND(CMP);
case 0xC1: LD_IX(CMP);
case 0xD1: LD_IY(CMP);

case 0xE0: LD_IM(CPX);
case 0xE4: LD_ZP(CPX);
case 0xEC: LD_AB(CPX);

case 0xC0: LD_IM(CPY);
case 0xC4: LD_ZP(CPY);
case 0xCC: LD_AB(CPY);

case 0x49: LD_IM(EOR);
case 0x45: LD_ZP(EOR);
case 0x55: LD_ZPX(EOR);
case 0x4D: LD_AB(EOR);
case 0x5D: LD_ABX(EOR);
case 0x59: LD_ABY(EOR);
case 0x52: LD_IND(EOR);
case 0x41: LD_IX(EOR);
case 0x51: LD_IY(EOR);

case 0xA9: LD_IM(LDA);
case 0xA5: LD_ZP(LDA);
case 0xB5: LD_ZPX(LDA);
case 0xAD: LD_AB(LDA);
case 0xBD: LD_ABX(LDA);
case 0xB9: LD_ABY(LDA);
case 0xB2: LD_IND(LDA);
case 0xA1: LD_IX(LDA);
case 0xB1: LD_IY(LDA);

case 0xA2: LD_IM(LDX);
case 0xA6: LD_ZP(LDX);
case 0xB6: LD_ZPY(LDX);
case 0xAE: LD_AB(LDX);
case 0xBE: LD_ABY(LDX);

case 0xA0: LD_IM(LDY);
case 0xA4: LD_ZP(LDY);
case 0xB4: LD_ZPX(LDY);
case 0xAC: LD_AB(LDY);
case 0xBC: LD_ABX(LDY);

case 0x09: LD_IM(ORA);
case 0x05: LD_ZP(ORA);
case 0x15: LD_ZPX(ORA);
case 0x0D: LD_AB(ORA);
case 0x1D: LD_ABX(ORA);
case 0x19: LD_ABY(ORA);
case 0x12: LD_IND(ORA);
case 0x01: LD_IX(ORA);
case 0x11: LD_IY(ORA);

case 0xE9: LD_IM(SBC);
case 0xE5: LD_ZP(SBC);
case 0xF5: LD_ZPX(SBC);
case 0xED: LD_AB(SBC);
case 0xFD: LD_ABX(SBC);
case 0xF9: LD_ABY(SBC);
case 0xF2: LD_IND(SBC);
case 0xE1: LD_IX(SBC);
case 0xF1: LD_IY(SBC);

case 0x85: ST_ZP(A);
case 0x95: ST_ZPX(A);
case 0x8D: ST_AB(A);
case 0x9D: ST_ABX(A);
case 0x99: ST_ABY(A);
case 0x92: ST_IND(A);
case 0x81: ST_IX(A);
case 0x91: ST_IY(A);

case 0x86: ST_ZP(X);
case 0x96: ST_ZPY(X);
case 0x8E: ST_AB(X);

case 0x84: ST_ZP(Y);
case 0x94: ST_ZPX(Y);
case 0x8C: ST_AB(Y);

/* BBRi */
case 0x0F: LD_ZP(BBRi(x, 0));
case 0x1F: LD_ZP(BBRi(x, 1));
case 0x2F: LD_ZP(BBRi(x, 2));
case 0x3F: LD_ZP(BBRi(x, 3));
case 0x4F: LD_ZP(BBRi(x, 4));
case 0x5F: LD_ZP(BBRi(x, 5));
case 0x6F: LD_ZP(BBRi(x, 6));
case 0x7F: LD_ZP(BBRi(x, 7));

/* BBSi */
case 0x8F: LD_ZP(BBSi(x, 0));
case 0x9F: LD_ZP(BBSi(x, 1));
case 0xAF: LD_ZP(BBSi(x, 2));
case 0xBF: LD_ZP(BBSi(x, 3));
case 0xCF: LD_ZP(BBSi(x, 4));
case 0xDF: LD_ZP(BBSi(x, 5));
case 0xEF: LD_ZP(BBSi(x, 6));
case 0xFF: LD_ZP(BBSi(x, 7));

/* BRA */
case 0x80: JR(1); break;

/* BSR */
case 0x44:
           {
            PUSH(PC >> 8);
            PUSH(PC);
	    ADDCYC(4);
	    JR(1);
           } 
	   break;

/* BCC */
case 0x90: JR(!(P&C_FLAG)); break;

/* BCS */
case 0xB0: JR(P&C_FLAG); break;

/* BEQ */
case 0xF0: JR(P&Z_FLAG); break;

/* BNE */
case 0xD0: JR(!(P&Z_FLAG)); break;

/* BMI */
case 0x30: JR(P&N_FLAG); break;

/* BPL */
case 0x10: JR(!(P&N_FLAG)); break;

/* BVC */
case 0x50: JR(!(P&V_FLAG)); break;

/* BVS */
case 0x70: JR(P&V_FLAG); break;


// RMB				65SC02
case 0x07: RMW_ZP_B(RMB(0));
case 0x17: RMW_ZP_B(RMB(1));
case 0x27: RMW_ZP_B(RMB(2));
case 0x37: RMW_ZP_B(RMB(3));
case 0x47: RMW_ZP_B(RMB(4));
case 0x57: RMW_ZP_B(RMB(5));
case 0x67: RMW_ZP_B(RMB(6));
case 0x77: RMW_ZP_B(RMB(7));

// SMB				65SC02
case 0x87: RMW_ZP_B(SMB(0));
case 0x97: RMW_ZP_B(SMB(1));
case 0xa7: RMW_ZP_B(SMB(2));
case 0xb7: RMW_ZP_B(SMB(3));
case 0xc7: RMW_ZP_B(SMB(4));
case 0xd7: RMW_ZP_B(SMB(5));
case 0xe7: RMW_ZP_B(SMB(6));
case 0xf7: RMW_ZP_B(SMB(7));

// STZ				65C02
case 0x64: ST_ZP(0);
case 0x74: ST_ZPX(0);
case 0x9C: ST_AB(0);
case 0x9E: ST_ABX(0);

// TRB				65SC02
case 0x14: RMW_ZP(TRB);
case 0x1C: RMW_AB(TRB);

// TSB				65SC02
case 0x04: RMW_ZP(TSB);
case 0x0C: RMW_AB(TSB);

// TST
case 0x83: LD_IM_ZP(TST);
case 0xA3: LD_IM_ZPX(TST);
case 0x93: LD_IM_AB(TST);
case 0xB3: LD_IM_ABX(TST);

case 0x02: IMP(SXY);
case 0x22: IMP(SAX);
case 0x42: IMP(SAY);



case 0x73: // TII
		LD_BMT(BMT_TII);

case 0xC3: // TDD
		LD_BMT(BMT_TDD);

case 0xD3: // TIN
		LD_BMT(BMT_TIN);

case 0xE3: // TIA
		LD_BMT(BMT_TIA);

case 0xF3: // TAI
		LD_BMT(BMT_TAI);

case 0x43: // TMAi
		LD_IM_COMPLEX(TMA);

case 0x53: // TAMi
		LD_IM_COMPLEX(TAM);

case 0x03:	// ST0
		LD_IM_COMPLEX(ST0);

case 0x13:	// ST1
		LD_IM_COMPLEX(ST1);

case 0x23:	// ST2
		LD_IM_COMPLEX(ST2);


case 0xCB:
	if(EmulateWAI)
	{
	 if(next_event > 1)
	  ADDCYC(next_event - 1);
	 LASTCYCLE;
	 break;
	}
default:  //MDFN_printf("Bad %02x at $%04x\n", lastop, PC);
          ADDCYC(1);
          LASTCYCLE;
          break;

