;
; Gens: CPU Flags.
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

; CPU flags (IA32/x86_64)
CPUFLAG_MMX		equ	(1 << 0)
CPUFLAG_SSE		equ	(1 << 1)
CPUFLAG_SSE2		equ	(1 << 2)
CPUFLAG_SSE3		equ	(1 << 3)
CPUFLAG_SSSE3		equ	(1 << 4)
CPUFLAG_SSE41		equ	(1 << 5)
CPUFLAG_SSE42		equ	(1 << 6)
CPUFLAG_SSE4A		equ	(1 << 7)
CPUFLAG_SSE5		equ	(1 << 8)

; CPU flags (IA32/x86_64; AMD only)
CPUFLAG_MMXEXT		equ	(1 << 9)
CPUFLAG_3DNOW		equ	(1 << 10)
CPUFLAG_3DNOWEXT	equ	(1 << 11)

; Contains CPU flags from getCPUFlags().
%ifdef __OBJ_ELF
%define _CPU_Flags CPU_Flags
%endif

extern _CPU_Flags;
