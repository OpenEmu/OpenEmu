/*
	io_m3_common.c

	Routines common to all version of the M3

	Some code based on M3 SD drivers supplied by M3Adapter.
	Some code written by SaTa may have been unknowingly used.

 Copyright (c) 2006 Michael "Chishm" Chisholm

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "io_m3_common.h"

static u16 _M3_readHalfword (u32 addr) {
	return *((vu16*)addr);
}

void _M3_changeMode (u32 mode) {
	_M3_readHalfword (0x08e00002);
	_M3_readHalfword (0x0800000e);
	_M3_readHalfword (0x08801ffc);
	_M3_readHalfword (0x0800104a);
	_M3_readHalfword (0x08800612);
	_M3_readHalfword (0x08000000);
	_M3_readHalfword (0x08801b66);
	_M3_readHalfword (0x08000000 + (mode << 1));
	_M3_readHalfword (0x0800080e);
	_M3_readHalfword (0x08000000);

	if ((mode & 0x0f) != 4) {
		_M3_readHalfword (0x09000000);
	} else {
		_M3_readHalfword (0x080001e4);
		_M3_readHalfword (0x080001e4);
		_M3_readHalfword (0x08000188);
		_M3_readHalfword (0x08000188);
	}
}
