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


#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "scumm/bomp.h"
#include "scumm/smush/codec47.h"

namespace Scumm {

#if defined(SCUMM_NEED_ALIGNMENT)

#define COPY_4X1_LINE(dst, src)			\
	do {					\
		(dst)[0] = (src)[0];	\
		(dst)[1] = (src)[1];	\
		(dst)[2] = (src)[2];	\
		(dst)[3] = (src)[3];	\
	} while (0)

#define COPY_2X1_LINE(dst, src)			\
	do {					\
		(dst)[0] = (src)[0];	\
		(dst)[1] = (src)[1];	\
	} while (0)


#else /* SCUMM_NEED_ALIGNMENT */

#define COPY_4X1_LINE(dst, src)			\
	*(uint32 *)(dst) = *(const uint32 *)(src)

#define COPY_2X1_LINE(dst, src)			\
	*(uint16 *)(dst) = *(const uint16 *)(src)

#endif

#define FILL_4X1_LINE(dst, val)			\
	do {					\
		(dst)[0] = val;	\
		(dst)[1] = val;	\
		(dst)[2] = val;	\
		(dst)[3] = val;	\
	} while (0)

#define FILL_2X1_LINE(dst, val)			\
	do {					\
		(dst)[0] = val;	\
		(dst)[1] = val;	\
	} while (0)

static const  int8 codec47_table_small1[] = {
  0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 1, 2, 2, 1,
};

static const int8 codec47_table_small2[] = {
  0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 1, 1, 2, 2,
};

static const int8 codec47_table_big1[] = {
  0, 2, 5, 7, 7, 7, 7, 7, 7, 5, 2, 0, 0, 0, 0, 0,
};

static const int8 codec47_table_big2[] = {
  0, 0, 0, 0, 1, 3, 4, 6, 7, 7, 7, 7, 6, 4, 3, 1,
};

static const int8 codec47_table[] = {
		0,   0,  -1, -43,   6, -43,  -9, -42,  13, -41,
	-16, -40,  19, -39, -23, -36,  26, -34,  -2, -33,
	  4, -33, -29, -32,  -9, -32,  11, -31, -16, -29,
	 32, -29,  18, -28, -34, -26, -22, -25,  -1, -25,
	  3, -25,  -7, -24,   8, -24,  24, -23,  36, -23,
	-12, -22,  13, -21, -38, -20,   0, -20, -27, -19,
	 -4, -19,   4, -19, -17, -18,  -8, -17,   8, -17,
	 18, -17,  28, -17,  39, -17, -12, -15,  12, -15,
	-21, -14,  -1, -14,   1, -14, -41, -13,  -5, -13,
	  5, -13,  21, -13, -31, -12, -15, -11,  -8, -11,
	  8, -11,  15, -11,  -2, -10,   1, -10,  31, -10,
	-23,  -9, -11,  -9,  -5,  -9,   4,  -9,  11,  -9,
	 42,  -9,   6,  -8,  24,  -8, -18,  -7,  -7,  -7,
	 -3,  -7,  -1,  -7,   2,  -7,  18,  -7, -43,  -6,
	-13,  -6,  -4,  -6,   4,  -6,   8,  -6, -33,  -5,
	 -9,  -5,  -2,  -5,   0,  -5,   2,  -5,   5,  -5,
	 13,  -5, -25,  -4,  -6,  -4,  -3,  -4,   3,  -4,
	  9,  -4, -19,  -3,  -7,  -3,  -4,  -3,  -2,  -3,
	 -1,  -3,   0,  -3,   1,  -3,   2,  -3,   4,  -3,
	  6,  -3,  33,  -3, -14,  -2, -10,  -2,  -5,  -2,
	 -3,  -2,  -2,  -2,  -1,  -2,   0,  -2,   1,  -2,
	  2,  -2,   3,  -2,   5,  -2,   7,  -2,  14,  -2,
	 19,  -2,  25,  -2,  43,  -2,  -7,  -1,  -3,  -1,
	 -2,  -1,  -1,  -1,   0,  -1,   1,  -1,   2,  -1,
	  3,  -1,  10,  -1,  -5,   0,  -3,   0,  -2,   0,
	 -1,   0,   1,   0,   2,   0,   3,   0,   5,   0,
	  7,   0, -10,   1,  -7,   1,  -3,   1,  -2,   1,
	 -1,   1,   0,   1,   1,   1,   2,   1,   3,   1,
	-43,   2, -25,   2, -19,   2, -14,   2,  -5,   2,
	 -3,   2,  -2,   2,  -1,   2,   0,   2,   1,   2,
	  2,   2,   3,   2,   5,   2,   7,   2,  10,   2,
	 14,   2, -33,   3,  -6,   3,  -4,   3,  -2,   3,
	 -1,   3,   0,   3,   1,   3,   2,   3,   4,   3,
	 19,   3,  -9,   4,  -3,   4,   3,   4,   7,   4,
	 25,   4, -13,   5,  -5,   5,  -2,   5,   0,   5,
	  2,   5,   5,   5,   9,   5,  33,   5,  -8,   6,
	 -4,   6,   4,   6,  13,   6,  43,   6, -18,   7,
	 -2,   7,   0,   7,   2,   7,   7,   7,  18,   7,
	-24,   8,  -6,   8, -42,   9, -11,   9,  -4,   9,
	  5,   9,  11,   9,  23,   9, -31,  10,  -1,  10,
	  2,  10, -15,  11,  -8,  11,   8,  11,  15,  11,
	 31,  12, -21,  13,  -5,  13,   5,  13,  41,  13,
	 -1,  14,   1,  14,  21,  14, -12,  15,  12,  15,
	-39,  17, -28,  17, -18,  17,  -8,  17,   8,  17,
	 17,  18,  -4,  19,   0,  19,   4,  19,  27,  19,
	 38,  20, -13,  21,  12,  22, -36,  23, -24,  23,
	 -8,  24,   7,  24,  -3,  25,   1,  25,  22,  25,
	 34,  26, -18,  28, -32,  29,  16,  29, -11,  31,
		9,  32,  29,  32,  -4,  33,   2,  33, -26,  34,
	 23,  36, -19,  39,  16,  40, -13,  41,   9,  42,
	 -6,  43,   1,  43,   0,   0,   0,   0,   0,   0
};

void Codec47Decoder::makeTablesInterpolation(int param) {
	int32 variable1, variable2;
	int32 b1, b2;
	int32 value_table47_1_2, value_table47_1_1, value_table47_2_2, value_table47_2_1;
	int32 tableSmallBig[64], tmp, s;
	const int8 *table47_1 = 0, *table47_2 = 0;
	int32 *ptr_small_big;
	byte *ptr;
	int i, x, y;

	if (param == 8) {
		table47_1 = codec47_table_big1;
		table47_2 = codec47_table_big2;
		ptr = _tableBig;
		for (i = 0; i < 256; i++) {
			ptr[384] = 0;
			ptr[385] = 0;
			ptr += 388;
		}
	} else if (param == 4) {
		table47_1 = codec47_table_small1;
		table47_2 = codec47_table_small2;
		ptr = _tableSmall;
		for (i = 0; i < 256; i++) {
			ptr[96] = 0;
			ptr[97] = 0;
			ptr += 128;
		}
	} else {
		error("Codec47Decoder::makeTablesInterpolation: unknown param %d", param);
	}

	s = 0;
	for (x = 0; x < 16; x++) {
		value_table47_1_1 = table47_1[x];
		value_table47_2_1 = table47_2[x];
		for (y = 0; y < 16; y++) {
			value_table47_1_2 = table47_1[y];
			value_table47_2_2 = table47_2[y];

			if (value_table47_2_1 == 0) {
				b1 = 0;
			} else if (value_table47_2_1 == param - 1) {
				b1 = 1;
			} else if (value_table47_1_1 == 0) {
				b1 = 2;
			} else if (value_table47_1_1 == param - 1) {
				b1 = 3;
			} else {
				b1 = 4;
			}

			if (value_table47_2_2 == 0) {
				b2 = 0;
			} else if (value_table47_2_2 == param - 1) {
				b2 = 1;
			} else if (value_table47_1_2 == 0) {
				b2 = 2;
			} else if (value_table47_1_2 == param - 1) {
				b2 = 3;
			} else {
				b2 = 4;
			}

			memset(tableSmallBig, 0, param * param * 4);

			variable2 = ABS(value_table47_2_2 - value_table47_2_1);
			tmp = ABS(value_table47_1_2 - value_table47_1_1);
			if (variable2 <= tmp) {
				variable2 = tmp;
			}

			for (variable1 = 0; variable1 <= variable2; variable1++) {
				int32 variable3, variable4;

				if (variable2 > 0) {
					// Linearly interpolate between value_table47_1_1 and value_table47_1_2
					// respectively value_table47_2_1 and value_table47_2_2.
					variable4 = (value_table47_1_1 * variable1 + value_table47_1_2 * (variable2 - variable1) + variable2 / 2) / variable2;
					variable3 = (value_table47_2_1 * variable1 + value_table47_2_2 * (variable2 - variable1) + variable2 / 2) / variable2;
				} else {
					variable4 = value_table47_1_1;
					variable3 = value_table47_2_1;
				}
				ptr_small_big = &tableSmallBig[param * variable3 + variable4];
				*ptr_small_big = 1;

				if ((b1 == 2 && b2 == 3) || (b2 == 2 && b1 == 3) ||
				    (b1 == 0 && b2 != 1) || (b2 == 0 && b1 != 1)) {
					if (variable3 >= 0) {
						i = variable3 + 1;
						while (i--) {
							*ptr_small_big = 1;
							ptr_small_big -= param;
						}
					}
				} else if ((b2 != 0 && b1 == 1) || (b1 != 0 && b2 == 1)) {
					if (param > variable3) {
						i = param - variable3;
						while (i--) {
							*ptr_small_big = 1;
							ptr_small_big += param;
						}
					}
				} else if ((b1 == 2 && b2 != 3) || (b2 == 2 && b1 != 3)) {
					if (variable4 >= 0) {
						i = variable4 + 1;
						while (i--) {
							*(ptr_small_big--) = 1;
						}
					}
				} else if ((b1 == 0 && b2 == 1) || (b2 == 0 && b1 == 1) ||
				           (b1 == 3 && b2 != 2) || (b2 == 3 && b1 != 2)) {
					if (param > variable4) {
						i = param - variable4;
						while (i--) {
							*(ptr_small_big++) = 1;
						}
					}
				}
			}

			if (param == 8) {
				for (i = 64 - 1; i >= 0; i--) {
					if (tableSmallBig[i] != 0) {
						_tableBig[256 + s + _tableBig[384 + s]] = (byte)i;
						_tableBig[384 + s]++;
					} else {
						_tableBig[320 + s + _tableBig[385 + s]] = (byte)i;
						_tableBig[385 + s]++;
					}
				}
				s += 388;
			}
			if (param == 4) {
				for (i = 16 - 1; i >= 0; i--) {
					if (tableSmallBig[i] != 0) {
						_tableSmall[64 + s + _tableSmall[96 + s]] = (byte)i;
						_tableSmall[96 + s]++;
					} else {
						_tableSmall[80 + s + _tableSmall[97 + s]] = (byte)i;
						_tableSmall[97 + s]++;
					}
				}
				s += 128;
			}
		}
	}
}

void Codec47Decoder::makeTables47(int width) {
	if (_lastTableWidth == width)
		return;

	_lastTableWidth = width;

	int32 a, c, d;
	int16 tmp;

	for (int l = 0; l < ARRAYSIZE(codec47_table); l += 2) {
		_table[l / 2] = (int16)(codec47_table[l + 1] * width + codec47_table[l]);
	}
	// Note: _table[255] is never inited; but since only the first 0xF8
	// entries of it are used anyway, this doesn't matter.

	a = 0;
	c = 0;
	do {
		for (d = 0; d < _tableSmall[96 + c]; d++) {
			tmp = _tableSmall[64 + c + d];
			tmp = (int16)((byte)(tmp >> 2) * width + (tmp & 3));
			_tableSmall[c + d * 2] = (byte)tmp;
			_tableSmall[c + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableSmall[97 + c]; d++) {
			tmp = _tableSmall[80 + c + d];
			tmp = (int16)((byte)(tmp >> 2) * width + (tmp & 3));
			_tableSmall[32 + c + d * 2] = (byte)tmp;
			_tableSmall[32 + c + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableBig[384 + a]; d++) {
			tmp = _tableBig[256 + a + d];
			tmp = (int16)((byte)(tmp >> 3) * width + (tmp & 7));
			_tableBig[a + d * 2] = (byte)tmp;
			_tableBig[a + d * 2 + 1] = tmp >> 8;
		}
		for (d = 0; d < _tableBig[385 + a]; d++) {
			tmp = _tableBig[320 + a + d];
			tmp = (int16)((byte)(tmp >> 3) * width + (tmp & 7));
			_tableBig[128 + a + d * 2] = (byte)tmp;
			_tableBig[128 + a + d * 2 + 1] = tmp >> 8;
		}

		a += 388;
		c += 128;
	} while (c < 32768);
}

#ifdef USE_ARM_SMUSH_ASM

#ifndef IPHONE
#define ARM_Smush_decode2 _ARM_Smush_decode2
#endif

extern "C" void ARM_Smush_decode2(      byte  *dst,
                                  const byte  *src,
                                        int    width,
                                        int    height,
                                  const byte  *param_ptr,
                                        int16 *_table,
                                        byte  *_tableBig,
                                        int32  offset1,
                                        int32  offset2,
                                        byte  *_tableSmall);

#define decode2(SRC,DST,WIDTH,HEIGHT,PARAM) \
 ARM_Smush_decode2(SRC,DST,WIDTH,HEIGHT,PARAM,_table,_tableBig, \
                   _offset1,_offset2,_tableSmall)

#else
void Codec47Decoder::level3(byte *d_dst) {
	int32 tmp;
	byte code = *_d_src++;

	if (code < 0xF8) {
		tmp = _table[code] + _offset1;
		COPY_2X1_LINE(d_dst, d_dst + tmp);
		COPY_2X1_LINE(d_dst + _d_pitch, d_dst + _d_pitch + tmp);
	} else if (code == 0xFF) {
		COPY_2X1_LINE(d_dst, _d_src + 0);
		COPY_2X1_LINE(d_dst + _d_pitch, _d_src + 2);
		_d_src += 4;
	} else if (code == 0xFE) {
		byte t = *_d_src++;
		FILL_2X1_LINE(d_dst, t);
		FILL_2X1_LINE(d_dst + _d_pitch, t);
	} else if (code == 0xFC) {
		tmp = _offset2;
		COPY_2X1_LINE(d_dst, d_dst + tmp);
		COPY_2X1_LINE(d_dst + _d_pitch, d_dst + _d_pitch + tmp);
	} else {
		byte t = _paramPtr[code];
		FILL_2X1_LINE(d_dst, t);
		FILL_2X1_LINE(d_dst + _d_pitch, t);
	}
}

void Codec47Decoder::level2(byte *d_dst) {
	int32 tmp;
	byte code = *_d_src++;
	int i;

	if (code < 0xF8) {
		tmp = _table[code] + _offset1;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst, d_dst + tmp);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFF) {
		level3(d_dst);
		d_dst += 2;
		level3(d_dst);
		d_dst += _d_pitch * 2 - 2;
		level3(d_dst);
		d_dst += 2;
		level3(d_dst);
	} else if (code == 0xFE) {
		byte t = *_d_src++;
		for (i = 0; i < 4; i++) {
			FILL_4X1_LINE(d_dst, t);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFD) {
		byte *tmp_ptr = _tableSmall + *_d_src++ * 128;
		int32 l = tmp_ptr[96];
		byte val = *_d_src++;
		int16 *tmp_ptr2 = (int16 *)tmp_ptr;
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
		l = tmp_ptr[97];
		val = *_d_src++;
		tmp_ptr2 = (int16 *)(tmp_ptr + 32);
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
	} else if (code == 0xFC) {
		tmp = _offset2;
		for (i = 0; i < 4; i++) {
			COPY_4X1_LINE(d_dst, d_dst + tmp);
			d_dst += _d_pitch;
		}
	} else {
		byte t = _paramPtr[code];
		for (i = 0; i < 4; i++) {
			FILL_4X1_LINE(d_dst, t);
			d_dst += _d_pitch;
		}
	}
}

void Codec47Decoder::level1(byte *d_dst) {
	int32 tmp, tmp2;
	byte code = *_d_src++;
	int i;

	if (code < 0xF8) {
		tmp2 = _table[code] + _offset1;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp2);
			COPY_4X1_LINE(d_dst + 4, d_dst + tmp2 + 4);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFF) {
		level2(d_dst);
		d_dst += 4;
		level2(d_dst);
		d_dst += _d_pitch * 4 - 4;
		level2(d_dst);
		d_dst += 4;
		level2(d_dst);
	} else if (code == 0xFE) {
		byte t = *_d_src++;
		for (i = 0; i < 8; i++) {
			FILL_4X1_LINE(d_dst, t);
			FILL_4X1_LINE(d_dst + 4, t);
			d_dst += _d_pitch;
		}
	} else if (code == 0xFD) {
		tmp = *_d_src++;
		byte *tmp_ptr = _tableBig + tmp * 388;
		byte l = tmp_ptr[384];
		byte val = *_d_src++;
		int16 *tmp_ptr2 = (int16 *)tmp_ptr;
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
		l = tmp_ptr[385];
		val = *_d_src++;
		tmp_ptr2 = (int16 *)(tmp_ptr + 128);
		while (l--) {
			*(d_dst + READ_LE_UINT16(tmp_ptr2)) = val;
			tmp_ptr2++;
		}
	} else if (code == 0xFC) {
		tmp2 = _offset2;
		for (i = 0; i < 8; i++) {
			COPY_4X1_LINE(d_dst + 0, d_dst + tmp2);
			COPY_4X1_LINE(d_dst + 4, d_dst + tmp2 + 4);
			d_dst += _d_pitch;
		}
	} else {
		byte t = _paramPtr[code];
		for (i = 0; i < 8; i++) {
			FILL_4X1_LINE(d_dst, t);
			FILL_4X1_LINE(d_dst + 4, t);
			d_dst += _d_pitch;
		}
	}
}

void Codec47Decoder::decode2(byte *dst, const byte *src, int width, int height, const byte *param_ptr) {
	_d_src = src;
	_paramPtr = param_ptr - 0xf8;
	int bw = (width + 7) / 8;
	int bh = (height + 7) / 8;
	int next_line = width * 7;
	_d_pitch = width;

	do {
		int tmp_bw = bw;
		do {
			level1(dst);
			dst += 8;
		} while (--tmp_bw);
		dst += next_line;
	} while (--bh);
}
#endif

Codec47Decoder::Codec47Decoder(int width, int height) {
	_lastTableWidth = -1;
	_width = width;
	_height = height;
	_tableBig = (byte *)malloc(256 * 388);
	_tableSmall = (byte *)malloc(256 * 128);
	if ((_tableBig != NULL) && (_tableSmall != NULL)) {
		makeTablesInterpolation(4);
		makeTablesInterpolation(8);
	}

	_frameSize = _width * _height;
	_deltaSize = _frameSize * 3;
	_deltaBuf = (byte *)malloc(_deltaSize);
	_deltaBufs[0] = _deltaBuf;
	_deltaBufs[1] = _deltaBuf + _frameSize;
	_curBuf = _deltaBuf + _frameSize * 2;
}

Codec47Decoder::~Codec47Decoder() {
	if (_tableBig) {
		free(_tableBig);
		_tableBig = NULL;
	}
	if (_tableSmall) {
		free(_tableSmall);
		_tableSmall = NULL;
	}
	_lastTableWidth = -1;
	if (_deltaBuf) {
		free(_deltaBuf);
		_deltaSize = 0;
		_deltaBuf = NULL;
		_deltaBufs[0] = NULL;
		_deltaBufs[1] = NULL;
	}
}

bool Codec47Decoder::decode(byte *dst, const byte *src) {
	if ((_tableBig == NULL) || (_tableSmall == NULL) || (_deltaBuf == NULL))
		return false;

	_offset1 = _deltaBufs[1] - _curBuf;
	_offset2 = _deltaBufs[0] - _curBuf;

	int32 seq_nb = READ_LE_UINT16(src + 0);

	const byte *gfx_data = src + 26;

	if (seq_nb == 0) {
		makeTables47(_width);
		memset(_deltaBufs[0], src[12], _frameSize);
		memset(_deltaBufs[1], src[13], _frameSize);
		_prevSeqNb = -1;
	}

	if ((src[4] & 1) != 0) {
		gfx_data += 32896;
	}

	switch (src[2]) {
	case 0:
		memcpy(_curBuf, gfx_data, _frameSize);
		break;
	case 1:
		// Used by Outlaws, but not by any SCUMM game.
		error("codec47: not implemented decode1 proc");
		break;
	case 2:
		if (seq_nb == _prevSeqNb + 1) {
			decode2(_curBuf, gfx_data, _width, _height, src + 8);
		}
		break;
	case 3:
		memcpy(_curBuf, _deltaBufs[1], _frameSize);
		break;
	case 4:
		memcpy(_curBuf, _deltaBufs[0], _frameSize);
		break;
	case 5:
		bompDecodeLine(_curBuf, gfx_data, READ_LE_UINT32(src + 14));
		break;
	}

	memcpy(dst, _curBuf, _frameSize);

	if (seq_nb == _prevSeqNb + 1) {
		if (src[3] == 1) {
			SWAP(_curBuf, _deltaBufs[1]);
		} else if (src[3] == 2) {
			SWAP(_deltaBufs[0], _deltaBufs[1]);
			SWAP(_deltaBufs[1], _curBuf);
		}
	}
	_prevSeqNb = seq_nb;

	return true;
}

} // End of namespace Scumm
