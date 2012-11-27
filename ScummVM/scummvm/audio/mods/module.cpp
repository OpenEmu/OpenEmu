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

#include "audio/mods/module.h"

#include "common/util.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Modules {

const int16 Module::periods[16][60] = {
	{1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960 , 906,
	 856 , 808 , 762 , 720 , 678 , 640 , 604 , 570 , 538 , 508 , 480 , 453,
	 428 , 404 , 381 , 360 , 339 , 320 , 302 , 285 , 269 , 254 , 240 , 226,
	 214 , 202 , 190 , 180 , 170 , 160 , 151 , 143 , 135 , 127 , 120 , 113,
	 107 , 101 , 95  , 90  , 85  , 80  , 75  , 71  , 67  , 63  , 60  , 56 },
	{1700, 1604, 1514, 1430, 1348, 1274, 1202, 1134, 1070, 1010, 954 , 900,
	 850 , 802 , 757 , 715 , 674 , 637 , 601 , 567 , 535 , 505 , 477 , 450,
	 425 , 401 , 379 , 357 , 337 , 318 , 300 , 284 , 268 , 253 , 239 , 225,
	 213 , 201 , 189 , 179 , 169 , 159 , 150 , 142 , 134 , 126 , 119 , 113,
	 106 , 100 , 94  , 89  , 84  , 79  , 75  , 71  , 67  , 63  , 59  , 56 },
	{1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948 , 894,
	 844 , 796 , 752 , 709 , 670 , 632 , 597 , 563 , 532 , 502 , 474 , 447,
	 422 , 398 , 376 , 355 , 335 , 316 , 298 , 282 , 266 , 251 , 237 , 224,
	 211 , 199 , 188 , 177 , 167 , 158 , 149 , 141 , 133 , 125 , 118 , 112,
	 105 , 99  , 94  , 88  , 83  , 79  , 74  , 70  , 66  , 62  , 59  , 56 },
	{1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996 , 940 , 888,
	 838 , 791 , 746 , 704 , 665 , 628 , 592 , 559 , 528 , 498 , 470 , 444,
	 419 , 395 , 373 , 352 , 332 , 314 , 296 , 280 , 264 , 249 , 235 , 222,
	 209 , 198 , 187 , 176 , 166 , 157 , 148 , 140 , 132 , 125 , 118 , 111,
	 104 , 99  , 93  , 88  , 83  , 78  , 74  , 70  , 66  , 62  , 59  , 55 },
	{1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 990 , 934 , 882,
	 832 , 785 , 741 , 699 , 660 , 623 , 588 , 555 , 524 , 495 , 467 , 441,
	 416 , 392 , 370 , 350 , 330 , 312 , 294 , 278 , 262 , 247 , 233 , 220,
	 208 , 196 , 185 , 175 , 165 , 156 , 147 , 139 , 131 , 124 , 117 , 110,
	 104 , 98  , 92  , 87  , 82  , 78  , 73  , 69  , 65  , 62  , 58  , 55 },
	{1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982 , 926 , 874,
	 826 , 779 , 736 , 694 , 655 , 619 , 584 , 551 , 520 , 491 , 463 , 437,
	 413 , 390 , 368 , 347 , 328 , 309 , 292 , 276 , 260 , 245 , 232 , 219,
	 206 , 195 , 184 , 174 , 164 , 155 , 146 , 138 , 130 , 123 , 116 , 109,
	 103 , 97  , 92  , 87  , 82  , 77  , 73  , 69  , 65  , 61  , 58  , 54 },
	{1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974 , 920 , 868,
	 820 , 774 , 730 , 689 , 651 , 614 , 580 , 547 , 516 , 487 , 460 , 434,
	 410 , 387 , 365 , 345 , 325 , 307 , 290 , 274 , 258 , 244 , 230 , 217,
	 205 , 193 , 183 , 172 , 163 , 154 , 145 , 137 , 129 , 122 , 115 , 109,
	 102 , 96  , 91  , 86  , 81  , 77  , 72  , 68  , 64  , 61  , 57  , 54 },
	{1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968 , 914 , 862,
	 814 , 768 , 725 , 684 , 646 , 610 , 575 , 543 , 513 , 484 , 457 , 431,
	 407 , 384 , 363 , 342 , 323 , 305 , 288 , 272 , 256 , 242 , 228 , 216,
	 204 , 192 , 181 , 171 , 161 , 152 , 144 , 136 , 128 , 121 , 114 , 108,
	 102 , 96  , 90  , 85  , 80  , 76  , 72  , 68  , 64  , 60  , 57  , 54 },
	{1814, 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960,
	 907 , 856 , 808 , 762 , 720 , 678 , 640 , 604 , 570 , 538 , 508 , 480,
	 453 , 428 , 404 , 381 , 360 , 339 , 320 , 302 , 285 , 269 , 254 , 240,
	 226 , 214 , 202 , 190 , 180 , 170 , 160 , 151 , 143 , 135 , 127 , 120,
	 113 , 107 , 101 , 95  , 90  , 85  , 80  , 75  , 71  , 67  , 63  , 60 },
	{1800, 1700, 1604, 1514, 1430, 1350, 1272, 1202, 1134, 1070, 1010, 954,
	 900 , 850 , 802 , 757 , 715 , 675 , 636 , 601 , 567 , 535 , 505 , 477,
	 450 , 425 , 401 , 379 , 357 , 337 , 318 , 300 , 284 , 268 , 253 , 238,
	 225 , 212 , 200 , 189 , 179 , 169 , 159 , 150 , 142 , 134 , 126 , 119,
	 112 , 106 , 100 , 94  , 89  , 84  , 79  , 75  , 71  , 67  , 63  , 59 },
	{1788, 1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948,
	 894 , 844 , 796 , 752 , 709 , 670 , 632 , 597 , 563 , 532 , 502 , 474,
	 447 , 422 , 398 , 376 , 355 , 335 , 316 , 298 , 282 , 266 , 251 , 237,
	 223 , 211 , 199 , 188 , 177 , 167 , 158 , 149 , 141 , 133 , 125 , 118,
	 111 , 105 , 99  , 94  , 88  , 83  , 79  , 74  , 70  , 66  , 62  , 59 },
	{1774, 1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996 , 940,
	 887 , 838 , 791 , 746 , 704 , 665 , 628 , 592 , 559 , 528 , 498 , 470,
	 444 , 419 , 395 , 373 , 352 , 332 , 314 , 296 , 280 , 264 , 249 , 235,
	 222 , 209 , 198 , 187 , 176 , 166 , 157 , 148 , 140 , 132 , 125 , 118,
	 111 , 104 , 99  , 93  , 88  , 83  , 78  , 74  , 70  , 66  , 62  , 59 },
	{1762, 1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 988 , 934,
	 881 , 832 , 785 , 741 , 699 , 660 , 623 , 588 , 555 , 524 , 494 , 467,
	 441 , 416 , 392 , 370 , 350 , 330 , 312 , 294 , 278 , 262 , 247 , 233,
	 220 , 208 , 196 , 185 , 175 , 165 , 156 , 147 , 139 , 131 , 123 , 117,
	 110 , 104 , 98  , 92  , 87  , 82  , 78  , 73  , 69  , 65  , 61  , 58 },
	{1750, 1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982 , 926,
	 875 , 826 , 779 , 736 , 694 , 655 , 619 , 584 , 551 , 520 , 491 , 463,
	 437 , 413 , 390 , 368 , 347 , 328 , 309 , 292 , 276 , 260 , 245 , 232,
	 219 , 206 , 195 , 184 , 174 , 164 , 155 , 146 , 138 , 130 , 123 , 116,
	 109 , 103 , 97  , 92  , 87  , 82  , 77  , 73  , 69  , 65  , 61  , 58 },
	{1736, 1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974 , 920,
	 868 , 820 , 774 , 730 , 689 , 651 , 614 , 580 , 547 , 516 , 487 , 460,
	 434 , 410 , 387 , 365 , 345 , 325 , 307 , 290 , 274 , 258 , 244 , 230,
	 217 , 205 , 193 , 183 , 172 , 163 , 154 , 145 , 137 , 129 , 122 , 115,
	 108 , 102 , 96  , 91  , 86  , 81  , 77  , 72  , 68  , 64  , 61  , 57 },
	{1724, 1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968 , 914,
	 862 , 814 , 768 , 725 , 684 , 646 , 610 , 575 , 543 , 513 , 484 , 457,
	 431 , 407 , 384 , 363 , 342 , 323 , 305 , 288 , 272 , 256 , 242 , 228,
	 216 , 203 , 192 , 181 , 171 , 161 , 152 , 144 , 136 , 128 , 121 , 114,
	 108 , 101 , 96  , 90  , 85  , 80  , 76  , 72  , 68  , 64  , 60  , 57 }};

const uint32 Module::signatures[] = {
	MKTAG('M','.','K','.'), MKTAG('M','!','K','!'), MKTAG('F','L','T','4')
};

bool Module::load(Common::SeekableReadStream &st, int offs) {
	if (offs) {
		// Load the module with the common sample data
		load(st, 0);
	}

	st.seek(offs);
	st.read(songname, 20);
	songname[20] = '\0';

	for (int i = 0; i < NUM_SAMPLES; ++i) {
		st.read(sample[i].name, 22);
		sample[i].name[22] = '\0';
		sample[i].len = 2 * st.readUint16BE();

		sample[i].finetune = st.readByte();
		assert(sample[i].finetune < 0x10);

		sample[i].vol = st.readByte();
		sample[i].repeat = 2 * st.readUint16BE();
		sample[i].replen = 2 * st.readUint16BE();
	}

	songlen = st.readByte();
	undef = st.readByte();

	st.read(songpos, 128);

	sig = st.readUint32BE();

	bool foundSig = false;
	for (int i = 0; i < ARRAYSIZE(signatures); i++) {
		if (sig == signatures[i]) {
			foundSig = true;
			break;
		}
	}

	if (!foundSig) {
		warning("No known signature found in protracker module");
		return false;
	}

	int maxpattern = 0;
	for (int i = 0; i < 128; ++i)
		if (maxpattern < songpos[i])
			maxpattern = songpos[i];

	pattern = new pattern_t[maxpattern + 1];

	for (int i = 0; i <= maxpattern; ++i) {
		for (int j = 0; j < 64; ++j) {
			for (int k = 0; k < 4; ++k) {
				uint32 note = st.readUint32BE();
				pattern[i][j][k].sample = (note & 0xf0000000) >> 24 | (note & 0x0000f000) >> 12;
				pattern[i][j][k].period = (note >> 16) & 0xfff;
				pattern[i][j][k].effect = note & 0xfff;
				pattern[i][j][k].note = periodToNote((note >> 16) & 0xfff);
			}
		}
	}

	for (int i = 0; i < NUM_SAMPLES; ++i) {
		if (offs) {
			// Restore information for modules that use common sample data
			for (int j = 0; j < NUM_SAMPLES; ++j) {
				if (!scumm_stricmp((const char *)commonSamples[j].name, (const char *)sample[i].name)) {
					sample[i].len = commonSamples[j].len;
					st.seek(commonSamples[j].offs);
					break;
				}
			}
		} else {
			// Store information for modules that use common sample data
			memcpy(commonSamples[i].name, sample[i].name, 22);
			commonSamples[i].len = sample[i].len;
			commonSamples[i].offs = st.pos();

		}

		if (!sample[i].len) {
			sample[i].data = 0;
		} else {
			sample[i].data = new int8[sample[i].len];
			st.read((byte *)sample[i].data, sample[i].len);
		}
	}

	return true;
}

Module::Module() {
	pattern = 0;
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		sample[i].data = 0;
	}
}

Module::~Module() {
	delete[] pattern;
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		delete[] sample[i].data;
	}
}

byte Module::periodToNote(int16 period, byte finetune) {
	int16 diff1;
	int16 diff2;

	diff1 = ABS(periods[finetune][0] - period);
	if (diff1 == 0)
		return 0;

	for (int i = 1; i < 60; i++) {
		diff2 = ABS(periods[finetune][i] - period);
		if (diff2 == 0)
			return i;
		else if (diff2 > diff1)
			return i-1;
		diff1 = diff2;
	}
	return 59;
}

int16 Module::noteToPeriod(byte note, byte finetune) {
	if (finetune > 15)
		finetune = 15;
	if (note > 59)
		note = 59;

	return periods[finetune][note];
}

} // End of namespace Modules
