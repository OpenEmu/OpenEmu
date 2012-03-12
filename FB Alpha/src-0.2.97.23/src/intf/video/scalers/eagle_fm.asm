;
; Eagle.asm
;
; Eagle version 0.41 for NASM
;
; Written by Dirk Stevens
;
; compile with : nasm -f coff eagle.asm for use with DJGPP
; Using optimization with DJGPP and coff objects could result in
; erronous behaviour!
;
; History :
; Date		Version		Comments
;
; 30-Sep-1998     0.41			- Minor modification in 16bit MMX code
;
; 20-Jun-1998	0.40			-Added 16bit color support for MMX
;						-Incorporated Larry Bank's suggestions
;
; 29-March-1998   0.31			-eagle_bmp for use in plain bmps
;
; 28-March-1998	0.30			-Handle buffer internally
;						-Added optimised MMX copying (fast!)
;						-Everything is done in one routine
;
; 15-March-1998	0.20			-First NASM version
;						-Added parameter for Eagle_Lines
;						-Added support for MMX
;						-Optimised further for non-MMX
;
;------------------------------------------------------------

;
; This 16-bit MMX Eagle implementation uses a flat memory map
;
;------------------------------------------------------------

	  BITS 32
	  GLOBAL _eagle_mmx16
	  GLOBAL __eagle_mmx16
	  SECTION .text

;
; Eagle for 16bit color
; Supported for MMX *only*
;

_eagle_mmx16:
__eagle_mmx16:

; void eagle_mmx16(unsigned long *lb,
;				   unsigned long *lb2,
;				   short width,
;				   unsigned long *screen_address1,
;				   unsigned long *screen_address2);

	pushad

	push ebp

      mov esi,[esp+8+32]
	mov ebx,[esp+12+32]
	mov ecx,[esp+16+32]
	mov edi, _eagle_buffer
	mov eax, _eagle_buffer+4096
	and edi, 0xFFFFFFE0
	and eax, 0xFFFFFFE0


	mov ebp, ebx

	shr ecx, 3              ; divide by 8 because mmx registers / 16 bit color

	xor dx,dx

.L0
	push ecx
	xchg eax,ecx

	movq mm1,[ebp]
	movq mm0,[esi]
	add ebp,8
	add esi,8

	; added for mmx16

	push ebp
	push ecx
	mov ebp, ecx

	mov word cx, [_dlx]

	; end of added

	movd eax, mm0
	movd ebx, mm1

;	cmp dhx, bx
	cmp dx, bx
	jne .L6001
.L1001
	cmp bx, ax
	jne .L3001
.L2001
	mov [edi],ax
;	mov [ecx],dhx
	mov [ebp],dx
	mov [edi+2],ax
	mov [ebp+2],bx
	jmp .L11
.L3001
;	cmp dhx, dlx
        cmp dx, cx
	jne .L5001
.L4001
;	mov [edi],dx  ; mmmh
;	mov [ecx],dx  ; mmmh
;	mov [edi],dl
;	mov [ecx],dl
;	mov [edi+1],dh
;	mov [ecx+1],dh
      mov [edi],cx
      mov [ebp],cx
	mov [edi+2],dx
	mov [ebp+2],dx

	jmp .L11
.L5001
      mov [edi],cx
;	mov [ecx],dhx
	mov [ebp],dx
	mov [edi+2],ax
	mov [ebp+2],bx
	jmp .L11
.L6001
;	cmp dlx, ax
      cmp cx, ax
	jne .L12001
.L7001
;	cmp dlx, dhx
      cmp cx, dx
	jne .L9001
.L8001
;	mov [edi],dx  ; mmmh
;	mov [ecx],dx  ; mmmh
;	mov [edi],dl
;	mov [ecx],dl
;	mov [edi+1],dh
;	mov [ecx+1],dh
;	mov [edi],dlx
;	mov [ecx],dlx
      mov [edi],cx
      mov [ebp],cx
;	mov [edi+2],dhx
;	mov [ecx+2],dhx
	mov [edi+2],dx
	mov [ebp+2],dx

	jmp .L11
.L9001
	cmp ax,bx
	jne .L11001
.L10001
;	mov [edi],dlx
      mov [edi],cx
	mov [ebp],bx
	mov [edi+2],ax
	mov [ebp+2],bx
	jmp .L11
.L11001
.L12001
;	mov [edi],dlx
;	mov [ecx],dhx
      mov [edi],cx
	mov [ebp],dx
	mov [edi+2],ax
	mov [ebp+2],bx

.L11

	movq mm4, mm1

	movq mm2, mm0

      pcmpeqd mm7, mm7		; set mm7 to FFFFFFFFFFFFFFFF

	;pcmpeqb mm4, mm0		; byte compare equal mm0 with mm1 and store in mm4
	pcmpeqw mm4, mm0		; word compare equal mm0 with mm1 and store in mm4

	;psllq mm2, 8		; shift mm0 left one byte and store in mm2
	psllq mm2, 16		; shift mm0 left one word and store in mm2

	movq mm3, mm0

	movq mm5, mm4		; store byte compare mm0 with mm1 in mm5

	;pcmpeqb mm2, mm0		; byte compare mm0 with mm2 and store in mm2
	pcmpeqw mm2, mm0		; word compare mm0 with mm2 and store in mm2

	;psrlq mm3, 8		; shift mm0 right one byte and store in mm3
	psrlq mm3, 16		; shift mm0 right one word and store in mm3

	pand mm2, mm5

      movq mm6, mm1		; and mm5 with mm1

	movq mm5, mm2		; not mm2 and store in mm5

      pxor mm5, mm7

;      psllq mm6, 8
      psllq mm6, 16

	pand mm2, mm0		; and mm0 with mm2

      pand mm5, mm6

	por mm2, mm5    		; mm2 now contains right-bottom of quad

	;pcmpeqb mm3, mm0		; byte compare mm3 with mm0 and store in mm3
	pcmpeqw mm3, mm0		; word compare mm3 with mm0 and store in mm3

	movq mm5, mm4		; word compare of mm0 with mm1 store in mm5

	pand mm3, mm4

	movq mm5, mm3		; not mm3 store in mm5

	movq mm6, mm1

      pxor mm5, mm7

	pand mm3, mm0

      ;psrlq mm6, 8
      psrlq mm6, 16

	pand mm5, mm6

      ;psrlq mm2, 8
      psrlq mm2, 16

	por mm3, mm5

; now write the 16 bytes of the bottom line

      movq mm4, mm2

	movq mm6, mm2

;      punpcklbw mm4, mm3

;	punpckhbw mm6, mm3

        punpcklwd mm4, mm3

        punpckhwd mm6, mm3

;	movq [ecx+2],mm4
	movq [ebp+4],mm4

;	movq [ecx+10],mm6
	movq [ebp+12],mm6


; start with top line

	movq mm2, mm1
	movq mm3, mm1
	movq mm4, mm0

      pcmpeqd mm7, mm7		; set mm7 to FFFFFFFFFFFFFFFF

	;pcmpeqb mm4, mm1		; byte compare equal mm0 with mm1 and store in mm4
	pcmpeqw mm4, mm1		; word compare equal mm0 with mm1 and store in mm4

	;psllq mm2, 8		; shift mm0 left one byte and store in mm2
	psllq mm2, 16		; shift mm0 left one word and store in mm2

	;psrlq mm3, 8		; shift mm0 right one byte and store in mm3
	psrlq mm3, 16		; shift mm0 right one word and store in mm3

	movq mm5, mm4		; store byte compare mm0 with mm1 in mm5

	;pcmpeqb mm2, mm1		; byte compare mm0 with mm2 and store in mm2
	pcmpeqw mm2, mm1		; word compare mm0 with mm2 and store in mm2

	pand mm2, mm5

	movq mm5, mm2		; not mm2 and store in mm5
      pxor mm5, mm7

	pand mm2, mm1		; and mm0 with mm2

      movq mm6, mm0		; and mm5 with mm1
      ;psllq mm6, 8
      psllq mm6, 16
      pand mm5, mm6

	por mm2, mm5    		; mm2 now contains right-bottom of quad


	movq mm5, mm4		; byte compare of mm0 with mm1 store in mm5

	;pcmpeqb mm3, mm1		; byte compare mm3 with mm0 and store in mm3
	pcmpeqw mm3, mm1		; word compare mm3 with mm0 and store in mm3

	pand mm3, mm5

	movq mm5, mm3		; not mm3 store in mm5
      pxor mm5, mm7

	pand mm3, mm1

	movq mm6, mm0
      ;psrlq mm6, 8
      psrlq mm6, 16
	pand mm5, mm6

	por mm3, mm5

; now write the 16 bytes of the top line

      ;psrlq mm2, 8
      psrlq mm2, 16

      movq mm4, mm2

      movq mm6, mm2

;      punpcklbw mm4, mm3

;      punpckhbw mm6, mm3

      punpcklwd mm4, mm3

      punpckhwd mm6, mm3

;	movq [edi+2],mm4
	movq [edi+4],mm4
;      psrlq mm0, 56			; 64-8 = 56
      psrlq mm0, 48			; 64-16 = 48
;	movq [edi+10],mm6
	movq [edi+12],mm6
      ;psrlq mm1, 56
      psrlq mm1, 48


;
; restore ebp and ecx
;

	pop ecx
	pop ebp

;
; End of restore
;

      add edi, 16
      add ecx, 16

	movd edx, mm0
	mov word [_dlx], dx
	movd ebx, mm1
	mov  dx, bx

	pop eax
	xchg eax,ecx

	dec ecx
	jnz near .L0

.L333

	mov esi, _eagle_buffer
	mov ebx, _eagle_buffer+4096
	and esi, 0xFFFFFFE0
	and ebx, 0xFFFFFFE0

	mov ecx,[esp+16+32]
	mov edi,[esp+20+32]
	mov eax,[esp+24+32]

	;shr ecx, 5
	shr ecx, 4

	push ecx

.first_loop:
	movq mm0, [ esi ]
	movq mm1, [ esi + 8 ]
	movq mm2, [ esi + 16]
	movq mm3, [ esi + 24]
	movq mm4, [ esi + 32]
	movq mm5, [ esi + 40]
	movq mm6, [ esi + 48]
	movq mm7, [ esi + 56]

	movq [edi], mm0
	movq [edi + 8], mm1
	movq [edi + 16], mm2
	movq [edi + 24], mm3
	movq [edi + 32], mm4
	movq [edi + 40], mm5
	movq [edi + 48], mm6
	movq [edi + 56], mm7

	add edi, 64
	add esi, 64

	dec ecx
	jnz .first_loop

	pop ecx

	mov edi, eax
	mov esi, ebx


.second_loop:
	movq mm0, [ esi ]
	movq mm1, [ esi + 8 ]
	movq mm2, [ esi + 16]
	movq mm3, [ esi + 24]
	movq mm4, [ esi + 32]
	movq mm5, [ esi + 40]
	movq mm6, [ esi + 48]
	movq mm7, [ esi + 56]

	movq [edi], mm0
	movq [edi + 8], mm1
	movq [edi + 16], mm2
	movq [edi + 24], mm3
	movq [edi + 32], mm4
	movq [edi + 40], mm5
	movq [edi + 48], mm6
	movq [edi + 56], mm7

	add edi, 64
	add esi, 64

	dec ecx
	jz .continue_here
	jmp .second_loop

.continue_here
	pop eax
	mov ebp, eax

	popad

	ret

	SECTION .bss
_offset32		resb 32
_eagle_buffer	resb 16384
_tripline1		resb 8
_tripline2		resb 8
_tripline3		resb 8
_dlx			resb 32

