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


#include "queen/bankman.h"
#include "queen/resource.h"

#include "common/debug.h"

namespace Queen {

BankManager::BankManager(Resource *res)
	: _res(res) {
	memset(_frames, 0, sizeof(_frames));
	memset(_banks, 0, sizeof(_banks));
}

BankManager::~BankManager() {
	for (uint32 i = 0; i < MAX_BANKS_NUMBER; ++i) {
		close(i);
	}
	eraseFrames(true);
}

void BankManager::load(const char *bankname, uint32 bankslot) {
	debug(9, "BankManager::load(%s, %d)", bankname, bankslot);

	assert(bankslot < MAX_BANKS_NUMBER);
	PackedBank *bank = &_banks[bankslot];

	if (!scumm_stricmp(bankname, bank->name)) {
		debug(9, "BankManager::load() bank '%s' already loaded", bankname);
		return;
	}

	close(bankslot);

	if (_res->getPlatform() == Common::kPlatformAmiga && !_res->fileExists(bankname)) {
		debug(9, "BankManager::load() bank '%s' doesn't exist", bankname);
		return;
	}

	bank->data = _res->loadFile(bankname);

	if (_res->getPlatform() == Common::kPlatformAmiga) {
		uint16 entries = READ_BE_UINT16(bank->data + 4);
		debug(9, "BankManager::load() entries = %d", entries);
		assert(entries < MAX_BANK_SIZE);
		uint32 offset = 6;
		_banks[bankslot].indexes[0] = offset;
		for (uint16 i = 1; i <= entries; ++i) {
			_banks[bankslot].indexes[i] = offset;
			uint16 dataSize = READ_BE_UINT16(bank->data + offset + 10);
			offset += dataSize + 12;
		}
	} else {
		uint16 entries = READ_LE_UINT16(bank->data);
		debug(9, "BankManager::load() entries = %d", entries);
		assert(entries < MAX_BANK_SIZE);
		uint32 offset = 2;
		_banks[bankslot].indexes[0] = offset;
		for (uint16 i = 1; i <= entries; ++i) {
			_banks[bankslot].indexes[i] = offset;
			uint16 w = READ_LE_UINT16(bank->data + offset + 0);
			uint16 h = READ_LE_UINT16(bank->data + offset + 2);
			offset += w * h + 8;
		}
	}

	// mark this bank as loaded
	strcpy(bank->name, bankname);
}

static void convertPlanarBitmap(uint8 *dst, int dstPitch, const uint8 *src, int w, int h, int plane) {
	assert(w != 0 && h != 0);
	int planarSize = plane * h * w * 2;
	uint8 *planarBuf = new uint8[ planarSize ];
	uint8 *dstPlanar = planarBuf;
	while (planarSize > 0) {
		if (src[0] == 0) {
			int count = src[1];
			memset(dstPlanar, 0, count);
			dstPlanar += count;
			src += 2;
			planarSize -= count;
		} else {
			*dstPlanar++ = *src++;
			--planarSize;
		}
	}

	src = planarBuf;
	int i = 0;
	int planeSize = h * w * 2;
	while (h--) {
		for (int x = 0; x < w * 2; ++x) {
			for (int b = 0; b < 8; ++b) {
				const uint8 mask = (1 << (7 - b));
				uint8 color = 0;
				for (int p = 0; p < plane; ++p) {
					if (src[planeSize * p + i] & mask) {
						color |= (1 << p);
					}
				}
				dst[8 * x + b] = color;
			}
			++i;
		}
		dst += dstPitch;
	}

	delete[] planarBuf;
}

void BankManager::unpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	debug(9, "BankManager::unpack(%d, %d, %d)", srcframe, dstframe, bankslot);

	assert(bankslot < MAX_BANKS_NUMBER);
	PackedBank *bank = &_banks[bankslot];
	assert(bank->data != NULL);

	assert(dstframe < MAX_FRAMES_NUMBER);
	BobFrame *bf = &_frames[dstframe];
	delete[] bf->data;
	bf->data = NULL;

	const uint8 *p = bank->data + bank->indexes[srcframe];

	if (_res->getPlatform() == Common::kPlatformAmiga) {
		uint16 w     = READ_BE_UINT16(p + 0);
		uint16 h     = READ_BE_UINT16(p + 2);
		uint16 plane = READ_BE_UINT16(p + 4);
		bf->xhotspot = READ_BE_UINT16(p + 6);
		bf->yhotspot = READ_BE_UINT16(p + 8);
		bf->width    = w * 16;
		bf->height   = h;

		uint32 size = bf->width * bf->height;
		if (size != 0) {
			bf->data = new uint8[ size ];
			convertPlanarBitmap(bf->data, bf->width, p + 12, w, h, plane);
		}
	} else {
		bf->width    = READ_LE_UINT16(p + 0);
		bf->height   = READ_LE_UINT16(p + 2);
		bf->xhotspot = READ_LE_UINT16(p + 4);
		bf->yhotspot = READ_LE_UINT16(p + 6);

		uint32 size = bf->width * bf->height;
		if (size != 0) {
			bf->data = new uint8[ size ];
			memcpy(bf->data, p + 8, size);
		}
	}
}

void BankManager::overpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	debug(9, "BankManager::overpack(%d, %d, %d)", srcframe, dstframe, bankslot);

	assert(bankslot < MAX_BANKS_NUMBER);
	PackedBank *bank = &_banks[bankslot];
	assert(bank->data != NULL);

	assert(dstframe < MAX_FRAMES_NUMBER);
	BobFrame *bf = &_frames[dstframe];

	const uint8 *p = bank->data + bank->indexes[srcframe];

	if (_res->getPlatform() == Common::kPlatformAmiga) {
		uint16 w     = READ_BE_UINT16(p + 0);
		uint16 h     = READ_BE_UINT16(p + 2);
		uint16 plane = READ_BE_UINT16(p + 4);
		uint16 src_w = w * 16;
		uint16 src_h = h;

		if (bf->width < src_w || bf->height < src_h) {
			unpack(srcframe, dstframe, bankslot);
		} else {
			convertPlanarBitmap(bf->data, bf->width, p + 12, w, h, plane);
		}
	} else {
		uint16 src_w = READ_LE_UINT16(p + 0);
		uint16 src_h = READ_LE_UINT16(p + 2);

		// unpack if destination frame is smaller than source
		if (bf->width < src_w || bf->height < src_h) {
			unpack(srcframe, dstframe, bankslot);
		} else {
			// copy data 'over' destination frame (without updating frame header)
			memcpy(bf->data, p + 8, src_w * src_h);
		}
	}
}

void BankManager::close(uint32 bankslot) {
	debug(9, "BankManager::close(%d)", bankslot);
	assert(bankslot < MAX_BANKS_NUMBER);
	PackedBank *bank = &_banks[bankslot];
	delete[] bank->data;
	memset(bank, 0, sizeof(PackedBank));
}

BobFrame *BankManager::fetchFrame(uint32 index) {
	debug(9, "BankManager::fetchFrame(%d)", index);
	assert(index < MAX_FRAMES_NUMBER);
	BobFrame *bf = &_frames[index];
	assert((bf->width == 0 && bf->height == 0) || bf->data != 0);
	return bf;
}

void BankManager::eraseFrame(uint32 index) {
	debug(9, "BankManager::eraseFrame(%d)", index);
	assert(index < MAX_FRAMES_NUMBER);
	BobFrame *bf = &_frames[index];
	delete[] bf->data;
	memset(bf, 0, sizeof(BobFrame));
}

void BankManager::eraseFrames(bool joe) {
	for (uint32 i = joe ? 0 : FRAMES_JOE; i < MAX_FRAMES_NUMBER; ++i) {
		eraseFrame(i);
	}
}

} // End of namespace Queen
