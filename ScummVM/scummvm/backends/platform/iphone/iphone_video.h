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

#ifndef BACKENDS_PLATFORM_IPHONE_IPHONE_VIDEO_H
#define BACKENDS_PLATFORM_IPHONE_IPHONE_VIDEO_H

#include <UIKit/UIKit.h>
#include <Foundation/Foundation.h>
#include <QuartzCore/QuartzCore.h>

#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "iphone_keyboard.h"
#include "iphone_common.h"

#include "common/list.h"

@interface iPhoneView : UIView {
	VideoContext _videoContext;

	Common::List<InternalEvent> _events;
	NSLock *_eventLock;
	SoftKeyboard *_keyboardView;

	EAGLContext *_context;
	GLuint _viewRenderbuffer;
	GLuint _viewFramebuffer;
	GLuint _screenTexture;
	GLuint _overlayTexture;
	GLuint _mouseCursorTexture;

	UIDeviceOrientation _orientation;

	GLint _renderBufferWidth;
	GLint _renderBufferHeight;

	GLfloat _gameScreenVertCoords[4 * 2];
	GLfloat _gameScreenTexCoords[4 * 2];
	CGRect _gameScreenRect;

	GLfloat _overlayVertCoords[4 * 2];
	GLfloat _overlayTexCoords[4 * 2];
	CGRect _overlayRect;

	GLfloat _mouseVertCoords[4 * 2];
	GLfloat _mouseTexCoords[4 * 2];
	GLint _mouseHotspotX, _mouseHotspotY;
	GLint _mouseWidth, _mouseHeight;
	GLfloat _mouseScaleX, _mouseScaleY;

	int _scaledShakeOffsetY;

	UITouch *_firstTouch;
	UITouch *_secondTouch;
}

- (id)initWithFrame:(struct CGRect)frame;

- (VideoContext *)getVideoContext;

- (void)drawRect:(CGRect)frame;

- (void)createScreenTexture;
- (void)initSurface;
- (void)setViewTransformation;

- (void)setGraphicsMode;

- (void)updateSurface;
- (void)updateMainSurface;
- (void)updateOverlaySurface;
- (void)updateMouseSurface;
- (void)clearColorBuffer;

- (void)notifyMouseMove;
- (void)updateMouseCursorScaling;
- (void)updateMouseCursor;

- (void)deviceOrientationChanged:(UIDeviceOrientation)orientation;

- (void)applicationSuspend;

- (void)applicationResume;

- (bool)fetchEvent:(InternalEvent *)event;

@end

extern iPhoneView *g_iPhoneViewInstance;

#endif
