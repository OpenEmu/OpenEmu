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

#include "common/system.h"

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/gc.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/maciconbar.h"
#include "sci/console.h"

namespace Sci {

reg_t kRestartGame(EngineState *s, int argc, reg_t *argv) {
	s->shrinkStackToBase();

	s->abortScriptProcessing = kAbortRestartGame; // Force vm to abort ASAP
	return NULL_REG;
}

/* kGameIsRestarting():
** Returns the restarting_flag in acc
*/
reg_t kGameIsRestarting(EngineState *s, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, s->gameIsRestarting);

	if (argc) { // Only happens during replay
		if (!argv[0].toUint16()) // Set restarting flag
			s->gameIsRestarting = GAMEISRESTARTING_NONE;
	}

	uint32 neededSleep = 30;

	// WORKAROUNDS for scripts that are polling too quickly in scenes that
	// are not animating much
	switch (g_sci->getGameId()) {
	case GID_CASTLEBRAIN:
		// In Castle of Dr. Brain, memory color matching puzzle in the first
		// room (room 100), the game scripts constantly poll the state of each
		// stone when the user clicks on one. Since the scene is not animating
		// much, this results in activating and deactivating each stone very
		// quickly (together with its associated tone sound), depending on how
		// low it is in the animate list. This worked somewhat in older PCs, but
		// not in modern computers. We throttle the scene in order to allow the
		// stones to display, otherwise the game scripts reset them too soon.
		// Fixes bug #3127824.
		if (s->currentRoomNumber() == 100) {
			s->_throttleTrigger = true;
			neededSleep = 60;
		}
		break;
	case GID_ICEMAN:
		// In ICEMAN the submarine control room is not animating much, so it
		// runs way too fast. We calm it down even more, otherwise fighting
		// against other submarines is almost impossible.
		if (s->currentRoomNumber() == 27) {
			s->_throttleTrigger = true;
			neededSleep = 60;
		}
		break;
	case GID_LSL3:
		// LSL3 calculates a machinespeed variable during game startup
		// (right after the filthy questions). This one would go through w/o
		// throttling resulting in having to do 1000 pushups or something. Another
		// way of handling this would be delaying incrementing of "machineSpeed"
		// selector.
		if (s->currentRoomNumber() == 290)
			s->_throttleTrigger = true;
		break;
	case GID_SQ4:
		// In SQ4 (floppy and CD) the sequel police appear way too quickly in
		// the Skate-o-rama rooms, resulting in all sorts of timer issues, like
		// #3109139 (which occurs because a police officer instantly teleports
		// just before Roger exits and shoots him). We throttle these scenes a
		// bit more, in order to prevent timer bugs related to the sequel police.
		if (s->currentRoomNumber() == 405 || s->currentRoomNumber() == 406 ||
			s->currentRoomNumber() == 410 || s->currentRoomNumber() == 411) {
			s->_throttleTrigger = true;
			neededSleep = 60;
		}
	default:
		break;
	}

	s->speedThrottler(neededSleep);
	return s->r_acc;
}

reg_t kHaveMouse(EngineState *s, int argc, reg_t *argv) {
	return SIGNAL_REG;
}

enum kMemoryInfoFunc {
	K_MEMORYINFO_LARGEST_HEAP_BLOCK = 0, // Largest heap block available
	K_MEMORYINFO_FREE_HEAP = 1, // Total free heap memory
	K_MEMORYINFO_LARGEST_HUNK_BLOCK = 2, // Largest available hunk memory block
	K_MEMORYINFO_FREE_HUNK = 3, // Amount of free DOS paragraphs
	K_MEMORYINFO_TOTAL_HUNK = 4 // Total amount of hunk memory (SCI01)
};

reg_t kMemoryInfo(EngineState *s, int argc, reg_t *argv) {
	// The free heap size returned must not be 0xffff, or some memory
	// calculations will overflow. Crazy Nick's games handle up to 32746
	// bytes (0x7fea), otherwise they throw a warning that the memory is
	// fragmented
	const uint16 size = 0x7fea;

	switch (argv[0].getOffset()) {
	case K_MEMORYINFO_LARGEST_HEAP_BLOCK:
		// In order to prevent "Memory fragmented" dialogs from
		// popping up in some games, we must return FREE_HEAP - 2 here.
		return make_reg(0, size - 2);
	case K_MEMORYINFO_FREE_HEAP:
	case K_MEMORYINFO_LARGEST_HUNK_BLOCK:
	case K_MEMORYINFO_FREE_HUNK:
	case K_MEMORYINFO_TOTAL_HUNK:
		return make_reg(0, size);

	default:
		error("Unknown MemoryInfo operation: %04x", argv[0].getOffset());
	}

	return NULL_REG;
}

enum kMemorySegmentFunc {
	K_MEMORYSEGMENT_SAVE_DATA = 0,
	K_MEMORYSEGMENT_RESTORE_DATA = 1
};

reg_t kMemorySegment(EngineState *s, int argc, reg_t *argv) {
	// MemorySegment provides access to a 256-byte block of memory that remains
	// intact across restarts and restores

	switch (argv[0].toUint16()) {
	case K_MEMORYSEGMENT_SAVE_DATA: {
		if (argc < 3)
			error("Insufficient number of arguments passed to MemorySegment");
		uint16 size = argv[2].toUint16();

		if (!size)
			size = s->_segMan->strlen(argv[1]) + 1;

		if (size > EngineState::kMemorySegmentMax) {
			// This was set to cut the block to 256 bytes. This should be an
			// error, as we won't restore the full block that the game scripts
			// request, thus error out instead.
			//size = EngineState::kMemorySegmentMax;
			error("kMemorySegment: Requested to save more than 256 bytes (%d)", size);
		}

		s->_memorySegmentSize = size;

		// We assume that this won't be called on pointers
		s->_segMan->memcpy(s->_memorySegment, argv[1], size);
		break;
	}
	case K_MEMORYSEGMENT_RESTORE_DATA:
		s->_segMan->memcpy(argv[1], s->_memorySegment, s->_memorySegmentSize);
		break;
	default:
		error("Unknown MemorySegment operation %04x", argv[0].toUint16());
	}

	return argv[1];
}

reg_t kFlushResources(EngineState *s, int argc, reg_t *argv) {
	run_gc(s);
	debugC(kDebugLevelRoom, "Entering room number %d", argv[0].toUint16());
	return s->r_acc;
}

reg_t kSetDebug(EngineState *s, int argc, reg_t *argv) {
	// WORKAROUND: For some reason, GK1 calls this unconditionally when
	// watching the intro. Older (SCI0) games call it on room change if
	// a flag is set, in which case the debugger SHOULD get activated.
	// Therefore, don't break into the debugger in GK1, but do so elsewhere.

	if (g_sci->getGameId() != GID_GK1) {
		debug("Debug mode activated");

		g_sci->getDebugger()->attach();
	}

	return s->r_acc;
}

enum {
	KGETTIME_TICKS = 0,
	KGETTIME_TIME_12HOUR = 1,
	KGETTIME_TIME_24HOUR = 2,
	KGETTIME_DATE = 3
};

reg_t kGetTime(EngineState *s, int argc, reg_t *argv) {
	TimeDate loc_time;
	uint32 elapsedTime = g_engine->getTotalPlayTime();
	int retval = 0; // Avoid spurious warning

	g_system->getTimeAndDate(loc_time);

	int mode = (argc > 0) ? argv[0].toUint16() : 0;

	// Modes 2 and 3 are supported since 0.629.
	// This condition doesn't check that exactly, but close enough.
	if (getSciVersion() == SCI_VERSION_0_EARLY && mode > 1)
		error("kGetTime called in SCI0 with mode %d (expected 0 or 1)", mode);

	switch (mode) {
	case KGETTIME_TICKS :
		retval = elapsedTime * 60 / 1000;
		debugC(kDebugLevelTime, "GetTime(elapsed) returns %d", retval);
		break;
	case KGETTIME_TIME_12HOUR :
		retval = ((loc_time.tm_hour % 12) << 12) | (loc_time.tm_min << 6) | (loc_time.tm_sec);
		debugC(kDebugLevelTime, "GetTime(12h) returns %d", retval);
		break;
	case KGETTIME_TIME_24HOUR :
		retval = (loc_time.tm_hour << 11) | (loc_time.tm_min << 5) | (loc_time.tm_sec >> 1);
		debugC(kDebugLevelTime, "GetTime(24h) returns %d", retval);
		break;
	case KGETTIME_DATE :
		retval = loc_time.tm_mday | ((loc_time.tm_mon + 1) << 5) | (((loc_time.tm_year + 1900) & 0x7f) << 9);
		debugC(kDebugLevelTime, "GetTime(date) returns %d", retval);
		break;
	default:
		error("Attempt to use unknown GetTime mode %d", mode);
		break;
	}

	return make_reg(0, retval);
}

enum {
	K_MEMORY_ALLOCATE_CRITICAL		= 1,
	K_MEMORY_ALLOCATE_NONCRITICAL   = 2,
	K_MEMORY_FREE					= 3,
	K_MEMORY_MEMCPY					= 4,
	K_MEMORY_PEEK					= 5,
	K_MEMORY_POKE					= 6
};

reg_t kMemory(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case K_MEMORY_ALLOCATE_CRITICAL: {
		int byteCount = argv[1].toUint16();
		// WORKAROUND:
		//  - pq3 (multilingual) room 202
		//     when plotting crimes, allocates the returned bytes from kStrLen
		//     on "W" and "E" and wants to put a string in there, which doesn't
		//     fit of course.
		//  - lsl5 (multilingual) room 280
		//     allocates memory according to a previous kStrLen for the name of
		//     the airport ladies (bug #3093818), which isn't enough

		// We always allocate 1 byte more, because of this
		byteCount++;

		if (!s->_segMan->allocDynmem(byteCount, "kMemory() critical", &s->r_acc)) {
			error("Critical heap allocation failed");
		}
		break;
	}
	case K_MEMORY_ALLOCATE_NONCRITICAL:
		s->_segMan->allocDynmem(argv[1].toUint16(), "kMemory() non-critical", &s->r_acc);
		break;
	case K_MEMORY_FREE :
		if (!s->_segMan->freeDynmem(argv[1])) {
			if (g_sci->getGameId() == GID_QFG1VGA) {
				// Ignore script bug in QFG1VGA, when closing any conversation dialog with esc
			} else {
				// Usually, the result of a script bug. Non-critical
				warning("Attempt to kMemory::free() non-dynmem pointer %04x:%04x", PRINT_REG(argv[1]));
			}
		}
		break;
	case K_MEMORY_MEMCPY : {
		int size = argv[3].toUint16();
		s->_segMan->memcpy(argv[1], argv[2], size);
		break;
	}
	case K_MEMORY_PEEK : {
		if (!argv[1].getSegment()) {
			// This occurs in KQ5CD when interacting with certain objects
			warning("Attempt to peek invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}

		SegmentRef ref = s->_segMan->dereference(argv[1]);

		if (!ref.isValid() || ref.maxSize < 2) {
			error("Attempt to peek invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}
		if (ref.isRaw)
			return make_reg(0, (int16)READ_SCIENDIAN_UINT16(ref.raw));
		else {
			if (ref.skipByte)
				error("Attempt to peek memory at odd offset %04X:%04X", PRINT_REG(argv[1]));
			return *(ref.reg);
		}
		break;
	}
	case K_MEMORY_POKE : {
		SegmentRef ref = s->_segMan->dereference(argv[1]);

		if (!ref.isValid() || ref.maxSize < 2) {
			error("Attempt to poke invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}

		if (ref.isRaw) {
			if (argv[2].getSegment()) {
				error("Attempt to poke memory reference %04x:%04x to %04x:%04x", PRINT_REG(argv[2]), PRINT_REG(argv[1]));
				return s->r_acc;
			}
			WRITE_SCIENDIAN_UINT16(ref.raw, argv[2].getOffset());		// Amiga versions are BE
		} else {
			if (ref.skipByte)
				error("Attempt to poke memory at odd offset %04X:%04X", PRINT_REG(argv[1]));
			*(ref.reg) = argv[2];
		}
		break;
	}
	}

	return s->r_acc;
}

#ifdef ENABLE_SCI32
reg_t kGetConfig(EngineState *s, int argc, reg_t *argv) {
	Common::String setting = s->_segMan->getString(argv[0]);
	reg_t data = readSelector(s->_segMan, argv[1], SELECTOR(data));

	// This function is used to get the benchmarked results stored in the
	// resource.cfg configuration file in Phantasmagoria 1. Normally,
	// the configuration file contains values stored by the installer
	// regarding audio and video settings, which are then used by the
	// executable. In Phantasmagoria, two extra executable files are used
	// to perform system benchmarks:
	// - CPUID for the CPU benchmarks, sets the cpu and cpuspeed settings
	// - HDDTEC for the graphics and CD-ROM benchmarks, sets the videospeed setting
	//
	// These settings are then used by the game scripts directly to modify
	// the game speed and graphics output. The result of this call is stored
	// in global 178. The scripts check these values against the value 425.
	// Anything below that makes Phantasmagoria awfully sluggish, so we're
	// setting everything to 500, which makes the game playable.

	setting.toLowercase();

	if (setting == "videospeed") {
		s->_segMan->strcpy(data, "500");
	} else if (setting == "cpu") {
		// We always return the fastest CPU setting that CPUID can detect
		// (i.e. 586).
		s->_segMan->strcpy(data, "586");
	} else if (setting == "cpuspeed") {
		s->_segMan->strcpy(data, "500");
	} else if (setting == "language") {
		Common::String languageId = Common::String::format("%d", g_sci->getSciLanguage());
		s->_segMan->strcpy(data, languageId.c_str());
	} else if (setting == "torindebug") {
		// Used to enable the debug mode in Torin's Passage (French).
		// If true, the debug mode is enabled.
		s->_segMan->strcpy(data, "");
	} else if (setting == "leakdump") {
		// An unknown setting in LSL7. Likely used for debugging.
		s->_segMan->strcpy(data, "");
	} else if (setting == "startroom") {
		// Debug setting in LSL7, specifies the room to start from.
		s->_segMan->strcpy(data, "");
	} else {
		error("GetConfig: Unknown configuration setting %s", setting.c_str());
	}

	return argv[1];
}

// Likely modelled after the Windows 3.1 function GetPrivateProfileInt:
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724345%28v=vs.85%29.aspx
reg_t kGetSierraProfileInt(EngineState *s, int argc, reg_t *argv) {
	Common::String category = s->_segMan->getString(argv[0]);	// always "config"
	category.toLowercase();
	Common::String setting = s->_segMan->getString(argv[1]);
	setting.toLowercase();
	// The third parameter is the default value returned if the configuration key is missing

	if (category == "config" && setting == "videospeed") {
		// We return the same fake value for videospeed as with kGetConfig
		return make_reg(0, 500);
	}

	warning("kGetSierraProfileInt: Returning default value %d for unknown setting %s.%s", argv[2].toSint16(), category.c_str(), setting.c_str());
	return argv[2];
}

reg_t kGetWindowsOption(EngineState *s, int argc, reg_t *argv) {
	uint16 windowsOption = argv[0].toUint16();
	switch (windowsOption) {
	case 0:
		// Title bar on/off in Phantasmagoria, we return 0 (off)
		return NULL_REG;
	default:
		warning("GetWindowsOption: Unknown option %d", windowsOption);
		return NULL_REG;
	}
}

#endif

// kIconBar is really a subop of kMacPlatform for SCI1.1 Mac
reg_t kIconBar(EngineState *s, int argc, reg_t *argv) {
	// Mac versions use their own tertiary platform functions
	// to handle the outside-of-the-screen icon bar.

	// QFG1 Mac calls this function to load the Mac icon bar (of which
	// the resources do exist), but the game completely ignores it and
	// uses the standard icon bar for the game. We do the same.
	if (!g_sci->hasMacIconBar())
		return NULL_REG;

	switch (argv[0].toUint16()) {
	case 0: // InitIconBar
		for (int i = 0; i < argv[1].toUint16(); i++)
			g_sci->_gfxMacIconBar->addIcon(argv[i + 2]);
		break;
	case 1: // DisposeIconBar
		warning("kIconBar(Dispose)");
		break;
	case 2: // EnableIconBar (-1 = all)
		debug(0, "kIconBar(Enable, %i)", argv[1].toSint16());
		g_sci->_gfxMacIconBar->setIconEnabled(argv[1].toSint16(), true);
		break;
	case 3: // DisableIconBar (-1 = all)
		debug(0, "kIconBar(Disable, %i)", argv[1].toSint16());
		g_sci->_gfxMacIconBar->setIconEnabled(argv[1].toSint16(), false);
		break;
	case 4: // SetIconBarIcon
		debug(0, "kIconBar(SetIcon, %d, %d)", argv[1].toUint16(), argv[2].toUint16());
		if (argv[2].toSint16() == -1)
			g_sci->_gfxMacIconBar->setInventoryIcon(argv[2].toSint16());
		break;
	default:
		error("Unknown kIconBar(%d)", argv[0].toUint16());
	}

	g_sci->_gfxMacIconBar->drawIcons();

	return NULL_REG;
}

// kMacPlatform is really a subop of kPlatform for SCI1.1+ Mac
reg_t kMacPlatform(EngineState *s, int argc, reg_t *argv) {
	// Mac versions use their own secondary platform functions
	// to do various things. Why didn't they just declare a new
	// kernel function?

	switch (argv[0].toUint16()) {
	case 0:
		// Subop 0 has changed a few times
		// In SCI1, its usage is still unknown
		// In SCI1.1, it's NOP
		// In SCI32, it's used for remapping cursor ID's
		if (getSciVersion() >= SCI_VERSION_2_1) // Set Mac cursor remap
			g_sci->_gfxCursor->setMacCursorRemapList(argc - 1, argv + 1);
		else if (getSciVersion() != SCI_VERSION_1_1)
			warning("Unknown SCI1 kMacPlatform(0) call");
		break;
	case 4: // Handle icon bar code
		return kIconBar(s, argc - 1, argv + 1);
	case 7: // Unknown, but always return -1
		return SIGNAL_REG;
	case 1:	// Unknown, calls QuickDraw region functions (KQ5, QFG1VGA, Dr. Brain 1)
		break;	// removed warning, as it produces a lot of spam in the console
	case 2: // Unknown, "UseNextWaitEvent" (Various)
	case 3: // Unknown, "ProcessOpenDocuments" (Various)
	case 5: // Unknown, plays a sound (KQ7)
	case 6: // Unknown, menu-related (Unused?)
		warning("Unhandled kMacPlatform(%d)", argv[0].toUint16());
		break;
	default:
		error("Unknown kMacPlatform(%d)", argv[0].toUint16());
	}

	return s->r_acc;
}

enum kSciPlatforms {
	kSciPlatformDOS = 1,
	kSciPlatformWindows = 2
};

enum kPlatformOps {
	kPlatformUnk0 = 0,
	kPlatformCDSpeed = 1,
	kPlatformUnk2 = 2,
	kPlatformCDCheck = 3,
	kPlatformGetPlatform = 4,
	kPlatformUnk5 = 5,
	kPlatformIsHiRes = 6,
	kPlatformIsItWindows = 7
};

reg_t kPlatform(EngineState *s, int argc, reg_t *argv) {
	bool isWindows = g_sci->getPlatform() == Common::kPlatformWindows;

	if (argc == 0 && getSciVersion() < SCI_VERSION_2) {
		// This is called in KQ5CD with no parameters, where it seems to do some
		// graphics driver check. This kernel function didn't have subfunctions
		// then. If 0 is returned, the game functions normally, otherwise all
		// the animations show up like a slideshow (e.g. in the intro). So we
		// return 0. However, the behavior changed for kPlatform with no
		// parameters in SCI32.
		return NULL_REG;
	}

	uint16 operation = (argc == 0) ? 0 : argv[0].toUint16();

	switch (operation) {
	case kPlatformCDSpeed:
		// TODO: Returns CD Speed?
		warning("STUB: kPlatform(CDSpeed)");
		break;
	case kPlatformUnk2:
		// Always returns 2
		return make_reg(0, 2);
	case kPlatformCDCheck:
		// TODO: Some sort of CD check?
		warning("STUB: kPlatform(CDCheck)");
		break;
	case kPlatformUnk0:
		// For Mac versions, kPlatform(0) with other args has more functionality
		if (g_sci->getPlatform() == Common::kPlatformMacintosh && argc > 1)
			return kMacPlatform(s, argc - 1, argv + 1);
		// Otherwise, fall through
	case kPlatformGetPlatform:
		return make_reg(0, (isWindows) ? kSciPlatformWindows : kSciPlatformDOS);
	case kPlatformUnk5:
		// This case needs to return the opposite of case 6 to get hires graphics
		return make_reg(0, !isWindows);
	case kPlatformIsHiRes:
		return make_reg(0, isWindows);
	case kPlatformIsItWindows:
		return make_reg(0, isWindows);
	default:
		error("Unsupported kPlatform operation %d", operation);
	}

	return NULL_REG;
}

reg_t kEmpty(EngineState *s, int argc, reg_t *argv) {
	// Placeholder for empty kernel functions which are still called from the
	// engine scripts (like the empty kSetSynonyms function in SCI1.1). This
	// differs from dummy functions because it does nothing and never throws a
	// warning when it is called.
	return s->r_acc;
}

reg_t kStub(EngineState *s, int argc, reg_t *argv) {
	Kernel *kernel = g_sci->getKernel();
	int kernelCallNr = -1;

	Common::List<ExecStack>::const_iterator callIterator = s->_executionStack.end();
	if (callIterator != s->_executionStack.begin()) {
		callIterator--;
		ExecStack lastCall = *callIterator;
		kernelCallNr = lastCall.debugSelector;
	}

	Common::String warningMsg = "Dummy function k" + kernel->getKernelName(kernelCallNr) +
								Common::String::format("[%x]", kernelCallNr) +
								" invoked. Params: " +
								Common::String::format("%d", argc) + " (";

	for (int i = 0; i < argc; i++) {
		warningMsg +=  Common::String::format("%04x:%04x", PRINT_REG(argv[i]));
		warningMsg += (i == argc - 1 ? ")" : ", ");
	}

	warning("%s", warningMsg.c_str());
	return s->r_acc;
}

reg_t kStubNull(EngineState *s, int argc, reg_t *argv) {
	kStub(s, argc, argv);
	return NULL_REG;
}

reg_t kDummy(EngineState *s, int argc, reg_t *argv) {
	kStub(s, argc, argv);
	error("Kernel function was called, which was considered to be unused - see log for details");
}

} // End of namespace Sci
