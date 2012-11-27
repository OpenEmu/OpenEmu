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

// Based on eos' (I)FFT code which is in turn
// based upon the (I)FFT code in FFmpeg
// Copyright (c) 2008 Loren Merritt
// Copyright (c) 2002 Fabrice Bellard
// Partly based on libdjbfft by D. J. Bernstein

#ifndef COMMON_FFT_H
#define COMMON_FFT_H

#include "common/scummsys.h"
#include "common/math.h"

namespace Common {

class CosineTable;

/**
 * (Inverse) Fast Fourier Transform.
 *
 * Used in engines:
 *  - scumm
 */
class FFT {
public:
	FFT(int bits, int inverse);
	~FFT();

	/** Do the permutation needed BEFORE calling calc(). */
	void permute(Complex *z);

	/** Do a complex FFT.
	 *
	 *  The input data must be permuted before.
	 *  No 1.0/sqrt(n) normalization is done.
	 */
	void calc(Complex *z);

private:
	int _bits;
	int _inverse;

	uint16 *_revTab;

	Complex *_expTab;
	Complex *_tmpBuf;

	int _splitRadix;
	int _permutation;

	static int splitRadixPermutation(int i, int n, int inverse);

	CosineTable *_cosTables[13];

	void fft4(Complex *z);
	void fft8(Complex *z);
	void fft16(Complex *z);
	void fft(int n, int logn, Complex *z);
};

} // End of namespace Common

#endif // COMMON_FFT_H
