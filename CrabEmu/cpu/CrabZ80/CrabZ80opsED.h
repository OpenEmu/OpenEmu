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

++cpu->ir.b.l;
FETCH_ARG8(inst);

switch(inst)    {
    case 0x00:  /* All undefined ED-prefixed opcodes are essentially 2 NOPs. */
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x77:
    case 0x7F:
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    case 0x8E:
    case 0x8F:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0x9E:
    case 0x9F:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xA7:
    case 0xAC:
    case 0xAD:
    case 0xAE:
    case 0xAF:
    case 0xB4:
    case 0xB5:
    case 0xB6:
    case 0xB7:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF:
    case 0xC0:
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD0:
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
    case 0xE0:
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
    case 0xE8:
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
    case 0xF0:
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
    case 0xF8:
    case 0xF9:
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
        cycles_done += 8;
        goto out;

    case 0x40:  /* IN B, (C) */
    case 0x48:  /* IN C, (C) */
    case 0x50:  /* IN D, (C) */
    case 0x58:  /* IN E, (C) */
    case 0x60:  /* IN H, (C) */
    case 0x68:  /* IN L, (C) */
    case 0x78:  /* IN A, (C) */
        REG8(inst >> 3) = _value = cpu->pread(cpu->bc.b.l);

INCOP:
        cpu->af.b.l = ZSPXYtable[_value] | (cpu->af.b.l & 0x01);
        cycles_done += 12;
        goto out;

    case 0x70:  /* IN (C) */
        _value = cpu->pread(cpu->bc.b.l);
        goto INCOP;

    case 0x41:  /* OUT (C), B */
    case 0x49:  /* OUT (C), C */
    case 0x51:  /* OUT (C), D */
    case 0x59:  /* OUT (C), E */
    case 0x61:  /* OUT (C), H */
    case 0x69:  /* OUT (C), L */
    case 0x79:  /* OUT (C), A */
        _value = REG8(inst >> 3);

OUTCOP:
        cpu->pwrite(cpu->bc.b.l, _value);
        cycles_done += 12;
        goto out;

    case 0x71:  /* OUT (C), 0 */
        _value = 0;
        goto OUTCOP;

    case 0x42:  /* SBC HL, BC */
    case 0x52:  /* SBC HL, DE */
    case 0x62:  /* SBC HL, HL */
        _value = REG16(inst >> 4);

SBCHLOP:
        OP_SBC16(cpu->hl.w, _value);
        cycles_done += 15;
        goto out;

    case 0x72:  /* SBC HL, SP */
        _value = cpu->sp.w;
        goto SBCHLOP;

    case 0x43:  /* LD (nn), BC */
    case 0x53:  /* LD (nn), DE */
    case 0x63:  /* LD (nn), HL */
        FETCH_ARG16(_value);
        cpu->mwrite16(_value, REG16(inst >> 4));
        cycles_done += 20;
        goto out;

    case 0x73:  /* LD (nn), SP */
        FETCH_ARG16(_value);
        cpu->mwrite16(_value, cpu->sp.w);
        cycles_done += 20;
        goto out;

    case 0x44:  /* NEG */
    case 0x4C:
    case 0x54:
    case 0x5C:
    case 0x64:
    case 0x6C:
    case 0x74:
    case 0x7C:
        OP_NEG();
        cycles_done += 8;
        goto out;

    case 0x45:  /* RETN */
    case 0x4D:  /* RETI */
    case 0x55:  /* RETN */
    case 0x5D:  /* RETN */
    case 0x65:  /* RETN */
    case 0x6D:  /* RETN */
    case 0x75:  /* RETN */
    case 0x7D:  /* RETN */
        cpu->pc.w = cpu->mread16(cpu->sp.w);
        cpu->sp.w += 2;
        cpu->iff1 = cpu->iff2;
        cycles_done += 14;
        goto out;

    case 0x46:  /* IM 0 */
    case 0x4E:  /* IM 0 */
    case 0x66:  /* IM 0 */
    case 0x6E:  /* IM 0 */
        cpu->im = 0;
        cycles_done += 8;
        goto out;

    case 0x56:  /* IM 1 */
    case 0x76:  /* IM 1 */
        cpu->im = 1;
        cycles_done += 8;
        goto out;

    case 0x5E:  /* IM 2 */
    case 0x7E:  /* IM 2 */
        cpu->im = 2;
        cycles_done += 8;
        goto out;

    case 0x47:  /* LD I, A */
        cpu->ir.b.h = cpu->af.b.h;
        cycles_done += 9;
        goto out;

    case 0x4A:  /* ADC HL, BC */
    case 0x5A:  /* ADC HL, DE */
    case 0x6A:  /* ADC HL, HL */
        _value = REG16(inst >> 4);

ADCHLOP:
        OP_ADC16(cpu->hl.w, _value);
        cycles_done += 15;
        goto out;

    case 0x7A:  /* ADC HL, SP */
        _value = cpu->sp.w;
        goto ADCHLOP;

    case 0x4B:  /* LD BC, (nn) */
    case 0x5B:  /* LD DE, (nn) */
    case 0x6B:  /* LD HL, (nn) */
        FETCH_ARG16(_value);
        REG16(inst >> 4) = cpu->mread16(_value);
        cycles_done += 20;
        goto out;

    case 0x7B:  /* LD SP, (nn) */
        FETCH_ARG16(_value);
        cpu->sp.w = cpu->mread16(_value);
        cycles_done += 20;
        goto out;

    case 0x4F:  /* LD R, A */
        cpu->ir.b.l = cpu->af.b.h;
        cpu->r_top = cpu->af.b.h & 0x80;
        cycles_done += 9;
        goto out;

    case 0x57:  /* LD A, I */
        cpu->af.b.h = cpu->ir.b.h;
        cpu->af.b.l = ZSXYtable[cpu->ir.b.h] | (cpu->iff2 << 2) |
            (cpu->af.b.l & 0x01);
        cycles_done += 9;
        goto out;

    case 0x5F:  /* LD A, R */
        cpu->af.b.h = (cpu->ir.b.l & 0x7F) | (cpu->r_top);
        cpu->af.b.l = ZSXYtable[cpu->ir.b.l] | (cpu->iff2 << 2) |
            (cpu->af.b.l & 0x01);
        cycles_done += 9;
        goto out;

    case 0x67:  /* RRD */
        OP_RRD();
        cycles_done += 18;
        goto out;

    case 0x6F:  /* RLD */
        OP_RLD();
        cycles_done += 18;
        goto out;

    case 0xA0:  /* LDI */
LDIOP:
        OP_LDI();
        cycles_done += 16;
        goto out;

    case 0xA1:  /* CPI */
        OP_CPI();
        cycles_done += 16;
        goto out;

    case 0xA2:  /* INI */
INIOP:
        OP_INI();
        cycles_done += 16;
        goto out;

    case 0xA3:  /* OUTI */
OUTIOP:
        OP_OUTI();
        cycles_done += 16;
        goto out;

    case 0xA8:  /* LDD */
LDDOP:
        OP_LDD();
        cycles_done += 16;
        goto out;

    case 0xA9:  /* CPD */
        OP_CPD();
        cycles_done += 16;
        goto out;

    case 0xAA:  /* IND */
INDOP:
        OP_IND();
        cycles_done += 16;
        goto out;

    case 0xAB:  /* OUTD */
OUTDOP:
        OP_OUTD();
        cycles_done += 16;
        goto out;

    case 0xB0:  /* LDIR */
        if(cpu->bc.w != 1)  {
            cpu->pc.w -= 2;
            cycles_done += 5;
        }

        goto LDIOP;

    case 0xB1:  /* CPIR */
        OP_CPIR();
        cycles_done += 16;
        goto out;

    case 0xB2:  /* INIR */
        if(cpu->bc.b.h != 1)    {
            cpu->pc.w -= 2;
            cycles_done += 5;
        }

        goto INIOP;

    case 0xB3:  /* OTIR */
        if(cpu->bc.b.h != 1)    {
            cpu->pc.w -= 2;
            cycles_done += 5;
        }

        goto OUTIOP;

    case 0xB8:  /* LDDR */
        if(cpu->bc.w != 1)  {
            cpu->pc.w -= 2;
            cycles_done += 5;
        }

        goto LDDOP;

    case 0xB9:  /* CPDR */
        OP_CPDR();
        cycles_done += 16;
        goto out;

    case 0xBA:  /* INDR */
        if(cpu->bc.b.h != 1)    {
            cpu->pc.w -= 2;
            cycles_done += 5;
        }

        goto INDOP;

    case 0xBB:  /* OTDR */
        if(cpu->bc.b.h != 1)    {
            cpu->pc.w -= 2;
            cycles_done += 5;
        }

        goto OUTDOP;
}
