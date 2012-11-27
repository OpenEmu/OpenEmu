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

// SAGA Image resource management routines

#include "saga/saga.h"

namespace Saga {

static int granulate(int value, int granularity) {
	int remainder;

	if (value == 0)
		return 0;

	if (granularity == 0)
		return 0;

	remainder = value % granularity;

	if (remainder == 0) {
		return value;
	} else {
		return (granularity - remainder + value);
	}
}

bool SagaEngine::decodeBGImage(const ByteArray &imageData, ByteArray &outputBuffer, int *w, int *h, bool flip) {
	ImageHeader hdr;
	int modex_height;
	const byte *RLE_data_ptr;
	size_t RLE_data_len;
	ByteArray decodeBuffer;

	if (imageData.size() <= SAGA_IMAGE_DATA_OFFSET) {
		error("decodeBGImage() Image size is way too small (%d)", (int)imageData.size());
	}

	ByteArrayReadStreamEndian readS(imageData, isBigEndian());

	hdr.width = readS.readUint16();
	hdr.height = readS.readUint16();
	// The next four bytes of the image header aren't used.
	readS.readUint16();
	readS.readUint16();

	RLE_data_ptr = &imageData.front() + SAGA_IMAGE_DATA_OFFSET;
	RLE_data_len = imageData.size() - SAGA_IMAGE_DATA_OFFSET;

	modex_height = granulate(hdr.height, 4);

	decodeBuffer.resize(hdr.width * modex_height);

	outputBuffer.resize(hdr.width * hdr.height);

	if (!decodeBGImageRLE(RLE_data_ptr, RLE_data_len, decodeBuffer)) {
		return false;
	}

	unbankBGImage(outputBuffer.getBuffer(), decodeBuffer.getBuffer(), hdr.width, hdr.height);

	// For some reason bg images in IHNM are upside down
	if (getGameId() == GID_IHNM && !flip) {
		flipImage(outputBuffer.getBuffer(), hdr.width, hdr.height);
	}

	*w = hdr.width;
	*h = hdr.height;

	return true;
}

bool SagaEngine::decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, ByteArray &outbuf) {
	const byte *inbuf_ptr;
	byte *outbuf_ptr;
	byte *outbuf_start;
	uint32 inbuf_remain;

	const byte *inbuf_end;
	byte *outbuf_end;
	uint32 outbuf_remain;

	byte mark_byte;
	int test_byte;

	uint32 runcount;

	byte bitfield;
	byte bitfield_byte1;
	byte bitfield_byte2;

	byte *backtrack_ptr;
	int backtrack_amount;

	uint16 c, b;

	int decode_err = 0;

	inbuf_ptr = inbuf;
	inbuf_remain = inbuf_len;

	outbuf_start = outbuf_ptr = outbuf.getBuffer();
	outbuf_remain = outbuf.size();
	outbuf_end = (outbuf_start + outbuf_remain) - 1;
	memset(outbuf_start, 0, outbuf_remain);

	inbuf_end = (inbuf + inbuf_len) - 1;


	while ((inbuf_remain > 1) && (outbuf_remain > 0) && !decode_err) {

		if ((inbuf_ptr > inbuf_end) || (outbuf_ptr > outbuf_end)) {
			return false;
		}

		mark_byte = *inbuf_ptr++;
		inbuf_remain--;

		test_byte = mark_byte & 0xC0; // Mask all but two high order bits

		switch (test_byte) {
		case 0xC0: // 1100 0000
			// Uncompressed run follows: Max runlength 63
			runcount = mark_byte & 0x3f;
			if ((inbuf_remain < runcount) || (outbuf_remain < runcount)) {
				return false;
			}

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr++;
			}

			inbuf_remain -= runcount;
			outbuf_remain -= runcount;
			continue;
			break;
		case 0x80: // 1000 0000
			// Compressed run follows: Max runlength 63
			runcount = (mark_byte & 0x3f) + 3;
			if (!inbuf_remain || (outbuf_remain < runcount)) {
				return false;
			}

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr;
			}

			inbuf_ptr++;
			inbuf_remain--;
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x40: // 0100 0000
			// Repeat decoded sequence from output stream:
			// Max runlength 10

			runcount = ((mark_byte >> 3) & 0x07U) + 3;
			backtrack_amount = *inbuf_ptr;

			if (!inbuf_remain || (backtrack_amount > (outbuf_ptr - outbuf_start)) || (runcount > outbuf_remain)) {
				return false;
			}

			inbuf_ptr++;
			inbuf_remain--;

			backtrack_ptr = outbuf_ptr - backtrack_amount;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *backtrack_ptr++;
			}

			outbuf_remain -= runcount;
			continue;
			break;
		default: // 0000 0000
			break;
		}

		// Mask all but the third and fourth highest order bits
		test_byte = mark_byte & 0x30;

		switch (test_byte) {

		case 0x30: // 0011 0000
			// Bitfield compression
			runcount = (mark_byte & 0x0F) + 1;

			if ((inbuf_remain < (runcount + 2)) || (outbuf_remain < (runcount * 8))) {
				return false;
			}

			bitfield_byte1 = *inbuf_ptr++;
			bitfield_byte2 = *inbuf_ptr++;

			for (c = 0; c < runcount; c++) {
				bitfield = *inbuf_ptr;
				for (b = 0; b < 8; b++) {
					if (bitfield & 0x80) {
						*outbuf_ptr = bitfield_byte2;
					} else {
						*outbuf_ptr = bitfield_byte1;
					}
					bitfield <<= 1;
					outbuf_ptr++;
				}
				inbuf_ptr++;
			}

			inbuf_remain -= (runcount + 2);
			outbuf_remain -= (runcount * 8);
			continue;
			break;
		case 0x20: // 0010 0000
			// Uncompressed run follows
			runcount = ((mark_byte & 0x0F) << 8) + *inbuf_ptr;
			if ((inbuf_remain < (runcount + 1)) || (outbuf_remain < runcount)) {
				return false;
			}

			inbuf_ptr++;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr++;
			}

			inbuf_remain -= (runcount + 1);
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x10: // 0001 0000
			// Repeat decoded sequence from output stream
			backtrack_amount = ((mark_byte & 0x0F) << 8) + *inbuf_ptr;
			if (inbuf_remain < 2) {
				return false;
			}

			inbuf_ptr++;
			runcount = *inbuf_ptr++;

			if ((backtrack_amount > (outbuf_ptr - outbuf_start)) || (outbuf_remain < runcount)) {
				return false;
			}

			backtrack_ptr = outbuf_ptr - backtrack_amount;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *backtrack_ptr++;
			}

			inbuf_remain -= 2;
			outbuf_remain -= runcount;
			continue;
			break;
		default:
			return false;
		}
	}

	return true;
}

void SagaEngine::flipImage(byte *imageBuffer, int columns, int scanlines) {
	int line;
	ByteArray tmp_scan;

	byte *flip_p1;
	byte *flip_p2;
	byte *flip_tmp;

	int flipcount = scanlines / 2;

	tmp_scan.resize(columns);
	flip_tmp = tmp_scan.getBuffer();
	if (flip_tmp == NULL) {
		return;
	}

	flip_p1 = imageBuffer;
	flip_p2 = imageBuffer + (columns * (scanlines - 1));

	for (line = 0; line < flipcount; line++) {
		memcpy(flip_tmp, flip_p1, columns);
		memcpy(flip_p1, flip_p2, columns);
		memcpy(flip_p2, flip_tmp, columns);
		flip_p1 += columns;
		flip_p2 -= columns;
	}
}

void SagaEngine::unbankBGImage(byte *dst_buf, const byte *src_buf, int columns, int scanlines) {
	int x, y;
	int temp;
	int quadruple_rows;
	int remain_rows;
	int rowjump_src;
	int rowjump_dest;
	const byte *src_p;
	const byte *srcptr1, *srcptr2, *srcptr3, *srcptr4;
	byte *dstptr1, *dstptr2, *dstptr3, *dstptr4;

	quadruple_rows = scanlines - (scanlines % 4);
	remain_rows = scanlines - quadruple_rows;

	assert(scanlines > 0);

	src_p = src_buf;

	srcptr1 = src_p;
	srcptr2 = src_p + 1;
	srcptr3 = src_p + 2;
	srcptr4 = src_p + 3;

	dstptr1 = dst_buf;
	dstptr2 = dst_buf + columns;
	dstptr3 = dst_buf + columns * 2;
	dstptr4 = dst_buf + columns * 3;

	rowjump_src = columns * 4;
	rowjump_dest = columns * 4;

	// Unbank groups of 4 first
	for (y = 0; y < quadruple_rows; y += 4) {
		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
			dstptr3[x] = srcptr3[temp];
			dstptr4[x] = srcptr4[temp];
		}

		// This is to avoid generating invalid pointers -
		// usually innocuous, but undefined
		if (y < quadruple_rows - 4) {
			dstptr1 += rowjump_dest;
			dstptr2 += rowjump_dest;
			dstptr3 += rowjump_dest;
			dstptr4 += rowjump_dest;
			srcptr1 += rowjump_src;
			srcptr2 += rowjump_src;
			srcptr3 += rowjump_src;
			srcptr4 += rowjump_src;
		}
	}

	// Unbank rows remaining
	switch (remain_rows) {
	case 1:
		dstptr1 += rowjump_dest;
		srcptr1 += rowjump_src;
		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
		}
		break;
	case 2:
		dstptr1 += rowjump_dest;
		dstptr2 += rowjump_dest;
		srcptr1 += rowjump_src;
		srcptr2 += rowjump_src;
		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
		}
		break;
	case 3:
		dstptr1 += rowjump_dest;
		dstptr2 += rowjump_dest;
		dstptr3 += rowjump_dest;
		srcptr1 += rowjump_src;
		srcptr2 += rowjump_src;
		srcptr3 += rowjump_src;
		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
			dstptr3[x] = srcptr3[temp];
		}
		break;
	default:
		break;
	}
}

} // End of namespace Saga
