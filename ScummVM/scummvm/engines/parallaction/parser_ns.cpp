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

#include "common/textconsole.h"

#include "parallaction/parallaction.h"
#include "parallaction/parser.h"
#include "parallaction/sound.h"

namespace Parallaction {

#define CMD_SET				1
#define CMD_CLEAR			2
#define CMD_START			3
#define CMD_SPEAK			4
#define CMD_GET				5
#define CMD_LOCATION		6
#define CMD_OPEN			7
#define CMD_CLOSE			8
#define CMD_ON				9
#define CMD_OFF				10
#define CMD_CALL			11
#define CMD_TOGGLE			12
#define CMD_DROP			13
#define CMD_QUIT			14
#define CMD_MOVE			15
#define CMD_STOP			16

#define INST_ON				1
#define INST_OFF			2
#define INST_X				3
#define INST_Y				4
#define INST_Z				5
#define INST_F				6
#define INST_LOOP			7
#define INST_ENDLOOP		8
#define INST_SHOW			9
#define INST_INC			10
#define INST_DEC			11
#define INST_SET			12
#define INST_PUT			13
#define INST_CALL			14
#define INST_WAIT			15
#define INST_START			16
#define INST_SOUND			17
#define INST_MOVE			18
#define INST_END			19


const char *_zoneFlagNamesRes_ns[] = {
	"closed",
	"active",
	"remove",
	"acting",
	"locked",
	"fixed",
	"noname",
	"nomasked",
	"looping",
	"added",
	"character",
	"nowalk"
};

const char *_zoneTypeNamesRes_ns[] = {
	"examine",
	"door",
	"get",
	"merge",
	"taste",
	"hear",
	"feel",
	"speak",
	"none",
	"trap",
	"yourself",
	"Command"
};

const char *_commandsNamesRes_ns[] = {
	"set",
	"clear",
	"start",
	"speak",
	"get",
	"location",
	"open",
	"close",
	"on",
	"off",
	"call",
	"toggle",
	"drop",
	"quit",
	"move",
	"stop",
	"endcommands",
	"endzone"
};

const char *_locationStmtRes_ns[] = {
	"endlocation",
	"location",
	"disk",
	"nodes",
	"zone",
	"animation",
	"localflags",
	"commands",
	"acommands",
	"flags",
	"comment",
	"endcomment",
	"sound",
	"music"
};

const char *_locationZoneStmtRes_ns[] = {
	"limits",
	"moveto",
	"type",
	"commands",
	"label",
	"flags",
	"endzone"
};

const char *_locationAnimStmtRes_ns[] = {
	"script",
	"commands",
	"type",
	"label",
	"flags",
	"file",
	"position",
	"moveto",
	"endanimation"
};

const char *_instructionNamesRes_ns[] = {
	"on",
	"off",
	"x",
	"y",
	"z",
	"f",
	"loop",
	"endloop",
	"show",
	"inc",
	"dec",
	"set",
	"put",
	"call",
	"wait",
	"start",
	"sound",
	"move",
	"endscript"
};


#define DECLARE_ZONE_PARSER(sig) void LocationParser_ns::locZoneParse_##sig()
#define DECLARE_ANIM_PARSER(sig) void LocationParser_ns::locAnimParse_##sig()
#define DECLARE_COMMAND_PARSER(sig) void LocationParser_ns::cmdParse_##sig()
#define DECLARE_LOCATION_PARSER(sig) void LocationParser_ns::locParse_##sig()

#define DECLARE_INSTRUCTION_PARSER(sig) void ProgramParser_ns::instParse_##sig()


void LocationParser_ns::warning_unexpected() {
	debugC(1, kDebugParser, "unexpected keyword '%s' in line %i", _tokens[0], _script->getLine());
}


DECLARE_ANIM_PARSER(script)  {
	debugC(7, kDebugParser, "ANIM_PARSER(script) ");

	ctxt.a->_scriptName = strdup(_tokens[1]);
}


DECLARE_ANIM_PARSER(commands)  {
	debugC(7, kDebugParser, "ANIM_PARSER(commands) ");

	 parseCommands(ctxt.a->_commands);
}


DECLARE_ANIM_PARSER(type)  {
	debugC(7, kDebugParser, "ANIM_PARSER(type) ");

	ctxt.a->_type = buildZoneType(_tokens[1], _tokens[2]);
	if ((ACTIONTYPE(ctxt.a) != 0) && (ACTIONTYPE(ctxt.a) != kZoneCommand)) {
		parseZoneTypeBlock(ctxt.a);
	}

	ctxt.a->_flags |= 0x1000000;

	_parser->popTables();
}


DECLARE_ANIM_PARSER(label)  {
	debugC(7, kDebugParser, "ANIM_PARSER(label) ");

	ctxt.a->_label = _vm->_gfx->renderFloatingLabel(_vm->_labelFont, _tokens[1]);
	ctxt.a->_flags &= ~kFlagsNoName;
}


DECLARE_ANIM_PARSER(flags)  {
	debugC(7, kDebugParser, "ANIM_PARSER(flags) ");

	uint16 _si = 1;

	do {
		byte _al = _zoneFlagNames->lookup(_tokens[_si]);
		_si++;
		ctxt.a->_flags |= 1 << (_al - 1);
	} while (!scumm_stricmp(_tokens[_si++], "|"));
}


DECLARE_ANIM_PARSER(file)  {
	debugC(7, kDebugParser, "ANIM_PARSER(file) ");

	char vC8[200];
	strcpy(vC8, _tokens[1]);
	if (g_engineFlags & kEngineTransformedDonna) {
		if (!scumm_stricmp(_tokens[1], "donnap") || !scumm_stricmp(_tokens[1], "donnapa")) {
			strcat(vC8, "tras");
		}
	}
	ctxt.a->gfxobj = _vm->_gfx->loadAnim(vC8);
}


DECLARE_ANIM_PARSER(position)  {
	debugC(7, kDebugParser, "ANIM_PARSER(position) ");

	ctxt.a->setX(atoi(_tokens[1]));
	ctxt.a->setY(atoi(_tokens[2]));
	ctxt.a->setZ(atoi(_tokens[3]));
}


DECLARE_ANIM_PARSER(moveto)  {
	debugC(7, kDebugParser, "ANIM_PARSER(moveto) ");

	ctxt.a->_moveTo.x = atoi(_tokens[1]);
	ctxt.a->_moveTo.y = atoi(_tokens[2]);
}


DECLARE_ANIM_PARSER(endanimation)  {
	debugC(7, kDebugParser, "ANIM_PARSER(endanimation) ");

	ctxt.a->_flags |= 0x1000000;

	_parser->popTables();
}

void LocationParser_ns::parseAnimation(AnimationList &list, char *name) {
	debugC(5, kDebugParser, "parseAnimation(name: %s)", name);

	if (_vm->_location.findAnimation(name)) {
		_zoneProg++;
		_script->skip("endanimation");
		return;
	}

	AnimationPtr a(new Animation);
	_zoneProg++;

	strncpy(a->_name, name, ZONENAME_LENGTH);
	a->_flags |= kFlagsIsAnimation;

	list.push_front(AnimationPtr(a));

	ctxt.a = a;
	_parser->pushTables(&_locationAnimParsers, _locationAnimStmt);
}

void ProgramParser_ns::parseInstruction() {

	_script->readLineToken(true);

	if (_tokens[0][1] == '.') {
		_tokens[0][1] = '\0';
		ctxt.a = _vm->_location.findAnimation(&_tokens[0][2]);
	} else
	if (_tokens[1][1] == '.') {
		_tokens[1][1] = '\0';
		ctxt.a = _vm->_location.findAnimation(&_tokens[1][2]);
	} else
		ctxt.a = _program->_anim;

	if (!ctxt.a) {
		return;
	}

	InstructionPtr inst(new Instruction);
	ctxt.inst = inst;

	_currentInstruction = _program->_instructions.size();

	_parser->parseStatement();
	_program->_instructions.push_back(inst);

	return;
}

void ProgramParser_ns::parse(Script *script, ProgramPtr program) {
	_script = script;
	_program = program;

	ctxt.end = false;
	ctxt.locals = program->_locals;

	_parser->reset();
	_parser->pushTables(&_instructionParsers, _instructionNames);
	do {
		parseInstruction();
	} while (!ctxt.end);
	_parser->popTables();

	program->_ip = 0;
}

void Parallaction_ns::loadProgram(AnimationPtr a, const char *filename) {
	debugC(1, kDebugParser, "loadProgram(Animation: %s, script: %s)", a->_name, filename);

	Script *script = _disk->loadScript(filename);
	ProgramPtr program(new Program);
	program->_anim = a;

	_programParser->parse(script, program);

	delete script;

	_location._programs.push_back(program);

	debugC(1, kDebugParser, "loadProgram() done");

	return;
}

DECLARE_INSTRUCTION_PARSER(animation)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(animation) ");

	if (!scumm_stricmp(_tokens[1], ctxt.a->_name)) {
		ctxt.inst->_a = ctxt.a;
	} else {
		ctxt.inst->_a = _vm->_location.findAnimation(_tokens[1]);
	}

	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(loop)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(loop) ");

	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(x)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(x) ");

	parseLValue(ctxt.inst->_opA, "X");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(y)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(y) ");

	parseLValue(ctxt.inst->_opA, "Y");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(z)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(z) ");

	parseLValue(ctxt.inst->_opA, "Z");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(f)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(f) ");

	parseLValue(ctxt.inst->_opA, "F");
	parseRValue(ctxt.inst->_opB, _tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(inc)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(inc) ");

	parseLValue(ctxt.inst->_opA, _tokens[1]);
	parseRValue(ctxt.inst->_opB, _tokens[2]);

	if (!scumm_stricmp(_tokens[3], "mod")) {
		ctxt.inst->_flags |= kInstMod;
	}
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(set)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(set) ");

	// WORKAROUND: At least one script (balzo.script) in Amiga versions didn't declare
	//	local variables before using them, thus leading to crashes. The line launching the
	// script was commented out on Dos version. This workaround enables the engine
	// to dynamically add a local variable when it is encountered the first time in
	// the script, so should fix any other occurrence as well.
	if (_program->findLocal(_tokens[1]) == -1) {
		_program->addLocal(_tokens[1]);
	}

	parseLValue(ctxt.inst->_opA, _tokens[1]);
	parseRValue(ctxt.inst->_opB, _tokens[2]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(move)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(move) ");

	parseRValue(ctxt.inst->_opA, _tokens[1]);
	parseRValue(ctxt.inst->_opB, _tokens[2]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(put)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(put) ");

	if (!scumm_stricmp(_tokens[1], ctxt.a->_name)) {
		ctxt.inst->_a = ctxt.a;
	} else {
		ctxt.inst->_a = _vm->_location.findAnimation(_tokens[1]);
	}

	parseRValue(ctxt.inst->_opA, _tokens[2]);
	parseRValue(ctxt.inst->_opB, _tokens[3]);
	if (!scumm_stricmp(_tokens[4], "masked")) {
		ctxt.inst->_flags |= kInstMaskedPut;
	}
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(call)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(call) ");

	int index = _vm->_callableNames->lookup(_tokens[1]);
	if (index == Table::notFound)
		error("unknown callable '%s'", _tokens[1]);
	ctxt.inst->_immediate = index - 1;
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(sound)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(sound) ");

	ctxt.inst->_z = _vm->_location.findZone(_tokens[1]);
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(null)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(null) ");
	ctxt.inst->_index = _parser->_lookup;
}


DECLARE_INSTRUCTION_PARSER(defLocal)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(defLocal) ");

	int16 val = atoi(_tokens[2]);
	int16 index;

	if (_tokens[3][0] != '\0') {
		index = _program->addLocal(_tokens[0], val, atoi(_tokens[3]), atoi(_tokens[4]));
	} else {
		index = _program->addLocal(_tokens[0], val);
	}

	ctxt.inst->_opA.setLocal(&ctxt.locals[index]);
	ctxt.inst->_opB.setImmediate(ctxt.locals[index].getValue());

	ctxt.inst->_index = INST_SET;
}

DECLARE_INSTRUCTION_PARSER(endscript)  {
	debugC(7, kDebugParser, "INSTRUCTION_PARSER(endscript) ");

	ctxt.end = true;
	ctxt.inst->_index = _parser->_lookup;
}



void ProgramParser_ns::parseRValue(ScriptVar &v, const char *str) {

	if (Common::isDigit(str[0]) || str[0] == '-') {
		v.setImmediate(atoi(str));
		return;
	}

	int index = _program->findLocal(str);
	if (index != -1) {
		v.setLocal(&ctxt.locals[index]);
		return;
	}

	AnimationPtr a;
	if (str[1] == '.') {
		a = _vm->_location.findAnimation(&str[2]);
	} else {
		a = ctxt.a;
	}

	if (str[0] == 'X') {
		v.setField(a.get(), &Animation::getX);
	} else
	if (str[0] == 'Y') {
		v.setField(a.get(), &Animation::getY);
	} else
	if (str[0] == 'Z') {
		v.setField(a.get(), &Animation::getZ);
	} else
	if (str[0] == 'F') {
		v.setField(a.get(), &Animation::getF);
	}

}

void ProgramParser_ns::parseLValue(ScriptVar &v, const char *str) {

	int index = _program->findLocal(str);
	if (index != -1) {
		v.setLocal(&ctxt.locals[index]);
		return;
	}

	AnimationPtr a;
	if (str[1] == '.') {
		a = _vm->_location.findAnimation(&str[2]);
	} else {
		a = ctxt.a;
	}

	if (str[0] == 'X') {
		v.setField(a.get(), &Animation::getX, &Animation::setX);
	} else
	if (str[0] == 'Y') {
		v.setField(a.get(), &Animation::getY, &Animation::setY);
	} else
	if (str[0] == 'Z') {
		v.setField(a.get(), &Animation::getZ, &Animation::setZ);
	} else
	if (str[0] == 'F') {
		v.setField(a.get(), &Animation::getF, &Animation::setF);
	}

}


DECLARE_COMMAND_PARSER(flags)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(flags) ");

	createCommand(_parser->_lookup);

	if (_vm->_globalFlagsNames->lookup(_tokens[1]) == Table::notFound) {
		do {
			char _al = _vm->_localFlagNames->lookup(_tokens[ctxt.nextToken]);
			ctxt.nextToken++;
			ctxt.cmd->_flags |= 1 << (_al - 1);
		} while (!scumm_stricmp(_tokens[ctxt.nextToken++], "|"));
		ctxt.nextToken--;
	} else {
		ctxt.cmd->_flags |= kFlagsGlobal;
		do {
			char _al = _vm->_globalFlagsNames->lookup(_tokens[1]);
			ctxt.nextToken++;
			ctxt.cmd->_flags |= 1 << (_al - 1);
		} while (!scumm_stricmp(_tokens[ctxt.nextToken++], "|"));
		ctxt.nextToken--;
	}

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(zone)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(zone) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->_zoneName = _tokens[ctxt.nextToken];
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(location)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(location) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->_string = strdup(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(invObject)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(drop) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->_object = 4 + _vm->_objectsNames->lookup(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(call)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(call) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->_callable = _vm->_callableNames->lookup(_tokens[ctxt.nextToken]) - 1;
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(simple)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(simple) ");

	createCommand(_parser->_lookup);
	parseCommandFlags();
	addCommand();
}


DECLARE_COMMAND_PARSER(move)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(move) ");

	createCommand(_parser->_lookup);

	ctxt.cmd->_move.x = atoi(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;
	ctxt.cmd->_move.y = atoi(_tokens[ctxt.nextToken]);
	ctxt.nextToken++;

	parseCommandFlags();
	addCommand();
}

DECLARE_COMMAND_PARSER(endcommands)  {
	debugC(7, kDebugParser, "COMMAND_PARSER(endcommands) ");

	_parser->popTables();

	// temporary trick to handle dialogue commands
	ctxt.endcommands = true;
}

void LocationParser_ns::parseCommandFlag(CommandPtr cmd, const char *flag, Table *table) {

	if (!scumm_stricmp(flag, "exit")) {
		cmd->_flagsOn |= kFlagsExit;
	} else
	if (!scumm_stricmp(flag, "exittrap")) {
		cmd->_flagsOn |= kFlagsExit;
	} else
	if (!scumm_stricmp(flag, "enter")) {
		cmd->_flagsOn |= kFlagsEnter;
	} else
	if (!scumm_stricmp(flag, "entertrap")) {
		cmd->_flagsOn |= kFlagsEnter;
	} else
	if (!scumm_strnicmp(flag, "no", 2)) {
		byte _al = table->lookup(flag+2);
		if (_al != Table::notFound) {
			cmd->_flagsOff |= 1 << (_al - 1);
		} else {
			warning("Flag '%s' not found", flag);
		}
	} else {
		byte _al = table->lookup(flag);
		if (_al != Table::notFound) {
			cmd->_flagsOn |= 1 << (_al - 1);
		} else {
			warning("Flag '%s' not found", flag);
		}
	}
}

void LocationParser_ns::parseCommandFlags() {

	int _si = ctxt.nextToken;
	CommandPtr cmd = ctxt.cmd;

	if (!scumm_stricmp(_tokens[_si], "flags")) {
		do {
			_si++;
			parseCommandFlag(cmd, _tokens[_si], _vm->_localFlagNames);
			_si++;
		} while (!scumm_stricmp(_tokens[_si], "|"));
	}

	if (!scumm_stricmp(_tokens[_si], "gflags")) {
		do {
			_si++;
			parseCommandFlag(cmd, _tokens[_si], _vm->_globalFlagsNames);
			_si++;
		} while (!scumm_stricmp(_tokens[_si], "|"));
		cmd->_flagsOn |= kFlagsGlobal;
	}
}

void LocationParser_ns::addCommand() {
	ctxt.list->push_front(ctxt.cmd);	// NOTE: command lists are written backwards in scripts
}

void LocationParser_ns::createCommand(uint id) {

	ctxt.nextToken = 1;
	ctxt.cmd = CommandPtr(new Command);
	ctxt.cmd->_id = id;
	ctxt.cmd->_valid = true;

}

void LocationParser_ns::parseCommands(CommandList& list) {
	debugC(5, kDebugParser, "parseCommands()");

	ctxt.list = &list;
	ctxt.endcommands = false;

	_parser->pushTables(&_commandParsers, _commandsNames);
}

Dialogue *LocationParser_ns::parseDialogue() {
	debugC(7, kDebugParser, "parseDialogue()");

	Dialogue *dialogue = new Dialogue;
	assert(dialogue);

	_script->readLineToken(true);

	while (scumm_stricmp(_tokens[0], "enddialogue")) {
		if (!scumm_stricmp(_tokens[0], "question")) {
			Question *q = new Question(_tokens[1]);
			assert(q);
			parseQuestion(q);
			dialogue->addQuestion(q);
		}
		_script->readLineToken(true);
	}

	debugC(7, kDebugParser, "parseDialogue() done");

	return dialogue;
}

void LocationParser_ns::parseQuestion(Question *q) {
	q->_text = parseDialogueString();

	_script->readLineToken(true);
	q->_mood = atoi(_tokens[0]);

	uint16 numAnswers = 0;

	_script->readLineToken(true);
	while (scumm_stricmp(_tokens[0], "endquestion")) {	// parse answers
		q->_answers[numAnswers] = parseAnswer();
		numAnswers++;
	}
}

void LocationParser_ns::parseAnswerBody(Answer *answer) {
	answer->_text = parseDialogueString();

	_script->readLineToken(true);
	answer->_mood = atoi(_tokens[0]);
	answer->_followingName = parseDialogueString();

	_script->readLineToken(true);
	if (!scumm_stricmp(_tokens[0], "commands")) {

		parseCommands(answer->_commands);
		ctxt.endcommands = false;
		do {
			_script->readLineToken(true);
			_parser->parseStatement();
		} while (!ctxt.endcommands);

		_script->readLineToken(true);
	}
}

void LocationParser_ns::parseAnswerFlags(Answer *answer) {
	if (!_tokens[1][0]) {
		return;
	}

	Table* flagNames;
	uint16 token;

	if (!scumm_stricmp(_tokens[1], "global")) {
		token = 2;
		flagNames = _vm->_globalFlagsNames;
		answer->_yesFlags |= kFlagsGlobal;
	} else {
		token = 1;
		flagNames = _vm->_localFlagNames;
	}

	do {

		if (!scumm_strnicmp(_tokens[token], "no", 2)) {
			byte _al = flagNames->lookup(_tokens[token]+2);
			answer->_noFlags |= 1 << (_al - 1);
		} else {
			byte _al = flagNames->lookup(_tokens[token]);
			answer->_yesFlags |= 1 << (_al - 1);
		}

		token++;

	} while (!scumm_stricmp(_tokens[token++], "|"));
}

Answer *LocationParser_ns::parseAnswer() {
	Answer *answer = new Answer;
	assert(answer);
	parseAnswerFlags(answer);
	parseAnswerBody(answer);
	return answer;
}


Common::String LocationParser_ns::parseDialogueString() {
	char buf[400];
	char *line = _script->readLine(buf, 400);
	if (line == 0) {
		return 0;
	}
	return Common::String(line);
}


DECLARE_LOCATION_PARSER(endlocation)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(endlocation) ");

	ctxt.end = true;
}


DECLARE_LOCATION_PARSER(location)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(location) ");

	// The parameter for location is 'location.mask'.
	// If mask is not present, then it is assumed
	// that path & mask are encoded in the background
	// bitmap, otherwise a separate .msk file exists.

	char *mask = strchr(_tokens[1], '.');
	if (mask) {
		mask[0] = '\0';
		mask++;
	}

	strcpy(_vm->_location._name, _tokens[1]);
	_vm->changeBackground(_vm->_location._name, mask);

	if (_tokens[2][0] != '\0') {
		_vm->_char._ani->setX(atoi(_tokens[2]));
		_vm->_char._ani->setY(atoi(_tokens[3]));
	}

	if (_tokens[4][0] != '\0') {
		_vm->_char._ani->setF(atoi(_tokens[4]));
	}
}


DECLARE_LOCATION_PARSER(disk)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(disk) ");

	_vm->_disk->selectArchive(_tokens[1]);
}


DECLARE_LOCATION_PARSER(nodes)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(nodes) ");

	parsePointList(_vm->_location._walkPoints);
}


DECLARE_LOCATION_PARSER(zone)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(zone) ");

	parseZone(_vm->_location._zones, _tokens[1]);
}


DECLARE_LOCATION_PARSER(animation)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(animation) ");

	parseAnimation(_vm->_location._animations, _tokens[1]);
}


DECLARE_LOCATION_PARSER(localflags)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(localflags) ");

	int _si = 1;
	while (_tokens[_si][0] != '\0') {
		_vm->_localFlagNames->addData(_tokens[_si]);
		_si++;
	}
}


DECLARE_LOCATION_PARSER(commands)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(commands) ");

	parseCommands(_vm->_location._commands);
}


DECLARE_LOCATION_PARSER(acommands)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(acommands) ");

	parseCommands(_vm->_location._aCommands);
}


DECLARE_LOCATION_PARSER(flags)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(flags) ");

	if ((_vm->getLocationFlags() & kFlagsVisited) == 0) {
		// only for 1st visit
		_vm->clearLocationFlags((uint32)kFlagsAll);
		int _si = 1;

		do {
			byte _al = _vm->_localFlagNames->lookup(_tokens[_si]);
			_vm->setLocationFlags(1 << (_al - 1));

			_si++;
			if (scumm_stricmp(_tokens[_si], "|")) break;
			_si++;
		} while (true);
	}
}


DECLARE_LOCATION_PARSER(comment)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(comment) ");

	_vm->_location._comment = parseComment();
}


DECLARE_LOCATION_PARSER(endcomment)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(endcomment) ");

	_vm->_location._endComment = parseComment();
}


DECLARE_LOCATION_PARSER(sound)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(sound) ");

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		strcpy(_vm->_location._soundFile, _tokens[1]);
		_vm->_location._hasSound = true;
	}
}


DECLARE_LOCATION_PARSER(music)  {
	debugC(7, kDebugParser, "LOCATION_PARSER(music) ");

	if (_vm->getPlatform() == Common::kPlatformAmiga)
		_vm->_soundMan->execute(SC_SETMUSICFILE, _tokens[1]);
}

void LocationParser_ns::parse(Script *script) {
	_zoneProg = 0;

	ctxt.end = false;
	_script = script;
	ctxt.filename = 0;//filename;

	_parser->reset();
	_parser->pushTables(&_locationParsers, _locationStmt);
	do {
		_script->readLineToken(true);
		_parser->parseStatement();
	} while (!ctxt.end);
	_parser->popTables();
}

void LocationParser_ns::parsePointList(PointList &list) {
	debugC(5, kDebugParser, "parsePointList()");

	_script->readLineToken(true);
	while (scumm_stricmp(_tokens[0], "ENDNODES")) {

		if (!scumm_stricmp(_tokens[0], "COORD")) {
			list.push_front(Common::Point(atoi(_tokens[1]), atoi(_tokens[2])));
		}

		_script->readLineToken(true);
	}

	debugC(5, kDebugParser, "parsePointList() done");

	return;
}
/*
typedef OpcodeImpl<ProgramParser_ns> OpcodeV1;
#define INSTRUCTION_PARSER(sig) OpcodeV1(this, &ProgramParser_ns::instParse_##sig)

typedef OpcodeImpl<LocationParser_ns> OpcodeV2;
#define ZONE_PARSER(sig)		OpcodeV2(this, &LocationParser_ns::locZoneParse_##sig)
#define ANIM_PARSER(sig)		OpcodeV2(this, &LocationParser_ns::locAnimParse_##sig)
#define LOCATION_PARSER(sig)	OpcodeV2(this, &LocationParser_ns::locParse_##sig)
#define COMMAND_PARSER(sig)		OpcodeV2(this, &LocationParser_ns::cmdParse_##sig)

#define WARNING_PARSER(sig)		OpcodeV2(this, &LocationParser_br::warning_##sig)
*/

#define SetOpcodeTable(x) table = &x;


typedef Common::Functor0Mem<void, ProgramParser_ns> OpcodeV1;
#define SetOpcodeTable(x) table = &x;
#define INSTRUCTION_PARSER(sig) table->push_back(new OpcodeV1(this, &ProgramParser_ns::instParse_##sig))

typedef Common::Functor0Mem<void, LocationParser_ns> OpcodeV2;
#define ZONE_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_ns::locZoneParse_##sig))
#define ANIM_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_ns::locAnimParse_##sig))
#define LOCATION_PARSER(sig)	table->push_back(new OpcodeV2(this, &LocationParser_ns::locParse_##sig))
#define COMMAND_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_ns::cmdParse_##sig))

#define WARNING_PARSER(sig)		table->push_back(new OpcodeV2(this, &LocationParser_br::warning_##sig))


void LocationParser_ns::init() {

	_parser = new Parser;

	_zoneFlagNames = new Table(ARRAYSIZE(_zoneFlagNamesRes_ns), _zoneFlagNamesRes_ns);
	_zoneTypeNames = new Table(ARRAYSIZE(_zoneTypeNamesRes_ns), _zoneTypeNamesRes_ns);
	_commandsNames = new Table(ARRAYSIZE(_commandsNamesRes_ns), _commandsNamesRes_ns);
	_locationStmt = new Table(ARRAYSIZE(_locationStmtRes_ns), _locationStmtRes_ns);
	_locationZoneStmt = new Table(ARRAYSIZE(_locationZoneStmtRes_ns), _locationZoneStmtRes_ns);
	_locationAnimStmt = new Table(ARRAYSIZE(_locationAnimStmtRes_ns), _locationAnimStmtRes_ns);

	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_commandParsers);
	WARNING_PARSER(unexpected);
	COMMAND_PARSER(flags);			// set
	COMMAND_PARSER(flags);			// clear
	COMMAND_PARSER(zone);		// start
	COMMAND_PARSER(zone);			// speak
	COMMAND_PARSER(zone);			// get
	COMMAND_PARSER(location);		// location
	COMMAND_PARSER(zone);			// open
	COMMAND_PARSER(zone);			// close
	COMMAND_PARSER(zone);			// on
	COMMAND_PARSER(zone);			// off
	COMMAND_PARSER(call);			// call
	COMMAND_PARSER(flags);			// toggle
	COMMAND_PARSER(invObject);			// drop
	COMMAND_PARSER(simple);			// quit
	COMMAND_PARSER(move);			// move
	COMMAND_PARSER(zone);		// stop
	COMMAND_PARSER(endcommands);	// endcommands
	COMMAND_PARSER(endcommands);		// endzone

	SetOpcodeTable(_locationParsers);
	WARNING_PARSER(unexpected);
	LOCATION_PARSER(endlocation);
	LOCATION_PARSER(location);
	LOCATION_PARSER(disk);
	LOCATION_PARSER(nodes);
	LOCATION_PARSER(zone);
	LOCATION_PARSER(animation);
	LOCATION_PARSER(localflags);
	LOCATION_PARSER(commands);
	LOCATION_PARSER(acommands);
	LOCATION_PARSER(flags);
	LOCATION_PARSER(comment);
	LOCATION_PARSER(endcomment);
	LOCATION_PARSER(sound);
	LOCATION_PARSER(music);

	SetOpcodeTable(_locationZoneParsers);
	WARNING_PARSER(unexpected);
	ZONE_PARSER(limits);
	ZONE_PARSER(moveto);
	ZONE_PARSER(type);
	ZONE_PARSER(commands);
	ZONE_PARSER(label);
	ZONE_PARSER(flags);
	ZONE_PARSER(endzone);

	SetOpcodeTable(_locationAnimParsers);
	WARNING_PARSER(unexpected);
	ANIM_PARSER(script);
	ANIM_PARSER(commands);
	ANIM_PARSER(type);
	ANIM_PARSER(label);
	ANIM_PARSER(flags);
	ANIM_PARSER(file);
	ANIM_PARSER(position);
	ANIM_PARSER(moveto);
	ANIM_PARSER(endanimation);

}

void ProgramParser_ns::init() {

	_parser = new Parser;

	_instructionNames = new Table(ARRAYSIZE(_instructionNamesRes_ns), _instructionNamesRes_ns);

	Common::Array<const Opcode *> *table = 0;
	SetOpcodeTable(_instructionParsers);
	INSTRUCTION_PARSER(defLocal);	// invalid opcode -> local definition
	INSTRUCTION_PARSER(animation);	// on
	INSTRUCTION_PARSER(animation);	// off
	INSTRUCTION_PARSER(x);
	INSTRUCTION_PARSER(y);
	INSTRUCTION_PARSER(z);
	INSTRUCTION_PARSER(f);
	INSTRUCTION_PARSER(loop);
	INSTRUCTION_PARSER(null);		// endloop
	INSTRUCTION_PARSER(null);		// show
	INSTRUCTION_PARSER(inc);
	INSTRUCTION_PARSER(inc);		// dec
	INSTRUCTION_PARSER(set);
	INSTRUCTION_PARSER(put);
	INSTRUCTION_PARSER(call);
	INSTRUCTION_PARSER(null);		// wait
	INSTRUCTION_PARSER(animation);	// start
	INSTRUCTION_PARSER(sound);
	INSTRUCTION_PARSER(move);
	INSTRUCTION_PARSER(endscript);
}

//
//	a comment can appear both at location and Zone levels
//	comments are displayed into rectangles on the screen
//
Common::String LocationParser_ns::parseComment() {
	Common::String comment;
	char buf[400];
	do {
		char *line = _script->readLine(buf, 400);
		if (!scumm_stricmp(line, "endtext"))
			break;

		if (comment.size() > 0)
			comment += " ";

		comment += line;
	} while (true);

	if (comment.size() == 0) {
		return 0;
	}

	return comment;
}

DECLARE_ZONE_PARSER(null) {
	debugC(7, kDebugParser, "ZONE_PARSER(null) ");
}


DECLARE_ZONE_PARSER(endzone)  {
	debugC(7, kDebugParser, "ZONE_PARSER(endzone) ");

	_parser->popTables();
}

DECLARE_ZONE_PARSER(limits)  {
	debugC(7, kDebugParser, "ZONE_PARSER(limits) ");
	ctxt.z->setRect(atoi(_tokens[1]), atoi(_tokens[2]), atoi(_tokens[3]), atoi(_tokens[4]));
}


DECLARE_ZONE_PARSER(moveto)  {
	debugC(7, kDebugParser, "ZONE_PARSER(moveto) ");

	ctxt.z->_moveTo.x = atoi(_tokens[1]);
	ctxt.z->_moveTo.y = atoi(_tokens[2]);
}

uint32 LocationParser_ns::buildZoneType(const char *t0, const char* t1) {
	uint16 it = 0;
	if (t1[0] != '\0') {
		it = 4 + _vm->_objectsNames->lookup(t1);
	}
	uint16 zt = _zoneTypeNames->lookup(t0);
	return PACK_ZONETYPE(zt, it);
}


DECLARE_ZONE_PARSER(type)  {
	debugC(7, kDebugParser, "ZONE_PARSER(type) ");

	ctxt.z->_type = buildZoneType(_tokens[1], _tokens[2]);
	if (ACTIONTYPE(ctxt.z) != 0) {
		parseZoneTypeBlock(ctxt.z);
	}

	_parser->popTables();
}


DECLARE_ZONE_PARSER(commands)  {
	debugC(7, kDebugParser, "ZONE_PARSER(commands) ");

	 parseCommands(ctxt.z->_commands);
}


DECLARE_ZONE_PARSER(label)  {
	debugC(7, kDebugParser, "ZONE_PARSER(label) ");

//			debug("label: %s", _tokens[1]);
	ctxt.z->_label = _vm->_gfx->renderFloatingLabel(_vm->_labelFont, _tokens[1]);
	ctxt.z->_flags &= ~kFlagsNoName;
}


DECLARE_ZONE_PARSER(flags)  {
	debugC(7, kDebugParser, "ZONE_PARSER(flags) ");

	uint16 _si = 1;

	do {
		char _al = _zoneFlagNames->lookup(_tokens[_si]);
		_si++;
		ctxt.z->_flags |= 1 << (_al - 1);
	} while (!scumm_stricmp(_tokens[_si++], "|"));
}

void LocationParser_ns::parseZone(ZoneList &list, char *name) {
	debugC(5, kDebugParser, "parseZone(name: %s)", name);

	if (_vm->_location.findZone(name)) {
		_zoneProg++;
		_script->skip("endzone");
		return;
	}

	ZonePtr z(new Zone);
	_zoneProg++;

	strncpy(z->_name, name, ZONENAME_LENGTH);

	ctxt.z = z;

	list.push_front(z);

	_parser->pushTables(&_locationZoneParsers, _locationZoneStmt);

	return;
}




void LocationParser_ns::parseGetData(ZonePtr z) {
	TypeData *data = &z->u;
	if (!scumm_stricmp(_tokens[0], "file")) {
		GfxObj *obj = _vm->_gfx->loadGet(_tokens[1]);
		obj->frame = 0;
		obj->x = z->getX();
		obj->y = z->getY();
		obj->_prog = _zoneProg;

		// WORKAROUND for script bug #2969913
		// The katana object has the same default z index (kGfxObjGetZ or -100)
		// as the cripta object (the safe) - a script bug.
		// Game scripts do not set an explicit z for the katana (as it isn't an
		// animation), but rather rely on the draw order to draw it over the
		// safe. In this particular case, the safe is added to the scene after
		// the katana, thus it is drawn over the katana. We explicitly set the
		// z index of the katana to be higher than the safe, so that the katana
		// is drawn correctly over it.
		// This is a regression from the graphics rewrite (commits be2c5d3,
		// 3c2c16c and 44906f5).
		if (!scumm_stricmp(obj->getName(), "katana"))
			obj->z = 0;

		bool visible = (z->_flags & kFlagsRemove) == 0;
		_vm->_gfx->showGfxObj(obj, visible);
		data->_gfxobj = obj;
	} else
	if (!scumm_stricmp(_tokens[0], "icon")) {
		data->_getIcon = 4 + _vm->_objectsNames->lookup(_tokens[1]);
	}
}

void LocationParser_ns::parseExamineData(ZonePtr z) {
	TypeData *data = &z->u;
	if (!scumm_stricmp(_tokens[0], "file")) {
		data->_filename = _tokens[1];
	} else
	if (!scumm_stricmp(_tokens[0], "desc")) {
		data->_examineText = parseComment();
	}
}

void LocationParser_ns::parseDoorData(ZonePtr z) {
	TypeData *data = &z->u;
	if (!scumm_stricmp(_tokens[0], "slidetext")) {
		_vm->_location._slideText[0] = _tokens[1];
		_vm->_location._slideText[1] = _tokens[2];
	} else
	if (!scumm_stricmp(_tokens[0], "location")) {
		data->_doorLocation = _tokens[1];
	} else
	if (!scumm_stricmp(_tokens[0], "file")) {
		GfxObj *obj = _vm->_gfx->loadDoor(_tokens[1]);
		obj->frame = z->_flags & kFlagsClosed ? 0 : 1;
		obj->x = z->getX();
		obj->y = z->getY();
		_vm->_gfx->showGfxObj(obj, true);
		data->_gfxobj = obj;
	} else
	if (!scumm_stricmp(_tokens[0],	"startpos")) {
		data->_doorStartPos.x = atoi(_tokens[1]);
		data->_doorStartPos.y = atoi(_tokens[2]);
		data->_doorStartFrame = atoi(_tokens[3]);
	}
}

void LocationParser_ns::parseMergeData(ZonePtr z) {
	TypeData *data = &z->u;
	if (!scumm_stricmp(_tokens[0], "obj1")) {
		data->_mergeObj1 = 4 + _vm->_objectsNames->lookup(_tokens[1]);
	} else
	if (!scumm_stricmp(_tokens[0], "obj2")) {
		data->_mergeObj2 = 4 + _vm->_objectsNames->lookup(_tokens[1]);
	} else
	if (!scumm_stricmp(_tokens[0], "newobj")) {
		data->_mergeObj3 = 4 + _vm->_objectsNames->lookup(_tokens[1]);
	}
}

void LocationParser_ns::parseHearData(ZonePtr z) {
	TypeData *data = &z->u;
	if (!scumm_stricmp(_tokens[0], "sound")) {
		data->_filename = _tokens[1];
		data->_hearChannel = atoi(_tokens[2]);
	} else
	if (!scumm_stricmp(_tokens[0], "freq")) {
		data->_hearFreq = atoi(_tokens[1]);
	}
}

void LocationParser_ns::parseSpeakData(ZonePtr z) {
	TypeData *data = &z->u;
	if (!scumm_stricmp(_tokens[0], "file")) {
		data->_filename = _tokens[1];
	} else
	if (!scumm_stricmp(_tokens[0], "Dialogue")) {
		data->_speakDialogue = parseDialogue();
	}
}

void LocationParser_ns::parseNoneData(ZonePtr z) {
	// "None" zones should have no content, but some
	// inconsistently define their command list after
	// the TYPE marker. This routine catches these
	// command lists that would be lost otherwise.
	if (!scumm_stricmp(_tokens[0], "commands")) {
		parseCommands(z->_commands);
		ctxt.endcommands = false;
		do {
			_script->readLineToken(true);
			_parser->parseStatement();
		} while (!ctxt.endcommands);

		// no need to parse one more line here, as
		// it is done by the caller
	}
}

typedef void (LocationParser_ns::*ZoneTypeParser)(ZonePtr);
static ZoneTypeParser parsers[] = {
	0,	// no type
	&LocationParser_ns::parseExamineData,
	&LocationParser_ns::parseDoorData,
	&LocationParser_ns::parseGetData,
	&LocationParser_ns::parseMergeData,
	0,	// taste
	&LocationParser_ns::parseHearData,
	0,	// feel
	&LocationParser_ns::parseSpeakData,
	&LocationParser_ns::parseNoneData,
	0,	// trap
	0,	// you
	0	// command
};

void LocationParser_ns::parseZoneTypeBlock(ZonePtr z) {
	debugC(7, kDebugParser, "parseZoneTypeBlock(name: %s, type: %x)", z->_name, z->_type);

	ZoneTypeParser p = parsers[ACTIONTYPE(z)];
	do {
		if (p) {
			(this->*p)(z);
		}
		_script->readLineToken(true);
	} while (scumm_stricmp(_tokens[0], "endzone") && scumm_stricmp(_tokens[0], "endanimation"));
	debugC(7, kDebugParser, "parseZoneTypeBlock() done");
}


} // namespace Parallaction
