@ ScummVM - Graphic Adventure Engine
@
@ ScummVM is the legal property of its developers, whose names
@ are too numerous to list here. Please refer to the COPYRIGHT
@ file distributed with this source distribution.
@
@ This program is free software; you can redistribute it and/or
@ modify it under the terms of the GNU General Public License
@ as published by the Free Software Foundation; either version 2
@ of the License, or (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program; if not, write to the Free Software
@ Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
@
@ @author Robin Watts (robin@wss.co.uk)

	@ For 16 source pixels 0123456789ABCDEF, we want to produce 11 output
	@ pixels.

	@0000000000011111111111222222222223333333333344444444444555555555
	@<------||------><------||------><------||------><------||------>

	@5566666666666777777777778888888888899999999999AAAAAAAAAAABBBBBBB
	@<------||------><------||------><------||------><------||------>

	@BBBBCCCCCCCCCCCDDDDDDDDDDDEEEEEEEEEEEFFFFFFFFFFF
	@<------||------><------||------><------||------>

	@ So, use the following weights (approximately right)

	@ d0 = (3*s0 + 1*s1)>>2                Every source pixel constitutes
	@ d1 = (2*s1 + 2*s2)>>2                3/4 of a destination pixel,
	@ d2 = (1*s2 + 3*s3)>>2                except for s4,s5,sA and sB which
	@ d3 = (2*s4 + 2*s5)>>2                constitute 1/2 each.
	@ d4 = (3*s6 + 1*s7)>>2
	@ d5 = (2*s7 + 2*s8)>>2
	@ d6 = (1*s8 + 3*s9)>>2
	@ d7 = (2*sA + 2*sB)>>2
	@ d8 = (3*sC + 1*sD)>>2
	@ d9 = (2*sD + 2*sE)>>2
	@ dA = (1*sE + 3*sF)>>2

	.text

	.global	SmartphoneLandscapeARM

	@ scales a width x height block of 16bpp pixels from srcPtr to dstPtr,
	@ scaling each scanline down by 11/16ths. Every 8th scanline is dropped
	@ srcPitch and dstPitch identify how to reach subsequent lines.
	@ mask allows for one routine to do both 565 and 565 formats.

SmartphoneLandscapeARM:
	@ r0 = srcPtr
	@ r1 = srcSpan
	@ r2 = dstPtr
	@ r3 = dstSpan
	@ <> = width
	@ <> = height
	@ <> = mask
	MOV	r12,r13
	STMFD	r13!,{r4-r11,r14}
	LDMFD	r12,{r4,r5,r11}		@ r4 = width
					@ r5 = height
					@ r11= mask
	MOV	r7, #7			@ r7 = line
	SUB	r8, r1, r4, LSL #1	@ r8 = srcSpan - width*2
y_loop:
	MOV	r6, r4			@ r6 = i
	MOV	r9, r2			@ r9 = dstPtr
x_loop:
	LDRH	r14,[r0],#2		@ r14 = s0
	LDRH	r12,[r0],#2		@ r12 = s1
	LDRH	r10,[r0],#2		@ r10 = s2
	ORR	r14,r14,r14,LSL #16	@ r14 = s0s0
	ORR	r12,r12,r12,LSL #16	@ r12 = s1s1
	AND	r14,r14,r11		@ r14 = s0 as g_b_r
	AND	r12,r12,r11		@ r12 = s1 as g_b_r
	ADD	r14,r14,r14,LSL #1	@ r14 = s0*3 as g_b_r
	ORR	r10,r10,r10,LSL #16	@ r10 = s2s2
	ADD	r14,r14,r12		@ r14 = (s0*3 + s1) as g_b_r
	AND	r10,r10,r11		@ r10 = s2 as g_b_r
	AND	r14,r11,r14,LSR #2	@ r14 = d0 as g_b_r
	ORR	r14,r14,r14,LSR #16	@ r14 = d0
	STRH	r14,[r9],#2		@ store d0
	ADD	r12,r12,r10		@ r12 = (s1 + s2) as g_b_r
	LDRH	r14,[r0],#2		@ r14 = s3
	AND	r12,r11,r12,LSR #1	@ r12 = d1 as g_b_r
	ORR	r12,r12,r12,LSR #16	@ r12 = d1
	STRH	r12,[r9],#2		@ store d1
	ORR	r14,r14,r14,LSL #16	@ r14 = s3s3
	AND	r14,r14,r11		@ r14 = s3 as g_b_r
	ADD	r10,r10,r14		@ r10 = (s2 + s3) as g_b_r
	ADD	r10,r10,r14,LSL #1	@ r10 = (s2 + s3*3) as g_b_r
	LDRH	r14,[r0],#2		@ r14 = s4
	LDRH	r12,[r0],#2		@ r12 = s5
	AND	r10,r11,r10,LSR #2	@ r10 = d2 as g_b_r
	ORR	r10,r10,r10,LSR #16	@ r10 = d2
	STRH	r10,[r9],#2		@ store d2
	ORR	r14,r14,r14,LSL #16	@ r14 = s4s4
	ORR	r12,r12,r12,LSL #16	@ r12 = s5s5
	AND	r14,r14,r11		@ r14 = s4 as g_b_r
	AND	r12,r12,r11		@ r12 = s5 as g_b_r
	ADD	r14,r14,r12		@ r14 = (s4 + s5) as g_b_r
	LDRH	r12,[r0],#2		@ r12 = s6
	LDRH	r10,[r0],#2		@ r10 = s7
	AND	r14,r11,r14,LSR #1	@ r14 = d3 as g_b_r
	ORR	r14,r14,r14,LSR #16	@ r14 = d3
	STRH	r14,[r9],#2		@ store d3
	ORR	r12,r12,r12,LSL #16	@ r12 = s6s6
	ORR	r10,r10,r10,LSL #16	@ r10 = s7s7
	LDRH	r14,[r0],#2		@ r14 = s8
	AND	r12,r12,r11		@ r12 = s6 as g_b_r
	AND	r10,r10,r11		@ r10 = s7 as g_b_r
	ORR	r14,r14,r14,LSL #16	@ r14 = s8s8
	ADD	r12,r12,r12,LSL #1	@ r12 = 3*s6 as g_b_r
	AND	r14,r14,r11		@ r14 = s8 as g_b_r
	ADD	r12,r12,r10		@ r12 = (3*s6+s7) as g_b_r
	AND	r12,r11,r12,LSR #2	@ r12 = d4 as g_b_r
	ORR	r12,r12,r12,LSR #16	@ r12 = d4
	STRH	r12,[r9],#2		@ store d4
	ADD	r10,r10,r14		@ r10 = (s7+s8) as g_b_r
	AND	r10,r11,r10,LSR #1	@ r10 = d5 as g_b_r
	LDRH	r12,[r0],#2		@ r12 = s9
	ORR	r10,r10,r10,LSR #16	@ r10 = d5
	STRH	r10,[r9],#2		@ store d5
	ORR	r12,r12,r12,LSL #16	@ r12 = s9s9
	AND	r12,r12,r11		@ r12 = s9 as g_b_r
	ADD	r12,r12,r12,LSL #1	@ r12 = s9*3 as g_b_r
	ADD	r12,r12,r14		@ r12 = (s8+s9*3) as g_b_r
	AND	r12,r11,r12,LSR #2	@ r12 = d6 as g_b_r
	LDRH	r14,[r0],#2		@ r14 = sA
	LDRH	r10,[r0],#2		@ r10 = sB
	ORR	r12,r12,r12,LSR #16	@ r12 = d6
	STRH	r12,[r9],#2		@ store d6
	ORR	r14,r14,r14,LSL #16	@ r14 = sAsA
	ORR	r10,r10,r10,LSL #16	@ r10 = sBsB
	LDRH	r12,[r0],#2		@ r12 = sC
	AND	r14,r14,r11		@ r14 = sA as g_b_r
	AND	r10,r10,r11		@ r10 = sB as g_b_r
	ORR	r12,r12,r12,LSL #16	@ r12 = sCsC
	ADD	r14,r14,r10		@ r14 = (sA + sB) as g_b_r
	LDRH	r10,[r0],#2		@ r10 = sD
	AND	r14,r11,r14,LSR #1	@ r14 = d7 as g_b_r
	AND	r12,r12,r11		@ r12 = sC as g_b_r
	ORR	r14,r14,r14,LSR #16	@ r14 = d7
	ORR	r10,r10,r10,LSL #16	@ r10 = sDsD
	STRH	r14,[r9],#2		@ store d7
	AND	r10,r10,r11		@ r10 = sD as g_b_r
	ADD	r12,r12,r12,LSL #1	@ r12 = 3*sC as g_b_r
	LDRH	r14,[r0],#2		@ r14 = sE
	ADD	r12,r12,r10		@ r12 = (3*sC+sD) as g_b_r
	AND	r12,r11,r12,LSR #2	@ r12 = d8 as g_b_r
	ORR	r14,r14,r14,LSL #16	@ r14 = sEsE
	ORR	r12,r12,r12,LSR #16	@ r12 = d8
	AND	r14,r14,r11		@ r14 = sE as g_b_r
	STRH	r12,[r9],#2		@ store d8
	ADD	r10,r10,r14		@ r10 = (sD+sE) as g_b_r
	LDRH	r12,[r0],#2		@ r12 = sF
	AND	r10,r11,r10,LSR #1	@ r10 = d9 as g_b_r
	ORR	r10,r10,r10,LSR #16	@ r10 = d9
	STRH	r10,[r9],#2		@ store d9
	ORR	r12,r12,r12,LSL #16	@ r12 = sFsF
	AND	r12,r12,r11		@ r12 = sF as g_b_r
	ADD	r12,r12,r12,LSL #1	@ r12 = 3*sF as g_b_r
	ADD	r12,r12,r14		@ r12 = (sE+3*sF) as g_b_r
	AND	r12,r11,r12,LSR #2	@ r12 = dA as g_b_r
	ORR	r12,r12,r12,LSR #16	@ r12 = dA
	SUBS	r6,r6,#16		@ width -= 16
	STRH	r12,[r9],#2		@ store dA
	BGT	x_loop

	ADD	r0, r0, r8		@ srcPtr += srcSpan
	ADD	r2, r2, r3		@ dstPtr += dstSpan
	SUBS	r7, r7, #1
	ADDEQ	r0, r0, r1
	MOVEQ	r7, #7
	SUBEQ	r5, r5, #1
	SUBS	r5, r5, #1
	BGT	y_loop

	LDMFD	r13!,{r4-r11,PC}
