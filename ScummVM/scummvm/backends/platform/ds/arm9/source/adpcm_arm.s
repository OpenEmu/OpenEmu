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
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program@ if not, write to the Free Software
@ Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
@
@ @author Robin Watts (robin@wss.co.uk)

	.global	ARM_adpcm
	.section .itcm,"ax", %progbits
	.align 2
	.code 32

	@ ARM implementation of inner ADPCM decoding loop
	@
	@ C prototype would be:
	@
	@ extern "C" void ARM_adpcm(int *block,
	@                           int  len,
	@                           int  stepTableIndex,
	@                           int  firstSample,
	@                           s16 *decompressionBuffer)
ARM_adpcm
	@ r0 = block
	@ r1 = len
	@ r2 = stepTableIndex
	@ r3 = firstSample
	@ <> = decompressionBuffer
	STMFD	r13!,{r4-r11,r14}
	LDR	r4,[r13,#4*9]

	LDR	r12,[r0],#4	@ r12 = word = block[r>>3]
	MOV	r14,#0
	ADR	r11,stepTab
	ADR	r7, indexTab
	MOV	r2, r2, LSL #1	@ r2 = stepTableIndex*2 - hold it doubled
	STR	r3, [r4],#2	@ decompBuff[0] = firstSample
	MOV	r3, r3, LSL #16	@ r3 = firstSample<<16 (for saturated maths)
	SUBS	r1, r1, #1
	BLE	end
loop:
	LDRH	r10,[r11,r2]	@ r10 = stepTab[stepTableIndex]
	TST	r12,#4		@ if ((offset & 4) == 0)
	MOVEQ	r9, #0		@ 	r9 = diff = 0
	MOVNE	r9, r10		@ else	r9 = diff = stepTab[stepTableIndex]

	TST	r12,#2		@ if (offset & 2)
	ADDNE	r9, r9, r10,ASR #1	@ 	diff += r10>>1

	TST	r12,#1		@ if (offset & 1)
	ADDNE	r9, r9, r10,ASR #2	@ 	diff += r10>>2

	ADD	r9, r9, r10,ASR #3	@ diff += r10>>3

	TST	r12,#8		@ if (offset & 8
	RSBNE	r9, r9, r10

	ADDS	r3, r3, r9	@ r3 = newSample = prevSample+diff
	RSCVS	r3, r3, #1<<31	@ r3 = clip(newSample)
	MOV	r8, r3, LSR #16

	AND	r6, r12,#4	@ r6 = offset
	LDRB	r6, [r7,r6]	@ r6 = indexTab[offset]
	MOV	r12,r12,LSR #4
	STRH	r8, [r4],#2

	ADDS	r2,r2,r6,LSL #1	@ r2 = stepTableIndex += indexTab[offset]
	MOVLT	r2,#0
	CMP	r2,#88*2
	MOVGT	r2,#88*2
	SUBS	r1,r1,#1
	BEQ	end
	SUBS	r14,r14,#0x10000000
	LDREQ	r12,[r0],#4
	B	loop

end:
	LDMFD	r13!,{r4-r11,PC}

stepTab:
	DCW	7, 8, 9, 10, 11, 12, 13, 14,
	DCW	16, 17, 19, 21, 23, 25, 28, 31,
	DCW	34, 37, 41, 45, 50, 55, 60, 66,
	DCW	73, 80, 88, 97, 107, 118, 130, 143,
	DCW	157, 173, 190, 209, 230, 253, 279, 307,
	DCW	337, 371, 408, 449, 494, 544, 598, 658,
	DCW	724, 796, 876, 963, 1060, 1166, 1282, 1411,
	DCW	1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	DCW	3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	DCW	7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	DCW	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	DCW	32767
indexTab:
	DCB -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
