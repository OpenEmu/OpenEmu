// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "hq_shared32.h"

const unsigned __int64 reg_blank = 0x0000000000000000;
const unsigned __int64 const7    = 0x0000000700070007;
const unsigned __int64 treshold  = 0x0000000000300706;

void Interp1(unsigned char * pc, unsigned int c1, unsigned int c2)
{
	//*((int*)pc) = (c1*3+c2)/4;

#ifdef MMX
	__asm
	{
		mov		eax, pc
		movd	mm1, c1
		movd	mm2, c2
		movq	mm0, mm1
		pslld	mm0, 2
		psubd	mm0, mm1
		paddd	mm0, mm2
		psrld	mm0, 2
		movd	[eax], mm0
		EMMS
	}
#else
	__asm
	{
		mov        eax, pc
		mov        edx, c1
		shl        edx, 2
		add        edx, c2
		sub        edx, c1
		shr        edx, 2
		mov        [eax], edx
	}
#endif
}

void Interp2(unsigned char * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
	//*((int*)pc) = (c1*2+c2+c3)/4;

#ifdef MMX
	__asm
	{
		mov		eax, pc
		movd	mm0, c1
		movd	mm1, c2
		movd	mm2, c3
		pslld	mm0, 1
		paddd	mm0, mm1
		paddd	mm0, mm2
		psrad	mm0, 2
		movd [eax], mm0
		EMMS
	}
#else
	__asm
	{
		mov        eax, pc
		mov        edx, c1
		shl        edx, 1
		add        edx, c2
		add        edx, c3
		shr        edx, 2
		mov        [eax], edx
	}
#endif
}

void Interp3(unsigned char * pc, unsigned int c1, unsigned int c2)
{
	//*((int*)pc) = (c1*7+c2)/8;
	//*((int*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
	//	            (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;

#ifdef MMX
	__asm
	{
		mov        eax, pc
		movd       mm1, c1
		movd       mm2, c2
		punpcklbw  mm1, reg_blank
		punpcklbw  mm2, reg_blank
		pmullw     mm1, const7
		paddw      mm1, mm2
		psrlw      mm1, 3
		packuswb   mm1, reg_blank
		movd       [eax], mm1
		EMMS
	}
#else
	__asm
	{
		mov		eax, c1
		mov		ebx, c2
		mov		ecx, eax
		shl		ecx, 3
		sub		ecx, eax
		add		ecx, ebx
		shr		ecx, 3
		mov		eax, pc
		mov		[eax], ecx
	}
#endif
}

void Interp4(unsigned char * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
	//*((int*)pc) = (c1*2+(c2+c3)*7)/16;
	//*((int*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) & 0x000FF000) +
	//              (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) & 0x0FF00FF0)) >> 4;

#ifdef MMX	
	__asm
	{
		mov        eax, pc
		movd       mm1, c1
		movd       mm2, c2
		movd       mm3, c3
		punpcklbw  mm1, reg_blank
		punpcklbw  mm2, reg_blank
		punpcklbw  mm3, reg_blank
		psllw      mm1, 1
		paddw      mm2, mm3
		pmullw     mm2, const7
		paddw      mm1, mm2
		psrlw      mm1, 4
		packuswb   mm1, reg_blank
		movd       [eax], mm1
		EMMS
	}
#else

	__asm
	{
		mov		eax, [c1]
		and		eax, 0FF00h
		shl		eax, 1
		mov		ecx, [c2]
		and		ecx, 0FF00h
		mov		edx, [c3]
		and		edx, 0FF00h
		add		ecx, edx
		imul	ecx, ecx,7
		add		eax, ecx
		and		eax, 0FF000h

		mov		ebx, [c1]
		and		ebx, 0FF00FFh
		shl		ebx, 1
		mov		ecx, [c2]
		and		ecx, 0FF00FFh
		mov		edx, [c3]
		and		edx, 0FF00FFh
		add		ecx, edx
		imul	ecx, ecx,7
		add		ebx, ecx
		and		ebx, 0FF00FF0h

		add		eax, ebx
		shr		eax, 4

		mov		ebx, pc
		mov		[ebx], eax
	}
#endif
}

void Interp5(unsigned char * pc, unsigned int c1, unsigned int c2)
{
	//*((int*)pc) = (c1+c2)/2;

#ifdef MMX
	__asm
	{
		mov		eax, pc
		movd	mm0, c1
		movd	mm1, c2
		paddd	mm0, mm1
		psrad	mm0, 1
		movd	[eax], mm0
		EMMS
	}
#else
	__asm
	{
		mov        eax, pc
		mov        edx, c1
		add        edx, c2
		shr        edx, 1
		mov        [eax], edx
	}
#endif
}


#include "interp.h"

void Interp1_16(unsigned char * pc, unsigned short c1, unsigned short c2)
{
	*((unsigned short*)pc) = interp_16_31(c1, c2);
	//*((int*)pc) = (c1*3+c2)/4;
}

void Interp2_16(unsigned char * pc, unsigned short c1, unsigned short c2, unsigned short c3)
{
	*((unsigned short*)pc) = interp_16_211(c1, c2, c3);
	//*((int*)pc) = (c1*2+c2+c3)/4;
}

void Interp3_16(unsigned char * pc, unsigned short c1, unsigned short c2)
{
	*((unsigned short*)pc) = interp_16_71(c1, c2);	
//	*((unsigned short*)pc) = (c1*7+c2)/8;
//	*((unsigned short*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
//		            (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

void Interp4_16(unsigned char * pc, unsigned short c1, unsigned short c2, unsigned short c3)
{
	*((unsigned short*)pc) = interp_16_772(c2, c3, c1);
//	*((unsigned short*)pc) = (c1*2+(c2+c3)*7)/16;
//	*((unsigned short*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) & 0x000FF000) +
//	              (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) & 0x0FF00FF0)) >> 4;
}

void Interp5_16(unsigned char * pc, unsigned short c1, unsigned short c2)
{
	*((unsigned short*)pc) = interp_16_11(c1, c2);
}




bool Diff(unsigned int c1, unsigned int c2)
{
	unsigned int
		YUV1 = RGBtoYUV(c1),
		YUV2 = RGBtoYUV(c2);

	if (YUV1 == YUV2) return false; // Save some processing power

#ifdef MMX
	unsigned int retval;
	__asm
	{
   		mov		eax, 0x7FFFFFFF
		movd	mm7, eax			;mm7 = ABS_MASK = 0x7FFFFFFF

		; Copy source colors in first reg
		movd	mm0, YUV1
		movd	mm1, YUV2

		mov		eax, 0x00FF0000
		movd	mm6, eax			;mm6 = Ymask = 0x00FF0000

		; Calculate color Y difference
		movq	mm2, mm0
		movq	mm3, mm1
		pand	mm2, mm6
		pand	mm3, mm6
		psubd	mm2, mm3
		pand	mm2, mm7

		mov		eax, 0x0000FF00
		movd	mm6, eax			;mm6 = Umask = 0x0000FF00

		; Calculate color U difference
		movq	mm3, mm0
		movq	mm4, mm1
		pand	mm3, mm6
		pand	mm4, mm6
		psubd	mm3, mm4
		pand	mm3, mm7

		mov		eax, 0x000000FF
		movd	mm6, eax			;mm6 = Vmask = 0x000000FF

		; Calculate color V difference
		movq	mm4, mm0
		movq	mm5, mm1
		pand	mm4, mm6
		pand	mm5, mm6
		psubd	mm4, mm5
		pand	mm4, mm7

   		mov		eax, 0x00300000
		movd	mm5, eax			;mm5 = trY = 0x00300000
   		mov		eax, 0x00000700
		movd	mm6, eax			;mm6 = trU = 0x00000700
   		mov		eax, 0x00000006
		movd	mm7, eax			;mm7 = trV = 0x00000006

		; Compare the results
        pcmpgtd	mm2, trY
        pcmpgtd	mm3, trU
        pcmpgtd	mm4, trV
		por		mm2, mm3
		por		mm2, mm4
		
		movd	retval, mm2

		EMMS
	}
	return (retval != 0);
#else
	return
		( abs32((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
		( abs32((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
		( abs32((YUV1 & Vmask) - (YUV2 & Vmask)) > trV );
#endif
}


unsigned int RGBtoYUV(unsigned int c)
{	// Division through 3 slows down the emulation about 10% !!!
#ifdef MMX
	unsigned int retval;
	__asm
	{
		movd	mm0, c
		movq	mm1, mm0
		movq	mm2, mm0		;mm0=mm1=mm2=c

		mov		eax, 0x000000FF
		movd	mm5, eax		;mm5 = REDMASK = 0x000000FF
		mov		eax, 0x0000FF00
		movd	mm6, eax		;mm6 = GREENMASK = 0x0000FF00
		mov		eax, 0x00FF0000
		movd	mm7, eax		;mm7 = BLUEMASK = 0x00FF0000


		pand	mm0, mm5
		pand	mm1, mm6
		pand	mm2, mm7		;mm0=R mm1=G mm2=B

		movq	mm3, mm0
		paddd	mm3, mm1
		paddd	mm3, mm2
;		psrld	mm3, 2			;mm3=Y
;		pslld	mm3, 16
		pslld	mm3, 14			;mm3=Y<<16

		mov		eax, 512
		movd	mm7, eax		;mm7 = 128 << 2 = 512

		movq	mm4, mm0
		psubd	mm4, mm2
;		psrld	mm4, 2
;		paddd	mm4, mm7		;mm4=U
;		pslld	mm4, 8			;mm4=U<<8
		paddd	mm4, mm7
		pslld	mm4, 6

		mov		eax, 128
		movd	mm7, eax		;mm7 = 128

		movq	mm5, mm1
		pslld	mm5, 1
		psubd	mm5, mm0
		psubd	mm5, mm2
		psrld	mm5, 3
		paddd	mm5, mm7		;mm5=V

		paddd	mm5, mm4
		paddd	mm5, mm3

        movd	retval, mm5

		EMMS
	}
	return retval;
#else
	unsigned char r, g, b, Y, u, v;
	r = (c & 0x000000FF);
	g = (c & 0x0000FF00) >> 8;
	b = (c & 0x00FF0000) >> 16;
	Y = (r + g + b) >> 2;
	u = 128 + ((r - b) >> 2);
	v = 128 + ((-r + 2*g -b)>>3);
	return (Y<<16) + (u<<8) + v;

	// Extremely High Quality Code
	//unsigned char r, g, b;
	//r = c & 0xFF;
	//g = (c >> 8) & 0xFF;
	//b = (c >> 16) & 0xFF;
	//unsigned char y, u, v;
	//y = (0.256788 * r  +  0.504129 * g  +  0.097906 * b) + 16;
	//u = (-0.148223 * r  -  0.290993 * g  +  0.439216 * b) + 128;
	//v = (0.439216 * r  -  0.367788 * g  -  0.071427 * b) + 128;
	//return (y << 16) + (u << 8) + v;
#endif
}
