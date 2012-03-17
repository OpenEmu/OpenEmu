;
; Gens: MMX audio functions.
;

arg_left	equ 8
arg_right	equ 12
arg_dest	equ 16
arg_length	equ 20

section .text align=64
	
	; Symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define _writeSoundMono_MMX	writeSoundMono_MMX
		%define _writeSoundStereo_MMX	writeSoundStereo_MMX
	%endif
	
	; void writeSoundMono_MMX(int *left, int *right, short *dest, int length)
	global _writeSoundMono_MMX
	_writeSoundMono_MMX:
		
		; Set up the frame pointer.
		push	ebp
		mov	ebp, esp
		pushad
		
		; Copy the function parameters to registers.
		mov	edi, [ebp + arg_left]		; Left
		mov	esi, [ebp + arg_right]		; Right
		mov	ecx, [ebp + arg_length]		; Length
		mov	ebx, [ebp + arg_dest]		; Dest
		
		shr	ecx, 1
		jnc	short .Double_Trans
	
	.Simple_Trans:
		mov	eax, [edi]
		mov	dword [edi], 0
		add	eax, [esi]
		mov	dword [esi], 0
		
		cmp	eax, 0xFFFF
		jle	short .lower_s
		
		mov	word [ebx], 0x7FFF
		jmp	short .ok_s
	
	.lower_s:
		cmp	eax, -0xFFFF
		jge	short .greater_s
		
		mov	word [ebx], -0x7FFF
		jmp	short .ok_s
	
	.greater_s:
		shr	eax, 1
		mov	[ebx], ax
	
	.ok_s:
		add	edi, 4
		add	esi, 4
		add	ebx, 2
	
	.Double_Trans:
		mov	eax, 32
		pxor	mm4, mm4
		movd	mm5, eax
		mov	eax, 1
		test	ecx, ecx
		movd	mm6, eax
		jnz	short .Loop
		jmp	short .End
	
	align 64
	
	.Loop:
			movq		mm0, [edi]		; L2 | L1
			add		ebx, 4
			movq		[edi], mm4
			movq		mm1, [esi]		; R2 | R1
			add		edi, 8
			movq		[esi], mm4
			packssdw	mm0, mm0		; 0 | 0 | L2 | L1
			packssdw	mm1, mm1		; 0 | 0 | R2 | R1
			psraw		mm0, 1
			psraw		mm1, 1
			add		esi, 8
			paddw		mm0, mm1		; 0 | 0 | R2 + L2 | R1 + L1
			
			dec	ecx
			movd	[ebx - 4], mm0
			jnz	short .Loop
		
	.End:
		; Reset the frame pointer.
		popad
		mov	esp, ebp
		pop	ebp
		emms
		ret
	
	align 64
	
	; void writeSoundStereo_MMX(int *left, int *right, short *dest, int length)
	global _writeSoundStereo_MMX
	_writeSoundStereo_MMX:
		
		; Set up the frame pointer.
		push	ebp
		mov	ebp, esp
		pushad
		
		; Copy the function parameters to registers.
		mov	edi, [ebp + arg_left]		; Left
		mov	esi, [ebp + arg_right]		; Right
		mov	ecx, [ebp + arg_length]		; Length
		mov	ebx, [ebp + arg_dest]		; Dest
		
		shr	ecx, 1
		jnc	short .Double_Trans
	
	.Simple_Trans:
		mov	eax, [edi]
		cmp	eax, 0x7FFF
		mov	dword [edi], 0
		jle	short .lower_s1
		
		mov	word [ebx + 0], 0x7FFF
		jmp	short .right_s1
	
	.lower_s1:
		cmp	eax, -0x7FFF
		jge	short .greater_s1
		
		mov	word [ebx + 0], -0x7FFF
		jmp	short .right_s1
	
	.greater_s1:
		mov	[ebx + 0], ax
	
	.right_s1:
		mov	edx, [esi]
		cmp	edx, 0x7FFF
		mov	dword [esi], 0
		jle	short .lower_s2
		
		mov	word [ebx + 2], 0x7FFF
		jmp	short .ok_s1
	
	.lower_s2:
		cmp	edx, -0x7FFF
		jge	short .greater_s2
		
		mov	word [ebx + 2], -0x7FFF
		jmp	short .ok_s1
	
	.greater_s2:
		mov	[ebx + 2], dx
	
	.ok_s1:
		add	edi, 4
		add	esi, 4
		add	ebx, 4
	
	.Double_Trans:
		mov	eax, 32
		pxor	mm4, mm4
		test	ecx, ecx
		movd	mm5, eax
		jnz	short .Loop
		jmp	short .End
	
	align 64
	
	.Loop:
			movd		mm0, [esi]		; 0  | R1
			add		edi, 8
			movd		mm1, [esi + 4]		; 0  | R2
			psllq		mm0, mm5		; R1 |  0
			movq		[esi], mm4
			psllq		mm1, mm5		; R2 |  0
			movd		mm2, [edi - 8]		; 0  | L1
			add		esi, 8
			movd		mm3, [edi - 8 + 4]	; 0  | L2
			add		ebx, 8
			paddd		mm0, mm2		; R1 | L1
			paddd		mm1, mm3		; R2 | L2
			movq		[edi - 8], mm4
			packssdw	mm0, mm1		; R2 | L2 | R1 | L1
			
			dec	ecx
			movq	[ebx - 8], mm0
			jnz	short .Loop
	
	.End:
		; Reset the frame pointer.
		popad
		mov	esp, ebp
		pop	ebp
		emms
		ret
