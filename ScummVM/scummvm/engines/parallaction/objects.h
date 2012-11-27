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

#ifndef PARALLACTION_ZONE_H
#define PARALLACTION_ZONE_H

#include "common/list.h"
#include "common/ptr.h"

#include "parallaction/graphics.h"


namespace Parallaction {

struct Zone;
struct Animation;
struct Command;
struct Question;
struct Answer;
struct Instruction;
struct Program;

typedef Common::SharedPtr<Zone> ZonePtr;
typedef Common::List<ZonePtr> ZoneList;

typedef Common::SharedPtr<Animation> AnimationPtr;
typedef Common::List<AnimationPtr> AnimationList;

typedef Common::SharedPtr<Instruction> InstructionPtr;
typedef Common::Array<InstructionPtr> InstructionList;

typedef Common::List<Common::Point> PointList;

enum ZoneTypes {
	kZoneExamine	   = 1,					// zone displays comment if activated
	kZoneDoor		   = 2,					// zone activated on click (after some walk if needed)
	kZoneGet		   = 3,					// for pickable items
	kZoneMerge		   = 4,					// tags items which can be merged in inventory
	kZoneTaste		   = 5,				// NEVER USED
	kZoneHear		   = 6,				// NEVER USED: they ran out of time before integrating sfx
	kZoneFeel		   = 7,				// NEVER USED
	kZoneSpeak		   = 8,				// tags NPCs the character can talk with
	kZoneNone		   = 9,				// used to prevent parsing on peculiar Animations
	kZoneTrap		   = 10,				// zone activated when character enters
	kZoneYou		   = 11,				// marks the character
	kZoneCommand	   = 12,

	// BRA specific
	kZonePath          = 13,			// defines nodes for assisting walk calculation routines
	kZoneBox           = 14
};


enum ZoneFlags {
	kFlagsClosed		= 1,				// Zone: door is closed / switch is off
	kFlagsActive		= 2,				// Zone/Animation: object is visible
	kFlagsRemove		= 4,				// Zone/Animation: object is soon to be removed
	kFlagsActing		= 8,				// Animation: script execution is active
	kFlagsLocked		= 0x10,				// Zone: door or switch cannot be toggled
	kFlagsFixed			= 0x20,				// Zone: Zone item cannot be picked up
	kFlagsNoName		= 0x40,				// Zone with no name (used to prevent some kEvEnterZone events)
	kFlagsNoMasked		= 0x80,				// Animation is to be drawn ignoring z buffer
	kFlagsLooping		= 0x100,			// Animation: script is to be executed repeatedly
	kFlagsAdded			= 0x200,			// NEVER USED in Nippon Safes
	kFlagsCharacter		= 0x400,			//
	kFlagsNoWalk		= 0x800,			// Zone: character doesn't need to walk towards object to interact

	// BRA specific
	kFlagsYourself		= 0x1000,			// BRA: marks zones used by the character on him/herself
	kFlagsScaled		= 0x2000,
	kFlagsSelfuse		= 0x4000,			// BRA: marks zones to be preserved across location changes (see Parallaction::freeZones)
	kFlagsIsAnimation	= 0x1000000,		// BRA: used in walk code (trap check), to tell is a Zone is an Animation
	kFlagsAnimLinked	= 0x2000000
};


enum CommandFlags {
	kFlagsAll			= 0xFFFFFFFFU,

	kFlagsVisited		= 1,
	kFlagsExit			= 0x10000000,
	kFlagsEnter			= 0x20000000,
	kFlagsGlobal		= 0x40000000,

	// BRA specific
	kFlagsTestTrue		= 2
};

struct Command {
	uint16			_id;
	uint32			_flagsOn;
	uint32			_flagsOff;
	bool			_valid;

	Command();
	~Command();

	// Common fields
	uint32			_flags;
	ZonePtr			_zone;
	Common::String	_zoneName;
	char*			_string;
	uint16			_callable;
	uint16			_object;
	Common::Point	 _move;

	// BRA specific
	Common::Point	_startPos;
	Common::Point	_startPos2;
	Common::String	_counterName;
	int				_counterValue;
	int				_zeta0;
	int				_zeta1;
	int				_zeta2;
	int				_characterId;
	char*			_string2;
	int				_musicCommand;
	int				_musicParm;
};

typedef Common::SharedPtr<Command> CommandPtr;
typedef Common::List<CommandPtr> CommandList;


#define NUM_QUESTIONS		40
#define NUM_ANSWERS			20

struct Answer {
	Common::String	_text;
	uint16			_mood;
	Common::String 	_followingName;

	CommandList	_commands;
	uint32		_noFlags;
	uint32		_yesFlags;

	// BRA specific
	bool _hasCounterCondition;
	Common::String	_counterName;
	int	_counterValue;
	int	_counterOp;

	Answer();
	bool textIsNull();
	int speakerMood();
};

struct Question {
	Common::String	_name;
	Common::String	_text;
	uint16			_mood;
	Answer*			_answers[NUM_ANSWERS];

	Question(const Common::String &name);
	~Question();
	bool textIsNull();
	int speakerMood();
	int balloonWinding();
};

struct Dialogue {
	Question	*_questions[NUM_QUESTIONS];
	uint		_numQuestions;

	Question *findQuestion(const Common::String &name) const;
	void addQuestion(Question *q);

	Dialogue();
	~Dialogue();
};

#define MAX_WALKPOINT_LISTS 	20
#define FREE_HEAR_CHANNEL		-1
#define MUSIC_HEAR_CHANNEL		-2

struct TypeData {
	// common
	GfxObj		*_gfxobj;	// get, examine, door
	Common::String	_filename; // speak, examine, hear

	// get
	uint32		_getIcon;

	// speak
	Dialogue		*_speakDialogue;

	// examine
	Common::String	_examineText;

	// door
	Common::String	_doorLocation;
	Common::Point	_doorStartPos;
	uint16		_doorStartFrame;
	Common::Point	_doorStartPos2_br;
	uint16		_doorStartFrame2_br;

	// hear
	int		_hearChannel;
	int		_hearFreq;

	// merge
	uint32	_mergeObj1;
	uint32	_mergeObj2;
	uint32	_mergeObj3;

	// path
	int		_pathNumLists;
	PointList	_pathLists[MAX_WALKPOINT_LISTS];

	TypeData() {
		_gfxobj = 0;
		_getIcon = 0;
		_speakDialogue = 0;
		_doorStartFrame = 0;
		_doorStartPos.x = -1000;
		_doorStartPos.y = -1000;
		_doorStartFrame2_br = 0;
		_doorStartPos2_br.x = -1000;
		_doorStartPos2_br.y = -1000;
		_hearChannel = FREE_HEAR_CHANNEL;
		_hearFreq = -1;
		_mergeObj1 = 0;
		_mergeObj2 = 0;
		_mergeObj3 = 0;
		_pathNumLists = 0;
	}

	~TypeData() {
		if (_gfxobj) {
			_gfxobj->release();
		}
		delete _speakDialogue;
	}
};

#define ACTIONTYPE(z) ((z)->_type & 0xFFFF)
#define ITEMTYPE(z) ((z)->_type & 0xFFFF0000)

#define PACK_ZONETYPE(zt,it) (((zt) & 0xFFFF) | (((it) & 0xFFFF) << 16))

#define ZONENAME_LENGTH 32

#define INVALID_LOCATION_INDEX ((uint32)-1)
#define INVALID_ZONE_INDEX ((uint32)-1)

struct Zone {
private:
	int16			_right;
	int16			_bottom;

protected:
	int16			_left;
	int16			_top;

public:
	char			_name[ZONENAME_LENGTH];

	uint32			_type;
	uint32			_flags;
	GfxObj			*_label;

	TypeData		u;
	CommandList		_commands;
	Common::Point	_moveTo;

	// BRA specific
	uint			_index;
	uint			_locationIndex;
	Common::String	_linkedName;
	AnimationPtr	_linkedAnim;

	Zone();
	virtual ~Zone();

	void translate(int16 x, int16 y);

	bool hitRect(int x, int y) const;

	void setRect(int16 left, int16 top, int16 right, int16 bottom) {
		setX(left);
		setY(top);
		_right = right;
		_bottom = bottom;
	}

	void getRect(Common::Rect& r) {
		r.left = _left;	r.right = _right;
		r.top = _top; r.bottom = _bottom;
	}


	// getters/setters
	virtual int16 getX()			{ return _left; }
	virtual void  setX(int16 value) { _left = value; }

	virtual int16 getY()			{ return _top; }
	virtual void  setY(int16 value) { _top = value; }
};


struct LocalVariable {
protected:
	int16		_value;
	int16		_min;
	int16		_max;

public:

	LocalVariable() {
		_value = 0;
		_min = -10000;
		_max = 10000;
	}

	void setRange(int16 min, int16 max);

	int16 getValue() const;
	void setValue(int16 value);
};


enum ParaFlags {
	kParaImmediate	= 1,				// instruction is using an immediate parameter
	kParaLocal		= 2,				// instruction is using a local variable
	kParaField		= 0x10,				// instruction is using an animation's field
	kParaRandom		= 0x100,

	kParaLValue		= 0x20
};


struct AnimationField {
	typedef Common::Functor0Mem<int16, Animation> Accessor;
	typedef Common::Functor1Mem<int16, void, Animation> Mutator;

	typedef Accessor::FuncType AccessorFunc;
	typedef Mutator::FuncType MutatorFunc;

protected:
	Accessor *_accessor;
	Mutator *_mutator;

public:
	AnimationField(Animation* instance, AccessorFunc accessor, MutatorFunc mutator) {
		_accessor = new Accessor(instance, accessor);
		_mutator = new Mutator(instance, mutator);
	}

	AnimationField(Animation* instance, AccessorFunc accessor) {
		_accessor = new Accessor(instance, accessor);
		_mutator = 0;
	}

	~AnimationField() {
		delete _accessor;
		delete _mutator;
	}

	int16 getValue() const {
		assert(_accessor);
		return (*_accessor)();
	}

	void setValue(int16 value) {
		assert(_mutator);
		(*_mutator)(value);
	}
};


struct ScriptVar {
	uint32			_flags;

	int16			_value;
	LocalVariable*	_local;
	AnimationField*	_field;

	ScriptVar();
	~ScriptVar();

	int16	getValue();
	void	setValue(int16 value);

	void	setLocal(LocalVariable *local);
	void	setField(Animation *anim, AnimationField::AccessorFunc accessor, AnimationField::MutatorFunc mutator);
	void	setField(Animation *anim, AnimationField::AccessorFunc accessor);
	void	setImmediate(int16 value);
	void	setRandom(int16 seed);
};

enum InstructionFlags {
	kInstMod			= 4,
	kInstMaskedPut		= 8,
	kInstUnk20			= 0x20
};


struct Instruction {
	uint32	_index;
	uint32	_flags;

	// common
	AnimationPtr	_a;
	ZonePtr		_z;
	int16		_immediate;
	ScriptVar	_opA;
	ScriptVar	_opB;

	// BRA specific
	byte		_colors[3];
	ScriptVar	_opC;
	char		*_text;
	char		*_text2;
	int			_y;
	uint32		_endif;

	Instruction();
	~Instruction();

};

enum {
	kProgramIdle,		// awaiting execution
	kProgramRunning,	// running
	kProgramDone		// execution completed
};

struct Program {
	AnimationPtr	_anim;
	LocalVariable	*_locals;

	uint16		_loopCounter;
	uint16		_numLocals;

	uint32				_ip;
	uint32 				_loopStart;
	InstructionList		_instructions;

	uint32	_status;

	Program();
	~Program();

	int16		findLocal(const char* name);
	int16		addLocal(const char *name, int16 value = 0, int16 min = -10000, int16 max = 10000);
};

typedef Common::SharedPtr<Program> ProgramPtr;
typedef Common::List<ProgramPtr> ProgramList;

struct Animation : public Zone {
protected:
	int16		_frame;
	int16		_z;
public:

	GfxObj		*gfxobj;
	char		*_scriptName;

	Animation();
	virtual ~Animation();
	uint16 getFrameNum() const;
	byte* getFrameData() const;

	void resetZ();
	bool hitFrameRect(int x, int y) const;
	void getFrameRect(Common::Rect &r) const;
	int16 getBottom() const;

	// HACK: this routine is only used to download initialisation
	// parameter to a script used when moving sarcophagi around in
	// the museum. It bypasses all the consistency checks that
	// can be performed by the individual setters. See the comment
	// in startMovingSarcophagus() in callables_ns.cpp
	void forceXYZF(int16 x, int16 y, int16 z, int16 f);

	// getters/setters used by scripts
	int16 getX()			{ return _left; }
	void  setX(int16 value) { _left = value; }

	int16 getY()			{ return _top; }
	void  setY(int16 value) { _top = value; }

	int16 getZ()			{ return _z; }
	void  setZ(int16 value) { _z = value; }

	int16 getF()			{ return _frame; }
	void  setF(int16 value);

	void getFoot(Common::Point &foot);
	void setFoot(const Common::Point &foot);
};

class Table {

protected:
	char	**_data;
	uint16	_size;
	uint16	_used;
	bool	_disposeMemory;

public:
	Table(uint32 size);
	Table(uint32 size, const char** data);

	virtual ~Table();

	enum {
		notFound = 0
	};

	uint count() const { return _used; }
	const char *item(uint index) const;

	virtual void addData(const char* s);
	virtual void clear();
	virtual uint16 lookup(const char* s);
};

class FixedTable : public Table {

	uint16	_numFixed;

public:
	FixedTable(uint32 size, uint32 fixed);
	void clear();
};

Table* createTableFromStream(uint32 size, Common::SeekableReadStream *stream);

} // namespace Parallaction

#endif
