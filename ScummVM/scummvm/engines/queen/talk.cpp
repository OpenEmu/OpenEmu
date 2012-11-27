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


#include "common/rect.h"
#include "common/textconsole.h"

#include "queen/talk.h"

#include "queen/bankman.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/state.h"
#include "queen/walk.h"

#include "common/file.h"

namespace Queen {

void Talk::talk(
		const char *filename,
		int personInRoom,
		char *cutawayFilename,
		QueenEngine *vm) {
	Talk *talk = new Talk(vm);
	talk->talk(filename, personInRoom, cutawayFilename);
	delete talk;
}

bool Talk::speak(
		const char *sentence,
		Person *person,
		const char *voiceFilePrefix,
		QueenEngine *vm) {
	Talk *talk = new Talk(vm);
	bool result;
	if (sentence)
		result = talk->speak(sentence, person, voiceFilePrefix);
	else
		result = false;
	delete talk;
	return result;
}

Talk::Talk(QueenEngine *vm)
	: _vm(vm), _fileData(NULL) {
	_vm->input()->talkQuitReset();
}

Talk::~Talk() {
	delete[] _fileData;
}

void Talk::talk(const char *filename, int personInRoom, char *cutawayFilename) {
	int i;
	_oldSelectedSentenceIndex = 0;
	_oldSelectedSentenceValue = 0;

	debug(6, "----- talk(\"%s\") -----", filename);

	cutawayFilename[0] = '\0';

	load(filename);

	Person person;
	memset(&person, 0, sizeof(Person));
	_vm->logic()->initPerson(personInRoom, "", false, &person);

	if (NULL == person.name) {
		error("Invalid person object");
	}

	int16 oldLevel = 0;

	// Lines 828-846 in talk.c
	for (i = 1; i <= 4; i++) {
		if (selectedValue(i) > 0) {
			// This option has been redefined so display new dialogue option
			_dialogueTree[1][i].head = selectedValue(i);
		} else if (selectedValue(i) == -1) {
			// Already selected so don't redisplay
			if (_dialogueTree[1][i].gameStateIndex >= 0) {
				_dialogueTree[1][i].head = -1;
				_dialogueTree[1][i].dialogueNodeValue1 = -1;
				_dialogueTree[1][i].gameStateIndex = -1;
				_dialogueTree[1][i].gameStateValue = -1;
			}
		}
	}

	initialTalk();

	// Lines 906-? in talk.c
	_vm->display()->showMouseCursor(true);

	int16 level=1, retval=0;
	int16 head = _dialogueTree[level][0].head;

	// TODO: split this loop in several functions
	while (retval != -1) {
		char otherVoiceFilePrefix[MAX_STRING_SIZE];

		_talkString[0][0] = '\0';

		if (hasTalkedTo() && head == 1)
			strcpy(_talkString[0], _person2String);
		else
			findDialogueString(_person1PtrOff, head, _pMax, _talkString[0]);

		if (hasTalkedTo() && head == 1)
			sprintf(otherVoiceFilePrefix, "%2dXXXXP", _talkKey);
		else
			sprintf(otherVoiceFilePrefix, "%2d%4xP", _talkKey, head);

		if (_talkString[0][0] == '\0' && retval > 1) {
			findDialogueString(_person1PtrOff, retval, _pMax, _talkString[0]);
			sprintf(otherVoiceFilePrefix,"%2d%4xP", _talkKey, retval);
		}

		// Joe dialogue

		for (i = 1; i <= 4; i++) {
			findDialogueString(_joePtrOff, _dialogueTree[level][i].head, _jMax, _talkString[i]);

			int16 index = _dialogueTree[level][i].gameStateIndex;

			if (index < 0 && _vm->logic()->gameState(ABS(index)) != _dialogueTree[level][i].gameStateValue)
				_talkString[i][0] = '\0';

			sprintf(_joeVoiceFilePrefix[i], "%2d%4xJ", _talkKey, _dialogueTree[level][i].head);
		}

		// Check to see if (all the dialogue options have been selected.
		// if this is the case, and the last one left is the exit option,
		// then automatically set S to that and exit.

		int choicesLeft = 0;
		int selectedSentence = 0;

		for (i = 1; i <= 4; i++) {
			if (_talkString[i][0] != '\0') {
				choicesLeft++;
				selectedSentence = i;
			}
		}

		debug(6, "choicesLeft = %i", choicesLeft);

		if (1 == choicesLeft) {
			// Automatically run the final dialogue option
			speak(_talkString[0], &person, otherVoiceFilePrefix);

			if (_vm->input()->talkQuit())
				break;

			speak(_talkString[selectedSentence], NULL, _joeVoiceFilePrefix[selectedSentence]);
		} else {
			if (person.actor->bobNum > 0) {
				speak(_talkString[0], &person, otherVoiceFilePrefix);
				selectedSentence = selectSentence();
			} else {
				warning("bobBum is %i", person.actor->bobNum);
				selectedSentence = 0;
			}
		}

		if (_vm->input()->talkQuit())
			break;

		retval   = _dialogueTree[level][selectedSentence].dialogueNodeValue1;
		head     = _dialogueTree[level][selectedSentence].head;
		oldLevel = level;
		level    = 0;

		// Set LEVEL to the selected child in dialogue tree

		for (i = 1; i <= _levelMax; i++)
			if (_dialogueTree[i][0].head == head)
				level = i;

		if (0 == level) {
			// No new level has been selected, so lets set LEVEL to the
			// tree path pointed to by the RETVAL

			for (i = 1; i <= _levelMax; i++)
				for (int j = 0; j <= 5; j++)
					if (_dialogueTree[i][j].head == retval)
						level = i;

			disableSentence(oldLevel, selectedSentence);
		} else { // 0 != level
			// Check to see if Person Return value is positive, if it is, then
			// change the selected dialogue option to the Return value

			if (_dialogueTree[level][0].dialogueNodeValue1 > 0) {
				if (1 == oldLevel) {
						_oldSelectedSentenceIndex = selectedSentence;
						_oldSelectedSentenceValue = selectedValue(selectedSentence);
						selectedValue(selectedSentence, _dialogueTree[level][0].dialogueNodeValue1);
				}

				_dialogueTree[oldLevel][selectedSentence].head = _dialogueTree[level][0].dialogueNodeValue1;
				_dialogueTree[level][0].dialogueNodeValue1 = -1;
			} else {
				disableSentence(oldLevel, selectedSentence);
			}
		}

		// Check selected person to see if any Gamestates need setting

		int16 index = _dialogueTree[level][0].gameStateIndex;
		if (index > 0)
			_vm->logic()->gameState(index, _dialogueTree[level][0].gameStateValue);

		// if the selected dialogue line has a POSITIVE game state value
		// then set gamestate to Value = TALK(OLDLEVEL,S,3)

		index = _dialogueTree[oldLevel][selectedSentence].gameStateIndex;
		if (index > 0)
			_vm->logic()->gameState(index, _dialogueTree[oldLevel][selectedSentence].gameStateValue);

		// check to see if person has something final to say
		if (-1 == retval) {
			findDialogueString(_person1PtrOff, head, _pMax, _talkString[0]);
			if (_talkString[0][0] != '\0') {
				sprintf(otherVoiceFilePrefix, "%2d%4xP", _talkKey, head);
				speak(_talkString[0], &person, otherVoiceFilePrefix);
			}
		}
	}

	cutawayFilename[0] = '\0';

	for (i = 0; i < 2; i++) {
		if (_gameState[i] > 0) {
			if (_vm->logic()->gameState(_gameState[i]) == _testValue[i]) {
				if (_itemNumber[i] > 0)
					_vm->logic()->inventoryInsertItem(_itemNumber[i]);
				else
					_vm->logic()->inventoryDeleteItem(ABS(_itemNumber[i]));
			}
		}
	}

	_vm->grid()->setupPanel();

	uint16 offset = _cutawayPtrOff;

	int16 cutawayGameState = (int16)READ_BE_INT16(_fileData + offset); offset += 2;
	int16 cutawayTestValue = (int16)READ_BE_INT16(_fileData + offset); offset += 2;

	if (_vm->logic()->gameState(cutawayGameState) == cutawayTestValue) {
		getString(_fileData, offset, cutawayFilename, 20);
		if (cutawayFilename[0]) {
			//CR 2 - 7/3/95, If we're executing a cutaway scene, then make sure
			// Joe can talk, so set TALKQUIT to 0 just in case we exit on the
			// line that set's the cutaway game states.
			_vm->input()->talkQuitReset();
		}
	}
	if (_vm->input()->talkQuit()) {
		if (_oldSelectedSentenceIndex > 0)
			selectedValue(_oldSelectedSentenceIndex, _oldSelectedSentenceValue);
		_vm->input()->talkQuitReset();
		_vm->display()->clearTexts(0, 198);
		_vm->logic()->makeJoeSpeak(15, false);
	} else {
		setHasTalkedTo();
	}

	_vm->logic()->joeFace();

	if (cutawayFilename[0] == '\0') {
		BobSlot *pbs = _vm->graphics()->bob(person.actor->bobNum);

		pbs->x = person.actor->x;
		pbs->y = person.actor->y;

		// Better kick start the persons anim sequence
		_vm->graphics()->resetPersonAnim(person.actor->bobNum);
	}

	_vm->logic()->joeWalk(JWM_NORMAL);
}

void Talk::disableSentence(int oldLevel, int selectedSentence) {
	// Mark off selected option

	if (1 == oldLevel) {
		if (_dialogueTree[oldLevel][selectedSentence].dialogueNodeValue1 != -1) {
			// Make sure choice is not exit option
			_oldSelectedSentenceIndex = selectedSentence;
			_oldSelectedSentenceValue = selectedValue(selectedSentence);
			selectedValue(selectedSentence, -1);
		}
	}

	// Cancel selected dialogue line, so that its no longer displayed
	_dialogueTree[oldLevel][selectedSentence].head = -1;
	_dialogueTree[oldLevel][selectedSentence].dialogueNodeValue1 = -1;
}

void Talk::findDialogueString(uint16 offset, int16 id, int16 max, char *str) {
	str[0] = '\0';
	for (int i = 1; i <= max; i++) {
		offset += 2;
		int16 currentId = (int16)READ_BE_INT16(_fileData + offset);
		offset += 2;
		if (id == currentId) {
			getString(_fileData, offset, str, MAX_STRING_LENGTH, 4);
			break;
		} else {
			getString(_fileData, offset, NULL, MAX_STRING_LENGTH, 4);
		}
	}
}

byte *Talk::loadDialogFile(const char *filename) {
	static const struct {
		const char *filename;
		Common::Language language;
	} dogFiles[] = {
		{ "CHIEF1.DOG", Common::FR_FRA },
		{ "CHIEF2.DOG", Common::FR_FRA },
		{ "BUD1.DOG",   Common::IT_ITA }
	};
	for (int i = 0; i < ARRAYSIZE(dogFiles); ++i) {
		if (!scumm_stricmp(filename, dogFiles[i].filename) &&
			_vm->resource()->getLanguage() == dogFiles[i].language) {
			Common::File fdog;
			fdog.open(filename);
			if (fdog.isOpen()) {
				debug(6, "Loading dog file '%s' from game data path", filename);
				uint32 size = fdog.size() - DOG_HEADER_SIZE;
				byte *buf = new byte[size];
				fdog.seek(DOG_HEADER_SIZE);
				fdog.read(buf, size);
				return buf;
			}
		}
	}
	return _vm->resource()->loadFile(filename, DOG_HEADER_SIZE);
}

void Talk::load(const char *filename) {
	int i;
	byte *ptr = _fileData = loadDialogFile(filename);

	// Load talk header

	_levelMax = (int16)READ_BE_INT16(ptr); ptr += 2;

	if (_levelMax < 0) {
		_levelMax = -_levelMax;
		_vm->input()->canQuit(false);
	} else {
		_vm->input()->canQuit(true);
	}

	_uniqueKey      = (int16)READ_BE_INT16(ptr); ptr += 2;
	_talkKey        = (int16)READ_BE_INT16(ptr); ptr += 2;
	_jMax           = (int16)READ_BE_INT16(ptr); ptr += 2;
	_pMax           = (int16)READ_BE_INT16(ptr); ptr += 2;

	for (i = 0; i < 2; i++) {
		_gameState [i] = (int16)READ_BE_INT16(ptr); ptr += 2;
		_testValue [i] = (int16)READ_BE_INT16(ptr); ptr += 2;
		_itemNumber[i] = (int16)READ_BE_INT16(ptr); ptr += 2;
	}

	_person1PtrOff = READ_BE_UINT16(ptr); ptr += 2;
	_cutawayPtrOff = READ_BE_UINT16(ptr); ptr += 2;
	_person2PtrOff = READ_BE_UINT16(ptr); ptr += 2;
	_joePtrOff     = 32 + _levelMax * 96;

	// Load dialogue tree
	ptr = _fileData + 32;
	memset(&_dialogueTree[0], 0, sizeof(_dialogueTree[0]));
	for (i = 1; i <= _levelMax; i++)
		for (int j = 0; j <= 5; j++) {
			ptr += 2;
			_dialogueTree[i][j].head = (int16)READ_BE_INT16(ptr); ptr += 2;
			ptr += 2;
			_dialogueTree[i][j].dialogueNodeValue1 = (int16)READ_BE_INT16(ptr); ptr += 2;
			ptr += 2;
			_dialogueTree[i][j].gameStateIndex = (int16)READ_BE_INT16(ptr); ptr += 2;
			ptr += 2;
			_dialogueTree[i][j].gameStateValue = (int16)READ_BE_INT16(ptr); ptr += 2;
		}
}

void Talk::initialTalk() {
	// Lines 848-903 in talk.c

	uint16 offset = _joePtrOff + 2;
	uint16 hasNotString = READ_BE_UINT16(_fileData + offset); offset += 2;

	char joeString[MAX_STRING_SIZE];
	if (!hasNotString) {
		getString(_fileData, offset, joeString, MAX_STRING_LENGTH);
	} else {
		joeString[0] = '\0';
	}

	offset = _person2PtrOff;
	char joe2String[MAX_STRING_SIZE];
	getString(_fileData, offset, _person2String, MAX_STRING_LENGTH);
	getString(_fileData, offset, joe2String, MAX_STRING_LENGTH);

	if (!hasTalkedTo()) {
		// Not yet talked to this person
		if (joeString[0] != '0') {
			char voiceFilePrefix[MAX_STRING_SIZE];
			sprintf(voiceFilePrefix, "%2dSSSSJ", _talkKey);
			speak(joeString, NULL, voiceFilePrefix);
		}
	} else {
		// Already spoken to them, choose second response
		if (joe2String[0] != '0') {
			char voiceFilePrefix[MAX_STRING_SIZE];
			sprintf(voiceFilePrefix, "%2dXXXXJ", _talkKey);
			speak(joe2String, NULL, voiceFilePrefix);
		}
	}
}

int Talk::getSpeakCommand(const Person *person, const char *sentence, unsigned &index) {
	// Lines 1299-1362 in talk.c
	int commandCode = SPEAK_DEFAULT;
	uint16 id = (sentence[index] << 8) | sentence[index + 1];
	switch (id) {
	case 'AO':
		commandCode = SPEAK_AMAL_ON;
		break;
	case 'FL':
		commandCode = SPEAK_FACE_LEFT;
		break;
	case 'FF':
		commandCode = SPEAK_FACE_FRONT;
		break;
	case 'FB':
		commandCode = SPEAK_FACE_BACK;
		break;
	case 'FR':
		commandCode = SPEAK_FACE_RIGHT;
		break;
	case 'GD':
		_vm->logic()->joeGrab(STATE_GRAB_DOWN);
		commandCode = SPEAK_NONE;
		break;
	case 'GM':
		_vm->logic()->joeGrab(STATE_GRAB_MID);
		commandCode = SPEAK_NONE;
		break;
	case 'WT':
		commandCode = SPEAK_PAUSE;
		break;
	case 'XY':
		// For example *XY00(237,112)
		{
			commandCode = atoi(sentence + index + 2);
			int x = atoi(sentence + index + 5);
			int y = atoi(sentence + index + 9);
			if (0 == strcmp(person->name, "JOE"))
				_vm->walk()->moveJoe(0, x, y, _vm->input()->cutawayRunning());
			else
				_vm->walk()->movePerson(person, x, y, _vm->graphics()->numFrames(), 0);
			index += 11;
			// if (JOEWALK==3) CUTQUIT=0;
			// XXX personWalking = true;
		}
		break;
	default:
		if (sentence[index + 0] >= '0' && sentence[index + 0] <= '9' &&
				sentence[index + 1] >= '0' && sentence[index + 1] <= '9') {
			commandCode = (sentence[index] - '0') * 10 + (sentence[index + 1] - '0');
		} else
			warning("Unknown command string: '%2s'", sentence + index);
	}

	index += 2;

	return commandCode;
}


bool Talk::speak(const char *sentence, Person *person, const char *voiceFilePrefix) {
	// Function SPEAK, lines 1266-1384 in talk.c
	bool personWalking = false;
	unsigned segmentIndex = 0;
	unsigned segmentStart = 0;
	unsigned i;

	Person joe_person;
	ActorData joe_actor;

	_vm->logic()->joeWalk(JWM_SPEAK);

	if (!person) {
		// Fill in values for use by speakSegment() etc.
		memset(&joe_person, 0, sizeof(Person));
		memset(&joe_actor, 0, sizeof(ActorData));

		joe_actor.bobNum = 0;
		joe_actor.color = 14;
		joe_actor.bankNum = 7;

		joe_person.actor = &joe_actor;
		joe_person.name = "JOE";

		person = &joe_person;
	}

	debug(6, "Sentence '%s' is said by person '%s' and voice files with prefix '%s' played",
			sentence, person->name, voiceFilePrefix);

	if (sentence[0] == '\0') {
		return personWalking;
	}

	if (0 == strcmp(person->name, "FAYE-H") ||
		0 == strcmp(person->name, "FRANK-H") ||
		0 == strcmp(person->name, "AZURA-H") ||
		0 == strcmp(person->name, "X3_RITA") ||
		(0 == strcmp(person->name, "JOE") && _vm->logic()->currentRoom() == FAYE_HEAD) ||
		(0 == strcmp(person->name, "JOE") && _vm->logic()->currentRoom() == AZURA_HEAD) ||
		(0 == strcmp(person->name, "JOE") && _vm->logic()->currentRoom() == FRANK_HEAD))
		_talkHead = true;
	else
		_talkHead = false;

	for (i = 0; i < strlen(sentence); ) {
		if (sentence[i] == '*') {
			int segmentLength = i - segmentStart;

			i++;
			int command = getSpeakCommand(person, sentence, i);

			if (SPEAK_NONE != command) {
				speakSegment(
						sentence + segmentStart,
						segmentLength,
						person,
						command,
						voiceFilePrefix,
						segmentIndex);
				// XXX if (JOEWALK == 2) break
			}

			segmentIndex++;
			segmentStart = i;
		} else
			i++;

		if (_vm->input()->cutawayQuit() || _vm->input()->talkQuit())
			return personWalking;
	}

	if (segmentStart != i) {
		speakSegment(
				sentence + segmentStart,
				i - segmentStart,
				person,
				0,
				voiceFilePrefix,
				segmentIndex);
	}

	return personWalking;
}

int Talk::countSpaces(const char *segment) {
	int tmp = 0;

	while (*segment++)
		tmp++;

	if (tmp < 10)
		tmp = 10;

	return (tmp * 2) / (_vm->talkSpeed() / 3);
}

void Talk::headStringAnimation(const SpeechParameters *parameters, int bobNum, int bankNum) {
	// talk.c lines 1612-1635
	BobSlot *bob2 = _vm->graphics()->bob(2);

	if (parameters->animation[0] == 'E') {
		int offset = 1;

		BobSlot *bob  = _vm->graphics()->bob(bobNum);
		int16 x = bob->x;
		int16 y = bob->y;

		for (;;) {
			uint16 frame;

			frame = atoi(parameters->animation + offset);
			if (!frame)
				break;

			offset += 4;

			_vm->bankMan()->unpack(frame, _vm->graphics()->numFrames(), bankNum);

			bob2->frameNum = _vm->graphics()->numFrames();
			bob2->scale = 100;
			bob2->active = true;
			bob2->x = x;
			bob2->y = y;

			_vm->update();
		}
	} else
		bob2->active = false;
}

void Talk::stringAnimation(const SpeechParameters *parameters, int startFrame, int bankNum) {
	// lines 1639-1690 in talk.c

	int offset = 0;
	bool torso;

	if (parameters->animation[0] == 'T') {
		// Torso animation
		torso = true;
		_vm->bankMan()->overpack(parameters->body, startFrame, bankNum);
		offset++;
	} else if (parameters->animation[0] == 'E') {
		// Talking head animation
		return;
	} else if (!Common::isDigit(parameters->animation[0])) {
		debug(6, "Error in speak string animation: '%s'", parameters->animation);
		return;
	} else
		torso = false;

	for (;;) {
		uint16 frame;

		frame = atoi(parameters->animation + offset);
		if (!frame)
			break;

		offset += 4;

		if (frame > 500) {
			frame -= 500;
			_vm->sound()->playSfx(_vm->logic()->currentRoomSfx());
		}

		if (torso) {
			_vm->bankMan()->overpack(frame, startFrame, bankNum);
		} else {
			_vm->bankMan()->unpack(frame, startFrame, bankNum);
			// XXX bobs[BNUM].scale=SF;
		}

		_vm->update();
	}
}

void Talk::defaultAnimation(
		const char *segment,
		bool isJoe,
		const SpeechParameters *parameters,
		int startFrame,
		int bankNum) {
	// lines 1730-1823 in talk.c

	if (segment[0] != 0)  {

		// Why on earth would someone name a variable qzx?
		short qzx = 0;

		int len = countSpaces(segment);
		while (1) {
			if (parameters != NULL) {

				int bf;
				if (segment[0] == ' ')
					bf = 0;
				else
					bf = parameters->bf;

				int head;
				if (parameters->rf > 0)
					head = bf + _vm->randomizer.getRandomNumber(parameters->rf);
				else
					head = bf;

				if (bf > 0) {
					// Make the head move
					qzx ^= 1;
					if (parameters->af && qzx)
						_vm->bankMan()->overpack(parameters->af + head, startFrame, bankNum);
					else {
						_vm->bankMan()->overpack(head, startFrame, bankNum);
					}
				} else {
					debug(6, "[Talk::defaultAnimation] Body action");
					// Just do a body action
					_vm->bankMan()->overpack(parameters->body, startFrame, bankNum);
				}

				if (!_talkHead)
					_vm->update();
			} else { // (_talkHead && isJoe)
				_vm->update();
			}

			if (_vm->input()->talkQuit())
				break;

			if (_vm->logic()->joeWalk() == JWM_SPEAK) {
				_vm->update();
			} else {
				_vm->update(true);
				if (_vm->logic()->joeWalk() == JWM_EXECUTE)
					// Selected a command, so exit
					break;
			}

			// Skip through text more quickly
			if (_vm->input()->keyVerb() == VERB_SKIP_TEXT) {
				_vm->input()->clearKeyVerb();
				_vm->sound()->stopSpeech();
				break;
			}

			if (_vm->sound()->speechOn() && _vm->sound()->speechSfxExists()) {
				// sfx is finished, stop the speak animation
				if (!_vm->sound()->isSpeechActive()) {
					break;
				}
			} else {
				// no sfx, stop the animation when speak segment 'length' is 0
				--len;
				if (len <= 0) {
					break;
				}
			}
		}
	}

	// Make sure that Person closes their mouth
	if (!isJoe && parameters && parameters->ff > 0)
		_vm->bankMan()->overpack(parameters->ff, startFrame, bankNum);
}


void Talk::speakSegment(
		const char *segmentStart,
		int length,
		Person *person,
		int command,
		const char *voiceFilePrefix,
		int index)
{
	int i;
	char segment[MAX_STRING_SIZE];
	memcpy(segment, segmentStart, length);
	segment[length] = '\0';

	char voiceFileName[MAX_STRING_SIZE];
	sprintf(voiceFileName, "%s%1x", voiceFilePrefix, index + 1);

	// French talkie version has a useless voice file ;	c30e_102 file is the same as c30e_101,
	// so there is no need to play it. This voice was used in room 30 (N8) when talking to Klunk.
	if (!(_vm->resource()->getLanguage() == Common::FR_FRA && !strcmp(voiceFileName, "c30e_102")))
		_vm->sound()->playSpeech(voiceFileName);

	int faceDirectionCommand = 0;

	switch (command) {
	case SPEAK_PAUSE:
		for (i = 0; i < 10 && !_vm->input()->talkQuit() && !_vm->shouldQuit(); i++) {
			_vm->update();
		}
		return;

	case SPEAK_FACE_LEFT:
	case SPEAK_FACE_RIGHT:
	case SPEAK_FACE_FRONT:
	case SPEAK_FACE_BACK:
		faceDirectionCommand = command;
		command = 0;
		break;
	}

	bool isJoe = (0 == person->actor->bobNum);

	int16  bobNum  = person->actor->bobNum;
	uint16 color   = person->actor->color;
	uint16 bankNum = person->actor->bankNum;

	BobSlot *bob = _vm->graphics()->bob(bobNum);

	bool oracle = false;
	int textX = 0;
	int textY = 0;

	if (!isJoe) {
		if (SPEAK_AMAL_ON == command) {
			// It's the oracle!
			// Don't turn AMAL animation off, and don't manually anim person
			command = SPEAK_ORACLE;
			oracle = true;
			uint16 frameNum = _vm->graphics()->personFrames(bobNum);
			for (i = 5; i <= 8; ++i) {
				_vm->bankMan()->unpack(i, frameNum, bankNum);
				++frameNum;
			}
		} else {
			bob->animating = false;
			bob->frameNum = 31 + bobNum;
		}
	}

	if (_talkHead) {
		// talk.c lines 1491-1533
		switch (_vm->logic()->currentRoom()) {
		case FAYE_HEAD:
			textX = 15;
			if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
				color = isJoe ? 15 : 29;
			}
			break;
		case AZURA_HEAD:
			textX = 15;
			if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
				color = isJoe ? 6 : 30;
			}
			break;
		default: // FRANK_HEAD
			textX = 150;
			if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
				color = 17;
			}
			break;
		}
		textY = isJoe ? 30 : 60;
	} else {
		textX = bob->x;
		textY = bob->y;
	}

	// Set the focus rectangle
	// FIXME: This may not be correct!
	BobFrame *pbf = _vm->bankMan()->fetchFrame(bob->frameNum);

	int height = (pbf->height * bob->scale) / 100;

	Common::Rect focus(textX - 96, textY - height - 64, textX + 96, textY + height + 64);
	_vm->display()->setFocusRect(focus);


	//int SF = _vm->grid()->findScale(textX, textY);

	const SpeechParameters *parameters = NULL;
	int startFrame = 0;

	if (_talkHead && isJoe) {
		if (_vm->subtitles())
			_vm->graphics()->setBobText(bob, segment, textX, textY, color, true);
		defaultAnimation(segment, isJoe, parameters, startFrame, bankNum);
	} else {
		if (SPEAK_UNKNOWN_6 == command)
			return;

		if (isJoe) {
			if (_vm->logic()->currentRoom() == 108)
				parameters = findSpeechParameters("JOE-E", command, 0);
			else
				parameters = findSpeechParameters("JOE", command, _vm->logic()->joeFacing());
		}
		else
			parameters = findSpeechParameters(person->name, command, 0);

		startFrame = 31 + bobNum;
		int faceDirection = 0;

		if (isJoe && _vm->logic()->joeFacing() == DIR_LEFT)
			faceDirection = DIR_LEFT;
		else if (!isJoe) {
			ObjectData *data = _vm->logic()->objectData(_vm->logic()->objectForPerson(bobNum));

			if (data->image == -3)
				faceDirection = DIR_LEFT;

			if (faceDirectionCommand == SPEAK_FACE_LEFT)
				data->image = -3;
			else if (faceDirectionCommand == SPEAK_FACE_RIGHT)
				data->image = -4;
		}

		if (faceDirectionCommand) {
			switch (faceDirectionCommand) {
			case SPEAK_FACE_LEFT:
				faceDirection = DIR_LEFT;
				break;
			case SPEAK_FACE_RIGHT:
				faceDirection = DIR_RIGHT;
				break;
			case SPEAK_FACE_FRONT:
				faceDirection = DIR_FRONT;
				break;
			case SPEAK_FACE_BACK:
				faceDirection = DIR_BACK;
				break;
			}
			if (isJoe)
				_vm->logic()->joeFacing(faceDirection);
		}

		if (!isJoe) {
			bob->xflip = (faceDirection == DIR_LEFT);
		}

		// Run animated sequence if SANIMstr is primed

		if (_talkHead) {
			// talk.c lines 1612-1635
			headStringAnimation(parameters, bobNum, bankNum);
		}

		if (_vm->subtitles())
			_vm->graphics()->setBobText(bob, segment, textX, textY, color, _talkHead);

		if (parameters->animation[0] != '\0' && parameters->animation[0] != 'E') {
			stringAnimation(parameters, startFrame, bankNum);
		} else {
			_vm->bankMan()->unpack(parameters->body, startFrame, bankNum);

			if (length == 0 && !isJoe && parameters->bf > 0) {
				_vm->bankMan()->overpack(parameters->bf, startFrame, bankNum);
				_vm->update();
			}

			if (-1 == parameters->rf) {
				// Setup the Torso frames
				_vm->bankMan()->overpack(parameters->bf, startFrame, bankNum);
				if (isJoe)
					parameters = findSpeechParameters(person->name, 0, _vm->logic()->joeFacing());
				else
					parameters = findSpeechParameters(person->name, 0, 0);
			}

			if (-2 == parameters->rf) {
				// Setup the Torso frames
				_vm->bankMan()->overpack(parameters->bf, startFrame, bankNum);
				if (isJoe)
					parameters = findSpeechParameters(person->name, 14, _vm->logic()->joeFacing());
				else
					parameters = findSpeechParameters(person->name, 14, 0);
			}

			defaultAnimation(segment, isJoe, parameters, startFrame, bankNum);
		}
	}

	// Moved here so that Text is cleared when a Torso command done!
	_vm->display()->clearTexts(0,198);

	if (oracle) {
		uint16 frameNum = _vm->graphics()->personFrames(bobNum);
		for (i = 1; i <= 4; ++i) {
			_vm->bankMan()->unpack(i, frameNum, bankNum);
			++frameNum;
		}
	}

	// Ensure that the correct buffer frame is selected

	if (isJoe && !_talkHead) {
		if (_vm->logic()->joeFacing() == DIR_FRONT ||
				_vm->logic()->joeFacing() == DIR_BACK) {
			// Joe is facing either Front or Back!
			//  - Don't FACE_JOE in room 69, because Joe is probably
			//       holding the Dino Ray gun.
			if (_vm->logic()->currentRoom() != 69)
				_vm->logic()->joeFace();
		} else {
			if (command == SPEAK_DEFAULT ||
					command == 6 ||
					command == 7) {
				_vm->logic()->joeFace();
			} else if (command != 5) {
				// 7/11/94, Ensure that correct mouth closed frame is used!
				if (parameters->rf != -1)
					// XXX should really be just "bf", but it is not always calculated... :-(
					_vm->bankMan()->overpack(parameters->bf, startFrame, bankNum);

				if (parameters->ff == 0)
					_vm->bankMan()->overpack(10, startFrame, bankNum);
				else
					_vm->bankMan()->overpack(parameters->ff, startFrame, bankNum);
			}
		}
	}

	_vm->update();
}

const Talk::SpeechParameters *Talk::findSpeechParameters(
		const char *name,
		int state,
		int faceDirection) {
	const SpeechParameters *iterator = _speechParameters;
	if (faceDirection == DIR_RIGHT)
		faceDirection = DIR_LEFT;
	while (iterator->name[0] != '*') {
		if (0 == scumm_stricmp(iterator->name, name) &&
				iterator->state == state &&
				iterator->faceDirection == faceDirection)
			break;
		iterator++;
	}
	return iterator;
}

void Talk::getString(const byte *ptr, uint16 &offset, char *str, int maxLength, int align) {
	assert((align & 1) == 0);
	int length = *(ptr + offset);
	++offset;

	if (length > maxLength) {
		error("String too long. Length = %i, maxLength = %i", length, maxLength);
	} else if (length) {
		if (str) {
			memcpy(str, ptr + offset, length);
			str[length] = '\0';
		}
		offset = (offset + length + (align - 1)) & ~(align - 1);
	} else {
		if (str) {
			str[0] = '\0';
		}
	}
}

TalkSelected *Talk::talkSelected() {
	return _vm->logic()->talkSelected(_uniqueKey);
}

int Talk::splitOption(const char *str, char optionText[5][MAX_STRING_SIZE]) {
	char option[MAX_STRING_SIZE];
	strcpy(option, str);
	// option text ends at '*' char
	char *p = strchr(option, '*');
	if (p) {
		*p = '\0';
	}
	int lines;
	memset(optionText, 0, 5 * MAX_STRING_SIZE);
	if (_vm->resource()->getLanguage() == Common::EN_ANY || _vm->display()->textWidth(option) <= MAX_TEXT_WIDTH) {
		strcpy(optionText[0], option);
		lines = 1;
	} else if (_vm->resource()->getLanguage() == Common::HE_ISR) {
		lines = splitOptionHebrew(option, optionText);
	} else {
		lines = splitOptionDefault(option, optionText);
	}
	return lines;
}

int Talk::splitOptionHebrew(const char *str, char optionText[5][MAX_STRING_SIZE]) {
	char tmpString[MAX_STRING_SIZE] = "";
	uint16 len = 0;
	uint16 spaceCharWidth = _vm->display()->textWidth(" ");
	uint16 width = 0;
	uint16 optionLines = 0;
	uint16 maxTextLen = MAX_TEXT_WIDTH;
	const char *p = strchr(str, '\0');
	while (p != str - 1) {
		while (*p != ' ' && p != str - 1) {
			--p;
			++len;
		}
		if (p != str - 1) {
			uint16 wordWidth = _vm->display()->textWidth(p, len);
			width += wordWidth;
			if (width > maxTextLen) {
				++optionLines;
				strncpy(optionText[optionLines], p, len);
				optionText[optionLines][len] = '\0';
				width = wordWidth;
				maxTextLen = MAX_TEXT_WIDTH - OPTION_TEXT_MARGIN;
			} else {
				strcpy(tmpString, optionText[optionLines]);
				strncpy(optionText[optionLines], p, len);
				optionText[optionLines][len] = '\0';
				strcat(optionText[optionLines], tmpString);
			}
			--p;
			len = 1;
			width += spaceCharWidth;
		} else {
				if (len > 1) {
				if (width + _vm->display()->textWidth(p + 1, len) > maxTextLen) {
					++optionLines;
				}
				strcpy(tmpString, optionText[optionLines]);
				strncpy(optionText[optionLines], p + 1, len);
				optionText[optionLines][len] = '\0';
				strcat(optionText[optionLines], tmpString);
			}
			++optionLines;
		}
	}
	return optionLines;
}

int Talk::splitOptionDefault(const char *str, char optionText[5][MAX_STRING_SIZE]) {
	// Split up multiple line option at closest space character
	uint16 spaceCharWidth = _vm->display()->textWidth(" ");
	uint16 width = 0;
	uint16 optionLines = 0;
	uint16 maxTextLen = MAX_TEXT_WIDTH;
	const char *p = str;
	while (p) {
		p = strchr(str, ' ');
		if (p) {
			uint16 len = p - str;
			uint16 wordWidth = _vm->display()->textWidth(str, len);
			width += wordWidth;
			if (width > maxTextLen) {
				++optionLines;
				strncpy(optionText[optionLines], str, len + 1);
				width = wordWidth;
				maxTextLen = MAX_TEXT_WIDTH - OPTION_TEXT_MARGIN;
			} else {
				strncat(optionText[optionLines], str, len + 1);
			}
			width += spaceCharWidth;
			str = p + 1;
		} else {
			if (str[0]) {
				if (width + _vm->display()->textWidth(str) > maxTextLen) {
					++optionLines;
				}
				strcat(optionText[optionLines], str);
			}
			++optionLines;
		}
	}
	return optionLines;
}

int16 Talk::selectSentence() {
	int selectedSentence = 0;

	int startOption = 1;
	int optionLines = 0;
	char optionText[5][MAX_STRING_SIZE];
	int talkZone[5];
	int i;

	_vm->display()->textCurrentColor(_vm->display()->getInkColor(INK_TALK_NORMAL));

	_vm->graphics()->setupArrows();
	BobSlot *arrowBobUp   = _vm->graphics()->bob(Graphics::ARROW_BOB_UP);
	arrowBobUp->active    = false;
	BobSlot *arrowBobDown = _vm->graphics()->bob(Graphics::ARROW_BOB_DOWN);
	arrowBobDown->active  = false;

	bool rezone = true;

	while (rezone) {
		rezone = false;

		// Set zones for UP/DOWN text arrows when not English version

		_vm->grid()->clear(GS_PANEL);

		if (_vm->resource()->getLanguage() != Common::EN_ANY) {
			_vm->grid()->setZone(GS_PANEL, ARROW_ZONE_UP,   MAX_TEXT_WIDTH + 1, 0,  319, 24);
			_vm->grid()->setZone(GS_PANEL, ARROW_ZONE_DOWN, MAX_TEXT_WIDTH + 1, 25, 319, 49);
		}

		_vm->display()->clearTexts(151, 199);

		int sentenceCount = 0;
		int yOffset = 1;

		for (i = startOption; i <= 4; i++) {
			talkZone[i] = 0;

			if (_talkString[i][0] != '\0') {
				sentenceCount++;
				optionLines = splitOption(_talkString[i], optionText);

				if (yOffset < 5) {
					_vm->grid()->setZone(
							GS_PANEL,
							i,
							0,
							yOffset * LINE_HEIGHT - PUSHUP,
							(_vm->resource()->getLanguage() == Common::EN_ANY) ? 319 : MAX_TEXT_WIDTH,
							(yOffset + optionLines) * LINE_HEIGHT - PUSHUP);
				}

				int j;
				for (j = 0; j < optionLines; j++) {
					if (yOffset < 5) {
						_vm->display()->setText(
								(j == 0) ? 0 : OPTION_TEXT_MARGIN,
								150 - PUSHUP + yOffset * LINE_HEIGHT,
								optionText[j]);
					}
					yOffset++;
				}

				talkZone[i] = sentenceCount;
			}
		}

		yOffset--;

		// Up and down dialogue arrows

		if (_vm->resource()->getLanguage() != Common::EN_ANY) {
			arrowBobUp->active    = (startOption > 1);
			arrowBobDown->active  = (yOffset > 4);
		}

		_vm->input()->clearKeyVerb();

		if (sentenceCount > 0) {
			int zone = 0;
			int oldZone = 0;

			while (0 == selectedSentence) {

				if (_vm->input()->talkQuit())
					break;

				_vm->update();

				Common::Point mouse = _vm->input()->getMousePos();
				zone = _vm->grid()->findZoneForPos(GS_PANEL, mouse.x, mouse.y);

				int mouseButton = _vm->input()->mouseButton();
				_vm->input()->clearMouseButton();

				if (ARROW_ZONE_UP == zone || ARROW_ZONE_DOWN == zone) {
					if (oldZone > 0) {
						int16 y;
						const Box *b = _vm->grid()->zone(GS_PANEL, oldZone);
						for (y = b->y1; y < b->y2; y += 10)
							_vm->display()->textColor(150 + y, _vm->display()->getInkColor(INK_TALK_NORMAL));
						oldZone = 0;
					}
					if (mouseButton != 0) {
						if (zone == ARROW_ZONE_UP && arrowBobUp->active) {
							startOption--;
						} else if (zone == ARROW_ZONE_DOWN && arrowBobDown->active) {
							startOption++;
						}
					}
					rezone = true;
					break;
				} else {
					if (oldZone != zone) {
						// Changed zone, change text colors
						int y;

						debug(6, "Changed zone. oldZone = %i, zone = %i",
								oldZone, zone);

						if (zone > 0) {
							const Box *b = _vm->grid()->zone(GS_PANEL, zone);
							for (y = b->y1; y < b->y2; y += 10)
								_vm->display()->textColor(150 + y, _vm->display()->getInkColor(INK_JOE));
						}

						if (oldZone > 0) {
							const Box *b = _vm->grid()->zone(GS_PANEL, oldZone);
							for (y = b->y1; y < b->y2; y += 10)
								_vm->display()->textColor(150 + y, _vm->display()->getInkColor(INK_TALK_NORMAL));
						}

						oldZone = zone;
					}

				}

				Verb v = _vm->input()->keyVerb();
				if (v >= VERB_DIGIT_FIRST && v <= VERB_DIGIT_LAST) {
					int n = v - VERB_DIGIT_FIRST + 1;
					for (i = 1; i <= 4; i++) {
						if (talkZone[i] == n) {
							selectedSentence = i;
							break;
						}
					}

					_vm->input()->clearKeyVerb();
				} else if (mouseButton) {
					selectedSentence = zone;
				}

			} // while ()
		}
	}

	debug(6, "Selected sentence %i", selectedSentence);

	arrowBobUp->active    = false;
	arrowBobDown->active  = false;

	if (selectedSentence > 0) {
		_vm->display()->clearTexts(0, 198);

		speak(_talkString[selectedSentence], NULL, _joeVoiceFilePrefix[selectedSentence]);
	}

	_vm->display()->clearTexts(151, 151);

	return selectedSentence;
}

const Talk::SpeechParameters Talk::_speechParameters[] = {
	{ "JOE", 0, 1, 1, 10, 2, 3, "", 0 },
	{ "JOE", 0, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 0, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 1, 1, 1, 45, -1, 0, "", 0 },
	{ "JOE", 1, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 1, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 2, 1, 1, 46, -1, 0, "", 0 },
	{ "JOE", 2, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 2, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 3, 1, 1, 47, -1, 0, "", 0 },
	{ "JOE", 3, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 3, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 4, 1, 1, 50, -1, 0, "", 0 },
	{ "JOE", 4, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 4, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 5, 1, 2, 0, 0, 0, "", 0 },
	{ "JOE", 5, 3, 4, 0, 0, 0, "", 0 },
	{ "JOE", 5, 4, 6, 0, 0, 0, "", 0 },

	{ "JOE", 6, 1, 1, 48, 0, 1, "", 0 },
	{ "JOE", 6, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 6, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 7, 1, 1, 51, 0, 1, "", 0 },
	{ "JOE", 7, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 7, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 8, 1, 1, 26, 0, 0, "", 0 },
	{ "JOE", 8, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 8, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 9, 1, 1, 29, 0, 0, "", 0 },
	{ "JOE", 9, 3, 3, 28, 0, 0, "", 0 },
	{ "JOE", 9, 4, 5, 38, 0, 0, "", 0 },

	{ "JOE", 10, 1, 1, 12, 0, 0, "T046,010,010,010,012,012,012,012,012,012,012,012,012,012,012,012,012,012,010,000", 0 },
	{ "JOE", 10, 3, 3, 18, 0, 0, "", 0 },
	{ "JOE", 10, 4, 5, 44, 0, 0, "", 0 },

	{ "JOE", 11, 1, 1, 53, -1, 0, "", 0 },
	{ "JOE", 11, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 11, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 12, 1, 1, 10, 2, 3, "", 0 },
	{ "JOE", 12, 3, 3, 28, 2, 0, "", 0 },
	{ "JOE", 12, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 13, 1, 1, 10, 2, 3, "T012,013,019,019,019,019,019,019,019,019,019,019,013,010,000", 0 },
	{ "JOE", 13, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 13, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 14, 1, 1, 16, 2, 3, "", 16 },
	{ "JOE", 14, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 14, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 15, 1, 1, 58, -1, 0, "", 0 },
	{ "JOE", 15, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 15, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 16, 1, 1, 59, -1, 0, "", 0 },
	{ "JOE", 16, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 16, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 17, 1, 1, 56, -1, 0, "", 0 },
	{ "JOE", 17, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 17, 4, 5, 38,  1, 0, "", 0 },

	{ "JOE", 18, 1, 56, 16, 2, 3, "T056,057,057,057,056,056,000", 0 },
	{ "JOE", 18, 3,  3, 28, 2, 3, "", 0 },
	{ "JOE", 18, 4,  5, 38, 1, 0, "", 0 },

	{ "JOE", 19, 1, 54, 16, 2, 3, "T054,055,057,056,000", 0 },
	{ "JOE", 19, 3,  3, 28, 2, 3, "", 0 },
	{ "JOE", 19, 4,  5, 38, 1, 0, "", 0 },

	{ "JOE", 20, 1, 56, 16, 2, 3, "T056,057,055,054,001,000", 0 },
	{ "JOE", 20, 3,  3, 28, 2, 3, "", 0 },
	{ "JOE", 20, 4,  5, 38, 1, 0, "", 0 },

	{ "JOE", 21, 1,  1, 60, -1, 0, "", 0 },
	{ "JOE", 21, 3,  3, 28,  2, 3, "", 0 },
	{ "JOE", 21, 4, 61, 38,  1, 0, "", 0 },

	{ "JOE", 22, 1, 1, 16, 2, 3, "T063,060,000", 0 },
	{ "JOE", 22, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 22, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 23, 1, 1, 16, 2, 3, "T060,063,001,000", 0 },
	{ "JOE", 23, 3, 3, 28, 2, 3, "", 0 },
	{ "JOE", 23, 4, 5, 38, 1, 0, "", 0 },

	{ "JOE", 24, 1, 1, 47, -2, 0, "", 0 },
	{ "JOE", 24, 3, 3, 28,  2, 3, "", 0 },
	{ "JOE", 24, 4, 5, 38,  1, 0, "", 0 },

	{ "RICO", 0, 0, 1, 7,  1, 3, "", 7 },
	{ "RICO", 1, 0, 1, 5, -1, 0, "", 7 },
	{ "RICO", 2, 0, 1, 9,  0, 3, "", 7 },
	{ "RICO", 3, 0, 1, 4, -1, 0, "", 7 },

	{ "EDDY", 0, 0, 14, 18, 1, 3, "", 18 },
	{ "EDDY", 1, 0, 14,  0, 0, 0, "T016,017,017,016,016,017,017,016,016,017,017,000", 18 },

	{ "MIKE", 0, 0, 1, 2, 2, 3, "", 2 },

	{ "LOLA", 0, 0, 30, 33, 2, 3, "", 33 },
	{ "LOLA", 1, 0,  9, 10, 2, 3, "", 33 },
	{ "LOLA", 2, 0, 30, 33, 2, 3, "", 33 },
	{ "LOLA", 3, 0, 32, 33, 2, 3, "", 33 },
	{ "LOLA", 4, 0,  8,  0, 0, 0, "", 33 },
	{ "LOLA", 5, 0, 31,  0, 0, 0, "", 0 },
	{ "LOLA", 6, 0, 31,  0, 0, 0, "047,048,049,050,000", 33 },

	{ "LOLA_SHOWER", 0, 0,  7, 10, 2, 3, "", 10 },
	{ "LOLA_SHOWER", 1, 0,  9, 10, 2, 3, "", 10 },
	{ "LOLA_SHOWER", 2, 0, 30, 33, 2, 3, "", 10 },
	{ "LOLA_SHOWER", 3, 0, 32, 33, 2, 3, "", 10 },
	{ "LOLA_SHOWER", 4, 0,  8,  0, 0, 0, "", 0 },
	{ "LOLA_SHOWER", 5, 0, 61,  0, 0, 0, "", 0 },
	{ "LOLA_SHOWER", 6, 0, 64, 10, 2, 3, "", 0 },
	{ "LOLA_SHOWER", 7, 0, 31,  0, 0, 0, "062,063,064,000", 0 },

	{ "SECRETARY", 0, 0, 1, 12, 2, 3, "", 12 },
	{ "SECRETARY", 1, 0, 1, 12, 2, 0, "", 12 },
	{ "SECRETARY", 2, 0, 1, 12, 2, 0, "", 12 },

	{ "SPARKY",  0, 0, 21, 23, 2, 3, "", 23 },
	{ "SPARKY",  1, 0, 21, 22, 0, 0, "", 0 },
	{ "SPARKY",  2, 0, 21, 22, 0, 0, "021,042,043,000", 0 },
	{ "SPARKY",  3, 0, 21, 22, 0, 0, "043,042,021,000", 0 },
	{ "SPARKY",  4, 0, 43, 43, 1, 0, "", 0 },
	{ "SPARKY", 14, 0, 21, 29, 5, 0, "", 29 },

	{ "SPARKY-F",  0, 0, 45, 23, 5, 0, "", 23 },
	{ "SPARKY-F",  1, 0, 45, 47, 0, 0, "", 0 },
	{ "SPARKY-F",  2, 0, 45, 23, 5, 0, "045,046,046,045,000", 23 },
	{ "SPARKY-F", 14, 0, 45, 29, 5, 0, "", 29 },

	{ "FAYE", 0, 0, 19, 35,  2, 3, "", 35 },
	{ "FAYE", 1, 0, 19, 41,  2, 3, "", 35 },
	{ "FAYE", 2, 0, 19, 28, -1, 0, "", 35 },
	{ "FAYE", 3, 0, 19, 20,  0, 0, "", 0 },
	{ "FAYE", 4, 0, 19, 27, -1, 0, "", 35 },
	{ "FAYE", 5, 0, 19, 29, -1, 0, "", 35 },
	{ "FAYE", 6, 0, 59, 35,  2, 3, "", 35 },
	{ "FAYE", 7, 0, 19, 30, -1, 0, "", 35 },
	{ "FAYE", 8, 0, 19, 31, -1, 0, "", 35 },

	{ "BOB", 0, 0, 27, 34,  2, 3, "", 34 },
	{ "BOB", 1, 0, 27, 28, -1, 0, "", 34 },
	{ "BOB", 2, 0, 30,  0,  0, 0, "", 0 },
	{ "BOB", 3, 0, 31,  0,  0, 0, "", 0 },
	{ "BOB", 4, 0, 27, 61, -1, 0, "", 34 },
	{ "BOB", 5, 0, 27, 42,  1, 0, "", 42 },

	{ "PYGMY", 0, 0, 20, 21, 2, 6, "", 0 },
	{ "PYGMY", 1, 0, 20, 21, 2, 6, "020,068,068,068,068,068,068,068,068,020,000", 0 },
	{ "PYGMY", 2, 0, 20, 21, 2, 6, "T028,029,030,031,031,031,031,030,029,028,035,000", 0 },
	{ "PYGMY", 3, 0, 20, 21, 2, 6, "T035,036,037,038,037,038,037,038,036,035,000", 0 },
	{ "PYGMY", 4, 0, 20, 21, 2, 6, "T032,033,032,033,032,033,035,000", 0 },
	{ "PYGMY", 5, 0, 20, 21, 2, 6, "T023,022,021,022,023,024,025,026,027,026,025,024,023,000", 0 },
	{ "PYGMY", 6, 0, 20, 21, 2, 6, "T034,034,034,035,000", 0 },

	{ "WITCH", 0, 0, 39, 40,  2, 6, "", 40 },
	{ "WITCH", 1, 0, 39, 40,  2, 6, "073,074,000", 40 },
	{ "WITCH", 2, 0, 39, 40,  2, 6, "074,073,000", 40 },
	{ "WITCH", 3, 0, 39, 40,  2, 6, "T047,048,049,050,051,000", 40 },
	{ "WITCH", 4, 0, 39, 40,  2, 6, "T052,053,054,055,056,057,058,057,056,056,056,055,054,053,052,000", 40 },
	{ "WITCH", 5, 0, 39, 40,  2, 6, "069,070,071,072,073,074,000", 40 },
	{ "WITCH", 6, 0, 39, 40,  2, 6, "074,073,072,071,070,069,070,000", 40 },
	{ "WITCH", 7, 0, 39, 51, -1, 0, "", 40 },
	{ "WITCH", 8, 0, 39, 40,  2, 6, "T051,050,049,048,047,000", 40 },

	{ "CHIEF", 0, 0, 1,  7,  1, 7, "", 3 },
	{ "CHIEF", 1, 0, 1,  2,  2, 6, "062,063,064,065,066,000", 0 },
	{ "CHIEF", 2, 0, 1,  2,  2, 6, "066,065,064,063,062,000", 0 },
	{ "CHIEF", 3, 0, 1, 17, -1, 0, "", 3 },
	{ "CHIEF", 4, 0, 1, 18, -1, 0, "", 3 },
	{ "CHIEF", 5, 0, 1, 19, -1, 0, "", 3 },

	{ "NAOMI", 0, 0, 1,  2,  2, 3, "", 2 },
	{ "NAOMI", 1, 0, 1,  2,  2, 6, "048,049,050,051,052,053,054,055,000", 0 },
	{ "NAOMI", 2, 0, 1,  2,  2, 6, "055,054,053,052,051,050,049,048,000", 0 },
	{ "NAOMI", 3, 0, 1, 13, -1, 0, "", 2 },
	{ "NAOMI", 4, 0, 1, 14, -1, 0, "", 2 },
	{ "NAOMI", 5, 0, 1, 10, -1, 0, "", 2 },
	{ "NAOMI", 6, 0, 1, 12, -1, 0, "", 2 },
	{ "NAOMI", 7, 0, 1, 12, -1, 0, "T008,008,008,002,000", 2 },

	{ "WEDGEWOOD", 0, 0, 8, 1, 2, 0, "", 8 },
	{ "WEDGEWOOD", 1, 0, 1, 1, 3, 0, "", 1 },

	{ "BUD", 0, 0, 1,  2,  3, 2, "", 2 },
	{ "BUD", 1, 0, 1,  2,  4, 2, "T017,018,000", 2 },
	{ "BUD", 2, 0, 1, 21, -1, 0, "", 2 },
	{ "BUD", 3, 0, 1, 14, -1, 0, "", 2 },
	{ "BUD", 4, 0, 1, 15, -1, 0, "", 2 },
	{ "BUD", 5, 0, 1, 20, -1, 0, "", 2 },
	{ "BUD", 6, 0, 1, 16, -1, 0, "", 2 },
	{ "BUD", 7, 0, 1, 19, -1, 0, "", 2 },
	{ "BUD", 8, 0, 1, 17, -1, 0, "", 2 },
	{ "BUD", 9, 0, 1, 14, -1, 0, "T014,008,008,003,003,008,008,003,003,010,010,012,012,000", 2 },

	{ "LOU", 0, 0, 1, 2, 2, 3, "", 2 },
	{ "LOU", 1, 0, 1, 2, 4, 2, "013,014,015,016,017,018,000", 2 },
	{ "LOU", 2, 0, 1, 2, 4, 2, "018,017,016,015,014,013,000", 2 },

	{ "JIMMY", 0, 0, 16, 17,  2, 3, "", 17 },
	{ "JIMMY", 1, 0, 16, 25, -1, 0, "", 17 },
	{ "JIMMY", 2, 0, 16, 26, -1, 0, "", 17 },
	{ "JIMMY", 3, 0, 16, 27, -1, 0, "", 17 },
	{ "JIMMY", 4, 0, 16, 28, -1, 0, "", 17 },
	{ "JIMMY", 5, 0, 16, 29, -1, 0, "", 17 },

	{ "TAMMY", 0, 0, 1, 2, 2, 3, "", 2 },
	{ "TAMMY", 1, 0, 1, 2, 2, 3, "T008,008,009,009,008,008,009,009,008,008,009,009,002,000", 2 },
	{ "TAMMY", 2, 0, 1, 2, 2, 3, "T002,010,010,010,002,000", 2 },
	{ "TAMMY", 3, 0, 1, 2, 2, 3, "T011,011,011,011,011,002,000", 2 },
	{ "TAMMY", 4, 0, 1, 2, 2, 3, "T013,014,015,013,014,015,013,009,001,000", 2 },

	{ "SKULL", 0, 0, 9, 9, 4, 0, "", 0 },
	{ "SKULL", 1, 0, 1, 9, 4, 0, "001,002,003,004,005,006,007,008,009,000", 0 },
	{ "SKULL", 2, 0, 1, 9, 4, 0, "009,008,007,006,005,004,003,002,001,000", 0 },

	{ "APE", 0, 0, 1, 6, 7, 0, "", 6 },
	{ "APE", 1, 0, 1, 6, 7, 0, "002,001,000", 6 },
	{ "APE", 2, 0, 1, 6, 7, 0, "002,003,001,000", 6 },
	{ "APE", 3, 0, 1, 6, 7, 0, "004,005,004,005,004,001,000", 6 },
	{ "APE", 4, 0, 1, 6, 7, 0, "001,003,005,004,005,004,001,000", 6 },

	{ "APE1", 0, 0, 15, 16, 7, 0, "", 16 },
	{ "APE2", 0, 0, 14,  6, 7, 0, "", 6 },

	{ "SHOWERAM", 0, 0, 1, 2, 3, 0, "", 2 },
	{ "SHOWERAM", 1, 0, 1, 2, 3, 0, "026,027,028,029,001,000", 2 },
	{ "SHOWERAM", 2, 0, 1, 2, 3, 0, "001,029,028,027,026,000", 2 },

	{ "PRINCESS1", 0, 0, 19, 23,  2, 3, "", 23 },
	{ "PRINCESS1", 1, 0, 19, 41, -1, 0, "", 23 },
	{ "PRINCESS1", 2, 0, 19, 42, -1, 0, "", 23 },
	{ "PRINCESS1", 3, 0, 19, 45, -1, 0, "", 23 },
	{ "PRINCESS1", 4, 0, 19, 40, -1, 0, "", 23 },
	{ "PRINCESS1", 5, 0, 19, 45,  2, 3, "T40,043,044,045,000", 45 },
	{ "PRINCESS1", 6, 0, 19, 45, -1, 0, "T041,038,000", 38 },
	{ "PRINCESS1", 7, 0, 22,  0,  0, 0, "", 0 },
	{ "PRINCESS1", 8, 0, 19, 45,  2, 3, "T045,044,043,040,039,000", 39 },

	{ "PRINCESS2", 0, 0, 46, 23, 2, 3, "", 23 },
	{ "PRINCESS2", 1, 0, 46, 29, 2, 3, "", 29 },
	{ "PRINCESS2", 2, 0, 46, 29, 2, 3, "T029,036,035,000", 35 },

	{ "GUARDS", 0, 0, 7, 7, 0, 0, "", 7 },

	{ "AMGUARD", 0, 0, 19, 22, 2, 3, "", 22 },

	{ "MAN1", 0, 0, 2, 3, 2, 3, "", 3 },
	{ "MAN2", 0, 0, 9, 10, 1, 2, "", 10 },

	{ "DOG", 0, 0, 6, 6, 1, 0, "", 0 },
	{ "DOG", 1, 0, 6, 6, 1, 0, "010,009,008,000", 0 },
	{ "DOG", 2, 0, 6, 6, 1, 0, "008,009,010,000", 0 },

	{ "CHEF", 0, 0, 5, 6, 2, 3, "", 6 },

	{ "HENRY",  0, 0,  7,  9,  2, 3, "", 9 },
	{ "HENRY",  1, 0,  7, 21, -1, 0, "", 9 },
	{ "HENRY",  2, 0,  7, 19, -1, 0, "", 9 },
	{ "HENRY",  3, 0,  7, 20, -1, 0, "", 9 },
	{ "HENRY",  4, 0,  8,  9,  2, 3, "", 9 },
	{ "HENRY",  5, 0, 23,  9, -1, 0, "", 9 },
	{ "HENRY",  6, 0,  7,  9,  2, 3, "T019,015,017,017,017,017,017,017,017,015,009,000", 9 },
	{ "HENRY",  7, 0,  7,  9,  2, 3, "T018,010,000", 10 },
	{ "HENRY",  8, 0,  7,  9,  2, 3, "T018,016,000", 16 },
	{ "HENRY",  9, 0,  7,  9,  2, 3, "T018,011,000", 11 },
	{ "HENRY", 10, 0, 29, 33,  1, 1, "", 33 },
	{ "HENRY", 11, 0,  7, 30,  2, 0, "", 9 },
	{ "HENRY", 12, 0,  7,  9,  2, 3, "025,026,000", 26 },
	{ "HENRY", 13, 0,  7,  9,  2, 3, "027,028,027,028,000", 28 },
	{ "HENRY", 14, 0,  7,  9,  2, 3, "026,025,007,000", 9 },

	{ "JOHAN", 0, 0, 1, 15,  2, 3, "", 15 },
	{ "JOHAN", 1, 0, 1,  0,  0, 0, "T006,007,008,000", 15 },
	{ "JOHAN", 2, 0, 1, 15,  2, 3, "T002,003,004,005,004,005,004,005,004,005,004,005,004,003,002,000", 15 },
	{ "JOHAN", 3, 0, 1,  8, -1, 0, "", 15 },
	{ "JOHAN", 4, 0, 1,  0,  0, 0, "T008,007,006,001,000", 15 },

	{ "KLUNK", 0, 0, 1, 2, 2, 3, "", 2 },
	{ "KLUNK", 1, 0, 1, 2, 2, 3, "019,020,021,022,001,000", 2 },
	{ "KLUNK", 2, 0, 1, 2, 2, 3, "001,022,021,020,019,016,517,000", 2 },
	{ "KLUNK", 3, 0, 1, 2, 2, 3, "T010,011,010,011,010,011,009,000", 2 },

	{ "FRANK", 0, 0, 13, 14, 2, 3, "", 14 },
	{ "FRANK", 1, 0, 13, 20, 0, 1, "", 14 },
	{ "FRANK", 2, 0, 13, 14, 2, 3, "025,026,027,027,027,026,026,026,027,027,026,026,027,025,013,000", 14 },
	{ "FRANK", 3, 0, 28, 14, 2, 3, "", 14 },

	{ "DEATH", 0, 0, 1, 2, 2, 3, "", 2 },
	{ "DEATH", 1, 0, 1, 2, 2, 3, "013,014,015,016,017,001,000", 0 },
	{ "DEATH", 2, 0, 1, 2, 2, 3, "001,017,016,015,014,013,000", 0 },
	{ "DEATH", 3, 0, 1, 2, 2, 3, "T018,019,020,021,021,022,022,020,021,022,020,021,022,023,024,524,000", 2 },
	{ "DEATH", 4, 0, 1, 2, 2, 3, "T025,026,027,028,028,028,028,028,028,028,028,028,029,035,000", 2 },
	{ "DEATH", 5, 0, 1, 2, 2, 3, "T030,031,032,033,033,033,033,033,033,033,033,033,034,035,000", 2 },
	{ "DEATH", 6, 0, 1, 2, 2, 3, "T023,022,020,019,018,001,000", 2 },

	{ "JASPAR", 0, 0, 1, 1, 22, 0, "026,027,028,029,028,029,028,029,030,023,000", 0 },
	{ "JASPAR", 1, 0, 1, 1, 22, 0, "023,026,000", 0 },

	{ "ORACLE", 0, 0, 1, 5, 3, 0, "", 0 },

	{ "ZOMBIE", 0, 0, 1,  5,  2, 3, "", 5 },
	{ "ZOMBIE", 1, 0, 1, 12, -1, 0, "", 5 },
	{ "ZOMBIE", 2, 0, 1, 13, -1, 0, "", 5 },
	{ "ZOMBIE", 3, 0, 1,  1,  5, 5, "", 5 },

	{ "ZOMBIE2", 0, 0, 14, 14, 0, 0, "", 0 },
	{ "ZOMBIE3", 0, 0, 18, 18, 0, 0, "", 0 },

	{ "ANDERSON", 0, 0, 7,  8,  2, 3, "", 8 },
	{ "ANDERSON", 1, 0, 7,  8,  1, 0, "", 8 },
	{ "ANDERSON", 2, 0, 7, 16, -1, 0, "", 8 },
	{ "ANDERSON", 3, 0, 7, 18, -1, 0, "", 8 },
	{ "ANDERSON", 4, 0, 7, 19, -1, 0, "", 8 },
	{ "ANDERSON", 5, 0, 7, 20, -1, 0, "", 8 },
	{ "ANDERSON", 6, 0, 7, 21,  1, 0, "", 8 },

	{ "COMPY", 0, 0, 12, 12, -1, 0, "", 0 },
	{ "COMPY", 1, 0, 10, 10, 10, 0, "010,011,012,012,013,014,014,000", 0 },
	{ "COMPY", 2, 0, 10, 10, 10, 0, "014,013,012,000", 0 },

	{ "DEINO", 0, 0, 13, 13, -1, 0, "", 0 },
	{ "DEINO", 1, 0,  9,  9,  9, 0, "009,010,000", 0 },

	{ "TMPD", 0, 0, 19, 22, 2, 3, "", 22 },

	{ "IAN", 0, 0, 7,  9,  2, 3, "", 9 },
	{ "IAN", 1, 0, 8, 25,  3, 0, "", 25 },
	{ "IAN", 2, 0, 7, 21, -1, 0, "", 9 },
	{ "IAN", 3, 0, 7, 22,  1, 0, "", 9 },
	{ "IAN", 4, 0, 7, 22, -1, 0, "", 9 },
	{ "IAN", 5, 0, 7, 24, -1, 0, "", 9 },
	{ "IAN", 6, 0, 7,  9,  2, 3, "034,034,034,035,035,036,036,035,035,036,035,036,035,000", 9 },
	{ "IAN", 7, 0, 7, 31, -1, 0, "", 9 },

	{ "FAYE-H", 0, 0, 1, 1, 4, 1, "", 1 },
	{ "FAYE-H", 1, 0, 1, 1, 4, 1, "007,000", 7 },
	{ "FAYE-H", 2, 0, 1, 1, 4, 1, "009,010,011,009,001,000", 1 },
	{ "FAYE-H", 3, 0, 1, 1, 4, 1, "E012,013,000", 1 },
	{ "FAYE-H", 4, 0, 1, 1, 4, 1, "E015,000", 1 },
	{ "FAYE-H", 5, 0, 1, 1, 4, 1, "E014,000", 1 },

	{ "AZURA-H", 0, 0, 1, 1, 4, 1, "", 1 },
	{ "AZURA-H", 1, 0, 1, 1, 4, 1, "007,000", 7 },
	{ "AZURA-H", 2, 0, 1, 1, 4, 1, "009,010,011,009,001,000", 1 },
	{ "AZURA-H", 3, 0, 1, 1, 4, 1, "E012,013, 000", 1 },
	{ "AZURA-H", 4, 0, 1, 1, 4, 1, "E015,000", 1 },
	{ "AZURA-H", 5, 0, 1, 1, 4, 1, "E014,000", 1 },

	{ "FRANK-H", 0, 0, 1, 1, 4, 1, "", 1 },
	{ "FRANK-H", 1, 0, 1, 1, 4, 1, "E009,000", 1 },
	{ "FRANK-H", 2, 0, 1, 1, 4, 1, "E007,000", 1 },
	{ "FRANK-H", 3, 0, 1, 1, 4, 1, "010,011,012,013,014,015,010,000", 1 },

	{ "JOE-E", 0, 0, 1, 2, 4, 1, "", 2 },
	{ "JOE-E", 6, 0, 1, 2, 4, 1, "008,009,008,002,000", 2 },

	{ "AZURA-E", 0, 0, 1, 1, 5, 1, "", 1 },
	{ "AZURA-E", 1, 0, 1, 1, 5, 1, "009,010,009,008,000", 1 },

	{ "FAYE-E", 0, 0, 1, 4, 4, 1, "", 1 },
	{ "FAYE-E", 1, 0, 1, 4, 4, 1, "002,003,002,001,000", 1 },

	{ "ANDSON-E", 0, 0, 1, 3, 4, 1, "", 1 },
	{ "ANDSON-E", 1, 0, 1, 3, 4, 1, "002,001,000", 1 },

	{ "JOE-H", 0, 0, 1,  1, 4, 4, "", 1 },
	{ "JOE-H", 1, 0, 1,  1, 2, 3, "012,013,014,000", 14 },
	{ "JOE-H", 2, 0, 1,  1, 2, 3, "010,011,000", 11 },
	{ "JOE-H", 3, 0, 1,  1, 2, 3, "014,013,012,001,000", 1 },
	{ "JOE-H", 4, 0, 1, 13, 1, 0, "", 13 },

	{ "RITA-H", 0, 0, 7, 1, 2, 3, "", 1 },
	{ "RITA-H", 1, 0, 7, 0, 0, 0, "009,010,011,012,013,000", 13 },
	{ "RITA-H", 2, 0, 7, 0, 0, 0, "014,015,016,000", 16 },
	{ "RITA-H", 3, 0, 7, 0, 0, 0, "013,012,011,010,000", 10 },
	{ "RITA-H", 4, 0, 7, 0, 0, 0, "009,007,008,007,009,000", 9 },
	{ "RITA-H", 5, 0, 7, 0, 0, 0, "016,015,014,000", 14 },

	{ "RITA", 0, 0, 1, 4, 2, 3, "", 4 },
	{ "RITA", 1, 0, 2, 4, 2, 3, "", 4 },

	{ "SPARKY-H", 0, 0, 1, 1, 2, 3, "", 1 },

	{ "HUGH", 0, 0, 1, 1, 2, 3, "", 1 },
	{ "HUGH", 1, 0, 7, 7, 2, 3, "", 7 },

	{ "X2_JOE", 0, 0, 1, 1, 2, 3, "", 1 },
	{ "X2_JOE", 1, 0, 1, 1, 2, 3, "001,007,008,008,007,001,000", 1 },

	{ "X2_RITA", 0, 0, 1, 1, 2, 3, "", 1 },
	{ "X2_RITA", 1, 0, 1, 1, 2, 3, "001,007,008,008,007,001,000", 1 },

	{ "X3_RITA", 0, 0, 1, 1, 4, 1, "", 1 },
	{ "X3_RITA", 1, 0, 1, 1, 4, 1, "007,000", 7 },
	{ "X3_RITA", 2, 0, 1, 1, 4, 1, "009,010,011,009,001,000", 1 },
	{ "X3_RITA", 3, 0, 1, 1, 4, 1, "E012,013,000", 1 },
	{ "X3_RITA", 4, 0, 1, 1, 4, 1, "E015,000", 1 },
	{ "X3_RITA", 5, 0, 1, 1, 4, 1, "E014,000", 1 },

	{ "X4_JOE", 0, 0, 1,  1, 3, 4, "", 1 },
	{ "X4_JOE", 1, 0, 1, 13, 2, 3, "", 13 },
	{ "X4_JOE", 2, 0, 1,  1, 3, 4, "009, 010, 011, 012, 013, 000", 13 },
	{ "X4_JOE", 3, 0, 1,  1, 3, 4, "012, 011, 010, 009, 000", 9 },
	{ "X4_JOE", 4, 0, 1,  1, 3, 4, "001, 019, 000", 19 },

	{ "X4_RITA", 0, 0, 1, 1, 0, 1, "", 1 },
	{ "X4_RITA", 1, 0, 1, 7, 0, 1, "", 7 },
	{ "X4_RITA", 2, 0, 1, 1, 3, 4, "004,005,006,006,006,006,007,000", 7 },
	{ "X4_RITA", 3, 0, 1, 1, 3, 4, "005,004,001,000", 1 },
	{ "X4_RITA", 4, 0, 1, 1, 3, 4, "001,003,000", 3 },

	{ "X5_SPARKY", 0, 0, 1, 1, 2, 3, "", 1 },
	{ "X5_SPARKY", 1, 0, 1, 1, 2, 3, "001,010,011,011,001,000", 1 },
	{ "X5_SPARKY", 2, 0, 1, 1, 2, 3, "001,007,008,009,000", 9 },

	{ "X6_HUGH", 0, 0, 1, 1, 2, 3, "", 1 },
	{ "X6_HUGH", 1, 0, 1, 1, 2, 3, "007,007,007,007,,001,000", 1 },
	{ "X6_HUGH", 2, 0, 1, 1, 2, 3, "008,008,008,008,008,009,009,008,008,008,009,008,000", 8 },

	{ "X10_JOE", 0, 0, 1, 2, 2, 3, "", 2 },
	{ "X10_JOE", 1, 0, 1, 8, 2, 3, "", 8 },
	{ "X10_JOE", 2, 0, 1, 2, 2, 3, "014,014,014,015,015,014,014,015,015,000", 2 },

	{ "X10_RITA", 0, 0, 1, 2, 2, 3, "", 2 },

	{ "X11_JOE", 0, 0, 1, 2, 0, 1, "", 2 },

	{ "X11_RITA", 0, 0, 1, 2, 0, 1, "", 2 },
	{ "X11_RITA", 1, 0, 1, 2, 1, 0, "003,004,000", 4 },

	{ "JOHN", 0, 0, 1,  2,  2, 3, "", 1 },
	{ "JOHN", 1, 0, 1, 15, -1, 0, "", 1 },
	{ "JOHN", 2, 0, 1, 16, -1, 0, "", 1 },
	{ "JOHN", 3, 0, 1, 17, -1, 0, "", 1 },

	{ "STEVE", 0, 0, 8,  2,  2, 3, "", 2 },
	{ "STEVE", 1, 0, 8, 16, -1, 0, "", 2 },
	{ "STEVE", 2, 0, 9, 18, -1, 0, "T016,017,017,016,008,000", 2 },
	{ "STEVE", 3, 0, 8, 18, -1, 0, "", 2 },

	{ "TONY", 0, 0, 1,  2,  2, 3, "", 1 },
	{ "TONY", 1, 0, 1, 12, -1, 0, "", 1 },

	{ "*", 0, 0, 0, 0, 0, 0, "", 0 }
};

} // End of namespace Queen
