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

#ifndef SCI_INCLUDE_ENGINE_H
#define SCI_INCLUDE_ENGINE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/serializer.h"
#include "common/str-array.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

#include "sci/sci.h"
#include "sci/engine/file.h"
#include "sci/engine/seg_manager.h"

#include "sci/parser/vocabulary.h"

#include "sci/sound/soundcmd.h"

namespace Sci {

class FileHandle;
class DirSeeker;
class EventManager;
class MessageState;
class SoundCommandParser;
class VirtualIndexFile;

enum AbortGameState {
	kAbortNone = 0,
	kAbortLoadGame = 1,
	kAbortRestartGame = 2,
	kAbortQuitGame = 3
};

// We assume that scripts give us savegameId 0->99 for creating a new save slot
//  and savegameId 100->199 for existing save slots ffs. kfile.cpp
enum {
	SAVEGAMEID_OFFICIALRANGE_START = 100,
	SAVEGAMEID_OFFICIALRANGE_END = 199
};

enum {
	GAMEISRESTARTING_NONE = 0,
	GAMEISRESTARTING_RESTART = 1,
	GAMEISRESTARTING_RESTORE = 2
};

enum VideoFlags {
	kNone            = 0,
	kDoubled         = 1 << 0,
	kDropFrames      = 1 << 1,
	kBlackLines      = 1 << 2,
	kUnkBit3         = 1 << 3,
	kGammaBoost      = 1 << 4,
	kHoldBlackFrame  = 1 << 5,
	kHoldLastFrame   = 1 << 6,
	kUnkBit7         = 1 << 7,
	kStretch         = 1 << 8
};

struct VideoState {
	Common::String fileName;
	uint16 x;
	uint16 y;
	uint16 flags;

	void reset() {
		fileName = "";
		x = y = flags = 0;
	}
};

struct EngineState : public Common::Serializable {
public:
	EngineState(SegManager *segMan);
	virtual ~EngineState();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

public:
	SegManager *_segMan; /**< The segment manager */

	/* Non-VM information */

	uint32 lastWaitTime; /**< The last time the game invoked Wait() */
	uint32 _screenUpdateTime;	/**< The last time the game updated the screen */

	void speedThrottler(uint32 neededSleep);
	void wait(int16 ticks);

	uint32 _throttleCounter; /**< total times kAnimate was invoked */
	uint32 _throttleLastTime; /**< last time kAnimate was invoked */
	bool _throttleTrigger;
	bool _gameIsBenchmarking;

	/* Kernel File IO stuff */

	Common::Array<FileHandle> _fileHandles; /**< Array of file handles. Dynamically increased if required. */

	DirSeeker _dirseeker;

	int16 _lastSaveVirtualId; // last virtual id fed to kSaveGame, if no kGetSaveFiles was called inbetween
	int16 _lastSaveNewId;    // last newly created filename-id by kSaveGame

#ifdef ENABLE_SCI32
	VirtualIndexFile *_virtualIndexFile;
#endif

	uint _chosenQfGImportItem; // Remembers the item selected in QfG import rooms

	bool _cursorWorkaroundActive; // ffs. GfxCursor::setPosition()
	Common::Point _cursorWorkaroundPoint;
	Common::Rect _cursorWorkaroundRect;

public:
	/* VM Information */

	Common::List<ExecStack> _executionStack; /**< The execution stack */
	/**
	 * When called from kernel functions, the vm is re-started recursively on
	 * the same stack. This variable contains the stack base for the current vm.
	 */
	int executionStackBase;
	bool _executionStackPosChanged;   /**< Set to true if the execution stack position should be re-evaluated by the vm */

	// Registers
	reg_t r_acc; /**< Accumulator */
	reg_t r_prev; /**< previous comparison result */
	int16 r_rest; /**< current &rest register */

	StackPtr stack_base; /**< Pointer to the least stack element */
	StackPtr stack_top; /**< First invalid stack element */

	// Script state
	ExecStack *xs;
	reg_t *variables[4];		///< global, local, temp, param, as immediate pointers
	reg_t *variablesBase[4];	///< Used for referencing VM ops
	SegmentId variablesSegment[4];	///< Same as above, contains segment IDs
	int variablesMax[4];		///< Max. values for all variables

	AbortGameState abortScriptProcessing;
	int16 gameIsRestarting; // is set when restarting (=1) or restoring the game (=2)

	int scriptStepCounter; // Counts the number of steps executed
	int scriptGCInterval; // Number of steps in between gcs

	uint16 currentRoomNumber() const;
	void setRoomNumber(uint16 roomNumber);

	/**
	 * Sets global variables from script 0
	 */
	void initGlobals();

	/**
	 * Shrink execution stack to size.
	 * Contains an assert if it is not already smaller.
	 */
	void shrinkStackToBase();

	int gcCountDown; /**< Number of kernel calls until next gc */

	MessageState *_msgState;

	// MemorySegment provides access to a 256-byte block of memory that remains
	// intact across restarts and restores
	enum {
		kMemorySegmentMax = 256
	};
	uint16 _memorySegmentSize;
	byte _memorySegment[kMemorySegmentMax];

	VideoState _videoState;
	uint16 _vmdPalStart, _vmdPalEnd;
	bool _syncedAudioOptions;

	uint16 _palCycleToColor;

	/**
	 * Resets the engine state.
	 */
	void reset(bool isRestoring);
};

} // End of namespace Sci

#endif // SCI_INCLUDE_ENGINE_H
