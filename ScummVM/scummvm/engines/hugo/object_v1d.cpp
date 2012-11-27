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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "common/random.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/object.h"
#include "hugo/display.h"
#include "hugo/file.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"

namespace Hugo {

ObjectHandler_v1d::ObjectHandler_v1d(HugoEngine *vm) : ObjectHandler(vm) {
}

ObjectHandler_v1d::~ObjectHandler_v1d() {
}

/**
 * Draw all objects on screen as follows:
 * 1. Sort 'FLOATING' objects in order of y2 (base of object)
 * 2. Display new object frames/positions in dib
 * Finally, cycle any animating objects to next frame
 */
void ObjectHandler_v1d::updateImages() {
	debugC(5, kDebugObject, "updateImages");

	// Initialize the index array to visible objects in current screen
	int  objNumb = 0;
	byte objindex[kMaxObjNumb];                     // Array of indeces to objects

	for (int i = 0; i < _numObj; i++) {
		Object *obj = &_objects[i];
		if ((obj->_screenIndex == *_vm->_screenPtr) && (obj->_cycling >= kCycleAlmostInvisible))
			objindex[objNumb++] = i;
	}

	// Sort the objects into increasing y+y2 (painter's algorithm)
	qsort(objindex, objNumb, sizeof(objindex[0]), y2comp);

	// Add each visible object to display list
	for (int i = 0; i < objNumb; i++) {
		Object *obj = &_objects[objindex[i]];
		// Count down inter-frame timer
		if (obj->_frameTimer)
			obj->_frameTimer--;

		if (obj->_cycling > kCycleAlmostInvisible) {      // Only if visible
			switch (obj->_cycling) {
			case kCycleNotCycling:
				_vm->_screen->displayFrame(obj->_x, obj->_y, obj->_currImagePtr, false);
				break;
			case kCycleForward:
				if (obj->_frameTimer)                // Not time to see next frame yet
					_vm->_screen->displayFrame(obj->_x, obj->_y, obj->_currImagePtr, false);
				else
					_vm->_screen->displayFrame(obj->_x, obj->_y, obj->_currImagePtr->_nextSeqPtr, false);
				break;
			case kCycleBackward: {
				Seq *seqPtr = obj->_currImagePtr;
				if (!obj->_frameTimer) {             // Show next frame
					while (seqPtr->_nextSeqPtr != obj->_currImagePtr)
						seqPtr = seqPtr->_nextSeqPtr;
				}
				_vm->_screen->displayFrame(obj->_x, obj->_y, seqPtr, false);
				break;
				}
			default:
				break;
			}
		}
	}

	_vm->_scheduler->waitForRefresh();

	// Cycle any animating objects
	for (int i = 0; i < objNumb; i++) {
		Object *obj = &_objects[objindex[i]];
		if (obj->_cycling != kCycleInvisible) {
			// Only if it's visible
			if (obj->_cycling == kCycleAlmostInvisible)
				obj->_cycling = kCycleInvisible;

			// Now Rotate to next picture in sequence
			switch (obj->_cycling) {
			case kCycleNotCycling:
				break;
			case kCycleForward:
				if (!obj->_frameTimer) {
					// Time to step to next frame
					obj->_currImagePtr = obj->_currImagePtr->_nextSeqPtr;
					// Find out if this is last frame of sequence
					// If so, reset frame_timer and decrement n_cycle
					if (obj->_frameInterval || obj->_cycleNumb) {
						obj->_frameTimer = obj->_frameInterval;
						for (int j = 0; j < obj->_seqNumb; j++) {
							if (obj->_currImagePtr->_nextSeqPtr == obj->_seqList[j]._seqPtr) {
								if (obj->_cycleNumb) { // Decr cycleNumb if Non-continous
									if (!--obj->_cycleNumb)
										obj->_cycling = kCycleNotCycling;
								}
							}
						}
					}
				}
				break;
			case kCycleBackward: {
				if (!obj->_frameTimer) {
					// Time to step to prev frame
					Seq *seqPtr = obj->_currImagePtr;
					while (obj->_currImagePtr->_nextSeqPtr != seqPtr)
						obj->_currImagePtr = obj->_currImagePtr->_nextSeqPtr;
					// Find out if this is first frame of sequence
					// If so, reset frame_timer and decrement n_cycle
					if (obj->_frameInterval || obj->_cycleNumb) {
						obj->_frameTimer = obj->_frameInterval;
						for (int j = 0; j < obj->_seqNumb; j++) {
							if (obj->_currImagePtr == obj->_seqList[j]._seqPtr) {
								if (obj->_cycleNumb){ // Decr cycleNumb if Non-continous
									if (!--obj->_cycleNumb)
										obj->_cycling = kCycleNotCycling;
								}
							}
						}
					}
				}
				break;
				}
			default:
				break;
			}
			obj->_oldx = obj->_x;
			obj->_oldy = obj->_y;
		}
	}
}

/**
 * Update all object positions.  Process object 'local' events
 * including boundary events and collisions
 */
void ObjectHandler_v1d::moveObjects() {
	debugC(4, kDebugObject, "moveObjects");

	// Added to DOS version in order to handle mouse properly
	// Do special route processing
	_vm->_route->processRoute();

	// Perform any adjustments to velocity based on special path types
	// and store all (visible) object baselines into the boundary file.
	// Don't store foreground or background objects
	for (int i = 0; i < _numObj; i++) {
		Object *obj = &_objects[i];               // Get pointer to object
		Seq *currImage = obj->_currImagePtr;       // Get ptr to current image
		if (obj->_screenIndex == *_vm->_screenPtr) {
			switch (obj->_pathType) {
			case kPathChase: {
				// Allowable motion wrt boundary
				int dx = _vm->_hero->_x + _vm->_hero->_currImagePtr->_x1 - obj->_x - currImage->_x1;
				int dy = _vm->_hero->_y + _vm->_hero->_currImagePtr->_y2 - obj->_y - currImage->_y2 - 1;
				if (abs(dx) <= 1)
					obj->_vx = 0;
				else
					obj->_vx = (dx > 0) ? MIN(dx, obj->_vxPath) : MAX(dx, -obj->_vxPath);
				if (abs(dy) <= 1)
					obj->_vy = 0;
				else
					obj->_vy = (dy > 0) ? MIN(dy, obj->_vyPath) : MAX(dy, -obj->_vyPath);

				// Set first image in sequence (if multi-seq object)
				if (obj->_seqNumb == 4) {
					if (!obj->_vx) {                 // Got 4 directions
						if (obj->_vx != obj->_oldvx) {// vx just stopped
							if (dy > 0)
								obj->_currImagePtr = obj->_seqList[SEQ_DOWN]._seqPtr;
							else
								obj->_currImagePtr = obj->_seqList[SEQ_UP]._seqPtr;
						}
					} else if (obj->_vx != obj->_oldvx) {
						if (dx > 0)
							obj->_currImagePtr = obj->_seqList[SEQ_RIGHT]._seqPtr;
						else
							obj->_currImagePtr = obj->_seqList[SEQ_LEFT]._seqPtr;
					}
				}

				if (obj->_vx || obj->_vy) {
					if (obj->_seqNumb > 1)
						obj->_cycling = kCycleForward;
				} else {
					obj->_cycling = kCycleNotCycling;
					boundaryCollision(obj);         // Must have got hero!
				}
				obj->_oldvx = obj->_vx;
				obj->_oldvy = obj->_vy;
				currImage = obj->_currImagePtr;      // Get (new) ptr to current image
				break;
				}
			case kPathWander:
				if (!_vm->_rnd->getRandomNumber(3 * _vm->_normalTPS)) {       // Kick on random interval
					obj->_vx = _vm->_rnd->getRandomNumber(obj->_vxPath << 1) - obj->_vxPath;
					obj->_vy = _vm->_rnd->getRandomNumber(obj->_vyPath << 1) - obj->_vyPath;

					// Set first image in sequence (if multi-seq object)
					if (obj->_seqNumb > 1) {
						if (!obj->_vx && (obj->_seqNumb > 2)) {
							if (obj->_vx != obj->_oldvx) { // vx just stopped
								if (obj->_vy > 0)
									obj->_currImagePtr = obj->_seqList[SEQ_DOWN]._seqPtr;
								else
									obj->_currImagePtr = obj->_seqList[SEQ_UP]._seqPtr;
							}
						} else if (obj->_vx != obj->_oldvx) {
							if (obj->_vx > 0)
								obj->_currImagePtr = obj->_seqList[SEQ_RIGHT]._seqPtr;
							else
								obj->_currImagePtr = obj->_seqList[SEQ_LEFT]._seqPtr;
						}

						if (obj->_vx || obj->_vy)
							obj->_cycling = kCycleForward;
						else
							obj->_cycling = kCycleNotCycling;
					}
					obj->_oldvx = obj->_vx;
					obj->_oldvy = obj->_vy;
					currImage = obj->_currImagePtr;  // Get (new) ptr to current image
				}
				break;
			default:
				; // Really, nothing
			}
			// Store boundaries
			if ((obj->_cycling > kCycleAlmostInvisible) && (obj->_priority == kPriorityFloating))
				storeBoundary(obj->_x + currImage->_x1, obj->_x + currImage->_x2, obj->_y + currImage->_y2);
		}
	}

	// Move objects, allowing for boundaries
	for (int i = 0; i < _numObj; i++) {
		Object *obj = &_objects[i];               // Get pointer to object
		if ((obj->_screenIndex == *_vm->_screenPtr) && (obj->_vx || obj->_vy)) {
			// Only process if it's moving

			// Do object movement.  Delta_x,y return allowed movement in x,y
			// to move as close to a boundary as possible without crossing it.
			Seq *currImage = obj->_currImagePtr;   // Get ptr to current image
			// object coordinates
			int x1 = obj->_x + currImage->_x1;        // Left edge of object
			int x2 = obj->_x + currImage->_x2;        // Right edge
			int y1 = obj->_y + currImage->_y1;        // Top edge
			int y2 = obj->_y + currImage->_y2;        // Bottom edge

			if ((obj->_cycling > kCycleAlmostInvisible) && (obj->_priority == kPriorityFloating))
				clearBoundary(x1, x2, y2);     // Clear our own boundary

			// Allowable motion wrt boundary
			int dx = deltaX(x1, x2, obj->_vx, y2);
			if (dx != obj->_vx) {
				// An object boundary collision!
				boundaryCollision(obj);
				obj->_vx = 0;
			}

			int dy = deltaY(x1, x2, obj->_vy, y2);
			if (dy != obj->_vy) {
				// An object boundary collision!
				boundaryCollision(obj);
				obj->_vy = 0;
			}

			if ((obj->_cycling > kCycleAlmostInvisible) && (obj->_priority == kPriorityFloating))
				storeBoundary(x1, x2, y2);     // Re-store our own boundary

			obj->_x += dx;                           // Update object position
			obj->_y += dy;

			// Don't let object go outside screen
			if (x1 < kEdge)
				obj->_x = kEdge2;
			if (x2 > (kXPix - kEdge))
				obj->_x = kXPix - kEdge2 - (x2 - x1);
			if (y1 < kEdge)
				obj->_y = kEdge2;
			if (y2 > (kYPix - kEdge))
				obj->_y = kYPix - kEdge2 - (y2 - y1);

			if ((obj->_vx == 0) && (obj->_vy == 0))
				obj->_cycling = kCycleNotCycling;
		}
	}

	// Clear all object baselines from the boundary file.
	for (int i = 0; i < _numObj; i++) {
		Object *obj = &_objects[i];               // Get pointer to object
		Seq *currImage = obj->_currImagePtr;       // Get ptr to current image
		if ((obj->_screenIndex == *_vm->_screenPtr) && (obj->_cycling > kCycleAlmostInvisible) && (obj->_priority == kPriorityFloating))
			clearBoundary(obj->_oldx + currImage->_x1, obj->_oldx + currImage->_x2, obj->_oldy + currImage->_y2);
	}

	// If maze mode is enabled, do special maze processing
	if (_vm->_maze._enabledFl) {
		Seq *currImage = _vm->_hero->_currImagePtr;// Get ptr to current image
		// hero coordinates
		int x1 = _vm->_hero->_x + currImage->_x1;     // Left edge of object
		int x2 = _vm->_hero->_x + currImage->_x2;     // Right edge
		int y1 = _vm->_hero->_y + currImage->_y1;     // Top edge
		int y2 = _vm->_hero->_y + currImage->_y2;     // Bottom edge

		_vm->_scheduler->processMaze(x1, x2, y1, y2);
	}
}

/**
 * Swap all the images of one object with another.  Set hero_image (we make
 * the assumption for now that the first obj is always the HERO) to the object
 * number of the swapped image
 */
void ObjectHandler_v1d::swapImages(int objIndex1, int objIndex2) {
	debugC(1, kDebugObject, "swapImages(%d, %d)", objIndex1, objIndex2);

	SeqList tmpSeqList[kMaxSeqNumb];
	int seqListSize = sizeof(SeqList) * kMaxSeqNumb;

	memmove(tmpSeqList, _objects[objIndex1]._seqList, seqListSize);
	memmove(_objects[objIndex1]._seqList, _objects[objIndex2]._seqList, seqListSize);
	memmove(_objects[objIndex2]._seqList, tmpSeqList, seqListSize);
	_objects[objIndex1]._currImagePtr = _objects[objIndex1]._seqList[0]._seqPtr;
	_objects[objIndex2]._currImagePtr = _objects[objIndex2]._seqList[0]._seqPtr;
	_vm->_heroImage = (_vm->_heroImage == kHeroIndex) ? objIndex2 : kHeroIndex;
}

void ObjectHandler_v1d::homeIn(int objIndex1, const int objIndex2, const int8 objDx, const int8 objDy) {
	// object obj1 will home in on object obj2
	Object *obj1 = &_objects[objIndex1];
	Object *obj2 = &_objects[objIndex2];
	obj1->_pathType = kPathAuto;
	int dx = obj1->_x + obj1->_currImagePtr->_x1 - obj2->_x - obj2->_currImagePtr->_x1;
	int dy = obj1->_y + obj1->_currImagePtr->_y1 - obj2->_y - obj2->_currImagePtr->_y1;

	if (dx == 0)                                    // Don't EVER divide by zero!
		dx = 1;
	if (dy == 0)
		dy = 1;

	if (abs(dx) > abs(dy)) {
		obj1->_vx = objDx * -sign<int8>(dx);
		obj1->_vy = abs((objDy * dy) / dx) * -sign<int8>(dy);
	} else {
		obj1->_vy = objDy * sign<int8>(dy);
		obj1->_vx = abs((objDx * dx) / dy) * sign<int8>(dx);
	}
}
} // End of namespace Hugo
