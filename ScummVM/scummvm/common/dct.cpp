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

// Based on eos' (I)RDFT code which is in turn
// Based upon the (I)DCT code in FFmpeg
// Copyright (c) 2009 Peter Ross <pross@xvid.org>
// Copyright (c) 2010 Alex Converse <alex.converse@gmail.com>
// Copyright (c) 2010 Vitor Sessak

#include "common/dct.h"

namespace Common {

DCT::DCT(int bits, TransformType trans) : _bits(bits), _cos(_bits + 2), _trans(trans), _rdft(0) {
	int n = 1 << _bits;

	_tCos = _cos.getTable();

	_csc2 = new float[n / 2];

	_rdft = new RDFT(_bits, (_trans == DCT_III) ? RDFT::IDFT_C2R : RDFT::DFT_R2C);

	for (int i = 0; i < (n / 2); i++)
		_csc2[i] = 0.5 / sin((M_PI / (2 * n) * (2 * i + 1)));
}

DCT::~DCT() {
	delete _rdft;
	delete[] _csc2;
}

void DCT::calc(float *data) {
	switch (_trans) {
	case DCT_I:
		calcDCTI(data);
		break;
	case DCT_II:
		calcDCTII(data);
		break;
	case DCT_III:
		calcDCTIII(data);
		break;
	case DST_I:
		calcDSTI(data);
		break;
	}
}

/* sin((M_PI * x / (2*n)) */
#define SIN(n,x) (_tCos[(n) - (x)])
/* cos((M_PI * x / (2*n)) */
#define COS(n,x) (_tCos[x])

void DCT::calcDCTI(float *data) {
	int n = 1 << _bits;

	float next = -0.5 * (data[0] - data[n]);

	for (int i = 0; i < (n / 2); i++) {
		float tmp1 = data[i    ];
		float tmp2 = data[n - i];

		float s = SIN(n, 2 * i);
		float c = COS(n, 2 * i);

		c *= tmp1 - tmp2;
		s *= tmp1 - tmp2;

		next += c;

		tmp1 = (tmp1 + tmp2) * 0.5;

		data[i    ] = tmp1 - s;
		data[n - i] = tmp1 + s;
	}

	_rdft->calc(data);

	data[n] = data[1];
	data[1] = next;

	for (int i = 3; i <= n; i += 2)
		data[i] = data[i - 2] - data[i];
}

void DCT::calcDCTII(float *data) {
	int n = 1 << _bits;

	for (int i = 0; i < (n / 2); i++) {
		float tmp1 = data[i        ];
		float tmp2 = data[n - i - 1];

		float s = SIN(n, 2 * i + 1);

		s *= tmp1 - tmp2;

		tmp1 = (tmp1 + tmp2) * 0.5;

		data[i        ] = tmp1 + s;
		data[n - i - 1] = tmp1 - s;
	}

	_rdft->calc(data);

	float next = data[1] * 0.5;

	data[1] *= -1;

	for (int i = n - 2; i >= 0; i -= 2) {
		float inr = data[i    ];
		float ini = data[i + 1];

		float c = COS(n, i);
		float s = SIN(n, i);

		data[i    ] = c * inr + s * ini;
		data[i + 1] = next;

		next += s * inr - c * ini;
	}
}

void DCT::calcDCTIII(float *data) {
	int n = 1 << _bits;

	float next  = data[n - 1];
	float inv_n = 1.0 / n;

	for (int i = n - 2; i >= 2; i -= 2) {
		float val1 = data[i    ];
		float val2 = data[i - 1] - data[i + 1];

		float c = COS(n, i);
		float s = SIN(n, i);

		data[i    ] = c * val1 + s * val2;
		data[i + 1] = s * val1 - c * val2;
	}

	data[1] = 2 * next;

	_rdft->calc(data);

	for (int i = 0; i < (n / 2); i++) {
		float tmp1 = data[i        ] * inv_n;
		float tmp2 = data[n - i - 1] * inv_n;

		float csc = _csc2[i] * (tmp1 - tmp2);

		tmp1 += tmp2;

		data[i        ] = tmp1 + csc;
		data[n - i - 1] = tmp1 - csc;
	}
}

void DCT::calcDSTI(float *data) {
	int n = 1 << _bits;

	data[0] = 0;

	for (int i = 1; i < (n / 2); i++) {
		float tmp1 = data[i    ];
		float tmp2 = data[n - i];
		float s = SIN(n, 2 * i);

		s   *=  tmp1 + tmp2;
		tmp1 = (tmp1 - tmp2) * 0.5;

		data[i    ] = s + tmp1;
		data[n - i] = s - tmp1;
	}

	data[n / 2] *= 2;

	_rdft->calc(data);

	data[0] *= 0.5;

	for (int i = 1; i < (n - 2); i += 2) {
		data[i + 1] +=  data[i - 1];
		data[i    ]  = -data[i + 2];
	}

	data[n - 1] = 0;
}

} // End of namespace Common
