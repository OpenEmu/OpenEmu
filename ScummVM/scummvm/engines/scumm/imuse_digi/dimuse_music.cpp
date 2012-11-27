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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_tables.h"

namespace Scumm {

#define DIG_STATE_OFFSET 11
#define DIG_SEQ_OFFSET (DIG_STATE_OFFSET + 65)
#define COMI_STATE_OFFSET 3

void IMuseDigital::setDigMusicState(int stateId) {
	int l, num = -1;

	for (l = 0; _digStateMusicTable[l].soundId != -1; l++) {
		if ((_digStateMusicTable[l].soundId == stateId)) {
			debug(5, "Set music state: %s, %s", _digStateMusicTable[l].name, _digStateMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1) {
		for (l = 0; _digStateMusicMap[l].roomId != -1; l++) {
			if ((_digStateMusicMap[l].roomId == stateId)) {
				break;
			}
		}
		num = l;

		int offset = _attributes[_digStateMusicMap[num].offset];
		if (offset == 0) {
			if (_attributes[_digStateMusicMap[num].attribPos] != 0) {
				num = _digStateMusicMap[num].stateIndex3;
			} else {
				num = _digStateMusicMap[num].stateIndex1;
			}
		} else {
			int stateIndex2 = _digStateMusicMap[num].stateIndex2;
			if (stateIndex2 == 0) {
				num = _digStateMusicMap[num].stateIndex1 + offset;
			} else {
				num = stateIndex2;
			}
		}
	}

	debug(5, "Set music state: %s, %s", _digStateMusicTable[num].name, _digStateMusicTable[num].filename);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playDigMusic(NULL, &_digStateMusicTable[0], num, false);
		else
			playDigMusic(_digStateMusicTable[num].name, &_digStateMusicTable[num], num, false);
	}

	_curMusicState = num;
}

void IMuseDigital::setDigMusicSequence(int seqId) {
	int l, num = -1;

	if (seqId == 0)
		seqId = 2000;

	for (l = 0; _digSeqMusicTable[l].soundId != -1; l++) {
		if ((_digSeqMusicTable[l].soundId == seqId)) {
			debug(5, "Set music sequence: %s, %s", _digSeqMusicTable[l].name, _digSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq && ((_digSeqMusicTable[_curMusicSeq].transitionType == 4)
				|| (_digSeqMusicTable[_curMusicSeq].transitionType == 6))) {
			_nextSeqToPlay = num;
			return;
		} else {
			playDigMusic(_digSeqMusicTable[num].name, &_digSeqMusicTable[num], 0, true);
			_nextSeqToPlay = 0;
			_attributes[DIG_SEQ_OFFSET + num] = 1; // _attributes[COMI_SEQ_OFFSET] in Comi are not used as it doesn't have 'room' attributes table
		}
	} else {
		if (_nextSeqToPlay != 0) {
			playDigMusic(_digSeqMusicTable[_nextSeqToPlay].name, &_digSeqMusicTable[_nextSeqToPlay], 0, true);
			_attributes[DIG_SEQ_OFFSET + _nextSeqToPlay] = 1; // _attributes[COMI_SEQ_OFFSET] in Comi are not used as it doesn't have 'room' attributes table
			num = _nextSeqToPlay;
			_nextSeqToPlay = 0;
		} else {
			if (_curMusicState != 0) {
				playDigMusic(_digStateMusicTable[_curMusicState].name, &_digStateMusicTable[_curMusicState], _curMusicState, true);
			} else
				playDigMusic(NULL, &_digStateMusicTable[0], _curMusicState, true);
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playDigMusic(const char *songName, const imuseDigTable *table, int attribPos, bool sequence) {
	int hookId = 0;

	if (songName != NULL) {
		if ((_attributes[DIG_SEQ_OFFSET + 38]) && (!_attributes[DIG_SEQ_OFFSET + 41])) {
			if ((attribPos == 43) || (attribPos == 44))
				hookId = 3;
		}

		if ((_attributes[DIG_SEQ_OFFSET + 46] != 0) && (_attributes[DIG_SEQ_OFFSET + 48] == 0)) {
			if ((attribPos == 38) || (attribPos == 39))
				hookId = 3;
		}

		if ((_attributes[DIG_SEQ_OFFSET + 53] != 0)) {
			if ((attribPos == 50) || (attribPos == 51))
				hookId = 3;
		}

		if ((attribPos != 0) && (hookId == 0)) {
			if (table->attribPos != 0)
				attribPos = table->attribPos;
			hookId = _attributes[DIG_STATE_OFFSET + attribPos];
			if (table->hookId != 0) {
				if ((hookId != 0) && (table->hookId > 1)) {
					_attributes[DIG_STATE_OFFSET + attribPos] = 2;
				} else {
					_attributes[DIG_STATE_OFFSET + attribPos] = hookId + 1;
					if (table->hookId < hookId + 1)
						_attributes[DIG_STATE_OFFSET + attribPos] = 1;
				}
			}
		}
	}

	if (!songName) {
		fadeOutMusic(120);
		return;
	}

	switch (table->transitionType) {
	case 0:
	case 5:
		break;
	case 3:
	case 4:
		if (table->filename[0] == 0) {
			fadeOutMusic(60);
			return;
		}
		if (table->transitionType == 4)
			_stopingSequence = 1;
		if ((!sequence) && (table->attribPos != 0) &&
				(table->attribPos == _digStateMusicTable[_curMusicState].attribPos)) {
			fadeOutMusicAndStartNew(108, table->filename, table->soundId);
		} else {
			fadeOutMusic(108);
			startMusic(table->filename, table->soundId, hookId, 127);
		}
		break;
	case 6:
		_stopingSequence = 1;
		break;
	}
}

void IMuseDigital::setComiMusicState(int stateId) {
	int l, num = -1;

	if (stateId == 4) // look into #1881415 bug, ignore stateId == 4 it's seems needed after all
		return;

	if (stateId == 0)
		stateId = 1000;

	for (l = 0; _comiStateMusicTable[l].soundId != -1; l++) {
		if ((_comiStateMusicTable[l].soundId == stateId)) {
			debug(5, "Set music state: %s, %s", _comiStateMusicTable[l].name, _comiStateMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playComiMusic(NULL, &_comiStateMusicTable[0], num, false);
		else
			playComiMusic(_comiStateMusicTable[num].name, &_comiStateMusicTable[num], num, false);
	}

	_curMusicState = num;
}

void IMuseDigital::setComiMusicSequence(int seqId) {
	int l, num = -1;

	if (seqId == 0)
		seqId = 2000;

	for (l = 0; _comiSeqMusicTable[l].soundId != -1; l++) {
		if ((_comiSeqMusicTable[l].soundId == seqId)) {
			debug(5, "Set music sequence: %s, %s", _comiSeqMusicTable[l].name, _comiSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq && ((_comiSeqMusicTable[_curMusicSeq].transitionType == 4)
				|| (_comiSeqMusicTable[_curMusicSeq].transitionType == 6))) {
			_nextSeqToPlay = num;
			return;
		} else {
			playComiMusic(_comiSeqMusicTable[num].name, &_comiSeqMusicTable[num], 0, true);
			_nextSeqToPlay = 0;
		}
	} else {
		if (_nextSeqToPlay != 0) {
			playComiMusic(_comiSeqMusicTable[_nextSeqToPlay].name, &_comiSeqMusicTable[_nextSeqToPlay], 0, true);
			num = _nextSeqToPlay;
			_nextSeqToPlay = 0;
		} else {
			if (_curMusicState != 0) {
				playComiMusic(_comiStateMusicTable[_curMusicState].name, &_comiStateMusicTable[_curMusicState], _curMusicState, true);
			} else
				playComiMusic(NULL, &_comiStateMusicTable[0], _curMusicState, true);
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playComiMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence) {
	int hookId = 0;

	if ((songName != NULL) && (attribPos != 0)) {
		if (table->attribPos != 0)
			attribPos = table->attribPos;
		hookId = _attributes[COMI_STATE_OFFSET + attribPos];
		if (table->hookId != 0) {
			if ((hookId != 0) && (table->hookId > 1)) {
				_attributes[COMI_STATE_OFFSET + attribPos] = 2;
			} else {
				_attributes[COMI_STATE_OFFSET + attribPos] = hookId + 1;
				if (table->hookId < hookId + 1)
					_attributes[COMI_STATE_OFFSET + attribPos] = 1;
			}
		}
	}

	if (!songName) {
		fadeOutMusic(120);
		return;
	}

	switch (table->transitionType) {
	case 0:
		break;
	case 8:
		setHookIdForMusic(table->hookId);
		break;
	case 9:
		_stopingSequence = 1;
		setHookIdForMusic(table->hookId);
		break;
	case 2:
	case 3:
	case 4:
	case 12:
		if (table->filename[0] == 0) {
			fadeOutMusic(60);
			return;
		}
		if (getCurMusicSoundId() == table->soundId)
			return;
		if (table->transitionType == 4)
			_stopingSequence = 1;
		if (table->transitionType == 2) {
			fadeOutMusic(table->fadeOutDelay);
			startMusic(table->filename, table->soundId, table->hookId, 127);
			return;
		}
		if ((!sequence) && (table->attribPos != 0) &&
				(table->attribPos == _comiStateMusicTable[_curMusicState].attribPos)) {
			fadeOutMusicAndStartNew(table->fadeOutDelay, table->filename, table->soundId);
		} else if (table->transitionType == 12) {
			TriggerParams trigger;
			strcpy(trigger.marker, "exit"); trigger.fadeOutDelay = table->fadeOutDelay;
			strcpy(trigger.filename, table->filename); trigger.soundId = table->soundId;
			trigger.hookId = table->hookId; trigger.volume = 127;
			setTrigger(&trigger);
		} else {
			fadeOutMusic(table->fadeOutDelay);
			startMusic(table->filename, table->soundId, hookId, 127);
		}
		break;
	}
}

void IMuseDigital::setFtMusicState(int stateId) {
	if (stateId > 48)
		return;

	debug(5, "State music: %s, %s", _ftStateMusicTable[stateId].name, _ftStateMusicTable[stateId].audioName);

	if (_curMusicState == stateId)
		return;

	if (_curMusicSeq == 0) {
		if (stateId == 0)
			playFtMusic(NULL, 0, 0);
		else
			playFtMusic(_ftStateMusicTable[stateId].audioName, _ftStateMusicTable[stateId].transitionType, _ftStateMusicTable[stateId].volume);
	}

	_curMusicState = stateId;
}

void IMuseDigital::setFtMusicSequence(int seqId) {
	if (seqId > 52)
		return;

	debug(5, "Sequence music: %s", _ftSeqNames[seqId].name);

	if (_curMusicSeq == seqId)
		return;

	if (seqId == 0) {
		if (_curMusicState == 0)
			playFtMusic(NULL, 0, 0);
		else {
			playFtMusic(_ftStateMusicTable[_curMusicState].audioName, _ftStateMusicTable[_curMusicState].transitionType, _ftStateMusicTable[_curMusicState].volume);
		}
	} else {
		int seq = (seqId - 1) * 4;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].transitionType, _ftSeqMusicTable[seq].volume);
	}

	_curMusicSeq = seqId;
	_curMusicCue = 0;
}

void IMuseDigital::setFtMusicCuePoint(int cueId) {
	if (cueId > 3)
		return;

	debug(5, "Cue point sequence: %d", cueId);

	if (_curMusicSeq == 0)
		return;

	if (_curMusicCue == cueId)
		return;

	if (cueId == 0)
		playFtMusic(NULL, 0, 0);
	else {
		int seq = ((_curMusicSeq - 1) * 4) + cueId;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].transitionType, _ftSeqMusicTable[seq].volume);
	}

	_curMusicCue = cueId;
}

void IMuseDigital::setAudioNames(int32 num, char *names) {
	free(_audioNames);
	_numAudioNames = num;
	_audioNames = names;
}

int IMuseDigital::getSoundIdByName(const char *soundName) {
	if (soundName && soundName[0] != 0) {
		for (int r = 0; r < _numAudioNames; r++) {
			if (strcmp(soundName, &_audioNames[r * 9]) == 0) {
				return r;
			}
		}
	}

	return -1;
}

void IMuseDigital::playFtMusic(const char *songName, int opcode, int volume) {
	fadeOutMusic(200);

	switch (opcode) {
	case 0:
	case 4:
		break;
	case 1:
	case 2:
	case 3:
		{
			int soundId = getSoundIdByName(songName);
			if (soundId != -1) {
				startMusic(soundId, volume);
			}
		}
		break;
	}
}


} // End of namespace Scumm
