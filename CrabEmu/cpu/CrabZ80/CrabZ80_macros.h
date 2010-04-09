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

#define OP_EXSP(reg) {   \
    uint16 _tmp = (reg).w; \
    (reg).w = cpu->mread16(cpu->sp.w); \
    cpu->mwrite16(cpu->sp.w, _tmp); \
}

#define OP_PUSHAF() {   \
    cpu->mwrite(--cpu->sp.w, cpu->af.b.h); \
    cpu->mwrite(--cpu->sp.w, cpu->af.b.l); \
}

#define OP_POPAF()  {   \
    cpu->af.b.l = cpu->mread(cpu->sp.w++); \
    cpu->af.b.h = cpu->mread(cpu->sp.w++); \
}

#define OP_INC8(val)    {   \
    uint32 _tmp = (val) + 1; \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | \
        ((_tmp & 0x0F) ? 0x00 : 0x10) | \
        (((val) == 0x7F) ? 0x04: 0x00) | \
        (cpu->af.b.l & 0x01); \
    (val) = _tmp; \
}

#define OP_DEC8(val)    {   \
    uint32 _tmp = (val) - 1; \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | \
        ((val) == 0x80 ? 0x04 : 0x00) | \
        (cpu->af.b.l & 0x01) | \
        (((val) ^ _tmp) & 0x10) | \
        0x02; \
    (val) = _tmp; \
}

#define OP_ADC16(reg1, reg2)    {   \
    uint32 _tmp = (reg1) + (reg2) + (cpu->af.b.l & 0x01); \
    cpu->af.b.l = ((_tmp >> 8) & 0xA8) | (((uint16)_tmp) ? 0 : 0x40) | \
        ((((reg1) ^ _tmp ^ (reg2)) >> 8) & 0x10) | \
        ((((reg2) ^ (reg1) ^ 0x8000) & ((reg2) ^ _tmp) & 0x8000) >> 13) | \
        ((_tmp >> 16) & 0x01); \
    (reg1) = _tmp; \
}

#define OP_SBC16(reg1, reg2)    { \
    uint32 _tmp = (reg1) - (cpu->af.b.l & 0x01) - (reg2); \
    cpu->af.b.l = ((_tmp >> 8) & 0xA8) | (((uint16)_tmp) ? 0x00 : 0x40) | \
        ((((reg1) ^ _tmp ^ (reg2)) >> 8) & 0x10) | \
        (((((reg2) ^ (reg1)) & ((reg1) ^ _tmp)) >> 13) & 0x04) | 0x02 | \
        ((_tmp >> 16) & 0x01); \
    (reg1) = _tmp; \
}

#define OP_RLCA()   {   \
    cpu->af.b.h = (cpu->af.b.h << 1) | (cpu->af.b.h >> 7); \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | (cpu->af.b.h & 0x29); \
}

#define OP_RRCA()   {   \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | (cpu->af.b.h & 0x01); \
    cpu->af.b.h = (cpu->af.b.h >> 1) | (cpu->af.b.h << 7); \
    cpu->af.b.l |= (cpu->af.b.h & 0x28); \
}

#define OP_RLA()    {   \
    uint8 _tmp = (cpu->af.b.h << 1) | (cpu->af.b.l & 0x01); \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | (cpu->af.b.h >> 7) | (_tmp & 0x28); \
    cpu->af.b.h = _tmp; \
}

#define OP_RRA()    {   \
    uint8 _tmp = (cpu->af.b.h >> 1) | (cpu->af.b.l << 7); \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | (cpu->af.b.h & 0x01) | (_tmp & 0x28); \
    cpu->af.b.h = _tmp; \
}

#define OP_CPL()    {   \
    cpu->af.b.h ^= 0xFF; \
    cpu->af.b.l = (cpu->af.b.l & 0xC5) | 0x12 | (cpu->af.b.h & 0x28); \
}

#define OP_SCF()    {   \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | 0x01 | (cpu->af.b.h & 0x28); \
}

#define OP_CCF()    {   \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | \
    ((cpu->af.b.l & 0x01) ? 0x10 : 0x01) | (cpu->af.b.h & 0x28); \
}

#define OP_EX(reg1, reg2)   {   \
    uint16 _tmp = (reg1).w; \
    (reg1).w = (reg2).w; \
    (reg2).w = _tmp;  \
}

#define OP_EXX()    {   \
    OP_EX(cpu->bc, cpu->bcp); \
    OP_EX(cpu->de, cpu->dep); \
    OP_EX(cpu->hl, cpu->hlp); \
}

#define OP_HALT()   {   \
    --cpu->pc.w; \
    cpu->halt = 1; \
}

#define OP_RLC(reg) {   \
    uint8 _tmp = ((reg) << 1) | (((reg) & 0x80) >> 7); \
    cpu->af.b.l = ZSPXYtable[_tmp] | (((reg) & 0x80) >> 7); \
    (reg) = _tmp; \
}

#define OP_RRC(reg) {   \
    uint8 _tmp = ((reg) >> 1) | (((reg) & 0x01) << 7); \
    cpu->af.b.l = ZSPXYtable[_tmp] | ((reg) & 0x01); \
    (reg) = _tmp; \
}

#define OP_RRD()    {   \
    uint8 _byte = cpu->mread(cpu->hl.w); \
    uint8 _tmp = ((cpu->af.b.h & 0x0F) << 4) | ((_byte & 0xF0) >> 4); \
    cpu->af.b.h = (cpu->af.b.h & 0xF0) | (_byte & 0x0F); \
    cpu->af.b.l = ZSPXYtable[cpu->af.b.h] | (cpu->af.b.l & 0x01); \
    cpu->mwrite(cpu->hl.w, _tmp); \
}

#define OP_RLD()    {   \
    uint8 _byte = cpu->mread(cpu->hl.w); \
    uint8 _tmp = ((_byte & 0x0F) << 4) | (cpu->af.b.h & 0x0F); \
    cpu->af.b.h = (cpu->af.b.h & 0xF0) | ((_byte & 0xF0) >> 4); \
    cpu->af.b.l = ZSPXYtable[cpu->af.b.h] | (cpu->af.b.l & 0x01); \
    cpu->mwrite(cpu->hl.w, _tmp); \
}

#define OP_LDI()    {   \
    uint8 _tmp = cpu->mread(cpu->hl.w++); \
    cpu->mwrite(cpu->de.w++, _tmp); \
    --cpu->bc.w; \
    _tmp += cpu->af.b.h; \
    cpu->af.b.l = (cpu->af.b.l & 0xC1) | (cpu->bc.w ? 0x04 : 0x00) | \
        ((_tmp & 0x02) << 4) | (_tmp & 0x08); \
}

#define OP_LDD()    {   \
    uint8 _tmp = cpu->mread(cpu->hl.w--); \
    cpu->mwrite(cpu->de.w--, _tmp); \
    --cpu->bc.w; \
    _tmp += cpu->af.b.h; \
    cpu->af.b.l = (cpu->af.b.l & 0xC1) | (cpu->bc.w ? 0x04 : 0x00) | \
        ((_tmp & 0x02) << 4) | (_tmp & 0x08); \
}

#define OP_CPI()    {   \
    uint8 _byte = cpu->mread(cpu->hl.w++); \
    uint32 _tmp = cpu->af.b.h - _byte; \
    --cpu->bc.w; \
    cpu->af.b.l = ZStable[_tmp & 0xFF] | ((cpu->af.b.h ^ _tmp ^ _byte) & 0x10) | \
        (cpu->bc.w ? 0x04 : 0x00) | 0x02 | (cpu->af.b.l & 0x01); \
    _tmp -= (cpu->af.b.l & 0x10) ? 1 : 0; \
    cpu->af.b.l |= ((_tmp & 0x02) << 4) | (_tmp & 0x08); \
}

#define OP_CPIR()   {   \
    uint8 _byte = cpu->mread(cpu->hl.w++); \
    uint32 _tmp = cpu->af.b.h - _byte; \
    --cpu->bc.w; \
    cpu->af.b.l = ZStable[_tmp & 0xFF] | ((_byte ^ cpu->af.b.h ^ _tmp) & 0x10) | \
        (cpu->bc.w ? 0x04 : 0x00) | 0x02 | (cpu->af.b.l & 0x01); \
    _tmp -= (cpu->af.b.l & 0x10) ? 1 : 0; \
    cpu->af.b.l |= ((_tmp & 0x02) << 4) | (_tmp & 0x08); \
    if(cpu->bc.w && cpu->af.b.h - _byte)    { \
        cpu->pc.w -= 2; \
        cycles_done += 5; \
    } \
}

#define OP_CPD()    {   \
    uint8 _byte = cpu->mread(cpu->hl.w--); \
    uint32 _tmp = cpu->af.b.h - _byte; \
    --cpu->bc.w; \
    cpu->af.b.l = ZStable[_tmp & 0xFF] | ((_byte ^ cpu->af.b.h ^ _tmp) & 0x10) | \
        (cpu->bc.w ? 0x04 : 0x00) | 0x02 | (cpu->af.b.l & 0x01); \
    _tmp -= (cpu->af.b.l & 0x10) ? 1 : 0; \
    cpu->af.b.l |= ((_tmp & 0x02) << 4) | (_tmp & 0x08); \
}

#define OP_CPDR()   {   \
    uint8 _byte = cpu->mread(cpu->hl.w--); \
    uint32 _tmp = cpu->af.b.h - _byte; \
    --cpu->bc.w; \
    cpu->af.b.l = ZStable[_tmp & 0xFF] | ((_byte ^ cpu->af.b.h ^ _tmp) & 0x10) | \
        (cpu->bc.w ? 0x04 : 0x00) | 0x02 | (cpu->af.b.l & 0x01); \
    _tmp -= (cpu->af.b.l & 0x10) ? 1 : 0; \
    cpu->af.b.l |= ((_tmp & 0x02) << 4) | (_tmp & 0x08); \
    if(cpu->bc.w && _tmp)	{ \
        cpu->pc.w -= 2; \
        cycles_done += 5; \
    } \
}

#define OP_INI()    {   \
    uint8 _byte = cpu->pread(cpu->bc.b.l); \
    cpu->mwrite(cpu->hl.w++, _byte); \
    --cpu->bc.b.h; \
    cpu->af.b.l = ZSXYtable[cpu->bc.b.h] | \
        ((_byte + ((cpu->bc.b.l + 1) & 0xFF)) > 0xFF ? 0x11 : 0x00) | \
        (ZSPXYtable[((_byte + ((cpu->bc.b.l + 1) & 0xFF)) & 0x07) ^ cpu->bc.b.h] & 0x04) | \
        ((_byte >> 6) & 0x02); \
}

#define OP_IND()    {   \
    uint8 _byte = cpu->pread(cpu->bc.b.l); \
    cpu->mwrite(cpu->hl.w--, _byte); \
    --cpu->bc.b.h; \
    cpu->af.b.l = ZSXYtable[cpu->bc.b.h] | \
        ((_byte + ((cpu->bc.b.l - 1) & 0xFF)) > 0xFF ? 0x11 : 0x00) | \
        (ZSPXYtable[((_byte + ((cpu->bc.b.l - 1) & 0xFF)) & 0x07) ^ cpu->bc.b.h] & 0x04) | \
        ((_byte >> 6) & 0x02); \
}

#define OP_OUTI()   {   \
    uint8 _byte = cpu->mread(cpu->hl.w++); \
    --cpu->bc.b.h; \
    cpu->pwrite(cpu->bc.b.l, _byte); \
    cpu->af.b.l = ZSXYtable[cpu->bc.b.h] | \
        ((cpu->hl.b.l + _byte) > 0xFF ? 0x11 : 0x00) | \
        (ZSPXYtable[((cpu->hl.b.l + _byte) & 0x07) ^ cpu->bc.b.h] & 0x04) | \
        ((_byte >> 6) & 0x02); \
}

#define OP_OUTD()   {   \
    uint8 _byte = cpu->mread(cpu->hl.w--); \
    --cpu->bc.b.h; \
    cpu->pwrite(cpu->bc.b.l, _byte); \
    cpu->af.b.l = ZSXYtable[cpu->bc.b.h] | \
        ((cpu->hl.b.l + _byte) > 0xFF ? 0x11 : 0x00) | \
        (ZSPXYtable[((cpu->hl.b.l + _byte) & 0x07) ^ cpu->bc.b.h] & 0x04) | \
        ((_byte >> 6) & 0x02); \
}

#define OP_NEG()    {   \
    uint32 _tmp = 0 - cpu->af.b.h; \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | ((_tmp ^ cpu->af.b.h) & 0x10) | \
        ((cpu->af.b.h & _tmp & 0x80) >> 5) | 0x02 | \
        ((_tmp >> 8) & 0x01); \
    cpu->af.b.h = _tmp; \
}

#define OP_ADD()    {   \
    uint32 _tmp = cpu->af.b.h + _value; \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | \
        ((cpu->af.b.h ^ _tmp ^ _value) & 0x10) | \
        (((_value ^ cpu->af.b.h ^ 0x80) & (_value ^ _tmp) & 0x80) >> 5) | \
        ((_tmp >> 8) & 0x01); \
    cpu->af.b.h = _tmp;  \
}

#define OP_ADC()    {   \
    uint32 _tmp = cpu->af.b.h + _value + (cpu->af.b.l & 0x01); \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | \
        ((cpu->af.b.h ^ _tmp ^ _value) & 0x10) | \
        (((_value ^ cpu->af.b.h ^ 0x80) & (_value ^ _tmp) & 0x80) >> 5) | \
        ((_tmp >> 8) & 0x01); \
    cpu->af.b.h = _tmp; \
}

#define OP_SUB()    {   \
    uint32 _tmp = cpu->af.b.h - _value; \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | \
        ((cpu->af.b.h ^ _tmp ^ _value) & 0x10) | \
        (((_value ^ cpu->af.b.h) & (cpu->af.b.h ^ _tmp) & 0x80) >> 5) | \
        0x02 | ((_tmp >> 8) & 0x01); \
    cpu->af.b.h = _tmp; \
}

#define OP_SBC()    {   \
    uint32 _tmp = cpu->af.b.h - (cpu->af.b.l & 0x01) - _value; \
    cpu->af.b.l = ZSXYtable[_tmp & 0xFF] | \
        ((cpu->af.b.h ^ _tmp ^ _value) & 0x10) | \
        (((_value ^ cpu->af.b.h) & (cpu->af.b.h ^ _tmp) & 0x80) >> 5) | \
        0x02 | ((_tmp >> 8) & 0x01); \
    cpu->af.b.h = _tmp; \
}

#define OP_AND()    {   \
    uint32 _tmp = cpu->af.b.h & _value; \
    cpu->af.b.l = ZSPXYtable[_tmp] | 0x10; \
    cpu->af.b.h = _tmp; \
}

#define OP_XOR()    {   \
    uint32 _tmp = cpu->af.b.h ^ _value; \
    cpu->af.b.l = ZSPXYtable[_tmp]; \
    cpu->af.b.h = _tmp; \
}

#define OP_OR() {   \
    uint32 _tmp = cpu->af.b.h | _value; \
    cpu->af.b.l = ZSPXYtable[_tmp]; \
    cpu->af.b.h = _tmp; \
}

#define OP_CP() {   \
    uint32 _tmp = cpu->af.b.h - _value; \
    cpu->af.b.l = ZStable[_tmp & 0xFF] | \
        ((cpu->af.b.h ^ _tmp ^ _value) & 0x10) | \
        (_value & 0x28) | \
        (((_value ^ cpu->af.b.h) & (cpu->af.b.h ^ _tmp) & 0x80) >> 5) | \
        0x02 | ((_tmp >> 8) & 0x01); \
}

#define OP_INCR(s)  {   \
    ++REG8(s); \
    cpu->af.b.l = ZSXYtable[REG8(s)] | \
        ((REG8(s) & 0x0F) ? 0x00 : 0x10) | \
        (((REG8(s)) == 0x80) ? 0x04: 0x00) | \
        (cpu->af.b.l & 0x01); \
}

#define OP_ADDHL()  {   \
    uint32 _tmp = cpu->hl.w + _value; \
    cpu->af.b.l = (cpu->af.b.l & 0xC4) | \
        (((cpu->hl.w ^ _tmp ^ _value) >> 8) & 0x10) | \
        ((_tmp >> 8) & 0x28) | \
        ((_tmp >> 16) & 0x01); \
    cpu->hl.w = _tmp; \
}

#define OP_ADDIx()  {   \
    uint32 _tmp = cpu->offset->w + _value; \
        cpu->af.b.l = (cpu->af.b.l & 0xC4) | \
        (((cpu->offset->w ^ _tmp ^ _value) >> 8) & 0x10) | \
        ((_tmp >> 8) & 0x28) | \
        ((_tmp >> 16) & 0x01); \
    cpu->offset->w = _tmp; \
}
