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

#ifdef ENABLE_EOB

#include "kyra/eob.h"
#include "kyra/screen_eob.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

#include "base/version.h"

namespace Kyra {

class EoBIntroPlayer {
public:
	EoBIntroPlayer(EoBEngine *vm, Screen_EoB *screen);
	~EoBIntroPlayer() {}

	void start();

private:
	void openingCredits();
	void tower();
	void orb();
	void waterdeepEntry();
	void king();
	void hands();
	void waterdeepExit();
	void tunnel();

	void loadAndSetPalette(const char *filename);
	void copyBlurRegion(int x1, int y1, int x2, int y2, int w, int h, int step);
	void boxMorphTransition(int targetDestX, int targetDestY, int targetFinalX, int targetFinalY, int targetSrcX, int targetSrcY, int targetFinalW, int targetFinalH, int originX1, int originY1, int originW, int originH);
	void whirlTransition();

	EoBEngine *_vm;
	Screen_EoB *_screen;

	const char *const *_filesOpening;
	const char *const *_filesTower;
	const char *const *_filesOrb;
	const char *const *_filesWdEntry;
	const char *const *_filesKing;
	const char *const *_filesHands;
	const char *const *_filesWdExit;
	const char *const *_filesTunnel;
	const uint8 *_openingFrmDelay;
	const uint8 *_wdEncodeX;
	const uint8 *_wdEncodeY;
	const uint8 *_wdEncodeWH;
	const uint16 *_wdDsX;
	const uint8 *_wdDsY;
	const uint8 *_tvlX1;
	const uint8 *_tvlY1;
	const uint8 *_tvlX2;
	const uint8 *_tvlY2;
	const uint8 *_tvlW;
	const uint8 *_tvlH;
};

EoBIntroPlayer::EoBIntroPlayer(EoBEngine *vm, Screen_EoB *screen) : _vm(vm), _screen(screen) {
	int temp = 0;
	_filesOpening = _vm->staticres()->loadStrings(kEoB1IntroFilesOpening, temp);
	_filesTower = _vm->staticres()->loadStrings(kEoB1IntroFilesTower, temp);
	_filesOrb = _vm->staticres()->loadStrings(kEoB1IntroFilesOrb, temp);
	_filesWdEntry = _vm->staticres()->loadStrings(kEoB1IntroFilesWdEntry, temp);
	_filesKing = _vm->staticres()->loadStrings(kEoB1IntroFilesKing, temp);
	_filesHands = _vm->staticres()->loadStrings(kEoB1IntroFilesHands, temp);
	_filesWdExit = _vm->staticres()->loadStrings(kEoB1IntroFilesWdExit, temp);
	_filesTunnel = _vm->staticres()->loadStrings(kEoB1IntroFilesTunnel, temp);
	_openingFrmDelay = _vm->staticres()->loadRawData(kEoB1IntroOpeningFrmDelay, temp);
	_wdEncodeX = _vm->staticres()->loadRawData(kEoB1IntroWdEncodeX, temp);
	_wdEncodeY = _vm->staticres()->loadRawData(kEoB1IntroWdEncodeY, temp);
	_wdEncodeWH = _vm->staticres()->loadRawData(kEoB1IntroWdEncodeWH, temp);
	_wdDsX = _vm->staticres()->loadRawDataBe16(kEoB1IntroWdDsX, temp);
	_wdDsY = _vm->staticres()->loadRawData(kEoB1IntroWdDsY, temp);
	_tvlX1 = _vm->staticres()->loadRawData(kEoB1IntroTvlX1, temp);
	_tvlY1 = _vm->staticres()->loadRawData(kEoB1IntroTvlY1, temp);
	_tvlX2 = _vm->staticres()->loadRawData(kEoB1IntroTvlX2, temp);
	_tvlY2 = _vm->staticres()->loadRawData(kEoB1IntroTvlY2, temp);
	_tvlW = _vm->staticres()->loadRawData(kEoB1IntroTvlW, temp);
	_tvlH = _vm->staticres()->loadRawData(kEoB1IntroTvlH, temp);
}

void EoBIntroPlayer::start() {
	_vm->_allowSkip = true;
	openingCredits();

	if (!_vm->shouldQuit() && !_vm->skipFlag()) {
		_vm->snd_playSong(2);
		_screen->loadBitmap((_vm->_configRenderMode == Common::kRenderCGA || _vm->_configRenderMode == Common::kRenderEGA) ? "TITLE-E.CMP" : "TITLE-V.CMP", 3, 5, 0);
		_screen->convertPage(5, 2, _vm->_cgaMappingDefault);
		_screen->crossFadeRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_vm->delay(120 * _vm->_tickLength);
	}

	Common::SeekableReadStream *s = _vm->resource()->createReadStream("TEXT.RAW");
	if (s) {
		s->seek(768);
		_screen->loadFileDataToPage(s, 5, s->size() - 768);
		delete s;
	} else {
		_screen->loadBitmap("TEXT.CMP", 3, 5, 0);
	}
	_screen->convertPage(5, 6, _vm->_cgaMappingAlt);

	tower();
	orb();
	waterdeepEntry();
	king();
	hands();
	waterdeepExit();
	tunnel();

	whirlTransition();
	_vm->snd_stopSound();
	_vm->_allowSkip = false;
}

void EoBIntroPlayer::openingCredits() {
	loadAndSetPalette(_filesOpening[5]);

	_screen->loadBitmap(_filesOpening[4], 5, 3, 0);
	_screen->convertPage(3, 0, _vm->_cgaMappingAlt);
	_screen->updateScreen();

	_vm->snd_playSong(1);
	_vm->delay(_openingFrmDelay[0] * _vm->_tickLength);

	for (int i = 0; i < 4 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		_screen->loadBitmap(_filesOpening[i], 5, 3, 0);
		uint32 nextFrameTimer = _vm->_system->getMillis() + _openingFrmDelay[i + 1] * _vm->_tickLength;
		_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
		_screen->crossFadeRegion(0, 50, 0, 50, 320, 102, 4, 0);
		_vm->delayUntil(nextFrameTimer);
	}
}

void EoBIntroPlayer::tower() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	_screen->loadBitmap(_filesTower[1], 5, 3, 0);
	_screen->setCurPage(2);
	uint8 *shp = _screen->encodeShape(0, 0, 16, 56, true, _vm->_cgaMappingAlt);
	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
	_screen->clearCurPage();

	for (int i = 0; i < 200; i += 64)
		_screen->copyRegion(128, 104, 96, i, 128, 64, 4, 2, Screen::CR_NO_P_CHECK);

	_screen->fillRect(0, 184, 319, 199, 12);
	int cp = _screen->setCurPage(0);
	whirlTransition();
	loadAndSetPalette(_filesTower[0]);

	_screen->setCurPage(cp);
	_screen->clearCurPage();

	for (int i = 0; i < 200; i += 64)
		_screen->copyRegion(128, 104, 0, i, 128, 64, 4, 2, Screen::CR_NO_P_CHECK);

	_screen->setCurPage(0);

	for (int i = 0; i < 64 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 2) {
		uint32 end = _vm->_system->getMillis() + 2 * _vm->_tickLength;
		_screen->copyRegion(0, 142 - i, 96, 0, 128, i + 1, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, 96, i + 1, 128, 167 - i, 2, 0, Screen::CR_NO_P_CHECK);
		if (!i)
			_screen->copyRegion(0, 0, 0, 168, 320, 32, 6, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	for (int i = 0; i < 24 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 2) {
		uint32 end = _vm->_system->getMillis() + 2 * _vm->_tickLength;
		_screen->copyRegion(0, 79 - i, 96, 0, 24, 65 + i, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(104, 79 - i, 200, 0, 24, 65 + i, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(24, 110, 120, i + 31, 80, 34, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(152, 0, 120, 32, 80, i + 1, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, 96, 65 + i, 128, 103 - i, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	for (int i = 0; i < 56 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 2) {
		uint32 end = _vm->_system->getMillis() + 2 * _vm->_tickLength;
		_screen->copyRegion(0, 56, 96, i, 24, 54, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(104, 56, 200, i, 24, 54, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 110, 96, 54 + i, 128, 34, 4, 0, Screen::CR_NO_P_CHECK);

		if (i < 32) {
			_screen->fillRect(128, 0, 255, i + 1, 12, 2);
			_screen->copyRegion(152, 0, 120, 32, 80, i + 25, 4, 0, Screen::CR_NO_P_CHECK);
		} else {
			_screen->fillRect(128, 0, 255, i + 1, 12, 2);
			_screen->copyRegion(152, i + 1, 120, 32 + i + 1, 80, 23, 4, 0, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(152, 0, 152, 32, 80, i + 1, 4, 2, Screen::CR_NO_P_CHECK);
		}

		_screen->drawShape(2, shp, 128, i - 55, 0);
		_screen->copyRegion(128, 0, 96, 0, 128, i + 1, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, 96, i + 89, 128, 79 - i, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->copyRegion(0, 32, 0, 168, 320, 32, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(65 * _vm->_tickLength);
	delete[] shp;
}

void EoBIntroPlayer::orb() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	uint8 *shp[5];
	_screen->loadBitmap(_filesOrb[0], 5, 3, 0);
	_screen->setCurPage(2);
	shp[4] = _screen->encodeShape(0, 0, 20, 136, true, _vm->_cgaMappingAlt);
	_screen->loadBitmap(_filesOrb[1], 5, 3, 0);
	shp[3] = _screen->encodeShape(16, 0, 16, 104, true, _vm->_cgaMappingAlt);

	_screen->fillRect(0, 0, 127, 103, 12);
	for (int i = 1; i < 4; i++) {
		copyBlurRegion(128, 0, 0, 0, 128, 104, i);
		shp[3 - i] = _screen->encodeShape(0, 0, 16, 104, true, _vm->_cgaMappingAlt);
	}

	_screen->fillRect(0, 0, 159, 135, 12);
	_screen->setCurPage(0);
	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
	_screen->clearCurPage();

	_vm->snd_playSoundEffect(6);

	for (int i = -1; i < 4 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + 3 * _vm->_tickLength;
		if (i >= 0)
			_screen->drawShape(2, shp[i], 16, 16, 0);
		_screen->drawShape(2, shp[4], 0, 0, 0);
		_screen->copyRegion(0, 0, 80, 24, 160, 136, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->copyRegion(0, 64, 0, 168, 320, 16, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(40 * _vm->_tickLength);

	_vm->snd_playSoundEffect(6);

	for (int i = 3; i > -2 && !_vm->shouldQuit() && !_vm->skipFlag(); i--) {
		uint32 end = _vm->_system->getMillis() + 3 * _vm->_tickLength;
		_screen->fillRect(16, 16, 143, 119, 12, 2);
		if (i >= 0)
			_screen->drawShape(2, shp[i], 16, 16, 0);
		_screen->drawShape(2, shp[4], 0, 0, 0);
		_screen->copyRegion(0, 0, 80, 24, 160, 136, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->delay(40 * _vm->_tickLength);

	for (int i = 0; i < 5; i++)
		delete[] shp[i];
}

void EoBIntroPlayer::waterdeepEntry() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	uint8 *shp[4];
	uint8 *shp2[31];
	uint8 *shp3[3];

	loadAndSetPalette(_filesWdEntry[0]);
	_screen->loadBitmap(_filesWdEntry[1], 5, 3, 0);
	_screen->setCurPage(2);
	shp[3] = _screen->encodeShape(0, 0, 20, 136, true, _vm->_cgaMappingAlt);
	for (int i = 1; i < 4; i++) {
		copyBlurRegion(0, 0, 0, 0, 160, 136, i);
		shp[3 - i] = _screen->encodeShape(0, 0, 20, 136, true, _vm->_cgaMappingAlt);
	}
	_screen->setCurPage(0);

	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
	_screen->fillRect(0, 168, 319, 199, 12, 0);
	_vm->snd_playSoundEffect(6);

	for (int i = 0; i < 4 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + 3 * _vm->_tickLength;
		_screen->drawShape(0, shp[i], 80, 24, 0);
		delete[] shp[i];
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->copyRegion(0, 80, 0, 168, 320, 16, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(50 * _vm->_tickLength);

	_screen->setCurPage(2);
	shp[0] = _screen->encodeShape(20, 0, 20, 136, true, _vm->_cgaMappingAlt);
	_screen->loadBitmap(_filesWdEntry[2], 5, 3, 0);
	shp[1] = _screen->encodeShape(0, 0, 20, 136, true, _vm->_cgaMappingAlt);
	shp[2] = _screen->encodeShape(20, 0, 20, 136, true, _vm->_cgaMappingAlt);
	_screen->loadBitmap(_filesWdEntry[3], 5, 3, 0);

	for (int i = 0; i < 31; i++)
		shp2[i] = _screen->encodeShape(_wdEncodeX[i], 136 + (_wdEncodeY[i] << 3), _wdEncodeWH[i], _wdEncodeWH[i] << 3, true, _vm->_cgaMappingAlt);
	for (int i = 0; i < 3; i++)
		shp3[i] = _screen->encodeShape(5 * i, 152, 5, 32, true, _vm->_cgaMappingAlt);

	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);

	for (int i = 0; i < 3 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + 3 * _vm->_tickLength;
		_screen->fillRect(0, 0, 159, 135, 12, 2);
		_screen->drawShape(2, shp[i], 0, 0, 0);
		_screen->copyRegion(0, 0, 80, 24, 160, 136, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->copyRegion(0, 0, 80, 24, 160, 136, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(4 * _vm->_tickLength);
	_screen->copyRegion(160, 0, 80, 24, 160, 136, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->fillRect(0, 168, 319, 199, 12, 0);
	_screen->updateScreen();
	_vm->delay(4 * _vm->_tickLength);
	_screen->copyRegion(0, 184, 40, 184, 232, 16, 4, 0, Screen::CR_NO_P_CHECK);

	int cx = 264;
	int cy = 11;

	for (int i = 0; i < 70 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + 3 * _vm->_tickLength;

		_screen->copyRegion(cx - 2, cy - 2, 0, 0, 48, 36, 4, 4, Screen::CR_NO_P_CHECK);
		_screen->drawShape(4, shp3[((i & 3) == 3) ? 1 : (i & 3)], cx, cy, 0);
		_screen->copyRegion(cx - 2, cy - 2, cx - 82, cy + 22, 48, 36, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, cx - 2, cy - 2, 48, 36, 4, 4, Screen::CR_NO_P_CHECK);
		cx--;
		cy++;

		for (int ii = 0; ii < 5; ii++) {
			int s = _vm->_rnd.getRandomNumber(255) % 31;
			_screen->drawShape(0, shp2[s], _wdDsX[s] - 80, _wdDsY[s] + 24, 0);
		}

		if (!(_vm->_rnd.getRandomNumber(255) & 7))
			_vm->snd_playSoundEffect(_vm->_rnd.getRandomBit() ? 5 : 14);

		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	for (int i = 0; i < 3; i++) {
		delete[] shp[i];
		delete[] shp3[i];
	}

	for (int i = 0; i < 31; i++)
		delete[] shp2[i];
}

void EoBIntroPlayer::king() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	_screen->loadBitmap(_filesKing[0], 5, 3, 0);
	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);

	int x = 15;
	int y = 14;
	int w = 1;
	int h = 1;

	for (int i = 0; i < 10 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->copyRegion(x << 3, y << 3, x << 3, y << 3, w << 3, h << 3, 4, 0, Screen::CR_NO_P_CHECK);
		if (x > 6)
			x --;
		if (y > 0)
			y -= 2;
		w += 3;
		if (x + w > 34)
			w = 34 - x;
		h += 3;
		if (y + h > 23)
			h = 23 - y;
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->delay(25 * _vm->_tickLength);

	uint8 *shp[4];
	int16 dy[4];
	int16 stepY[4];

	static const uint8 advEncX[] = { 0, 6, 12, 19 };
	static const uint8 advEncW[] = { 6, 6, 7, 6 };
	static const int8 modY[] = { -4, -8, -2, -2, 1, 0, 0, 0 };

	_screen->loadBitmap(_filesKing[1], 5, 3, 0);
	_screen->setCurPage(2);
	for (int i = 0; i < 4; i++) {
		shp[i] = _screen->encodeShape(advEncX[i], 0, advEncW[i], 98, true, _vm->_cgaMappingAlt);
		dy[i] = 180 + ((_vm->_rnd.getRandomNumber(255) & 3) << 3);
		stepY[i] = (i * 5) & 3;
	}

	_screen->copyPage(0, 4);

	for (bool runloop = true; runloop && !_vm->shouldQuit() && !_vm->skipFlag();) {
		runloop = false;
		uint32 end = _vm->_system->getMillis() + 2 * _vm->_tickLength;

		for (int i = 0; i < 4; i++) {
			if (dy[i] <= 82)
				continue;
			stepY[i] = (stepY[i] + 1) & 7;
			dy[i] += modY[stepY[i]];

			if (dy[i] < 82)
				dy[i] = 82;

			if (dy[i] < 180) {
				_screen->copyRegion((advEncX[i] + 8) << 3, dy[i] - 2, 0, dy[i] - 2, advEncW[i] << 3, 182 - dy[i], 4, 4, Screen::CR_NO_P_CHECK);
				_screen->drawShape(4, shp[i], 0, dy[i], 0);
				_screen->copyRegion(0, dy[i] - 2, (advEncX[i] + 8) << 3, dy[i] - 2, advEncW[i] << 3, 182 - dy[i], 4, 0, Screen::CR_NO_P_CHECK);
			}

			runloop = true;
		}

		if (!(_vm->_rnd.getRandomNumber(255) & 3))
			_vm->snd_playSoundEffect(7);

		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->copyRegion(0, 96, 0, 160, 320, 32, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(70 * _vm->_tickLength);

	for (int i = 0; i < 4; i++)
		delete[] shp[i];
}

void EoBIntroPlayer::hands() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	_screen->setCurPage(2);
	uint8 *shp1 = _screen->encodeShape(0, 140, 21, 60, true, _vm->_cgaMappingAlt);
	uint8 *shp2 = _screen->encodeShape(21, 140, 12, 60, true, _vm->_cgaMappingAlt);
	_screen->loadBitmap(_filesHands[0], 3, 5, 0);

	_screen->fillRect(0, 160, 319, 199, 12, 0);
	_screen->fillRect(0, 0, 191, 63, 157, 2);
	_screen->drawShape(2, shp1, 0, 4, 0);
	_screen->drawShape(2, shp2, 151, 4, 0);
	boxMorphTransition(25, 8, 18, 4, 3, 0, 21, 8, 6, 0, 28, 23);
	_screen->copyRegion(0, 128, 0, 176, 320, 16, 6, 0, Screen::CR_NO_P_CHECK);

	_screen->updateScreen();
	_vm->delay(15 * _vm->_tickLength);
	_vm->snd_playSoundEffect(11);

	for (int i = -22; i <= 20 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 4) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->fillRect(0, 0, 167, 63, 157);
		_screen->drawShape(2, shp1, i, 4, 0);
		_screen->drawShape(2, shp2, 105 - i, 4, 0);
		_screen->copyRegion(0, 0, 144, 32, 168, 64, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->snd_playSoundEffect(10);

	delete[] shp1;
	delete[] shp2;
	_vm->delay(15 * _vm->_tickLength);

	_screen->setCurPage(4);
	shp1 = _screen->encodeShape(17, 0, 11, 120, true, _vm->_cgaMappingAlt);
	shp2 = _screen->encodeShape(28, 112, 1, 31, true, _vm->_cgaMappingAlt);
	uint8 *shp3 = _screen->encodeShape(9, 138, 14, 54, true, _vm->_cgaMappingAlt);

	_screen->setCurPage(2);
	_screen->fillRect(0, 0, 135, 63, 157);
	_screen->drawShape(2, shp1, 32, -80, 0);
	_screen->drawShape(2, shp2, 40, -16, 0);
	boxMorphTransition(18, 16, 10, 12, 0, 0, 17, 8, 17, 3, 25, 10);
	_vm->delay(15 * _vm->_tickLength);

	for (int i = -80; i <= 0 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 4) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->fillRect(0, 0, 135, 63, 157);
		_screen->drawShape(2, shp1, 32, i, 0);
		_screen->drawShape(2, shp2, 40, i + 64, 0);
		_screen->copyRegion(0, 0, 80, 96, 136, 64, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->snd_playSoundEffect(12);
	_vm->delay(5 * _vm->_tickLength);

	for (int i = 0; i > -54 && !_vm->shouldQuit() && !_vm->skipFlag(); i -= 4) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->fillRect(0, 0, 135, 63, 157);
		_screen->drawShape(2, shp3, 12, 64 + i, 0);
		_screen->drawShape(2, shp1, 32, i, 0);
		_screen->copyRegion(0, 0, 80, 96, 136, 64, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	delete[] shp1;
	delete[] shp2;
	delete[] shp3;
	_vm->delay(15 * _vm->_tickLength);

	_screen->setCurPage(4);
	shp1 = _screen->encodeShape(0, 0, 17, 136, true, _vm->_cgaMappingAlt);
	shp2 = _screen->encodeShape(0, 136, 9, 48, true, _vm->_cgaMappingAlt);

	_screen->setCurPage(2);
	_screen->fillRect(0, 0, 143, 95, 157);
	_screen->drawShape(2, shp1, -56, -56, 0);
	_screen->drawShape(2, shp2, 52, 49, 0);
	boxMorphTransition(9, 6, 0, 0, 0, 0, 18, 12, 8, 11, 21, 10);
	_vm->delay(15 * _vm->_tickLength);
	_vm->snd_playSoundEffect(11);

	for (int i = -56; i <= -8 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 4) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->fillRect(0, 0, 143, 95, 157);
		_screen->drawShape(2, shp1, i, i, 0);
		_screen->drawShape(2, shp2, (i == -8) ? 55 : 52, (i == -8) ? 52 : 49, 0);
		_screen->copyRegion(0, 0, 0, 0, 144, 96, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->snd_playSoundEffect(10);
	delete[] shp1;
	delete[] shp2;
	_vm->delay(30 * _vm->_tickLength);

	_screen->setCurPage(4);
	shp1 = _screen->encodeShape(28, 0, 11, 40, true, _vm->_cgaMappingAlt);
	shp2 = _screen->encodeShape(28, 40, 10, 72, true, _vm->_cgaMappingAlt);

	_screen->setCurPage(2);
	_screen->fillRect(0, 0, 87, 112, 157);
	_screen->drawShape(2, shp2, 0, 90, 0);
	boxMorphTransition(20, 13, 15, 6, 0, 0, 11, 14, 0, 0, 24, 16);
	_vm->delay(15 * _vm->_tickLength);

	int dy = 90;
	for (int i = -40; i <= 0 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 4) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->fillRect(0, 0, 87, 112, 157);
		_screen->drawShape(2, shp2, 0, dy, 0);
		_screen->copyRegion(0, 0, 120, 48, 88, 112, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
		dy -= 5;
	}

	_vm->snd_playSoundEffect(13);

	for (int i = -40; i <= 0 && !_vm->shouldQuit() && !_vm->skipFlag(); i += 4) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->fillRect(0, 0, 87, 39, 157);
		_screen->drawShape(2, shp1, 0, i, 0);
		_screen->copyRegion(0, 0, 120, 48, 88, 112, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	delete[] shp1;
	delete[] shp2;
	_vm->delay(48 * _vm->_tickLength);
}

void EoBIntroPlayer::waterdeepExit() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	uint8 *shp2[31];
	uint8 *shp3[3];

	_screen->loadBitmap(_filesWdExit[0], 5, 3, 0);
	_screen->setCurPage(2);
	for (int i = 0; i < 31; i++)
		shp2[i] = _screen->encodeShape(_wdEncodeX[i], 136 + (_wdEncodeY[i] << 3), _wdEncodeWH[i], _wdEncodeWH[i] << 3, true, _vm->_cgaMappingAlt);
	for (int i = 0; i < 3; i++)
		shp3[i] = _screen->encodeShape(5 * i + 15, 152, 5, 32, true, _vm->_cgaMappingAlt);
	uint8 *shp1 = _screen->encodeShape(31, 136, 5, 32, true, _vm->_cgaMappingAlt);
	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
	_screen->copyRegion(0, 0, 0, 136, 48, 36, 4, 4, Screen::CR_NO_P_CHECK);
	_screen->fillRect(0, 168, 319, 199, 12, 0);
	_screen->copyRegion(160, 0, 80, 24, 160, 136, 4, 0, Screen::CR_NO_P_CHECK);

	int cx = 140;
	int cy = 128;

	for (int i = 0; i < 70 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + 3 * _vm->_tickLength;
		int fx = cx - 2;
		if (fx < 160)
			fx = 160;
		int fy = cy - 2;
		if (fy > 98)
			fy = 98;

		_screen->copyRegion(fx, fy, 0, 0, 48, 36, 4, 4, Screen::CR_NO_P_CHECK);
		_screen->drawShape(4, shp3[((i & 3) == 3) ? 1 : (i & 3)], cx, cy, 0);
		_screen->drawShape(4, shp1, 160, 104, 0);
		_screen->copyRegion(fx, fy, fx - 80, fy + 24, 48, 36, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, fx, fy, 48, 36, 4, 4, Screen::CR_NO_P_CHECK);
		cx++;
		cy--;

		for (int ii = 0; ii < 5; ii++) {
			int s = _vm->_rnd.getRandomNumber(255) % 31;
			_screen->drawShape(0, shp2[s], _wdDsX[s] - 80, _wdDsY[s] + 24, 0);
		}

		if (!(_vm->_rnd.getRandomNumber(255) & 7))
			_vm->snd_playSoundEffect(_vm->_rnd.getRandomBit() ? 5 : 14);

		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	for (int i = 0; i < 3; i++)
		delete[] shp3[i];

	for (int i = 0; i < 31; i++)
		delete[] shp2[i];
	delete[] shp1;

	_screen->setCurPage(0);
	_screen->fillRect(0, 168, 319, 199, 12, 0);
	_screen->copyRegion(0, 136, 0, 0, 48, 36, 0, 4, Screen::CR_NO_P_CHECK);

	loadAndSetPalette(_filesWdExit[1]);
	_screen->loadBitmap(_filesWdExit[2], 3, 5, 0);
	_screen->convertPage(5, 2, _vm->_cgaMappingAlt);
	whirlTransition();
	_vm->delay(6 * _vm->_tickLength);

	_screen->copyRegion(0, 144, 0, 184, 320, 16, 6, 0, Screen::CR_NO_P_CHECK);

	cx = 0;
	cy = 136;
	int dy = 0;
	for (int i = 0; i < 19 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->copyRegion(cx, cy, 80, dy + 16, 160, 8, 2, 0, Screen::CR_NO_P_CHECK);
		cy += 8;
		dy += 8;
		if (i == 6) {
			cx = 160;
			cy = 0;
		}
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->snd_playSong(3);
	_vm->delay(60 * _vm->_tickLength);

	for (int i = 0; i < 56 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->copyRegion(0, 136 + i, 80, 16, 160, 56 - i, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(160, 0, 80, 72 - i, 160, 96 + i, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	for (int i = 1; i < 48 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->copyRegion(160, i, 80, 16, 160, 152, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->loadBitmap(_filesWdExit[3], 3, 5, 0);
	_screen->convertPage(5, 2, _vm->_cgaMappingAlt);
	_vm->delay(30 * _vm->_tickLength);
	_screen->setCurPage(0);
	_screen->fillRect(0, 16, 319, 31, 12);
	_screen->fillRect(0, 136, 319, 199, 12);
	_screen->copyRegion(0, 0, 80, 32, 160, 120, 2, 0, Screen::CR_NO_P_CHECK);
	loadAndSetPalette(_filesWdExit[4]);
	_screen->updateScreen();
	_vm->delay(50 * _vm->_tickLength);
}

void EoBIntroPlayer::tunnel() {
	if (_vm->shouldQuit() || _vm->skipFlag())
		return;

	_screen->setCurPage(4);
	uint8 *shp2 = _screen->encodeShape(20, 0, 20, 120, true, _vm->_cgaMappingAlt);
	uint8 *shp1 = _screen->encodeShape(0, 0, 20, 120, true, _vm->_cgaMappingAlt);
	_vm->drawBlockObject(1, 4, shp2, 160, 0, 0);
	_vm->drawBlockObject(1, 4, shp1, 0, 0, 0);
	delete[] shp1;
	delete[] shp2;

	for (int i = 0; i < 3 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + 8 * _vm->_tickLength;
		_screen->copyRegion(0, 0, 80, 32, 160, 120, 4, 0, Screen::CR_NO_P_CHECK);
		_vm->snd_playSoundEffect(7);
		_screen->updateScreen();
		_vm->delayUntil(end);
		_screen->copyRegion(0, 0, 80, 32, 160, 120, 2, 0, Screen::CR_NO_P_CHECK);
		_vm->snd_playSoundEffect(7);
		end = _vm->_system->getMillis() + 8 * _vm->_tickLength;
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_screen->copyRegion(0, 160, 0, 184, 320, 16, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(18 * _vm->_tickLength);
	_screen->copyRegion(160, 0, 80, 32, 160, 120, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(5 * _vm->_tickLength);
	_screen->copyRegion(0, 122, 80, 32, 160, 60, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(160, 122, 80, 92, 160, 60, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(5 * _vm->_tickLength);
	_screen->copyRegion(160, 0, 80, 32, 160, 120, 4, 0, Screen::CR_NO_P_CHECK);
	for (int i = 0; i < 6; i++)
		_screen->copyRegion(i * 48, 185, 56, (i << 3) + 24, 48, 8, 2, 2, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(5 * _vm->_tickLength);
	_screen->copyRegion(0, 0, 80, 32, 160, 120, 2, 0, Screen::CR_NO_P_CHECK);

	_screen->loadBitmap(_filesTunnel[0], 5, 3, 0);
	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
	_screen->updateScreen();
	_vm->delay(40 * _vm->_tickLength);

	_screen->copyRegion(264, 0, 136, 56, 48, 48, 4, 0, Screen::CR_NO_P_CHECK);
	_vm->snd_playSoundEffect(8);
	_screen->copyRegion(0, 0, 0, 0, 320, 184, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(16 * _vm->_tickLength);
	_vm->snd_playSoundEffect(4);

	for (int i = 0; i < 30 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		if (i == 0)
			_screen->fillRect(0, 168, 319, 199, 12, 0);
		_screen->copyRegion(80, 25 + (_vm->_rnd.getRandomNumber(255) & 7), 80, 24, 160, 144, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_vm->delayUntil(end);
	}

	_vm->snd_playSoundEffect(9);

	for (int i = 0; i < 6 && !_vm->shouldQuit() && !_vm->skipFlag(); i++) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->copyRegion(_tvlX1[i] << 3, _tvlY1[i], _tvlX2[i] << 3, _tvlY2[i], _tvlW[i] << 3, _tvlH[i], 4, 2, Screen::CR_NO_P_CHECK);
		for (int ii = 0; ii < 4 && !_vm->shouldQuit() && !_vm->skipFlag(); ii++) {
			_screen->updateScreen();
			_vm->delayUntil(end);
			end = _vm->_system->getMillis() + _vm->_tickLength;
			_screen->copyRegion(80, 25 + (_vm->_rnd.getRandomNumber(255) & 7), 80, 24, 160, 144, 2, 0, Screen::CR_NO_P_CHECK);
		}
	}
	_screen->copyRegion(0, 0, 0, 0, 320, 168, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(40 * _vm->_tickLength);

	_screen->loadBitmap(_filesTunnel[1], 5, 3, 0);
	_screen->convertPage(3, 4, _vm->_cgaMappingAlt);
	_vm->snd_playSoundEffect(6);
	_screen->copyRegion(0, 0, 80, 32, 160, 120, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(2 * _vm->_tickLength);
	_screen->copyRegion(160, 0, 80, 32, 160, 120, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_vm->delay(2 * _vm->_tickLength);
	_screen->copyRegion(0, 120, 80, 30, 160, 64, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(160, 120, 80, 94, 160, 64, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, 176, 0, 184, 320, 16, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->setCurPage(0);
	_screen->updateScreen();
	_vm->delay(50 * _vm->_tickLength);
}

void EoBIntroPlayer::loadAndSetPalette(const char *filename) {
	if (_vm->_configRenderMode == Common::kRenderCGA || _vm->_configRenderMode == Common::kRenderEGA)
		return;
	_screen->loadPalette(filename, _screen->getPalette(0));
	_screen->getPalette(0).fill(0, 1, 0);
	_screen->setScreenPalette(_screen->getPalette(0));
}

void EoBIntroPlayer::copyBlurRegion(int x1, int y1, int x2, int y2, int w, int h, int step) {
	const uint8 *ptr2 = _screen->getCPagePtr(3) + y1 * 320 + x1;

	if (step == 1) {
		while (h > 0) {
			int dx = x2;
			for (int i = 0; i < w; i += 2) {
				_screen->setPagePixel(3, dx++, y2, ptr2[i]);
				_screen->setPagePixel(3, dx++, y2, 0);
			}
			dx = x2;
			y2++;
			ptr2 += 320;
			for (int i = 0; i < w; i += 2) {
				_screen->setPagePixel(3, dx++, y2, 0);
				_screen->setPagePixel(3, dx++, y2, ptr2[i + 1]);
			}
			y2++;
			ptr2 += 320;
			h -= 2;
		}
	} else if (step == 2) {
		while (h > 0) {
			int dx = x2;
			for (int i = 0; i < w; i += 2) {
				_screen->setPagePixel(3, dx++, y2, ptr2[i]);
				_screen->setPagePixel(3, dx++, y2, 0);
			}
			dx = x2;
			y2++;
			ptr2 += 320;
			for (int i = 0; i < w; i++)
				_screen->setPagePixel(3, dx++, y2, 0);

			y2++;
			ptr2 += 320;
			h -= 2;
		}
	} else if (step == 3) {
		for (int i = 0; i < h; i++) {
			int dx = x2;
			if ((i % 3) == 0) {
				int ii = 0;
				for (; ii < w - 3; ii += 3) {
					_screen->setPagePixel(3, dx++, y2, ptr2[ii]);
					_screen->setPagePixel(3, dx++, y2, 0);
					_screen->setPagePixel(3, dx++, y2, 0);
				}
				for (; ii < w; ii++)
					_screen->setPagePixel(3, dx++, y2, 0);
			} else {
				for (int ii = 0; ii < w; ii++)
					_screen->setPagePixel(3, dx++, y2, 0);
			}
			y2++;
			ptr2 += 320;
		}
	}
}

void EoBIntroPlayer::boxMorphTransition(int targetDestX, int targetDestY, int targetFinalX, int targetFinalY, int targetSrcX, int targetSrcY, int targetFinalW, int targetFinalH, int originX1, int originY1, int originW, int originH) {
	int originX2 = originX1 + originW;
	int originY2 = originY1 + originH;
	if (originY2 > 21)
		originY2 = 21;

	int w = 1;
	int h = 1;
	for (bool runloop = true; runloop && !_vm->shouldQuit() && !_vm->skipFlag();) {
		uint32 end = _vm->_system->getMillis() + _vm->_tickLength;
		_screen->copyRegion(targetSrcX << 3, targetSrcY << 3, targetDestX << 3, targetDestY << 3, w << 3, h << 3, 2, 0, Screen::CR_NO_P_CHECK);
		if (originX1 < targetDestX)
			_screen->copyRegion(312, 0, originX1 << 3, 0, 8, 176, 0, 0, Screen::CR_NO_P_CHECK);
		if (originY1 < targetDestY)
			_screen->copyRegion(0, 192, 0, originY1 << 3, 320, 8, 0, 0, Screen::CR_NO_P_CHECK);
		if ((targetFinalX + targetFinalW) <= originX2)
			_screen->copyRegion(312, 0, originX2 << 3, 0, 8, 176, 0, 0, Screen::CR_NO_P_CHECK);
		if ((targetFinalY + targetFinalH) <= originY2)
			_screen->copyRegion(0, 192, 0, originY2 << 3, 320, 8, 0, 0, Screen::CR_NO_P_CHECK);

		if (!(targetDestX != targetFinalX || targetDestY != targetFinalY || w != targetFinalW || h != targetFinalH || originX1 < targetFinalX || originY1 < targetFinalY || (targetFinalX + targetFinalW) < originX2 || (targetFinalY + targetFinalH) < originY2))
			runloop = false;

		int v = targetFinalX - targetDestX;
		v = (v < 0) ? -1 : ((v > 0) ? 1 : 0);
		targetDestX += v;
		v = targetFinalY - targetDestY;
		v = (v < 0) ? -1 : ((v > 0) ? 1 : 0);
		targetDestY += v;

		if (w != targetFinalW)
			w += 2;
		if (w > targetFinalW)
			w = targetFinalW;

		if (h != targetFinalH)
			h += 2;
		if (h > targetFinalH)
			h = targetFinalH;

		if (++originX1 > targetFinalX)
			originX1 = targetFinalX;

		if (++originY1 > targetFinalY)
			originY1 = targetFinalY;

		if ((targetFinalX + targetFinalW) < originX2)
			originX2--;

		if ((targetFinalY + targetFinalH) < originY2)
			originY2--;

		_screen->updateScreen();
		_vm->delayUntil(end);
	}
}

void EoBIntroPlayer::whirlTransition() {
	for (int i = 0; i < 2; i++) {
		for (int ii = 0; ii < 8; ii++) {
			uint32 e = _vm->_system->getMillis() + 3;
			if (ii & 1) {
				for (int iii = i + ii; iii < 320; iii += 8)
					_screen->drawClippedLine(iii, 0, iii, 199, 12);
			} else {
				for (int iii = i + ii; iii < 200; iii += 8)
					_screen->drawClippedLine(0, iii, 319, iii, 12);
			}
			_screen->updateScreen();
			uint32 c = _vm->_system->getMillis();
			if (e > c)
				_vm->_system->delayMillis(e - c);
		}
	}
}

int EoBEngine::mainMenu() {
	int menuChoice = _menuChoiceInit;
	_menuChoiceInit = 0;

	Screen::FontId of = _screen->_currentFont;

	while (menuChoice >= 0 && !shouldQuit()) {
		switch (menuChoice) {
		case 0: {
			if (_configRenderMode != Common::kRenderEGA)
				_screen->loadPalette("EOBPAL.COL", _screen->getPalette(0));
			_screen->loadEoBBitmap("INTRO", _cgaMappingDefault, 5, 3, 2);
			_screen->setScreenPalette(_screen->getPalette(0));
			_screen->_curPage = 2;
			of = _screen->setFont(Screen::FID_6_FNT);
			Common::String versionString(Common::String::format("ScummVM %s", gScummVMVersion));
			_screen->printText(versionString.c_str(), 280 - versionString.size() * 6, 153, _screen->getPagePixel(2, 0, 0), 0);
			_screen->setFont(of);
			_screen->fillRect(0, 159, 319, 199, _screen->getPagePixel(2, 0, 0));
			gui_drawBox(77, 165, 173, 29, 14, 13, 12);
			gui_drawBox(76, 164, 175, 31, 14, 13, -1);
			_screen->_curPage = 0;
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			_allowImport = true;
			menuChoice = mainMenuLoop();
			_allowImport = false;
			} break;

		case 1:
			// load game in progress
			menuChoice = -1;
			break;

		case 2:
			// create new party
			menuChoice = -2;
			break;

		case 3:
			// quit
			menuChoice = -5;
			break;

		case 4:
			// intro
			_sound->loadSoundFile("SOUND");
			_screen->hideMouse();
			seq_playIntro();
			_screen->showMouse();
			_sound->loadSoundFile("ADLIB");
			menuChoice = 0;
			break;
		}
	}

	return shouldQuit() ? -5 : menuChoice;
}

int EoBEngine::mainMenuLoop() {
	int sel = -1;
	do {
		_screen->setScreenDim(28);
		_gui->simpleMenu_setup(8, 0, _mainMenuStrings, -1, 0, 0);

		while (sel == -1 && !shouldQuit())
			sel = _gui->simpleMenu_process(8, _mainMenuStrings, 0, -1, 0);
	} while ((sel < 0 || sel > 5) && !shouldQuit());

	return sel + 1;
}

void EoBEngine::seq_playIntro() {
	EoBIntroPlayer(this, _screen).start();
}

void EoBEngine::seq_playFinale() {
	Common::SeekableReadStream *s = _res->createReadStream("TEXT.DAT");
	_screen->loadFileDataToPage(s, 5, 32000);
	delete s;

	snd_playSoundEffect(20);

	_txt->resetPageBreakString();
	_txt->setWaitButtonMode(1);
	_txt->setupField(12, true);
	gui_drawBox(0, 0, 176, 175, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	_txt->printDialogueText(51, _moreStrings[0]);

	if (!checkScriptFlags(0x1ffe)) {
		_screen->fadeToBlack();
		return;
	}

	_txt->printDialogueText(_finBonusStrings[0]);
	for (int i = 0; i < 6; i++) {
		_txt->printDialogueText(_finBonusStrings[1]);
		if (_characters[i].flags & 1)
			_txt->printDialogueText(_characters[i].name);
	}

	uint32 password = 0;
	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1))
			continue;

		int len = strlen(_characters[i].name);
		for (int ii = 0; ii < len; ii++) {
			uint32 c = _characters[i].name[ii];
			password += (c * c);
		}
	}

	_txt->printDialogueText(Common::String::format(_finBonusStrings[2], password).c_str(), true);
	_screen->fadeToBlack();
}

void EoBEngine::seq_xdeath() {
	uint8 *shapes1[5];
	uint8 *shapes2;

	_screen->loadShapeSetBitmap("XDEATH2", 5, 3);
	for (int i = 0; i < 4; i++)
		shapes1[i] = _screen->encodeShape(i / 2 * 14, i / 2 * 88, 14, 88, true, _cgaMappingDefault);
	_screen->loadShapeSetBitmap("XDEATH3", 5, 3);
	shapes2 = _screen->encodeShape(22, 0, 16, 95, true, _cgaMappingDefault);
	_screen->loadEoBBitmap("XDEATH1", _cgaMappingDefault, 5, 3, -1);
	_screen->convertPage(3, 2, _cgaMappingDefault);
	_screen->setCurPage(0);

	for (int i = 0; i < 10; i++) {
		if (i == 2)
			snd_playSoundEffect(72);
		else if (i == 4 || i == 6)
			snd_playSoundEffect(54);
		else
			snd_playSoundEffect(34);

		if (i < 6) {
			_screen->copyRegion((i % 3) * 104, i / 3 * 88, 32, 10, 104, 88, 2, 0, Screen::CR_NO_P_CHECK);
		} else {
			snd_playSoundEffect(42);
			_screen->drawShape(0, shapes1[i - 6], 32, 10, 0);
		}

		_screen->updateScreen();
		delay(4 * _tickLength);
	}

	const ScreenDim *dm = _screen->getScreenDim(5);
	_screen->modifyScreenDim(5, dm->sx, 8, dm->w, dm->h);
	_screen->copyRegion(0, 0, 0, 0, 176, 120, 0, 5, Screen::CR_NO_P_CHECK);

	for (int i = 0; i < 19; i++) {
		snd_playSoundEffect(119);
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 5, 2, Screen::CR_NO_P_CHECK);
		_screen->drawShape(2, shapes2, 24, i * 5 - 90, 5);
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delay(2 * _tickLength);
	}

	_screen->modifyScreenDim(5, dm->sx, 0, dm->w, dm->h);

	snd_playSoundEffect(5);
	delay(60 * _tickLength);

	for (int i = 0; i < 4; i++)
		delete[] shapes1[i];
	delete[] shapes2;

	gui_drawPlayField(false);
	gui_drawAllCharPortraitsWithStats();
}

} // End of namespace Kyra

#endif // ENABLE_EOB
