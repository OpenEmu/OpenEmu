/* Mednafen - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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
            _PC++;
	    _P &= ~T_FLAG;
            PUSH(_PC>>8);
            PUSH(_PC);
            PUSH(_P|B_FLAG);
	    _P|=I_FLAG;
	    _P &= ~D_FLAG;
	    _PI|=I_FLAG;
            _PC=RdOp(0xFFF6);
            _PC|=RdOp(0xFFF7)<<8;
	    ADDBT(_PC);
            break;

case 0x40:  /* RTI */
            _P = POP();
	    /* _PI=_P; This is probably incorrect, so it's commented out. */
	    _PI = _P;
            _PC = POP();
            _PC |= POP() << 8;
	    ADDBT(_PC);
	    if(!HuCPU.preexec) goto skip_T_flag_clear;
            break;
            
case 0x60:  /* RTS */
            _PC = POP();
            _PC |= POP() << 8;
            _PC++;
	    ADDBT(_PC);
            break;

case 0x48: /* PHA */
           PUSH(_A);
           break;

case 0x08: /* PHP */
	   _P &= ~T_FLAG;
           PUSH(_P|B_FLAG);
           break;

case 0xDA: // PHX	65C02
           PUSH(_X);
	   break;

case 0x5A: // PHY	65C02
	   PUSH(_Y);
	   break;

case 0x68: /* PLA */
           _A = POP();
           X_ZN(_A);
           break;

case 0xFA: // PLX	65C02
	   _X = POP();
	   X_ZN(_X);
	   break;

case 0x7A: // PLY	65C02
	   _Y = POP();
	   X_ZN(_Y);
	   break;

case 0x28: /* PLP */
           _P = POP();
	   if(!HuCPU.preexec) goto skip_T_flag_clear;
           break;

case 0x4C:
	  {
	   uint16 ptmp=_PC;
	   unsigned int npc;

	   npc=RdOp(ptmp);
	   ptmp++;
	   TBOL(ptmp);
	   npc|=RdOp(ptmp)<<8;
	   _PC=npc;
	   ADDBT(_PC);
	  }
	  break; /* JMP ABSOLUTE */

case 0x6C: /* JMP Indirect */
	   {
	    uint32 tmp;
	    GetAB(tmp);
	    _PC=RdMem(tmp);
	    _PC|=RdMem(tmp + 1)<<8;
	    ADDBT(_PC);
	   }
	   break;

case 0x7C: // JMP Indirect X - 65C02
           {
            uint32 tmp;
            GetAB(tmp);
	    tmp += _X;
            _PC=RdMem(tmp);
            _PC|=RdMem(tmp + 1)<<8;
	    ADDBT(_PC);
           }
           break;
case 0x20: /* JSR */
	   {
	    uint8 npc;
	    npc=RdOp(_PC);
	    _PC++;
	    TBOL(_PC);
            PUSH(_PC>>8);
            PUSH(_PC);
            _PC=RdOp(_PC)<<8;
	    _PC|=npc;
	    ADDBT(_PC);
	   }
           break;

case 0xAA: /* TAX */
           _X=_A;
           X_ZN(_A);
           break;

case 0x8A: /* TXA */
           _A=_X;
           X_ZN(_A);
           break;

case 0xA8: /* TAY */
           _Y=_A;
           X_ZN(_A);
           break;
case 0x98: /* TYA */
           _A=_Y;
           X_ZN(_A);
           break;

case 0xBA: /* TSX */
           _X=_S;
           X_ZN(_X);
           break;
case 0x9A: /* TXS */
           _S=_X;
           break;

case 0xCA: /* DEX */
           _X--;
           X_ZN(_X);
           break;
case 0x88: /* DEY */
           _Y--;
           X_ZN(_Y);
           break;

case 0xE8: /* INX */
           _X++;
           X_ZN(_X);
           break;
case 0xC8: /* INY */
           _Y++;
           X_ZN(_Y);
           break;

case 0x54: CSL; break;
case 0xD4: CSH; break;

case 0x62: _A = 0; break; // CLA
case 0x82: _X = 0; break; // CLX
case 0xC2: _Y = 0; break; // CLY

case 0x18: /* CLC */
           _P&=~C_FLAG;
           break;

case 0xD8: /* CLD */
           _P&=~D_FLAG;
           break;
case 0x58: /* CLI */
           _P&=~I_FLAG;
           break;
case 0xB8: /* CLV */
           _P&=~V_FLAG;
           break;

case 0x38: /* SEC */
           _P|=C_FLAG;
           break;

case 0xF8: /* SED */
           _P|=D_FLAG;
           break;

case 0x78: /* SEI */
           _P|=I_FLAG;
           break;

case 0xF4: /* SET */
	   //puts("SET");
	   _P |= T_FLAG;
	   if(!HuCPU.preexec) goto skip_T_flag_clear;
	   break;

case 0xEA: /* NOP */
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

case 0x85: ST_ZP(_A);
case 0x95: ST_ZPX(_A);
case 0x8D: ST_AB(_A);
case 0x9D: ST_ABX(_A);
case 0x99: ST_ABY(_A);
case 0x92: ST_IND(_A);
case 0x81: ST_IX(_A);
case 0x91: ST_IY(_A);

case 0x86: ST_ZP(_X);
case 0x96: ST_ZPY(_X);
case 0x8E: ST_AB(_X);

case 0x84: ST_ZP(_Y);
case 0x94: ST_ZPX(_Y);
case 0x8C: ST_AB(_Y);

/* BBRi */
case 0x0F: LD_ZP(BBRi(0));
case 0x1F: LD_ZP(BBRi(1));
case 0x2F: LD_ZP(BBRi(2));
case 0x3F: LD_ZP(BBRi(3));
case 0x4F: LD_ZP(BBRi(4));
case 0x5F: LD_ZP(BBRi(5));
case 0x6F: LD_ZP(BBRi(6));
case 0x7F: LD_ZP(BBRi(7));

/* BBSi */
case 0x8F: LD_ZP(BBSi(0));
case 0x9F: LD_ZP(BBSi(1));
case 0xAF: LD_ZP(BBSi(2));
case 0xBF: LD_ZP(BBSi(3));
case 0xCF: LD_ZP(BBSi(4));
case 0xDF: LD_ZP(BBSi(5));
case 0xEF: LD_ZP(BBSi(6));
case 0xFF: LD_ZP(BBSi(7));

/* BRA */
case 0x80: JR(1); break;

/* BSR */
case 0x44:
           {
            PUSH(_PC>>8);
            PUSH(_PC);
	    JR(1);
           } 
	   break;

/* BCC */
case 0x90: JR(!(_P&C_FLAG)); break;

/* BCS */
case 0xB0: JR(_P&C_FLAG); break;

/* BEQ */
case 0xF0: JR(_P&Z_FLAG); break;

/* BNE */
case 0xD0: JR(!(_P&Z_FLAG)); break;

/* BMI */
case 0x30: JR(_P&N_FLAG); break;

/* BPL */
case 0x10: JR(!(_P&N_FLAG)); break;

/* BVC */
case 0x50: JR(!(_P&V_FLAG)); break;

/* BVS */
case 0x70: JR(_P&V_FLAG); break;


// RMB				65SC02
case 0x07: RMW_ZP(RMB(0));
case 0x17: RMW_ZP(RMB(1));
case 0x27: RMW_ZP(RMB(2));
case 0x37: RMW_ZP(RMB(3));
case 0x47: RMW_ZP(RMB(4));
case 0x57: RMW_ZP(RMB(5));
case 0x67: RMW_ZP(RMB(6));
case 0x77: RMW_ZP(RMB(7));

// SMB				65SC02
case 0x87: RMW_ZP(SMB(0));
case 0x97: RMW_ZP(SMB(1));
case 0xa7: RMW_ZP(SMB(2));
case 0xb7: RMW_ZP(SMB(3));
case 0xc7: RMW_ZP(SMB(4));
case 0xd7: RMW_ZP(SMB(5));
case 0xe7: RMW_ZP(SMB(6));
case 0xf7: RMW_ZP(SMB(7));

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
case 0x83: { uint8 zoomhack=RdOp(_PC); _PC++; LD_ZP(TST); }
case 0xA3: { uint8 zoomhack=RdOp(_PC); _PC++; LD_ZPX(TST); }
case 0x93: { uint8 zoomhack=RdOp(_PC); _PC++; LD_AB(TST); }
case 0xB3: { uint8 zoomhack=RdOp(_PC); _PC++; LD_ABX(TST); }

case 0x22: // SAX(amaphone!)
	{
	 uint8 tmp = _X;
	 _X = _A;
	 _A = tmp;
	}
	break;

case 0x42: // SAY(what?)
	{
	 uint8 tmp = _Y;
	 _Y = _A;
	 _A = tmp;
	}
	break;

case 0x02:	// SXY
	{
	 uint8 tmp = _X;
	 _X = _Y;
	 _Y = tmp;
	}
	break;

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
		LD_IM(TMA);

case 0x53: // TAMi
		LD_IM(TAM);

case 0x03:	// ST0
		LD_IM(ST0);

case 0x13:	// ST1
		LD_IM(ST1);

case 0x23:	// ST2
		LD_IM(ST2);

//default: MDFN_printf("Bad %02x at $%04x\n",b1,HuCPU.PC);break;
