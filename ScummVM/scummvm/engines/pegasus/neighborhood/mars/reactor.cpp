/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/mars/reactor.h"

namespace Pegasus {

static const CoordType kCurrentGuessWidth = 121;
static const CoordType kCurrentGuessHeight = 23;

static const CoordType kOneGuessWidth = 25;
static const CoordType kOneGuessHeight = 23;

static const ResIDType kReactorChoicesPICTID = 905;

static const CoordType kCurrentGuessLeft = kNavAreaLeft + 146;
static const CoordType kCurrentGuessTop = kNavAreaTop + 90;

ReactorGuess::ReactorGuess(const DisplayElementID id) : DisplayElement(id) {
	setBounds(kCurrentGuessLeft, kCurrentGuessTop, kCurrentGuessLeft + kCurrentGuessWidth,
			kCurrentGuessTop + kCurrentGuessHeight);
	setDisplayOrder(kMonitorLayer);
	_currentGuess[0] = -1;
	_currentGuess[1] = -1;
	_currentGuess[2] = -1;
}

void ReactorGuess::initReactorGuess() {
	_colors.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kReactorChoicesPICTID);
	startDisplaying();
	show();
}

void ReactorGuess::disposeReactorGuess() {
	stopDisplaying();
	_colors.deallocateSurface();
}

void ReactorGuess::setGuess(int32 a, int32 b, int32 c) {
	_currentGuess[0] = a;
	_currentGuess[1] = b;
	_currentGuess[2] = c;
	triggerRedraw();
}

void ReactorGuess::draw(const Common::Rect &) {
	if (_colors.isSurfaceValid()) {
		Common::Rect r1(0, 0, kOneGuessWidth, kOneGuessHeight);
		Common::Rect r2 = r1;

		for (int i = 0; i < 3; i++) {
			if (_currentGuess[i] >= 0) {
				r1.moveTo(kOneGuessWidth * _currentGuess[i], 0);
				r2.moveTo(kCurrentGuessLeft + 48 * i, kCurrentGuessTop);
				_colors.copyToCurrentPortTransparent(r1, r2);
			}
		}
	}
}

static const CoordType kReactorChoiceHiliteWidth = 166;
static const CoordType kReactorChoiceHiliteHeight = 26;

static const CoordType kChoiceHiliteLefts[6] = {
	0,
	34,
	34 + 34,
	34 + 34 + 32,
	34 + 34 + 32 + 34,
	34 + 34 + 32 + 34 + 32
};

static const ResIDType kReactorChoiceHilitePICTID = 901;

static const CoordType kReactorChoiceHiliteLeft = kNavAreaLeft + 116;
static const CoordType kReactorChoiceHiliteTop = kNavAreaTop + 158;

ReactorChoiceHighlight::ReactorChoiceHighlight(const DisplayElementID id) : DisplayElement(id) {
	setBounds(kReactorChoiceHiliteLeft, kReactorChoiceHiliteTop, kReactorChoiceHiliteLeft + kReactorChoiceHiliteWidth,
			kReactorChoiceHiliteTop + kReactorChoiceHiliteHeight);
	setDisplayOrder(kMonitorLayer);
}

void ReactorChoiceHighlight::initReactorChoiceHighlight() {
	_colors.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kReactorChoiceHilitePICTID);
	startDisplaying();
	show();
}

void ReactorChoiceHighlight::disposeReactorChoiceHighlight() {
	stopDisplaying();
	_colors.deallocateSurface();
}

void ReactorChoiceHighlight::draw(const Common::Rect &) {
	if (_colors.isSurfaceValid()) {
		for (int i = 0; i < 5; ++i) {
			if (_choices.getFlag(i)) {
				Common::Rect r1(0, 0, kChoiceHiliteLefts[i + 1] - kChoiceHiliteLefts[i], kReactorChoiceHiliteHeight);
				Common::Rect r2 = r1;
				r1.moveTo(kChoiceHiliteLefts[i], 0);
				r2.moveTo(kReactorChoiceHiliteLeft + kChoiceHiliteLefts[i], kReactorChoiceHiliteTop);
				_colors.copyToCurrentPort(r1, r2);
			}
		}
	}
}

static const CoordType kReactorHistoryWidth = 128;
static const CoordType kReactorHistoryHeight = 168;

static const CoordType kColorWidths[5] = { 24, 25, 25, 26, 27 };
static const CoordType kColorHeights[5] = { 14, 15, 17, 17, 19};

static const CoordType kHistoryLefts[5][3] = {
	{ 302 + kNavAreaLeft, 329 + kNavAreaLeft, 357 + kNavAreaLeft },
	{ 302 + kNavAreaLeft, 331 + kNavAreaLeft, 360 + kNavAreaLeft },
	{ 303 + kNavAreaLeft, 333 + kNavAreaLeft, 363 + kNavAreaLeft },
	{ 304 + kNavAreaLeft, 335 + kNavAreaLeft, 366 + kNavAreaLeft },
	{ 305 + kNavAreaLeft, 337 + kNavAreaLeft, 369 + kNavAreaLeft }
};

static const CoordType kHistoryTops[5] = {
	39 + kNavAreaTop,
	61 + kNavAreaTop,
	84 + kNavAreaTop,
	110 + kNavAreaTop,
	137 + kNavAreaTop
};

static const CoordType kOneAnswerWidth = 35;
static const CoordType kOneAnswerHeight = 27;

static const CoordType kDigitWidth = 16;
static const CoordType kDigitHeight = 12;

static const CoordType kCorrectCountLefts[5] = {
	388 + kNavAreaLeft,
	392 + kNavAreaLeft,
	398 + kNavAreaLeft,
	402 + kNavAreaLeft,
	406 + kNavAreaLeft
};

static const CoordType kCorrectCountTops[5] = {
	40 + kNavAreaTop,
	62 + kNavAreaTop,
	86 + kNavAreaTop,
	112 + kNavAreaTop,
	140 + kNavAreaTop
};

static const ResIDType kReactorDigitsPICTID = 902;
static const ResIDType kReactorHistoryPICTID = 903;
static const ResIDType kReactorAnswerPICTID = 904;

static const CoordType kReactorHistoryLeft = kNavAreaLeft + 302;
static const CoordType kReactorHistoryTop = kNavAreaTop + 39;

static const CoordType kAnswerLeft = kNavAreaLeft + 304;
static const CoordType kAnswerTop = kNavAreaTop + 180;

ReactorHistory::ReactorHistory(const DisplayElementID id) : DisplayElement(id) {
	setBounds(kReactorHistoryLeft, kReactorHistoryTop, kReactorHistoryLeft + kReactorHistoryWidth,
			kReactorHistoryTop + kReactorHistoryHeight);
	setDisplayOrder(kMonitorLayer);
	_numGuesses = 0;
	_answer[0] = -1;
	_answer[1] = -1;
	_answer[2] = -1;
	_showAnswer = false;
}

void ReactorHistory::initReactorHistory() {
	_colors.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kReactorHistoryPICTID);
	_digits.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kReactorDigitsPICTID);
	_answerColors.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kReactorAnswerPICTID);
	startDisplaying();
	show();
}

void ReactorHistory::disposeReactorHistory() {
	stopDisplaying();
	_colors.deallocateSurface();
}

void ReactorHistory::addGuess(int32 a, int32 b, int32 c) {
	_history[_numGuesses][0] = a;
	_history[_numGuesses][1] = b;
	_history[_numGuesses][2] = c;
	_numGuesses++;
	triggerRedraw();
}

void ReactorHistory::clearHistory() {
	_numGuesses = 0;
	_showAnswer = false;
	triggerRedraw();
}

void ReactorHistory::setAnswer(int32 a, int32 b, int32 c) {
	_answer[0] = a;
	_answer[1] = b;
	_answer[2] = c;
}

void ReactorHistory::showAnswer() {
	_showAnswer = true;
	triggerRedraw();
}

bool ReactorHistory::isSolved() {
	for (int i = 0; i < _numGuesses; i++)
		if (_history[i][0] == _answer[0] && _history[i][1] == _answer[1] && _history[i][2] == _answer[2])
			return true;

	return false;
}

void ReactorHistory::draw(const Common::Rect &) {
	static const CoordType kColorTops[5] = {
		0,
		kColorHeights[0],
		kColorHeights[0] + kColorHeights[1],
		kColorHeights[0] + kColorHeights[1] + kColorHeights[2],
		kColorHeights[0] + kColorHeights[1] + kColorHeights[2] + kColorHeights[3],
	};

	if (_colors.isSurfaceValid() && _digits.isSurfaceValid()) {
		for (int i = 0; i < _numGuesses; ++i) {
			Common::Rect r1(0, 0, kColorWidths[i], kColorHeights[i]);
			Common::Rect r2 = r1;
			Common::Rect r3(0, 0, kDigitWidth, kDigitHeight);
			Common::Rect r4 = r3;
			int correct = 0;

			for (int j = 0; j < 3; ++j) {
				r1.moveTo(kColorWidths[i] * _history[i][j], kColorTops[i]);
				r2.moveTo(kHistoryLefts[i][j], kHistoryTops[i]);
				_colors.copyToCurrentPortTransparent(r1, r2);

				if (_history[i][j] == _answer[j])
					correct++;
			}

			r3.moveTo(kDigitWidth * correct, 0);
			r4.moveTo(kCorrectCountLefts[i], kCorrectCountTops[i]);
			_digits.copyToCurrentPort(r3, r4);
		}

		if (_showAnswer && _answerColors.isSurfaceValid()) {
			Common::Rect r1(0, 0, kOneAnswerWidth, kOneAnswerHeight);
			Common::Rect r2 = r1;

			for (int i = 0; i < 3; i++) {
				r1.moveTo(kOneAnswerWidth * _answer[i], 0);
				r2.moveTo(kAnswerLeft + 34 * i, kAnswerTop);
				_answerColors.copyToCurrentPortTransparent(r1, r2);
			}
		}
	}
}

int32 ReactorHistory::getCurrentNumCorrect() {
	int correct = 0;

	for (int i = 0; i < 3; i++)
		if (_history[_numGuesses - 1][i] == _answer[i])
			correct++;

	return correct;
}

} // End of namespace Pegasus
