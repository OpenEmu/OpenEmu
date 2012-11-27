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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_PARSER_H
#define HUGO_PARSER_H

namespace Common {
struct Event;
}

namespace Hugo {

enum seqTextParser {
	kTBExit  = 0, kTBMaze,    kTBNoPoint, kTBNoun,       kTBVerb,
	kTBEh,        kTBUnusual, kTBHave,    kTBNoUse,      kTBDontHave,
	kTBNeed,      kTBOk,      kCmtAny1,   kCmtAny2,      kCmtAny3,
	kCmtClose,    kTBIntro,   kTBOutro,   kTBUnusual_1d, kCmtAny4,
	kCmtAny5,     kTBExit_1d, kTBEh_1d,   kTBEh_2d,      kTBNoUse_2d
};

/**
 * The following determines how a verb is acted on, for an object
 */
struct cmd {
	uint16 _verbIndex;                               // the verb
	uint16 _reqIndex;                                // ptr to list of required objects
	uint16 _textDataNoCarryIndex;                    // ptr to string if any of above not carried
	byte   _reqState;                                // required state for verb to be done
	byte   _newState;                                // new states if verb done
	uint16 _textDataWrongIndex;                      // ptr to string if wrong state
	uint16 _textDataDoneIndex;                       // ptr to string if verb done
	uint16 _actIndex;                                // Ptr to action list if verb done
};

/**
 * Following is structure of verbs and nouns for 'background' objects
 * These are objects that appear in the various screens, but nothing
 * interesting ever happens with them.  Rather than just be dumb and say
 * "don't understand" we produce an interesting msg to keep user sane.
 */
struct Background {
	uint16 _verbIndex;
	uint16 _nounIndex;
	int    _commentIndex;                            // Index of comment produced on match
	bool   _matchFl;                                 // TRUE if noun must match when present
	byte   _roomState;                               // "State" of room. Comments might differ.
	byte   _bonusIndex;                              // Index of bonus score (0 = no bonus)
};

typedef Background *ObjectList;

class Parser {
public:
	Parser(HugoEngine *vm);
	virtual ~Parser();

	bool isWordPresent(char **wordArr) const;

	uint16 getCmdDefaultVerbIdx(const uint16 index) const;

	void charHandler();
	void command(const char *format, ...);
	void freeParser();
	void keyHandler(Common::Event event);
	void loadArrayReqs(Common::SeekableReadStream &in);
	void loadBackgroundObjects(Common::ReadStream &in);
	void loadCatchallList(Common::ReadStream &in);
	void loadCmdList(Common::ReadStream &in);
	void switchTurbo();
	const char *useBG(const char *name);

	virtual void lineHandler() = 0;
	virtual void showInventory() const = 0;
	virtual void takeObject(Object *obj) = 0;

protected:
	HugoEngine *_vm;

	int16     _cmdLineIndex;                        // Index into line
	uint32    _cmdLineTick;                         // For flashing cursor
	char      _cmdLineCursor;
	Command   _cmdLine;                             // Build command line
	uint16    _backgroundObjectsSize;
	uint16    _cmdListSize;

	uint16       **_arrayReqs;
	Background **_backgroundObjects;
	Background  *_catchallList;
	cmd          **_cmdList;

	const char *findNoun() const;
	const char *findVerb() const;
	void  readBG(Common::ReadStream &in, Background &curBG);
	void  readCmd(Common::ReadStream &in, cmd &curCmd);
	void  showDosInventory() const;

	bool   _checkDoubleF1Fl;                        // Flag used to display user help or instructions
	uint16 _getIndex;                               // Index into ring buffer
	uint16 _putIndex;
	char   _ringBuffer[32];                         // Ring buffer

private:
	static const int kBlinksPerSec = 2;             // Cursor blinks per second
};

class Parser_v1d : public Parser {
public:
	Parser_v1d(HugoEngine *vm);
	~Parser_v1d();

	virtual void lineHandler();
	virtual void showInventory() const;
	virtual void takeObject(Object *obj);

protected:
	virtual void dropObject(Object *obj);

	const char *findNextNoun(const char *noun) const;
	bool  isBackgroundWord_v1(const char *noun, const char *verb, ObjectList obj) const;
	bool  isCatchallVerb_v1(bool testNounFl, const char *noun, const char *verb, ObjectList obj) const;
	bool  isGenericVerb_v1(const char *word, Object *obj);
	bool  isNear_v1(const char *verb, const char *noun, Object *obj, char *comment) const;
	bool  isObjectVerb_v1(const char *word, Object *obj);
};

class Parser_v2d : public Parser_v1d {
public:
	Parser_v2d(HugoEngine *vm);
	~Parser_v2d();

	void lineHandler();
};

class Parser_v3d : public Parser_v1d {
public:
	Parser_v3d(HugoEngine *vm);
	~Parser_v3d();

	virtual void lineHandler();
protected:
	void  dropObject(Object *obj);
	bool  isBackgroundWord_v3(ObjectList obj) const;
	bool  isCatchallVerb_v3(ObjectList obj) const;
	bool  isGenericVerb_v3(Object *obj, char *comment);
	bool  isNear_v3(Object *obj, const char *verb, char *comment) const;
	bool  isObjectVerb_v3(Object *obj, char *comment);
	void  takeObject(Object *obj);
};

class Parser_v1w : public Parser_v3d {
public:
	Parser_v1w(HugoEngine *vm);
	~Parser_v1w();

	virtual void showInventory() const;

	void  lineHandler();
};

} // End of namespace Hugo

#endif //HUGO_PARSER_H
