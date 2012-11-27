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
 * This file contains utilities to handle object animation.
 */

#include "tinsel/anim.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/sched.h"
#include "tinsel/tinsel.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

/**
 * Advance to next frame routine.
 * @param pAnim			Animation data structure
 */
SCRIPTSTATE DoNextFrame(ANIM *pAnim) {
	// get a pointer to the script
	const ANI_SCRIPT *pAni = (const ANI_SCRIPT *)LockMem(pAnim->hScript);

	while (1) {	// repeat until a real image
		debugC(DEBUG_DETAILED, kTinselDebugAnimations,
		"DoNextFrame %ph index=%d, op=%xh", (byte *)pAnim, pAnim->scriptIndex,
		FROM_LE_32(pAni[pAnim->scriptIndex].op));

		switch ((int32)FROM_LE_32(pAni[pAnim->scriptIndex].op)) {
		case ANI_END:	// end of animation script

			// move to next opcode
			pAnim->scriptIndex++;

			// indicate script has finished
			return ScriptFinished;

		case ANI_JUMP:	// do animation jump

			// move to jump address
			pAnim->scriptIndex++;

			// jump to new frame position
			pAnim->scriptIndex += (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op);

			// go fetch a real image
			break;

		case ANI_HFLIP:	// flip animated object horizontally

			// next opcode
			pAnim->scriptIndex++;

			MultiHorizontalFlip(pAnim->pObject);

			// go fetch a real image
			break;

		case ANI_VFLIP:	// flip animated object vertically

			// next opcode
			pAnim->scriptIndex++;

			MultiVerticalFlip(pAnim->pObject);

			// go fetch a real image
			break;
		case ANI_HVFLIP:	// flip animated object in both directions

			// next opcode
			pAnim->scriptIndex++;

			MultiHorizontalFlip(pAnim->pObject);
			MultiVerticalFlip(pAnim->pObject);

			// go fetch a real image
			break;

		case ANI_ADJUSTX:	// adjust animated object x animation point

			// move to x adjustment operand
			pAnim->scriptIndex++;

			MultiAdjustXY(pAnim->pObject, (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op), 0);

			// next opcode
			pAnim->scriptIndex++;

			// go fetch a real image
			break;

		case ANI_ADJUSTY:	// adjust animated object y animation point

			// move to y adjustment operand
			pAnim->scriptIndex++;

			MultiAdjustXY(pAnim->pObject, 0, (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op));

			// next opcode
			pAnim->scriptIndex++;

			// go fetch a real image
			break;

		case ANI_ADJUSTXY:	// adjust animated object x & y animation points
		{
			int x, y;

			// move to x adjustment operand
			pAnim->scriptIndex++;
			x = (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op);

			// move to y adjustment operand
			pAnim->scriptIndex++;
			y = (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op);

			MultiAdjustXY(pAnim->pObject, x, y);

			// next opcode
			pAnim->scriptIndex++;

			// go fetch a real image
			break;
		}

		case ANI_NOSLEEP:	// do not sleep for this frame

			// next opcode
			pAnim->scriptIndex++;

			// indicate not to sleep
			return ScriptNoSleep;

		case ANI_CALL:		// call routine

			// move to function address
			pAnim->scriptIndex++;

			// make function call

			// REMOVED BUGGY CODE
			// pFunc is a function pointer that's part of a union and is assumed to be 32-bits.
			// There is no known place where a function pointer is stored inside the animation
			// scripts, something which wouldn't have worked anyway. Having played through the
			// entire game, there hasn't been any occurence of this case, so just error out here
			// in case we missed something (highly unlikely though)
			error("ANI_CALL opcode encountered! Please report this error to the ScummVM team");
			//(*pAni[pAnim->scriptIndex].pFunc)(pAnim);
			return ScriptSleep;	// for compilers that don't support NORETURN

#if 0
			// next opcode
			pAnim->scriptIndex++;

			// go fetch a real image
			break;
#endif

		case ANI_HIDE:		// hide animated object

			MultiHideObject(pAnim->pObject);

			// next opcode
			pAnim->scriptIndex++;

			// dont skip a sleep
			return ScriptSleep;

		default:	// must be an actual animation frame handle

			// set objects new animation frame
			pAnim->pObject->hShape = FROM_LE_32(pAni[pAnim->scriptIndex].hFrame);

			// re-shape the object
			MultiReshape(pAnim->pObject);

			// next opcode
			pAnim->scriptIndex++;

			// dont skip a sleep
			return ScriptSleep;
		}
	}
}

/**
 * Init a ANIM structure for single stepping through a animation script.
 * @param pAnim				Animation data structure
 * @param pAniObj			Object to animate
 * @param hNewScript		Script of multipart frames
 * @param aniSpeed			Sets speed of animation in frames
 */
void InitStepAnimScript(ANIM *pAnim, OBJECT *pAniObj, SCNHANDLE hNewScript, int aniSpeed) {
	OBJECT *pObj;			// multi-object list iterator

	debugC(DEBUG_DETAILED, kTinselDebugAnimations,
		"InitStepAnimScript Object=(%d,%d,%xh) script=%xh aniSpeed=%d rec=%ph",
		!pAniObj ? 0 : fracToInt(pAniObj->xPos),
		!pAniObj ? 0 : fracToInt(pAniObj->yPos),
		!pAniObj ? 0 : pAniObj->hImg, hNewScript, aniSpeed, (byte *)pAnim);

	pAnim->aniDelta = 1;		// will animate on next call to NextAnimRate
	pAnim->pObject = pAniObj;	// set object to animate
	pAnim->hScript = hNewScript;	// set animation script
	pAnim->scriptIndex = 0;		// start of script
	pAnim->aniRate = aniSpeed;	// set speed of animation

	// reset flip flags for the object - let the script do the flipping
	for (pObj = pAniObj; pObj != NULL; pObj = pObj->pSlave) {
		AnimateObjectFlags(pObj, pObj->flags & ~(DMA_FLIPH | DMA_FLIPV),
			pObj->hImg);
	}
}

/**
 * Execute the next command in a animation script.
 * @param pAnim			Animation data structure
 */
SCRIPTSTATE StepAnimScript(ANIM *pAnim) {
	SCRIPTSTATE state;

	if (--pAnim->aniDelta == 0) {
		// re-init animation delta counter
		pAnim->aniDelta = pAnim->aniRate;

		if (TinselV2)
			state = DoNextFrame(pAnim);
		else {
			// move to next frame
			while ((state = DoNextFrame(pAnim)) == ScriptNoSleep)
				;
		}

		return state;
	}

	// indicate calling task should sleep
	return ScriptSleep;
}

/**
 * Skip the specified number of frames.
 * @param pAnim			Animation data structure
 * @param numFrames		Number of frames to skip
 */
void SkipFrames(ANIM *pAnim, int numFrames) {
	// get a pointer to the script
	const ANI_SCRIPT *pAni = (const ANI_SCRIPT *)LockMem(pAnim->hScript);

	if (!TinselV2 && (numFrames <= 0))
		// do nothing
		return;

	while (1) {	// repeat until a real image

		switch ((int32)FROM_LE_32(pAni[pAnim->scriptIndex].op)) {
		case ANI_END:	// end of animation script
			// going off the end is probably a error, but only in Tinsel 1
			if (!TinselV2)
				error("SkipFrames(): formally 'assert(0)!'");
			return;

		case ANI_JUMP:	// do animation jump

			// move to jump address
			pAnim->scriptIndex++;

			// jump to new frame position
			pAnim->scriptIndex += (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op);

			if (TinselV2)
				// Done if skip to jump
				return;
			break;

		case ANI_HFLIP:	// flip animated object horizontally

			// next opcode
			pAnim->scriptIndex++;

			MultiHorizontalFlip(pAnim->pObject);
			break;

		case ANI_VFLIP:	// flip animated object vertically

			// next opcode
			pAnim->scriptIndex++;

			MultiVerticalFlip(pAnim->pObject);
			break;

		case ANI_HVFLIP:	// flip animated object in both directions

			// next opcode
			pAnim->scriptIndex++;

			MultiHorizontalFlip(pAnim->pObject);
			MultiVerticalFlip(pAnim->pObject);
			break;

		case ANI_ADJUSTX:	// adjust animated object x animation point

			// move to x adjustment operand
			pAnim->scriptIndex++;

			MultiAdjustXY(pAnim->pObject, (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op), 0);

			// next opcode
			pAnim->scriptIndex++;
			break;

		case ANI_ADJUSTY:	// adjust animated object y animation point

			// move to y adjustment operand
			pAnim->scriptIndex++;

			MultiAdjustXY(pAnim->pObject, 0, (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op));

			// next opcode
			pAnim->scriptIndex++;
			break;

		case ANI_ADJUSTXY:	// adjust animated object x & y animation points
		{
			int x, y;

			// move to x adjustment operand
			pAnim->scriptIndex++;
			x = (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op);

			// move to y adjustment operand
			pAnim->scriptIndex++;
			y = (int32)FROM_LE_32(pAni[pAnim->scriptIndex].op);

			MultiAdjustXY(pAnim->pObject, x, y);

			// next opcode
			pAnim->scriptIndex++;

			break;
		}

		case ANI_NOSLEEP:	// do not sleep for this frame

			// next opcode
			pAnim->scriptIndex++;
			break;

		case ANI_CALL:		// call routine

			// skip function address
			pAnim->scriptIndex += 2;
			break;

		case ANI_HIDE:		// hide animated object

			// next opcode
			pAnim->scriptIndex++;
			break;

		default:	// must be an actual animation frame handle

			// one less frame
			if (numFrames == 0)
				return;

			if (numFrames == -1 || numFrames-- > 0) {
				// next opcode
				pAnim->scriptIndex++;
			} else {
				// set objects new animation frame
				pAnim->pObject->hShape = FROM_LE_32(pAni[pAnim->scriptIndex].hFrame);

				// re-shape the object
				MultiReshape(pAnim->pObject);

				// we have skipped to the correct place
				return;
			}
			break;
		}
	}
}

/**
 * About to jump or end
 * @param pAnim			Animation data structure
 */
bool AboutToJumpOrEnd(PANIM pAnim) {
	if (pAnim->aniDelta == 1) {
		// get a pointer to the script
		ANI_SCRIPT *pAni = (ANI_SCRIPT *)LockMem(pAnim->hScript);
		int	zzz = pAnim->scriptIndex;

		for (;;) {
			// repeat until a real image
			switch (FROM_LE_32(pAni[zzz].op)) {
			case ANI_END:		// end of animation script
			case ANI_JUMP:		// do animation jump
				return true;

			case ANI_HFLIP:		// flip animated object horizontally
			case ANI_VFLIP:		// flip animated object vertically
			case ANI_HVFLIP:	// flip animated object in both directions
				zzz++;
				break;

			case ANI_ADJUSTX:	// adjust animated object x animation point
			case ANI_ADJUSTY:	// adjust animated object y animation point
				zzz += 2;
				break;

			case ANI_ADJUSTXY:	// adjust animated object x & y animation points
				zzz += 3;
				break;

			case ANI_HIDE:		// hide animated object
			default:		// must be an actual animation frame handle
				return false;
			}
		}
	}

	return false;
}



} // End of namespace Tinsel
