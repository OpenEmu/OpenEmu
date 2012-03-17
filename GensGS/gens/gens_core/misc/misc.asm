%include "nasmhead.inc"

section .data align=64
	
	Small_Police:
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 32   
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000000, 0x00000700, 0x00000000			; 33	!
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 34	"
		dd 0x00000000, 0x00040002, 0x00050403, 0x00060004, 0x00070605, 0x00080006, 0x00000000			; 35	#
		dd 0x00000000, 0x00040300, 0x00000403, 0x00000500, 0x00070600, 0x00000706, 0x00000000			; 36	$
		dd 0x00000000, 0x00000002, 0x00050000, 0x00000500, 0x00000005, 0x00080000, 0x00000000			; 37	%
		dd 0x00000000, 0x00000300, 0x00050003, 0x00000500, 0x00070005, 0x00080700, 0x00000000			; 38	&
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 39	'
		dd 0x00000000, 0x00000300, 0x00000003, 0x00000004, 0x00000005, 0x00000700, 0x00000000			; 40	(
		dd 0x00000000, 0x00000300, 0x00050000, 0x00060000, 0x00070000, 0x00000700, 0x00000000			; 41	)
		dd 0x00000000, 0x00000000, 0x00050003, 0x00000500, 0x00070005, 0x00000000, 0x00000000			; 42	*
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060504, 0x00000600, 0x00000000, 0x00000000			; 43	+
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000600, 0x00000700, 0x00000007			; 44	,
		dd 0x00000000, 0x00000000, 0x00000000, 0x00060504, 0x00000000, 0x00000000, 0x00000000			; 45	-
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000700, 0x00000000			; 46	.
		dd 0x00030000, 0x00040000, 0x00000400, 0x00000500, 0x00000005, 0x00000006, 0x00000000			; 47	/
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 48	0
		dd 0x00000000, 0x00000300, 0x00000403, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 49	1
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000500, 0x00000005, 0x00080706, 0x00000000			; 50	2
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000504, 0x00070000, 0x00000706, 0x00000000			; 51	3
		dd 0x00000000, 0x00000300, 0x00000003, 0x00060004, 0x00070605, 0x00080000, 0x00000000			; 52	4
		dd 0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00070000, 0x00000706, 0x00000000			; 53	5
		dd 0x00000000, 0x00000300, 0x00000003, 0x00000504, 0x00070005, 0x00000700, 0x00000000			; 54	6
		dd 0x00000000, 0x00040302, 0x00050000, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 55	7
		dd 0x00000000, 0x00000300, 0x00050003, 0x00000500, 0x00070005, 0x00000700, 0x00000000			; 56	8
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060500, 0x00070000, 0x00000700, 0x00000000			; 57	9
		dd 0x00000000, 0x00000000, 0x00000400, 0x00000000, 0x00000000, 0x00000700, 0x00000000			; 58	:
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000500, 0x00000000, 0x00000700, 0x00000007			; 59	;
		dd 0x00000000, 0x00040000, 0x00000400, 0x00000004, 0x00000600, 0x00080000, 0x00000000			; 60	<
		dd 0x00000000, 0x00000000, 0x00050403, 0x00000000, 0x00070605, 0x00000000, 0x00000000			; 61	=
		dd 0x00000000, 0x00000002, 0x00000400, 0x00060000, 0x00000600, 0x00000006, 0x00000000			; 62	>
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060000, 0x00000600, 0x00000000, 0x00000800			; 63	?
		dd 0x00000000, 0x00000300, 0x00050400, 0x00060004, 0x00070600, 0x00000000, 0x00000000			; 64	@
		dd 0x00000000, 0x00000300, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000			; 65	A
		dd 0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00070005, 0x00000706, 0x00000000			; 66	B
		dd 0x00000000, 0x00040300, 0x00000003, 0x00000004, 0x00000005, 0x00080700, 0x00000000			; 67 	C
		dd 0x00000000, 0x00000302, 0x00050003, 0x00060004, 0x00070005, 0x00000706, 0x00000000			; 68	D
		dd 0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00000005, 0x00080706, 0x00000000			; 69	E
		dd 0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 70	F
		dd 0x00000000, 0x00040300, 0x00000003, 0x00060004, 0x00070005, 0x00080700, 0x00000000			; 71	G
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000			; 72	H
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 73	I
		dd 0x00000000, 0x00040000, 0x00050000, 0x00060000, 0x00070005, 0x00000700, 0x00000000			; 74	J
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000504, 0x00070005, 0x00080006, 0x00000000			; 75	K
		dd 0x00000000, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00080706, 0x00000000			; 76	l
		dd 0x00000000, 0x00040002, 0x00050403, 0x00060004, 0x00070005, 0x00080006, 0x00000000			; 77	M
		dd 0x00000000, 0x00000302, 0x00050003, 0x00060004, 0x00070005, 0x00080006, 0x00000000			; 78	N
		dd 0x00000000, 0x00040302, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00000000			; 79	O
		dd 0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 80	P
		dd 0x00000000, 0x00040302, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00090000			; 81	Q
		dd 0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00070005, 0x00080006, 0x00000000			; 82	R
		dd 0x00000000, 0x00040300, 0x00000003, 0x00000500, 0x00070000, 0x00000706, 0x00000000			; 83 	S
		dd 0x00000000, 0x00040302, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 84	T
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 85	U
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00000600, 0x00000700, 0x00000000			; 86	V
		dd 0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00070605, 0x00080006, 0x00000000			; 87	W
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000500, 0x00070005, 0x00080006, 0x00000000			; 88	X
		dd 0x00000000, 0x00040002, 0x00050003, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 89	Y
		dd 0x00000000, 0x00040302, 0x00050000, 0x00000500, 0x00000005, 0x00080706, 0x00000000			; 90	Z
		dd 0x00000000, 0x00040300, 0x00000400, 0x00000500, 0x00000600, 0x00080700, 0x00000000			; 91	[
		dd 0x00000001, 0x00000002, 0x00000400, 0x00000500, 0x00070000, 0x00080000, 0x00000000			; 92	'\'
		dd 0x00000000, 0x00000302, 0x00000400, 0x00000500, 0x00000600, 0x00000706, 0x00000000			; 93	]
		dd 0x00000000, 0x00000300, 0x00050003, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 94	^
		dd 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00080706, 0x00000000			; 95	_
		dd 0x00000000, 0x00000002, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 96	`
		dd 0x00000000, 0x00000000, 0x00050400, 0x00060004, 0x00070005, 0x00080700, 0x00000000			; 97	a
		dd 0x00000000, 0x00000002, 0x00000003, 0x00000504, 0x00070005, 0x00000706, 0x00000000			; 98	b
		dd 0x00000000, 0x00000000, 0x00050400, 0x00000004, 0x00000005, 0x00080700, 0x00000000			; 99	c
		dd 0x00000000, 0x00040000, 0x00050000, 0x00060500, 0x00070005, 0x00080700, 0x00000000			; 100	d
		dd 0x00000000, 0x00000000, 0x00050400, 0x00060504, 0x00000005, 0x00080700, 0x00000000			; 101	e
		dd 0x00000000, 0x00040300, 0x00000003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 102	f
		dd 0x00000000, 0x00000000, 0x00050400, 0x00060004, 0x00070600, 0x00080000, 0x00000807			; 103	g
		dd 0x00000000, 0x00000002, 0x00000003, 0x00000504, 0x00070005, 0x00080006, 0x00000000			; 104	h
		dd 0x00000000, 0x00000300, 0x00000000, 0x00000500, 0x00000600, 0x00000700, 0x00000000			; 105	i
		dd 0x00000000, 0x00000300, 0x00000000, 0x00000500, 0x00000600, 0x00000700, 0x00000007			; 106	j
		dd 0x00000000, 0x00000002, 0x00000403, 0x00060004, 0x00000605, 0x00080006, 0x00000000			; 107	k
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00080000, 0x00000000			; 108	l
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000			; 109	m
		dd 0x00000000, 0x00000000, 0x00000403, 0x00060004, 0x00070005, 0x00080006, 0x00000000			; 110	n
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 111	o
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00000605, 0x00000006, 0x00000007			; 112	p
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070600, 0x00080000, 0x00090000			; 113	q
		dd 0x00000000, 0x00000000, 0x00050003, 0x00000504, 0x00000005, 0x00000006, 0x00000000			; 114	r
		dd 0x00000000, 0x00000000, 0x00050400, 0x00000004, 0x00070600, 0x00000706, 0x00000000			; 115	s
		dd 0x00000000, 0x00000300, 0x00050400, 0x00000500, 0x00000600, 0x00080000, 0x00000000			; 116	t
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070005, 0x00080700, 0x00000000			; 117	u
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000			; 118	v
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070605, 0x00080006, 0x00000000			; 119	w
		dd 0x00000000, 0x00000000, 0x00050003, 0x00000500, 0x00070005, 0x00080006, 0x00000000			; 120	x
		dd 0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00000600, 0x00000700, 0x00000000			; 121	y
		dd 0x00000000, 0x00000000, 0x00050403, 0x00000500, 0x00000005, 0x00080706, 0x00000000			; 122	z
		dd 0x00000000, 0x00040300, 0x00000400, 0x00000504, 0x00000600, 0x00080700, 0x00000000			; 123	{
		dd 0x00000000, 0x00000300, 0x00000400, 0x00000000, 0x00000600, 0x00000700, 0x00000000			; 124	|
		dd 0x00000000, 0x00000302, 0x00000400, 0x00060500, 0x00000600, 0x00000706, 0x00000000			; 125	}
		dd 0x00000000, 0x00000302, 0x00050000, 0x00000000, 0x00000000, 0x00000000, 0x00000000			; 126	~
		dd 0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070605, 0x00000000, 0x00000000			; 127 	

	Palette_Blanc_15:

	%assign i 16
	%rep 16
		dw (i * 1024 + i * 32 + i)
	%assign i i+1
	%endrep

	Palette_Bleu_15:

	%assign i 16
	%rep 16
		dw i
	%assign i i+1
	%endrep

	Palette_Vert_15:

	%assign i 16
	%rep 16
		dw (i * 32)
	%assign i i+1
	%endrep

	Palette_Rouge_15:

	%assign i 16
	%rep 16
		dw (i * 1024)
	%assign i i+1
	%endrep

	Palette_Blanc_16:

	%assign i 16
	%rep 16
		dw (i * 2048 + i * 64 + i)
	%assign i i+1
	%endrep

	Palette_Bleu_16:

	%assign i 16
	%rep 16
		dw i
	%assign i i+1
	%endrep

	Palette_Vert_16:

	%assign i 16
	%rep 16
		dw (i * 64)
	%assign i i+1
	%endrep

	Palette_Rouge_16:

	%assign i 16
	%rep 16
		dw (i * 2048)
	%assign i i+1
	%endrep

	Palette_Blanc_32:

	%assign i 128
	%rep 16
		dd (i * 65536 + i * 256 + i)
	%assign i i+8
	%endrep

	Palette_Bleu_32:

	%assign i 128
	%rep 16
		dd i
	%assign i i+8
	%endrep

	Palette_Vert_32:

	%assign i 128
	%rep 16
		dd (i * 256)
	%assign i i+8
	%endrep

	Palette_Rouge_32:

	%assign i 128
	%rep 16
		dd (i * 65536)
	%assign i i+8
	%endrep

	Mask:		dd 0x00000000, 0x00000000
	Mask_GG_15:	dd 0x03E003E0, 0x03E003E0
	Mask_RBRB_15:	dd 0x7C1F7C1F, 0x7C1F7C1F
	Mask_GG_16:	dd 0x07C007C0, 0x07C007C0
	Mask_RBRB_16:	dd 0xF81FF81F, 0xF81FF81F

	Mask_1001_64:	dd 0xFFFF0000, 0x0000FFFF
	Mask_0011_64:	dd 0xFFFFFFFF, 0x00000000
	Mask_1100_64:	dd 0x00000000, 0xFFFFFFFF


section .bss align=64
	
	; Symbol redefines for ELF
	%ifdef __OBJ_ELF
		%define	_MD_Screen		MD_Screen
		%define	_MD_Palette		MD_Palette
		%define	_MD_Screen32		MD_Screen32
		%define	_MD_Palette32		MD_Palette32
		
		%define	_CDD			CDD
		%define	_CDD.Control		CDD.Control
		%define	_CDD.Rcv_Status		CDD.Rcv_Status
		%define	_CDD.Status		CDD.Status
		%define _CDD.Minute		CDD.Minute
		%define	_CDD.Seconde		CDD.Seconde
		%define	_CDD.Frame		CDD.Frame
		%define _CDD.Ext		CDD.Ext
		
		%define	_VDP_Reg		VDP_Reg
		
		%define _bppMD			bppMD
	%endif
	
	extern _MD_Screen
	extern _MD_Palette
	extern _MD_Screen32
	extern _MD_Palette32
	
	extern _CDD.Control
	extern _CDD.Rcv_Status
	extern _CDD.Status
	extern _CDD.Minute
	extern _CDD.Seconde
	extern _CDD.Frame
	extern _CDD.Ext
	
	extern _VDP_Reg
	
	; MD bpp
	extern _bppMD
	
section .text align=64

%macro AFF_PIXEL 2

		mov eax, ebx					; eax = data pixels
		shr eax, %2					; keep the first
		and eax, 0xF
		mov ax, [_MD_Palette + eax * 2 + ebp]		; conversion 8->16 bits palette
		mov [edi + (%1 * 2)], ax			; write the pixel to Dest

%endmacro


; TODO: Gens Rerecording had additional jumps here...
%macro AFF_PIXEL32 2

		mov eax, ebx					; eax = data pixels
		shr eax, %2					; keep the first
		and eax, 0xF
		mov eax, [_MD_Palette32 + eax * 4 + ebp]		; conversion 8->16 bits palette
		mov [edi + (%1 * 4)], eax			; write the pixel to Dest

%endmacro


%macro AFF_LINE_LETTER 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 0], ax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 2], ax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 4], ax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 2 * %1) + 6], ax

	%%End

%endmacro


%macro AFF_LINE_LETTER32 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 0], eax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 4], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 8], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 4 * %1) + 12], eax

	%%End

%endmacro


%macro AFF_LINE_LETTER_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 0], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 0], eax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 4], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 4], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 8], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 8], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		rol eax, 16
		mov ax, [ebx + edx * 2]
		mov [edi + (336 * 4 * %1) + 12], eax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 12], eax

	%%End

%endmacro


%macro AFF_LINE_LETTER32_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 0], eax
		mov [edi + (336 * 8 * %1) + 4], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 0], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 4], eax

	%%Pix1
		mov	dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 8], eax
		mov [edi + (336 * 8 * %1) + 12], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 8], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 12], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 16], eax
		mov [edi + (336 * 8 * %1) + 20], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 16], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 20], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov [edi + (336 * 8 * %1) + 24], eax
		mov [edi + (336 * 8 * %1) + 28], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 24], eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 28], eax

	%%End

%endmacro


%macro AFF_LINE_LETTER_T 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 0]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 0], ax

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 2]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 2], ax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 4]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 4], ax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 2 * %1) + 6]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add ax, dx
		xor dh, dh
		mov [edi + (336 * 2 * %1) + 6], ax

	%%End

%endmacro


%macro AFF_LINE_LETTER32_T 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 4 * %1) + 0]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 0], eax

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 4 * %1) + 4]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 4], eax

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 4]
		mov dx, [edi + (336 * 4 * %1) + 8]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 8], eax

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 4]
		mov dx, [edi + (336 * 4 * %1) + 12]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add eax, edx
		xor edx, edx
		mov [edi + (336 * 4 * %1) + 12], eax

	%%End

%endmacro


%macro AFF_LINE_LETTER_T_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 0]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 0], dx
		mov dx, [edi + (336 * 4 * %1) + 2]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 2], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 0]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 0], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 2]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 2], ax
		xor dh, dh

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 4]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 4], dx
		mov dx, [edi + (336 * 4 * %1) + 6]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 6], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 4]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 4], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 6]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 6], ax
		xor dh, dh

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 8]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 8], dx
		mov dx, [edi + (336 * 4 * %1) + 10]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 10], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 8]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 8], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 10]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 10], ax
		xor dh, dh

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov ax, [ebx + edx * 2]
		mov dx, [edi + (336 * 4 * %1) + 12]
		and ax, [Mask]
		and dx, [Mask]
		shr ax, 1
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 12], dx
		mov dx, [edi + (336 * 4 * %1) + 14]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + 14], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 12]
		and dx, [Mask]
		shr dx, 1
		add dx, ax
		mov [edi + (336 * 4 * %1) + (336 * 2) + 12], dx
		mov dx, [edi + (336 * 4 * %1) + (336 * 2) + 14]
		and dx, [Mask]
		shr dx, 1
		add ax, dx
		mov [edi + (336 * 4 * %1) + (336 * 2) + 14], ax
		xor dh, dh

	%%End

%endmacro


%macro AFF_LINE_LETTER32_T_X2 1

	%%Pix0
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 0]
		test dl, dl
		jz %%Pix1
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 0]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 0], edx
		mov edx, [edi + (336 * 8 * %1) + 4]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 4], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 0]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 0], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 4]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 4], eax
		xor edx, edx

	%%Pix1
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 1]
		test dl, dl
		jz %%Pix2
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 8]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 8], edx
		mov edx, [edi + (336 * 8 * %1) + 12]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 12], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 8]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 8], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 12]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 12], eax
		xor edx, edx

	%%Pix2
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 2]
		test dl, dl
		jz %%Pix3
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 16]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 16], edx
		mov edx, [edi + (336 * 8 * %1) + 20]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 20], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 16]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 16], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 20]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 20], eax
		xor edx, edx

	%%Pix3
		mov dl, byte [Small_Police + ecx + (%1 * 4) + 3]
		test dl, dl
		jz %%End
		mov eax, [ebx + edx * 4]
		mov edx, [edi + (336 * 8 * %1) + 24]
		and eax, [Mask]
		and edx, [Mask]
		shr eax, 1
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 24], edx
		mov edx, [edi + (336 * 8 * %1) + 28]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + 28], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 4) + 24]
		and edx, [Mask]
		shr edx, 1
		add edx, eax
		mov [edi + (336 * 8 * %1) + (336 * 4) + 24], edx
		mov edx, [edi + (336 * 8 * %1) + (336 * 2) + 28]
		and edx, [Mask]
		shr edx, 1
		add eax, edx
		mov [edi + (336 * 8 * %1) + (336 * 4) + 28], eax
		xor edx, edx

	%%End

%endmacro


	ALIGN4
	
		; void Print_Text(char *str, int Size, int Pos_X, int Pos_Y, int Style)
		DECL Print_Text

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov esi, [esp + 24]				; esi = *string
		
		; Check if 32-bit color is in use.
		cmp byte [_bppMD], 32
		je Print_Text32
		
		; 15/16-bit color functions.
		lea edi, [_MD_Screen + 8 * 2]	; edi = Dest
		
		; Print on the bottom of the screen.
		mov ebx, 336 * 2				; Pitch Dest
		mov eax, [esp + 36]				; eax = Pos Y
		mul ebx
		
		mov ecx, [esp + 32]				; Pos X
		lea edi, [edi + ecx * 2]		; offset pour Pos X
		add edi, eax					; + offset Pos Y
		mov ebx, 320					; longueur d'une ligne
		mov eax, [esp + 40]				; eax = Style
		mov ecx, [esp + 28]				; ecx = Size
		test eax, 0x1					; on teste si on est en mode emulation
		jz short .No_Emulation

		test byte [_VDP_Reg + 12 * 4], 1	; on teste si on est en mode 32 ou 40 cells
		jnz short .No_Emulation

		mov ebx, 256					; Taille = 256

	.No_Emulation
		mov edx, [esp + 32]				; edx = Pos X
		test eax, 0x10					; teste si mode x2
		lea edx, [edx + ecx * 4]		; edx = Pos X finale
		jz short .Size_x1

		lea edx, [edx + ecx * 4]		; edx = Pos X finale

	.Size_x1
		sub edx, ebx					; on teste si la chaine n'est pas trop grande
		jb short .String_OK				; si c ok on passe � la suite sinon

		shr edx, 2
		test eax, 0x10					; teste si mode x2
		jz short .Size_x1_2

		shr edx, 1

	.Size_x1_2
		inc edx							; edx = nombre de caract�re en trop
		sub ecx, edx					; ecx = nouvelle taille pour �tre OK
		mov byte [esi + ecx - 2], '.'	; on termine la chiane avec des points
		mov byte [esi + ecx - 1], '.'

	.String_OK
		mov ebx, eax
		mov byte [esi + ecx - 0], 0		; fin de la chaine pour �tre sur
		shr ebx, 1
		sub esp, 4
		and ebx, 0x3					; on garde uniquement le type palette
		mov dword [Mask], 0xF7DE
		cmp byte [_bppMD], 15  ; 15 == mode 555; 16 == mode 565
		jne short .Mode_565
		or ebx, 0x4
		mov dword [Mask], 0x7BDE

	.Mode_565
		test byte [esi], 0xFF				; on teste la premi�re lettre
		mov ebx, [.Palette_Table + ebx * 4]	; ebx pointe sur la palette utilis�e
		jz near .End						; si NULL alors on quitte
		add ebx, 12							; couleurs claires

		and eax, 0x18						; on isole les bits pour le type d'affichage
		shr eax, 1
		mov eax, [.Table_Style + eax]
		mov [esp], eax
		jmp [esp]

	ALIGN4

	.Palette_Table
		dd Palette_Blanc_16, Palette_Bleu_16, Palette_Vert_16, Palette_Rouge_16
		dd Palette_Blanc_15, Palette_Bleu_15, Palette_Vert_15, Palette_Rouge_15

	ALIGN4

	.Table_Style
		dd .Mode_Normal, .Mode_Trans
		dd .Mode_Normal_X2, .Mode_Trans_X2

	ALIGN4

	.Mode_Normal
		movzx eax, byte [esi]
		sub eax, 32
		jae .MN_Car_OK
		xor eax, eax
	.MN_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER 0
		AFF_LINE_LETTER 1
		AFF_LINE_LETTER 2
		AFF_LINE_LETTER 3
		AFF_LINE_LETTER 4
		AFF_LINE_LETTER 5
		AFF_LINE_LETTER 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Trans
		movzx eax, byte [esi]
		sub eax, 32
		jae .MT_Car_OK
		xor eax, eax
	.MT_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER_T 0
		AFF_LINE_LETTER_T 1
		AFF_LINE_LETTER_T 2
		AFF_LINE_LETTER_T 3
		AFF_LINE_LETTER_T 4
		AFF_LINE_LETTER_T 5
		AFF_LINE_LETTER_T 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Normal_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MN_X2_Car_OK
		xor eax, eax
	.MN_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER_X2 0
		AFF_LINE_LETTER_X2 1
		AFF_LINE_LETTER_X2 2
		AFF_LINE_LETTER_X2 3
		AFF_LINE_LETTER_X2 4
		AFF_LINE_LETTER_X2 5
		AFF_LINE_LETTER_X2 6

		jmp .Next_Letter_X2

	ALIGN4

	.Mode_Trans_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MT_X2_Car_OK
		xor eax, eax
	.MT_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER_T_X2 0
		AFF_LINE_LETTER_T_X2 1
		AFF_LINE_LETTER_T_X2 2
		AFF_LINE_LETTER_T_X2 3
		AFF_LINE_LETTER_T_X2 4
		AFF_LINE_LETTER_T_X2 5
		AFF_LINE_LETTER_T_X2 6

		jmp .Next_Letter_X2

	ALIGN4
	
	.Next_Letter
		inc esi
		add edi, 4 * 2
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	ALIGN4
	
	.Next_Letter_X2
		inc esi
		add edi, 8 * 2
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	.End
		add esp, 4
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN4
	
	Print_Text32 ; 32-bit color text printing functions.

		shl ebx, 1						; Pitch Dest
		lea edi, [_MD_Screen32 + 8 * 4]	; edi = Dest
		
		; print on the bottom of the screen
		mov ebx, 336 * 4				; Pitch Dest
		mov eax, [esp + 36]				; eax = Pos Y
		mul ebx
		
		mov ecx, [esp + 32]				; Pos X
		lea edi, [edi + ecx * 4]		; offset for Pos X
		add edi, eax					; + offset Pos Y
		mov ebx, 320					; length of a row
		mov eax, [esp + 40]				; eax = Style
		mov ecx, [esp + 28]				; ecx = Size
		test eax, 0x1					; test the emulation mode
		jz short .No_Emulation

		test byte [_VDP_Reg + 12 * 4], 1	; on teste si on est en mode 32 ou 40 cells
		jnz short .No_Emulation

		mov ebx, 256					; Taille = 256

	.No_Emulation
		mov edx, [esp + 32]				; edx = Pos X
		test eax, 0x10					; teste if mode x2
		lea edx, [edx + ecx * 4]		; edx = Pos X finale
		jz short .Size_x1

		lea edx, [edx + ecx * 4]		; edx = Pos X finale

	.Size_x1
		sub edx, ebx					; on teste si la chaine n'est pas trop grande
		jb short .String_OK				; si c ok on passe � la suite sinon

		shr edx, 2
		test eax, 0x10					; teste si mode x2
		jz short .Size_x1_2

		shr edx, 1

	.Size_x1_2
		inc edx							; edx = nombre de caract�re en trop
		sub ecx, edx					; ecx = nouvelle taille pour �tre OK
		mov byte [esi + ecx - 2], '.'	; on termine la chiane avec des points
		mov byte [esi + ecx - 1], '.'

	.String_OK
		mov ebx, eax
		mov byte [esi + ecx - 0], 0		; fin de la chaine pour �tre sur
		shr ebx, 1
		sub esp, 4
		and ebx, 0x3					; on garde uniquement le type palette
		mov dword [Mask], 0xFEFEFE
		test byte [esi], 0xFF				; test the first byte
		mov ebx, [.Palette_Table + ebx * 4]	; ebx pointe to the palette in use
		jz near .End						; if NULL then we leave...
		add ebx, 12							; ...a clear color

		and eax, 0x18						; isolate the bits for copying
		shr eax, 1
		mov eax, [.Table_Style + eax]
		mov [esp], eax
		jmp [esp]

	ALIGN4

	.Palette_Table
		dd Palette_Blanc_32, Palette_Bleu_32, Palette_Vert_32, Palette_Rouge_32

	ALIGN4

	.Table_Style
		dd .Mode_Normal, .Mode_Trans
		dd .Mode_Normal_X2, .Mode_Trans_X2

	ALIGN4

	.Mode_Normal
		movzx eax, byte [esi]
		sub eax, 32
		jae .MN_Car_OK
		xor eax, eax
	.MN_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32 0
		AFF_LINE_LETTER32 1
		AFF_LINE_LETTER32 2
		AFF_LINE_LETTER32 3
		AFF_LINE_LETTER32 4
		AFF_LINE_LETTER32 5
		AFF_LINE_LETTER32 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Trans
		movzx eax, byte [esi]
		sub eax, 32
		jae .MT_Car_OK
		xor eax, eax
	.MT_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32_T 0
		AFF_LINE_LETTER32_T 1
		AFF_LINE_LETTER32_T 2
		AFF_LINE_LETTER32_T 3
		AFF_LINE_LETTER32_T 4
		AFF_LINE_LETTER32_T 5
		AFF_LINE_LETTER32_T 6

		jmp .Next_Letter

	ALIGN4

	.Mode_Normal_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MN_X2_Car_OK
		xor eax, eax
	.MN_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32_X2 0
		AFF_LINE_LETTER32_X2 1
		AFF_LINE_LETTER32_X2 2
		AFF_LINE_LETTER32_X2 3
		AFF_LINE_LETTER32_X2 4
		AFF_LINE_LETTER32_X2 5
		AFF_LINE_LETTER32_X2 6

		jmp .Next_Letter_X2

	ALIGN4

	.Mode_Trans_X2
		movzx eax, byte [esi]
		sub eax, 32
		jae short .MT_X2_Car_OK
		xor eax, eax
	.MT_X2_Car_OK
		mov ecx, eax
		lea eax, [eax * 4]
		shl ecx, 5
		xor edx, edx
		sub ecx, eax

		AFF_LINE_LETTER32_T_X2 0
		AFF_LINE_LETTER32_T_X2 1
		AFF_LINE_LETTER32_T_X2 2
		AFF_LINE_LETTER32_T_X2 3
		AFF_LINE_LETTER32_T_X2 4
		AFF_LINE_LETTER32_T_X2 5
		AFF_LINE_LETTER32_T_X2 6

		jmp .Next_Letter_X2

	ALIGN4
	
	.Next_Letter
		inc esi
		add edi, 4 * 4
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	ALIGN4
	
	.Next_Letter_X2
		inc esi
		add edi, 8 * 4
		test byte [esi], 0xFF
		jz short .End
		jmp [esp]

	.End
		add esp, 4
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN32
	
		;void Cell_8x8_Dump(unsigned char *Adr, int Palette)
		DECL Cell_8x8_Dump

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		xor eax, eax					; eax = 0
		mov ebp, [esp + 32]				; ebp = palette number
		mov edx, 20					; edx = Number of rows of the pattern to be copied
		mov esi, [esp + 28]				; esi = Address
		
		; Check if 32-bit color is in use.
		cmp byte [_bppMD], 32
		je .32BIT
		
		shl ebp, 5					; ebp = palette number * 32
		lea edi, [_MD_Screen	+ 6780]			; edi = MD_Screen + copy offset

	.Loop_EDX
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; on avance de 4 dans la Src
		add edi, 336 * 2				; va sur la prochaine ligne dans Dest
		dec ebx							; s'il reste des lignes du pattern
		jnz near .Loop_EBX				; alors on continue

		sub edi, ((336 * 8) - 8) * 2	; on retourne 8 lignes en arriere et on avance de 8 pixels pour Dest
		dec ecx							; s'il reste des pattern en longueur	
		jnz near .Loop_ECX				; alors on continue

		add edi, ((336 * 8) - (8 * 16)) * 2		; on avance de 8 lignes et on recule de 16 * 8 pixels (pattern) pour Dest
		dec edx
		jnz near .Loop_EDX
		jmp near .END
	
	.32BIT
		shl ebp, 6
		lea edi, [_MD_Screen32 + 13560 ]	; edi = MD_Screen + copy offset

	.Loop_EDX32
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX32
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX32
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX32				; then keep going

		sub edi, ((336 * 8) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX32			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX32
	.END
		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN32
	
		;void Cell_16x16_Dump(unsigned char *Adr, int Palette)
		DECL Cell_16x16_Dump

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		xor eax, eax					; eax = 0
		mov ebp, [esp + 32]				; ebp = palette_number
		mov edx, 10					; edx = Number of rows of the pattern to be copied
		mov esi, [esp + 28]				; esi = Address
		shl ebp, 5					; ebp = palette_number * 32
		
		; Check if 32-bit color is in use.
		cmp byte [_bppMD], 32
		je .32BIT
		
		lea edi, [_MD_Screen	+ 6780]			; edi = MD_Screen + copy offset

	.Loop_EDX
		mov ecx, 16					; ecx = Number of patterns per row

	.Loop_ECX
		mov ebx, 8					; ebx = Number of rows in each pattern

	.Loop_EBX
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; on avance de 4 dans la Src
		add edi, 336 * 2				; va sur la prochaine ligne dans Dest
		dec ebx							; s'il reste des lignes du pattern
		jnz near .Loop_EBX				; alors on continue

		sub edi, ((336 * 8) - 8) * 2	; on retourne 8 lignes en arriere et on avance de 8 pixels pour Dest
		add esi, 0x20					; pattern � droite
		dec ecx							; s'il reste des pattern en longueur	
		jnz near .Loop_ECX				; alors on continue

		add edi, ((336 * 8) - (8 * 16)) * 2		; on avance de 8 lignes et on recule de 16 * 8 pixels (pattern) pour Dest
		sub esi, 0x400 - 0x20
		mov ecx, 16						; ecx = Nombre de pattern par ligne

	.Loop_ECX_2
		mov ebx, 8						; ebx = Nombre de ligne de chaque pattern

	.Loop_EBX_2
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4					; on avance de 4 dans la Src
		add edi, 336 * 2				; va sur la prochaine ligne dans Dest
		dec ebx						; s'il reste des lignes du pattern
		jnz near .Loop_EBX_2				; alors on continue

		sub edi, ((336 * 8) - 8) * 2			; on retourne 8 lignes en arriere et on avance de 8 pixels pour Dest
		add esi, 0x20					; pattern � droite
		dec ecx						; s'il reste des pattern en longueur	
		jnz near .Loop_ECX_2				; alors on continue

		add edi, ((336 * 8) - (8 * 16)) * 2		; on avance de 8 lignes et on recule de 16 * 8 pixels (pattern) pour Dest
		sub esi, 0x20
		dec edx
		jnz near .Loop_EDX
		jmp near .END
	
	
	.32BIT
		shl ebp, 1
		lea edi, [_MD_Screen32 + 13560]	; edi = MD_Screen + copy offset

	.Loop_EDX32
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX32
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX32
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX32			; then keep going

		sub edi, ((336 * 8) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		add esi, 0x20					; pattern � droite
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX32			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		sub esi, 0x400 - 0x20
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX_232
		mov ebx, 8						; ebx = Number of rows in each pattern

	.Loop_EBX_232
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX_232			; then keep going

		sub edi, ((336 * 8) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		add esi, 0x20					; pattern � droite
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX_232			; then keep going

		add edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		sub esi, 0x20
		dec edx
		jnz near .Loop_EDX32

	.END

		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN32
	
		;void Cell_32x32_Dump(unsigned char *Adr, int Palette)
		DECL Cell_32x32_Dump

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		xor eax, eax					; eax = 0
		mov ebp, [esp + 32]				; ebp = palette_number
		mov edx, 6					; edx = Number of rows of the pattern to be copied
		shl ebp, 5					; ebp = palette_number * 32
		mov esi, [esp + 28]				; esi = Address
		
		; Check if 32-bit color is in use.
		cmp byte [_bppMD], 32
		je .32BIT
		
		lea edi, [_MD_Screen	+ 6780]		; edi = MD_Screen + copy offset

	.Loop_EDX
		mov ecx, 16				; ecx = Number of patterns per row

	.Loop_ECX
		mov ebx, 32				; ebx = Number of rows in each pattern

	.Loop_EBX
		push ebx
		mov ebx, [esi]
		AFF_PIXEL 0, 12
		AFF_PIXEL 1, 8
		AFF_PIXEL 2, 4
		AFF_PIXEL 3, 0
		AFF_PIXEL 4, 28
		AFF_PIXEL 5, 24
		AFF_PIXEL 6, 20
		AFF_PIXEL 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 2				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX				; then keep going

		sub edi, ((336 * 32) - 8) * 2	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX				; then keep going

		add edi, ((336 * 32) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX
		jmp near .END
		
	.32BIT
		shl ebp, 1
		lea edi, [_MD_Screen32 + 13560 ]	; edi = MD_Screen + copy offset

	.Loop_EDX32
		mov ecx, 16						; ecx = Number of patterns per row

	.Loop_ECX32
		mov ebx, 32						; ebx = Number of rows in each pattern

	.Loop_EBX32
		push ebx
		mov ebx, [esi]
		AFF_PIXEL32 0, 12
		AFF_PIXEL32 1, 8
		AFF_PIXEL32 2, 4
		AFF_PIXEL32 3, 0
		AFF_PIXEL32 4, 28
		AFF_PIXEL32 5, 24
		AFF_PIXEL32 6, 20
		AFF_PIXEL32 7, 16
		pop ebx
		add esi, 4						; advance Src by 4
		add edi, 336 * 4				; go to the next Dest row
		dec ebx							; if there are any more rows
		jnz near .Loop_EBX32				; then keep going

		sub edi, ((336 * 32) - 8) * 4	; we skip 8 rows from the top and 8 pixels from the left of Dest
		dec ecx							; if there is more to copy on this row
		jnz near .Loop_ECX32			; then keep going

		add edi, ((336 * 32) - (8 * 16)) * 4	; we skip 8 rows from the top and 16*8 pixels from the left of Dest
		dec edx
		jnz near .Loop_EDX32
		
	.END
		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
	
	
	ALIGN32

	; void CDD_Export_Status(void)
	DECL CDD_Export_Status
		push ebx
		push ecx

		mov ax, [_CDD.Status]
		mov bx, [_CDD.Minute]
		mov cx, [_CDD.Seconde]
		mov [_CDD.Rcv_Status + 0], ax
		mov [_CDD.Rcv_Status + 2], bx
		mov [_CDD.Rcv_Status + 4], cx
		add al, bl
		add al, bh
		mov bx, [_CDD.Frame]
		add al, ch
		add al, cl
		and byte [_CDD.Control], 0x3
		add al, ah
		add al, bl
		mov ah, [_CDD.Ext]
		add al, bh
		add al, ah
		mov [_CDD.Rcv_Status + 6], bx
		not al
		and al, 0x0F
		mov [_CDD.Rcv_Status + 8], ax

		pop ecx
		pop ebx
		ret
