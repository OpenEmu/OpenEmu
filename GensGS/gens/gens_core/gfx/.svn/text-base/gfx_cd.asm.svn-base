%include "nasmhead.inc"

; macro MAKE_IMAGE_PIXEL
; construit un pixel de l'image destination (rotation/compression)
;
; Paramètres :
; %1 = 0 pour normal et 1 pour titled
; %2 = 0 pour 16x16 dot et 1 pour 32x32 dot
; %3 = 0 pour 1x1 screen et 1 pour 16x16 screen
; %4 = Priority mode : 0 = OFF, 1 = Underwrite, 2 = Overwrite
;
; Entrée :
; di = donnée du stamp
; ecx = XS, edx = YS
;
; Résultat :
; un pixel du buffer image a été caculé

%macro MAKE_IMAGE_PIXEL 4

%if %1 < 1							; NO TITLED
	%if %3 > 0						; 16x16 screen
		test edx, 0x00800000
		mov esi, edi
		jnz near %%Pixel_Out
		shr edi, 11
		test ecx, 0x00800000
		jnz near %%Pixel_Out
	%else							; 1x1 screen
		test edx, 0x00F80000
		mov esi, edi
		jnz near %%Pixel_Out
		shr edi, 11
		test ecx, 0x00F80000
		jnz near %%Pixel_Out
	%endif
%else
		mov esi, edi
		shr edi, 11
%endif
		and esi, 0x07FF
		jz near %%Pixel_Out			; Stamp 0 = 0 FILL
		and edi, 0x001C
		shl esi, 7
		mov eax, ecx
		mov ebx, edx
		jmp [%%Table + edi]

	ALIGN4

	%%Table

		dd %%No_Flip_0, %%No_Flip_90, %%No_Flip_180, %%No_Flip_270
		dd %%Flip_0, %%Flip_90, %%Flip_180, %%Flip_270

	ALIGN32

	%%No_Flip_90
		%if %2 > 0				; 32x32 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 7
			and eax, 0x7C
			and ebx, 0x180
			and edi, 0x3800
			xor ebx, 0x180
			add eax, esi
			xor edi, 0x2800					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%else					; 16x16 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 8
			and eax, 0x3C
			and ebx, 0x40
			and edi, 0x3800
			xor ebx, 0x40
			add eax, esi
			xor edi, 0x2800					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%endif

	ALIGN32

	%%No_Flip_180
		%if %2 > 0				; 32x32 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 7
			and ebx, 0x7C
			and eax, 0x180
			xor ebx, 0x7C
			xor eax, 0x180
			and edi, 0x3800
			add ebx, esi
			xor edi, 0x2800					; because byte swapped & flipped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%else					; 16x16 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 8
			and ebx, 0x3C
			and eax, 0x40
			xor ebx, 0x3C
			xor eax, 0x40
			and edi, 0x3800
			add ebx, esi
			xor edi, 0x2800					; because byte swapped & flipped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%endif

	ALIGN32

	%%No_Flip_270
		%if %2 > 0				; 32x32 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 7
			and eax, 0x7C
			and ebx, 0x180
			xor eax, 0x7C
			and edi, 0x3800
			add eax, esi
			xor edi, 0x1000					; because byte swapped & flipped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%else					; 16x16 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 8
			and eax, 0x3C
			and ebx, 0x40
			xor eax, 0x3C
			and edi, 0x3800
			add eax, esi
			xor edi, 0x1000					; because byte swapped & flipped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%endif

	ALIGN32

	%%Flip_90
		%if %2 > 0				; 32x32 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 7
			and eax, 0x7C
			and ebx, 0x180
			xor eax, 0x7C		; FLIP
			xor ebx, 0x180
			and edi, 0x3800
			add eax, esi
			xor edi, 0x2800					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%else					; 16x16 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 8
			and eax, 0x3C
			and ebx, 0x40
			xor eax, 0x3C		; FLIP
			xor ebx, 0x40
			and edi, 0x3800
			add eax, esi
			xor edi, 0x2800					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%endif

	ALIGN32

	%%Flip_180
		%if %2 > 0				; 32x32 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 7
			and ebx, 0x7C
			and eax, 0x180
			xor ebx, 0x7C
			and edi, 0x3800
			add ebx, esi
			xor edi, 0x1000					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%else					; 16x16 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 8
			and ebx, 0x3C
			and eax, 0x40
			xor ebx, 0x3C
			and edi, 0x3800
			add ebx, esi
			xor edi, 0x1000					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%endif

	ALIGN32

	%%Flip_270
		%if %2 > 0				; 32x32 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 7
			and eax, 0x7C
			and ebx, 0x180
			add eax, esi
			and edi, 0x3800
			add eax, ebx
			xor edi, 0x1000					; because byte swapped & flipped
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%else					; 16x16 dots
			shr eax, 9
			mov edi, ebx
			shr ebx, 8
			and eax, 0x3C
			and ebx, 0x40
			add eax, esi
			and edi, 0x3800
			add eax, ebx
			xor edi, 0x1000					; because byte swapped & flipped
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc near %%Finish
			shr bl, 4
			jmp %%Finish
		%endif

	ALIGN32

	%%Flip_0
		%if %2 > 0				; 32x32 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 7
			and ebx, 0x7C
			and eax, 0x180
			and edi, 0x3800
			add ebx, esi
			xor eax, 0x180
			xor edi, 0x2800					; because byte swapped & flipped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc short %%Finish
			shr bl, 4
			jmp short %%Finish
		%else					; 16x16 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 8
			and ebx, 0x3C
			and eax, 0x40
			and edi, 0x3800
			add ebx, esi
			xor eax, 0x40
			xor edi, 0x2800					; because byte swapped & flipped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc short %%Finish
			shr bl, 4
			jmp short %%Finish
		%endif

	ALIGN32

	%%No_Flip_0
		%if %2 > 0				; 32x32 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 7
			and ebx, 0x7C
			and eax, 0x180
			and edi, 0x3800
			add ebx, esi
			xor edi, 0x1000					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc short %%Finish
			shr bl, 4
			jmp short %%Finish
		%else					; 16x16 dots
			shr ebx, 9
			mov edi, eax
			shr eax, 8
			and ebx, 0x3C
			and eax, 0x40
			and edi, 0x3800
			add ebx, esi
			xor edi, 0x1000					; because byte swapped
			add eax, ebx
			shr edi, 12
			mov bl, [_Ram_Word_2M + edi + eax]	; pixel source
			jc short %%Finish
			shr bl, 4
			jmp short %%Finish
		%endif

	ALIGN4

%%Pixel_Out
		xor bl, bl

%%Finish
		and bl, 0x0F
		mov edi, [XD]
%if %4 == 2
		jz short %%End
%endif
		xor edi, 2					; because byte swapped
		mov esi, [Buffer_Adr]
		shr edi, 1
		mov al, [esi + edi]
		jc short %%Other_Pix

	%%First_Pix
%if %4 == 1
		test al, 0xF0
		jnz short %%End
%endif
		shl bl, 4
		and al, 0x0F
		add al, bl
		mov [esi + edi], al
		jmp short %%End

	ALIGN4
	
	%%Other_Pix
%if %4 == 1
		test al, 0x0F
		jnz short %%End
%endif
		and al, 0xF0
		add al, bl
		mov [esi + edi], al

	%%End

%endmacro



; macro MAKE_IMAGE_LINE
; construit une ligne de l'image destination (rotation/compression)
;
; Paramètres :
; %1 = 0 pour normal et 1 pour titled
; %2 = 0 pour 16x16 dot et 1 pour 32x32 dot
; %3 = 0 pour 1x1 screen et 1 pour 16x16 screen
; %4 = Prioritie mode : 0 = OFF, 1 = Underwrite, 2 = Overwrite
;
; Entrée :
; ecx = X Start (11 bits floating part)
; edx = Y Start (11 bits floating part)
;
; Résultat :
; une ligne du buffer image a été caculée

%macro MAKE_IMAGE_LINE 4

		jmp short %%LOOP_H
	
	ALIGN32
	
	%%LOOP_H
		mov ebx, edx
		mov eax, ecx
	
%if %2 > 0					; mode 32x32 dot
	%if %3 > 0				; mode 32x32 dot 16x16 screen
		shr ebx, 11 - 2
		and eax, 0x007F0000
		and ebx, 0x3F80
		shr eax, 11 + 5
	%else					; mode 32x32 dot 1x1 screen
		shr ebx, 11 + 2
		and eax, 0x00070000
		and ebx, 0x38
		shr eax, 11 + 5
	%endif
%else						; mode 16x16 dot
	%if %3 > 0				; mode 16x16 dot 16x16 screen
		shr ebx, 11 - 4
		and eax, 0x007F8000
		and ebx, 0xFF00
		shr eax, 11 + 4
	%else					; mode 16x16 dot 1x1 screen
		shr ebx, 11 + 0
		and eax, 0x00078000
		and ebx, 0xF0
		shr eax, 11 + 4
	%endif
%endif

		mov esi, [Stamp_Map_Adr]
		add ebx, eax					; numéro de stamp
		mov edi, [esi + ebx * 2]		; di = donnée stamp concerné

		MAKE_IMAGE_PIXEL %1, %2, %3, %4

	%%Next_Pixel

		mov eax, [XD]
		add ecx, [DXS]
		inc eax
		add edx, [DYS]
		cmp eax, 8
		jb short %%Same_Col

		mov eax, [Rot_Comp.Reg_5C]
		mov ebx, [Buffer_Adr]
		and eax, 0x001F
		shl eax, 5
		lea ebx, [ebx + eax + 32]
		xor eax, eax
		mov [Buffer_Adr], ebx

	%%Same_Col
		dec dword [H_Dot]
		mov [XD], eax
		jnz near %%LOOP_H

%endmacro


; macro MAKE_IMAGE
; construit l'image destination (rotation/compression)
;
; Paramètres :
; %1 = 0 pour normal et 1 pour titled
; %2 = 0 pour 16x16 dot et 1 pour 32x32 dot
; %3 = 0 pour 1x1 screen et 1 pour 16x16 screen
; %4 = Prioritie mode : 0 = OFF, 1 = Underwrite, 2 = Overwrite
;
; Résultat :
; tout le buffer image a été caculée

%macro MAKE_IMAGE 4

		mov eax, [Rot_Comp.Reg_60]
		mov ebx, [YD]
		mov ecx, [Rot_Comp.Reg_5E]
		and eax, 0x7
		shl ebx, 2
		and ecx, 0xFFF8
		mov [XD], eax
		lea ecx, [_Ram_Word_2M + ecx * 4 + ebx]
		mov eax, [Rot_Comp.Reg_62]
		mov [Buffer_Adr], ecx
		and eax, 0x01FF
		mov ebx, [Vector_Adr]
		mov [H_Dot], eax
		mov ecx, [ebx]
		mov edx, [ebx + 2]
		shl ecx, 8
		and edx, 0xFFFF
		and ecx, 0x00FFFF00
		shl edx, 8
		movsx eax, word [ebx + 4]
		mov [DXS], eax
		movsx eax, word [ebx + 6]
		add ebx ,8
		mov [DYS], eax
		test dword [H_Dot], 0x1FF
		mov [Vector_Adr], ebx
		jz near %%Nothing_To_Draw

		MAKE_IMAGE_LINE %1, %2, %3, %4

	%%Nothing_To_Draw

		inc dword [YD]
		dec byte [V_Dot]

		jmp .Finish
	
%endmacro


section .bss align=64
	
	; External symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define	_Ram_Word_2M		Ram_Word_2M
		%define	_Ram_Word_State		Ram_Word_State
		%define	_S68K_Mem_PM		S68K_Mem_PM
		%define	_Int_Mask_S68K		Int_Mask_S68K
	%endif
	
	extern _Ram_Word_2M
	extern _Ram_Word_State
	extern _S68K_Mem_PM
	extern _Int_Mask_S68K
	
;	DECL Table_Rot_Time
;		resd 64

	DECL Rot_Comp
	DECL Rot_Comp.Reg_58
	Stamp_Size	resd 1
	DECL Rot_Comp.Reg_5A
		resd 1
	DECL Rot_Comp.Reg_5C
		resd 1
	DECL Rot_Comp.Reg_5E
		resd 1
	DECL Rot_Comp.Reg_60
		resd 1
	DECL Rot_Comp.Reg_62
		resd 1
	DECL Rot_Comp.Reg_64
	V_Dot		resd 1
	DECL Rot_Comp.Reg_66
		resd 1
	DECL Rotation_Running
		resd 1
	
	; Converted to DECL for GENS Re-Recording
	DECL Stamp_Map_Adr
		resd 1
	DECL Buffer_Adr
		resd 1
	DECL Vector_Adr
		resd 1
	DECL Jmp_Adr
		resd 1
	DECL Float_Part
		resd 1
	DECL Draw_Speed
		resd 1

	; Converted to DECL for GENS Re-Recording
	DECL XS
		resd 1
	DECL YS
		resd 1
	DECL DXS
		resd 1
	DECL DYS
		resd 1
	DECL XD
		resd 1
	DECL YD
		resd 1
	DECL XD_Mul
		resd 1
	DECL H_Dot
		resd 1


section .data align=64

	DECL Table_Rot_Time

	dd	0x00054000, 0x00048000, 0x00040000, 0x00036000		; 008-032		; briefing - sprite
	dd	0x0002E000, 0x00028000, 0x00024000, 0x00022000		; 036-064		; arbre souvent
	dd	0x00021000, 0x00020000, 0x0001E000, 0x0001B800		; 068-096		; map thunderstrike
	dd	0x00019800, 0x00017A00, 0x00015C00, 0x00013E00		; 100-128		; logo défoncé

	dd	0x00012000, 0x00011800, 0x00011000, 0x00010800		; 132-160		; briefing - map
	dd	0x00010000, 0x0000F800, 0x0000F000, 0x0000E800		; 164-192
	dd	0x0000E000, 0x0000D800, 0x0000D000, 0x0000C800		; 196-224
	dd	0x0000C000, 0x0000B800, 0x0000B000, 0x0000A800		; 228-256		; batman visage

	dd	0x0000A000, 0x00009F00, 0x00009E00, 0x00009D00		; 260-288
	dd	0x00009C00, 0x00009B00, 0x00009A00, 0x00009900		; 292-320
	dd	0x00009800, 0x00009700, 0x00009600, 0x00009500		; 324-352
	dd	0x00009400, 0x00009300, 0x00009200, 0x00009100		; 356-384

	dd	0x00009000, 0x00008F00, 0x00008E00, 0x00008D00		; 388-416
	dd	0x00008C00, 0x00008B00, 0x00008A00, 0x00008900		; 420-448
	dd	0x00008800, 0x00008700, 0x00008600, 0x00008500		; 452-476
	dd	0x00008400, 0x00008300, 0x00008200, 0x00008100		; 480-512


section .text align=64

	extern _sub68k_interrupt

	; void Init_RS_GFX(void)
	DECL Init_RS_GFX

		xor eax, eax
		mov [Rot_Comp.Reg_58], eax
		mov [Rot_Comp.Reg_5A], eax
		mov [Rot_Comp.Reg_5C], eax
		mov [Rot_Comp.Reg_5E], eax
		mov [Rot_Comp.Reg_60], eax
		mov [Rot_Comp.Reg_62], eax
		mov [Rot_Comp.Reg_64], eax
		mov [Rot_Comp.Reg_66], eax
		ret

	ALIGN32

	DECL Calcul_Rot_Comp
		push ebx
		push ecx

		cmp byte [_Ram_Word_State], 1
		ja near .End

		mov eax, [Rot_Comp.Reg_5C]
		mov ebx, [Stamp_Size]
		and eax, 0x001F
		and ebx, 0x7
		lea eax, [eax * 4 + 4]
		mov [XD_Mul], eax

		mov eax, [Rot_Comp.Reg_5E]
		or ebx, [_S68K_Mem_PM]
		and eax, 0xFFF8
		mov ecx, [Rot_Comp.Reg_60]
		lea eax, [_Ram_Word_2M + eax * 4]
		shr ecx, 3
		mov [Buffer_Adr], eax

		and ecx, 0x7
		and ebx, 0x1F
		mov [YD], ecx

		mov eax, [Rot_Comp.Reg_62]
		mov ecx, [Rot_Comp.Reg_66]
		and eax, 0x1FF
		and ecx, 0xFFFE
		shr eax, 3

		lea ecx, [_Ram_Word_2M + ecx * 4]
		mov ebx, [Table_Jump_Rot + ebx * 4]
		mov [Vector_Adr], ecx
		mov [Jmp_Adr], ebx						; we save the jump address (for later use)

		mov eax, [Table_Rot_Time + eax * 4]
		mov ecx, [Stamp_Size]
		mov [Draw_Speed], eax
		mov [Float_Part], eax					; we start a new GFX operation
		or ecx, 0x8000							; we start a new GFX operation
		mov eax, [Rot_Comp.Reg_5A]
		test ecx, 0x4
		mov [Stamp_Size], ecx

		jnz short .Scr_16
		test ecx, 0x2
		jnz short .Dot_32

		and eax, 0xFF80
		lea eax, [_Ram_Word_2M + eax * 4]
		mov [Stamp_Map_Adr], eax
		jmp short .Initialised

	ALIGN4

	.Dot_32
		and eax, 0xFFE0
		lea eax, [_Ram_Word_2M + eax * 4]
		mov [Stamp_Map_Adr], eax
		jmp short .Initialised

	ALIGN4

	.Scr_16
		test ecx, 0x2
		jnz short .Scr_16_Dot_32

		lea eax, [_Ram_Word_2M + 0x20000]
		mov [Stamp_Map_Adr], eax
		jmp short .Initialised

	ALIGN4

	.Scr_16_Dot_32
		and eax, 0xE000
		lea eax, [_Ram_Word_2M + eax * 4]
		mov [Stamp_Map_Adr], eax

	ALIGN4
	
	.Initialised
		call Update_Rot

	.End
		pop ecx
		pop ebx
		ret


	ALIGN32

	DECL Update_Rot
		test byte [V_Dot], 0xFF
		mov eax, [Float_Part]
		jnz short .GFX_Not_Completed

		and dword [Stamp_Size], 0x7FFF
		test byte [_Int_Mask_S68K], 0x02
		mov dword [Rot_Comp.Reg_64], 0			; GFX Completed
		jz short .INT1_OFF_0

		push dword -1
		push dword 1
		call _sub68k_interrupt
		add esp, 8

	.INT1_OFF_0
		ret

	ALIGN4

	.GFX_Not_Completed
		test eax, 0xFFFF0000
		jnz short .Have_To_Draw

		add eax, [Draw_Speed]
		mov [Float_Part], eax
		ret

	ALIGN4
	
	.Have_To_Draw
		push edx
		push ebx
		push ecx
		push edi
		push esi

		mov ebx, eax
		and eax, 0xFFFF
		shr ebx, 16
		add eax, [Draw_Speed]
		push ebx
		mov [Float_Part], eax

		jmp [Jmp_Adr]

	ALIGN4

	Table_Jump_Rot
		dd .Norm_D16_S1, .Titled_D16_S1
		dd .Norm_D32_S1, .Titled_D32_S1
		dd .Norm_D16_S16, .Titled_D16_S16
		dd .Norm_D32_S16, .Titled_D32_S16

		dd .U_Norm_D16_S1, .U_Titled_D16_S1
		dd .U_Norm_D32_S1, .U_Titled_D32_S1
		dd .U_Norm_D16_S16, .U_Titled_D16_S16
		dd .U_Norm_D32_S16, .U_Titled_D32_S16

		dd .O_Norm_D16_S1, .O_Titled_D16_S1
		dd .O_Norm_D32_S1, .O_Titled_D32_S1
		dd .O_Norm_D16_S16, .O_Titled_D16_S16
		dd .O_Norm_D32_S16, .O_Titled_D32_S16

		dd .Norm_D16_S1, .Titled_D16_S1
		dd .Norm_D32_S1, .Titled_D32_S1
		dd .Norm_D16_S16, .Titled_D16_S16
		dd .Norm_D32_S16, .Titled_D32_S16

	ALIGN32

	.Norm_D16_S1

		MAKE_IMAGE 0,0,0,0
		
	ALIGN32

	.Titled_D16_S1

		MAKE_IMAGE 1,0,0,0

	ALIGN32

	.Norm_D32_S1

		MAKE_IMAGE 0,1,0,0
		
	ALIGN32

	.Titled_D32_S1

		MAKE_IMAGE 1,1,0,0

	ALIGN32

	.Norm_D16_S16

		MAKE_IMAGE 0,0,1,0

	ALIGN32

	.Titled_D16_S16

		MAKE_IMAGE 1,0,1,0

	ALIGN32

	.Norm_D32_S16

		MAKE_IMAGE 0,1,1,0
		
	ALIGN32

	.Titled_D32_S16

		MAKE_IMAGE 1,1,1,0

	ALIGN32

	.U_Norm_D16_S1

		MAKE_IMAGE 0,0,0,1
		
	ALIGN32

	.U_Titled_D16_S1

		MAKE_IMAGE 1,0,0,1

	ALIGN32

	.U_Norm_D32_S1

		MAKE_IMAGE 0,1,0,1
		
	ALIGN32

	.U_Titled_D32_S1

		MAKE_IMAGE 1,1,0,1

	ALIGN32

	.U_Norm_D16_S16

		MAKE_IMAGE 0,0,1,1

	ALIGN32

	.U_Titled_D16_S16

		MAKE_IMAGE 1,0,1,1

	ALIGN32

	.U_Norm_D32_S16

		MAKE_IMAGE 0,1,1,1
		
	ALIGN32

	.U_Titled_D32_S16

		MAKE_IMAGE 1,1,1,1

	ALIGN32

	.O_Norm_D16_S1

		MAKE_IMAGE 0,0,0,2
		
	ALIGN32

	.O_Titled_D16_S1

		MAKE_IMAGE 1,0,0,2

	ALIGN32

	.O_Norm_D32_S1

		MAKE_IMAGE 0,1,0,2
		
	ALIGN32

	.O_Titled_D32_S1

		MAKE_IMAGE 1,1,0,2

	ALIGN32

	.O_Norm_D16_S16

		MAKE_IMAGE 0,0,1,2

	ALIGN32

	.O_Titled_D16_S16

		MAKE_IMAGE 1,0,1,2

	ALIGN32

	.O_Norm_D32_S16

		MAKE_IMAGE 0,1,1,2
		
	ALIGN32

	.O_Titled_D32_S16

		MAKE_IMAGE 1,1,1,2

	ALIGN4
	
	.Finish
		jz short GFX_Completed
		dec dword [esp]
		jz short GFX_Part_Completed
		jmp [Jmp_Adr]

	ALIGN4

	GFX_Part_Completed
		pop eax
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN4

	GFX_Completed
		and dword [Stamp_Size], 0x7FFF
		test byte [_Int_Mask_S68K], 0x02
		mov dword [Rot_Comp.Reg_64], 0			; GFX Completed

		jz short .INT1_OFF

		push dword -1
		push dword 1
		call _sub68k_interrupt
		add esp, 8

	.INT1_OFF
		pop eax
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


