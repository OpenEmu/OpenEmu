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

#ifndef SCI_ENGINE_SELECTOR_H
#define SCI_ENGINE_SELECTOR_H

#include "common/scummsys.h"

#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/engine/vm.h"

namespace Sci {

/** Contains selector IDs for a few selected selectors */
struct SelectorCache {
	SelectorCache() {
		memset(this, 0, sizeof(*this));
	}

	// Statically defined selectors, (almost the) same in all SCI versions
	Selector _info_;	///< Removed in SCI3
	Selector y;
	Selector x;
	Selector view, loop, cel; ///< Description of a specific image
	Selector underBits; ///< Used by the graphics subroutines to store backupped BG pic data
	Selector nsTop, nsLeft, nsBottom, nsRight; ///< View boundaries ('now seen')
	Selector lsTop, lsLeft, lsBottom, lsRight; ///< Used by Animate() subfunctions and scroll list controls
	Selector signal; ///< Used by Animate() to control a view's behavior
	Selector illegalBits; ///< Used by CanBeHere
	Selector brTop, brLeft, brBottom, brRight; ///< Bounding Rectangle
	// name, key, time
	Selector text; ///< Used by controls
	Selector elements; ///< Used by SetSynonyms()
	// color, back
	Selector mode; ///< Used by text controls (-> DrawControl())
	// style
	Selector state, font, type;///< Used by controls
	// window
	Selector cursor; ///< Used by EditControl
	Selector max; ///< Used by EditControl, removed in SCI3
	Selector mark; //< Used by list controls (script internal, is needed by us for the QfG import rooms)
	Selector sort; //< Used by list controls (script internal, is needed by us for QfG3 import room)
	// who
	Selector message; ///< Used by GetEvent
	// edit
	Selector play; ///< Play function (first function to be called)
	Selector number;
	Selector handle;	///< Replaced by nodePtr in SCI1+
	Selector nodePtr;	///< Replaces handle in SCI1+
	Selector client; ///< The object that wants to be moved
	Selector dx, dy; ///< Deltas
	Selector b_movCnt, b_i1, b_i2, b_di, b_xAxis, b_incr; ///< Various Bresenham vars
	Selector xStep, yStep; ///< BR adjustments
	Selector xLast, yLast; ///< BR last position of client
	Selector moveSpeed; ///< Used for DoBresen
	Selector canBeHere; ///< Funcselector: Checks for movement validity in SCI0
	Selector heading, mover; ///< Used in DoAvoider
	Selector doit; ///< Called (!) by the Animate() system call
	Selector isBlocked, looper;	///< Used in DoAvoider
	Selector priority;
	Selector modifiers; ///< Used by GetEvent
	Selector replay; ///< Replay function
	// setPri, at, next, done, width
	Selector wordFail, syntaxFail; ///< Used by Parse()
	// semanticFail, pragmaFail
	// said
	Selector claimed; ///< Used generally by the event mechanism
	// value, save, restore, title, button, icon, draw
	Selector delete_; ///< Called by Animate() to dispose a view object
	Selector z;

	// SCI1+ static selectors
	Selector parseLang;
	Selector printLang; ///< Used for i18n
	Selector subtitleLang;
	Selector size;
	Selector points; ///< Used by AvoidPath()
	Selector palette;	///< Used by the SCI0-SCI1.1 animate code, unused in SCI2-SCI2.1, removed in SCI3
	Selector dataInc;	///< Used to sync music with animations, removed in SCI3
	// handle (in SCI1)
	Selector min; ///< SMPTE time format
	Selector sec;
	Selector frame;
	Selector vol;
	Selector pri;
	// perform
	Selector moveDone;	///< used for DoBresen

	// SCI1 selectors which have been moved a bit in SCI1.1, but otherwise static
	Selector cantBeHere; ///< Checks for movement avoidance in SCI1+. Replaces canBeHere
	Selector topString; ///< SCI1 scroll lists use this instead of lsTop. Removed in SCI3
	Selector flags;

	// SCI1+ audio sync related selectors, not static. They're used for lip syncing in
	// CD talkie games
	Selector syncCue; ///< Used by DoSync()
	Selector syncTime;

	// SCI1.1 specific selectors
	Selector scaleSignal; //< Used by kAnimate() for cel scaling (SCI1.1+)
	Selector scaleX, scaleY;	///< SCI1.1 view scaling
	Selector maxScale;		///< SCI1.1 view scaling, limit for cel, when using global scaling
	Selector vanishingX;	///< SCI1.1 view scaling, used by global scaling
	Selector vanishingY;	///< SCI1.1 view scaling, used by global scaling

	// Used for auto detection purposes
	Selector overlay;	///< Used to determine if a game is using old gfx functions or not

	// SCI1.1 Mac icon bar selectors
	Selector iconIndex; ///< Used to index icon bar objects
	Selector select;

#ifdef ENABLE_SCI32
	Selector data; // Used by Array()/String()
	Selector picture; // Used to hold the picture ID for SCI32 pictures
	Selector bitmap; // Used to hold the text bitmap for SCI32 texts

	Selector plane;
	Selector top;
	Selector left;
	Selector bottom;
	Selector right;
	Selector resX;
	Selector resY;

	Selector fore;
	Selector back;
	Selector skip;
	Selector dimmed;

	Selector fixPriority;
	Selector mirrored;
	Selector visible;

	Selector useInsetRect;
	Selector inTop, inLeft, inBottom, inRight;
#endif
};

/**
 * Map a selector name to a selector id. Shortcut for accessing the selector cache.
 */
#define SELECTOR(_slc_)		(g_sci->getKernel()->_selectorCache._slc_)

/**
 * Retrieves a selector from an object.
 * @param segMan	the segment mananger
 * @param _obj_		the address of the object which the selector should be read from
 * @param _slc_		the selector to refad
 * @return			the selector value as a reg_t
 * This macro halts on error. 'selector' must be a selector name registered in vm.h's
 * SelectorCache and mapped in script.cpp.
 */
reg_t readSelector(SegManager *segMan, reg_t object, Selector selectorId);
#define readSelectorValue(segMan, _obj_, _slc_) (readSelector(segMan, _obj_, _slc_).getOffset())

/**
 * Writes a selector value to an object.
 * @param segMan	the segment mananger
 * @param _obj_		the address of the object which the selector should be written to
 * @param _slc_		the selector to read
 * @param _val_		the value to write
 * This macro halts on error. 'selector' must be a selector name registered in vm.h's
 * SelectorCache and mapped in script.cpp.
 */
void writeSelector(SegManager *segMan, reg_t object, Selector selectorId, reg_t value);
#define writeSelectorValue(segMan, _obj_, _slc_, _val_) writeSelector(segMan, _obj_, _slc_, make_reg(0, _val_))

/**
 * Invokes a selector from an object.
 */
void invokeSelector(EngineState *s, reg_t object, int selectorId,
	int k_argc, StackPtr k_argp, int argc = 0, const reg_t *argv = 0);

} // End of namespace Sci

#endif // SCI_ENGINE_KERNEL_H
