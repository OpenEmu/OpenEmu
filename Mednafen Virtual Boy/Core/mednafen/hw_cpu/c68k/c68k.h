/*  Copyright 2003-2004 Stephane Dallongeville
    Copyright 2004 Theo Berkau

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
 * C68K.H :
 *
 * C68K include file
 *
 ********************************************************************************/

#ifndef _C68K_H_
#define _C68K_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define C68K_FASTCALL
#include <inttypes.h>

typedef int64_t s64;
typedef uint64_t u64;

typedef int32_t s32;
typedef uint32_t u32;

typedef int16_t s16;
typedef uint16_t u16;

typedef int8_t s8;
typedef uint8_t u8;

#ifdef __cplusplus
extern "C" {
#endif

// setting
///////////

//#define C68K_GEN

#ifdef MSB_FIRST
#define C68K_BIG_ENDIAN
#endif

#ifdef C68K_BIG_ENDIAN
 #define BYTE_OFF 3
 #define WORD_OFF 1
#else
 #define BYTE_OFF 0
 #define WORD_OFF 0
#endif

#define C68K_NO_JUMP_TABLE
//#define C68K_CONST_JUMP_TABLE
//#define C68K_AUTOVECTOR_CALLBACK

// 68K core types definitions
//////////////////////////////

#define C68K_ADR_BITS   24

#define C68K_SR_C_SFT   8
#define C68K_SR_V_SFT   7
#define C68K_SR_Z_SFT   0
#define C68K_SR_N_SFT   7
#define C68K_SR_X_SFT   8

#define C68K_SR_S_SFT   13

#define C68K_SR_C       (1 << C68K_SR_C_SFT)
#define C68K_SR_V       (1 << C68K_SR_V_SFT)
#define C68K_SR_Z       0
#define C68K_SR_N       (1 << C68K_SR_N_SFT)
#define C68K_SR_X       (1 << C68K_SR_X_SFT)

#define C68K_SR_S       (1 << C68K_SR_S_SFT)

#define C68K_CCR_MASK   0x1F
#define C68K_SR_MASK    (0x2700 | C68K_CCR_MASK)

// exception defines taken from musashi core
#define C68K_RESET_EX                   1
#define C68K_BUS_ERROR_EX               2
#define C68K_ADDRESS_ERROR_EX           3
#define C68K_ILLEGAL_INSTRUCTION_EX     4
#define C68K_ZERO_DIVIDE_EX             5
#define C68K_CHK_EX                     6
#define C68K_TRAPV_EX                   7
#define C68K_PRIVILEGE_VIOLATION_EX     8
#define C68K_TRACE_EX                   9
#define C68K_1010_EX                    10
#define C68K_1111_EX                    11
#define C68K_FORMAT_ERROR_EX            14
#define C68K_UNINITIALIZED_INTERRUPT_EX 15
#define C68K_SPURIOUS_INTERRUPT_EX      24
#define C68K_INTERRUPT_AUTOVECTOR_EX    24
#define C68K_TRAP_BASE_EX               32

#define C68K_INT_ACK_AUTOVECTOR         -1

#define C68K_HALTED     0x02
#define C68K_WAITING    0x04
#define C68K_DISABLE    0x10
#define C68K_FAULTED    0x40

typedef u8 C68K_FASTCALL C68K_READ8(const u32 adr);
typedef u16 C68K_FASTCALL C68K_READ16(const u32 adr);

typedef void C68K_FASTCALL C68K_WRITE8(const u32 adr, u8 data);
typedef void C68K_FASTCALL C68K_WRITE16(const u32 adr, u16 data);

typedef s32  C68K_FASTCALL C68K_INT_CALLBACK(s32 level);
typedef void C68K_FASTCALL C68K_RESET_CALLBACK(void);

typedef struct 
{
    union
    {   
     struct
     {
      u32 D[8];       // 32 bytes aligned
      u32 A[8];       // 16 bytes aligned
     };
     u32 DA[16];
    };

    u32 flag_C;     // 32 bytes aligned
    u32 flag_V;
    u32 flag_notZ;
    u32 flag_N;

    u32 flag_X;     // 16 bytes aligned
    u32 flag_I;
    u32 flag_S;
    
    u32 USP;

    u32 PC;         // 32 bytes aligned
    u32 Status;
    s32 IRQLine;
    
    s32 timestamp;

    u32 dirty1;
    
    C68K_READ8 *Read_Byte;                   // 32 bytes aligned
    C68K_READ16 *Read_Word;

    C68K_WRITE8 *Write_Byte;
    C68K_WRITE16 *Write_Word;

    C68K_INT_CALLBACK *Interrupt_CallBack;  // 16 bytes aligned
    C68K_RESET_CALLBACK *Reset_CallBack;

    int TAS_Hack;

} c68k_struc;


// 68K core var declaration
////////////////////////////

extern  c68k_struc C68K;


// 68K core function declaration
/////////////////////////////////

void    C68k_Init(c68k_struc *cpu, C68K_INT_CALLBACK *int_cb);
void	C68k_Set_TAS_Hack(c68k_struc *cpu, int value);

s32     C68K_FASTCALL C68k_Reset(c68k_struc *cpu);

void	C68K_FASTCALL C68k_Exec(c68k_struc *cpu);

void    C68K_FASTCALL C68k_Set_IRQ(c68k_struc *cpu, s32 level);

void    C68k_Set_ReadB(c68k_struc *cpu, C68K_READ8 *Func);
void    C68k_Set_ReadW(c68k_struc *cpu, C68K_READ16 *Func);
void    C68k_Set_WriteB(c68k_struc *cpu, C68K_WRITE8 *Func);
void    C68k_Set_WriteW(c68k_struc *cpu, C68K_WRITE16 *Func);

void	C68k_Set_Debug(c68k_struc *cpu, void (*exec_hook)(u32 address, u16 opcode));

u32     C68k_Get_DReg(c68k_struc *cpu, u32 num);
u32     C68k_Get_AReg(c68k_struc *cpu, u32 num);
u32     C68k_Get_PC(c68k_struc *cpu);
u32     C68k_Get_SR(c68k_struc *cpu);
u32     C68k_Get_USP(c68k_struc *cpu);
u32     C68k_Get_MSP(c68k_struc *cpu);

void    C68k_Set_DReg(c68k_struc *cpu, u32 num, u32 val);
void    C68k_Set_AReg(c68k_struc *cpu, u32 num, u32 val);
void    C68k_Set_PC(c68k_struc *cpu, u32 val);
void    C68k_Set_SR(c68k_struc *cpu, u32 val);
void    C68k_Set_USP(c68k_struc *cpu, u32 val);
void    C68k_Set_MSP(c68k_struc *cpu, u32 val);

#include <string.h>

static inline void C68k_Copy_State(const c68k_struc *source, c68k_struc *dest)
{
 memcpy(&dest->D[0], &source->D[0], (&(source->dirty1)) - (&(source->D[0])));
}

unsigned int C68k_Get_State_Max_Len(void);
void C68k_Save_State(c68k_struc *cpu, u8 *buffer);
void C68k_Load_State(c68k_struc *cpu, const u8 *buffer);

#ifdef __cplusplus
}
#endif

#endif  // _C68K_H_

