/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef INSIDE_CRABZ80_EXECUTE
#error This file can only be compiled inside of CrabZ80.c. Do not try to include
#error this file in other files.
#endif

cycles_done += 4;
++cpu->ir.b.l;
FETCH_ARG8(inst);

switch(inst)    {
    case 0x00:  /* NOP */
    case 0x40:  /* LD B, B */
    case 0x49:  /* LD C, C */
    case 0x52:  /* LD D, D */
    case 0x5B:  /* LD E, E */
    case 0x64:  /* LD IxH, IxH */
    case 0x6D:  /* LD IxL, IxL */
    case 0x7F:  /* LD A, A */
        cycles_done += 4;
        goto out;

    case 0x01:  /* LD BC, nn */
    case 0x11:  /* LD DE, nn */
        goto LD16IMMOP;

    case 0x21:  /* LD Ix, nn */
        FETCH_ARG16(_value);
        cpu->offset->w = _value;
        cycles_done += 10;
        goto out;

    case 0x31:  /* LD SP, nn */
        goto LDSPIMMOP;

    case 0x02:  /* LD (BC), A */
    case 0x12:  /* LD (DE), A */
        goto LDATMOP;

    case 0x03:  /* INC BC */
        ++cpu->bc.w;
        cycles_done += 6;
        goto out;

    case 0x13:  /* INC DE */
        ++cpu->de.w;
        cycles_done += 6;
        goto out;

    case 0x23:  /* INC Ix */
        ++cpu->offset->w;
        cycles_done += 6;
        goto out;

    case 0x33:  /* INC SP */
        ++cpu->sp.w;
        cycles_done += 6;
        goto out;

    case 0x04:  /* INC B */
    case 0x0C:  /* INC C */
    case 0x14:  /* INC D */
    case 0x1C:  /* INC E */
    case 0x3C:  /* INC A */
        goto INCR8OP;

    case 0x24:  /* INC IxH */
    case 0x2C:  /* INC IxL */
        OP_INC8(cpu->offset->bytes[((inst >> 3) & 0x01) ^ 0x01]);
        cycles_done += 4;
        goto out;

    case 0x34:  /* INC (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(_disp + cpu->offset->w);
        OP_INC8(_value);
        cpu->mwrite(_disp + cpu->offset->w, _value);
        cycles_done += 19;
        goto out;

    case 0x05:  /* DEC B */
    case 0x0D:  /* DEC C */
    case 0x15:  /* DEC D */
    case 0x1D:  /* DEC E */
    case 0x3D:  /* DEC A */
        goto DECR8OP;

    case 0x25:  /* DEC IxH */
    case 0x2D:  /* DEC IxL */
        OP_DEC8(cpu->offset->bytes[((inst >> 3) & 0x01) ^ 0x01]);
        cycles_done += 4;
        goto out;

    case 0x35:  /* DEC (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(_disp + cpu->offset->w);
        OP_DEC8(_value);
        cpu->mwrite(_disp + cpu->offset->w, _value);
        cycles_done += 19;
        goto out;

    case 0x06:  /* LD B, n */
    case 0x0E:  /* LD C, n */
    case 0x16:  /* LD D, n */
    case 0x1E:  /* LD E, n */
    case 0x3E:  /* LD A, n */
        goto LD8IMMOP;

    case 0x26:  /* LD IxH, n */
    case 0x2E:  /* LD IxL, n */
        FETCH_ARG8(_value);
        cpu->offset->bytes[((inst >> 3) & 0x01) ^ 0x01] = _value;
        cycles_done += 7;
        goto out;

    case 0x36:  /* LD (Ix + d), n */
        FETCH_ARG8(_disp);
        FETCH_ARG8(_value);
        cpu->mwrite(_disp + cpu->offset->w, _value);
        cycles_done += 15;
        goto out;

    case 0x07:  /* RLCA */
        goto RLCAOP;

    case 0x08:  /* EX AF, AF' */
        goto EXAFAFPOP;

    case 0x09:  /* ADD Ix, BC */
    case 0x19:  /* ADD Ix, DE */
        _value = REG16(inst >> 4);

ADDIxOP:
        cpu->internal_reg = cpu->offset->b.h;
        OP_ADDIx();
        cycles_done += 11;
        goto out;

    case 0x29:  /* ADD Ix, Ix */
        _value = cpu->offset->w;
        goto ADDIxOP;

    case 0x39:  /* ADD Ix, SP */
        _value = cpu->sp.w;
        goto ADDIxOP;

    case 0x0A:  /* LD A, (BC) */
    case 0x1A:  /* LD A, (DE) */
        goto LDAFMEMOP;

    case 0x0B:  /* DEC BC */
        --cpu->bc.w;
        cycles_done += 6;
        goto out;
        
    case 0x1B:  /* DEC DE */
        --cpu->de.w;
        cycles_done += 6;
        goto out;
        
    case 0x2B:  /* DEC Ix */
        --cpu->offset->w;
        cycles_done += 6;
        goto out;
        
    case 0x3B:  /* DEC SP */
        --cpu->sp.w;
        cycles_done += 6;
        goto out;

    case 0x0F:  /* RRCA */
        goto RRCAOP;

    case 0x10:  /* DJNZ e */
        goto DJNZOP;

    case 0x18:  /* JR e */
        goto JROP;

    case 0x20:  /* JR NZ, e */
        goto JRNZOP;

    case 0x28:  /* JR Z, e */
        goto JRZOP;

    case 0x30:  /* JR NC, e */
        goto JRNCOP;

    case 0x38:  /* JR C, e */
        goto JRCOP;

    case 0x17:  /* RLA */
        goto RLAOP;

    case 0x1F:  /* RRA */
        goto RRAOP;

    case 0x22:  /* LD (nn), Ix */
        FETCH_ARG16(_value);
        cpu->mwrite16(_value, cpu->offset->w);
        cycles_done += 20;
        goto out;

    case 0x27:  /* DAA */
        goto DAAOP;

    case 0x2A:  /* LD Ix, (nn) */
        FETCH_ARG16(_value);
        cpu->offset->w = cpu->mread16(_value);
        cycles_done += 16;
        goto out;

    case 0x2F:  /* CPL */
        goto CPLOP;

    case 0x32:  /* LD (nn), A */
        goto LDATMABSOP;

    case 0x37:  /* SCF */
        goto SCFOP;

    case 0x3A:  /* LD A, (nn) */
        goto LDAFMABSOP;

    case 0x3F:  /* CCF */
        goto CCFOP;

    case 0x44:  /* LD B, IxH */
    case 0x45:  /* LD B, IxL */
    case 0x4C:  /* LD C, IxH */
    case 0x4D:  /* LD C, IxL */
    case 0x54:  /* LD D, IxH */
    case 0x55:  /* LD D, IxL */
    case 0x5C:  /* LD E, IxH */
    case 0x5D:  /* LD E, IxL */
    case 0x7C:  /* LD A, IxH */
    case 0x7D:  /* LD A, IxL */
        REG8(inst >> 3) = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        cycles_done += 4;
        goto out;

    case 0x65:  /* LD IxH, IxL */
        cpu->offset->b.h = cpu->offset->b.l;
        cycles_done += 4;
        goto out;

    case 0x6C:  /* LD IxL, IxH */
        cpu->offset->b.l = cpu->offset->b.h;
        cycles_done += 4;
        goto out;

    case 0x60:  /* LD IxH, B */
    case 0x61:  /* LD IxH, C */
    case 0x62:  /* LD IxH, D */
    case 0x63:  /* LD IxH, E */
    case 0x67:  /* LD IxH, A */
    case 0x68:  /* LD IxL, B */
    case 0x69:  /* LD IxL, C */
    case 0x6A:  /* LD IxL, D */
    case 0x6B:  /* LD IxL, E */
    case 0x6F:  /* LD IxL, A */
        cpu->offset->bytes[((inst >> 3) & 0x01) ^ 0x01] = REG8(inst);
        cycles_done += 4;
        goto out;

    case 0x41:  /* LD B, C */
    case 0x42:  /* LD B, D */
    case 0x43:  /* LD B, E */
    case 0x47:  /* LD B, A */
    case 0x48:  /* LD C, B */
    case 0x4A:  /* LD C, D */
    case 0x4B:  /* LD C, E */
    case 0x4F:  /* LD C, A */
    case 0x50:  /* LD D, B */
    case 0x51:  /* LD D, C */
    case 0x53:  /* LD D, E */
    case 0x57:  /* LD D, A */
    case 0x58:  /* LD E, B */
    case 0x59:  /* LD E, C */
    case 0x5A:  /* LD E, D */
    case 0x5F:  /* LD E, A */
    case 0x78:  /* LD A, B */
    case 0x79:  /* LD A, C */
    case 0x7A:  /* LD A, D */
    case 0x7B:  /* LD A, E */
        REG8(inst >> 3) = REG8(inst);
        cycles_done += 4;
        goto out;

    case 0x46:  /* LD B, (Ix + d) */
    case 0x4E:  /* LD C, (Ix + d) */
    case 0x56:  /* LD D, (Ix + d) */
    case 0x5E:  /* LD E, (Ix + d) */
    case 0x66:  /* LD H, (Ix + d) */
    case 0x6E:  /* LD L, (Ix + d) */
    case 0x7E:  /* LD A, (Ix + d) */
        FETCH_ARG8(_disp);
        REG8(inst >> 3) = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto out;

    case 0x70:  /* LD (Ix + d), B */
    case 0x71:  /* LD (Ix + d), C */
    case 0x72:  /* LD (Ix + d), D */
    case 0x73:  /* LD (Ix + d), E */
    case 0x74:  /* LD (Ix + d), H */
    case 0x75:  /* LD (Ix + d), L */
    case 0x77:  /* LD (Ix + d), A */
        FETCH_ARG8(_disp);
        cpu->mwrite(cpu->offset->w + _disp, REG8(inst));
        cycles_done += 15;
        goto out;

    case 0x76:  /* HALT */
        OP_HALT();
        cycles_done += 4;
        goto out;

    case 0x80:  /* ADD A, B */
    case 0x81:  /* ADD A, C */
    case 0x82:  /* ADD A, D */
    case 0x83:  /* ADD A, E */
    case 0x87:  /* ADD A, A */
        goto ADD8OP;

    case 0x86: /* ADD A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto ADDOP;

    case 0xC6:  /* ADD A, n */
        goto ADD8IMMOP;

    case 0x84:  /* ADD A, IxH */
    case 0x85:  /* ADD A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto ADDOP;

    case 0x88:  /* ADC A, B */
    case 0x89:  /* ADC A, C */
    case 0x8A:  /* ADC A, D */
    case 0x8B:  /* ADC A, E */
    case 0x8F:  /* ADC A, A */
        goto ADC8OP;

    case 0x8E:  /* ADC A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto ADCOP;

    case 0xCE:  /* ADC A, n */
        goto ADC8IMMOP;

    case 0x8C:  /* ADC A, IxH */
    case 0x8D:  /* ADC A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto ADCOP;

    case 0x90:  /* SUB A, B */
    case 0x91:  /* SUB A, C */
    case 0x92:  /* SUB A, D */
    case 0x93:  /* SUB A, E */
    case 0x97:  /* SUB A, A */
        goto SUB8OP;

    case 0x96:  /* SUB A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto SUBOP;
        
    case 0xD6:  /* SUB A, n */
        goto SUB8IMMOP;

    case 0x94:  /* SUB A, IxH */
    case 0x95:  /* SUB A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto SUBOP;

    case 0x98:  /* SBC A, B */
    case 0x99:  /* SBC A, C */
    case 0x9A:  /* SBC A, D */
    case 0x9B:  /* SBC A, E */
    case 0x9F:  /* SBC A, A */
        goto SBC8OP;

    case 0x9E:  /* SBC A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto SBCOP;

    case 0xDE:  /* SBC A, n */
        goto SBC8IMMOP;

    case 0x9C:  /* SBC A, IxH */
    case 0x9D:  /* SBC A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto SBCOP;

    case 0xA0:  /* AND A, B */
    case 0xA1:  /* AND A, C */
    case 0xA2:  /* AND A, D */
    case 0xA3:  /* AND A, E */
    case 0xA7:  /* AND A, A */
        goto AND8OP;

    case 0xA6:  /* AND A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto ANDOP;

    case 0xE6:  /* AND A, n */
        goto AND8IMMOP;

    case 0xA4:  /* AND A, IxH */
    case 0xA5:  /* AND A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto ANDOP;

    case 0xA8:  /* XOR A, B */
    case 0xA9:  /* XOR A, C */
    case 0xAA:  /* XOR A, D */
    case 0xAB:  /* XOR A, E */
    case 0xAF:  /* XOR A, A */
        goto XOR8OP;

    case 0xAE:  /* XOR A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto XOROP;

    case 0xEE:  /* XOR A, n */
        goto XOR8IMMOP;

    case 0xAC:  /* XOR A, IxH */
    case 0xAD:  /* XOR A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto XOROP;

    case 0xB0:  /* OR A, B */
    case 0xB1:  /* OR A, C */
    case 0xB2:  /* OR A, D */
    case 0xB3:  /* OR A, E */
    case 0xB7:  /* OR A, A */
        goto OR8OP;

    case 0xB6:  /* OR A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto OROP;

    case 0xF6:  /* OR A, n */
        goto OR8IMMOP;

    case 0xB4:  /* OR A, IxH */
    case 0xB5:  /* OR A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto OROP;

    case 0xB8:  /* CP A, B */
    case 0xB9:  /* CP A, C */
    case 0xBA:  /* CP A, D */
    case 0xBB:  /* CP A, E */
    case 0xBF:  /* CP A, A */
        goto CP8OP;

    case 0xBE:  /* CP A, (Ix + d) */
        FETCH_ARG8(_disp);
        _value = cpu->mread(cpu->offset->w + _disp);
        cycles_done += 15;
        goto CPOP;

    case 0xFE:  /* CP A, n */
        goto CP8IMMOP;

    case 0xBC:  /* CP A, IxH */
    case 0xBD:  /* CP A, IxL */
        _value = cpu->offset->bytes[(inst & 0x01) ^ 0x01];
        goto CPOP;

    case 0xC0:  /* RET NZ */
        goto RETNZOP;

    case 0xC8:  /* RET Z */
        goto RETZOP;

    case 0xC9:  /* RET */
        goto RETOP;

    case 0xD0:  /* RET NC */
        goto RETNCOP;

    case 0xD8:  /* RET C */
        goto RETCOP;

    case 0xE0:  /* RET PO */
        goto RETPOOP;

    case 0xE8:  /* RET PE */
        goto RETPEOP;

    case 0xF0:  /* RET P */
        goto RETPOP;

    case 0xF8:  /* RET M */
        goto RETMOP;

    case 0xC1:  /* POP BC */
    case 0xD1:  /* POP DE */
        goto POP16OP;

    case 0xE1:  /* POP Ix */
        cpu->offset->w = cpu->mread16(cpu->sp.w);
        cpu->sp.w += 2;
        cycles_done += 10;
        goto out;

    case 0xF1:  /* POP AF */
        goto POPAFOP;

    case 0xC2:  /* JP NZ, ee */
        goto JPNZOP;

    case 0xC3:  /* JP ee */
        goto JPOP;

    case 0xCA:  /* JP Z, ee */
        goto JPZOP;

    case 0xD2:  /* JP NC, ee */
        goto JPNCOP;

    case 0xDA:  /* JP C, ee */
        goto JPCOP;

    case 0xE2:  /* JP PO, ee */
        goto JPPOOP;

    case 0xEA:  /* JP PE, ee */
        goto JPPEOP;

    case 0xF2:  /* JP P, ee */
        goto JPPOP;

    case 0xFA:  /* JP M, ee */
        goto JPMOP;

    case 0xC4:  /* CALL NZ, ee */
        goto CALLNZOP;

    case 0xCC:  /* CALL Z, ee */
        goto CALLZOP;

    case 0xCD:  /* CALL ee */
        goto CALLOP;

    case 0xD4:  /* CALL NC, ee */
        goto CALLNCOP;
                
    case 0xDC:  /* CALL C, ee */
        goto CALLCOP;

    case 0xE4:  /* CALL PO, ee */
        goto CALLPOOP;

    case 0xEC:  /* CALL PE, ee */
        goto CALLPEOP;

    case 0xF4:  /* CALL P, ee */
        goto CALLPOP;

    case 0xFC:  /* CALL M, ee */
        goto CALLMOP;

    case 0xC5:  /* PUSH BC */
    case 0xD5:  /* PUSH DE */
        goto PUSH16OP;

    case 0xE5:  /* PUSH Ix */
        cpu->sp.w -= 2;
        cpu->mwrite16(cpu->sp.w, cpu->offset->w);
        cycles_done += 11;
        goto out;

    case 0xF5:  /* PUSH AF */
        goto PUSHAFOP;

    case 0xC7:  /* RST 0h */
    case 0xCF:  /* RST 8h */
    case 0xD7:  /* RST 10h */
    case 0xDF:  /* RST 18h */
    case 0xE7:  /* RST 20h */
    case 0xEF:  /* RST 28h */
    case 0xF7:  /* RST 30h */
    case 0xFF:  /* RST 38h */
        goto RSTOP;

    case 0xD3:  /* OUT (n), A */
        goto OUTIMMOP;

    case 0xD9:  /* EXX */
        goto EXXOP;

    case 0xDB:  /* IN A, (n) */
        goto INIMMOP;

    case 0xE3:  /* EX (SP), Ix */
        OP_EXSP(*cpu->offset);
        cycles_done += 19;
        goto out;

    case 0xE9:  /* JP (Ix) */
        cpu->pc.w = cpu->offset->w;
        cycles_done += 4;
        goto out;

    case 0xEB:  /* EX DE, Ix */
        OP_EX(cpu->de, *cpu->offset);
        cycles_done += 4;
        goto out;

    case 0xF3:  /* DI */
        goto DIOP;

    case 0xF9:  /* LD SP, Ix */
        cpu->sp.w = cpu->offset->w;
        cycles_done += 6;
        goto out;

    case 0xFB:  /* EI */
        goto EIOP;

    case 0xCB:  /* CB-prefix */
        goto execDDCB_FDCB;

    case 0xDD:  /* DD-prefix */
        cpu->offset = &cpu->ix;
        goto execDD_FD;

    case 0xED:  /* ED-prefix */
        goto execED;

    case 0xFD:  /* FD-prefix */
        cpu->offset = &cpu->iy;
        goto execDD_FD;
}
