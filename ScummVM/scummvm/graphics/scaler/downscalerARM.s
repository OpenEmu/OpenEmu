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

	.text

	.global	DownscaleAllByHalfARM

	@ ARM implementation of DownscaleAllByHalf scaler.
	@ Scales a width x height block of 16bpp pixels from srcPtr to
	@ dstPtr. srcPitch and dstPitch identify how to reach subsequent
	@ lines. redblueMask and round allow for one routine to do both
	@ 565 and 555 formats.
DownscaleAllByHalfARM:
	@ r0 = srcPtr
	@ r1 = srcPitch
	@ r2 = dstPtr
	@ r3 = dstPitch
	MOV	r12,r13
	STMFD	r13!,{r4-r11,r14}
	LDMIA	r12,{r4-r7}
	@ r4 = width
	@ r5 = height
	@ r6 = redblueMask
	@ r7 = round

	SUB	r3,r3,r4		@ dstPitch -= width
	SUBS	r5,r5,#2		@ while ((height-=2) >= 0)
	BLT	end
height_loop:

        SUBS	r11, r4, #8		@ r11= width_minus_8
	BLT	thin

width_loop:
	@ unroll loop 4 times here
	LDRH	r8,[r0],r1		@ r8 = A = srcPtr[0]
	LDRH	r9,[r0],#2		@ r9 = C = srcPtr[dstPitch]
	LDRH	r12,[r0],-r1		@ r12= D = srcPtr[dstPitch+2]
	LDRH	r14,[r0],#2		@ r14= B = srcPtr[2]

	ORR	r8, r8, r8, LSL #16	@ r8 = b | g | r | b | g | r
	ORR	r9, r9, r9, LSL #16	@ r9 = b | g | r | b | g | r
	ORR	r12,r12,r12,LSL #16	@ r12= b | g | r | b | g | r
	ORR	r14,r14,r14,LSL #16	@ r14= b | g | r | b | g | r
	AND	r8, r8, r6		@ r8 = 0 | g | 0 | b | 0 | r
	AND	r9, r9, r6		@ r9 = 0 | g | 0 | b | 0 | r
	AND	r12,r12,r6		@ r12= 0 | g | 0 | b | 0 | r
	AND	r14,r14,r6		@ r14= 0 | g | 0 | b | 0 | r
	ADD	r8, r8, r9
	ADD	r8, r8, r12
	ADD	r8, r8, r14
	ADD	r8, r8, r7		@ r8 = summed pixels + rounding
	AND	r8, r6, r8, LSR #2	@ r8 = 0 | g | 0 | b | 0 | r
	ORR	r10,r8, r8, LSR #16	@ r10= 0 | g | 0 | b | g | r

	LDRH	r8,[r0],r1		@ r8 = A = srcPtr[0]
	LDRH	r9,[r0],#2		@ r9 = C = srcPtr[dstPitch]
	LDRH	r12,[r0],-r1		@ r12= D = srcPtr[dstPitch+2]
	LDRH	r14,[r0],#2		@ r14= B = srcPtr[2]

	STRH	r10,[r2],#2		@ *dstPtr++

	ORR	r8, r8, r8, LSL #16	@ r8 = b | g | r | b | g | r
	ORR	r9, r9, r9, LSL #16	@ r9 = b | g | r | b | g | r
	ORR	r12,r12,r12,LSL #16	@ r12= b | g | r | b | g | r
	ORR	r14,r14,r14,LSL #16	@ r14= b | g | r | b | g | r
	AND	r8, r8, r6		@ r8 = 0 | g | 0 | b | 0 | r
	AND	r9, r9, r6		@ r9 = 0 | g | 0 | b | 0 | r
	AND	r12,r12,r6		@ r12= 0 | g | 0 | b | 0 | r
	AND	r14,r14,r6		@ r14= 0 | g | 0 | b | 0 | r
	ADD	r8, r8, r9
	ADD	r8, r8, r12
	ADD	r8, r8, r14
	ADD	r8, r8, r7		@ r8 = summed pixels + rounding
	AND	r8, r6, r8, LSR #2	@ r8 = 0 | g | 0 | b | 0 | r
	ORR	r10,r8, r8, LSR #16	@ r10= 0 | g | 0 | b | g | r

	LDRH	r8,[r0],r1		@ r8 = A = srcPtr[0]
	LDRH	r9,[r0],#2		@ r9 = C = srcPtr[dstPitch]
	LDRH	r12,[r0],-r1		@ r12= D = srcPtr[dstPitch+2]
	LDRH	r14,[r0],#2		@ r14= B = srcPtr[2]

	STRH	r10,[r2],#2		@ *dstPtr++

	ORR	r8, r8, r8, LSL #16	@ r8 = b | g | r | b | g | r
	ORR	r9, r9, r9, LSL #16	@ r9 = b | g | r | b | g | r
	ORR	r12,r12,r12,LSL #16	@ r12= b | g | r | b | g | r
	ORR	r14,r14,r14,LSL #16	@ r14= b | g | r | b | g | r
	AND	r8, r8, r6		@ r8 = 0 | g | 0 | b | 0 | r
	AND	r9, r9, r6		@ r9 = 0 | g | 0 | b | 0 | r
	AND	r12,r12,r6		@ r12= 0 | g | 0 | b | 0 | r
	AND	r14,r14,r6		@ r14= 0 | g | 0 | b | 0 | r
	ADD	r8, r8, r9
	ADD	r8, r8, r12
	ADD	r8, r8, r14
	ADD	r8, r8, r7		@ r8 = summed pixels + rounding
	AND	r8, r6, r8, LSR #2	@ r8 = 0 | g | 0 | b | 0 | r
	ORR	r10,r8, r8, LSR #16	@ r10= 0 | g | 0 | b | g | r

	LDRH	r8,[r0],r1		@ r8 = A = srcPtr[0]
	LDRH	r9,[r0],#2		@ r9 = C = srcPtr[dstPitch]
	LDRH	r12,[r0],-r1		@ r12= D = srcPtr[dstPitch+2]
	LDRH	r14,[r0],#2		@ r14= B = srcPtr[2]

	STRH	r10,[r2],#2		@ *dstPtr++

	ORR	r8, r8, r8, LSL #16	@ r8 = b | g | r | b | g | r
	ORR	r9, r9, r9, LSL #16	@ r9 = b | g | r | b | g | r
	ORR	r12,r12,r12,LSL #16	@ r12= b | g | r | b | g | r
	ORR	r14,r14,r14,LSL #16	@ r14= b | g | r | b | g | r
	AND	r8, r8, r6		@ r8 = 0 | g | 0 | b | 0 | r
	AND	r9, r9, r6		@ r9 = 0 | g | 0 | b | 0 | r
	AND	r12,r12,r6		@ r12= 0 | g | 0 | b | 0 | r
	AND	r14,r14,r6		@ r14= 0 | g | 0 | b | 0 | r
	ADD	r8, r8, r9
	ADD	r8, r8, r12
	ADD	r8, r8, r14
	ADD	r8, r8, r7		@ r8 = summed pixels + rounding
	AND	r8, r6, r8, LSR #2	@ r8 = 0 | g | 0 | b | 0 | r
	ORR	r10, r8, r8, LSR #16	@ r8 = 0 | g | 0 | b | g | r

	STRH	r10,[r2],#2		@ *dstPtr++

	SUBS	r11,r11,#8		@ width_minus_8 -= 8
	BGE	width_loop		@ (width_minus_8 >= 0) => do 8+ more

thin:
	ADDS	r11,r11,#8		@ r11= width
	BEQ	width_end		@ if no more left to do, then bail
thin_lp:
	@ single output pixels done in this bit
	LDRH	r8,[r0],r1		@ r8 = A = srcPtr[0]
	LDRH	r9,[r0],#2		@ r9 = C = srcPtr[dstPitch]
	LDRH	r12,[r0],-r1		@ r12= D = srcPtr[dstPitch+2]
	LDRH	r14,[r0],#2		@ r14= B = srcPtr[2]

	ORR	r8, r8, r8, LSL #16	@ r8 = b | g | r | b | g | r
	ORR	r9, r9, r9, LSL #16	@ r9 = b | g | r | b | g | r
	ORR	r12,r12,r12,LSL #16	@ r12= b | g | r | b | g | r
	ORR	r14,r14,r14,LSL #16	@ r14= b | g | r | b | g | r
	AND	r8, r8, r6		@ r8 = 0 | g | 0 | b | 0 | r
	AND	r9, r9, r6		@ r9 = 0 | g | 0 | b | 0 | r
	AND	r12,r12,r6		@ r12= 0 | g | 0 | b | 0 | r
	AND	r14,r14,r6		@ r14= 0 | g | 0 | b | 0 | r
	ADD	r8, r8, r9
	ADD	r8, r8, r12
	ADD	r8, r8, r14
	ADD	r8, r8, r7		@ r8 = summed pixels + rounding
	AND	r8, r6, r8, LSR #2	@ r8 = 0 | g | 0 | b | 0 | r
	ORR	r8, r8, r8, LSR #16	@ r8 = 0 | g | 0 | b | g | r

	STRH	r8,[r2],#2		@ *dstPtr++

	SUBS	r11,r11,#2
	BGT	thin_lp
width_end:
	ADD	r2,r2,r3		@ dstPtr += dstPitch
	ADD	r0,r0,r1,LSL #1		@ srcPtr += 2*srcPitch
	SUB	r0,r0,r4,LSL #1		@ srcPtr -= 2*width

	SUBS	r5,r5,#2
	BGE	height_loop

end:
	LDMFD	r13!,{r4-r11,PC}
