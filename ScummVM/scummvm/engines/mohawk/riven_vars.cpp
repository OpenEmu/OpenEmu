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

#include "common/str.h"

#include "mohawk/riven.h"

namespace Mohawk {

// The Riven variable system is complex. The scripts of each stack give a number, but the number has to be matched
// to a variable name defined in NAME resource 4.

static const char *variableNames[] = {
	// aspit
	"aatrusbook",
	"aatruspage",
	"acathbook",
	"acathpage",
	"acathstate",
	"adoit",
	"adomecombo",
	"agehn",
	"ainventory",
	"aova",
	"apower",
	"araw",
	"atemp",
	"atrap",
	"atrapbook",
	"auservolume",
	"azip",

	// bspit
	"bbacklock",
	"bbait",
	"bbigbridge",
	"bbirds",
	"bblrarm",
	"bblrdoor",
	"bblrgrt",
	"bblrsw",
	"bblrvalve",
	"bblrwtr",
	"bbook",
	"bbrlever",
	"bcavedoor",
	"bcombo",
	"bcpipegr",
	"bcratergg",
	"bdome",
	"bdrwr",
	"bfans",
	"bfmdoor",
	"bidvlv",
	"blab",
	"blabbackdr",
	"blabbook",
	"blabeye",
	"blabfrontdr",
	"blabpage",
	"blever",
	"bfrontlock",
	"bheat",
	"bmagcar",
	"bpipdr",
	"bprs",
	"bstove",
	"btrap",
	"bvalve",
	"bvise",
	"bytram",
	"bytramtime",
	"bytrap",
	"bytrapped",

	// gspit
	"gbook",
	"gcathtime",
	"gcathstate",
	"gcombo",
	"gdome",
	"gemagcar",
	"gimagecurr",
	"gimagemax",
	"gimagerot",
	"glkbtns",
	"glkbridge",
	"glkelev",
	"glview",
	"glviewmpos",
	"glviewpos",
	"gnmagrot",
	"gnmagcar",
	"gpinup",
	"gpinpos",
	"gpinsmpos",
	"grview",
	"grviewmpos",
	"grviewpos",
	"gscribe",
	"gscribetime",
	"gsubelev",
	"gsubdr",
	"gupmoov",
	"gwhark",
	"gwharktime",

	// jspit
	"jwmagcar",
	"jbeetle",
	"jbeetlepool",
	"jbook",
	"jbridge1",
	"jbridge2",
	"jbridge3",
	"jbridge4",
	"jbridge5",
	"jccb",
	"jcombo",
	"jcrg",
	"jdome",
	"jdrain",
	"jgallows",
	"jgate",
	"jgirl",
	"jiconcorrectorder",
	"jiconorder",
	"jicons",
	"jladder",
	"jleftpos",
	"jpeek",
	"jplaybeetle",
	"jprebel",
	"jprisondr",
	"jprisonsecdr",
	"jrbook",
	"jrightpos",
	"jsouthpathdr",
	"jschooldr",
	"jsub",
	"jsubdir",
	"jsubhatch",
	"jsubsw",
	"jsunners",
	"jsunnertime",
	"jthronedr",
	"jtunneldr",
	"jtunnellamps",
	"jvillagepeople",
	"jwarning",
	"jwharkpos",
	"jwharkram",
	"jwmouth",
	"jwmagcar",
	"jymagcar",

	// ospit
	"oambient",
	"obutton",
	"ocage",
	"odeskbook",
	"ogehnpage",
	"omusicplayer",
	"ostanddrawer",
	"ostove",

	// pspit
	"pbook",
	"pcage",
	"pcathcheck",
	"pcathstate",
	"pcathtime",
	"pcombo",
	"pcorrectorder",
	"pdome",
	"pelevcombo",
	"pleftpos",
	"prightpos",
	"ptemp",
	"pwharkpos",

	// rspit
	"rrebel",
	"rrebelview",
	"rrichard",
	"rvillagetime",

	// tspit
	"tbars",
	"tbeetle",
	"tblue",
	"tbook",
	"tbookvalve",
	"tcage",
	"tcombo",
	"tcorrectorder",
	"tcovercombo",
	"tdl",
	"tdome",
	"tdomeelev",
	"tdomeelevbtn",
	"tgatebrhandle",
	"tgatebridge",
	"tgatestate",
	"tgreen",
	"tgridoor",
	"tgrodoor",
	"tgrmdoor",
	"tguard",
	"timagedoor",
	"tmagcar",
	"torange",
	"tred",
	"tsecdoor",
	"tsubbridge",
	"ttelecover",
	"ttelehandle",
	"ttelepin",
	"ttelescope",
	"ttelevalve",
	"ttemple",
	"ttempledoor",
	"ttunneldoor",
	"tviewer",
	"tviolet",
	"twabrvalve",
	"twaffle",
	"tyellow",

	// Miscellaneous
	"elevbtn1",
	"elevbtn2",
	"elevbtn3",
	"domecheck",
	"transitionsenabled",
	"transitionmode",
	"waterenabled",
	"rivenambients",
	"stackvarsinitialized",
	"doingsetupscreens",
	"all_book",
	"playerhasbook",
	"returnstackid",
	"returncardid",
	"newpos",
	"themarble",
	"currentstackid",
	"currentcardid"
};

uint32 &MohawkEngine_Riven::getStackVar(uint32 index) {
	Common::String name = getName(VariableNames, index);

	if (!_vars.contains(name))
		error("Could not find variable '%s' (stack variable %d)", name.c_str(), index);

	return _vars[name];
}

void MohawkEngine_Riven::initVars() {
	// Most variables just start at 0, it's simpler to do this
	for (uint32 i = 0; i < ARRAYSIZE(variableNames); i++)
		_vars[variableNames[i]] = 0;

	// Initialize the rest of the variables to their proper state
	_vars["ttelescope"] = 5;
	_vars["tgatestate"] = 1;
	_vars["jbridge1"] = 1;
	_vars["jbridge4"] = 1;
	_vars["jgallows"] = 1;
	_vars["jiconcorrectorder"] = 12068577;
	_vars["bblrvalve"] = 1;
	_vars["bblrwtr"] = 1;
	_vars["bfans"] = 1;
	_vars["bytrap"] = 2;
	_vars["aatruspage"] = 1;
	_vars["acathpage"] = 1;
	_vars["bheat"] = 1;
	_vars["waterenabled"] = 1;
	_vars["ogehnpage"] = 1;
	_vars["bblrsw"] = 1;
	_vars["ocage"] = 1;
	_vars["jbeetle"] = 1;
	_vars["tdl"] = 1;
	_vars["bmagcar"] = 1;
	_vars["gnmagcar"] = 1;
	_vars["omusicplayer"] = 1;
	_vars["transitionmode"] = kRivenTransitionSpeedFastest;
	_vars["tdomeelev"] = 1;

	// Randomize the telescope combination
	uint32 &teleCombo = _vars["tcorrectorder"];
	for (byte i = 0; i < 5; i++) {
		teleCombo *= 10;
		teleCombo += _rnd->getRandomNumberRng(1, 5); // 5 buttons
	}

	// Randomize the prison combination
	uint32 &prisonCombo = _vars["pcorrectorder"];
	for (byte i = 0; i < 5; i++) {
		prisonCombo *= 10;
		prisonCombo += _rnd->getRandomNumberRng(1, 3); // 3 buttons/sounds
	}

	// Randomize the dome combination -- each bit represents a slider position,
	// the highest bit (1 << 24) represents 1, (1 << 23) represents 2, etc.
	uint32 &domeCombo = _vars["adomecombo"];
	for (byte bitsSet = 0; bitsSet < 5;) {
		uint32 randomBit = 1 << (24 - _rnd->getRandomNumber(24));

		// Don't overwrite a bit we already set, and throw out the bottom five bits being set
		if (domeCombo & randomBit || (domeCombo | randomBit) == 31)
			continue;

		domeCombo |= randomBit;
		bitsSet++;
	}
}

} // End of namespace Mohawk
