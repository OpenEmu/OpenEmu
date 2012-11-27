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

// Disable printf override in common/forbidden.h to avoid
// clashes with pspdebug.h from the PSP SDK.
// That header file uses
//   __attribute__((format(printf,1,2)));
// which gets messed up by our override mechanism; this could
// be avoided by either changing the PSP SDK to use the equally
// legal and valid
//   __attribute__((format(__printf__,1,2)));
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the PSP port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <pspkernel.h>
#include <pspdebug.h>
#include <stdarg.h>
#include <stdio.h>
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

int psp_debug_indent = 0;
bool firstWriteToFile = true;

void PspDebugTrace(bool alsoToScreen, const char *format, ...) {
	va_list	opt;
	char		buffer[2048];
	int			bufsz;
	FILE *fd = 0;

	va_start(opt, format);
	bufsz = vsnprintf(buffer, (size_t) sizeof(buffer), format, opt);
	va_end(opt);

	if (firstWriteToFile) {
		fd = fopen("SCUMMTRACE.TXT", "wb");		// erase the file the first time we write
		firstWriteToFile = false;
	} else {
		fd = fopen("SCUMMTRACE.TXT", "ab");
	}

	if (fd == 0)
		return;

	fwrite(buffer, 1, bufsz, fd);
	fclose(fd);

	if (alsoToScreen)
		fprintf(stderr, buffer);
}

// Assembly functions to get the Return Address register and the Stack pointer register
#define GET_RET(retAddr) \
	asm volatile ("move %0,$ra\n\t"	\
		 : "=&r" (retAddr) : )

#define GET_SP(stackPtr) \
	asm volatile ("move %0,$sp\n\t"	\
		 : "=&r" (stackPtr) : )

// Function to retrieve a backtrace for the MIPS processor
// This is not trivial since the MIPS doesn't use a frame pointer.
// Takes the number of levels wanted above the calling function (included) and an array of void *
//
void mipsBacktrace(uint32 levels, void **addresses) {
	// get the current return address
	register byte *retAddr;
	register byte *stackPointer;
	GET_RET(retAddr);
	GET_SP(stackPointer);
	char string[100];

	if (!levels)
		return;

	memset(addresses, 0, sizeof(void *) * levels);

	uint32 curLevel = 0;

	const uint32 SP_SUBTRACT = 0x27bd8000;		// The instruction to subtract from the SP looks like this
	const uint32 SP_SUB_HIGH_MASK = 0xffff8000;	// The mask to check for the subtract SP instruction
	const uint32 SP_SUB_LOW_MASK = 0x0000ffff;	// The mask that gives us how much was subtracted

	// make sure we go out of the stack of this current level
	// we already have the return address for this level from the register
	if (curLevel < levels) {
		void *thisFunc = (void *)mipsBacktrace;
		for (uint32 *seekPtr = (uint32 *)thisFunc; ; seekPtr++) {
			if ((*seekPtr & SP_SUB_HIGH_MASK) == SP_SUBTRACT) {
				// we found the $sp subtraction at the beginning of the function
				int16 subAmount = (int16)((*seekPtr) & SP_SUB_LOW_MASK);
				//sprintf(string, "found local $sp sub at %p. Data[%x]. Sub amount %d\n", seekPtr, *seekPtr, subAmount);
				//fputs(string, stderr);
				stackPointer -= subAmount;
				byte *testRetAddr = (byte *)*((uint32 *)(stackPointer - 4));
				if (testRetAddr != retAddr) {
					sprintf(string, "mismatch in testretAddr.\n");
					fputs(string, stderr);
				}
				break;
			}
		}
	}

	// keep scanning while more levels are requested
	while (curLevel < levels) {
		// now scan backwards from the return address to find the size of the stack
		for(uint32 *seekPtr = (uint32 *)retAddr; ; seekPtr--) {
			if (((*seekPtr) & SP_SUB_HIGH_MASK) == SP_SUBTRACT) {
				// we found the $sp subtraction at the beginning of the function
				int16 subAmount = (int16)((*seekPtr) & SP_SUB_LOW_MASK);
				//sprintf(string, "found $sp sub at %p. Data[%x]. Sub amount %d\n", seekPtr, *seekPtr, subAmount);
				//fputs(string, stderr);
				stackPointer -= subAmount;
				retAddr = (byte *)*((uint32 *)(stackPointer - 4));
				if (retAddr < (byte *)0x8900000 || retAddr > (byte *)0xC900000) {
					sprintf(string, "invalid retAddr %p\n", retAddr);
					fputs(string, stderr);
					return;
				}
				//sprintf(string, "retAddr[%p]\n", retAddr);
				//fputs(string, stderr);
				addresses[curLevel++] = retAddr;
				break;
			}
		}
	}
}
