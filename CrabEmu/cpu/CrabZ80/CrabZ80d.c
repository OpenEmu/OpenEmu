/*
    This file is part of CrabEmu.

    Copyright (C) 2006 Lawrence Sebald

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

#include <stdio.h>
#include <string.h>
#include "CrabZ80d.h"

static const char regs16[4][3] = { "BC", "DE", "HL", "SP" };
static const char regs8[8][5] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static const char cond[8][3] = { "NZ", "Z", "NC", "C", "PO", "PE", "P", "M" };

static uint16 disasm_cb(char str[], CrabZ80_t *cpu, uint16 addr)    {
    uint8 opcode = cpu->mread(addr++);

    if(opcode < 0x08)   {
        sprintf(str, "RLC %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x10)  {
        sprintf(str, "RRC %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x18)  {
        sprintf(str, "RL %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x20)  {
        sprintf(str, "RR %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x28)  {
        sprintf(str, "SLA %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x30)  {
        sprintf(str, "SRA %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x38)  {
        sprintf(str, "SLA %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x40)  {
        sprintf(str, "SRL %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x80)  {
        sprintf(str, "BIT %d, %s", (opcode & 0x38) >> 3, regs8[opcode & 0x07]);
    }
    else if(opcode < 0xC0)  {
        sprintf(str, "RES %d, %s", (opcode & 0x38) >> 3, regs8[opcode & 0x07]);
    }
    else    {
        sprintf(str, "SET %d, %s", (opcode & 0x38) >> 3, regs8[opcode & 0x07]);
    }

    return addr;
}

static uint16 disasm_indexcb(char str[], CrabZ80_t *cpu, uint16 addr,
                             uint8 prefix) {
    char reg[3];
    uint8 offset = cpu->mread(addr++);
    uint8 opcode = cpu->mread(addr++);

    if(prefix == 0xDD)
        strcpy(reg, "IX");
    else
        strcpy(reg, "IY");

    if(opcode == 0x06)   {
        sprintf(str, "RLC (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x0E)  {
        sprintf(str, "RRC (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x16)  {
        sprintf(str, "RL (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x1E)  {
        sprintf(str, "RR (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x26)  {
        sprintf(str, "SLA (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x2E)  {
        sprintf(str, "SRA (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x36)  {
        sprintf(str, "SLA (%s + 0x%02X)", reg, offset);
    }
    else if(opcode == 0x3E)  {
        sprintf(str, "SRL (%s + 0x%02X)", reg, offset);
    }
    else if(opcode < 0x08)   {
        sprintf(str, "RLC (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x10)  {
        sprintf(str, "RRC (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x18)  {
        sprintf(str, "RL (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x20)  {
        sprintf(str, "RR (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x28)  {
        sprintf(str, "SLA (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x30)  {
        sprintf(str, "SRA (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x38)  {
        sprintf(str, "SLA (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x40)  {
        sprintf(str, "SRL (%s + 0x%02X), %s", reg, offset,
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x80)  {
        sprintf(str, "BIT %d, (%s + 0x%02X)", (opcode & 0x38) >> 3, reg,
                offset);
    }
    else if(opcode < 0xC0)  {
        sprintf(str, "RES %d, (%s + 0x%02X), %s", (opcode & 0x38) >> 3, reg,
                offset, regs8[opcode & 0x07]);
    }
    else    {
        sprintf(str, "SET %d, (%s + 0x%02X), %s", (opcode & 0x38) >> 3, reg,
                offset, regs8[opcode & 0x07]);
    }

    return addr;
}

static uint16 disasm_index(char str[], CrabZ80_t *cpu, uint16 addr,
                           uint8 prefix)    {
    char reg[3];
    uint8 opcode = cpu->mread(addr++);

    if(prefix == 0xDD)
        strcpy(reg, "IX");
    else
        strcpy(reg, "IY");

    if(opcode < 0x40)   {
        switch(opcode)  {
            case 0x09:
                sprintf(str, "ADD %s, BC", reg);
                break;

            case 0x19:
                sprintf(str, "ADD %s, DE", reg);
                break;

            case 0x21:
                sprintf(str, "LD %s, 0x%04X", reg,
                        cpu->mread(addr) | (cpu->mread(addr + 1) << 8));
                addr += 2;
                break;

            case 0x22:
                sprintf(str, "LD (0x%04X), %s",
                        cpu->mread(addr) | (cpu->mread(addr + 1) << 8), reg);
                addr += 2;
                break;

            case 0x23:
                sprintf(str, "INC %s", reg);
                break;

            case 0x24:
                sprintf(str, "INC %sh", reg);
                break;

            case 0x25:
                sprintf(str, "DEC %sh", reg);
                break;

            case 0x26:
                sprintf(str, "LD %sh, 0x%02X", reg, cpu->mread(addr++));
                break;

            case 0x29:
                sprintf(str, "ADD %s, %s", reg, reg);
                break;

            case 0x2A:
                sprintf(str, "LD %s, (0x%04X)",
                        reg, cpu->mread(addr) | (cpu->mread(addr + 1) << 8));
                addr += 2;
                break;

            case 0x2B:
                sprintf(str, "DEC %s", reg);
                break;

            case 0x2C:
                sprintf(str, "INC %sl", reg);
                break;

            case 0x2D:
                sprintf(str, "DEC %sl", reg);
                break;

            case 0x2E:
                sprintf(str, "LD %sl, 0x%02X", reg, cpu->mread(addr++));
                break;

            case 0x34:
                sprintf(str, "INC (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0x35:
                sprintf(str, "DEC (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0x36:
                sprintf(str, "LD (%s + 0x%02X), 0x%02X", reg, cpu->mread(addr), 
                        cpu->mread(addr + 1));
                addr += 2;
                break;

            case 0x39:
                sprintf(str, "ADD %s, SP", reg);
                break;

            default:
                addr = CrabZ80_disassemble(str, cpu, addr);
        }
    }
    else if(opcode < 0x60)  {
        switch(opcode & 0x07)   {
            case 0x04:
                sprintf(str, "LD %s, %sh", regs8[(opcode & 0x38) >> 3], reg);
                break;

            case 0x05:
                sprintf(str, "LD %s, %sl", regs8[(opcode & 0x38) >> 3], reg);
                break;

            case 0x06:
                sprintf(str, "LD %s, (%s + 0x%02X)",
                        regs8[(opcode & 0x38) >> 3], reg, cpu->mread(addr++));
                break;

            default:
                addr = CrabZ80_disassemble(str, cpu, addr);
        }
    }
    else if(opcode < 0x64 || opcode == 0x67)  {
        sprintf(str, "LD %sh, %s", reg, regs8[opcode & 0x07]);
    }
    else if(opcode == 0x64) {
        sprintf(str, "LD %sh, %sh", reg, reg);
    }
    else if(opcode == 0x65) {
        sprintf(str, "LD %sh, %sl", reg, reg);
    }
    else if(opcode == 0x66) {
        sprintf(str, "LD H, (%s + 0x%02X)", reg, cpu->mread(addr++));
    }
    else if(opcode < 0x6C || opcode == 0x6F)  {
        sprintf(str, "LD %sl, %s", reg, regs8[opcode & 0x07]);
    }
    else if(opcode == 0x6C) {
        sprintf(str, "LD %sl, %sh", reg, reg);
    }
    else if(opcode == 0x6D) {
        sprintf(str, "LD %sl, %sl", reg, reg);
    }
    else if(opcode == 0x6E) {
        sprintf(str, "LD L, (%s + 0x%02X)", reg, cpu->mread(addr++));
    }
    else if(opcode == 0x76) {
        addr = CrabZ80_disassemble(str, cpu, addr);
    }
    else if(opcode < 0x78)  {
        sprintf(str, "LD (%s + 0x%02X), %s", reg, cpu->mread(addr++),
                regs8[opcode & 0x07]);
    }
    else    {
        switch(opcode)  {
            case 0x7C:
                sprintf(str, "LD A, %sh", reg);
                break;

            case 0x7D:
                sprintf(str, "LD A, %sl", reg);
                break;

            case 0x7E:
                sprintf(str, "LD A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0x84:
                sprintf(str, "ADD A, %sh", reg);
                break;

            case 0x85:
                sprintf(str, "ADD A, %sl", reg);
                break;

            case 0x86:
                sprintf(str, "ADD A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0x8C:
                sprintf(str, "ADC A, %sh", reg);
                break;

            case 0x8D:
                sprintf(str, "ADC A, %sl", reg);
                break;

            case 0x8E:
                sprintf(str, "ADC A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0x94:
                sprintf(str, "SUB A, %sh", reg);
                break;

            case 0x95:
                sprintf(str, "SUB A, %sl", reg);
                break;

            case 0x96:
                sprintf(str, "SUB A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0x9C:
                sprintf(str, "SBC A, %sh", reg);
                break;

            case 0x9D:
                sprintf(str, "SBC A, %sl", reg);
                break;

            case 0x9E:
                sprintf(str, "SBC A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0xA4:
                sprintf(str, "AND A, %sh", reg);
                break;

            case 0xA5:
                sprintf(str, "AND A, %sl", reg);
                break;

            case 0xA6:
                sprintf(str, "AND A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0xAC:
                sprintf(str, "XOR A, %sh", reg);
                break;

            case 0xAD:
                sprintf(str, "XOR A, %sl", reg);
                break;

            case 0xAE:
                sprintf(str, "XOR A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0xB4:
                sprintf(str, "OR A, %sh", reg);
                break;

            case 0xB5:
                sprintf(str, "OR A, %sl", reg);
                break;

            case 0xB6:
                sprintf(str, "OR A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0xBC:
                sprintf(str, "CP A, %sh", reg);
                break;

            case 0xBD:
                sprintf(str, "CP A, %sl", reg);
                break;

            case 0xBE:
                sprintf(str, "CP A, (%s + 0x%02X)", reg, cpu->mread(addr++));
                break;

            case 0xCB:
                addr = disasm_indexcb(str, cpu, addr, prefix);
                break;

            case 0xE1:
                sprintf(str, "POP %s", reg);
                break;

            case 0xE3:
                sprintf(str, "EX (SP), %s", reg);
                break;

            case 0xE5:
                sprintf(str, "PUSH %s", reg);
                break;

            case 0xE9:
                sprintf(str, "JP (%s)", reg);
                break;

            case 0xF9:
                sprintf(str, "LD SP, %s", reg);
                break;

            default:
                addr = CrabZ80_disassemble(str, cpu, addr);
        }
    }

    return addr;
}

static uint16 disasm_ed(char str[], CrabZ80_t *cpu, uint16 addr)    {
    uint8 opcode = cpu->mread(addr++);

    if(opcode < 0x40 || opcode == 0x77 || opcode == 0x7F)   {
        sprintf(str, "NOP [0xED%02X]", opcode);
    }
    else if(opcode == 0x47) {
        sprintf(str, "LD I, A");
    }
    else if(opcode == 0x4F) {
        sprintf(str, "LD R, A");
    }
    else if(opcode == 0x57) {
        sprintf(str, "LD A, I");
    }
    else if(opcode == 0x5F) {
        sprintf(str, "LD A, R");
    }
    else if(opcode == 0x67) {
        sprintf(str, "RRD");
    }
    else if(opcode == 0x6F) {
        sprintf(str, "RLD");
    }
    else if(opcode < 0x80)  {
        switch(opcode & 0x0F)   {
            case 0x00:
            case 0x08:
                if(opcode != 0x70)
                    sprintf(str, "IN %s, (C)", regs8[(opcode & 0x38) >> 3]);
                else
                    sprintf(str, "IN (C)");
                break;

            case 0x01:
            case 0x09:
                if(opcode != 0x71)
                    sprintf(str, "OUT (C), %s", regs8[(opcode & 0x38) >> 3]);
                else
                    sprintf(str, "OUT (C), 0x00");
                break;

            case 0x02:
                sprintf(str, "SBC HL, %s", regs16[(opcode & 0x30) >> 4]);
                break;

            case 0x03:
                sprintf(str, "LD (0x%04X), %s", cpu->mread(addr) |
                        (cpu->mread(addr + 1) << 8),
                        regs16[(opcode & 0x30) >> 4]);
                addr += 2;
                break;

            case 0x04:
            case 0x0C:
                sprintf(str, "NEG");
                break;

            case 0x05:
            case 0x0D:
                if(opcode != 0x4D)
                    sprintf(str, "RETN");
                else
                    sprintf(str, "RETI");
                break;

            case 0x06:
                if((opcode & 0xF0) == 0x40 || (opcode & 0xF0) == 0x60)
                    sprintf(str, "IM 0");
                else
                    sprintf(str, "IM 1");
                break;

            case 0x0A:
                sprintf(str, "ADC HL, %s", regs16[(opcode & 0x30) >> 4]);
                break;

            case 0x0B:
                sprintf(str, "LD %s, (0x%04X)", regs16[(opcode & 0x30) >> 4],
                        cpu->mread(addr) | (cpu->mread(addr + 1) << 8));
                addr += 2;
                break;

            case 0x0E:
                if((opcode & 0xF0) == 0x40 || (opcode & 0xF0) == 0x60)
                    sprintf(str, "IM 0");
                else
                    sprintf(str, "IM 2");
                break;
        }
    }
    else if(opcode == 0xA0) {
        sprintf(str, "LDI");
    }
    else if(opcode == 0xA1) {
        sprintf(str, "CPI");
    }
    else if(opcode == 0xA2) {
        sprintf(str, "INI");
    }
    else if(opcode == 0xA3) {
        sprintf(str, "OUTI");
    }
    else if(opcode == 0xA8) {
        sprintf(str, "LDD");
    }
    else if(opcode == 0xA9) {
        sprintf(str, "CPD");
    }
    else if(opcode == 0xAA) {
        sprintf(str, "IND");
    }
    else if(opcode == 0xAB) {
        sprintf(str, "OUTD");
    }
    else if(opcode == 0xB0) {
        sprintf(str, "LDIR");
    }
    else if(opcode == 0xB1) {
        sprintf(str, "CPIR");
    }
    else if(opcode == 0xB2) {
        sprintf(str, "INIR");
    }
    else if(opcode == 0xB3) {
        sprintf(str, "OTIR");
    }
    else if(opcode == 0xB8) {
        sprintf(str, "LDDR");
    }
    else if(opcode == 0xB9) {
        sprintf(str, "CPDR");
    }
    else if(opcode == 0xBA) {
        sprintf(str, "INDR");
    }
    else if(opcode == 0xBB) {
        sprintf(str, "OTDR");
    }
    else    {
        sprintf(str, "NOP [0xED%02X]", opcode);
    }

    return addr;
}

uint16 CrabZ80_disassemble(char str[], CrabZ80_t *cpu, uint16 addr)   {
    uint8 opcode = cpu->mread(addr++);

    if(opcode < 0x40)   {
        switch(opcode & 0x07)   {
            case 0x00:
            {
                switch(opcode)   {
                    case 0x00:
                        sprintf(str, "NOP");
                        break;

                    case 0x08:
                        sprintf(str, "EX AF, AF'");
                        break;

                    case 0x10:
                        sprintf(str, "DJNZ (PC + 0x%02X)", cpu->mread(addr++));
                        break;

                    case 0x18:
                        sprintf(str, "JR 0x%02X", cpu->mread(addr++));
                        break;

                    case 0x20:
                        sprintf(str, "JR NZ, 0x%02X", cpu->mread(addr++));
                        break;

                    case 0x28:
                        sprintf(str, "JR Z, 0x%02X", cpu->mread(addr++));
                        break;

                    case 0x30:
                        sprintf(str, "JR NC, 0x%02X", cpu->mread(addr++));
                        break;

                    case 0x38:
                        sprintf(str, "JR C, 0x%02X", cpu->mread(addr++));
                        break;
                }
                break;
            }

            case 0x01:
            {
                if(!(opcode & 0x08))    {
                    sprintf(str, "LD %s, 0x%04X", regs16[(opcode & 0xF0) >> 4],
                            cpu->mread(addr) | (cpu->mread(addr + 1) << 8));
                    addr += 2;
                }
                else    {
                    sprintf(str, "ADD HL, %s", regs16[(opcode & 0xF0) >> 4]);
                    break;
                }
                break;
            }

            case 0x02:
            {
                if(!(opcode & 0x08))    {
                    switch(opcode & 0xF0)   {
                        case 0x00:
                        case 0x10:
                            sprintf(str, "LD (%s), A",
                                    regs16[(opcode & 0xF0) >> 4]);
                            break;

                        case 0x20:
                            sprintf(str, "LD (0x%04X), HL",
                                    cpu->mread(addr) |
                                    (cpu->mread(addr + 1) << 8));
                            addr += 2;
                            break;

                        case 0x30:
                            sprintf(str, "LD (0x%04X), A",
                                    cpu->mread(addr) |
                                    (cpu->mread(addr + 1) << 8));
                            addr += 2;
                            break;
                    }
                }
                else    {
                    switch(opcode & 0xF0)    {
                        case 0x00:
                        case 0x10:
                            sprintf(str, "LD A, (%s)",
                                    regs16[(opcode & 0xF0) >> 4]);
                            break;

                        case 0x20:
                            sprintf(str, "LD HL, (0x%04X)",
                                    cpu->mread(addr) |
                                    (cpu->mread(addr + 1) << 8));
                            addr += 2;
                            break;

                        case 0x30:
                            sprintf(str, "LD A, (0x%04X)",
                                    cpu->mread(addr) |
                                    (cpu->mread(addr + 1) << 8));
                            addr += 2;
                            break;
                    }
                }
                break;
            }

            case 0x03:
            {
                if(!(opcode & 0x08))    {
                    sprintf(str, "INC %s", regs16[(opcode & 0xF0) >> 4]);
                }
                else    {
                    sprintf(str, "DEC %s", regs16[(opcode & 0xF0) >> 4]);
                }
                break;
            }

            case 0x04:
            {
                sprintf(str, "INC %s", regs8[(opcode & 0xF8) >> 3]);
                break;
            }

            case 0x05:
            {
                sprintf(str, "DEC %s", regs8[(opcode & 0xF8) >> 3]);
                break;
            }

            case 0x06:
            {
                sprintf(str, "LD %s, 0x%02X", regs8[(opcode & 0xF8) >> 3],
                        cpu->mread(addr++));
                break;
            }

            case 0x07:
            {
                switch(opcode)  {
                    case 0x07:
                        sprintf(str, "RLCA");
                        break;

                    case 0x0F:
                        sprintf(str, "RRCA");
                        break;

                    case 0x17:
                        sprintf(str, "RLA");
                        break;

                    case 0x1F:
                        sprintf(str, "RRA");
                        break;

                    case 0x27:
                        sprintf(str, "DAA");
                        break;

                    case 0x2F:
                        sprintf(str, "CPL");
                        break;

                    case 0x37:
                        sprintf(str, "SCF");
                        break;

                    case 0x3F:
                        sprintf(str, "CCF");
                        break;
                }
                break;
            }
        }
    }
    else if(opcode == 0x76) {
        sprintf(str, "HALT");
    }
    else if(opcode < 0x80)  {
        sprintf(str, "LD %s, %s", regs8[(opcode & 0x38) >> 3],
                regs8[opcode & 0x07]);
    }
    else if(opcode < 0x88)  {
        sprintf(str, "ADD A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x90)  {
        sprintf(str, "ADC A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0x98)  {
        sprintf(str, "SUB A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0xA0)  {
        sprintf(str, "SBC A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0xA8)  {
        sprintf(str, "AND A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0xB0)  {
        sprintf(str, "XOR A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0xB8)  {
        sprintf(str, "OR A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode < 0xC0)  {
        sprintf(str, "CP A, %s", regs8[opcode & 0x07]);
    }
    else if(opcode == 0xC3) {
        sprintf(str, "JP 0x%04X", cpu->mread(addr) |
                (cpu->mread(addr + 1) << 8));
        addr += 2;
    }
    else if(opcode == 0xC6) {
        sprintf(str, "ADD A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xC9) {
        sprintf(str, "RET");
    }
    else if(opcode == 0xCB) {
        addr = disasm_cb(str, cpu, addr);
    }
    else if(opcode == 0xCD) {
        sprintf(str, "CALL 0x%04X", cpu->mread(addr) |
                (cpu->mread(addr + 1) << 8));
        addr += 2;
    }
    else if(opcode == 0xCE) {
        sprintf(str, "ADC A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xD3) {
        sprintf(str, "OUT (0x%02X), A", cpu->mread(addr++));
    }
    else if(opcode == 0xD6) {
        sprintf(str, "SUB A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xD9) {
        sprintf(str, "EXX");
    }
    else if(opcode == 0xDB) {
        sprintf(str, "IN A, (0x%02X)", cpu->mread(addr++));
    }
    else if(opcode == 0xDD) {
        addr = disasm_index(str, cpu, addr, 0xDD);
    }
    else if(opcode == 0xDE) {
        sprintf(str, "SBC A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xE3) {
        sprintf(str, "EX (SP), HL");
    }
    else if(opcode == 0xE6) {
        sprintf(str, "AND A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xE9) {
        sprintf(str, "JP (HL)");
    }
    else if(opcode == 0xEB) {
        sprintf(str, "EX DE, HL");
    }
    else if(opcode == 0xED) {
        addr = disasm_ed(str, cpu, addr);
    }
    else if(opcode == 0xEE) {
        sprintf(str, "XOR A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xF3) {
        sprintf(str, "DI");
    }
    else if(opcode == 0xF6) {
        sprintf(str, "OR A, 0x%02X", cpu->mread(addr++));
    }
    else if(opcode == 0xF9) {
        sprintf(str, "LD SP, HL");
    }
    else if(opcode == 0xFB) {
        sprintf(str, "EI");
    }
    else if(opcode == 0xFD) {
        addr = disasm_index(str, cpu, addr, 0xFD);
    }
    else if(opcode == 0xFE) {
        sprintf(str, "CP A, 0x%02X", cpu->mread(addr++));
    }
    else    {
        switch(opcode & 0x0F)   {
            case 0x00:
            case 0x08:
                sprintf(str, "RET %s", cond[(opcode & 0x38) >> 3]);
                break;

            case 0x01:
                sprintf(str, "POP %s", regs16[(opcode & 0x30) >> 4]);
                break;

            case 0x02:
            case 0x0A:
                sprintf(str, "JP %s, 0x%04X", cond[(opcode & 0x38) >> 3],
                        cpu->mread(addr) | (cpu->mread(addr + 1) << 8));
                addr += 2;
                break;

            case 0x04:
            case 0x0C:
                sprintf(str, "CALL %s, 0x%04X", cond[(opcode & 0x38) >> 3],
                        cpu->mread(addr) | (cpu->mread(addr + 1) << 8));
                addr += 2;
                break;

            case 0x05:
                sprintf(str, "PUSH %s", regs16[(opcode & 0x30) >> 4]);
                break;

            case 0x07:
            case 0x0F:
                sprintf(str, "RST 0x%02X", opcode & 0x38);
                break;
        }
    }

    return addr;
}
