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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "extract.h"
#include "common/language.h"
#include "common/platform.h"

#include <algorithm>

namespace {

// Extraction function prototypes

bool extractRaw(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractStrings(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractStrings10(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractRooms(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractShapes(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractKyraForestSeqData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractAmigaSfx(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractWdSfx(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);

bool extractHofSeqData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractHofShapeAnimDataV1(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractHofShapeAnimDataV2(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);

bool extractStringsWoSuffix(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractPaddedStrings(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractRaw16to8(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractMrShapeAnimData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractRaw16(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractRaw32(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractLoLButtonDefs(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);

bool extractEoB2SeqData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractEoB2ShapeData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
bool extractEoBNpcData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id);
// Extraction type table

const ExtractType extractTypeTable[] = {
	{ kTypeStringList, extractStrings },
	{ kTypeRoomList, extractRooms },
	{ kTypeShapeList, extractShapes },
	{ kTypeRawData, extractRaw },
	{ kTypeForestSeqData, extractKyraForestSeqData },
	{ kTypeAmigaSfxTable, extractAmigaSfx },
	{ kTypeTownsWDSfxTable, extractWdSfx },

	{ k2TypeSeqData, extractHofSeqData },
	{ k2TypeShpDataV1, extractHofShapeAnimDataV1 },
	{ k2TypeShpDataV2, extractHofShapeAnimDataV2 },
	{ k2TypeSoundList, extractStringsWoSuffix },
	{ k2TypeLangSoundList, extractStringsWoSuffix },
	{ k2TypeSize10StringList, extractStrings10 },
	{ k2TypeSfxList, extractPaddedStrings },
	{ k3TypeRaw16to8, extractRaw16to8 },
	{ k3TypeShpData, extractMrShapeAnimData },

	{ kLoLTypeCharData, extractRaw },
	{ kLoLTypeSpellData, extractRaw },
	{ kLoLTypeCompassData, extractRaw16to8 },
	{ kLoLTypeFlightShpData, extractRaw16to8 },
	{ kLoLTypeRaw16, extractRaw16 },
	{ kLoLTypeRaw32, extractRaw32 },
	{ kLoLTypeButtonDef, extractLoLButtonDefs },

	{ kEoB2TypeSeqData, extractEoB2SeqData },
	{ kEoB2TypeShapeData, extractEoB2ShapeData },
	{ kEoBTypeNpcData, extractEoBNpcData },

	{ -1, 0 }
};

// TODO: Clean up the mess of data types we have... it seems some special types
// we have (even in the main KYRA code, is just raw data access, but used specially
// to have a nice wrapper from inside StaticResource...).
const TypeTable typeTable[] = {
	{ kTypeStringList, 0 },
	{ kTypeRawData, 1 },
	{ kTypeRoomList, 2 },
	{ kTypeShapeList, 3 },
	{ kTypeForestSeqData, 1 },
	{ kTypeAmigaSfxTable, 4 },
	{ kTypeTownsWDSfxTable, 1 },
	{ k2TypeSeqData, 5 },
	{ k2TypeShpDataV1, 6 },
	{ k2TypeShpDataV2, 7 },
	{ k2TypeSoundList, 0 },
	{ k2TypeLangSoundList, 0 },
	{ k2TypeSize10StringList, 0 },
	{ k2TypeSfxList, 0 },
	{ k3TypeRaw16to8, 1 },
	{ k3TypeShpData, 7 },
	{ kLoLTypeRaw16, 13 },
	{ kLoLTypeRaw32, 14 },
	{ kLoLTypeButtonDef, 12 },
	{ kLoLTypeCharData, 8 },
	{ kLoLTypeSpellData, 9 },
	{ kLoLTypeCompassData, 10 },
	{ kLoLTypeFlightShpData, 11 },
	{ kEoB2TypeSeqData, 15 },
	{ kEoB2TypeShapeData, 16 },
	{ kEoBTypeNpcData, 17},
	{ -1, 1 }
};

} // end of anonymous namespace

// misc

const ExtractType *findExtractType(const int type) {
	for (const ExtractType *i = extractTypeTable; i->type != -1; ++i) {
		if (i->type == type)
			return i;
	}
	return 0;
}

byte getTypeID(int type) {
	return std::find(typeTable, ARRAYEND(typeTable) - 1, type)->value;
}
// Extractor implementation

namespace {

bool extractRaw(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	uint8 *buffer = new uint8[size];
	assert(buffer);
	memcpy(buffer, data, size);

	return out.addFile(filename, buffer, size);
}

bool extractStrings(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	// Skip tables for skipping English string left-overs in the hacky Russian fan translations
	static const uint8 rusFanSkip_k2SeqplayStrings[] = { 1, 3, 5, 8, 10, 11, 13, 15, 17, 20, 22, 25, 26, 30, 33, 38, 40, 41, 44, 49, 51, 55, 104, 119, 121, 123 };
	static const uint8 rusFanSkip_k1IntroStrings[] = { 3, 5, 9, 11, 13, 16, 18, 21, 24, 32, 34, 36, 38, 41, 44, 49, 52, 55, 57, 59, 61, 64, 66, 69, 72, 75 };
	static const uint8 rusFanSkip_k1ThePoisonStrings[] = { 1, 4 };
	static const uint8 rusFanSkip_k1FullFlaskStrings[] = { 1, 2, 4, 5, 7 };
	static const uint8 rusFanSkip_k1WispJewelStrings[] = { 2 };
	static const uint8 rusFanSkip_k1GUIStrings[] = { 1, 3, 6, 8, 11, 13, 18 };
	uint32 rusFanSkipIdLen = 0;
	const uint8 *rusFanSkipId = 0;
	uint rusFanEmptyId = 10000;
	uint32 skipCount = 0;

	int patch = 0;
	// FM Towns files that need addional patches
	if (info->platform == kPlatformFMTowns) {
		if (id == k1TakenStrings || id == k1NoDropStrings || id == k1PoisonGoneString ||
			id == k1ThePoisonStrings || id == k1FluteStrings || id == k1WispJewelStrings)
			patch = 1;
		else if (id == k1IntroStrings)
			patch = 2;
		else if (id == k2SeqplayStrings)
			patch = 3;
	} else if (info->platform == kPlatformPC) {
		if (id == k2IngamePakFiles)
			patch = 4;

		if (info->lang == Common::RU_RUS && info->special == kNoSpecial) {
			patch = 5;
			if (id == k2SeqplayStrings) {
				rusFanSkipId = rusFanSkip_k2SeqplayStrings;
				rusFanSkipIdLen = ARRAYSIZE(rusFanSkip_k2SeqplayStrings);
				rusFanEmptyId = 81;
			} else if (id == k1IntroStrings) {
				rusFanSkipId = rusFanSkip_k1IntroStrings;
				rusFanSkipIdLen = ARRAYSIZE(rusFanSkip_k1IntroStrings);
				rusFanEmptyId = 30;
			} else if (id == k1ThePoisonStrings) {
				rusFanSkipId = rusFanSkip_k1ThePoisonStrings;
				rusFanSkipIdLen = ARRAYSIZE(rusFanSkip_k1ThePoisonStrings);
			} else if (id == k1FullFlaskString) {
				rusFanSkipId = rusFanSkip_k1FullFlaskStrings;
				rusFanSkipIdLen = ARRAYSIZE(rusFanSkip_k1FullFlaskStrings);
			} else if (id == k1GUIStrings) {
				rusFanSkipId = rusFanSkip_k1GUIStrings;
				rusFanSkipIdLen = ARRAYSIZE(rusFanSkip_k1GUIStrings);
			} else if (id == k1WispJewelStrings) {
				rusFanSkipId = rusFanSkip_k1WispJewelStrings;
				rusFanSkipIdLen = ARRAYSIZE(rusFanSkip_k1WispJewelStrings);
			}
		}

		// HACK
		if (id == k2SeqplayIntroTracks && info->game == kLoL)
			return extractStringsWoSuffix(out, info, data, size, filename, id);
	}

	uint32 entries = 0;
	uint32 targetsize = size + 4;
	for (uint32 i = 0; i < size; ++i) {
		if (!data[i]) {
			if (info->platform == kPlatformAmiga) {
				if (i + 1 >= size)
					++entries;
				else if (!data[i+1] && !(i & 1))
					continue;
				else
					++entries;
			} else {
				++entries;
			}

			if (info->platform == kPlatformFMTowns) {
				// prevents creation of empty entries (which we have mostly between all strings in the FM-TOWNS version)
				while (!data[++i]) {
					if (i == size)
						break;
					targetsize--;
				}
				if (patch == 1) {
					// Here is the first step of the extra treatment for all FM-TOWNS string arrays that
					// contain more than one string and which the original code
					// addresses via stringname[boolJapanese].
					// We simply skip every other string
					if (i == size)
						continue;
					uint32 len = strlen((const char*) data + i);
					i += len;

					targetsize = targetsize - 1 - len;

					while (!data[++i]) {
						if (i == len)
							break;
						targetsize--;
					}
				}
			} else if (patch == 5) {
				++skipCount;
				while (!data[i + 1]) {
					if (skipCount == rusFanEmptyId) {
						++skipCount;
						++entries;
						break;
					}
					if (++i == size)
						break;
					targetsize--;
				}

				// Skip English string left-overs in the hacky Russian fan translation
				for (uint32 ii = 0; ii < rusFanSkipIdLen; ++ii) {
					if (skipCount == rusFanSkipId[ii]) {
						++skipCount;
						uint32 len = strlen((const char*) data + i);
						i += len;
						targetsize = targetsize - 1 - len;
						while (!data[i + 1]) {
							if (++i == len)
								break;
							targetsize--;
						}
					}
				}
			}
		}
	}

	if (patch == 2) {
		if (info->lang == EN_ANY) {
			targetsize--;
			entries += 1;
		} else if (info->lang == JA_JPN) {
			targetsize += 2;
			entries += 2;
		}
	}

	if (patch == 3) {
		entries++;
		targetsize++;
	}

	if (patch == 4) {
		targetsize -= 9;
	}

	uint8 *buffer = new uint8[targetsize];
	assert(buffer);
	memset(buffer, 0, targetsize);
	uint8 *output = buffer;
	const uint8 *input = (const uint8*) data;
	skipCount = 0;

	WRITE_BE_UINT32(output, entries); output += 4;
	if (info->platform == kPlatformFMTowns) {
		const byte *c = data + size;
		do {
			if (patch == 2 && input - data == 0x3C0 && input[0x10] == 0x32) {
				memcpy(output, input, 0x0F);
				input += 0x11; output += 0x0F;
			}

			strcpy((char *) output, (const char*) input);
			uint32 stringsize = strlen((const char*)output) + 1;
			input += stringsize; output += stringsize;
			// skip empty entries
			while (!*input) {
				// Write one empty string into intro strings file
				if (patch == 2) {
					if ((info->lang == EN_ANY && input - data == 0x260) ||
						(info->lang == JA_JPN && (input - data == 0x2BD || input - data == 0x2BE)))
							*output++ = *input;
				}

				// insert one dummy string at hof sequence strings position 59
				if (patch == 3) {
					if ((info->lang == EN_ANY && input - data == 0x695) ||
						(info->lang == JA_JPN && input - data == 0x598))
							*output++ = *input;
				}

				if (++input == c)
					break;
			}

			if (patch == 1) {
				// Here is the extra treatment for all FM-TOWNS string arrays that
				// contain more than one string and which the original code
				// addresses via stringname[boolJapanese].
				// We simply skip every other string
				if (input == c)
					continue;
				input += strlen((const char*)input);
				while (!*input) {
					if (++input == c)
						break;
				}
			}

		} while (input < c);
	} else if (info->platform == kPlatformAmiga) {
		// we need to strip some aligment zeros out here
		int dstPos = 0;
		for (uint32 i = 0; i < size; ++i) {
			if (!data[i] && !(i & 1)) {
				if (i + 1 > size)
					continue;
				else if (i + 1 < size && !data[i+1])
					continue;
			}

			*output++ = data[i];
			++dstPos;
		}
		targetsize = dstPos + 4;
	} else if (patch == 5) {
		const byte *c = data + size;
		do {
			strcpy((char *) output, (const char*) input);
			uint32 stringsize = strlen((const char*)output) + 1;
			input += stringsize; output += stringsize;

			++skipCount;
			while (!*input) {
				if (skipCount == rusFanEmptyId) {
					*output++ = *input;
					++skipCount;
				}
				if (++input == c)
					break;
			}
			// Skip English string left-overs in the hacky Russian fan translation
			for (uint32 ii = 0; ii < rusFanSkipIdLen; ++ii) {
				if (skipCount == rusFanSkipId[ii]) {
					++skipCount;
					input += strlen((const char*)input);
					while (!*input) {
						if (++input == c)
							break;
					}
				}
			}

		} while (input < c);
	} else {
		uint32 copySize = size;
		if (patch == 4) {
			memcpy(output, data, 44);
			output += 44;
			data += 44;
			for (int t = 1; t != 10; t++) {
				sprintf((char *) output, "COST%d_SH.PAK", t);
				output += 13;
			}
			data += 126;
			copySize -= 170;
		}
		memcpy(output, data, copySize);
	}

	return out.addFile(filename, buffer, targetsize);
}

bool extractStrings10(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	// HACK...
	if (info->platform == kPlatformFMTowns && id == k2IngameSfxFiles)
		return extractStringsWoSuffix(out, info, data, size, filename, id);

	const int strSize = 10;
	uint32 entries = (size + (strSize - 1)) / strSize;

	uint8 *buffer = new uint8[size + 4];
	assert(buffer);
	uint8 *output = buffer;
	WRITE_BE_UINT32(output, entries); output += 4;

	for (uint32 i = 0; i < entries; ++i) {
		const byte *src = data + i * strSize;

		while (*src)
			*output++ = *src++;
		*output++ = '\0';
	}

	return out.addFile(filename, buffer, output - buffer);
}

bool extractRooms(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	// different entry size for the FM-TOWNS version
	const int roomEntrySize = (info->platform == kPlatformFMTowns) ? (0x69) : ((info->platform == kPlatformAmiga) ? 0x52 : 0x51);
	const int countRooms = size / roomEntrySize;

	uint8 *buffer = new uint8[countRooms * 9 + 4];
	assert(buffer);
	uint8 *output = buffer;

	WRITE_BE_UINT32(output, countRooms); output += 4;

	const byte *src = data;
	if (info->platform == kPlatformAmiga) {
		for (int i = 0; i < countRooms; ++i) {
			*output++ = *src++; assert(*src == 0); ++src;
			memcpy(output, src, 8); output += 0x8;
			src += roomEntrySize - 0x2;
		}
	} else {
		for (int i = 0; i < countRooms; ++i) {
			*output++ = *src++;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			src += roomEntrySize - 0x9;
		}
	}

	return out.addFile(filename, buffer, countRooms * 9 + 4);
}

bool extractShapes(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	byte *buffer = new byte[size + 1 * 4];
	assert(buffer);
	byte *output = buffer;

	const int count = size / 0x07;
	WRITE_BE_UINT32(output, count); output += 4;
	memcpy(output, data, size);

	return out.addFile(filename, buffer, size + 1 * 4);
}

bool extractKyraForestSeqData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	if (info->platform != kPlatformPC98)
		return extractRaw(out, info, data, size, filename, id);

	struct PatchEntry {
		uint16 pos;
		uint8 val;
	};

	// This data has been taken from the FM-Towns version
	static const PatchEntry patchData[] = {
		{ 0x0019, 0x06 }, { 0x001A, 0x09 }, { 0x001B, 0x00 }, { 0x002E, 0x06 }, { 0x002F, 0x09 }, { 0x0030, 0x00 },
		{ 0x003D, 0x06 }, { 0x003E, 0x09 }, { 0x003F, 0x00 }, { 0x004C, 0x06 }, { 0x004D, 0x09 }, { 0x004E, 0x00 },
		{ 0x005B, 0x06 }, { 0x005C, 0x09 }, { 0x005D, 0x00 }, { 0x0064, 0x06 }, { 0x0065, 0x09 }, { 0x0066, 0x00 },
		{ 0x0079, 0x06 }, { 0x007A, 0x09 }, { 0x007B, 0x00 }, { 0x0088, 0x06 }, { 0x0089, 0x09 }, { 0x008A, 0x00 },
		{ 0x0097, 0x06 }, { 0x0098, 0x09 }, { 0x0099, 0x00 }, { 0x00A6, 0x06 }, { 0x00A7, 0x09 }, { 0x00A8, 0x00 },
		{ 0x00AD, 0x06 }, { 0x00AE, 0x09 }, { 0x00AF, 0x00 }, { 0x00B4, 0x06 }, { 0x00B5, 0x09 }, { 0x00B6, 0x00 },
		{ 0x00C3, 0x06 }, { 0x00C4, 0x09 }, { 0x00C5, 0x00 }, { 0x00CA, 0x06 }, { 0x00CB, 0x09 }, { 0x00CC, 0x00 },
		{ 0x00D1, 0x06 }, { 0x00D2, 0x09 }, { 0x00D3, 0x00 }, { 0x00E0, 0x06 }, { 0x00E1, 0x09 }, { 0x00E2, 0x00 },
		{ 0x00E7, 0x06 }, { 0x00E8, 0x09 }, { 0x00E9, 0x00 }, { 0x00EE, 0x06 }, { 0x00EF, 0x09 }, { 0x00F0, 0x00 },
		{ 0x00FD, 0x06 }, { 0x00FE, 0x09 }, { 0x00FF, 0x00 }, { 0x010A, 0x06 }, { 0x010B, 0x09 }, { 0x010C, 0x00 },
		{ 0x011D, 0x06 }, { 0x011E, 0x09 }, { 0x011F, 0x00 }, { 0x012C, 0x06 }, { 0x012D, 0x09 }, { 0x012E, 0x00 },
		{ 0x013D, 0x06 }, { 0x013E, 0x09 }, { 0x013F, 0x00 }, { 0x0148, 0x06 }, { 0x0149, 0x09 }, { 0x014A, 0x00 },
		{ 0x0153, 0x06 }, { 0x0154, 0x09 }, { 0x0155, 0x00 }, { 0x015E, 0x06 }, { 0x015F, 0x09 }, { 0x0160, 0x00 },
		{ 0x0169, 0x06 }, { 0x016A, 0x09 }, { 0x016B, 0x00 }, { 0x016C, 0x06 }, { 0x016D, 0x12 }, { 0x016E, 0x00 },
		{ 0x017B, 0x06 }, { 0x017C, 0x09 }, { 0x017D, 0x00 }, { 0x0188, 0x06 }, { 0x0189, 0x09 }, { 0x018A, 0x00 },
		{ 0x0190, 0x13 }, { 0x0000, 0x00 }
	};

	uint32 outsize = size + (ARRAYSIZE(patchData) - 1);
	uint8 *buffer = new uint8[outsize];
	assert(buffer);

	const uint8 *src = data;
	uint8 *dst = buffer;
	const PatchEntry *patchPos = patchData;

	while (dst < (buffer + outsize)) {
		if ((dst - buffer) == patchPos->pos) {
			*dst++ = patchPos->val;
			patchPos++;
		} else {
			*dst++ = *src++;
		}
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractAmigaSfx(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	const uint32 entries = size / 8;
	byte *buffer = new byte[entries * 6 + 1 * 4];

	byte *output = buffer;
	WRITE_BE_UINT32(output, entries); output += 4;

	for (uint32 i = 0; i < entries; ++i) {
		*output++ = *data++;	// Note
		*output++ = *data++;	// Patch
		data += 2;				// Unused
		WRITE_BE_UINT16(output, READ_BE_UINT16(data)); output += 2; data += 2; // Duration
		*output++ = *data++;	// Volume
		*output++ = *data++;	// Pan
	}

	return out.addFile(filename, buffer, entries * 6 + 1 * 4);
}

bool extractWdSfx(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	const int bufferSize = 0x12602;

	uint8 *buffer = new uint8[0x12602];
	assert(buffer);
	memcpy(buffer, data, 0x7EE5);
	memcpy(buffer + 0x7EE5, data + 0x7EE7, 0x7FFF);
	memcpy(buffer + 0xFEE4, data + 0xFEE8, 0x271E);

	return out.addFile(filename, buffer, bufferSize);
}

int extractHofSeqData_checkString(const void *ptr, uint8 checkSize);
int extractHofSeqData_isSequence(const void *ptr, const ExtractInformation *info, uint32 maxCheckSize);
int extractHofSeqData_isControl(const void *ptr, uint32 size);

bool extractHofSeqData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int numSequences = 0;
	int numNestedSequences = 0;

	uint16 headerSize = 50 * sizeof(uint16);
	uint16 bufferSize = size + headerSize;
	byte *buffer = new byte[bufferSize];
	assert(buffer);
	memset(buffer, 0, bufferSize );
	uint16 *header = (uint16 *) buffer;
	byte *output = buffer + headerSize;
	uint16 *hdout = header;

	//debug(1, "\nProcessing Hand of Fate sequence data:\n--------------------------------------\n");
	for (int cycle = 0; cycle < 2; cycle++) {
		const byte *ptr = data;
		hdout++;

		const byte *endOffs = (const byte *)(data + size);

		// detect sequence structs
		while (ptr < endOffs) {
			if (ptr[1]) {
				error("invalid sequence data encountered");
				delete[] buffer;
				return false;
			}

			int v = extractHofSeqData_isSequence(ptr, info, endOffs - ptr);

			if (cycle == 0 && v == 1) {
				if ((info->platform == kPlatformPC && info->special == kNoSpecial && *ptr == 5) || (info->special == kDemoVersion && (ptr - data == 312))) {
					// patch for floppy version: skips invalid ferb sequence
					// patch for demo: skips invalid title sequence
					ptr += 54;
					continue;
				}

				numSequences++;
				uint16 relOffs = (uint16) (output - buffer);
				WRITE_BE_UINT16(hdout, relOffs);
				hdout++;

				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // flags
				ptr += 2;
				output += 2;

				memcpy(output, ptr, 28); // wsa and cps file names
				ptr += 28;
				output += 28;

				if (info->platform == kPlatformFMTowns) { // startupCommand + finalCommand
					memcpy(output, ptr, 2);
					ptr += 2;
					output += 2;
				} else {
					*output++ = READ_LE_UINT16(ptr) & 0xff;
					ptr += 2;
					*output++ = READ_LE_UINT16(ptr) & 0xff;
					ptr += 2;
				}

				for (int w = 0; w < 7; w++) { //stringIndex1 to yPos
					WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
					ptr += 2;
					output += 2;
				}

				ptr += 4;
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // duration
				ptr += 2;
				output+= 2;

			} else if (cycle == 1 && v != 1 && v != -2) {
				uint16 controlOffs = 0;
				uint16 ctrSize = 0;
				if (v) {
					const byte *ctrStart = ptr;
					while (v && v != -2) {
						ptr++;
						v = extractHofSeqData_isSequence(ptr, info, endOffs - ptr);
					}

					if (v == -2)
						break;

					ctrSize = (uint16)(ptr - ctrStart);

					if (info->special != kDemoVersion &&
						extractHofSeqData_isControl(ctrStart, ctrSize)) {
						controlOffs = (uint16) (output - buffer);
						*output++ = ctrSize >> 2;

						for (int cc = 0; cc < ctrSize; cc += 2)
							WRITE_BE_UINT16(output + cc, READ_LE_UINT16(ctrStart + cc)); // frame control
						output += ctrSize;
					}
				}

				numNestedSequences++;
				uint16 relOffs = (uint16) (output - buffer);
				WRITE_BE_UINT16(hdout, relOffs);
				hdout++;

				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // flags
				ptr += 2;
				output += 2;

				memcpy(output, ptr, 14); // wsa file name
				ptr += 14;
				output += 14;

				// startframe
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				// endFrame
				WRITE_BE_UINT16(output, (ctrSize && ((ctrSize >> 2)  < READ_LE_UINT16(ptr))) ? (ctrSize >> 2) : READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				// frameDelay
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				ptr += 4;

				for (int w = 0; w < 2; w++) { //x, y
					WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
					ptr += 2;
					output += 2;
				}

				if (!READ_LE_UINT32(ptr))
					controlOffs = 0;

				WRITE_BE_UINT16(output, controlOffs);
				if (info->special != kDemoVersion)
					ptr += 4;
				output += 2;

				if (info->special != kDemoVersion) {
					for (int w = 0; w < 2; w++) { //startupCommand, finalCommand
						WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
						ptr += 2;
						output += 2;
					}
				} else {
					memset(output, 0, 4);
					output += 4;
				}

				if (info->platform == kPlatformFMTowns)
					ptr += 2;

			} else if (cycle == 0) {
				while (v != 1 && v != -2) {
					ptr++;
					v = extractHofSeqData_isSequence(ptr, info, endOffs - ptr);
				}

				if (v == -2)
					break;


			} else if (cycle == 1) {
				while (v == 1 && v != -2) {
					ptr++;
					v = extractHofSeqData_isSequence(ptr, info, endOffs - ptr);
				}

				if (v == -2)
					break;
			}
		}
	}

	uint16 finHeaderSize = (2 + numSequences + numNestedSequences) * sizeof(uint16);
	uint16 finBufferSize = ((output - buffer) - headerSize) + finHeaderSize;
	byte *finBuffer = new byte[finBufferSize];
	assert(finBuffer);
	uint16 diff = headerSize - finHeaderSize;
	uint16 *finHeader = (uint16 *) finBuffer;

	for (int i = 1; i < finHeaderSize; i++)
		WRITE_BE_UINT16(&finHeader[i], (READ_BE_UINT16(&header[i]) - diff));
	WRITE_BE_UINT16(finHeader, numSequences);
	WRITE_BE_UINT16(&finHeader[numSequences + 1], numNestedSequences);
	memcpy (finBuffer + finHeaderSize, buffer + headerSize, finBufferSize - finHeaderSize);
	delete[] buffer;

	finHeader = (uint16 *) (finBuffer + ((numSequences + 2) * sizeof(uint16)));
	for (int i = 0; i < numNestedSequences; i++) {
		uint8 * offs = finBuffer + READ_BE_UINT16(finHeader++) + 26;
		uint16 ctrl = READ_BE_UINT16(offs);
		if (ctrl)
			ctrl -= diff;
		WRITE_BE_UINT16(offs, ctrl);
	}

	return out.addFile(filename, finBuffer, finBufferSize);
}

int extractHofSeqData_checkString(const void *ptr, uint8 checkSize) {
	// return values: 1 = text; 0 = zero string; -1 = other

	int t = 0;
	int c = checkSize;
	const uint8 *s = (const uint8*)ptr;

	// check for character string
	while (c--) {
		if (*s > 31 && *s < 123)
			t++;
		s++;
	}

	if (t == checkSize)
		return 1;

	// check for zero string
	c = checkSize;
	uint32 sum = 0;
	s = (const uint8*)ptr;
	while (c--)
		sum += *s++;

	return (sum) ? -1 : 0;
}

int extractHofSeqData_isSequence(const void *ptr, const ExtractInformation *info, uint32 maxCheckSize) {
	// return values: 1 = Sequence; 0 = Nested Sequence; -1 = other; -2 = overflow

	if (maxCheckSize < 30)
		return -2;

	const uint8 * s = (const uint8*)ptr;
	int c1 = extractHofSeqData_checkString(s + 2, 6);
	int c2 = extractHofSeqData_checkString(s + 16, 6);
	int c3 = extractHofSeqData_checkString(s + 2, 14);
	int c4 = extractHofSeqData_checkString(s + 16, 14);
	int c0 = s[1];
	int c5 = s[0];

	if (c0 == 0 && c5 && ((c1 + c2) >= 1) && (!(c3 == 0 && c2 != 1)) && (!(c4 == 0 && c1 != 1))) {
		if (maxCheckSize < 41)
			return -2;

		if (info->platform == kPlatformFMTowns) {
			if (!(s[37] | s[39]) && s[38] > s[36])
				return 1;
		} else {
			if (!(s[39] | s[41]) && s[40] > s[38])
				return 1;
		}
	}

	if (c0 == 0 && c5 == 4 && c3 == 0 && c4 == 0) {
		if (maxCheckSize >= 41 && READ_LE_UINT32(s + 34) && !(s[39] | s[41]) && s[40] > s[38])
			return 1;
	}

	if (c0 == 0 && c5 && c1 == 1 && c4 == -1 && s[20])
		return 0;

	return -1;
}

int extractHofSeqData_isControl(const void *ptr, uint32 size) {
	// return values: 1 = possible frame control data; 0 = definitely not frame control data

	const uint8 *s = (const uint8*)ptr;
	for (uint32 i = 2; i < size; i += 4) {
		if (!s[i])
			return 0;
	}

	for (uint32 i = 1; i < size; i += 2) {
		if (s[i])
			return 0;
	}
	return 1;
}

bool extractHofShapeAnimDataV1(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src = data;
	uint8 *dst = buffer + 1;

	for (int i = 0; i < 4; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 4;
		dst += 2;
		outsize += 4;

		for (int j = 0; j < 20; j++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2;
			dst += 2;
			outsize += 2;
		}

	};

	*buffer = 4; // number of items

	return out.addFile(filename, buffer, outsize);
}

bool extractHofShapeAnimDataV2(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src = data;
	uint8 *dst = buffer + 1;
	const uint8 *fin = data + size;
	int count = 0;

	do {
		if (READ_LE_UINT16(src) == 0xffff)
			break;

		count++;

		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;

		uint8 numFrames = *src;
		*dst++ = numFrames;
		src += 6;
		outsize += 3;

		for (int i = 0; i < (numFrames << 1); i++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2;
			dst += 2;
			outsize += 2;
		}

		src += (48 - (numFrames << 2));

	} while (src < fin);

	*buffer = count; // number of items

	return out.addFile(filename, buffer, outsize);
}

bool extractStringsWoSuffix(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int outsize = size + 4;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer + 4;
	const uint8 *fin = src + size;
	int entries = 0;

	while (src < fin) {
		while (!*src && src < fin)
			src++;
		while (*src && *src != '.' && src < fin)
			*dst++ = *src++;

		*dst++ = '\0';
		entries++;

		if (*src == '.') {
			while (*src && src < fin)
				src++;
		}
	}

	WRITE_BE_UINT32(buffer, entries);
	outsize = dst - buffer;

	return out.addFile(filename, buffer, outsize);
}

bool extractPaddedStrings(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int outsize = size + 4;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer + 4;
	const uint8 *fin = src + size;
	int entries = 0;

	while (src < fin) {
		while (!*src && src < fin)
			src++;
		while (*src && src < fin)
			*dst++ = *src++;

		*dst++ = '\0';
		entries++;
	}

	WRITE_BE_UINT32(buffer, entries);
	outsize = dst - buffer;

	return out.addFile(filename, buffer, outsize);
}

bool extractRaw16to8(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int outsize = size >> 1;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < outsize; i++) {
		*dst++ = *src++;
		src++;
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractRaw16(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (uint32 i = 0; i < (size >> 1); i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;
	}

	return out.addFile(filename, buffer, size);
}

bool extractRaw32(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (uint32 i = 0; i < (size >> 2); i++) {
		WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
		src += 4;
		dst += 4;
	}

	return out.addFile(filename, buffer, size);
}

bool extractLoLButtonDefs(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int num = size / 22;
	uint8 *buffer = new uint8[size];
	uint32 outsize = num * 18;
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < num; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 6; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractEoB2SeqData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int num = size / 11;
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < num; i++) {
		memcpy(dst, src, 2);
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		memcpy(dst, src, 7);
		src += 7; dst += 7;
	}

	return out.addFile(filename, buffer, size);
}

bool extractEoB2ShapeData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int num = size / 6;
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < num; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		memcpy(dst, src, 4);
		src += 4; dst += 4;
	}

	return out.addFile(filename, buffer, size);
}

bool extractEoBNpcData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	// We use one extraction routine for both EOB 1 and EOB 2 (in spite of the data format differences)
	// since it is easy enough to generate a common output usable by both engines

	uint8 *buffer = 0;
	uint32 outsize = 0;

	if (info->game == kEoB1) {
		uint16 num = size / 243;
		outsize = num * 111 + 2;
		buffer = new uint8[outsize];
		const uint8 *src = data;
		uint8 *dst = buffer;

		WRITE_BE_UINT16(dst, num);
		dst += 2;

		for (int i = 0; i < num; i++) {
			memcpy(dst, src, 27);
			src += 27; dst += 27;
			WRITE_BE_UINT16(dst, *src++);
			dst += 2;
			WRITE_BE_UINT16(dst, *src++);
			dst += 2;
			memcpy(dst, src, 10);
			src += 10; dst += 10;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			// skipping lots of zero space
			src += 64;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			for (int ii = 0; ii < 27; ii++) {
				WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
				src += 2; dst += 2;
			}
			// skipping more zero space
			src += 70;
		}
	} else {
		uint16 num = size / 345;
		outsize = num * 111 + 2;
		buffer = new uint8[outsize];
		const uint8 *src = data;
		uint8 *dst = buffer;

		WRITE_BE_UINT16(dst, num);
		dst += 2;

		for (int i = 0; i < num; i++) {
			memcpy(dst, src, 27);
			src += 27; dst += 27;
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2; dst += 2;
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2; dst += 2;
			memcpy(dst, src, 10);
			src += 10; dst += 10;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			// skipping lots of zero space
			src += 164;
			WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
			src += 4; dst += 4;
			for (int ii = 0; ii < 27; ii++) {
				WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
				src += 2; dst += 2;
			}
			// skipping more zero space
			src += 70;
		}
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractMrShapeAnimData(PAKFile &out, const ExtractInformation *info, const byte *data, const uint32 size, const char *filename, int id) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src2 = data;
	const uint8 *src1 = data + 324;
	uint8 *dst = buffer + 1;
	const uint8 *fin = data + size;
	int count = 0;

	do {
		if (READ_LE_UINT16(src1) == 0xffff)
			break;

		count++;

		WRITE_BE_UINT16(dst, READ_LE_UINT16(src1));
		src1 += 2;
		dst += 2;

		uint8 numFrames = *src1;
		*dst++ = numFrames;
		src1 += 10;
		outsize += 3;

		for (int i = 0; i < (numFrames << 1); i++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src2));
			src2 += 2;
			dst += 2;
			outsize += 2;
		}
	} while (src1 < fin);

	*buffer = count; // number of items

	return out.addFile(filename, buffer, outsize);
}

} // end of anonymous namespace
