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

#include "cruise/cruise_main.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"

namespace Cruise {

enum fileTypeEnum {
	type_UNK,
	type_SPL,
	type_SET,
	type_FNT
};

int loadSingleFile;

/**
 * Takes care of decoding a compressed graphic
 */
void decodeGfxUnified(dataFileEntry *pCurrentFileEntry, int16 format) {
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;
	int spriteSize;

	// Unified how to get spriteSize
	switch (format) {
	case 1:
	case 4:
		spriteSize = pCurrentFileEntry->height * pCurrentFileEntry->width;
		break;
	case 5:
		spriteSize = pCurrentFileEntry->height * pCurrentFileEntry->widthInColumn;
		break;

	default:
		error("Unknown gfx format %d", format);
	}

	uint8 *buffer = (uint8 *)MemAlloc(spriteSize);

	// Perform format specific decoding
	switch (format) {
	case 1:
	case 4: {
		int x = 0;
		while (x < spriteSize) {
			uint8 c;
			uint16 p0;
			// Format 4
			uint16 p1 = 0, p2 = 0, p3 = 0;

			p0 = (dataPtr[0] << 8) | dataPtr[1];

			// Format 4
			if (format == 4) {
				p1 = (dataPtr[2] << 8) | dataPtr[3];
				p2 = (dataPtr[4] << 8) | dataPtr[5];
				p3 = (dataPtr[6] << 8) | dataPtr[7];
			}

			/* decode planes */
			for (c = 0; c < 16; c++) {
				// Format 4
				if (format == 4) {
					buffer[x + c] = ((p0 >> 15) & 1) | ((p1 >> 14) & 2) | ((p2 >> 13) & 4) | ((p3 >> 12) & 8);
				} else {
					buffer[x + c] = ((p0 >> 15) & 1);
				}

				p0 <<= 1;

				// Format 4
				if (format == 4) {
					p1 <<= 1;
					p2 <<= 1;
					p3 <<= 1;
				}
			}

			x += 16;

			dataPtr += (2 * format);
		}

		break;
	}
	case 5: {
		uint8 *destP = buffer;
		int range = pCurrentFileEntry->height * pCurrentFileEntry->width;

		for (int line = 0; line < pCurrentFileEntry->height; line++) {
			uint8 p0, p1, p2, p3, p4;

			for (int x = 0; x < pCurrentFileEntry->widthInColumn; x++) {
				int bit = 7 - (x % 8);
				int col = x / 8;

				p0 = (dataPtr[line*pCurrentFileEntry->width + col + range * 0] >> bit) & 1;
				p1 = (dataPtr[line*pCurrentFileEntry->width + col + range * 1] >> bit) & 1;
				p2 = (dataPtr[line*pCurrentFileEntry->width + col + range * 2] >> bit) & 1;
				p3 = (dataPtr[line*pCurrentFileEntry->width + col + range * 3] >> bit) & 1;
				p4 = (dataPtr[line*pCurrentFileEntry->width + col + range * 4] >> bit) & 1;

				*destP++ = p0 | (p1 << 1) | (p2 << 2) | (p3 << 3) | (p4 << 4);
			}
		}

		break;
	}
	}

	MemFree(pCurrentFileEntry->subData.ptr);
	pCurrentFileEntry->subData.ptr = buffer;
}

int updateResFileEntry(int height, int width, int size, int entryNumber, int resType) {
	int div = 0;

	resetFileEntry(entryNumber);

	filesDatabase[entryNumber].subData.compression = 0;

	int maskSize = size;

	if (resType == 4) {
		div = maskSize / 4;
	} else if (resType == 5) {
		width = (width * 8) / 5;
		maskSize = MAX(size, height * width);
	}

	filesDatabase[entryNumber].subData.ptr = (uint8 *)mallocAndZero(maskSize + div);

	if (!filesDatabase[entryNumber].subData.ptr)
		return (-2);

	filesDatabase[entryNumber].widthInColumn = width;
	filesDatabase[entryNumber].subData.ptrMask = (uint8 *) mallocAndZero(maskSize);
	filesDatabase[entryNumber].width = width / 8;
	filesDatabase[entryNumber].resType = resType;
	filesDatabase[entryNumber].height = height;
	filesDatabase[entryNumber].subData.index = -1;

	return entryNumber;
}

int createResFileEntry(int width, int height, int size, int resType) {
	error("Executing untested createResFileEntry");
	return 0;	// for compilers that don't support NORETURN

#if 0
	int i;
	int entryNumber;
	int div = 0;

	for (i = 0; i < NUM_FILE_ENTRIES; i++) {
		if (!filesDatabase[i].subData.ptr)
			break;
	}

	if (i >= NUM_FILE_ENTRIES) {
		return (-19);
	}

	entryNumber = i;

	filesDatabase[entryNumber].subData.compression = 0;

	if (resType == 4) {
		div = size / 4;
	} else if (resType == 5) {
		width = (width * 8) / 5;
	}

	filesDatabase[entryNumber].subData.ptr = (uint8 *) mallocAndZero(size + div);

	if (!filesDatabase[entryNumber].subData.ptr) {
		return (-2);
	}

	filesDatabase[entryNumber].widthInColumn = width;
	filesDatabase[entryNumber].subData.ptrMask = filesDatabase[entryNumber].subData.ptr + size;
	filesDatabase[entryNumber].width = width / 8;
	filesDatabase[entryNumber].resType = resType;
	filesDatabase[entryNumber].height = height;
	filesDatabase[entryNumber].subData.index = -1;

	return entryNumber;
#endif
}

fileTypeEnum getFileType(const char *name) {
	char extentionBuffer[16];

	fileTypeEnum newFileType = type_UNK;

	getFileExtention(name, extentionBuffer);

	if (!strcmp(extentionBuffer, ".SPL")) {
		newFileType = type_SPL;
	} else if (!strcmp(extentionBuffer, ".SET")) {
		newFileType = type_SET;
	} else if (!strcmp(extentionBuffer, ".FNT")) {
		newFileType = type_FNT;
	}

	ASSERT(newFileType != type_UNK);

	return newFileType;
}

int getNumMaxEntiresInSet(uint8 *ptr) {
	uint16 numEntries = READ_BE_UINT16(ptr + 4);
	return numEntries;
}

int loadFile(const char* name, int idx, int destIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	switch (fileType) {
	case type_SET: {

		int numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);

		if (destIdx > numMaxEntriesInSet) {
			return 0;	// exit if limit is reached
		}
		return loadSetEntry(name, ptr, destIdx, idx);
	}
	case type_FNT: {
		return loadFNTSub(ptr, idx);
	}
	case type_SPL: {
		// Sound file
		loadSPLSub(ptr, idx);
		break;
	}
	default:
		error("Unknown fileType in loadFile");
	}

	return -1;
}

int loadFileRange(const char *name, int startIdx, int currentEntryIdx, int numIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	switch (fileType) {
	case type_SET: {
		int i;
		int numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);

		for (i = 0; i < numIdx; i++) {
			if ((startIdx + i) > numMaxEntriesInSet) {
				return 0;	// exit if limit is reached
			}
			loadSetEntry(name, ptr, startIdx + i, currentEntryIdx + i);
		}

		break;
	}
	case type_FNT: {
		loadFNTSub(ptr, startIdx);
		break;
	}
	case type_SPL: {
		// Sound file
		loadSPLSub(ptr, startIdx);
		break;
	}
	default:
		error("Unknown fileType in loadFileRange");
	}

	MemFree(ptr);

	return 0;
}

int loadFullBundle(const char *name, int startIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	if (ptr == NULL)
		return 0;

	switch (fileType) {
	case type_SET: {
		// Sprite set
		int i;
		int numMaxEntriesInSet;

		numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);	// get maximum number of sprites/animations in SET file

		for (i = 0; i < numMaxEntriesInSet; i++) {
			loadSetEntry(name, ptr, i, startIdx + i);
		}

		break;
	}
	case type_FNT: {
		// Font file
		loadFNTSub(ptr, startIdx);
		break;
	}
	case type_SPL: {
		// Sound file
		loadSPLSub(ptr, startIdx);
		break;
	}
	default:
		error("Unknown fileType in loadFullBundle");
	}

	MemFree(ptr);

	return 0;
}

int loadFNTSub(uint8 *ptr, int destIdx) {
	uint8 *ptr2 = ptr;
	uint8 *destPtr;
	int fileIndex;
	//uint32 fontSize;

	ptr2 += 4;
	loadFileVar1 = READ_BE_UINT32(ptr2);

	if (destIdx == -1) {
		fileIndex = createResFileEntry(loadFileVar1, 1, loadFileVar1, 1);
	} else {
		fileIndex = updateResFileEntry(loadFileVar1, 1, loadFileVar1, destIdx, 1);
	}

	destPtr = filesDatabase[fileIndex].subData.ptr;

	if (destPtr != NULL) {
		int32 i;
		uint8 *currentPtr;

		memcpy(destPtr, ptr2, loadFileVar1);

		//fontSize = READ_BE_UINT32(ptr2);

		destPtr = filesDatabase[fileIndex].subData.ptr;

		bigEndianLongToNative((int32 *) destPtr);
		bigEndianLongToNative((int32 *)(destPtr + 4));
		flipGen(destPtr + 8, 6);

		currentPtr = destPtr + 14;

		for (i = 0; i < (int16)READ_UINT16(destPtr + 8); i++) {
			bigEndianLongToNative((int32 *) currentPtr);
			currentPtr += 4;

			flipGen(currentPtr, 8);
			currentPtr += 8;
		}
	}

	return 1;
}

int loadSPLSub(uint8 *ptr, int destIdx) {
	uint8 *destPtr;
	int fileIndex;

	if (destIdx == -1) {
		fileIndex = createResFileEntry(loadFileVar1, 1, loadFileVar1, 1);
	} else {
		fileIndex = updateResFileEntry(loadFileVar1, 1, loadFileVar1, destIdx, 1);
	}

	destPtr = filesDatabase[fileIndex].subData.ptr;
	memcpy(destPtr, ptr, loadFileVar1);

	return 1;
}


int loadSetEntry(const char *name, uint8 *ptr, int currentEntryIdx, int currentDestEntry) {
	uint8 *ptr3;
	int offset;
	int sec = 0;
	uint16 numIdx;

	if (!strcmp((char *)ptr, "SEC")) {
		sec = 1;
	}

	numIdx = READ_BE_UINT16(ptr + 4);

	ptr3 = ptr + 6;

	offset = currentEntryIdx * 16;

	{
		int resourceSize;
		int fileIndex;
		setHeaderEntry localBuffer;
		uint8 *ptr5;

		Common::MemoryReadStream s4(ptr + offset + 6, 16);

		localBuffer.offset = s4.readUint32BE();
		localBuffer.width = s4.readUint16BE();
		localBuffer.height = s4.readUint16BE();
		localBuffer.type = s4.readUint16BE();
		localBuffer.transparency = s4.readUint16BE() & 0x1F;
		localBuffer.hotspotY = s4.readUint16BE();
		localBuffer.hotspotX = s4.readUint16BE();

		if (sec == 1)
			// Type 1: Width - (1*2) , Type 5: Width - (5*2)
			localBuffer.width -= localBuffer.type * 2;

		resourceSize = localBuffer.width * localBuffer.height;

		if (!sec && (localBuffer.type == 5))
			// Type 5: Width - (2*5)
			localBuffer.width -= 10;

		if (currentDestEntry == -1) {
			fileIndex = createResFileEntry(localBuffer.width, localBuffer.height, resourceSize, localBuffer.type);
		} else {
			fileIndex = updateResFileEntry(localBuffer.height, localBuffer.width, resourceSize, currentDestEntry, localBuffer.type);
		}

		if (fileIndex < 0) {
			return -1;	// TODO: buffer is not freed
		}

		if (!sec && (localBuffer.type == 5)) {
			// There are sometimes sprites with a reduced width than what their pixels provide.
			// The original handled this here by copy parts of each line - for ScummVM, we're
			// simply setting the width in bytes and letting the decoder do the rest
			filesDatabase[fileIndex].width += 2;
		}

		ptr5 = ptr3 + localBuffer.offset + numIdx * 16;

		memcpy(filesDatabase[fileIndex].subData.ptr, ptr5, resourceSize);

		ptr5 += resourceSize;

		switch (localBuffer.type) {
		case 0: { // polygon
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_POLY;
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			break;
		}
		case 1: {
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn * 8;
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_BGMASK;
			decodeGfxUnified(&filesDatabase[fileIndex], localBuffer.type);
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = 0;
			break;
		}
		case 4: {
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn * 2;
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_SPRITE;
			decodeGfxUnified(&filesDatabase[fileIndex], localBuffer.type);
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = localBuffer.transparency % 0x10;
			break;
		}
		case 5: {
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_SPRITE;
			decodeGfxUnified(&filesDatabase[fileIndex], localBuffer.type);
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn;
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = localBuffer.transparency;
			break;
		}
		case 8: {
			filesDatabase[fileIndex].subData.resourceType = OBJ_TYPE_SPRITE;
			filesDatabase[fileIndex].width = filesDatabase[fileIndex].widthInColumn;
			filesDatabase[fileIndex].subData.index = currentEntryIdx;
			filesDatabase[fileIndex].subData.transparency = localBuffer.transparency;
			break;
		}
		default: {
			warning("Unsuported gfx loading type: %d", localBuffer.type);
			break;
		}
		}

		if (name != filesDatabase[fileIndex].subData.name)
			strcpy(filesDatabase[fileIndex].subData.name, name);

		// create the mask
		switch (localBuffer.type) {
		case 1:
		case 4:
		case 5:
		case 8: {
			int maskX;
			int maskY;

			memset(filesDatabase[fileIndex].subData.ptrMask, 0, filesDatabase[fileIndex].width / 8 * filesDatabase[fileIndex].height);

			for (maskY = 0; maskY < filesDatabase[fileIndex].height; maskY++) {
				for (maskX = 0; maskX < filesDatabase[fileIndex].width; maskX++) {
					if (*(filesDatabase[fileIndex].subData.ptr + filesDatabase[fileIndex].width * maskY + maskX) != filesDatabase[fileIndex].subData.transparency) {
						*(filesDatabase[fileIndex].subData.ptrMask + filesDatabase[fileIndex].width / 8 * maskY + maskX / 8) |= 0x80 >> (maskX & 7);
					}
				}
			}

			break;
		}
		default: {
		}
		}
	}

	// TODO: free

	return 1;
}

} // End of namespace Cruise
