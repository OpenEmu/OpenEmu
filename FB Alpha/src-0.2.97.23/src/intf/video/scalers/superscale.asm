;SuperScale by ElSemi
;
;	Based on the original idea of 2xScale of AdvanceMAME.
;		AdvanceMAME code is GPLed and I didn't get permisson to use it. so this code is
;		my own implementation of that effect.
;	similar to Kawaks' KScale effect
;
;It expands the central pixel of a 3x3 matrix to 2x2 pixels resulting in a 2x scale in
;both axis.
;
;A B C		E0 E1
;D E F	->	E2 E3
;G H I
;
;
;Original algorithm (AdvanceMAME)
;
;E0 = (D == B && B != F && D != H) ? D : E;
;E1 = (B == F && B != D && F != H) ? F : E;
;E2 = (D == H && D != B && H != F) ? D : E;
;E3 = (H == F && D != H && B != F) ? F : E;
;
;
;(I'll only put the first 2 equations as the 3rd and 4th
;are symmetrical ABC<->GHI)
;
;Reduce variable usage
;
;D==B => E0=D -> E0=B
;	D!=H -> B!=H
;
;B==F => E1=F -> E1=B
;	F!=H -> B!=H
;
;Group and reorder
;
;E0=(B==D && B!=F && B!=H)?B:E;
;E1=(B!=D && B==F && B!=H)?B:E;
;
;as you can see, there are only 3 "real" conditions (B==D, B==F, and B!=H). the other
;2 can be get reversing the first 1 conditions (B!=D, B!=F) that makes the code
;suitable to pcmpeqw,pand,pandn

	BITS 32
	SECTION .text ALIGN = 32
	GLOBAL _superscale_line
	GLOBAL _superscale_line_75

%macro ALIGN32 0
        times ($$-$)&31 nop
%endmacro

_superscale_line:
	push ebp
	mov ebp,esp
	push eax
	push esi
	push ebx
	push edi
	push ecx

	mov       eax,[ebp+8h]		;ABC   ;line -1
	mov       esi,[ebp+0Ch]		;DEF   ;current line
	mov       ebx,[ebp+10h]		;GHI   ;line +1
	mov       edi,[ebp+14h]		;dst
	mov       ecx,[ebp+18h]		;count
	shr       ecx,2			;/4, we'll make 4 pixels per loop
	ALIGN32
iloop:
;This code makes 4 pixels per loop
;The comments refer only to 1 pixel (the 3rd one to make explanations clearer and 
;closer to the algorithm)
;					   v
	movq      mm0,[eax]		;xABC
;v	movq 	  mm1,[esi]		;xDEF
	movq      mm2,[ebx]		;xGHI
	movq      mm3,[esi-2]		;xxDE
	movq 	  mm1,[esi]		;xDEF
	movq      mm4,[esi+2]		;DEFx

;Prepare basic comparisons
	pcmpeqw   mm2,mm0		;mm2=xABC==xGHI (B==H)
	pcmpeqw   mm3,mm0		;mm3=xABC==xxDE	(B==D)
	pcmpeqw   mm4,mm0		;mm4=xABC==DEFx	(B==F)
	movq      mm5,mm2		;mm5=(B==H) (we need this for both pixel conditions)

;prepare pixel masks
	pandn     mm2,mm3		;mm2=(B==D) && (B!=H)
	pandn     mm5,mm4		;mm5=(B==F) && (B!=H)
	pandn     mm4,mm2		;mm4=(B==D) && (B!=H) && (B!=F)	-> left pixel mask
	pandn     mm3,mm5		;mm3=(B==F) && (B!=H) && (B!=D) -> right pixel mask

;Mask pixels and merge
	movq      mm2,mm4		;store to temp as I will overwrite them in the masking process
	movq      mm5,mm3		;
	pand      mm4,mm0		;Mask out pixels that don't satisfy the conditions
	pand      mm3,mm0		;leaving only the "new" pixels (E0,E1=B)
	pandn     mm2,mm1		;Invert the mask and merge to get the pixels that remain
	pandn     mm5,mm1		;unchanged (E0,E1=E)
	por       mm4,mm2		;merge both data to get the final pixels xxE0x
	por       mm3,mm5		;xxE1x
	movq      mm0,mm4		
					;merge both regs, interleaving E0 and E1 data
	punpcklwd mm4,mm3		;in 2 regs (mm4|mm0)
	punpckhwd mm0,mm3		;
	movq      [edi],mm4
	movq      [edi+8],mm0
	add       eax,8 
	add       esi,8 
	add       ebx,8 
	add       edi,16 
	loop	  iloop

	pop ecx
	pop edi
	pop ebx
	pop esi
	pop eax 
	pop ebp
	ret

ALIGN32

_superscale_line_75:    ;do a 75% reduction on the final pixel colour
	push ebp
	mov ebp,esp
	push eax
	push esi
	push ebx
	push edi
	push ecx

	mov       eax,[ebp+8h]		;ABC   ;line -1
	mov       esi,[ebp+0Ch]		;DEF   ;current line
	mov       ebx,[ebp+10h]		;GHI   ;line +1
	mov       edi,[ebp+14h]		;dst
    	mov       ecx,[ebp+1Ch]     ;mask
    	movq      mm7,[ecx]     ;
	mov       ecx,[ebp+18h]		;count
	shr       ecx,2			;/4, we'll make 4 pixels per loop
	ALIGN32
iloop2:
;This code makes 4 pixels per loop resulting in 8 pixels expansion
;The comments refer only to 1 pixel (the 3rd one to make explanations clearer and 
;closer to the algorithm)
;					   v
	movq      mm0,[eax]		;xABC
;v	movq 	  mm1,[esi]		;xDEF
	movq      mm2,[ebx]		;xGHI
	movq      mm3,[esi-2]		;xxDE
	movq 	  mm1,[esi]		;xDEF
	movq      mm4,[esi+2]		;DEFx

;Prepare basic conditions
	pcmpeqw   mm2,mm0		;mm2=xABC==xGHI (B==H)
	pcmpeqw   mm3,mm0		;mm3=xABC==xxDE	(B==D)
	pcmpeqw   mm4,mm0		;mm4=xABC==DEFx	(B==F)
	movq      mm5,mm2		;mm5=(B==H) (we need this for both pixel conditions)

;Prepare pixel masks
	pandn     mm2,mm3		;mm2=(B==D) && (B!=H)
	pandn     mm5,mm4		;mm5=(B==F) && (B!=H)
	pandn     mm4,mm2		;mm4=(B==D) && (B!=H) && (B!=F)	-> left pixel mask
	pandn     mm3,mm5		;mm3=(B==F) && (B!=H) && (B!=D) -> right pixel mask

;Mask pixels and merge
	movq      mm2,mm4		;store to temp as I will overwrite them in the masking process
	movq      mm5,mm3		;
	pand      mm4,mm0		;Mask out pixels that don't satisfy the conditions
	pand      mm3,mm0		;leaving only the "new" pixels (E0,E1=B)
	pandn     mm2,mm1		;Invert the mask and merge to get the pixels that remain
	pandn     mm5,mm1		;unchanged (E0,E1=E)
	por       mm4,mm2		;merge both data to get the final pixels xxE0x
	por       mm3,mm5		;xxE1x
	movq      mm0,mm4		;merge both regs, interleaving E0 and E1 data
	punpcklwd mm4,mm3		;in 2 regs (mm4|mm0)
	punpckhwd mm0,mm3		;

;Reduce color bright to 75% using shift/mask
	psrlw	  mm4,1
	psrlw	  mm0,1
	pand	  mm4,mm7
	pand	  mm0,mm7
	movq	  mm2,mm4
	movq	  mm1,mm0
	psrlw	  mm2,1
	psrlw	  mm1,1
	pand	  mm2,mm7
   	pand      mm1,mm7
	paddw	  mm4,mm2
	paddw	  mm0,mm1
	movq      [edi],mm4
	movq      [edi+8],mm0
	add       eax,8 
	add       esi,8 
	add       ebx,8
	add       edi,16 
        dec ecx
	jnz near iloop2

	pop ecx
	pop edi
	pop ebx
	pop esi
	pop eax 
	pop ebp
	ret
