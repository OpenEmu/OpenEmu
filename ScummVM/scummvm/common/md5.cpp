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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * RFC 1321 compliant MD5 implementation,
 * by Christophe Devine <devine(at)cr0.net>
 * this program is licensed under the GPL.
 */

#include "common/md5.h"
#include "common/endian.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {

struct md5_context {
	uint32 total[2];
	uint32 state[4];
	uint8 buffer[64];
};

static void md5_starts(md5_context *ctx);
static void md5_update(md5_context *ctx, const uint8 *input, uint32 length);
static void md5_finish(md5_context *ctx, uint8 digest[16]);


#define GET_UINT32(n, b, i)	(n) = READ_LE_UINT32(b + i)
#define PUT_UINT32(n, b, i)	WRITE_LE_UINT32(b + i, n)

void md5_starts(md5_context *ctx) {
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
}

static void md5_process(md5_context *ctx, const uint8 data[64]) {
	uint32 X[16], A, B, C, D;

	GET_UINT32(X[0],  data,  0);
	GET_UINT32(X[1],  data,  4);
	GET_UINT32(X[2],  data,  8);
	GET_UINT32(X[3],  data, 12);
	GET_UINT32(X[4],  data, 16);
	GET_UINT32(X[5],  data, 20);
	GET_UINT32(X[6],  data, 24);
	GET_UINT32(X[7],  data, 28);
	GET_UINT32(X[8],  data, 32);
	GET_UINT32(X[9],  data, 36);
	GET_UINT32(X[10], data, 40);
	GET_UINT32(X[11], data, 44);
	GET_UINT32(X[12], data, 48);
	GET_UINT32(X[13], data, 52);
	GET_UINT32(X[14], data, 56);
	GET_UINT32(X[15], data, 60);

#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a, b, c, d, k, s, t)                    \
{                                                 \
	a += F(b,c,d) + X[k] + t; a = S(a,s) + b; \
}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];

#define F(x, y, z) (z ^ (x & (y ^ z)))

	P(A, B, C, D,  0,  7, 0xD76AA478);
	P(D, A, B, C,  1, 12, 0xE8C7B756);
	P(C, D, A, B,  2, 17, 0x242070DB);
	P(B, C, D, A,  3, 22, 0xC1BDCEEE);
	P(A, B, C, D,  4,  7, 0xF57C0FAF);
	P(D, A, B, C,  5, 12, 0x4787C62A);
	P(C, D, A, B,  6, 17, 0xA8304613);
	P(B, C, D, A,  7, 22, 0xFD469501);
	P(A, B, C, D,  8,  7, 0x698098D8);
	P(D, A, B, C,  9, 12, 0x8B44F7AF);
	P(C, D, A, B, 10, 17, 0xFFFF5BB1);
	P(B, C, D, A, 11, 22, 0x895CD7BE);
	P(A, B, C, D, 12,  7, 0x6B901122);
	P(D, A, B, C, 13, 12, 0xFD987193);
	P(C, D, A, B, 14, 17, 0xA679438E);
	P(B, C, D, A, 15, 22, 0x49B40821);

#undef F

#define F(x, y, z) (y ^ (z & (x ^ y)))

	P(A, B, C, D,  1,  5, 0xF61E2562);
	P(D, A, B, C,  6,  9, 0xC040B340);
	P(C, D, A, B, 11, 14, 0x265E5A51);
	P(B, C, D, A,  0, 20, 0xE9B6C7AA);
	P(A, B, C, D,  5,  5, 0xD62F105D);
	P(D, A, B, C, 10,  9, 0x02441453);
	P(C, D, A, B, 15, 14, 0xD8A1E681);
	P(B, C, D, A,  4, 20, 0xE7D3FBC8);
	P(A, B, C, D,  9,  5, 0x21E1CDE6);
	P(D, A, B, C, 14,  9, 0xC33707D6);
	P(C, D, A, B,  3, 14, 0xF4D50D87);
	P(B, C, D, A,  8, 20, 0x455A14ED);
	P(A, B, C, D, 13,  5, 0xA9E3E905);
	P(D, A, B, C,  2,  9, 0xFCEFA3F8);
	P(C, D, A, B,  7, 14, 0x676F02D9);
	P(B, C, D, A, 12, 20, 0x8D2A4C8A);

#undef F

#define F(x, y, z) (x ^ y ^ z)

	P(A, B, C, D,  5,  4, 0xFFFA3942);
	P(D, A, B, C,  8, 11, 0x8771F681);
	P(C, D, A, B, 11, 16, 0x6D9D6122);
	P(B, C, D, A, 14, 23, 0xFDE5380C);
	P(A, B, C, D,  1,  4, 0xA4BEEA44);
	P(D, A, B, C,  4, 11, 0x4BDECFA9);
	P(C, D, A, B,  7, 16, 0xF6BB4B60);
	P(B, C, D, A, 10, 23, 0xBEBFBC70);
	P(A, B, C, D, 13,  4, 0x289B7EC6);
	P(D, A, B, C,  0, 11, 0xEAA127FA);
	P(C, D, A, B,  3, 16, 0xD4EF3085);
	P(B, C, D, A,  6, 23, 0x04881D05);
	P(A, B, C, D,  9,  4, 0xD9D4D039);
	P(D, A, B, C, 12, 11, 0xE6DB99E5);
	P(C, D, A, B, 15, 16, 0x1FA27CF8);
	P(B, C, D, A,  2, 23, 0xC4AC5665);

#undef F

#define F(x, y, z) (y ^ (x | ~z))

	P(A, B, C, D,  0,  6, 0xF4292244);
	P(D, A, B, C,  7, 10, 0x432AFF97);
	P(C, D, A, B, 14, 15, 0xAB9423A7);
	P(B, C, D, A,  5, 21, 0xFC93A039);
	P(A, B, C, D, 12,  6, 0x655B59C3);
	P(D, A, B, C,  3, 10, 0x8F0CCC92);
	P(C, D, A, B, 10, 15, 0xFFEFF47D);
	P(B, C, D, A,  1, 21, 0x85845DD1);
	P(A, B, C, D,  8,  6, 0x6FA87E4F);
	P(D, A, B, C, 15, 10, 0xFE2CE6E0);
	P(C, D, A, B,  6, 15, 0xA3014314);
	P(B, C, D, A, 13, 21, 0x4E0811A1);
	P(A, B, C, D,  4,  6, 0xF7537E82);
	P(D, A, B, C, 11, 10, 0xBD3AF235);
	P(C, D, A, B,  2, 15, 0x2AD7D2BB);
	P(B, C, D, A,  9, 21, 0xEB86D391);

#undef F

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
}

void md5_update(md5_context *ctx, const uint8 *input, uint32 length) {
	uint32 left, fill;

	if (!length)
		return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += length;
	ctx->total[0] &= 0xFFFFFFFF;

	if (ctx->total[0] < length)
		ctx->total[1]++;

	if (left && length >= fill) {
		memcpy((void *)(ctx->buffer + left), (const void *)input, fill);
		md5_process(ctx, ctx->buffer);
		length -= fill;
		input  += fill;
		left = 0;
	}

	while (length >= 64) {
		md5_process(ctx, input);
		length -= 64;
		input  += 64;
	}

	if (length) {
		memcpy((void *)(ctx->buffer + left), (const void *)input, length);
	}
}

static const uint8 md5_padding[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void md5_finish(md5_context *ctx, uint8 digest[16]) {
	uint32 last, padn;
	uint32 high, low;
	uint8 msglen[8];

	high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
	low  = (ctx->total[0] <<  3);

	PUT_UINT32(low,  msglen, 0);
	PUT_UINT32(high, msglen, 4);

	last = ctx->total[0] & 0x3F;
	padn = (last < 56) ? (56 - last) : (120 - last);

	md5_update(ctx, md5_padding, padn);
	md5_update(ctx, msglen, 8);

	PUT_UINT32(ctx->state[0], digest,  0);
	PUT_UINT32(ctx->state[1], digest,  4);
	PUT_UINT32(ctx->state[2], digest,  8);
	PUT_UINT32(ctx->state[3], digest, 12);
}


bool computeStreamMD5(ReadStream &stream, uint8 digest[16], uint32 length) {

#ifdef DISABLE_MD5
	memset(digest, 0, 16);
#else
	md5_context ctx;
	int i;
	unsigned char buf[1000];
	bool restricted = (length != 0);
	uint32 readlen;

	if (!restricted || sizeof(buf) <= length)
		readlen = sizeof(buf);
	else
		readlen = length;

	md5_starts(&ctx);

	while ((i = stream.read(buf, readlen)) > 0) {
		md5_update(&ctx, buf, i);

		if (restricted) {
			length -= i;
			if (length == 0)
				break;

			if (sizeof(buf) > length)
				readlen = length;
		}
	}

	md5_finish(&ctx, digest);
#endif
	return true;
}

String computeStreamMD5AsString(ReadStream &stream, uint32 length) {
	String md5;
	uint8 digest[16];
	if (computeStreamMD5(stream, digest, length)) {
		for (int i = 0; i < 16; i++) {
			md5 += String::format("%02x", (int)digest[i]);
		}
	}

	return md5;
}

} // End of namespace Common
