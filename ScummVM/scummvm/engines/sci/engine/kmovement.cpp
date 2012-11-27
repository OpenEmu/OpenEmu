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
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/screen.h"

namespace Sci {

/**
 * Compute "velocity" vector (xStep,yStep)=(vx,vy) for a jump from (0,0) to
 * (dx,dy), with gravity constant gy. The gravity is assumed to be non-negative.
 *
 * If this was ordinary continuous physics, we would compute the desired
 * (floating point!) velocity vector (vx,vy) as follows, under the assumption
 * that vx and vy are linearly correlated by a constant c, i.e., vy = c * vx:
 *    dx = t * vx
 *    dy = t * vy + gy * t^2 / 2
 * => dy = c * dx + gy * (dx/vx)^2 / 2
 * => |vx| = sqrt( gy * dx^2 / (2 * (dy - c * dx)) )
 * Here, the sign of vx must be chosen equal to the sign of dx, obviously.
 *
 * This square root only makes sense in our context if the denominator is
 * positive, or equivalently, (dy - c * dx) must be positive. For simplicity
 * and by symmetry along the x-axis, we assume dx to be positive for all
 * computations, and only adjust for its sign in the end. Switching the sign of
 * c appropriately, we set tmp := (dy + c * dx) and compute c so that this term
 * becomes positive.
 *
 * Remark #1: If the jump is straight up, i.e. dx == 0, then we should not
 * assume the above linear correlation vy = c * vx of the velocities (as vx
 * will be 0, but vy shouldn't be, unless we drop down).
 *
 * Remark #2: We are actually in a discrete setup. The motion is computed
 * iteratively: each iteration, we add vx and vy to the position, then add gy
 * to vy. So the real formula is the following (where t ideally is close to an int):
 *
 *   dx = t * vx
 *   dy = t * vy + gy * t*(t-1) / 2
 *
 * But the solution resulting from that is a lot more complicated, so we use
 * the above approximation instead.
 *
 * Still, what we compute in the end is of course not a real velocity anymore,
 * but an integer approximation, used in an iterative stepping algorithm.
 */
reg_t kSetJump(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	// Input data
	reg_t object = argv[0];
	int dx = argv[1].toSint16();
	int dy = argv[2].toSint16();
	int gy = argv[3].toSint16();

	// Derived data
	int c;
	int tmp;
	int vx = 0;  // x velocity
	int vy = 0;  // y velocity

	int dxWasNegative = (dx < 0);
	dx = ABS(dx);

	assert(gy >= 0);

	if (dx == 0) {
		// Upward jump. Value of c doesn't really matter
		c = 1;
	} else {
		// Compute a suitable value for c respectively tmp.
		// The important thing to consider here is that we want the resulting
		// *discrete* x/y velocities to be not-too-big integers, for a smooth
		// curve (i.e. we could just set vx=dx, vy=dy, and be done, but that
		// is hardly what you would call a parabolic jump, would ya? ;-).
		//
		// So, we make sure that 2.0*tmp will be bigger than dx (that way,
		// we ensure vx will be less than sqrt(gy * dx)).
		if (dx + dy < 0) {
			// dy is negative and |dy| > |dx|
			c = (2 * ABS(dy)) / dx;
			//tmp = ABS(dy);  // ALMOST the resulting value, except for obvious rounding issues
		} else {
			// dy is either positive, or |dy| <= |dx|
			c = (dx * 3 / 2 - dy) / dx;

			// We force c to be strictly positive
			if (c < 1)
				c = 1;

			//tmp = dx * 3 / 2;  // ALMOST the resulting value, except for obvious rounding issues

			// FIXME: Where is the 3 coming from? Maybe they hard/coded, by "accident", that usually gy=3 ?
			// Then this choice of scalar will make t equal to roughly sqrt(dx)
		}
	}
	// POST: c >= 1
	tmp = c * dx + dy;
	// POST: (dx != 0)  ==>  ABS(tmp) > ABS(dx)
	// POST: (dx != 0)  ==>  ABS(tmp) ~>=~ ABS(dy)

	debugC(kDebugLevelBresen, "c: %d, tmp: %d", c, tmp);

	// Compute x step
	if (tmp != 0 && dx != 0)
		vx = (int16)((float)(dx * sqrt(gy / (2.0 * tmp))));
	else
		vx = 0;

	// Restore the left/right direction: dx and vx should have the same sign.
	if (dxWasNegative)
		vx = -vx;

	if ((dy < 0) && (vx == 0)) {
		// Special case: If this was a jump (almost) straight upward, i.e. dy < 0 (upward),
		// and vx == 0 (i.e. no horizontal movement, at least not after rounding), then we
		// compute vy directly.
		// For this, we drop the assumption on the linear correlation of vx and vy (obviously).

		// FIXME: This choice of vy makes t roughly (2+sqrt(2))/gy * sqrt(dy);
		// so if gy==3, then t is roughly sqrt(dy)...
		vy = (int)sqrt((float)gy * ABS(2 * dy)) + 1;
	} else {
		// As stated above, the vertical direction is correlated to the horizontal by the
		// (non-zero) factor c.
		// Strictly speaking, we should probably be using the value of vx *before* rounding
		// it to an integer... Ah well
		vy = c * vx;
	}

	// Always force vy to be upwards
	vy = -ABS(vy);

	debugC(kDebugLevelBresen, "SetJump for object at %04x:%04x", PRINT_REG(object));
	debugC(kDebugLevelBresen, "xStep: %d, yStep: %d", vx, vy);

	writeSelectorValue(segMan, object, SELECTOR(xStep), vx);
	writeSelectorValue(segMan, object, SELECTOR(yStep), vy);

	return s->r_acc;
}

reg_t kInitBresen(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t mover = argv[0];
	reg_t client = readSelector(segMan, mover, SELECTOR(client));
	int16 stepFactor = (argc >= 2) ? argv[1].toUint16() : 1;
	int16 mover_x = readSelectorValue(segMan, mover, SELECTOR(x));
	int16 mover_y = readSelectorValue(segMan, mover, SELECTOR(y));
	int16 client_xStep = readSelectorValue(segMan, client, SELECTOR(xStep)) * stepFactor;
	int16 client_yStep = readSelectorValue(segMan, client, SELECTOR(yStep)) * stepFactor;

	int16 client_step;
	if (client_xStep < client_yStep)
		client_step = client_yStep * 2;
	else
		client_step = client_xStep * 2;

	int16 deltaX = mover_x - readSelectorValue(segMan, client, SELECTOR(x));
	int16 deltaY = mover_y - readSelectorValue(segMan, client, SELECTOR(y));
	int16 mover_dx = 0;
	int16 mover_dy = 0;
	int16 mover_i1 = 0;
	int16 mover_i2 = 0;
	int16 mover_di = 0;
	int16 mover_incr = 0;
	int16 mover_xAxis = 0;

	while (1) {
		mover_dx = client_xStep;
		mover_dy = client_yStep;
		mover_incr = 1;

		if (ABS(deltaX) >= ABS(deltaY)) {
			mover_xAxis = 1;
			if (deltaX < 0)
				mover_dx = -mover_dx;
			mover_dy = deltaX ? mover_dx * deltaY / deltaX : 0;
			mover_i1 = ((mover_dx * deltaY) - (mover_dy * deltaX)) * 2;
			if (deltaY < 0) {
				mover_incr = -1;
				mover_i1 = -mover_i1;
			}
			mover_i2 = mover_i1 - (deltaX * 2);
			mover_di = mover_i1 - deltaX;
			if (deltaX < 0) {
				mover_i1 = -mover_i1;
				mover_i2 = -mover_i2;
				mover_di = -mover_di;
			}
		} else {
			mover_xAxis = 0;
			if (deltaY < 0)
				mover_dy = -mover_dy;
			mover_dx = deltaY ? mover_dy * deltaX / deltaY : 0;
			mover_i1 = ((mover_dy * deltaX) - (mover_dx * deltaY)) * 2;
			if (deltaX < 0) {
				mover_incr = -1;
				mover_i1 = -mover_i1;
			}
			mover_i2 = mover_i1 - (deltaY * 2);
			mover_di = mover_i1 - deltaY;
			if (deltaY < 0) {
				mover_i1 = -mover_i1;
				mover_i2 = -mover_i2;
				mover_di = -mover_di;
			}
			break;
		}
		if (client_xStep <= client_yStep)
			break;
		if (!client_xStep)
			break;
		if (client_yStep >= ABS(mover_dy + mover_incr))
			break;

		client_step--;
		if (!client_step)
			error("kInitBresen failed");
		client_xStep--;
	}

	// set mover
	writeSelectorValue(segMan, mover, SELECTOR(dx), mover_dx);
	writeSelectorValue(segMan, mover, SELECTOR(dy), mover_dy);
	writeSelectorValue(segMan, mover, SELECTOR(b_i1), mover_i1);
	writeSelectorValue(segMan, mover, SELECTOR(b_i2), mover_i2);
	writeSelectorValue(segMan, mover, SELECTOR(b_di), mover_di);
	writeSelectorValue(segMan, mover, SELECTOR(b_incr), mover_incr);
	writeSelectorValue(segMan, mover, SELECTOR(b_xAxis), mover_xAxis);
	return s->r_acc;
}

reg_t kDoBresen(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t mover = argv[0];
	reg_t client = readSelector(segMan, mover, SELECTOR(client));
	bool completed = false;
	bool handleMoveCount = g_sci->_features->handleMoveCount();

	if (getSciVersion() >= SCI_VERSION_1_EGA_ONLY) {
		uint client_signal = readSelectorValue(segMan, client, SELECTOR(signal));
		writeSelectorValue(segMan, client, SELECTOR(signal), client_signal & ~kSignalHitObstacle);
	}

	int16 mover_moveCnt = 1;
	int16 client_moveSpeed = 0;
	if (handleMoveCount) {
		mover_moveCnt = readSelectorValue(segMan, mover, SELECTOR(b_movCnt));
		client_moveSpeed = readSelectorValue(segMan, client, SELECTOR(moveSpeed));
		mover_moveCnt++;
	}

	if (client_moveSpeed < mover_moveCnt) {
		mover_moveCnt = 0;
		int16 client_x = readSelectorValue(segMan, client, SELECTOR(x));
		int16 client_y = readSelectorValue(segMan, client, SELECTOR(y));
		int16 mover_x = readSelectorValue(segMan, mover, SELECTOR(x));
		int16 mover_y = readSelectorValue(segMan, mover, SELECTOR(y));
		int16 mover_xAxis = readSelectorValue(segMan, mover, SELECTOR(b_xAxis));
		int16 mover_dx = readSelectorValue(segMan, mover, SELECTOR(dx));
		int16 mover_dy = readSelectorValue(segMan, mover, SELECTOR(dy));
		int16 mover_incr = readSelectorValue(segMan, mover, SELECTOR(b_incr));
		int16 mover_i1 = readSelectorValue(segMan, mover, SELECTOR(b_i1));
		int16 mover_i2 = readSelectorValue(segMan, mover, SELECTOR(b_i2));
		int16 mover_di = readSelectorValue(segMan, mover, SELECTOR(b_di));
		int16 mover_org_i1 = mover_i1;
		int16 mover_org_i2 = mover_i2;
		int16 mover_org_di = mover_di;

		if ((getSciVersion() >= SCI_VERSION_1_EGA_ONLY)) {
			// save current position into mover
			writeSelectorValue(segMan, mover, SELECTOR(xLast), client_x);
			writeSelectorValue(segMan, mover, SELECTOR(yLast), client_y);
		}

		// Store backups of all client selector variables. We will restore them
		// in case of a collision.
		Object* clientObject = segMan->getObject(client);
		uint clientVarNum = clientObject->getVarCount();
		reg_t* clientBackup = new reg_t[clientVarNum];
		for (uint i = 0; i < clientVarNum; ++i)
			clientBackup[i] = clientObject->getVariable(i);

		if (mover_xAxis) {
			if (ABS(mover_x - client_x) < ABS(mover_dx))
				completed = true;
		} else {
			if (ABS(mover_y - client_y) < ABS(mover_dy))
				completed = true;
		}
		if (completed) {
			client_x = mover_x;
			client_y = mover_y;
		} else {
			client_x += mover_dx;
			client_y += mover_dy;
			if (mover_di < 0) {
				mover_di += mover_i1;
			} else {
				mover_di += mover_i2;
				if (mover_xAxis == 0) {
					client_x += mover_incr;
				} else {
					client_y += mover_incr;
				}
			}
		}
		writeSelectorValue(segMan, client, SELECTOR(x), client_x);
		writeSelectorValue(segMan, client, SELECTOR(y), client_y);

		// Now call client::canBeHere/client::cantBehere to check for collisions
		bool collision = false;
		reg_t cantBeHere = NULL_REG;

		if (SELECTOR(cantBeHere) != -1) {
			// adding this here for hoyle 3 to get happy. CantBeHere is a dummy in hoyle 3 and acc is != 0 so we would
			//  get a collision otherwise
			s->r_acc = NULL_REG;
			invokeSelector(s, client, SELECTOR(cantBeHere), argc, argv);
			if (!s->r_acc.isNull())
				collision = true;
			cantBeHere = s->r_acc;
		} else {
			invokeSelector(s, client, SELECTOR(canBeHere), argc, argv);
			if (s->r_acc.isNull())
				collision = true;
		}

		if (collision) {
			// We restore the backup of the client variables
			for (uint i = 0; i < clientVarNum; ++i)
				clientObject->getVariableRef(i) = clientBackup[i];

			mover_i1 = mover_org_i1;
			mover_i2 = mover_org_i2;
			mover_di = mover_org_di;

			uint16 client_signal = readSelectorValue(segMan, client, SELECTOR(signal));
			writeSelectorValue(segMan, client, SELECTOR(signal), client_signal | kSignalHitObstacle);
		}
		delete[] clientBackup;

		writeSelectorValue(segMan, mover, SELECTOR(b_i1), mover_i1);
		writeSelectorValue(segMan, mover, SELECTOR(b_i2), mover_i2);
		writeSelectorValue(segMan, mover, SELECTOR(b_di), mover_di);

		if (getSciVersion() >= SCI_VERSION_1_EGA_ONLY) {
			// In sci1egaonly this block of code was outside of the main if,
			// but client_x/client_y aren't set there, so it was an
			// uninitialized read in SSCI. (This issue was fixed in sci1early.)
			if (handleMoveCount)
				writeSelectorValue(segMan, mover, SELECTOR(b_movCnt), mover_moveCnt);
			// We need to compare directly in here, complete may have happened during
			//  the current move
			if ((client_x == mover_x) && (client_y == mover_y))
				invokeSelector(s, mover, SELECTOR(moveDone), argc, argv);
			return s->r_acc;
		}
	}

	if (handleMoveCount)
		writeSelectorValue(segMan, mover, SELECTOR(b_movCnt), mover_moveCnt);

	return s->r_acc;
}

extern void kDirLoopWorker(reg_t obj, uint16 angle, EngineState *s, int argc, reg_t *argv);
extern uint16 kGetAngleWorker(int16 x1, int16 y1, int16 x2, int16 y2);

reg_t kDoAvoider(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t avoider = argv[0];
	int16 timesStep = argc > 1 ? argv[1].toUint16() : 1;

	if (!s->_segMan->isHeapObject(avoider)) {
		error("DoAvoider() where avoider %04x:%04x is not an object", PRINT_REG(avoider));
		return SIGNAL_REG;
	}

	reg_t client = readSelector(segMan, avoider, SELECTOR(client));
	reg_t mover = readSelector(segMan, client, SELECTOR(mover));
	if (mover.isNull())
		return SIGNAL_REG;

	// call mover::doit
	invokeSelector(s, mover, SELECTOR(doit), argc, argv);

	// Read mover again
	mover = readSelector(segMan, client, SELECTOR(mover));
	if (mover.isNull())
		return SIGNAL_REG;

	int16 clientX = readSelectorValue(segMan, client, SELECTOR(x));
	int16 clientY = readSelectorValue(segMan, client, SELECTOR(y));
	int16 moverX = readSelectorValue(segMan, mover, SELECTOR(x));
	int16 moverY = readSelectorValue(segMan, mover, SELECTOR(y));
	int16 avoiderHeading = readSelectorValue(segMan, avoider, SELECTOR(heading));

	// call client::isBlocked
	invokeSelector(s, client, SELECTOR(isBlocked), argc, argv);

	if (s->r_acc.isNull()) {
		// not blocked
		if (avoiderHeading == -1)
			return SIGNAL_REG;
		avoiderHeading = -1;

		uint16 angle = kGetAngleWorker(clientX, clientY, moverX, moverY);

		reg_t clientLooper = readSelector(segMan, client, SELECTOR(looper));
		if (clientLooper.isNull()) {
			kDirLoopWorker(client, angle, s, argc, argv);
		} else {
			// call looper::doit
			reg_t params[2] = { make_reg(0, angle), client };
			invokeSelector(s, clientLooper, SELECTOR(doit), argc, argv, 2, params);
		}
		s->r_acc = SIGNAL_REG;

	} else {
		// is blocked
		if (avoiderHeading == -1)
			avoiderHeading = g_sci->getRNG().getRandomBit() ? 45 : -45;
		int16 clientHeading = readSelectorValue(segMan, client, SELECTOR(heading));
		clientHeading = (clientHeading / 45) * 45;

		int16 clientXstep = readSelectorValue(segMan, client, SELECTOR(xStep)) * timesStep;
		int16 clientYstep = readSelectorValue(segMan, client, SELECTOR(yStep)) * timesStep;
		int16 newHeading = clientHeading;

		while (1) {
			int16 newX = clientX;
			int16 newY = clientY;
			switch (newHeading) {
			case 45:
			case 90:
			case 135:
				newX += clientXstep;
				break;
			case 225:
			case 270:
			case 315:
				newX -= clientXstep;
			}

			switch (newHeading) {
			case 0:
			case 45:
			case 315:
				newY -= clientYstep;
				break;
			case 135:
			case 180:
			case 225:
				newY += clientYstep;
			}
			writeSelectorValue(segMan, client, SELECTOR(x), newX);
			writeSelectorValue(segMan, client, SELECTOR(y), newY);

			// call client::canBeHere
			invokeSelector(s, client, SELECTOR(canBeHere), argc, argv);

			if (!s->r_acc.isNull()) {
				s->r_acc = make_reg(0, newHeading);
				break; // break out
			}

			newHeading += avoiderHeading;
			if (newHeading >= 360)
				newHeading -= 360;
			if (newHeading < 0)
				newHeading += 360;
			if (newHeading == clientHeading) {
				// tried everything
				writeSelectorValue(segMan, client, SELECTOR(x), clientX);
				writeSelectorValue(segMan, client, SELECTOR(y), clientY);
				s->r_acc = SIGNAL_REG;
				break; // break out
			}
		}
	}
	writeSelectorValue(segMan, avoider, SELECTOR(heading), avoiderHeading);
	return s->r_acc;
}

} // End of namespace Sci
