;****************************************
;
; SH2 emulator 1.60 (ASM source part)
; Copyright 2002 Stéphane Dallongeville
;
;****************************************


;********************************************************************
;
; Register Usage Description for SH2a.asm
; ---------------------------------------
;
; EAX: Opcode copy & General use (value read from memory)
; EBX: Bits 15 - 00 = Current opcode (word), register number...
;      Bits 31 - 16 = 0 (almost time)
; ECX: Main address  (VC fastcall convention)
; EDX: Main data     (VC fastcall convention)
; EBP: SH2 context pointer
; ESI: PC (+ base offset)
; EDI: Cycle counter
;
; Instruction is fetched from PC - 4 because a real SH2 execute it
; in the 3th stage of the pipeline.
; Don't forget memory is big indian style...
; High and low byte are inverted for each instruction.
;
;********************************************************************




;**************************
;
; Some usefull ASM macros
;
;**************************


%define ALIGN4		align 4
%define ALIGN32		align 32
%define ALIGN64		align 64
%define ALIGNB4		alignb 4
%define ALIGNB32	alignb 32
%define ALIGNB64	alignb 64


; By default, regcall functions use the MSVC __fastcall convention.
; In case you're using GCC, uncomment the __GCC definition.

;%define __GCC
;%define __GCC2

%macro DECLV 1

	global _%1
	global %1

	_%1:
	%1:

%endmacro

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




;*************************
;
; Constantes definitions 
;
;*************************

;%define SH2_SPEED       OPTIMIST
%define SH2_SPEED       REALIST
;%define SH2_SPEED       PESSIMIST

%define FRT_SUPPORT

%define OPTIMIST        0
%define REALIST         1
%define PESSIMIST       2

%define SH2_NOP_CODE    0x0900

%define SH2_RUNNING     0x01
%define SH2_HALTED      0x02
%define SH2_DISABLE     0x04
%define SH2_FAULTED     0x10



;*******************
;
; Data declaration
;
;*******************

%ifdef __GCC

section .bss		; sthief: coff format doesn't support section alignment specification, must align manually
bits 32

%elifdef __GCC2

section .bss; align=64
bits 32

%else
section .bss align=64
;bits 32

%endif

	%include "sh2_context.inc"
	
	ALIGNB64

	DECLV OP_Table
		resd	0x10000

	ALIGNB64

	DECLV OPDS_Table
		resd	0x10000

	ALIGNB64

	DECLV Set_SR_Table
		resd	0x400

	ALIGNB64

	DECLV M_SH2
		SH2_CONTEXT

	ALIGNB64

	DECLV S_SH2
		SH2_CONTEXT



%ifdef __GCC

section .text		; sthief: coff format doesn't support section alignment specification, must align manually
bits 32

%else

section .text align=64
bits 32

%endif


;*****************************************
;
; Macros definitions for code generation
;
;*****************************************


; F_NM : NM instruction format (xxxx nnnn mmmm xxxx -> mmmm xxxx xxxx nnnn)
; eax = reg src
; ebx = red dst 


%macro F_NM 0
	shr eax, 12
	and ebx, byte 0x0F
%endmacro


; F_NM2 : NM instruction format (xxxx nnnn mmmm xxxx -> mmmm xxxx xxxx nnnn)
; eax = red dst 
; ebx = reg src


%macro F_NM2 0
	shr ebx, 12
	and eax, byte 0x0F
%endmacro


; NI instruction format (xxxx nnnn iiii iiii -> iiii iiii xxxx nnnn)
; eax = sign extended immediat value
; ebx = reg dst

%macro F_NI 0
	and ebx, byte 0x0F
	movsx eax, ah
%endmacro


; ND8 instruction format (xxxx nnnn dddd dddd -> dddd dddd xxxx nnnn)
; eax = displacement
; ebx = reg dst

%macro F_ND8 0
	shr eax, 8
	and ebx, byte 0x0F
%endmacro


; ND4 instruction format (xxxx xxxx nnnn dddd -> nnnn dddd xxxx xxxx
; eax = displacement
; ebx = reg dst

%macro F_ND4 0
	shr ebx, 12
	and eax, 0x0F00
	shr eax, 8
%endmacro


; MD4 instruction format (xxxx xxxx mmmm dddd -> mmmm dddd xxxx xxxx)
; eax = displacement
; ebx = reg dst

%macro F_MD4 0
	shr ebx, 12
	and eax, 0x0F00
	shr eax, 8
%endmacro


; NMD instruction format (xxxx nnnn mmmm dddd -> mmmm dddd xxxx nnnn)
; eax = displacement
; ebx = reg dst
; edx = src

%macro F_NMD 0
	shr eax, 8
	mov edx, ebx
	and ebx, byte 0x0F
	and eax, byte 0x0F
	shr edx, 12
%endmacro



; GET_SR macro
; ============
;
; Return SR value in eax
; EDX modified
;
; IN:
; ebp = SH2 context pointer
; edi = cycles counter
; esi = PC
;
; OUT:
; ax = SR

%macro GET_SR 0

	mov dh, [ebp + SH2.SR_IMask]
	mov dl, [ebp + SH2.SR_S]
	shl dh, 4
	mov al, [ebp + SH2.SR_T]
	add dl, dl
	or al, dh
	mov ah, [ebp + SH2.SR_MQ]
	or al, dl

%endmacro


; RET macro
; =========
;
; End the current instruction to process the next.
;
; IN:
; ebp = SH2 context pointer
; edi = cycles counter
; esi = PC
;
; OUT:
; eax = ebx = opcode

%macro RET 3

%if SH2_SPEED = OPTIMIST
%define ret_cycle %1
%elif SH2_SPEED = REALIST
%define ret_cycle %2
%else
%define ret_cycle %3
%endif

	add esi, byte 2

%if (ret_cycle > 0)
	%if (ret_cycle = 1)
		dec edi
	%else
		sub edi, byte ret_cycle
	%endif
%else
	test edi, edi
%endif

	movzx ebx, word [esi - 4]
	js near SH2_Exec_Quit
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

%endmacro


; GO_DS macro
; ============
;
; Execute an instruction in a delay slot
;
; IN:
; ebp = SH2 context pointer
; ecx = old PC based & updated (no need to add 2)
; edi = cycles counter
; esi = new PC based & updated (no need to add 2, ready for delay slot execution)
;
; OUT:
; nothing

%macro GO_DS 3

%if SH2_SPEED = OPTIMIST
%define ret_cycle %1
%elif SH2_SPEED = REALIST
%define ret_cycle %2
%else
%define ret_cycle %3
%endif

	movzx ebx, word [ecx - 4]
%if (ret_cycle > 0)
	%if (ret_cycle = 1)
		dec edi
	%else
		sub edi, byte ret_cycle
	%endif
%endif
	mov [ebp + SH2.DS_PC], esi
	mov eax, ebx
	mov esi, ecx
	jmp [OPDS_Table + ebx * 4]

%endmacro


; RET_DS macro
; ============
;
; Return from an instruction executed in a delay slot
;
; IN:
; ebp = SH2 context pointer
; edi = cycles counter
; esi = old PC based
;
; OUT:
; nothing

%macro RET_DS 3

%if SH2_SPEED = OPTIMIST
%define ret_cycle %1
%elif SH2_SPEED = REALIST
%define ret_cycle %2
%else
%define ret_cycle %3
%endif

	mov esi, [ebp + SH2.DS_PC]

%if (ret_cycle > 0)
	%if (ret_cycle = 1)
		dec edi
	%else
		sub edi, byte ret_cycle
	%endif
%else
	test edi, edi
%endif

	movzx ebx, word [esi - 4]		; movzx prevent PPro 32 bits -> 16/8 bits penality
	js near SH2_Exec_Quit
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

%endmacro


; READ_BYTE macro
; ===============
;
; Read a byte from memory.
; FASTCALL convention.
; ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; eax = address
;
; OUT:
; eax = value

%macro READ_BYTE 0

%ifdef __GCC
	mov ecx, eax
	mov [ebp + SH2.Cycle_IO], edi
	shr ecx, 24
	push ebp
	call [ebp + SH2.Read_Byte + ecx * 4]
	pop ebp
	mov edi, [ebp + SH2.Cycle_IO]
%else
	mov ecx, eax
	shr eax, 24
	mov [ebp + SH2.Cycle_IO], edi
	call [ebp + SH2.Read_Byte + eax * 4]
	mov edi, [ebp + SH2.Cycle_IO]
%endif

%endmacro


; READ_WORD macro
; ===============
;
; Read a word from memory.
; FASTCALL convention.
; ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; eax = address
;
; OUT:
; eax = value

%macro READ_WORD 0

%ifdef __GCC
	mov ecx, eax
	mov [ebp + SH2.Cycle_IO], edi
	shr ecx, 24
	push ebp
	call [ebp + SH2.Read_Word + ecx * 4]
	pop ebp
	mov edi, [ebp + SH2.Cycle_IO]
%else
	mov ecx, eax
	shr eax, 24
	mov [ebp + SH2.Cycle_IO], edi
	call [ebp + SH2.Read_Word + eax * 4]
	mov edi, [ebp + SH2.Cycle_IO]
%endif

%endmacro


; READ_LONG macro
; ===============
;
; Read a long from memory.
; FASTCALL convention.
; ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; eax = address
;
; OUT:
; eax = value

%macro READ_LONG 0

%ifdef __GCC
	mov ecx, eax
	mov [ebp + SH2.Cycle_IO], edi
	shr ecx, 24
	push ebp
	call [ebp + SH2.Read_Long + ecx * 4]
	pop ebp
	mov edi, [ebp + SH2.Cycle_IO]
%else
	mov ecx, eax
	shr eax, 24
	mov [ebp + SH2.Cycle_IO], edi
	call [ebp + SH2.Read_Long + eax * 4]
	mov edi, [ebp + SH2.Cycle_IO]
%endif

%endmacro


; WRITE_BYTE macro
; ================
;
; Write a byte in memory.
; FASTCALL convention.
; ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; eax = address
; edx = data
;
; OUT:
; nothing

%macro WRITE_BYTE 0

%ifdef __GCC
	mov ecx, eax
	mov [ebp + SH2.Cycle_IO], edi
	shr ecx, 24
	push ebp
	call [ebp + SH2.Write_Byte + ecx * 4]
	pop ebp
	mov edi, [ebp + SH2.Cycle_IO]
%else
	mov ecx, eax
	shr eax, 24
	mov [ebp + SH2.Cycle_IO], edi
	call [ebp + SH2.Write_Byte + eax * 4]
	mov edi, [ebp + SH2.Cycle_IO]
%endif

%endmacro


; WRITE_WORD macro
; ================
;
; Write a word memory.
; FASTCALL convention.
; ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; eax = address
; edx = data
;
; OUT:
; nothing

%macro WRITE_WORD 0

%ifdef __GCC
	mov ecx, eax
	mov [ebp + SH2.Cycle_IO], edi
	shr ecx, 24
	push ebp
	call [ebp + SH2.Write_Word + ecx * 4]
	pop ebp
	mov edi, [ebp + SH2.Cycle_IO]
%else
	mov ecx, eax
	shr eax, 24
	mov [ebp + SH2.Cycle_IO], edi
	call [ebp + SH2.Write_Word + eax * 4]
	mov edi, [ebp + SH2.Cycle_IO]
%endif

%endmacro


; WRITE_LONG macro
; ================
;
; Write a long in memory.
; FASTCALL convention.
; ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; eax = address
; edx = data
;
; OUT:
; nothing

%macro WRITE_LONG 0

%ifdef __GCC
	mov ecx, eax
	mov [ebp + SH2.Cycle_IO], edi
	shr ecx, 24
	push ebp
	call [ebp + SH2.Write_Long + ecx * 4]
	pop ebp
	mov edi, [ebp + SH2.Cycle_IO]
%else
	mov ecx, eax
	shr eax, 24
	mov [ebp + SH2.Cycle_IO], edi
	call [ebp + SH2.Write_Long + eax * 4]
	mov edi, [ebp + SH2.Cycle_IO]
%endif

%endmacro


; REBASE_PC macro
; ===============
;
; Perform PC base calculation.
; EDX modified
;
; IN:
; ebp = SH2 context pointer
; esi = PC unbased
;
; OUT:
; esi = PC based if (%0 = 0)
; edx = PC base if (%0 = 1)
; eax = 0 if no error

%macro REBASE_PC 0-1
	xor eax, eax

%%Loop
	cmp esi, [ebp + SH2.Fetch_Region + eax + 0]
	jb short %%Next
	cmp esi, [ebp + SH2.Fetch_Region + eax + 4]
	jbe short %%Base

%%Next
	mov edx, [ebp + SH2.Fetch_Region + eax + 8]
	add eax, byte 12
	cmp edx, byte -1
	jne short %%Loop

%%Error
%if %0 > 0
	mov al, [ebp + SH2.Status]
	mov [ebp + SH2.Fetch_Start], edx
	xor edi, edi
	or al, SH2_FAULTED
	mov [ebp + SH2.Base_PC], edi
	mov [ebp + SH2.Fetch_End], edi
	mov [ebp + SH2.Status], al
	jmp short %%End
%else
	mov al, [ebp + SH2.Status]
	mov [ebp + SH2.Fetch_Start], edx
	xor edi, edi
	or al, SH2_FAULTED
	mov [ebp + SH2.Base_PC], edi
	mov [ebp + SH2.Fetch_End], edi
	mov [ebp + SH2.Status], al
	mov edi, edx
	jmp SH2_Exec_Really_Quit
%endif

%%Base
%if %0 > 0
	mov edx, [ebp + SH2.Fetch_Region + eax + 8]
	add esi, edx
	mov [ebp + SH2.Base_PC], edx
	mov edx, [ebp + SH2.Fetch_Region + eax + 0]
	mov [ebp + SH2.Fetch_Start], edx
	mov edx, [ebp + SH2.Fetch_Region + eax + 4]
	mov [ebp + SH2.Fetch_End], edx
	xor eax, eax
%else
	mov edx, [ebp + SH2.Fetch_Region + eax + 0]
	mov [ebp + SH2.Fetch_Start], edx
	mov edx, [ebp + SH2.Fetch_Region + eax + 4]
	mov [ebp + SH2.Fetch_End], edx
	mov edx, [ebp + SH2.Fetch_Region + eax + 8]
	mov [ebp + SH2.Base_PC], edx
%endif

%%End

%endmacro


; FAST_REBASE_PC macro
; ====================
;
; Perform a fast PC recalculation for esi.
; We only do it during execution and for long jump instruction.
; EDX modified
;
; IN:
; ebp = SH2 context pointer
; edi = cycle counter
; esi = unbased PC
;
; OUT:
; esi = based PC
; eax = 0 if no error

%macro FAST_REBASE_PC 0

	cmp esi, [ebp + SH2.Fetch_Start]
	jb short %%Rebase
	cmp esi,[ebp + SH2.Fetch_End]
	mov edx, [ebp + SH2.Base_PC]
	jbe near %%No_Rebase

%%Rebase
	REBASE_PC
	
%%No_Rebase
	add esi, edx

%%End

%endmacro



; CHECK_INT macro
; ===============
;
; Check if some interrupt are pending and process them.
; Called only when SH2 is running...
; EAX, EBX, ECX and EDX modified
;
; IN:
; ebp = SH2 context pointer
; edi = cycle counter
;
; OUT:
; interrupt processed if needed
; 31-16 EBX bits always = 0

%macro CHECK_INT 0

	mov al, [ebp + SH2.SR_IMask]
	mov dl, [ebp + SH2.INT_Prio]
	mov bl, [ebp + SH2.INT_Vect]
	cmp dl, al
	mov bh, dl
	jbe short %%End

	and edx, byte 0x1F
	mov ecx, [ebp + SH2.R + 15 * 4]
	mov byte [ebp + SH2.INT_QUEUE + edx], 0
	dec edx

%%Loop
	mov al, [ebp + SH2.INT_QUEUE + edx]		; INT_QUEUE[x] = Vector X
	test al, al
	jnz short %%New_Line
	dec edx
	jns short %%Loop

	inc dl

%%New_Line
	mov [ebp + SH2.INT_Vect], al
	mov [ebp + SH2.INT_Prio], dl
	call Do_Exception

%%End

%endmacro




; ***********************
;
; Real code start here
;
; ***********************



; Specials functions used only by others ASM functions
; ====================================================

; Do_Exception
; ------------
;
; process the exception
; called only when SH2 is running (CHECK_INT macro)
; This function permit to reduce the SH2_Exec code size.
;
; IN:
; bl = interrupt vector
; bh = interrupt level
; ecx = R15 value
; edi = cycle counter
; esi = old PC based
; ebp = context pointer
;
; OUT:
; esi = new PC based


Do_Exception:

	GET_SR
	cmp bh, 0xF
	jbe short .ok
	
	mov bh, 0xF

.ok
	mov [ebp + SH2.SR_IMask], bh
	mov edx, eax
	sub ecx, byte 8
	and edx, 0x3F3
	lea eax, [ecx + 4]
	mov [ebp + SH2.R + 15 * 4], ecx
	WRITE_LONG
	lea edx, [esi - 4]
	mov eax, [ebp + SH2.R + 15 * 4]
	xor bh, bh
	sub edx, [ebp + SH2.Base_PC]
	WRITE_LONG
	mov dl, [ebp + SH2.Status]
	mov eax, [ebp + SH2.VBR]
	and dl, ~SH2_HALTED
	lea eax, [eax + ebx * 4]
	mov [ebp + SH2.Status], dl
	READ_LONG
	lea esi, [eax + 4]
	FAST_REBASE_PC
	sub edi, byte 12
	ret



; SH2 INSTRUCTIONS
; ================


ALIGN32

DECLF SH2I_ADD

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	add eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_ADDI

	F_NI
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_ADDC

	F_NM
	mov cl, [ebp + SH2.SR_T]
	mov edx, [ebp + SH2.R + ebx * 4]
	shr cl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	adc eax, edx
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_ADDV

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	add eax, edx
	seto [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_AND

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	and eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_ANDI

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	and eax, edx
	mov [ebp + SH2.R + 0], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_ANDM

	mov edx, [ebp + SH2.GBR]
	mov eax, [ebp + SH2.R + 0]
	shr ebx, 8
	add eax, edx
	push eax
	READ_BYTE
	mov edx, eax
	pop eax
	and edx, ebx
	WRITE_BYTE
	RET 3, 5, 7


ALIGN32

DECLF SH2I_BF

	test byte [ebp + SH2.SR_T], 1
	movsx eax, ah
	jnz short .true

	lea esi, [esi + eax * 2 + 4]

%if SH2_SPEED = OPTIMIST
	sub edi, byte 3
%elif SH2_SPEED = REALIST
	sub edi, byte 4
%else
	sub edi, byte 6
%endif

	movzx ebx, word [esi - 4]
	js near SH2_Exec_Quit
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

ALIGN4

.true
	RET 1, 1, 2


ALIGN32

DECLF SH2I_BFfast

	test byte [ebp + SH2.SR_T], 1
	jnz short .true

	movzx ecx, word [esi - 4 - 2]
	mov ebx, ecx
	and ecx, 0xFFF0
	and ebx, byte 0x0F
	cmp ecx, 0x1040
	jne short .norm

	mov eax, [ebp + SH2.R + ebx * 4]
	xor edx, edx
	mov byte [ebp + SH2.SR_T], 1

%if SH2_SPEED = OPTIMIST
	lea eax, [eax * 4 - 2]
%elif SH2_SPEED = REALIST
	lea eax, [eax * 4 + 0]
%else
	lea eax, [eax * 4 + 2]
%endif

	add esi, byte 2
	sub edi, eax
	mov [ebp + SH2.R + ebx * 4], edx
	js near SH2_Exec_Quit

	movzx ebx, word [esi - 4]
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

ALIGN32

.true
	RET 1, 1, 2

ALIGN32

.norm
	sub esi, 2

%if SH2_SPEED = OPTIMIST
	sub edi, byte 3
%elif SH2_SPEED = REALIST
	sub edi, byte 4
%else
	sub edi, byte 6
%endif

	movzx ebx, word [esi - 4]
	js near SH2_Exec_Quit
	mov eax, ebx
	jmp [OP_Table + ebx * 4]


ALIGN32

DECLF SH2I_BFS

	test byte [ebp + SH2.SR_T], 1
	movsx eax, ah
	jnz short .true

	mov ecx, esi
	lea esi, [esi + eax * 2 + 4]
	add ecx, byte 2
	GO_DS 2, 3, 5

ALIGN4

.true
	RET 1, 1, 2


ALIGN32

DECLF SH2I_BFSfast

	test byte [ebp + SH2.SR_T], 1
	movsx edx, ah
	jnz short .true

	movzx eax, word [esi - 4 + 2]
	movzx ecx, word [esi - 4 - 2]
	shl eax, 16
	mov ebx, ecx
	or eax, ecx
	and ebx, byte 0xF
	and eax, byte ~0xF
	mov ecx, esi
	cmp eax, (SH2_NOP_CODE << 16) | 0x1040
	je short .special

	add ecx, byte 2
	lea esi, [esi + edx * 2 + 4]
	GO_DS 2, 3, 5

ALIGN32

.true
	RET 1, 1, 2

ALIGN32

.special
	mov eax, [ebp + SH2.R + ebx * 4]
	xor edx, edx
	mov byte [ebp + SH2.SR_T], 1

%if SH2_SPEED = OPTIMIST
	lea eax, [eax * 4 + eax - 2]
%elif SH2_SPEED = REALIST
	lea eax, [eax * 4 + eax + 0]
%else
	lea eax, [eax * 4 + eax + 2]
%endif

	add esi, byte 2
	sub edi, eax
	mov [ebp + SH2.R + ebx * 4], edx
	js near SH2_Exec_Quit

	movzx ebx, word [esi - 4]
	mov eax, ebx
	jmp [OP_Table + ebx * 4]


ALIGN32

DECLF SH2I_BRA

	; eax = dddd dddd xxxx dddd
	
	ror ax, 4
	lea ecx, [esi + 2]
	movsx eax, ax
	sar eax, 4
	lea esi, [esi + eax * 2 + 4]
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_BRAfast1

	; eax = dddd dddd xxxx dddd
	
	ror ax, 4
	movzx edx, word [esi - 4 + 2]
	movsx eax, ax
	lea ecx, [esi + 2]
	sar eax, 4
	cmp edx, SH2_NOP_CODE
	lea esi, [esi + eax * 2 + 4]
	mov eax, [ebp + SH2.Status]
	jne short .not_waiting

	or eax, SH2_HALTED
	or edi, byte -1
	mov [ebp + SH2.Status], eax
	jmp SH2_Exec_Really_Quit

ALIGN4

.not_waiting
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_BRAfast2

	; eax = dddd dddd xxxx dddd
	
	ror ax, 4
	movzx edx, word [esi - 4 + 2]
	movzx ecx, word [esi - 4 - 2]
	shl edx, 16
	movsx eax, ax
	or edx, ecx
	sar eax, 4
	lea ecx, [esi + 2]
	lea esi, [esi + eax * 2 + 4]
	cmp edx, (SH2_NOP_CODE << 16) | SH2_NOP_CODE
	mov eax, [ebp + SH2.Status]
	jne short .not_waiting

	or eax, SH2_HALTED
	or edi, byte -1
	mov [ebp + SH2.Status], eax
	jmp SH2_Exec_Really_Quit

ALIGN4

.not_waiting
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_BRAF

	lea ecx, [esi + 2]
	mov edx, [ebp + SH2.Base_PC]
	and eax, byte 0x0F
	sub esi, edx
	mov eax, [ebp + SH2.R + eax * 4]
	lea esi, [esi + eax + 4]
	FAST_REBASE_PC
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_BSR

	; eax = dddd dddd xxxx dddd

	mov ecx, [ebp + SH2.Base_PC]
	mov edx, esi
	ror ax, 4
	sub edx, ecx
	movsx eax, ax
	lea ecx, [esi + 2]
	sar eax, 4
	mov [ebp + SH2.PR], edx
	lea esi, [esi + eax * 2 + 4]
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_BSRF

	lea ecx, [esi + 2]
	mov edx, [ebp + SH2.Base_PC]
	and eax, byte 0x0F
	sub esi, edx
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.PR], esi
	lea esi, [esi + eax + 4]
	FAST_REBASE_PC
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_BT

	test byte [ebp + SH2.SR_T], 1
	movsx eax, ah
	jz short .false

	lea esi, [esi + eax * 2 + 4]

%if SH2_SPEED = OPTIMIST
	sub edi, byte 3
%elif SH2_SPEED = REALIST
	sub edi, byte 4
%else
	sub edi, byte 6
%endif

	movzx ebx, word [esi - 4]
	js near SH2_Exec_Quit
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

ALIGN4

.false
	RET 1, 1, 2


ALIGN32

DECLF SH2I_BTS

	test byte [ebp + SH2.SR_T], 1
	movsx eax, ah
	jz short .false

	lea ecx, [esi + 2]
	lea esi, [esi + eax * 2 + 4]
	GO_DS 2, 3, 5

ALIGN4

.false
	RET 1, 1, 2


ALIGN32

DECLF SH2I_CLRMAC

	xor eax, eax
	mov [ebp + SH2.MACH], eax
	mov [ebp + SH2.MACL], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CLRT

	mov byte [ebp + SH2.SR_T], 0
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPEQ

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	sete [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPGE

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	setge [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPGT

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	setg [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPHI

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	seta [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPHS

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	setae [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPPL

	and eax, byte 0x0F
	mov eax, [ebp + SH2.R + eax * 4]
	test eax, eax
	setg [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPPZ

	and eax, byte 0x0F
	mov eax, [ebp + SH2.R + eax * 4]
	test eax, eax
	setge [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_CMPSTR

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	xor eax, edx
	test al, al
	setz cl
	test ah, ah
	setz ch
	shr eax, 16
	and cl, ch
	test al, al
	setz ch
	and cl, ch
	test ah, ah
	setz ch
	and cl, ch
	setz [ebp + SH2.SR_T]
	RET 1, 2, 2


ALIGN32

DECLF SH2I_CMPIM

	movsx eax, ah
	mov edx, [ebp + SH2.R + 0]
	cmp eax, edx
	sete [ebp + SH2.SR_T]
	RET 1, 1, 2


ALIGN32

DECLF SH2I_DIV0S

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	test eax, eax
	sets cl
	shl edx, 1
	mov al, cl
	setc ch
	adc al, al
	xor cl, ch
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], cl
	RET 1, 1, 1


ALIGN32

DECLF SH2I_DIV0U

	xor al, al
	mov [ebp + SH2.SR_T], al
	mov [ebp + SH2.SR_MQ], al
	RET 1, 1, 1


ALIGN32

DECLF SH2I_DIV1

	F_NM
	mov edx, [ebp + SH2.SR_T]
	mov ecx, [ebp + SH2.R + ebx * 4]
	shr edx, 1
	mov eax, [ebp + SH2.R + eax * 4]
	movzx edx, byte [ebp + SH2.SR_MQ]
	adc ecx, ecx
	adc edx, edx						; M | old Q | Q
	jmp [.table + edx * 4]

ALIGN4

.table
	dd .D0M0Q0, .D0M0Q1
	dd .D1M0Q0, .D1M0Q1
	dd .D0M1Q0, .D0M1Q1
	dd .D1M1Q0, .D1M1Q1

ALIGN32

.D0M0Q0
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET 1, 1, 2

ALIGN32

.D0M0Q1
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setnb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET 1, 1, 2

ALIGN32

.D0M1Q0
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setna ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET 1, 1, 2

ALIGN32

.D0M1Q1
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	seta ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET 1, 1, 2

ALIGN32

.D1M0Q0
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	seta ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET 1, 1, 2

ALIGN32

.D1M0Q1
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setna ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET 1, 1, 2

ALIGN32

.D1M1Q0
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setnb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET 1, 1, 2

ALIGN32

.D1M1Q1
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET 1, 1, 2


ALIGN32

DECLF SH2I_DMULS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	imul dword [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.MACL], eax
	mov [ebp + SH2.MACH], edx
	RET 3, 4, 6


ALIGN32

DECLF SH2I_DMULU

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mul dword [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.MACL], eax
	mov [ebp + SH2.MACH], edx
	RET 3, 4, 6


ALIGN32

DECLF SH2I_DT

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	dec eax
	setz [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_EXTSB

	F_NM
	movsx eax, byte [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_EXTSW

	F_NM
	movsx eax, word [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_EXTUB

	F_NM
	movzx eax, byte [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_EXTUW

	F_NM
	movzx eax, word [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_ILLEGAL

	mov al, [ebp + SH2.Status]
	or al, SH2_DISABLE
	or edi, byte -1
	mov [ebp + SH2.Status], al
	jmp SH2_Exec_Really_Quit


ALIGN32

DECLF SH2I_JMP

	and ebx, byte 0x0F
	mov ecx, esi
	mov esi, [ebp + SH2.R + ebx * 4]
	add ecx, byte 2
	add esi, byte 4
	FAST_REBASE_PC
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_JSR

	mov eax, esi
	mov edx, [ebp + SH2.Base_PC]
	and ebx, byte 0x0F
	sub eax, edx
	mov edx, [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.PR], eax
	lea ecx, [esi + 2]
	lea esi, [edx + 4]
	FAST_REBASE_PC
	GO_DS 2, 3, 5


ALIGN32

DECLF SH2I_LDCSR

	and eax, byte 0xF
	mov [ebp + SH2.Cycle_Sup], edi
	mov eax, [ebp + SH2.R + eax * 4]
	and eax, 0x3F3
	xor edi, edi							; Force Interrupt Check
	mov eax, [Set_SR_Table + eax * 4]
	mov [ebp + SH2.SR], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_LDCGBR

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.GBR], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_LDCVBR

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.VBR], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_LDCMSR

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.Cycle_Sup], edi
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	and eax, 0x3F3
	xor edi, edi							; Force Interrupt Check
	mov eax, [Set_SR_Table + eax * 4]
	mov [ebp + SH2.SR], eax
	RET 3, 5, 7


ALIGN32

DECLF SH2I_LDCMGBR

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.GBR], eax
	RET 3, 4, 6


ALIGN32

DECLF SH2I_LDCMVBR

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.VBR], eax
	RET 3, 4, 6


ALIGN32

DECLF SH2I_LDSMACH

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.MACH], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_LDSMACL

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.MACL], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_LDSPR

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.PR], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_LDSMMACH

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.MACH], eax
	RET 1, 3, 5


ALIGN32

DECLF SH2I_LDSMMACL

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.MACL], eax
	RET 1, 3, 5


ALIGN32

DECLF SH2I_LDSMPR

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.PR], eax
	RET 1, 3, 5


ALIGN32

DECLF SH2I_MACL

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	add edx, byte 4
	mov [ebp + SH2.R + eax * 4], edx
	lea eax, [edx - 4]
	READ_LONG
	push eax
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, edx
	add edx, byte 4
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	pop edx
	mov cl, [ebp + SH2.SR_S]
	imul edx
	test cl, cl
	jnz short .sat48

	add [ebp + SH2.MACL], eax
	adc [ebp + SH2.MACH], edx
	RET 3, 5, 8

ALIGN4

.sat48
	mov ecx, [ebp + SH2.MACH]
	add [ebp + SH2.MACL], eax
	adc ecx, edx
	cmp ecx, 0xFFFF8000
	jge short .no_sign_sat

	mov dword [ebp + SH2.MACH], 0xFFFF8000
	mov dword [ebp + SH2.MACL], 0x00000000
	RET 3, 5, 8

ALIGN4

.no_sign_sat
	cmp ecx, 0x00007FFF
	jle short .no_sat

	mov dword [ebp + SH2.MACH], 0x00007FFF
	mov dword [ebp + SH2.MACL], 0xFFFFFFFF
	RET 3, 5, 8

ALIGN4

.no_sat
	mov [ebp + SH2.MACH], ecx
	RET 3, 5, 8


ALIGN32

DECLF SH2I_MACW

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	add edx, byte 2
	mov [ebp + SH2.R + eax * 4], edx
	lea eax, [edx - 2]
	READ_WORD
	movsx eax, ax
	mov edx, [ebp + SH2.R + ebx * 4]
	push eax
	add edx, byte 2
	mov [ebp + SH2.R + ebx * 4], edx
	lea eax, [edx - 2]
	READ_WORD
	pop edx
	movsx eax, ax
	mov cl, [ebp + SH2.SR_S]
	imul edx
	test cl, cl
	jnz short .sat32

	add [ebp + SH2.MACL], eax
	adc [ebp + SH2.MACH], edx
	RET 3, 4, 6

ALIGN4

.sat32
	mov ecx, [ebp + SH2.MACH]
	add [ebp + SH2.MACL], eax
	adc ecx, edx
	cmp ecx, 0xFFFFFFFF
	jge short .no_sign_sat

	mov dword [ebp + SH2.MACH], 0xFFFFFFFF
	mov dword [ebp + SH2.MACL], 0x80000000
	RET 3, 4, 6

ALIGN4

.no_sign_sat
	cmp ecx, 0x00000000
	jle short .no_sat

	mov dword [ebp + SH2.MACH], 0x00000000
	mov dword [ebp + SH2.MACL], 0x7FFFFFFF
	RET 3, 4, 6

ALIGN4

.no_sat
	mov [ebp + SH2.MACH], ecx
	RET 3, 4, 6


ALIGN32

DECLF SH2I_MOV

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_MOVBS

	F_NM2
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	WRITE_BYTE
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWS

	F_NM2
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	WRITE_WORD
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLS

	F_NM2
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	WRITE_LONG
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBL

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWL

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLL

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBM

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	mov eax, [ebp + SH2.R + ebx * 4]
	dec eax
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_BYTE
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWM

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 2
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_WORD
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLM

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBP

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	cmp eax, ebx
	lea edx, [edx + 1]
	je short .no_inc

	mov [ebp + SH2.R + eax * 4], edx

.no_inc
	lea eax, [edx - 1]
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWP

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	cmp eax, ebx
	lea edx, [edx + 2]
	je short .no_inc

	mov [ebp + SH2.R + eax * 4], edx

.no_inc
	lea eax, [edx - 2]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLP

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	cmp eax, ebx
	lea edx, [edx + 4]
	je short .no_inc

	mov [ebp + SH2.R + eax * 4], edx

.no_inc
	lea eax, [edx - 4]
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBS0

	F_NM2
	mov ecx, [ebp + SH2.R + 0]
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, ecx
	WRITE_BYTE
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWS0

	F_NM2
	mov ecx, [ebp + SH2.R + 0]
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, ecx
	WRITE_WORD
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLS0

	F_NM2
	mov ecx, [ebp + SH2.R + 0]
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, ecx
	WRITE_LONG
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBL0

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, edx
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWL0

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, edx
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLL0

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, edx
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVI

	F_NI
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_MOVWI 

	F_ND8
	mov dh, [esi + eax * 2 + 0]
	mov dl, [esi + eax * 2 + 1]
	movsx eax, dx
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLI 

	shr eax, 8
	mov edx, esi
	and ebx, byte 0x0F
	and edx, byte ~3
	mov eax, [edx + eax * 4]
	bswap eax
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVBLG

	shr eax, 8
	mov edx, [ebp + SH2.GBR]
	add eax, edx
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + 0], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWLG

	shr eax, 8
	mov edx, [ebp + SH2.GBR]
	lea eax, [edx + eax * 2]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + 0], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLLG

	shr eax, 8
	mov edx, [ebp + SH2.GBR]
	lea eax, [edx + eax * 4]
	READ_LONG
	mov [ebp + SH2.R + 0], eax
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBSG

	shr eax, 8
	mov ecx, [ebp + SH2.GBR]
	mov edx, [ebp + SH2.R + 0]
	add eax, ecx
	WRITE_BYTE
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWSG

	shr eax, 8
	mov ecx, [ebp + SH2.GBR]
	mov edx, [ebp + SH2.R + 0]
	lea eax, [ecx + eax * 2]
	WRITE_WORD
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLSG

	shr eax, 8
	mov ecx, [ebp + SH2.GBR]
	mov edx, [ebp + SH2.R + 0]
	lea eax, [ecx + eax * 4]
	WRITE_LONG
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBS4

	F_ND4
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, ecx
	WRITE_BYTE
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWS4

	F_ND4
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, [ebp + SH2.R + 0]
	lea eax, [ecx + eax * 2]
	WRITE_WORD
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLS4

	F_NMD
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, [ebp + SH2.R + edx * 4]
	lea eax, [ecx + eax * 4]
	WRITE_LONG
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVBL4

	F_MD4
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, edx
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + 0], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVWL4

	F_MD4
	mov edx, [ebp + SH2.R + ebx * 4]
	lea eax, [edx + eax * 2]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + 0], eax
	RET 1, 2, 3


ALIGN32

DECLF SH2I_MOVLL4

	F_NMD
	mov edx, [ebp + SH2.R + edx * 4]
	lea eax, [edx + eax * 4]
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 2, 4


ALIGN32

DECLF SH2I_MOVA

	mov edx, esi
	mov ecx, [ebp + SH2.Base_PC]
	shr eax, 8
	sub edx, ecx
	and edx, byte ~3
	lea eax, [edx + eax * 4]
	mov [ebp + SH2.R + 0], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_MOVT

	movzx eax, byte [ebp + SH2.SR_T]
	and ebx, byte 0x0F
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_MULL

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	imul eax, dword [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.MACL], eax
	RET 3, 4, 6


ALIGN32

DECLF SH2I_MULS

	F_NM
	movsx edx, word [ebp + SH2.R + ebx * 4]
	movsx eax, word [ebp + SH2.R + eax * 4]
	imul eax, edx
	mov [ebp + SH2.MACL], eax
	RET 2, 3, 5


ALIGN32

DECLF SH2I_MULU

	F_NM
	movzx edx, word [ebp + SH2.R + ebx * 4]
	movzx eax, word [ebp + SH2.R + eax * 4]
	imul eax, edx
	mov [ebp + SH2.MACL], eax
	RET 2, 3, 5


ALIGN32

DECLF SH2I_NEG

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	neg eax
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_NEGC

	F_NM
	mov cl, [ebp + SH2.SR_T]
	xor edx, edx
	shr cl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	sbb edx, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], edx
	RET 1, 1, 2


ALIGN32

DECLF SH2I_NOP

	RET 1, 1, 1


ALIGN32

DECLF SH2I_NOT

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	not eax
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_OR

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	or eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_ORI

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	or eax, edx
	mov [ebp + SH2.R + 0], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_ORM

	mov edx, [ebp + SH2.GBR]
	mov eax, [ebp + SH2.R + 0]
	shr ebx, 8
	add eax, edx
	push eax
	READ_BYTE
	mov edx, eax
	pop eax
	or edx, ebx
	WRITE_BYTE
	RET 3, 5, 7


ALIGN32

DECLF SH2I_ROTCL 

	mov cl, [ebp + SH2.SR_T]
	and ebx, byte 0x0F
	shr cl, 1
	mov eax, [ebp + SH2.R + ebx * 4]
	rcl eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_ROTCR 

	mov cl, [ebp + SH2.SR_T]
	and ebx, byte 0x0F
	shr cl, 1
	mov eax, [ebp + SH2.R + ebx * 4]
	rcr eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_ROTL

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	rol eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_ROTR

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	ror eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_RTE

	mov eax, [ebp + SH2.R + 15 * 4]
	lea ebx, [esi + 2]
	lea edx, [eax + 8]
	lea esi, [eax + 4]
	mov [ebp + SH2.R + 15 * 4], edx
	READ_LONG
	xchg esi, eax
	READ_LONG
	and eax, 0x3FF
	mov [ebp + SH2.Cycle_Sup], edi
	mov eax, [Set_SR_Table + eax * 4]
	mov ecx, ebx
	xor edi, edi							; Force Interrupt Check
	mov [ebp + SH2.SR], eax
	add esi, byte 4
	FAST_REBASE_PC
	GO_DS 4, 6, 8


ALIGN32

DECLF SH2I_RTS

	mov eax, [ebp + SH2.PR]
	lea ecx, [esi + 2]
	lea esi, [eax + 4]
	FAST_REBASE_PC
	GO_DS 2, 3, 4


ALIGN32

DECLF SH2I_SETT

	mov byte [ebp + SH2.SR_T], 1
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHAL

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	add eax, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHAR

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	sar eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLL

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	add eax, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLL2 

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shl eax, 2
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLL8 

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shl eax, 8
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLL16

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shl eax, 16
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLR

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLR2 

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 2
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLR8 

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 8
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SHLR16

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 16
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SLEEP 

	mov al, [ebp + SH2.Status]
	or edi, byte -1
	or al, SH2_HALTED
	add esi, byte 2
	mov [ebp + SH2.Status], al
	jmp SH2_Exec_Really_Quit

;	mov eax, [ebp + SH2.R + 15 * 4]
;	sub eax, byte 8
;	mov [ebp + SH2.R + 15 * 4], eax
;	add esi, byte 4
;	RET 3, 3, 3


ALIGN32

DECLF SH2I_STCSR

	xor eax, eax
	and ebx, byte 0xF
	GET_SR
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_STCGBR

	mov eax, [ebp + SH2.GBR]
	and ebx, byte 0xF
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_STCVBR

	mov eax, [ebp + SH2.VBR]
	and ebx, byte 0xF
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_STCMSR

	and ebx, byte 0xF
	GET_SR
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, eax
	lea eax, [ecx - 4]
	and edx, 0x3F3
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 2, 3, 4


ALIGN32

DECLF SH2I_STCMGBR

	and ebx, byte 0xF
	mov edx, [ebp + SH2.GBR]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 2, 3, 4


ALIGN32

DECLF SH2I_STCMVBR

	and ebx, byte 0xF
	mov edx, [ebp + SH2.VBR]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 2, 3, 4


ALIGN32

DECLF SH2I_STSMACH

	and ebx, byte 0xF
	mov eax, [ebp + SH2.MACH]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_STSMACL

	and ebx, byte 0xF
	mov eax, [ebp + SH2.MACL]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_STSPR 

	and ebx, byte 0xF
	mov eax, [ebp + SH2.PR]
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_STSMMACH

	and ebx, byte 0xF
	mov edx, [ebp + SH2.MACH]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 2, 3, 4


ALIGN32

DECLF SH2I_STSMMACL

	and ebx, byte 0xF
	mov edx, [ebp + SH2.MACL]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 2, 3, 4


ALIGN32

DECLF SH2I_STSMPR

	and ebx, byte 0xF
	mov edx, [ebp + SH2.PR]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET 2, 3, 4


ALIGN32

DECLF SH2I_SUB

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	sub edx, eax
	mov [ebp + SH2.R + ebx * 4], edx
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SUBC

	F_NM
	mov cl, [ebp + SH2.SR_T]
	mov edx, [ebp + SH2.R + ebx * 4]
	shr cl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	sbb edx, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], edx
	RET 1, 1, 2


ALIGN32

DECLF SH2I_SUBV

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	sub edx, eax
	seto [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], edx
	RET 1, 1, 2


ALIGN32

DECLF SH2I_SWAPB

	F_NM
	mov dx, [ebp + SH2.R + eax * 4 + 0]
	mov eax, [ebp + SH2.R + eax * 4 + 0]
	rol dx, 8
	mov [ebp + SH2.R + ebx * 4 + 0], eax
	mov [ebp + SH2.R + ebx * 4 + 0], dx
	RET 1, 1, 1


ALIGN32

DECLF SH2I_SWAPW

	F_NM
	mov eax, [ebp + SH2.R + eax * 4 + 0]
	rol eax, 16
	mov [ebp + SH2.R + ebx * 4 + 0], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_TAS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	READ_BYTE
	test al, al
	mov dl, al
	setz [ebp + SH2.SR_T]
	or dl, 0x80							; work only if BUS can be locked !
	mov eax, [ebp + SH2.R + ebx * 4]
	WRITE_BYTE
	RET 4, 6, 8


ALIGN32

DECLF SH2I_TRAPA

	shr ebx, 8
	mov ecx, [ebp + SH2.R + 15 * 4]
	GET_SR
	mov edx, eax
	sub ecx, byte 8
	and edx, 0x3F3
	lea eax, [ecx + 4]
	mov [ebp + SH2.R + 15 * 4], ecx
	WRITE_LONG
	lea edx, [esi - 4]
	mov ecx, [ebp + SH2.VBR]
	mov eax, [ebp + SH2.R + 15 * 4]
	lea ebx, [ecx + ebx * 4]
	sub edx, [ebp + SH2.Base_PC]
	WRITE_LONG
	mov eax, ebx
	READ_LONG
	lea esi, [eax + 4]
	FAST_REBASE_PC
	RET 8, 12, 16


ALIGN32

DECLF SH2I_TST

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	test eax, edx
	setz [ebp + SH2.SR_T]
	RET 1, 1, 1


ALIGN32

DECLF SH2I_TSTI

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	test eax, edx
	setz [ebp + SH2.SR_T]
	RET 1, 1, 2


ALIGN32

DECLF SH2I_TSTM

	mov eax, [ebp + SH2.R + 0]
	mov edx, [ebp + SH2.GBR]
	shr ebx, 8
	add eax, edx
	READ_BYTE
	test eax, ebx
	setz [ebp + SH2.SR_T]
	RET 3, 4, 6


ALIGN32

DECLF SH2I_XOR

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	xor eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET 1, 1, 1


ALIGN32

DECLF SH2I_XORI

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	xor eax, edx
	mov [ebp + SH2.R + 0], eax
	RET 1, 1, 2


ALIGN32

DECLF SH2I_XORM

	mov eax, [ebp + SH2.R + 0]
	mov edx, [ebp + SH2.GBR]
	shr ebx, 8
	add eax, edx
	push eax
	READ_BYTE
	mov edx, eax
	pop eax
	xor edx, ebx
	WRITE_BYTE
	RET 3, 5, 7


ALIGN32

DECLF SH2I_XTRCT

	F_NM
	mov cx, [ebp + SH2.R + eax * 4 + 0]
	mov dx, [ebp + SH2.R + ebx * 4 + 2]
	mov [ebp + SH2.R + ebx * 4 + 2], cx
	mov [ebp + SH2.R + ebx * 4 + 0], dx
	RET 1, 1, 2


; SH2 DELAY SLOT INSTRUCTIONS
; ===========================


ALIGN32

DECLF SH2I_ADD_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	add eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_ADDI_DS

	F_NI
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_ADDC_DS

	F_NM
	mov cl, [ebp + SH2.SR_T]
	mov edx, [ebp + SH2.R + ebx * 4]
	shr cl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	adc eax, edx
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_ADDV_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	add eax, edx
	seto [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_AND_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	and eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_ANDI_DS

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	and eax, edx
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_ANDM_DS

	mov eax, [ebp + SH2.R + 0]
	mov edx, [ebp + SH2.GBR]
	shr ebx, 8
	add eax, edx
	push eax
	READ_BYTE
	mov edx, eax
	pop eax
	and edx, ebx
	WRITE_BYTE
	RET_DS 3, 5, 7


ALIGN32

DECLF SH2I_CLRMAC_DS

	xor eax, eax
	mov [ebp + SH2.MACH], eax
	mov [ebp + SH2.MACL], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CLRT_DS

	mov byte [ebp + SH2.SR_T], 0
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPEQ_DS

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	sete [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPGE_DS

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	setge [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPGT_DS

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	setg [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPHI_DS

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	seta [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPHS_DS

	F_NM2
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	cmp eax, edx
	setae [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPPL_DS

	and eax, byte 0x0F
	mov eax, [ebp + SH2.R + eax * 4]
	test eax, eax
	setg [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPPZ_DS

	and eax, byte 0x0F
	mov eax, [ebp + SH2.R + eax * 4]
	test eax, eax
	setge [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_CMPSTR_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	xor eax, edx
	test al, al
	setz cl
	test ah, ah
	setz ch
	shr eax, 16
	and cl, ch
	test al, al
	setz ch
	and cl, ch
	test ah, ah
	setz ch
	and cl, ch
	setz [ebp + SH2.SR_T]
	RET_DS 1, 2, 2


ALIGN32

DECLF SH2I_CMPIM_DS

	movsx eax, ah
	mov edx, [ebp + SH2.R + 0]
	cmp eax, edx
	sete [ebp + SH2.SR_T]
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_DIV0S_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	test eax, eax
	sets cl
	shl edx, 1
	mov al, cl
	setc ch
	adc al, al
	xor cl, ch
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], cl
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_DIV0U_DS

	xor al, al
	mov [ebp + SH2.SR_T], al
	mov [ebp + SH2.SR_MQ], al
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_DIV1_DS

	F_NM
	mov dl, [ebp + SH2.SR_T]
	mov ecx, [ebp + SH2.R + ebx * 4]
	shr dl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	movzx edx, byte [ebp + SH2.SR_MQ]
	adc ecx, ecx
	adc edx, edx						; M | old Q | Q
	jmp [.table + edx * 4]

ALIGN4

.table
	dd .D0M0Q0, .D0M0Q1
	dd .D1M0Q0, .D1M0Q1
	dd .D0M1Q0, .D0M1Q1
	dd .D1M1Q0, .D1M1Q1

ALIGN32

.D0M0Q0
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET_DS 1, 1, 2

ALIGN32

.D0M0Q1
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setnb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET_DS 1, 1, 2

ALIGN32

.D0M1Q0
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setna ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET_DS 1, 1, 2

ALIGN32

.D0M1Q1
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	seta ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET_DS 1, 1, 2

ALIGN32

.D1M0Q0
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	seta ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET_DS 1, 1, 2

ALIGN32

.D1M0Q1
	mov edx, ecx
	add ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setna ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	xor ah, 1
	mov [ebp + SH2.SR_MQ], al
	mov [ebp + SH2.SR_T], ah
	RET_DS 1, 1, 2

ALIGN32

.D1M1Q0
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setnb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET_DS 1, 1, 2

ALIGN32

.D1M1Q1
	mov edx, ecx
	sub ecx, eax
	cmp edx, ecx
	mov al, [ebp + SH2.SR_MQ]
	setb ah
	and al, 2
	mov [ebp + SH2.R + ebx * 4], ecx
	or al, ah
	mov [ebp + SH2.SR_T], ah
	mov [ebp + SH2.SR_MQ], al
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_DMULS_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	imul dword [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.MACL], eax
	mov [ebp + SH2.MACH], edx
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_DMULU_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mul dword [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.MACL], eax
	mov [ebp + SH2.MACH], edx
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_DT_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	dec eax
	setz [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_EXTSB_DS

	F_NM
	movsx eax, byte [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_EXTSW_DS

	F_NM
	movsx eax, word [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_EXTUB_DS

	F_NM
	movzx eax, byte [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_EXTUW_DS

	F_NM
	movzx eax, word [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_ILLEGAL_DS

	mov al, [ebp + SH2.Status]
	mov esi, [ebp + SH2.DS_PC]
	or al, SH2_HALTED
	or edi, byte -1
	mov [ebp + SH2.Status], al
	jmp SH2_Exec_Really_Quit


ALIGN32

DECLF SH2I_LDCSR_DS

	and eax, byte 0xF
	mov [ebp + SH2.Cycle_Sup], edi
	mov eax, [ebp + SH2.R + eax * 4]
	and eax, 0x3F3
	xor edi, edi							; Force Interrupt Check
	mov eax, [Set_SR_Table + eax * 4]
	mov [ebp + SH2.SR], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_LDCGBR_DS

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.GBR], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_LDCVBR_DS

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.VBR], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_LDCMSR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.Cycle_Sup], edi
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	and eax, 0x3F3
	xor edi, edi							; Force Interrupt Check
	mov eax, [Set_SR_Table + eax * 4]
	mov [ebp + SH2.SR], eax
	RET_DS 3, 5, 7


ALIGN32

DECLF SH2I_LDCMGBR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.GBR], eax
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_LDCMVBR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.VBR], eax
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_LDSMACH_DS

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.MACH], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_LDSMACL_DS

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.MACL], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_LDSPR_DS

	and eax, byte 0xF
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.PR], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_LDSMMACH_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.MACH], eax
	RET_DS 1, 3, 5


ALIGN32

DECLF SH2I_LDSMMACL_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.MACL], eax
	RET_DS 1, 3, 5


ALIGN32

DECLF SH2I_LDSMPR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	lea edx, [eax + 4]
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	mov [ebp + SH2.PR], eax
	RET_DS 1, 3, 5


ALIGN32

DECLF SH2I_MACL_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	add edx, byte 4
	mov [ebp + SH2.R + eax * 4], edx
	lea eax, [edx - 4]
	READ_LONG
	push eax
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, edx
	add edx, byte 4
	mov [ebp + SH2.R + ebx * 4], edx
	READ_LONG
	pop edx
	mov cl, [ebp + SH2.SR_S]
	imul edx
	test cl, cl
	jnz short .sat48

	add [ebp + SH2.MACL], eax
	adc [ebp + SH2.MACH], edx
	RET_DS 3, 5, 8

ALIGN4

.sat48
	mov ecx, [ebp + SH2.MACH]
	add [ebp + SH2.MACL], eax
	adc ecx, edx
	cmp ecx, 0xFFFF8000
	jge short .no_sign_sat

	mov dword [ebp + SH2.MACH], 0xFFFF8000
	mov dword [ebp + SH2.MACL], 0x00000000
	RET_DS 3, 5, 8

ALIGN4

.no_sign_sat
	cmp ecx, 0x00007FFF
	jle short .no_sat

	mov dword [ebp + SH2.MACH], 0x00007FFF
	mov dword [ebp + SH2.MACL], 0xFFFFFFFF
	RET_DS 3, 5, 8

ALIGN4

.no_sat
	mov [ebp + SH2.MACH], ecx
	RET_DS 3, 5, 8


ALIGN32

DECLF SH2I_MACW_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	add edx, byte 2
	mov [ebp + SH2.R + eax * 4], edx
	lea eax, [edx - 2]
	READ_WORD
	movsx eax, ax
	mov edx, [ebp + SH2.R + ebx * 4]
	push eax
	add edx, byte 2
	mov [ebp + SH2.R + ebx * 4], edx
	lea eax, [edx - 2]
	READ_WORD
	pop edx
	movsx eax, ax
	mov cl, [ebp + SH2.SR_S]
	imul edx
	test cl, cl
	jnz short .sat32

	add [ebp + SH2.MACL], eax
	adc [ebp + SH2.MACH], edx
	RET_DS 3, 4, 6

ALIGN4

.sat32
	mov ecx, [ebp + SH2.MACH]
	add [ebp + SH2.MACL], eax
	adc ecx, edx
	cmp ecx, byte -1
	jge short .no_sign_sat

	mov dword [ebp + SH2.MACH], 0xFFFFFFFF
	mov dword [ebp + SH2.MACL], 0x80000000
	RET_DS 3, 4, 6

ALIGN4

.no_sign_sat
	cmp ecx, byte 0
	jle short .no_sat

	mov dword [ebp + SH2.MACH], 0x00000000
	mov dword [ebp + SH2.MACL], 0x7FFFFFFF
	RET_DS 3, 4, 6

ALIGN4

.no_sat
	mov [ebp + SH2.MACH], ecx
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_MOV_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_MOVBS_DS

	F_NM2
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	WRITE_BYTE
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWS_DS

	F_NM2
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	WRITE_WORD
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLS_DS

	F_NM2
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	WRITE_LONG
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBL_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWL_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLL_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBM_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	mov eax, [ebp + SH2.R + ebx * 4]
	dec eax
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_BYTE
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWM_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 2
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_WORD
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLM_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBP_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	cmp eax, ebx
	lea edx, [edx + 1]
	je short .no_inc

	mov [ebp + SH2.R + eax * 4], edx

.no_inc
	lea eax, [edx - 1]
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWP_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	cmp eax, ebx
	lea edx, [edx + 2]
	je short .no_inc

	mov [ebp + SH2.R + eax * 4], edx

.no_inc
	lea eax, [edx - 2]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLP_DS

	F_NM
	mov edx, [ebp + SH2.R + eax * 4]
	cmp eax, ebx
	lea edx, [edx + 4]
	je short .no_inc

	mov [ebp + SH2.R + eax * 4], edx

.no_inc
	lea eax, [edx - 4]
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBS0_DS

	F_NM2
	mov ecx, [ebp + SH2.R + 0]
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, ecx
	WRITE_BYTE
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWS0_DS

	F_NM2
	mov ecx, [ebp + SH2.R + 0]
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, ecx
	WRITE_WORD
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLS0_DS

	F_NM2
	mov ecx, [ebp + SH2.R + 0]
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, ecx
	WRITE_LONG
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBL0_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, edx
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWL0_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, edx
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLL0_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, edx
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVI_DS

	F_NI
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_MOVWI_DS

	F_ND8
	mov dh, [esi + eax * 2 + 0]
	mov dl, [esi + eax * 2 + 1]
	movsx eax, dx
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLI_DS 

	shr eax, 8
	mov edx, esi
	and ebx, byte 0x0F
	and edx, byte ~3
	mov eax, [edx + eax * 4]
	bswap eax
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVBLG_DS

	shr eax, 8
	mov edx, [ebp + SH2.GBR]
	add eax, edx
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWLG_DS

	shr eax, 8
	mov edx, [ebp + SH2.GBR]
	lea eax, [edx + eax * 2]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLLG_DS

	shr eax, 8
	mov edx, [ebp + SH2.GBR]
	lea eax, [edx + eax * 4]
	READ_LONG
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBSG_DS

	shr eax, 8
	mov ecx, [ebp + SH2.GBR]
	mov edx, [ebp + SH2.R + 0]
	add eax, ecx
	WRITE_BYTE
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWSG_DS

	shr eax, 8
	mov ecx, [ebp + SH2.GBR]
	mov edx, [ebp + SH2.R + 0]
	lea eax, [ecx + eax * 2]
	WRITE_WORD
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLSG_DS

	shr eax, 8
	mov ecx, [ebp + SH2.GBR]
	mov edx, [ebp + SH2.R + 0]
	lea eax, [ecx + eax * 4]
	WRITE_LONG
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBS4_DS

	F_ND4
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, [ebp + SH2.R + 0]
	add eax, ecx
	WRITE_BYTE
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWS4_DS

	F_ND4
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, [ebp + SH2.R + 0]
	lea eax, [ecx + eax * 2]
	WRITE_WORD
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLS4_DS

	F_NMD
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, [ebp + SH2.R + edx * 4]
	lea eax, [ecx + eax * 4]
	WRITE_LONG
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVBL4_DS

	F_MD4
	mov edx, [ebp + SH2.R + ebx * 4]
	add eax, edx
	READ_BYTE
	movsx eax, al
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVWL4_DS

	F_MD4
	mov edx, [ebp + SH2.R + ebx * 4]
	lea eax, [edx + eax * 2]
	READ_WORD
	movsx eax, ax
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 2, 3


ALIGN32

DECLF SH2I_MOVLL4_DS

	F_NMD
	mov edx, [ebp + SH2.R + edx * 4]
	lea eax, [edx + eax * 4]
	READ_LONG
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 2, 4


ALIGN32

DECLF SH2I_MOVA_DS

	mov edx, esi
	mov ecx, [ebp + SH2.Base_PC]
	shr eax, 8
	sub edx, ecx
	and edx, byte ~3
	lea eax, [edx + eax * 4]
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_MOVT_DS

	movzx eax, byte [ebp + SH2.SR_T]
	and ebx, byte 0x0F
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_MULL_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	imul eax, dword [ebp + SH2.R + ebx * 4]
	mov [ebp + SH2.MACL], eax
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_MULS_DS

	F_NM
	movsx edx, word [ebp + SH2.R + ebx * 4]
	movsx eax, word [ebp + SH2.R + eax * 4]
	imul eax, edx
	mov [ebp + SH2.MACL], eax
	RET_DS 2, 3, 5


ALIGN32

DECLF SH2I_MULU_DS

	F_NM
	movzx edx, word [ebp + SH2.R + ebx * 4]
	movzx eax, word [ebp + SH2.R + eax * 4]
	imul eax, edx
	mov [ebp + SH2.MACL], eax
	RET_DS 2, 3, 5


ALIGN32

DECLF SH2I_NEG_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	neg eax
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_NEGC_DS

	F_NM
	mov cl, [ebp + SH2.SR_T]
	xor edx, edx
	shr cl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	sbb edx, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], edx
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_NOP_DS

	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_NOT_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4]
	not eax
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_OR_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	or eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_ORI_DS

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	or eax, edx
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_ORM_DS

	mov eax, [ebp + SH2.R + 0]
	mov edx, [ebp + SH2.GBR]
	shr ebx, 8
	add eax, edx
	push eax
	READ_BYTE
	mov edx, eax
	pop eax
	or edx, ebx
	WRITE_BYTE
	RET_DS 3, 5, 7


ALIGN32

DECLF SH2I_ROTCL_DS

	mov cl, [ebp + SH2.SR_T]
	and ebx, byte 0x0F
	shr cl, 1
	mov eax, [ebp + SH2.R + ebx * 4]
	rcl eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_ROTCR_DS

	mov cl, [ebp + SH2.SR_T]
	and ebx, byte 0x0F
	shr cl, 1
	mov eax, [ebp + SH2.R + ebx * 4]
	rcr eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_ROTL_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	rol eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_ROTR_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	ror eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SETT_DS

	mov byte [ebp + SH2.SR_T], 1
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHAL_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	add eax, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHAR_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	sar eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLL_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	add eax, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLL2_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shl eax, 2
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLL8_DS 

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shl eax, 8
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLL16_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shl eax, 16
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLR_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 1
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLR2_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 2
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLR8_DS 

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 8
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SHLR16_DS

	and ebx, byte 0x0F
	mov eax, [ebp + SH2.R + ebx * 4]
	shr eax, 16
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SLEEP_DS

	mov al, [ebp + SH2.Status]
	or edi, byte -1
	or al, SH2_HALTED
	add esi, byte 2
	mov [ebp + SH2.Status], al
	jmp SH2_Exec_Really_Quit


ALIGN32

DECLF SH2I_STCSR_DS

	xor eax, eax
	and ebx, byte 0xF
	GET_SR
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_STCGBR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.GBR]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_STCVBR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.VBR]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_STCMSR_DS

	and ebx, byte 0xF
	GET_SR
	mov ecx, [ebp + SH2.R + ebx * 4]
	mov edx, eax
	lea eax, [ecx - 4]
	and edx, 0x3F3
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 2, 3, 4


ALIGN32

DECLF SH2I_STCMGBR_DS

	and ebx, byte 0xF
	mov edx, [ebp + SH2.GBR]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 2, 3, 4


ALIGN32

DECLF SH2I_STCMVBR_DS

	and ebx, byte 0xF
	mov edx, [ebp + SH2.VBR]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 2, 3, 4


ALIGN32

DECLF SH2I_STSMACH_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.MACH]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_STSMACL_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.MACL]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_STSPR_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.PR]
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_STSMMACH_DS

	and ebx, byte 0xF
	mov edx, [ebp + SH2.MACH]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 2, 3, 4


ALIGN32

DECLF SH2I_STSMMACL_DS

	and ebx, byte 0xF
	mov edx, [ebp + SH2.MACL]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 2, 3, 4


ALIGN32

DECLF SH2I_STSMPR_DS

	and ebx, byte 0xF
	mov edx, [ebp + SH2.PR]
	mov eax, [ebp + SH2.R + ebx * 4]
	sub eax, byte 4
	mov [ebp + SH2.R + ebx * 4], eax
	WRITE_LONG
	RET_DS 2, 3, 4


ALIGN32

DECLF SH2I_SUB_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	sub edx, eax
	mov [ebp + SH2.R + ebx * 4], edx
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SUBC_DS

	F_NM
	mov cl, [ebp + SH2.SR_T]
	mov edx, [ebp + SH2.R + ebx * 4]
	shr cl, 1
	mov eax, [ebp + SH2.R + eax * 4]
	sbb edx, eax
	setc [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], edx
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_SUBV_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	sub edx, eax
	seto [ebp + SH2.SR_T]
	mov [ebp + SH2.R + ebx * 4], edx
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_SWAPB_DS

	F_NM
	mov dx, [ebp + SH2.R + eax * 4 + 0]
	mov eax, [ebp + SH2.R + eax * 4 + 0]
	rol dx, 8
	mov [ebp + SH2.R + ebx * 4 + 0], eax
	mov [ebp + SH2.R + ebx * 4 + 0], dx
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_SWAPW_DS

	F_NM
	mov eax, [ebp + SH2.R + eax * 4 + 0]
	rol eax, 16
	mov [ebp + SH2.R + ebx * 4 + 0], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_TAS_DS

	and ebx, byte 0xF
	mov eax, [ebp + SH2.R + ebx * 4]
	READ_BYTE
	test al, al
	mov dl, al
	setz cl
	or dl, 0x80							; work only if BUS can be locked !
	mov [ebp + SH2.SR_T], cl
	mov eax, [ebp + SH2.R + ebx * 4]
	WRITE_BYTE
	RET_DS 4, 6, 8


ALIGN32

DECLF SH2I_TST_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	test eax, edx
	setz [ebp + SH2.SR_T]
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_TSTI_DS

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	test eax, edx
	setz [ebp + SH2.SR_T]
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_TSTM_DS

	mov eax, [ebp + SH2.R + 0]
	mov edx, [ebp + SH2.GBR]
	shr ebx, 8
	add eax, edx
	READ_BYTE
	test eax, ebx
	setz [ebp + SH2.SR_T]
	RET_DS 3, 4, 6


ALIGN32

DECLF SH2I_XOR_DS

	F_NM
	mov edx, [ebp + SH2.R + ebx * 4]
	mov eax, [ebp + SH2.R + eax * 4]
	xor eax, edx
	mov [ebp + SH2.R + ebx * 4], eax
	RET_DS 1, 1, 1


ALIGN32

DECLF SH2I_XORI_DS

	shr eax, 8
	mov edx, [ebp + SH2.R + 0]
	xor eax, edx
	mov [ebp + SH2.R + 0], eax
	RET_DS 1, 1, 2


ALIGN32

DECLF SH2I_XORM_DS

	mov eax, [ebp + SH2.R + 0]
	mov edx, [ebp + SH2.GBR]
	shr ebx, 8
	add eax, edx
	push eax
	READ_BYTE
	mov edx, eax
	pop eax
	xor edx, ebx
	WRITE_BYTE
	RET_DS 3, 5, 7


ALIGN32

DECLF SH2I_XTRCT_DS

	F_NM
	mov cx, [ebp + SH2.R + eax * 4 + 0]
	mov dx, [ebp + SH2.R + ebx * 4 + 2]
	mov [ebp + SH2.R + ebx * 4 + 2], cx
	mov [ebp + SH2.R + ebx * 4 + 0], dx
	RET_DS 1, 1, 2



; Public functions
; ================


ALIGN64

; int FASTCALL SH2_Exec(SH2_CONTEXT *sh2, int odo)
; eax/ecx = context pointer
; edx = odometer to raise
;
; RETURN:
; 0  -> ok
; !0 -> error (status returned) or no cycle to do (-1)

DECLF SH2_Exec, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	sub edx, [ecx + SH2.Odometer]
	push ebx
	push edi
	push esi
	push ebp
	jbe short SH2_Exec_Error

	lea edi, [edx - 1]
	mov ebp, ecx
	xor ebx, ebx
	mov esi, [ecx + SH2.PC]

	CHECK_INT

	mov al, [ebp + SH2.Status]
	xor ecx, ecx
	test al, SH2_HALTED | SH2_DISABLE | SH2_FAULTED | SH2_RUNNING
	jz short SH2_Exec_OK

	test al, SH2_HALTED
	jnz near SH2_Exec_Halted

	test al, SH2_DISABLE
	jnz near SH2_Exec_Disable

SH2_Exec_Error:

	or eax, byte -1
	pop ebp
	pop esi
	pop edi
	pop ebx
	FUNC_OUT

ALIGN4

SH2_Exec_OK:

	or al, SH2_RUNNING
	movzx ebx, word [esi - 4]
	mov [ebp + SH2.Status], al
	mov [ebp + SH2.Cycle_Sup], ecx
	mov [ebp + SH2.Cycle_TD], edi
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

ALIGN32

SH2_Exec_Interrupt_Happened:

	CHECK_INT

	movzx ebx, word [esi - 4]
	mov eax, ebx
	jmp [OP_Table + ebx * 4]

ALIGN32

SH2_Exec_Quit:

	mov eax, [ebp + SH2.Cycle_Sup]
	xor ebx, ebx
	add edi, eax
	mov [ebp + SH2.Cycle_Sup], ebx
	jns short SH2_Exec_Interrupt_Happened

ALIGN4

SH2_Exec_Really_Quit:

	mov eax, [ebp + SH2.Cycle_TD]
	xor ecx, ecx
	mov ebx, [ebp + SH2.Odometer]
	mov cl, [ebp + SH2.Status]
	sub eax, edi
	mov [ebp + SH2.PC], esi
	add eax, ebx
	mov dl, [ebp + SH2.WDTSR]
	and cl, ~SH2_RUNNING
	mov [ebp + SH2.Odometer], eax
	test dl, 0x20
	mov [ebp + SH2.Status], cl
	jz short SH2_Exec_Do_FRT

	call SH2_Exec_Check_WTC

ALIGN4

SH2_Exec_Do_FRT

%ifdef FRT_SUPPORT
	mov eax, [ebp + SH2.Cycle_TD]
	mov cl, [ebp + SH2.FRT_Sft]
	inc eax
	mov esi, [ebp + SH2.FRTCNT]		; old FRT
	shl eax, cl
	mov edx, [ebp + SH2.FRTOCRA]
	mov bl, [ebp + SH2.FRTCSR]
	mov bh, [ebp + SH2.FRTTIER]
	lea edi, [esi + eax]			; new FRT

	cmp edx, esi
	ja short .compA_done
	cmp edx, edi
	jb short .compA_done

	test bl, 0x01
	jz short .no_clearA

	xor edi, edi

.no_clearA
	or bl, 0x08
	test bh, 0x08
	jz short .compA_done

	mov dh, [ebp + SH2.IO_Reg + 0x67]
	mov dl, [ebp + SH2.IO_Reg + 0x60]
	mov ecx, ebp
	and dh, 0x7F
	and dl, 0x0F
	FUNC_CALL_IN
	call SH2_Interrupt_Internal
	FUNC_CALL_OUT

.compA_done
	mov edx, [ebp + SH2.FRTOCRA]
	cmp edx, esi
	ja short .compB_done
	cmp edx, edi
	jb short .compB_done

	or bl, 0x04
	test bh, 0x04
	jz short .compB_done

	mov dh, [ebp + SH2.IO_Reg + 0x67]
	mov dl, [ebp + SH2.IO_Reg + 0x60]
	mov ecx, ebp
	and dh, 0x7F
	and dl, 0x0F
	FUNC_CALL_IN
	call SH2_Interrupt_Internal
	FUNC_CALL_OUT

.compB_done
	test edi, 0xFF000000
	jz short .FRT_done

	or bl, 0x02
	and edi, 0x00FFFFFF
	test bh, 0x02
	jz short .FRT_done

	mov dh, [ebp + SH2.IO_Reg + 0x68]
	mov dl, [ebp + SH2.IO_Reg + 0x60]
	mov ecx, ebp
	and dh, 0x7F
	and dl, 0x0F
	FUNC_CALL_IN
	call SH2_Interrupt_Internal
	FUNC_CALL_OUT

.FRT_done
	mov [ebp + SH2.FRTCNT], edi
	mov [ebp + SH2.FRTCSR], bl
%endif

	xor eax, eax
	pop ebp
	pop esi
	pop edi
	pop ebx
	FUNC_OUT

ALIGN32

SH2_Exec_Halted:

	mov [ebp + SH2.Cycle_TD], edi
	mov eax, [ebp + SH2.Odometer]
	mov dl, [ebp + SH2.WDTSR]
	add eax, edi
	test dl, 0x20
	mov [ebp + SH2.Odometer], eax
	jz near SH2_Exec_Do_FRT

	call SH2_Exec_Check_WTC
	jmp SH2_Exec_Do_FRT


ALIGN32

SH2_Exec_Disable:

	mov eax, [ebp + SH2.Odometer]
	add eax, edi
	mov [ebp + SH2.Odometer], eax

	xor eax, eax
	pop ebp
	pop esi
	pop edi
	pop ebx
	FUNC_OUT

ALIGN32

SH2_Exec_Check_WTC:

	mov eax, [ebp + SH2.Cycle_TD]
	mov cl, [ebp + SH2.WDT_Sft]
	inc eax
	mov ebx, [ebp + SH2.WDTCNT]
	shl eax, cl
	add ebx, eax
	test ebx, 0xFF000000
	jnz short .Overflow

	mov [ebp + SH2.WDTCNT], ebx
	FUNC_OUT

ALIGN4

.Overflow
	xor eax, eax
	test dl, 0x40
	mov [ebp + SH2.WDTCNT], eax
	jnz short .W_Mode

	or dl, 0x80
	mov ecx, ebp
	mov [ebp + SH2.WDTSR], dl
	mov dh, [ebp + SH2.VCRWDT]
	mov dl, [ebp + SH2.IPWDT]
	FUNC_CALL_IN
	call SH2_Interrupt_Internal
	FUNC_CALL_OUT
	FUNC_OUT

ALIGN4

.W_Mode
	mov [ebp + SH2.WDTSR], dl
	mov dl, [ebp + SH2.WDTRST]
	mov ecx, ebp
	test dl, 0x40
	jz short .No_Reset

	shr dl, 5
	FUNC_CALL_IN
	call SH2_Reset
	FUNC_CALL_OUT
	or byte [ebp + SH2.WDTRST], 0x80
	FUNC_OUT

ALIGN4

.No_Reset
	or dl, 0x80
	mov [ebp + SH2.WDTRST], dl
	FUNC_OUT


ALIGN64

; int FASTCALL SH2_Reset(SH2_CONTEXT *sh2, int manual)
; ecx = context pointer
; edx = manual reset flag (1 for manual, 0 for power-up)
;
; RETURN:
; 0  -> ok
; -1 -> error

DECLF SH2_Reset, 8

	FUNC_IN
	push edi
	push esi
	push ebp
%ifdef __GCC2
	mov ecx, eax
%endif
	mov eax, [ecx + SH2.Status]
	and eax, SH2_DISABLE
	push eax
	
	mov edi, ecx
	xor eax, eax
	and edx, byte 1
	mov ecx, SH2.Reset_Size / 4
	mov ebp, edi
	lea edx, [edx * 8]

	rep stosd
	
	pop eax
	mov [ebp + SH2.Status], al	; we keep the disable status during reset

	mov eax, edx
	mov edi, edx			; edi not lost with READ_LONG

	READ_LONG				; Read PC

	mov esi, eax
	lea eax, [edi + 4]
	add esi, byte 4

	READ_LONG				; Read SP

	mov [ebp + SH2.R + 15 * 4], eax

	REBASE_PC 1

	mov [ebp + SH2.PC], esi
	mov byte [ebp + SH2.SR_IMask], 0x0F

	xor eax, eax
	mov [ebp + SH2.WDTCNT], eax
	mov [ebp + SH2.WDTSR], eax
	mov [ebp + SH2.WDTRST], eax
	mov [ebp + SH2.BARA], eax
	mov [ebp + SH2.BAMRA], eax
	mov [ebp + SH2.TCR0], eax
	mov [ebp + SH2.TCR1], eax

	mov dword [ebp + SH2.FRTOCRA], 0xFFFF << 8
	mov dword [ebp + SH2.FRTOCRB], 0xFFFF << 8

	mov byte [ebp + SH2.WDT_Sft], 16 - 1
	mov byte [ebp + SH2.FRT_Sft], 8 - 3

	pop ebp
	pop esi
	pop edi
	FUNC_OUT

ALIGN4

.error
	or eax, byte -1			; SH2 not correctly reseted

	pop ebp
	pop esi
	pop edi
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Enable(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Return the number of cycles executed since the last
; clear odometer command

DECLF SH2_Enable, 4

	FUNC_IN
	and byte [ecx + SH2.Status], ~SH2_DISABLE
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Disable(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Return the number of cycles executed since the last
; clear odometer command

DECLF SH2_Disable, 4

	FUNC_IN
	or byte [ecx + SH2.Status], SH2_DISABLE
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Read_Odo(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Return the number of cycles executed since the last
; clear odometer command

DECLF SH2_Read_Odo, 4

	FUNC_IN
	test byte [ecx + SH2.Status], SH2_RUNNING
	mov eax, [ecx + SH2.Odometer]
	jz short .not_running

	add eax, [ecx + SH2.Cycle_TD]
	mov edx, [ecx + SH2.Cycle_IO]
	sub eax, edx

.not_running
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Write_Odo(SH2_CONTEXT *sh2, UINT32 odo)
; ecx = context pointer
; edx = new odometer
; Work only if not running
;
; RETURN:
; nothing ...

DECLF SH2_Write_Odo, 8

	FUNC_IN
	mov [ecx + SH2.Odometer], edx
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Clear_Odo(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; nothing ...

DECLF SH2_Clear_Odo, 4
%ifdef __GCC2
		mov ecx, eax
%endif
	FUNC_IN
	mov dword [ecx + SH2.Odometer], 0
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Add_Cycles(SH2_CONTEXT *sh2, UINT32 cycles)
; ecx = context pointer
; edx = number of cycles to add
;
; RETURN:
; nothing ...

DECLF SH2_Add_Cycles, 8

	FUNC_IN
	test byte [ecx + SH2.Status], SH2_RUNNING
	mov eax, [ecx + SH2.Cycle_IO]
	jz short .not_running

	sub eax, edx
	mov [ecx + SH2.Cycle_IO], eax
	FUNC_OUT

ALIGN4

.not_running
	mov eax, [ecx + SH2.Odometer]
	add eax, edx
	mov [ecx + SH2.Odometer], eax
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Interrupt(SH2_CONTEXT *sh2, int level)
; ecx = context pointer
; edx = interrupt level
;
; RETURN:
; nothing ...

DECLF SH2_Interrupt, 8
%ifdef __GCC2
		mov ecx, eax
%endif
	FUNC_IN
	mov ah, dl
	and edx, byte 0x0F
	shr ah, 1
	mov al, [ecx + SH2.INT_Prio]
	add ah, 64
	cmp dl, al
	mov [ecx + SH2.INT_QUEUE + edx], ah
	jbe short .not_higher

	mov al, [ecx + SH2.Status]
	mov [ecx + SH2.INT_Prio], dl
	test al, SH2_RUNNING
	mov [ecx + SH2.INT_Vect], ah
	jnz short .running

	FUNC_OUT

ALIGN4

.running
	mov eax, [ecx + SH2.Cycle_IO]
	xor edx, edx
	mov [ecx + SH2.Cycle_Sup], eax
	mov [ecx + SH2.Cycle_IO], edx
	FUNC_OUT

ALIGN4

.not_higher
	mov al, 0
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_Interrupt_Internal(SH2_CONTEXT *sh2, int level_vector)
; ecx = context pointer
; dl = interrupt level
; dh = interrupt vector
;
; RETURN:
; nothing ...

DECLF SH2_Interrupt_Internal, 8

	FUNC_IN

SH2_Interrupt_Internal_Short
	mov ah, dh
	and edx, byte 0x1F
	mov al, [ecx + SH2.INT_Prio]
	cmp dl, al
	mov [ecx + SH2.INT_QUEUE + edx], ah
	jbe short .not_higher

	mov al, [ecx + SH2.Status]
	mov [ecx + SH2.INT_Prio], dl
	test al, SH2_RUNNING
	mov [ecx + SH2.INT_Vect], ah
	jnz short .running

	FUNC_OUT

ALIGN4

.running
	mov eax, [ecx + SH2.Cycle_IO]
	xor edx, edx
	mov [ecx + SH2.Cycle_Sup], eax
	mov [ecx + SH2.Cycle_IO], edx
	FUNC_OUT

ALIGN4

.not_higher
	mov al, 0
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_NMI(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; nothing ...

DECLF SH2_NMI, 4

	FUNC_IN
	or byte [ecx + SH2.DMAOR], 2
	mov dl, 16
	mov dh, 11
	jmp short SH2_Interrupt_Internal_Short


ALIGN32

; void FASTCALL SH2_DMA0_Request(SH2_CONTEXT *sh2, UINT8 state)
; ecx = context pointer
; edx = request line for external (0 = off, !0 = on)
;
; RETURN:
; nothing ...

DECLF SH2_DMA0_Request, 8

	FUNC_IN
	push ebp
	mov [ecx + SH2.DREQ0], dl
	mov ebp, ecx
	mov edx, [ecx + SH2.CHCR0]
	mov ecx, 0xFFFFFF8C
	FUNC_CALL_IN
	call [ebp + SH2.Write_Long + 0xFF * 4]
	FUNC_CALL_OUT
	pop ebp
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_DMA1_Request(SH2_CONTEXT *sh2, UINT8 state)
; ecx = context pointer
; edx = request line for external (0 = off, !0 = on)
;
; RETURN:
; nothing ...

DECLF SH2_DMA1_Request, 8

	FUNC_IN
	push ebp
	mov [ecx + SH2.DREQ1], dl
	mov ebp, ecx
	mov edx, [ecx + SH2.CHCR1]
	mov ecx, 0xFFFFFF9C
	FUNC_CALL_IN
	call [ebp + SH2.Write_Long + 0xFF * 4]
	FUNC_CALL_OUT
	pop ebp
	FUNC_OUT


ALIGN32

; void FASTCALL SH2_FRT_Signal(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; nothing ...

DECLF SH2_FRT_Signal, 4

	FUNC_IN
	mov dl, [ecx + SH2.FRTCSR]
	mov dh, [ecx + SH2.FRTTIER]
	or dl, 0x80
	mov eax, [ecx + SH2.FRTCNT]
	test dh, 0x80
	mov [ecx + SH2.FRTICR], eax
	mov [ecx + SH2.FRTCSR], dl
	jz short .done

	mov dh, [ecx + SH2.IO_Reg + 0x66]
	mov dl, [ecx + SH2.IO_Reg + 0x60]
	and dh, 0x7F
	and dl, 0x0F
	FUNC_CALL_IN
	call SH2_Interrupt_Internal
	FUNC_CALL_OUT

.done
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_R(SH2_CONTEXT *sh2, int num)
; ecx = context pointer
;
; RETURN:
; Current Rx value

DECLF SH2_Get_R, 8

	FUNC_IN
	and edx, 0x0F
	mov eax, [ecx + SH2.R + edx * 4]
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_PC(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current PC value (don't work during SH2_Exec)

DECLF SH2_Get_PC, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	test byte [ecx + SH2.Status], SH2_RUNNING
	mov eax, [ecx + SH2.PC]
	jnz short .running

	sub eax, [ecx + SH2.Base_PC]
	FUNC_OUT

ALIGN4

.running
	mov eax, -1
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_SR(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current SR value

DECLF SH2_Get_SR, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	xor eax, eax
	mov dh, [ecx + SH2.SR_IMask]
	mov dl, [ecx + SH2.SR_S]
	shl dh, 4
	mov al, [ecx + SH2.SR_T]
	add dl, dl
	or al, dh
	mov ah, [ecx + SH2.SR_MQ]
	or al, dl
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_GBR(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current GBR value

DECLF SH2_Get_GBR, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov eax, [ecx + SH2.GBR]
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_VBR(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current VBR value

DECLF SH2_Get_VBR, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov eax, [ecx + SH2.VBR]
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_PR(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current PR value

DECLF SH2_Get_PR, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov eax, [ecx + SH2.PR]
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_MACH(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current PR value

DECLF SH2_Get_MACH, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov eax, [ecx + SH2.MACH]
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Get_MACL(SH2_CONTEXT *sh2)
; ecx = context pointer
;
; RETURN:
; Current PR value

DECLF SH2_Get_MACL, 4

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov eax, [ecx + SH2.MACL]
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_PC(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_PC, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	test byte [ecx + SH2.Status], SH2_RUNNING
	jz short .not_running

	FUNC_OUT

.not_running
	push esi
	xor eax, eax
	mov esi, edx

.Loop
	cmp esi, [ecx + SH2.Fetch_Region + eax + 0]
	jb short .Next
	cmp esi, [ecx + SH2.Fetch_Region + eax + 4]
	jbe short .Base

.Next
	mov edx, [ecx + SH2.Fetch_Region + eax + 8]
	add eax, byte 12
	cmp edx, byte -1
	jne short .Loop

.Error
	mov al, [ecx + SH2.Status]
	mov [ecx + SH2.Fetch_Start], edx
	xor edx, edx
	or al, SH2_FAULTED
	mov [ecx + SH2.Base_PC], edx
	mov [ecx + SH2.Fetch_End], edx
	mov [ecx + SH2.Status], al
	pop esi
	FUNC_OUT

.Base
	mov edx, [ecx + SH2.Fetch_Region + eax + 0]
	mov [ecx + SH2.Fetch_Start], edx
	mov edx, [ecx + SH2.Fetch_Region + eax + 4]
	mov [ecx + SH2.Fetch_End], edx
	mov edx, [ecx + SH2.Fetch_Region + eax + 8]
	mov [ecx + SH2.Base_PC], edx
	add esi, edx
	mov [ecx + SH2.PC], esi
	pop esi
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_SR(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_SR, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	and edx, 0x3F3
	mov edx, [Set_SR_Table + edx * 4]
	mov [ecx + SH2.SR], edx
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_GBR(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_GBR, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov [ecx + SH2.GBR], edx
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_VBR(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_VBR, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov [ecx + SH2.VBR], edx
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_PR(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_PR, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov [ecx + SH2.PR], edx
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_MACH(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_MACH, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov [ecx + SH2.MACH], edx
	FUNC_OUT


ALIGN32

; int FASTCALL SH2_Set_MACL(SH2_CONTEXT *sh2, UINT32 val)
; ecx = context pointer
; edx = new value
;
; RETURN: none

DECLF SH2_Set_MACL, 8

	FUNC_IN
%ifdef __GCC2
	mov ecx, eax
%endif
	mov [ecx + SH2.MACL], edx
	FUNC_OUT
 