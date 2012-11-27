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

#ifndef HUGO_SCHEDULE_H
#define HUGO_SCHEDULE_H

#include "common/file.h"

namespace Hugo {

/**
 * Following defines the action types and action list
 */
enum Action {                                       // Parameters:
	ANULL              = 0xff,                      // Special NOP used to 'delete' events in DEL_EVENTS
	ASCHEDULE          = 0,                         //  0 - Ptr to action list to be rescheduled
	START_OBJ,                                      //  1 - Object number
	INIT_OBJXY,                                     //  2 - Object number, x,y
	PROMPT,                                         //  3 - index of prompt & response string, ptrs to action
	                                                //      lists.  First if response matches, 2nd if not.
	BKGD_COLOR,                                     //  4 - new background color
	INIT_OBJVXY,                                    //  5 - Object number, vx, vy
	INIT_CARRY,                                     //  6 - Object number, carried status
	INIT_HF_COORD,                                  //  7 - Object number (gets hero's 'feet' coordinates)
	NEW_SCREEN,                                     //  8 - New screen number
	INIT_OBJSTATE,                                  //  9 - Object number, new object state
	INIT_PATH,                                      // 10 - Object number, new path type
	COND_R,                                         // 11 - Conditional on object state - req state, 2 act_lists
	TEXT,                                           // 12 - Simple text box
	SWAP_IMAGES,                                    // 13 - Swap 2 object images
	COND_SCR,                                       // 14 - Conditional on current screen
	AUTOPILOT,                                      // 15 - Set object to home in on another (stationary) object
	INIT_OBJ_SEQ,                                   // 16 - Object number, sequence index to set curr_seqPtr to
	SET_STATE_BITS,                                 // 17 - Objnum, mask to OR with obj states word
	CLEAR_STATE_BITS,                               // 18 - Objnum, mask to ~AND with obj states word
	TEST_STATE_BITS,                                // 19 - Objnum, mask to test obj states word
	DEL_EVENTS,                                     // 20 - Action type to delete all occurrences of
	GAMEOVER,                                       // 21 - Disable hero & commands.  Game is over
	INIT_HH_COORD,                                  // 22 - Object number (gets hero's actual coordinates)
	EXIT,                                           // 23 - Exit game back to DOS
	BONUS,                                          // 24 - Get score bonus for an action
	COND_BOX,                                       // 25 - Conditional on object within bounding box
	SOUND,                                          // 26 - Set currently playing sound
	ADD_SCORE,                                      // 27 - Add object's value to current score
	SUB_SCORE,                                      // 28 - Subtract object's value from current score
	COND_CARRY,                                     // 29 - Conditional on carrying object
	INIT_MAZE,                                      // 30 - Start special maze hotspot processing
	EXIT_MAZE,                                      // 31 - Exit special maze processing
	INIT_PRIORITY,                                  // 32 - Initialize fbg field
	INIT_SCREEN,                                    // 33 - Initialize screen field of object
	AGSCHEDULE,                                     // 34 - Global schedule - lasts over new screen
	REMAPPAL,                                       // 35 - Remappe palette - palette index, color
	COND_NOUN,                                      // 36 - Conditional on noun appearing in line
	SCREEN_STATE,                                   // 37 - Set new screen state - used for comments
	INIT_LIPS,                                      // 38 - Position lips object for supplied object
	INIT_STORY_MODE,                                // 39 - Set story mode TRUE/FALSE (user can't type)
	WARN,                                           // 40 - Same as TEXT but can't dismiss box by typing
	COND_BONUS,                                     // 41 - Conditional on bonus having been scored
	TEXT_TAKE,                                      // 42 - Issue text box with "take" info string
	YESNO,                                          // 43 - Prompt user for Yes or No
	STOP_ROUTE,                                     // 44 - Skip any route in progress (hero still walks)
	COND_ROUTE,                                     // 45 - Conditional on route in progress
	INIT_JUMPEXIT,                                  // 46 - Initialize status.jumpexit
	INIT_VIEW,                                      // 47 - Initialize viewx, viewy, dir
	INIT_OBJ_FRAME,                                 // 48 - Object number, seq,frame to set curr_seqPtr to
	OLD_SONG           = 49                         // Added by Strangerke - Set currently playing sound, old way: that is, using a string index instead of a reference in a file
};

struct act0 {                                       // Type 0 - Schedule
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	uint16   _actIndex;                             // Ptr to an action list
};

struct act1 {                                       // Type 1 - Start an object
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _cycleNumb;                            // Number of times to cycle
	Cycle    _cycle;                                // Direction to start cycling
};

struct act2 {                                       // Type 2 - Initialize an object coords
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _x, _y;                                // Coordinates
};

struct act3 {                                       // Type 3 - Prompt user for text
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	uint16   _promptIndex;                          // Index of prompt string
	int     *_responsePtr;                          // Array of indexes to valid response string(s) (terminate list with -1)
	uint16   _actPassIndex;                         // Ptr to action list if success
	uint16   _actFailIndex;                         // Ptr to action list if failure
	bool     _encodedFl;                            // (HUGO 1 DOS ONLY) Whether response is encoded or not
};

struct act4 {                                       // Type 4 - Set new background color
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	long     _newBackgroundColor;                   // New color
};

struct act5 {                                       // Type 5 - Initialize an object velocity
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _vx, _vy;                              // velocity
};

struct act6 {                                       // Type 6 - Initialize an object carrying
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	bool     _carriedFl;                            // carrying
};

struct act7 {                                       // Type 7 - Initialize an object to hero's coords
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
};

struct act8 {                                       // Type 8 - switch to new screen
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _screenIndex;                          // The new screen number
};

struct act9 {                                       // Type 9 - Initialize an object state
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	byte     _newState;                             // New state
};

struct act10 {                                      // Type 10 - Initialize an object path type
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _newPathType;                          // New path type
	int8     _vxPath, _vyPath;                      // Max delta velocities e.g. for CHASE
};

struct act11 {                                      // Type 11 - Conditional on object's state
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	byte     _stateReq;                             // Required state
	uint16   _actPassIndex;                         // Ptr to action list if success
	uint16   _actFailIndex;                         // Ptr to action list if failure
};

struct act12 {                                      // Type 12 - Simple text box
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _stringIndex;                          // Index (enum) of string in strings.dat
};

struct act13 {                                      // Type 13 - Swap first object image with second
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex1;                            // Index of first object
	int      _objIndex2;                            // 2nd
};

struct act14 {                                      // Type 14 - Conditional on current screen
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The required object
	int      _screenReq;                            // The required screen number
	uint16   _actPassIndex;                         // Ptr to action list if success
	uint16   _actFailIndex;                         // Ptr to action list if failure
};

struct act15 {                                      // Type 15 - Home in on an object
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex1;                            // The object number homing in
	int      _objIndex2;                            // The object number to home in on
	int8     _dx, _dy;                              // Max delta velocities
};

// Note: Don't set a sequence at time 0 of a new screen, it causes
// problems clearing the boundary bits of the object!  timer > 0 is safe
struct act16 {                                      // Type 16 - Set curr_seqPtr to seq
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _seqIndex;                             // The index of seq array to set to
};

struct act17 {                                      // Type 17 - SET obj individual state bits
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _stateMask;                            // The mask to OR with current obj state
};

struct act18 {                                      // Type 18 - CLEAR obj individual state bits
	Action _actType;                                // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _stateMask;                            // The mask to ~AND with current obj state
};

struct act19 {                                      // Type 19 - TEST obj individual state bits
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _stateMask;                            // The mask to AND with current obj state
	uint16   _actPassIndex;                         // Ptr to action list (all bits set)
	uint16   _actFailIndex;                         // Ptr to action list (not all set)
};

struct act20 {                                      // Type 20 - Remove all events with this type of action
	Action _actType;                                // The type of action
	int    _timer;                                  // Time to set off the action
	Action _actTypeDel;                             // The action type to remove
};

struct act21 {                                      // Type 21 - Gameover.  Disable hero & commands
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
};

struct act22 {                                      // Type 22 - Initialize an object to hero's coords
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
};

struct act23 {                                      // Type 23 - Exit game back to DOS
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
};

struct act24 {                                      // Type 24 - Get bonus score
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _pointIndex;                           // Index into points array
};

struct act25 {                                      // Type 25 - Conditional on bounding box
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The required object number
	int      _x1, _y1, _x2, _y2;                    // The bounding box
	uint16   _actPassIndex;                         // Ptr to action list if success
	uint16   _actFailIndex;                         // Ptr to action list if failure
};

struct act26 {                                      // Type 26 - Play a sound
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int16    _soundIndex;                           // Sound index in data file
};

struct act27 {                                      // Type 27 - Add object's value to score
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // object number
};

struct act28 {                                      // Type 28 - Subtract object's value from score
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // object number
};

struct act29 {                                      // Type 29 - Conditional on object carried
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The required object number
	uint16   _actPassIndex;                         // Ptr to action list if success
	uint16   _actFailIndex;                         // Ptr to action list if failure
};

struct act30 {                                      // Type 30 - Start special maze processing
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	byte     _mazeSize;                             // Size of (square) maze
	int      _x1, _y1, _x2, _y2;                    // Bounding box of maze
	int      _x3, _x4;                              // Extra x points for perspective correction
	byte     _firstScreenIndex;                     // First (top left) screen of maze
};

struct act31 {                                      // Type 31 - Exit special maze processing
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
};

struct act32 {                                      // Type 32 - Init fbg field of object
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	byte     _priority;                             // Value of foreground/background field
};

struct act33 {                                      // Type 33 - Init screen field of object
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _screenIndex;                          // Screen number
};

struct act34 {                                      // Type 34 - Global Schedule
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	uint16   _actIndex;                             // Ptr to an action list
};

struct act35 {                                      // Type 35 - Remappe palette
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int16    _oldColorIndex;                        // Old color index, 0..15
	int16    _newColorIndex;                        // New color index, 0..15
};

struct act36 {                                      // Type 36 - Conditional on noun mentioned
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	uint16   _nounIndex;                            // The required noun (list)
	uint16   _actPassIndex;                         // Ptr to action list if success
	uint16   _actFailIndex;                         // Ptr to action list if failure
};

struct act37 {                                      // Type 37 - Set new screen state
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _screenIndex;                          // The screen number
	byte     _newState;                             // The new state
};

struct act38 {                                      // Type 38 - Position lips
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _lipsObjIndex;                         // The LIPS object
	int      _objIndex;                             // The object to speak
	byte     _dxLips;                               // Relative offset of x
	byte     _dyLips;                               // Relative offset of y
};

struct act39 {                                      // Type 39 - Init story mode
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	bool     _storyModeFl;                          // New state of story_mode flag
};

struct act40 {                                      // Type 40 - Unsolicited text box
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _stringIndex;                          // Index (enum) of string in strings.dat
};

struct act41 {                                      // Type 41 - Conditional on bonus scored
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _bonusIndex;                           // Index into bonus list
	uint16   _actPassIndex;                         // Index of the action list if scored for the first time
	uint16   _actFailIndex;                         // Index of the action list if already scored
};

struct act42 {                                      // Type 42 - Text box with "take" string
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object taken
};

struct act43 {                                      // Type 43 - Prompt user for Yes or No
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _promptIndex;                          // index of prompt string
	uint16   _actYesIndex;                          // Ptr to action list if YES
	uint16   _actNoIndex;                           // Ptr to action list if NO
};

struct act44 {                                      // Type 44 - Stop any route in progress
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
};

struct act45 {                                      // Type 45 - Conditional on route in progress
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _routeIndex;                           // Must be >= current status.rindex
	uint16   _actPassIndex;                         // Ptr to action list if en-route
	uint16   _actFailIndex;                         // Ptr to action list if not
};

struct act46 {                                      // Type 46 - Init status.jumpexit
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	bool     _jumpExitFl;                           // New state of jumpexit flag
};

struct act47 {                                      // Type 47 - Init viewx,viewy,dir
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object
	int16    _viewx;                                // object.viewx
	int16    _viewy;                                // object.viewy
	int16    _direction;                            // object.dir
};

struct act48 {                                      // Type 48 - Set curr_seqPtr to frame n
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	int      _objIndex;                             // The object number
	int      _seqIndex;                             // The index of seq array to set to
	int      _frameIndex;                           // The index of frame to set to
};

struct act49 {                                      // Added by Strangerke - Type 49 - Play a song (DOS way)
	Action   _actType;                              // The type of action
	int      _timer;                                // Time to set off the action
	uint16   _songIndex;                            // Song index in string array
};

union Act {
	act0     _a0;
	act1     _a1;
	act2     _a2;
	act3     _a3;
	act4     _a4;
	act5     _a5;
	act6     _a6;
	act7     _a7;
	act8     _a8;
	act9     _a9;
	act10    _a10;
	act11    _a11;
	act12    _a12;
	act13    _a13;
	act14    _a14;
	act15    _a15;
	act16    _a16;
	act17    _a17;
	act18    _a18;
	act19    _a19;
	act20    _a20;
	act21    _a21;
	act22    _a22;
	act23    _a23;
	act24    _a24;
	act25    _a25;
	act26    _a26;
	act27    _a27;
	act28    _a28;
	act29    _a29;
	act30    _a30;
	act31    _a31;
	act32    _a32;
	act33    _a33;
	act34    _a34;
	act35    _a35;
	act36    _a36;
	act37    _a37;
	act38    _a38;
	act39    _a39;
	act40    _a40;
	act41    _a41;
	act42    _a42;
	act43    _a43;
	act44    _a44;
	act45    _a45;
	act46    _a46;
	act47    _a47;
	act48    _a48;
	act49    _a49;
};

struct Event {
	Act           *_action;                         // Ptr to action to perform
	bool           _localActionFl;                  // true if action is only for this screen
	uint32         _time;                           // (absolute) time to perform action
	struct Event  *_prevEvent;                      // Chain to previous event
	struct Event  *_nextEvent;                      // Chain to next event
};

/**
 * Following are points for achieving certain actions.
 */
struct Point {
	byte _score;                                    // The value of the point
	bool _scoredFl;                                 // Whether scored yet
};

class Scheduler {
public:
	Scheduler(HugoEngine *vm);
	virtual ~Scheduler();

	virtual void decodeString(char *line) = 0;
	virtual void runScheduler() = 0;

	int16 calcMaxPoints() const;

	void freeScheduler();
	void initCypher();
	void initEventQueue();
	void insertActionList(const uint16 actIndex);
	void loadActListArr(Common::ReadStream &in);
	void loadAlNewscrIndex(Common::ReadStream &in);
	void loadPoints(Common::SeekableReadStream &in);
	void loadScreenAct(Common::SeekableReadStream &in);
	void newScreen(const int screenIndex);
	void processBonus(const int bonusIndex);
	void processMaze(const int x1, const int x2, const int y1, const int y2);
	void restoreSchedulerData(Common::ReadStream *in);
	void restoreScreen(const int screenIndex);
	void saveSchedulerData(Common::WriteStream *out);
	void waitForRefresh();

protected:
	HugoEngine *_vm;
	static const int kFilenameLength = 12;          // Max length of a DOS file name
	static const int kMaxEvents = 50;               // Max events in event queue
	static const int kShiftSize = 8;                // Place hero this far inside bounding box

	Common::String _cypher;

	uint16   _actListArrSize;
	uint16   _alNewscrIndex;
	uint16   _screenActsSize;
	uint16 **_screenActs;

	byte     _numBonuses;
	Point   *_points;

	uint32 _curTick;                                // Current system time in ticks
	uint32 _oldTime;                                // The previous wall time in ticks
	uint32 _refreshTimeout;

	Event *_freeEvent;                            // Free list of event structures
	Event *_headEvent;                            // Head of list (earliest time)
	Event *_tailEvent;                            // Tail of list (latest time)
	Event  _events[kMaxEvents];                   // Statically declare event structures

	Act    **_actListArr;

	virtual const char *getCypher() const = 0;

	virtual uint32 getTicks() = 0;

	virtual void promptAction(Act *action) = 0;

	Event *doAction(Event *curEvent);
	Event *getQueue();

	uint32 getDosTicks(const bool updateFl);
	uint32 getWinTicks() const;

	void delEventType(const Action actTypeDel);
	void delQueue(Event *curEvent);
	void findAction(const Act* action, int16* index, int16* subElem);
	void insertAction(Act *action);
	void readAct(Common::ReadStream &in, Act &curAct);
	void restoreActions(Common::ReadStream *f);
	void restoreEvents(Common::ReadStream *f);
	void restorePoints(Common::ReadStream *in);
	void saveActions(Common::WriteStream* f) const;
	void saveEvents(Common::WriteStream *f);
	void savePoints(Common::WriteStream *out) const;
	void screenActions(const int screenNum);

};

class Scheduler_v1d : public Scheduler {
public:
	Scheduler_v1d(HugoEngine *vm);
	~Scheduler_v1d();

	virtual void decodeString(char *line);
	virtual void runScheduler();

protected:
	virtual const char *getCypher() const;
	virtual uint32 getTicks();
	virtual void promptAction(Act *action);
};

class Scheduler_v2d : public Scheduler_v1d {
public:
	Scheduler_v2d(HugoEngine *vm);
	virtual ~Scheduler_v2d();

	void decodeString(char *line);

protected:
	virtual const char *getCypher() const;

	void promptAction(Act *action);
};

class Scheduler_v3d : public Scheduler_v2d {
public:
	Scheduler_v3d(HugoEngine *vm);
	~Scheduler_v3d();

protected:
	const char *getCypher() const;
};

class Scheduler_v1w : public Scheduler_v3d {
public:
	Scheduler_v1w(HugoEngine *vm);
	~Scheduler_v1w();

	void runScheduler();

protected:
	uint32 getTicks();

};
} // End of namespace Hugo
#endif //HUGO_SCHEDULE_H
