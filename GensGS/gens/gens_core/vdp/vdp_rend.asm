;
; Gens: VDP Rendering functions.
;

HIGH_B		equ 0x80
SHAD_B		equ 0x40
PRIO_B		equ 0x01
SPR_B		equ 0x20

HIGH_W		equ 0x8080
SHAD_W		equ 0x4040
NOSHAD_W	equ 0xBFBF
PRIO_W		equ 0x0100
SPR_W		equ 0x2000

SHAD_D		equ 0x40404040
NOSHAD_D	equ 0xBFBFBFBF

section .data align=64
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_TAB336		TAB336
		%define	_TAB320		TAB320
		%define	_VDP_Layers	VDP_Layers
	%endif
	
	global _TAB336
	_TAB336:
	%assign i 0
	%rep 240
		dd (i * 336)
	%assign i i+1
	%endrep
	
	align 32
	
	global _TAB320
	_TAB320:
	%assign i 0
	%rep 240
		dd (i * 336)
	%assign i i+1
	%endrep
	
	align 32
	
	Mask_N:
		dd 0xFFFFFFFF, 0xFFF0FFFF, 0xFF00FFFF, 0xF000FFFF
		dd 0x0000FFFF, 0x0000FFF0, 0x0000FF00, 0x0000F000
	
	Mask_F:
		dd 0xFFFFFFFF, 0xFFFF0FFF, 0xFFFF00FF, 0xFFFF000F
		dd 0xFFFF0000, 0x0FFF0000, 0x00FF0000, 0x000F0000
	
	align 32
	
	; VDP layer flags (ported from Gens Rerecording; original by Nitsuja)
	VDP_LAYER_SCROLLA_LOW		equ	(1 << 0)
	VDP_LAYER_SCROLLA_HIGH		equ	(1 << 1)
	VDP_LAYER_SCROLLA_SWAP		equ	(1 << 2)
	VDP_LAYER_SCROLLB_LOW		equ	(1 << 3)
	VDP_LAYER_SCROLLB_HIGH		equ	(1 << 4)
	VDP_LAYER_SCROLLB_SWAP		equ	(1 << 5)
	VDP_LAYER_SPRITE_LOW		equ	(1 << 6)
	VDP_LAYER_SPRITE_HIGH		equ	(1 << 7)
	VDP_LAYER_SPRITE_SWAP		equ	(1 << 8)
	VDP_LAYER_SPRITE_ALWAYSONTOP	equ	(1 << 9)
	VDP_LAYER_PALETTE_LOCK		equ	(1 << 10)
	
	; Default layer flags
	VDP_LAYER_DEFAULT		equ	VDP_LAYER_SCROLLA_LOW	| \
						VDP_LAYER_SCROLLA_HIGH	| \
						VDP_LAYER_SCROLLB_LOW	| \
						VDP_LAYER_SCROLLB_HIGH	| \
						VDP_LAYER_SPRITE_LOW	| \
						VDP_LAYER_SPRITE_HIGH
	
	; VDP_Layers: Active layers and layer settings.
	global _VDP_Layers
	_VDP_Layers:
		dd VDP_LAYER_DEFAULT
	
section .bss align=64
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_VRam			VRam
		%define	_CRam			CRam
		%define	_VSRam			VSRam
		%define	_VDP_Reg		VDP_Reg
		
		%define	_ScrA_Addr		ScrA_Addr
		%define	_ScrB_Addr		ScrB_Addr
		%define	_Win_Addr		Win_Addr
		%define	_Spr_Addr		Spr_Addr
		%define	_H_Scroll_Addr		H_Scroll_Addr
		%define	_H_Cell			H_Cell
		%define	_H_Win_Mul		H_Win_Mul
		%define	_H_Pix			H_Pix
		%define	_H_Pix_Begin		H_Pix_Begin
		
		%define _H_Scroll_Mask		H_Scroll_Mask
		%define	_H_Scroll_CMul		H_Scroll_CMul
		%define	_H_Scroll_CMask		H_Scroll_CMask
		%define	_V_Scroll_CMask		V_Scroll_CMask
		%define	_V_Scroll_MMask		V_Scroll_MMask
		
		%define	_Win_X_Pos		Win_X_Pos
		%define	_Win_Y_Pos		Win_Y_Pos
		
		%define	_VDP_Status		VDP_Status
		%define	_VDP_Current_Line	VDP_Current_Line
		%define	_CRam_Flag		CRam_Flag
		%define	_VRam_Flag		VRam_Flag
		
		%define	_bppMD			bppMD
	%endif
	
	extern _VRam
	extern _CRam
	extern _VSRam
	extern _VDP_Reg
	
	extern _ScrA_Addr
	extern _ScrB_Addr
	extern _Win_Addr
	extern _Spr_Addr
	extern _H_Scroll_Addr
	extern _H_Cell
	extern _H_Win_Mul
	extern _H_Pix
	extern _H_Pix_Begin
	
	extern _H_Scroll_Mask
	extern _H_Scroll_CMul
	extern _H_Scroll_CMask
	extern _V_Scroll_CMask
	extern _V_Scroll_MMask
	
	extern _Win_X_Pos
	extern _Win_Y_Pos
	
	extern _VDP_Status
	extern _VDP_Current_Line
	extern _CRam_Flag
	extern _VRam_Flag
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	__32X_VDP_Ram			_32X_VDP_Ram
		
		%define	__32X_VDP			_32X_VDP
		%define __32X_VDP.Mode			_32X_VDP.Mode
		%define __32X_VDP.State			_32X_VDP.State
		%define __32X_VDP.AF_Data		_32X_VDP.AF_Data
		%define __32X_VDP.AF_St			_32X_VDP.AF_St
		%define __32X_VDP.AF_Len		_32X_VDP.AF_Len
		%define __32X_VDP.AF_Line		_32X_VDP.AF_Line
	%endif
	
	extern __32X_VDP_Ram
	extern __32X_VDP
	
	; MD bpp
	extern _bppMD
	
	struc vx
		.Mode:		resd 1
		.State:		resd 1
		.AF_Data:	resd 1
		.AF_St:		resd 1
		.AF_Len:	resd 1
	endstruc
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_MD_Screen		MD_Screen
		%define	_MD_Palette		MD_Palette
		%define	_Palette		Palette
		%define	_MD_Screen32		MD_Screen32
		%define	_MD_Palette32		MD_Palette32
		%define	_Palette32		Palette32
		%define	_Sprite_Struct		Sprite_Struct
		%define	_Sprite_Visible		Sprite_Visible
		%define	__32X_Rend_Mode		_32X_Rend_Mode
		%define	_Sprite_Over		Sprite_Over
	%endif
	
	; MD screen buffer (16-bit)
	alignb 16
		resw (320 + 32)
	global _MD_Screen
	_MD_Screen:
		resw (336 * 240)
		resw (320 + 32)
	
	; MD active palette (16-bit)
	alignb 16
	global _MD_Palette
	_MD_Palette:
		resw 0x100
	
	; MD total palette (16-bit)
	alignb 16
	global _Palette
	_Palette:
		resw 0x1000
	
	; MD screen buffer (32-bit)
	alignb 16
		resd (320 + 32)
	global _MD_Screen32
	_MD_Screen32:
		resd (336 * 240)
		resd (320 + 32)
	
	; MD active palette (32-bit)
	alignb 16
	global _MD_Palette32
	_MD_Palette32:
		resd 0x100
	
	; MD total palette (32-bit)
	alignb 16
	global _Palette32
	_Palette32:
		resd 0x1000
	
	global _Sprite_Struct
	_Sprite_Struct:
		resd (0x100 * 8)
	
	global _Sprite_Visible
	_Sprite_Visible:
		resd 0x100
	
	Data_Spr:
		.H_Min:		resd 1
		.H_Max:		resd 1
	
	alignb 32
	
	Data_Misc:
		.Pattern_Adr:	resd 1
		.Line_7:	resd 1
		.X:		resd 1
		.Cell:		resd 1
		.Start_A:	resd 1
		.Length_A:	resd 1
		.Start_W:	resd 1
		.Length_W:	resd 1
		.Mask:		resd 1
		.Spr_End:	resd 1
		.Next_Cell:	resd 1
		.Palette:	resd 1
		.Borne:		resd 1
	
	alignb 16
	
	; _32X_Rend_Mode is used for the 32X 32-bit color C macros.
	; See g_32x_32bit.h
	global __32X_Rend_Mode
	__32X_Rend_Mode:
		resb 1
	
	; _Sprite_Over: If set, enforces the sprite limit.
	global _Sprite_Over
	_Sprite_Over:
		resd 1
	
section .text align=64

	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_Render_Line		Render_Line
		%define	_Render_Line_32X	Render_Line_32X
	%endif

;****************************************

; macro GET_X_OFFSET
; param :
; %1 = 0 for scroll B and 1 scroll A
; return :
; - esi: contains X offset of the line in court
; - edi: contains the number of line in court

%macro GET_X_OFFSET 1
	
	mov	eax, [_VDP_Current_Line]
	mov	ebx, [_H_Scroll_Addr]		; ebx points to the H-Scroll data
	mov	edi, eax
	and	eax, [_H_Scroll_Mask]
	
%if %1 > 0
	mov	esi, [ebx + eax * 4]		; X Cell offset
%else
	mov	si, [ebx + eax * 4 + 2]		; X Cell offset
%endif
	
%endmacro


;****************************************

; macro UPDATE_Y_OFFSET
; takes :
; eax = cell courant
; param :
; %1 = 0 for scroll B and 1 for scroll A
; %2 = 0 for normal mode and 1 for interlaced mode
; returns :
; edi = Y Offset in function of current cell

%macro UPDATE_Y_OFFSET 2
	
	mov	eax, [Data_Misc.Cell]		; Current cell for the V Scroll
	test	eax, 0xFF81			; outside the limits of the VRAM? Then don't change...
	jnz	short %%End
	mov	edi, [_VDP_Current_Line]	; edi = line number
	
%if %1 > 0
	mov	eax, [_VSRam + eax * 2 + 0]
%else
	mov	ax, [_VSRam + eax * 2 + 2]
%endif
	
%if %2 > 0
	shr	eax, 1				; divide Y scroll by 2 if interlaced
%endif
	
	add	edi, eax
	mov	eax, edi
	shr	edi, 3				; V Cell Offset
	and	eax, byte 7			; adjust for pattern
	and	edi, [_V_Scroll_CMask]		; prevent V Cell Offset from overflowing
	mov	[Data_Misc.Line_7], eax
	
%%End

%endmacro


;****************************************

; macro GET_PATTERN_INFO
; takes :
; - H_Scroll_CMul must be correctly initialized
; - esi and edi contain X offset and Y offset respectively
; param :
; %1 = 0 for scroll B and 1 for scroll A
; returns :
; -  ax = Pattern Info

%macro GET_PATTERN_INFO 1
	
	mov	cl, [_H_Scroll_CMul]
	mov	eax, edi			; eax = V Cell Offset
	mov	edx, esi			; edx = H Cell Offset
	
	shl	eax, cl				; eax = V Cell Offset * H Width
	
%if %1 > 0
	mov	ebx, [_ScrA_Addr]
%else
	mov	ebx, [_ScrB_Addr]
%endif
	
	add	edx, eax			; edx = (V Offset / 8) * H Width + (H Offset / 8)
	mov	ax, [ebx + edx * 2]		; ax = Cell Info inverse
	
%endmacro


;****************************************

; macro GET_PATTERN_DATA
; param :
; %1 = 0 for normal mode and 1 for interlaced mode
; %2 = 0 pour les scrolls, 1 for la window
; takes :
; - ax = Pattern Info
; - edi contains Y offset (if %2 = 0)
; - Data_Misc.Line_7 contains Line & 7 (if %2! = 0)
; returns :
; - ebx = Pattern Data
; - edx = Palette Num * 16

%macro GET_PATTERN_DATA 2
	
	mov	ebx, [Data_Misc.Line_7]		; ebx = V Offset
	mov	edx, eax			; edx = Cell Info
	mov	ecx, eax			; ecx = Cell Info
	shr	edx, 9
	and	ecx, 0x7FF
	and	edx, byte 0x30			; edx = Palette
	
%if %1 > 0
	shl	ecx, 6				; numero pattern * 64 (entrelacé)
%else
	shl	ecx, 5				; numero pattern * 32 (normal)
%endif
	
	test 	eax, 0x1000			; V-Flip?
	jz	%%No_V_Flip			; if yes, then
	
	xor	ebx, byte 7
	
%%No_V_Flip
	
%if %1 > 0
	mov	ebx, [_VRam + ecx + ebx * 8]		; ebx = Line of the pattern = Pattern Data (interlaced)
%else
	mov	ebx, [_VRam + ecx + ebx * 4]		; ebx = Line of the pattern = Pattern Data (normal)
%endif
	
%endmacro


;****************************************

; macro MAKE_SPRITE_STRUCT
; param :
; %1 = 0 for normal mode and 1 for interlaced mode

%macro MAKE_SPRITE_STRUCT 1
	
	mov	ebp, [_Spr_Addr]
	xor	edi, edi				; edi = 0
	mov	esi, ebp				; esi point on the table of sprite data
	jmp	short %%Loop
	
	align 16
	
%%Loop
		mov	ax, [ebp + 0]			; ax = Pos Y
		mov	cx, [ebp + 6]			; cx = Pos X
		mov	dl, [ebp + (2 ^ 1)]		; dl = Sprite Size
	%if %1 > 0
		shr	eax, 1				; if interlaced, the position is divided by 2
	%endif
		mov	dh, dl
		and	eax, 0x1FF
		and	ecx, 0x1FF
		and	edx, 0x0C03				; isolate Size X and Size Y in dh and dl respectively
		sub	eax, 0x80				; eax = Pos Y correct
		sub	ecx, 0x80				; ecx = Pos X correct
		shr	dh, 2					; dh = Size X - 1
		mov	[_Sprite_Struct + edi + 4], eax		; store Pos Y
		inc	dh					; dh = Size X
		mov	[_Sprite_Struct + edi + 0], ecx		; store Pos X
		mov	bl, dh					; bl = Size X
		mov	[_Sprite_Struct + edi + 8], dh		; store Size X
		and	ebx, byte 7				; ebx = Size X
		mov	[_Sprite_Struct + edi + 12], dl		; store Size Y - 1
		and	edx, byte 3				; edx = Size Y - 1
		lea	ecx, [ecx + ebx * 8 - 1]		; ecx = Pos X Max
		lea	eax, [eax + edx * 8 + 7]		; eax = Pos Y Max
		mov	bl, [ebp + (3 ^ 1)]			; bl = Pointer towards next the sprite
		mov	[_Sprite_Struct + edi + 16], ecx	; store Pos X Max
		mov	dx, [ebp + 4]				; dx = 1st tile of the sprite
		mov	[_Sprite_Struct + edi + 20], eax	; store Pos Y Max
		add	edi, byte (8 * 4)			; advance to the next sprite structure
		and	ebx, byte 0x7F				; clear the highest order bit.
		mov	[_Sprite_Struct + edi - 32 + 24], dx	; store the first tile of the sprite
		jz	short %%End				; if the next pointer is 0, end
		lea	ebp, [esi + ebx * 8]			; ebp Pointer towards next the sprite
		cmp	edi, (8 * 4 * 80)			; if there are already 80 sprites defined then stop
		jb	near %%Loop
		
%%End
	sub	edi, 8 * 4
	mov	[Data_Misc.Spr_End], edi		; store the pointer to the last sprite
	
%endmacro


;****************************************

; macro MAKE_SPRITE_STRUCT_PARTIAL
; param :

%macro MAKE_SPRITE_STRUCT_PARTIAL 0
	
	mov	ebp, [_Spr_Addr]
;	xor	eax, eax
	xor	ebx, ebx
	xor	edi, edi				; edi = 0
	mov	esi, ebp				; esi point on the table of sprite data
	jmp	short %%Loop
	
	align 16
	
%%Loop
		mov	al, [ebp + (2 ^ 1)]			; al = Sprite Size
		mov	bl, [ebp + (3 ^ 1)]			; bl = point towards the next sprite
		mov	cx, [ebp + 6]				; cx = Pos X
		mov	dx, [ebp + 4]				; dx = 1st tile of the sprite
		and	ecx, 0x1FF
		mov	[_Sprite_Struct + edi + 24], dx		; store the 1st tile of the sprite
		sub	ecx, 0x80				; ecx = Pos X correct
		and	eax, 0x0C
		mov	[_Sprite_Struct + edi + 0], ecx		; store Pos X
		lea	ecx, [ecx + eax * 2 + 7]		; ecx = Pos X Max
		and	bl, 0x7F				; clear the highest order bit.
		mov	[_Sprite_Struct + edi + 16], ecx	; store Pos X Max
		jz	short %%End				; if the next pointer is 0, end
		
		add	edi, byte (8 * 4)			; advance to the next sprite structure
		lea	ebp, [esi + ebx * 8]			; ebp Pointer towards next the sprite
		cmp	edi, (8 * 4 * 80)			; if there are already 80 sprites defined then stop
		jb	short %%Loop

%%End

%endmacro


;****************************************

; macro UPDATE_MASK_SPRITE
; param :
; %1 = Sprite Limit Emulation (1 = enable et 0 = disable)
; takes :
; - Sprite_Struct must be correctly initialized
; returns :
; - edi points on the first sprite structure to post.
; - edx contains the number of line

%macro UPDATE_MASK_SPRITE 1
	
	xor	edi, edi
%if %1 > 0
	mov	ecx, [_H_Cell]
%endif
	xor	ax, ax				; used for masking
	mov	ebx, [_H_Pix]
	xor	esi, esi
	mov	edx, [_VDP_Current_Line]
	jmp	short %%Loop_1
	
	align 16
	
%%Loop_1
		cmp	[_Sprite_Struct + edi + 4], edx		; one tests if the sprite is on the current line
		jg	short %%Out_Line_1
		cmp	[_Sprite_Struct + edi + 20], edx	; one tests if the sprite is on the current line
		jl	short %%Out_Line_1
		
%if %1 > 0
		sub	ecx, [_Sprite_Struct + edi + 8]
%endif
		cmp	[_Sprite_Struct + edi + 0], ebx		; one tests if the sprite is not outside of the screen
		jge	short %%Out_Line_1_2
		cmp	dword [_Sprite_Struct + edi + 16], 0	; one tests if the sprite is not outside of the screen
		jl	short %%Out_Line_1_2
		
		mov	[_Sprite_Visible + esi], edi
		add	esi, byte 4
		
%%Out_Line_1_2
		add	edi, byte (8 * 4)
		cmp	edi, [Data_Misc.Spr_End]
		jle	short %%Loop_2
		
		jmp	%%End
	
	align 16
	
%%Out_Line_1
		add	edi, byte (8 * 4)
		cmp	edi, [Data_Misc.Spr_End]
		jle	short %%Loop_1
		
		jmp	%%End
	
	align 16
	
%%Loop_2
		cmp	[_Sprite_Struct + edi + 4], edx		; one tests if the sprite is on the current line
		jg	short %%Out_Line_2
		cmp	[_Sprite_Struct + edi + 20], edx	; one tests if the sprite is on the current line
		jl	short %%Out_Line_2
		
%%Loop_2_First
		cmp	dword [_Sprite_Struct + edi + 0], -128	; is the sprite is a mask?
		je	short %%End				; next sprites are masked
		
%if %1 > 0
		sub	ecx, [_Sprite_Struct + edi + 8]
%endif
		cmp	[_Sprite_Struct + edi + 0], ebx		; one tests if the sprite is not outside of the screen
		jge	short %%Out_Line_2
		cmp	dword [_Sprite_Struct + edi + 16], 0	; one tests if the sprite is not outside of the screen
		jl	short %%Out_Line_2
		
		mov	[_Sprite_Visible + esi], edi
		add	esi, byte 4

%%Out_Line_2
		add	edi, byte (8 * 4)
%if %1 > 0
		cmp	ecx, byte 0
		jle	short %%Sprite_Overflow
%endif
		cmp	edi, [Data_Misc.Spr_End]
		jle	short %%Loop_2
		jmp	short %%End
		
	align 16
	
%%Sprite_Overflow
	cmp	edi, [Data_Misc.Spr_End]
	jg	short %%End
	jmp	short %%Loop_3
	
	align 16
	
	%%Loop_3
		cmp	[_Sprite_Struct + edi + 4], edx		; one tests if the sprite is on the current line
		jg	short %%Out_Line_3
		cmp	[_Sprite_Struct + edi + 20], edx	; one tests if the sprite is on the current line
		jl	short %%Out_Line_3

		or 	byte [_VDP_Status], 0x40
		jmp	short %%End
		
%%Out_Line_3
		add	edi, byte (8 * 4)
		cmp	edi, [Data_Misc.Spr_End]
		jle	short %%Loop_3
		jmp	short %%End
	
	align 16
	
%%End
	mov	[Data_Misc.Borne], esi
	
%endmacro


;****************************************

; macro PUTPIXEL_P0
; param :
; %1 = Number of the pixel
; %2 = Mask to isolate the good pixel
; %3 = Shift
; %4 = 0 for scroll B and one if not
; %5 = Shadow/Highlight enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 64

%macro PUTPIXEL_P0 5
	
	mov	eax, ebx
	and	eax, %2
	jz	short %%Trans
	
%if %4 > 0
	%if %5 > 0
		mov	cl, [_MD_Screen + ebp * 2 + (%1 * 2) + 1]
		test	cl, PRIO_B
		jnz	short %%Trans
	%else
		test	byte [_MD_Screen + ebp * 2 + (%1 * 2) + 1], PRIO_B
		jnz	short %%Trans
	%endif
%endif

%if %3 > 0
	shr	eax, %3
%endif

%if %4 > 0
	%if %5 > 0
		and	cl, SHAD_B
		add	al, dl
		add	al, cl
	%else
		add	al, dl
	%endif
%else
	%if %5 > 0
		lea	eax, [eax + edx + SHAD_W]
	%else
		add	al, dl
	%endif
%endif
	
	mov	[_MD_Screen + ebp * 2 + (%1 * 2)], al	; set the pixel
	
%%Trans

%endmacro


;****************************************
; background layer graphics background graphics layer 1
; macro PUTPIXEL_P1
; param :
; %1 = pixel number
; %2 = mask to isolate the good pixel
; %3 = Shift
; %4 = Shadow/Highlight enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 64

%macro PUTPIXEL_P1 4
	
	mov	eax, ebx
	and	eax, %2
	jz	short %%Trans
	
%if %3 > 0
	shr	eax, %3
%endif
	
	lea	eax, [eax + edx + PRIO_W]
	mov	[_MD_Screen + ebp * 2 + (%1 * 2)], ax
	
%%Trans

%endmacro


;****************************************

; macro PUTPIXEL_SPRITE
; param :
; %1 = pixel number
; %2 = mask to isolate the good pixel
; %3 = Shift
; %4 = Priority
; %5 = Highlight/Shadow Enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 16

%macro PUTPIXEL_SPRITE 5

	mov	eax, ebx
	and	eax, %2
	jz	short %%Trans

	mov	cl, [_MD_Screen + ebp * 2 + (%1 * 2) + 16 + 1]
	test	cl, (PRIO_B + SPR_B - %4)
	jz	short %%Affich
	
%%Prio
	or	ch, cl
%if %4 < 1
	or	byte [_MD_Screen + ebp * 2 + (%1 * 2) + 16 + 1], SPR_B
%endif
	jmp	%%Trans
	
	align 16
	
%%Affich
	
%if %3 > 0
	shr	eax, %3
%endif
	
	lea	eax, [eax + edx + SPR_W]
	
%if %5 > 0
	%if %4 < 1
		and	cl, SHAD_B | HIGH_B
	%else
		and	cl, HIGH_B
	%endif
	
	cmp	eax, (0x3E + SPR_W)
	jb	short %%Normal
	ja	short %%Shadow
	
%%Highlight
	or	word [_MD_Screen + ebp * 2 + (%1 * 2) + 16], HIGH_W
	jmp	short %%Trans
	
%%Shadow
	or	word [_MD_Screen + ebp * 2 + (%1 * 2) + 16], SHAD_W
	jmp	short %%Trans

%%Normal
	add	al, cl
	
%endif
	
	mov	[_MD_Screen + ebp * 2 + (%1 * 2) + 16], ax
	
%%Trans

%endmacro


;****************************************

; macro PUTLINE_P0
; param :
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_P0 2

%if %1 < 1
	%if %2 > 0
		mov	dword [_MD_Screen + ebp * 2 +  0], SHAD_D
		mov	dword [_MD_Screen + ebp * 2 +  4], SHAD_D
		mov	dword [_MD_Screen + ebp * 2 +  8], SHAD_D
		mov	dword [_MD_Screen + ebp * 2 + 12], SHAD_D
	%else
		mov	dword [_MD_Screen + ebp * 2 +  0], 0x00000000
		mov	dword [_MD_Screen + ebp * 2 +  4], 0x00000000
		mov	dword [_MD_Screen + ebp * 2 +  8], 0x00000000
		mov	dword [_MD_Screen + ebp * 2 + 12], 0x00000000
	%endif
	
	; If ScrollA Low is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLA_LOW
	jz	near %%Full_Trans
%else
	; If ScrollB Low is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLB_LOW
	jz	near %%Full_Trans
%endif
	
	test	ebx, ebx
	jz	near %%Full_Trans
	
	PUTPIXEL_P0 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_P0 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_P0 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_P0 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_P0 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_P0 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_P0 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_P0 7, 0x000f0000, 16, %1, %2
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P0
; param :
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_FLIP_P0 2

%if %1 < 1
	%if %2 > 0
		mov	dword [_MD_Screen + ebp * 2 +  0], SHAD_D
		mov	dword [_MD_Screen + ebp * 2 +  4], SHAD_D
		mov	dword [_MD_Screen + ebp * 2 +  8], SHAD_D
		mov	dword [_MD_Screen + ebp * 2 + 12], SHAD_D
	%else
		mov	dword [_MD_Screen + ebp * 2 +  0], 0x00000000
		mov	dword [_MD_Screen + ebp * 2 +  4], 0x00000000
		mov	dword [_MD_Screen + ebp * 2 +  8], 0x00000000
		mov	dword [_MD_Screen + ebp * 2 + 12], 0x00000000
	%endif
	
	; If ScrollA Low is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLA_LOW
	jz	near %%Full_Trans
%else
	; If ScrollB Low is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLB_LOW
	jz	near %%Full_Trans
%endif
	
	test	ebx, ebx
	jz	near %%Full_Trans
	
	PUTPIXEL_P0 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_P0 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_P0 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_P0 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_P0 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_P0 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_P0 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_P0 7, 0x0000f000, 12, %1, %2
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_P1
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_P1 2

%if %1 < 1
	mov	dword [_MD_Screen + ebp * 2 +  0], 0x00000000
	mov	dword [_MD_Screen + ebp * 2 +  4], 0x00000000
	mov	dword [_MD_Screen + ebp * 2 +  8], 0x00000000
	mov	dword [_MD_Screen + ebp * 2 + 12], 0x00000000
	
	; If ScrollA High is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLA_HIGH
	jz	near %%Full_Trans
%else
	; If ScrollB High is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLB_HIGH
	jz	near %%Full_Trans
	
	%if %2 > 0
		; Faster on most CPUs (because of pairable instructions)
		mov	eax, [_MD_Screen + ebp * 2 +  0]
		mov	ecx, [_MD_Screen + ebp * 2 +  4]
		and	eax, NOSHAD_D
		and	ecx, NOSHAD_D
		mov	[_MD_Screen + ebp * 2 +  0], eax
		mov	[_MD_Screen + ebp * 2 +  4], ecx
		mov	eax, [_MD_Screen + ebp * 2 +  8]
		mov	ecx, [_MD_Screen + ebp * 2 + 12]
		and	eax, NOSHAD_D
		and	ecx, NOSHAD_D
		mov	[_MD_Screen + ebp * 2 +  8], eax
		mov	[_MD_Screen + ebp * 2 + 12], ecx
		
		; Faster on K6 CPU
		;and	dword [_MD_Screen + ebp * 2 +  0], NOSHAD_D
		;and	dword [_MD_Screen + ebp * 2 +  4], NOSHAD_D
		;and	dword [_MD_Screen + ebp * 2 +  8], NOSHAD_D
		;and	dword [_MD_Screen + ebp * 2 + 12], NOSHAD_D
	%endif
%endif
	
	test	ebx, ebx
	jz	near %%Full_Trans
	
	PUTPIXEL_P1 0, 0x0000f000, 12, %2
	PUTPIXEL_P1 1, 0x00000f00,  8, %2
	PUTPIXEL_P1 2, 0x000000f0,  4, %2
	PUTPIXEL_P1 3, 0x0000000f,  0, %2
	PUTPIXEL_P1 4, 0xf0000000, 28, %2
	PUTPIXEL_P1 5, 0x0f000000, 24, %2
	PUTPIXEL_P1 6, 0x00f00000, 20, %2
	PUTPIXEL_P1 7, 0x000f0000, 16, %2
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P1
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_FLIP_P1 2

%if %1 < 1
	mov	dword [_MD_Screen + ebp * 2 +  0], 0x00000000
	mov	dword [_MD_Screen + ebp * 2 +  4], 0x00000000
	mov	dword [_MD_Screen + ebp * 2 +  8], 0x00000000
	mov	dword [_MD_Screen + ebp * 2 + 12], 0x00000000
	
	; If ScrollA High is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLA_HIGH
	jz	near %%Full_Trans
%else
	; If ScrollB High is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLB_HIGH
	jz	near %%Full_Trans
	
	%if %2 > 0
		; Faster on most CPUs (because of pairable instructions)
		mov	eax, [_MD_Screen + ebp * 2 +  0]
		mov	ecx, [_MD_Screen + ebp * 2 +  4]
		and	eax, NOSHAD_D
		and	ecx, NOSHAD_D
		mov	[_MD_Screen + ebp * 2 +  0], eax
		mov	[_MD_Screen + ebp * 2 +  4], ecx
		mov	eax, [_MD_Screen + ebp * 2 +  8]
		mov	ecx, [_MD_Screen + ebp * 2 + 12]
		and	eax, NOSHAD_D
		and	ecx, NOSHAD_D
		mov	[_MD_Screen + ebp * 2 +  8], eax
		mov	[_MD_Screen + ebp * 2 + 12], ecx

		; Faster on K6 CPU
		;and	dword [_MD_Screen + ebp * 2 +  0], NOSHAD_D
		;and	dword [_MD_Screen + ebp * 2 +  4], NOSHAD_D
		;and	dword [_MD_Screen + ebp * 2 +  8], NOSHAD_D
		;and	dword [_MD_Screen + ebp * 2 + 12], NOSHAD_D
	%endif
%endif
	
	test	ebx, ebx
	jz	near %%Full_Trans
	
	PUTPIXEL_P1 0, 0x000f0000, 16, %2
	PUTPIXEL_P1 1, 0x00f00000, 20, %2
	PUTPIXEL_P1 2, 0x0f000000, 24, %2
	PUTPIXEL_P1 3, 0xf0000000, 28, %2
	PUTPIXEL_P1 4, 0x0000000f,  0, %2
	PUTPIXEL_P1 5, 0x000000f0,  4, %2
	PUTPIXEL_P1 6, 0x00000f00,  8, %2
	PUTPIXEL_P1 7, 0x0000f000, 12, %2
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_SPRITE
; param :
; %1 = Priority
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest mais sans le screen

%macro PUTLINE_SPRITE 2
	
%if %1 > 0
	; If Sprite High is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SPRITE_HIGH
	jz	near %%Full_Trans
%else
	; If Sprite Low is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SPRITE_LOW
	jz	near %%Full_Trans
%endif
	
	xor	ecx, ecx
	add	ebp, [esp]
	
	PUTPIXEL_SPRITE 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_SPRITE 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 7, 0x000f0000, 16, %1, %2
	
	and	ch, 0x20
	sub	ebp, [esp]
	or	byte [_VDP_Status], ch
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_SPRITE_FLIP
; param :
; %1 = Priority
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_SPRITE_FLIP 2
	
%if %1 > 0
	; If Sprite High is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SPRITE_HIGH
	jz	near %%Full_Trans
%else
	; If Sprite Low is disabled, don't do anything.
	test	dword [_VDP_Layers], VDP_LAYER_SPRITE_LOW
	jz	near %%Full_Trans
%endif
	
	xor	ecx, ecx
	add	ebp, [esp]
	
	PUTPIXEL_SPRITE 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_SPRITE 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 7, 0x0000f000, 12, %1, %2
	
	and	ch, 0x20
	sub	ebp, [esp]
	or	byte [_VDP_Status], ch
	
%%Full_Trans

%endmacro

	
;****************************************

; macro UPDATE_PALETTE
; param :
; %1 = Highlight/Shadow Enable

%macro UPDATE_PALETTE 1
	
	; Check if the palette is locked.
	test	dword [_VDP_Layers], VDP_LAYER_PALETTE_LOCK
	jnz	near %%End
	
	xor	eax, eax
	mov	byte [_CRam_Flag], 0		; one updates the palette, one gives the flag has 0 for modified
	mov	cx, 0x7BEF
	xor	edx, edx
	mov	ebx, (64 / 2) - 1		; ebx = Number of Colours
	
	; Check for 15-bit color (555)
	cmp	byte [_bppMD], 15
	jne	short %%Loop

	; 15-bit color requires an extra step.
	mov	cx, 0x3DEF
	jmp	short %%Loop
	
	align 16
	
%%Loop
		mov	ax, [_CRam + ebx * 4 + 0]		; ax = data color
		mov	dx, [_CRam + ebx * 4 + 2]		; dx = data color
		and	ax, 0x0FFF
		and	dx, 0x0FFF
		mov	ax, [_Palette + eax * 2]
		mov	dx, [_Palette + edx * 2]
		mov	[_MD_Palette + ebx * 4 + 0], ax		; normal color
		mov	[_MD_Palette + ebx * 4 + 2], dx		; normal color
		
%if %1 > 0
		mov	[_MD_Palette + ebx * 4 + 192 * 2 + 0], ax	; normal color
		mov	[_MD_Palette + ebx * 4 + 192 * 2 + 2], dx	; normal color
		shr	ax, 1
		shr	dx, 1
		and	ax, cx						; one divides by 2 the components for the shadow
		and	dx, cx						; one divides by 2 the components for the shadow
		mov	[_MD_Palette + ebx * 4 + 64 * 2 + 0], ax	; darkened color
		mov	[_MD_Palette + ebx * 4 + 64 * 2 + 2], dx	; darkened color
		add	ax, cx
		add	dx, cx
		mov	[_MD_Palette + ebx * 4 + 128 * 2 + 0], ax	; lightened color
		mov	[_MD_Palette + ebx * 4 + 128 * 2 + 2], dx	; lightened color
%endif
		
		dec	ebx			; if the 64 colors were not yet made
		jns	short %%Loop		; then one continues
		
		mov	ebx, [_VDP_Reg + 7 * 4]
		and	ebx, byte 0x3F
		mov	ax, [_MD_Palette + ebx * 2]
		mov	[_MD_Palette + 0 * 2], ax
		
%if %1 > 0
		mov	[_MD_Palette + 0 * 2 + 192 * 2], ax
		shr	ax, 1
		and	ax, cx
		mov	[_MD_Palette + 0 * 2 + 64 * 2], ax
		add	ax, cx
		mov	[_MD_Palette + 0 * 2 + 128 * 2], ax
%endif
%%End
%endmacro

; %1 = Highlight/Shadow Enable

%macro UPDATE_PALETTE32 1
	
	; Check if the palette is locked.
	test	dword [_VDP_Layers], VDP_LAYER_PALETTE_LOCK
	jnz	near %%End
	
	xor	eax, eax
	mov	byte [_CRam_Flag], 0		; one updates the palette, one gives the flag has 0 for modified
	mov	ecx, 0x007F7F7F
	xor	edx, edx
	mov	ebx, (64 / 2) - 1		; ebx = Number of Colours
	jmp	short %%Loop32
	
	align 16
	
%%Loop32
		mov	ax, [_CRam + ebx * 4 + 0]			; ax = data color
		mov	dx, [_CRam + ebx * 4 + 2]			; dx = data color
		and	eax, 0x0FFF
		and	edx, 0x0FFF
		mov	eax, [_Palette32 + eax * 4]
		mov	edx, [_Palette32 + edx * 4]
		mov	[_MD_Palette32 + ebx * 8 + 0], eax		; normal color
		mov	[_MD_Palette32 + ebx * 8 + 4], edx		; normal color
		
%if %1 > 0
		mov	[_MD_Palette32 + ebx * 8 + 192 * 4 + 0], eax	; normal color
		mov	[_MD_Palette32 + ebx * 8 + 192 * 4 + 4], edx	; normal color
		shr	eax, 1
		shr	edx, 1
		and	eax, ecx					; one divides by 2 the components for the shadow
		and	edx, ecx					; one divides by 2 the components for the shadow
		mov	[_MD_Palette32 + ebx * 8 + 64 * 4 + 0], eax	; darkened color
		mov	[_MD_Palette32 + ebx * 8 + 64 * 4 + 4], edx	; darkened color
		add	eax, ecx
		add	edx, ecx
		mov	[_MD_Palette32 + ebx * 8 + 128 * 4 + 0], eax	; lightened color
		mov	[_MD_Palette32 + ebx * 8 + 128 * 4 + 4], edx	; lightened color
%endif
		
		dec	ebx			; if the 64 colors were not yet made
		jns	short %%Loop32		; then one continues
		
		mov	ebx, [_VDP_Reg + 7 * 4]
		and	ebx, byte 0x3F
		mov	eax, [_MD_Palette32 + ebx * 4]
		mov	[_MD_Palette32 + 0 * 4], eax
		
%if %1 > 0
		mov	[_MD_Palette32 + 0 * 4 + 192 * 4], eax
		shr	eax, 1
		and	eax, ecx
		mov	[_MD_Palette32 + 0 * 4 + 64 * 4], eax
		add	eax, ecx
		mov	[_MD_Palette32 + 0 * 4 + 128 * 4], eax
%endif
	
	UPDATE_PALETTE %1
	
%%End
%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_B
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 if V-Scroll mode in 2 cell and 0 if full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_B 3
	
	mov	ebp, [esp]			; ebp point on surface where one renders
	
	GET_X_OFFSET 0
	
	mov	eax, esi			; eax = scroll X inv
	xor	esi, 0x3FF			; esi = scroll X norm
	and	eax, byte 7			; eax = completion for offset
	shr	esi, 3				; esi = current cell
	add	ebp, eax			; ebp updated for clipping
	mov	ebx, esi
	and	esi, [_H_Scroll_CMask]		; prevent H Cell Offset from overflowing
	and	ebx, byte 1
	mov	eax, [_H_Cell]
	sub	ebx, byte 2			; start with cell -2 or -1 (for V Scroll)
	mov	[Data_Misc.X], eax		; number of cells to post
	mov	[Data_Misc.Cell], ebx		; Current cell for the V Scroll
	
	mov	edi, [_VDP_Current_Line]	; edi = line number
	mov	eax, [_VSRam + 2]
	
%if %1 > 0
	shr	eax, 1				; divide Y scroll in 2 if interlaced
%endif
	
	add	edi, eax
	mov	eax, edi
	shr	edi, 3				; V Cell Offset
	and	eax, byte 7			; adjust for pattern
	and	edi, [_V_Scroll_CMask]		; prevent V Cell Offset from overflowing
	mov	[Data_Misc.Line_7], eax
	
	jmp	short %%First_Loop
	
	align 16
	
	%%Loop
	
%if %2 > 0
		UPDATE_Y_OFFSET 0, %1
%endif
	
	%%First_Loop
		
		GET_PATTERN_INFO 0
		GET_PATTERN_DATA %1, 0
		
		; Check for swapped Scroll B priority.
		test	dword [_VDP_Layers], VDP_LAYER_SCROLLB_SWAP
		jz	short %%No_Swap_ScrollB_Priority
		xor	ax, 0x8000
		
	%%No_Swap_ScrollB_Priority
		test	eax, 0x0800		; test if H-Flip?
		jz	near %%No_H_Flip	; if yes, then
		
	%%H_Flip
			
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%H_Flip_P1
			
	%%H_Flip_P0
				PUTLINE_FLIP_P0 0, %3
				jmp	%%End_Loop
				
				align 16
				
	%%H_Flip_P1
				PUTLINE_FLIP_P1 0, %3
				jmp	%%End_Loop
				
				align 16
				
	%%No_H_Flip
			
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%No_H_Flip_P1
			
	%%No_H_Flip_P0
				PUTLINE_P0 0, %3
				jmp 	%%End_Loop
				
				align 16
				
	%%No_H_Flip_P1
				PUTLINE_P1 0, %3
				jmp	short %%End_Loop
				
				align 16
				
	%%End_Loop
		inc	dword [Data_Misc.Cell]		; Next H cell for the V Scroll
		inc	esi				; Next H cell
		add	ebp, byte 8			; advance to the next pattern
		and	esi, [_H_Scroll_CMask]		; prevent H Offset from overflowing
		dec	byte [Data_Misc.X]		; decrement number of cells to treat
		jns	near %%Loop
		
%%End


%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_A_WIN
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 si V-Scroll mode en 2 cell et 0 si full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_A_WIN 3
	
	mov	eax, [_VDP_Current_Line]
	mov	cl, [_VDP_Reg + 18 * 4]
	shr	eax, 3
	mov	ebx, [_H_Cell]
	shr	cl, 7				; cl = 1 si window at bottom
	cmp	eax, [_Win_Y_Pos]
	setae	ch				; ch = 1 si current line >= pos Y window
	xor	cl, ch				; cl = 0 si line window sinon line Scroll A
	jz	near %%Full_Win
	
	test	byte [_VDP_Reg + 17 * 4], 0x80
	mov	edx, [_Win_X_Pos]
	jz	short %%Win_Left
	
%%Win_Right
	sub	ebx, edx
	mov	[Data_Misc.Start_W], edx	; Start Win (Cell)
	mov	[Data_Misc.Length_W], ebx	; Length Win (Cell)
	dec	edx				; 1 cell en moins car on affiche toujours le dernier à part
	mov	dword [Data_Misc.Start_A], 0	; Start Scroll A (Cell)
	mov	[Data_Misc.Length_A], edx	; Length Scroll A (Cell)
	jns	short %%Scroll_A
	jmp	%%Window
	
	align 16
	
%%Win_Left
	sub	ebx, edx
	mov	dword [Data_Misc.Start_W], 0	; Start Win (Cell)
	mov	[Data_Misc.Length_W], edx	; Length Win (Cell)
	dec	ebx				; 1 cell en moins car on affiche toujours le dernier à part
	mov	[Data_Misc.Start_A], edx	; Start Scroll A (Cell)
	mov	[Data_Misc.Length_A], ebx	; Length Scroll A (Cell)
	jns	short %%Scroll_A
	jmp	%%Window
	
	align 16
	
%%Scroll_A
	mov	ebp, [esp]			; ebp point on surface where one renders
	
	GET_X_OFFSET 1
	
	mov	eax, esi			; eax = scroll X inv
	mov	ebx, [Data_Misc.Start_A]	; Premier Cell
	xor	esi, 0x3FF			; esi = scroll X norm
	and	eax, byte 7			; eax = completion pour offset
	shr	esi, 3				; esi = cell courant (début scroll A)
	mov	[Data_Misc.Mask], eax		; mask pour le dernier pattern
	mov	ecx, esi			; ecx = cell courant (début scroll A) 
	add	esi, ebx			; esi = cell courant ajusté pour window clip
	and	ecx, byte 1
	lea	eax, [eax + ebx * 8]		; clipping + window clip
	sub	ecx, byte 2			; on démarre au cell -2 ou -1 (pour le V Scroll)
	and	esi, [_H_Scroll_CMask]		; on empeche H Cell Offset de deborder
	add	ebp, eax			; ebp mis à jour pour clipping + window clip
	add	ebx, ecx			; ebx = Cell courant pour le V Scroll
	
	mov	edi, [_VDP_Current_Line]	; edi = line number
	mov	[Data_Misc.Cell], ebx		; Cell courant pour le V Scroll
	jns	short %%Not_First_Cell
	
	mov	eax, [_VSRam + 0]
	jmp	short %%First_VScroll_OK
	
%%Not_First_Cell
	and	ebx, [_V_Scroll_MMask]
	mov	eax, [_VSRam + ebx * 2]
	
%%First_VScroll_OK
	
%if %1 > 0
	shr	 eax, 1				; divide Y scroll in 2 if interlaced
%endif
	
	add	edi, eax
	mov	eax, edi
	shr	edi, 3				; V Cell Offset
	and	eax, byte 7			; adjust for pattern
	and	edi, [_V_Scroll_CMask]		; prevent V Cell Offset from overflowing
	mov	[Data_Misc.Line_7], eax
	
	jmp	short %%First_Loop_SCA
	
	align 16
	
%%Loop_SCA

%if %2 > 0
		UPDATE_Y_OFFSET 1, %1
%endif

%%First_Loop_SCA
		
		GET_PATTERN_INFO 1
		GET_PATTERN_DATA %1, 0
		
		; Check for swapped Scroll A priority.
		test	dword [_VDP_Layers], VDP_LAYER_SCROLLA_SWAP
		jz	short %%No_Swap_ScrollA_Priority_1
		xor	ax, 0x8000
		
	%%No_Swap_ScrollA_Priority_1
		test	 eax, 0x0800		; test if H-Flip ?
		jz	near %%No_H_Flip	; if yes, then
		
	%%H_Flip
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%H_Flip_P1
			
	%%H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp	%%End_Loop
				
				align 16
				
	%%H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp	%%End_Loop
				
				align 16
				
	%%No_H_Flip
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%No_H_Flip_P1
			
	%%No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp	%%End_Loop
				
				align 16
				
	%%No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp	short %%End_Loop
				
				align 16
				
	%%End_Loop
		inc	dword [Data_Misc.Cell]		; Next H cell for the V Scroll
		inc	esi				; Next H cell
		add	ebp, byte 8			; advance to the next pattern
		and	esi, [_H_Scroll_CMask]		; prevent H Offset from overflowing
		dec	byte [Data_Misc.Length_A]	; decrement number of cells to treat for Scroll A
		jns	near %%Loop_SCA

%%LC_SCA

%if %2 > 0
	UPDATE_Y_OFFSET 1, %1
%endif

	GET_PATTERN_INFO 1
	GET_PATTERN_DATA %1, 0
	
	; Check for swapped Scroll A priority.
	test	dword [_VDP_Layers], VDP_LAYER_SCROLLA_SWAP
	jz	short %%No_Swap_ScrollA_Priority_2
	xor	ax, 0x8000
	
%%No_Swap_ScrollA_Priority_2
	test	eax, 0x0800			; test if H-Flip ?
	mov	ecx, [Data_Misc.Mask]
	jz	near %%LC_SCA_No_H_Flip		; if yes, then

	%%LC_SCA_H_Flip
		and	ebx, [Mask_F + ecx * 4]		; apply the mask
		test	eax, 0x8000			; test the priority of the current pattern
		jnz	near %%LC_SCA_H_Flip_P1
		
	%%LC_SCA_H_Flip_P0
			PUTLINE_FLIP_P0 1, %3
			jmp	%%LC_SCA_End
			
			align 16
			
	%%LC_SCA_H_Flip_P1
			PUTLINE_FLIP_P1 1, %3
			jmp	%%LC_SCA_End
			
			align 16
			
	%%LC_SCA_No_H_Flip
		and	ebx, [Mask_N + ecx * 4]		; apply the mask
		test	eax, 0x8000			; test the priority of the current pattern
		jnz	near %%LC_SCA_No_H_Flip_P1

	%%LC_SCA_No_H_Flip_P0
			PUTLINE_P0 1, %3
			jmp	%%LC_SCA_End
			
			align 16
			
	%%LC_SCA_No_H_Flip_P1
			PUTLINE_P1 1, %3
			jmp	short %%LC_SCA_End
			
			align 16
			
%%LC_SCA_End
	test	byte [Data_Misc.Length_W], 0xFF
	jnz	short %%Window
	jmp	%%End
	
	align 16
	
%%Full_Win
	xor	esi, esi	; Start Win (Cell)
	mov	edi, ebx	; Length Win (Cell)
	jmp	short %%Window_Initialised
	
	align 16
	
%%Window
	mov	esi, [Data_Misc.Start_W]
	mov	edi, [Data_Misc.Length_W]		; edi = # of cells to render
	
%%Window_Initialised
	mov	edx, [_VDP_Current_Line]
	mov	cl, [_H_Win_Mul]
	mov	ebx, edx				; ebx = Line
	mov	ebp, [esp]				; ebp point on surface where one renders
	shr	edx, 3					; edx = Line / 8
	mov	eax, [_Win_Addr]
	shl	edx, cl
	lea	ebp, [ebp + esi * 8 + 8]		; no clipping for the window, return directly to the first pixel
	lea	eax, [eax + edx * 2]			; eax point on the pattern data for the window
	and	ebx, byte 7				; ebx = Line & 7 for the V Flip
	mov	[Data_Misc.Pattern_Adr], eax		; store this pointer
	mov	[Data_Misc.Line_7], ebx			; store Line & 7
	jmp	short %%Loop_Win
	
	align 16
	
%%Loop_Win
		mov	ebx, [Data_Misc.Pattern_Adr]
		mov	ax, [ebx + esi * 2]
		
		GET_PATTERN_DATA %1, 1
		
		test	ax, 0x0800		; test if H-Flip ?
		jz	near %%W_No_H_Flip	; if yes, then
		
	%%W_H_Flip
			test	ax, 0x8000		; test the priority of the current pattern
			jnz	near %%W_H_Flip_P1
			
	%%W_H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp	%%End_Loop_Win
				
				align 16
				
	%%W_H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp	%%End_Loop_Win
				
				align 16
				
	%%W_No_H_Flip
			test	ax, 0x8000		; test the priority of the current pattern
			jnz	near %%W_No_H_Flip_P1
			
	%%W_No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp	%%End_Loop_Win
				
				align 16
				
	%%W_No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp	short %%End_Loop_Win
				
				align 16
				
	%%End_Loop_Win
		inc	esi			; next pattern
		add	ebp, byte 8		; next pattern for the render
		dec	edi
		jnz	near %%Loop_Win
		
%%End

%endmacro


;****************************************

; macro RENDER_LINE_SPR
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = Shadow / Highlight (0 = Disable and 1 = Enable)

%macro RENDER_LINE_SPR 2
	
	test	dword [_Sprite_Over], 1
	jz	near %%No_Sprite_Over
	
%%Sprite_Over
	
	UPDATE_MASK_SPRITE 1			; edi point on the sprite to post
	xor	edi, edi
	test	esi, esi
	mov	dword [Data_Misc.X], edi
	jnz	near %%First_Loop
	jmp	%%End				; quit
	
%%No_Sprite_Over
	
	UPDATE_MASK_SPRITE 0			; edi = point on the sprite to post
	xor	edi, edi
	test	esi, esi
	mov	dword [Data_Misc.X], edi
	jnz	short %%First_Loop
	jmp	%%End				; quit
	
	align 16
	
%%Sprite_Loop
		mov	edx, [_VDP_Current_Line]
%%First_Loop
		mov	edi, [_Sprite_Visible + edi]
		mov	eax, [_Sprite_Struct + edi + 24]		; eax = CellInfo of the sprite
		sub	edx, [_Sprite_Struct + edi + 4]			; edx = Line - Y Pos (Y Offset)
		mov	ebx, eax					; ebx = CellInfo
		mov	esi, eax					; esi = CellInfo
		
		; Check for swapped sprite priority.
		test	dword [_VDP_Layers], VDP_LAYER_SPRITE_SWAP
		jz	short %%No_Swap_Sprite_Priority
		xor	ax, 0x8000
		
	%%No_Swap_Sprite_Priority
		shr	bx, 9					; isolate the palette in ebx
		mov	ecx, edx				; ecx = Y Offset
		and	ebx, 0x30				; keep the palette number an even multiple of 16
		
		and	esi, 0x7FF				; esi = number of the first pattern of the sprite
		mov	[Data_Misc.Palette], ebx		; store the palette number * 64 in Palette
		and	edx, 0xF8				; one erases the 3 least significant bits = Num Pattern * 8
		mov	ebx, [_Sprite_Struct + edi + 12]	; ebx = Size Y
		and	ecx, byte 7				; ecx = (Y Offset & 7) = Line of the current pattern
%if %1 > 0
		shl	ebx, 6					; ebx = Size Y * 64
		lea	edx, [edx * 8]				; edx = Num Pattern * 64
		shl	esi, 6					; esi = point on the contents of the pattern
%else
		shl	ebx, 5					; ebx = Size Y * 32
		lea	edx, [edx * 4]				; edx = Num Pattern * 32
		shl	esi, 5					; esi = point on the contents of the pattern
%endif
		
		test	eax, 0x1000				; test for V Flip
		jz	%%No_V_Flip
		
	%%V_Flip
		sub	ebx, edx
		xor	ecx, 7					; ecx = 7 - (Y Offset & 7)
		add	esi, ebx				; esi = point on the pattern to post
%if %1 > 0
		lea	ebx, [ebx + edx + 64]			; restore the value of ebx + 64
		lea	esi, [esi + ecx * 8]			; and load the good line of the pattern
		jmp	short %%Suite
%else
		lea	ebx, [ebx + edx + 32]			; restore the value of ebx + 64
		lea	esi, [esi + ecx * 4]			; and load the good line of the pattern
		jmp	short %%Suite
%endif
		
		align 16
		
	%%No_V_Flip
		add	esi, edx				; esi = point on the pattern to post
%if %1 > 0
		add	ebx, byte 64				; add 64 to ebx
		lea	esi, [esi + ecx * 8]			; and load the good line of the pattern
%else			
		add	ebx, byte 32				; add 32 to ebx
		lea	esi, [esi + ecx * 4]			; and load the good line of the pattern
%endif
		
	%%Suite
		mov	 [Data_Misc.Next_Cell], ebx		; next Cell X of this sprite is with ebx bytes
		mov	edx, [Data_Misc.Palette]		; edx = Palette number * 64
		
		test	eax, 0x800				; test H Flip
		jz	near %%No_H_Flip
			
	%%H_Flip
		mov	ebx, [_Sprite_Struct + edi + 0]
		mov	ebp, [_Sprite_Struct + edi + 16]	; position for X
		cmp	ebx, -7					; test for the minimum edge of the sprite
		mov	edi, [Data_Misc.Next_Cell]
		jg	short %%Spr_X_Min_Norm
		mov	ebx, -7					; minimum edge = clip screen
		
	%%Spr_X_Min_Norm
		mov	[Data_Spr.H_Min], ebx			; spr min = minimum edge
		
	%%Spr_X_Min_OK
		sub	ebp, byte 7				; to post the last pattern in first
		jmp	short %%Spr_Test_X_Max
		
		align 16
		
	%%Spr_Test_X_Max_Loop
			sub	ebp, byte 8			; one moves back on the preceding pattern (screen)
			add	esi, edi			; one goes on next the pattern (mem)
			
	%%Spr_Test_X_Max
			cmp	ebp, [_H_Pix]
			jge	%%Spr_Test_X_Max_Loop
		
		; Check if sprites should always be on top.
		test	dword [_VDP_Layers], VDP_LAYER_SPRITE_ALWAYSONTOP
		jnz	near %%H_Flip_P1
		
		test	eax, 0x8000				; test the priority
		jnz	near %%H_Flip_P1
		jmp	short %%H_Flip_P0
		
		align 16
		
	%%H_Flip_P0
	%%H_Flip_P0_Loop
			mov	ebx, [_VRam + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE_FLIP 0, %2		; one posts the line of the sprite pattern
			
			sub	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Min]		; test if one did all the sprite patterns
			jge	near %%H_Flip_P0_Loop		; if not, continue
		jmp	%%End_Sprite_Loop
		
		align 16
		
	%%H_Flip_P1
	%%H_Flip_P1_Loop
			mov	ebx, [_VRam + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE_FLIP 1, %2		; one posts the line of the sprite pattern
			
			sub	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Min]		; test if one did all the sprite patterns
			jge	near %%H_Flip_P1_Loop		; if not, continue
		jmp	%%End_Sprite_Loop
		
		align 16
		
	%%No_H_Flip
		mov	ebx, [_Sprite_Struct + edi + 16]
		mov	ecx, [_H_Pix]
		mov	ebp, [_Sprite_Struct + edi + 0]		; position the pointer ebp
		cmp	ebx, ecx				; test for the maximum edge of the sprite
		mov	edi, [Data_Misc.Next_Cell]
		jl	%%Spr_X_Max_Norm
		mov	[Data_Spr.H_Max], ecx			; max edge = clip screan
		jmp	short %%Spr_Test_X_Min
		
		align 16
		
	%%Spr_X_Max_Norm
		mov	[Data_Spr.H_Max], ebx			; spr max = max edge
		jmp	short %%Spr_Test_X_Min
		
		align 16
		
	%%Spr_Test_X_Min_Loop
			add	ebp, byte 8			; advance to the next pattern (screen)
			add	esi, edi			; one goes on next the pattern (mem)
			
	%%Spr_Test_X_Min
			cmp	ebp, -7
			jl	%%Spr_Test_X_Min_Loop
		
		; Check if sprites should always be on top.
		test	dword [_VDP_Layers], VDP_LAYER_SPRITE_ALWAYSONTOP
		jnz	near %%No_H_Flip_P1
		
		test	ax, 0x8000				; test the priority
		jnz	near %%No_H_Flip_P1
		jmp	short %%No_H_Flip_P0
		
		align 16
		
	%%No_H_Flip_P0
	%%No_H_Flip_P0_Loop
			mov	ebx, [_VRam + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE 0, %2			; one posts the line of the sprite pattern 
			
			add	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Max]		; test if one did all the sprite patterns
			jl	near %%No_H_Flip_P0_Loop	; if not, continue
		jmp	%%End_Sprite_Loop
		
		align 16
		
	%%No_H_Flip_P1
	%%No_H_Flip_P1_Loop
			mov	ebx, [_VRam + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE 1, %2			; on affiche la ligne du pattern sprite
			
			add	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Max]		; test if one did all the sprite patterns
			jl	near %%No_H_Flip_P1_Loop	; if not, continue
		jmp	short %%End_Sprite_Loop
		
		align 16
		
	%%End_Sprite_Loop
		mov	edi, [Data_Misc.X]
		add	edi, byte 4
		cmp	edi, [Data_Misc.Borne]
		mov	[Data_Misc.X], edi
		jb	near %%Sprite_Loop
		
%%End

%endmacro


;****************************************

; macro RENDER_LINE
; param :
; %1 = 1 for interlace mode and 0 if not
; %2 = Shadow / Highlight (0 = Disable et 1 = Enable)

%macro RENDER_LINE 2
	
	test	dword [_VDP_Reg + 11 * 4], 4
	jz	near %%Full_VScroll
	
%%Cell_VScroll
	RENDER_LINE_SCROLL_B     %1, 1, %2
	RENDER_LINE_SCROLL_A_WIN %1, 1, %2
	jmp	%%Scroll_OK
	
%%Full_VScroll
	RENDER_LINE_SCROLL_B     %1, 0, %2
	RENDER_LINE_SCROLL_A_WIN %1, 0, %2
	
%%Scroll_OK
	RENDER_LINE_SPR          %1, %2
	
%endmacro


; *******************************************************
	
	global _Render_Line
	_Render_Line:
		
		pushad
		
		mov	ebx, [_VDP_Current_Line]
		xor	eax, eax
		mov	edi, [_TAB336 + ebx * 4]
		test	dword [_VDP_Reg + 1 * 4], 0x40		; test if the VDP is active
		push	edi					; we need this value later
		jnz	short .VDP_Enable			; if not, nothing is posted
		
		test	byte [_VDP_Reg + 12 * 4], 0x08
		cld
		mov	ecx, 160
		jz	short .No_Shadow
		
		; Shadow effect is currently enabled.
		mov	eax, 0x40404040
		
	.No_Shadow:
		lea	edi, [_MD_Screen + edi * 2 + 8 * 2]
		rep	stosd
		jmp	.VDP_OK
		
	align 16
	
	.VDP_Enable:
		mov	ebx, [_VRam_Flag]
		mov	eax, [_VDP_Reg + 12 * 4]
		and	ebx, byte 3
		and	eax, byte 4
		mov	byte [_VRam_Flag], 0
		jmp	[.Table_Sprite_Struct + ebx * 8 + eax]
	
	align 16
	
	.Table_Sprite_Struct:
		dd 	.Sprite_Struc_OK
		dd 	.Sprite_Struc_OK
		dd 	.MSS_Complete, .MSS_Complete_Interlace
		dd 	.MSS_Partial, .MSS_Partial_Interlace
		dd 	.MSS_Complete, .MSS_Complete_Interlace
	
	align 16
	
	.MSS_Complete:
			MAKE_SPRITE_STRUCT 0
			jmp .Sprite_Struc_OK
	
	align 16
	
	.MSS_Complete_Interlace:
			MAKE_SPRITE_STRUCT 1
			jmp .Sprite_Struc_OK
	
	align 16
	
	.MSS_Partial:
	.MSS_Partial_Interlace:
			MAKE_SPRITE_STRUCT_PARTIAL
			jmp short .Sprite_Struc_OK
	
	align 16
	
	.Sprite_Struc_OK:
		mov	eax, [_VDP_Reg + 12 * 4]
		and	eax, byte 0xC
		jmp	[.Table_Render_Line + eax]
	
	align 16
	
	.Table_Render_Line:
		dd 	.NHS_NInterlace
		dd 	.NHS_Interlace
		dd 	.HS_NInterlace
		dd 	.HS_Interlace
		
	align 16
	
	.NHS_NInterlace:
			RENDER_LINE 0, 0
			jmp .VDP_OK
	
	align 16
	
	.NHS_Interlace:
			RENDER_LINE 1, 0
			jmp .VDP_OK
	
	align 16
	
	.HS_NInterlace:
			RENDER_LINE 0, 1
			jmp .VDP_OK
	
	align 16
	
	.HS_Interlace:
			RENDER_LINE 1, 1
			jmp short .VDP_OK
	
	align 16
	
	.VDP_OK:
		test	byte [_CRam_Flag], 1		; teste si la palette a etait modifiee
		jz	near .Palette_OK		; si oui
		
		test	byte [_VDP_Reg + 12 * 4], 8
		jnz	near .Palette_HS
		
		UPDATE_PALETTE32 0
		jmp	.Palette_OK
	
	align 16
		
	.Palette_HS:
		UPDATE_PALETTE32 1

	align 16
	
	.Palette_OK:
		cmp	byte [_bppMD], 32	; check if this is 32-bit color
		jne	short .Render16
		
	.Render32: ; 32-bit color
		mov	ecx, 160
		mov	eax, [_H_Pix_Begin]
		mov	edi, [esp]
		sub	ecx, eax
		add	esp, byte 4
		lea	edx, [_MD_Screen + edi * 2 + 8 * 2]
		shr	ecx, 1
		lea	edi, [_MD_Screen32 + edi * 4 + 8 * 4]
		mov	esi, _MD_Palette32
	
	align 16
	
	.Genesis_Loop32:
			movzx	eax, byte [edx + 0]
			movzx	ebx, byte [edx + 2]
			mov	eax, [esi + eax * 4]
			mov	ebx, [esi + ebx * 4]
			mov	[edi + 0], eax
			mov	[edi + 4], ebx
			movzx	ebp, byte [edx + 4]
			movzx	eax, byte [edx + 6]
			mov	ebp, [esi + ebp * 4]
			mov	eax, [esi + eax * 4]
			mov	[edi + 8], ebp
			mov	[edi + 12], eax
			add	edx, byte 8
			add	edi, byte 16
			dec	ecx
			jnz	short .Genesis_Loop32
		
		popad
		ret
	
	.Render16: ; 15-bit/16-bit color
		mov	ecx, 160
		mov	eax, [_H_Pix_Begin]
		mov	edi, [esp]
		sub	ecx, eax
		add	esp, byte 4
		lea	edi, [_MD_Screen + edi * 2 + 8 * 2]
		shr	ecx, 1
		mov	esi, _MD_Palette
		jmp	short .Genesis_Loop16
	
	align 16
	
	.Genesis_Loop16:
			movzx	eax, byte [edi + 0]
			movzx	ebx, byte [edi + 2]
			movzx	edx, byte [edi + 4]
			movzx	ebp, byte [edi + 6]
			mov	ax, [esi + eax * 2]
			mov	bx, [esi + ebx * 2]
			mov	dx, [esi + edx * 2]
			mov	bp, [esi + ebp * 2]
			mov	[edi + 0], ax
			mov	[edi + 2], bx
			mov	[edi + 4], dx
			mov	[edi + 6], bp
			add	edi, byte 8
			
;			movzx	ebx, byte [edi + 0]
;			movzx	ebp, byte [edi + 2]
;			mov	ax, [esi + ebx * 2]
;			mov	dx, [esi + ebp * 2]
;			mov	[edi + 0], ax
;			mov	[edi + 2], dx
;			add	edi, byte 4
			
;			mov	bl, byte [edi + 0]
;			mov	dl, byte [edi + 2]
;			mov	ax, [esi + ebx * 2]
;			mov	bp, [esi + edx * 2]
;			mov	[edi + 0], ax
;			mov	[edi + 2], bp
;			add	edi, byte 4
			
			dec	ecx
			jnz	short .Genesis_Loop16
		
		popad
		ret


; *******************************************************

	global _Render_Line_32X
	_Render_Line_32X:
		
		pushad
		
		mov	ebx, [_VDP_Current_Line]
		xor	eax, eax
		mov	edi, [_TAB336 + ebx * 4]
		test	dword [_VDP_Reg + 1 * 4], 0x40		; test if the VDP is active
		push	edi					; we need this value later
		jnz	short .VDP_Enable			; if not, nothing is posted
			
		test	byte [_VDP_Reg + 12 * 4], 0x08
		cld
		mov	ecx, 160
		jz	short .No_Shadow
		
		; Shadow effect is currently enabled.
		mov	eax, 0x40404040
		
	.No_Shadow:
		lea	edi, [_MD_Screen + edi * 2 + 8 * 2]
		rep	stosd
		jmp	.VDP_OK
		
	align 16
	
	.VDP_Enable:
		mov	ebx, [_VRam_Flag]
		xor	eax, eax
		and	ebx, byte 3
		mov	[_VRam_Flag], eax
		jmp	[.Table_Sprite_Struct + ebx * 4]
	
	align 16
	
	.Table_Sprite_Struct:
		dd 	.Sprite_Struc_OK
		dd 	.MSS_Complete
		dd 	.MSS_Partial
		dd 	.MSS_Complete
	
	align 16
	
	.MSS_Complete:
			MAKE_SPRITE_STRUCT 0
			jmp .Sprite_Struc_OK
	
	align 16
	
	.MSS_Partial:
			MAKE_SPRITE_STRUCT_PARTIAL
	
	align 16
	
	.Sprite_Struc_OK:
		test	byte [_VDP_Reg + 12 * 4], 0x8	; no interlace in 32X mode
		jnz	near .HS
	
	.NHS:
			RENDER_LINE 0, 0
			test	byte [_CRam_Flag], 1		; test if palette was modified
			jz	near .VDP_OK
			UPDATE_PALETTE32 0
			jmp	.VDP_OK
	
	align 16

	.HS:
			RENDER_LINE 0, 1
			test	byte [_CRam_Flag], 1		; test if palette was modified
			jz	near .VDP_OK
			UPDATE_PALETTE32 1
	
	align 16
	
	.VDP_OK:
		mov	ecx, 160
		mov	eax, [_H_Pix_Begin]
		mov	edi, [esp]
		sub	ecx, eax
		add	esp, byte 4
		lea	edi, [_MD_Screen + edi * 2 + 8 * 2]
		mov	esi, [__32X_VDP + vx.State]
		mov	eax, [__32X_VDP + vx.Mode]
		and	esi, byte 1
		mov	edx, eax
		shl	esi, 17
		mov	ebp, eax
		shr	edx, 3
		mov	ebx, [_VDP_Current_Line]
		shr	ebp, 11
		and	eax, byte 3
		and	edx, byte 0x10
		and	ebp, byte 0x20
		mov	bx, [__32X_VDP_Ram + esi + ebx * 2]
		or	edx, ebp
		lea	esi, [__32X_VDP_Ram + esi + ebx * 2]
		
		; Set the 32X render mode for the 32-bit color C macros.
		shr	edx, 2
		mov	[__32X_Rend_Mode], al
		or	[__32X_Rend_Mode], dl
		shl	edx, 2
		
		; 32X line rendering is now done via g_32x_32bit.c.
		popad
		ret
