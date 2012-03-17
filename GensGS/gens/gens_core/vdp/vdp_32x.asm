;
; Gens: 32X VDP functions.
;

section .bss align=64
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	__32X_Palette_16B		_32X_Palette_16B
		%define	__32X_Palette_32B		_32X_Palette_32B
		
		%define	__32X_VDP_Ram			_32X_VDP_Ram
		%define __32X_VDP_CRam			_32X_VDP_CRam
		
		%define __32X_VDP_CRam_Ajusted		_32X_VDP_CRam_Ajusted
		%define	__32X_VDP_CRam_Ajusted32	_32X_VDP_CRam_Ajusted32
		
		%define	__32X_VDP			_32X_VDP
		%define __32X_VDP.Mode			_32X_VDP.Mode
		%define __32X_VDP.State			_32X_VDP.State
		%define __32X_VDP.AF_Data		_32X_VDP.AF_Data
		%define __32X_VDP.AF_St			_32X_VDP.AF_St
		%define __32X_VDP.AF_Len		_32X_VDP.AF_Len
		%define __32X_VDP.AF_Line		_32X_VDP.AF_Line
		
		%define	_MD_Screen			MD_Screen
		%define	_MD_Screen32			MD_Screen32
		%define	_bppMD				bppMD
	%endif
	
	extern _MD_Screen
	extern _MD_Screen32
	
	; MD bpp
	extern _bppMD
	
	global __32X_Palette_16B
	__32X_Palette_16B:
		resw 0x10000
	global __32X_Palette_32B
	__32X_Palette_32B:
		resd 0x10000
	
	global __32X_VDP_Ram
	__32X_VDP_Ram:
		resb 0x100 * 1024
	
	global __32X_VDP_CRam
	__32X_VDP_CRam:
		resw 0x100
	
	global __32X_VDP_CRam_Ajusted
	__32X_VDP_CRam_Ajusted:
		resw 0x100
	global __32X_VDP_CRam_Ajusted32
	__32X_VDP_CRam_Ajusted32:
		resd 0x100
	
	alignb 32
	
	global __32X_VDP
	__32X_VDP:
		.Mode:		resd 1
		.State:		resd 1
		.AF_Data:	resd 1
		.AF_St:		resd 1
		.AF_Len:	resd 1
		.AF_Line:	resd 1
	
section .text align=64
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	__32X_VDP_Reset		_32X_VDP_Reset
		%define	__32X_VDP_Draw		_32X_VDP_Draw
	%endif
	
	; void _32X_VDP_Reset()
	global __32X_VDP_Reset
	__32X_VDP_Reset:
		
		push	ecx
		push	edi
		
		xor	eax, eax
		mov	ecx, (256 * 1024 / 4)
		mov	[__32X_VDP.Mode], eax
		mov	edi, __32X_VDP_Ram
		mov	[__32X_VDP.State], eax
		mov	[__32X_VDP.AF_Data], eax
		rep	stosd
		mov	[__32X_VDP.AF_St], eax
		mov	[__32X_VDP.AF_Len], eax
		mov	[__32X_VDP.AF_Line], eax
		
		pop	edi
		pop	ecx
		ret
	
	align 16
	
	; void _32X_VDP_Draw(int FB_Num)
	global __32X_VDP_Draw
	__32X_VDP_Draw:
		
		; FB_Num is stored in eax.
		; No frame pointer is used in this function.
		mov	eax, [esp + 4]
		pushad
		
		and	eax, byte 1
		shl	eax, 17
		xor	ebp, ebp
		lea	esi, [eax + __32X_VDP_Ram]
		xor	ebx, ebx
		lea	esi, [eax + __32X_VDP_Ram]
		
		; Check if 32-bit color mode is enabled.
		; If 32-bit is enabled, do 32-bit drawing.
		cmp	byte [_bppMD], 32
		je	near ._32BIT
	
	; 16-bit color
	._16BIT:
		lea	edi, [_MD_Screen + 8 * 2]
	
	.Loop_Y:
		mov	eax, [__32X_VDP.Mode]
		mov	bx, [esi + ebp * 2]
		and	eax, byte 3
		mov	ecx, 160
		jmp	[.Table_32X_Draw + eax * 4]
	
	align 16
	
	.Table_32X_Draw:
		dd	._32X_Draw_M00, ._32X_Draw_M01, ._32X_Draw_M10, ._32X_Draw_M11
	
	align 16
	
	._32X_Draw_M10:
	._32X_Draw_M10_P:
			movzx	eax, word [esi + ebx * 2 + 0]
			movzx	edx, word [esi + ebx * 2 + 2]
			mov	ax, [__32X_Palette_16B + eax * 2]
			mov	dx, [__32X_Palette_16B + edx * 2]
			mov	[edi + 0], ax
			mov	[edi + 2], dx
			add	bx, byte 2
			add	edi, byte 4
			dec	ecx
			jnz	short ._32X_Draw_M10
			jmp	.End_Loop_Y
	
	._32X_Draw_M00_P:
	._32X_Draw_M00:
		popad
		ret
	
	align 16
	
	._32X_Draw_M01:
	._32X_Draw_M01_P:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	edx, byte [esi + ebx * 2 + 1]
			mov	ax, [__32X_VDP_CRam_Ajusted + eax * 2]
			mov	dx, [__32X_VDP_CRam_Ajusted + edx * 2]
			mov	[edi + 2], ax
			mov	[edi + 0], dx
			inc	bx
			add	edi, byte 4
			dec	ecx
			jnz	short ._32X_Draw_M01
			jmp	.End_Loop_Y
	
	align 16
	
	._32X_Draw_M11:
	._32X_Draw_M11_P:
			mov	edx, 320
			jmp	short ._32X_Draw_M11_Loop
	
	align 16
	
	._32X_Draw_M11_Loop:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	ecx, byte [esi + ebx * 2 + 1]
			mov	ax, [__32X_VDP_CRam_Ajusted + eax * 2]
			inc	ecx
			inc	bx
			sub	edx, ecx
			jbe	short ._32X_Draw_M11_End
			rep	stosw
			jmp	short ._32X_Draw_M11_Loop
	
	align 16

	._32X_Draw_M11_End:
		add	ecx, edx
		rep	stosw

	.End_Loop_Y:
		inc	ebp
		add	edi, byte 8 * 2 * 2
		cmp	ebp, 220
		jb	near .Loop_Y
		
		lea	edi, [_MD_Screen + 8 * 2 + 336 * 2 * 220]
		xor	eax, eax
		mov	ecx, 128
	
	.Palette_Loop:
			mov	dx, [__32X_VDP_CRam_Ajusted + eax * 2]
			mov	[edi + 0], dx
			mov	[edi + 2], dx
			mov	[edi + 336 * 2 + 0], dx
			mov	[edi + 336 * 2 + 2], dx
			mov	dx, [__32X_VDP_CRam_Ajusted + eax * 2 + 128 * 2]
			mov	[edi + 336 * 4 + 0], dx
			mov	[edi + 336 * 4 + 2], dx
			mov	[edi + 336 * 6 + 0], dx
			mov	[edi + 336 * 6 + 2], dx
			add	edi, byte 4
			inc	eax
			dec	ecx
			jnz	short .Palette_Loop 
		
		popad
		ret
	
	; 32-bit color
	._32BIT:
		lea	edi, [_MD_Screen32 + 8 * 4]

	.Loop_Y32:
		mov	eax, [__32X_VDP.Mode]
		mov	bx, [esi + ebp * 2]
		and	eax, byte 3
		mov	ecx, 160
		jmp	[.Table_32X_Draw32 + eax * 4]
	
	align 16
	
	.Table_32X_Draw32:
		dd ._32X_Draw_M0032, ._32X_Draw_M0132, ._32X_Draw_M1032, ._32X_Draw_M1132
	
	align 16
	
	._32X_Draw_M1032:
	._32X_Draw_M10_P32:
			movzx	eax, word [esi + ebx * 2 + 0]
			movzx	edx, word [esi + ebx * 2 + 2]
			mov	eax, [__32X_Palette_32B + eax * 4]
			mov	edx, [__32X_Palette_32B + edx * 4]
			mov	[edi + 0], eax
			mov	[edi + 4], edx
			add	bx, byte 2
			add	edi, byte 8
			dec	ecx
			jnz	short ._32X_Draw_M1032
			jmp	.End_Loop_Y32
	
	._32X_Draw_M00_P32:
	._32X_Draw_M0032:
		popad
		ret
	
	align 16
	
	._32X_Draw_M0132:
	._32X_Draw_M01_P32:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	edx, byte [esi + ebx * 2 + 1]
			mov	eax, [__32X_VDP_CRam_Ajusted32 + eax * 4]
			mov	edx, [__32X_VDP_CRam_Ajusted32 + edx * 4]
			mov	[edi + 4], eax
			mov	[edi + 0], edx
			inc	bx
			add	edi, byte 8
			dec	ecx
			jnz	short ._32X_Draw_M0132
			jmp	.End_Loop_Y32
	
	align 16
	
	._32X_Draw_M1132:
	._32X_Draw_M11_P32:
			mov	edx, 320
			jmp	short ._32X_Draw_M11_Loop32
	
	align 16
	
	._32X_Draw_M11_Loop32:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	ecx, byte [esi + ebx * 2 + 1]
			mov	eax, [__32X_VDP_CRam_Ajusted32 + eax * 4]
			inc	ecx
			inc	bx
			sub	edx, ecx
			jbe	short ._32X_Draw_M11_End32
			rep	stosd
			jmp	short ._32X_Draw_M11_Loop32
	
	align 16
	
	._32X_Draw_M11_End32:
		add	ecx, edx
		rep	stosw
	
	.End_Loop_Y32:
		inc	ebp
		add	edi, byte 8 * 2 * 4
		cmp	ebp, 220
		jb	near .Loop_Y32
		
		lea	edi, [_MD_Screen32 + 8 * 4 + 336 * 4 * 220]
		xor	eax, eax
		mov	ecx, 128
	
	.Palette_Loop32:
			mov	edx, [__32X_VDP_CRam_Ajusted32 + eax * 4]
			mov	[edi + 0], edx
			mov	[edi + 4], edx
			mov	[edi + 336 * 4 + 0], edx
			mov	[edi + 336 * 4 + 4], edx
			mov	edx, [__32X_VDP_CRam_Ajusted32 + eax * 4 + 128 * 4]
			mov	[edi + 336 * 8 + 0], edx
			mov	[edi + 336 * 8 + 4], edx
			mov	[edi + 336 * 12 + 0], edx
			mov	[edi + 336 * 12 + 4], edx
			add	edi, byte 8
			inc	eax
			dec	ecx
			jnz	short .Palette_Loop32
	
	.End32:
		popad
		ret

