;hq3x filter
;----------------------------------------------------------
;Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

GLOBAL _hq3x_32

EXTERN _LUT16to32
EXTERN _RGBtoYUV

SECTION .bss
firstline resd 1
linesleft resd 1
xcounter  resd 1
cross     resd 1
nextline  resd 1
prevline  resd 1
w1        resd 1
w2        resd 1
w3        resd 1
w4        resd 1
w5        resd 1
w6        resd 1
w7        resd 1
w8        resd 1
w9        resd 1
c1        resd 1
c2        resd 1
c3        resd 1
c4        resd 1
c5        resd 1
c6        resd 1
c7        resd 1
c8        resd 1
c9        resd 1

SECTION .data

reg_blank    dd  0,0
const7       dd  0x00070007,0x00000007
threshold    dd  0x00300706,0x00000000

SECTION .text

%macro TestDiff 2
    xor     ecx,ecx
    mov     edx,[%1]
    cmp     edx,[%2]
    je      %%fin
    mov     ecx,_RGBtoYUV
    movd    mm1,[ecx+edx*4]
    movq    mm5,mm1
    mov     edx,[%2]
    movd    mm2,[ecx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    ecx,mm1
%%fin:
%endmacro

%macro DiffOrNot 4
   TestDiff %1,%2
   test ecx,ecx
   jz   %%same
   %3
   jmp %%fin
%%same:
   %4
%%fin
%endmacro

%macro DiffOrNot 6
   TestDiff %1,%2
   test ecx,ecx
   jz   %%same
   %3
   %4
   jmp %%fin
%%same:
   %5
   %6
%%fin
%endmacro

%macro DiffOrNot 8
   TestDiff %1,%2
   test ecx,ecx
   jz   %%same
   %3
   %4
   %5
   jmp %%fin
%%same:
   %6
   %7
   %8
%%fin
%endmacro

%macro DiffOrNot 10
   TestDiff %1,%2
   test ecx,ecx
   jz %%same
   %3
   %4
   %5
   %6
   jmp %%fin
%%same:
   %7
   %8
   %9
   %10
%%fin
%endmacro

%macro Interp1 3
    mov edx,%2
    shl edx,2
    add edx,%3
    sub edx,%2
    shr edx,2
    mov %1,edx
%endmacro

%macro Interp2 4
    mov edx,%2
    shl edx,1
    add edx,%3
    add edx,%4
    shr edx,2
    mov %1,edx
%endmacro

%macro Interp3 2
    movd       mm1, eax
    movd       mm2, %2
    punpcklbw  mm1, [reg_blank]
    punpcklbw  mm2, [reg_blank]
    pmullw     mm1, [const7]
    paddw      mm1, mm2
    psrlw      mm1, 3
    packuswb   mm1, [reg_blank]
    movd       %1, mm1
%endmacro

%macro Interp4 3
    movd       mm1, eax
    movd       mm2, %2
    movd       mm3, %3
    punpcklbw  mm1, [reg_blank]
    punpcklbw  mm2, [reg_blank]
    punpcklbw  mm3, [reg_blank]
    psllw      mm1, 1
    paddw      mm2, mm3
    pmullw     mm2, [const7]
    paddw      mm1, mm2
    psrlw      mm1, 4
    packuswb   mm1, [reg_blank]
    movd       %1, mm1
%endmacro

%macro Interp5 3
    mov edx,%2
    add edx,%3
    shr edx,1
    mov %1,edx
%endmacro

%macro PIXEL00_1M 0
    Interp1 [edi],eax,[c1]
%endmacro

%macro PIXEL00_1U 0
    Interp1 [edi],eax,[c2]
%endmacro

%macro PIXEL00_1L 0
    Interp1 [edi],eax,[c4]
%endmacro

%macro PIXEL00_2 0
    Interp2 [edi],eax,[c4],[c2]
%endmacro

%macro PIXEL00_4 0
    Interp4 [edi],[c4],[c2]
%endmacro

%macro PIXEL00_5 0
    Interp5 [edi],[c4],[c2]
%endmacro

%macro PIXEL00_C 0
    mov [edi],eax
%endmacro

%macro PIXEL01_1 0
    Interp1 [edi+4],eax,[c2]
%endmacro

%macro PIXEL01_3 0
    Interp3 [edi+4],[c2]
%endmacro

%macro PIXEL01_6 0
    Interp1 [edi+4],[c2],eax
%endmacro

%macro PIXEL01_C 0
    mov [edi+4],eax
%endmacro

%macro PIXEL02_1M 0
    Interp1 [edi+8],eax,[c3]
%endmacro

%macro PIXEL02_1U 0
    Interp1 [edi+8],eax,[c2]
%endmacro

%macro PIXEL02_1R 0
    Interp1 [edi+8],eax,[c6]
%endmacro

%macro PIXEL02_2 0
    Interp2 [edi+8],eax,[c2],[c6]
%endmacro

%macro PIXEL02_4 0
    Interp4 [edi+8],[c2],[c6]
%endmacro

%macro PIXEL02_5 0
    Interp5 [edi+8],[c2],[c6]
%endmacro

%macro PIXEL02_C 0
    mov [edi+8],eax
%endmacro

%macro PIXEL10_1 0
    Interp1 [edi+ebx],eax,[c4]
%endmacro

%macro PIXEL10_3 0
    Interp3 [edi+ebx],[c4]
%endmacro

%macro PIXEL10_6 0
    Interp1 [edi+ebx],[c4],eax
%endmacro

%macro PIXEL10_C 0
    mov [edi+ebx],eax
%endmacro

%macro PIXEL11 0
    mov [edi+ebx+4],eax
%endmacro

%macro PIXEL12_1 0
    Interp1 [edi+ebx+8],eax,[c6]
%endmacro

%macro PIXEL12_3 0
    Interp3 [edi+ebx+8],[c6]
%endmacro

%macro PIXEL12_6 0
    Interp1 [edi+ebx+8],[c6],eax
%endmacro

%macro PIXEL12_C 0
    mov [edi+ebx+8],eax
%endmacro

%macro PIXEL20_1M 0
    Interp1 [edi+ebx*2],eax,[c7]
%endmacro

%macro PIXEL20_1D 0
    Interp1 [edi+ebx*2],eax,[c8]
%endmacro

%macro PIXEL20_1L 0
    Interp1 [edi+ebx*2],eax,[c4]
%endmacro

%macro PIXEL20_2 0
    Interp2 [edi+ebx*2],eax,[c8],[c4]
%endmacro

%macro PIXEL20_4 0
    Interp4 [edi+ebx*2],[c8],[c4]
%endmacro

%macro PIXEL20_5 0
    Interp5 [edi+ebx*2],[c8],[c4]
%endmacro

%macro PIXEL20_C 0
    mov [edi+ebx*2],eax
%endmacro

%macro PIXEL21_1 0
    Interp1 [edi+ebx*2+4],eax,[c8]
%endmacro

%macro PIXEL21_3 0
    Interp3 [edi+ebx*2+4],[c8]
%endmacro

%macro PIXEL21_6 0
    Interp1 [edi+ebx*2+4],[c8],eax
%endmacro

%macro PIXEL21_C 0
    mov [edi+ebx*2+4],eax
%endmacro

%macro PIXEL22_1M 0
    Interp1 [edi+ebx*2+8],eax,[c9]
%endmacro

%macro PIXEL22_1D 0
    Interp1 [edi+ebx*2+8],eax,[c8]
%endmacro

%macro PIXEL22_1R 0
    Interp1 [edi+ebx*2+8],eax,[c6]
%endmacro

%macro PIXEL22_2 0
    Interp2 [edi+ebx*2+8],eax,[c6],[c8]
%endmacro

%macro PIXEL22_4 0
    Interp4 [edi+ebx*2+8],[c6],[c8]
%endmacro

%macro PIXEL22_5 0
    Interp5 [edi+ebx*2+8],[c6],[c8]
%endmacro

%macro PIXEL22_C 0
    mov [edi+ebx*2+8],eax
%endmacro

inbuffer     equ 8
outbuffer    equ 12
Xres         equ 16
Yres         equ 20
pitch        equ 24

_hq3x_32:
    push ebp
    mov ebp,esp
    pushad

    mov esi,[ebp+inbuffer]
    mov edi,[ebp+outbuffer]
    mov edx,[ebp+Yres]
    mov [linesleft],edx
    mov dword[firstline],1
.loopy
	push edi
    mov ebx,[ebp+Xres]
    shl ebx,1
    movzx eax,word[esi]
    mov [w5],eax
    mov [w6],eax
    mov edx,eax
    xor ecx,ecx
    cmp dword[firstline],1
    je  .firstline
    sub ecx,ebx
    movzx eax,word[esi+ecx]
.firstline
    mov [prevline],ecx
    mov [w2],eax
    mov [w3],eax
    xor ecx,ecx
    cmp dword[linesleft],1
    je  .lastline
    add ecx,ebx
    movzx edx,word[esi+ecx]
.lastline
    mov [nextline],ecx
    mov [w8],edx
    mov [w9],edx
    mov ecx,[ebp+Xres]
    mov dword[xcounter],ecx
.loopx
    mov eax,[w2]
    mov ebx,[w5]
    mov edx,[w8]
    mov [w1],eax
    mov [w4],ebx
    mov [w7],edx
    mov eax,[w3]
    mov ebx,[w6]
    mov edx,[w9]
    mov [w2],eax
    mov [w5],ebx
    mov [w8],edx
    cmp dword[xcounter],1
    je .flags
    movzx eax,word[esi+2]
    mov [w6],eax
    mov ebx,[prevline]
    movzx edx,word[esi+ebx+2]
    mov [w3],edx
    mov ebx,[nextline]
    movzx eax,word[esi+ebx+2]
    mov [w9],eax
.flags
    mov     ebx,_RGBtoYUV
    mov     eax,[w5]
    xor     ecx,ecx
    movd    mm5,[ebx+eax*4]
    mov     dword[cross],0

    mov     edx,[w2]
    cmp     eax,edx
    je      .noflag2
    or      dword[cross],1
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag2
    or      ecx,2
.noflag2
    mov     edx,[w4]
    cmp     eax,edx
    je      .noflag4
    or      dword[cross],2
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag4
    or      ecx,8
.noflag4
    mov     edx,[w6]
    cmp     eax,edx
    je      .noflag6
    or      dword[cross],4
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag6
    or      ecx,16
.noflag6
    mov     edx,[w8]
    cmp     eax,edx
    je      .noflag8
    or      dword[cross],8
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag8
    or      ecx,64
.noflag8
    test    ecx,ecx
    jnz     .testflag1
    mov     ecx,[cross]
    mov     ebx,_LUT16to32
    mov     eax,[ebx+eax*4]
    jmp     [FuncTable2+ecx*4]
.testflag1
    mov     edx,[w1]
    cmp     eax,edx
    je      .noflag1
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag1
    or      ecx,1
.noflag1
    mov     edx,[w3]
    cmp     eax,edx
    je      .noflag3
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag3
    or      ecx,4
.noflag3
    mov     edx,[w7]
    cmp     eax,edx
    je      .noflag7
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag7
    or      ecx,32
.noflag7
    mov     edx,[w9]
    cmp     eax,edx
    je      .noflag9
    movq    mm1,mm5
    movd    mm2,[ebx+edx*4]
    psubusb mm1,mm2
    psubusb mm2,mm5
    por     mm1,mm2
    psubusb mm1,[threshold]
    movd    edx,mm1
    test    edx,edx
    jz      .noflag9
    or      ecx,128
.noflag9
    mov ebx,_LUT16to32
    mov eax,[ebx+eax*4]
    mov edx,[w2]
    mov edx,[ebx+edx*4]
    mov [c2],edx
    mov edx,[w4]
    mov edx,[ebx+edx*4]
    mov [c4],edx
    mov edx,[w6]
    mov edx,[ebx+edx*4]
    mov [c6],edx
    mov edx,[w8]
    mov edx,[ebx+edx*4]
    mov [c8],edx
    test ecx,0x005A
    jz  .switch
    mov edx,[w1]
    mov edx,[ebx+edx*4]
    mov [c1],edx
    mov edx,[w3]
    mov edx,[ebx+edx*4]
    mov [c3],edx
    mov edx,[w7]
    mov edx,[ebx+edx*4]
    mov [c7],edx
    mov edx,[w9]
    mov edx,[ebx+edx*4]
    mov [c9],edx
.switch
    mov ebx,[ebp+pitch]
    jmp [FuncTable+ecx*4]

..@flag0
..@flag1
..@flag4
..@flag32
..@flag128
..@flag5
..@flag132
..@flag160
..@flag33
..@flag129
..@flag36
..@flag133
..@flag164
..@flag161
..@flag37
..@flag165
;    PIXEL00_2
;    PIXEL01_1
;    PIXEL02_2
;    PIXEL10_1
;    PIXEL11
;    PIXEL12_1
;    PIXEL20_2
;    PIXEL21_1
;    PIXEL22_2

;   the same, only optimized
    mov ecx,eax
    shl ecx,1
    add ecx,[c2]
    mov edx,ecx
    add edx,[c4]
    shr edx,2
    mov [edi],edx
    mov edx,ecx
    add edx,eax
    shr edx,2
    mov [edi+4],edx
    add ecx,[c6]
    shr ecx,2
    mov [edi+8],ecx
    mov ecx,eax
    shl ecx,2
    sub ecx,eax
    mov edx,ecx
    add edx,[c4]
    shr edx,2
    mov [edi+ebx],edx
    mov [edi+ebx+4],eax
    add ecx,[c6]
    shr ecx,2
    mov [edi+ebx+8],ecx
    mov ecx,eax
    shl ecx,1
    add ecx,[c8]
    mov edx,ecx
    add edx,[c4]
    shr edx,2
    mov [edi+ebx*2],edx
    mov edx,ecx
    add edx,eax
    shr edx,2
    mov [edi+ebx*2+4],edx
    add ecx,[c6]
    shr ecx,2
    mov [edi+ebx*2+8],ecx
    jmp .loopx_end
..@flag2
..@flag34
..@flag130
..@flag162
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_2
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag16
..@flag17
..@flag48
..@flag49
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag64
..@flag65
..@flag68
..@flag69
    PIXEL00_2
    PIXEL01_1
    PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag8
..@flag12
..@flag136
..@flag140
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag3
..@flag35
..@flag131
..@flag163
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_2
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag6
..@flag38
..@flag134
..@flag166
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_2
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag20
..@flag21
..@flag52
..@flag53
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag144
..@flag145
..@flag176
..@flag177
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag192
..@flag193
..@flag196
..@flag197
    PIXEL00_2
    PIXEL01_1
    PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag96
..@flag97
..@flag100
..@flag101
    PIXEL00_2
    PIXEL01_1
    PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag40
..@flag44
..@flag168
..@flag172
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag9
..@flag13
..@flag137
..@flag141
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag18
..@flag50
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_1M,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_1
    PIXEL11
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag80
..@flag81
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_1M,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag72
..@flag76
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_2
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_1M,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag10
..@flag138
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag66
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag24
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag7
..@flag39
..@flag135
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_2
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag148
..@flag149
..@flag180
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag224
..@flag228
..@flag225
    PIXEL00_2
    PIXEL01_1
    PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag41
..@flag169
..@flag45
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag22
..@flag54
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_1
    PIXEL11
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag208
..@flag209
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag104
..@flag108
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_2
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag11
..@flag139
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag19
..@flag51
    DiffOrNot w2,w6,PIXEL00_1L,PIXEL01_C,PIXEL02_1M,PIXEL12_C,PIXEL00_2,PIXEL01_6,PIXEL02_5,PIXEL12_1
    PIXEL10_1
    PIXEL11
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag146
..@flag178
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_1M,PIXEL12_C,PIXEL22_1D,PIXEL01_1,PIXEL02_5,PIXEL12_6,PIXEL22_2
    PIXEL00_1M
    PIXEL10_1
    PIXEL11
    PIXEL20_2
    PIXEL21_1
    jmp .loopx_end
..@flag84
..@flag85
    DiffOrNot w6,w8,PIXEL02_1U,PIXEL12_C,PIXEL21_C,PIXEL22_1M,PIXEL02_2,PIXEL12_6,PIXEL21_1,PIXEL22_5
    PIXEL00_2
    PIXEL01_1
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    jmp .loopx_end
..@flag112
..@flag113
    DiffOrNot w6,w8,PIXEL12_C,PIXEL20_1L,PIXEL21_C,PIXEL22_1M,PIXEL12_1,PIXEL20_2,PIXEL21_6,PIXEL22_5
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    jmp .loopx_end
..@flag200
..@flag204
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_1M,PIXEL21_C,PIXEL22_1R,PIXEL10_1,PIXEL20_5,PIXEL21_6,PIXEL22_2
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_2
    PIXEL11
    PIXEL12_1
    jmp .loopx_end
..@flag73
..@flag77
    DiffOrNot w8,w4,PIXEL00_1U,PIXEL10_C,PIXEL20_1M,PIXEL21_C,PIXEL00_2,PIXEL10_6,PIXEL20_5,PIXEL21_1
    PIXEL01_1
    PIXEL02_2
    PIXEL11
    PIXEL12_1
    PIXEL22_1M
    jmp .loopx_end
..@flag42
..@flag170
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL01_C,PIXEL10_C,PIXEL20_1D,PIXEL00_5,PIXEL01_1,PIXEL10_6,PIXEL20_2
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag14
..@flag142
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL01_C,PIXEL02_1R,PIXEL10_C,PIXEL00_5,PIXEL01_6,PIXEL02_2,PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag67
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag70
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag28
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag152
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag194
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag98
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag56
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag25
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag26
..@flag31
    DiffOrNot w4,w2,PIXEL00_C,PIXEL10_C,PIXEL00_4,PIXEL10_3
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL12_C,PIXEL02_4,PIXEL12_3
    PIXEL11
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag82
..@flag214
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL01_3,PIXEL02_4
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL21_C,PIXEL22_C,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag88
..@flag248
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL11
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL10_3,PIXEL20_4
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL12_C,PIXEL22_C,PIXEL12_3,PIXEL22_4
    jmp .loopx_end
..@flag74
..@flag107
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL00_4,PIXEL01_3
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_C,PIXEL21_C,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag27
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag86
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag216
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag106
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag30
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_C
    PIXEL11
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag210
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag120
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag75
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag29
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag198
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag184
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag99
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag57
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag71
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag156
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag226
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag60
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag195
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag102
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag153
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag58
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag83
    PIXEL00_1L
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag92
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag202
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag78
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag154
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag114
    PIXEL00_1M
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1L
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag89
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag90
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag55
..@flag23
    DiffOrNot w2,w6,PIXEL00_1L,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL00_2,PIXEL01_6,PIXEL02_5,PIXEL12_1
    PIXEL10_1
    PIXEL11
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag182
..@flag150
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL22_1D,PIXEL01_1,PIXEL02_5,PIXEL12_6,PIXEL22_2
    PIXEL00_1M
    PIXEL10_1
    PIXEL11
    PIXEL20_2
    PIXEL21_1
    jmp .loopx_end
..@flag213
..@flag212
    DiffOrNot w6,w8,PIXEL02_1U,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL02_2,PIXEL12_6,PIXEL21_1,PIXEL22_5
    PIXEL00_2
    PIXEL01_1
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    jmp .loopx_end
..@flag241
..@flag240
    DiffOrNot w6,w8,PIXEL12_C,PIXEL20_1L,PIXEL21_C,PIXEL22_C,PIXEL12_1,PIXEL20_2,PIXEL21_6,PIXEL22_5
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    jmp .loopx_end
..@flag236
..@flag232
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL22_1R,PIXEL10_1,PIXEL20_5,PIXEL21_6,PIXEL22_2
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_2
    PIXEL11
    PIXEL12_1
    jmp .loopx_end
..@flag109
..@flag105
    DiffOrNot w8,w4,PIXEL00_1U,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL00_2,PIXEL10_6,PIXEL20_5,PIXEL21_1
    PIXEL01_1
    PIXEL02_2
    PIXEL11
    PIXEL12_1
    PIXEL22_1M
    jmp .loopx_end
..@flag171
..@flag43
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL20_1D,PIXEL00_5,PIXEL01_1,PIXEL10_6,PIXEL20_2
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag143
..@flag15
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL02_1R,PIXEL10_C,PIXEL00_5,PIXEL01_6,PIXEL02_2,PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag124
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag203
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag62
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_C
    PIXEL11
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag211
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag118
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_1
    PIXEL11
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag217
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag110
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag155
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag188
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag185
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag61
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag157
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag103
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag227
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag230
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag199
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag220
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag158
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_C
    PIXEL11
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag234
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    PIXEL02_1M
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1R
    jmp .loopx_end
..@flag242
    PIXEL00_1M
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL20_1L
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag59
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag121
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag87
    PIXEL00_1L
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_1
    PIXEL11
    PIXEL20_1M
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag79
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1R
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag122
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag94
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL10_C
    PIXEL11
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag218
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_C
    PIXEL11
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag91
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag229
    PIXEL00_2
    PIXEL01_1
    PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag167
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_2
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag173
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag181
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag186
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag115
    PIXEL00_1L
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1L
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag93
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag206
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag205
..@flag201
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_1M,PIXEL20_2
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag174
..@flag46
    DiffOrNot w4,w2,PIXEL00_1M,PIXEL00_2
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag179
..@flag147
    PIXEL00_1L
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_1M,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag117
..@flag116
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1L
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_1M,PIXEL22_2
    jmp .loopx_end
..@flag189
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag231
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag126
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_4,PIXEL12_3
    PIXEL11
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag219
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_4,PIXEL01_3,PIXEL10_3
    PIXEL02_1M
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag125
    DiffOrNot w8,w4,PIXEL00_1U,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL00_2,PIXEL10_6,PIXEL20_5,PIXEL21_1
    PIXEL01_1
    PIXEL02_1U
    PIXEL11
    PIXEL12_C
    PIXEL22_1M
    jmp .loopx_end
..@flag221
    DiffOrNot w6,w8,PIXEL02_1U,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL02_2,PIXEL12_6,PIXEL21_1,PIXEL22_5
    PIXEL00_1U
    PIXEL01_1
    PIXEL10_C
    PIXEL11
    PIXEL20_1M
    jmp .loopx_end
..@flag207
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL02_1R,PIXEL10_C,PIXEL00_5,PIXEL01_6,PIXEL02_2,PIXEL10_1
    PIXEL11
    PIXEL12_1
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag238
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL22_1R,PIXEL10_1,PIXEL20_5,PIXEL21_6,PIXEL22_2
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1R
    PIXEL11
    PIXEL12_1
    jmp .loopx_end
..@flag190
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL22_1D,PIXEL01_1,PIXEL02_5,PIXEL12_6,PIXEL22_2
    PIXEL00_1M
    PIXEL10_C
    PIXEL11
    PIXEL20_1D
    PIXEL21_1
    jmp .loopx_end
..@flag187
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL20_1D,PIXEL00_5,PIXEL01_1,PIXEL10_6,PIXEL20_2
    PIXEL02_1M
    PIXEL11
    PIXEL12_C
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag243
    DiffOrNot w6,w8,PIXEL12_C,PIXEL20_1L,PIXEL21_C,PIXEL22_C,PIXEL12_1,PIXEL20_2,PIXEL21_6,PIXEL22_5
    PIXEL00_1L
    PIXEL01_C
    PIXEL02_1M
    PIXEL10_1
    PIXEL11
    jmp .loopx_end
..@flag119
    DiffOrNot w2,w6,PIXEL00_1L,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL00_2,PIXEL01_6,PIXEL02_5,PIXEL12_1
    PIXEL10_1
    PIXEL11
    PIXEL20_1L
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag237
..@flag233
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_C,PIXEL20_2
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag175
..@flag47
    DiffOrNot w4,w2,PIXEL00_C,PIXEL00_2
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_2
    jmp .loopx_end
..@flag183
..@flag151
    PIXEL00_1L
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_2
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag245
..@flag244
    PIXEL00_2
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1L
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_C,PIXEL22_2
    jmp .loopx_end
..@flag250
    PIXEL00_1M
    PIXEL01_C
    PIXEL02_1M
    PIXEL11
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL10_3,PIXEL20_4
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL12_C,PIXEL22_C,PIXEL12_3,PIXEL22_4
    jmp .loopx_end
..@flag123
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL00_4,PIXEL01_3
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_C,PIXEL21_C,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag95
    DiffOrNot w4,w2,PIXEL00_C,PIXEL10_C,PIXEL00_4,PIXEL10_3
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL12_C,PIXEL02_4,PIXEL12_3
    PIXEL11
    PIXEL20_1M
    PIXEL21_C
    PIXEL22_1M
    jmp .loopx_end
..@flag222
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL01_3,PIXEL02_4
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL21_C,PIXEL22_C,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag252
    PIXEL00_1M
    PIXEL01_1
    PIXEL02_1U
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL10_3,PIXEL20_4
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_C,PIXEL22_2
    jmp .loopx_end
..@flag249
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    DiffOrNot w8,w4,PIXEL20_C,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL12_C,PIXEL22_C,PIXEL12_3,PIXEL22_4
    jmp .loopx_end
..@flag235
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL00_4,PIXEL01_3
    PIXEL02_1M
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_C,PIXEL20_2
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag111
    DiffOrNot w4,w2,PIXEL00_C,PIXEL00_2
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_C,PIXEL21_C,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag63
    DiffOrNot w4,w2,PIXEL00_C,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL12_C,PIXEL02_4,PIXEL12_3
    PIXEL10_C
    PIXEL11
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1M
    jmp .loopx_end
..@flag159
    DiffOrNot w4,w2,PIXEL00_C,PIXEL10_C,PIXEL00_4,PIXEL10_3
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL02_2
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag215
    PIXEL00_1L
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL21_C,PIXEL22_C,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag246
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL01_3,PIXEL02_4
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1L
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_C,PIXEL22_2
    jmp .loopx_end
..@flag254
    PIXEL00_1M
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL01_3,PIXEL02_4
    PIXEL11
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL10_3,PIXEL20_4
    DiffOrNot w6,w8,PIXEL12_C,PIXEL21_C,PIXEL22_C,PIXEL12_3,PIXEL21_3,PIXEL22_2
    jmp .loopx_end
..@flag253
    PIXEL00_1U
    PIXEL01_1
    PIXEL02_1U
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_C,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_C,PIXEL22_2
    jmp .loopx_end
..@flag251
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL00_4,PIXEL01_3
    PIXEL02_1M
    PIXEL11
    DiffOrNot w8,w4,PIXEL10_C,PIXEL20_C,PIXEL21_C,PIXEL10_3,PIXEL20_2,PIXEL21_3
    DiffOrNot w6,w8,PIXEL12_C,PIXEL22_C,PIXEL12_3,PIXEL22_4
    jmp .loopx_end
..@flag239
    DiffOrNot w4,w2,PIXEL00_C,PIXEL00_2
    PIXEL01_C
    PIXEL02_1R
    PIXEL10_C
    PIXEL11
    PIXEL12_1
    DiffOrNot w8,w4,PIXEL20_C,PIXEL20_2
    PIXEL21_C
    PIXEL22_1R
    jmp .loopx_end
..@flag127
    DiffOrNot w4,w2,PIXEL00_C,PIXEL01_C,PIXEL10_C,PIXEL00_2,PIXEL01_3,PIXEL10_3
    DiffOrNot w2,w6,PIXEL02_C,PIXEL12_C,PIXEL02_4,PIXEL12_3
    PIXEL11
    DiffOrNot w8,w4,PIXEL20_C,PIXEL21_C,PIXEL20_4,PIXEL21_3
    PIXEL22_1M
    jmp .loopx_end
..@flag191
    DiffOrNot w4,w2,PIXEL00_C,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    PIXEL20_1D
    PIXEL21_1
    PIXEL22_1D
    jmp .loopx_end
..@flag223
    DiffOrNot w4,w2,PIXEL00_C,PIXEL10_C,PIXEL00_4,PIXEL10_3
    DiffOrNot w2,w6,PIXEL01_C,PIXEL02_C,PIXEL12_C,PIXEL01_3,PIXEL02_2,PIXEL12_3
    PIXEL11
    PIXEL20_1M
    DiffOrNot w6,w8,PIXEL21_C,PIXEL22_C,PIXEL21_3,PIXEL22_4
    jmp .loopx_end
..@flag247
    PIXEL00_1L
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL02_2
    PIXEL10_1
    PIXEL11
    PIXEL12_C
    PIXEL20_1L
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_C,PIXEL22_2
    jmp .loopx_end
..@flag255
    DiffOrNot w4,w2,PIXEL00_C,PIXEL00_2
    PIXEL01_C
    DiffOrNot w2,w6,PIXEL02_C,PIXEL02_2
    PIXEL10_C
    PIXEL11
    PIXEL12_C
    DiffOrNot w8,w4,PIXEL20_C,PIXEL20_2
    PIXEL21_C
    DiffOrNot w6,w8,PIXEL22_C,PIXEL22_2
    jmp .loopx_end

..@cross0
    mov     ebx,[ebp+pitch]
    mov     [edi],eax
    mov     [edi+4],eax
    mov     [edi+8],eax
    mov     [edi+ebx],eax
    mov     [edi+ebx+4],eax
    mov     [edi+ebx+8],eax
    mov     [edi+ebx*2],eax
    mov     [edi+ebx*2+4],eax
    mov     [edi+ebx*2+8],eax
    jmp     .loopx_end
..@cross1
    mov     ecx,[w2]
    mov     edx,eax
    shl     edx,2
    add     edx,[ebx+ecx*4]
    sub     edx,eax
    shr     edx,2
    mov     ebx,[ebp+pitch]
    mov     [edi],edx
    mov     [edi+4],edx
    mov     [edi+8],edx
    mov     [edi+ebx],eax
    mov     [edi+ebx+4],eax
    mov     [edi+ebx+8],eax
    mov     [edi+ebx*2],eax
    mov     [edi+ebx*2+4],eax
    mov     [edi+ebx*2+8],eax
    jmp     .loopx_end
..@cross2
    mov     ecx,[w4]
    mov     edx,eax
    shl     edx,2
    add     edx,[ebx+ecx*4]
    sub     edx,eax
    shr     edx,2
    mov     ebx,[ebp+pitch]
    mov     [edi],edx
    mov     [edi+4],eax
    mov     [edi+8],eax
    mov     [edi+ebx],edx
    mov     [edi+ebx+4],eax
    mov     [edi+ebx+8],eax
    mov     [edi+ebx*2],edx
    mov     [edi+ebx*2+4],eax
    mov     [edi+ebx*2+8],eax
    jmp     .loopx_end
..@cross4
    mov     ecx,[w6]
    mov     edx,eax
    shl     edx,2
    add     edx,[ebx+ecx*4]
    sub     edx,eax
    shr     edx,2
    mov     ebx,[ebp+pitch]
    mov     [edi],eax
    mov     [edi+4],eax
    mov     [edi+8],edx
    mov     [edi+ebx],eax
    mov     [edi+ebx+4],eax
    mov     [edi+ebx+8],edx
    mov     [edi+ebx*2],eax
    mov     [edi+ebx*2+4],eax
    mov     [edi+ebx*2+8],edx
    jmp     .loopx_end
..@cross8
    mov     ecx,[w8]
    mov     edx,eax
    shl     edx,2
    add     edx,[ebx+ecx*4]
    sub     edx,eax
    shr     edx,2
    mov     ebx,[ebp+pitch]
    mov     [edi],eax
    mov     [edi+4],eax
    mov     [edi+8],eax
    mov     [edi+ebx],eax
    mov     [edi+ebx+4],eax
    mov     [edi+ebx+8],eax
    mov     [edi+ebx*2],edx
    mov     [edi+ebx*2+4],edx
    mov     [edi+ebx*2+8],edx
    jmp     .loopx_end
..@crossN
    mov     edx,[w2]
    mov     ecx,[ebx+edx*4]
    mov     [c2],ecx
    mov     edx,[w4]
    mov     ecx,[ebx+edx*4]
    mov     [c4],ecx
    mov     edx,[w6]
    mov     ecx,[ebx+edx*4]
    mov     [c6],ecx
    mov     edx,[w8]
    mov     ecx,[ebx+edx*4]
    mov     [c8],ecx
    mov     ebx,[ebp+pitch]
    jmp     ..@flag0

.loopx_end
    add esi,2
    add edi,12
    dec dword[xcounter]
    jz  .nexty
    jmp .loopx
.nexty
    mov dword[firstline],0

	pop edi
	add edi,ebx
	add edi,ebx
	add edi,ebx

    dec dword[linesleft]
    jz  .fin
    jmp .loopy
.fin
    emms
    popad
    mov esp,ebp
    pop ebp
    ret

SECTION .data
FuncTable
    dd ..@flag0, ..@flag1, ..@flag2, ..@flag3, ..@flag4, ..@flag5, ..@flag6, ..@flag7
    dd ..@flag8, ..@flag9, ..@flag10, ..@flag11, ..@flag12, ..@flag13, ..@flag14, ..@flag15
    dd ..@flag16, ..@flag17, ..@flag18, ..@flag19, ..@flag20, ..@flag21, ..@flag22, ..@flag23
    dd ..@flag24, ..@flag25, ..@flag26, ..@flag27, ..@flag28, ..@flag29, ..@flag30, ..@flag31
    dd ..@flag32, ..@flag33, ..@flag34, ..@flag35, ..@flag36, ..@flag37, ..@flag38, ..@flag39
    dd ..@flag40, ..@flag41, ..@flag42, ..@flag43, ..@flag44, ..@flag45, ..@flag46, ..@flag47
    dd ..@flag48, ..@flag49, ..@flag50, ..@flag51, ..@flag52, ..@flag53, ..@flag54, ..@flag55
    dd ..@flag56, ..@flag57, ..@flag58, ..@flag59, ..@flag60, ..@flag61, ..@flag62, ..@flag63
    dd ..@flag64, ..@flag65, ..@flag66, ..@flag67, ..@flag68, ..@flag69, ..@flag70, ..@flag71
    dd ..@flag72, ..@flag73, ..@flag74, ..@flag75, ..@flag76, ..@flag77, ..@flag78, ..@flag79
    dd ..@flag80, ..@flag81, ..@flag82, ..@flag83, ..@flag84, ..@flag85, ..@flag86, ..@flag87
    dd ..@flag88, ..@flag89, ..@flag90, ..@flag91, ..@flag92, ..@flag93, ..@flag94, ..@flag95
    dd ..@flag96, ..@flag97, ..@flag98, ..@flag99, ..@flag100, ..@flag101, ..@flag102, ..@flag103
    dd ..@flag104, ..@flag105, ..@flag106, ..@flag107, ..@flag108, ..@flag109, ..@flag110, ..@flag111
    dd ..@flag112, ..@flag113, ..@flag114, ..@flag115, ..@flag116, ..@flag117, ..@flag118, ..@flag119
    dd ..@flag120, ..@flag121, ..@flag122, ..@flag123, ..@flag124, ..@flag125, ..@flag126, ..@flag127
    dd ..@flag128, ..@flag129, ..@flag130, ..@flag131, ..@flag132, ..@flag133, ..@flag134, ..@flag135
    dd ..@flag136, ..@flag137, ..@flag138, ..@flag139, ..@flag140, ..@flag141, ..@flag142, ..@flag143
    dd ..@flag144, ..@flag145, ..@flag146, ..@flag147, ..@flag148, ..@flag149, ..@flag150, ..@flag151
    dd ..@flag152, ..@flag153, ..@flag154, ..@flag155, ..@flag156, ..@flag157, ..@flag158, ..@flag159
    dd ..@flag160, ..@flag161, ..@flag162, ..@flag163, ..@flag164, ..@flag165, ..@flag166, ..@flag167
    dd ..@flag168, ..@flag169, ..@flag170, ..@flag171, ..@flag172, ..@flag173, ..@flag174, ..@flag175
    dd ..@flag176, ..@flag177, ..@flag178, ..@flag179, ..@flag180, ..@flag181, ..@flag182, ..@flag183
    dd ..@flag184, ..@flag185, ..@flag186, ..@flag187, ..@flag188, ..@flag189, ..@flag190, ..@flag191
    dd ..@flag192, ..@flag193, ..@flag194, ..@flag195, ..@flag196, ..@flag197, ..@flag198, ..@flag199
    dd ..@flag200, ..@flag201, ..@flag202, ..@flag203, ..@flag204, ..@flag205, ..@flag206, ..@flag207
    dd ..@flag208, ..@flag209, ..@flag210, ..@flag211, ..@flag212, ..@flag213, ..@flag214, ..@flag215
    dd ..@flag216, ..@flag217, ..@flag218, ..@flag219, ..@flag220, ..@flag221, ..@flag222, ..@flag223
    dd ..@flag224, ..@flag225, ..@flag226, ..@flag227, ..@flag228, ..@flag229, ..@flag230, ..@flag231
    dd ..@flag232, ..@flag233, ..@flag234, ..@flag235, ..@flag236, ..@flag237, ..@flag238, ..@flag239
    dd ..@flag240, ..@flag241, ..@flag242, ..@flag243, ..@flag244, ..@flag245, ..@flag246, ..@flag247
    dd ..@flag248, ..@flag249, ..@flag250, ..@flag251, ..@flag252, ..@flag253, ..@flag254, ..@flag255

FuncTable2
    dd ..@cross0, ..@cross1, ..@cross2, ..@crossN,
    dd ..@cross4, ..@crossN, ..@crossN, ..@crossN,
    dd ..@cross8, ..@crossN, ..@crossN, ..@crossN,
    dd ..@crossN, ..@crossN, ..@crossN, ..@crossN

