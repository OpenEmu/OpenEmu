;/*
;* Glide64 - Glide video plugin for Nintendo 64 emulators.
;*
;* This program is free software; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation; either version 2 of the License, or
;* any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public License
;* along with this program; if not, write to the Free Software
;* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;*/
;
;****************************************************************
;
; Glide64 - Glide Plugin for Nintendo 64 emulators
; Project started on December 29th, 2001
;
; Authors:
; Dave2001, original author, founded the project in 2001, left it in 2002
; Gugaman, joined the project in 2002, left it in 2002
; Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
; Hiroshi 'KoolSmoky' Morii, joined the project in 2007
;
;****************************************************************
;
; To modify Glide64:
; * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
; * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
;
;****************************************************************

%include "inc/c32.mac"

segment .text

proc DetectSIMD
      %$func    arg 
      %$iedx    arg 
      %$iecx    arg 
      mov       eax,[ebp + %$func]
      cpuid
      mov       eax,[ebp + %$iedx]
      mov       [eax],edx
      mov       eax,[ebp + %$iecx]
      mov       [eax],ecx
endproc ;DetectSIMD

;****************************************************************
;
;                     ******** SSE ********
;
;****************************************************************

proc TransformVectorSSE
CPU P3 
      %$src     arg           ; float *src     
      %$dst     arg           ; float *dst     
      %$mat     arg           ; float mat[4][4]

      mov       ecx,[ebp + %$src]
      mov       eax,[ebp + %$dst]
      mov       edx,[ebp + %$mat]
                   
      movss     xmm0,[ecx]    ; 0 0 0 src[0]
      movss     xmm5,[edx]    ; 0 0 0 mat[0][0]
      movhps    xmm5,[edx+4]  ; mat[0][2] mat[0][1] 0 mat[0][0]
      shufps    xmm0,xmm0, 0  ; src[0] src[0] src[0] src[0]
      movss     xmm1,[ecx+4]  ; 0 0 0 src[1]
      movss     xmm3,[edx+16] ; 0 0 0 mat[1][0]
      movhps    xmm3,[edx+20] ; mat[1][2] mat[1][1] 0 mat[1][0]
      shufps    xmm1,xmm1, 0  ; src[1] src[1] src[1] src[1]
      mulps     xmm0,xmm5     ; mat[0][2]*src[0] mat[0][1]*src[0] 0 mat[0][0]*src[0]
      mulps     xmm1,xmm3     ; mat[1][2]*src[1] mat[1][1]*src[1] 0 mat[1][0]*src[1]
      movss     xmm2,[ecx+8]  ; 0 0 0 src[2]
      shufps    xmm2,xmm2, 0  ; src[2] src[2] src[2] src[2]
      movss     xmm4,[edx+32] ; 0 0 0 mat[2][0]
      movhps    xmm4,[edx+36] ; mat[2][2] mat[2][1] 0 mat[2][0]
      addps     xmm0,xmm1     ; mat[0][2]*src[0]+mat[1][2]*src[1] mat[0][1]*src[0]+mat[1][1]*src[1] 0 mat[0][0]*src[0]+mat[1][0]*src[1]
      mulps     xmm2,xmm4     ; mat[2][2]*src[2] mat[2][1]*src[2] 0 mat[2][0]*src[2]
      addps     xmm0,xmm2     ; mat[0][2]*src[0]+mat[1][2]*src[1]+mat[2][2]*src[2] mat[0][1]*src[0]+mat[1][1]*src[1]+mat[2][1]*src[2] 0 mat[0][0]*src[0]+mat[1][0]*src[1]+mat[2][0]*src[2]
      movss     [eax],xmm0    ; mat[0][0]*src[0]+mat[1][0]*src[1]+mat[2][0]*src[2]
      movhps    [eax+4],xmm0  ; mat[0][2]*src[0]+mat[1][2]*src[1]+mat[2][2]*src[2] mat[0][1]*src[0]+mat[1][1]*src[1]+mat[2][1]*src[2]

endproc ;TransformVectorSSE

proc MulMatricesSSE
CPU P3 
      %$m1      arg  ; float m1[4][4] 
      %$m2      arg  ; float m2[4][4] 
      %$r       arg  ; float r[4][4]  

      mov       eax,[ebp + %$r]      
      mov       ecx,[ebp + %$m1]
      mov       edx,[ebp + %$m2]
      
      movaps    xmm0,[edx]
      movaps    xmm1,[edx+16]
      movaps    xmm2,[edx+32]
      movaps    xmm3,[edx+48]
      
      ; r[0][0],r[0][1],r[0][2],r[0][3]
      
      movaps    xmm4,[ecx]
      movaps    xmm5,xmm4
      movaps    xmm6,xmm4
      movaps    xmm7,xmm4
      
      shufps    xmm4,xmm4,00000000b
      shufps    xmm5,xmm5,01010101b
      shufps    xmm6,xmm6,10101010b
      shufps    xmm7,xmm7,11111111b
      
      mulps     xmm4,xmm0
      mulps     xmm5,xmm1
      mulps     xmm6,xmm2
      mulps     xmm7,xmm3
      
      addps     xmm4,xmm5
      addps     xmm4,xmm6
      addps     xmm4,xmm7
      
      movaps    [eax],xmm4
      
      ; r[1][0],r[1][1],r[1][2],r[1][3]
      
      movaps    xmm4,[ecx+16]
      movaps    xmm5,xmm4
      movaps    xmm6,xmm4
      movaps    xmm7,xmm4
      
      shufps    xmm4,xmm4,00000000b
      shufps    xmm5,xmm5,01010101b
      shufps    xmm6,xmm6,10101010b
      shufps    xmm7,xmm7,11111111b
      
      mulps     xmm4,xmm0
      mulps     xmm5,xmm1
      mulps     xmm6,xmm2
      mulps     xmm7,xmm3
      
      addps     xmm4,xmm5
      addps     xmm4,xmm6
      addps     xmm4,xmm7
      
      movaps    [eax+16],xmm4
      
      
      ; r[2][0],r[2][1],r[2][2],r[2][3]
      
      movaps    xmm4,[ecx+32]
      movaps    xmm5,xmm4
      movaps    xmm6,xmm4
      movaps    xmm7,xmm4
      
      shufps    xmm4,xmm4,00000000b
      shufps    xmm5,xmm5,01010101b
      shufps    xmm6,xmm6,10101010b
      shufps    xmm7,xmm7,11111111b
      
      mulps     xmm4,xmm0
      mulps     xmm5,xmm1
      mulps     xmm6,xmm2
      mulps     xmm7,xmm3
      
      addps     xmm4,xmm5
      addps     xmm4,xmm6
      addps     xmm4,xmm7
      
      movaps    [eax+32],xmm4
      
      ; r[3][0],r[3][1],r[3][2],r[3][3]
      
      movaps    xmm4,[ecx+48]
      movaps    xmm5,xmm4
      movaps    xmm6,xmm4
      movaps    xmm7,xmm4
      
      shufps    xmm4,xmm4,00000000b
      shufps    xmm5,xmm5,01010101b
      shufps    xmm6,xmm6,10101010b
      shufps    xmm7,xmm7,11111111b
      
      mulps     xmm4,xmm0
      mulps     xmm5,xmm1
      mulps     xmm6,xmm2
      mulps     xmm7,xmm3
      
      addps     xmm4,xmm5
      addps     xmm4,xmm6
      addps     xmm4,xmm7
      
      movaps    [eax+48],xmm4

endproc ;MulMatricesSSE

proc NormalizeVectorSSE
CPU P3
      %$v arg
      
      mov edx, [ebp + %$v]
      movaps xmm0, [edx]      ; x y z 0
      movaps xmm2, xmm0       ; x y z 0
      mulps  xmm0, xmm0       ; x*x y*y z*z 0
      movaps xmm1, xmm0       ; x*x y*y z*z 0
      shufps xmm0, xmm1, 0x4e ; z*z 0 x*x y*y
      addps  xmm0, xmm1       ; x*x+z*z y*y z*z+x*x y*y
      movaps xmm1, xmm0       ; x*x+z*z y*y z*z+x*x y*y
      shufps xmm1, xmm1, 0x11 ; y*y z*z+x*x y*y z*z+x*x
      addps  xmm0, xmm1       ; x*x+z*z+y*y
      rsqrtps xmm0, xmm0      ; 1.0/sqrt(x*x+z*z+y*y)
      mulps  xmm2, xmm0       ; x/sqrt(x*x+z*z+y*y) y/sqrt(x*x+z*z+y*y) z/sqrt(x*x+z*z+y*y) 0
      movaps [edx], xmm2
      
endproc ;NormalizeVectorSSE

;****************************************************************
;
;                     ******** SSE3 ********
;
;****************************************************************

proc DotProductSSE3
CPU PRESCOTT
      %$v1 arg
      %$v2 arg
      
      mov eax,[ebp + %$v1]
      mov edx,[ebp + %$v2]
      movaps xmm0, [eax]
      mulps xmm0, [edx]
      haddps xmm0, xmm0
      haddps xmm0, xmm0
;      movss eax, xmm0
      
endproc ;DotProductSSE3

;****************************************************************
;
;                     ******** 3DNOW ********
;
;****************************************************************

proc TransformVector3DNOW
CPU 586
      %$src       arg           ; float *src     
      %$dst       arg           ; float *dst     
      %$mat       arg           ; float mat[4][4]

    femms
      mov         ecx,[ebp + %$src]
      mov         eax,[ebp + %$dst]
      mov         edx,[ebp + %$mat]
      movq        mm0,[ecx]     ; src[1] src[0]
      movd        mm2,[ecx+8]   ; 0 src[2]
      movq        mm1,mm0       ; src[1] src[0]
      punpckldq   mm0,mm0       ; src[0] src[0]
      punpckhdq   mm1,mm1       ; src[1] src[1]
      punpckldq   mm2,mm2       ; src[2] src[2]
      movq        mm3,mm0       ; src[0] src[0]
      movq        mm4,mm1       ; src[1] src[1]
      movq        mm5,mm2       ; src[2] src[2]
      pfmul       mm0,[edx]     ; src[0]*mat[0][1] src[0]*mat[0][0]
      pfmul       mm3,[edx+8]   ; 0 src[0]*mat[0][2]
      pfmul       mm1,[edx+16]  ; src[1]*mat[1][1] src[1]*mat[1][0]
      pfmul       mm4,[edx+24]  ; 0 src[1]*mat[1][2]
      pfmul       mm2,[edx+32]  ; src[2]*mat[2][1] src[2]*mat[2][0]
      pfmul       mm5,[edx+40]  ; 0 src[2]*mat[2][2]
      pfadd       mm0,mm1       ; src[0]*mat[0][1]+src[1]*mat[1][1] src[0]*mat[0][0]+src[1]*mat[1][0]
      pfadd       mm3,mm4       ; 0 src[0]*mat[0][2]+src[1]*mat[1][2]
      pfadd       mm0,mm2       ; src[0]*mat[0][1]+src[1]*mat[1][1]+src[2]*mat[2][1] src[0]*mat[0][0]+src[1]*mat[1][0]+src[2]*mat[2][0]
      pfadd       mm3,mm5       ; 0 src[0]*mat[0][2]+src[1]*mat[1][2]+src[2]*mat[2][2]
      movq        [eax],mm0     ; mat[0][1]*src[0]+mat[1][1]*src[1]+mat[2][1]*src[2] mat[0][0]*src[0]+mat[1][0]*src[1]+mat[2][0]*src[2]
      movd        [eax+8],mm3   ; mat[0][2]*src[0]+mat[1][2]*src[1]+mat[2][2]*src[2]
      femms

endproc ;TransformVector3DNOW

proc InverseTransformVector3DNOW
CPU 586
      %$src       arg           ; float *src       
      %$dst       arg           ; float *dst       
      %$mat       arg           ; float mat[4][4]  

    femms
      mov         ecx,[ebp + %$src]
      mov         eax,[ebp + %$dst]
      mov         edx,[ebp + %$mat]
      movq        mm0,[ecx]     ; src[1] src[0]
      movd        mm4,[ecx+8]   ; 0 src[2]
      movq        mm1,mm0       ; src[1] src[0]
      pfmul       mm0,[edx]     ; src[1]*mat[0][1] src[0]*mat[0][0]
      movq        mm5,mm4       ; 0 src[2]
      pfmul       mm4,[edx+8]   ; 0 src[2]*mat[0][2]
      movq        mm2,mm1       ; src[1] src[0]
      pfmul       mm1,[edx+16]  ; src[1]*mat[1][1] src[0]*mat[1][0]
      movq        mm6,mm5       ; 0 src[2]
      pfmul       mm5,[edx+24]  ; 0 src[2]*mat[1][2]
      movq        mm3,mm2       ; src[1] src[0]
      pfmul       mm2,[edx+32]  ; src[1]*mat[2][1] src[0]*mat[2][0]
      movq        mm7,mm6       ; 0 src[2]
      pfmul       mm6,[edx+40]  ; 0 src[2]*mat[2][2]
      pfacc       mm0,mm4       ; src[2]*mat[0][2] src[1]*mat[0][1]+src[0]*mat[0][0]
      pfacc       mm1,mm5       ; src[2]*mat[1][2] src[1]*mat[1][1]+src[0]*mat[1][0]
      pfacc       mm2,mm6       ; src[2]*mat[2][2] src[1]*mat[2][1]+src[0]*mat[2][0]
      pfacc       mm0,mm1       ; src[2]*mat[1][2]+src[1]*mat[1][1]+src[0]*mat[1][0] src[2]*mat[0][2]+src[1]*mat[0][1]+src[0]*mat[0][0]
      pfacc       mm2,mm3       ; 0 src[2]*mat[2][2]+src[1]*mat[2][1]+src[0]*mat[2][0]
      movq        [eax],mm0     ; mat[1][0]*src[0]+mat[1][1]*src[1]+mat[1][2]*src[2] mat[0][0]*src[0]+mat[0][1]*src[1]+mat[0][2]*src[2]
      movd        [eax+8],mm2   ; mat[2][0]*src[0]+mat[2][1]*src[1]+mat[2][2]*src[2]
      femms                    

endproc ;InverseTransformVector3DNOW

proc MulMatrices3DNOW
CPU 586
      %$m1        arg    ; float m1[4][4] 
      %$m2        arg    ; float m2[4][4] 
      %$r         arg    ; float r[4][4]  

    femms
      mov         ecx,[ebp + %$m1]
      mov         eax,[ebp + %$r]
      mov         edx,[ebp + %$m2]
      
      movq        mm0,[ecx]
      movq        mm1,[ecx+8]
      movq        mm4,[edx]
      punpckhdq   mm2,mm0
      movq        mm5,[edx+16]
      punpckhdq   mm3,mm1
      movq        mm6,[edx+32]
      punpckldq   mm0,mm0
      punpckldq   mm1,mm1
      pfmul       mm4,mm0
      punpckhdq   mm2,mm2
      pfmul       mm0,[edx+8]
      movq        mm7,[edx+48]
      pfmul       mm5,mm2
      punpckhdq   mm3,mm3
      pfmul       mm2,[edx+24]
      pfmul       mm6,mm1
      pfadd       mm5,mm4
      pfmul       mm1,[edx+40]
      pfadd       mm2,mm0
      pfmul       mm7,mm3
      pfadd       mm6,mm5
      pfmul       mm3,[edx+56]
      pfadd       mm2,mm1
      pfadd       mm7,mm6
      movq        mm0,[ecx+16]
      pfadd       mm3,mm2
      movq        mm1,[ecx+24]
      movq        [eax],mm7
      movq        mm4,[edx]
      movq        [eax+8],mm3
      
      punpckhdq   mm2,mm0
      movq        mm5,[edx+16]
      punpckhdq   mm3,mm1
      movq        mm6,[edx+32]
      punpckldq   mm0,mm0
      punpckldq   mm1,mm1
      pfmul       mm4,mm0
      punpckhdq   mm2,mm2
      pfmul       mm0,[edx+8]
      movq        mm7,[edx+48]
      pfmul       mm5,mm2
      punpckhdq   mm3,mm3
      pfmul       mm2,[edx+24]
      pfmul       mm6,mm1
      pfadd       mm5,mm4
      pfmul       mm1,[edx+40]
      pfadd       mm2,mm0
      pfmul       mm7,mm3
      pfadd       mm6,mm5
      pfmul       mm3,[edx+56]
      pfadd       mm2,mm1
      pfadd       mm7,mm6
      movq        mm0,[ecx+32]
      pfadd       mm3,mm2
      movq        mm1,[ecx+40]
      movq        [eax+16],mm7
      movq        mm4,[edx]
      movq        [eax+24],mm3
      
      punpckhdq   mm2,mm0
      movq        mm5,[edx+16]
      punpckhdq   mm3,mm1
      movq        mm6,[edx+32]
      punpckldq   mm0,mm0
      punpckldq   mm1,mm1
      pfmul       mm4,mm0
      punpckhdq   mm2,mm2
      pfmul       mm0,[edx+8]
      movq        mm7,[edx+48]
      pfmul       mm5,mm2
      punpckhdq   mm3,mm3
      pfmul       mm2,[edx+24]
      pfmul       mm6,mm1
      pfadd       mm5,mm4
      pfmul       mm1,[edx+40]
      pfadd       mm2,mm0
      pfmul       mm7,mm3
      pfadd       mm6,mm5
      pfmul       mm3,[edx+56]
      pfadd       mm2,mm1
      pfadd       mm7,mm6
      movq        mm0,[ecx+48]
      pfadd       mm3,mm2
      movq        mm1,[ecx+56]
      movq        [eax+32],mm7
      movq        mm4,[edx]
      movq        [eax+40],mm3
      
      punpckhdq   mm2,mm0
      movq        mm5,[edx+16]
      punpckhdq   mm3,mm1
      movq        mm6,[edx+32]
      punpckldq   mm0,mm0
      punpckldq   mm1,mm1
      pfmul       mm4,mm0
      punpckhdq   mm2,mm2
      pfmul       mm0,[edx+8]
      movq        mm7,[edx+48]
      pfmul       mm5,mm2
      punpckhdq   mm3,mm3
      pfmul       mm2,[edx+24]
      pfmul       mm6,mm1
      pfadd       mm5,mm4
      pfmul       mm1,[edx+40]
      pfadd       mm2,mm0
      pfmul       mm7,mm3
      pfadd       mm6,mm5
      pfmul       mm3,[edx+56]
      pfadd       mm2,mm1
      pfadd       mm7,mm6
      pfadd       mm3,mm2
      movq        [eax+48],mm7
      movq        [eax+56],mm3
      femms

endproc ;MulMatrices3DNOW

proc DotProduct3DNOW
CPU 586
      %$v1        arg
      %$v2        arg
      
      femms
      mov         edx,[ebp + %$v1]
      mov         eax,[ebp + %$v2]
      movq        mm0,[edx]
      movq        mm3,[eax]
      pfmul       mm0,mm3
      movq        mm2,[edx+8]
      movq        mm1,[eax+8]
      pfacc       mm0,mm0
      pfmul       mm1,mm2
      pfadd       mm0,mm1
      movd        eax,mm0
      femms

endproc ;DotProduct3DNOW

proc NormalizeVector3DNOW
CPU 586
      %$v          arg
      
      femms
      mov          edx,[ebp + %$v]
      movq         mm0,[edx]
      movq         mm3,[edx+8]
      movq         mm1,mm0
      movq         mm2,mm3
      pfmul        mm0,mm0
      pfmul        mm3,mm3
      pfacc        mm0,mm0
      pfadd        mm0,mm3
      ;movq mm4,mm0 ; prepare for 24bit precision
      ;punpckldq mm4,mm4 ; prepare for 24bit precision
      pfrsqrt      mm0,mm0 ; 15bit precision 1/sqrtf(v)
      ;movq mm3,mm0
      ;pfmul mm0,mm0
      ;pfrsqit1 mm0,mm4
      ;pfrcpit2 mm0,mm3 ; 24bit precision 1/sqrtf(v)
      pfmul        mm1,mm0
      pfmul        mm2,mm0
      movq         [edx],mm1
      movq         [edx+8],mm2
      femms
      
endproc ;NormalizeVector3DNOW
