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

// These tables are modified versions of the FFmpeg ones so that they
// will work with our BitStream class directly.

#ifndef VIDEO_CODECS_SVQ1_VLC_H
#define VIDEO_CODECS_SVQ1_VLC_H

#include "common/scummsys.h"

namespace Video {

static const byte s_svq1BlockTypeLengths[4] = {
	1, 2, 3, 3
};

static const uint32 s_svq1BlockTypeCodes[4] = {
	1, 1, 1, 0
};

static const byte s_svq1IntraMultistageLengths0[8] = {
	5, 1, 3, 3, 4, 4, 5, 4
};

static const uint32 s_svq1IntraMultistageCodes0[8] = {
	1, 1, 3, 2, 3, 2, 0, 1
};

static const byte s_svq1IntraMultistageLengths1[8] = {
	4, 2, 3, 3, 3, 3, 4, 3
};

static const uint32 s_svq1IntraMultistageCodes1[8] = {
	1, 3, 5, 4, 3, 2, 0, 1
};

static const byte s_svq1IntraMultistageLengths2[8] = {
	5, 1, 3, 5, 4, 3, 4, 4
};

static const uint32 s_svq1IntraMultistageCodes2[8] = {
	1, 1, 3, 0, 3, 2, 2, 1
};

static const byte s_svq1IntraMultistageLengths3[8] = {
	6, 1, 2, 6, 4, 4, 5, 4
};

static const uint32 s_svq1IntraMultistageCodes3[8] = {
	1, 1, 1, 0, 3, 2, 1, 1
};

static const byte s_svq1IntraMultistageLengths4[8] = {
	6, 1, 2, 5, 5, 6, 5, 3
};

static const uint32 s_svq1IntraMultistageCodes4[8] = {
	1, 1, 1, 3, 2, 0, 1, 1
};

static const byte s_svq1IntraMultistageLengths5[8] = {
	7, 1, 2, 3, 4, 6, 7, 5
};

static const uint32 s_svq1IntraMultistageCodes5[8] = {
	1, 1, 1, 1, 1, 1, 0, 1
};

static const byte *s_svq1IntraMultistageLengths[6] = {
	s_svq1IntraMultistageLengths0, s_svq1IntraMultistageLengths1, s_svq1IntraMultistageLengths2,
	s_svq1IntraMultistageLengths3, s_svq1IntraMultistageLengths4, s_svq1IntraMultistageLengths5
};

static const uint32 *s_svq1IntraMultistageCodes[6] = {
	s_svq1IntraMultistageCodes0, s_svq1IntraMultistageCodes1, s_svq1IntraMultistageCodes2,
	s_svq1IntraMultistageCodes3, s_svq1IntraMultistageCodes4, s_svq1IntraMultistageCodes5
};

static const byte s_svq1InterMultistageLengths0[8] = {
	2, 3, 3, 3, 3, 3, 4, 4
};

static const uint32 s_svq1InterMultistageCodes0[8] = {
	3, 5, 4, 3, 2, 1, 1, 0
};

static const byte s_svq1InterMultistageLengths1[8] = {
	2, 3, 3, 3, 3, 3, 4, 4
};

static const uint32 s_svq1InterMultistageCodes1[8] = {
	3, 5, 4, 3, 2, 1, 1, 0
};

static const byte s_svq1InterMultistageLengths2[8] = {
	1, 3, 3, 4, 4, 4, 5, 5
};

static const uint32 s_svq1InterMultistageCodes2[8] = {
	1, 3, 2, 3, 2, 1, 1, 0
};

static const byte s_svq1InterMultistageLengths3[8] = {
	1, 3, 3, 4, 4, 4, 5, 5
};

static const uint32 s_svq1InterMultistageCodes3[8] = {
	1, 3, 2, 3, 2, 1, 1, 0
};

static const byte s_svq1InterMultistageLengths4[8] = {
	1, 3, 3, 4, 4, 4, 5, 5
};

static const uint32 s_svq1InterMultistageCodes4[8] = {
	1, 3, 2, 3, 2, 1, 1, 0
};

static const byte s_svq1InterMultistageLengths5[8] = {
	1, 2, 3, 5, 5, 5, 6, 6
};

static const uint32 s_svq1InterMultistageCodes5[8] = {
	1, 1, 1, 3, 2, 1, 1, 0
};

static const byte *s_svq1InterMultistageLengths[6] = {
	s_svq1InterMultistageLengths0, s_svq1InterMultistageLengths1, s_svq1InterMultistageLengths2,
	s_svq1InterMultistageLengths3, s_svq1InterMultistageLengths4, s_svq1InterMultistageLengths5
};

static const uint32 *s_svq1InterMultistageCodes[6] = {
	s_svq1InterMultistageCodes0, s_svq1InterMultistageCodes1, s_svq1InterMultistageCodes2,
	s_svq1InterMultistageCodes3, s_svq1InterMultistageCodes4, s_svq1InterMultistageCodes5
};

static const byte s_svq1IntraMeanLengths[256] = {
	6, 7, 17, 20, 20, 20, 20, 20, 20, 19,
	11, 9, 11, 14, 14, 15, 16, 12, 10, 11,
	11, 9, 8, 8, 7, 4, 4, 6, 7, 8,
	8, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 7, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 7, 8, 8, 7, 8,
	8, 8, 8, 8, 7, 8, 7, 7, 8, 7,
	7, 8, 7, 8, 8, 8, 7, 7, 8, 7,
	8, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 6, 6,
	7, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 9,
	9, 9, 9, 9, 8, 8, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 11, 11, 11, 10, 11, 11, 11,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 11, 14
};

static const uint32 s_svq1IntraMeanCodes[256] = {
	55, 86, 1, 1, 2, 3, 0, 4, 5, 3,
	21, 66, 20, 3, 2, 1, 1, 1, 43, 24,
	12, 65, 120, 108, 85, 15, 14, 52, 81, 114,
	110, 64, 63, 62, 61, 60, 59, 58, 57, 56,
	55, 67, 70, 71, 69, 68, 73, 72, 74, 121,
	118, 119, 113, 117, 116, 115, 106, 85, 112, 111,
	82, 109, 76, 107, 64, 105, 104, 103, 102, 101,
	100, 99, 98, 97, 96, 95, 94, 93, 92, 91,
	90, 89, 88, 87, 86, 61, 84, 83, 63, 81,
	80, 79, 78, 77, 65, 75, 83, 62, 72, 79,
	82, 69, 80, 67, 66, 65, 66, 67, 62, 68,
	60, 69, 70, 71, 72, 73, 74, 75, 76, 77,
	78, 88, 89, 90, 91, 92, 93, 68, 73, 41,
	63, 61, 59, 44, 40, 37, 38, 94, 87, 84,
	95, 98, 99, 100, 97, 101, 103, 102, 53, 54,
	96, 57, 58, 56, 55, 54, 53, 52, 51, 50,
	49, 48, 45, 43, 42, 39, 64, 70, 71, 38,
	37, 36, 35, 34, 46, 47, 31, 54, 29, 33,
	27, 28, 25, 26, 24, 23, 22, 30, 32, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 53,
	49, 50, 51, 52, 25, 42, 23, 22, 21, 40,
	38, 37, 34, 33, 24, 20, 41, 18, 13, 14,
	15, 16, 17, 26, 27, 28, 29, 30, 31, 32,
	19, 35, 36, 9, 8, 7, 39, 5, 11, 6,
	4, 3, 2, 1, 10, 22, 25, 23, 13, 14,
	15, 16, 17, 18, 19, 1
};

static const byte s_svq1InterMeanLengths[512] = {
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 21, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 21,
	22, 22, 22, 22, 22, 22, 20, 21, 22, 21,
	22, 22, 20, 22, 22, 21, 19, 18, 20, 22,
	22, 21, 20, 19, 20, 20, 19, 19, 19, 18,
	19, 18, 19, 20, 19, 19, 18, 18, 18, 19,
	18, 18, 18, 17, 19, 18, 18, 17, 18, 18,
	18, 17, 17, 17, 17, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 15, 16, 15, 15, 15, 15,
	15, 15, 15, 15, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 13, 13, 13, 13, 13, 13, 13,
	13, 12, 12, 12, 12, 12, 12, 11, 11, 11,
	11, 11, 11, 10, 10, 10, 10, 10, 10, 9,
	9, 9, 9, 9, 8, 8, 8, 8, 7, 7,
	7, 6, 6, 5, 5, 4, 1, 3, 5, 5,
	6, 6, 7, 7, 7, 7, 8, 8, 8, 9,
	9, 9, 9, 9, 10, 10, 10, 10, 11, 11,
	11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
	12, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	14, 14, 14, 14, 14, 14, 14, 14, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 17, 17,
	17, 17, 17, 17, 17, 17, 18, 17, 17, 17,
	17, 17, 17, 17, 19, 18, 18, 19, 18, 18,
	19, 18, 18, 18, 19, 18, 19, 19, 18, 20,
	20, 19, 19, 19, 19, 19, 19, 18, 19, 20,
	19, 19, 21, 20, 19, 20, 19, 19, 20, 20,
	22, 20, 22, 22, 21, 22, 22, 21, 21, 22,
	22, 20, 22, 22, 21, 22, 22, 22, 20, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 21, 21, 22, 22, 22, 21,
	22, 21, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22
};

static const uint32 s_svq1InterMeanCodes[512] = {
	90, 212, 213, 214, 215, 216, 217, 218, 219, 220,
	221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
	232, 203, 233, 234, 231, 236, 237, 238, 239, 240,
	241, 242, 243, 244, 245, 246, 247, 248, 258, 235,
	249, 252, 253, 254, 256, 92, 96, 257, 113, 260,
	261, 251, 255, 134, 250, 124, 117, 259, 120, 211,
	123, 130, 210, 209, 208, 207, 206, 205, 204, 195,
	202, 201, 200, 199, 198, 197, 139, 196, 194, 193,
	192, 191, 190, 189, 188, 187, 186, 185, 97, 132,
	133, 134, 135, 136, 137, 138, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
	154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
	164, 165, 166, 167, 168, 169, 170, 171, 127, 143,
	172, 173, 174, 175, 176, 177, 83, 144, 178, 145,
	179, 180, 84, 181, 182, 140, 52, 61, 85, 183,
	184, 139, 86, 61, 87, 88, 64, 67, 71, 42,
	46, 44, 70, 89, 73, 45, 56, 54, 57, 69,
	40, 48, 53, 32, 68, 50, 49, 31, 47, 46,
	45, 33, 34, 35, 36, 39, 35, 32, 29, 37,
	30, 36, 42, 38, 33, 41, 34, 35, 36, 27,
	26, 29, 31, 39, 23, 24, 25, 27, 28, 30,
	37, 32, 33, 19, 20, 21, 22, 23, 24, 25,
	26, 24, 23, 21, 20, 19, 18, 15, 16, 18,
	19, 27, 26, 14, 19, 15, 16, 17, 18, 13,
	20, 21, 12, 19, 15, 14, 16, 17, 12, 9,
	10, 8, 9, 9, 8, 5, 1, 3, 7, 6,
	11, 10, 14, 15, 11, 13, 11, 13, 12, 15,
	16, 17, 14, 18, 23, 20, 22, 21, 25, 24,
	23, 22, 21, 20, 17, 25, 26, 22, 29, 27,
	28, 32, 28, 35, 34, 33, 31, 30, 27, 29,
	36, 22, 26, 34, 29, 31, 21, 35, 24, 32,
	41, 40, 38, 37, 25, 28, 30, 23, 44, 43,
	28, 33, 45, 40, 31, 27, 26, 34, 45, 50,
	44, 39, 49, 51, 47, 43, 55, 42, 46, 48,
	41, 40, 38, 37, 47, 51, 52, 48, 58, 59,
	49, 60, 43, 41, 72, 39, 66, 65, 38, 82,
	81, 63, 62, 57, 60, 59, 58, 37, 56, 80,
	55, 54, 135, 79, 53, 78, 51, 50, 77, 76,
	131, 75, 129, 128, 142, 126, 125, 132, 141, 122,
	121, 74, 119, 118, 137, 116, 115, 114, 73, 112,
	111, 110, 109, 108, 107, 106, 105, 104, 103, 102,
	101, 100, 99, 98, 138, 136, 95, 94, 93, 133,
	91, 131, 89, 88, 87, 86, 85, 84, 83, 82,
	81, 80, 79, 78, 77, 76, 75, 74, 73, 72,
	71, 70, 69, 68, 67, 66, 65, 64, 63, 62,
	61, 60, 59, 58, 57, 56, 55, 54, 53, 52,
	51, 50, 49, 48, 47, 46, 45, 44, 43, 42,
	41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
	31, 30, 29, 28, 27, 26, 25, 24, 23, 22,
	21, 20, 19, 18, 17, 16, 15, 14, 13, 12,
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	1, 0
};

static const byte s_svq1MotionComponentLengths[33] = {
	1, 2, 3, 4, 6, 7, 7, 7, 9, 9,
	9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 11, 11, 11, 11, 11,
	11, 12, 12
};

static const uint32 s_svq1MotionComponentCodes[33] = {
	1, 1, 1, 1, 3, 5, 4, 3, 11, 10,
	9, 17, 16, 15, 14, 13, 12, 11, 10, 9,
	8, 7, 6, 5, 4, 7, 6, 5, 4, 3,
	2, 3, 2
};

} // End of namespace Video

#endif
