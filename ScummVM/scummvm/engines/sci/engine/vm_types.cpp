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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/vm_types.h"
#include "sci/engine/workarounds.h"

namespace Sci {

reg_t reg_t::lookForWorkaround(const reg_t right) const {
	SciTrackOriginReply originReply;
	SciWorkaroundSolution solution = trackOriginAndFindWorkaround(0, arithmeticWorkarounds, &originReply);
	if (solution.type == WORKAROUND_NONE)
		error("Invalid arithmetic operation (params: %04x:%04x and %04x:%04x) from method %s::%s (room %d, script %d, localCall %x)",
		PRINT_REG(*this), PRINT_REG(right), originReply.objectName.c_str(),
		originReply.methodName.c_str(), g_sci->getEngineState()->currentRoomNumber(), originReply.scriptNr,
		originReply.localCallOffset);
	assert(solution.type == WORKAROUND_FAKE);
	return make_reg(0, solution.value);
}

reg_t reg_t::operator+(const reg_t right) const {
	if (isPointer() && right.isNumber()) {
		// Pointer arithmetics. Only some pointer types make sense here
		SegmentObj *mobj = g_sci->getEngineState()->_segMan->getSegmentObj(getSegment());

		if (!mobj)
			error("[VM]: Attempt to add %d to invalid pointer %04x:%04x", right.getOffset(), PRINT_REG(*this));

		switch (mobj->getType()) {
		case SEG_TYPE_LOCALS:
		case SEG_TYPE_SCRIPT:
		case SEG_TYPE_STACK:
		case SEG_TYPE_DYNMEM:
			return make_reg(getSegment(), getOffset() + right.toSint16());
		default:
			return lookForWorkaround(right);
		}
	} else if (isNumber() && right.isPointer()) {
		// Adding a pointer to a number, flip the order
		return right + *this;
	} else if (isNumber() && right.isNumber()) {
		// Normal arithmetics
		return make_reg(0, toSint16() + right.toSint16());
	} else {
		return lookForWorkaround(right);
	}
}

reg_t reg_t::operator-(const reg_t right) const {
	if (getSegment() == right.getSegment()) {
		// We can subtract numbers, or pointers with the same segment,
		// an operation which will yield a number like in C
		return make_reg(0, toSint16() - right.toSint16());
	} else {
		return *this + make_reg(right.getSegment(), -right.toSint16());
	}
}

reg_t reg_t::operator*(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toSint16() * right.toSint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator/(const reg_t right) const {
	if (isNumber() && right.isNumber() && !right.isNull())
		return make_reg(0, toSint16() / right.toSint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator%(const reg_t right) const {
	if (isNumber() && right.isNumber() && !right.isNull()) {
		// Support for negative numbers was added in Iceman, and perhaps in
		// SCI0 0.000.685 and later. Theoretically, this wasn't really used
		// in SCI0, so the result is probably unpredictable. Such a case
		// would indicate either a script bug, or a modulo on an unsigned
		// integer larger than 32767. In any case, such a case should be
		// investigated, instead of being silently accepted.
		if (getSciVersion() <= SCI_VERSION_0_LATE && (toSint16() < 0 || right.toSint16() < 0))
			warning("Modulo of a negative number has been requested for SCI0. This *could* lead to issues");
		int16 value = toSint16();
		int16 modulo = ABS(right.toSint16());
		int16 result = value % modulo;
		if (result < 0)
			result += modulo;
		return make_reg(0, result);
	} else
		return lookForWorkaround(right);
}

reg_t reg_t::operator>>(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() >> right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator<<(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() << right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator+(int16 right) const {
	return *this + make_reg(0, right);
}

reg_t reg_t::operator-(int16 right) const {
	return *this - make_reg(0, right);
}

uint16 reg_t::requireUint16() const {
	if (isNumber())
		return toUint16();
	else
		// The right parameter is NULL_REG because
		// we're not comparing *this with anything here.
		return lookForWorkaround(NULL_REG).toUint16();
}

int16 reg_t::requireSint16() const {
	if (isNumber())
		return toSint16();
	else
		// The right parameter is NULL_REG because
		// we're not comparing *this with anything here.
		return lookForWorkaround(NULL_REG).toSint16();
}

reg_t reg_t::operator&(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() & right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator|(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() | right.toUint16());
	else
		return lookForWorkaround(right);
}

reg_t reg_t::operator^(const reg_t right) const {
	if (isNumber() && right.isNumber())
		return make_reg(0, toUint16() ^ right.toUint16());
	else
		return lookForWorkaround(right);
}

int reg_t::cmp(const reg_t right, bool treatAsUnsigned) const {
	if (getSegment() == right.getSegment()) { // can compare things in the same segment
		if (treatAsUnsigned || !isNumber())
			return toUint16() - right.toUint16();
		else
			return toSint16() - right.toSint16();
	} else if (pointerComparisonWithInteger(right)) {
		return 1;
	} else if (right.pointerComparisonWithInteger(*this)) {
		return -1;
	} else
		return lookForWorkaround(right).toSint16();
}

bool reg_t::pointerComparisonWithInteger(const reg_t right) const {
	// This function handles the case where a script tries to compare a pointer
	// to a number. Normally, we would not want to allow that. However, SCI0 -
	// SCI1.1 scripts do this in order to distinguish references to
	// external resources (which are numbers) from pointers. In
	// our SCI implementation, such a check may seem pointless, as
	// one can simply use the segment value to achieve this goal.
	// But Sierra's SCI did not have the notion of segment IDs, so
	// both pointer and numbers were simple integers.
	//
	// But for some things, scripts had (and have) to distinguish between
	// numbers and pointers. Lacking the segment information, Sierra's
	// developers resorted to a hack: If an integer is smaller than a certain
	// bound, it can be assumed to be a number, otherwise it is assumed to be a
	// pointer. This allowed them to implement polymorphic functions, such as
	// the Print function, which can be called in two different ways, with a
	// pointer or a far text reference:
	//
	// (Print "foo") // Pointer to a string
	// (Print 420 5) // Reference to the fifth message in text resource 420
	// It works because in those games, the maximum resource number is 999,
	// so any parameter value above that threshold must be a pointer.
	// PQ2 japanese compares pointers to 2000 to find out if its a pointer
	// or a resource ID. Thus, we check for all integers <= 2000.
	//
	// Some examples where game scripts check for arbitrary numbers against
	// pointers:
	// Hoyle 3, Pachisi, when any opponent is about to talk
	// SQ1, room 28, when throwing water at the Orat
	// SQ1, room 58, when giving the ID card to the robot
	// SQ4 CD, at the first game screen, when the narrator is about to speak
	return (isPointer() && right.isNumber() && right.getOffset() <= 2000 && getSciVersion() <= SCI_VERSION_1_1);
}

} // End of namespace Sci
