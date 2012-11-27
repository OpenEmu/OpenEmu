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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_ANIMATION_H
#define SWORD25_ANIMATION_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/gfx/timedrenderobject.h"

namespace Sword25 {

// Forward declarations
class Kernel;
class AnimationResource;
class AnimationTemplate;
class AnimationDescription;
class InputPersistenceBlock;

class Animation : public TimedRenderObject {
	friend class RenderObject;

private:
	Animation(RenderObjectPtr<RenderObject> parentPtr, const Common::String &fileName);
	Animation(RenderObjectPtr<RenderObject> parentPtr, const AnimationTemplate &template_);
	Animation(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle);

public:
	enum ANIMATION_TYPES {
		AT_ONESHOT,
		AT_LOOP,
		AT_JOJO
	};

	virtual ~Animation();

	void play();
	void pause();
	void stop();
	void setFrame(uint nr);

	virtual void setPos(int x, int y);
	virtual void setX(int x);
	virtual void setY(int y);

	virtual int getX() const;
	virtual int getY() const;
	virtual int getAbsoluteX() const;
	virtual int getAbsoluteY() const;

	/**
	    @brief Setzt den Alphawert der Animation.
	    @param Alpha der neue Alphawert der Animation (0 = keine Deckung, 255 = volle Deckung).
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsAlphaAllowed() true zurückgibt.
	*/
	void setAlpha(int alpha);

	/**
	    @brief Setzt die Modulationfarbe der Animation.
	    @param Color eine 24-Bit Farbe, die die Modulationsfarbe der Animation festlegt.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsColorModulationAllowed() true zurückgibt.
	*/
	void setModulationColor(uint modulationColor);

	/**
	    @brief Setzt den Skalierungsfaktor der Animation.
	    @param ScaleFactor der Faktor um den die Animation in beide Richtungen gestreckt werden soll.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void setScaleFactor(float scaleFactor);

	/**
	    @brief Setzt den Skalierungsfaktor der Animation auf der X-Achse.
	    @param ScaleFactor der Faktor um den die Animation in Richtungen der X-Achse gestreckt werden soll.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void setScaleFactorX(float scaleFactorX);

	/**
	    @brief Setzt den Skalierungsfaktor der Animation auf der Y-Achse.
	    @param ScaleFactor der Faktor um den die Animation in Richtungen der Y-Achse gestreckt werden soll.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void setScaleFactorY(float scaleFactorY);

	/**
	@brief Gibt den Skalierungsfakter der Animation auf der X-Achse zurück.
	@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float getScaleFactorX() const {
		return _scaleFactorX;
	}

	/**
	@brief Gibt den Skalierungsfakter der Animation auf der Y-Achse zurück.
	@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float getScaleFactorY() const {
		return _scaleFactorY;
	}

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

	virtual void frameNotification(int timeElapsed);

	ANIMATION_TYPES     getAnimationType() const;
	int                 getFPS() const;
	int                 getFrameCount() const;
	bool                isScalingAllowed() const;
	bool                isAlphaAllowed() const;
	bool                isColorModulationAllowed() const;
	uint getCurrentFrame() const {
		return _currentFrame;
	}
	const Common::String   &getCurrentAction() const;
	bool isRunning() const {
		return _running;
	}

	typedef bool (*ANIMATION_CALLBACK)(uint);

	void setCallbacks();

protected:
	virtual bool doRender();

private:
	enum Direction {
		FORWARD,
		BACKWARD
	};

	int _relX;
	int _relY;
	float _scaleFactorX;
	float _scaleFactorY;
	uint _modulationColor;
	uint _currentFrame;
	int  _currentFrameTime;
	bool _running;
	bool _finished;
	Direction _direction;
	AnimationResource *_animationResourcePtr;
	uint _animationTemplateHandle;
	bool _framesLocked;

	ANIMATION_CALLBACK _loopPointCallback;
	ANIMATION_CALLBACK _actionCallback;
	ANIMATION_CALLBACK _deleteCallback;

	/**
	    @brief Lockt alle Frames.
	    @return Gibt false zurück, falls nicht alle Frames gelockt werden konnten.
	*/
	bool lockAllFrames();

	/**
	    @brief Unlockt alle Frames.
	    @return Gibt false zurück, falls nicht alles Frames freigegeben werden konnten.
	*/
	bool unlockAllFrames();

	/**
	    @brief Diese Methode aktualisiert die Parameter (Größe, Position) der Animation anhand des aktuellen Frames.

	    Diese Methode muss bei jedem Framewechsel aufgerufen werden damit der RenderObject-Manager immer aktuelle Daten hat.
	*/
	void computeCurrentCharacteristics();

	/**
	    @brief Berechnet den Abstand zwischen dem linken Rand und dem Hotspot auf X-Achse in der aktuellen Darstellung.
	*/
	int computeXModifier() const;

	/**
	    @brief Berechnet den Abstand zwischen dem linken Rand und dem Hotspot auf X-Achse in der aktuellen Darstellung.
	*/
	int computeYModifier() const;

	void initMembers();
	AnimationDescription *getAnimationDescription() const;

	/**
	  * Initializes a new animation resource from an XML file.
	  */
	void initializeAnimationResource(const Common::String &fileName);
};

} // End of namespace Sword25

#endif
