;
; Gens: [MDP] 2x renderer. [16-bit color] (x86 asm version)
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

arg_destScreen	equ 8
arg_mdScreen	equ 12
arg_destPitch	equ 16
arg_srcPitch	equ 20
arg_width	equ 24
arg_height	equ 28

%ifdef __OBJ_ELF
%define _mdp_render_2x_16_x86 mdp_render_2x_16_x86
%define _mdp_render_2x_16_x86_mmx mdp_render_2x_16_x86_mmx
%endif

section .text align=64
	
	align 64
	
	;************************************************************************
	; void mdp_render_2x_16_x86(uint16_t *destScreen, uint16_t *mdScreen,
	;			    int destPitch, int srcPitch,
	;			    int width, int height);
	global _mdp_render_2x_16_x86
	_mdp_render_2x_16_x86:
		
		; Set up the frame pointer.
		push	ebp
		mov	ebp, esp
		pushad
		
		mov	ecx, [ebp + arg_width]		; ecx = Number of pixels per line
		mov	ebx, [ebp + arg_destPitch]	; ebx = Pitch of destination surface (bytes per line)
		mov	esi, [ebp + arg_mdScreen]	; esi = Source
		add	ecx, ecx
		sub	[ebp + arg_srcPitch], ecx	; arg_srcPitch = offset
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	ebx, ecx			; ebx = Difference between dest pitch and src pitch
		mov	edi, [ebp + arg_destScreen]	; edi = Destination
		shr	ecx, 4				; Transfer 16 bytes per cycle. (32 16-bit pixels)
		mov	[ebp + arg_width], ecx		; Initialize the X counter.
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X1:
				; First two pixels.
				mov	eax, [esi]
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi], eax
				mov	[edi + 4], edx
				
				; Second two pixels.
				mov	eax, [esi + 4]
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi + 8], eax
				mov	[edi + 12], edx
				
				; Next group of pixels.
				add	esi, 8
				add	edi, 16
				dec	ecx
				jnz	short .Loop_X1
	
			mov	ecx, [ebp + arg_width]	; ecx = Number of pixels per line
			add	edi, ebx		; Add the destination pitch difference.
			shl	ecx, 3
			sub	esi, ecx		; Go back to the beginning of the source line.
			shr	ecx, 3
			jmp short .Loop_X2
	
	align 64
	
	.Loop_X2:
				; First two pixels.
				mov	eax, [esi]
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi], eax
				mov	[edi + 4], edx
				
				; Second two pixels.
				mov	eax, [esi + 4]
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi + 8], eax
				mov	[edi + 12], edx
				
				; Next group of pixels.
				add	esi, 8
				add	edi, 16
				dec	ecx
				jnz	short .Loop_X2
			
			add	esi, [ebp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebx			; Add the destination pitch difference.
			mov	ecx, [ebp + arg_width]		; Reset the X conuter.
			dec	dword [ebp + arg_height]	; Decrement the Y counter.
			jnz	near .Loop_Y
		
		; Reset the frame pointer.
		popad
		mov	esp, ebp
		pop	ebp
		ret
	
	align 64
	
	;************************************************************************
	; void mdp_render_2x_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
	;				int destPitch, int srcPitch,
	;				int width, int height);
	global _mdp_render_2x_16_x86_mmx
	_mdp_render_2x_16_x86_mmx:
		
		; Set up the frame pointer.
		push	ebp
		mov	ebp, esp
		pushad
		
		mov	ecx, [ebp + arg_width]		; ecx = Number of pixels per line
		mov	ebx, [ebp + arg_destPitch]	; ebx = Pitch of destination surface (bytes per line)
		mov	esi, [ebp + arg_mdScreen]	; esi = Source
		add	ecx, ecx
		sub	[ebp + arg_srcPitch], ecx	; arg_srcPitch = offset
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	ebx, ecx			; ebx = Difference between dest pitch and src pitch
		mov	edi, [ebp + arg_destScreen]	; edi = Destination
		shr	ecx, 6				; Transfer 64 bytes per cycle. (128 16-bit pixels)
		mov	[ebp + arg_width], ecx		; Initialize the X counter.
		jmp	short .Loop_Y

	align 64

	.Loop_Y:
	.Loop_X1:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm1, mm0
				movq	mm2, [esi + 8]
				movq	mm3, mm2
				movq	mm4, [esi + 16]
				movq	mm5, mm4
				movq	mm6, [esi + 24]
				movq	mm7, mm6
				add	esi, byte 32
				
				; Unpack source pixels.
				punpcklwd	mm1, mm1
				punpckhwd	mm0, mm0
				punpcklwd	mm3, mm3
				punpckhwd	mm2, mm2
				punpcklwd	mm5, mm5
				punpckhwd	mm4, mm4
				punpcklwd	mm7, mm7
				punpckhwd	mm6, mm6
				
				; Put destination pixels.
				movq	[edi], mm1
				movq	[edi + 8], mm0
				movq	[edi + 16], mm3
				movq	[edi + 24], mm2
				movq	[edi + 32], mm5
				movq	[edi + 40], mm4
				movq	[edi + 48], mm7
				movq	[edi + 56], mm6
				add	edi, byte 64
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X1
			
			mov	ecx, [ebp + arg_width]	; Reset the X counter.
			add	edi, ebx		; Add the destination pitch difference.
			shl	ecx, 5
			sub	esi, ecx		; Go back to the beginning of the source line.
			shr	ecx, 5
			jmp	short .Loop_X2
	
	align 64
	
	.Loop_X2:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm1, mm0
				movq	mm2, [esi + 8]
				movq	mm3, mm2
				movq	mm4, [esi + 16]
				movq	mm5, mm4
				movq	mm6, [esi + 24]
				movq	mm7, mm6
				add	esi, byte 32
				
				; Unpack source pixels.
				punpcklwd	mm1, mm1
				punpckhwd	mm0, mm0
				punpcklwd	mm3, mm3
				punpckhwd	mm2, mm2
				punpcklwd	mm5, mm5
				punpckhwd	mm4, mm4
				punpcklwd	mm7, mm7
				punpckhwd	mm6, mm6
				
				; Put destination pixels.
				movq	[edi], mm1
				movq	[edi + 8], mm0
				movq	[edi + 16], mm3
				movq	[edi + 24], mm2
				movq	[edi + 32], mm5
				movq	[edi + 40], mm4
				movq	[edi + 48], mm7
				movq	[edi + 56], mm6
				add	edi, byte 64
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X2
			
			add	esi, [ebp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebx			; Add the destination pitch difference.
			mov	ecx, [ebp + arg_width]		; Reset the X counter.
			dec	dword [ebp + arg_height]	; Decrement the Y counter.
			jnz	near .Loop_Y
		
		; Reset the frame pointer.
		popad
		mov	esp, ebp
		pop	ebp
		emms
		ret
