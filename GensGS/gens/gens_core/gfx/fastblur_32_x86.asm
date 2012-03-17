;
; Gens: Fast Blur function. (32-bit color, x86 asm version.)
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
%define _Fast_Blur_32_x86 Fast_Blur_32_x86
%define _Fast_Blur_32_x86_mmx Fast_Blur_32_x86_mmx
%define _MD_Screen32 MD_Screen32
%endif

MD_SCREEN_SIZE	equ 336 * 240
MASK_DIV2_32	equ 0x007F7F7F

section .data align=64
	
	; MD screen buffer and bpp.
	extern _MD_Screen32
	
	; 64-bit mask used for the mmx version.
	MASK_DIV2_32_MMX:	dd 0x007F7F7F, 0x007F7F7F
	
section .text align=64
	
	; void Fast_Blur_32_x86_mmx()
	; 32-bit color Fast Blur function, non-MMX.
	global _Fast_Blur_32_x86
	_Fast_Blur_32_x86:
		
		push	ebx
		push	ecx
		push	edx
		push	esi
		
		; Start at the beginning of the actual display data.
		mov	esi, _MD_Screen32 + (8*4)
		mov	ecx, MD_SCREEN_SIZE - 8
		xor	ebx, ebx
		
	align 32
	
	.Loop:
			mov	eax, [esi]		; Get the current pixel.
			shr	eax, 1			; Reduce pixel brightness by 50%.
			and	eax, MASK_DIV2_32	; Apply color mask.
			add	ebx, eax		; Blur the pixel with the previous pixel.
			mov	[esi - 4], ebx		; Write the new pixel.
			mov	ebx, eax		; Store the current pixel.
			
			; Next pixel.
			add	esi, 4
			dec	ecx
			jnz	short .Loop
		
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 32
	
	; void Fast_Blur_32_x86_mmx()
	; 32-bit color Fast Blur function, using MMX instructions.
	global _Fast_Blur_32_x86_mmx
	_Fast_Blur_32_x86_mmx:
		
		push	ebx
		push	ecx
		push	esi
		
		; Start at the beginning of the actual display data.
		mov	esi, _MD_Screen32 + (8*4)
		mov	ecx, (MD_SCREEN_SIZE - 8) / 2
		
		; Load the 32-bit color mask.
		movq	mm7, [MASK_DIV2_32_MMX]
		jmp	short .Loop
	
	align 32
	
	.Loop:
			; Get source pixels.
			movq	mm0, [esi]
			movq	mm1, [esi + 4]
			
			; Blur source pixels.
			psrld	mm0, 1
			psrld	mm1, 1
			pand	mm0, mm7
			pand	mm1, mm7
			paddd	mm0, mm1
			
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
