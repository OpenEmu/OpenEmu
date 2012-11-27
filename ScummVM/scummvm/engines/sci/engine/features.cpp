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

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"

#include "common/config-manager.h"
#include "common/file.h"

namespace Sci {

GameFeatures::GameFeatures(SegManager *segMan, Kernel *kernel) : _segMan(segMan), _kernel(kernel) {
	_setCursorType = SCI_VERSION_NONE;
	_doSoundType = SCI_VERSION_NONE;
	_lofsType = SCI_VERSION_NONE;
	_gfxFunctionsType = SCI_VERSION_NONE;
	_messageFunctionType = SCI_VERSION_NONE;
	_moveCountType = kMoveCountUninitialized;
#ifdef ENABLE_SCI32
	_sci21KernelType = SCI_VERSION_NONE;
	_sci2StringFunctionType = kSci2StringFunctionUninitialized;
#endif
	_usesCdTrack = Common::File::exists("cdaudio.map");
	if (!ConfMan.getBool("use_cdaudio"))
		_usesCdTrack = false;
	_forceDOSTracks = false;
}

reg_t GameFeatures::getDetectionAddr(const Common::String &objName, Selector slc, int methodNum) {
	// Get address of target object
	reg_t objAddr = _segMan->findObjectByName(objName, 0);
	reg_t addr;

	if (objAddr.isNull()) {
		error("getDetectionAddr: %s object couldn't be found", objName.c_str());
		return NULL_REG;
	}

	if (methodNum == -1) {
		if (lookupSelector(_segMan, objAddr, slc, NULL, &addr) != kSelectorMethod) {
			error("getDetectionAddr: target selector is not a method of object %s", objName.c_str());
			return NULL_REG;
		}
	} else {
		addr = _segMan->getObject(objAddr)->getFunction(methodNum);
	}

	return addr;
}

bool GameFeatures::autoDetectSoundType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Sound", SELECTOR(play));

	if (!addr.getSegment())
		return false;

	uint16 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());
	uint16 intParam = 0xFFFF;
	bool foundTarget = false;

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		// The play method of the Sound object pushes the DoSound command that
		// it will use just before it calls DoSound. We intercept that here in
		// order to check what sound semantics are used, cause the position of
		// the sound commands has changed at some point during SCI1 middle.
		if (opcode == op_pushi) {
			// Load the pushi parameter
			intParam = opparams[0];
		} else if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Late SCI1 games call kIsObject before kDoSound
			if (kFuncNum == 6) {	// kIsObject (SCI0-SCI11)
				foundTarget = true;
			} else if (kFuncNum == 45) {	// kDoSound (SCI1)
				// First, check which DoSound function is called by the play
				// method of the Sound object
				switch (intParam) {
				case 1:
					_doSoundType = SCI_VERSION_0_EARLY;
					break;
				case 7:
					_doSoundType = SCI_VERSION_1_EARLY;
					break;
				case 8:
					_doSoundType = SCI_VERSION_1_LATE;
					break;
				default:
					// Unknown case... should never happen. We fall back to
					// alternative detection here, which works in general, apart
					// from some transitive games like Jones CD
					_doSoundType = foundTarget ? SCI_VERSION_1_LATE : SCI_VERSION_1_EARLY;
					break;
				}

				if (_doSoundType != SCI_VERSION_NONE)
					return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectDoSoundType() {
	if (_doSoundType == SCI_VERSION_NONE) {
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			// Almost all of the SCI0EARLY games use different sound resources than
			//  SCI0LATE. Although the last SCI0EARLY game (lsl2) uses SCI0LATE resources
			_doSoundType = g_sci->getResMan()->detectEarlySound() ? SCI_VERSION_0_EARLY : SCI_VERSION_0_LATE;
#ifdef ENABLE_SCI32
		} else if (getSciVersion() >= SCI_VERSION_2_1) {
			_doSoundType = SCI_VERSION_2_1;
#endif
		} else if (SELECTOR(nodePtr) == -1) {
			// No nodePtr selector, so this game is definitely using newer
			// SCI0 sound code (i.e. SCI_VERSION_0_LATE)
			_doSoundType = SCI_VERSION_0_LATE;
		} else if (getSciVersion() >= SCI_VERSION_1_LATE) {
			// All SCI1 late games use the newer doSound semantics
			_doSoundType = SCI_VERSION_1_LATE;
		} else {
			if (!autoDetectSoundType()) {
				warning("DoSound detection failed, taking an educated guess");

				if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
					_doSoundType = SCI_VERSION_1_LATE;
				else if (getSciVersion() > SCI_VERSION_01)
					_doSoundType = SCI_VERSION_1_EARLY;
			}
		}

		debugC(1, kDebugLevelSound, "Detected DoSound type: %s", getSciVersionDesc(_doSoundType));
	}

	return _doSoundType;
}

SciVersion GameFeatures::detectSetCursorType() {
	if (_setCursorType == SCI_VERSION_NONE) {
		if (getSciVersion() <= SCI_VERSION_1_MIDDLE) {
			// SCI1 middle and older games never use cursor views
			_setCursorType = SCI_VERSION_0_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_1_1) {
			// SCI1.1 games always use cursor views
			_setCursorType = SCI_VERSION_1_1;
		} else {	// SCI1 late game, detect cursor semantics
			// If the Cursor object doesn't exist, we're using the SCI0 early
			// kSetCursor semantics.
			if (_segMan->findObjectByName("Cursor") == NULL_REG) {
				_setCursorType = SCI_VERSION_0_EARLY;
				debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
				return _setCursorType;
			}

			// Check for the existence of the handCursor object (first found).
			// This is based on KQ5.
			reg_t objAddr = _segMan->findObjectByName("handCursor", 0);

			// If that doesn't exist, we assume it uses SCI1.1 kSetCursor semantics
			if (objAddr == NULL_REG) {
				_setCursorType = SCI_VERSION_1_1;
				debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
				return _setCursorType;
			}

			// Now we check what the number variable holds in the handCursor
			// object.
			uint16 number = readSelectorValue(_segMan, objAddr, SELECTOR(number));

			// If the number is 0, it uses views and therefore the SCI1.1
			// kSetCursor semantics, otherwise it uses the SCI0 early kSetCursor
			// semantics.
			if (number == 0)
				_setCursorType = SCI_VERSION_1_1;
			else
				_setCursorType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelGraphics, "Detected SetCursor type: %s", getSciVersionDesc(_setCursorType));
	}

	return _setCursorType;
}

bool GameFeatures::autoDetectLofsType(Common::String gameSuperClassName, int methodNum) {
	// Look up the script address
	reg_t addr = getDetectionAddr(gameSuperClassName.c_str(), -1, methodNum);

	if (!addr.getSegment())
		return false;

	uint16 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_lofsa || opcode == op_lofss) {
			// Load lofs operand
			uint16 lofs = opparams[0];

			// Check for going out of bounds when interpreting as abs/rel
			if (lofs >= script->getBufSize())
				_lofsType = SCI_VERSION_0_EARLY;

			if ((signed)offset + (int16)lofs < 0)
				_lofsType = SCI_VERSION_1_MIDDLE;

			if ((signed)offset + (int16)lofs >= (signed)script->getBufSize())
				_lofsType = SCI_VERSION_1_MIDDLE;

			if (_lofsType != SCI_VERSION_NONE)
				return true;

			// If we reach here, we haven't been able to deduce the lofs
			// parameter type so far.
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectLofsType() {
	if (_lofsType == SCI_VERSION_NONE) {
		// This detection only works (and is only needed) for SCI 1
		if (getSciVersion() <= SCI_VERSION_01) {
			_lofsType = SCI_VERSION_0_EARLY;
			return _lofsType;
		}

		if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
			// SCI1.1 type, i.e. we compensate for the fact that the heap is attached
			// to the end of the script
			_lofsType = SCI_VERSION_1_1;
			return _lofsType;
		}

		if (getSciVersion() == SCI_VERSION_3) {
			// SCI3 type, same as pre-SCI1.1, really, as there is no separate heap
			// resource
			_lofsType = SCI_VERSION_3;
			return _lofsType;
		}

		// Find a function of the "Game" object (which is the game super class) which invokes lofsa/lofss
		const Object *gameObject = _segMan->getObject(g_sci->getGameObject());
		const Object *gameSuperObject = _segMan->getObject(gameObject->getSuperClassSelector());
		bool found = false;
		if (gameSuperObject) {
			Common::String gameSuperClassName = _segMan->getObjectName(gameObject->getSuperClassSelector());

			for (uint m = 0; m < gameSuperObject->getMethodCount(); m++) {
				found = autoDetectLofsType(gameSuperClassName, m);
				if (found)
					break;
			}
		} else {
			warning("detectLofsType(): Could not find superclass of game object");
		}

		if (!found) {
			warning("detectLofsType(): failed, taking an educated guess");

			if (getSciVersion() >= SCI_VERSION_1_MIDDLE)
				_lofsType = SCI_VERSION_1_MIDDLE;
			else
				_lofsType = SCI_VERSION_0_EARLY;
		}

		debugC(1, kDebugLevelVM, "Detected Lofs type: %s", getSciVersionDesc(_lofsType));
	}

	return _lofsType;
}

bool GameFeatures::autoDetectGfxFunctionsType(int methodNum) {
	// Look up the script address
	reg_t addr = getDetectionAddr("Rm", SELECTOR(overlay), methodNum);

	if (!addr.getSegment())
		return false;

	uint16 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];
			uint16 argc = opparams[1];

			if (kFuncNum == 8) {	// kDrawPic	(SCI0 - SCI11)
				// If kDrawPic is called with 6 parameters from the overlay
				// selector, the game is using old graphics functions.
				// Otherwise, if it's called with 8 parameters, it's using new
				// graphics functions.
				_gfxFunctionsType = (argc == 8) ? SCI_VERSION_0_LATE : SCI_VERSION_0_EARLY;
				return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectGfxFunctionsType() {
	if (_gfxFunctionsType == SCI_VERSION_NONE) {
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			// Old SCI0 games always used old graphics functions
			_gfxFunctionsType = SCI_VERSION_0_EARLY;
		} else if (getSciVersion() >= SCI_VERSION_01) {
			// SCI01 and newer games always used new graphics functions
			_gfxFunctionsType = SCI_VERSION_0_LATE;
		} else {	// SCI0 late
			// Check if the game is using an overlay
			bool searchRoomObj = false;
			reg_t rmObjAddr = _segMan->findObjectByName("Rm");

			if (SELECTOR(overlay) != -1) {
				// The game has an overlay selector, check how it calls kDrawPic
				// to determine the graphics functions type used
				if (lookupSelector(_segMan, rmObjAddr, SELECTOR(overlay), NULL, NULL) == kSelectorMethod) {
					if (!autoDetectGfxFunctionsType()) {
						warning("Graphics functions detection failed, taking an educated guess");

						// Try detecting the graphics function types from the
						// existence of the motionCue selector (which is a bit
						// of a hack)
						if (_kernel->findSelector("motionCue") != -1)
							_gfxFunctionsType = SCI_VERSION_0_LATE;
						else
							_gfxFunctionsType = SCI_VERSION_0_EARLY;
					}
				} else {
					// The game has an overlay selector, but it's not a method
					// of the Rm object (like in Hoyle 1 and 2), so search for
					// other methods
					searchRoomObj = true;
				}
			} else {
				// The game doesn't have an overlay selector, so search for it
				// manually
				searchRoomObj = true;
			}

			if (searchRoomObj) {
				// If requested, check if any method of the Rm object is calling
				// kDrawPic, as the overlay selector might be missing in demos
				bool found = false;

				const Object *obj = _segMan->getObject(rmObjAddr);
				for (uint m = 0; m < obj->getMethodCount(); m++) {
					found = autoDetectGfxFunctionsType(m);
					if (found)
						break;
				}

				if (!found) {
					// No method of the Rm object is calling kDrawPic, thus the
					// game doesn't have overlays and is using older graphics
					// functions
					_gfxFunctionsType = SCI_VERSION_0_EARLY;
				}
			}
		}

		debugC(1, kDebugLevelVM, "Detected graphics functions type: %s", getSciVersionDesc(_gfxFunctionsType));
	}

	return _gfxFunctionsType;
}

SciVersion GameFeatures::detectMessageFunctionType() {
	if (_messageFunctionType != SCI_VERSION_NONE)
		return _messageFunctionType;

	if (getSciVersion() > SCI_VERSION_1_1) {
		_messageFunctionType = SCI_VERSION_1_1;
		return _messageFunctionType;
	} else if (getSciVersion() < SCI_VERSION_1_1) {
		_messageFunctionType = SCI_VERSION_1_LATE;
		return _messageFunctionType;
	}

	Common::List<ResourceId> resources = g_sci->getResMan()->listResources(kResourceTypeMessage, -1);

	if (resources.empty()) {
		// No messages found, so this doesn't really matter anyway...
		_messageFunctionType = SCI_VERSION_1_1;
		return _messageFunctionType;
	}

	Resource *res = g_sci->getResMan()->findResource(*resources.begin(), false);
	assert(res);

	// Only v2 Message resources use the kGetMessage kernel function.
	// v3-v5 use the kMessage kernel function.

	if (READ_SCI11ENDIAN_UINT32(res->data) / 1000 == 2)
		_messageFunctionType = SCI_VERSION_1_LATE;
	else
		_messageFunctionType = SCI_VERSION_1_1;

	debugC(1, kDebugLevelVM, "Detected message function type: %s", getSciVersionDesc(_messageFunctionType));
	return _messageFunctionType;
}

#ifdef ENABLE_SCI32
bool GameFeatures::autoDetectSci21KernelType() {
	// First, check if the Sound object is loaded
	reg_t soundObjAddr = _segMan->findObjectByName("Sound");
	if (soundObjAddr.isNull()) {
		// Usually, this means that the Sound object isn't loaded yet.
		// This case doesn't occur in early SCI2.1 games, and we've only
		// seen it happen in the RAMA demo, thus we can assume that the
		// game is using a SCI2.1 table

		// HACK: The Inside the Chest Demo doesn't have sounds at all, but
		// it's using a SCI2 kernel
		if (g_sci->getGameId() == GID_CHEST) {
			_sci21KernelType = SCI_VERSION_2;
			return true;
		}

		warning("autoDetectSci21KernelType(): Sound object not loaded, assuming a SCI2.1 table");
		_sci21KernelType = SCI_VERSION_2_1;
		return true;
	}

	// Look up the script address
	reg_t addr = getDetectionAddr("Sound", SELECTOR(play));

	if (!addr.getSegment())
		return false;

	uint16 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Here we check for the kDoSound opcode that's used in SCI2.1.
			// Finding 0x40 as kDoSound in the Sound::play() function means the
			// game is using the modified SCI2 kernel table found in some older
			// SCI2.1 games (GK2 demo, KQ7 v1.4).
			// Finding 0x75 as kDoSound means the game is using the regular
			// SCI2.1 kernel table.
			if (kFuncNum == 0x40) {
				_sci21KernelType = SCI_VERSION_2;
				return true;
			} else if (kFuncNum == 0x75) {
				_sci21KernelType = SCI_VERSION_2_1;
				return true;
			}
		}
	}

	return false;	// not found
}

SciVersion GameFeatures::detectSci21KernelType() {
	if (_sci21KernelType == SCI_VERSION_NONE) {
		if (!autoDetectSci21KernelType())
			error("Could not detect the SCI2.1 kernel table type");

		debugC(1, kDebugLevelVM, "Detected SCI2.1 kernel type: %s", getSciVersionDesc(_sci21KernelType));
	}
	return _sci21KernelType;
}

Sci2StringFunctionType GameFeatures::detectSci2StringFunctionType() {
	if (_sci2StringFunctionType == kSci2StringFunctionUninitialized) {
		if (getSciVersion() <= SCI_VERSION_1_1) {
			error("detectSci21StringFunctionType() called from SCI1.1 or earlier");
		} else if (getSciVersion() == SCI_VERSION_2) {
			// SCI2 games are always using the old type
			_sci2StringFunctionType = kSci2StringFunctionOld;
		} else if (getSciVersion() == SCI_VERSION_3) {
			// SCI3 games are always using the new type
			_sci2StringFunctionType = kSci2StringFunctionNew;
		} else {	// SCI2.1
			if (!autoDetectSci21StringFunctionType())
				_sci2StringFunctionType = kSci2StringFunctionOld;
			else
				_sci2StringFunctionType = kSci2StringFunctionNew;
		}
	}

	debugC(1, kDebugLevelVM, "Detected SCI2 kString type: %s", (_sci2StringFunctionType == kSci2StringFunctionOld) ? "old" : "new");

	return _sci2StringFunctionType;
}

bool GameFeatures::autoDetectSci21StringFunctionType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Str", SELECTOR(size));

	if (!addr.getSegment())
		return false;

	uint16 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// SCI2.1 games which use the new kString functions call kString(8).
			// Earlier ones call the callKernel script function, but not kString
			// directly
			if (_kernel->getKernelName(kFuncNum) == "String")
				return true;
		}
	}

	return false;	// not found a call to kString
}

#endif

bool GameFeatures::autoDetectMoveCountType() {
	// Look up the script address
	reg_t addr = getDetectionAddr("Motion", SELECTOR(doit));

	if (!addr.getSegment())
		return false;

	uint16 offset = addr.getOffset();
	Script *script = _segMan->getScript(addr.getSegment());
	bool foundTarget = false;

	while (true) {
		int16 opparams[4];
		byte extOpcode;
		byte opcode;
		offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
		opcode = extOpcode >> 1;

		// Check for end of script
		if (opcode == op_ret || offset >= script->getBufSize())
			break;

		if (opcode == op_callk) {
			uint16 kFuncNum = opparams[0];

			// Games which ignore move count call kAbs before calling kDoBresen
			if (_kernel->getKernelName(kFuncNum) == "Abs") {
				foundTarget = true;
			} else if (_kernel->getKernelName(kFuncNum) == "DoBresen") {
				_moveCountType = foundTarget ? kIgnoreMoveCount : kIncrementMoveCount;
				return true;
			}
		}
	}

	return false;	// not found
}

MoveCountType GameFeatures::detectMoveCountType() {
	if (_moveCountType == kMoveCountUninitialized) {
		// SCI0/SCI01 games always increment move count
		if (getSciVersion() <= SCI_VERSION_01) {
			_moveCountType = kIncrementMoveCount;
		} else if (getSciVersion() >= SCI_VERSION_1_1) {
			// SCI1.1 and newer games always ignore move count
			_moveCountType = kIgnoreMoveCount;
		} else {
			if (!autoDetectMoveCountType()) {
				error("Move count autodetection failed");
				_moveCountType = kIncrementMoveCount;	// Most games do this, so best guess
			}
		}

		debugC(1, kDebugLevelVM, "Detected move count handling: %s", (_moveCountType == kIncrementMoveCount) ? "increment" : "ignore");
	}

	return _moveCountType;
}

bool GameFeatures::useAltWinGMSound() {
	if (g_sci && g_sci->getPlatform() == Common::kPlatformWindows && g_sci->isCD() && !_forceDOSTracks) {
		SciGameId id = g_sci->getGameId();
		return (id == GID_ECOQUEST ||
				id == GID_JONES ||
				id == GID_KQ5 ||
				//id == GID_FREDDYPHARKAS ||	// Has alternate tracks, but handles them differently
				id == GID_SQ4);
	} else {
		return false;
	}
}

} // End of namespace Sci
