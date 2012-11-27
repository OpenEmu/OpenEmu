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

#ifndef QUEEN_CUTAWAY_H
#define QUEEN_CUTAWAY_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

class QueenEngine;

class Cutaway {
public:

	//! Public interface to run a cutaway from a file
	static void run(const char *filename, char *nextFilename, QueenEngine *vm);

	//! Collection of constants used by QueenCutaway
	enum {
		PREVIOUS_ROOM = 0,
		CURRENT_ROOM = 0,
		OBJECT_ROOMFADE = -1,
		PERSON_JOE = -1,
		OBJECT_JOE = 0,
		MAX_PERSON_COUNT = 6,
		CUTAWAY_BANK = 8,
		MAX_BANK_NAME_COUNT = 5,
		MAX_FILENAME_LENGTH = 12,
		MAX_FILENAME_SIZE = (MAX_FILENAME_LENGTH + 1),
		MAX_PERSON_FACE_COUNT = 13,
		MAX_STRING_LENGTH = 255,
		MAX_STRING_SIZE = (MAX_STRING_LENGTH + 1),
		LEFT = 1,
		RIGHT = 2,
		FRONT = 3,
		BACK = 4
	};

	//! Different kinds of cutaway objects
	enum ObjectType {
		OBJECT_TYPE_ANIMATION               = 0,
		OBJECT_TYPE_PERSON                  = 1,
		OBJECT_TYPE_NO_ANIMATION            = 2,
		OBJECT_TYPE_TEXT_SPEAK              = 3,
		OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK  = 4,
		OBJECT_TYPE_TEXT_DISPLAY            = 5
	};

private:
	//! Data for a cutaway object
	struct CutawayObject {
		int16 objectNumber;    // 0 = JOE, -1 = MESSAGE
		int16 moveToX;
		int16 moveToY;
		int16 bank;             // 0 = PBOB, 13 = Joe Bank, else BANK NAMEstr()
		int16 animList;
		int16 execute;          // 1 Yes, 0 No
		int16 limitBobX1;
		int16 limitBobY1;
		int16 limitBobX2;
		int16 limitBobY2;
		int16 specialMove;
		int16 animType;        // 0 - Packet, 1 - Amal, 2 - Unpack
		int16 fromObject;
		int16 bobStartX;
		int16 bobStartY;
		int16 room;
		int16 scale;
		// Variables derived from the variables above
		int song;

		//! People to turn on
		int person[MAX_PERSON_COUNT];

		//! Number of elements used in _person array
		int personCount;
	};

	struct CutawayAnim {
		int16 object;
		int16 unpackFrame;          // Frame to unpack
		int16 speed;
		int16 bank;
		int16 mx;
		int16 my;
		int16 cx;
		int16 cy;
		int16 scale;
		int16 currentFrame;  // Index to Current Frame
		int16 originalFrame;  // Index to Original Object Frame
		int16 song;
		bool flip;		// set this if unpackFrame is negative
	};

	struct ObjectDataBackup {
		int index;
		int16 name;
		int16 image;
	};

	struct PersonFace {
	  int16 index;
	  int16 image;
	};

	QueenEngine *_vm;

	//! Raw .cut file data (without 20 byte header)
	byte *_fileData;

	//! COMPANEL
	int16 _comPanel;

	//! Game state data inside of _fileDat
	byte *_gameStatePtr;

	//! Actual cutaway data inside of _fileData
	byte *_objectData;

	//! Pointer to next sentence string in _fileData
	uint16 _nextSentenceOff;

	//! ???
	bool _roomFade;

	//! Number of cutaway objects at _cutawayData
	int16 _cutawayObjectCount;

	//! This cutaway is followed by another
	bool _anotherCutaway;

	//! Room before cutaway
	int _initialRoom;

	//! Temporary room for cutaway
	int _temporaryRoom;

	//! Room to stay in
	int _finalRoom;

	//! Bank names
	char _bankNames[MAX_BANK_NAME_COUNT][MAX_FILENAME_SIZE];

	//! Filename without ".cut"
	char _basename[MAX_FILENAME_SIZE];

	//! Name of .dog file
	char _talkFile[MAX_FILENAME_SIZE];

	//! Person to talk to
	int16 _talkTo;

	//! Used by changeRooms
	ObjectDataBackup _personData[MAX_PERSON_COUNT];

	//! Number of elements used in _personData array
	int _personDataCount;

	//! Used by handlePersonRecord()
	PersonFace _personFace[MAX_PERSON_FACE_COUNT];

	//! Number of entries in _personFace array
	int _personFaceCount;

	//! Play this song when leaving cutaway
	int16 _lastSong;

	//! Song played before running comic.cut
	int16 _songBeforeComic;

	int16 _currentImage;

	Cutaway(const char *filename, QueenEngine *vm);
	~Cutaway();

	//! Run this cutaway object
	void run(char *nextFilename);

	//! Load cutaway data from file
	void load(const char *filename);

	//! Used by load to read string data
	void loadStrings(uint16 offset);

	//! Get persons
	const byte *turnOnPeople(const byte *ptr, CutawayObject &object);

	//! Limit the BOB
	void limitBob(CutawayObject &object);

	//! This cutaway object occurs in another room
	void changeRooms(CutawayObject &object);

	//! Get the object type for this CutawayObject
	ObjectType getObjectType(CutawayObject &object);

	//! Perform actions for an animation
	const byte *handleAnimation(const byte *ptr, CutawayObject &object);

	//! Perform actions for a person record
	void handlePersonRecord(int index, CutawayObject &object, const char *sentence);

	//! Perform text actions
	void handleText(int index, ObjectType type, CutawayObject &object, const char *sentence);

	//! Restore Logic::_objectData from _personData
	void restorePersonData();

	//! Stop the cutaway
	void stop();

	//! Update game state after cutaway
	void updateGameState();

	//! Prepare for talk after cutaway
	void talk(char *nextFilename);

	//! Get CutawayAnim data from ptr and return new ptr
	const byte *getCutawayAnim(const byte *ptr, int header, CutawayAnim &anim);

	//! Special animation
	int makeComplexAnimation(int16 currentImage, CutawayAnim *objAnim, int frameCount);

	//! Read a CutawayObject from ptr and return new ptr
	static const byte *getCutawayObject(const byte *ptr, CutawayObject &object);

	//! Dump a CutawayObject with debug()
	void dumpCutawayObject(int index, CutawayObject &object);

	//! Used by handleText()
	int countSpaces(ObjectType type, const char *segment);

	//! Scale Joe
	int scale(CutawayObject &object);

	//! Dump CutawayAnum data with debug()
	static void dumpCutawayAnim(CutawayAnim &anim);

	bool inRange(int16 x, int16 l, int16 h) const { return (x <= h && x >= l); }
};

} // End of namespace Queen

#endif
