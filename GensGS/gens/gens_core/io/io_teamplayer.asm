;
; Gens: Teamplayer I/O.
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

section .data align=64
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_Controller_1_Counter	Controller_1_Counter
		%define	_Controller_1_Delay	Controller_1_Delay
		%define	_Controller_1_State	Controller_1_State
		%define	_Controller_1_COM	Controller_1_COM
		
		%define	_Controller_1_Type	Controller_1_Type
		%define	_Controller_1B_Type	Controller_1B_Type
		%define	_Controller_1C_Type	Controller_1C_Type
		%define	_Controller_1D_Type	Controller_1D_Type
		
		%define	_Controller_1_Buttons	Controller_1_Buttons
		%define	_Controller_1B_Buttons	Controller_1B_Buttons
		%define	_Controller_1C_Buttons	Controller_1C_Buttons
		%define	_Controller_1D_Buttons	Controller_1D_Buttons
		
		%define	_Controller_2_Counter	Controller_2_Counter
		%define	_Controller_2_Delay	Controller_2_Delay
		%define	_Controller_2_State	Controller_2_State
		%define	_Controller_2_COM	Controller_2_COM
		
		%define	_Controller_2_Type	Controller_2_Type
		%define	_Controller_2B_Type	Controller_2B_Type
		%define	_Controller_2C_Type	Controller_2C_Type
		%define	_Controller_2D_Type	Controller_2D_Type
		
		%define	_Controller_2_Buttons	Controller_2_Buttons
		%define	_Controller_2B_Buttons	Controller_2B_Buttons
		%define	_Controller_2C_Buttons	Controller_2C_Buttons
		%define	_Controller_2D_Buttons	Controller_2D_Buttons
	%endif
	
	extern _Controller_1_State
	extern _Controller_1_COM
	extern _Controller_1_Counter
	extern _Controller_1_Delay
	
	extern _Controller_1_Type
	extern _Controller_1B_Type
	extern _Controller_1C_Type
	extern _Controller_1D_Type
	
	extern _Controller_1_Buttons
	extern _Controller_1B_Buttons
	extern _Controller_1C_Buttons
	extern _Controller_1D_Buttons
	
	extern _Controller_2_State
	extern _Controller_2_COM
	extern _Controller_2_Counter
	extern _Controller_2_Delay
	
	extern _Controller_2_Buttons
	extern _Controller_2B_Buttons
	extern _Controller_2C_Buttons
	extern _Controller_2D_Buttons
	
	extern _Controller_2_Type
	extern _Controller_2B_Type
	extern _Controller_2C_Type
	extern _Controller_2D_Type
	
	; Old button variables for io_old.asm compatibility.
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_Controller_1_Up	Controller_1_Up
		%define	_Controller_1_Down	Controller_1_Down
		%define	_Controller_1_Left	Controller_1_Left
		%define	_Controller_1_Right	Controller_1_Right
		%define	_Controller_1_Start	Controller_1_Start
		%define	_Controller_1_A		Controller_1_A
		%define	_Controller_1_B		Controller_1_B
		%define	_Controller_1_C		Controller_1_C
		%define	_Controller_1_Mode	Controller_1_Mode
		%define	_Controller_1_X		Controller_1_X
		%define	_Controller_1_Y		Controller_1_Y
		%define	_Controller_1_Z		Controller_1_Z
		
		%define	_Controller_1B_Up	Controller_1B_Up
		%define	_Controller_1B_Down	Controller_1B_Down
		%define	_Controller_1B_Left	Controller_1B_Left
		%define	_Controller_1B_Right	Controller_1B_Right
		%define	_Controller_1B_Start	Controller_1B_Start
		%define	_Controller_1B_A	Controller_1B_A
		%define	_Controller_1B_B	Controller_1B_B
		%define	_Controller_1B_C	Controller_1B_C
		%define	_Controller_1B_Mode	Controller_1B_Mode
		%define	_Controller_1B_X	Controller_1B_X
		%define	_Controller_1B_Y	Controller_1B_Y
		%define	_Controller_1B_Z	Controller_1B_Z
		
		%define	_Controller_1C_Up	Controller_1C_Up
		%define	_Controller_1C_Down	Controller_1C_Down
		%define	_Controller_1C_Left	Controller_1C_Left
		%define	_Controller_1C_Right	Controller_1C_Right
		%define	_Controller_1C_Start	Controller_1C_Start
		%define	_Controller_1C_A	Controller_1C_A
		%define	_Controller_1C_B	Controller_1C_B
		%define	_Controller_1C_C	Controller_1C_C
		%define	_Controller_1C_Mode	Controller_1C_Mode
		%define	_Controller_1C_X	Controller_1C_X
		%define	_Controller_1C_Y	Controller_1C_Y
		%define	_Controller_1C_Z	Controller_1C_Z
		
		%define	_Controller_1D_Up	Controller_1D_Up
		%define	_Controller_1D_Down	Controller_1D_Down
		%define	_Controller_1D_Left	Controller_1D_Left
		%define	_Controller_1D_Right	Controller_1D_Right
		%define	_Controller_1D_Start	Controller_1D_Start
		%define	_Controller_1D_A	Controller_1D_A
		%define	_Controller_1D_B	Controller_1D_B
		%define	_Controller_1D_C	Controller_1D_C
		%define	_Controller_1D_Mode	Controller_1D_Mode
		%define	_Controller_1D_X	Controller_1D_X
		%define	_Controller_1D_Y	Controller_1D_Y
		%define	_Controller_1D_Z	Controller_1D_Z
		
		%define	_Controller_2_Up	Controller_2_Up
		%define	_Controller_2_Down	Controller_2_Down
		%define	_Controller_2_Left	Controller_2_Left
		%define	_Controller_2_Right	Controller_2_Right
		%define	_Controller_2_Start	Controller_2_Start
		%define	_Controller_2_A		Controller_2_A
		%define	_Controller_2_B		Controller_2_B
		%define	_Controller_2_C		Controller_2_C
		%define	_Controller_2_Mode	Controller_2_Mode
		%define	_Controller_2_X		Controller_2_X
		%define	_Controller_2_Y		Controller_2_Y
		%define	_Controller_2_Z		Controller_2_Z
		
		%define	_Controller_2B_Up	Controller_2B_Up
		%define	_Controller_2B_Down	Controller_2B_Down
		%define	_Controller_2B_Left	Controller_2B_Left
		%define	_Controller_2B_Right	Controller_2B_Right
		%define	_Controller_2B_Start	Controller_2B_Start
		%define	_Controller_2B_A	Controller_2B_A
		%define	_Controller_2B_B	Controller_2B_B
		%define	_Controller_2B_C	Controller_2B_C
		%define	_Controller_2B_Mode	Controller_2B_Mode
		%define	_Controller_2B_X	Controller_2B_X
		%define	_Controller_2B_Y	Controller_2B_Y
		%define	_Controller_2B_Z	Controller_2B_Z
		
		%define	_Controller_2C_Up	Controller_2C_Up
		%define	_Controller_2C_Down	Controller_2C_Down
		%define	_Controller_2C_Left	Controller_2C_Left
		%define	_Controller_2C_Right	Controller_2C_Right
		%define	_Controller_2C_Start	Controller_2C_Start
		%define	_Controller_2C_A	Controller_2C_A
		%define	_Controller_2C_B	Controller_2C_B
		%define	_Controller_2C_C	Controller_2C_C
		%define	_Controller_2C_Mode	Controller_2C_Mode
		%define	_Controller_2C_X	Controller_2C_X
		%define	_Controller_2C_Y	Controller_2C_Y
		%define	_Controller_2C_Z	Controller_2C_Z
		
		%define	_Controller_2D_Up	Controller_2D_Up
		%define	_Controller_2D_Down	Controller_2D_Down
		%define	_Controller_2D_Left	Controller_2D_Left
		%define	_Controller_2D_Right	Controller_2D_Right
		%define	_Controller_2D_Start	Controller_2D_Start
		%define	_Controller_2D_A	Controller_2D_A
		%define	_Controller_2D_B	Controller_2D_B
		%define	_Controller_2D_C	Controller_2D_C
		%define	_Controller_2D_Mode	Controller_2D_Mode
		%define	_Controller_2D_X	Controller_2D_X
		%define	_Controller_2D_Y	Controller_2D_Y
		%define	_Controller_2D_Z	Controller_2D_Z
	%endif
	
	extern _Controller_1_Up
	extern _Controller_1_Down
	extern _Controller_1_Left
	extern _Controller_1_Right
	extern _Controller_1_Start
	extern _Controller_1_A
	extern _Controller_1_B
	extern _Controller_1_C
	extern _Controller_1_Mode
	extern _Controller_1_X
	extern _Controller_1_Y
	extern _Controller_1_Z
	
	extern _Controller_1B_Up
	extern _Controller_1B_Down
	extern _Controller_1B_Left
	extern _Controller_1B_Right
	extern _Controller_1B_Start
	extern _Controller_1B_A
	extern _Controller_1B_B
	extern _Controller_1B_C
	extern _Controller_1B_Mode
	extern _Controller_1B_X
	extern _Controller_1B_Y
	extern _Controller_1B_Z
	
	extern _Controller_1C_Up
	extern _Controller_1C_Down
	extern _Controller_1C_Left
	extern _Controller_1C_Right
	extern _Controller_1C_Start
	extern _Controller_1C_A
	extern _Controller_1C_B
	extern _Controller_1C_C
	extern _Controller_1C_Mode
	extern _Controller_1C_X
	extern _Controller_1C_Y
	extern _Controller_1C_Z
	
	extern _Controller_1D_Up
	extern _Controller_1D_Down
	extern _Controller_1D_Left
	extern _Controller_1D_Right
	extern _Controller_1D_Start
	extern _Controller_1D_A
	extern _Controller_1D_B
	extern _Controller_1D_C
	extern _Controller_1D_Mode
	extern _Controller_1D_X
	extern _Controller_1D_Y
	extern _Controller_1D_Z
	
	extern _Controller_2_Up
	extern _Controller_2_Down
	extern _Controller_2_Left
	extern _Controller_2_Right
	extern _Controller_2_Start
	extern _Controller_2_A
	extern _Controller_2_B
	extern _Controller_2_C
	extern _Controller_2_Mode
	extern _Controller_2_X
	extern _Controller_2_Y
	extern _Controller_2_Z
	
	extern _Controller_2B_Up
	extern _Controller_2B_Down
	extern _Controller_2B_Left
	extern _Controller_2B_Right
	extern _Controller_2B_Start
	extern _Controller_2B_A
	extern _Controller_2B_B
	extern _Controller_2B_C
	extern _Controller_2B_Mode
	extern _Controller_2B_X
	extern _Controller_2B_Y
	extern _Controller_2B_Z
	
	extern _Controller_2C_Up
	extern _Controller_2C_Down
	extern _Controller_2C_Left
	extern _Controller_2C_Right
	extern _Controller_2C_Start
	extern _Controller_2C_A
	extern _Controller_2C_B
	extern _Controller_2C_C
	extern _Controller_2C_Mode
	extern _Controller_2C_X
	extern _Controller_2C_Y
	extern _Controller_2C_Z
	
	extern _Controller_2D_Up
	extern _Controller_2D_Down
	extern _Controller_2D_Left
	extern _Controller_2D_Right
	extern _Controller_2D_Start
	extern _Controller_2D_A
	extern _Controller_2D_B
	extern _Controller_2D_C
	extern _Controller_2D_Mode
	extern _Controller_2D_X
	extern _Controller_2D_Y
	extern _Controller_2D_Z
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_RD_Controller_1_TP		RD_Controller_1_TP
		%define	_RD_Controller_1_TP.TP_L0L	RD_Controller_1_TP.TP_L0L
		%define	_RD_Controller_1_TP.TP_L0H	RD_Controller_1_TP.TP_L0H
		%define	_RD_Controller_1_TP.TP_H0L	RD_Controller_1_TP.TP_H0L
		%define	_RD_Controller_1_TP.TP_H0H	RD_Controller_1_TP.TP_H0H
		%define	_RD_Controller_1_TP.TP_L1L	RD_Controller_1_TP.TP_L1L
		%define	_RD_Controller_1_TP.TP_L1H	RD_Controller_1_TP.TP_L1H
		%define	_RD_Controller_1_TP.TP_H1L	RD_Controller_1_TP.TP_H1L
		%define	_RD_Controller_1_TP.TP_H1H	RD_Controller_1_TP.TP_H1H
		%define	_RD_Controller_1_TP.TP_L2L	RD_Controller_1_TP.TP_L2L
		%define	_RD_Controller_1_TP.TP_L2H	RD_Controller_1_TP.TP_L2H
		%define	_RD_Controller_1_TP.TP_H2L	RD_Controller_1_TP.TP_H2L
		%define	_RD_Controller_1_TP.TP_H2H	RD_Controller_1_TP.TP_H2H
		%define	_RD_Controller_1_TP.TP_L3L	RD_Controller_1_TP.TP_L3L
		%define	_RD_Controller_1_TP.TP_L3H	RD_Controller_1_TP.TP_L3H
		%define	_RD_Controller_1_TP.TP_H3L	RD_Controller_1_TP.TP_H3L
		%define	_RD_Controller_1_TP.TP_H3H	RD_Controller_1_TP.TP_H3H
		%define	_RD_Controller_1_TP.UNDEF	RD_Controller_1_TP.UNDEF
		%define	_RD_Controller_1_TP.TP_PA_DIR	RD_Controller_1_TP.TP_PA_DIR
		%define	_RD_Controller_1_TP.TP_PA_ABC	RD_Controller_1_TP.TP_PA_ABC
		%define	_RD_Controller_1_TP.TP_PA_XYZ	RD_Controller_1_TP.TP_PA_XYZ
		%define	_RD_Controller_1_TP.TP_PB_DIR	RD_Controller_1_TP.TP_PB_DIR
		%define	_RD_Controller_1_TP.TP_PB_ABC	RD_Controller_1_TP.TP_PB_ABC
		%define	_RD_Controller_1_TP.TP_PB_XYZ	RD_Controller_1_TP.TP_PB_XYZ
		%define	_RD_Controller_1_TP.TP_PC_DIR	RD_Controller_1_TP.TP_PC_DIR
		%define	_RD_Controller_1_TP.TP_PC_ABC	RD_Controller_1_TP.TP_PC_ABC
		%define	_RD_Controller_1_TP.TP_PC_XYZ	RD_Controller_1_TP.TP_PC_XYZ
		%define	_RD_Controller_1_TP.TP_PD_DIR	RD_Controller_1_TP.TP_PD_DIR
		%define	_RD_Controller_1_TP.TP_PD_ABC	RD_Controller_1_TP.TP_PD_ABC
		%define	_RD_Controller_1_TP.TP_PD_XYZ	RD_Controller_1_TP.TP_PD_XYZ
		
		%define	_RD_Controller_2_TP		RD_Controller_2_TP
		%define	_RD_Controller_2_TP.TP_L0L	RD_Controller_2_TP.TP_L0L
		%define	_RD_Controller_2_TP.TP_L0H	RD_Controller_2_TP.TP_L0H
		%define	_RD_Controller_2_TP.TP_H0L	RD_Controller_2_TP.TP_H0L
		%define	_RD_Controller_2_TP.TP_H0H	RD_Controller_2_TP.TP_H0H
		%define	_RD_Controller_2_TP.TP_L1L	RD_Controller_2_TP.TP_L1L
		%define	_RD_Controller_2_TP.TP_L1H	RD_Controller_2_TP.TP_L1H
		%define	_RD_Controller_2_TP.TP_H1L	RD_Controller_2_TP.TP_H1L
		%define	_RD_Controller_2_TP.TP_H1H	RD_Controller_2_TP.TP_H1H
		%define	_RD_Controller_2_TP.TP_L2L	RD_Controller_2_TP.TP_L2L
		%define	_RD_Controller_2_TP.TP_L2H	RD_Controller_2_TP.TP_L2H
		%define	_RD_Controller_2_TP.TP_H2L	RD_Controller_2_TP.TP_H2L
		%define	_RD_Controller_2_TP.TP_H2H	RD_Controller_2_TP.TP_H2H
		%define	_RD_Controller_2_TP.TP_L3L	RD_Controller_2_TP.TP_L3L
		%define	_RD_Controller_2_TP.TP_L3H	RD_Controller_2_TP.TP_L3H
		%define	_RD_Controller_2_TP.TP_H3L	RD_Controller_2_TP.TP_H3L
		%define	_RD_Controller_2_TP.TP_H3H	RD_Controller_2_TP.TP_H3H
		%define	_RD_Controller_2_TP.UNDEF	RD_Controller_2_TP.UNDEF
		%define	_RD_Controller_2_TP.TP_PA_DIR	RD_Controller_2_TP.TP_PA_DIR
		%define	_RD_Controller_2_TP.TP_PA_ABC	RD_Controller_2_TP.TP_PA_ABC
		%define	_RD_Controller_2_TP.TP_PA_XYZ	RD_Controller_2_TP.TP_PA_XYZ
		%define	_RD_Controller_2_TP.TP_PB_DIR	RD_Controller_2_TP.TP_PB_DIR
		%define	_RD_Controller_2_TP.TP_PB_ABC	RD_Controller_2_TP.TP_PB_ABC
		%define	_RD_Controller_2_TP.TP_PB_XYZ	RD_Controller_2_TP.TP_PB_XYZ
		%define	_RD_Controller_2_TP.TP_PC_DIR	RD_Controller_2_TP.TP_PC_DIR
		%define	_RD_Controller_2_TP.TP_PC_ABC	RD_Controller_2_TP.TP_PC_ABC
		%define	_RD_Controller_2_TP.TP_PC_XYZ	RD_Controller_2_TP.TP_PC_XYZ
		%define	_RD_Controller_2_TP.TP_PD_DIR	RD_Controller_2_TP.TP_PD_DIR
		%define	_RD_Controller_2_TP.TP_PD_ABC	RD_Controller_2_TP.TP_PD_ABC
		%define	_RD_Controller_2_TP.TP_PD_XYZ	RD_Controller_2_TP.TP_PD_XYZ
		
		%define _Make_IO_Table			Make_IO_Table
	%endif
	
	Table_TP1:
		dd	_RD_Controller_1_TP.TP_L0L, _RD_Controller_1_TP.TP_L0H	; 00-01
		dd	_RD_Controller_1_TP.TP_H0L, _RD_Controller_1_TP.TP_H0H	; 02-03
		dd	_RD_Controller_1_TP.TP_L1L, _RD_Controller_1_TP.TP_L1H	; 04-05
		dd	_RD_Controller_1_TP.TP_H1L, _RD_Controller_1_TP.TP_H1H	; 06-07
		dd	_RD_Controller_1_TP.TP_L2L, _RD_Controller_1_TP.TP_L2H	; 08-09
		dd	_RD_Controller_1_TP.TP_H2L, _RD_Controller_1_TP.TP_H2H	; 0A-0B
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.TP_L3H	; 0C-0D
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.TP_H3H	; 0E-0F
		
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 10-11
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 12-13
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 14-15
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 16-17
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 18-19
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 1A-1B
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 1C-1D
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 1E-1F
		
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 20-21
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 22-23
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 24-25
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 26-27
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 28-29
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 2A-2B
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 2C-2D
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 2E-2F
		
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 30-31
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 32-33
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 34-35
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 36-37
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 38-39
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 3A-3B
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 3C-3D
		dd	_RD_Controller_1_TP.UNDEF,  _RD_Controller_1_TP.UNDEF	; 3E-3F
		
	Table_TP2:
		dd	_RD_Controller_2_TP.TP_L0L, _RD_Controller_2_TP.TP_L0H	; 00-01
		dd	_RD_Controller_2_TP.TP_H0L, _RD_Controller_2_TP.TP_H0H	; 02-03
		dd	_RD_Controller_2_TP.TP_L1L, _RD_Controller_2_TP.TP_L1H	; 04-05
		dd	_RD_Controller_2_TP.TP_H1L, _RD_Controller_2_TP.TP_H1H	; 06-07
		dd	_RD_Controller_2_TP.TP_L2L, _RD_Controller_2_TP.TP_L2H	; 08-09
		dd	_RD_Controller_2_TP.TP_H2L, _RD_Controller_2_TP.TP_H2H	; 0A-0B
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.TP_L3H	; 0C-0D
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.TP_H3H	; 0E-0F
		
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 10-11
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 12-13
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 14-15
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 16-17
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 18-19
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 1A-1B
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 1C-1D
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 1E-1F
		
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 20-21
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 22-23
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 24-25
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 26-27
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 28-29
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 2A-2B
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 2C-2D
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 2E-2F
		
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 30-31
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 32-33
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 34-35
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 36-37
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 38-39
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 3A-3B
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 3C-3D
		dd	_RD_Controller_2_TP.UNDEF,  _RD_Controller_2_TP.UNDEF	; 3E-3F
	
section .text align=64
	
	; unsigned char RD_Controller_1_TP(void)
	global _RD_Controller_1_TP
	_RD_Controller_1_TP:
		
		push	ebx
		
		mov	eax, [_Controller_1_State]
		mov	ebx, [_Controller_1_Counter]
		test	byte [_Controller_2_State], 0xC
		jnz	near .Team_Player_2
		
		shr	eax, 3
		and	ebx, 0x0F0000
		and	eax, 0x0C
		shr	ebx, 12
		add	eax, ebx
		mov	ebx, eax
		test	eax, 0x4
		jz	short .TP_Low
		or	ebx, 0x2
		
	.TP_Low:
		shl	ebx, 3
		and	ebx, 0x70
		jmp	[Table_TP1 + eax]
	
	align 4
	
	.TP_H0H:
		mov	eax, 0x73
		pop	ebx
		ret
	
	align 4
	
	.TP_L0H:
		mov	eax, 0x3F
		pop	ebx
		ret
	
	align 4
	
	.TP_H0L:
	.TP_L0L:
	.TP_H1H:
	.TP_L1H:
		mov	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_H1L:
	.TP_L1L:
		mov	ax, [_Controller_1_Type]
		and	ax, 1
		or	ax, bx
		pop	ebx
		ret
	
	align 4
	
	.TP_H2H:
	.TP_L2H:
		mov	ax, [_Controller_1B_Type]
		or	ax, bx
		pop	ebx
		ret
	
	align 4
	
	.TP_H2L:
	.TP_L2L:
		mov	ax, [_Controller_1C_Type]
		or	ax, bx
		pop	ebx
		ret
	
	align 4
	
	.TP_H3H:
	.TP_L3H:
		mov	ax, [_Controller_1D_Type]
		or	ax, bx
		pop	ebx
		ret
	
	align 4
	
	.TP_PA_DIR:
		mov	eax, [_Controller_1_Right]
		add	eax, eax
		add	eax, [_Controller_1_Left]
		add	eax, eax
		add	eax, [_Controller_1_Down]
		add	eax, eax
		add	eax, [_Controller_1_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PA_ABC:
		mov	eax, [_Controller_1_Start]
		add	eax, eax
		add	eax, [_Controller_1_A]
		add	eax, eax
		add	eax, [_Controller_1_C]
		add	eax, eax
		add	eax, [_Controller_1_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PA_XYZ:
		mov	eax, [_Controller_1_Mode]
		add	eax, eax
		add	eax, [_Controller_1_X]
		add	eax, eax
		add	eax, [_Controller_1_Y]
		add	eax, eax
		add	eax, [_Controller_1_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PB_DIR:
		mov	eax, [_Controller_1B_Right]
		add	eax, eax
		add	eax, [_Controller_1B_Left]
		add	eax, eax
		add	eax, [_Controller_1B_Down]
		add	eax, eax
		add	eax, [_Controller_1B_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PB_ABC:
		mov	eax, [_Controller_1B_Start]
		add	eax, eax
		add	eax, [_Controller_1B_A]
		add	eax, eax
		add	eax, [_Controller_1B_C]
		add	eax, eax
		add	eax, [_Controller_1B_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PB_XYZ:
		mov	eax, [_Controller_1B_Mode]
		add	eax, eax
		add	eax, [_Controller_1B_X]
		add	eax, eax
		add	eax, [_Controller_1B_Y]
		add	eax, eax
		add	eax, [_Controller_1B_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PC_DIR:
		mov	eax, [_Controller_1C_Right]
		add	eax, eax
		add	eax, [_Controller_1C_Left]
		add	eax, eax
		add	eax, [_Controller_1C_Down]
		add	eax, eax
		add	eax, [_Controller_1C_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PC_ABC:
		mov	eax, [_Controller_1C_Start]
		add	eax, eax
		add	eax, [_Controller_1C_A]
		add	eax, eax
		add	eax, [_Controller_1C_C]
		add	eax, eax
		add	eax, [_Controller_1C_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PC_XYZ:
		mov	eax, [_Controller_1C_Mode]
		add	eax, eax
		add	eax, [_Controller_1C_X]
		add	eax, eax
		add	eax, [_Controller_1C_Y]
		add	eax, eax
		add	eax, [_Controller_1C_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PD_DIR:
		mov	eax, [_Controller_1D_Right]
		add	eax, eax
		add	eax, [_Controller_1D_Left]
		add	eax, eax
		add	eax, [_Controller_1D_Down]
		add	eax, eax
		add	eax, [_Controller_1D_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PD_ABC:
		mov	eax, [_Controller_1D_Start]
		add	eax, eax
		add	eax, [_Controller_1D_A]
		add	eax, eax
		add	eax, [_Controller_1D_C]
		add	eax, eax
		add	eax, [_Controller_1D_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PD_XYZ:
		mov	eax, [_Controller_1D_Mode]
		add	eax, eax
		add	eax, [_Controller_1D_X]
		add	eax, eax
		add	eax, [_Controller_1D_Y]
		add	eax, eax
		add	eax, [_Controller_1D_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4

	.UNDEF:
		mov	eax, 0xF
		or	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.Table_Get_Cont:
		dd	_Controller_1_Type
		dd	_Controller_1B_Type
		dd	_Controller_1C_Type
		dd	_Controller_1D_Type
	
	align 32
	
	.Team_Player_2:
		push	edx
		mov	edx, [_Controller_2_State]
		test	edx, 0x40
		jnz	short .TP2_Spec
		
		and	edx, 0x30
		shr	edx, 2
		mov	edx, [.Table_Get_Cont + edx]
		
;		dec ebx
		shr	eax, 4
		and	ebx, 0x03
		and	eax, 0x04
		test	byte [edx + 0], 1	; 6 buttons controller
		jnz	short .TP2_Six_Buttons
		
		xor	ebx, ebx
		
	.TP2_Six_Buttons:
		jmp	[.TP2_Table_Cont + ebx * 8 + eax]
	
	align 4
	
	.TP2_Table_Cont:
		dd	.TP2_First_Low, .TP2_First_High
		dd	.TP2_Second_Low, .TP2_Second_High
		dd	.TP2_Third_Low, .TP2_Third_High
		dd	.TP2_Fourth_Low, .TP2_Fourth_High
	
	align 4
	
	.TP2_Spec:
		mov	eax, 0x70
		pop	edx
		pop	ebx
		ret
	
	align 16
	
	.TP2_First_High:
	.TP2_Second_High:
	.TP2_Third_High:
		mov	eax, [edx + 36]
		mov	ebx, [edx + 32]
		lea	eax, [eax * 4]
		lea	ebx, [ebx * 4]
		or	eax, [edx + 16]
		or	ebx, [edx + 12]
		lea	eax, [eax * 4]
		lea	ebx, [ebx * 4]
		or	eax, [edx + 8]
		or	ebx, [edx + 4]
		lea	eax, [eax * 2 + ebx + 0x40]
		pop	edx
		pop	ebx
		ret
	
	align 16
	
	.TP2_First_Low:
	.TP2_Second_Low:
  		mov	eax, [edx + 20]
		mov	ebx, [edx + 28]
		shl	eax, 4
		shl	ebx, 4
		or	eax, [edx + 8]
		or	ebx, [edx + 4]
		lea	eax, [eax * 2 + ebx]
		pop	edx
		pop	ebx
		ret
	
	align 16
		
	.TP2_Third_Low:
		mov	eax, [_Controller_1_Start]
		mov	ebx, [_Controller_1_A]
		lea	eax, [eax * 2 + ebx]
		shl	eax, 4
		pop	edx
		pop	ebx
		ret
	
	align 16

	.TP2_Fourth_High:
		mov	eax, [edx + 36]
		mov	ebx, [edx + 32]
		lea	eax, [eax * 4]
		lea	ebx, [ebx * 4]
		or	eax, [edx + 24]
		or	ebx, [edx + 40]
		lea	eax, [eax * 4]
		lea	ebx, [ebx * 4]
		or	eax, [edx + 44]
		or	ebx, [edx + 48]
		lea	eax, [eax * 2 + ebx + 0x40]
		pop	edx
		pop	ebx
		ret
	
	align 16
	
	.TP2_Fourth_Low:
		mov	eax, [edx + 20]
		mov	ebx, [edx + 28]
		lea	eax, [eax * 2 + ebx]
		shl	eax, 4
		or	eax, 0xF
		pop	edx
		pop	ebx
		ret
	
	align 64
	
	; unsigned char RD_Controller_2_TP(void)
	global _RD_Controller_2_TP
	_RD_Controller_2_TP:
		
		push	ebx
		
		mov	eax, [_Controller_2_State]
		mov	ebx, [_Controller_2_Counter]
		shr	eax, 3
		and	ebx, 0x0F0000
		and	eax, 0x0C
		shr	ebx, 12
		add	eax, ebx
		mov	ebx, eax
		test	eax, 0x4
		jz	short .TP_Low
		or	ebx, 0x2
		
	.TP_Low:
		shl	ebx, 3
		and	ebx, 0x70
		jmp	[Table_TP1 + eax]		; TODO: Should this be TP1 or TP2?
	
	align 4
	
	.TP_H0H:
		mov	eax, 0x73
		pop	ebx
		ret
	
	align 4

	.TP_L0H:
		mov	eax, 0x3F
		pop	ebx
		ret
	
	align 4
	
	.TP_H0L:
	.TP_L0L:
	.TP_H1H:
	.TP_L1H:
		mov	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_H1L:
	.TP_L1L:
		mov	ax, [_Controller_2_Type]
		and	ax, 1
		or	ax, bx
		pop	ebx
		ret
	
	align 4
	
	.TP_H2H:
	.TP_L2H:
		mov	ax, [_Controller_2B_Type]
		or	ax, bx
		pop	ebx
		ret
	
	align 4

	.TP_H2L:
	.TP_L2L:
		mov	ax, [_Controller_2C_Type]
		or	ax, bx
		ret
	
	align 4
	
	.TP_H3H:
	.TP_L3H:
		mov	ax, [_Controller_2D_Type]
		or	ax, bx
		pop	ebx
		ret
	
	align 4
	
	.TP_PA_DIR:
		mov	eax, [_Controller_2_Right]
		add	eax, eax
		add	eax, [_Controller_2_Left]
		add	eax, eax
		add	eax, [_Controller_2_Down]
		add	eax, eax
		add	eax, [_Controller_2_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PA_ABC:
		mov	eax, [_Controller_2_Start]
		add	eax, eax
		add	eax, [_Controller_2_A]
		add	eax, eax
		add	eax, [_Controller_2_C]
		add	eax, eax
		add	eax, [_Controller_2_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PA_XYZ:
		mov	eax, [_Controller_2_Mode]
		add	eax, eax
		add	eax, [_Controller_2_X]
		add	eax, eax
		add	eax, [_Controller_2_Y]
		add	eax, eax
		add	eax, [_Controller_2_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PB_DIR:
		mov	eax, [_Controller_2B_Right]
		add	eax, eax
		add	eax, [_Controller_2B_Left]
		add	eax, eax
		add	eax, [_Controller_2B_Down]
		add	eax, eax
		add	eax, [_Controller_2B_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PB_ABC:
		mov	eax, [_Controller_2B_Start]
		add	eax, eax
		add	eax, [_Controller_2B_A]
		add	eax, eax
		add	eax, [_Controller_2B_C]
		add	eax, eax
		add	eax, [_Controller_2B_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PB_XYZ:
		mov	eax, [_Controller_2B_Mode]
		add	eax, eax
		add	eax, [_Controller_2B_X]
		add	eax, eax
		add	eax, [_Controller_2B_Y]
		add	eax, eax
		add	eax, [_Controller_2B_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PC_DIR:
		mov	eax, [_Controller_2C_Right]
		add	eax, eax
		add	eax, [_Controller_2C_Left]
		add	eax, eax
		add	eax, [_Controller_2C_Down]
		add	eax, eax
		add	eax, [_Controller_2C_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PC_ABC:
		mov	eax, [_Controller_2C_Start]
		add	eax, eax
		add	eax, [_Controller_2C_A]
		add	eax, eax
		add	eax, [_Controller_2C_C]
		add	eax, eax
		add	eax, [_Controller_2C_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PC_XYZ:
		mov	eax, [_Controller_2C_Mode]
		add	eax, eax
		add	eax, [_Controller_2C_X]
		add	eax, eax
		add	eax, [_Controller_2C_Y]
		add	eax, eax
		add	eax, [_Controller_2C_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PD_DIR:
		mov	eax, [_Controller_2D_Right]
		add	eax, eax
		add	eax, [_Controller_2D_Left]
		add	eax, eax
		add	eax, [_Controller_2D_Down]
		add	eax, eax
		add	eax, [_Controller_2D_Up]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PD_ABC:
		mov	eax, [_Controller_2D_Start]
		add	eax, eax
		add	eax, [_Controller_2D_A]
		add	eax, eax
		add	eax, [_Controller_2D_C]
		add	eax, eax
		add	eax, [_Controller_2D_B]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.TP_PD_XYZ:
		mov	eax, [_Controller_2D_Mode]
		add	eax, eax
		add	eax, [_Controller_2D_X]
		add	eax, eax
		add	eax, [_Controller_2D_Y]
		add	eax, eax
		add	eax, [_Controller_2D_Z]
		add	eax, ebx
		pop	ebx
		ret
	
	align 4
	
	.UNDEF:
		mov	eax, 0xF
		or	eax, ebx
		pop	ebx
		ret
	
	align 64
	
; CALC_OFFSET: Calculates offsets for Teamplayer functions.
%macro CALC_OFFSET 0
	mov	ecx, ebx
	mov	edx, ebx
	and	ecx, 0x1
	and	edx, 0xE
	xor	ecx, 0x1
	shl	edx, 3
	inc	ebx
%endmacro
	
	; void Make_IO_Table(void)
	global _Make_IO_Table
	_Make_IO_Table:
		
		push	eax
		push	ebx
		push	ecx
		push	edx
	
	.P1A:
		mov	ebx, 1
		test	byte [_Controller_1_Type], 1
		jnz	short .P1A_6
	
	.P1A_3:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PA_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PA_ABC
		jmp	short .P1B
	
	align 4
	
	.P1A_6:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PA_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PA_ABC
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PA_XYZ
		jmp	short .P1B
	
	align 4
	
	.P1B:
		test	byte [_Controller_1B_Type], 1
		jnz	short .P1B_6
	
	.P1B_3:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PB_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PB_ABC
		jmp	short .P1C
	
	align 4
	
	.P1B_6:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PB_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PB_ABC
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PB_XYZ
		jmp	short .P1C
	
	align 4
	
	.P1C:
		test	byte [_Controller_1C_Type], 1
		jnz	short .P1C_6
	
	.P1C_3:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PC_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PC_ABC
		jmp	short .P1D
	
	align 4
	
	.P1C_6:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PC_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PC_ABC
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PC_XYZ
		jmp	short .P1D
	
	align 4
	
	.P1D:
		test	byte [_Controller_1D_Type], 1
		jnz	short .P1D_6

	.P1D_3:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PD_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PD_ABC
		jmp	short .P2A
	
	align 4
	
	.P1D_6:
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PD_DIR
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PD_ABC
		CALC_OFFSET
		mov	dword [Table_TP1 + 0x30 + edx + ecx * 4], _RD_Controller_1_TP.TP_PD_XYZ
		jmp	short .P2A
	
	align 4
	
	.P2A:
		mov	ebx, 1
		test	byte [_Controller_2_Type], 1
		jnz	short .P2A_6
	
	.P2A_3:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PA_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PA_ABC
		jmp	short .P2B
	
	align 4
	
	.P2A_6:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PA_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PA_ABC
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PA_XYZ
		jmp	short .P2B
	
	align 4
	
	.P2B:
		test	byte [_Controller_2B_Type], 1
		jnz	short .P2B_6
	
	.P2B_3:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PB_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PB_ABC
		jmp	short .P2C
	
	align 4
	
	.P2B_6:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PB_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PB_ABC
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PB_XYZ
		jmp	short .P2C
	
	align 4
	
	.P2C:
		test	byte [_Controller_2C_Type], 1
		jnz	short .P2C_6
	
	.P2C_3:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PC_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PC_ABC
		jmp	short .P2D
	
	align 4
	
	.P2C_6:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PC_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PC_ABC
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PC_XYZ
		jmp	short .P2D
	
	align 4
	
	.P2D:
		test	byte [_Controller_2D_Type], 1
		jnz	short .P2D_6
	
	.P2D_3:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PD_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PD_ABC
		jmp	short .End
	
	align 4
	
	.P2D_6:
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PD_DIR
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PD_ABC
		CALC_OFFSET
		mov	dword [Table_TP2 + 0x30 + edx + ecx * 4], _RD_Controller_2_TP.TP_PD_XYZ
		jmp	short .End
	
	align 4
	
	.End:
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
