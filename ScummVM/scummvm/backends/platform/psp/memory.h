
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PSP_MEMORY_H
#define PSP_MEMORY_H

#define MIN_AMOUNT_FOR_COMPLEX_COPY  8
#define MIN_AMOUNT_FOR_MISALIGNED_COPY 8

//#define __PSP_DEBUG_PRINT__

//#include "backends/platform/psp/trace.h"

// These instructions don't generate automatically but are faster then copying byte by byte
inline void lwl_copy(byte *dst, const byte *src) {
	register uint32 data;
	asm volatile ("lwr %0,0(%1)\n\t"
		 "lwl %0,3(%1)\n\t"
		 : "=&r" (data) : "r" (src), "m" (*src));

	asm volatile ("swr %1,0(%2)\n\t"
		 "swl %1,3(%2)\n\t"
		 : "=m" (*dst) : "r" (data), "r" (dst));
}

/**
 *	Class that does memory copying and swapping if needed
 */
class PspMemory {
private:
	static void testCopy(const byte *debugDst, const byte *debugSrc, uint32 debugBytes);
	static void copy(byte *dst, const byte *src, uint32 bytes);
	static void copy32Aligned(uint32 *dst32, const uint32 *src32, uint32 bytes);
	static void copy32Misaligned(uint32 *dst32, const byte *src, uint32 bytes, uint32 alignSrc);

	static inline void copy8(byte *dst, const byte *src, int32 bytes) {
		//PSP_DEBUG_PRINT("copy8 called with dst[%p], src[%p], bytes[%d]\n", dst, src, bytes);
		uint32 words = bytes >> 2;
		for (; words; words--) {
			lwl_copy(dst, src);
			dst += 4;
			src += 4;
		}

		uint32 bytesLeft = bytes & 0x3;
		for (; bytesLeft; bytesLeft--) {
			*dst++ = *src++;
		}
	}

public:
	// This is the interface to the outside world
	static void *fastCopy(void *dstv, const void *srcv, int32 bytes) {
		byte *dst = (byte *)dstv;
		byte *src = (byte *)srcv;

		if (bytes < MIN_AMOUNT_FOR_COMPLEX_COPY) {
			copy8(dst, src, bytes);
		} else {	// go to more powerful copy
			copy(dst, src, bytes);
		}

		return dstv;
	}
};

inline void *psp_memcpy(void *dst, const void *src, int32 bytes) {
	return PspMemory::fastCopy(dst, src, bytes);
}

#endif /* PSP_MEMORY_H */

#if defined(PSP_INCLUDE_SWAP) && !defined(PSP_MEMORY_SWAP_H)
#define PSP_MEMORY_SWAP_H

//#include "backends/platform/psp/psppixelformat.h"

class PspMemorySwap {
private:
	static void testSwap(const uint16 *debugDst, const uint16 *debugSrc, uint32 debugBytes, PSPPixelFormat &format);
	static void swap(uint16 *dst16, const uint16 *src16, uint32 bytes, PSPPixelFormat &format);
	static void swap32Aligned(uint32 *dst32, const uint32 *src32, uint32 bytes, PSPPixelFormat &format);
	static void swap32Misaligned(uint32 *dst32, const uint16 *src16, uint32 bytes, PSPPixelFormat &format);
	// For swapping, we know that we have multiples of 16 bits
	static void swap16(uint16 *dst16, const uint16 *src16, uint32 bytes, PSPPixelFormat &format) {
	PSP_DEBUG_PRINT("swap16 called with dst16[%p], src16[%p], bytes[%d]\n", dst16, src16, bytes);
	uint32 shorts = bytes >> 1;

	while (shorts--) {
		*dst16++ = format.swapRedBlue16(*src16++);
	}
}

public:
	static void fastSwap(byte *dst, const byte *src, uint32 bytes, PSPPixelFormat &format) {
		if (bytes < MIN_AMOUNT_FOR_COMPLEX_COPY * 2) {
			swap16((uint16 *)dst, (uint16 *)src, bytes, format);
		} else {	// go to more powerful copy
			swap((uint16 *)dst, (uint16 *)src, bytes, format);
		}
	}
};

#endif /* PSP_INCLUDE_SWAP */
