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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "pegasus/fader.h"
#include "pegasus/pegasus.h"
#include "pegasus/sound.h"
#include "pegasus/util.h"

namespace Pegasus {

Fader::Fader() {
	_currentValue = 0;
	_currentFaderMove._numKnots = 0;
}

void Fader::setFaderValue(const int32 newValue) {
	_currentValue = newValue;
}

bool Fader::initFaderMove(const FaderMoveSpec &spec) {
	bool faderMoves = false;
	int32 value = 0;

	if (spec._numKnots > 0) {
		stopFader();
		value = spec._knots[0].knotValue;
		TimeValue startTime = spec._knots[0].knotTime;

		if (startTime != 0xffffffff) {
			if (spec._numKnots > 1) {
				TimeValue stopTime = spec._knots[spec._numKnots - 1].knotTime;

				if (spec._faderScale > 0) {
					if (stopTime > startTime) {
						for (uint32 i = 1; i < spec._numKnots; ++i) {
							if (spec._knots[i - 1].knotValue != spec._knots[i].knotValue) {
								faderMoves = true;
								break;
							}
						}

						if (faderMoves)
							_currentFaderMove = spec;
					} else if (spec._knots[spec._numKnots - 1].knotValue != value) {
						value = spec._knots[spec._numKnots - 1].knotValue;
					}
				}
			}
		}
	}

	setFaderValue(value);
	return faderMoves;
}

void Fader::startFader(const FaderMoveSpec &spec) {
	if (initFaderMove(spec)) {
		setFlags(0);
		setScale(spec._faderScale);
		setSegment(spec._knots[0].knotTime, spec._knots[spec._numKnots - 1].knotTime);
		setTime(spec._knots[0].knotTime);
		start();
	}
}

void Fader::startFaderSync(const FaderMoveSpec &spec) {
	if (initFaderMove(spec)) {
		setFlags(0);
		setScale(spec._faderScale);
		setSegment(spec._knots[0].knotTime, spec._knots[spec._numKnots - 1].knotTime);
		setTime(spec._knots[0].knotTime);
		start();

		while (isFading()) {
			((PegasusEngine *)g_engine)->checkCallBacks();
			useIdleTime();
		}

		// Once more, for good measure, to make sure that there are no boundary
		// condition problems.
		useIdleTime();
		stopFader();
	}
}

void Fader::loopFader(const FaderMoveSpec &spec) {
	if (initFaderMove(spec)) {
		setFlags(kLoopTimeBase);
		setScale(spec._faderScale);
		setSegment(spec._knots[0].knotTime, spec._knots[spec._numKnots - 1].knotTime);
		setTime(spec._knots[0].knotTime);
		start();
	}
}

void Fader::stopFader() {
	stop();
}

void Fader::pauseFader() {
	stopFader();
}

void Fader::continueFader() {
	if (getTime() < getStop())
		start();
}

void Fader::timeChanged(const TimeValue newTime) {
	if (_currentFaderMove._numKnots != 0) {
		uint32 i;
		for (i = 0; i < _currentFaderMove._numKnots; i++)
			if (_currentFaderMove._knots[i].knotTime > newTime)
				break;

		int32 newValue;
		if (i == 0)
			newValue = _currentFaderMove._knots[0].knotValue;
		else if (i == _currentFaderMove._numKnots)
			newValue = _currentFaderMove._knots[i - 1].knotValue;
		else
			newValue = linearInterp(_currentFaderMove._knots[i - 1].knotTime, _currentFaderMove._knots[i].knotTime, newTime, _currentFaderMove._knots[i - 1].knotValue, _currentFaderMove._knots[i].knotValue);

		if (newValue != _currentValue)
			setFaderValue(newValue);
	}
}

void FaderMoveSpec::makeOneKnotFaderSpec(const int32 knotValue) {
	_numKnots = 1;
	_knots[0].knotTime = 0;
	_knots[0].knotValue = knotValue;
}

void FaderMoveSpec::makeTwoKnotFaderSpec(const TimeScale faderScale, const TimeValue time1, const int32 value1, const TimeValue time2, const int32 value2) {
	_numKnots = 2;
	_faderScale = faderScale;
	_knots[0].knotTime = time1;
	_knots[0].knotValue = value1;
	_knots[1].knotTime = time2;
	_knots[1].knotValue = value2;
}

void FaderMoveSpec::insertFaderKnot(const TimeValue knotTime, const int32 knotValue) {
	if (_numKnots != kMaxFaderKnots) {
		uint32 index;
		for (index = 0; index < _numKnots; index++) {
			if (knotTime == _knots[index].knotTime) {
				_knots[index].knotValue = knotValue;
				return;
			} else if (knotTime < _knots[index].knotTime) {
				break;
			}
		}

		for (uint32 i = _numKnots; i > index; i--)
			_knots[i] = _knots[i - 1];

		_knots[index].knotTime = knotTime;
		_knots[index].knotValue = knotValue;
		_numKnots++;
	}
}

void FaderAnimation::setFaderValue(const int32 newValue) {
	if (getFaderValue() != newValue) {
		Fader::setFaderValue(newValue);
		triggerRedraw();
	}
}

SoundFader::SoundFader() {
	_sound = 0;
	_masterVolume = 0xff;
}

void SoundFader::attachSound(Sound *sound) {
	if (!sound && isFading())
		stopFader();

	_sound = sound;
}

void SoundFader::setFaderValue(const int32 newVolume) {
	if (_sound)
		_sound->setVolume((newVolume * _masterVolume) >> 8);

	_currentValue = newVolume;
}

void SoundFader::setMasterVolume(const uint16 masterVolume) {
	_masterVolume = masterVolume;
	setFaderValue(getFaderValue());
}

} // End of namespace Pegasus
