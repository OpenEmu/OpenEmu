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

#ifndef PARALLACTION_PARSER_H
#define PARALLACTION_PARSER_H

#include "common/stream.h"
#include "common/stack.h"
#include "parallaction/objects.h"

namespace Parallaction {

#define MAX_TOKEN_LEN	50
extern int  _numTokens;
extern char _tokens[][MAX_TOKEN_LEN];

class Script {

	Common::ReadStream *_input;
	bool	_disposeSource;
	uint	_line;				// for debug messages

	void clearTokens();
	char *parseNextToken(char *s, char *tok, uint16 count, const char *brk);
	char *readLineIntern(char *buf, size_t bufSize);

public:
	Script(Common::ReadStream *, bool _disposeSource = false);
	~Script();

	char *readLine(char *buf, size_t bufSize);
	uint16 readLineToken(bool errorOnEOF = false);

	void skip(const char* endToken);

	uint	getLine() { return _line; }
};


typedef Common::Functor0<void> Opcode;
typedef Common::Array<const Opcode *>	OpcodeSet;



class Parser {

public:
	Parser() { reset(); }
	~Parser() { reset(); }

	uint	_lookup;

	Common::Stack<OpcodeSet *>	_opcodes;
	Common::Stack<Table *>		_statements;

	OpcodeSet	*_currentOpcodes;
	Table		*_currentStatements;

	void	reset();
	void	pushTables(OpcodeSet *opcodes, Table* statements);
	void	popTables();
	void	parseStatement();

};

#define DECLARE_UNQUALIFIED_ZONE_PARSER(sig) void locZoneParse_##sig()
#define DECLARE_UNQUALIFIED_ANIM_PARSER(sig) void locAnimParse_##sig()
#define DECLARE_UNQUALIFIED_COMMAND_PARSER(sig) void cmdParse_##sig()
#define DECLARE_UNQUALIFIED_LOCATION_PARSER(sig) void locParse_##sig()
#define DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(sig) void instParse_##sig()

#define MAX_FORWARDS	50

class Parallaction_ns;
class Parallaction_br;


class LocationParser_ns {

protected:
	Parallaction_ns*	_vm;
	Script	*_script;
	Parser	*_parser;

	Table		*_zoneTypeNames;
	Table		*_zoneFlagNames;
	uint		_zoneProg;

	// location parser
	OpcodeSet	_locationParsers;
	OpcodeSet	_locationZoneParsers;
	OpcodeSet	_locationAnimParsers;
	OpcodeSet	_commandParsers;
	Table		*_commandsNames;
	Table		*_locationStmt;
	Table		*_locationZoneStmt;
	Table		*_locationAnimStmt;

	struct ParserContext {
		bool		end;

		const char	*filename;
		ZonePtr		z;
		AnimationPtr	a;
		int			nextToken;
		CommandList *list;
		bool		endcommands;
		CommandPtr	cmd;
	} ctxt;

	void warning_unexpected();

	DECLARE_UNQUALIFIED_LOCATION_PARSER(endlocation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(location);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(disk);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(nodes);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zone);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(animation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(localflags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(commands);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(acommands);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(flags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(comment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(endcomment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(sound);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(music);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ZONE_PARSER(commands);
	DECLARE_UNQUALIFIED_ZONE_PARSER(label);
	DECLARE_UNQUALIFIED_ZONE_PARSER(flags);
	DECLARE_UNQUALIFIED_ZONE_PARSER(endzone);
	DECLARE_UNQUALIFIED_ZONE_PARSER(null);
	DECLARE_UNQUALIFIED_ANIM_PARSER(script);
	DECLARE_UNQUALIFIED_ANIM_PARSER(commands);
	DECLARE_UNQUALIFIED_ANIM_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(label);
	DECLARE_UNQUALIFIED_ANIM_PARSER(flags);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(flags);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(animation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zone);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(invObject);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(call);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(simple);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(move);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endcommands);
public:
	virtual void parseGetData(ZonePtr z);
	virtual void parseExamineData(ZonePtr z);
	virtual void parseDoorData(ZonePtr z);
	virtual void parseMergeData(ZonePtr z);
	virtual void parseHearData(ZonePtr z);
	virtual void parseSpeakData(ZonePtr z);
	virtual void parseNoneData(ZonePtr z);
protected:
	Common::String	parseComment();
	Common::String	parseDialogueString();
	Dialogue	*parseDialogue();
	virtual Answer *parseAnswer();
	void		parseAnswerFlags(Answer *answer);
	void		parseAnswerBody(Answer *answer);
	void		parseQuestion(Question *q);

	uint32		buildZoneType(const char *t0, const char* t1);

	void		parseZone(ZoneList &list, char *name);
	virtual void parseZoneTypeBlock(ZonePtr z);
	void		parsePointList(PointList &list);
	void		parseAnimation(AnimationList &list, char *name);
	void		parseCommands(CommandList&);
	void		parseCommandFlags();
	void		parseCommandFlag(CommandPtr cmd, const char *flag, Table *table);
	void		createCommand(uint id);
	void		addCommand();

	void clearSet(OpcodeSet &opcodes) {
		for (Common::Array<const Opcode *>::iterator i = opcodes.begin(); i != opcodes.end(); ++i)
			delete *i;
		opcodes.clear();
	}

public:
	LocationParser_ns(Parallaction_ns *vm) : _vm(vm), _commandsNames(0), _locationStmt(0),
		_locationZoneStmt(0), _locationAnimStmt(0) {
	}

	virtual void init();

	virtual ~LocationParser_ns() {
		delete _parser;
		delete _commandsNames;
		delete _locationStmt;
		delete _locationZoneStmt;
		delete _locationAnimStmt;
		delete _zoneTypeNames;
		delete _zoneFlagNames;

		clearSet(_commandParsers);
		clearSet(_locationAnimParsers);
		clearSet(_locationZoneParsers);
		clearSet(_locationParsers);
	}

	void parse(Script *script);

};


struct LocationParserOutput_br {
	BackgroundInfo	*_info;

	Common::String _characterName;
	Common::String _backgroundName;
	Common::String _maskName;
	Common::String _pathName;
};

class LocationParser_br : public LocationParser_ns {

protected:
	Parallaction_br*	_vm;
	Table		*_audioCommandsNames;

	LocationParserOutput_br *_out;

	DECLARE_UNQUALIFIED_LOCATION_PARSER(location);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zone);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(animation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(localflags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(flags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(comment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(endcomment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(sound);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(music);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(redundant);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(ifchar);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(character);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(mask);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(path);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(escape);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zeta);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(null);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(ifchar);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endif);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(toggle);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(string);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(math);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(test);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(music);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zeta);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(swap);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(give);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(text);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(unary);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);

	virtual void	parseZoneTypeBlock(ZonePtr z);
public:
	virtual void	parsePathData(ZonePtr z);
	virtual void	parseGetData(ZonePtr z);
	virtual void	parseDoorData(ZonePtr z);
	virtual void	parseHearData(ZonePtr z);
	virtual void 	parseNoneData(ZonePtr z);
protected:
	void	parseAnswerCounter(Answer *answer);
	virtual Answer *parseAnswer();

public:
	LocationParser_br(Parallaction_br *vm) : LocationParser_ns((Parallaction_ns*)vm), _vm(vm),
		_audioCommandsNames(0) {
	}

	virtual void init();

	virtual ~LocationParser_br() {
		delete _audioCommandsNames;
	}

	void parse(Script *script, LocationParserOutput_br *out);

};



class ProgramParser_ns {

protected:
	Parallaction_ns *_vm;
	Parser	*_parser;

	Script	*_script;
	ProgramPtr	_program;

	// program parser
	OpcodeSet	_instructionParsers;
	Table		*_instructionNames;

	uint32		_currentInstruction; // index of the instruction being parsed

	struct ParserContext {
		bool		end;
		AnimationPtr	a;
		InstructionPtr inst;
		LocalVariable *locals;
	} ctxt;

	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(defLocal);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(animation);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(x);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(y);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(z);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(f);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(call);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(sound);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(null);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(endscript);

	void		parseInstruction();
	void		parseLValue(ScriptVar &var, const char *str);
	virtual void	parseRValue(ScriptVar &var, const char *str);

	void clearSet(OpcodeSet &opcodes) {
		for (Common::Array<const Opcode *>::iterator i = opcodes.begin(); i != opcodes.end(); ++i)
			delete *i;
		opcodes.clear();
	}

public:
	ProgramParser_ns(Parallaction_ns *vm) : _vm(vm), _parser(0), _instructionNames(0) {
	}

	virtual void init();

	virtual ~ProgramParser_ns() {
		delete _parser;
		delete _instructionNames;

		clearSet(_instructionParsers);
	}

	virtual void parse(Script *script, ProgramPtr program);

};


class ProgramParser_br : public ProgramParser_ns {

protected:
	Parallaction_br *_vm;

	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(zone);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(color);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(mask);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(print);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(text);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(if_op);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(endif);

	int32 _openIfStatement;
	void beginIfStatement();
	void endIfStatement();

	virtual void parseRValue(ScriptVar &var, const char *str);

public:
	ProgramParser_br(Parallaction_br *vm) : ProgramParser_ns((Parallaction_ns*)vm), _vm(vm) {
	}

	virtual void init();
	virtual void parse(Script *script, ProgramPtr program);
};

} // End of namespace Parallaction

#endif
