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
@ This file, provides an ARM optimised version of sections of codec47.cpp.
@ The algorithm is essentially the same as that within codec47.cpp
@ so to understand this file you should understand codec47.cpp first.

	.text

	.global	_ARM_Smush_decode2

_ARM_Smush_decode2:
	@ r0 = dst
	@ r1 = src
	@ r2 = width
	@ r3 = height
	@ r4 = param
	@ <> = _table
	@ <> = _tableBig
	@ <> = _offset1
	@ <> = _offset2
	@ <> = _tableSmall
	STMFD	r13!,{r2,r4-r11,R14}

        LDR	r4,[r13,#40]	@ r4 = param (40 = (9+1)*4)
	@ stall
	@ stall
	SUB	r4,r4,#0xF8

	@ r0 = dst
	@ r1 = _d_src
	@ r2 = _d_pitch
	@ r3 = height
	@ r4 = param
	ADD	r7,r2,#7		@ r14 = bw
	MOV	r7,r7,LSR #3
y_loop:
x_loop:
	@ LEVEL 1
	LDRB	r6,[r1],#1		@ r6 = *_d_src++
	@ stall
	@ stall
	CMP	r6,#0xF8
	BLT	level1codeSMALL
	CMP	r6,#0xFC
	BLT	level1codeMID
	BEQ	level1codeFC
	CMP	r6,#0xFE
	BGT	level1codeFF
	BEQ	level1codeFE
level1codeFD:
	LDRB	r6,[r1],#1		@ r6 = tmp = *_d_src++
	LDR	r8,[r13,#48]		@ r8 = _tableBig (48 = (9+1+2)*4)
	@ stall
	ADD	r12,r6,r6,LSL #1	@ r12= tmp*3
	ADD	r6,r6,r12,LSL #5	@ r6 = tmp*97
	ADD	r8,r8,r6,LSL #2		@ r8 = _tableBig + tmp*388
	LDRB	r9,[r8,#384]		@ r9 = l = tmp_ptr[384]
	LDRB	r6,[r1],#1		@ r6 = val = *_d_src++
	ADD	r12,r8,#384		@ r12= &tmp_ptr[384]
	@ I don''t really believe the next 2 lines are necessary, but...
	CMP	r9,#0
	BEQ	level1codeFD_over1
level1codeFD_loop1:
	LDRB	r10,[r8],#1
	LDRB	r11,[r8],#1
	SUBS	r9,r9,#1
	ADD	r10,r10,r0
	STRB	r6,[r10,r11,LSL #8]	@ *(_d_dst + (*tmp_ptr2++)) = val
	BGT	level1codeFD_loop1
level1codeFD_over1:
	LDRB	r9,[r12,#1]		@ r9 = l = tmp_ptr[385]
	LDRB	r6,[r1],#1		@ r6 = val = *_d_src++
	SUB	r12,r12,#256		@ r12= &tmp_ptr[128] (256 = 384-128)
	@ I don''t really believe the next 2 lines are necessary, but...
	CMP	r9,#0
	BEQ	level1codeFD_over2
level1codeFD_loop2:
	LDRB	r10,[r12],#1
	LDRB	r11,[r12],#1
	SUBS	r9,r9,#1
	ADD	r10,r10,r0
	STRB	r6,[r10,r11,LSL #8]	@ *(_d_dst + (*tmp_ptr2++)) = val
	BGT	level1codeFD_loop2
level1codeFD_over2:
level1_end:

	ADD	r0,r0,#8
	SUBS	r7,r7,#1
	BGT	x_loop

	ADD	r7,r2,#7
	MOV	r7,r7,LSR #3
	ADD	r0,r0,r2,LSL #3
	SUB	r0,r0,r7,LSL #3	@ r0 = dst += next_line
	SUBS	r3,r3,#8	@ if (--bh > 0)
	BGT	y_loop		@   loop back

	LDMFD	r13!,{r2,r4-r11,PC}

level1codeSMALL:
	LDR	r8,[r13,#44]		@ r8 = _table (44 = (9+1+1)*4)
	LDR	r9,[r13,#52]		@ r9 = _offset1 (52 = (9+1+3)*4)
	MOV	r6,r6,LSL #1		@ r6 = code<<1
	LDRSH	r8,[r8,r6]		@ tmp2 = _table[code]
level1codeFC:
	@ EQ => FC
	LDREQ	r9,[r13,#56]		@ r9 = _offset2 (56 = (9+1+4)*4)
	MOVEQ	r8,#0
	SUB	r11,r2,#7		@ r11 = _d_pitch-7
	ADD	r9,r9,r0		@ tmp2 = _d_dst+_offset
	ADD	r8,r8,r9		@ tmp2 = _d_dst+_table[code]+_offset
					@ r8 = &_dst[tmp2]
	MOV	r12,#8
level1codeSMALL_loop:
	LDRB	r5, [r8],#1		@ r5  = d_dst[tmp2]
	LDRB	r6, [r8],#1		@ r10 = d_dst[tmp2]
	LDRB	r9, [r8],#1		@ r10 = d_dst[tmp2]
	LDRB	r10,[r8],#1		@ r10 = d_dst[tmp2]
	STRB	r5, [r0],#1		@ d_dst[0] = r5
	STRB	r6, [r0],#1		@ d_dst[1] = r6
	STRB	r9, [r0],#1		@ d_dst[2] = r9
	STRB	r10,[r0],#1		@ d_dst[3] = r10
	LDRB	r5, [r8],#1		@ r5  = d_dst[tmp2]
	LDRB	r6, [r8],#1		@ r10 = d_dst[tmp2]
	LDRB	r9, [r8],#1		@ r10 = d_dst[tmp2]
	LDRB	r10,[r8],r11		@ r10 = d_dst[tmp2]
	STRB	r5, [r0],#1		@ d_dst[4] = r5
	STRB	r6, [r0],#1		@ d_dst[5] = r6
	STRB	r9, [r0],#1		@ d_dst[6] = r9
	STRB	r10,[r0],r11		@ d_dst[7] = r10   d_dst += d_pitch
	SUBS	r12,r12,#1
	BGT	level1codeSMALL_loop
	SUB	r0,r0,r2,LSL #3		@ revert d_dst
	B	level1_end

level1codeMID:
	@ LT => F8<=code<FC case
	@ EQ => FE case
	LDRB	r6,[r4,r6]		@ r6 = t = _paramPtr[code]
level1codeFE:
	LDREQB	r6,[r1],#1		@ r6 = t = *_d_src++
	MOV	r12,#8
	SUB	r11,r2,#7		@ r11 = _d_pitch-7
level1codeMID_loop:
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],r11
	SUBS	r12,r12,#1
	BGT	level1codeMID_loop
	SUB	r0,r0,r2,LSL #3		@ revert d_dst
	B	level1_end

level1codeFF:
	BL	level2
	ADD	r0,r0,#4
	BL	level2
	ADD	r0,r0,r2,LSL #2
	SUB	r0,r0,#4
	BL	level2
	ADD	r0,r0,#4
	BL	level2
	SUB	r0,r0,#4
	SUB	r0,r0,r2,LSL #2
	B	level1_end

level2:
	@ r0 = _d_dst
	@ r1 = _d_src
	@ r2 = _d_pitch
	@ r3 = PRESERVE
	@ r4 = param
	@ r7 = PRESERVE
	@ r14= return address
	LDRB	r6,[r1],#1		@ r6 = *_d_src++
	@ stall
	@ stall
	CMP	r6,#0xF8
	BLT	level2codeSMALL
	CMP	r6,#0xFC
	BLT	level2codeMID
	BEQ	level2codeFC
	CMP	r6,#0xFE
	BGT	level2codeFF
	BEQ	level2codeFE
level2codeFD:
	LDRB	r6,[r1],#1		@ r6 = tmp = *_d_src++
	LDR	r8,[r13,#60]		@ r8 = _tableSmall (60 = (9+1+5)*4)
	@ stall
	@ stall
	ADD	r8,r8,r6,LSL #7		@ r8 = _tableSmall + tmp*128
	LDRB	r9,[r8,#96]		@ r9 = l = tmp_ptr[96]
	LDRB	r6,[r1],#1		@ r6 = val = *_d_src++
	ADD	r12,r8,#32		@ r12 = tmp_ptr + 32
	@ I don''t really believe the next 2 lines are necessary, but...
	CMP	r9,#0
	BEQ	level2codeFD_over1
level2codeFD_loop1:
	LDRB	r10,[r8],#1
	LDRB	r11,[r8],#1
	SUBS	r9,r9,#1
	ADD	r10,r10,r0
	STRB	r6,[r10,r11,LSL #8]	@ *(_d_dst + (*tmp_ptr2++)) = val
	BGT	level2codeFD_loop1
level2codeFD_over1:
	LDRB	r9,[r12,#65]		@ r9 = l = tmp_ptr[97] (65 = 97-32)
	LDRB	r6,[r1],#1		@ r6 = val = *_d_src++
	@ I don''t really believe the next 2 lines are necessary, but...
	CMP	r9,#0
	MOVEQ	PC,R14
level2codeFD_loop2:
	LDRB	r10,[r12],#1
	LDRB	r11,[r12],#1
	SUBS	r9,r9,#1
	ADD	r10,r10,r0
	STRB	r6,[r10,r11,LSL #8]	@ *(_d_dst + (*tmp_ptr2++)) = val
	BGT	level2codeFD_loop2

	MOV	PC,R14

level2codeSMALL:
	LDR	r8,[r13,#44]		@ r8 = _table (44 = (9+1+1)*4)
	LDR	r9,[r13,#52]		@ r9 = _offset1 (52 = (9+1+3)*4)
	MOV	r6,r6,LSL #1		@ r6 = code<<1
	LDRSH	r8,[r8,r6]		@ tmp2 = _table[code]
level2codeFC:
	@ EQ => FC
	LDREQ	r9,[r13,#56]		@ r9 = _offset2 (56 = (9+1+4)*4)
	MOVEQ	r8,#0
	SUB	r11,r2,#3		@ r11 = _d_pitch-3
	ADD	r9,r9,r0		@ tmp2 = _d_dst + _table[code]
	ADD	r8,r8,r9		@ tmp2 = _d_dst+_table[code]+_offset1
					@ r8 = &_dst[tmp2]
	MOV	r12,#4
level2codeSMALL_loop:
	LDRB	r5, [r8],#1		@ r5  = d_dst[tmp2]
	LDRB	r6, [r8],#1		@ r10 = d_dst[tmp2]
	LDRB	r9, [r8],#1		@ r10 = d_dst[tmp2]
	LDRB	r10,[r8],r11		@ r10 = d_dst[tmp2]
	STRB	r5, [r0],#1		@ d_dst[4] = r5
	STRB	r6, [r0],#1		@ d_dst[5] = r6
	STRB	r9, [r0],#1		@ d_dst[6] = r9
	STRB	r10,[r0],r11		@ d_dst[7] = r10   d_dst += d_pitch
	SUBS	r12,r12,#1
	BGT	level2codeSMALL_loop
	SUB	r0,r0,r2,LSL #2		@ revert d_dst
	MOV	PC,R14

level2codeMID:
	@ LT => F8<=code<FC case
	@ EQ => FE case
	LDRB	r6,[r4,r6]		@ r6 = t = _paramPtr[code]
level2codeFE:
	LDREQB	r6,[r1],#1		@ r6 = t = *_d_src++
	MOV	r12,#4
	SUB	r11,r2,#3		@ r11 = _d_pitch-7
level2codeMID_loop:
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],#1
	STRB	r6,[r0],r11
	SUBS	r12,r12,#1
	BGT	level2codeMID_loop
	SUB	r0,r0,r2,LSL #2		@ revert d_dst
	MOV	PC,R14

level2codeFF:
	MOV	r5,r14
	BL	level3
	ADD	r0,r0,#2
	BL	level3
	ADD	r0,r0,r2,LSL #1
	SUB	r0,r0,#2
	BL	level3
	ADD	r0,r0,#2
	BL	level3
	SUB	r0,r0,#2
	SUB	r0,r0,r2,LSL #1
	MOV	PC,R5

level3:
	@ r0 = _d_dst
	@ r1 = _d_src
	@ r2 = _d_pitch
	@ r3 = PRESERVE
	@ r4 = param
	@ r5 = preserve
	@ r7 = PRESERVE
	@ r14= return address
	LDRB	r6,[r1],#1		@ r6 = code = *_d_src++
	@ stall
	@ stall
	CMP	r6,#0xF8
	BLT	level3codeSMALL
	CMP	r6,#0xFC
	BLT	level3codeMID
	BEQ	level3codeFC
	CMP	r6,#0xFE
	BGT	level3codeFF
level3codeFE:
	LDRB	r6,[r1],#1		@ r6 = t = *_d_src++
level3codeMID:
	@ LT => F8<=code<FC case
	@ EQ => FE case
	LDRLTB	r6,[r4,r6]		@ r6 = t = _paramPtr[code]
	@ stall
	@ stall
	STRB	r6,[r0,#1]
	STRB	r6,[r0],r2
	STRB	r6,[r0,#1]
	STRB	r6,[r0],-r2
	MOV	PC,R14

level3codeFF:
	LDRB	r6,[r1],#1
	LDRB	r9,[r1],#1
	LDRB	r10,[r1],#1
	LDRB	r11,[r1],#1
	STRB	r9, [r0,#1]
	STRB	r6, [r0],r2
	STRB	r11,[r0,#1]
	STRB	r10,[r0],-r2
	MOV	PC,R14

level3codeSMALL:
	LDR	r8,[r13,#44]		@ r8 = _table (44 = (9+1+1)*4)
	LDR	r9,[r13,#52]		@ r9 = _offset1 (52 = (9+1+3)*4)
	MOV	r6,r6,LSL #1		@ r6 = code<<1
	LDRSH	r8,[r8,r6]		@ tmp2 = _table[code]
level3codeFC:
	@ EQ => FC
	LDREQ	r9,[r13,#56]		@ r9 = _offset2 (56 = (9+1+4)*4)
	MOVEQ	r8,#0
	ADD	r9,r9,r0		@ tmp2 = _d_dst+offset
	ADD	r8,r8,r9		@ tmp2 = _d_dst+_table[code]+_offset
					@ r8 = &_dst[tmp2]
	LDRB	r6, [r8,#1]		@ r6 = d_dst[tmp2+1]
	LDRB	r9, [r8],r2		@ r9 = d_dst[tmp2+0]
	LDRB	r10,[r8,#1]		@ r10= d_dst[tmp2+dst+1]
	LDRB	r11,[r8],-r2		@ r11= d_dst[tmp2+dst]
	STRB	r6, [r0,#1]		@ d_dst[1    ] = r6
	STRB	r9, [r0],r2		@ d_dst[0    ] = r9
	STRB	r10,[r0,#1]		@ d_dst[dst+1] = r10
	STRB	r11,[r0],-r2		@ d_dst[dst  ] = r11
	MOV	PC,R14
