; MMX general sound support + PCM channel mixing
; based on code by Daniel Moreno (ComaC) - 2001  < comac2k@teleline.es >

; ChannelMix_QS mixes one channel using 4 point, 3rd order interpolation.
; The volume of the resulting audio is 64 times louder, to have extra precision
; when mixing the channels.
;
; The BurnSoundCopy* functions convert a 32-bit (interleaved stereo or mono) buffer
; to interleaved 16-bit stereo, clamping and dividing volume by 256 on the fly.
;
; BurnSoundCopy_FM interleaves 2 16-bit buffers into a single interleaved stereo buffer,
; correcting volume (variable) on the fly
;
; BurnSoundCopy_FM interleaves 2 16-bit buffers into a single interleaved stereo buffer,
; adding a 3rd 32-bit buffer correcting volume on the 3rd buffer (variable) on the fly

[BITS 32]

global ChannelMix_QS_A
global _ChannelMix_QS_A

global ChannelMix_8U_A
global _ChannelMix_8U_A

global BurnSoundCopyClamp_A
global _BurnSoundCopyClamp_A
global BurnSoundCopyClamp_Add_A
global _BurnSoundCopyClamp_Add_A

global _BurnSoundCopyClamp_Mono_A
global BurnSoundCopyClamp_Mono_A
global _BurnSoundCopyClamp_Mono_Add_A
global BurnSoundCopyClamp_Mono_Add_A

global BurnSoundCopy_FM_A
global _BurnSoundCopy_FM_A
global BurnSoundCopy_FM_Add_A
global _BurnSoundCopy_FM_Add_A

global _BurnSoundCopy_FM_OPN_A
global BurnSoundCopy_FM_OPN_A
global _BurnSoundCopy_FM_OPN_Add_A
global BurnSoundCopy_FM_OPN_Add_A

extern _Precalc



section .text

; This macro is used from other functions. It contains the main mixing
; loop for 8 bit samples. At its input it expects:
;
; EAX -> pointer to sample buffer
; EBX -> index inside sample (current position)
; ECX -> index of the first sample that does NOT have to play (LoopEnd)
; ESI -> pointer to destination buffer
; mm3 -> Volumes Low = Left, High = Right
;
; Param 1 -> dword containing position increment
; Param 2 -> dword containing loop length
; Param 3 -> dword containing size of out buffer in samples
; Param 4 -> pointer to enf buffer
; Param 5 -> instructions to do to unpacked samples, or text NONE if none
;
; At exit:
;
; EBX -> next position to play
; EDX -> destroyed
; [Param 2] -> updated acordingly
; [Param 4] -> zero
; mm0 -> destroyed
; mm1 -> destroyed

%macro Mix_Loop	5

%define INCR	%1
%define LOOP	%2
%define BUFLEN	%3
%define ENDBUF	%4

	cmp			EBX, ECX				; Past the end?
	jl			%%Sigue					; No? -> Continue

%%PassedEnd:
	mov			EDX, ECX				; We are at or near the end of the sample
	add			EDX, 0x3000				; We need to either stop, set the pointer
	and			EDX, (0xFFFF << 12)		; to the loop point, or render a few samples
	cmp			EBX, EDX				; from the end buffer
	jge			%%LoopCheck				;

	; We need to render samples from the end buffer

	push		EBX						; We need to copy EBX to EDX (!!!)
	pxor		mm0, mm0				; Zero mm0 (to do zero extension of sample)

	add			EBX, 0x4000				; load end buffer in EDX, index in EBX
	sub			EBX, EDX				;
	mov			EDX, ENDBUF				;
	shr			EBX, 12					;

	punpcklbw	mm0, [EDX + EBX]		; Load & unpack samples (from the end buffer) to 16 bit

	pop			EDX						; !!!
	jmp			%%Interpolate

%%LoopCheck:
	cmp			LOOP, 0x1000			; > 0x1000
	jg			%%SetLoopPoint			; Yes -> Loop Sample
	cmp			LOOP, 0
	je			%%EndSample

	mov			EBX, ECX				; Kludge to work around some situations
	add			EBX, 0x2000				; where the QSound hardware is left in
	jmp			%%End_StayOn			; a non-standard state (e.g. DDTOD, DDSOM)

%%EndSample:
	xor			EBX, EBX
    xor			EAX, EAX				; set bKey to zero
	jmp			%%End					; exit function

%%SetLoopPoint:
	sub			EBX, LOOP				; Set EBX to the loop point

%%NextSample:
	cmp			EBX, ECX				; Passed the end?
	jge			%%PassedEnd

%%Sigue:
    mov			EDX, EBX
	and			EBX, (0xFFFF << 12)		; QSound banks are 0x10000 bytes
	shr			EBX, 12					; EBX = Integer(nPos)

	pxor		mm0, mm0				; Zero mm0 (to do zero extension of sample)
	punpcklbw	mm0, [EAX + EBX]		; Load & unpack samples to 16 bit

%%Interpolate:
	mov			EBX, EDX
	and			EDX, 0x0FFF				; EDX = Decimal(nPos)

	; Execute sign adaptation instructions if necessary:

%ifnidni %5, NONE
	pxor		mm0, %5
%endif

	; Interpolate sample

	pmaddwd		mm0, [_Precalc + EDX*8]	; Get multipliers
	movq		mm1, mm0				; We need High(mm0) + Low(mm0)
	psrlq		mm0, 32					; mm0 = High(mm0)
	paddd		mm0, mm1				; mm0 = Sample interpolated * 16384
	psrad		mm0, 16					; Shift samples right
	packssdw	mm0, mm0				; Hi(mm0) = sample, Low(mm0) = sample

	add			EBX, INCR				; Advance counters
	pmaddwd		mm0, mm3				; Multiply samples with volume
	dec			BUFLEN					; 1 sample less left

	paddd		mm0, [ESI]				; Add to buffer
	movq		[ESI], mm0				; Store result on buffer

	lea			ESI, [ESI + 8]
	jnz			%%NextSample			; Continue if there are more samples

%%End_StayOn:
	mov			EAX, 1					; leave bKey at 1
%%End:

%endmacro

; Parameters to ChannelMix_QS/8U:

%define BuffDest	dword [EBP +  8]
%define BuffLen		dword [EBP + 12]
%define SampleBuff	dword [EBP + 16]
%define LoopEnd		dword [EBP + 20]
%define PosPtr		dword [EBP + 24]
%define Volumes		qword [EBP + 28]
%define LoopLen		dword [EBP + 36]
%define IncrPos		dword [EBP + 40]
%define EndBuff		dword [EBP + 44]

_ChannelMix_QS_A:
ChannelMix_QS_A:

	push		EBP
	mov			EBP, ESP

	push		EBX
	push		ESI
	push		EDI

	mov			EAX, SampleBuff			; EAX = Sample Buffer
	mov			EBX, PosPtr

	movq		mm3, Volumes			; mm3 = Volumes

	mov			EBX, [EBX]				; EBX = Sample Position
	mov			ECX, LoopEnd			; ECX = LoopEnd
	sub			ECX, 0x3000
	mov			ESI, BuffDest			; ESI = destionation buffer
	mov			EDI, IncrPos			; EDI = IncrPos

	Mix_Loop	EDI, LoopLen, BuffLen, EndBuff, NONE

	mov			ECX, PosPtr
	pop			EDI
	pop			ESI
	mov			[ECX], EBX				; Save position

;	emms
	pop			EBX
	pop			EBP
	ret


_ChannelMix_8U_A:
ChannelMix_8U_A:

	push		EBP
	mov			EBP, ESP

	push		EBX
	push		ESI
	push		EDI

	mov			EAX, SampleBuff		; EAX = Sample Buffer
	mov			EBX, PosPtr

	movq		mm3, Volumes		; mm3 = Volumes
	movq		mm4, [XorSign]		; mm4 = Value to xor to adapt sign

	dec			EAX
	mov			EBX, [EBX]			; EBX = Sample Position
	mov			ECX, LoopEnd		; ECX = LoopEnd
	mov			ESI, BuffDest		; ESI = destionation buffer
	mov			EDI, IncrPos		; EDI = IncrPos

	Mix_Loop	EDI, LoopLen, BuffLen, EndBuff, mm4

	mov			ECX, PosPtr
	pop			EDI
	pop			ESI
	mov			[ECX], EBX			; Save position

;	emms
	pop			EBX
	pop			EBP
	ret


; Parameters to BurnSoundCopyClamp* functions

%define BufSrc	[EBP +  8]
%define BufDest	[EBP + 12]
%define Len		[EBP + 16]

_BurnSoundCopyClamp_A:
BurnSoundCopyClamp_A:

	push		EBP
	mov			EBP, ESP

	mov			ECX, BufSrc		; ECX = Buff Src
	mov			EDX, BufDest	; EDX = Buff Dest

	mov			EAX, Len		; EAX = Length
	shr			EAX, 1
	je			.LastSample

.Loop:
	movq		mm0, [ECX]		; Hi(mm0) = SampleR*256, Low(mm0) = SampleL*256
	movq		mm1, [ECX + 8]	; Load next sample too
	psrad		mm0, 8			; Hi(mm0) = Sample1R, Low(mm0) = Sample1L
	psrad		mm1, 8			; Hi(mm1) = Sample2R, Low(mm1) = Sample2L
	add			ECX, byte 16	; ECX -> next samples

	packssdw	mm0, mm1		; We have both samples packed here
	dec			EAX				; 2 samples less left

	movq		[EDX], mm0		; Save the result

	lea			EDX, [EDX + 8]	; EDX -> next sample
	jnz			.Loop			; Continue if there are more samples

.LastSample:
	mov			EAX, Len		; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of sample pairs.
	; Handle the last sample pair here

	movq		mm0, [ECX]
	psrad		mm0, 8
	packssdw	mm0, mm0
	movd		[EDX], mm0

.End:
	emms						; Done with MMX
	pop			EBP
	ret

_BurnSoundCopyClamp_Add_A:
BurnSoundCopyClamp_Add_A:

	push		EBP
	mov			EBP, ESP

	mov			ECX, BufSrc		; ECX = Buff Src
	mov			EDX, BufDest	; EDX = Buff Dest

	mov			EAX, Len		; EAX = Length
	shr			EAX, 1
	je			.LastSample

.Loop:
	movq		mm0, [ECX]		; Hi(mm0) = SampleR*256, Low(mm0) = SampleL*256
	movq		mm1, [ECX + 8]	; Load next sample too
	psrad		mm0, 8			; Hi(mm0) = Sample1R, Low(mm0) = Sample1L
	psrad		mm1, 8			; Hi(mm1) = Sample2R, Low(mm1) = Sample2L
	add			ECX, byte 16	; ECX -> next samples

	packssdw	mm0, mm1		; Here we have the 2 samples packed

	paddsw		mm0, [EDX]		; Add to the contents of the buffer
	dec			EAX				; 2 samples less left

	movq		[EDX], mm0		; Save the result

	lea			EDX, [EDX + 8]	; EDX -> next 2 samples
	jnz			.Loop			; Continue if there are more samples

.LastSample:
	mov			EAX, Len		; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of sample pairs.
	; Handle the last sample pair here

	movq		mm0, [ECX]
	psrad		mm0, 8
	packssdw	mm0, mm0
	movd		mm1, [EDX]
	paddsw		mm0, mm1
	movd		[EDX], mm0

.End:
	emms						; Done with MMX
	pop	EBP
	ret

_BurnSoundCopyClamp_Mono_A:
BurnSoundCopyClamp_Mono_A:

	push		EBP
	mov			EBP, ESP

	mov			ECX, BufSrc		; ECX = Buff Src
	mov			EDX, BufDest	; EDX = Buff Dest

	mov			EAX, Len		; EAX = Length
	shr			EAX, 1
	je			.LastSample

.Loop:
	movq		mm0, [ECX]		; Hi(mm0) = Sample2*256, Low(mm0) = Sample1*256
	psrad		mm0, 8			; Hi(mm0) = Sample2, Low(mm0) = Sample1
	add			ECX, byte 8		; ECX -> next samples
	packssdw	mm0, mm0		; We have both samples packed here
	punpcklwd	mm0, mm0

	dec			EAX				; 2 samples less left

	movq		[EDX], mm0		; Save the result

	lea			EDX, [EDX + 8]	; EDX -> next sample
	jnz			.Loop			; Continue if there are more samples

.LastSample:
	mov			EAX, Len		; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of samples.
	; Handle the last sample here

	movd		mm0, [ECX]
	psrad		mm0, 8
	packssdw	mm0, mm0
	punpcklwd	mm0, mm0
	movd		[EDX], mm0

.End:
	emms						; Done with MMX
	pop			EBP
	ret

_BurnSoundCopyClamp_Mono_Add_A:
BurnSoundCopyClamp_Mono_Add_A:

	push		EBP
	mov			EBP, ESP

	mov			ECX, BufSrc		; ECX = Buff Src
	mov			EDX, BufDest	; EDX = Buff Dest

	mov			EAX, Len		; EAX = Length
	shr			EAX, 1
	je			.LastSample

.Loop:
	movq		mm0, [ECX]		; Hi(mm0) = Sample2*256, Low(mm0) = Sample1*256
	psrad		mm0, 8			; Hi(mm0) = Sample2, Low(mm0) = Sample1
	add			ECX, byte 8		; ECX -> next samples
	packssdw	mm0, mm0		; We have both samples packed here
	punpcklwd	mm0, mm0

	paddsw		mm0, [EDX]		; Add to the contents of the buffer
	dec			EAX				; 2 samples less left

	movq		[EDX], mm0		; Save the result

	lea			EDX, [EDX + 8]	; EDX -> next 2 samples
	jnz			.Loop			; Continue if there are more samples

.LastSample:
	mov			EAX, Len		; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of samples.
	; Handle the last sample here

	movd		mm0, [ECX]
	psrad		mm0, 8
	packssdw	mm0, mm0
	punpcklwd	mm0, mm0
	movd		mm1, [EDX]
	paddsw		mm0, mm1
	movd		[EDX], mm0

.End:
	emms						; Done with MMX
	pop	EBP
	ret

; Parameters to BurnSoundCopy_FM

%define BufSrcL	dword [EBP +  8]
%define BufSrcR	dword [EBP + 12]
%define BufDest	dword [EBP + 16]
%define Len		dword [EBP + 20]
%define Volumes qword [EBP + 24]

_BurnSoundCopy_FM_A:
BurnSoundCopy_FM_A:

	push		EBP
	mov			EBP, ESP
	push		EBX

	movq		mm1, Volumes
	packssdw	mm1, Volumes

	; mm1 now holds the volumes as packed words

	mov			EAX, Len		; EAX = Length

	mov			EDX, BufDest	; EDX = BufDest
	mov			ECX, BufSrcR	; ECX = BufSrcR
	mov			EBX, BufSrcL	; EBX = BufSrcL

	shr			EAX, 1
	je			.LastSample

.Loop:
	movd		mm0, [EBX]
	punpcklwd	mm0, [ECX]

	; mm0 now has SrcR2 - SrcL2 - SrcR1 - SrcL1

	pmulhw		mm0, mm1		; Multiply mm0 with volumes

	add			EBX, byte 4		; EBX -> next samples
	add			ECX, byte 4		; ECX -> next samples

	dec			EAX				; 2 sample less left

	movq		[EDX], mm0		; Save the result

	lea			EDX, [EDX + 8]	; EDX -> next sample
	jnz			.Loop			; Continue if there are more samples

.LastSample:
	mov			EAX, Len		; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of sample pairs.
	; Handle the last sample pair here

	movd		mm0, [EBX]
	punpcklwd	mm0, [ECX]

	; mm0 now has ? - ? - SrcR1 - SrcL1

	pmulhw		mm0, mm1		; Multiply mm0 with volumes

	movd		[EDX], mm0		; Save the result

.End:
	emms						; Done with MMX
	pop			EBX
	pop			EBP
	ret

_BurnSoundCopy_FM_Add_A:
BurnSoundCopy_FM_Add_A:

	push		EBP
	mov			EBP, ESP
	push		EBX

	movq		mm1, Volumes
	packssdw	mm1, Volumes

	; mm1 now holds the volumes as packed words

	mov			EAX, Len		; EAX = Length

	mov			EDX, BufDest	; EDX = BufDest
	mov			ECX, BufSrcR	; ECX = BufSrcR
	mov			EBX, BufSrcL	; EBX = BufSrcL

	shr			EAX, 1
	je			.LastSample

.Loop:
	movd		mm0, [EBX]
	punpcklwd	mm0, [ECX]

	; mm0 now has SrcR2 - SrcL2 - SrcR1 - SrcL1

	pmulhw		mm0, mm1		; Multiply mm0 with volumes

	add			EBX, byte 4		; EBX -> next samples
	add			ECX, byte 4		; ECX -> next samples

	paddsw		mm0, [EDX]		; Add to the contents of the buffer
	dec			EAX				; 2 sample less left

	movq		[EDX], mm0		; Save the result

	lea			EDX, [EDX + 8]	; EDX -> next sample
	jnz			.Loop			; Continue if there are more samples

.LastSample:
	mov			EAX, Len		; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of sample pairs.
	; Handle the last sample pair here

	movd		mm0, [EBX]
	punpcklwd	mm0, [ECX]

	; mm0 now has ? - ? - SrcR1 - SrcL1

	pmulhw		mm0, mm1		; Multiply mm0 with volumes

	movd		mm1, [EDX]
	paddsw		mm0, mm1		; Add to the contents of the buffer
	movd		[EDX], mm0		; Save the result

.End:
	emms						; Done with MMX
	pop			EBX
	pop			EBP
	ret

; Parameters to BurnSoundCopyFM_OPN_A

%define BufSrcOPN dword [EBP +  8]
%define BufSrcPSG dword [EBP + 12]
%define BufDest	  dword [EBP + 16]
%define Len		  dword [EBP + 20]
%define Volumes	  qword [EBP + 24]

_BurnSoundCopy_FM_OPN_A:
BurnSoundCopy_FM_OPN_A:

	push		EBP
	mov			EBP, ESP
	push		EBX

	movq		mm4, Volumes
	packssdw	mm4, Volumes

	mov			EBX, BufSrcOPN		; EBX = Buff Src (OPN)
	mov			ECX, BufSrcPSG		; ECX = Buff Src (PSG)
	mov			EDX, BufDest		; EDX = Buff Dest

	mov			EAX, Len			; EAX = Length
	shr			EAX, 1
	je			.LastSample

.Loop:
	movq		mm0, [ECX]			; Hi(mm0) = Sample2, Low(mm0) = Sample1
	add			ECX, byte 8			; ECX -> next samples

	packssdw	mm0, mm0

	movd		mm1, [EBX]			; OPN sample left

	punpcklwd	mm0, mm0

	; mm0 now has SrcPSGR2 - SrcPSGL2 - SrcPSGR1 - SrcPSGL1

	punpcklwd	mm1, [EBX + 8192]	; OPN sample right

	; mm1 now has SrcOPNR2 - SrcOPNL2 - SrcOPNR1 - SrcOPNL1

	pmulhw		mm0, mm4			; Multiply mm0 with volumes

	add			EBX, byte 4			; EBX -> next samples

	paddsw		mm0, mm1

	dec			EAX					; 2 samples less left

	movq		[EDX], mm0			; Save the result

	lea			EDX, [EDX + 8]		; EDX -> next sample
	jnz			.Loop				; Continue if there are more samples

.LastSample:
	mov			EAX, Len			; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of samples.
	; Handle the last sample here

	movd		mm0, [ECX]
	packssdw	mm0, mm0
	movd		mm1, [EBX]
	punpcklwd	mm0, mm0
	punpcklwd	mm1, [EBX + 8192]
	pmulhw		mm0, mm4
	paddsw		mm0, mm1

	movd		[EDX], mm0

.End:
	emms							; Done with MMX
	pop			EBX
	pop			EBP
	ret

_BurnSoundCopy_FM_OPN_Add_A:
BurnSoundCopy_FM_OPN_Add_A:

	push		EBP
	mov			EBP, ESP
	push		EBX

	movq		mm4, Volumes
	packssdw	mm4, Volumes

	mov			EBX, BufSrcOPN		; EBX = Buff Src (OPN)
	mov			ECX, BufSrcPSG		; ECX = Buff Src (PSG)
	mov			EDX, BufDest		; EDX = Buff Dest

	mov			EAX, Len			; EAX = Length
	shr			EAX, 1
	je			.LastSample

.Loop:
	movq		mm0, [ECX]			; Hi(mm0) = Sample2, Low(mm0) = Sample1
	add			ECX, byte 8			; ECX -> next samples

	packssdw	mm0, mm0

	movd		mm1, [EBX]			; OPN sample left

	punpcklwd	mm0, mm0

	; mm0 now has SrcPSGR2 - SrcPSGL2 - SrcPSGR1 - SrcPSGL1

	punpcklwd	mm1, [EBX + 8192]	; OPN sample right

	; mm1 now has SrcOPNR2 - SrcOPNL2 - SrcOPNR1 - SrcOPNL1

	pmulhw		mm0, mm4			; Multiply mm0 with volumes

	add			EBX, byte 4			; EBX -> next samples

	paddsw		mm0, mm1

	paddsw		mm0, [EDX]		; Add to the contents of the buffer

	dec			EAX					; 2 samples less left

	movq		[EDX], mm0			; Save the result

	lea			EDX, [EDX + 8]		; EDX -> next sample
	jnz			.Loop				; Continue if there are more samples

.LastSample:
	mov			EAX, Len			; EAX = Length
	test		EAX, 1
	je			.End

	; We need to handle an odd amount of samples.
	; Handle the last sample here

	movd		mm0, [ECX]
	packssdw	mm0, mm0
	movd		mm1, [EBX]
	punpcklwd	mm0, mm0
	punpcklwd	mm1, [EBX + 8192]
	pmulhw		mm0, mm4
	paddsw		mm0, mm1

	movd		mm1, [EDX]
	paddsw		mm0, mm1		; Add to the contents of the buffer

	movd		[EDX], mm0

.End:
	emms							; Done with MMX
	pop			EBX
	pop			EBP
	ret


section	.data

; Used to xor with the unpacked samples to change sign:
XorSign:		dw	0x8000, 0x8000, 0x8000, 0x8000



section	.bss

VolMMX:         resd	2
ActiveFlag:		resb	1
