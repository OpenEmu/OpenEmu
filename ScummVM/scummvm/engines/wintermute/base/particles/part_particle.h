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

#ifndef WINTERMUTE_PARTPARTICLE_H
#define WINTERMUTE_PARTPARTICLE_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/vector2.h"

namespace Wintermute {

class PartEmitter;
class BaseSprite;
class BasePersistenceManager;

class PartParticle : public BaseClass {
public:
	enum TParticleState {
	    PARTICLE_NORMAL, PARTICLE_FADEIN, PARTICLE_FADEOUT
	};

	PartParticle(BaseGame *inGame);
	virtual ~PartParticle(void);

	float _growthRate;
	bool _exponentialGrowth;

	float _rotation;
	float _angVelocity;

	int _alpha1;
	int _alpha2;

	Rect32 _border;
	Vector2 _pos;
	float _posZ;
	Vector2 _velocity;
	float _scale;
	BaseSprite *_sprite;
	uint32 _creationTime;
	int _lifeTime;
	bool _isDead;
	TParticleState _state;

	bool update(PartEmitter *emitter, uint32 currentTime, uint32 timerDelta);
	bool display(PartEmitter *emitter);

	bool setSprite(const Common::String &filename);

	bool fadeIn(uint32 currentTime, int fadeTime);
	bool fadeOut(uint32 currentTime, int fadeTime);

	bool persist(BasePersistenceManager *PersistMgr);
private:
	uint32 _fadeStart;
	int _fadeTime;
	int _currentAlpha;
	int _fadeStartAlpha;
};

} // end of namespace Wintermute

#endif
