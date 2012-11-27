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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/kyra_rpg.h"
#include "kyra/resource.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void KyraRpgEngine::setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim) {
	if (_lvlShapeLeftRight[index << 1] == -1) {
		x1 = 0;
		x2 = 22;

		int16 y1 = 0;
		int16 y2 = 120;

		int m = index * 18;

		for (int i = 0; i < 18; i++) {
			uint8 d = _visibleBlocks[i]->walls[_sceneDrawVarDown];
			uint8 a = _wllWallFlags[d];

			if (a & 8) {
				int t = _dscDim2[(m + i) << 1];

				if (t > x1) {
					x1 = t;
					if (!(a & 0x10))
						setDoorShapeDim(index, y1, y2, -1);
				}

				t = _dscDim2[((m + i) << 1) + 1];

				if (t < x2) {
					x2 = t;
					if (!(a & 0x10))
						setDoorShapeDim(index, y1, y2, -1);
				}
			} else {
				int t = _dscDim1[m + i];

				if (!_wllVmpMap[d] || t == -40)
					continue;

				if (t == -41) {
					x1 = 22;
					x2 = 0;
					break;
				}

				if (t > 0 && x2 > t)
					x2 = t;

				if (t < 0 && x1 < -t)
					x1 = -t;
			}

			if (x2 < x1)
				break;
		}

		x1 += (_sceneXoffset >> 3);
		x2 += (_sceneXoffset >> 3);


		_lvlShapeTop[index] = y1;
		_lvlShapeBottom[index] = y2;
		_lvlShapeLeftRight[index << 1] = x1;
		_lvlShapeLeftRight[(index << 1) + 1] = x2;
	} else {
		x1 = _lvlShapeLeftRight[index << 1];
		x2 = _lvlShapeLeftRight[(index << 1) + 1];
	}

	drawLevelModifyScreenDim(dim, x1, 0, x2, 15);
}

void KyraRpgEngine::setDoorShapeDim(int index, int16 &y1, int16 &y2, int dim) {
	uint8 a = _dscDimMap[index];

	if (_flags.gameID != GI_EOB1 && dim == -1 && a != 3)
		a++;

	uint8 b = a;
	if (_flags.gameID == GI_EOB1) {
		a += _dscDoorFrameIndex1[_currentLevel - 1];
		b += _dscDoorFrameIndex2[_currentLevel - 1];
	}

	y1 = _dscDoorFrameY1[a];
	y2 = _dscDoorFrameY2[b];

	if (dim == -1)
		return;

	const ScreenDim *cDim = screen()->getScreenDim(dim);

	screen()->modifyScreenDim(dim, cDim->sx, y1, cDim->w, y2 - y1);
}

void KyraRpgEngine::drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2) {
	screen()->modifyScreenDim(dim, x1, y1 << 3, x2 - x1, (y2 - y1) << 3);
}

void KyraRpgEngine::generateBlockDrawingBuffer() {
	_sceneDrawVarDown = _dscBlockMap[_currentDirection];
	_sceneDrawVarRight = _dscBlockMap[_currentDirection + 4];
	_sceneDrawVarLeft = _dscBlockMap[_currentDirection + 8];

	/*******************************************
	*             _visibleBlocks map           *
	*                                          *
	*     |     |     |     |     |     |      *
	*  00 |  01 |  02 |  03 |  04 |  05 |  06  *
	* ____|_____|_____|_____|_____|_____|_____ *
	*     |     |     |     |     |     |      *
	*     |  07 |  08 |  09 |  10 |  11 |      *
	*     |_____|_____|_____|_____|_____|      *
	*           |     |     |     |            *
	*           |  12 |  13 |  14 |            *
	*           |_____|_____|_____|            *
	*                 |     |                  *
	*              15 |  16 |  17              *
	*                 | (P) |                  *
	********************************************/

	memset(_blockDrawingBuffer, 0, 660 * sizeof(uint16));

	_wllProcessFlag = ((_currentBlock >> 5) + (_currentBlock & 0x1f) + _currentDirection) & 1;

	if (_wllProcessFlag) // floor and ceiling
		generateVmpTileDataFlipped(0, 15, 1, -330, 22, 15);
	else
		generateVmpTileData(0, 15, 1, -330, 22, 15);

	assignVisibleBlocks(_currentBlock, _currentDirection);

	uint8 t = _visibleBlocks[0]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(-2, 3, t, 102, 3, 5);

	t = _visibleBlocks[6]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(21, 3, t, 102, 3, 5);

	t = _visibleBlocks[1]->walls[_sceneDrawVarRight];
	uint8 t2 = _visibleBlocks[2]->walls[_sceneDrawVarDown];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateVmpTileData(2, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateVmpTileData(2, 3, t2, 102, 3, 5);

	t = _visibleBlocks[5]->walls[_sceneDrawVarLeft];
	t2 = _visibleBlocks[4]->walls[_sceneDrawVarDown];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateVmpTileDataFlipped(17, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateVmpTileDataFlipped(17, 3, t2, 102, 3, 5);

	t = _visibleBlocks[2]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(8, 3, t, 97, 1, 5);

	t = _visibleBlocks[4]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(13, 3, t, 97, 1, 5);

	t = _visibleBlocks[1]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(-4, 3, t, 129, 6, 5);

	t = _visibleBlocks[5]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(20, 3, t, 129, 6, 5);

	t = _visibleBlocks[2]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(2, 3, t, 129, 6, 5);

	t = _visibleBlocks[4]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(14, 3, t, 129, 6, 5);

	t = _visibleBlocks[3]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(8, 3, t, 129, 6, 5);

	t = _visibleBlocks[7]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(0, 3, t, 117, 2, 6);

	t = _visibleBlocks[11]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(20, 3, t, 117, 2, 6);

	t = _visibleBlocks[8]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(6, 2, t, 81, 2, 8);

	t = _visibleBlocks[10]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(14, 2, t, 81, 2, 8);

	t = _visibleBlocks[8]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(-4, 2, t, 159, 10, 8);

	t = _visibleBlocks[10]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(16, 2, t, 159, 10, 8);

	t = _visibleBlocks[9]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(6, 2, t, 159, 10, 8);

	t = _visibleBlocks[12]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(3, 1, t, 45, 3, 12);

	t = _visibleBlocks[14]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(16, 1, t, 45, 3, 12);

	t = _visibleBlocks[12]->walls[_sceneDrawVarDown];
	if (!(_wllWallFlags[t] & 8))
		generateVmpTileData(-13, 1, t, 239, 16, 12);

	t = _visibleBlocks[14]->walls[_sceneDrawVarDown];
	if (!(_wllWallFlags[t] & 8))
		generateVmpTileData(19, 1, t, 239, 16, 12);

	t = _visibleBlocks[13]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(3, 1, t, 239, 16, 12);

	t = _visibleBlocks[15]->walls[_sceneDrawVarRight];
	t2 = _visibleBlocks[17]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileData(0, 0, t, 0, 3, 15);
	if (t2)
		generateVmpTileDataFlipped(19, 0, t2, 0, 3, 15);
}

void KyraRpgEngine::generateVmpTileData(int16 startBlockX, uint8 startBlockY, uint8 vmpMapIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY) {
	if (!_wllVmpMap[vmpMapIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[vmpMapIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numBlocksY; i++) {
		uint16 *bl = &_blockDrawingBuffer[(startBlockY + i) * 22 + startBlockX];
		for (int ii = 0; ii < numBlocksX; ii++) {
			if ((startBlockX + ii >= 0) && (startBlockX + ii < 22) && *vmp)
				*bl = *vmp;
			bl++;
			vmp++;
		}
	}
}

void KyraRpgEngine::generateVmpTileDataFlipped(int16 startBlockX, uint8 startBlockY, uint8 vmpMapIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY) {
	if (!_wllVmpMap[vmpMapIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[vmpMapIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numBlocksY; i++) {
		for (int ii = 0; ii < numBlocksX; ii++) {
			if ((startBlockX + ii) < 0 || (startBlockX + ii) > 21)
				continue;

			uint16 v = vmp[i * numBlocksX + (numBlocksX - 1 - ii)];
			if (!v)
				continue;

			if (v & 0x4000)
				v -= 0x4000;
			else
				v |= 0x4000;

			_blockDrawingBuffer[(startBlockY + i) * 22 + startBlockX + ii] = v;
		}
	}
}

bool KyraRpgEngine::hasWall(int index) {
	if (!index || (_wllWallFlags[index] & 8))
		return false;
	return true;
}

void KyraRpgEngine::assignVisibleBlocks(int block, int direction) {
	for (int i = 0; i < 18; i++) {
		uint16 t = (block + _dscBlockIndex[direction * 18 + i]) & 0x3ff;
		_visibleBlockIndex[i] = t;

		_visibleBlocks[i] = &_levelBlockProperties[t];
		_lvlShapeLeftRight[i] = _lvlShapeLeftRight[18 + i] = -1;
	}
}

bool KyraRpgEngine::checkSceneUpdateNeed(int block) {
	if (_sceneUpdateRequired)
		return true;

	for (int i = 0; i < 15; i++) {
		if (_visibleBlockIndex[i] == block) {
			_sceneUpdateRequired = true;
			return true;
		}
	}

	if (_currentBlock == block) {
		_sceneUpdateRequired = true;
		return true;
	}

	return false;
}

void KyraRpgEngine::drawVcnBlocks() {
	uint8 *d = _sceneWindowBuffer;
	uint16 *bdb = _blockDrawingBuffer;

	for (int y = 0; y < 15; y++) {
		for (int x = 0; x < 22; x++) {
			bool horizontalFlip = false;
			uint16 vcnOffset = *bdb++;
			uint16 vcnExtraOffsetWll = 0;
			int wllVcnOffset = 0;
			int wllVcnRmdOffset = 0;

			if (vcnOffset & 0x8000) {
				// this renders a wall block over the transparent pixels of a floor/ceiling block
				vcnExtraOffsetWll = vcnOffset - 0x8000;
				vcnOffset = 0;
				wllVcnRmdOffset = _wllVcnOffset;
			}

			if (vcnOffset & 0x4000) {
				horizontalFlip = true;
				vcnOffset &= 0x3fff;
			}

			uint8 *src = 0;
			if (vcnOffset) {
				src = &_vcnBlocks[vcnOffset << 5];
				wllVcnOffset = _wllVcnOffset;
			} else {
				// floor/ceiling blocks
				vcnOffset = bdb[329];
				if (vcnOffset & 0x4000) {
					horizontalFlip = true;
					vcnOffset &= 0x3fff;
				}

				src = (_vcfBlocks ? _vcfBlocks : _vcnBlocks) + (vcnOffset << 5);
			}

			uint8 shift = _vcnShift ? _vcnShift[vcnOffset] : _blockBrightness;

			if (horizontalFlip) {
				for (int blockY = 0; blockY < 8; blockY++) {
					src += 3;
					for (int blockX = 0; blockX < 4; blockX++) {
						uint8 bl = *src--;
						*d++ = _vcnColTable[((bl & 0x0f) + wllVcnOffset) | shift];
						*d++ = _vcnColTable[((bl >> 4) + wllVcnOffset) | shift];
					}
					src += 5;
					d += 168;
				}
			} else {
				for (int blockY = 0; blockY < 8; blockY++) {
					for (int blockX = 0; blockX < 4; blockX++) {
						uint8 bl = *src++;
						*d++ = _vcnColTable[((bl >> 4) + wllVcnOffset) | shift];
						*d++ = _vcnColTable[((bl & 0x0f) + wllVcnOffset) | shift];
					}
					d += 168;
				}
			}
			d -= 1400;

			if (vcnExtraOffsetWll) {
				d -= 8;
				horizontalFlip = false;

				if (vcnExtraOffsetWll & 0x4000) {
					vcnExtraOffsetWll &= 0x3fff;
					horizontalFlip = true;
				}

				shift = _vcnShift ? _vcnShift[vcnExtraOffsetWll] : _blockBrightness;
				src = &_vcnBlocks[vcnExtraOffsetWll << 5];
				uint8 *maskTable = _vcnTransitionMask ? &_vcnTransitionMask[vcnExtraOffsetWll << 5] : 0;

				if (horizontalFlip) {
					for (int blockY = 0; blockY < 8; blockY++) {
						src += 3;
						maskTable += 3;
						for (int blockX = 0; blockX < 4; blockX++) {
							uint8 bl = *src--;
							uint8 mask = _vcnTransitionMask ? *maskTable-- : 0;
							uint8 h = _vcnColTable[((bl & 0x0f) + wllVcnRmdOffset) | shift];
							uint8 l = _vcnColTable[((bl >> 4) + wllVcnRmdOffset) | shift];

							if (_vcnTransitionMask)
								*d = (*d & (mask & 0x0f)) | h;
							else if (h)
								*d = h;
							d++;

							if (_vcnTransitionMask)
								*d = (*d & (mask >> 4)) | l;
							else if (l)
								*d = l;
							d++;
						}
						src += 5;
						maskTable += 5;
						d += 168;
					}
				} else {
					for (int blockY = 0; blockY < 8; blockY++) {
						for (int blockX = 0; blockX < 4; blockX++) {
							uint8 bl = *src++;
							uint8 mask = _vcnTransitionMask ? *maskTable++ : 0;
							uint8 h = _vcnColTable[((bl >> 4) + wllVcnRmdOffset) | shift];
							uint8 l = _vcnColTable[((bl & 0x0f) + wllVcnRmdOffset) | shift];

							if (_vcnTransitionMask)
								*d = (*d & (mask >> 4)) | h;
							else if (h)
								*d = h;
							d++;

							if (_vcnTransitionMask)
								*d = (*d & (mask & 0x0f)) | l;
							else if (l)
								*d = l;
							d++;
						}
						d += 168;
					}
				}
				d -= 1400;
			}
		}
		d += 1232;
	}

	screen()->copyBlockToPage(_sceneDrawPage1, _sceneXoffset, 0, 176, 120, _sceneWindowBuffer);
}

uint16 KyraRpgEngine::calcNewBlockPosition(uint16 curBlock, uint16 direction) {
	static const int16 blockPosTable[] = { -32, 1, 32, -1 };
	return (curBlock + blockPosTable[direction]) & 0x3ff;
}

int KyraRpgEngine::clickedWallShape(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	snd_stopSpeech(true);
	runLevelScript(block, 0x40);

	return 1;
}

int KyraRpgEngine::clickedLeverOn(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	_levelBlockProperties[block].walls[direction]++;
	_sceneUpdateRequired = true;

	if (_flags.gameID == GI_LOL)
		snd_playSoundEffect(30, -1);

	runLevelScript(block, _clickedSpecialFlag);

	return 1;
}

int KyraRpgEngine::clickedLeverOff(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	_levelBlockProperties[block].walls[direction]--;
	_sceneUpdateRequired = true;

	if (_flags.gameID == GI_LOL)
		snd_playSoundEffect(29, -1);

	runLevelScript(block, _clickedSpecialFlag);
	return 1;
}

int KyraRpgEngine::clickedWallOnlyScript(uint16 block) {
	runLevelScript(block, _clickedSpecialFlag);
	return 1;
}

void KyraRpgEngine::processDoorSwitch(uint16 block, int openClose) {
	if (block == _currentBlock)
		return;

	if ((_flags.gameID == GI_LOL && (_levelBlockProperties[block].assignedObjects & 0x8000)) || (_flags.gameID != GI_LOL  && (_levelBlockProperties[block].flags & 7)))
		return;

	if (openClose == 0) {
		for (int i = 0; i < 3; i++) {
			if (_openDoorState[i].block != block)
				continue;
			openClose = -_openDoorState[i].state;
			break;
		}
	}

	if (openClose == 0) {
		openClose = (_wllWallFlags[_levelBlockProperties[block].walls[_wllWallFlags[_levelBlockProperties[block].walls[0]] & 8 ? 0 : 1]] & 1) ? 1 : -1;
		if (_flags.gameID != GI_LOL)
			openClose *= -1;
	}

	openCloseDoor(block, openClose);
}

void KyraRpgEngine::openCloseDoor(int block, int openClose) {
	int s1 = -1;
	int s2 = -1;

	int c = (_wllWallFlags[_levelBlockProperties[block].walls[0]] & 8) ? 0 : 1;
	int v = _levelBlockProperties[block].walls[c];
	int flg = (openClose == 1) ? 0x10 : (openClose == -1 ? 0x20 : 0);

	if (_wllWallFlags[v] & flg)
		return;

	for (int i = 0; i < 3; i++) {
		if (_openDoorState[i].block == block) {
			s1 = i;
			break;
		} else if (_openDoorState[i].block == 0 && s2 == -1) {
			s2 = i;
		}
	}

	if (s1 != -1 || s2 != -1) {
		if (s1 == -1)
			s1 = s2;

		_openDoorState[s1].block = block;
		_openDoorState[s1].state = openClose;
		_openDoorState[s1].wall = c;

		flg = (-openClose == 1) ? 0x10 : (-openClose == -1 ? 0x20 : 0);

		if (_wllWallFlags[v] & flg) {
			_levelBlockProperties[block].walls[c] += openClose;
			_levelBlockProperties[block].walls[c ^ 2] += openClose;

			int snd = (openClose == -1) ? 4 : 3;
			if (_flags.gameID == GI_LOL) {
				snd_processEnvironmentalSoundEffect(snd + 28, _currentBlock);
				if (!checkSceneUpdateNeed(block))
					updateEnvironmentalSfx(0);
			} else {
				updateEnvironmentalSfx(snd);
			}
		}

		enableTimer(_flags.gameID == GI_LOL ? 0 : 4);

	} else {
		while (!(flg & _wllWallFlags[v]))
			v += openClose;

		_levelBlockProperties[block].walls[c] = _levelBlockProperties[block].walls[c ^ 2] = v;
		checkSceneUpdateNeed(block);
	}
}

void KyraRpgEngine::completeDoorOperations() {
	for (int i = 0; i < 3; i++) {
		if (!_openDoorState[i].block)
			continue;

		uint16 b = _openDoorState[i].block;

		do {
			_levelBlockProperties[b].walls[_openDoorState[i].wall] += _openDoorState[i].state;
			_levelBlockProperties[b].walls[_openDoorState[i].wall ^ 2] += _openDoorState[i].state;
		} while (!(_wllWallFlags[_levelBlockProperties[b].walls[_openDoorState[i].wall]] & 0x30));

		_openDoorState[i].block = 0;
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL
