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

#ifndef CRABZ80_MAMEZ80_COMPAT
++cpu->ir.b.l;
#endif
FETCH_ARG8(_disp);
FETCH_ARG8(inst);
_tmp = cpu->mread(cpu->offset->w + _disp);

switch(inst)    {
    case 0x00:  /* RLC (Ix + d), B */
    case 0x01:  /* RLC (Ix + d), C */
    case 0x02:  /* RLC (Ix + d), D */
    case 0x03:  /* RLC (Ix + d), E */
    case 0x04:  /* RLC (Ix + d), H */
    case 0x05:  /* RLC (Ix + d), L */
    case 0x07:  /* RLC (Ix + d), A */
        _value = REG8(inst) = (uint8)((_tmp << 1) | (_tmp >> 7));
        cpu->af.b.l = ZSPXYtable[_value] | (_value & 0x01);
        cycles_done += 23;
        goto writeResult;

    case 0x06:  /* RLC (Ix + d) */
        _value = (uint8)((_tmp << 1) | (_tmp >> 7));
        cpu->af.b.l = ZSPXYtable[_value] | (_value & 0x01);
        cycles_done += 23;
        goto writeResult;

    case 0x08:  /* RRC (Ix + d), B */
    case 0x09:  /* RRC (Ix + d), C */
    case 0x0A:  /* RRC (Ix + d), D */
    case 0x0B:  /* RRC (Ix + d), E */
    case 0x0C:  /* RRC (Ix + d), H */
    case 0x0D:  /* RRC (Ix + d), L */
    case 0x0F:  /* RRC (Ix + d), A */
        _value = (uint8)((_tmp >> 1) | (_tmp << 7));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;

    case 0x0E:  /* RRC (Ix + d) */
        _value = (uint8)((_tmp >> 1) | (_tmp << 7));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        cycles_done += 23;
        goto writeResult;

    case 0x10:  /* RL (Ix + d), B */
    case 0x11:  /* RL (Ix + d), C */
    case 0x12:  /* RL (Ix + d), D */
    case 0x13:  /* RL (Ix + d), E */
    case 0x14:  /* RL (Ix + d), H */
    case 0x15:  /* RL (Ix + d), L */
    case 0x17:  /* RL (Ix + d), A */
        _value = (uint8)((_tmp << 1) | (cpu->af.b.l & 0x01));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp >> 7);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;

    case 0x16:  /* RL (Ix + d) */
        _value = (uint8)((_tmp << 1) | (cpu->af.b.l & 0x01));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp >> 7);
        cycles_done += 23;
        goto writeResult;

    case 0x18:  /* RR (Ix + d), B */
    case 0x19:  /* RR (Ix + d), C */
    case 0x1A:  /* RR (Ix + d), D */
    case 0x1B:  /* RR (Ix + d), E */
    case 0x1C:  /* RR (Ix + d), H */
    case 0x1D:  /* RR (Ix + d), L */
    case 0x1F:  /* RR (Ix + d), A */
        _value = (uint8)((_tmp >> 1) | ((cpu->af.b.l & 0x01) << 7));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;

    case 0x1E:  /* RR (Ix + d) */
        _value = (uint8)((_tmp >> 1) | ((cpu->af.b.l & 0x01) << 7));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        cycles_done += 23;
        goto writeResult;

    case 0x20:  /* SLA (Ix + d), B */
    case 0x21:  /* SLA (Ix + d), C */
    case 0x22:  /* SLA (Ix + d), D */
    case 0x23:  /* SLA (Ix + d), E */
    case 0x24:  /* SLA (Ix + d), H */
    case 0x25:  /* SLA (Ix + d), L */
    case 0x27:  /* SLA (Ix + d), A */
        _value = (uint8)(_tmp << 1);
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp >> 7);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;

    case 0x26:  /* SLA (Ix + d) */
        _value = (uint8)(_tmp << 1);
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp >> 7);
        cycles_done += 23;
        goto writeResult;

    case 0x28:  /* SRA (Ix + d), B */
    case 0x29:  /* SRA (Ix + d), C */
    case 0x2A:  /* SRA (Ix + d), D */
    case 0x2B:  /* SRA (Ix + d), E */
    case 0x2C:  /* SRA (Ix + d), H */
    case 0x2D:  /* SRA (Ix + d), L */
    case 0x2F:  /* SRA (Ix + d), A */
        _value = (uint8)((_tmp >> 1) | (_tmp & 0x80));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;

    case 0x2E:  /* SRA (Ix + d) */
        _value = (uint8)((_tmp >> 1) | (_tmp & 0x80));
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        cycles_done += 23;
        goto writeResult;

    case 0x30:  /* SLL (Ix + d), B */
    case 0x31:  /* SLL (Ix + d), C */
    case 0x32:  /* SLL (Ix + d), D */
    case 0x33:  /* SLL (Ix + d), E */
    case 0x34:  /* SLL (Ix + d), H */
    case 0x35:  /* SLL (Ix + d), L */
    case 0x37:  /* SLL (Ix + d), A */
        _value = (uint8)((_tmp << 1) | 0x01);
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp >> 7);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;

    case 0x36:  /* SLL (Ix + d) */
        _value = (uint8)((_tmp << 1) | 0x01);
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp >> 7);
        cycles_done += 23;
        goto writeResult;

    case 0x38:  /* SRL (Ix + d), B */
    case 0x39:  /* SRL (Ix + d), C */
    case 0x3A:  /* SRL (Ix + d), D */
    case 0x3B:  /* SRL (Ix + d), E */
    case 0x3C:  /* SRL (Ix + d), H */
    case 0x3D:  /* SRL (Ix + d), L */
    case 0x3F:  /* SRL (Ix + d), A */
        _value = (uint8)(_tmp >> 1);
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        REG8(inst) = _value;
        cycles_done += 23;
        goto writeResult;
        
    case 0x3E:  /* SRL (Ix + d) */
        _value = (uint8)(_tmp >> 1);
        cpu->af.b.l = ZSPXYtable[_value] | (_tmp & 0x01);
        cycles_done += 23;
        goto writeResult;

    case 0x40:  /* BIT 0, (Ix + d) */
    case 0x41:  /* BIT 0, (Ix + d) */
    case 0x42:  /* BIT 0, (Ix + d) */
    case 0x43:  /* BIT 0, (Ix + d) */
    case 0x44:  /* BIT 0, (Ix + d) */
    case 0x45:  /* BIT 0, (Ix + d) */
    case 0x46:  /* BIT 0, (Ix + d) */
    case 0x47:  /* BIT 0, (Ix + d) */
    case 0x48:  /* BIT 1, (Ix + d) */
    case 0x49:  /* BIT 1, (Ix + d) */
    case 0x4A:  /* BIT 1, (Ix + d) */
    case 0x4B:  /* BIT 1, (Ix + d) */
    case 0x4C:  /* BIT 1, (Ix + d) */
    case 0x4D:  /* BIT 1, (Ix + d) */
    case 0x4E:  /* BIT 1, (Ix + d) */
    case 0x4F:  /* BIT 1, (Ix + d) */
    case 0x50:  /* BIT 2, (Ix + d) */
    case 0x51:  /* BIT 2, (Ix + d) */
    case 0x52:  /* BIT 2, (Ix + d) */
    case 0x53:  /* BIT 2, (Ix + d) */
    case 0x54:  /* BIT 2, (Ix + d) */
    case 0x55:  /* BIT 2, (Ix + d) */
    case 0x56:  /* BIT 2, (Ix + d) */
    case 0x57:  /* BIT 2, (Ix + d) */
    case 0x58:  /* BIT 3, (Ix + d) */
    case 0x59:  /* BIT 3, (Ix + d) */
    case 0x5A:  /* BIT 3, (Ix + d) */
    case 0x5B:  /* BIT 3, (Ix + d) */
    case 0x5C:  /* BIT 3, (Ix + d) */
    case 0x5D:  /* BIT 3, (Ix + d) */
    case 0x5E:  /* BIT 3, (Ix + d) */
    case 0x5F:  /* BIT 3, (Ix + d) */
    case 0x60:  /* BIT 4, (Ix + d) */
    case 0x61:  /* BIT 4, (Ix + d) */
    case 0x62:  /* BIT 4, (Ix + d) */
    case 0x63:  /* BIT 4, (Ix + d) */
    case 0x64:  /* BIT 4, (Ix + d) */
    case 0x65:  /* BIT 4, (Ix + d) */
    case 0x66:  /* BIT 4, (Ix + d) */
    case 0x67:  /* BIT 4, (Ix + d) */
    case 0x68:  /* BIT 5, (Ix + d) */
    case 0x69:  /* BIT 5, (Ix + d) */
    case 0x6A:  /* BIT 5, (Ix + d) */
    case 0x6B:  /* BIT 5, (Ix + d) */
    case 0x6C:  /* BIT 5, (Ix + d) */
    case 0x6D:  /* BIT 5, (Ix + d) */
    case 0x6E:  /* BIT 5, (Ix + d) */
    case 0x6F:  /* BIT 5, (Ix + d) */
    case 0x70:  /* BIT 6, (Ix + d) */
    case 0x71:  /* BIT 6, (Ix + d) */
    case 0x72:  /* BIT 6, (Ix + d) */
    case 0x73:  /* BIT 6, (Ix + d) */
    case 0x74:  /* BIT 6, (Ix + d) */
    case 0x75:  /* BIT 6, (Ix + d) */
    case 0x76:  /* BIT 6, (Ix + d) */
    case 0x77:  /* BIT 6, (Ix + d) */
    case 0x78:  /* BIT 7, (Ix + d) */
    case 0x79:  /* BIT 7, (Ix + d) */
    case 0x7A:  /* BIT 7, (Ix + d) */
    case 0x7B:  /* BIT 7, (Ix + d) */
    case 0x7C:  /* BIT 7, (Ix + d) */
    case 0x7D:  /* BIT 7, (Ix + d) */
    case 0x7E:  /* BIT 7, (Ix + d) */
    case 0x7F:  /* BIT 7, (Ix + d) */
        cpu->af.b.l = (ZSPXYtable[_tmp & (1 << ((inst >> 3) & 0x07))] & 0xD7) |
            0x10 | (cpu->af.b.l & 0x01);
        _tmp = (cpu->offset->w + _disp) >> 8;
        cpu->af.b.l |= _tmp & 0x28;
        cycles_done += 20;
        goto out;

    case 0x80:  /* RES 0, (Ix + d), B */
    case 0x81:  /* RES 0, (Ix + d), C */
    case 0x82:  /* RES 0, (Ix + d), D */
    case 0x83:  /* RES 0, (Ix + d), E */
    case 0x84:  /* RES 0, (Ix + d), H */
    case 0x85:  /* RES 0, (Ix + d), L */
    case 0x87:  /* RES 0, (Ix + d), A */
    case 0x88:  /* RES 1, (Ix + d), B */
    case 0x89:  /* RES 1, (Ix + d), C */
    case 0x8A:  /* RES 1, (Ix + d), D */
    case 0x8B:  /* RES 1, (Ix + d), E */
    case 0x8C:  /* RES 1, (Ix + d), H */
    case 0x8D:  /* RES 1, (Ix + d), L */
    case 0x8F:  /* RES 1, (Ix + d), A */
    case 0x90:  /* RES 2, (Ix + d), B */
    case 0x91:  /* RES 2, (Ix + d), C */
    case 0x92:  /* RES 2, (Ix + d), D */
    case 0x93:  /* RES 2, (Ix + d), E */
    case 0x94:  /* RES 2, (Ix + d), H */
    case 0x95:  /* RES 2, (Ix + d), L */
    case 0x97:  /* RES 2, (Ix + d), A */
    case 0x98:  /* RES 3, (Ix + d), B */
    case 0x99:  /* RES 3, (Ix + d), C */
    case 0x9A:  /* RES 3, (Ix + d), D */
    case 0x9B:  /* RES 3, (Ix + d), E */
    case 0x9C:  /* RES 3, (Ix + d), H */
    case 0x9D:  /* RES 3, (Ix + d), L */
    case 0x9F:  /* RES 3, (Ix + d), A */
    case 0xA0:  /* RES 4, (Ix + d), B */
    case 0xA1:  /* RES 4, (Ix + d), C */
    case 0xA2:  /* RES 4, (Ix + d), D */
    case 0xA3:  /* RES 4, (Ix + d), E */
    case 0xA4:  /* RES 4, (Ix + d), H */
    case 0xA5:  /* RES 4, (Ix + d), L */
    case 0xA7:  /* RES 4, (Ix + d), A */
    case 0xA8:  /* RES 5, (Ix + d), B */
    case 0xA9:  /* RES 5, (Ix + d), C */
    case 0xAA:  /* RES 5, (Ix + d), D */
    case 0xAB:  /* RES 5, (Ix + d), E */
    case 0xAC:  /* RES 5, (Ix + d), H */
    case 0xAD:  /* RES 5, (Ix + d), L */
    case 0xAF:  /* RES 5, (Ix + d), A */
    case 0xB0:  /* RES 6, (Ix + d), B */
    case 0xB1:  /* RES 6, (Ix + d), C */
    case 0xB2:  /* RES 6, (Ix + d), D */
    case 0xB3:  /* RES 6, (Ix + d), E */
    case 0xB4:  /* RES 6, (Ix + d), H */
    case 0xB5:  /* RES 6, (Ix + d), L */
    case 0xB7:  /* RES 6, (Ix + d), A */
    case 0xB8:  /* RES 7, (Ix + d), B */
    case 0xB9:  /* RES 7, (Ix + d), C */
    case 0xBA:  /* RES 7, (Ix + d), D */
    case 0xBB:  /* RES 7, (Ix + d), E */
    case 0xBC:  /* RES 7, (Ix + d), H */
    case 0xBD:  /* RES 7, (Ix + d), L */
    case 0xBF:  /* RES 7, (Ix + d), A */
        REG8(inst) = _value = _tmp & ~(1 << ((inst >> 3) & 0x07));
        cycles_done += 23;
        goto writeResult;

    case 0x86:  /* RES 0, (Ix + d) */
    case 0x8E:  /* RES 1, (Ix + d) */
    case 0x96:  /* RES 2, (Ix + d) */
    case 0x9E:  /* RES 3, (Ix + d) */
    case 0xA6:  /* RES 4, (Ix + d) */
    case 0xAE:  /* RES 5, (Ix + d) */
    case 0xB6:  /* RES 6, (Ix + d) */
    case 0xBE:  /* RES 7, (Ix + d) */
        _value = _tmp & ~(1 << ((inst >> 3) & 0x07));
        cycles_done += 23;
        goto writeResult;

    case 0xC0:  /* SET 0, (Ix + d), B */
    case 0xC1:  /* SET 0, (Ix + d), C */
    case 0xC2:  /* SET 0, (Ix + d), D */
    case 0xC3:  /* SET 0, (Ix + d), E */
    case 0xC4:  /* SET 0, (Ix + d), H */
    case 0xC5:  /* SET 0, (Ix + d), L */
    case 0xC7:  /* SET 0, (Ix + d), A */
    case 0xC8:  /* SET 1, (Ix + d), B */
    case 0xC9:  /* SET 1, (Ix + d), C */
    case 0xCA:  /* SET 1, (Ix + d), D */
    case 0xCB:  /* SET 1, (Ix + d), E */
    case 0xCC:  /* SET 1, (Ix + d), H */
    case 0xCD:  /* SET 1, (Ix + d), L */
    case 0xCF:  /* SET 1, (Ix + d), A */
    case 0xD0:  /* SET 2, (Ix + d), B */
    case 0xD1:  /* SET 2, (Ix + d), C */
    case 0xD2:  /* SET 2, (Ix + d), D */
    case 0xD3:  /* SET 2, (Ix + d), E */
    case 0xD4:  /* SET 2, (Ix + d), H */
    case 0xD5:  /* SET 2, (Ix + d), L */
    case 0xD7:  /* SET 2, (Ix + d), A */
    case 0xD8:  /* SET 3, (Ix + d), B */
    case 0xD9:  /* SET 3, (Ix + d), C */
    case 0xDA:  /* SET 3, (Ix + d), D */
    case 0xDB:  /* SET 3, (Ix + d), E */
    case 0xDC:  /* SET 3, (Ix + d), H */
    case 0xDD:  /* SET 3, (Ix + d), L */
    case 0xDF:  /* SET 3, (Ix + d), A */
    case 0xE0:  /* SET 4, (Ix + d), B */
    case 0xE1:  /* SET 4, (Ix + d), C */
    case 0xE2:  /* SET 4, (Ix + d), D */
    case 0xE3:  /* SET 4, (Ix + d), E */
    case 0xE4:  /* SET 4, (Ix + d), H */
    case 0xE5:  /* SET 4, (Ix + d), L */
    case 0xE7:  /* SET 4, (Ix + d), A */
    case 0xE8:  /* SET 5, (Ix + d), B */
    case 0xE9:  /* SET 5, (Ix + d), C */
    case 0xEA:  /* SET 5, (Ix + d), D */
    case 0xEB:  /* SET 5, (Ix + d), E */
    case 0xEC:  /* SET 5, (Ix + d), H */
    case 0xED:  /* SET 5, (Ix + d), L */
    case 0xEF:  /* SET 5, (Ix + d), A */
    case 0xF0:  /* SET 6, (Ix + d), B */
    case 0xF1:  /* SET 6, (Ix + d), C */
    case 0xF2:  /* SET 6, (Ix + d), D */
    case 0xF3:  /* SET 6, (Ix + d), E */
    case 0xF4:  /* SET 6, (Ix + d), H */
    case 0xF5:  /* SET 6, (Ix + d), L */
    case 0xF7:  /* SET 6, (Ix + d), A */
    case 0xF8:  /* SET 7, (Ix + d), B */
    case 0xF9:  /* SET 7, (Ix + d), C */
    case 0xFA:  /* SET 7, (Ix + d), D */
    case 0xFB:  /* SET 7, (Ix + d), E */
    case 0xFC:  /* SET 7, (Ix + d), H */
    case 0xFD:  /* SET 7, (Ix + d), L */
    case 0xFF:  /* SET 7, (Ix + d), A */
        REG8(inst) = _value = _tmp | (1 << ((inst >> 3) & 0x07));
        cycles_done += 23;
        goto writeResult;

    case 0xC6:  /* SET 0, (Ix + d) */
    case 0xCE:  /* SET 1, (Ix + d) */
    case 0xD6:  /* SET 2, (Ix + d) */
    case 0xDE:  /* SET 3, (Ix + d) */
    case 0xE6:  /* SET 4, (Ix + d) */
    case 0xEE:  /* SET 5, (Ix + d) */
    case 0xF6:  /* SET 6, (Ix + d) */
    case 0xFE:  /* SET 7, (Ix + d) */
        _value = _tmp | (1 << ((inst >> 3) & 0x07));
        cycles_done += 23;
        goto writeResult;
}

writeResult:
cpu->mwrite(cpu->offset->w + _disp, _value);
goto out;
