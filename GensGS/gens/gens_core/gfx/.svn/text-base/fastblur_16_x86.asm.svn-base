;
; Gens: Fast Blur function. (15/16-bit color, x86 asm version.)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008 by David Korth
;
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the
; Free Software Foundation; either version 2 of the License, or (at your
; option) any later version.
;
; This program is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;

%ifdef __OBJ_ELF
%define _Fast_Blur_16_x86 Fast_Blur_16_x86
%define _Fast_Blur_16_x86_mmx Fast_Blur_16_x86_mmx
%define _MD_Screen MD_Screen
%define _bppMD bppMD
%endif

MD_SCREEN_SIZE	equ 336 * 240
MASK_DIV2_15	equ 0x3DEF
MASK_DIV2_16	equ 0x7BEF

section .data align=64
	
	; MD screen buffer and bpp.
	extern _MD_Screen
	extern _bppMD
	
	; 64-bit masks used for the mmx version.
	MASK_DIV2_15_MMX:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16_MMX:	dd 0x7BEF7BEF, 0x7BEF7BEF
	
section .text align=64
	
	; void Fast_Blur_16_x86_mmx()
	; 15/16-bit color Fast Blur function, non-MMX.
	global _Fast_Blur_16_x86
	_Fast_Blur_16_x86:
		
		push	ebx
		push	ecx
		push	edx
		push	esi
		
		; Start at the beginning of the actual display data.
		mov	esi, _MD_Screen + (8*2)
		mov	ecx, MD_SCREEN_SIZE - 8
		xor	ebx, ebx
		
		; Default to 16-bit color.
		mov	dx, MASK_DIV2_16
		cmp	byte [_bppMD], 15
		jne	short .Loop
		
		; 15-bit color is in use.
		mov	dx, MASK_DIV2_15
		jmp	short .Loop
	
	align 32
	
	.Loop:
			mov	ax, [esi]	; Get the current pixel.
			shr	ax, 1		; Reduce pixel brightness by 50%.
			and	ax, dx		; Apply color mask.
			add	bx, ax		; Blur the pixel with the previous pixel.
			mov	[esi - 2], bx	; Write the new pixel.
			mov	bx, ax		; Store the current pixel.
			
			; Next pixel.
			add	esi, 2
			dec	ecx
			jnz	short .Loop
		
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 32
	
	; void Fast_Blur_16_x86_mmx()
	; 15/16-bit color Fast Blur function, using MMX instructions.
	global _Fast_Blur_16_x86_mmx
	_Fast_Blur_16_x86_mmx:
		
		push	ebx
		push	ecx
		push	esi
		
		; Start at the beginning of the actual display data.
		mov	esi, _MD_Screen + (8*2)
		mov	ecx, (MD_SCREEN_SIZE - 8) / 4
		
		; Default to 16-bit color.
		movq	mm7, [MASK_DIV2_16_MMX]
		cmp	byte [_bppMD], 15
		jne	short .Loop
		
		; 15-bit color is in use.
		movq	mm7, [MASK_DIV2_15_MMX]
		jmp	short .Loop
	
	align 32
	
	.Loop:
			; Get source pixels.
			movq	mm0, [esi]
			movq	mm1, [esi + 2]
			
			; Blur source pixels.
			psrlw	mm0, 1
			psrlw	mm1, 1
			pand	mm0, mm7
			pand	mm1, mm7
			paddw	mm0, mm1
			
			; Put destination pixels.
			movq	[esi], mm0
			
			; Next group of pixels.
			add	esi, 8
			dec	ecx
			jnz	short .Loop
		
		pop	esi
		pop	ecx
		pop	ebx
		emms
		ret
