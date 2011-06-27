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

// internals core macros
/////////////////////////

#define LSL(A, C)       ((A) << (C))
#define LSR(A, C)       ((A) >> (C))

#define LSR_32(A, C)    ((C) < 32 ? (A) >> (C) : 0)
#define LSL_32(A, C)    ((C) < 32 ? (A) << (C) : 0)

#define ROL_8(A, C)     (LSL(A, C) | LSR(A, 8-(C)))
#define ROL_9(A, C)     (LSL(A, C) | LSR(A, 9-(C)))
#define ROL_16(A, C)    (LSL(A, C) | LSR(A, 16-(C)))
#define ROL_17(A, C)    (LSL(A, C) | LSR(A, 17-(C)))
#define ROL_32(A, C)    (LSL_32(A, C) | LSR_32(A, 32-(C)))
#define ROL_33(A, C)    (LSL_32(A, C) | LSR_32(A, 33-(C)))

#define ROR_8(A, C)     (LSR(A, C) | LSL(A, 8-(C)))
#define ROR_9(A, C)     (LSR(A, C) | LSL(A, 9-(C)))
#define ROR_16(A, C)    (LSR(A, C) | LSL(A, 16-(C)))
#define ROR_17(A, C)    (LSR(A, C) | LSL(A, 17-(C)))
#define ROR_32(A, C)    (LSR_32(A, C) | LSL_32(A, 32-(C)))
#define ROR_33(A, C)    (LSR_32(A, C) | LSL_32(A, 33-(C)))


#define RET(A)                  \
    CCnt -= (A);                \
    goto C68k_Exec_End;

#define SET_PC(A)	PC = (A);

#define PRE_IO            
//    CPU->CycleIO = CCnt;
#define POST_IO    
//    CCnt = CPU->CycleIO;

#define READ_BYTE_F(A, D)           \
    D = CPU->Read_Byte(A) & 0xFF;

#define READ_WORD_F(A, D)           \
    D = CPU->Read_Word(A) & 0xFFFF;

#define READ_LONG_F(A, D)               \
    D = CPU->Read_Word((A)) << 16;          \
    D |= CPU->Read_Word((A) + 2) & 0xFFFF;

#define READSX_BYTE_F(A, D)             \
    D = (s32)(s8)CPU->Read_Byte(A);

#define READSX_WORD_F(A, D)             \
    D = (s32)(s16)CPU->Read_Word(A);
    
#define READSX_LONG_F(A, D)             \
    D = CPU->Read_Word((A)) << 16;          \
    D |= CPU->Read_Word((A) + 2) & 0xFFFF;

#define WRITE_BYTE_F(A, D)      \
    CPU->Write_Byte(A, D);

#define WRITE_WORD_F(A, D)      \
    CPU->Write_Word(A, D);

#define WRITE_LONG_F(A, D)              \
    CPU->Write_Word((A), (D) >> 16);        \
    CPU->Write_Word((A) + 2, (D) & 0xFFFF);

#define WRITE_LONG_DEC_F(A, D)          \
    CPU->Write_Word((A) + 2, (D) & 0xFFFF); \
    CPU->Write_Word((A), (D) >> 16);

#define PUSH_16_F(D)                    \
    CPU->A[7] -= 2;			\
    CPU->Write_Word(CPU->A[7], D); \

#define POP_16_F(D)                     \
    D = (u16)CPU->Read_Word(CPU->A[7]); \
    CPU->A[7] += 2;

#define PUSH_32_F(D)                            \
    CPU->A[7] -= 4;                                 \
    CPU->Write_Word(CPU->A[7] + 2, (D) & 0xFFFF);   \
    CPU->Write_Word(CPU->A[7], (D) >> 16);

#define POP_32_F(D)                             \
    D = CPU->Read_Word(CPU->A[7]) << 16;            \
    D |= CPU->Read_Word(CPU->A[7] + 2) & 0xFFFF;    \
    CPU->A[7] += 4;

#define FETCH_BYTE          ((u8)CPU->Read_Word(PC))
#define FETCH_WORD          CPU->Read_Word(PC)
#define FETCH_LONG          ((CPU->Read_Word(PC) << 16) | (CPU->Read_Word(PC + 2)))

// FIXME?
#define DECODE_EXT_WORD     \
{                           \
    u32 ext;                \
                            \
    ext = FETCH_WORD;       \
    PC += 2;                \
                            \
    adr += (s32)((s8)(ext));                            \
    if (ext & 0x0800) adr += (s32) CPU->D[ext >> 12];   \
    else adr += (s32)((s16)(CPU->D[ext >> 12]));        \
}

#define GET_CCR                                     \
    (((CPU->flag_C >> (C68K_SR_C_SFT - 0)) & 1) |   \
     ((CPU->flag_V >> (C68K_SR_V_SFT - 1)) & 2) |   \
     (((!CPU->flag_notZ) & 1) << 2) |               \
     ((CPU->flag_N >> (C68K_SR_N_SFT - 3)) & 8) |   \
     ((CPU->flag_X >> (C68K_SR_X_SFT - 4)) & 0x10))

#define GET_SR                  \
    ((CPU->flag_S << 0)  |      \
     (CPU->flag_I << 8)  |      \
     GET_CCR)

#define SET_CCR(A)                              \
    CPU->flag_C = (A) << (C68K_SR_C_SFT - 0);   \
    CPU->flag_V = (A) << (C68K_SR_V_SFT - 1);   \
    CPU->flag_notZ = ~(A) & 4;                  \
    CPU->flag_N = (A) << (C68K_SR_N_SFT - 3);   \
    CPU->flag_X = (A) << (C68K_SR_X_SFT - 4);

#define SET_SR(A)                   \
    SET_CCR(A)                      \
    CPU->flag_I = ((A) >> 8) & 7;   \
    CPU->flag_S = (A) & C68K_SR_S;

#define CHECK_INT                                   \
    {                                               \
        s32 line, vect;                             \
                                                    \
        line = CPU->IRQLine;                        \
        /*if(line) printf("Meow: %d, flag_I: %d\n", line, CPU->flag_I);          */     \
        if ((line == 7) || (line > CPU->flag_I))    \
        {                                           \
	    cpu->Status &= ~(C68K_HALTED | C68K_WAITING);	\
            PRE_IO                                  \
                                                    \
            /* get vector */                                        \
            CPU->IRQLine = 0;                                       \
            vect = CPU->Interrupt_CallBack(line);                   \
            if (vect == C68K_INT_ACK_AUTOVECTOR)                    \
                vect = C68K_INTERRUPT_AUTOVECTOR_EX + (line & 7);   \
                                                                    \
            /* adjust CCnt */                                       \
            CCnt -= c68k_exception_cycle_table[vect];               \
                                                                    \
            /* swap A7 and USP */           \
            if (!CPU->flag_S)               \
            {                               \
                u32 tmpSP;                  \
                                            \
                tmpSP = CPU->USP;           \
                CPU->USP = CPU->A[7];       \
                CPU->A[7] = tmpSP;          \
            }                               \
                                            \
            /* push PC and SR */            \
            PUSH_32_F(PC)     \
            PUSH_16_F(GET_SR)               \
                                            \
            /* adjust SR */                 \
            CPU->flag_S = C68K_SR_S;        \
            CPU->flag_I = line;             \
                                            \
            /* fetch new PC */              \
            READ_LONG_F(vect * 4, PC)       \
            SET_PC(PC)                      \
                                            \
            POST_IO                         \
        }                                   \
    }
