/*  Copyright 2003-2004 Stephane Dallongeville

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*********************************************************************************
 *
 * C68K (68000 CPU emulator) version 0.80
 * Compiled with Dev-C++
 * Copyright 2003-2004 Stephane Dallongeville
 *
 ********************************************************************************/

/* Modified heavily for usage in Mednafen:

    Made opcode fetches go through the external memory read function.
    Added A-line and F-line emulation.
    Added packed save state code.
    Gutted the timing code.
    Scrapped the goto table code.
    Redid the C/X flag calculation on some 32-bit instructions.

    Other stuff I can't remember.

  If you want a more-or-less original version of C68k(which, in its original state, can be quite fast),
  check out Yabause's code(though I think my A-line and F-line emulation improvements are in that tree now too).
*/

#include <stdio.h>
#include <string.h>

#include "c68k.h"

// shared global variable
//////////////////////////

c68k_struc C68K;

// include macro file
//////////////////////

#include "c68kmac.inc"

// prototype
/////////////

u8 C68K_FASTCALL C68k_Read8_Dummy(const u32 adr);
u16 C68K_FASTCALL C68k_Read16_Dummy(const u32 adr);
void C68K_FASTCALL C68k_Write8_Dummy(const u32 adr, u8 data);
void C68K_FASTCALL C68k_Write16_Dummy(const u32 adr, u16 data);

u32 C68k_Read_Byte(c68k_struc *cpu, u32 adr);
u32 C68k_Read_Word(c68k_struc *cpu, u32 adr);
u32 C68k_Read_Long(c68k_struc *cpu, u32 adr);
void C68k_Write_Byte(c68k_struc *cpu, u32 adr, u32 data);
void C68k_Write_Word(c68k_struc *cpu, u32 adr, u32 data);
void C68k_Write_Long(c68k_struc *cpu, u32 adr, u32 data);

s32  C68K_FASTCALL C68k_Interrupt_Ack_Dummy(s32 level);
void C68K_FASTCALL C68k_Reset_Dummy(void);

// core main functions
///////////////////////

void C68k_Init(c68k_struc *cpu, C68K_INT_CALLBACK *int_cb)
{
    memset(cpu, 0, sizeof(c68k_struc));

    C68k_Set_ReadB(cpu, C68k_Read8_Dummy);
    C68k_Set_ReadW(cpu, C68k_Read16_Dummy);

    C68k_Set_WriteB(cpu, C68k_Write8_Dummy);
    C68k_Set_WriteW(cpu, C68k_Write16_Dummy);

    C68k_Set_TAS_Hack(cpu, 0);

    //C68k_Set_Debug(cpu, NULL);

    if (int_cb) cpu->Interrupt_CallBack = int_cb;
    else cpu->Interrupt_CallBack = C68k_Interrupt_Ack_Dummy;
    cpu->Reset_CallBack = C68k_Reset_Dummy;
}

s32 C68K_FASTCALL C68k_Reset(c68k_struc *cpu)
{
    //memset(cpu, 0, (u32)(&(cpu->dirty1)) - (u32)(&(cpu->D[0])));
    memset(cpu, 0, ((u8 *)&(cpu->dirty1)) - ((u8 *)&(cpu->D[0])));
    
    cpu->flag_I = 7;
    cpu->flag_S = C68K_SR_S;

    cpu->A[7] = C68k_Read_Long(cpu, 0);
    C68k_Set_PC(cpu, C68k_Read_Long(cpu, 4));

    return cpu->Status;
}

void C68k_Set_TAS_Hack(c68k_struc *cpu, int value)
{
 cpu->TAS_Hack = value ? 1 : 0;
}

/////////////////////////////////

void C68K_FASTCALL C68k_Set_IRQ(c68k_struc *cpu, s32 level)
{
    cpu->IRQLine = level;
    //cpu->Status &= ~(C68K_HALTED | C68K_WAITING);
}

// Read / Write dummy functions
////////////////////////////////

u8 C68K_FASTCALL C68k_Read8_Dummy(const u32 adr) 
{
    return 0;
}

u16 C68K_FASTCALL C68k_Read16_Dummy(const u32 adr)
{
    return 0;
}

void C68K_FASTCALL C68k_Write8_Dummy(const u32 adr, u8 data)
{

}

void C68K_FASTCALL C68k_Write16_Dummy(const u32 adr, u16 data)
{

}

s32 C68K_FASTCALL C68k_Interrupt_Ack_Dummy(s32 level)
{
    // return vector
    return (C68K_INTERRUPT_AUTOVECTOR_EX + level);
}

void C68K_FASTCALL C68k_Reset_Dummy(void)
{

}

// Read / Write core functions
///////////////////////////////

u32 C68k_Read_Byte(c68k_struc *cpu, u32 adr)
{
    return cpu->Read_Byte(adr);
}

u32 C68k_Read_Word(c68k_struc *cpu, u32 adr)
{
    return cpu->Read_Word(adr);
}

u32 C68k_Read_Long(c68k_struc *cpu, u32 adr)
{
    return (cpu->Read_Word(adr) << 16) | (cpu->Read_Word(adr + 2) & 0xFFFF);
}

void C68k_Write_Byte(c68k_struc *cpu, u32 adr, u32 data)
{
    cpu->Write_Byte(adr, data);
}

void C68k_Write_Word(c68k_struc *cpu, u32 adr, u32 data)
{
    cpu->Write_Word(adr, data);
}

void C68k_Write_Long(c68k_struc *cpu, u32 adr, u32 data)
{
    cpu->Write_Word(adr, data >> 16);
    cpu->Write_Word(adr + 2, data & 0xFFFF);
}

// setting core functions
//////////////////////////

void C68k_Set_ReadB(c68k_struc *cpu, C68K_READ8 *Func)
{
    cpu->Read_Byte = Func;
}

void C68k_Set_ReadW(c68k_struc *cpu, C68K_READ16 *Func)
{
    cpu->Read_Word = Func;
}

void C68k_Set_WriteB(c68k_struc *cpu, C68K_WRITE8 *Func)
{
    cpu->Write_Byte = Func;
}

void C68k_Set_WriteW(c68k_struc *cpu, C68K_WRITE16 *Func)
{
    cpu->Write_Word = Func;
}

// externals main functions
////////////////////////////

u32 C68k_Get_DReg(c68k_struc *cpu, u32 num)
{
    return cpu->D[num];
}

u32 C68k_Get_AReg(c68k_struc *cpu, u32 num)
{
    return cpu->A[num];
}

u32 C68k_Get_PC(c68k_struc *cpu)
{
    return (cpu->PC);
}

u32 C68k_Get_SR(c68k_struc *cpu)
{
    c68k_struc *CPU = cpu;
    return GET_SR;
}

u32 C68k_Get_USP(c68k_struc *cpu)
{
    if (cpu->flag_S) return cpu->USP;
    else return cpu->A[7];
}

u32 C68k_Get_MSP(c68k_struc *cpu)
{
    if (cpu->flag_S) return cpu->A[7];
    else return cpu->USP;
}

void C68k_Set_DReg(c68k_struc *cpu, u32 num, u32 val)
{
    cpu->D[num] = val;
}

void C68k_Set_AReg(c68k_struc *cpu, u32 num, u32 val)
{
    cpu->A[num] = val;
}

void C68k_Set_PC(c68k_struc *cpu, u32 val)
{
    cpu->PC = val;
}

void C68k_Set_SR(c68k_struc *cpu, u32 val)
{
    c68k_struc *CPU = cpu;
    SET_SR(val);
}

void C68k_Set_USP(c68k_struc *cpu, u32 val)
{
    if (cpu->flag_S) cpu->USP = val;
    else cpu->A[7] = val;
}

void C68k_Set_MSP(c68k_struc *cpu, u32 val)
{
    if (cpu->flag_S) cpu->A[7] = val;
    else cpu->USP = val;
}


unsigned int C68k_Get_State_Max_Len(void)
{
 //printf("loopie: %d\n", (int)sizeof(c68k_struc));
 return(512);
}

#define PACK_U32(val) { u32 temp = (val); buffer[index++] = temp; buffer[index++] = temp >> 8; buffer[index++] = temp >> 16; buffer[index++] = temp >> 24; }
#define UNPACK_U32(target) { if(index >= length) goto BadBad; target = buffer[index] | (buffer[index + 1] << 8) | (buffer[index + 2] << 16) | (buffer[index + 3] << 24); index += 4; }

void C68k_Save_State(c68k_struc *cpu, u8 *buffer)
{
 int index = 0;
 int length = C68k_Get_State_Max_Len();
 int i;

 PACK_U32(0); //0xDEADBEEF);

 for(i = 0; i < 16; i++)
 {
  PACK_U32(cpu->DA[i]);
 }
 PACK_U32(cpu->flag_C);
 PACK_U32(cpu->flag_V);
 PACK_U32(cpu->flag_notZ);
 PACK_U32(cpu->flag_N);
 PACK_U32(cpu->flag_X);
 PACK_U32(cpu->flag_I);
 PACK_U32(cpu->flag_S);

 PACK_U32(cpu->USP);
 PACK_U32(cpu->PC);
 PACK_U32(cpu->Status);
 PACK_U32(cpu->IRQLine);

 PACK_U32(0xDEADBEEF);

 //printf("Save: %d\n", index);
}

void C68k_Load_State(c68k_struc *cpu, const u8 *buffer)
{
 int index = 0;
 int length = C68k_Get_State_Max_Len();
 int version;
 int i;
 u32 footer_check;
 
 UNPACK_U32(version);

 for(i = 0; i < 16; i++)
 {
  UNPACK_U32(cpu->DA[i]);
 }
 UNPACK_U32(cpu->flag_C);
 UNPACK_U32(cpu->flag_V);
 UNPACK_U32(cpu->flag_notZ);
 UNPACK_U32(cpu->flag_N);
 UNPACK_U32(cpu->flag_X);
 UNPACK_U32(cpu->flag_I);
 UNPACK_U32(cpu->flag_S);

 UNPACK_U32(cpu->USP);
 UNPACK_U32(cpu->PC);
 UNPACK_U32(cpu->Status);
 UNPACK_U32(cpu->IRQLine);
 UNPACK_U32(footer_check);

 //printf("%08x\n", footer_check);
 //printf("Load: %d\n", index);

 return;
 BadBad:
 puts("Very bad");

 return;
}
