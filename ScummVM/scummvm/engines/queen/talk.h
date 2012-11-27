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

#ifndef QUEEN_TALK_H
#define QUEEN_TALK_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

class QueenEngine;

class Talk {
public:

	//! Public interface to run a talk from a file
	static void talk(const char *filename, int personInRoom, char *cutawayFilename, QueenEngine *vm);

	//! Public interface to speak a sentence
	static bool speak(const char *sentence, Person *person, const char *voiceFilePrefix, QueenEngine *vm);

	//! Read a string from ptr and update offset
	static void getString(const byte *ptr, uint16 &offset, char *str, int maxLength, int align = 2);

private:

	//! Collection of constants used by Talk
	enum {
	  LINE_HEIGHT = 10,
	  MAX_STRING_LENGTH = 255,
	  MAX_STRING_SIZE = (MAX_STRING_LENGTH + 1),
	  MAX_TEXT_WIDTH = (320-18),
	  PUSHUP = 4,
	  ARROW_ZONE_UP   = 5,
	  ARROW_ZONE_DOWN = 6,
	  DOG_HEADER_SIZE = 20,
	  OPTION_TEXT_MARGIN = 24
	};

	//! Special commands for speech
	enum {
		SPEAK_DEFAULT      =  0,
		SPEAK_FACE_LEFT    = -1,
		SPEAK_FACE_RIGHT   = -2,
		SPEAK_FACE_FRONT   = -3,
		SPEAK_FACE_BACK    = -4,
		SPEAK_ORACLE       = -5,
		SPEAK_UNKNOWN_6    = -6,
		SPEAK_AMAL_ON      = -7,
		SPEAK_PAUSE        = -8,
		SPEAK_NONE         = -9
	};

	struct DialogueNode {
		int16 head;
		int16 dialogueNodeValue1;
		int16 gameStateIndex;
		int16 gameStateValue;
	};

	struct SpeechParameters {
		const char *name;
		signed char state,faceDirection;
		signed char body,bf,rf,af;
		const char *animation;
		signed char ff;
	};

	QueenEngine *_vm;

	bool _wasFullscren;

	//! Raw .dog file data (without 20 byte header)
	byte *_fileData;

	//! Number of dialogue levels
	int16 _levelMax;

	//! Unique key for this dialogue
	int16 _uniqueKey;

	//! Used to select voice files
	int16 _talkKey;

	int16 _jMax;

	//! Used by findDialogueString
	int16 _pMax;

	// Update game state efter dialogue
	int16 _gameState[2];
	int16 _testValue[2];
	int16 _itemNumber[2];

	//! String data
	uint16 _person1PtrOff;

	//! Cutaway data
	uint16 _cutawayPtrOff;

	//! Data used if we have talked to the person before
	uint16 _person2PtrOff;

	//! Data used if we haven't talked to the person before
	uint16 _joePtrOff;

	//! Is a talking head
	bool _talkHead;

	//! IDs for sentences
	DialogueNode _dialogueTree[18][6];

	//! Greeting from person Joe has talked to before
	char _person2String[MAX_STRING_SIZE];

	int _oldSelectedSentenceIndex;
	int _oldSelectedSentenceValue;

	char _talkString[5][MAX_STRING_SIZE];
	char _joeVoiceFilePrefix[5][MAX_STRING_SIZE];

	static const SpeechParameters _speechParameters[];

	Talk(QueenEngine *vm);
	~Talk();

	//! Perform talk in file and return a cutaway filename
	void talk(const char *filename, int personInRoom, char *cutawayFilename);

	byte *loadDialogFile(const char *filename);

	//! Load talk data from .dog file
	void load(const char *filename);

	//! First things spoken
	void initialTalk();

	//! Find a string in the dialogue tree
	void findDialogueString(uint16 offset, int16 id, int16 max, char *str);

	//! Get TalkSelected struct for this talk
	TalkSelected *talkSelected();

	//! Interface to the TalkSelected struct
	bool hasTalkedTo() { return talkSelected()->hasTalkedTo; }

	//! Interface to the TalkSelected struct
	void setHasTalkedTo() { talkSelected()->hasTalkedTo = true; }

	//! Get a selected value
	int16 selectedValue(int index) {
		return talkSelected()->values[index-1];
	}

	//! Set a selected value
	void selectedValue(int index, int16 value) {
		talkSelected()->values[index-1] = value;
	}

	//! The sentence will not be displayed again
	void disableSentence(int oldLevel, int selectedSentence);

	//! Select what to say
	int16 selectSentence();

	//! Speak sentence
	bool speak(const char *sentence, Person *person, const char *voiceFilePrefix);

	//! Convert command in sentence to command code
	int getSpeakCommand(const Person *person, const char *sentence, unsigned &index);

	//! Speak a part of a sentence
	void speakSegment(
			const char *segmentStart,
			int length,
			Person *person,
			int command,
			const char *voiceFilePrefix,
			int index);

	void headStringAnimation(const SpeechParameters *parameters, int bobNum, int bankNum);

	void stringAnimation(const SpeechParameters *parameters, int startFrame, int bankNum);

	void defaultAnimation(
		const char *segment,
		bool isJoe,
		const SpeechParameters *parameters,
		int startFrame,
		int bankNum);

	int countSpaces(const char *segment);

	//! Get special parameters for speech
	const SpeechParameters *findSpeechParameters(
			const char *name,
			int state,
			int faceDirection);

	int splitOption(const char *str, char optionText[5][MAX_STRING_SIZE]);

	int splitOptionHebrew(const char *str, char optionText[5][MAX_STRING_SIZE]);

	int splitOptionDefault(const char *str, char optionText[5][MAX_STRING_SIZE]);

};

} // End of namespace Queen

#endif
