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

#ifndef TOON_ANIM_H
#define TOON_ANIM_H

#include "common/stream.h"
#include "common/array.h"
#include "common/func.h"
#include "graphics/surface.h"

#include "toon/script.h"

namespace Toon {

class Picture;
class ToonEngine;

struct AnimationFrame {
	int16 _x1;
	int16 _y1;
	int16 _x2;
	int16 _y2;
	int32 _ref;
	uint8 *_data;
};

class Animation {
public:
	Animation(ToonEngine *vm);
	~Animation();

	int16 _x1;
	int16 _y1;
	int16 _x2;
	int16 _y2;
	int32 _numFrames;
	int32 _fps;
	AnimationFrame *_frames;
	uint8 *_palette;
	int32 _paletteEntries;
	char _name[32];

	bool loadAnimation(const Common::String &file);
	void drawFrame(Graphics::Surface &surface, int32 frame, int16 x, int16 y);
	void drawFontFrame(Graphics::Surface &surface, int32 frame, int16 x, int16 y, byte *colorMap);
	void drawFrameOnPicture(int32 frame, int16 x, int16 y);
	void drawFrameWithMask(Graphics::Surface &surface, int32 frame, int16 xx, int16 yy, int32 zz, Picture *mask);
	void drawFrameWithMaskAndScale(Graphics::Surface &surface, int32 frame, int16 xx, int16 yy, int32 zz, Picture *mask, int32 scale);
	void drawStrip(int32 offset = 0);
	void applyPalette(int32 offset, int32 srcOffset, int32 numEntries);
	Common::Rect getFrameRect(int32 frame);
	int16 getFrameWidth(int32 frame);
	int16 getFrameHeight(int32 frame);
	int16 getWidth() const;
	int16 getHeight() const;
	Common::Rect getRect();
protected:
	ToonEngine *_vm;
};

enum AnimationInstanceType {
	kAnimationCharacter = 1,
	kAnimationScene = 2,
	kAnimationCursor = 4
};

class AnimationInstance {
public:
	AnimationInstance(ToonEngine *vm, AnimationInstanceType type);
	void update(int32 timeIncrement);
	void render();
	void renderOnPicture();
	void setAnimation(Animation *animation, bool setRange = true);
	void playAnimation();
	void setAnimationRange(int32 rangeStart, int32 rangeEnd);
	void setFps(int32 fps);
	void setLooping(bool enable);
	void stopAnimation();
	void setFrame(int32 position);
	void forceFrame(int32 position);
	void setPosition(int16 x, int16 y, int32 z, bool relative = false);
	Animation *getAnimation() const { return _animation; }
	void setScale(int32 scale, bool align = false);
	void setVisible(bool visible);
	bool getVisible() const { return _visible; }
	void setUseMask(bool useMask);
	void moveRelative(int16 dx, int16 dy, int32 dz);
	void getRect(int16 *x1, int16 *y1, int16 *x2, int16 *y2) const;
	int16 getX() const { return _x; }
	int16 getY() const { return _y; }
	int32 getZ() const { return _z; }
	int16 getX2() const;
	int16 getY2() const;
	int32 getZ2() const;
	int32 getFrame() const { return _currentFrame; }
	void reset();
	void save(Common::WriteStream *stream);
	void load(Common::ReadStream *stream);

	void setId(int32 id) { _id = id; }
	int32 getId() const { return _id; }

	void setX(int16 x, bool relative = false);
	void setY(int16 y, bool relative = false);
	void setZ(int32 z, bool relative = false);
	void setLayerZ(int32 layer);
	int32 getLayerZ() const;

	AnimationInstanceType getType() const { return _type; }

protected:
	int32 _currentFrame;
	int32 _currentTime;
	int32 _fps;
	Animation *_animation;
	int16 _x;
	int16 _y;
	int32 _z;
	int32 _layerZ;
	int32 _rangeStart;
	int32 _rangeEnd;
	int32 _scale;
	int32 _id;

	AnimationInstanceType _type;

	bool _useMask;
	bool _playing;
	bool _looping;
	bool _visible;
	bool _alignBottom;

	ToonEngine *_vm;
};

class AnimationManager {
public:
	AnimationManager(ToonEngine *vm);
	AnimationInstance *createNewInstance(AnimationInstanceType type);
	void addInstance(AnimationInstance *instance);
	void removeInstance(AnimationInstance *instance);
	void updateInstance(AnimationInstance* instance);
	void removeAllInstances(AnimationInstanceType type);
	void render();
	void update(int32 timeIncrement);
	bool hasInstance(AnimationInstance* instance);

protected:
	ToonEngine *_vm;
	Common::Array<AnimationInstance *> _instances;
};

class SceneAnimation {
public:
	AnimationInstance *_originalAnimInstance;
	AnimationInstance *_animInstance;
	Animation *_animation;
	int32 _id;
	bool _active;

	void load(ToonEngine *vm, Common::ReadStream *stream);
	void save(ToonEngine *vm, Common::WriteStream *stream);
};

class SceneAnimationScript {
public:
	EMCData *_data;
	EMCState _state;
	uint32 _lastTimer;
	bool _frozen;
	bool _frozenForConversation;
	bool _active;
};

} // End of namespace Toon

#endif
