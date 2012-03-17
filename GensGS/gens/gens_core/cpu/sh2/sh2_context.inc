;****************************************
;
; SH2 emulator 1.60 (SH2_CONTEXT macro)
; Copyright 2002 Stéphane Dallongeville
;
;****************************************


%macro SH2_CONTEXT 0
		
		.Cache:		resb 0x1000
		
		.R:		resd 0x10
		
		.SR:
		.SR_T:		resb 1
		.SR_S:		resb 1
		.SR_IMask:	resb 1
		.SR_MQ:		resb 1
		
		.INT:
		.INT_Vect:	resb 1
		.INT_Prio:	resb 1
		.INT_res1:	resb 1
		.INT_res2:	resb 1
		
		.GBR:		resd 1
		.VBR:		resd 1
		
		.INT_QUEUE:	resb 0x20
		
		.MACH:		resd 1
		.MACL:		resd 1
		.PR:		resd 1
		.PC:		resd 1
		
		.Status:	resd 1
		.Base_PC:	resd 1
		.Fetch_Start:	resd 1
		.Fetch_End:	resd 1
		
		.DS_Inst:	resd 1
		.DS_PC:		resd 1
		.Unused1:	resd 1
		.Unused2:	resd 1
		
		.Odometer:	resd 1
		.Cycle_TD:	resd 1
		.Cycle_IO:	resd 1
		.Cycle_Sup:	resd 1
		
		.Reset_Size:
		
		.Read_Byte:	resd 0x100
		.Read_Word:	resd 0x100
		.Read_Long:	resd 0x100
		
		.Write_Byte:	resd 0x100
		.Write_Word:	resd 0x100
		.Write_Long:	resd 0x100
		
		.Fetch_Region:	resd 0x100 * 3
		
		.IO_Reg:	resb 0x200
		
		.DVCR:		resd 1
		.DVSR:		resd 1
		.DVDNTH:	resd 1
		.DVDNTL:	resd 1		; 4 dword
		
		.DRCR0:		resb 1
		.DRCR1:		resb 1
		.DREQ0:		resb 1
		.DREQ1:		resb 1
		
		.DMAOR:		resd 1
		
		.SAR0:		resd 1
		.DAR0:		resd 1		; 4 dword
		.TCR0:		resd 1
		.CHCR0:		resd 1
		
		.SAR1:		resd 1
		.DAR1:		resd 1		; 4 dword
		.TCR1:		resd 1
		.CHCR1:		resd 1
		
		.VCRDIV:	resd 1
		.VCRDMA0:	resd 1		; 4 dword
		.VCRDMA1	resd 1
		.VCRWDT:	resd 1
		
		.IPDIV:		resd 1
		.IPDMA:		resd 1		; 4 dword
		.IPWDT:		resd 1
		.IPBSC:		resd 1
		
		.BARA:		resd 1
		.BAMRA:		resd 1		; 4 dword
		
		.WDT_Tab:	resb 8
		.WDTCNT:	resd 1
		.WDT_Sft:	resb 1
		.WDTSR:		resb 1
		.WDTRST:	resb 1
		.Unused3:	resb 1		; 4 dword
		
		.FRT_Tab:	resb 4
		.FRTCNT:	resd 1
		.FRTOCRA:	resd 1
		.FRTOCRB:	resd 1		; 4 dword
		
		.FRTTIER:	resb 1
		.FRTCSR:	resb 1
		.FRTTCR:	resb 1
		.FRTTOCR:	resb 1
		.FRTICR:	resd 1
		.FRT_Sft:	resd 1
		.BCR1:		resd 1		; 4 dword
		
		.Init_Size:
		
%endmacro
	
	
	struc SH2
		
		SH2_CONTEXT
		
	endstruc
