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

#ifndef GOB_ANIFILE_H
#define GOB_ANIFILE_H

#include "common/system.h"
#include "common/str.h"
#include "common/array.h"
#include "common/list.h"

namespace Common {
	class SeekableSubReadStreamEndian;
}

namespace Gob {

class GobEngine;
class Surface;
class CMPFile;

/** An ANI file, describing an animation.
 *
 *  Used in hardcoded "actiony" parts of gob games.
 *  The principle is similar to an Anim in Scenery (see scenery.cpp), but
 *  instead of referencing indices in the sprites array, ANIs reference sprites
 *  directly by filename.
 */
class ANIFile {
public:
	/** The relative area a frame sprite occupies. */
	struct FrameArea {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	};

	/** An animation within an ANI file. */
	struct Animation {
		Common::String name; ///< The name of the animation.

		uint16 frameCount; ///< The number of frames in this animation.

		int16 x;     ///< The default x position for this animation.
		int16 y;     ///< The default y position for this animation.
		bool transp; ///< Should the animation frames be drawn with transparency?

		int16 deltaX; ///< # of pixels to advance in X direction after each cycle.
		int16 deltaY; ///< # of pixels to advance in Y direction after each cycle.

		/** The relative area each frame sprite occupies. */
		Common::Array<FrameArea> frameAreas;

		uint16 width;  ///< The maximum width of this animation's frames.
		uint16 height; ///< The maximum height of this animation's frames.
	};


	ANIFile(GobEngine *vm, const Common::String &fileName,
	        uint16 width = 320, uint8 bpp = 1);
	~ANIFile();

	/** Return the number of animations in this ANI file. */
	uint16 getAnimationCount() const;

	/** Return the maximum size of all animation frames. */
	void getMaxSize(uint16 &width, uint16 &height) const;

	/** Get this animation's properties. */
	const Animation &getAnimationInfo(uint16 animation) const;

	/** Draw an animation frame. */
	void draw(Surface &dest, uint16 animation, uint16 frame, int16 x, int16 y) const;

	/** Recolor the animation sprites. */
	void recolor(uint8 from, uint8 to);

private:
	typedef Common::Array<CMPFile *> LayerArray;
	typedef Common::Array<Animation> AnimationArray;

	/** A "chunk" of an animation frame. */
	struct AnimationChunk {
		int16 x; ///< The relative x offset of this chunk.
		int16 y; ///< The relative y offset of this chunk.

		uint16 layer; ///< The layer the chunk's sprite is on.
		uint16 part;  ///< The layer part the chunk's sprite is.
	};

	typedef Common::List<AnimationChunk> ChunkList;
	typedef Common::Array<ChunkList>     FrameArray;
	typedef Common::Array<FrameArray>    AnimationFrameArray;


	GobEngine *_vm;

	uint16 _width; ///< The width of a sprite layer.
	uint8  _bpp;   ///< Number of bytes per pixel in a sprite layer.

	byte _hasPadding;

	LayerArray          _layers;     ///< The animation sprite layers.
	AnimationArray      _animations; ///< The animations.
	AnimationFrameArray _frames;     ///< The animation frames.

	uint16 _maxWidth;
	uint16 _maxHeight;


	// Loading helpers

	void load(Common::SeekableSubReadStreamEndian &ani, const Common::String &fileName);

	CMPFile *loadLayer(Common::SeekableSubReadStreamEndian &ani);

	void loadAnimation(Animation &animation, FrameArray &frames,
	                   Common::SeekableSubReadStreamEndian &ani);
	void loadFrames(FrameArray &frames, Common::SeekableSubReadStreamEndian &ani);

	// Drawing helpers

	bool getCoordinates(uint16 layer, uint16 part,
	                    uint16 &left, uint16 &top, uint16 &right, uint16 &bottom) const;

	void drawLayer(Surface &dest, uint16 layer, uint16 part,
	              int16 x, int16 y, int32 transp) const;
};

} // End of namespace Gob

#endif // GOB_ANIFILE_H
