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

#ifndef SCI_INCLUDE_FEATURES_H
#define SCI_INCLUDE_FEATURES_H

#include "sci/resource.h"
#include "sci/engine/seg_manager.h"

namespace Sci {

enum MoveCountType {
	kMoveCountUninitialized,
	kIgnoreMoveCount,
	kIncrementMoveCount
};

enum Sci2StringFunctionType {
	kSci2StringFunctionUninitialized,
	kSci2StringFunctionOld,
	kSci2StringFunctionNew
};

class GameFeatures {
public:
	GameFeatures(SegManager *segMan, Kernel *kernel);
	~GameFeatures() {}

	/**
	 * Autodetects the DoSound type
	 * @return DoSound type, SCI_VERSION_0_EARLY / SCI_VERSION_0_LATE /
	 *                       SCI_VERSION_1_EARLY / SCI_VERSION_1_LATE
	 */
	SciVersion detectDoSoundType();

	/**
	 * Autodetects the SetCursor type
	 * @return SetCursor type, SCI_VERSION_0_EARLY / SCI_VERSION_1_1
	 */
	SciVersion detectSetCursorType();

	/**
	 * Autodetects the Lofs type
	 * @return Lofs type, SCI_VERSION_0_EARLY / SCI_VERSION_1_MIDDLE / SCI_VERSION_1_1 / SCI_VERSION_3
	 */
	SciVersion detectLofsType();

	/**
	 * Autodetects the graphics functions used
	 * @return Graphics functions type, SCI_VERSION_0_EARLY / SCI_VERSION_0_LATE
	 */
	SciVersion detectGfxFunctionsType();

	/**
	 * Autodetects the message function used
	 * @return Message function type, SCI_VERSION_1_LATE / SCI_VERSION_1_1
	 */
	SciVersion detectMessageFunctionType();

#ifdef ENABLE_SCI32
	/**
	 * Autodetects the kernel functions used in SCI2.1
	 * @return Graphics functions type, SCI_VERSION_2 / SCI_VERSION_2_1
	 */
	SciVersion detectSci21KernelType();

	/**
	 * Autodetects the string subfunctions used in SCI2 - SCI3
	 * @return string subfunctions type, kSci2StringFunctionOld / kSci2StringFunctionNew
	 */
	Sci2StringFunctionType detectSci2StringFunctionType();

#endif

	/**
	 * Applies to all versions before 0.000.502
	 * Old SCI versions used to interpret the third DrawPic() parameter inversely,
	 * with the opposite default value (obviously).
	 * Also, they used 15 priority zones from 42 to 200 instead of 14 priority
	 * zones from 42 to 190.
	 */
	bool usesOldGfxFunctions() { return detectGfxFunctionsType() == SCI_VERSION_0_EARLY; }

	/**
	 * Autodetects the Bresenham routine used in the actor movement functions
	 * @return Move count type, kIncrementMoveCnt / kIgnoreMoveCnt
	 */
	MoveCountType detectMoveCountType();

	bool handleMoveCount() { return detectMoveCountType() == kIncrementMoveCount; }

	bool usesCdTrack() { return _usesCdTrack; }

	/**
	 * Checks if the alternative Windows GM MIDI soundtrack should be used. Such
	 * soundtracks are available for the Windows CD versions of EcoQuest, Jones,
	 * KQ5 and SQ4.
	 */
	bool useAltWinGMSound();

	/**
	 * Forces DOS soundtracks in Windows CD versions when the user hasn't
	 * selected a MIDI output device
	 */
	void forceDOSTracks() { _forceDOSTracks = true; }

private:
	reg_t getDetectionAddr(const Common::String &objName, Selector slc, int methodNum = -1);

	bool autoDetectLofsType(Common::String gameSuperClassName, int methodNum);
	bool autoDetectGfxFunctionsType(int methodNum = -1);
	bool autoDetectSoundType();
	bool autoDetectMoveCountType();
#ifdef ENABLE_SCI32
	bool autoDetectSci21KernelType();
	bool autoDetectSci21StringFunctionType();
#endif

	SciVersion _doSoundType, _setCursorType, _lofsType, _gfxFunctionsType, _messageFunctionType;
#ifdef ENABLE_SCI32
	SciVersion _sci21KernelType;
	Sci2StringFunctionType _sci2StringFunctionType;
#endif

	MoveCountType _moveCountType;
	bool _usesCdTrack;
	bool _forceDOSTracks;

	SegManager *_segMan;
	Kernel *_kernel;
};

} // End of namespace Sci

#endif // SCI_INCLUDE_ENGINE_H
