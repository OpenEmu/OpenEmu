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

	.global	_ClassicProc3RendererShadowARM



.set	space,			48

.set	_scaleIndexY,	store_r14 + 28
.set	_numStrips,		store_r14 + 24
.set	_palette,		store_r14 + 20
.set	_shadow_table,	store_r14 + 16
.set	_scaleIndexX,	store_r14 + 12
.set	_scaleX,		store_r14 + 8
.set	_height,		store_r14 + 4

.set	store_r14,		space + 36
.set	store_r11,		space + 32
.set	store_r10,		space + 28
.set	store_r9,		space + 24
.set	store_r8,		space + 20
.set	store_r7,		space + 16
.set	store_r6,		space + 12
.set	store_r5,		space + 8
.set	store_r4,		space + 4

.set	src,			48
.set	height,			44
.set	len,			40
.set	v1_shr,			36
.set	v1_skip_width,	32
.set	v1_destptr,		28
.set	v1_scaleXstep,	24
.set	v1_mask_ptr,	20
.set	v1_y,			16
.set	v1_scaletable,	12
.set	pitch,			8
.set	scaleIdxXPtr,	4
.set	scaleIdxYPtr,	0

	@ r0 = _scaleY
	@ r1 = v1
	@ r2 = _out
	@ r3 = src
	@ <> = _height
	@ <> = _scaleX
	@ <> = _scaleIndexX
	@ <> = _shadow_table
	@ <> = _palette
	@ <> = _numstrips
	@ <> = _scaleIndexY
_ClassicProc3RendererShadowARM:
	@ shadow20 = false
	@ shadowed = true
	@ unscaled = false
	STMFD	r13!,{r3-r11,r14}
	LDRH	r6,[r2,#2]
	LDRH	r7,[r2]			@ r7 = _out.w
	LDRH	r8,[r2,#4]		@ r8 = _out.pitch
	ADD	r6,r6,#1		@ r6 = _out.h+1
	SUB	r13,r13,#space
	STR	r3,[r13,#src]
	STR	r8,[r13,#pitch]
	LDMIA	r1,{r3,r4,r5,r8,r9,r10,r11}
	@ r3 = v1.x
	@ r4 = v1.y
	@ r5 = scaletable
	@ r8 = skip_width
	@ r9 = destptr
	@ r10= mask_ptr
	@ r11= scaleXstep
	LDR	r2, [r13,#_scaleIndexY]
	LDR	r12,[r13,#_scaleIndexX]
	STR	r4, [r13,#v1_y]
	STR	r5, [r13,#v1_scaletable]
	ADD	r2, r5,r2		@ r2 = &scaletable[_scaleIndexY]
	ADD	r5, r5,r12		@ r5 = &scaletable[_scaleIndexX]
	STR	r5, [r13,#scaleIdxXPtr]
	STR	r2, [r13,#scaleIdxYPtr]
	STR	r8, [r13,#v1_skip_width]

	LDRB	r8, [r1,#29]		@ r8 = shr
	LDRB	r14,[r1,#31]		@ r14= replen
	LDRB	r1, [r1,#30]		@ r1 = repcolor
	STR	r8, [r13,#v1_shr]
	STR	r9, [r13,#v1_destptr]
	STR	r10,[r13,#v1_mask_ptr]
	STR	r11,[r13,#v1_scaleXstep]

	LDR	r12,[r13,#_height]

	@ r0 = _scaleY
	@ r1 = v1.repcolor
	@ r2 = &v1.scaletable[scaleIndexY]
	@ r3 = v1.x
	@ r4 = v1.y
	@ r5 =
	@ r6 = _out.h+1
	@ r7 = _out.w
	@ r8 =
	@ r9 = v1.destptr
	@ r10= v1.mask_ptr
	@ r11=
	@ r12= _height
	@ r14= v1.replen

	MOV	r8,#0x80
	AND	r11,r3,#7		@ r11= v1.x & 7
	MOV	r8,r8,LSR r11		@ r8 = maskbit = revBitMask(v1.x & 7)
	ADD	r10,r10,r3,ASR #3	@ r10= mask = v1.mask_ptr + (v1.x>>3)

	@ r0 = _scaleY
	@ r1 = color = v1.repcolor
	@ r2 = &v1.scaletable[scaleIndexY]
	@ r3 = v1.x
	@ r4 = y = v1.y
	@ r5 =
	@ r6 = _out.h+1
	@ r7 = _out.w
	@ r8 = maskbit
	@ r9 = v1.destptr
	@ r10= mask
	@ r11=
	@ r12= height = _height
	@ r14= len = v1.replen
	CMP	r14,#0
	BEQ	outerloop
	ADD	r12,r12,#1	@ r12= height++
	MOV	r5,r12		@ r5 = loopCount = height
	CMP	r5,r14		@ if (loopCount > len)
	MOVGT	r5,r14		@     loopCount = len
	SUB	r12,r12,r5
	SUB	r14,r14,r5
	STR	r12,[r13,#height]
	STR	r14,[r13,#len]
	LDR	r12,[r13,#pitch]
	LDR	r11,[r13,#_numStrips]
	B	startpos

outerloop:
	@ r0 = _scaleY
	@ r1 = color
	@ r2 = &v1.scaletable[scaleIndexY]
	@ r3 = x
	@ r4 = y
	@ r5 =
	@ r6 = _out.h+1
	@ r7 = _out.w
	@ r8 = maskbit
	@ r9 = dst
	@ r10= mask
	@ r11=
	@ r12= height
	@ r14=

	LDR	r11,[r13,#src]
	LDR	r5,[r13,#v1_shr]
	@ stall
	LDRB	r14,[r11],#1		@ r14= len = *src++
	@ stall
	@ stall
	MOV	r1, r14,LSR r5		@ r1 = color = len>>v1.shr
	BICS	r14,r14,r1,LSL r5	@ r14= len
	LDREQB	r14,[r11],#1		@ if (!len)  r14 = len = *src++
	STR	r11,[r13,#src]
	CMP	r14,#0
middleloop:
	@ r0 = _scaleY
	@ r1 = color
	@ r2 = &v1.scaletable[scaleIndexY]
	@ r3 = x
	@ r4 = y
	@ r5 = loopCount = min(height,len)
	@ r6 = _out.h+1
	@ r7 = _out.w
	@ r8 = maskbit
	@ r9 = dst
	@ r10= mask
	@ r11=
	@ r12= height
	@ r14= len

	MOV	r5,r12		@ loopCount = height
	CMPNE	r5,r14		@ if (len != 0 && loopCount > len)
	MOVGT	r5,r14		@	loopCount = len
	SUB	r12,r12,r5	@ height -= loopCount
	SUB	r14,r14,r5	@ len    -= loopCount
	STR	r12,[r13,#height]
	STR	r14,[r13,#len]

	LDR	r11,[r13,#_numStrips]
	LDR	r12,[r13,#pitch]
	@ r0 = _scaleY
	@ r1 = color
	@ r2 = &v1.scaletable[scaleIndexY]
	@ r3 = x
	@ r4 = y
	@ r5 = loopCount = min(height,len)
	@ r6 = _out.h+1
	@ r7 = _out.w
	@ r8 = maskbit
	@ r9 = dst
	@ r10= mask
	@ r11= _numStrips
	@ r12= _out.pitch
	@ r14= scratch
innerloop:
	@ inner loop
	CMP	r0,#254			@ if _scaleY <= 254
	LDRLEB	r14,[r2],#1		@ r14 = v1.scaletable[scaleIndexY++]
	@ stallLE
	@ stallLE
	CMPLE	r0,r14			@ || _scaleY >= r14
	BLE	startpos

	ADDS	r4,r4,#1		@ y >= 0 (equiv to y>-1,y+1>0)
	CMPGT   r1,#0			@ && color > 0
	CMPGT	r6,r4			@ && _out.h+1 > y+1
	CMNGT	r3,#1			@ && x >= 0 (equiv to x>-1,x+1>0)
	CMPGT	r7,r3			@ && _out.w > x
	BLE	masked
	LDRB	r14,[r10]		@ r14 = mask[0]
	TST	r14,r8			@ && !(mask[0] && maskbit)
	LDREQ	r14,[r13,#_palette]
	BNE	masked

	@ stall
	LDRB	r14,[r14,r1,lsl#1]		@ r14 = pcolor = _palette[color]
	@ stall
	@ stall
	CMP	r14,#13			@ if (pcolor == 13)
	LDREQ	r12,[r13,#_shadow_table]
	LDREQB	r14,[r9]		@ 	r14 = *dst
	@ stallEQ
	@ stallEQ
	LDREQB	r14,[r12,r14]		@	r14 = pcolor=_shadow_tab[r14]
	LDREQ	r12,[r13,#pitch]
	@ stallEQ
	STRB	r14,[r9]		@ *dst = pcolor
masked:
	ADD	r10,r10,r11		@ mask += _numStrips
	ADD	r9,r9,r12		@ dst += _out.pitch
startpos:
	SUBS	r5,r5,#1		@ loopCount -=1
	BNE	innerloop

	@ So we either ran out of height or len (or both)
	LDR	r12,[r13,#height]
	LDR	r14,[r13,#len]
	CMP	r12,#0
	BNE	notheight
	LDR	r10,[r13,#v1_skip_width]
	LDR	r11,[r13,#_scaleX]
	LDR	r2,[r13,#scaleIdxXPtr]
	LDR	r12,[r13,#v1_scaleXstep]
	SUBS	r10,r10,#1			@ if --v1.skip_width == 0
	BEQ	end				@	return _scaleIndexX
	LDRB	r9,[r2],r12                     @ r2 = scaleIdxXPtr+=sclXstp
	STR	r10,[r13,#v1_skip_width]
	CMP	r11,#254			@ if _scaleX <= 254
	CMPLE	r11,r9				@ && _scaleX <= scltb[scIdxX]
	LDR	r9,[r13,#v1_destptr]		@ r9 = dst = v1.destptr
	BLE	noXstep

	SUB	r11,r7,#1
	ADDS	r3,r3,r12			@ v1.x += v1.scaleXstep
				@ if v1.x < 0 ||
	CMPGE	r11,r3		@ _out.w-1 < v1.x
	BLT	end
	AND	r11,r3,#7
	MOV	r8,#0x80
	MOV	r8,r8,LSR r11	@ maskbit = revBitMask(v1.x & 7)
	ADD	r9,r9,r12	@ r10 = v1.destptr += v1.scaleXstep
	STR	r9,[r13,#v1_destptr]
noXstep:
	STR	r2,[r13,#scaleIdxXPtr]
	LDR	r10,[r13,#v1_mask_ptr]
	LDR	r12,[r13,#_height]		@ r12= height = _height
	LDR	r4,[r13,#v1_y]			@ r4 = y = v1.y
	LDR	r2,[r13,#scaleIdxYPtr]		@ r2 = v1.scaletable[sclIdxY]
	ADD	r10,r10,r3,ASR #3		@ mask=v1.mask_ptr+(v1.x>>3)
notheight:
	CMP	r14,#0			@ while (len > 0)
	BGT	middleloop
	B	outerloop		@ while (1)
end:
	LDR	r0,[r13,#v1_scaletable]
	SUB	r0,r2,r0
	ADD	r13,r13,#space
	LDMFD	r13!,{r3-r11,PC}
