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



#include "common/file.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

namespace AGOS {

void AGOSEngine::freezeBottom() {
	_vgaMemBase = _vgaMemPtr;
	_vgaFrozenBase = _vgaMemPtr;
}

void AGOSEngine::unfreezeBottom() {
	_vgaMemPtr = _vgaRealBase;
	_vgaMemBase = _vgaRealBase;
	_vgaFrozenBase = _vgaRealBase;
}

static const uint8 zoneTable[160] = {
	0,  0,  2,  2,  2,  2,  0,  2,  2,  2,
	3,  0,  0,  0,  0,  0,  0,  0,  1,  0,
	3,  3,  3,  1,  3,  0,  0,  0,  1,  0,
	2,  0,  3,  0,  3,  3,  0,  1,  1,  0,
	1,  2,  2,  2,  0,  2,  2,  2,  0,  2,
	1,  2,  2,  2,  0,  2,  2,  2,  2,  2,
	2,  2,  2,  1,  2,  2,  2,  2,  2,  2,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	2,  2,  0,  2,  0,  3,  2,  2,  2,  3,
	2,  3,  3,  3,  1,  3,  3,  1,  1,  0,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	2,  2,  2,  2,  2,  0,  0,  2,  2,  0,
	0,  2,  0,  2,  2,  2,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  2,  2,  2,  2,  2,
	2,  0,  2,  0,  0,  2,  2,  0,  2,  2,
	2,  2,  2,  2,  2,  0,  0,  0,  0,  0,
};

void AGOSEngine::loadZone(uint16 zoneNum, bool useError) {
	VgaPointersEntry *vpe;

	CHECK_BOUNDS(zoneNum, _vgaBufferPointers);

	if (getGameType() == GType_PN) {
		// Only a single zone is used in Personal Nightmare
		vpe = _vgaBufferPointers;
		vc27_resetSprite();
		_vgaMemPtr = _vgaMemBase;
	} else {
		vpe = _vgaBufferPointers + zoneNum;
		if (vpe->vgaFile1 != NULL)
			return;
	}

	// Loading order is important due to resource managment

	if (getPlatform() == Common::kPlatformAmiga && getGameType() == GType_WW &&
		zoneTable[zoneNum] == 3) {
		uint8 num = (zoneNum >= 85) ? 94 : 18;
		loadVGAVideoFile(num, 2, useError);
	} else {
		loadVGAVideoFile(zoneNum, 2, useError);
	}
	vpe->vgaFile2 = _block;
	vpe->vgaFile2End = _blockEnd;

	loadVGAVideoFile(zoneNum, 1, useError);
	vpe->vgaFile1 = _block;
	vpe->vgaFile1End = _blockEnd;

	vpe->sfxFile = NULL;

	if ((getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) &&
		getGameType() == GType_ELVIRA2) {
		// A singe sound file is used for Amiga and AtariST versions
		if (loadVGASoundFile(1, 3)) {
			vpe->sfxFile = _block;
			vpe->sfxFileEnd = _blockEnd;
		}
	} else if (!(getFeatures() & GF_ZLIBCOMP)) {
		if (loadVGASoundFile(zoneNum, 3)) {
			vpe->sfxFile = _block;
			vpe->sfxFileEnd = _blockEnd;
		}
	}
}

void AGOSEngine::setZoneBuffers() {
	_zoneBuffers = (byte *)malloc(_vgaMemSize);

	_vgaMemPtr = _zoneBuffers;
	_vgaMemBase = _zoneBuffers;
	_vgaFrozenBase = _zoneBuffers;
	_vgaRealBase = _zoneBuffers;
	_vgaMemEnd = _zoneBuffers + _vgaMemSize;
}

byte *AGOSEngine::allocBlock(uint32 size) {
	for (;;) {
		_block = _vgaMemPtr;
		_blockEnd = _block + size;

		if (_blockEnd >= _vgaMemEnd) {
			_vgaMemPtr = _vgaMemBase;
		} else {
			_rejectBlock = false;
			checkNoOverWrite();
			if (_rejectBlock)
				continue;
			checkRunningAnims();
			if (_rejectBlock)
				continue;
			checkZonePtrs();
			_vgaMemPtr = _blockEnd;
			return _block;
		}
	}
}

void AGOSEngine::checkRunningAnims() {
	if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		(_videoLockOut & 0x20)) {
		return;
	}

	VgaSprite *vsp;
	for (vsp = _vgaSprites; vsp->id; vsp++) {
		checkAnims(vsp->zoneNum);
		if (_rejectBlock == true)
			return;
	}
}

void AGOSEngine::checkNoOverWrite() {
	VgaPointersEntry *vpe;

	if (_noOverWrite == 0xFFFF)
		return;

	vpe = &_vgaBufferPointers[_noOverWrite];

	if (vpe->vgaFile1 < _blockEnd && vpe->vgaFile1End > _block) {
		_rejectBlock = true;
		_vgaMemPtr = vpe->vgaFile1End;
	} else if (vpe->vgaFile2 < _blockEnd && vpe->vgaFile2End > _block) {
		_rejectBlock = true;
		_vgaMemPtr = vpe->vgaFile2End;
	} else if (vpe->sfxFile && vpe->sfxFile < _blockEnd && vpe->sfxFileEnd > _block) {
		_rejectBlock = true;
		_vgaMemPtr = vpe->sfxFileEnd;
	} else {
		_rejectBlock = false;
	}
}

void AGOSEngine::checkAnims(uint a) {
	VgaPointersEntry *vpe;

	vpe = &_vgaBufferPointers[a];

	if (vpe->vgaFile1 < _blockEnd && vpe->vgaFile1End > _block) {
		_rejectBlock = true;
		_vgaMemPtr = vpe->vgaFile1End;
	} else if (vpe->vgaFile2 < _blockEnd && vpe->vgaFile2End > _block) {
		_rejectBlock = true;
		_vgaMemPtr = vpe->vgaFile2End;
	} else if (vpe->sfxFile && vpe->sfxFile < _blockEnd && vpe->sfxFileEnd > _block) {
		_rejectBlock = true;
		_vgaMemPtr = vpe->sfxFileEnd;
	} else {
		_rejectBlock = false;
	}
}

void AGOSEngine::checkZonePtrs() {
	uint count = ARRAYSIZE(_vgaBufferPointers);
	VgaPointersEntry *vpe = _vgaBufferPointers;
	do {
		if (((vpe->vgaFile1 < _blockEnd) && (vpe->vgaFile1End > _block)) ||
			((vpe->vgaFile2 < _blockEnd) && (vpe->vgaFile2End > _block)) ||
			((vpe->sfxFile < _blockEnd) && (vpe->sfxFileEnd > _block))) {
			vpe->vgaFile1 = NULL;
			vpe->vgaFile1End = NULL;
			vpe->vgaFile2 = NULL;
			vpe->vgaFile2End = NULL;
			vpe->sfxFile = NULL;
			vpe->sfxFileEnd = NULL;
		}
	} while (++vpe, --count);
}

} // End of namespace AGOS
