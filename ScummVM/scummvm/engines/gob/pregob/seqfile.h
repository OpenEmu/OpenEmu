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

#ifndef GOB_PREGOB_SEQFILE_H
#define GOB_PREGOB_SEQFILE_H

#include "common/system.h"
#include "common/array.h"
#include "common/list.h"

#include "gob/util.h"

namespace Common {
	class String;
	class SeekableReadStream;
}

namespace Gob {

class GobEngine;

class DECFile;
class ANIFile;
class ANIObject;

/** A SEQ file, describing a complex animation sequence.
 *
 *  Used in early hardcoded gob games.
 *  The principle is similar to the Mult class (see mult.h), but instead
 *  of depending on all the externally loaded animations, backgrounds and
 *  objects, a SEQ file references animation and background directly by
 *  filename.
 */
class SEQFile {
public:
	SEQFile(GobEngine *vm, const Common::String &fileName);
	virtual ~SEQFile();

	/** Play the SEQ.
	 *
	 *  @param abortable If true, end playback on any user input.
	 *  @param endFrame  The frame on where to end, or 0xFFFF for infinite playback.
	 *  @param frameRate The frame rate at which to play the SEQ, or 0 for playing at
	 *                   the speed the SEQ itself wants to.
	 */
	void play(bool abortable = true, uint16 endFrame = 0xFFFF, uint16 frameRate = 0);


protected:
	GobEngine *_vm;


	/** Returns the current frame number. */
	uint16 getFrame() const;

	/** Seek to a specific frame. */
	void seekFrame(uint16 frame);

	/** Add a frame loop. */
	uint addLoop(uint16 startFrame, uint16 endFrame, uint16 loopCount);

	/** Skip a frame loop. */
	void skipLoop(uint loopID);

	/** Delete a frame loop. */
	void delLoop(uint loopID);

	/** Ends SEQ playback. */
	void abortPlay();

	/** Callback for special frame events. */
	virtual void handleFrameEvent();
	/** Callback for special user input handling. */
	virtual void handleInput(int16 key, int16 mouseX, int16 mouseY, MouseButtons mouseButtons);


private:
	/** Number of animation objects that are visible at the same time. */
	static const uint kObjectCount = 4;

	/** A key for changing the background. */
	struct BackgroundKey {
		uint16 frame; ///< Frame the change is to happen.

		const DECFile *background; ///< The background to use.
	};

	/** A key for playing an object animation. */
	struct AnimationKey {
		uint object; ///< The object this key belongs to.

		uint16 frame; ///< Frame the change is to happen.

		const ANIFile *ani; ///< The ANI to use.

		uint16 animation; ///< The animation to use.

		int16 x; ///< X position of the animation.
		int16 y; ///< Y position of the animation.

		int16 order; ///< Used to determine in which order to draw the objects.
	};

	/** A managed animation object. */
	struct Object {
		ANIObject *object; ///< The actual animation object.

		int16 order; ///< The current drawing order.
	};

	/** A frame loop. */
	struct Loop {
		uint16 startFrame;
		uint16 endFrame;

		uint16 loopCount;
		uint16 currentLoop;

		bool empty;
	};

	typedef Common::Array<DECFile *> Backgrounds;
	typedef Common::Array<ANIFile *> Animations;

	typedef Common::Array<BackgroundKey> BackgroundKeys;
	typedef Common::Array<AnimationKey>  AnimationKeys;

	typedef Common::List<Object> Objects;

	typedef Common::Array<Loop> Loops;


	uint16 _frame;     ///< The current frame.
	bool   _abortPlay; ///< Was the end of the playback requested?

	uint16 _frameRate;

	Backgrounds _backgrounds; ///< All backgrounds in this SEQ.
	Animations  _animations;  ///< All animations in this SEQ.

	BackgroundKeys _bgKeys;   ///< The background change keyframes.
	AnimationKeys  _animKeys; ///< The animation change keyframes.

	Object _objects[kObjectCount]; ///< The managed animation objects.

	Loops _loops;

	/** Whether the playback should be abortable by user input. */
	bool _abortable;


	// -- Loading helpers --

	void load(Common::SeekableReadStream &seq);

	const ANIFile *findANI(uint16 index, uint16 &animation);

	// -- Playback helpers --

	void playFrame();

	/** Get a list of objects ordered by drawing order. */
	Objects getOrderedObjects();

	void clearAnims(); ///< Remove all animation frames.
	void drawAnims();  ///< Draw the animation frames.

	/** Look if we can compact the loop array. */
	void cleanLoops();
};

} // End of namespace Gob

#endif // GOB_PREGOB_SEQFILE_H
