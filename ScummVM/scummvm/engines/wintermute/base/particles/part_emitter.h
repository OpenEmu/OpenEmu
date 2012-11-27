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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_PARTEMITTER_H
#define WINTERMUTE_PARTEMITTER_H


#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/particles/part_force.h"

namespace Wintermute {
class BaseRegion;
class PartParticle;
class PartEmitter : public BaseObject {
public:
	DECLARE_PERSISTENT(PartEmitter, BaseObject)

	PartEmitter(BaseGame *inGame, BaseScriptHolder *Owner);
	virtual ~PartEmitter(void);

	int _fadeOutTime;

	bool start();

	bool update();
	bool display() { return display(NULL); } // To avoid shadowing the inherited display-function.
	bool display(BaseRegion *region);

	bool sortParticlesByZ();
	bool addSprite(const char *filename);
	bool removeSprite(const char *filename);
	bool setBorder(int x, int y, int width, int height);
	bool setBorderThickness(int thicknessLeft, int thicknessRight, int thicknessTop, int thicknessBottom);

	bool addForce(const Common::String &name, PartForce::TForceType type, int posX, int posY, float angle, float strength);
	bool removeForce(const Common::String &name);

	BaseArray<PartForce *> _forces;

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();


private:
	int _width;
	int _height;

	int _angle1;
	int _angle2;

	float _rotation1;
	float _rotation2;

	float _angVelocity1;
	float _angVelocity2;

	float _growthRate1;
	float _growthRate2;
	bool _exponentialGrowth;

	float _velocity1;
	float _velocity2;
	bool _velocityZBased;

	float _scale1;
	float _scale2;
	bool _scaleZBased;

	int _maxParticles;

	int _lifeTime1;
	int _lifeTime2;
	bool _lifeTimeZBased;

	int _genInterval;
	int _genAmount;

	bool _running;
	int _overheadTime;

	int _maxBatches;
	int _batchesGenerated;

	Rect32 _border;
	int _borderThicknessLeft;
	int _borderThicknessRight;
	int _borderThicknessTop;
	int _borderThicknessBottom;

	int _fadeInTime;

	int _alpha1;
	int _alpha2;
	bool _alphaTimeBased;

	bool _useRegion;

	char *_emitEvent;
	BaseScriptHolder *_owner;

	PartForce *addForceByName(const Common::String &name);
	int static compareZ(const void *obj1, const void *obj2);
	bool initParticle(PartParticle *particle, uint32 currentTime, uint32 timerDelta);
	bool updateInternal(uint32 currentTime, uint32 timerDelta);
	uint32 _lastGenTime;
	BaseArray<PartParticle *> _particles;
	BaseArray<char *> _sprites;
};

} // end of namespace Wintermute

#endif
