@ ScummVM - Graphic Adventure Engine
@
@ ScummVM is the legal property of its developers, whose names
@ are too numerous to list here. Please refer to the COPYRIGHT
@ file distributed with this source distribution.
@
@ This program is free software@ you can redistribute it and/or
@ modify it under the terms of the GNU General Public License
@ as published by the Free Software Foundation@ either version 2
@ of the License, or (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY@ without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program@ if not, write to the Free Software
@ Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
@
@ @author Robin Watts (robin@wss.co.uk)
@
@ This file, together with rate_arm.cpp, provides an ARM optimised version
@ of rate.cpp. The algorithm is essentially the same as that within rate.cpp
@ so to understand this file you should understand rate.cpp first.

        .text

        .global _ARM_CopyRate_M
        .global _ARM_CopyRate_S
        .global _ARM_CopyRate_R
        .global _ARM_SimpleRate_M
        .global _ARM_SimpleRate_S
        .global _ARM_SimpleRate_R
        .global _ARM_LinearRate_M
        .global _ARM_LinearRate_S
        .global _ARM_LinearRate_R

_ARM_CopyRate_M:
        @ r0 = len
        @ r1 = obuf
        @ r2 = vol_l
        @ r3 = vol_r
        @ <> = ptr
        LDR     r12,[r13]
        STMFD   r13!,{r4-r7,r14}

        MOV     r14,#0                  @ r14= 0
        ORR     r2, r2, r2, LSL #8      @ r2 = vol_l as 16 bits
        ORR     r3, r3, r3, LSL #8      @ r3 = vol_r as 16 bits
CopyRate_M_loop:
        LDRSH   r5, [r12], #2           @ r5 = tmp0 = tmp1 = *ptr++
        LDRSH   r6, [r1]                @ r6 = obuf[0]
        LDRSH   r7, [r1, #2]            @ r7 = obuf[1]
        MUL     r4, r2, r5              @ r4 = tmp0*vol_l
        MUL     r5, r3, r5              @ r5 = tmp1*vol_r

        ADDS    r6, r4, r6, LSL #16     @ r6 = obuf[0]<<16 + tmp0*vol_l
        RSCVS   r6, r14,#0x80000000     @ Clamp r6
        ADDS    r7, r5, r7, LSL #16     @ r7 = obuf[1]<<16 + tmp1*vol_r
        RSCVS   r7, r14,#0x80000000     @ Clamp r7

        MOV     r6, r6, LSR #16         @ Shift back to halfword
        MOV     r7, r7, LSR #16         @ Shift back to halfword

        STRH    r6, [r1], #2            @ Store output value
        STRH    r7, [r1], #2            @ Store output value

        SUBS    r0,r0,#1                @ len--
        BGT     CopyRate_M_loop         @ and loop

        MOV     r0, r1                  @ return obuf

        LDMFD   r13!,{r4-r7,PC}

_ARM_CopyRate_S:
        @ r0 = len
        @ r1 = obuf
        @ r2 = vol_l
        @ r3 = vol_r
        @ <> = ptr
        LDR     r12,[r13]
        STMFD   r13!,{r4-r7,r14}

        MOV     r14,#0                  @ r14= 0
        ORR     r2, r2, r2, LSL #8      @ r2 = vol_l as 16 bits
        ORR     r3, r3, r3, LSL #8      @ r3 = vol_r as 16 bits
CopyRate_S_loop:
        LDRSH   r4, [r12],#2            @ r4 = tmp0 = *ptr++
        LDRSH   r5, [r12],#2            @ r5 = tmp1 = *ptr++
        LDRSH   r6, [r1]                @ r6 = obuf[0]
        LDRSH   r7, [r1,#2]             @ r7 = obuf[1]
        MUL     r4, r2, r4              @ r4 = tmp0*vol_l
        MUL     r5, r3, r5              @ r5 = tmp1*vol_r

        ADDS    r6, r4, r6, LSL #16     @ r6 = obuf[0]<<16 + tmp0*vol_l
        RSCVS   r6, r14,#0x80000000     @ Clamp r6
        ADDS    r7, r5, r7, LSL #16     @ r7 = obuf[1]<<16 + tmp1*vol_r
        RSCVS   r7, r14,#0x80000000     @ Clamp r7

        MOV     r6, r6, LSR #16         @ Shift back to halfword
        MOV     r7, r7, LSR #16         @ Shift back to halfword

        STRH    r6, [r1],#2             @ Store output value
        STRH    r7, [r1],#2             @ Store output value

        SUBS    r0,r0,#2                @ len -= 2
        BGT     CopyRate_S_loop         @ and loop

        MOV     r0, r1                  @ return obuf

        LDMFD   r13!,{r4-r7,PC}

_ARM_CopyRate_R:
        @ r0 = len
        @ r1 = obuf
        @ r2 = vol_l
        @ r3 = vol_r
        @ <> = ptr
        LDR     r12,[r13]
        STMFD   r13!,{r4-r7,r14}

        MOV     r14,#0                  @ r14= 0
        ORR     r2, r2, r2, LSL #8      @ r2 = vol_l as 16 bits
        ORR     r3, r3, r3, LSL #8      @ r3 = vol_r as 16 bits
CopyRate_R_loop:
        LDRSH   r4, [r12],#2            @ r4 = tmp0 = *ptr++
        LDRSH   r5, [r12],#2            @ r5 = tmp1 = *ptr++
        LDRSH   r6, [r1]                @ r6 = obuf[0]
        LDRSH   r7, [r1,#2]             @ r7 = obuf[1]
        MUL     r4, r2, r4              @ r4 = tmp0*vol_l
        MUL     r5, r3, r5              @ r5 = tmp1*vol_r

        ADDS    r6, r5, r6, LSL #16     @ r6 = obuf[0]<<16 + tmp1*vol_r
        RSCVS   r6, r14,#0x80000000     @ Clamp r6
        ADDS    r7, r4, r7, LSL #16     @ r7 = obuf[1]<<16 + tmp0*vol_l
        RSCVS   r7, r14,#0x80000000     @ Clamp r7

        MOV     r6, r6, LSR #16         @ Shift back to halfword
        MOV     r7, r7, LSR #16         @ Shift back to halfword

        STRH    r6, [r1],#2             @ Store output value
        STRH    r7, [r1],#2             @ Store output value

        SUBS    r0,r0,#2                @ len -= 2
        BGT     CopyRate_R_loop         @ and loop

        MOV     r0, r1                  @ return obuf

        LDMFD   r13!,{r4-r7,PC}

_ARM_SimpleRate_M:
        @ r0 = AudioStream &input
        @ r1 = input.readBuffer
        @ r2 = input->sr
        @ r3 = obuf
        @ <> = osamp
        @ <> = vol_l
        @ <> = vol_r
        MOV     r12,r13
        STMFD   r13!,{r0-r2,r4-r8,r10-r11,r14}
        LDMFD   r12,{r11,r12,r14}       @ r11= osamp
                                        @ r12= vol_l
                                        @ r14= vol_r
        LDMIA   r2,{r0,r1,r2,r8}        @ r0 = inPtr
                                        @ r1 = inLen
                                        @ r2 = opos
                                        @ r8 = opos_inc
        CMP     r11,#0                  @ if (osamp <= 0)
        BLE     SimpleRate_M_end        @   bale
        MOV     r10,#0
        ORR     r12,r12,r12,LSL #8      @ r12= vol_l as 16 bits
        ORR     r14,r14,r14,LSL #8      @ r14= vol_r as 16 bits
SimpleRate_M_loop:
        SUBS    r1, r1, #1              @ r1 = inLen -= 1
        BLT     SimpleRate_M_read
        SUBS    r2, r2, #1              @ r2 = opos--
        ADDGE   r0, r0, #2              @ if (r2 >= 0) { sr.inPtr++
        BGE     SimpleRate_M_loop       @                and loop }
SimpleRate_M_read_return:
        LDRSH   r5, [r0],#2             @ r5 = tmp1 = *inPtr++
        LDRSH   r6, [r3]                @ r6 = obuf[0]
        LDRSH   r7, [r3,#2]             @ r7 = obuf[1]
        ADD     r2, r2, r8              @ r2 = opos += opos_inc
        MUL     r4, r12,r5              @ r4 = tmp0*vol_l
        MUL     r5, r14,r5              @ r5 = tmp1*vol_r

        ADDS    r6, r4, r6, LSL #16     @ r6 = obuf[0]<<16 + tmp0*vol_l
        RSCVS   r6, r10,#0x80000000     @ Clamp r6
        ADDS    r7, r5, r7, LSL #16     @ r7 = obuf[1]<<16 + tmp1*vol_r
        RSCVS   r7, r10,#0x80000000     @ Clamp r7

        MOV     r6, r6, LSR #16         @ Shift back to halfword
        MOV     r7, r7, LSR #16         @ Shift back to halfword

        STRH    r6, [r3],#2             @ Store output value
        STRH    r7, [r3],#2             @ Store output value

        SUBS    r11,r11,#1              @ len--
        BGT     SimpleRate_M_loop       @ and loop
SimpleRate_M_end:
        LDR     r14,[r13,#8]            @ r14 = sr
        ADD     r13,r13,#12             @ Skip over r0-r2 on stack
        STMIA   r14,{r0,r1,r2}          @ Store back updated values

        MOV     r0, r3                  @ return obuf

        LDMFD   r13!,{r4-r8,r10-r11,PC}
SimpleRate_M_read:
        LDR     r0, [r13,#8]            @ r0 = sr (8 = 4*2)
        ADD     r0, r0, #16             @ r0 = inPtr = inBuf
        STMFD   r13!,{r0,r2-r3,r12,r14}

        MOV     r1, r0                  @ r1 = inBuf
        LDR     r0, [r13,#20]           @ r0 = AudioStream & input (20 = 4*5)
        MOV     r2, #512                @ r2 = ARRAYSIZE(inBuf)

        @ Calling back into C++ here. WinCE is fairly easy about such things
        @ but other OS are more awkward. r9 is preserved for Symbian, and
        @ we have 3+8+5 = 16 things on the stack (an even number).
        MOV     r14,PC
        LDR     PC,[r13,#24]            @ inLen = input.readBuffer(inBuf,512) (24 = 4*6)
        SUBS    r1, r0, #1              @ r1 = inLen-1
        LDMFD   r13!,{r0,r2-r3,r12,r14}
        BLT     SimpleRate_M_end
        SUBS    r2, r2, #1              @ r2 = opos--
        ADDGE   r0, r0, #2              @ if (r2 >= 0) { sr.inPtr++
        BGE     SimpleRate_M_loop       @                and loop }
        B       SimpleRate_M_read_return


_ARM_SimpleRate_S:
        @ r0 = AudioStream &input
        @ r1 = input.readBuffer
        @ r2 = input->sr
        @ r3 = obuf
        @ <> = osamp
        @ <> = vol_l
        @ <> = vol_r
        MOV     r12,r13
        STMFD   r13!,{r0-r2,r4-r8,r10-r11,r14}
        LDMFD   r12,{r11,r12,r14}       @ r11= osamp
                                        @ r12= vol_l
                                        @ r14= vol_r
        LDMIA   r2,{r0,r1,r2,r8}        @ r0 = inPtr
                                        @ r1 = inLen
                                        @ r2 = opos
                                        @ r8 = opos_inc
        CMP     r11,#0                  @ if (osamp <= 0)
        BLE     SimpleRate_S_end        @   bale
        MOV     r10,#0
        ORR     r12,r12,r12,LSL #8      @ r12= vol_l as 16 bits
        ORR     r14,r14,r14,LSL #8      @ r14= vol_r as 16 bits
SimpleRate_S_loop:
        SUBS    r1, r1, #2              @ r1 = inLen -= 2
        BLT     SimpleRate_S_read
        SUBS    r2, r2, #1              @ r2 = opos--
        ADDGE   r0, r0, #4              @ if (r2 >= 0) { sr.inPtr += 2
        BGE     SimpleRate_S_loop       @                and loop }
SimpleRate_S_read_return:
        LDRSH   r4, [r0],#2             @ r4 = tmp0 = *inPtr++
        LDRSH   r5, [r0],#2             @ r5 = tmp1 = *inPtr++
        LDRSH   r6, [r3]                @ r6 = obuf[0]
        LDRSH   r7, [r3,#2]             @ r7 = obuf[1]
        ADD     r2, r2, r8              @ r2 = opos += opos_inc
        MUL     r4, r12,r4              @ r4 = tmp0*vol_l
        MUL     r5, r14,r5              @ r5 = tmp1*vol_r

        ADDS    r6, r4, r6, LSL #16     @ r6 = obuf[0]<<16 + tmp0*vol_l
        RSCVS   r6, r10,#0x80000000     @ Clamp r6
        ADDS    r7, r5, r7, LSL #16     @ r7 = obuf[1]<<16 + tmp1*vol_r
        RSCVS   r7, r10,#0x80000000     @ Clamp r7

        MOV     r6, r6, LSR #16         @ Shift back to halfword
        MOV     r7, r7, LSR #16         @ Shift back to halfword

        STRH    r6, [r3],#2             @ Store output value
        STRH    r7, [r3],#2             @ Store output value

        SUBS    r11,r11,#1              @ osamp--
        BGT     SimpleRate_S_loop       @ and loop
SimpleRate_S_end:
        LDR     r14,[r13,#8]            @ r14 = sr
        ADD     r13,r13,#12             @ skip over r0-r2 on stack
        STMIA   r14,{r0,r1,r2}          @ store back updated values
        MOV     r0, r3                  @ return obuf
        LDMFD   r13!,{r4-r8,r10-r11,PC}
SimpleRate_S_read:
        LDR     r0, [r13,#8]            @ r0 = sr (8 = 4*2)
        ADD     r0, r0, #16             @ r0 = inPtr = inBuf
        STMFD   r13!,{r0,r2-r3,r12,r14}
        MOV     r1, r0                  @ r1 = inBuf
        LDR     r0, [r13,#20]           @ r0 = AudioStream & input (20 = 4*5)
        MOV     r2, #512                @ r2 = ARRAYSIZE(inBuf)

        @ Calling back into C++ here. WinCE is fairly easy about such things
        @ but other OS are more awkward. r9 is preserved for Symbian, and
        @ we have 3+8+5 = 16 things on the stack (an even number).
        MOV     r14,PC
        LDR     PC,[r13,#24]            @ inLen = input.readBuffer(inBuf,512) (24 = 4*6)
        SUBS    r1, r0, #2              @ r1 = inLen-2
        LDMFD   r13!,{r0,r2-r3,r12,r14}
        BLT     SimpleRate_S_end
        SUBS    r2, r2, #1              @ r2 = opos--
        ADDGE   r0, r0, #4              @ if (r2 >= 0) { sr.inPtr += 2
        BGE     SimpleRate_S_loop       @                and loop }
        B       SimpleRate_S_read_return



_ARM_SimpleRate_R:
        @ r0 = AudioStream &input
        @ r1 = input.readBuffer
        @ r2 = input->sr
        @ r3 = obuf
        @ <> = osamp
        @ <> = vol_l
        @ <> = vol_r
        MOV     r12,r13
        STMFD   r13!,{r0-r2,r4-r8,r10-r11,r14}
        LDMFD   r12,{r11,r12,r14}       @ r11= osamp
                                        @ r12= vol_l
                                        @ r14= vol_r
        LDMIA   r2,{r0,r1,r2,r8}        @ r0 = inPtr
                                        @ r1 = inLen
                                        @ r2 = opos
                                        @ r8 = opos_inc
        CMP     r11,#0                  @ if (osamp <= 0)
        BLE     SimpleRate_R_end        @   bale
        MOV     r10,#0
        ORR     r12,r12,r12,LSL #8      @ r12= vol_l as 16 bits
        ORR     r14,r14,r14,LSL #8      @ r14= vol_r as 16 bits
SimpleRate_R_loop:
        SUBS    r1, r1, #2              @ r1 = inLen -= 2
        BLT     SimpleRate_R_read
        SUBS    r2, r2, #1              @ r2 = opos--
        ADDGE   r0, r0, #4              @ if (r2 >= 0) { sr.inPtr += 2
        BGE     SimpleRate_R_loop       @                and loop }
SimpleRate_R_read_return:
        LDRSH   r4, [r0],#2             @ r4 = tmp0 = *inPtr++
        LDRSH   r5, [r0],#2             @ r5 = tmp1 = *inPtr++
        LDRSH   r6, [r3]                @ r6 = obuf[0]
        LDRSH   r7, [r3,#2]             @ r7 = obuf[1]
        ADD     r2, r2, r8              @ r2 = opos += opos_inc
        MUL     r4, r12,r4              @ r4 = tmp0*vol_l
        MUL     r5, r14,r5              @ r5 = tmp1*vol_r

        ADDS    r6, r5, r6, LSL #16     @ r6 = obuf[0]<<16 + tmp1*vol_r
        RSCVS   r6, r10,#0x80000000     @ Clamp r6
        ADDS    r7, r4, r7, LSL #16     @ r7 = obuf[1]<<16 + tmp0*vol_l
        RSCVS   r7, r10,#0x80000000     @ Clamp r7

        MOV     r6, r6, LSR #16         @ Shift back to halfword
        MOV     r7, r7, LSR #16         @ Shift back to halfword

        STRH    r6, [r3],#2             @ Store output value
        STRH    r7, [r3],#2             @ Store output value

        SUBS    r11,r11,#1              @ osamp--
        BGT     SimpleRate_R_loop       @ and loop
SimpleRate_R_end:
        LDR     r14,[r13,#8]            @ r14 = sr
        ADD     r13,r13,#12             @ skip over r0-r2 on stack
        STMIA   r14,{r0,r1,r2}          @ store back updated values
        MOV     r0, r3                  @ return obuf
        LDMFD   r13!,{r4-r8,r10-r11,PC}
SimpleRate_R_read:
        LDR     r0, [r13,#8]            @ r0 = sr (8 = 4*2)
        ADD     r0, r0, #16             @ r0 = inPtr = inBuf
        STMFD   r13!,{r0,r2-r3,r12,r14}
        MOV     r1, r0                  @ r1 = inBuf
        LDR     r0, [r13,#20]           @ r0 = AudioStream & input (20 = 4*5)
        MOV     r2, #512                @ r2 = ARRAYSIZE(inBuf)

        @ Calling back into C++ here. WinCE is fairly easy about such things
        @ but other OS are more awkward. r9 is preserved for Symbian, and
        @ we have 3+8+5 = 16 things on the stack (an even number).
        MOV     r14,PC
        LDR     PC,[r13,#24]            @ inLen = input.readBuffer(inBuf,512) (24 = 4*6)
        SUBS    r1, r0, #2              @ r1 = inLen-2
        LDMFD   r13!,{r0,r2-r3,r12,r14}
        BLT     SimpleRate_R_end
        SUBS    r2, r2, #1              @ r2 = opos--
        ADDGE   r0, r0, #4              @ if (r2 >= 0) { sr.inPtr += 2
        BGE     SimpleRate_R_loop       @                and loop }
        B       SimpleRate_R_read_return


_ARM_LinearRate_M:
        @ r0 = AudioStream &input
        @ r1 = input.readBuffer
        @ r2 = input->sr
        @ r3 = obuf
        @ <> = osamp
        @ <> = vol_l
        @ <> = vol_r
        MOV     r12,r13
        STMFD   r13!,{r0-r1,r4-r11,r14}
        LDMFD   r12,{r11,r12,r14}       @ r11= osamp
                                        @ r12= vol_l
                                        @ r14= vol_r
        LDMIA   r2,{r0,r1,r8}           @ r0 = inPtr
                                        @ r1 = inLen
                                        @ r8 = opos
        MOV     r10,#0
        CMP     r11,#0                  @ if (osamp <= 0)
        BLE     LinearRate_M_end        @   bale
        ORR     r12,r12,r12,LSL #8      @ r12= vol_l as 16 bits
        ORR     r14,r14,r14,LSL #8      @ r14= vol_r as 16 bits
        CMP     r1,#0
        BGT     LinearRate_M_part2

        @ part1 - read input samples
LinearRate_M_loop:
        SUBS    r1, r1, #1              @ r1 = inLen -= 1
        BLT     LinearRate_M_read
LinearRate_M_read_return:
        LDRH    r4, [r2, #16]           @ r4 = icur[0]
        LDRSH   r5, [r0],#2             @ r5 = tmp1 = *inPtr++
        SUBS    r8, r8, #65536          @ r8 = opos--
        STRH    r4, [r2,#22]            @      ilast[0] = icur[0]
        STRH    r5, [r2,#16]            @      icur[0] = tmp1
        BGE     LinearRate_M_loop

        @ part2 - form output samples
LinearRate_M_part2:
        @ We are guaranteed that opos < 0 here
        LDR     r6, [r2,#20]            @ r6 = ilast[0]<<16 + 32768
        LDRSH   r5, [r2,#16]            @ r5 = icur[0]
        MOV     r4, r8, LSL #16
        MOV     r4, r4, LSR #16
        SUB     r5, r5, r6, ASR #16     @ r5 = icur[0] - ilast[0]
        MLA     r6, r4, r5, r6  @ r6 = (icur[0]-ilast[0])*opos_frac+ilast[0]

        LDRSH   r4, [r3]                @ r4 = obuf[0]
        LDRSH   r5, [r3,#2]             @ r5 = obuf[1]
        MOV     r6, r6, ASR #16         @ r6 = tmp0 = tmp1 >>= 16
        MUL     r7, r12,r6              @ r7 = tmp0*vol_l
        MUL     r6, r14,r6              @ r6 = tmp1*vol_r

        ADDS    r7, r7, r4, LSL #16     @ r7 = obuf[0]<<16 + tmp0*vol_l
        RSCVS   r7, r10, #0x80000000    @ Clamp r7
        ADDS    r6, r6, r5, LSL #16     @ r6 = obuf[1]<<16 + tmp1*vol_r
        RSCVS   r6, r10, #0x80000000    @ Clamp r6

        MOV     r7, r7, LSR #16         @ Shift back to halfword
        MOV     r6, r6, LSR #16         @ Shift back to halfword

        LDR     r5, [r2,#12]            @ r5 = opos_inc
        STRH    r7, [r3],#2             @ Store output value
        STRH    r6, [r3],#2             @ Store output value
        SUBS    r11, r11,#1             @ osamp--
        BLE     LinearRate_M_end        @ end if needed

        ADDS    r8, r8, r5              @ r8 = opos += opos_inc
        BLT     LinearRate_M_part2
        B       LinearRate_M_loop
LinearRate_M_end:
        ADD     r13,r13,#8
        STMIA   r2,{r0,r1,r8}
        MOV     r0, r3                  @ return obuf
        LDMFD   r13!,{r4-r11,PC}
LinearRate_M_read:
        ADD     r0, r2, #28             @ r0 = inPtr = inBuf
        STMFD   r13!,{r0,r2-r3,r12,r14}

        MOV     r1, r0                  @ r1 = inBuf
        LDR     r0, [r13,#20]           @ r0 = AudioStream & input (20 = 4*5)
        MOV     r2, #512                @ r2 = ARRAYSIZE(inBuf)

        @ Calling back into C++ here. WinCE is fairly easy about such things
        @ but other OS are more awkward. r9 is preserved for Symbian, and
        @ we have 2+9+5 = 16 things on the stack (an even number).
        MOV     r14,PC
        LDR     PC,[r13,#24]            @ inLen = input.readBuffer(inBuf,512) (24 = 4*6)
        SUBS    r1, r0, #1              @ r1 = inLen-1
        LDMFD   r13!,{r0,r2-r3,r12,r14}
        BLT     LinearRate_M_end
        B       LinearRate_M_read_return

_ARM_LinearRate_S:
        @ r0 = AudioStream &input
        @ r1 = input.readBuffer
        @ r2 = input->sr
        @ r3 = obuf
        @ <> = osamp
        @ <> = vol_l
        @ <> = vol_r
        MOV     r12,r13
        STMFD   r13!,{r0-r1,r4-r11,r14}
        LDMFD   r12,{r11,r12,r14}       @ r11= osamp
                                        @ r12= vol_l
                                        @ r14= vol_r
        LDMIA   r2,{r0,r1,r8}           @ r0 = inPtr
                                        @ r1 = inLen
                                        @ r8 = opos
        CMP     r11,#0                  @ if (osamp <= 0)
        BLE     LinearRate_S_end        @   bale
        ORR     r12,r12,r12,LSL #8      @ r12= vol_l as 16 bits
        ORR     r14,r14,r14,LSL #8      @ r14= vol_r as 16 bits
        CMP     r1,#0
        BGT     LinearRate_S_part2

        @ part1 - read input samples
LinearRate_S_loop:
        SUBS    r1, r1, #2              @ r1 = inLen -= 2
        BLT     LinearRate_S_read
LinearRate_S_read_return:
        LDR     r10,[r2, #16]           @ r10= icur[0,1]
        LDRSH   r5, [r0],#2             @ r5 = tmp0 = *inPtr++
        LDRSH   r6, [r0],#2             @ r6 = tmp1 = *inPtr++
        SUBS    r8, r8, #65536          @ r8 = opos--
        STRH    r10,[r2,#22]            @      ilast[0] = icur[0]
        MOV     r10,r10,LSR #16
        STRH    r10,[r2,#26]            @      ilast[1] = icur[1]
        STRH    r5, [r2,#16]            @      icur[0] = tmp0
        STRH    r6, [r2,#18]            @      icur[1] = tmp1
        BGE     LinearRate_S_loop

        @ part2 - form output samples
LinearRate_S_part2:
        @ We are guaranteed that opos < 0 here
        LDR     r6, [r2,#20]            @ r6 = ilast[0]<<16 + 32768
        LDRSH   r5, [r2,#16]            @ r5 = icur[0]
        MOV     r4, r8, LSL #16
        MOV     r4, r4, LSR #16
        SUB     r5, r5, r6, ASR #16     @ r5 = icur[0] - ilast[0]
        MLA     r6, r4, r5, r6  @ r6 = (icur[0]-ilast[0])*opos_frac+ilast[0]

        LDR     r7, [r2,#24]            @ r7 = ilast[1]<<16 + 32768
        LDRSH   r5, [r2,#18]            @ r5 = icur[1]
        LDRSH   r10,[r3]                @ r10= obuf[0]
        MOV     r6, r6, ASR #16         @ r6 = tmp1 >>= 16
        SUB     r5, r5, r7, ASR #16     @ r5 = icur[1] - ilast[1]
        MLA     r7, r4, r5, r7  @ r7 = (icur[1]-ilast[1])*opos_frac+ilast[1]

        LDRSH   r5, [r3,#2]             @ r5 = obuf[1]
        MOV     r7, r7, ASR #16         @ r7 = tmp0 >>= 16
        MUL     r7, r12,r7              @ r7 = tmp0*vol_l
        MUL     r6, r14,r6              @ r6 = tmp1*vol_r

        ADDS    r7, r7, r10, LSL #16    @ r7 = obuf[0]<<16 + tmp0*vol_l
        MOV     r4, #0
        RSCVS   r7, r4, #0x80000000     @ Clamp r7
        ADDS    r6, r6, r5, LSL #16     @ r6 = obuf[1]<<16 + tmp1*vol_r
        RSCVS   r6, r4, #0x80000000     @ Clamp r6

        MOV     r7, r7, LSR #16         @ Shift back to halfword
        MOV     r6, r6, LSR #16         @ Shift back to halfword

        LDR     r5, [r2,#12]            @ r5 = opos_inc
        STRH    r7, [r3],#2             @ Store output value
        STRH    r6, [r3],#2             @ Store output value
        SUBS    r11, r11,#1             @ osamp--
        BLE     LinearRate_S_end        @ and loop

        ADDS    r8, r8, r5              @ r8 = opos += opos_inc
        BLT     LinearRate_S_part2
        B       LinearRate_S_loop
LinearRate_S_end:
        ADD     r13,r13,#8
        STMIA   r2,{r0,r1,r8}
        MOV     r0, r3                  @ return obuf
        LDMFD   r13!,{r4-r11,PC}
LinearRate_S_read:
        ADD     r0, r2, #28             @ r0 = inPtr = inBuf
        STMFD   r13!,{r0,r2-r3,r12,r14}

        MOV     r1, r0                  @ r1 = inBuf
        LDR     r0, [r13,#20]           @ r0 = AudioStream & input (20 = 4*5)
        MOV     r2, #512                @ r2 = ARRAYSIZE(inBuf)

        @ Calling back into C++ here. WinCE is fairly easy about such things
        @ but other OS are more awkward. r9 is preserved for Symbian, and
        @ we have 2+9+5 = 16 things on the stack (an even number).
        MOV     r14,PC
        LDR     PC,[r13,#24]            @ inLen = input.readBuffer(inBuf,512) (24 = 4*6)
        SUBS    r1, r0, #2              @ r1 = inLen-2
        LDMFD   r13!,{r0,r2-r3,r12,r14}
        BLT     LinearRate_S_end
        B       LinearRate_S_read_return

_ARM_LinearRate_R:
        @ r0 = AudioStream &input
        @ r1 = input.readBuffer
        @ r2 = input->sr
        @ r3 = obuf
        @ <> = osamp
        @ <> = vol_l
        @ <> = vol_r
        MOV     r12,r13
        STMFD   r13!,{r0-r1,r4-r11,r14}
        LDMFD   r12,{r11,r12,r14}       @ r11= osamp
                                        @ r12= vol_l
                                        @ r14= vol_r
        LDMIA   r2,{r0,r1,r8}           @ r0 = inPtr
                                        @ r1 = inLen
                                        @ r8 = opos
        CMP     r11,#0                  @ if (osamp <= 0)
        BLE     LinearRate_R_end        @   bale
        ORR     r12,r12,r12,LSL #8      @ r12= vol_l as 16 bits
        ORR     r14,r14,r14,LSL #8      @ r14= vol_r as 16 bits
        CMP     r1,#0
        BGT     LinearRate_R_part2

        @ part1 - read input samples
LinearRate_R_loop:
        SUBS    r1, r1, #2              @ r1 = inLen -= 2
        BLT     LinearRate_R_read
LinearRate_R_read_return:
        LDR     r10,[r2, #16]           @ r10= icur[0,1]
        LDRSH   r5, [r0],#2             @ r5 = tmp0 = *inPtr++
        LDRSH   r6, [r0],#2             @ r6 = tmp1 = *inPtr++
        SUBS    r8, r8, #65536          @ r8 = opos--
        STRH    r10,[r2,#22]            @      ilast[0] = icur[0]
        MOV     r10,r10,LSR #16
        STRH    r10,[r2,#26]            @      ilast[1] = icur[1]
        STRH    r5, [r2,#16]            @      icur[0] = tmp0
        STRH    r6, [r2,#18]            @      icur[1] = tmp1
        BGE     LinearRate_R_loop

        @ part2 - form output samples
LinearRate_R_part2:
        @ We are guaranteed that opos < 0 here
        LDR     r6, [r2,#20]            @ r6 = ilast[0]<<16 + 32768
        LDRSH   r5, [r2,#16]            @ r5 = icur[0]
        MOV     r4, r8, LSL #16
        MOV     r4, r4, LSR #16
        SUB     r5, r5, r6, ASR #16     @ r5 = icur[0] - ilast[0]
        MLA     r6, r4, r5, r6  @ r6 = (icur[0]-ilast[0])*opos_frac+ilast[0]

        LDR     r7, [r2,#24]            @ r7 = ilast[1]<<16 + 32768
        LDRSH   r5, [r2,#18]            @ r5 = icur[1]
        LDRSH   r10,[r3,#2]             @ r10= obuf[1]
        MOV     r6, r6, ASR #16         @ r6 = tmp1 >>= 16
        SUB     r5, r5, r7, ASR #16     @ r5 = icur[1] - ilast[1]
        MLA     r7, r4, r5, r7  @ r7 = (icur[1]-ilast[1])*opos_frac+ilast[1]

        LDRSH   r5, [r3]                @ r5 = obuf[0]
        MOV     r7, r7, ASR #16         @ r7 = tmp0 >>= 16
        MUL     r7, r12,r7              @ r7 = tmp0*vol_l
        MUL     r6, r14,r6              @ r6 = tmp1*vol_r

        ADDS    r7, r7, r10, LSL #16    @ r7 = obuf[1]<<16 + tmp0*vol_l
        MOV     r4, #0
        RSCVS   r7, r4, #0x80000000     @ Clamp r7
        ADDS    r6, r6, r5, LSL #16     @ r6 = obuf[0]<<16 + tmp1*vol_r
        RSCVS   r6, r4, #0x80000000     @ Clamp r6

        MOV     r7, r7, LSR #16         @ Shift back to halfword
        MOV     r6, r6, LSR #16         @ Shift back to halfword

        LDR     r5, [r2,#12]            @ r5 = opos_inc
        STRH    r6, [r3],#2             @ Store output value
        STRH    r7, [r3],#2             @ Store output value
        SUBS    r11, r11,#1             @ osamp--
        BLE     LinearRate_R_end        @ and loop

        ADDS    r8, r8, r5              @ r8 = opos += opos_inc
        BLT     LinearRate_R_part2
        B       LinearRate_R_loop
LinearRate_R_end:
        ADD     r13,r13,#8
        STMIA   r2,{r0,r1,r8}
        MOV     r0, r3                  @ return obuf
        LDMFD   r13!,{r4-r11,PC}
LinearRate_R_read:
        ADD     r0, r2, #28             @ r0 = inPtr = inBuf
        STMFD   r13!,{r0,r2-r3,r12,r14}

        MOV     r1, r0                  @ r1 = inBuf
        LDR     r0, [r13,#20]           @ r0 = AudioStream & input (20 = 4*5)
        MOV     r2, #512                @ r2 = ARRAYSIZE(inBuf)

        @ Calling back into C++ here. WinCE is fairly easy about such things
        @ but other OS are more awkward. r9 is preserved for Symbian, and
        @ we have 2+9+5 = 16 things on the stack (an even number).
        MOV     r14,PC
        LDR     PC,[r13,#24]            @ inLen = input.readBuffer(inBuf,512) (24 = 4*6)
        SUBS    r1, r0, #2              @ r1 = inLen-2
        LDMFD   r13!,{r0,r2-r3,r12,r14}
        BLT     LinearRate_R_end
        B       LinearRate_R_read_return
