;******************************************
;
; SH2 emulator 1.60 (ASM source part)
; Copyright 2002 Stéphane Dallongeville
; This file is included in the main ASM
; source file sh2a.asm
;
;******************************************


;***********************************************************************************
;
; The following functions are publics :
;
; * SH2_Read_Byte -> UINT8 SH2_Read_Byte(SH2_CONTEXT *SH2, UINT32 adr)
; * SH2_Read_Word -> UINT16 SH2_Read_Word(SH2_CONTEXT *SH2, UINT32 adr)
; * SH2_Read_Long -> UINT32 SH2_Read_Long(SH2_CONTEXT *SH2, UINT32 adr)
; * SH2_Write_Byte -> void SH2_Write_Byte(SH2_CONTEXT *SH2, UINT32 adr, UINT8 data)
; * SH2_Write_Word -> void SH2_Write_Word(SH2_CONTEXT *SH2, UINT32 adr, UINT16 data)
; * SH2_Write_Long -> void SH2_Write_Long(SH2_CONTEXT *SH2, UINT32 adr, UINT32 data)
;
;***********************************************************************************


;**************************
;
; Some usefull ASM macros
;
;**************************


; By default, regcall functions use the MSVC __fastcall convention.
; In case you're using GCC, uncomment the __GCC definition.

;%define __GCC
;%define __GCC2


%ifdef __GCC2
%macro DECLF 1-2

%if %0 > 1
	global %1%2
%endif
	global _%1
	global %1

%if %0 > 1
	%1%2:
%endif
	_%1:
	%1:

%endmacro
%else
%macro DECLF 1-2

%if %0 > 1
	global @%1@%2
%endif
	global _%1
	global %1

%if %0 > 1
	@%1@%2:
%endif
	_%1:
	%1:

%endmacro
%endif

%macro FUNC_IN 0

%ifdef __GCC
;	push ecx
;	push edx
	mov ecx, eax
%endif

%endmacro


%macro FUNC_OUT 0

%ifdef __GCC
;	pop edx
;	pop ecx
%endif
	ret

%endmacro


%macro FUNC_CALL_IN 0

%ifdef __GCC
	push ebp
	mov eax, ecx
%endif

%endmacro


%macro FUNC_CALL_OUT 0

%ifdef __GCC
	pop ebp
%endif

%endmacro


section .bss align=64
	
	%include "sh2_context.inc"
	
section .text align=64
	
; External symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_SH2_Interrupt_Internal		SH2_Interrupt_Internal
%endif

extern _SH2_Interrupt_Internal

; Symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_SH2_Read_Byte		SH2_Read_Byte
	%define	_SH2_Read_Word		SH2_Read_Word
	%define	_SH2_Read_Long		SH2_Read_Long
	
	%define	_SH2_Write_Byte		SH2_Write_Byte
	%define	_SH2_Write_Word		SH2_Write_Word
	%define	_SH2_Write_Long		SH2_Write_Long
%endif

; UINT8 SH2_Read_Byte(SH2_CONTEXT *SH2, UINT32 adr)
global _SH2_Read_Byte
_SH2_Read_Byte:
	push	ecx
	push	edx
	push	ebp
	
	%ifdef __GCC
		mov	ecx, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	eax, ecx
		shr	ecx, 24
		call	[ebp + SH2.Read_Byte + ecx * 4]
	%else
		mov	eax, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	ecx, eax
		shr	eax, 24
		call	[ebp + SH2.Read_Byte + eax * 4]
	%endif
	
	pop	ebp
	pop	edx
	pop	ecx
	ret


align 32

; UINT16 SH2_Read_Word(SH2_CONTEXT *SH2, UINT32 adr)
global _SH2_Read_Word
_SH2_Read_Word:
	push	ecx
	push	edx
	push	ebp

	%ifdef __GCC
		mov	ecx, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	eax, ecx
		shr	ecx, 24
		call	[ebp + SH2.Read_Word + ecx * 4]
	%else
		mov	eax, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	ecx, eax
		shr	eax, 24
		call	[ebp + SH2.Read_Word + eax * 4]
	%endif
	
	pop	ebp
	pop	edx
	pop	ecx
	ret


align 32

; UINT32 SH2_Read_Long(SH2_CONTEXT *SH2, UINT32 adr)
global _SH2_Read_Long
_SH2_Read_Long:
	push	ecx
	push	edx
	push	ebp
	
	%ifdef __GCC
		mov	ecx, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	eax, ecx
		shr	ecx, 24
		call	[ebp + SH2.Read_Long + ecx * 4]
	%else
		mov	eax, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	ecx, eax
		shr	eax, 24
		call	[ebp + SH2.Read_Long + eax * 4]
	%endif
	
	pop	ebp
	pop	edx
	pop	ecx
	ret


align 32

; void SH2_Write_Byte(SH2_CONTEXT *SH2, UINT32 adr, UINT8 data)
global _SH2_Write_Byte
_SH2_Write_Byte:
	push	ecx
	push	edx
	push	ebp
	
	%ifdef __GCC
		mov	ecx, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	eax, ecx
		shr	ecx, 24
		mov	edx, [esp + 16 + 2 * 4]
		call	[ebp + SH2.Write_Byte + ecx * 4]
	%else
		mov	eax, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	ecx, eax
		shr	eax, 24
		mov	edx, [esp + 16 + 2 * 4]
		call	[ebp + SH2.Write_Byte + eax * 4]
	%endif
	
	pop	ebp
	pop	edx
	pop	ecx
	ret


align 32

; void SH2_Write_Word(SH2_CONTEXT *SH2, UINT32 adr, UINT16 data)
global _SH2_Write_Word
_SH2_Write_Word:
	push	ecx
	push	edx
	push	ebp
	
	%ifdef __GCC
		mov	ecx, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	eax, ecx
		shr	ecx, 24
		mov	edx, [esp + 16 + 2 * 4]
		call	[ebp + SH2.Write_Word + ecx * 4]
	%else
		mov	eax, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	ecx, eax
		shr	eax, 24
		mov	edx, [esp + 16 + 2 * 4]
		call	[ebp + SH2.Write_Word + eax * 4]
	%endif
	
		pop ebp
		pop edx
		pop ecx
		ret


align 32

; void SH2_Write_Long(SH2_CONTEXT *SH2, UINT32 adr, UINT32 data)

global _SH2_Write_Long
_SH2_Write_Long:
	push	ecx
	push	edx
	push	ebp
	
	%ifdef __GCC
		mov	ecx, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	eax, ecx
		shr	ecx, 24
		mov	edx, [esp + 16 + 2 * 4]
		call	[ebp + SH2.Write_Long + ecx * 4]
	%else
		mov	eax, [esp + 16 + 1 * 4]
		mov	ebp, [esp + 16 + 0 * 4]
		mov	ecx, eax
		shr	eax, 24
		mov	edx, [esp + 16 + 2 * 4]
		call	[ebp + SH2.Write_Long + eax * 4]
	%endif
	
	pop	ebp
	pop	edx
	pop	ecx
	ret



;***********************************************************************************
;
; This part contains the externals registers emulation & cache stuff
; ------------------------------------------------------------------
;
; The following functions have to be called only by SH2a.asm
; Register EBP is "alive" (context pointer) and shouldn't be modified.
;
; * SH2_Read_Byte_C0 -> UINT8 FASTCALL SH2_Read_Byte_C0(UINT32 adr)
; * SH2_Read_Word_C0 -> UINT16 FASTCALL SH2_Read_Word_C0(UINT32 adr)
; * SH2_Read_Long_C0 -> UINT32 FASTCALL SH2_Read_Long_C0(UINT32 adr)
; * SH2_Read_Byte_FF -> UINT8 FASTCALL SH2_Read_Byte_FF(UINT32 adr)
; * SH2_Read_Word_FF -> UINT16 FASTCALL SH2_Read_Word_FF(UINT32 adr)
; * SH2_Read_Long_FF -> UINT32 FASTCALL SH2_Read_Long_FF(UINT32 adr)
;
; * SH2_Write_Byte_C0 -> void FASTCALL SH2_Write_Byte_C0(UINT32 adr, UINT8 data)
; * SH2_Write_Word_C0 -> void FASTCALL SH2_Write_Word_C0(UINT32 adr, UINT16 data)
; * SH2_Write_Long_C0 -> void FASTCALL SH2_Write_Long_C0(UINT32 adr, UINT32 data)
; * SH2_Write_Byte_FF -> void FASTCALL SH2_Write_Byte_FF(UINT32 adr, UINT8 data)
; * SH2_Write_Word_FF -> void FASTCALL SH2_Write_Word_FF(UINT32 adr, UINT16 data)
; * SH2_Write_Long_FF -> void FASTCALL SH2_Write_Long_FF(UINT32 adr, UINT32 data)
;
;***********************************************************************************



align 32

; UINT8 FASTCALL SH2_Read_Byte_C0(UINT32 adr)
DECLF SH2_Read_Byte_C0, 4
	FUNC_IN
	and	ecx, 0xFFF
	mov	al, [ebp + SH2.Cache + ecx]
	FUNC_OUT


align 16

; UINT16 FASTCALL SH2_Read_Word_C0(UINT32 adr)
DECLF SH2_Read_Word_C0, 4
	FUNC_IN
	and	ecx, 0xFFE
	mov	ah, [ebp + SH2.Cache + ecx + 0]
	mov	al, [ebp + SH2.Cache + ecx + 1]
	FUNC_OUT


align 16

; UINT32 FASTCALL SH2_Read_Long_C0(UINT32 adr)
DECLF SH2_Read_Long_C0, 4
	FUNC_IN
	and	ecx, 0xFFC
	mov	eax, [ebp + SH2.Cache + ecx]
	bswap	eax
	FUNC_OUT


align 16

; void FASTCALL SH2_Write_Byte_C0(UINT32 adr, UINT8 data)
DECLF SH2_Write_Byte_C0, 8
	FUNC_IN
	and	ecx, 0xFFF
	mov	[ebp + SH2.Cache + ecx], dl
	FUNC_OUT


align 16

; void FASTCALL SH2_Write_Word_C0(UINT32 adr, UINT16 data)
DECLF SH2_Write_Word_C0, 8
	FUNC_IN
	and	ecx, 0xFFE
	mov	[ebp + SH2.Cache + ecx + 0], dh
	mov	[ebp + SH2.Cache + ecx + 1], dl
	FUNC_OUT


align 16

; void FASTCALL SH2_Write_Long_C0(UINT32 adr, UINT32 data)
DECLF SH2_Write_Long_C0, 8
	FUNC_IN
	bswap	edx
	and	ecx, 0xFFC
	mov	[ebp + SH2.Cache + ecx], edx
	FUNC_OUT


align 16

; UINT8 FASTCALL SH2_Read_Byte_FF(UINT32 adr)
DECLF SH2_Read_Byte_FF, 4
	FUNC_IN
	test	ecx, 0xFFFE00
	jz	near .Bad
	
	and	ecx, 0x1FF
	mov	al, [ebp + SH2.IO_Reg + ecx]
	jmp	[.Table_Jump + ecx * 4]

align 16

.Table_Jump:
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E00-E03
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E04-E07
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E08-E0B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E0C-E0F
	
	dd	.TIER,	.FTCSR,	.FRC_H,	.FRC_L		; E10-E13
	dd	.OCRA_H, .OCRA_L, .TCR,	.TOCR		; E14-E17
	dd	.ICR_H,	.ICR_L,	.Bad,	.Bad		; E18-E1B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E1C-E1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E20-E23
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E24-E27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E28-E2B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E2C-E2F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E30-E33
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E34-E37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E38-E3B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E3C-E3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E40-E43
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E44-E47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E48-E4B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E4C-E4F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E50-E53
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E54-E57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E58-E5B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E5C-E5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E60-E63
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E64-E67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E68-E6B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E6C-E6F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E70-E73
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E74-E77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E78-E7B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E7C-E7F
	
	dd	.WDTCSR, .WDTCNT, .Bad, .WDTRST		; E80-E83
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E84-E87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E88-E8B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E8C-E8F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E90-E93
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E94-E97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E98-E9B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E9C-E9F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA0-EA3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA4-EA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA8-EAB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EAC-EAF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB0-EB3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB4-EB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB8-EBB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EBC-EBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC0-EC3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC4-EC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC8-ECB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ECC-ECF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED0-ED3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED4-ED7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED8-EDB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EDC-EDF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE0-EE3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE4-EE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE8-EEB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EEC-EEF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF0-EF3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF4-EF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF8-EFB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EFC-EFF
	
	; ***********************************************************
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F00-F03
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F04-F07
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F08-F0B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F0C-F0F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F10-F13
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F14-F17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F18-F1B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F1C-F1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F20-F23
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F24-F27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F28-F2B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F2C-F2F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F30-F33
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F34-F37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F38-F3B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F3C-F3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F40-F43
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F44-F47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F48-F4B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F4C-F4F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F50-F53
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F54-F57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F58-F5B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F5C-F5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F60-F63
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F64-F67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F68-F6B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F6C-F6F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F70-F73
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F74-F77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F78-F7B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F7C-F7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F80-F83
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F84-F87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F88-F8B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F8C-F8F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F90-F93
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F94-F97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F98-F9B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F9C-F9F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA0-FA3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA4-FA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA8-FAB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FAC-FAF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB0-FB3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB4-FB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB8-FBB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FBC-FBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC0-FC3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC4-FC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC8-FCB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FCC-FCF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD0-FD3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD4-FD7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD8-FDB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FDC-FDF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE0-FE3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE4-FE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE8-FEB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FEC-FEF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF0-FF3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF4-FF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF8-FFB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FFC-FFF

align 16

.Bad:
	FUNC_OUT

align 16

.TIER:
	mov	al, [ebp + SH2.FRTTIER]
	or	al, 0x01
	FUNC_OUT

align 8

.FTCSR:
	mov	al, [ebp + SH2.FRTCSR]
	FUNC_OUT

align 8

.FRC_H:
	mov	al, [ebp + SH2.FRTCNT + 2]
	FUNC_OUT

align 8

.FRC_L:
	mov	al, [ebp + SH2.FRTCNT + 1]
	FUNC_OUT

align 8

.OCRA_H:
	test	byte [ebp + SH2.FRTTOCR], 0x10
	mov	al, [ebp + SH2.FRTOCRA + 2]
	jnz	short .OCRB_H
	FUNC_OUT

align 8

.OCRA_L:
	test	byte [ebp + SH2.FRTTOCR], 0x10
	mov	al, [ebp + SH2.FRTOCRA + 1]
	jnz	short .OCRB_L
	FUNC_OUT

align 8

.OCRB_H:
	mov	al, [ebp + SH2.FRTOCRB + 2]
	FUNC_OUT

align 8

.OCRB_L:
	mov	al, [ebp + SH2.FRTOCRB + 1]
	FUNC_OUT

align 8

.TCR:
	mov	al, [ebp + SH2.FRTTCR]
	FUNC_OUT

align 8

.TOCR:
	mov	al, [ebp + SH2.FRTTOCR]
	or	al, 0xE0
	FUNC_OUT

align 8

.ICR_H:
	mov	al, [ebp + SH2.FRTICR + 2]
	FUNC_OUT

align 8

.ICR_L:
	mov	al, [ebp + SH2.FRTICR + 1]
	FUNC_OUT

align 8

.WDTCSR:
	mov	al, [ebp + SH2.WDTSR]
	FUNC_OUT

align 8

.WDTCNT:
	mov	al, [ebp + SH2.WDTCNT + 2]
	FUNC_OUT

align 8

.WDTRST:
	mov	al, [ebp + SH2.WDTRST]
	FUNC_OUT


align 32

; UINT16 FASTCALL SH2_Read_Word_FF(UINT32 adr)
DECLF SH2_Read_Word_FF, 4
	FUNC_IN
	test	ecx, 0xFFFE00
	jz	near .Bad
	
	and	ecx, 0x1FE
	mov	ah, [ebp + SH2.IO_Reg + ecx + 0]
	mov	al, [ebp + SH2.IO_Reg + ecx + 1]
	jmp	[.Table_Jump + ecx * 2]

align 16

.Table_Jump:
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E00-E07
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E08-E0F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E10-E17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E18-E1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E20-E27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E28-E2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E30-E37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E38-E3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E40-E47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E48-E4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E50-E57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E58-E5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E60-E67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E68-E6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E70-E77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E78-E7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E80-E87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E88-E8F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E90-E97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E98-E9F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA0-EA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA8-EAF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB0-EB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB8-EBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC0-EC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC8-ECF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED0-ED7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED8-EDF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE0-EE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE8-EEF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF0-EF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF8-EFF
	
	; ***********************************************************
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F00-F07
	dd	.DVCR_H, .DVCR_L, .VCRDIV_H, .VCRDIV_L	; F08-F0F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F10-F17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F18-F1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F20-F27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F28-F2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F30-F37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F38-F3F
	
	dd	.BARA_H, .BARA_L, .BAMRA_H, .BAMRA_L	; F40-F47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F48-F4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F50-F57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F58-F5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F60-F67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F68-F6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F70-F77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F78-F7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F80-F87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F88-F8F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F90-F97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F98-F9F
	
	dd	.VCRDMA0_H, .VCRDMA0_L, .VCRDMA1_H, .VCRDMA1_L	; FA0-FA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA8-FAF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB0-FB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB8-FBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC0-FC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC8-FCF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD0-FD7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD8-FDF
	
	dd	.BCR1,	.Bad,	.Bad,	.Bad		; FE0-FE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE8-FEF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF0-FF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF8-FFF

align 16

.Bad:
	FUNC_OUT


align 16

.DVCR_H:
	mov	ax, [ebp + SH2.DVCR + 2]
	FUNC_OUT
	
align 8

.DVCR_L:
	mov	ax, [ebp + SH2.DVCR + 0]
	FUNC_OUT

align 8

.VCRDIV_H:
	mov	ax, [ebp + SH2.VCRDIV + 2]
	FUNC_OUT

align 8

.VCRDIV_L:
	mov	ax, [ebp + SH2.VCRDIV]
	FUNC_OUT

align 8

.VCRDMA0_H:
	mov	ax, [ebp + SH2.VCRDMA0 + 2]
	FUNC_OUT

align 8

.VCRDMA0_L:
	mov	ax, [ebp + SH2.VCRDMA0]
	FUNC_OUT

align 8

.VCRDMA1_H:
	mov	ax, [ebp + SH2.VCRDMA1 + 2]
	FUNC_OUT

align 8

.VCRDMA1_L:
	mov	ax, [ebp + SH2.VCRDMA1]
	FUNC_OUT

align 8

.BARA_H:
	mov	ax, [ebp + SH2.BARA + 2]
	FUNC_OUT

align 8

.BARA_L:
	mov	ax, [ebp + SH2.BARA]
	FUNC_OUT

align 8

.BAMRA_H:
	mov	ax, [ebp + SH2.BAMRA + 2]
	FUNC_OUT

align 8

.BAMRA_L:
	mov	ax, [ebp + SH2.BAMRA]
	FUNC_OUT

align 8

.BCR1:
	mov	ax, [ebp + SH2.BCR1]
	FUNC_OUT


align 32

; UINT32 FASTCALL SH2_Read_Long_FF(UINT32 adr)
DECLF SH2_Read_Long_FF, 4
	FUNC_IN
	test	ecx, 0xFFFE00
	jz	near .Bad
	
	and	ecx, 0x1FC
	mov	eax, [ebp + SH2.IO_Reg + ecx]
	bswap	eax
	jmp	[.Table_Jump + ecx]

align 16

.Table_Jump:
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E00-E0F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E10-E1F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E20-E2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E30-E3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E40-E4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E50-E5F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E60-E6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E70-E7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E80-E8F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E90-E9F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA0-EAF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB0-EBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC0-ECF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED0-EDF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE0-EEF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF0-EFF
	
	; ***********************************************************
	
	dd	.DVSR,	.DVDNT,	.DVCR,	.VCRDIV		; F00-F0F
	dd	.DVDNTH, .DVDNTL, .DVDNTH, .DVDNTL	; F10-F1F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F20-F2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F30-F3F
	
	dd	.BARA,	.BAMRA,	.Bad,	.Bad		; F40-F4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F50-F5F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F60-F6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F70-F7F
	
	dd	.SAR0,	.DAR0,	.TCR0,	.CHCR0		; F80-F8F
	dd	.SAR1,	.DAR1,	.TCR1,	.CHCR1		; F90-F9F
	dd	.VCRDMA0, .VCRDMA1, .Bad, .Bad		; FA0-FAF
	dd	.DMAOR,	.Bad,	.Bad,	.Bad		; FB0-FBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC0-FCF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD0-FDF
	dd	.BCR1,	.Bad,	.Bad,	.Bad		; FE0-FEF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF0-FFF

align 16

.Bad:
	FUNC_OUT

align 16

.DVSR:
	mov	eax, [ebp + SH2.DVSR]
	FUNC_OUT

align 8

.DVDNT:
	mov	eax, [ebp + SH2.DVDNTL]
	FUNC_OUT

align 8

.DVCR:
	mov	eax, [ebp + SH2.DVCR]
	FUNC_OUT

align 8

.VCRDIV:
	mov	eax, [ebp + SH2.VCRDIV]
	FUNC_OUT

align 8

.DVDNTH:
	mov	eax, [ebp + SH2.DVDNTH]
	FUNC_OUT

align 8

.DVDNTL:
	mov 	eax, [ebp + SH2.DVDNTL]
	FUNC_OUT

align 8

.SAR0:
	mov	eax, [ebp + SH2.SAR0]
	FUNC_OUT

align 8

.DAR0:
	mov	eax, [ebp + SH2.DAR0]
	FUNC_OUT

align 8

.TCR0:
	mov	eax, [ebp + SH2.TCR0]
	FUNC_OUT

align 8

.CHCR0:
	mov	eax, [ebp + SH2.CHCR0]
	FUNC_OUT

align 8

.SAR1:
	mov	eax, [ebp + SH2.SAR1]
	FUNC_OUT

align 8

.DAR1:
	mov	eax, [ebp + SH2.DAR1]
	FUNC_OUT

align 8

.TCR1:
	mov	eax, [ebp + SH2.TCR1]
	FUNC_OUT

align 8

.CHCR1:
	mov	eax, [ebp + SH2.CHCR1]
	FUNC_OUT

align 8

.VCRDMA0:
	mov	eax, [ebp + SH2.VCRDMA0]
	FUNC_OUT

align 8

.VCRDMA1:
	mov	eax, [ebp + SH2.VCRDMA1]
	FUNC_OUT

align 8

.DMAOR:
	mov	eax, [ebp + SH2.DMAOR]
	FUNC_OUT

align 8

.BARA:
	mov	eax, [ebp + SH2.BARA]
	FUNC_OUT

align 8

.BAMRA:
	mov	eax, [ebp + SH2.BAMRA]
	FUNC_OUT

align 8

.BCR1:
	mov	eax, [ebp + SH2.BCR1]
	FUNC_OUT


align 32

; void FASTCALL SH2_Write_Byte_FF(UINT32 adr, UINT8 data)
DECLF SH2_Write_Byte_FF, 8
	FUNC_IN
	test	ecx, 0xFFFE00
	jz	near .Bad
	
	and	ecx, 0x1FF
	mov	[ebp + SH2.IO_Reg + ecx], dl
	jmp	[.Table_Jump + ecx * 4]

align 16

.Table_Jump:
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E00-E03
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E04-E07
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E08-E0B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E0C-E0F
	
	dd	.TIER,	.FTCSR,	.FRC_H,	.FRC_L		; E10-E13
	dd	.OCRA_H, .OCRA_L, .TCR,	.TOCR		; E14-E17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E18-E1B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E1C-E1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E20-E23
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E24-E27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E28-E2B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E2C-E2F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E30-E33
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E34-E37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E38-E3B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E3C-E3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E40-E43
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E44-E47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E48-E4B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E4C-E4F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E50-E53
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E54-E57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E58-E5B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E5C-E5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E60-E63
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E64-E67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E68-E6B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E6C-E6F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E70-E73
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E74-E77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E78-E7B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E7C-E7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E80-E83
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E84-E87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E88-E8B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E8C-E8F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E90-E93
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E94-E97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E98-E9B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E9C-E9F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA0-EA3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA4-EA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA8-EAB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EAC-EAF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB0-EB3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB4-EB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB8-EBB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EBC-EBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC0-EC3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC4-EC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC8-ECB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ECC-ECF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED0-ED3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED4-ED7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED8-EDB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EDC-EDF
	
	dd	.Bad,	.Bad,	.IPRA_H, .IPRA_L	; EE0-EE3
	dd	.VCRWDT, .Bad,	.Bad,	.Bad		; EE4-EE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE8-EEB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EEC-EEF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF0-EF3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF4-EF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF8-EFB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EFC-EFF
	
	; ***********************************************************
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F00-F03
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F04-F07
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F08-F0B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F0C-F0F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F10-F13
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F14-F17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F18-F1B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F1C-F1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F20-F23
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F24-F27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F28-F2B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F2C-F2F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F30-F33
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F34-F37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F38-F3B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F3C-F3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F40-F43
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F44-F47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F48-F4B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F4C-F4F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F50-F53
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F54-F57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F58-F5B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F5C-F5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F60-F63
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F64-F67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F68-F6B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F6C-F6F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F70-F73
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F74-F77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F78-F7B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F7C-F7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F80-F83
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F84-F87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F88-F8B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F8C-F8F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F90-F93
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F94-F97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F98-F9B
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F9C-F9F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA0-FA3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA4-FA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA8-FAB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FAC-FAF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB0-FB3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB4-FB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB8-FBB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FBC-FBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC0-FC3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC4-FC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC8-FCB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FCC-FCF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD0-FD3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD4-FD7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD8-FDB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FDC-FDF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE0-FE3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE4-FE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE8-FEB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FEC-FEF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF0-FF3
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF4-FF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF8-FFB
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FFC-FFF

align 16

.Bad:
	FUNC_OUT

align 16

.IPRA_H:
	mov	al, dl
	and	dl, 0xF
	shr	al, 4
	mov	[ebp + SH2.IPDMA], dl
	mov	[ebp + SH2.IPDIV], al
	FUNC_OUT

align 8

.IPRA_L:
	shr	dl, 4
	mov	[ebp + SH2.IPWDT], dl
	FUNC_OUT

align 8

.VCRWDT
	and	dl, 0x7F
	mov	[ebp + SH2.VCRWDT], dl
	FUNC_OUT

align 8

.TIER:
	and	dl, 0x8E
	mov	[ebp + SH2.FRTTIER], dl
	FUNC_OUT

align 16

.FTCSR:
	mov	al, [ebp + SH2.FRTCSR]
	and	al, dl
	and	dl, 1
	or	al, dl
	mov	[ebp + SH2.FRTCSR], al
	FUNC_OUT

align 8

.FRC_H:
	mov	[ebp + SH2.FRTCNT + 2], dl
	FUNC_OUT

align 8

.FRC_L:
	mov	[ebp + SH2.FRTCNT + 1], dl
	FUNC_OUT

align 16

.OCRA_H:
	test	byte [ebp + SH2.FRTTOCR], 0x10
	jnz	short .OCRB_H
	mov	[ebp + SH2.FRTOCRA + 2], dl
	FUNC_OUT

align 16

.OCRA_L:
	test	byte [ebp + SH2.FRTTOCR], 0x10
	jnz	short .OCRB_L
	mov	[ebp + SH2.FRTOCRA + 1], dl
	FUNC_OUT

align 8

.OCRB_H:
	mov	[ebp + SH2.FRTOCRB + 2], dl
	FUNC_OUT

align 8

.OCRB_L:
	mov	[ebp + SH2.FRTOCRB + 1], dl
	FUNC_OUT

align 16

.TCR:
	movzx	eax, dl
	mov	[ebp + SH2.FRTTCR], dl
	and	eax, byte 3
	mov	cl, [ebp + SH2.FRT_Tab + eax]
	mov	[ebp + SH2.FRT_Sft], cl
	FUNC_OUT

align 8

.TOCR:
	mov	[ebp + SH2.FRTTOCR], dl
	FUNC_OUT


align 32

; void FASTCALL SH2_Write_Word_FF(UINT32 adr, UINT16 data)
DECLF SH2_Write_Word_FF, 8
	FUNC_IN
	test	ecx, 0xFFFE00
	jz	near .Bad
	
	and	ecx, 0x1FE
	mov	[ebp + SH2.IO_Reg + ecx + 0], dh
	mov	[ebp + SH2.IO_Reg + ecx + 1], dl
	jmp	[.Table_Jump + ecx * 2]

align 16

.Table_Jump:
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E00-E07
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E08-E0F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E10-E17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E18-E1F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E20-E27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E28-E2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E30-E37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E38-E3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E40-E47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E48-E4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E50-E57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E58-E5F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E60-E67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E68-E6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E70-E77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E78-E7F
	
	dd	.WDTSR,	.WDTRST, .Bad,	.Bad		; E80-E87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E88-E8F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E90-E97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E98-E9F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA0-EA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA8-EAF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB0-EB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB8-EBF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC0-EC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC8-ECF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED0-ED7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED8-EDF

	dd	.Bad, .IPRA, .VCRWDT, .Bad		; EE0-EE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE8-EEF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF0-EF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF8-EFF

	; ***********************************************************

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F00-F07
	dd	.DVCR_H, .DVCR_L, .VCRDIV_H, .VCRDIV_L			; F08-F0F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F10-F17
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F18-F1F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F20-F27
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F28-F2F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F30-F37
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F38-F3F

	dd	.BARA_H, .BARA_L, .BAMRA_H, .BAMRA_L			; F40-F47
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F48-F4F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F50-F57
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F58-F5F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F60-F67
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F68-F6F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F70-F77
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F78-F7F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F80-F87
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F88-F8F

	dd	.Bad,	.Bad,	.Bad,	.Bad		; F90-F97
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F98-F9F

	dd	.VCRDMA0_H, .VCRDMA0_L, .VCRDMA1_H, .VCRDMA1_L	; FA0-FA7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FA8-FAF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB0-FB7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FB8-FBF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC0-FC7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC8-FCF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD0-FD7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD8-FDF

	dd	.BCR1, .Bad, .Bad, .Bad		; FE0-FE7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FE8-FEF

	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF0-FF7
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF8-FFF


align 16

.Bad:
	FUNC_OUT

align 16

.IPRA:
	mov	al, dl
	mov	ah, dh
	shr	al, 4
	and	dl, 0xF
	shr	ah, 4
	and	dh, 0xF
	mov	[ebp + SH2.IPDIV], ah
	mov	[ebp + SH2.IPDMA], dh
	mov	[ebp + SH2.IPWDT], al
	FUNC_OUT

align 8

.VCRWDT:
	and	dh, 0x7F
	mov	[ebp + SH2.VCRWDT], dh
	FUNC_OUT

align 16

.WDTSR:
	cmp	dh, 0xA5
	movzx	eax, dl
	jne	short .WDTCNT
	
	and	eax, byte 7
	mov	[ebp + SH2.WDTSR], dl
	mov	cl, [ebp + SH2.WDT_Tab + eax]
	mov	[ebp + SH2.WDT_Sft], cl
	FUNC_OUT

align 8

.WDTCNT:
	shl	eax, 16
	mov	[ebp + SH2.WDTCNT], eax
	FUNC_OUT
	
align 16

.WDTRST:
	cmp	dh, 0x5A
	mov	al, [ebp + SH2.WDTRST]
	jne	short .WDTRST_other
	
	and	al, 0x80
	and	dl, 0x60
	or	al, dl
	mov	[ebp + SH2.WDTRST], al
	FUNC_OUT

.WDTRST_other:
	test	dl, dl
	js	short .WDTRST_other_nc
	
	and	al, ~0x80
	mov	[ebp + SH2.WDTRST], al

.WDTRST_other_nc:
	FUNC_OUT

align 8

.DVCR_H:
	mov	[ebp + SH2.DVCR + 2], dx
	FUNC_OUT

align 8

.DVCR_L:
	mov	[ebp + SH2.DVCR + 0], dx
	FUNC_OUT

align 8

.VCRDIV_H:
	mov	[ebp + SH2.VCRDIV + 2], dx
	FUNC_OUT

align 8

.VCRDIV_L:
	mov	[ebp + SH2.VCRDIV], dx
	FUNC_OUT

align 8

.VCRDMA0_H:
	mov	[ebp + SH2.VCRDMA0 + 2], dx
	FUNC_OUT

align 8

.VCRDMA0_L:
	mov	[ebp + SH2.VCRDMA0], dx
	FUNC_OUT

align 8

.VCRDMA1_H:
	mov	[ebp + SH2.VCRDMA1 + 2], dx
	FUNC_OUT

align 8

.VCRDMA1_L:
	mov	[ebp + SH2.VCRDMA1], dx
	FUNC_OUT

align 8

.BARA_H:
	mov	[ebp + SH2.BARA + 2], dx
	FUNC_OUT

align 8

.BARA_L:
	mov	[ebp + SH2.BARA], dx
	FUNC_OUT

align 8

.BAMRA_H:
	mov	[ebp + SH2.BAMRA + 2], dx
	FUNC_OUT

align 8

.BAMRA_L:
	mov	[ebp + SH2.BAMRA], dx
	FUNC_OUT

align 16

.BCR1:
	and	dx, 0x1FF7
	mov	[ebp + SH2.BCR1], dx
	FUNC_OUT


align 32

; void FASTCALL SH2_Write_Long_FF(UINT32 adr, UINT32 data)
DECLF SH2_Write_Long_FF, 8
	FUNC_IN
	test	ecx, 0xFFFE00
	jz	near .Bad
	
	bswap	edx
	and	ecx, 0x1FC
	mov	[ebp + SH2.IO_Reg + ecx], edx
	bswap	edx
	jmp	[.Table_Jump + ecx]

align 16

.Table_Jump:
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E00-E0F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E10-E1F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E20-E2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E30-E3F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E40-E4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E50-E5F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E60-E6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E70-E7F
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E80-E8F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; E90-E9F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EA0-EAF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EB0-EBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EC0-ECF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; ED0-EDF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EE0-EEF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; EF0-EFF
	
	; ***********************************************************
	
	dd	.DVSR,	.DVDNT,	.DVCR,	.VCRDIV		; F00-F0F
	dd	.DVDNTH, .DVDNTL, .DVDNTH, .DVDNTL	; F10-F1F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F20-F2F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F30-F3F
	
	dd	.BARA,	.BAMRA,	.Bad,	.Bad		; F40-F4F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F50-F5F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F60-F6F
	dd	.Bad,	.Bad,	.Bad,	.Bad		; F70-F7F
	
	dd	.SAR0,	.DAR0,	.TCR0,	.CHCR0		; F80-F8F
	dd	.SAR1,	.DAR1,	.TCR1,	.CHCR1		; F90-F9F
	dd	.VCRDMA0, .VCRDMA1, .Bad, .Bad		; FA0-FAF
	dd	.DMAOR,	.Bad,	.Bad,	.Bad		; FB0-FBF
	
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FC0-FCF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FD0-FDF
	dd	.BCR1,	.Bad,	.Bad,	.Bad		; FE0-FEF
	dd	.Bad,	.Bad,	.Bad,	.Bad		; FF0-FFF

align 16

.Bad:
	FUNC_OUT

align 16

.DVSR:
	mov	[ebp + SH2.DVSR], edx
	FUNC_OUT

align 16

.DVDNT:
	mov	[ebp + SH2.DVDNTL], edx
	mov	eax, edx
	mov	ecx, [ebp + SH2.DVSR]
	cdq
	test	ecx, ecx
	jz	short .DVDNT_Zero
	
	idiv	ecx
	mov	[ebp + SH2.DVDNTH], edx
	mov	[ebp + SH2.DVDNTL], eax
	FUNC_OUT

align 16

.DVDNT_Zero:
	mov	eax, [ebp + SH2.DVCR]
	mov	[ebp + SH2.DVDNTH], edx
	or	eax, byte 1
	mov	[ebp + SH2.DVCR], eax
	FUNC_OUT

align 8

.DVCR:
	mov	[ebp + SH2.DVCR], edx
	FUNC_OUT

align 8

.VCRDIV:
	mov	[ebp + SH2.VCRDIV], edx
	FUNC_OUT

align 8

.DVDNTH:
	mov	[ebp + SH2.DVDNTH], edx
	FUNC_OUT

align 16

.DVDNTL:
	mov	[ebp + SH2.DVDNTL], edx
	mov	eax, edx
	mov	ecx, [ebp + SH2.DVSR]
	mov	edx, [ebp + SH2.DVDNTH]
	test	ecx, ecx
	jz	short .DVDNTL_Zero
	
; this part check for overflow, need to be optimised !
; ----------------------------------------------------
	
	push	edx
	push	ecx
	
	shld	edx, eax, 1
	test	ecx, ecx
	jns	short .ecx_ok
	
	neg ecx

.ecx_ok:
	test	edx, edx
	jns	short .edx_ok
	
	cmp	edx, -1
	jz	short .div_ok
	neg	edx

.edx_ok:
	cmp	edx, ecx
	jae	short .DVDNTL_Overflow

.div_ok:
	pop	ecx
	pop	edx

; end overflow check
; ------------------

	idiv	ecx
	mov	[ebp + SH2.DVDNTH], edx
	mov	[ebp + SH2.DVDNTL], eax
	FUNC_OUT

align 16

.DVDNTL_Zero:
	mov	eax, [ebp + SH2.DVCR]
	or	eax, byte 1
	mov	[ebp + SH2.DVCR], eax
	FUNC_OUT

align 16

.DVDNTL_Overflow:
	pop	ecx
	pop	edx
	xor	edx, ecx
	mov	eax, 0x80000000
	js	short .DVDNTL_Overflow_Signed
	
	mov	eax, 0x7FFFFFFF

.DVDNTL_Overflow_Signed:
	mov	edx, [ebp + SH2.DVCR]
	mov	[ebp + SH2.DVDNTL], eax
	or	edx, byte 1
	mov	dword [ebp + SH2.DVDNTH], 0
	mov	[ebp + SH2.DVCR], edx
	FUNC_OUT

align 8

.SAR0:
	mov	[ebp + SH2.SAR0], edx
	FUNC_OUT

align 8

.DAR0:
	mov	[ebp + SH2.DAR0], edx
	FUNC_OUT

align 8

.TCR0:
	and	edx, 0xFFFFFF
	mov	[ebp + SH2.TCR0], edx
	FUNC_OUT

align 16

.CHCR0:
	test	edx, 2
	jz	short .CHCR0_ok
	
	mov	eax, [ebp + SH2.CHCR0]
	and	edx, byte ~2
	and	eax, byte 2
	or	edx, eax

.CHCR0_ok:
	mov	cl, [ebp + SH2.DMAOR]
	mov	[ebp + SH2.CHCR0], edx
	test	dh, 2
	mov	ch, dl
	jnz	near .DMA0_Internal

.DMA0_External:
	and	cx, 0x0107
	mov	al, dh
	cmp	cx, 0x0101
	jne	short .DREQ0_bad
	
	mov	dl, [ebp + SH2.DREQ0]
	and	eax, byte 0x0C
	test	dl, dl
	mov	ecx, [ebp + SH2.TCR0]
	jz	short .DREQ0_bad
	test	ecx, ecx
	mov	dword [ebp + SH2.TCR0], 0	; prevent recursivity with DMA request during DMA
	jnz	short .DREQ0

.DREQ0_bad:
	FUNC_OUT

align 16

.DMA0_Size_Tab:
	dd 1, 2, 4, 8 + 4

.DMA0_Read_Tab:
	dd SH2.Read_Byte, SH2.Read_Word, SH2.Read_Long, SH2.Read_Long

.DMA0_Write_Tab:
	dd SH2.Write_Byte, SH2.Write_Word, SH2.Write_Long, SH2.Write_Long

align 16

.DREQ0:
	push	ebx
	push	edi
	mov	ebx, [.DMA0_Size_Tab + eax]
	push	esi
	test	ebx, 8
	mov	esi, [ebp + SH2.SAR0]
	jz	short .DREQ0_no_16
	
	lea ecx, [ecx * 4]

.DREQ0_no_16:
	and	ebx, byte 7
	push	ecx
	shr	esi, 24
	mov	ecx, [.DMA0_Read_Tab + eax]
	mov	edi, [ebp + SH2.DAR0]
	lea	ecx, [ecx + esi * 4]
	shr	edi, 24
	mov	ecx, [ebp + ecx]
	lea	edi, [ebp + edi * 4]
	push	ecx
	shr	edx, 10
	mov	eax, [.DMA0_Write_Tab + eax]
	and	edx, byte 0x3C
	mov	eax, [eax + edi]
	mov	esi, [ebp + SH2.SAR0]
	mov	edi, [ebp + SH2.DAR0]
	push	eax
	jmp	[.DREQ0_Tab + edx]

align 16

.DREQ0_Tab:
	dd	.DREQ0_D_S_,	.DREQ0_D_SI,	.DREQ0_D_SD,	.DREQ0_D_S_
	dd	.DREQ0_DIS_,	.DREQ0_DISI,	.DREQ0_DISD,	.DREQ0_DIS_
	dd	.DREQ0_DDS_,	.DREQ0_DDSI,	.DREQ0_DDSD,	.DREQ0_DDS_
	dd	.DREQ0_D_S_,	.DREQ0_D_SI,	.DREQ0_D_SD,	.DREQ0_D_S_

align 16

.DREQ0_D_S_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_D_S_
	jmp	.DREQ0_End

align 16

.DREQ0_D_SI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_D_SI
	jmp	.DREQ0_End

align 16

.DREQ0_D_SD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_D_SD
	jmp	.DREQ0_End

align 16

.DREQ0_DIS_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_DIS_
	jmp	.DREQ0_End

align 16

.DREQ0_DISI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		add	esi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_DISI
	jmp	.DREQ0_End

align 16

.DREQ0_DISD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_DISD
	jmp	.DREQ0_End

align 16

.DREQ0_DDS_:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_DDS_
	jmp	.DREQ0_End

align 16

.DREQ0_DDSI:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_DDSI
	jmp	short .DREQ0_End

align 16

.DREQ0_DDSD:
		sub	edi, ebx
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]			; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]			; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]			; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]			; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ0]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ0_DDSD
	jmp	short .DREQ0_End

align 16

.DREQ0_End:
	mov	edx, [esp + 8]
	mov	al, [ebp + SH2.CHCR0]
	add	esp, byte 3 * 4
	test	edx, edx
	mov	[ebp + SH2.TCR0], edx
	setle	ah
	mov	[ebp + SH2.SAR0], esi
	add	ah, ah
	mov	[ebp + SH2.DAR0], edi
	or	al, ah
	pop	esi
	mov	[ebp + SH2.CHCR0], al
	pop	edi
	pop	ebx

.DREQ0_rend:
	FUNC_OUT

align 32

.DMA0_Internal:
	and	cx, 0x0307
	movzx	eax, dh
	cmp	cx, 0x0101
	jne	short .DREQ0_rend
	
	mov	ecx, [ebp + SH2.TCR0]
	and	eax, byte 0x0C
	test	ecx, ecx
	mov	dword [ebp + SH2.TCR0], 0	; prevent recursivity with DMA request during DMA
	jz	short .DREQ0_rend

.DMA0:
	push	ebx
	push	edi
	mov	ebx, [.DMA0_Size_Tab + eax]
	push	esi
	test	ebx, 8
	mov	esi, [ebp + SH2.SAR0]
	jz	short .DMA0_no_16
	
	lea	ecx, [ecx * 4]

.DMA0_no_16:
	and	ebx, byte 7
	push	ecx
	shr	esi, 24
	mov	ecx, [.DMA0_Read_Tab + eax]
	mov	edi, [ebp + SH2.DAR0]
	lea	ecx, [ecx + esi * 4]
	shr	edi, 24
	mov	ecx, [ebp + ecx]
	lea	edi, [ebp + edi * 4]
	push	ecx
	shr	edx, 10
	mov	eax, [.DMA0_Write_Tab + eax]
	and	edx, byte 0x3C
	mov	eax, [eax + edi]
	mov	esi, [ebp + SH2.SAR0]
	mov	edi, [ebp + SH2.DAR0]
	push	eax
	jmp	[.DMA0_Tab + edx]

align 16

.DMA0_Tab:
	dd	.DMA0_D_S_,	.DMA0_D_SI,	.DMA0_D_SD,	.DMA0_D_S_
	dd	.DMA0_DIS_,	.DMA0_DISI,	.DMA0_DISD,	.DMA0_DIS_
	dd	.DMA0_DDS_,	.DMA0_DDSI,	.DMA0_DDSD,	.DMA0_DDS_
	dd	.DMA0_D_S_,	.DMA0_D_SI,	.DMA0_D_SD,	.DMA0_D_S_

align 16

.DMA0_D_S_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA0_D_S_
	jmp	.DMA0_End

align 16

.DMA0_D_SI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA0_D_SI
	jmp	.DMA0_End

align 16

.DMA0_D_SD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA0_D_SD
	jmp	.DMA0_End

align 16

.DMA0_DIS_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA0_DIS_
	jmp	.DMA0_End

align 16

.DMA0_DISI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		add	esi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA0_DISI
	jmp	.DMA0_End

align 16

.DMA0_DISD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA0_DISD
	jmp	.DMA0_End

align 16

.DMA0_DDS_:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA0_DDS_
	jmp 	.DMA0_End

align 16

.DMA0_DDSI:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA0_DDSI
	jmp	short .DMA0_End

align 16

.DMA0_DDSD:
		sub	edi, ebx
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA0_DDSD
	jmp	short .DMA0_End

align 16

.DMA0_End:
	mov	al, [ebp + SH2.CHCR0]
	xor	edx, edx
	add	esp, byte 3 * 4
	mov	[ebp + SH2.TCR0], edx
	or	al, 0x02
	mov	[ebp + SH2.SAR0], esi
	test	al, 0x04
	jz	short .DMA0_No_Exception
	
	mov	bl, al
	mov	dh, [ebp + SH2.VCRDMA0]
	mov	dl, [ebp + SH2.IPDMA]
	mov	ecx, ebp
	FUNC_CALL_IN
	call	_SH2_Interrupt_Internal
	FUNC_CALL_OUT
	mov	al, bl

.DMA0_No_Exception:
	mov	[ebp + SH2.DAR0], edi
	mov	[ebp + SH2.CHCR0], al
	pop	esi
	pop	edi
	pop	ebx
	FUNC_OUT

align 16

.SAR1:
	mov	[ebp + SH2.SAR1], edx
	FUNC_OUT

align 8

.DAR1:
	mov	[ebp + SH2.DAR1], edx
	FUNC_OUT

align 8

.TCR1:
	and	edx, 0xFFFFFF
	mov	[ebp + SH2.TCR1], edx
	FUNC_OUT

align 16

.CHCR1:
	test	edx, 2
	jz	short .CHCR1_ok
	
	mov	eax, [ebp + SH2.CHCR1]
	and	edx, byte ~2
	and	eax, byte 2
	or	edx, eax

.CHCR1_ok:
	mov	cl, [ebp + SH2.DMAOR]
	mov	[ebp + SH2.CHCR1], edx
	test	dh, 2
	mov	ch, dl
	jnz	near .DMA1_Internal

.DMA1_External:
	and	cx, 0x0107
	movzx	eax, dh
	cmp	cx, 0x0101
	jne	short .DREQ1_bad
	
	mov	dl, [ebp + SH2.DREQ1]
	and	eax, byte 0x0C
	test	dl, dl
	mov	ecx, [ebp + SH2.TCR1]
	jz	short .DREQ1_bad
	test	ecx, ecx
	mov	dword [ebp + SH2.TCR1], 0
	jnz	short .DREQ1

.DREQ1_bad:
	FUNC_OUT

align 16

.DMA1_Size_Tab:
	dd 1, 2, 4, 8 + 4

.DMA1_Read_Tab:
	dd SH2.Read_Byte, SH2.Read_Word, SH2.Read_Long, SH2.Read_Long

.DMA1_Write_Tab:
	dd SH2.Write_Byte, SH2.Write_Word, SH2.Write_Long, SH2.Write_Long

align 16

.DREQ1:
	push	ebx
	push	edi
	mov	ebx, [.DMA1_Size_Tab + eax]
	push	esi
	test	ebx, 8
	mov	esi, [ebp + SH2.SAR1]
	jz	short .DREQ1_no_16
	
	lea	ecx, [ecx * 4]

.DREQ1_no_16:
	and	ebx, byte 7
	push	ecx
	shr	esi, 24
	mov	ecx, [.DMA1_Read_Tab + eax]
	mov	edi, [ebp + SH2.DAR1]
	lea	ecx, [ecx + esi * 4]
	shr	edi, 24
	mov	ecx, [ebp + ecx]
	lea	edi, [ebp + edi * 4]
	push	ecx
	shr	edx, 10
	mov	eax, [.DMA1_Write_Tab + eax]
	and	edx, byte 0x3C
	mov	eax, [eax + edi]
	mov	esi, [ebp + SH2.SAR1]
	mov	edi, [ebp + SH2.DAR1]
	push	eax
	jmp	[.DREQ1_Tab + edx]

align 16

.DREQ1_Tab:
	dd	.DREQ1_D_S_,	.DREQ1_D_SI,	.DREQ1_D_SD,	.DREQ1_D_S_
	dd	.DREQ1_DIS_,	.DREQ1_DISI,	.DREQ1_DISD,	.DREQ1_DIS_
	dd	.DREQ1_DDS_,	.DREQ1_DDSI,	.DREQ1_DDSD,	.DREQ1_DDS_
	dd	.DREQ1_D_S_,	.DREQ1_D_SI,	.DREQ1_D_SD,	.DREQ1_D_S_

align 16

.DREQ1_D_S_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_D_S_
	jmp	.DREQ1_End

align 16

.DREQ1_D_SI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_D_SI
	jmp	.DREQ1_End

align 16

.DREQ1_D_SD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_D_SD
	jmp	.DREQ1_End

align 16

.DREQ1_DIS_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_DIS_
	jmp	.DREQ1_End

align 16

.DREQ1_DISI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		add	esi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_DISI
	jmp	.DREQ1_End

align 16

.DREQ1_DISD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_DISD
	jmp	.DREQ1_End

align 16

.DREQ1_DDS_:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_DDS_
	jmp	.DREQ1_End

align 16

.DREQ1_DDSI:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_DDSI
	jmp	short .DREQ1_End

align 16

.DREQ1_DDSD:
		sub	edi, ebx
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		mov	eax, [esp + 8]
		mov	dl, [ebp + SH2.DREQ1]
		dec	eax
		test	dl, dl
		mov	[esp + 8], eax
		jnz	short .DREQ1_DDSD
	jmp	short .DREQ1_End

align 16

.DREQ1_End:
	mov	edx, [esp + 8]
	mov	al, [ebp + SH2.CHCR1]
	add	esp, byte 3 * 4
	test	edx, edx
	mov	[ebp + SH2.TCR1], edx
	setle	ah
	mov	[ebp + SH2.SAR1], esi
	add	ah, ah
	mov	[ebp + SH2.DAR1], edi
	or	al, ah
	pop	esi
	mov	[ebp + SH2.CHCR1], al
	pop	edi
	pop	ebx

.DREQ1_rend:
	FUNC_OUT


align 32

.DMA1_Internal:
	and	cx, 0x0307
	movzx	eax, dh
	cmp	cx, 0x0101
	jne	short .DREQ1_rend
	
	mov	ecx, [ebp + SH2.TCR1]
	and	eax, byte 0x0C
	test	ecx, ecx
	mov	dword [ebp + SH2.TCR1], 0
	jz	short .DREQ1_rend

.DMA1:
	push	ebx
	push	edi
	mov	ebx, [.DMA1_Size_Tab + eax]
	push	esi
	test	ebx, 8
	mov	esi, [ebp + SH2.SAR1]
	jz	short .DMA1_no_16
	
	lea	ecx, [ecx * 4]

.DMA1_no_16:
	and	ebx, byte 7
	push	ecx
	shr	esi, 24
	mov	ecx, [.DMA1_Read_Tab + eax]
	mov	edi, [ebp + SH2.DAR1]
	lea	ecx, [ecx + esi * 4]
	shr	edi, 24
	mov	ecx, [ebp + ecx]
	lea	edi, [ebp + edi * 4]
	push	ecx
	shr	edx, 10
	mov	eax, [.DMA1_Write_Tab + eax]
	and	edx, byte 0x3C
	mov	eax, [eax + edi]
	mov	esi, [ebp + SH2.SAR1]
	mov	edi, [ebp + SH2.DAR1]
	push	eax
	jmp	[.DMA1_Tab + edx]

align 16

.DMA1_Tab:
	dd	.DMA1_D_S_,	.DMA1_D_SI,	.DMA1_D_SD,	.DMA1_D_S_
	dd	.DMA1_DIS_,	.DMA1_DISI,	.DMA1_DISD,	.DMA1_DIS_
	dd	.DMA1_DDS_,	.DMA1_DDSI,	.DMA1_DDSD,	.DMA1_DDS_
	dd	.DMA1_D_S_,	.DMA1_D_SI,	.DMA1_D_SD,	.DMA1_D_S_

align 16

.DMA1_D_S_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA1_D_S_
	jmp	.DMA1_End

align 16

.DMA1_D_SI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA1_D_SI
	jmp	.DMA1_End

align 16

.DMA1_D_SD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA1_D_SD
	jmp	.DMA1_End

align 16

.DMA1_DIS_:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA1_DIS_
	jmp	.DMA1_End

align 16

.DMA1_DISI:
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		add	esi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA1_DISI
	jmp	.DMA1_End

align 32

.DMA1_DISD:
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	edi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA1_DISD
	jmp	.DMA1_End

align 16

.DMA1_DDS_:
		sub	edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA1_DDS_
	jmp	.DMA1_End

align 16

.DMA1_DDSI:
		sub edi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		add	esi, ebx
		dec	dword [esp + 8]
		jnz	short .DMA1_DDSI
	jmp	short .DMA1_End

align 16

.DMA1_DDSD:
		sub	edi, ebx
		sub	esi, ebx
		%ifdef __GCC
			mov	ecx, [esp + 4]		; Read function pointer
			mov	eax, esi
			push	ebp
			call	ecx
			pop	ebp
			mov	edx, eax
			mov	ecx, [esp + 0]		; Write function pointer
			mov	eax, edi
			push	ebp
			call	ecx
			pop	ebp
		%else
			mov	eax, [esp + 4]		; Read function pointer
			mov	ecx, esi
			call	eax
			mov	edx, eax
			mov	eax, [esp + 0]		; Write function pointer
			mov	ecx, edi
			call	eax
		%endif
		dec	dword [esp + 8]
		jnz	short .DMA1_DDSD
	jmp	short .DMA1_End

align 16

.DMA1_End:
	mov	al, [ebp + SH2.CHCR1]
	xor	edx, edx
	add	esp, byte 3 * 4
	mov	[ebp + SH2.TCR1], edx
	or	al, 0x02
	mov	[ebp + SH2.SAR1], esi
	test	al, 0x04
	jz	short .DMA1_No_Exception
	
	mov	bl, al
	mov	dh, [ebp + SH2.VCRDMA1]
	mov	dl, [ebp + SH2.IPDMA]
	mov	ecx, ebp
	FUNC_CALL_IN
	call	_SH2_Interrupt_Internal
	FUNC_CALL_OUT
	mov	al, bl

.DMA1_No_Exception:
	mov	[ebp + SH2.DAR1], edi
	mov	[ebp + SH2.CHCR1], al
	pop	esi
	pop	edi
	pop	ebx
	FUNC_OUT

align 8

.VCRDMA0:
	mov	[ebp + SH2.VCRDMA0], edx
	FUNC_OUT

align 8

.VCRDMA1:
	mov	[ebp + SH2.VCRDMA1], edx
	FUNC_OUT

align 16

.DMAOR:
	mov	dh, dl
	mov	al, [ebp + SH2.DMAOR]
	and	dl, 0x6
	and	dh, 0x9
	and	al, dl
	or	al, dh
	mov	[ebp + SH2.DMAOR], al
	
	mov	edx, [ebp + SH2.CHCR0]
	call	.CHCR0
	mov	edx, [ebp + SH2.CHCR1]
	call	.CHCR1
	FUNC_OUT

align 8

.BARA:
	mov	[ebp + SH2.BARA], edx
	FUNC_OUT

align 8

.BAMRA:
	mov	[ebp + SH2.BAMRA], edx
	FUNC_OUT

align 8

.BCR1:
	and	edx, 0x1FF7
	mov	[ebp + SH2.BCR1], edx
	FUNC_OUT
