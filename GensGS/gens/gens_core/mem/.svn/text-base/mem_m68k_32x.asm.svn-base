%include "nasmhead.inc"

%define CYCLE_FOR_TAKE_Z80_BUS_GENESIS 16
%define PWM_BUF_SIZE 4

section .bss align=64
	
	; External symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define	_Controller_1_Counter	Controller_1_Counter
		%define	_Controller_1_Delay	Controller_1_Delay
		%define	_Controller_1_State	Controller_1_State
		%define	_Controller_1_COM	Controller_1_COM
		
		%define	_Controller_2_Counter	Controller_2_Counter
		%define	_Controller_2_Delay	Controller_2_Delay
		%define	_Controller_2_State	Controller_2_State
		%define	_Controller_2_COM	Controller_2_COM
	%endif
	
	extern _Controller_1_Counter
	extern _Controller_1_Delay
	extern _Controller_1_State
	extern _Controller_1_COM
	
	extern _Controller_2_Counter
	extern _Controller_2_Delay
	extern _Controller_2_State
	extern _Controller_2_COM
	
	extern Rom_Data
	
	; External symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define	_M_Z80	M_Z80
	%endif
	extern _M_Z80
	extern Z80_State
	extern _Last_BUS_REQ_Cnt
	extern _Last_BUS_REQ_St
	
	extern M_SH2
	extern S_SH2
	
	extern Game_Mode
	extern CPU_Mode
	extern Gen_Version
	
	extern _32X_Comm
	extern _32X_ADEN
	extern _32X_RES
	extern _32X_FM
	extern _32X_RV
	extern _32X_DREQ_ST
	extern _32X_DREQ_SRC
	extern _32X_DREQ_DST
	extern _32X_DREQ_LEN
	extern _32X_FIFO_A
	extern _32X_FIFO_B
	extern _32X_FIFO_Block
	extern _32X_FIFO_Write
	extern _32X_FIFO_Read
	extern _32X_MINT
	extern _32X_SINT
	
	; External Symbol redefines for ELF.
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
	%endif
	
	extern __32X_Palette_16B
	extern __32X_Palette_32B
	extern __32X_VDP_CRam_Ajusted
	extern __32X_VDP_CRam_Ajusted32
	extern __32X_VDP_Ram
	extern __32X_VDP_CRam
	extern __32X_VDP
	
	; External symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define _PWM_FIFO_R PWM_FIFO_R
		%define _PWM_FIFO_L PWM_FIFO_L
		%define _PWM_FULL_TAB PWM_FULL_TAB
		%define _PWM_RP_R PWM_RP_R
		%define _PWM_WP_R PWM_WP_R
		%define _PWM_RP_L PWM_RP_L
		%define _PWM_WP_L PWM_WP_L
		%define _PWM_Mode PWM_Mode
	%endif

	extern _PWM_FIFO_R
	extern _PWM_FIFO_L
	extern _PWM_FULL_TAB
	extern _PWM_RP_R
	extern _PWM_WP_R
	extern _PWM_RP_L
	extern _PWM_WP_L
	extern _PWM_Mode
	
	; External symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define _PWM_Cycle_Tmp		PWM_Cycle_Tmp
		%define _PWM_Int_Tmp		PWM_Int_Tmp
		%define _PWM_FIFO_L_Tmp		PWM_FIFO_L_Tmp
		%define _PWM_FIFO_R_Tmp		PWM_FIFO_R_Tmp
	%endif

	extern _PWM_Cycle_Tmp
	extern _PWM_Int_Tmp
	extern _PWM_FIFO_L_Tmp
	extern _PWM_FIFO_R_Tmp
	
	extern Z80_M68K_Cycle_Tab
	extern Rom_Data
	extern Bank_M68K
	extern Fake_Fetch
	
	extern Cycles_M68K
	extern Cycles_Z80
	
	extern SRAM_ON
	extern SRAM_Write
	
	; 32X MC68000 firmware
	DECL _32X_Genesis_Rom
		resb 256
	
	DECL Bank_SH2
		resd 1
	
	struc vx
		.Mode:		resd 1
		.State:		resd 1
		.AF_Data:	resd 1
		.AF_St:		resd 1
		.AF_Len:	resd 1
		.AF_Line:	resd 1
	endstruc
	
section .data align=64
	
	; 32X Default Jump Table
	
	extern M68K_Read_Byte_Bad
	
	extern Genesis_M68K_Read_Byte_Table
	extern Genesis_M68K_Read_Word_Table
	
	extern M68K_Read_Byte_Table
	extern M68K_Read_Word_Table
	
	DECL _32X_M68K_Read_Byte_Table
		dd	M68K_Read_Byte_Rom0,		; 0x000000 - 0x07FFFF
		dd	M68K_Read_Byte_Rom1,		; 0x080000 - 0x0FFFFF
		dd	M68K_Read_Byte_Rom2,		; 0x100000 - 0x17FFFF
		dd	M68K_Read_Byte_Rom3,		; 0x180000 - 0x1FFFFF
		dd	M68K_Read_Byte_Rom4,		; 0x200000 - 0x27FFFF
		dd	M68K_Read_Byte_Rom5,		; 0x280000 - 0x2FFFFF
		dd	M68K_Read_Byte_Rom6,		; 0x300000 - 0x37FFFF
		dd	M68K_Read_Byte_Rom7,		; 0x380000 - 0x3FFFFF
		dd	M68K_Read_Byte_Bios_32X,	; 0x400000 - 0x47FFFF
		dd	M68K_Read_Byte_BiosR_32X,; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x500000 - 0x57FFFF
		dd	M68K_Read_Byte_Bad,		; 0x580000 - 0x5FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x600000 - 0x67FFFF
		dd	M68K_Read_Byte_Bad,		; 0x680000 - 0x6FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x700000 - 0x77FFFF
		dd	M68K_Read_Byte_32X_FB0,	; 0x780000 - 0x7FFFFF
		dd	M68K_Read_Byte_32X_FB1,	; 0x800000 - 0x87FFFF
		dd	M68K_Read_Byte_Rom0,		; 0x880000 - 0x8FFFFF
		dd	M68K_Read_Byte_Rom1,		; 0x900000 - 0x97FFFF
		dd	M68K_Read_Byte_Rom2,		; 0x980000 - 0x9FFFFF
		dd	M68K_Read_Byte_Misc_32X,	; 0xA00000 - 0xA7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xA80000 - 0xAFFFFF
		dd	M68K_Read_Byte_Bad,		; 0xB00000 - 0xB7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xB80000 - 0xBFFFFF
		dd	M68K_Read_Byte_VDP,		; 0xC00000 - 0xC7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xC80000 - 0xCFFFFF
		dd	M68K_Read_Byte_Bad,		; 0xD00000 - 0xD7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xD80000 - 0xDFFFFF
		dd	M68K_Read_Byte_Ram,		; 0xE00000 - 0xE7FFFF
		dd	M68K_Read_Byte_Ram,		; 0xE80000 - 0xEFFFFF
		dd	M68K_Read_Byte_Ram,		; 0xF00000 - 0xF7FFFF
		dd	M68K_Read_Byte_Ram,		; 0xF80000 - 0xFFFFFF

	DECL _32X_M68K_Read_Word_Table
		dd	M68K_Read_Word_Rom0,		; 0x000000 - 0x07FFFF
		dd	M68K_Read_Word_Rom1,		; 0x080000 - 0x0FFFFF
		dd	M68K_Read_Word_Rom2,		; 0x100000 - 0x17FFFF
		dd	M68K_Read_Word_Rom3,		; 0x180000 - 0x1FFFFF
		dd	M68K_Read_Word_Rom4,		; 0x200000 - 0x27FFFF
		dd	M68K_Read_Word_Rom5,		; 0x280000 - 0x2FFFFF
		dd	M68K_Read_Word_Rom6,		; 0x300000 - 0x37FFFF
		dd	M68K_Read_Word_Rom7,		; 0x380000 - 0x3FFFFF
		dd	M68K_Read_Word_Bios_32X,	; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Word_BiosR_32X,	; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Word_Bad,		; 0x500000 - 0x57FFFF
		dd	M68K_Read_Word_Bad,		; 0x580000 - 0x5FFFFF
		dd	M68K_Read_Word_Bad,		; 0x600000 - 0x67FFFF
		dd	M68K_Read_Word_Bad,		; 0x680000 - 0x6FFFFF
		dd	M68K_Read_Word_Bad,		; 0x700000 - 0x77FFFF
		dd	M68K_Read_Word_32X_FB0,		; 0x780000 - 0x7FFFFF
		dd	M68K_Read_Word_32X_FB1,		; 0x800000 - 0x87FFFF
		dd	M68K_Read_Word_Rom0,		; 0x880000 - 0x8FFFFF
		dd	M68K_Read_Word_Rom1,		; 0x900000 - 0x97FFFF
		dd	M68K_Read_Word_Rom2,		; 0x980000 - 0x9FFFFF
		dd	M68K_Read_Word_Misc_32X,	; 0xA00000 - 0xA7FFFF
		dd	M68K_Read_Word_Bad,		; 0xA80000 - 0xAFFFFF
		dd	M68K_Read_Word_Bad,		; 0xB00000 - 0xB7FFFF
		dd	M68K_Read_Word_Bad,		; 0xB80000 - 0xBFFFFF
		dd	M68K_Read_Word_VDP,		; 0xC00000 - 0xC7FFFF
		dd	M68K_Read_Word_Bad,		; 0xC80000 - 0xCFFFFF
		dd	M68K_Read_Word_Bad,		; 0xD00000 - 0xD7FFFF
		dd	M68K_Read_Word_Bad,		; 0xD80000 - 0xDFFFFF
		dd	M68K_Read_Word_Ram,		; 0xE00000 - 0xE7FFFF
		dd	M68K_Read_Word_Ram,		; 0xE80000 - 0xEFFFFF
		dd	M68K_Read_Word_Ram,		; 0xF00000 - 0xF7FFFF
		dd	M68K_Read_Word_Ram,		; 0xF80000 - 0xFFFFFF
	
	DECL _32X_M68K_Write_Byte_Table
		dd	M68K_Write_Bad,			; 0x000000 - 0x0FFFFF
		dd	M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x200000 - 0x2FFFFF
		dd	M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd	M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd	M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd	M68K_Write_Bad,			; 0x600000 - 0x6FFFFF
		dd	M68K_Write_Byte_32X_FB0,	; 0x700000 - 0x7FFFFF
		dd	M68K_Write_Byte_32X_FB1,	; 0x800000 - 0x8FFFFF
		dd	M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd	M68K_Write_Byte_Misc_32X,	; 0xA00000 - 0xAFFFFF
		dd	M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd	M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd	M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd	M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd	M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF
	
	DECL _32X_M68K_Write_Word_Table
		dd	M68K_Write_Bad,			; 0x000000 - 0x0FFFFF
		dd	M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x200000 - 0x2FFFFF
		dd	M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd	M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd	M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd	M68K_Write_Bad,			; 0x600000 - 0x6FFFFF
		dd	M68K_Write_Word_32X_FB0,	; 0x700000 - 0x7FFFFF
		dd	M68K_Write_Word_32X_FB1,	; 0x800000 - 0x8FFFFF
		dd	M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd	M68K_Write_Word_Misc_32X,	; 0xA00000 - 0xAFFFFF
		dd	M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd	M68K_Write_Word_VDP,		; 0xC00000 - 0xCFFFFF
		dd	M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd	M68K_Write_Word_Ram,		; 0xE00000 - 0xEFFFFF
		dd	M68K_Write_Word_Ram,		; 0xF00000 - 0xFFFFFF
	
	_32X_ID_Tab:
		db	'A', 'M', 'S', 'R'
	
section .text align=64
	
	extern Z80_ReadB_Table
	extern Z80_WriteB_Table
	
	extern M68K_Read_Byte_Rom0
	extern M68K_Read_Byte_Rom1
	extern M68K_Read_Byte_Rom2
	extern M68K_Read_Byte_Rom3
	extern M68K_Read_Byte_Rom4
	extern M68K_Read_Byte_Rom5
	extern M68K_Read_Byte_Rom6
	extern M68K_Read_Byte_Rom7
	
	extern M68K_Read_Byte_VDP
	extern M68K_Read_Byte_Ram
	
	extern M68K_Read_Word_Rom0
	extern M68K_Read_Word_Rom1
	extern M68K_Read_Word_Rom2
	extern M68K_Read_Word_Rom3
	extern M68K_Read_Word_Rom4
	extern M68K_Read_Word_Rom5
	extern M68K_Read_Word_Rom6
	extern M68K_Read_Word_Rom7
	
	extern M68K_Read_Word_Bad
	extern M68K_Read_Word_VDP
	extern M68K_Read_Word_Ram
	
	extern M68K_Write_Bad
	extern M68K_Write_Byte_SRAM
	extern M68K_Write_Byte_VDP
	extern M68K_Write_Byte_Ram
	
	extern M68K_Write_Word_SRAM
	extern M68K_Write_Word_VDP
	extern M68K_Write_Word_Ram
	
	extern _main68k_readOdometer
	extern z80_Reset
	extern z80_Exec
	extern z80_Set_Odo
	
	extern SH2_Reset
	extern SH2_Interrupt
	extern SH2_DMA0_Request
	
	extern _YM2612_Reset
	
	extern _RD_Controller_1
	extern _RD_Controller_2 
	extern _WR_Controller_1
	extern _WR_Controller_2
	
	extern _M68K_32X_Mode
	extern _M68K_Set_32X_Rom_Bank
	extern __32X_Set_FB
	extern _PWM_Set_Cycle
	extern _PWM_Set_Int
	
	; 32X extended Read Byte
	; *******************************************
	
	align 64
	
	M68K_Read_Byte_Bios_32X:
		and	ebx, 0xFF
		xor	ebx, byte 1
		mov	al, [_32X_Genesis_Rom + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_BiosR_32X:
		cmp	ebx, 0x100
		jae	short .Rom
		
		xor	ebx, byte 1
		mov	al, [_32X_Genesis_Rom + ebx]
		pop	ebx
		ret
	
	align 16
	
	.Rom:
		xor	ebx, byte 1
		mov	al, [Rom_Data + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_Misc_32X:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_mem
		
		test	byte [Z80_State], 6
		jnz	short .bad
		
		push	ecx
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		call	[Z80_ReadB_Table + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		xor	al, al
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_mem:
		cmp	ebx, 0xA11100
		jne	short .no_busreq
		
		test	byte [Z80_State], 2
		jnz	short .z80_on
	
	.z80_off:
		call 	_main68k_readOdometer
		sub	eax, [_Last_BUS_REQ_Cnt]
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja	short .bus_taken
		
		mov	al, [_Last_BUS_REQ_St]
		pop	ebx
		or	al, 0x80
		ret
	
	align 4
	
	.bus_taken:
		mov	al, 0x80
		pop	ebx
		ret
	
	align 4
	
	.z80_on:
		mov	al, 0x81
		pop	ebx
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA15100
		jae	near ._32X_Reg
		
		cmp	ebx, 0xA130EC
		jae	short ._32X_ID
		
		cmp	ebx, 0xA1000F
		ja	short .bad
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_RB + ebx * 2]
	
	align 16
	
	._32X_ID:
		and	ebx, 3
		mov	al, [_32X_ID_Tab + ebx]
		pop	ebx
		ret
	
	align 16
	
	.Table_IO_RB:
		dd	.MD_Spec, .Pad_1, .Pad_2, .Ser
		dd	.CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad
	
	align 16
	
	.MD_Spec:
		mov	al, [Game_Mode]
		add	al, al
		or	al, [CPU_Mode]
		shl	al, 6
		pop	ebx
		or	al, [Gen_Version]
		ret
	
	align 8
	
	.Pad_1:
		call	_RD_Controller_1
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		call	_RD_Controller_2
		pop	ebx
		ret
	
	align 8
	
	.Ser:
		mov	al, 0xFF
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	al, [_Controller_1_COM]
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	al, [_Controller_2_COM]
		pop	ebx
		ret
	
	align 8
	
	.CT_Ser:
		xor	al, al
		pop	ebx
		ret
	
	align 16
	
	._32X_Reg:
		cmp	ebx, 0xA15180
		jae	near ._32X_VDP_Reg
		
		and	ebx, 0x3F
		jmp	[.Table_32X_Reg + ebx * 4]
	
	align 16
	
	.Table_32X_Reg:
		dd	._32X_ACR_H,	._32X_ACR_L,	._32X_bad,	._32X_Int	; 00-03
		dd	._32X_bad,	._32X_Bank,	._32X_bad,	._32X_DREQ_C	; 04-07
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 08-0B
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 0C-0F
		
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 10-13
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 14-17
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 18-1B
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 1C-1F
		
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm	; 20
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		
		dd	._32X_PWM_Cont_H,	._32X_PWM_Cont_L
		dd	._32X_PWM_Cycle_H,	._32X_PWM_Cycle_L
		dd	._32X_PWM_Pulse_L,	._32X_bad
		dd	._32X_PWM_Pulse_R,	._32X_bad
		dd	._32X_PWM_Pulse_L,	._32X_bad
		dd	._32X_bad,	._32X_bad
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad
	
	align 16
	
	._32X_ACR_H:
		mov	al, [_32X_FM]
		pop	ebx
		ret
	
	align 16
	
	._32X_ACR_L:
		mov	al, [_32X_ADEN]
		mov	ah, [_32X_RES]
		or	al, ah
		pop	ebx
		or	al, 0x80
		ret
	
	align 4
	
	._32X_Int:
		xor	al, al
		pop	ebx
		ret
	
	align 8
	
	._32X_Bank:
		mov	al, [Bank_SH2]
		pop	ebx
		ret
	
	align 16
	
	._32X_DREQ_C:
		mov	al, [_32X_RV]
		mov	bl, [_32X_DREQ_ST + 0]
		mov	ah, [_32X_DREQ_ST + 1]
		or	al, bl
		and	ah, 0x80
		pop	ebx
		or	al, ah
		ret
	
	align 8
	
	._32X_Comm:
		mov	al, [_32X_Comm + ebx - 0x20]
		pop	ebx
		ret
	
	align 8
	
	._32X_PWM_Cont_H:
		mov	al, [_PWM_Mode + 1]
		pop	ebx
		ret 
	
	align 8
	
	._32X_PWM_Cont_L:
		mov	al, [_PWM_Mode + 0]
		pop	ebx
		ret 
	
	align 8
	
	._32X_PWM_Cycle_H:
		mov	al, [_PWM_Cycle_Tmp + 1]
		pop	ebx
		ret
	
	align 8
	
	._32X_PWM_Cycle_L:
		mov	al, [_PWM_Cycle_Tmp + 0]
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_L:
		mov	ebx, [_PWM_RP_L]
		mov	eax, [_PWM_WP_L]
		mov	al, [_PWM_FULL_TAB + ebx * PWM_BUF_SIZE + eax]
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_R:
		mov	ebx, [_PWM_RP_R]
		mov	eax, [_PWM_WP_R]
		mov	al, [_PWM_FULL_TAB + ebx * PWM_BUF_SIZE + eax]
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_C:
		mov	ebx, [_PWM_RP_L]
		mov	eax, [_PWM_WP_L]
		mov	al, [_PWM_FULL_TAB + ebx * PWM_BUF_SIZE + eax]
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_Reg:
		test	byte [_32X_FM], 0xFF
		jnz	near ._32X_bad
		cmp	ebx, 0xA15200
		jae	near ._32X_bad
		
		and	ebx, 0xF
		jmp	[.Table_32X_VDP_Reg + ebx * 4]
	
	align 16

	.Table_32X_VDP_Reg:
		dd	._32X_VDP_Mode_H, ._32X_VDP_Mode_L, ._32X_bad, ._32X_VDP_Shift
		dd	._32X_bad, ._32X_VDP_AF_Len_L, ._32X_VDP_AF_St_H, ._32X_VDP_AF_St_L
		dd	._32X_VDP_AF_Data_H, ._32X_VDP_AF_Data_L, ._32X_VDP_State_H, ._32X_VDP_State_L
		dd	._32X_bad, ._32X_bad, ._32X_bad, ._32X_bad
	
	align 16
	
	._32X_VDP_Mode_H:
		mov	al, [__32X_VDP + vx.Mode + 1]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_Mode_L:
		mov	al, [__32X_VDP + vx.Mode + 0]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_Shift:
		mov	al, [__32X_VDP + vx.Mode + 2]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_Len_L:
		mov	al, [__32X_VDP + vx.AF_Len + 0]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_St_H:
		mov	al, [__32X_VDP + vx.AF_St + 1]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_St_L:
		mov	al, [__32X_VDP + vx.AF_St + 0]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_Data_H:
		mov	al, [__32X_VDP + vx.AF_Data + 1]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_Data_L:
		mov	al, [__32X_VDP + vx.AF_Data + 0]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_State_H:
		mov	al, [__32X_VDP + vx.State + 1]
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_State_L:
		mov	al, [__32X_VDP + vx.State]
		xor	al, 2
		mov	[__32X_VDP + vx.State], al
		pop	ebx
		ret
	
	align 4
	
	._32X_bad:
		xor	al, al
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Read_Byte_32X_FB0
		and	ebx, 0x1FFFF
		xor	ebx, byte 1
		mov	al, [__32X_VDP_Ram + ebx]
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Read_Byte_32X_FB1
		and	ebx, 0x1FFFF
		xor	ebx, byte 1
		mov	al, [__32X_VDP_Ram + ebx + 0x20000]
		pop	ebx
		ret
	
	; 32X extended Read Word
	; *******************************************
	
	align 64
	
	M68K_Read_Word_Bios_32X:
		and	ebx, 0xFE
		mov	ax, [_32X_Genesis_Rom + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_BiosR_32X:
		cmp	ebx, 0x100
		jae	short .Rom
		
		mov	ax, [_32X_Genesis_Rom + ebx]
		pop	ebx
		ret
	
	align 16
	
	.Rom:
		mov	ax, [Rom_Data + ebx]
		pop	ebx
		ret
	
	align 16

	M68K_Read_Word_Misc_32X:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_ram
		
		test	byte [Z80_State], 6
		jnz	near .bad
		
		push	ecx
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		call	[Z80_ReadB_Table + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_ram:
		cmp	ebx, 0xA11100
		jne	short .no_busreq
		
		test	byte [Z80_State], 2
		jnz	short .z80_on
	
	.z80_off:
		call	_main68k_readOdometer
		sub	eax, [_Last_BUS_REQ_Cnt]
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja	short .bus_taken

		mov	al, [Fake_Fetch]
		mov	ah, [_Last_BUS_REQ_St]
		xor	al, 0xFF
		add	ah, 0x80
		mov	[Fake_Fetch], al		; fake the next fetched instruction (random)
		pop	ebx
		ret
	
	align 16
	
	.bus_taken:
		mov	al, [Fake_Fetch]
		mov	ah, 0x80
		xor	al, 0xFF
		pop	ebx
		mov	[Fake_Fetch], al		; fake the next fetched instruction (random)
		ret
	
	align 16
	
	.z80_on:
		mov	al, [Fake_Fetch]
		mov	ah, 0x81
		xor	al, 0xFF
		pop	ebx
		mov	[Fake_Fetch], al		; fake the next fetched instruction (random)
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA15100
		jae	near ._32X_Reg
		
		cmp	ebx, 0xA130EC
		jae	short ._32X_ID
		
		cmp	ebx, 0xA1000F
		ja	short .bad
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_RW + ebx * 2]
	
	align 16
	
	._32X_ID:
		and ebx, 3
		mov ax, [_32X_ID_Tab + ebx]
		pop ebx
		ret
	
	align 16
	
	.Table_IO_RW:
		dd	.MD_Spec, .Pad_1, .Pad_2, .Ser
		dd	.CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad
	
	align 16
	
	.MD_Spec:
		mov	eax, [Game_Mode]
		add	eax, eax
		or	eax, [CPU_Mode]
		shl	eax, 6
		pop	ebx
		or	eax, [Gen_Version]		; on recupere les infos hardware de la machine
		ret
	
	align 8
	
	.Pad_1:
		call	_RD_Controller_1
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		call	_RD_Controller_2
		pop	ebx
		ret
	
	align 8
	
	.Ser:
		mov	ax, 0xFF00
		pop	ebx
		ret
	
	align 8
	
	.bad:
		xor	eax, eax
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	eax, [_Controller_1_COM]
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	eax, [_Controller_2_COM]
		pop	ebx
		ret
	
	align 8
	
	.CT_Ser:
		xor	eax, eax
		pop	ebx
		ret
	
	align 16
	
	._32X_Reg:
		cmp	ebx, 0xA15180
		jae	near ._32X_VDP_Reg
		
		and	ebx, 0x3E
		jmp	[.Table_32X_Reg + ebx * 2]
	
	align 16

	.Table_32X_Reg:
		dd	._32X_ACR,	._32X_INT,	._32X_Bank,	._32X_DREQ_C	; 00-07
		dd	._32X_DREQ_Src_H,	._32X_DREQ_Src_L,			; 08-0B
		dd	._32X_DREQ_Dest_H,	._32X_DREQ_Dest_L,			; 0C-0F
		
		dd	._32X_DREQ_Len,	._32X_FIFO,	._32X_bad,	._32X_bad	; 10-17
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 18-1F
		
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm	; 20
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		
		dd	._32X_PWM_Cont,		._32X_PWM_Cycle
		dd	._32X_PWM_Pulse_L,	._32X_PWM_Pulse_R
		dd	._32X_PWM_Pulse_C,	._32X_bad
		dd	._32X_bad,		._32X_bad
	
	align 16
	
	._32X_ACR:
		mov	al, [_32X_ADEN]
		mov	ah, [_32X_RES]
		or	al, ah
		mov	ah, [_32X_FM]
		or	al, 0x80
		pop	ebx
		ret
	
	align 8
	
	._32X_INT:
		xor	ax, ax
		pop	ebx
		ret
	
	align 16
	
	._32X_Bank:
		mov	al, [Bank_SH2]
		xor	ah, ah
		pop	ebx
		ret
	
	align 16
	
	._32X_DREQ_C:
		mov	bl, [_32X_DREQ_ST + 0]
		mov	ah, [_32X_DREQ_ST + 1]
		mov	al, [_32X_RV]
		and	ah, 0x80
		or	al, bl
		or	al, ah
		pop	ebx
		xor	ah, ah
		ret
	
	align 8
	
	._32X_DREQ_Src_H:
		mov	ax, [_32X_DREQ_SRC + 2]
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Src_L:
		mov	ax, [_32X_DREQ_SRC]
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Dest_H:
		mov	ax, [_32X_DREQ_DST + 2]
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Dest_L:
		mov	ax, [_32X_DREQ_DST]
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Len:
		mov	ax, [_32X_DREQ_LEN]
		pop	ebx
		ret
	
	align 8
	
	._32X_FIFO:
		pop	ebx
		ret
	
	align 16
	
	._32X_Comm:
		mov	ah, [_32X_Comm + ebx - 0x20 + 0]
		mov	al, [_32X_Comm + ebx - 0x20 + 1]
		pop	ebx
		ret
	
	align 8
	
	._32X_PWM_Cont:
		mov	ax, [_PWM_Mode]
		pop	ebx
		ret 
	
	align 8
	
	._32X_PWM_Cycle:
		mov	ax, [_PWM_Cycle_Tmp]
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_L:
		mov	ebx, [_PWM_RP_L]
		mov	eax, [_PWM_WP_L]
		mov	 ah, [_PWM_FULL_TAB + ebx * PWM_BUF_SIZE + eax]
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_R:
		mov	ebx, [_PWM_RP_R]
		mov	eax, [_PWM_WP_R]
		mov	ah, [_PWM_FULL_TAB + ebx * PWM_BUF_SIZE + eax]
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_C:
		mov	ebx, [_PWM_RP_L]
		mov	eax, [_PWM_WP_L]
		mov	ah, [_PWM_FULL_TAB + ebx * PWM_BUF_SIZE + eax]
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_Reg:
		test	byte [_32X_FM], 0xFF
		jnz	near ._32X_bad
		cmp	ebx, 0xA15200
		jae	near ._32X_CRAM
		
		and	ebx, 0xE
		jmp	[.Table_32X_VDP_Reg + ebx * 2]
	
	align 16
	
	.Table_32X_VDP_Reg:
		; VDP REG
		dd	._32X_VDP_Mode, ._32X_VDP_Shift, ._32X_VDP_AF_Len, ._32X_VDP_AF_St
		dd	._32X_VDP_AF_Data, ._32X_VDP_State, ._32X_bad, ._32X_bad
	
	align 16
	
	._32X_VDP_Mode:
		mov	ax, [__32X_VDP + vx.Mode]
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_Shift:
		mov	al, [__32X_VDP + vx.Mode + 2]
		xor	ah, ah
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_AF_Len:
		mov	al, [__32X_VDP + vx.AF_Len]
		xor	ah, ah
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_St:
		mov	ax, [__32X_VDP + vx.AF_St]
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_Data:
		mov	ax, [__32X_VDP + vx.AF_Data]
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_State:
		mov	ax, [__32X_VDP + vx.State]
		xor	ax, byte 2
		mov	[__32X_VDP + vx.State], ax
		pop	ebx
		ret
	
	align 4
	
	._32X_bad:
		xor	ax, ax
		pop	ebx
		ret
	
	align 16
	
	._32X_CRAM:
		cmp	ebx, 0xA15400
		jae	short ._32X_bad
		
		mov	ax, [__32X_VDP_CRam + ebx - 0xA15200]
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Read_Word_32X_FB0
		and	ebx, 0x1FFFE
		mov	ax, [__32X_VDP_Ram + ebx]
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Read_Word_32X_FB1
		and	ebx, 0x1FFFE
		mov	ax, [__32X_VDP_Ram + ebx + 0x20000]
		pop	ebx
		ret
	
	; 32X extended Write Byte
	; *******************************************
	
	align 64
	
	M68K_Write_Byte_Misc_32X:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_mem
		
		test	byte [Z80_State], 6
		jnz	short .bad
		
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		mov	edx, eax
		call	[Z80_WriteB_Table + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_mem:
		cmp	ebx, 0xA11100
		jne	near .no_busreq
		
		xor	ecx, ecx
		mov	ah, [Z80_State]
		mov	dword [_Controller_1_Counter], ecx
		test	al, 1
		mov	dword [_Controller_1_Delay], ecx
		mov	dword [_Controller_2_Counter], ecx
		mov	dword [_Controller_2_Delay], ecx
		jnz	short .deactivated
		
		test	ah, 2
		jnz	short .already_activated
		
		or	ah, 2
		push	edx
		mov	[Z80_State], ah
		mov	ebx, [Cycles_M68K]
		call	_main68k_readOdometer
		sub	ebx, eax
		mov	edx, [Cycles_Z80]
		mov	ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov	ecx, _M_Z80
		sub	edx, ebx
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	z80_Set_Odo
		pop	edx
	
	.already_activated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.deactivated:
		call	_main68k_readOdometer
		mov	cl, [Z80_State]
		mov	[_Last_BUS_REQ_Cnt], eax
		test	cl, 2
		setnz	[_Last_BUS_REQ_St]
		jz	short .already_deactivated
		
		push	edx
		mov	ebx, [Cycles_M68K]
		and	cl, ~2
		sub	ebx, eax
		mov	[Z80_State], cl
		mov	edx, [Cycles_Z80]
		mov	ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov	ecx, _M_Z80
		sub	edx, ebx
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov eax, ecx
%endif
		call	z80_Exec
		pop	edx
	
	.already_deactivated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA11200
		jne	short .no_reset_z80
		
		test	al, 1
		jnz	short .no_reset
		
		push	edx
		mov	ecx, _M_Z80
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	z80_Reset
		or	byte [Z80_State], 4
		call	_YM2612_Reset
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	.no_reset:
		and	byte [Z80_State], ~4
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_reset_z80:
		cmp	ebx, 0xA15100
		jae	near ._32X_Reg
		
		cmp	ebx, 0xA130F0
		jae	short .Genesis_Bank
		
		cmp	ebx, 0xA1000F
		ja	near .bad
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_WB + ebx * 2]
	
	align 16
	
	.Table_IO_WB:
		dd	.bad, .Pad_1, .Pad_2, .bad
		dd	.CT_Pad_1, .CT_Pad_2, .bad, .bad
	
	align 16
	
	.Pad_1:
		push	eax
		call	_WR_Controller_1
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		push	eax
		call	_WR_Controller_2
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	[_Controller_1_COM], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	[_Controller_2_COM], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Genesis_Bank:
		cmp	ebx, 0xA130F2
		jb	short .bank_0
		cmp	ebx, 0xA130FF
		ja	near .bad
		
		and	ebx, 0xF
		and	eax, 0x1F
		shr	ebx, 1
		mov	ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
		mov	[M68K_Read_Byte_Table + ebx * 4], ecx
		mov	ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
		mov	[M68K_Read_Word_Table + ebx * 4], ecx
		
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.bank_0:
		test	al, 1
		setnz	[SRAM_ON]
		test	al, 2
		setz	[SRAM_Write]
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Reg:
		cmp	ebx, 0xA15180
		jae	near ._32X_VDP_Reg
		
		;pushad
		;push	eax
		;push	ebx
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		and	ebx, 0x3F
		jmp	[.Table_32X_Reg + ebx * 4]
	
	align 16
	
	.Table_32X_Reg:
		dd	._32X_ACR_H,	._32X_ACR_L,	._32X_bad,	._32X_Int	; 00-03
		dd	._32X_bad,	._32X_Bank,	._32X_bad,	._32X_DREQ_C	; 04-07
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 08-0B
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 0C-0F
		
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 10-13
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 14-17
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 18-1B
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 1C-1F
		
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm	; 20
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		
		dd	._32X_bad,		._32X_PWM_Cont_L
		dd	._32X_PWM_Cycle_H,	._32X_PWM_Cycle_L
		dd	._32X_PWM_Pulse_L_H,	._32X_PWM_Pulse_L_L
		dd	._32X_PWM_Pulse_R_H,	._32X_PWM_Pulse_R_L
		dd	._32X_PWM_Pulse_C_H,	._32X_PWM_Pulse_C_L
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad
		dd	._32X_bad,	._32X_bad
	
	align 16
	
	._32X_ACR_H:
		and	al, 0x80
		mov	ah, [_32X_FM]
		pop	ecx
		xor	ah, al
		pop	ebx
		mov	[_32X_FM], al
		jnz	near __32X_Set_FB
		ret
	
	align 16
	
	._32X_ACR_L:
		mov	ah, [_32X_RES]
		mov	bl, al
		and	al, 2
		mov	[_32X_RES], al
		cmp	ax, byte 2
		jne	short .no_SH2_reset
		
		push	edx
		mov	ecx, M_SH2
		mov	edx, 1
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Reset
		mov	ecx, S_SH2
		mov	edx, 1
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Reset
		pop	edx
	
	.no_SH2_reset:
		mov	al, [_32X_ADEN]
		and	bl, 1
		xor	al, bl
		jz	short .no_32X_change
		
		mov	[_32X_ADEN], bl
		call	_M68K_32X_Mode
	
	.no_32X_change:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Int:
		mov	bl, al
		mov	ah, [_32X_MINT]
		add	al, al
		and	bl, 2
		and	al, 2
		mov	bh, [_32X_SINT]
		test	ah, al
		push	edx
		jz	short .no_MINT
		
		mov	edx, 8
		mov	ecx, M_SH2
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Interrupt
	
	.no_MINT:
		test	bh, bl
		jz	short .no_SINT
		
		mov	edx, 8
		mov	ecx, S_SH2
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Interrupt
	
	.no_SINT:
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Bank:
		and	al, 3
		mov	[Bank_SH2], al
		call	_M68K_Set_32X_Rom_Bank
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_DREQ_C:
		mov	bl, al
		mov	ah, [_32X_RV]
		and	al, 1
		mov	bh, [_32X_DREQ_ST]
		and	bl, 4
		xor	ah, al
		jz	short .RV_not_changed
		
		mov	[_32X_RV], al
		call	_M68K_32X_Mode
	
	.RV_not_changed:
		cmp	bx, 0x0004
		jne	short .No_DREQ
		
		xor	al, al
		mov	byte [_32X_DREQ_ST + 1], 0x40
		mov	[_32X_FIFO_Block], al
		mov	[_32X_FIFO_Read], al
		mov	[_32X_FIFO_Write], al
	
	.No_DREQ:
		mov	[_32X_DREQ_ST], bl
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Comm:
		mov	[_32X_Comm + ebx - 0x20], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Cont_L:
		mov	[_PWM_Mode + 0], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Cycle_H:
		mov	cl, [_PWM_Cycle_Tmp + 0]
		mov	[_PWM_Cycle_Tmp + 1], al
		mov	ch, al
		push	ecx
		call	_PWM_Set_Cycle
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Cycle_L:
		mov	ch, [_PWM_Cycle_Tmp + 1]
		mov	[_PWM_Cycle_Tmp + 0], al
		mov	cl, al
		push	ecx
		call	_PWM_Set_Cycle
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_L_H:
		mov	[_PWM_FIFO_L_Tmp + 1], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_L_L:
		mov	ecx, [_PWM_RP_L]
		mov	ebx, [_PWM_WP_L]
		mov	ah, [_PWM_FIFO_L_Tmp + 1]
		test	byte [_PWM_FULL_TAB + ecx * PWM_BUF_SIZE + ebx], 0x80
		jnz	short ._32X_PWM_Pulse_L_full
		
		mov	[_PWM_FIFO_L + ebx * 2], ax
		inc	ebx
		and	ebx, byte (PWM_BUF_SIZE - 1)
		mov	[_PWM_WP_L], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_PWM_Pulse_L_full:
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_PWM_Pulse_R_H:
		mov	[_PWM_FIFO_R_Tmp + 1], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_R_L:
		mov	ecx, [_PWM_RP_R]
		mov	ebx, [_PWM_WP_R]
		mov	ah, [_PWM_FIFO_R_Tmp + 1]
		test	byte [_PWM_FULL_TAB + ecx * PWM_BUF_SIZE + ebx], 0x80
		jnz	short ._32X_PWM_Pulse_R_full
		
		mov	[_PWM_FIFO_R + ebx * 2], ax
		inc	ebx
		and	ebx, byte (PWM_BUF_SIZE - 1)
		mov	[_PWM_WP_R], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_PWM_Pulse_R_full:
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_PWM_Pulse_C_H:
		mov	[_PWM_FIFO_L_Tmp + 1], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_C_L:
		mov	ecx, [_PWM_RP_L]
		mov	ebx, [_PWM_WP_L]
		mov	ah, [_PWM_FIFO_L_Tmp + 1]
		test	byte [_PWM_FULL_TAB + ecx * PWM_BUF_SIZE + ebx], 0x80
		jnz	short ._32X_PWM_Pulse_C_full
		
		mov	[_PWM_FIFO_L + ebx * 2], ax
		mov	[_PWM_FIFO_R + ebx * 2], ax
		inc	ebx
		and	ebx, byte (PWM_BUF_SIZE - 1)
		mov	[_PWM_WP_L], ebx
		mov	[_PWM_WP_R], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_PWM_Pulse_C_full:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_Reg:
		test	byte [_32X_FM], 0xFF
		jnz	near ._32X_bad
		cmp	ebx, 0xA15200
		jae	near ._32X_bad
		
		;pushad
		;push	eax
		;push	ebx
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		and	ebx, 0xF
		jmp	[.Table_32X_VDP_Reg + ebx * 4]
	
	align 16
	
	.Table_32X_VDP_Reg:
		dd	._32X_bad,	._32X_VDP_Mode,		._32X_bad,	._32X_VDP_Shift
		dd	._32X_bad,	._32X_VDP_AF_Len,	._32X_bad,	._32X_bad
		dd	._32X_bad,	._32X_bad,		._32X_bad,	._32X_VDP_State
		dd	._32X_bad,	._32X_bad,		._32X_bad,	._32X_bad
	
	align 16
	
	._32X_VDP_Mode:
		mov	[__32X_VDP + vx.Mode], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_Shift:
		mov	[__32X_VDP + vx.Mode + 2], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_Len:
		mov	[__32X_VDP + vx.AF_Len], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_State:
		mov	bh, [__32X_VDP + vx.Mode + 0]
		mov	bl, [__32X_VDP + vx.State + 1]
		test	bh, 3
		mov	[__32X_VDP + vx.State + 2], al
		jz	short ._32X_VDP_blank
		
		test	bl, bl
		jns	short ._32X_VDP_State_nvb
	
	._32X_VDP_blank:
		mov	[__32X_VDP + vx.State + 0], al
		call	__32X_Set_FB
	
	._32X_VDP_State_nvb:
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_bad:
		pop	ecx
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Write_Byte_32X_FB0
		and	ebx, 0x1FFFF
		test	al, al
		jz	short .blank
		
		xor	ebx, byte 1
		mov	[__32X_VDP_Ram + ebx], al
	
	.blank:
		pop	ecx
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Write_Byte_32X_FB1
		and	ebx, 0x1FFFF
		test	al, al
		jz	short .blank
		
		xor	ebx, byte 1
		mov	[__32X_VDP_Ram + ebx + 0x20000], al
	
	.blank:
		pop	ecx
		pop	ebx
		ret
	
	; 32X extended Write Word
	; *******************************************
	
	align 64
	
	M68K_Write_Word_Misc_32X:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_ram
		
		test	byte [Z80_State], 6
		jnz	near .bad
		
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		mov	dh, al
		shr	ebx, 10
		mov	dl, al
		call	[Z80_WriteB_Table + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_ram:
		cmp	ebx, 0xA11100
		jne	near .no_busreq
		
		xor	ecx, ecx
		mov	al, [Z80_State]
		mov	dword [_Controller_1_Counter], ecx
		test	ah, 1
		mov	dword [_Controller_1_Delay], ecx
		mov	dword [_Controller_2_Counter], ecx
		mov	dword [_Controller_2_Delay], ecx
		jnz	short .deactivated
		
		test	al, 2
		jnz	short .already_activated
		
		or	al, 2
		push	edx
		mov	[Z80_State], al
		mov	ebx, [Cycles_M68K]
		call	_main68k_readOdometer
		sub	ebx, eax
		mov	edx, [Cycles_Z80]
		mov	ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov	ecx, _M_Z80
		sub	edx, ebx
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	z80_Set_Odo
		pop	edx
	
	.already_activated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.deactivated:
		call	_main68k_readOdometer
		mov	cl, [Z80_State]
		mov	[_Last_BUS_REQ_Cnt], eax
		test	cl, 2
		setnz	[_Last_BUS_REQ_St]
		jz	short .already_deactivated
		
		push	edx
		mov	ebx, [Cycles_M68K]
		and	cl, ~2
		sub	ebx, eax
		mov	[Z80_State], cl
		mov	edx, [Cycles_Z80]
		mov	ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov	ecx, _M_Z80
		sub	edx, ebx
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	z80_Exec
		pop	edx
	
	.already_deactivated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA11200
		jne	short .no_reset_z80
		
		test	ah, 1
		jnz	short .no_reset
		
		push	edx
		mov	ecx, _M_Z80
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	z80_Reset
		or	byte [Z80_State], 4
		call	_YM2612_Reset
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	.no_reset:
		and	byte [Z80_State], ~4
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_reset_z80:
		cmp	ebx, 0xA15100
		jae	near ._32X_Reg
		
		cmp	ebx, 0xA130F0
		jae	short .Genesis_Bank
		
		cmp	ebx, 0xA1000F
		ja	short .bad
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_WW + ebx * 2]
	
	align 16
	
	.Table_IO_WW:
		dd	.bad, .Pad_1, .Pad_2, .bad
		dd	.CT_Pad_1, .CT_Pad_2, .bad, .bad
	
	align 16
	
	.Pad_1:
		push	eax
		call	_WR_Controller_1
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		push	eax
		call	_WR_Controller_2
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	[_Controller_1_COM], ax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	[_Controller_2_COM], ax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Genesis_Bank:
		cmp	ebx, 0xA130F2
		jb	short .bank_0
		cmp	ebx, 0xA130FF
		ja	near .bad
		
		;and	ebx, 0xF
		;and	eax, 0x1F
		;shr	ebx, 1
		;mov	ecx, [Genesis_M68K_Read_Byte_Table + eax * 4]
		;mov	[M68K_Read_Byte_Table + ebx * 4], ecx
		;mov	ecx, [Genesis_M68K_Read_Word_Table + eax * 4]
		;mov	[M68K_Read_Word_Table + ebx * 4], ecx
		
		pop	ecx
		pop	ebx
		ret
	
	align 16

	.bank_0:
		test	al, 1
		setnz	[SRAM_ON]
		test	al, 2
		setz	[SRAM_Write]
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Reg:
		cmp	ebx, 0xA15180
		jae	near ._32X_VDP_Reg
		
		;pushad
		;push	eax
		;push	ebx
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		and	ebx, 0x3E
		jmp	[.Table_32X_Reg + ebx * 2]
	
	align 16
	
	.Table_32X_Reg
		dd	._32X_ACR,	._32X_INT,	._32X_Bank,	._32X_DREQ_C	; 00-07
		dd	._32X_DREQ_Src_H,	._32X_DREQ_Src_L,			; 08-0B
		dd	._32X_DREQ_Dest_H,	._32X_DREQ_Dest_L,			; 0C-0F
		
		dd	._32X_DREQ_Len,	._32X_FIFO,	._32X_bad,	._32X_bad	; 10-17
		dd	._32X_bad,	._32X_bad,	._32X_bad,	._32X_bad	; 18-1F
		
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm	; 20
		dd	._32X_Comm,	._32X_Comm,	._32X_Comm,	._32X_Comm
		
		dd	._32X_PWM_Cont,		._32X_PWM_Cycle
		dd	._32X_PWM_Pulse_L,	._32X_PWM_Pulse_R
		dd	._32X_PWM_Pulse_C,	._32X_bad
		dd	._32X_bad,		._32X_bad
	
	align 16
	
	._32X_ACR:
		mov	bh, [_32X_FM]
		and	ah, 0x80
		mov	bl, al
		xor	bh, ah
		mov	[_32X_FM], ah
		jz	short .no_update_FB
		
		call	__32X_Set_FB
	
	.no_update_FB:
		mov	al, bl
		mov	ah, [_32X_RES]
		and	al, 2
		mov	[_32X_RES], al
		cmp	ax, byte 2
		jne	short .no_SH2_reset
		
		push	edx
		mov	ecx, M_SH2
		mov	edx, 1
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Reset
		mov	ecx, S_SH2
		mov	edx, 1
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Reset
		pop	edx
	
	.no_SH2_reset:
		mov	al, [_32X_ADEN]
		and	bl, 1
		xor	al, bl
		jz	short .no_32X_change
		
		mov	[_32X_ADEN], bl
		call	_M68K_32X_Mode
	
	.no_32X_change:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_INT:
		mov	bl, al
		mov	ah, [_32X_MINT]
		add	al, al
		and	bl, 2
		and	al, 2
		mov	bh, [_32X_SINT]
		test	ah, al
		push	edx
		jz	short .no_MINT
		
		mov	edx, 8
		mov	ecx, M_SH2
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Interrupt
	
	.no_MINT:
		test	bh, bl
		jz	short .no_SINT
		
		mov	edx, 8
		mov	ecx, S_SH2
%ifdef __GCC2
		; TODO: This is a fastcall function.
		; Convert to standard cdecl.
		mov	eax, ecx
%endif
		call	SH2_Interrupt
	
	.no_SINT:
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Bank:
		and	al, 3
		mov	[Bank_SH2], al
		call	_M68K_Set_32X_Rom_Bank
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_DREQ_C:
		mov	bl, al
		mov	ah, [_32X_RV]
		and	al, 1
		mov	bh, [_32X_DREQ_ST]
		and	bl, 4
		xor	ah, al
		jz	short .RV_not_changed
		
		mov	[_32X_RV], al
		call	_M68K_32X_Mode
	
	.RV_not_changed:
		cmp	bx, 0x0004
		jne	short .No_DREQ
		
		xor	al, al
		mov	byte [_32X_DREQ_ST + 1], 0x40
		mov	[_32X_FIFO_Block], al
		mov	[_32X_FIFO_Read], al
		mov	[_32X_FIFO_Write], al
	
	.No_DREQ:
		mov	[_32X_DREQ_ST], bl
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Src_H:
		mov	[_32X_DREQ_SRC + 2], al
		pop	ecx
		pop	ebx
		ret
	
	align 8

	._32X_DREQ_Src_L:
		and	ax, byte ~1
		pop	ecx
		mov	[_32X_DREQ_SRC], ax
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Dest_H:
		mov	[_32X_DREQ_DST + 2], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Dest_L:
		mov	[_32X_DREQ_DST], ax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_DREQ_Len:
		and	ax, byte ~3
		pop	ecx
		mov	[_32X_DREQ_LEN], ax
		pop	ebx
		ret
	
	align 16
	
	._32X_FIFO:
		mov	cx, [_32X_DREQ_ST]
		mov	ebx, [_32X_FIFO_Write]
		and	cx, 0x8004
		cmp	cx, 0x0004
		mov	ecx, [_32X_FIFO_Block]
		jne	short ._32X_FIFO_End
		
		;pushad
		;push	eax
		;lea	eax, [0xA15500 + ecx + ebx * 2]
		;push	eax
		;call	_Write_To_68K_Space
		;pop	eax
		;pop	eax
		;popad
		
		mov	[_32X_FIFO_A + ecx + ebx * 2], ax
		inc	ebx
		cmp	ebx, 4
		jae	short ._32X_FIFO_Full_A
		
		mov	[_32X_FIFO_Write], ebx
	
	._32X_FIFO_End:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_FIFO_Full_A:
		mov	bl, [_32X_DREQ_ST + 1]
		push	edx
		test	bl, 0x40
		jz	short ._32X_FIFO_Full_B
		
		xor	al, al
		xor	ecx, byte (4 * 2)
		mov	[_32X_DREQ_ST + 1], al
		mov	[_32X_FIFO_Write], al
		mov	[_32X_FIFO_Read], al
		mov	[_32X_FIFO_Block], ecx
		
		mov	dl, 1
		mov	ecx, M_SH2
		call	SH2_DMA0_Request
		
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_FIFO_Full_B:
		mov	byte [_32X_DREQ_ST + 1], 0x80
		
		mov	dl, 1
		mov	ecx, M_SH2
		call	SH2_DMA0_Request
		
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_Comm:
		mov	[_32X_Comm + ebx - 0x20 + 0], ah
		mov	[_32X_Comm + ebx - 0x20 + 1], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Cont:
		and	al, 0x0F
		pop	ecx
		mov	[_PWM_Mode], al
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Cycle:
		push	eax
		call	_PWM_Set_Cycle
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_L:
		mov	ecx, [_PWM_RP_L]
		mov	ebx, [_PWM_WP_L]
		test	byte [_PWM_FULL_TAB + ecx * PWM_BUF_SIZE + ebx], 0x80
		jnz	short ._32X_PWM_Pulse_L_full
		
		mov	[_PWM_FIFO_L + ebx * 2], ax
		inc	ebx
		and	ebx, byte (PWM_BUF_SIZE - 1)
		mov	[_PWM_WP_L], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_PWM_Pulse_L_full:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_R:
		mov	ecx, [_PWM_RP_R]
		mov	ebx, [_PWM_WP_R]
		test	byte [_PWM_FULL_TAB + ecx * PWM_BUF_SIZE + ebx], 0x80
		jnz	short ._32X_PWM_Pulse_R_full
		
		mov	[_PWM_FIFO_R + ebx * 2], ax
		inc	ebx
		and	ebx, byte (PWM_BUF_SIZE - 1)
		mov	[_PWM_WP_R], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_PWM_Pulse_R_full:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_PWM_Pulse_C:
		mov	ecx, [_PWM_RP_L]
		mov	ebx, [_PWM_WP_L]
		test	byte [_PWM_FULL_TAB + ecx * PWM_BUF_SIZE + ebx], 0x80
		jnz	short ._32X_PWM_Pulse_C_full
		
		mov	[_PWM_FIFO_L + ebx * 2], ax
		mov	[_PWM_FIFO_R + ebx * 2], ax
		inc	ebx
		and	ebx, byte (PWM_BUF_SIZE - 1)
		mov	[_PWM_WP_L], ebx
		mov	[_PWM_WP_R], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_PWM_Pulse_C_full:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_Reg:
		test	byte [_32X_FM], 0xFF
		jnz	near ._32X_bad
		cmp	ebx, 0xA15200
		jae	near ._32X_CRAM
		
		;pushad
		;push	eax
		;push	ebx
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		and	ebx, 0xE
		jmp	[.Table_32X_VDP_Reg + ebx * 2]
	
	align 16
	
	.Table_32X_VDP_Reg:
		dd	._32X_VDP_Mode, ._32X_VDP_Shift, ._32X_VDP_AF_Len, ._32X_VDP_AF_St
		dd	._32X_VDP_AF_Data, ._32X_VDP_State, ._32X_bad, ._32X_bad
	
	align 16
	
	._32X_VDP_Mode:
		mov	[__32X_VDP + vx.Mode], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_Shift:
		mov	[__32X_VDP + vx.Mode + 2], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_Len:
		mov	[__32X_VDP + vx.AF_Len], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	._32X_VDP_AF_St:
		mov	[__32X_VDP + vx.AF_St], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_AF_Data:
		push	edi
		mov	[__32X_VDP + vx.AF_Data], ax
		mov	bx, ax
		mov	edi, [__32X_VDP + vx.State]
		shl	eax, 16
		and	edi, byte 1
		mov	ax, bx
		xor	edi, byte 1
		mov	ebx, [__32X_VDP + vx.AF_St]
		mov	ecx, [__32X_VDP + vx.AF_Len]
		shl	edi, 17
		inc	ecx
		shr	ecx, 1
		lea	edi, [edi + __32X_VDP_Ram]
		jz	short .Spec_Fill
		jnc	short .Loop
		
		mov	[edi + ebx * 2], ax
		inc	bl
		jmp	short .Loop
	
	align 16
	
	.Loop:
			mov	[edi + ebx * 2], eax
			add	bl, byte 2
			dec	ecx
			jns	short .Loop
		
		mov	[__32X_VDP + vx.AF_St], ebx
		pop	edi
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Spec_Fill:
		mov	[edi + ebx * 2], ax
		inc	bl
		pop	edi
		mov	[__32X_VDP + vx.AF_St], ebx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_VDP_State:
		mov	bh, [__32X_VDP + vx.Mode + 0]
		mov	bl, [__32X_VDP + vx.State + 1]
		test	bh, 3
		mov	[__32X_VDP + vx.State + 2], al
		jz	short ._32X_VDP_blank
		
		test	bl, bl
		jns	short ._32X_VDP_State_nvb
	
	._32X_VDP_blank:
		mov	[__32X_VDP + vx.State + 0], al
		call	__32X_Set_FB
	
	._32X_VDP_State_nvb:
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	._32X_bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	._32X_CRAM:
		cmp	ebx, 0xA15400
		jae	short ._32X_bad
		
		push	edx
		
		and	eax, 0xFFFF
		mov	cx, [__32X_Palette_16B + eax * 2]
		mov	edx, [__32X_Palette_32B + eax * 4]
		mov	[__32X_VDP_CRam + ebx - 0xA15200], ax
		mov	[__32X_VDP_CRam_Ajusted + ebx - 0xA15200], cx
		mov	[__32X_VDP_CRam_Ajusted32 + (ebx - 0xA15200) * 2], edx
		
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Write_Word_32X_FB0
		and	ebx, 0x3FFFE
		test	ebx, 0x20000
		jnz	short .overwrite
		
		mov	[__32X_VDP_Ram + ebx], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.overwrite:
		test	al, al
		jz	short .blank1
		
		mov	[__32X_VDP_Ram + ebx - 0x20000 + 0], al
	
	.blank1:
		test	ah, ah
		jz	short .blank2
		
		mov	[__32X_VDP_Ram + ebx - 0x20000 + 1], ah
	
	.blank2:
		pop	ecx
		pop	ebx
		ret
	
	align 64
	
	DECL M68K_Write_Word_32X_FB1
		and	ebx, 0x3FFFE
		test	ebx, 0x20000
		jnz	short .overwrite
		
		mov	[__32X_VDP_Ram + ebx + 0x20000], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.overwrite:
		test	al, al
		jz	short .blank1
		
		mov	[__32X_VDP_Ram + ebx - 0x20000 + 0x20000 + 0], al
	
	.blank1:
		test	ah, ah
		jz	short .blank2
		
		mov	[__32X_VDP_Ram + ebx - 0x20000 + 0x20000 + 1], ah
	
	.blank2:
		pop	ecx
		pop	ebx
		ret
