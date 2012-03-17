%include "nasmhead.inc"


section .bss align=64

	DECL Ram_Z80
	resb (8 * 1024)

	DECL Bank_Z80
	resd 1
 
section .text align=64

	; External symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_Read_VDP_Status	Read_VDP_Status
		%define	_Read_VDP_H_Counter	Read_VDP_H_Counter
		%define	_Read_VDP_V_Counter	Read_VDP_V_Counter
		%define	_Write_Byte_VDP_Data	Write_Byte_VDP_Data
		%define	_Write_Word_VDP_Data	Write_Word_VDP_Data
	%endif
	
	extern _Read_VDP_Status
	extern _Read_VDP_V_Counter
	extern _Read_VDP_H_Counter
	extern _Write_Byte_VDP_Data
	extern _Write_Word_VDP_Data
	
	extern M68K_RB
	extern M68K_WB
	extern _YM2612_Write
	extern _YM2612_Read
	extern _PSG_Write

	extern _Write_To_Bank
	extern _Read_To_68K_Space
	extern _Write_To_68K_Space


	;unsigned char Z80_ReadB(unsigned int Adr);
	DECL Z80_ReadB

		mov eax, [esp + 4]
		push ecx
		push edx

		mov ecx, eax
		and eax, 0xF000
		and ecx, 0x7FFF
		shr eax, 10
		call [Z80_ReadB_Table + eax]
		pop edx
		pop ecx
		ret

	ALIGN4

	DECL Z80_ReadB_Table
			dd Z80_ReadB_Ram,		; 0x0000 - 0x0FFF
			dd Z80_ReadB_Ram,		; 0x1000 - 0x1FFF
			dd Z80_ReadB_Ram,		; 0x2000 - 0x2FFF
			dd Z80_ReadB_Ram,		; 0x3000 - 0x3FFF
			dd Z80_ReadB_YM2612,	; 0x4000 - 0x4FFF
			dd Z80_ReadB_YM2612,	; 0x5000 - 0x5FFF
			dd Z80_ReadB_Bank,		; 0x6000 - 0x6FFF
			dd Z80_ReadB_PSG,		; 0x7000 - 0x7FFF
			dd Z80_ReadB_68K_Ram,	; 0x8000 - 0x8FFF
			dd Z80_ReadB_68K_Ram,	; 0x9000 - 0x9FFF
			dd Z80_ReadB_68K_Ram,	; 0xA000 - 0xAFFF
			dd Z80_ReadB_68K_Ram,	; 0xB000 - 0xBFFF
			dd Z80_ReadB_68K_Ram,	; 0xC000 - 0xCFFF
			dd Z80_ReadB_68K_Ram,	; 0xD000 - 0xDFFF
			dd Z80_ReadB_68K_Ram,	; 0xE000 - 0xEFFF
			dd Z80_ReadB_68K_Ram	; 0xF000 - 0xFFFF


	ALIGN32
	
	;unsigned short Z80_ReadW(unsigned int Adr);
	DECL Z80_ReadW

		mov eax, [esp + 4]
		push ecx
		push edx

		mov ecx, eax
		and eax, 0xF000
		and ecx, 0x7FFF
		shr eax, 10
		call [Z80_ReadW_Table + eax]
		pop edx
		pop ecx
		ret

	ALIGN4

	DECL Z80_ReadW_Table
			dd Z80_ReadW_Ram,		; 0x0000 - 0x0FFF
			dd Z80_ReadW_Ram,		; 0x1000 - 0x1FFF
			dd Z80_ReadW_Ram,		; 0x2000 - 0x2FFF
			dd Z80_ReadW_Ram,		; 0x3000 - 0x3FFF
			dd Z80_ReadW_YM2612,	; 0x4000 - 0x4FFF
			dd Z80_ReadW_YM2612,	; 0x5000 - 0x5FFF
			dd Z80_ReadW_Bank,		; 0x6000 - 0x6FFF
			dd Z80_ReadW_PSG,		; 0x7000 - 0x7FFF
			dd Z80_ReadW_68K_Ram,	; 0x8000 - 0x8FFF
			dd Z80_ReadW_68K_Ram,	; 0x9000 - 0x9FFF
			dd Z80_ReadW_68K_Ram,	; 0xA000 - 0xAFFF
			dd Z80_ReadW_68K_Ram,	; 0xB000 - 0xBFFF
			dd Z80_ReadW_68K_Ram,	; 0xC000 - 0xCFFF
			dd Z80_ReadW_68K_Ram,	; 0xD000 - 0xDFFF
			dd Z80_ReadW_68K_Ram,	; 0xE000 - 0xEFFF
			dd Z80_ReadW_68K_Ram	; 0xF000 - 0xFFFF


	ALIGN32

	;void Z80_WriteB(unsigned int Adr, unsigned char Data);
	DECL Z80_WriteB

		push ecx
		push edx

		mov eax, [esp + 12]
		mov edx, [esp + 16]
		mov ecx, eax
		and eax, 0xF000
		and edx, 0xFF
		shr eax, 10
		and ecx, 0x7FFF
		call [Z80_WriteB_Table + eax]
		pop edx
		pop ecx
		ret

	ALIGN4

	DECL Z80_WriteB_Table
			dd Z80_WriteB_Ram,		; 0x0000 - 0x0FFF
			dd Z80_WriteB_Ram,		; 0x1000 - 0x1FFF
			dd Z80_WriteB_Ram,		; 0x2000 - 0x2FFF
			dd Z80_WriteB_Ram,		; 0x3000 - 0x3FFF
			dd Z80_WriteB_YM2612,	; 0x4000 - 0x4FFF
			dd Z80_WriteB_YM2612,	; 0x5000 - 0x5FFF
			dd Z80_WriteB_Bank,		; 0x6000 - 0x6FFF
			dd Z80_WriteB_PSG,		; 0x7000 - 0x7FFF
			dd Z80_WriteB_68K_Ram,	; 0x8000 - 0x8FFF
			dd Z80_WriteB_68K_Ram,	; 0x9000 - 0x9FFF
			dd Z80_WriteB_68K_Ram,	; 0xA000 - 0xAFFF
			dd Z80_WriteB_68K_Ram,	; 0xB000 - 0xBFFF
			dd Z80_WriteB_68K_Ram,	; 0xC000 - 0xCFFF
			dd Z80_WriteB_68K_Ram,	; 0xD000 - 0xDFFF
			dd Z80_WriteB_68K_Ram,	; 0xE000 - 0xEFFF
			dd Z80_WriteB_68K_Ram	; 0xF000 - 0xFFFF

	ALIGN32

	;void Z80_WriteW(unsigned int Adr, unsigned short Data);
	DECL Z80_WriteW

		push ecx
		push edx

		mov eax, [esp + 12]
		mov edx, [esp + 16]
		mov ecx, eax
		and eax, 0xF000
		and edx, 0xFF
		shr eax, 10
		and ecx, 0x7FFF
		call [Z80_WriteW_Table + eax]
		pop edx
		pop ecx
		ret

	ALIGN4

	DECL Z80_WriteW_Table
			dd Z80_WriteW_Ram,		; 0x0000 - 0x0FFF
			dd Z80_WriteW_Ram,		; 0x1000 - 0x1FFF
			dd Z80_WriteW_Ram,		; 0x2000 - 0x2FFF
			dd Z80_WriteW_Ram,		; 0x3000 - 0x3FFF
			dd Z80_WriteW_YM2612,	; 0x4000 - 0x4FFF
			dd Z80_WriteW_YM2612,	; 0x5000 - 0x5FFF
			dd Z80_WriteW_Bank,		; 0x6000 - 0x6FFF
			dd Z80_WriteW_PSG,		; 0x7000 - 0x7FFF
			dd Z80_WriteW_68K_Ram,	; 0x8000 - 0x8FFF
			dd Z80_WriteW_68K_Ram,	; 0x9000 - 0x9FFF
			dd Z80_WriteW_68K_Ram,	; 0xA000 - 0xAFFF
			dd Z80_WriteW_68K_Ram,	; 0xB000 - 0xBFFF
			dd Z80_WriteW_68K_Ram,	; 0xC000 - 0xCFFF
			dd Z80_WriteW_68K_Ram,	; 0xD000 - 0xDFFF
			dd Z80_WriteW_68K_Ram,	; 0xE000 - 0xEFFF
			dd Z80_WriteW_68K_Ram	; 0xF000 - 0xFFFF

	; Read Byte
	; ---------

	ALIGN4

	DECLF Z80_ReadB_Bad, 4
		mov al, 0
		ret

	ALIGN4
	
	DECLF Z80_ReadB_Ram, 4
		and ecx, 0x1FFF
		mov al, [Ram_Z80 + ecx]
		ret

	ALIGN4
	
	DECLF Z80_ReadB_Bank, 4
		mov al, 0
		ret

	ALIGN4

	DECLF Z80_ReadB_YM2612, 4
		and ecx, 0x3
		push ecx
		call _YM2612_Read
		pop ecx
		ret
		
	ALIGN4

	DECLF Z80_ReadB_PSG, 4
		cmp ecx, 0x7F04
		jb short .bad
		cmp ecx, 0x7F08
		jb short .vdp_status
		cmp ecx, 0x7F09
		ja short .bad

	.vdp_counter
		test ecx, 1
		jnz short .vdp_h_counter

	.vdp_v_counter:
		call	_Read_VDP_V_Counter
		ret

	ALIGN4
	
	.vdp_h_counter:
		call	_Read_VDP_H_Counter
		ret

	ALIGN4

	.bad
		mov al, 0
		ret

	ALIGN4
	
	.vdp_status:
		call	_Read_VDP_Status
		test	ecx, 1
		jnz	short .no_swap_status
		mov	al, ah

	.no_swap_status
		ret

	ALIGN4

	DECLF Z80_ReadB_68K_Ram, 4
		mov eax, [Bank_Z80]
		and ecx, 0x7FFF
		add ecx, eax
		push ecx
;		call _Read_To_68K_Space
		call M68K_RB
		pop ecx
		ret

	; Read Word
	; ---------

	ALIGN4

	DECLF Z80_ReadW_Bad, 4
		mov ax, 0
		ret

	ALIGN4
	
	DECLF Z80_ReadW_Ram, 4
		and ecx, 0x1FFF
		mov al, [Ram_Z80 + ecx + 0]
		mov ah, [Ram_Z80 + ecx + 1]
		ret

	ALIGN4
	
	DECLF Z80_ReadW_Bank, 4
		mov ax, 0
		ret

	ALIGN4

	DECLF Z80_ReadW_YM2612, 4
		and ecx, 0x3
		push ecx
		call _YM2612_Read
		xor ah, ah
		pop ecx
		ret
	
	align 16
	
	DECLF Z80_ReadW_PSG, 4
		cmp	ecx, 0x7F04
		jb	short .bad
		cmp	ecx, 0x7F08
		jb	short .vdp_status
		cmp	ecx, 0x7F09
		ja	short .bad
		
		call	_Read_VDP_V_Counter
		mov	cl, al
		call	_Read_VDP_H_Counter
		mov	ah, cl
		ret
	
	align 4
	
	.bad:
		mov	al, 0
		ret
	
	align 4
	
	.vdp_status:
		call	_Read_VDP_Status
		ret
	
	align 16
	
	DECLF Z80_ReadW_68K_Ram, 4
		mov eax, [Bank_Z80]
		and ecx, 0x7FFF
		add ecx, eax
		push ecx
		call M68K_RB
		inc ecx
		mov dl, al
		push ecx
		call M68K_RB
		mov ah, al
		add esp, 8
		mov al, dl
		ret

	; Write Byte
	; ----------

	ALIGN4

	DECLF Z80_WriteB_Bad, 8
	DECLF Z80_WriteW_Bad, 8
		ret

	ALIGN4
	
	DECLF Z80_WriteB_Ram, 8
		and ecx, 0x1FFF
		mov [Ram_Z80 + ecx], dl
		ret

	ALIGN4
	
	DECLF Z80_WriteB_Bank, 8
	DECLF Z80_WriteW_Bank, 8
		cmp ecx, 0x60FF
		ja short .bad

		mov ecx, [Bank_Z80]
		and edx, 1

;		push edx

		and ecx, 0xFF0000
		shl edx, 23
		shr ecx, 1
		add edx, ecx
		mov [Bank_Z80], edx

;		call _Write_To_Bank
;		pop ecx

	.bad
		ret

	ALIGN4

	DECLF Z80_WriteB_YM2612, 8
		and ecx, 0x3
		push edx
		push ecx
		call _YM2612_Write
		add esp, 8
		ret
	
	align 16
	
	DECLF Z80_WriteB_PSG, 8
		cmp	ecx, 0x7F11
		jne	short .other
		
		push	edx
		call	_PSG_Write
		pop	edx
		ret
	
	align 16
	
	.other
		cmp	ecx, 0x7F03
		ja	short .bad
		
		push	edx
		call	_Write_Byte_VDP_Data
		pop	edx
		
	.bad:
		ret

	align 16

	DECLF Z80_WriteB_68K_Ram, 8
		mov eax, [Bank_Z80]
		and ecx, 0x7FFF
		add ecx, eax
		push edx
		push ecx
;		call _Write_To_68K_Space
		call M68K_WB
		pop ecx
		pop edx
		ret


	; Write Word
	; ----------

	ALIGN4
	
	DECLF Z80_WriteW_Ram, 8
		and ecx, 0x1FFF
		mov [Ram_Z80 + ecx + 0], dl
		mov [Ram_Z80 + ecx + 1], dh
		ret

	ALIGN4

	DECLF Z80_WriteW_YM2612, 8
		and ecx, 0x3
		push edx
		push ecx
		call _YM2612_Write
		inc ecx
		push edx
		push ecx
		call _YM2612_Write
		add esp, 16
		ret
	
	align 16
	
	DECLF Z80_WriteW_PSG, 8
		cmp	ecx, 0x7F11
		jne	short .other
		
		push	edx
		call	_PSG_Write
		pop	edx
		ret
	
	align 16
	
	.other:
		cmp	ecx, 0x7F03
		ja	short .bad
		
		push	edx
		call	_Write_Word_VDP_Data
		pop	edx
	
	.bad:
		ret
	
	align 16
	
	DECLF Z80_WriteW_68K_Ram, 8
		mov eax, [Bank_Z80]
		and ecx, 0x7FFF
		add ecx, eax
		push edx
		push ecx
;		call _Write_To_68K_Space
		call M68K_WB
		shr edx, 8
		inc ecx
		push edx
		push ecx
		call M68K_WB
		add esp, 16
		ret