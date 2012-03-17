;
; Gens: [MDP] 1x renderer. [16-bit color] (x86 asm version)
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
%define _mdp_render_1x_16_x86 mdp_render_1x_16_x86
%define _mdp_render_1x_16_x86_mmx mdp_render_1x_16_x86_mmx
%endif

section .text align=64

	align 64
	
	;************************************************************************
	; void mdp_render_1x_16_x86(uint16_t *destScreen, uint16_t *mdScreen,
	;			    int destPitch, int srcPitch,
	;			    int width, int height);
	global _mdp_render_1x_16_x86
	_mdp_render_1x_16_x86:
		
		; Set up the frame pointer.
		push	ebp
		mov	ebp, esp
		pushad
		
		mov	ecx, [ebp + arg_width]		; ecx = Number of pixels per line
		mov	ebx, [ebp + arg_destPitch]	; ebx = Pitch of destination surface (bytes per line)
		mov	esi, [ebp + arg_mdScreen]	; esi = Source
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	[ebp + arg_srcPitch], ecx	; arg_srcPitch = offset
		sub	ebx, ecx			; ebx = Difference between dest pitch and src pitch
		mov	edi, [ebp + arg_destScreen]	; edi = Destination
		shr	ecx, 3				; Transfer 8 bytes per cycle. (4 16-bit pixels)
		mov	[ebp + arg_width], ecx		; Initialize the X counter.
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X:
				; Get source pixels.
				mov	eax, [esi]	; First two pixels.
				mov	edx, [esi + 4]	; Second two pixels.
				add	esi, 8
				
				; Put destination pixels.
				mov	[edi], eax
				mov	[edi + 4], edx
				add	edi, 8
				
				; Next group of pixels.
				dec	ecx
				jnz	.Loop_X
			
			add	esi, [ebp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebx			; Add the destination pitch difference.
			mov	ecx, [ebp + arg_width]		; Reset the X counter.
			dec	dword [ebp + arg_height]	; Decrement the Y counter.
			jnz	.Loop_Y
		
		; Reset the frame pointer.
		popad
		mov	esp, ebp
		pop	ebp
		ret
	
	align 64
	
	;************************************************************************
	; void mdp_render_1x_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
	;				int destPitch, int srcPitch,
	;				int width, int height);
	global _mdp_render_1x_16_x86_mmx
	_mdp_render_1x_16_x86_mmx:
		
		; Set up the frame pointer.
		push	ebp
		mov	ebp, esp
		pushad
		
		mov	ecx, [ebp + arg_width]		; ecx = Number of pixels per line
		mov	ebx, [ebp + arg_destPitch]	; ebx = Pitch of destination surface (bytes per line)
		mov	esi, [ebp + arg_mdScreen]	; esi = Source
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	[ebp + arg_srcPitch], ecx	; arg_srcPitch = offset
		sub	ebx, ecx			; ebx = Difference between dest pitch and src pitch
		mov	edi, [ebp + arg_destScreen]	; edi = Destination
		shr	ecx, 6				; Transfer 64 bytes per cycle. (32 16-bit pixels)
		mov	[ebp + arg_width], ecx		; Initialize the X counter.
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm1, [esi + 8]
				movq	mm2, [esi + 16]
				movq	mm3, [esi + 24]
				movq	mm4, [esi + 32]
				movq	mm5, [esi + 40]
				movq	mm6, [esi + 48]
				movq	mm7, [esi + 56]
				add	esi, 64
				
				; Put destination pixels.
				movq	[edi], mm0
				movq	[edi + 8], mm1
				movq	[edi + 16], mm2
				movq	[edi + 24], mm3
				movq	[edi + 32], mm4
				movq	[edi + 40], mm5
				movq	[edi + 48], mm6
				movq	[edi + 56], mm7
				add	edi, 64
				
				; Next group of pixels.
				dec	ecx
				jnz	.Loop_X
			
			add	esi, [ebp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebx			; Add the destination pitch difference.
			mov	ecx, [ebp + arg_width]		; Reset the X counter.
			dec	dword [ebp + arg_height]	; Decrement the Y counter.
			jnz	.Loop_Y
		
		; Reset the frame pointer.
		popad
		mov	esp, ebp
		pop	ebp
		emms
		ret
