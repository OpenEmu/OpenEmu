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

/* Grr... stupid GCC thinks this variable may be used uninitialized... */
uint32 _value = 0;
int8 _disp;
uint32 _tmp;

switch(inst)    {
    case 0x00:  /* NOP */
    case 0x40:  /* LD B, B */
    case 0x49:  /* LD C, C */
    case 0x52:  /* LD D, D */
    case 0x5B:  /* LD E, E */
    case 0x64:  /* LD H, H */
    case 0x6D:  /* LD L, L */
    case 0x7F:  /* LD A, A */
        cycles_done += 4;
        goto out;

    case 0x01:  /* LD BC, nn */
    case 0x11:  /* LD DE, nn */
    case 0x21:  /* LD HL, nn */
LD16IMMOP:
        FETCH_ARG16(_value);
        REG16(inst >> 4) = _value;
        cycles_done += 10;
        goto out;

    case 0x31:  /* LD SP, nn */
LDSPIMMOP:
        FETCH_ARG16(_value);
        cpu->sp.w = _value;
        cycles_done += 10;
        goto out;

    case 0x02:  /* LD (BC), A */
    case 0x12:  /* LD (DE), A */
LDATMOP:
        cpu->mwrite(REG16(inst >> 4), cpu->af.b.h);
        cycles_done += 7;
        goto out;

    case 0x03:  /* INC BC */
        ++cpu->bc.w;
        cycles_done += 6;
        goto out;

    case 0x13:  /* INC DE */
        ++cpu->de.w;
        cycles_done += 6;
        goto out;

    case 0x23:  /* INC HL */
        ++cpu->hl.w;
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
    case 0x24:  /* INC H */
    case 0x2C:  /* INC L */
    case 0x3C:  /* INC A */
INCR8OP:
        OP_INCR(inst >> 3);
        cycles_done += 4;
        goto out;

    case 0x34:  /* INC (HL) */
        _value = cpu->mread(cpu->hl.w);
        OP_INC8(_value);
        cpu->mwrite(cpu->hl.w, _value);
        cycles_done += 11;
        goto out;

    case 0x05:  /* DEC B */
    case 0x0D:  /* DEC C */
    case 0x15:  /* DEC D */
    case 0x1D:  /* DEC E */
    case 0x25:  /* DEC H */
    case 0x2D:  /* DEC L */
    case 0x3D:  /* DEC A */
DECR8OP:
        OP_DEC8(REG8(inst >> 3));
        cycles_done += 4;
        goto out;

    case 0x35:  /* DEC (HL) */
        _value = cpu->mread(cpu->hl.w);
        OP_DEC8(_value);
        cpu->mwrite(cpu->hl.w, _value);
        cycles_done += 11;
        goto out;

    case 0x06:  /* LD B, n */
    case 0x0E:  /* LD C, n */
    case 0x16:  /* LD D, n */
    case 0x1E:  /* LD E, n */
    case 0x26:  /* LD H, n */
    case 0x2E:  /* LD L, n */
    case 0x3E:  /* LD A, n */
LD8IMMOP:
        FETCH_ARG8(_value);
        REG8(inst >> 3) = _value;
        cycles_done += 7;
        goto out;

    case 0x36:  /* LD (HL), n */
        FETCH_ARG8(_value);
        cpu->mwrite(cpu->hl.w, _value);
        cycles_done += 10;
        goto out;

    case 0x07:  /* RLCA */
RLCAOP:
        OP_RLCA();
        cycles_done += 4;
        goto out;

    case 0x08:  /* EX AF, AF' */
EXAFAFPOP:
        OP_EX(cpu->af, cpu->afp);
        cycles_done += 4;
        goto out;

    case 0x09:  /* ADD HL, BC */
    case 0x19:  /* ADD HL, DE */
    case 0x29:  /* ADD HL, HL */
        _value = REG16(inst >> 4);

ADDHLOP:
        cpu->internal_reg = cpu->hl.b.h;
        OP_ADDHL();
        cycles_done += 11;
        goto out;

    case 0x39:  /* ADD HL, SP */
        _value = cpu->sp.w;
        goto ADDHLOP;

    case 0x0A:  /* LD A, (BC) */
    case 0x1A:  /* LD A, (DE) */
LDAFMEMOP:
        cpu->af.b.h = cpu->mread(REG16(inst >> 4));
        cycles_done += 7;
        goto out;

    case 0x0B:  /* DEC BC */
        --cpu->bc.w;
        cycles_done += 6;
        goto out;
        
    case 0x1B:  /* DEC DE */
        --cpu->de.w;
        cycles_done += 6;
        goto out;
        
    case 0x2B:  /* DEC HL */
        --cpu->hl.w;
        cycles_done += 6;
        goto out;
        
    case 0x3B:  /* DEC SP */
        --cpu->sp.w;
        cycles_done += 6;
        goto out;

    case 0x0F:  /* RRCA */
RRCAOP:
        OP_RRCA();
        cycles_done += 4;
        goto out;

    case 0x10:  /* DJNZ e */
DJNZOP:
        if(--cpu->bc.b.h)   {
            cycles_done += 1;
            goto JROP;
        }

        ++cpu->pc.w;
        cycles_done += 8;
        goto out;

    case 0x18:  /* JR e */
JROP:
        FETCH_ARG8(_value);
        cpu->pc.w += (int8)_value;
        cycles_done += 12;
        cpu->internal_reg = cpu->pc.b.h;
        goto out;

    case 0x20:  /* JR NZ, e */
JRNZOP:
        if(!(cpu->af.b.l & 0x40))   {
            goto JROP;
        }

        ++cpu->pc.w;
        cycles_done += 7;
        goto out;

    case 0x28:  /* JR Z, e */
JRZOP:
        if(cpu->af.b.l & 0x40)  {
            goto JROP;
        }

        ++cpu->pc.w;
        cycles_done += 7;
        goto out;

    case 0x30:  /* JR NC, e */
JRNCOP:
        if(!(cpu->af.b.l & 0x01))   {
            goto JROP;
        }

        ++cpu->pc.w;
        cycles_done += 7;
        goto out;

    case 0x38:  /* JR C, e */
JRCOP:
        if(cpu->af.b.l & 0x01)  {
            goto JROP;
        }

        ++cpu->pc.w;
        cycles_done += 7;
        goto out;

    case 0x17:  /* RLA */
RLAOP:
        OP_RLA();
        cycles_done += 4;
        goto out;

    case 0x1F:  /* RRA */
RRAOP:
        OP_RRA();
        cycles_done += 4;
        goto out;

    case 0x22:  /* LD (nn), HL */
        FETCH_ARG16(_value);
        cpu->mwrite16(_value, cpu->hl.w);
        cycles_done += 16;
        goto out;

    case 0x27:  /* DAA */
DAAOP:
    {
        int low = cpu->af.b.h & 0x0F;
        int high = cpu->af.b.h >> 4;
        int cf = cpu->af.b.l & 0x01;
        int hf = cpu->af.b.l & 0x10;
        int nf = cpu->af.b.l & 0x02;
        
        if(cf) {
            _value = (low < 0x0A && !hf) ? 0x60 : 0x66;
        }
        else    {
            if(low < 0x0A)  {
                if(high < 0x0A) {
                    _value = (hf) ? 0x06 : 0x00;
                }
                else    {
                    _value = (hf) ? 0x66 : 0x60;
                }
            }
            else    {
                _value = (high < 0x09) ? 0x06 : 0x66;
            }
        }

        if(nf)  {
            cpu->af.b.h -= _value;
        }
        else    {
            cpu->af.b.h += _value;
        }

        cpu->af.b.l = ZSPXYtable[cpu->af.b.h] | (nf);

        if(_value >= 0x60)
            cpu->af.b.l |= 0x01;

        if(nf)  {
            if(hf && low < 0x06)    {
                cpu->af.b.l |= 0x10;
            }
        }
        else if(low >= 10)  {
            cpu->af.b.l |= 0x10;
        }

        cycles_done += 4;
        goto out;
    }

    case 0x2A:  /* LD HL, (nn) */
        FETCH_ARG16(_value);
        cpu->hl.w = cpu->mread16(_value);
        cycles_done += 16;
        goto out;

    case 0x2F:  /* CPL */
CPLOP:
        OP_CPL();
        cycles_done += 4;
        goto out;

    case 0x32:  /* LD (nn), A */
LDATMABSOP:
        FETCH_ARG16(_value);
        cpu->mwrite(_value, cpu->af.b.h);
        cycles_done += 13;
        goto out;

    case 0x37:  /* SCF */
SCFOP:
        OP_SCF();
        cycles_done += 4;
        goto out;

    case 0x3A:  /* LD A, (nn) */
LDAFMABSOP:
        FETCH_ARG16(_value);
        cpu->af.b.h = cpu->mread(_value);
        cycles_done += 13;
        goto out;

    case 0x3F:  /* CCF */
CCFOP:
        OP_CCF();
        cycles_done += 4;
        goto out;

    case 0x41:  /* LD B, C */
    case 0x42:  /* LD B, D */
    case 0x43:  /* LD B, E */
    case 0x44:  /* LD B, H */
    case 0x45:  /* LD B, L */
    case 0x47:  /* LD B, A */
    case 0x48:  /* LD C, B */
    case 0x4A:  /* LD C, D */
    case 0x4B:  /* LD C, E */
    case 0x4C:  /* LD C, H */
    case 0x4D:  /* LD C, L */
    case 0x4F:  /* LD C, A */
    case 0x50:  /* LD D, B */
    case 0x51:  /* LD D, C */
    case 0x53:  /* LD D, E */
    case 0x54:  /* LD D, H */
    case 0x55:  /* LD D, L */
    case 0x57:  /* LD D, A */
    case 0x58:  /* LD E, B */
    case 0x59:  /* LD E, C */
    case 0x5A:  /* LD E, D */
    case 0x5C:  /* LD E, H */
    case 0x5D:  /* LD E, L */
    case 0x5F:  /* LD E, A */
    case 0x60:  /* LD H, B */
    case 0x61:  /* LD H, C */
    case 0x62:  /* LD H, D */
    case 0x63:  /* LD H, E */
    case 0x65:  /* LD H, L */
    case 0x67:  /* LD H, A */
    case 0x68:  /* LD L, B */
    case 0x69:  /* LD L, C */
    case 0x6A:  /* LD L, D */
    case 0x6B:  /* LD L, E */
    case 0x6C:  /* LD L, H */
    case 0x6F:  /* LD L, A */
    case 0x78:  /* LD A, B */
    case 0x79:  /* LD A, C */
    case 0x7A:  /* LD A, D */
    case 0x7B:  /* LD A, E */
    case 0x7C:  /* LD A, H */
    case 0x7D:  /* LD A, L */
        REG8(inst >> 3) = REG8(inst);
        cycles_done += 4;
        goto out;

    case 0x46:  /* LD B, (HL) */
    case 0x4E:  /* LD C, (HL) */
    case 0x56:  /* LD D, (HL) */
    case 0x5E:  /* LD E, (HL) */
    case 0x66:  /* LD H, (HL) */
    case 0x6E:  /* LD L, (HL) */
    case 0x7E:  /* LD A, (HL) */
        REG8(inst >> 3) = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto out;

    case 0x70:  /* LD (HL), B */
    case 0x71:  /* LD (HL), C */
    case 0x72:  /* LD (HL), D */
    case 0x73:  /* LD (HL), E */
    case 0x74:  /* LD (HL), H */
    case 0x75:  /* LD (HL), L */
    case 0x77:  /* LD (HL), A */
        cpu->mwrite(cpu->hl.w, REG8(inst));
        cycles_done += 7;
        goto out;

    case 0x76:  /* HALT */
        OP_HALT();
        cycles_done += 4;
        goto out;

    case 0x80:  /* ADD A, B */
    case 0x81:  /* ADD A, C */
    case 0x82:  /* ADD A, D */
    case 0x83:  /* ADD A, E */
    case 0x84:  /* ADD A, H */
    case 0x85:  /* ADD A, L */
    case 0x87:  /* ADD A, A */
ADD8OP:
        _value = REG8(inst);
        cycles_done += 4;
        
ADDOP:
        OP_ADD();
        goto out;

    case 0x86: /* ADD A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto ADDOP;

    case 0xC6:  /* ADD A, n */
ADD8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto ADDOP;

    case 0x88:  /* ADC A, B */
    case 0x89:  /* ADC A, C */
    case 0x8A:  /* ADC A, D */
    case 0x8B:  /* ADC A, E */
    case 0x8C:  /* ADC A, H */
    case 0x8D:  /* ADC A, L */
    case 0x8F:  /* ADC A, A */
ADC8OP:
        _value = REG8(inst);
        cycles_done += 4;

ADCOP:
        OP_ADC();
        goto out;

    case 0x8E:  /* ADC A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto ADCOP;

    case 0xCE:  /* ADC A, n */
ADC8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto ADCOP;

    case 0x90:  /* SUB A, B */
    case 0x91:  /* SUB A, C */
    case 0x92:  /* SUB A, D */
    case 0x93:  /* SUB A, E */
    case 0x94:  /* SUB A, H */
    case 0x95:  /* SUB A, L */
    case 0x97:  /* SUB A, A */
SUB8OP:
        _value = REG8(inst);
        cycles_done += 4;

SUBOP:
        OP_SUB();
        goto out;

    case 0x96:  /* SUB A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto SUBOP;
        
    case 0xD6:  /* SUB A, n */
SUB8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto SUBOP;

    case 0x98:  /* SBC A, B */
    case 0x99:  /* SBC A, C */
    case 0x9A:  /* SBC A, D */
    case 0x9B:  /* SBC A, E */
    case 0x9C:  /* SBC A, H */
    case 0x9D:  /* SBC A, L */
    case 0x9F:  /* SBC A, A */
SBC8OP:
        _value = REG8(inst);
        cycles_done += 4;

SBCOP:
        OP_SBC();
        goto out;

    case 0x9E:  /* SBC A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto SBCOP;

    case 0xDE:  /* SBC A, n */
SBC8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto SBCOP;

    case 0xA0:  /* AND A, B */
    case 0xA1:  /* AND A, C */
    case 0xA2:  /* AND A, D */
    case 0xA3:  /* AND A, E */
    case 0xA4:  /* AND A, H */
    case 0xA5:  /* AND A, L */
    case 0xA7:  /* AND A, A */
AND8OP:
        _value = REG8(inst);
        cycles_done += 4;

ANDOP:
        OP_AND();
        goto out;

    case 0xA6:  /* AND A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto ANDOP;

    case 0xE6:  /* AND A, n */
AND8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto ANDOP;

    case 0xA8:  /* XOR A, B */
    case 0xA9:  /* XOR A, C */
    case 0xAA:  /* XOR A, D */
    case 0xAB:  /* XOR A, E */
    case 0xAC:  /* XOR A, H */
    case 0xAD:  /* XOR A, L */
    case 0xAF:  /* XOR A, A */
XOR8OP:
        _value = REG8(inst);
        cycles_done += 4;

XOROP:
        OP_XOR();
        goto out;

    case 0xAE:  /* XOR A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto XOROP;

    case 0xEE:  /* XOR A, n */
XOR8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto XOROP;

    case 0xB0:  /* OR A, B */
    case 0xB1:  /* OR A, C */
    case 0xB2:  /* OR A, D */
    case 0xB3:  /* OR A, E */
    case 0xB4:  /* OR A, H */
    case 0xB5:  /* OR A, L */
    case 0xB7:  /* OR A, A */
OR8OP:
        _value = REG8(inst);
        cycles_done += 4;

OROP:
        OP_OR();
        goto out;

    case 0xB6:  /* OR A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto OROP;

    case 0xF6:  /* OR A, n */
OR8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto OROP;

    case 0xB8:  /* CP A, B */
    case 0xB9:  /* CP A, C */
    case 0xBA:  /* CP A, D */
    case 0xBB:  /* CP A, E */
    case 0xBC:  /* CP A, H */
    case 0xBD:  /* CP A, L */
    case 0xBF:  /* CP A, A */
CP8OP:
        _value = REG8(inst);
        cycles_done += 4;

CPOP:
        OP_CP();
        goto out;

    case 0xBE:  /* CP A, (HL) */
        _value = cpu->mread(cpu->hl.w);
        cycles_done += 7;
        goto CPOP;

    case 0xFE:  /* CP A, n */
CP8IMMOP:
        FETCH_ARG8(_value);
        cycles_done += 7;
        goto CPOP;

    case 0xC0:  /* RET NZ */
RETNZOP:
        if(!(cpu->af.b.l & 0x40))   {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xC8:  /* RET Z */
RETZOP:
        if(!(cpu->af.b.l & 0x40))   {
            cycles_done += 5;
            goto out;
        }

        /* Fall through... */

CONDRET:
        cycles_done += 1;
    case 0xC9:  /* RET */
RETOP:
        cpu->pc.w = cpu->mread16(cpu->sp.w);
        cpu->sp.w += 2;
        cycles_done += 10;
        goto out;

    case 0xD0:  /* RET NC */
RETNCOP:
        if(!(cpu->af.b.l & 0x01))   {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xD8:  /* RET C */
RETCOP:
        if(cpu->af.b.l & 0x01)  {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xE0:  /* RET PO */
RETPOOP:
        if(!(cpu->af.b.l & 0x04))   {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xE8:  /* RET PE */
RETPEOP:
        if(cpu->af.b.l & 0x04)  {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xF0:  /* RET P */
RETPOP:
        if(!(cpu->af.b.l & 0x80))   {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xF8:  /* RET M */
RETMOP:
        if(cpu->af.b.l & 0x80)  {
            goto CONDRET;
        }

        cycles_done += 5;
        goto out;

    case 0xC1:  /* POP BC */
    case 0xD1:  /* POP DE */
    case 0xE1:  /* POP HL */
POP16OP:
        REG16(inst >> 4) = cpu->mread16(cpu->sp.w);
        cpu->sp.w += 2;
        cycles_done += 10;
        goto out;

    case 0xF1:  /* POP AF */
POPAFOP:
        OP_POPAF();
        cycles_done += 10;
        goto out;

    case 0xC2:  /* JP NZ, ee */
JPNZOP:
        if(cpu->af.b.l & 0x40)   {
            goto out_nocondjump;
        }

        /* Fall through... */

    case 0xC3:  /* JP ee */
JPOP:
        FETCH_ARG16(_value);
        cpu->pc.w = _value;
        cycles_done += 10;
        goto out;

    case 0xCA:  /* JP Z, ee */
JPZOP:
        if(cpu->af.b.l & 0x40)  {
            goto JPOP;
        }

        goto out_nocondjump;

    case 0xD2:  /* JP NC, ee */
JPNCOP:
        if(!(cpu->af.b.l & 0x01))   {
            goto JPOP;
        }
    
        goto out_nocondjump;

    case 0xDA:  /* JP C, ee */
JPCOP:
        if(cpu->af.b.l & 0x01)  {
            goto JPOP;
        }

        goto out_nocondjump;

    case 0xE2:  /* JP PO, ee */
JPPOOP:
        if(!(cpu->af.b.l & 0x04))   {
            goto JPOP;
        }

        goto out_nocondjump;

    case 0xEA:  /* JP PE, ee */
JPPEOP:
        if(cpu->af.b.l & 0x04)  {
            goto JPOP;
        }

        goto out_nocondjump;

    case 0xF2:  /* JP P, ee */
JPPOP:
        if(!(cpu->af.b.l & 0x80))   {
            goto JPOP;
        }

        goto out_nocondjump;

    case 0xFA:  /* JP M, ee */
JPMOP:
        if(cpu->af.b.l & 0x80)  {
            goto JPOP;
        }

        goto out_nocondjump;

    case 0xC4:  /* CALL NZ, ee */
CALLNZOP:
        if(!(cpu->af.b.l & 0x40))   {
            goto CALLOP;
        }

        goto out_nocondjump;

    case 0xCC:  /* CALL Z, ee */
CALLZOP:
        if(!(cpu->af.b.l & 0x40))   {
            goto out_nocondjump;
        }

        /* Fall through... */

    case 0xCD:  /* CALL ee */
CALLOP:
        FETCH_ARG16(_value);
        cpu->sp.w -= 2;
        cpu->mwrite16(cpu->sp.w, cpu->pc.w);
        cpu->pc.w = _value;
        cycles_done += 17;
        goto out;

    case 0xD4:  /* CALL NC, ee */
CALLNCOP:
        if(!(cpu->af.b.l & 0x01))   {
            goto CALLOP;
        }

        goto out_nocondjump;
                
    case 0xDC:  /* CALL C, ee */
CALLCOP:
        if(cpu->af.b.l & 0x01)  {
            goto CALLOP;
        }

        goto out_nocondjump;

    case 0xE4:  /* CALL PO, ee */
CALLPOOP:
        if(!(cpu->af.b.l & 0x04))   {
            goto CALLOP;
        }

        goto out_nocondjump;

    case 0xEC:  /* CALL PE, ee */
CALLPEOP:
        if(cpu->af.b.l & 0x04)  {
            goto CALLOP;
        }

        goto out_nocondjump;

    case 0xF4:  /* CALL P, ee */
CALLPOP:
        if(!(cpu->af.b.l & 0x80))   {
            goto CALLOP;
        }

        goto out_nocondjump;

    case 0xFC:  /* CALL M, ee */
CALLMOP:
        if(cpu->af.b.l & 0x80)  {
            goto CALLOP;
        }

        goto out_nocondjump;

    case 0xC5:  /* PUSH BC */
    case 0xD5:  /* PUSH DE */
    case 0xE5:  /* PUSH HL */
PUSH16OP:
        cpu->sp.w -= 2;
        cpu->mwrite16(cpu->sp.w, REG16(inst >> 4));
        cycles_done += 11;
        goto out;

    case 0xF5:  /* PUSH AF */
PUSHAFOP:
        OP_PUSHAF();
        cycles_done += 11;
        goto out;

    case 0xC7:  /* RST 0h */
    case 0xCF:  /* RST 8h */
    case 0xD7:  /* RST 10h */
    case 0xDF:  /* RST 18h */
    case 0xE7:  /* RST 20h */
    case 0xEF:  /* RST 28h */
    case 0xF7:  /* RST 30h */
    case 0xFF:  /* RST 38h */
RSTOP:
        cpu->sp.w -= 2;
        cpu->mwrite16(cpu->sp.w, cpu->pc.w);
        cpu->pc.w = inst & 0x38;
        cycles_done += 11;
        goto out;

    case 0xD3:  /* OUT (n), A */
OUTIMMOP:
        FETCH_ARG8(_value);
        cpu->pwrite(_value, cpu->af.b.h);
        cycles_done += 11;
        goto out;

    case 0xD9:  /* EXX */
EXXOP:
        OP_EXX();
        cycles_done += 4;
        goto out;

    case 0xDB:  /* IN A, (n) */
INIMMOP:
        FETCH_ARG8(_value);
        cpu->af.b.h = cpu->pread(_value);
        cycles_done += 11;
        goto out;

    case 0xE3:  /* EX (SP), HL */
        OP_EXSP(cpu->hl);
        cycles_done += 19;
        goto out;

    case 0xE9:  /* JP (HL) */
        cpu->pc.w = cpu->hl.w;
        cycles_done += 4;
        goto out;

    case 0xEB:  /* EX DE, HL */
        OP_EX(cpu->de, cpu->hl);
        cycles_done += 4;
        goto out;

    case 0xF3:  /* DI */
DIOP:
        cpu->iff1 = cpu->iff2 = 0;
        cycles_done += 4;
        goto out;

    case 0xF9:  /* LD SP, HL */
        cpu->sp.w = cpu->hl.w;
        cycles_done += 6;
        goto out;

    case 0xFB:  /* EI */
EIOP:
        cpu->iff1 = cpu->iff2 = cpu->ei = 1;
        cycles_done += 4;
        goto out;

    case 0xCB:  /* CB-prefix */
        goto execCB;

    case 0xDD:  /* DD-prefix */
        cpu->offset = &cpu->ix;
        goto execDD_FD;

    case 0xED:  /* ED-prefix */
        goto execED;

    case 0xFD:  /* FD-prefix */
        cpu->offset = &cpu->iy;
        goto execDD_FD;
}

execCB:
#include "CrabZ80opsCB.h"
/* We shouldn't get here. */

execDD_FD:
#include "CrabZ80opsDD-FD.h"
/* We shouldn't get here. */

execED:
#include "CrabZ80opsED.h"
/* We shouldn't get here. */

execDDCB_FDCB:
#include "CrabZ80opsDDCB-FDCB.h"
/* We shouldn't get here. */

/* Conditional JP and CALL instructions that don't end up jumping end up
   coming here instead. This falls through back to CrabZ80.c (essentially to the
   same place that goto out would put us). */
out_nocondjump:
    cycles_done += 10;
    cpu->pc.w += 2;

    /* Fall through... */
