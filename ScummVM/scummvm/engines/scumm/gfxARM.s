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

	.text

	.global	_asmDrawStripToScreen
	.global	_asmCopy8Col

	@ ARM implementation of asmDrawStripToScreen.
	@
	@ C prototype would be:
	@
	@ extern "C" void asmDrawStripToScreen(int         height,
	@                                      int         width,
	@                                      byte const *text,
	@                                      byte const *src,
	@                                      byte       *dst,
	@                                      int         vsPitch,
	@                                      int         vsScreenWidth,
	@                                      int         textSurfacePitch);
	@
	@ In addition, we assume that text, src and dst are all word (4 byte)
	@ aligned. This is the same assumption that the old 'inline' version
	@ made.
_asmDrawStripToScreen:
	@ r0 = height
	@ r1 = width
	@ r2 = text
	@ r3 = src
	MOV	r12,r13
	STMFD	r13!,{r4-r11,R14}
	LDMIA	r12,{r4,r5,r6,r7}
	@ r4 = dst
	@ r5 = vsPitch
	@ r6 = vmScreenWidth
	@ r7 = textSurfacePitch

	CMP	r0,#0			@ If height<=0
	MOVLE	r0,#1			@    height=1
	CMP	r1,#4			@ If width<4
	BLT	end			@    return

	SUB	r5,r5,r1		@ vsPitch          -= width
	SUB	r6,r6,r1		@ vmScreenWidth    -= width
	SUB	r7,r7,r1		@ textSurfacePitch -= width
	MOV	r10,#253
	ORR	r10,r10,r10,LSL #8
	ORR	r10,r10,r10,LSL #16	@ r10 = mask
	MOV	r8,#0x7F
	ORR	r8, r8, r8, LSL #8
	ORR	r8, r8, r8, LSL #16	@ r8  = 7f7f7f7f
	STR	r1,[r13,#-4]!		@ Stack width
	B	xLoop

notEntirelyTransparent:
	AND	r14,r9, r8		@ r14  =  mask & 7f7f7f7f
	ADD	r14,r14,r8		@ r14  = (mask & 7f7f7f7f)+7f7f7f7f
	ORR	r14,r14,r9		@ r14 |= mask
	BIC	r14,r14,r8		@ r14 &= 80808080
	ADD	r14,r8, r14,LSR #7	@ r14  = (rx>>7) + 7f7f7f7f
	EOR	r14,r14,r8		@ r14 ^= 7f7f7f7f
	@ So bytes of r14 are 00 where source was matching value,FF otherwise
	BIC	r11,r11,r14
	AND	r12,r12,r14
	ORR	r12,r11,r12
	STR	r12,[r4],#4
	SUBS	r1,r1,#4
	BLE	endXLoop
xLoop:
	LDR	r12,[r2],#4		@ r12 = temp = [text]
	LDR	r11,[r3],#4		@ r11 =        [src]
	@ Stall
	EORS	r9, r12,r10		@ r9  = mask = temp ^ TRANSPARENCY
	BNE	notEntirelyTransparent
	SUBS	r1, r1, #4
	STR	r11,[r4], #4		@ r4 = [dst]
	BGT	xLoop
endXLoop:
	ADD	r2,r2,r7		@ text += textSurfacePitch
	ADD	r3,r3,r5		@ src  += vsPitch
	ADD	r4,r4,r6		@ dst  += vmScreenWidth
	SUBS	r0,r0,#1
	LDRGT	r1,[r13]		@ r14 = width
	BGT	xLoop
	ADD	r13,r13,#4
end:
	LDMFD	r13!,{r4-r11,PC}

	@ ARM implementation of asmCopy8Col
	@
	@ C prototype would be:
	@
	@ extern "C" void asmCopy8Col(byte       *dst,
	@                             int         dstPitch,
	@                             const byte *src,
	@                             int         height,
	@                             uint8       bitdepth);
	@
	@ In addition, we assume that src and dst are both word (4 byte)
	@ aligned. This is the same assumption that the old 'inline' version
	@ made.
_asmCopy8Col:
	@ r0 = dst
	@ r1 = dstPitch
	@ r2 = src
	@ r3 = height
	@ <> = bitdepth (badly named, should be bytedepth, 1 or 2)
	LDR	r12,[r13]
	STR	r14,[r13,#-4]!
	CMP	r12,#1
	BNE	copy8Col16

	SUB	r1,r1,#4
	TST	r3,#1
	ADDNE   r3,r3,#1
	BNE	roll2
yLoop2:
	LDR	r12,[r2],#4
	LDR	r14,[r2],r1
	STR	r12,[r0],#4
	STR	r14,[r0],r1
roll2:
	LDR	r12,[r2],#4
	LDR	r14,[r2],r1
	SUBS	r3,r3,#2
	STR	r12,[r0],#4
	STR	r14,[r0],r1
	BNE	yLoop2

	LDR	PC,[r13],#4

copy8Col16:
	STMFD	r13!,{r4-r5}
	SUB	r1,r1,#12
	TST	r3,#1
	ADDNE   r3,r3,#1
	BNE	roll3
yLoop3:
	LDR	r4, [r2],#4
	LDR	r5, [r2],#4
	LDR	r12,[r2],#4
	LDR	r14,[r2],r1
	STR	r4, [r0],#4
	STR	r5, [r0],#4
	STR	r12,[r0],#4
	STR	r14,[r0],r1
roll3:
	LDR	r4, [r2],#4
	LDR	r5, [r2],#4
	LDR	r12,[r2],#4
	LDR	r14,[r2],r1
	SUBS	r3,r3,#2
	STR	r4, [r0],#4
	STR	r5, [r0],#4
	STR	r12,[r0],#4
	STR	r14,[r0],r1
	BNE	yLoop3

	LDMFD	r13!,{r4,r5,PC}
