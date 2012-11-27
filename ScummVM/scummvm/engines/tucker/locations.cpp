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

#include "tucker/tucker.h"
#include "tucker/graphics.h"
#include "common/system.h"
#include "graphics/palette.h"

namespace Tucker {

static const int _data3UpdateLocation1_xTable[] = { 287, 120,  61 };
static const int _data3UpdateLocation1_yTable[] = { 152,   8,  95 };

void TuckerEngine::execData3PreUpdate_locationNum1() {
	if (_flagsTable[1] == 1) {
		_flagsTable[1] = 2;
		_locationSoundsTable[3].type = 2;
		startSound(_locationSoundsTable[3].offset, 3, _locationSoundsTable[3].volume);
	}
	_mapSequenceFlagsLocationTable[0] = (_flagsTable[1] > 0) ? 1 : 0;
	if (_updateLocationCounter == 0) {
		execData3PreUpdate_locationNum1Helper1();
		execData3PreUpdate_locationNum1Helper2();
	} else {
		--_updateLocationCounter;
		if (_updateLocationCounter == 0) {
			++_updateLocationPos;
			if (_updateLocationPos > 1) {
				_updateLocationPos = 0;
			}
			for (int i = 0; i < 5; ++i) {
				_updateLocationXPosTable[i] = _data3UpdateLocation1_xTable[_updateLocationPos];
				_updateLocationYPosTable[i] = _data3UpdateLocation1_yTable[_updateLocationPos];
				_updateLocationFlagsTable[i] = 0;
			}
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum1Helper1() {
	int code = 0;
	if (_updateLocationXPosTable[0] > 0 && _updateLocationYPosTable[0] > 0) {
		for (int i = 4; i > 0; --i) {
			_updateLocationXPosTable[i] = _updateLocationXPosTable[i - 1];
			_updateLocationYPosTable[i] = _updateLocationYPosTable[i - 1];
			_updateLocationFlagsTable[i] = _updateLocationFlagsTable[i - 1];
		}
	}
	if (_updateLocationFlagsTable[0] == 1 && _updateLocationCounter == 0) {
		setVolumeSound(12, _locationSoundsTable[12].volume);
	} else {
		setVolumeSound(12, 0);
	}
	static const int dxTable[] = {  0, -1, -1, -1, 0, 1, 1,  1 };
	static const int dyTable[] = { -1, -1,  0,  1, 1, 1, 0, -1 };
	for (int i = 0; code == 0 && i < 8; ++i) {
		code = execData3PreUpdate_locationNum1Helper3(dxTable[i], dyTable[i]);
	}
	_updateLocationFlagsTable[0] = code;
}

void TuckerEngine::execData3PreUpdate_locationNum1Helper2() {
	static const int dxTable[] = { -95, 84, 110 };
	static const int dyTable[] = { -50, 46,   0 };
	int xPos = 0;
	int yPos = 0;
	for (int i = 0; i < 6; i += 2) {
		if (_updateLocationFlagsTable[i] == 1) {
			xPos = _updateLocationXPosTable[i] + dxTable[_updateLocationPos];
			yPos = _updateLocationYPosTable[i] + dyTable[_updateLocationPos];
		}
		if (xPos > 319 || yPos > 199 || xPos < 0 || yPos < 0) {
			xPos = 0;
			yPos = 0;
		}
		_locationBackgroundGfxBuf[yPos * 640 + xPos] = 100;
		addDirtyRect(xPos, yPos, 1, 1);
	}
}

int TuckerEngine::execData3PreUpdate_locationNum1Helper3(int dx, int dy) {
	static const int counterTable[3] = { 30, 50, 70 };
	const int xPos = _updateLocationXPosTable[0] + dx;
	const int yPos = _updateLocationYPosTable[0] + dy;
	static const int num[] = { 2, 3, 4, 1 };
	for (int i = 0; i < 4; ++i) {
		if (xPos == _updateLocationXPosTable[num[i]] && yPos == _updateLocationYPosTable[num[i]]) {
			return 0;
		}
	}
	const int code = (int8)_loadLocBufPtr[yPos * 320 + xPos];
	if (code > 0) {
		_updateLocationXPosTable[0] = xPos;
		_updateLocationYPosTable[0] = yPos;
		if (xPos == _data3UpdateLocation1_xTable[_updateLocationPos] && yPos == _data3UpdateLocation1_yTable[_updateLocationPos]) {
			_updateLocationCounter = counterTable[_updateLocationPos];
		}
	}
	return code;
}

void TuckerEngine::execData3PostUpdate_locationNum1() {
	if (_flagsTable[63] == 0) {
		if (getRandomNumber() < 400) {
			_flagsTable[63] = 1;
			startSound(_locationSoundsTable[2].offset, 2, _locationSoundsTable[2].volume);
		}
	}
	_locationHeightTable[1] = (_yPosCurrent > 104) ? 60 : 0;
}

void TuckerEngine::updateSprite_locationNum2() {
	if (_flagsTable[9] == 2) {
		_spritesTable[0].state = -1;
		return;
	}
	if (_flagsTable[9] == 1) {
		if (_flagsTable[10] == 1) {
			_spritesTable[0].state = -1;
		} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
			_spritesTable[0].state = 4;
			_spritesTable[0].needUpdate = 1;
		} else {
			_spritesTable[0].needUpdate = 0;
			_spritesTable[0].state = 5;
			_flagsTable[10] = 1;
			_spritesTable[0].gfxBackgroundOffset = 0;
		}
		return;
	}
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		_spritesTable[0].state = 3;
		_spritesTable[0].needUpdate = 1;
		return;
	}
	if (_csDataHandled) {
		_spritesTable[0].needUpdate = 0;
		if (_flagsTable[199] == 0) {
			_flagsTable[199] = 1;
			setCharacterAnimation(0, 0);
		} else if (getRandomNumber() > 20000) {
			_spritesTable[0].state = 6;
		} else {
			_spritesTable[0].state = 3;
			_spritesTable[0].updateDelay = 5;
		}
	} else {
		if (_flagsTable[199] == 0) {
			_flagsTable[199] = 1;
			setCharacterAnimation(1, 0);
		} else if (getRandomNumber() < 20000) {
			_spritesTable[0].state = 1;
			_spritesTable[0].updateDelay = 5;
		} else {
			_spritesTable[0].state = 1;
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum2() {
	static const int xPosTable[] = { 110, 117, 127, 146, 160, 183, 195, 210 };
	static const int yPosTable[] = {  50,  60,  60,  60,  50,  55,  50,  50 };
	static const int yMaxTable[] = { 100,  97,  94,  91,  89,  86,  83,  80 };
	if (_execData3Counter == 0) {
		_execData3Counter = 1;
		for (int i = 0; i < 3; ++i) {
			_updateLocationYPosTable2[i] = 0;
		}
		if (_flagsTable[10] == 0) {
			copyLocBitmap("path02b.pcx", 0, true);
		}
		for (int i = 0; i < 3; ++i) {
			if (_updateLocationYPosTable2[i] > 0) {
				for (int j = 0; j < 2; ++j) {
					const int offset = (_updateLocationYPosTable2[i] + j) * 640 + _updateLocationXPosTable2[i];
					_locationBackgroundGfxBuf[offset] = 142 + j * 2;
				}
				addDirtyRect(_updateLocationXPosTable2[i], _updateLocationYPosTable2[i], 1, 2);
				_updateLocationYPosTable2[i] += 2;
				if (_updateLocationYPosTable2[i] > _updateLocationYMaxTable[i]) {
					_updateLocationYPosTable2[i] = 0;
					const int num = (getRandomNumber() < 16000) ? 2 : 3;
					startSound(_locationSoundsTable[num].offset, num, _locationSoundsTable[num].volume);
				}
			} else if (getRandomNumber() > 32000) {
				const int num = getRandomNumber() / 8192;
				_updateLocationXPosTable2[i] = xPosTable[num];
				_updateLocationYPosTable2[i] = yPosTable[num];
				_updateLocationYMaxTable[i] = yMaxTable[num];
			}
		}
	}
	execData3PreUpdate_locationNum2Helper();
}

void TuckerEngine::execData3PreUpdate_locationNum2Helper() {
	if (_fadePaletteCounter != 16) {
		return;
	}
	int start, end;
	if (_locationNum == 2) {
		start = 116;
		end = 125;
	} else {
		start = 215;
		end = 223;
	}
	++_updateLocationFadePaletteCounter;
	if (_updateLocationFadePaletteCounter > 10 || _updateLocationFadePaletteCounter < 0) {
		_updateLocationFadePaletteCounter = 0;
	}
	int step;
	if (_updateLocationFadePaletteCounter < 6) {
		step = _updateLocationFadePaletteCounter;
	} else {
		step = 10 - _updateLocationFadePaletteCounter;
	}
	for (int i = start; i < end; ++i) {
		fadePaletteColor(i, step);
	}
}

void TuckerEngine::updateSprite_locationNum3_0(int i) {
	int num;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		num = 5;
		_spritesTable[i].needUpdate = 1;
	} else if (_spritesTable[i].prevState == 5 && _spritesTable[i].animationFrame == 1) {
		_spritesTable[i].needUpdate = 0;
		int r = getRandomNumber();
		if (r < 12000) {
			num = 3;
		} else if (r < 24000) {
			num = 1;
		} else {
			num = 2;
		}
	} else {
		num = 5;
		_spritesTable[i].animationFrame = 1;
		_spritesTable[i].updateDelay = 10;
	}
	_spritesTable[i].state = num;
	_spritesTable[i].prevAnimationFrame = 1;
}

void TuckerEngine::updateSprite_locationNum3_1(int i) {
	int num;
	if (_flagsTable[207] == 1) {
		num = -1;
	} else if (_flagsTable[203] == 1) {
		_spritesTable[i].needUpdate = 1;
		num = 20;
		_flagsTable[203] = 2;
	} else if (_flagsTable[203] == 2) {
		num = 12;
	} else if (_flagsTable[203] == 3) {
		_spritesTable[i].needUpdate = 0;
		num = 19;
		_flagsTable[203] = 0;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		if (_flagsTable[21] == 0) {
			num = 7;
		} else {
			num = 8;
		}
	} else {
		_spritesTable[i].needUpdate = 0;
		if (_csDataHandled) {
			num = 6;
			if (getRandomNumber() < 32000) {
				_spritesTable[i].updateDelay = 5;
			}
		} else if (_spritesTable[i].state == 6 && _spritesTable[i].animationFrame == 1) {
			if (getRandomNumber() < 20000) {
				num = 11;
				_spritesTable[i].defaultUpdateDelay = 5;
			} else {
				num = 10;
			}
		} else {
			num = 6;
			_spritesTable[i].animationFrame = 1;
			_spritesTable[i].updateDelay = 10;
		}
	}
	_spritesTable[i].state = num;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum3_2(int i) {
	int num;
	if (_flagsTable[205] == 1) {
		_flagsTable[205] = 0;
		num = 18;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		num = 17;
	} else if (_flagsTable[45] == 1) {
		_flagsTable[45] = 2;
		num = 16;
	} else if (_flagsTable[45] == 2) {
		num = 17;
		_spritesTable[i].updateDelay = 5;
		_spritesTable[i].needUpdate = 0;
	} else if (_flagsTable[45] == 3) {
		_flagsTable[45] = 0;
		num = 16;
		_updateSpriteFlag2 = 1;
	} else {
		_spritesTable[i].needUpdate = 0;
		++_spritesTable[i].counter;
		if (_spritesTable[i].counter <= 5) {
			num = 14;
		} else {
			if (_spritesTable[i].counter > 8) {
				_spritesTable[i].counter = 0;
			}
			num = 15;
		}
	}
	_spritesTable[i].state = num;
}

void TuckerEngine::execData3PreUpdate_locationNum3() {
	execData3PreUpdate_locationNum2Helper();
	if (_flagsTable[7] == 0) {
		_flagsTable[7] = 1;
	}
	if (_flagsTable[20] == 1 && _inventoryItemsState[7] == 1) {
		_flagsTable[20] = 2;
	}
	if (_inventoryItemsState[19] == 1 && _flagsTable[54] == 1) {
		_flagsTable[43] = 1;
	} else {
		_flagsTable[43] = 0;
	}
}

void TuckerEngine::updateSprite_locationNum4(int i) {
	int state = -1;
	if (_flagsTable[9] == 2) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			state = 1;
			_spritesTable[i].needUpdate = 1;
		} else {
			state = 2;
			_spritesTable[i].needUpdate = 0;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum4() {
	if (_flagsTable[6] == 0) {
		setVolumeSound(0, 0);
	} else {
		setVolumeSound(0, _locationSoundsTable[0].volume);
	}
}

void TuckerEngine::updateSprite_locationNum5_0() {
	++_spritesTable[0].counter;
	if (_spritesTable[0].counter > 100) {
		_spritesTable[0].counter = 0;
		_spritesTable[0].state = 1;
		_locationSoundsTable[1].type = 2;
		startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
	} else if (_spritesTable[0].counter == 50) {
		_locationSoundsTable[1].type = 2;
		_spritesTable[0].state = 2;
		startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
	} else {
		_spritesTable[0].state = -1;
		if (isSoundPlaying(1)) {
			stopSound(1);
		}
	}
}

void TuckerEngine::updateSprite_locationNum5_1(int i) {
	_spritesTable[i].state = 3;
}

void TuckerEngine::updateSprite_locationNum6_0(int i) {
	int state;
	if (_flagsTable[184] == 1) {
		state = -1;
	} else if (_flagsTable[26] > 0 && _flagsTable[26] < 4 && _flagsTable[27] > 0) {
		state = 14;
	} else if (_flagsTable[26] == 4 || _flagsTable[26] == 5) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
			_spritesTable[0].needUpdate = 1;
			state = 3;
			_spritesTable[0].counter = 0;
		} else if (_xPosCurrent < 370 && _flagsTable[26] == 4) {
			state = 2;
		} else if (_spritesTable[0].counter == 0) {
			setCharacterAnimation(0, 0);
			_updateSpriteFlag1 = 1;
			++_spritesTable[0].counter;
			return;
		} else {
			state = 1;
			_spritesTable[0].updateDelay = 2;
			++_spritesTable[0].counter;
			if (_spritesTable[0].counter > 100) {
				_spritesTable[0].counter = 0;
			}
		}
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum6_1(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		if (_flagsTable[209] == 1) {
			state = 9;
			_spritesTable[i].needUpdate = 1;
		} else {
			_spritesTable[i].needUpdate = 1;
			state = 10;
		}
	} else {
		if (getRandomNumber() < 30000 || (_csDataHandled && _xPosCurrent == 248)) {
			_spritesTable[i].needUpdate = 0;
			state = 7;
			_spritesTable[i].updateDelay = 5;
		} else {
			_spritesTable[i].needUpdate = 0;
			state = 7;
			_miscSoundFxNum[0] = 3;
			_miscSoundFxDelayCounter[0] = 70;
			_miscSoundFxNum[1] = 4;
			_miscSoundFxDelayCounter[1] = 25;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum6_2(int i) {
	int state;
	if (_flagsTable[26] < 5 || _flagsTable[207] > 0) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 12;
	} else {
		state = 12;
		_spritesTable[i].updateDelay = 2;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum6() {
	if (_flagsTable[26] > 3) {
		execData3PreUpdate_locationNum6Helper1();
	}
	if (_flagsTable[26] == 2) {
		_currentSpriteAnimationLength = 2;
		_currentSpriteAnimationFrame = 112;
		if (_spritesTable[0].xSource < 45) {
			_currentSpriteAnimationFrame2 = 144;
		} else if (_spritesTable[0].xSource > 80) {
			_currentSpriteAnimationFrame2 = 147;
		} else if (_spritesTable[0].xSource < 60) {
			_currentSpriteAnimationFrame2 = 145;
		} else {
			_currentSpriteAnimationFrame2 = 146;
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum6Helper1() {
	if (_mainLoopCounter1 < 2) {
		if (_flagsTable[26] == 1) {
			if (_flagsTable[27] < 15) {
				if (_flagsTable[27] == 0) {
					startSound(_locationSoundsTable[2].offset, 2, _locationSoundsTable[2].volume);
					startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
				}
				++_flagsTable[27];
				setVolumeSound(0, _locationSoundsTable[0].volume - _flagsTable[27] * 5);
				setVolumeMusic(0, _locationMusicsTable[0].volume - _flagsTable[27] * 5);
			}
		} else if (_flagsTable[26] == 3) {
			if (_flagsTable[27] > 0) {
				if (_flagsTable[27] == 15) {
					startSound(_locationSoundsTable[2].offset, 2, _locationSoundsTable[2].volume);
				}
				--_flagsTable[27];
				setVolumeSound(0, _locationSoundsTable[0].volume - _flagsTable[27] * 5);
				setVolumeMusic(0, _locationMusicsTable[0].volume - _flagsTable[27] * 5);
				int volume = _flagsTable[27];
				if (volume < _locationSoundsTable[1].volume) {
					volume = _locationSoundsTable[1].volume;
				}
				setVolumeSound(1, volume);
			}
		}
	}
	int x1, x2;
	if (_flagsTable[27] == 0) {
		x1 = 8;
		x2 = 0;
	} else if (_flagsTable[27] == 15) {
		x1 = 1;
		x2 = 0;
	} else if (_flagsTable[27] < 8) {
		x1 = 8 - _flagsTable[27];
		x2 = _flagsTable[27];
	} else {
		x1 = 1;
		x2 = 15 - _flagsTable[27];
	}
	for (int i = 0; i < x1; ++i) {
		execData3PreUpdate_locationNum6Helper2(20 * 640 + 325 + i * 8, _data3GfxBuf + _dataTable[238].sourceOffset);
		execData3PreUpdate_locationNum6Helper2(20 * 640 + 445 - i * 8, _data3GfxBuf + _dataTable[238].sourceOffset);
	}
	for (int i = 0; i < x2; ++i) {
		execData3PreUpdate_locationNum6Helper3(20 * 640 + 325 + x1 * 8 + i * 4, _data3GfxBuf + _dataTable[238].sourceOffset);
		execData3PreUpdate_locationNum6Helper3(20 * 640 + 449 - x1 * 8 - i * 4, _data3GfxBuf + _dataTable[238].sourceOffset);
	}
	addDirtyRect(0, 20, 640, 51);
}

void TuckerEngine::execData3PreUpdate_locationNum6Helper2(int dstOffset, const uint8 *src) {
	for (int j = 0; j < 46; ++j) {
		memcpy(_locationBackgroundGfxBuf + dstOffset + j * 640, src + j * 8, 8);
	}
	for (int j = 46; j < 51; ++j) {
		for (int i = 0; i < 8; ++i) {
			const int offset = dstOffset + j * 640 + i;
			uint8 color = _locationBackgroundGfxBuf[offset];
			if (color < 224) {
				_locationBackgroundGfxBuf[offset] = src[j * 8 + i];
			}
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum6Helper3(int dstOffset, const uint8 *src) {
	for (int j = 0; j < 51; ++j) {
		for (int i = 0; i < 4; ++i) {
			const int offset = dstOffset + j * 640 + i;
			uint8 color = _locationBackgroundGfxBuf[offset];
			if (color < 224) {
				_locationBackgroundGfxBuf[offset] = src[j * 8 + i * 2];
			}
		}
	}
}

void TuckerEngine::execData3PostUpdate_locationNum6() {
	if (_flagsTable[26] < 4) {
		execData3PreUpdate_locationNum6Helper1();
	}
}

void TuckerEngine::updateSprite_locationNum7_0(int i) {
	int state;
	++_spritesTable[i].counter;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 4;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (_spritesTable[i].counter > 0) {
			state = 2;
			_spritesTable[i].counter = 0;
		} else {
			state = 1;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum7_1(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 5;
		_spritesTable[i].updateDelay = 5;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum8_0(int i) {
	_spritesTable[i].state = (_flagsTable[28] < 20) ? -1 : 7;
}

void TuckerEngine::updateSprite_locationNum8_1(int i) {
	int state;
	if (_flagsTable[207] == 1) {
		state = -1;
	} else if (_flagsTable[28] == 18) {
		state = 6;
		_spritesTable[i].needUpdate = 0;
	} else if (_flagsTable[29] == 0) {
		state = 1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 1) {
		_spritesTable[i].needUpdate = 1;
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 3;
	}
	if (_flagsTable[28] == 19) {
		_flagsTable[28] = 20;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PostUpdate_locationNum8() {
	if (_execData3Counter == 0) {
		_updateLocationYPosTable2[0] = 0;
	}
	if (_updateLocationYPosTable2[0] == 0) {
		++_execData3Counter;
		if (_execData3Counter > 30) {
			_updateLocationYPosTable2[0] = 16;
			_updateLocationXPosTable2[0] = 264;
		}
	}
	if (_updateLocationYPosTable2[0] > 0) {
		const int offset = _updateLocationYPosTable2[0] * 640 + _updateLocationXPosTable2[0];
		static const int colorsTable[] = { 143, 143, 144, 144, 144, 145, 147, 143, 147 };
		_locationBackgroundGfxBuf[offset] = 142;
		for (int j = 1; j <= 3; ++j) {
			for (int i = -1; i <= 1; ++i) {
				_locationBackgroundGfxBuf[offset + 640 * j + i] = colorsTable[(j - 1) * 3  + i + 1];
			}
		}
		addDirtyRect(_updateLocationXPosTable2[0] - 1, _updateLocationYPosTable2[0], 3, 4);
		_updateLocationYPosTable2[0] += 2;
		if (_updateLocationYPosTable2[0] > 120) {
			_updateLocationYPosTable2[0] = 0;
			startSound(_locationSoundsTable[2].offset, 2, _locationSoundsTable[2].volume);
		}
	}
	if (_flagsTable[28] > 1 && _flagsTable[28] < 5) {
		_locationHeightTable[8] = 60;
	} else {
		_locationHeightTable[8] = 0;
	}
}

void TuckerEngine::updateSprite_locationNum9_0(int i) {
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		_spritesTable[i].needUpdate = 1;
		_spritesTable[i].state = 3;
	} else if (_updateLocationCounter2 > 0 || getRandomNumber() > 30000) {
		_spritesTable[i].state = 1;
		if (_updateLocationCounter2 == 0) {
			_updateLocationCounter2 = 6;
		}
	} else if (getRandomNumber() > 30000) {
		_spritesTable[i].state = 2;
		_spritesTable[i].defaultUpdateDelay = 5;
	} else {
		_spritesTable[i].state = 1;
		_spritesTable[i].animationFrame = 1;
		_spritesTable[i].updateDelay = 10;
	}
}

void TuckerEngine::updateSprite_locationNum9_1(int i) {
	++_spritesTable[i].counter;
	if (_spritesTable[i].counter > 10) {
		_spritesTable[i].counter = 0;
		_spritesTable[i].state = 5;
		_spritesTable[i].defaultUpdateDelay = 5;
	} else {
		_spritesTable[i].state = 4;
	}
}

void TuckerEngine::updateSprite_locationNum9_2(int i) {
	_spritesTable[i].state = 6;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum9() {
	if (_flagsTable[7] < 2) {
		_flagsTable[7] = 2;
	}
	if (_flagsTable[8] == 0 && _locationMusicsTable[0].volume != 0) {
		_locationMusicsTable[0].volume = 0;
	} else {
		_locationMusicsTable[0].volume = _xPosCurrent / 40;
	}
	setVolumeMusic(0, _locationMusicsTable[0].volume);
	if (!isSoundPlaying(1) && getRandomNumber() > 32000) {
		int i = getRandomNumber() / 5500 + 3;
		assert(i >= 0 && i < kLocationSoundsTableSize);
		startSound(_locationSoundsTable[i].offset, i, _locationSoundsTable[i].volume);
	}
	if (_flagsTable[8] == 2 && _locationMaskType == 0) {
		_flagsTable[8] = 0;
		startSound(_locationSoundsTable[7].offset, 7, _locationSoundsTable[7].volume);
	}
}

void TuckerEngine::execData3PostUpdate_locationNum9() {
	if (_spritesTable[1].state == 4) {
		if (_spritesTable[1].flipX == 1) {
			--_updateLocationXPosTable2[0];
			if (_updateLocationXPosTable2[0] < -50) {
				_spritesTable[1].flipX = 0;
				_updateLocationXPosTable2[0] = -50;
			}
		} else {
			++_updateLocationXPosTable2[0];
			if (_updateLocationXPosTable2[0] > 500) {
				_spritesTable[1].flipX = 1;
				_updateLocationXPosTable2[0] = 500;
			}
		}
	}
	_spritesTable[1].gfxBackgroundOffset = _updateLocationXPosTable2[0] + 8320;
	for (int i = 1; i < 3; ++i) {
		_spritesTable[i].colorType = 1;
		_spritesTable[i].yMaxBackground = 60;
		drawSprite(i);
		_spritesTable[i].colorType = 0;
	}
}

void TuckerEngine::updateSprite_locationNum10() {
	int state = 0;
	const int r = getRandomNumber();
	if (_flagsTable[99] == 1) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		_spritesTable[0].needUpdate = 1;
		if (r < 26000) {
			state = 1;
		} else if (r < 29000) {
			state = 2;
		} else {
			state = 3;
		}
	} else if (_csDataHandled) {
		state = 2;
		_spritesTable[0].updateDelay = 4;
	} else {
		_spritesTable[0].needUpdate = 0;
		if (r > 26000) {
			state = 5;
			_spritesTable[0].prevAnimationFrame = 1;
		} else if (r > 24000) {
			state = 6;
			_miscSoundFxDelayCounter[0] = 120;
			_miscSoundFxNum[0] = 0;
		} else {
			setCharacterAnimation(0, 0);
		}
	}
	_spritesTable[0].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum10() {
	if (_execData3Counter == 0) {
		_execData3Counter = 1;
		++_flagsTable[68];
		if (_flagsTable[68] > 2) {
			_flagsTable[68] = 0;
		}
	}
	if (_flagsTable[47] == 1 && _inventoryItemsState[26] == 1) {
		_flagsTable[47] = 2;
	}
	if (_spritesTable[0].state == 6 && _spritesTable[0].animationFrame == 18 && !isSoundPlaying(0)) {
		startSound(_locationSoundsTable[0].offset, 0, _locationSoundsTable[0].volume);
	}
	if (_flagsTable[230] == 1 && getRandomNumber() > 32000) {
		_flagsTable[230] = 0;
	}
}

void TuckerEngine::updateSprite_locationNum11_0(int i) {
	const int r = getRandomNumber();
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		_spritesTable[i].state = 2;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (r > 28000) {
			_spritesTable[i].state = 3;
		} else if (r > 20000) {
			_spritesTable[i].state = 4;
		} else {
			_spritesTable[i].state = 3;
			_spritesTable[i].animationFrame = 1;
			_spritesTable[i].updateDelay = 5;
		}
	}
}

void TuckerEngine::updateSprite_locationNum11_1(int i) {
	if (getRandomNumber() > 20000) {
		_spritesTable[i].state = 5;
	} else {
		_spritesTable[i].animationFrame = 14;
		_updateSpriteFlag1 = 1;
		_spritesTable[i].state = 5;
	}
}

void TuckerEngine::updateSprite_locationNum11_2(int i) {
	if (getRandomNumber() > 20000) {
		_spritesTable[i].state = 6;
	} else {
		_spritesTable[i].animationFrame = 17;
		_spritesTable[i].state = 6;
		_updateSpriteFlag1 = 1;
	}
}

void TuckerEngine::updateSprite_locationNum11_3(int i) {
	_spritesTable[i].state = 7;
}

void TuckerEngine::updateSprite_locationNum11_4(int i) {
	if (getRandomNumber() > 30000 && _flagsTable[55] < 2) {
		_spritesTable[i].state = 1;
		startSound(_locationSoundsTable[6].offset, 6, _locationSoundsTable[6].volume);
	} else {
		_spritesTable[i].state = -1;
	}
}

void TuckerEngine::updateSprite_locationNum12_0(int i) {
	++_updateLocationCounter2;
	if (_updateLocationCounter2 > 11) {
		_updateLocationCounter2 = 0;
	}
	int state;
	if (_flagsTable[207] > 0) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (_updateLocationCounter2 < 4) {
			state = 1;
		} else if (_updateLocationCounter2 < 8) {
			state = 2;
		} else {
			state = 4;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum12_1(int i) {
	int state;
	const int r = getRandomNumber();
	if (r > 22000) {
		state = 6;
	} else if (r > 10000) {
		state = 7;
	} else {
		state = 8;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum12() {
	if (_inventoryItemsState[12] == 1 && _flagsTable[38] == 0) {
		_flagsTable[38] = 1;
	}
	if (_flagsTable[41] == 0) {
		if (_locationMusicsTable[0].volume != 100) {
			_locationMusicsTable[0].volume = 100;
			setVolumeMusic(0, 100);
		}
	} else {
		if (_locationMusicsTable[0].volume != 20) {
			_locationMusicsTable[0].volume = 20;
			setVolumeMusic(0, 20);
		}
	}
}

void TuckerEngine::updateSprite_locationNum13(int i) {
	int state = 0;
	if (_csDataHandled) {
		if (_flagsTable[202] == 0) {
			_flagsTable[202] = 1;
			state = 3;
			_spritesTable[i].stateIndex = -1;
		} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			state = 4;
			_spritesTable[i].needUpdate = 1;
		} else {
			_spritesTable[i].needUpdate = 0;
			state = 5;
		}
	} else if (_flagsTable[202] == 1) {
		_spritesTable[i].needUpdate = 0;
		_flagsTable[202] = 0;
		state = 6;
	} else {
		setCharacterAnimation(0, 0);
		return;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum13() {
	if (_flagsTable[69] == 0) {
		if (getRandomNumber() > 31000) {
			_flagsTable[69] = 1;
			startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
		} else if (isSoundPlaying(1)) {
			stopSound(1);
		}
	}
	_flagsTable[8] = 2;
	if (_spritesTable[0].state == 1) {
		if (_spritesTable[0].animationFrame > 10 && _spritesTable[0].animationFrame < 20) {
			if (!isSoundPlaying(0)) {
				startSound(_locationSoundsTable[0].offset, 0, _locationSoundsTable[0].volume);
			}
		}
	}
	if (isSoundPlaying(0)) {
		if (_spritesTable[0].animationFrame > 10 && _spritesTable[0].animationFrame < 20) {
			stopSound(0);
		}
	}
}

void TuckerEngine::updateSprite_locationNum14(int i) {
	int state = 2;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = (getRandomNumber() < 12000) ? 1 : 3;
	} else if (getRandomNumber() < 26000) {
		_spritesTable[i].updateDelay = 5;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum14() {
	if (_yPosCurrent >= 127)
		return;

	if (!isSoundPlaying(0)) {
		int num = -1;
		const int i = getRandomNumber();
		if (i > 32000) {
			num = 0;
		} else if (i > 31800) {
			num = 3;
		} else if (i > 31600) {
			num = 4;
		} else if (i > 31400) {
			num = 7;
		}
		if (num != -1) {
			startSound(_locationSoundsTable[num].offset, num, _locationSoundsTable[num].volume);
		}
	}
	_locationHeightTable[14] = (_xPosCurrent < 100) ? 0 : 60;
	if (_updateLocationFadePaletteCounter == 0) {
		for (int i = 0; i < 10; ++i) {
			_updateLocation14ObjNum[i] = 0;
		}
	}
	++_updateLocationFadePaletteCounter;
	if (_updateLocationFadePaletteCounter > 4) {
		_updateLocationFadePaletteCounter = 1;
	}
	for (int i = 0; i < 10; ++i) {
		execData3PreUpdate_locationNum14Helper1(i);
		if (_updateLocationFadePaletteCounter == 1 && _updateLocation14ObjNum[i] > 0) {
			execData3PreUpdate_locationNum14Helper2(i);
		}
		const int num = _updateLocation14ObjNum[i];
		if (num > 0) {
			const int w = _dataTable[num].xSize;
			const int h = _dataTable[num].ySize;
			const int x = _updateLocationXPosTable2[i] - w / 2;
			const int y = _updateLocationYPosTable2[i] / 16 - h / 2;
			Graphics::decodeRLE_248(_locationBackgroundGfxBuf + y * 640 + x, _data3GfxBuf + _dataTable[num].sourceOffset, w, h, 0, 0, false);
			addDirtyRect(x, y, w, h);
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum14Helper1(int i) {
	const int y = 117 * 16;
	if (_updateLocation14ObjNum[i] == 0) {
		if (getRandomNumber() <= 30000) {
			return;
		}
		_updateLocationXPosTable2[i] = 155;
		_updateLocationYPosTable2[i] = y;
		_updateLocation14Step[i] = -55 - getRandomNumber() / 512;
		_updateLocation14ObjNum[i] = 231;
		_updateLocation14Delay[i] = 16 + getRandomNumber() / 2048;
	}
	_updateLocation14Step[i] += 4;
	_updateLocationYPosTable2[i] += _updateLocation14Step[i];
	if (_updateLocationYPosTable2[i] > y) {
		_updateLocationYPosTable2[i] = y;
		_updateLocation14Step[i] = (-(getRandomNumber() + 32000) * _updateLocation14Step[i]) / 65536;
	}
	--_updateLocationXPosTable2[i];
	if (_updateLocationXPosTable2[i] == 0) {
		_updateLocation14ObjNum[i] = 0;
	}
}

void TuckerEngine::execData3PreUpdate_locationNum14Helper2(int i) {
	--_updateLocation14Delay[i];
	if (_updateLocation14Delay[i] == 0) {
		_updateLocation14ObjNum[i] = 236;
	} else {
		++_updateLocation14ObjNum[i];
		if (_updateLocation14ObjNum[i] > 237) {
			_updateLocation14ObjNum[i] = 0;
		} else if (_updateLocation14ObjNum[i] == 235) {
			_updateLocation14ObjNum[i] = 231;
		}
	}
}

void TuckerEngine::execData3PostUpdate_locationNum14() {
	if (_yPosCurrent < 127) {
		execData3PreUpdate_locationNum14();
	}
}

void TuckerEngine::updateSprite_locationNum15_0(int i) {
	_spritesTable[i].state = -1;
}

void TuckerEngine::updateSprite_locationNum15_1(int i) {
	int state;
	int r = getRandomNumber();
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 6;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (r < 26000) {
			state = 5;
		} else if (r < 29000) {
			state = 2;
			_spritesTable[i].prevAnimationFrame = 1;
		} else {
			state = 4;
			_spritesTable[4].counter = 1;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum15_2(int i) {
	_spritesTable[i].state = 3;
	if (_spritesTable[4].counter > 0) {
		++_spritesTable[4].counter;
		if (_spritesTable[4].counter > 6) {
			_spritesTable[4].counter = 0;
		} else {
			_spritesTable[i].updateDelay = 2;
		}
	} else {
		_spritesTable[i].updateDelay = 2;
	}
}

void TuckerEngine::execData3PreUpdate_locationNum15() {
	++_updateLocationFadePaletteCounter;
	if (_updateLocationFadePaletteCounter > 500) {
		if (!isSoundPlaying(1) && getRandomNumber() > 31000) {
			const int i = getRandomNumber() / 4714;
			startSound(_locationSoundsTable[i].offset, i, _locationSoundsTable[i].volume);
			_updateLocationFadePaletteCounter = 0;
		}
	}
}

void TuckerEngine::updateSprite_locationNum16_0(int i) {
	int state = 0;
	int r = getRandomNumber();
	if (_flagsTable[210] > 0) {
		state = -1;
	} else if (_flagsTable[82] == 1) {
		state = 2;
		_flagsTable[82] = 2;
	} else if (_flagsTable[82] == 2) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		if (_spritesTable[0].needUpdate == 0) {
			_spritesTable[0].needUpdate = 1;
			state = 1;
		} else {
			state = 1;
			_spritesTable[0].animationFrame = 2;
			_updateSpriteFlag1 = 1;
		}
	} else if (_csDataHandled) {
		_spritesTable[0].needUpdate = 0;
		state = 4;
		_spritesTable[0].updateDelay = 5;
	} else if (r < 30000) {
		state = 4;
		_spritesTable[0].updateDelay = 5;
	} else if (r < 31000) {
		state = 4;
		if (_xPosCurrent < 300) {
			_miscSoundFxDelayCounter[0] = 2;
			_miscSoundFxNum[0] = 9;
		}
	} else if (r < 32000) {
		state = 5;
	} else {
		state = 6;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum16_1(int i) {
	int state;
	if (_flagsTable[61] == 0) {
		state = -1;
		if (isSoundPlaying(0)) {
			stopSound(0);
		} else if (isSoundPlaying(1)) {
			stopSound(1);
		}
	} else if (_flagsTable[60] == 0) {
		state = 10;
		if (isSoundPlaying(1)) {
			_locationSoundsTable[1].type = 2;
			startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
		}
	} else {
		state = 9;
		if (isSoundPlaying(0)) {
			_locationSoundsTable[0].type = 2;
			startSound(_locationSoundsTable[0].offset, 0, _locationSoundsTable[0].volume);
		}
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum16_2(int i) {
	int state = -1;
	if (_flagsTable[78] == 0) {
		if (_flagsTable[60] == 1 && _flagsTable[61] == 1) {
			_flagsTable[78] = 1;
			startSound(_locationSoundsTable[5].offset, 5, _locationSoundsTable[5].volume);
			state = 7;
		}
	} else if (_flagsTable[78] == 1) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 8;
		} else {
			if (getRandomNumber() > 32000) {
				state = 7;
			} else if (getRandomNumber() > 10000) {
				state = 13;
				_spritesTable[i].updateDelay = 5;
			} else {
				state = 13;
			}
			_spritesTable[i].needUpdate = 1;
		}
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum16() {
	_locationHeightTable[16] = (_xPosCurrent < 320) ? 60 : 0;
}

void TuckerEngine::updateSprite_locationNum17() {
	int state;
	if (_flagsTable[76] > 0) {
		state = -1;
		_disableCharactersPath = false;
	} else if (_flagsTable[82] == 2) {
		_flagsTable[82] = 3;
		_disableCharactersPath = true;
		_spritesTable[0].gfxBackgroundOffset = 100;
		state = 1;
		_spritesTable[0].backgroundOffset = -1;
	} else if (_spritesTable[0].gfxBackgroundOffset < -160) {
		state = -1;
	} else {
		state = 1;
		_spritesTable[0].yMaxBackground = 0;
		_spritesTable[0].colorType = 1;
	}
	_spritesTable[0].state = state;
}

void TuckerEngine::updateSprite_locationNum18() {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		_spritesTable[0].needUpdate = 1;
		state = 2;
	} else {
		_spritesTable[0].needUpdate = 0;
		state = 1;
		const int r = getRandomNumber();
		if (r > 31000) {
			state = 3;
		} else if (r > 30000) {
			state = 4;
		}
	}
	_spritesTable[0].gfxBackgroundOffset = 0;
	_spritesTable[0].backgroundOffset = 0;
	_spritesTable[0].state = state;
}

void TuckerEngine::updateSprite_locationNum19_0(int i) {
	int state;
	if (_flagsTable[206] == 1) {
		state = 7;
		_spritesTable[i].needUpdate = 0;
		_flagsTable[206] = 0;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		++_spritesTable[i].counter;
		_spritesTable[i].needUpdate = 1;
		if (_spritesTable[i].counter > 10) {
			_spritesTable[i].counter = 0;
			state = 2;
		} else {
			state = 1;
		}
	} else {
		_spritesTable[i].needUpdate = 0;
		if (_spritesTable[i].counter > 10) {
			_spritesTable[i].counter = 0;
		}
		if (_spritesTable[i].counter == 2) {
			state = 4;
			_spritesTable[i].prevAnimationFrame = 1;
		} else if (_spritesTable[i].counter == 5) {
			state = 5;
			_spritesTable[i].prevAnimationFrame = 1;
		} else {
			state = 5;
			_spritesTable[i].updateDelay = 6;
		}
	}
	_spritesTable[i].state = state;
	_spritesTable[i].prevAnimationFrame = 1;
}

void TuckerEngine::updateSprite_locationNum19_1(int i) {
	_spritesTable[i].state = 9;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum19_2(int i) {
	_spritesTable[i].gfxBackgroundOffset = 320;
	_spritesTable[i].state = -1;
}

void TuckerEngine::updateSprite_locationNum19_3(int i) {
	setCharacterAnimation(0, i);
}

void TuckerEngine::execData3PreUpdate_locationNum19() {
	_locationHeightTable[19] = (_locationMaskType == 0) ? 0 : 60;
}

void TuckerEngine::updateSprite_locationNum21() {
	int state;
	if (_flagsTable[58] == 2) {
		state = 5;
		_flagsTable[58] = 3;
		setVolumeSound(2, 0);
	} else if (_flagsTable[58] == 3) {
		state = 6;
		_flagsTable[58] = 4;
		_locationSoundsTable[0].volume = 60;
		_locationSoundsTable[5].volume = 60;
	} else if (_flagsTable[58] == 4) {
		state = 7;
		_locationSoundsTable[4].volume = 60;
	} else if (_flagsTable[59] == 4) {
		_spritesTable[0].needUpdate = 1;
		_flagsTable[59] = 2;
		state = 2;
		setVolumeSound(2, 0);
	} else if (_flagsTable[59] == 3) {
		_flagsTable[59] = 0;
		state = 4;
		setVolumeSound(2, _locationSoundsTable[2].volume);
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		_spritesTable[0].needUpdate = 1;
		state = 3;
	} else if (_flagsTable[59] == 2) {
		_spritesTable[0].needUpdate = 0;
		state = 3;
		_spritesTable[0].updateDelay = 5;
	} else if (_flagsTable[15] == 1) {
		state = 3;
		_spritesTable[0].updateDelay = 5;
	} else {
		_spritesTable[0].needUpdate = 0;
		state = 1;
	}
	_spritesTable[0].state = state;
	_spritesTable[0].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum21() {
	if (_xPosCurrent > 460 && _flagsTable[58] == 0 && _nextAction == 0) {
		_currentActionVerb = 0;
		_pendingActionDelay = 0;
		_flagsTable[59] = 1;
		_nextAction = 2;
		_csDataLoaded = 0;
	}
	if (_flagsTable[58] > 0 && !isSoundPlaying(0)) {
		const int r = getRandomNumber();
		if (r > 10000) {
			int num;
			if (r > 25000) {
				num = 0;
			} else if (r > 17000) {
				num = 4;
			} else {
				num = 5;
			}
			startSound(_locationSoundsTable[num].offset, num, _locationSoundsTable[num].volume);
		}
	}
	if (_spritesTable[0].state == 6) {
		if (_spritesTable[0].animationFrame < 18) {
			_spritesTable[0].gfxBackgroundOffset = 320 + _spritesTable[0].animationFrame * 638;
		} else {
			_spritesTable[0].gfxBackgroundOffset = 11840 - _spritesTable[0].animationFrame * 2;
		}
	} else {
		_spritesTable[0].gfxBackgroundOffset = 320;
	}
	if (_inventoryItemsState[19] > 0) {
		_flagsTable[43] = 1;
	}
}

void TuckerEngine::execData3PostUpdate_locationNum21() {
	if (_flagsTable[58] > 3) {
		drawSprite(0);
	}
}

void TuckerEngine::execData3PreUpdate_locationNum22() {
	if (_flagsTable[53] > 1 && _flagsTable[53] != 4) {
		if (_inventoryItemsState[5] > 0 && _inventoryItemsState[20] > 0 && _inventoryItemsState[16] > 0 && _inventoryItemsState[27] > 0) {
			_flagsTable[53] = 3;
		} else if (_inventoryItemsState[5] > 0 || _inventoryItemsState[20] > 0 || _inventoryItemsState[16] > 0 || _inventoryItemsState[27] > 0) {
			_flagsTable[53] = 5;
		}
	}
	if (_flagsTable[210] < 2 && !_csDataHandled && _flagsTable[54] == 1) {
		_nextAction = 25;
		_csDataLoaded = 0;
		_flagsTable[210] = 2;
	}
}

void TuckerEngine::updateSprite_locationNum22() {
	if (_flagsTable[207] == 1) {
		_spritesTable[0].state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		_spritesTable[0].needUpdate = 1;
		_spritesTable[0].state = 2;
	} else {
		_spritesTable[0].needUpdate = 0;
		_spritesTable[0].state = 1;
	}
}

void TuckerEngine::updateSprite_locationNum23_0(int i) {
	int state;
	if (_flagsTable[210] == 3) {
		state = 12;
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum23_1(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 14;
	} else if (getRandomNumber() < 30000) {
		_spritesTable[i].needUpdate = 0;
		state = 25;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 13;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum23_2(int i) {
	int state = 0;
	if (_flagsTable[210] == 0) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 6;
		} else {
			_spritesTable[i].needUpdate = 0;
			state = (getRandomNumber() < 25000) ? 4 : 5;
		}
	} else if (_flagsTable[210] == 1) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			if (_flagsTable[211] == 10) {
				state = 34;
			} else if (_flagsTable[211] == 0) {
				state = 30;
				_flagsTable[211] = 1;
			} else {
				state = 31;
			}
		} else {
			_spritesTable[i].needUpdate = 0;
			if (_flagsTable[211] == 1) {
				state = 32;
				_flagsTable[211] = 0;
			} else {
				state = (getRandomNumber() < 25000) ? 10 : 11;
			}
		}
	} else if (_flagsTable[210] == 2) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 33;
		} else {
			_spritesTable[i].needUpdate = 0;
			if (_flagsTable[212] == 0) {
				state = 3;
				_flagsTable[212] = 1;
			} else if (_flagsTable[212] == 2) {
				state = 29;
				_flagsTable[212] = 3;
			} else {
				state = (getRandomNumber() < 25000) ? 22 : 23;
			}
		}
	} else {
		state = 24;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum23_3(int i) {
	int state;
	if (_flagsTable[210] == 0 || _flagsTable[210] == 2) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 8;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 9;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum24_0(int i) {
	int state;
	if (_flagsTable[103] == 4) {
		_flagsTable[103] = 3;
		state = 5;
	} else if (_flagsTable[103] == 0) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 2;
		} else {
			_spritesTable[i].needUpdate = 1;
			state = 1;
			if (getRandomNumber() < 30000) {
				_spritesTable[i].updateDelay = 5;
			}
		}
	} else if (_flagsTable[103] == 1 || _flagsTable[103] == 3) {
		state = -1;
	} else {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 4;
		} else {
			if (getRandomNumber() < 30000) {
				_spritesTable[i].needUpdate = 0;
				state = 6;
				_spritesTable[i].updateDelay = 5;
			} else {
				_spritesTable[i].needUpdate = 0;
				state = 6;
			}
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum24_1(int i) {
	if (_flagsTable[103] > 1) {
		_spritesTable[i].state = 3;
	} else {
		_spritesTable[i].state = -1;
	}
}

void TuckerEngine::updateSprite_locationNum24_2(int i) {
	int state;
	if (_flagsTable[214] > 1) {
		state = -1;
	} else if (_flagsTable[214] == 1) {
		_spritesTable[i].needUpdate = 0;
		_flagsTable[214] = 2;
		state = 9;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		if (_flagsTable[213] == 1) {
			state = 10;
		} else if (_flagsTable[213] == 2) {
			state = 14;
		} else {
			state = 8;
		}
	} else {
		_spritesTable[i].needUpdate = 1;
		state = 7;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum24_3(int i) {
	int state;
	if (_flagsTable[239] == 1) {
		state = -1;
	} else if (_flagsTable[214] == 2) {
		_flagsTable[239] = 1;
		state = 13;
	} else if (getRandomNumber() > 32000) {
		state = 12;
	} else {
		state = 11;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum24() {
	_characterPrevBackFrontFacing = 0;
	if (_flagsTable[112] == 0) {
		_yPosCurrent = 132;
		_xPosCurrent = 112;
	} else if (_inventoryItemsState[2] == 1 && _inventoryItemsState[11] == 1 && _flagsTable[156] == 1 && _flagsTable[145] == 3) {
		_flagsTable[156] = 2;
		_nextAction = 61;
		_csDataLoaded = 0;
	}
	if (_flagsTable[103] > 0 && (_inventoryItemsState[2] > 0 || _inventoryItemsState[11] > 0 || _flagsTable[156] > 0 || _flagsTable[145] == 3) && _flagsTable[217] == 0) {
		_flagsTable[217] = 1;
	}
	_locationHeightTable[24] = (_yPosCurrent < 125) ? 60 : 0;
}

void TuckerEngine::execData3PostUpdate_locationNum24() {
	if (_yPosCurrent < 132) {
		drawSprite(1);
		drawSprite(0);
	}
}

void TuckerEngine::execData3PreUpdate_locationNum25() {
	_backgroundSprOffset = _xPosCurrent - 160;
}

void TuckerEngine::updateSprite_locationNum26_0(int i) {
	if (_flagsTable[125] > 0 && _flagsTable[125] < 300) {
		_flagsTable[126] = 1;
	}
	_spritesTable[i].gfxBackgroundOffset = _flagsTable[125];
	_spritesTable[i].state = 1;
	_spritesTable[i].colorType = 99;
}

void TuckerEngine::updateSprite_locationNum26_1(int i) {
	int state;
	if (_flagsTable[125] == 0) {
		state = -1;
	} else if (_flagsTable[125] > 299) {
		state = 2;
		_spritesTable[i].updateDelay = 5;
	} else {
		state = 2;
	}
	_spritesTable[i].gfxBackgroundOffset = _flagsTable[125];
	_spritesTable[i].colorType = 1;
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum26() {
	_characterPrevBackFrontFacing = 1;
	_spritesTable[1].gfxBackgroundOffset = _flagsTable[125];
	_spritesTable[0].gfxBackgroundOffset = _flagsTable[125];
	if (_flagsTable[125] > 0 && _flagsTable[125] < 300) {
		if (!isSoundPlaying(5)) {
			startSound(_locationSoundsTable[5].offset, 5, _locationSoundsTable[5].volume);
			startSound(_locationSoundsTable[6].offset, 6, _locationSoundsTable[6].volume);
		}
		++_flagsTable[125];
		_flagsTable[126] = 1;
	} else {
		if (isSoundPlaying(5)) {
			stopSound(5);
			stopSound(6);
		}
	}
	if (_panelLockedFlag == 0) {
		if (_xPosCurrent > 207 && _xPosCurrent < 256) {
			_objectKeysLocationTable[26] = 0;
			_objectKeysPosXTable[26] = 260;
		} else {
			_objectKeysLocationTable[26] = 1;
		}
	}
}

void TuckerEngine::updateSprite_locationNum27(int i) {
	int state;
	if (_flagsTable[155] < 3 || _flagsTable[155] == 5) {
		state = -1;
	} else if (_flagsTable[155] == 3) {
		state = 1;
		_flagsTable[155] = 4;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 2;
	} else if (getRandomNumber() < 30000) {
		_spritesTable[i].needUpdate = 0;
		state = 3;
		_spritesTable[i].updateDelay = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 3;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum27() {
	_characterPrevBackFrontFacing = 0;
}

void TuckerEngine::execData3PostUpdate_locationNum27() {
	if (_flagsTable[155] == 4 && _yPosCurrent < 125) {
		drawSprite(0);
	}
}

void TuckerEngine::updateSprite_locationNum28_0(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		state = 4;
		_spritesTable[i].needUpdate = 1;
	} else {
		_spritesTable[i].needUpdate = 0;
		++_spritesTable[i].counter;
		if (_spritesTable[i].counter > 30) {
			_spritesTable[i].counter = 0;
		}
		if (_flagsTable[86] == 1) {
			if (_spritesTable[i].counter == 16 || _spritesTable[i].counter == 27) {
				state = 3;
			} else {
				state = 3;
				_spritesTable[i].updateDelay = 5;
			}
		} else {
			if (_spritesTable[i].counter == 5 || _spritesTable[i].counter == 11) {
				state = 5;
			} else if (_spritesTable[i].counter == 16 || _spritesTable[i].counter == 27) {
				state = 6;
			} else {
				state = 6;
				_spritesTable[i].updateDelay = 5;
			}
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum28_1(int i) {
	_spritesTable[i].state = (_flagsTable[86] == 1) ? 1 : -1;
}

void TuckerEngine::updateSprite_locationNum28_2(int i) {
	int state;
	if (_flagsTable[155] == 1) {
		state = 8;
		_flagsTable[155] = 2;
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum28() {
	if (_flagsTable[86] == 0 && _xPosCurrent > 265 && _nextAction == 0) {
		_panelLockedFlag = 0;
		_nextAction = 21;
		_csDataLoaded = 0;
		_pendingActionDelay = 0;
		_pendingActionIndex = 0;
		_currentActionVerb = 0;
	}
}

void TuckerEngine::execData3PostUpdate_locationNum28() {
	if (_yPosCurrent < 135) {
		drawSprite(1);
	}
}

void TuckerEngine::updateSprite_locationNum29_0(int i) {
	_spritesTable[i].state = (getRandomNumber() < 32000) ? -1 : 2;
}

void TuckerEngine::updateSprite_locationNum29_1(int i) {
	int state = -1;
	if (getRandomNumber() >= 32000) {
		state = 1;
		_spritesTable[i].gfxBackgroundOffset = 320;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum29_2(int i) {
	_spritesTable[i].state = 3;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum29() {
	static const uint8 r[] = { 0x0D, 0x0E, 0x0E, 0x0E, 0x0D, 0x0E, 0x0D, 0x0E, 0x0E, 0x0E, 0x0D };
	static const uint8 g[] = { 0x0E, 0x14, 0x1B, 0x14, 0x0E, 0x0D, 0x0E, 0x14, 0x1B, 0x14, 0x0E };
	static const uint8 b[] = { 0x2C, 0x34, 0x3B, 0x34, 0x2C, 0x24, 0x2C, 0x34, 0x3B, 0x34, 0x2C };
	if (_fadePaletteCounter == 16) {
		++_updateLocationFadePaletteCounter;
		if (_updateLocationFadePaletteCounter > 10) {
			_updateLocationFadePaletteCounter = 0;
		}
		const int d = _updateLocationFadePaletteCounter / 2;
		uint8 scrollPal[5 * 3];
		for (int i = 0; i < 5; ++i) {
			scrollPal[i * 3]     = r[i + d];
			scrollPal[i * 3 + 1] = g[i + d];
			scrollPal[i * 3 + 2] = b[i + d];
		}
		_system->getPaletteManager()->setPalette(scrollPal, 118, 5);
		if (_flagsTable[143] == 1) {
			_locationObjectsTable[2].xPos = 999;
			_locationObjectsTable[3].xPos = 187;
		} else {
			_locationObjectsTable[2].xPos = 187;
			_locationObjectsTable[3].xPos = 999;
		}
	}
}

void TuckerEngine::updateSprite_locationNum30_34(int i) {
	_spritesTable[i].state = i + 1;
}

void TuckerEngine::execData3PreUpdate_locationNum30() {
	if (!isSoundPlaying(1) && getRandomNumber() > 32500) {
		startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
	}
}

void TuckerEngine::execData3PreUpdate_locationNum31() {
	if (getRandomNumber() > 32000 && _flagsTable[110] == 0) {
		_flagsTable[110] = 1;
	}
	if (getRandomNumber() > 31000 && _flagsTable[111] == 0) {
		_flagsTable[111] = 1;
	}
	if (_xPosCurrent < 112 && _flagsTable[104] == 0) {
		_flagsTable[104] = 1;
	}
}

void TuckerEngine::execData3PreUpdate_locationNum32() {
	if (_spritesTable[0].state == 12 && _spritesTable[0].animationFrame < 22) {
		_flagsTable[113] = 1;
	} else {
		_flagsTable[113] = 0;
	}
}

void TuckerEngine::execData3PostUpdate_locationNum32() {
	if (_yPosCurrent < 120) {
		drawSprite(0);
	}
}

void TuckerEngine::updateSprite_locationNum31_0(int i) {
	_spritesTable[i].state = (getRandomNumber() < 32000) ? 3 : 1;
}

void TuckerEngine::updateSprite_locationNum31_1(int i) {
	_spritesTable[i].state = (_flagsTable[86] == 1) ? 2 : -1;
}

void TuckerEngine::updateSprite_locationNum32_0(int i) {
	static const uint8 stateTable[] = { 12, 1, 11, 1, 11, 2, 1, 5, 5, 11, 1, 5, 5, 5 };
	++_spritesTable[i].counter;
	if (_flagsTable[123] == 2) {
		_flagsTable[123] = 0;
	}
	int state;
	if (_flagsTable[222] == 1) {
		state = 19;
		_flagsTable[222] = 2;
	} else if (_flagsTable[222] == 2) {
		state = 19;
		_spritesTable[i].animationFrame = 23;
		_updateSpriteFlag1 = 1;
	} else if (_flagsTable[123] == 1) {
		state = 17;
		_flagsTable[123] = 2;
	} else if (_flagsTable[222] == 3) {
		state = 18;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 4;
	} else if (_csDataHandled) {
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (_spritesTable[i].counter > 13) {
			_spritesTable[i].counter = 0;
		}
		state = stateTable[_spritesTable[i].counter];
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum33_0(int i) {
	int state = 5;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 7;
	} else if (_flagsTable[87] == 1) {
		state = 8;
	} else if (_flagsTable[222] == 5) {
		state = 4;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = (getRandomNumber() < 30000) ? 5 : 6;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum33_1(int i) {
	int state;
	if (_flagsTable[87] == 1) {
		state = 3;
	} else if (_flagsTable[222] == 5) {
		state = 2;
	} else {
		state = 1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum33_2(int i) {
	int state;
	if (_flagsTable[87] == 1) {
		state = 11;
	} else if (_flagsTable[222] == 5) {
		state = 10;
	} else {
		state = 9;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum33() {
	if (_xPosCurrent < 90) {
		if (_flagsTable[105] == 0) {
			_flagsTable[105] = 3;
			startSound(_locationSoundsTable[5].offset, 5, _locationSoundsTable[5].volume);
		}
	} else {
		if (_flagsTable[105] == 1) {
			_flagsTable[105] = 2;
			startSound(_locationSoundsTable[5].offset, 5, _locationSoundsTable[5].volume);
		}
	}
	if (_xPosCurrent > 230) {
		if (_flagsTable[106] == 0) {
			_flagsTable[106] = 3;
			startSound(_locationSoundsTable[5].offset, 5, _locationSoundsTable[5].volume);
		}
	} else {
		if (_flagsTable[106] == 1) {
			_flagsTable[106] = 2;
			startSound(_locationSoundsTable[5].offset, 5, _locationSoundsTable[5].volume);
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum34() {
	if (_flagsTable[143] == 1) {
		_locationObjectsTable[0].xPos = 0;
	}
}

void TuckerEngine::execData3PreUpdate_locationNum35() {
	if (_flagsTable[250] > 0) {
		_currentFxVolume = 0;
	}
}

void TuckerEngine::updateSprite_locationNum36(int i) {
	_spritesTable[i].state = (getRandomNumber() < 32000) ? 1 : 2;
}

void TuckerEngine::execData3PreUpdate_locationNum36() {
	if (_execData3Counter == 0) {
		_execData3Counter = 1;
		_flagsTable[107] = 1;
	} else if (_flagsTable[107] == 0 && _locationMaskType == 1 && _execData3Counter == 1) {
		_execData3Counter = 2;
		_flagsTable[107] = 1;
	}
}

void TuckerEngine::updateSprite_locationNum37(int i) {
	int j = i + 1;
	int offset = 200 - i * 45;
	++_spritesTable[i].counter;
	if (_spritesTable[i].counter > offset) {
		_spritesTable[i].state = j;
		_spritesTable[i].counter = 0;
	} else {
		_spritesTable[i].state = -1;
	}
}

void TuckerEngine::execData3PreUpdate_locationNum38() {
	if (_flagsTable[117] == 0 && _flagsTable[116] == 1) {
		_flagsTable[116] = 0;
	}
}

void TuckerEngine::updateSprite_locationNum41(int i) {
	int state;
	if (_flagsTable[223] > 1) {
		state = -1;
	} else if (_flagsTable[223] == 1) {
		state = 1;
		_flagsTable[158] = 2;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 3;
	} else if (getRandomNumber() < 30000) {
		_spritesTable[i].needUpdate = 0;
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 4;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum41() {
	if (_backgroundSpriteCurrentAnimation == 4) {
		if (_backgroundSpriteCurrentFrame == 8) {
			_flagsTable[77] = 2;
		} else if (_backgroundSpriteCurrentFrame == 7) {
			_flagsTable[77] = 0;
		}
		if (_changeBackgroundSprite == 0) {
			if (_backgroundSpriteCurrentFrame == 16 && !isSoundPlaying(4)) {
				_locationSoundsTable[4].type = 2;
				startSound(_locationSoundsTable[4].offset, 4, _locationSoundsTable[4].volume);
			} else {
				if (_backgroundSpriteCurrentFrame == 28) {
					stopSound(4);
				}
			}
		} else {
			if (_backgroundSpriteCurrentFrame == 28 && !isSoundPlaying(4)) {
				_locationSoundsTable[4].type = 2;
				startSound(_locationSoundsTable[4].offset, 4, _locationSoundsTable[4].volume);
			} else {
				if (_backgroundSpriteCurrentFrame == 18) {
					stopSound(4);
				}
			}
		}
	}
	if (_panelLockedFlag == 1 && _yPosCurrent > 130 && _selectedObject.yPos > 135 && _nextAction == 0 && _flagsTable[223] == 0) {
		_panelLockedFlag = 0;
		_csDataLoaded = false;
		_nextLocationNum = 0;
		_selectedObject.locationObject_locationNum = 0;
		_locationMaskType = 0;
		_nextAction = _flagsTable[163] + 32;
		++_flagsTable[163];
		if (_flagsTable[163] > 2) {
			_flagsTable[163] = 0;
		}
		if (_flagsTable[223] > 0 && _mapSequenceFlagsLocationTable[40] == 0) {
			for (int i = 41; i < 53; ++i) {
				_mapSequenceFlagsLocationTable[i - 1] = 1;
			}
		}
		if (_flagsTable[77] == 0) {
			_locationObjectsTable[3].xPos = 230;
		} else {
			_locationObjectsTable[3].xPos = 930;
		}
	}
}

void TuckerEngine::updateSprite_locationNum42(int i) {
	int state;
	if (_flagsTable[223] == 0 || _flagsTable[223] > 3) {
		state = -1;
	} else if (_flagsTable[223] == 1) {
		state = 1;
		_flagsTable[223] = 2;
	} else if (_flagsTable[223] == 2) {
		state = 5;
		_flagsTable[223] = 3;
	} else if (_flagsTable[223] == 3) {
		state = 5;
		_spritesTable[i].updateDelay = 5;
		_spritesTable[i].state = _spritesTable[i].firstFrame - 1; // FIXME: bug, fxNum ?
		_updateSpriteFlag1 = 1;
	} else {
		state = 2;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum42() {
	_flagsTable[167] = 1;
}

void TuckerEngine::updateSprite_locationNum43_2(int i) {
	int state;
	if (_flagsTable[237] > 0) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 5;
	} else if (_csDataHandled || _spritesTable[i].counter == 2) {
		_spritesTable[i].needUpdate = 0;
		state = 6;
	} else if (_spritesTable[i].counter == 0) {
		state = 3;
	} else {
		state = 4;
		_spritesTable[i].counter = 2;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum43_3(int i) {
	int state;
	if (_flagsTable[236] < 4) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		state = 7;
		_spritesTable[i].needUpdate = 1;
	} else {
		state = 8;
		_spritesTable[i].needUpdate = 0;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum43_4(int i) {
	int state;
	if (_flagsTable[236] < 4) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		state = 9;
		_spritesTable[i].needUpdate = 1;
	} else {
		state = 10;
		_spritesTable[i].needUpdate = 0;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum43_5(int i) {
	_spritesTable[i].state = (_flagsTable[236] < 4) ? -1 : 11;
}

void TuckerEngine::updateSprite_locationNum43_6(int i) {
	_spritesTable[i].state = (_flagsTable[236] < 4) ? -1 : 12;
}

void TuckerEngine::execData3PreUpdate_locationNum43() {
	if (_panelLockedFlag == 1 && _xPosCurrent > 67 && _selectedObject.xPos > 68 && _locationMaskType == 0) {
		_panelLockedFlag = 0;
		_csDataLoaded = 0;
		_nextAction = 5;
	}
	if (_xPosCurrent > 55 && _spritesTable[2].counter == 0) {
		_spritesTable[2].counter = 1;
	}
}

void TuckerEngine::execData3PreUpdate_locationNum44() {
	if (_backgroundSpriteCurrentAnimation == 1) {
		if (!isSoundPlaying(3) && _backgroundSpriteCurrentFrame == 1) {
			_locationSoundsTable[3].type = 2;
			startSound(_locationSoundsTable[3].offset, 3, _locationSoundsTable[3].volume);
		}
		if (_backgroundSpriteCurrentFrame == 21) {
			_flagsTable[77] = 1;
		} else if (_backgroundSpriteCurrentFrame == 20) {
			_flagsTable[77] = 2;
			stopSound(3);
		}
	} else if (_backgroundSpriteCurrentAnimation == 4) {
		if (_backgroundSpriteCurrentFrame == 20 && !isSoundPlaying(3)) {
			startSound(_locationSoundsTable[3].offset, 3, _locationSoundsTable[3].volume);
		}
	}
}

void TuckerEngine::updateSprite_locationNum48(int i) {
	int state;
	if (_flagsTable[160] == 0 || _flagsTable[160] > 2 || _flagsTable[207] > 0) {
		state = -1;
	} else if (_flagsTable[160] == 2) {
		_flagsTable[160] = 3;
		state = 3;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 2;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 2;
		_spritesTable[i].updateDelay = 5;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum49(int i) {
	if (_flagsTable[136] == 1 && _flagsTable[207] == 0) {
		_flagsTable[136] = 2;
	}
	if (_flagsTable[185] == 0 && _yPosCurrent < 125) {
		_flagsTable[185] = 1;
	} else if (_flagsTable[185] == 2 && (_yPosCurrent > 130 || _flagsTable[236] == 4)) {
		_flagsTable[185] = 3;
	}
	int state;
	if (_flagsTable[185] == 0 || _flagsTable[160] < 3) {
		state = -1;
	} else if (_flagsTable[185] == 1) {
		_flagsTable[185] = 2;
		state = 1;
	} else if (_flagsTable[185] == 3) {
		_flagsTable[185] = 0;
		state = 4;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 2;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (getRandomNumber() < 30000 || _backgroundSpriteCurrentAnimation <= -1) {
			state = 3;
			_spritesTable[i].updateDelay = 1;
		} else if (getRandomNumber() < 16000) {
			state = 5;
		} else {
			state = 6;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum49() {
	_flagsTable[132] = 0;
	if (_execData3Counter == 0) {
		_execData3Counter = 1;
		if (_flagsTable[181] == 2) {
			_flagsTable[181] = 3;
		}
	}
	static const int items[] = { 15, 44, 25, 27, 19, 21, 24, 13, 20, 29, 35, 23, 3 };
	for (int i = 0; i < 13; ++i) {
		if (_inventoryItemsState[items[i]] == 1) {
			_flagsTable[168 + i] = 1;
		}
	}
	int counter = 0;
	for (int i = 168; i < 181; ++i) {
		if (_flagsTable[i] == 2) {
			++counter;
		}
	}
	if (_nextAction == 0) {
		if (counter == 2 && _flagsTable[236] == 0) {
			_nextAction = 56;
			_csDataLoaded = false;
		} else if (counter == 6 && _flagsTable[236] == 1) {
			_nextAction = 59;
			_csDataLoaded = false;
		} else if (counter == 10 && _flagsTable[236] == 2) {
			_nextAction = 62;
			_csDataLoaded = false;
		} else if (counter == 13 && _flagsTable[236] == 3) {
			_nextAction =  65;
			_csDataLoaded = false;
		}
	}
}

void TuckerEngine::updateSprite_locationNum50(int i) {
	int state;
	if (_flagsTable[240] == 0) {
		state = i + 1;
		if (i == 5) {
			_flagsTable[240] = 1;
		}
	} else {
		_spritesTable[i].animationFrame = _spritesTable[i].firstFrame - 1;
		_spritesTable[i].updateDelay = 5;
		_updateSpriteFlag1 = 1;
		state = i + 1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum51(int i) {
	if (i == 2) {
		_spritesTable[i].state = 1;
	} else if (i == 0) {
		static const int stateTable[] = { 3, 3, 4, 5, 3, 3, 5, 4, 3, 3, 4, 5, 4, 4 };
		++_spritesTable[i].counter;
		if (_spritesTable[i].counter > 13) {
			_spritesTable[i].counter = 0;
		}
		_spritesTable[i].state = stateTable[_spritesTable[i].counter];
	} else {
		i = 1;
		_spritesTable[i].state = 6;
	}

	_spritesTable[i].colorType = 1;
	_spritesTable[i].yMaxBackground = 0;
}


void TuckerEngine::execData3PreUpdate_locationNum52() {
	if (_selectedObject.xPos > 108 && _panelLockedFlag > 0 && _nextAction == 0 && _locationMaskType == 0) {
		_nextAction = 1;
		_csDataLoaded = 0;
	}
}

void TuckerEngine::updateSprite_locationNum53_0(int i) {
	if (_flagsTable[197] == 2) {
		_flagsTable[197] = 3;
	}
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 1;
	} else if (_flagsTable[197] == 1) {
		state = 3;
		_spritesTable[i].needUpdate = 0;
		_flagsTable[197] = 2;
	} else if (_flagsTable[192] == 1) {
		_spritesTable[i].needUpdate = 0;
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 5;
		_spritesTable[i].updateDelay = 5;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum53_1(int i) {
	int state;
	if (_flagsTable[197] == 3) {
		state = 6;
	} else if (_flagsTable[197] == 4) {
		state = 7;
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum53() {
	if (_flagsTable[192] == 0 && _xPosCurrent < 200 && _nextAction == 0) {
		_panelLockedFlag = 0;
		_nextAction = 14;
		_csDataLoaded = 0;
		_pendingActionDelay = 0;
		_pendingActionIndex = 0;
		_currentActionVerb = 0;
	}
}

void TuckerEngine::updateSprite_locationNum54(int i) {
	int state = 3;
	if (_flagsTable[141] == 2) {
		_spritesTable[i].needUpdate = 0;
		setCharacterAnimation(0, i);
		_flagsTable[141] = 1;
		_spritesTable[i].counter = 0;
	} else if (_flagsTable[141] == 1 && _spritesTable[i].counter < 40) {
		setCharacterAnimation(1, i);
		++_spritesTable[i].counter;
	} else if (_flagsTable[141] == 1) {
		setCharacterAnimation(2, i);
		_flagsTable[141] = 3;
	} else {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 2;
			state = (getRandomNumber() < 12000) ? 2 : 4;
		} else if (_flagsTable[141] == 3) {
			state = 3;
			_flagsTable[141] = 0;
			_flagsTable[224] = 1;
			_spritesTable[i].counter = 0;
			if (_panelLockedFlag == 0 && _xPosCurrent > 130 && _inventoryItemsState[17] == 0) {
				_nextAction = 18;
				_csDataLoaded = false;
			}
		} else if (getRandomNumber() < 26000) {
			state = 3;
			_spritesTable[i].needUpdate = 0;
			_spritesTable[i].updateDelay = 5;
		} else {
			state = 3;
			_spritesTable[i].needUpdate = 0;
		}
		if (_inventoryItemsState[17] == 1) {
			_flagsTable[224] = 2;
		}
		_spritesTable[i].state = state;
	}
}

void TuckerEngine::updateSprite_locationNum55(int i) {
	if (_flagsTable[193] > 0 && _flagsTable[193] < 14) {
		setCharacterAnimation(_flagsTable[193] - 1, i);
		_updateSpriteFlag1 = 1;
		if (_flagsTable[193] == 1 || _flagsTable[193] == 3 || _flagsTable[193] == 5 || _flagsTable[193] == 7 || _flagsTable[193] == 11 || _flagsTable[193] == 13) {
			++_flagsTable[193];
		}
	} else {
		_spritesTable[i].state = -1;
		if (_flagsTable[193] == 14) {
			_flagsTable[193] = 15;
		}
	}
}

void TuckerEngine::updateSprite_locationNum56(int i) {
	int state;
	++_spritesTable[i].counter;
	if (_flagsTable[153] == 1) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 2;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum57_0(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		if (getRandomNumber() < 30000) {
			state = 1;
		} else if (getRandomNumber() < 16000) {
			state = 4;
		} else {
			state = 2;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum57_1(int i) {
	int state = 6;
	if (getRandomNumber() < 30000) {
		_spritesTable[i].updateDelay = 5;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum57() {
	if (_flagsTable[137] == 2 && _xPosCurrent < 42 && _yPosCurrent == 135) {
		_flagsTable[137] = 0;
		_backgroundSpriteCurrentAnimation = 2;
		_backgroundSpriteCurrentFrame = 0;
		_backgroundSprOffset = 0;
	}
}

void TuckerEngine::updateSprite_locationNum58(int i) {
	int state;
	if (_flagsTable[190] == 0) {
		state = 1;
	} else if (_flagsTable[190] == 1) {
		state = 2;
		_flagsTable[190] = 2;
	} else {
		state = 3;
		if (_flagsTable[190] == 2) {
			_flagsTable[190] = 3;
		}
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum58() {
	// workaround original game glitch #2872348: do not change position on location change
	if (_nextLocationNum == 0 && _flagsTable[190] < 3 && _xPosCurrent > 310) {
		_xPosCurrent = 310;
		_panelLockedFlag = 0;
	}
	if (_flagsTable[190] > 0 && _locationSoundsTable[0].volume > 0) {
		_locationSoundsTable[0].volume = 0;
		_locationSoundsTable[4].volume = 0;
		_locationSoundsTable[5].volume = 0;
		if (isSoundPlaying(0)) {
			stopSound(0);
		}
	}
}

void TuckerEngine::updateSprite_locationNum59(int i) {
	int state;
	if (_flagsTable[207] == 1) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		state = 3;
		_spritesTable[i].needUpdate = 1;
	} else if (_csDataHandled) {
		_spritesTable[i].needUpdate = 0;
		if (_flagsTable[199] == 0) {
			_flagsTable[199] = 1;
			setCharacterAnimation(0, 0);
			return;
		}
		_spritesTable[i].needUpdate = 0;
		state = 3;
		_spritesTable[i].updateDelay = 5;
	} else if (_flagsTable[199] == 1) {
		_flagsTable[199] = 0;
		_spritesTable[i].needUpdate = 0;
		setCharacterAnimation(1, 0);
		return;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum60_0(int i) {
	int state;
	if (_flagsTable[186] > 0) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 9;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = (getRandomNumber() > 32000) ? 8 : 7;
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum60_1(int i) {
	int state;
	if (_flagsTable[186] == 1) {
		_flagsTable[186] = 2;
		_spritesTable[i].needUpdate = 0;
		state = 2;
	} else if (_flagsTable[186] == 2) {
		state = 6;
		_flagsTable[187] = 1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 1;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = (getRandomNumber() > 32000) ? 5 : 4;
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PostUpdate_locationNum60() {
	drawSprite(0);
}

void TuckerEngine::updateSprite_locationNum61_0(int i) {
	int state;
	const int r = getRandomNumber();
	if (_flagsTable[88] == 1) {
		state = 3;
		_flagsTable[88] = 2;
	} else if (_flagsTable[88] == 2) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 0) {
		if (_spritesTable[i].needUpdate == 0) {
			_spritesTable[i].needUpdate = 1;
			state = 2;
		} else {
			state = 2;
			_spritesTable[0].animationFrame = 2;
			_updateSpriteFlag1 = 1;
		}
	} else {
		_spritesTable[i].needUpdate = 0;
		if (r < 30000) {
			state = 7;
			_spritesTable[i].updateDelay = 5;
		} else if (r < 31000) {
			state = 7;
		} else if (r < 32000) {
			state = 5;
		} else {
			state = 6;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum61_1(int i) {
	int state;
	if (_flagsTable[151] == 1) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 14;
	} else if (_csDataHandled || getRandomNumber() < 29000) {
		_spritesTable[i].needUpdate = 0;
		state = 12;
		_spritesTable[i].updateDelay = 5;
	} else if (getRandomNumber() < 20000) {
		state = 12;
	} else {
		state = 13;
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum61_2(int i) {
	int state;
	const int r = getRandomNumber();
	_flagsTable[150] = 1;
	if (_flagsTable[198] == 2) {
		_flagsTable[150] = 2;
		state = 16;
	} else if (_flagsTable[198] == 1) {
		if (_flagsTable[150] == 2) {
			state = 10;
		} else {
			state = 18;
		}
		_flagsTable[198] = 2;
		_flagsTable[150] = 0;
	} else if (r < 32000) {
		state = 15;
	} else if (r < 32100) {
		state = 1;
		if (_xPosCurrent < 340 || _xPosCurrent > 380) {
			_flagsTable[150] = 0;
			if (_flagsTable[136] == 0) {
				state = 17;
			}
		} else {
			state = 15;
		}
	} else if (r < 32200) {
		state = 4;
	} else if (r < 32300) {
		state = 8;
	} else {
		state = 9;
	}
	_flagsTable[248] = _flagsTable[150];
	if (_flagsTable[136] != 2) {
		_flagsTable[150] = 0;
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum61() {
	if (_inventoryItemsState[36] == 1) {
		_flagsTable[93] = (_inventoryItemsState[6] == 1) ? 3 : 1;
	} else {
		_flagsTable[93] = (_inventoryItemsState[6] == 1) ? 2 : 0;
	}
	_flagsTable[154] = _inventoryItemsState[16];
	if (_inventoryItemsState[16] == 2 && _inventoryItemsState[36] == 2) {
		if (!_csDataHandled && _flagsTable[88] == 0) {
			_flagsTable[88] = 1;
		}
	}
}

void TuckerEngine::updateSprite_locationNum63_0(int i) {
	int state;
	if (_flagsTable[136] > 0 && _flagsTable[132] == 2) {
		state = 12;
	} else if (_flagsTable[132] != 2 || _flagsTable[133] != 1 || _flagsTable[136] > 0) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 6;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum63_1(int i) {
	int state = -1;
	if (_flagsTable[132] == 2 && _flagsTable[133] == 1 && _flagsTable[136] <= 0) {
		if (getRandomNumber() > 30000) {
			state = 5;
		} else if (getRandomNumber() > 30000) {
			state = 8;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum63_2(int i) {
	++_spritesTable[i].counter;
	int state = -1;
	if (_flagsTable[132] == 2 && _flagsTable[133] == 1 && _flagsTable[136] <= 0) {
		if (_flagsTable[226] == 0) {
			state = 9;
			_spritesTable[i].updateDelay = 5;
		} else if (_flagsTable[226] == 1) {
			state = 9;
			_flagsTable[226] = 2;
		} else if (_flagsTable[226] == 2) {
			state = 3;
		} else {
			_flagsTable[226] = 0;
			state = 10;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum63_3(int i) {
	++_spritesTable[i].counter;
	int state = -1;
	if (_flagsTable[132] == 2 && _flagsTable[133] == 1 && _flagsTable[136] <= 0) {
		if (_spritesTable[i].counter > 80) {
			state = 7;
			_spritesTable[i].counter = 0;
		} else if (getRandomNumber() > 32000) {
			state = 2;
		} else if (getRandomNumber() > 32000) {
			state = 4;
		} else if (getRandomNumber() > 28000) {
			state = 8;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum63_4(int i) {
	int state = -1;
	if (_flagsTable[132] == 2 && _flagsTable[133] == 1 && _flagsTable[136] == 0) {
		state = 1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum63() {
	_currentGfxBackgroundCounter = 20 - _flagsTable[132] * 10;
	if (_flagsTable[132] != _execData3Counter) {
		_mainLoopCounter1 = 0;
		_execData3Counter = _flagsTable[132];
	}
	if (_flagsTable[133] == 0) {
		_currentGfxBackgroundCounter = 30;
	}
	if (_flagsTable[136] > 0 && _flagsTable[132] == 2) {
		_currentGfxBackgroundCounter = 20;
	}
	if (_flagsTable[133] == 0) {
		for (int i = 0; i < 3; ++i) {
			if (isSoundPlaying(i)) {
				stopSound(i);
			}
		}
	} else {
		if (_flagsTable[132] == 0 || (_flagsTable[132] == 2 && _flagsTable[136] > 0)) {
			if (!isSoundPlaying(1)) {
				_locationSoundsTable[1].type = 2;
				startSound(_locationSoundsTable[1].offset, 1, _locationSoundsTable[1].volume);
			}
		} else {
			if (isSoundPlaying(1)) {
				stopSound(1);
			}
		}
		if (_flagsTable[132] == 1) {
			if (!isSoundPlaying(0)) {
				_locationSoundsTable[0].type = 2;
				startSound(_locationSoundsTable[0].offset, 0, _locationSoundsTable[0].volume);
			}
		} else {
			if (isSoundPlaying(0)) {
				stopSound(0);
			}
		}
		if (_flagsTable[132] == 2 && _flagsTable[136] == 0) {
			if (!isSoundPlaying(2)) {
				startSound(_locationSoundsTable[2].offset, 2, _locationSoundsTable[2].volume);
			}
		} else {
			if (isSoundPlaying(2)) {
				stopSound(2);
			}
		}
	}
}

void TuckerEngine::execData3PreUpdate_locationNum64() {
	if (_currentGfxBackgroundCounter < 30) {
		_locationHeightTable[64] = 48 - (_currentGfxBackgroundCounter / 10);
	} else {
		_locationHeightTable[64] = 47;
	}
}

void TuckerEngine::updateSprite_locationNum65(int i) {
	int state;
	if (_flagsTable[188] == 1) {
		_flagsTable[188] = 2;
		state = 1;
		_spritesTable[i].gfxBackgroundOffset = 100;
	} else if (_flagsTable[188] > 0 && _flagsTable[189] > 0) {
		state = -1;
		if (_xPosCurrent < 150 || _yPosCurrent > 240) {
			_flagsTable[189] = 0;
		}
	} else {
		if (_xPosCurrent >= 150 && _yPosCurrent < 240) { // FIXME: bug
			if (getRandomNumber() > 32000) {
				state = 2;
				_flagsTable[189] = 1;
			} else {
				state = -1;
			}
		} else {
			state = -1;
			_flagsTable[189] = 0;
		}
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum65() {
	_flagsTable[137] = 0;
}

void TuckerEngine::updateSprite_locationNum66_0(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 2;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 2;
		_spritesTable[i].updateDelay = 5;
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum66_1(int i) {
	int state;
	if (_flagsTable[191] == 0 && _xPosCurrent > 568) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 8;
		} else {
			state = 10;
			_spritesTable[i].needUpdate = 0;
		}
	} else {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			_spritesTable[i].needUpdate = 1;
			state = 9;
		} else if (getRandomNumber() > 30000) {
			state = 6;
			_spritesTable[i].needUpdate = 0;
		} else {
			_spritesTable[i].needUpdate = 0;
			state = 7;
			_spritesTable[i].updateDelay = 5;
		}
	}
	_spritesTable[i].state = state;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::updateSprite_locationNum66_2(int i) {
	_spritesTable[i].disabled = 1;
	_spritesTable[i].state = 3;
}

void TuckerEngine::updateSprite_locationNum66_3(int i) {
	_spritesTable[i].state = 4;
}

void TuckerEngine::updateSprite_locationNum66_4(int i) {
	_spritesTable[i].state = 5;
	_spritesTable[i].gfxBackgroundOffset = 320;
}

void TuckerEngine::execData3PreUpdate_locationNum66() {
	// FIXME: shouldn't be executed after using the map
	_flagsTable[137] = 0;
	if (_xPosCurrent > 583 && _flagsTable[191] == 0 && _nextAction == 0 && _locationMaskType == 0) {
		_panelLockedFlag = 0;
		_csDataLoaded = 0;
		_nextLocationNum = 0;
		_selectedObject.locationObject_locationNum = 0;
		if (_flagsTable[131] == 0) {
			_nextAction = 13;
		} else if (_flagsTable[130] == 0) {
			_nextAction = 14;
		} else {
			_nextAction = 35;
		}
	}
}

void TuckerEngine::execData3PostUpdate_locationNum66() {
	if (_spritesTable[2].flipX == 1) {
		--_updateLocationXPosTable2[0];
		if (_updateLocationXPosTable2[0] < -50) {
			_spritesTable[2].flipX = 0;
			_updateLocationXPosTable2[0] = -50;
		}
	} else {
		++_updateLocationXPosTable2[0];
		if (_updateLocationXPosTable2[0] > 500) {
			_spritesTable[2].flipX = 1;
			_updateLocationXPosTable2[0] = 500;
		}
	}
	_spritesTable[2].gfxBackgroundOffset = _updateLocationXPosTable2[0] + 8320;
	const int spr = 2;
	_spritesTable[spr].colorType = 1;
	_spritesTable[spr].yMaxBackground = 0;
	drawSprite(spr);
	_spritesTable[spr].colorType = 0;
}

void TuckerEngine::updateSprite_locationNum69_1(int i) {
	int state;
	if (_flagsTable[236] == 1) {
		state = 4;
	} else if (_flagsTable[236] == 2) {
		state = 3;
	} else if (_flagsTable[236] == 3) {
		state = 2;
	} else if (_flagsTable[236] == 4) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			state = 9;
			_spritesTable[i].needUpdate = 1;
		} else {
			state = 14;
			_spritesTable[i].needUpdate = 0;
		}
	} else if (_flagsTable[236] == 5) {
		state = 7;
		_flagsTable[236] = 6;
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum69_2(int i) {
	if (_flagsTable[237] == 2) {
		_flagsTable[237] = 3;
	}
	int state;
	if (_flagsTable[236] < 4 || _flagsTable[237] == 0) {
		state = -1;
	} else if (_flagsTable[237] == 1) {
		state = 5;
		_flagsTable[237] = 2;
	} else if (_flagsTable[237] == 4 || _flagsTable[237] == 2 || _flagsTable[237] == 3 || _flagsTable[237] == 7) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			state = 10;
			_spritesTable[i].needUpdate = 1;
		} else {
			state = 12;
			_spritesTable[i].needUpdate = 0;
		}
	} else if (_flagsTable[237] == 5) {
		state = 16;
		_flagsTable[237] = 6;
	} else if (_flagsTable[237] == 6) {
		state = 15;
		_flagsTable[237] = 7;
	} else if (_flagsTable[237] == 8) {
		state = 6;
		_flagsTable[237] = 9;
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum69_3(int i) {
	int state;
	if (_flagsTable[236] > 4) {
		state = -1;
	} else if (_flagsTable[237] == 3) {
		state = 8;
		_flagsTable[237] = 4;
	} else if (_flagsTable[237] > 2 && _flagsTable[237] < 9) {
		_flagsTable[238] = 1;
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
			state = 11;
			_spritesTable[i].needUpdate = 1;
		} else {
			state = 13;
			_spritesTable[i].needUpdate = 0;
		}
	} else {
		state = -1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::execData3PreUpdate_locationNum70() {
	const uint8 color = 103;
	if (_execData3Counter == 0) {
		startSound(_locationSoundsTable[6].offset, 6, _locationSoundsTable[6].volume);
		_execData3Counter = 1;
		_flagsTable[143] = 0;
		_updateLocation70StringLen = 0;
		_forceRedrawPanelItems = true;
		_panelState = 1;
		setCursorType(2);
	}
	_forceRedrawPanelItems = true;
	_panelState = 1;
	setCursorType(2);
	int pos = getPositionForLine(22, _infoBarBuf);
	const int yPos = (_flagsTable[143] == 0) ? 90 : 72;
	drawStringAlt(88, yPos, color, &_infoBarBuf[pos]);
	Graphics::drawStringChar(_locationBackgroundGfxBuf, 88, yPos + 9, 640, 62, color, _charsetGfxBuf);
	if (_flagsTable[143] != 0) {
		pos = getPositionForLine(_flagsTable[143] * 2 + 23, _infoBarBuf);
		drawStringAlt(88, yPos + 18, color, &_infoBarBuf[pos]);
		pos = getPositionForLine(_flagsTable[143] * 2 + 24, _infoBarBuf);
		drawStringAlt(88, yPos + 27, color, &_infoBarBuf[pos]);
	}
	execData3PreUpdate_locationNum70Helper();
	drawStringAlt(88 + 8, yPos + 9, color, _updateLocation70String, _updateLocation70StringLen);
}

void TuckerEngine::execData3PreUpdate_locationNum70Helper() {
	if (_lastKeyPressed != 0 && _flagsTable[143] <= 0) {
		if (_lastKeyPressed == Common::KEYCODE_BACKSPACE || _lastKeyPressed == Common::KEYCODE_DELETE) {
			if (_updateLocation70StringLen > 0) {
				--_updateLocation70StringLen;
				startSound(_locationSoundsTable[0].offset, 0, _locationSoundsTable[0].volume);
			}
		} else if (_lastKeyPressed == Common::KEYCODE_RETURN) {
			_flagsTable[143] = 1;
			_nextAction = 1;
			_csDataLoaded = false;
			const int pos = getPositionForLine(23, _infoBarBuf);
			if (memcmp(&_infoBarBuf[pos + 3], _updateLocation70String, 16) != 0) {
				_flagsTable[143] = 2;
			}
			const int num = (_flagsTable[143] == 1) ? 4 : 5;
			startSound(_locationSoundsTable[num].offset, num, _locationSoundsTable[num].volume);
		} else if (_updateLocation70StringLen < 19) {
			uint8 chr = 0;
			switch (_lastKeyPressed) {
			case Common::KEYCODE_SPACE:
				chr = 32;
				break;
			case Common::KEYCODE_BACKQUOTE:
				chr = 35;
				break;
			case Common::KEYCODE_LEFTPAREN:
				chr = 40;
				break;
			case Common::KEYCODE_RIGHTPAREN:
				chr = 41;
				break;
			case Common::KEYCODE_LESS:
				chr = 44;
				break;
			case Common::KEYCODE_GREATER:
				chr = 46;
				break;
			case Common::KEYCODE_COLON:
				chr = 56;
				break;
			case Common::KEYCODE_QUESTION:
				chr = 92;
				break;
			case Common::KEYCODE_QUOTE:
				chr = 96;
				break;
			default:
				if (_lastKeyPressed >= Common::KEYCODE_a && _lastKeyPressed <= Common::KEYCODE_z) {
					chr = 'A' + (_lastKeyPressed - Common::KEYCODE_a);
				}
				break;
			}
			startSound(_locationSoundsTable[0].offset, 0, _locationSoundsTable[0].volume);
			if (chr > 0) {
				_updateLocation70String[_updateLocation70StringLen] = chr;
				++_updateLocation70StringLen;
			}
		}
		_lastKeyPressed = 0;
	}
}

void TuckerEngine::updateSprite_locationNum71(int i) {
	int state;
	if (_flagsTable[155] != 6 || _flagsTable[207] == 1) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 1;
	} else if (getRandomNumber() < 30000) {
		_spritesTable[i].needUpdate = 0;
		state = 2;
		_spritesTable[i].updateDelay = 5;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 2;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum72(int i) {
	int state;
	if (_flagsTable[155] == 7 || _flagsTable[207] == 1) {
		state = -1;
	} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 2;
	} else {
		_spritesTable[i].needUpdate = 0;
		state = 1;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum74(int i) {
	static const uint8 stateTable[] = { 1, 3, 5, 5, 10, 16, 16 };
	int num = _flagsTable[236] - 74;
	if (stateTable[num] + i == 21) {
		if (_updateLocationFlag == 0) {
			_updateLocationFlag = 1;
		} else {
			_spritesTable[i].animationFrame = _spritesTable[i].firstFrame - 1;
			_spritesTable[i].updateDelay = 5;
			_updateSpriteFlag1 = 1;
		}
	}
	_spritesTable[i].state = stateTable[num] + i;
}

void TuckerEngine::updateSprite_locationNum79(int i) {
	int state;
	if (_flagsTable[227] == 0) {
		state = 1;
		_flagsTable[227] = 1;
	} else if (_flagsTable[227] == 1 && _charSpeechSoundCounter > 0) {
		state = 2;
		_spritesTable[i].needUpdate = 1;
	} else if (_flagsTable[227] == 2) {
		state = 3;
		_flagsTable[227] = 3;
		_spritesTable[i].needUpdate = 1;
	} else {
		_spritesTable[i].animationFrame = _spritesTable[i].firstFrame - 1;
		_spritesTable[i].updateDelay = 5;
		_updateSpriteFlag1 = 1;
		state = 3;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum81_0(int i) {
	int state;
	if (_charSpeechSoundCounter > 0 && _actionCharacterNum == i) {
		_spritesTable[i].needUpdate = 1;
		state = 3;
		_flagsTable[288] = 1;
	} else if (_flagsTable[288] < 2) {
		_spritesTable[i].needUpdate = 0;
		state = 2;
		if (_flagsTable[288] == 1) {
			_flagsTable[288] = 2;
		}
	} else {
		_spritesTable[i].animationFrame = _spritesTable[i].firstFrame - 1;
		_spritesTable[i].updateDelay = 5;
		_updateSpriteFlag1 = 1;
		state = 2;
	}
	_spritesTable[i].state = state;
}

void TuckerEngine::updateSprite_locationNum81_1(int i) {
	_spritesTable[i].state = 1;
}

void TuckerEngine::updateSprite_locationNum82(int i) {
	int state;
	if (_charSpeechSoundCounter > 0) {
		state = 1;
		_flagsTable[229] = 1;
		_spritesTable[i].needUpdate = 1;
	} else if (_flagsTable[229] == 0) {
		state = 1;
	} else if (_flagsTable[229] == 1) {
		state = 2;
		_flagsTable[229] = 2;
		_spritesTable[i].needUpdate = 0;
	} else {
		_spritesTable[i].animationFrame = _spritesTable[i].firstFrame - 1;
		_spritesTable[i].updateDelay = 5;
		_updateSpriteFlag1 = 1;
		state = 2;
	}
	_spritesTable[i].state = state;
}

} // namespace Tucker
