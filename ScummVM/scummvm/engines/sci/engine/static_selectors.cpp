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

// We place selector vocab name tables here for any game that doesn't have
// them. This includes the King's Quest IV Demo and LSL3 Demo.

#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/vm.h"

namespace Sci {

struct SelectorRemap {
	SciVersion minVersion;
	SciVersion maxVersion;
	const char *name;
	uint32 slot;
};

static const char * const sci0Selectors[] = {
	           "y",          "x",         "view",      "loop",        "cel", //  0 -  4
	   "underBits",      "nsTop",       "nsLeft",  "nsBottom",    "nsRight", //  5 -  9
	       "lsTop",     "lsLeft",     "lsBottom",   "lsRight",     "signal", // 10 - 14
	 "illegalBits",      "brTop",       "brLeft",  "brBottom",    "brRight", // 15 - 19
	        "name",        "key",         "time",      "text",   "elements", // 20 - 25
	       "color",       "back",         "mode",     "style",      "state", // 25 - 29
	        "font",       "type",       "window",    "cursor",        "max", // 30 - 34
	        "mark",        "who",      "message",      "edit",       "play", // 35 - 39
	      "number",     "handle",       "client",        "dx",         "dy", // 40 - 44
	   "b-moveCnt",       "b-i1",         "b-i2",      "b-di",    "b-xAxis", // 45 - 49
	      "b-incr",      "xStep",        "yStep", "moveSpeed",  "canBeHere", // 50 - 54
	     "heading",      "mover",         "doit", "isBlocked",     "looper", // 55 - 59
	    "priority",  "modifiers",       "replay",    "setPri",         "at", // 60 - 64
	        "next",       "done",        "width",  "wordFail", "syntaxFail", // 65 - 69
	"semanticFail", "pragmaFail",         "said",   "claimed",      "value", // 70 - 74
	        "save",    "restore",        "title",    "button",       "icon", // 75 - 79
	        "draw",     "delete",            "z"                             // 80 - 82
};

static const char * const sci1Selectors[] = {
	  "parseLang",  "printLang", "subtitleLang",       "size",    "points", // 83 - 87
	    "palette",    "dataInc",       "handle",        "min",       "sec", // 88 - 92
	      "frame",        "vol",          "pri",    "perform",  "moveDone"  // 93 - 97
};

static const char * const sci11Selectors[] = {
	  "topString",      "flags",    "quitGame",     "restart",      "hide", // 98 - 102
	"scaleSignal",     "scaleX",      "scaleY",    "maxScale","vanishingX", // 103 - 107
	 "vanishingY"                                                           // 108
};

#ifdef ENABLE_SCI32
static const char * const sci2Selectors[] = {
	    "plane",           "x",           "y",            "z",     "scaleX", //  0 -  4
	   "scaleY",    "maxScale",    "priority",  "fixPriority",     "inLeft", //  5 -  9
	    "inTop",     "inRight",    "inBottom", "useInsetRect",       "view", // 10 - 14
	     "loop",         "cel",      "bitmap",       "nsLeft",      "nsTop", // 15 - 19
	  "nsRight",    "nsBottom",      "lsLeft",        "lsTop",    "lsRight", // 20 - 25
	 "lsBottom",      "signal", "illegalBits",       "brLeft",      "brTop", // 25 - 29
	  "brRight",    "brBottom",        "name",          "key",       "time", // 30 - 34
	     "text",    "elements",        "fore",         "back",       "mode", // 35 - 39
	    "style",       "state",        "font",         "type",     "window", // 40 - 44
	   "cursor",         "max",        "mark",          "who",    "message", // 45 - 49
	     "edit",        "play",      "number",      "nodePtr",     "client", // 50 - 54
	       "dx",          "dy",   "b-moveCnt",         "b-i1",       "b-i2", // 55 - 59
	     "b-di",     "b-xAxis",      "b-incr",        "xStep",      "yStep", // 60 - 64
	"moveSpeed",  "cantBeHere",     "heading",        "mover",       "doit", // 65 - 69
	"isBlocked",      "looper",   "modifiers",       "replay",     "setPri", // 70 - 74
	       "at",        "next",        "done",        "width", "pragmaFail", // 75 - 79
	  "claimed",       "value",        "save",      "restore",      "title", // 80 - 84
	   "button",        "icon",        "draw",       "delete",  "printLang", // 85 - 89
	     "size",      "points",     "palette",      "dataInc",     "handle", // 90 - 94
	      "min",         "sec",       "frame",          "vol",    "perform", // 95 - 99
	 "moveDone",   "topString",       "flags",     "quitGame",    "restart", // 100 - 104
	     "hide", "scaleSignal",  "vanishingX",   "vanishingY",    "picture", // 105 - 109
	     "resX",        "resY",   "coordType",         "data",       "skip", // 110 - 104
	   "center",         "all",        "show",     "textLeft",    "textTop", // 115 - 119
	"textRight",  "textBottom", "borderColor",    "titleFore",  "titleBack", // 120 - 124
	"titleFont",      "dimmed",    "frameOut",      "lastKey",  "magnifier", // 125 - 129
	 "magPower",    "mirrored",       "pitch",         "roll",        "yaw", // 130 - 134
	     "left",       "right",         "top",       "bottom",   "numLines"  // 135 - 139
};
#endif

static const SelectorRemap sciSelectorRemap[] = {
	{    SCI_VERSION_0_EARLY,     SCI_VERSION_0_LATE,   "moveDone",  170 },
	{    SCI_VERSION_0_EARLY,     SCI_VERSION_0_LATE,     "points",  316 },
	{    SCI_VERSION_0_EARLY,     SCI_VERSION_0_LATE,      "flags",  368 },
	{    SCI_VERSION_1_EARLY,     SCI_VERSION_1_LATE,    "nodePtr",   44 },
	{     SCI_VERSION_1_LATE,     SCI_VERSION_1_LATE, "cantBeHere",   57 },
	{    SCI_VERSION_1_EARLY,     SCI_VERSION_1_LATE,  "topString",  101 },
	{    SCI_VERSION_1_EARLY,     SCI_VERSION_1_LATE,      "flags",  102 },
	// SCI1.1
	{        SCI_VERSION_1_1,        SCI_VERSION_1_1,    "nodePtr",   41 },
	{        SCI_VERSION_1_1,        SCI_VERSION_1_1, "cantBeHere",   54 },
	// The following are not really needed. They've only been defined to
	// ease game debugging.
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1,    "-objID-", 4096 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1,     "-size-", 4097 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1, "-propDict-", 4098 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1, "-methDict-", 4099 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1, "-classScript-", 4100 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1,   "-script-", 4101 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1,    "-super-", 4102 },
	//
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1,     "-info-", 4103 },
	{ SCI_VERSION_NONE,             SCI_VERSION_NONE,            0,    0 }
};

struct ClassReference {
	int script;
	const char *className;
	const char *selectorName;
	SelectorType selectorType;
	uint selectorOffset;
};

// For variable selectors, we ignore the global selectors and start off from
// the object's selectors (i.e. from the name selector onwards). Thus, the
// following are not taken into consideration when calculating the indices of
// variable selectors in this array:
// SCI0 - SCI1: species, superClass, -info-
// SCI1.1: -objID-, -size-, -propDict-, -methDict-, -classScript-, -script-,
//         -super-, -info-
static const ClassReference classReferences[] = {
	{   0, "Character",         "say",   kSelectorMethod,  5 },	// Crazy Nick's Soft Picks
	{ 928,  "Narrator",         "say",   kSelectorMethod,  4 },
	{ 928,  "Narrator",   "startText",   kSelectorMethod,  5 },
	{ 929,      "Sync",    "syncTime", kSelectorVariable,  1 },
	{ 929,      "Sync",     "syncCue", kSelectorVariable,  2 },
	{ 981, "SysWindow",        "open",   kSelectorMethod,  1 },
	{ 999,    "Script",        "init",   kSelectorMethod,  0 },
	{ 999,    "Script",     "dispose",   kSelectorMethod,  2 },
	{ 999,    "Script", "changeState",   kSelectorMethod,  3 }
};

Common::StringArray Kernel::checkStaticSelectorNames() {
	Common::StringArray names;
	const int offset = (getSciVersion() < SCI_VERSION_1_1) ? 3 : 0;

#ifdef ENABLE_SCI32
	const int count = (getSciVersion() <= SCI_VERSION_1_1) ? ARRAYSIZE(sci0Selectors) + offset : ARRAYSIZE(sci2Selectors);
#else
	const int count = ARRAYSIZE(sci0Selectors) + offset;
#endif
	int countSci1 = ARRAYSIZE(sci1Selectors);
	int countSci11 = ARRAYSIZE(sci11Selectors);

	// Resize the list of selector names and fill in the SCI 0 names.
	names.resize(count);
	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		// Fill selectors 0 - 2 for SCI0 - SCI1 late
		names[0] = "species";
		names[1] = "superClass";
		names[2] = "-info-";
	}

	if (getSciVersion() <= SCI_VERSION_1_1) {
		// SCI0 - SCI11
		for (int i = offset; i < count; i++)
			names[i] = sci0Selectors[i - offset];

		if (getSciVersion() > SCI_VERSION_01) {
			// Several new selectors were added in SCI 1 and later.
			names.resize(count + countSci1);
			for (int i = count; i < count + countSci1; i++)
				names[i] = sci1Selectors[i - count];
		}

		if (getSciVersion() >= SCI_VERSION_1_1) {
			// Several new selectors were added in SCI 1.1
			names.resize(count + countSci1 + countSci11);
			for (int i = count + countSci1; i < count + countSci1 + countSci11; i++)
				names[i] = sci11Selectors[i - count - countSci1];
		}
#ifdef ENABLE_SCI32
	} else {
		// SCI2+
		for (int i = 0; i < count; i++)
			names[i] = sci2Selectors[i];
#endif
	}

	findSpecificSelectors(names);

	for (const SelectorRemap *selectorRemap = sciSelectorRemap; selectorRemap->slot; ++selectorRemap) {
		if (getSciVersion() >= selectorRemap->minVersion && getSciVersion() <= selectorRemap->maxVersion) {
			const uint32 slot = selectorRemap->slot;
			if (slot >= names.size())
				names.resize(slot + 1);
			names[slot] = selectorRemap->name;
		}
	}

	return names;
}

void Kernel::findSpecificSelectors(Common::StringArray &selectorNames) {
	// Now, we need to find out selectors which keep changing place...
	// We do that by dissecting game objects, and looking for selectors at
	// specified locations.

	// We need to initialize script 0 here, to make sure that it's always
	// located at segment 1.
	_segMan->instantiateScript(0);
	uint16 sci2Offset = (getSciVersion() >= SCI_VERSION_2) ? 64000 : 0;

	// The Actor class contains the init, xLast and yLast selectors, which
	// we reference directly. It's always in script 998, so we need to
	// explicitly load it here.
	if ((getSciVersion() >= SCI_VERSION_1_EGA_ONLY)) {
		uint16 actorScript = 998;

		if (_resMan->testResource(ResourceId(kResourceTypeScript, actorScript + sci2Offset))) {
			_segMan->instantiateScript(actorScript + sci2Offset);

			const Object *actorClass = _segMan->getObject(_segMan->findObjectByName("Actor"));

			if (actorClass) {
				// Find the xLast and yLast selectors, used in kDoBresen

				const int offset = (getSciVersion() < SCI_VERSION_1_1) ? 3 : 0;
				const int offset2 = (getSciVersion() >= SCI_VERSION_2) ? 12 : 0;
				// xLast and yLast always come between illegalBits and xStep
				int illegalBitsSelectorPos = actorClass->locateVarSelector(_segMan, 15 + offset + offset2);	// illegalBits
				int xStepSelectorPos = actorClass->locateVarSelector(_segMan, 51 + offset + offset2);	// xStep
				if (xStepSelectorPos - illegalBitsSelectorPos != 3) {
					error("illegalBits and xStep selectors aren't found in "
							"known locations. illegalBits = %d, xStep = %d",
							illegalBitsSelectorPos, xStepSelectorPos);
				}

				int xLastSelectorPos = actorClass->getVarSelector(illegalBitsSelectorPos + 1);
				int yLastSelectorPos = actorClass->getVarSelector(illegalBitsSelectorPos + 2);

				if (selectorNames.size() < (uint32)yLastSelectorPos + 1)
					selectorNames.resize((uint32)yLastSelectorPos + 1);

				selectorNames[xLastSelectorPos] = "xLast";
				selectorNames[yLastSelectorPos] = "yLast";
			}	// if (actorClass)

			_segMan->uninstantiateScript(998);
		}	// if (_resMan->testResource(ResourceId(kResourceTypeScript, 998)))
	}	// if ((getSciVersion() >= SCI_VERSION_1_EGA_ONLY))

	// Find selectors from specific classes

	for (int i = 0; i < ARRAYSIZE(classReferences); i++) {
		if (!_resMan->testResource(ResourceId(kResourceTypeScript, classReferences[i].script + sci2Offset)))
			continue;

		_segMan->instantiateScript(classReferences[i].script + sci2Offset);

		const Object *targetClass = _segMan->getObject(_segMan->findObjectByName(classReferences[i].className));
		int targetSelectorPos = 0;
		uint selectorOffset = classReferences[i].selectorOffset;

		if (targetClass) {
			if (classReferences[i].selectorType == kSelectorMethod) {
				if (targetClass->getMethodCount() < selectorOffset + 1)
					error("The %s class has less than %d methods (%d)",
							classReferences[i].className, selectorOffset + 1,
							targetClass->getMethodCount());

				targetSelectorPos = targetClass->getFuncSelector(selectorOffset);
			} else {
				// Add the global selectors to the selector ID
				selectorOffset += (getSciVersion() <= SCI_VERSION_1_LATE) ? 3 : 8;

				if (targetClass->getVarCount() < selectorOffset + 1)
					error("The %s class has less than %d variables (%d)",
							classReferences[i].className, selectorOffset + 1,
							targetClass->getVarCount());

				targetSelectorPos = targetClass->getVarSelector(selectorOffset);
			}

			if (selectorNames.size() < (uint32)targetSelectorPos + 1)
				selectorNames.resize((uint32)targetSelectorPos + 1);


			selectorNames[targetSelectorPos] = classReferences[i].selectorName;
		}
	}

	// Reset the segment manager
	_segMan->resetSegMan();
}

} // End of namespace Sci
