; ScummVM - Graphic Adventure Engine
;
; ScummVM is the legal property of its developers, whose names
; are too numerous to list here. Please refer to the COPYRIGHT
; file distributed with this source distribution.
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.

; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
;
;---------------------------------------------------------------------------
;
; SCITRACE
;  TSR for logging specific calls inside sierra sci
;  Written by M. Kiewitz
;
;---------------------------------------------------------------------------

; Simply replace 51h 51h 51h 51h in sierra sci with
;  CDh 90h 90h 90h. This code will then log copyrect calls to scitrace.bin

		.286

code_seg        segment public
                assume  cs:code_seg, ds:nothing, es:nothing
                org     00100h

scitrace:       jmp     install_my_prog

;---------------------------------------------------------------------------

filename:       db 'SCITRACE.BIN', 0

inthandler:     push    ax
                push    bx
                push    cx
                push    dx
                push    ds
                push    cs
                pop     ds
                mov     ah, 3Dh
                mov     al, 1
                mov     dx, offset filename
                xor     cl, cl
                int     21h
                pop     ds
                jc      int_error
                mov     bx, ax
                mov     ax, 4202h
                xor     cx, cx
                xor     dx, dx
                int     21h
                mov     dx, si
                mov     ah, 40h
                mov     cx, 8
                int     21h
                mov     ah, 3Eh
                int     21h
int_error:      pop     dx
                pop     cx
                pop     bx
                pop     ax
                movsw
                movsw
                movsw
                movsw
                iret

end_of_tsr:

;---------------------------------------------------------------------------

titlestr:       db 'SCITRACE - written by M. Kiewitz',13,10,'$'
errorfile:      db 'error creating file',13,10,'$'

;---------------------------------------------------------------------------

install_my_prog:
                push    cs
                pop     ds
                mov     ah, 09h
                mov     dx, offset titlestr
                int     21h

                mov     ah, 3Ch
                mov     cx, 0
                mov     dx, offset filename
                int     21h
                jnc     valid_open
                mov     ah, 09h
                mov     dx, offset errorfile
                int     21h
                mov     ax, 6200h
                int     21h
                mov     es, bx
                mov     ax, 4C00h
                int     21h

valid_open:     mov     bx, ax
                mov     ah, 3Eh
                int     21h

NowInstallTSR:
                mov     ax, 2590h
                mov     dx, offset inthandler
                int     21h                     ; int 90h pointer <- ds:dx

                mov     ax, ds:[002ch]          ; get envt segment
                mov     es, ax
                mov     ax, 4900h
                int     21h

                mov     dx, offset end_of_tsr
                add     dx, 15
                shr     dx, 4
                mov     ax, 3100h
                int     21h

;---------------------------------------------------------------------------

code_seg	ends
		end	scitrace
