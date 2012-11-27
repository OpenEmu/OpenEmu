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

        .text

        .global scale2x_8_arm
        .global scale2x_16_arm
        .global scale2x_32_arm

        @ r0 = dst0
        @ r1 = dst1
        @ r2 = src
        @ r3 = src_prev
        @ r4 = src_next
        @ r5 = len


        @ We hold:       r10            B
        @            r8  r14 r7       D E F
        @                r12            H
scale2x_8_arm:
        STMFD   r13!,{r4-r5,r7-r8,r10-r11,r14}
        LDR     r4, [r13,#4*7]
        LDR     r5, [r13,#4*8]

        LDRB    r14,[r2], #1
        CMP     r3, #0          @ Set NE
        @ Stall on Xscale
        MOV     r8, r14
        B       loop8
same8:
        STRB    r14,[r0], #1
        STRB    r14,[r0], #1
        STRB    r14,[r1], #1
        STRB    r14,[r1], #1
        SUBS    r5, r5, #1
        MOV     r8, r14
        MOV     r14,r7
        BLT     end8
loop8:
        LDRNEB  r7, [r3], #1    @ As long as we aren't on the last pixel
        LDRB    r10,[r2], #1
        LDRB    r12,[r4], #1
        @ Stall on Xscale
        CMP     r7, r8
        CMPNE   r10,r12
        BEQ     same8
        CMP     r8, r10
        STREQB  r8, [r0], #1
        STRNEB  r14,[r0], #1
        CMP     r10,r7
        STREQB  r7, [r0], #1
        STRNEB  r14,[r0], #1
        CMP     r8, r12
        STREQB  r8, [r1], #1
        STRNEB  r14,[r1], #1
        CMP     r12,r7
        STREQB  r7, [r1], #1
        STRNEB  r14,[r1], #1

        SUBS    r5, r5, #1
        MOV     r8, r14
        MOV     r14,r7
        BGE     loop8
end8:

        LDMFD   r13!,{r4-r5,r7-r8,r10-r11,PC}

scale2x_16_arm:
        STMFD   r13!,{r4-r5,r7-r8,r10-r11,r14}
        LDR     r4, [r13,#4*7]
        LDR     r5, [r13,#4*8]

        LDRH    r14,[r3], #2
        CMP     r3, #0          @ Set NE
        @ Stall on Xscale
        MOV     r8, r14
        B       loop16
same16:
        STRH    r14,[r0], #2
        STRH    r14,[r0], #2
        STRH    r14,[r1], #2
        STRH    r14,[r1], #2
        SUBS    r5, r5, #1
        MOV     r8, r14
        MOV     r14,r7
        BLT     end16
loop16:
        LDRNEH  r7, [r3], #2    @ As long as we aren't on the last pixel
        LDRH    r10,[r2], #2
        LDRH    r12,[r4], #2
        @ Stall on Xscale
        CMP     r7, r8
        CMPNE   r10,r12
        BEQ     same16
        CMP     r8, r10
        STREQH  r8, [r0], #2
        STRNEH  r14,[r0], #2
        CMP     r10,r7
        STREQH  r7, [r0], #2
        STRNEH  r14,[r0], #2
        CMP     r8, r12
        STREQH  r8, [r1], #2
        STRNEH  r14,[r1], #2
        CMP     r12,r7
        STREQH  r7, [r1], #2
        STRNEH  r14,[r1], #2

        SUBS    r5, r5, #1
        MOV     r8, r14
        MOV     r14,r7
        BGE     loop16
end16:

        LDMFD   r13!,{r4-r5,r7-r8,r10-r11,PC}

scale2x_32_arm:
        STMFD   r13!,{r4-r5,r7-r8,r10-r11,r14}
        LDR     r4, [r13,#4*7]
        LDR     r5, [r13,#4*8]

        LDR     r14,[r3], #4
        CMP     r3, #0          @ Set NE
        @ Stall on Xscale
        MOV     r8, r14
        B       loop32
same32:
        STR     r14,[r0], #4
        STR     r14,[r0], #4
        STR     r14,[r1], #4
        STR     r14,[r1], #4
        SUBS    r5, r5, #1
        MOV     r8, r14
        MOV     r14,r7
        BLT     end32
loop32:
        LDRNE   r7, [r3], #4    @ As long as we aren't on the last pixel
        LDR     r10,[r2], #4
        LDR     r12,[r4], #4
        @ Stall on Xscale
        CMP     r7, r8
        CMPNE   r10,r12
        BEQ     same32
        CMP     r8, r10
        STREQ   r8, [r0], #4
        STRNE   r14,[r0], #4
        CMP     r10,r7
        STREQ   r7, [r0], #4
        STRNE   r14,[r0], #4
        CMP     r8, r12
        STREQ   r8, [r1], #4
        STRNE   r14,[r1], #4
        CMP     r12,r7
        STREQ   r7, [r1], #4
        STRNE   r14,[r1], #4

        SUBS    r5, r5, #1
        MOV     r8, r14
        MOV     r14,r7
        BGE     loop32
end32:

        LDMFD   r13!,{r4-r5,r7-r8,r10-r11,PC}
