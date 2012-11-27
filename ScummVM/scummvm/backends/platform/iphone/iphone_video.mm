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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "iphone_video.h"

#include "graphics/colormasks.h"

iPhoneView *g_iPhoneViewInstance = nil;
static int g_fullWidth;
static int g_fullHeight;

static int g_needsScreenUpdate = 0;

#if 0
static long g_lastTick = 0;
static int g_frames = 0;
#endif

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(const char *file, int line) {
	int retCode = 0;

	// returns 1 if an OpenGL error occurred, 0 otherwise.
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		fprintf(stderr, "glError: %u (%s: %d)\n", glErr, file, line);
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

bool iPhone_isHighResDevice() {
	return g_fullHeight > 480;
}

void iPhone_updateScreen() {
	//printf("Mouse: (%i, %i)\n", mouseX, mouseY);
	if (!g_needsScreenUpdate) {
		g_needsScreenUpdate = 1;
		[g_iPhoneViewInstance performSelectorOnMainThread:@selector(updateSurface) withObject:nil waitUntilDone: NO];
	}
}

bool iPhone_fetchEvent(InternalEvent *event) {
	return [g_iPhoneViewInstance fetchEvent:event];
}

uint getSizeNextPOT(uint size) {
	if ((size & (size - 1)) || !size) {
		int log = 0;

		while (size >>= 1)
			++log;

		size = (2 << log);
	}

	return size;
}

const char *iPhone_getDocumentsDir() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	return [documentsDirectory UTF8String];
}

@implementation iPhoneView

+ (Class)layerClass {
	return [CAEAGLLayer class];
}

- (VideoContext *)getVideoContext {
	return &_videoContext;
}

- (void)createContext {
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
	                                [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];

	_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

	// In case creating the OpenGL ES context failed, we will error out here.
	if (_context == nil) {
		fprintf(stderr, "Could not create OpenGL ES context\n");
		exit(-1);
	}

	if ([EAGLContext setCurrentContext:_context]) {
		glGenFramebuffersOES(1, &_viewFramebuffer); printOpenGLError();
		glGenRenderbuffersOES(1, &_viewRenderbuffer); printOpenGLError();

		glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer); printOpenGLError();
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();
		[_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self.layer];

		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();

		// Retrieve the render buffer size. This *should* match the frame size,
		// i.e. g_fullWidth and g_fullHeight.
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_renderBufferWidth); printOpenGLError();
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_renderBufferHeight); printOpenGLError();

		if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
			NSLog(@"Failed to make complete framebuffer object %x.", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
			return;
		}

		_videoContext.overlayHeight = _renderBufferWidth;
		_videoContext.overlayWidth = _renderBufferHeight;
		uint overlayTextureWidth = getSizeNextPOT(_videoContext.overlayHeight);
		uint overlayTextureHeight = getSizeNextPOT(_videoContext.overlayWidth);

		// Since the overlay size won't change the whole run, we can
		// precalculate the texture coordinates for the overlay texture here
		// and just use it later on.
		_overlayTexCoords[2] = _overlayTexCoords[6] = _videoContext.overlayWidth / (GLfloat)overlayTextureWidth;
		_overlayTexCoords[5] = _overlayTexCoords[7] = _videoContext.overlayHeight / (GLfloat)overlayTextureHeight;

		_videoContext.overlayTexture.create(overlayTextureWidth, overlayTextureHeight, Graphics::createPixelFormat<5551>());

		glViewport(0, 0, _renderBufferWidth, _renderBufferHeight); printOpenGLError();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); printOpenGLError();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D); printOpenGLError();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY); printOpenGLError();
		glEnableClientState(GL_VERTEX_ARRAY); printOpenGLError();
	}
}

- (id)initWithFrame:(struct CGRect)frame {
	self = [super initWithFrame: frame];

	if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
		if ([self respondsToSelector:@selector(setContentScaleFactor:)]) {
			[self setContentScaleFactor:[[UIScreen mainScreen] scale]];
		}
	}

	g_fullWidth = (int)frame.size.width;
	g_fullHeight = (int)frame.size.height;

	g_iPhoneViewInstance = self;

	_keyboardView = nil;
	_screenTexture = 0;
	_overlayTexture = 0;
	_mouseCursorTexture = 0;

	_scaledShakeOffsetY = 0;

	_firstTouch = NULL;
	_secondTouch = NULL;

	_eventLock = [[NSLock alloc] init];

	_gameScreenVertCoords[0] = _gameScreenVertCoords[1] =
	    _gameScreenVertCoords[2] = _gameScreenVertCoords[3] =
	    _gameScreenVertCoords[4] = _gameScreenVertCoords[5] =
	    _gameScreenVertCoords[6] = _gameScreenVertCoords[7] = 0;

	_gameScreenTexCoords[0] = _gameScreenTexCoords[1] =
	    _gameScreenTexCoords[2] = _gameScreenTexCoords[3] =
	    _gameScreenTexCoords[4] = _gameScreenTexCoords[5] =
	    _gameScreenTexCoords[6] = _gameScreenTexCoords[7] = 0;

	_overlayVertCoords[0] = _overlayVertCoords[1] =
	    _overlayVertCoords[2] = _overlayVertCoords[3] =
	    _overlayVertCoords[4] = _overlayVertCoords[5] =
	    _overlayVertCoords[6] = _overlayVertCoords[7] = 0;

	_overlayTexCoords[0] = _overlayTexCoords[1] =
	    _overlayTexCoords[2] = _overlayTexCoords[3] =
	    _overlayTexCoords[4] = _overlayTexCoords[5] =
	    _overlayTexCoords[6] = _overlayTexCoords[7] = 0;

	_mouseVertCoords[0] = _mouseVertCoords[1] =
	    _mouseVertCoords[2] = _mouseVertCoords[3] =
	    _mouseVertCoords[4] = _mouseVertCoords[5] =
	    _mouseVertCoords[6] = _mouseVertCoords[7] = 0;

	_mouseTexCoords[0] = _mouseTexCoords[1] =
	    _mouseTexCoords[2] = _mouseTexCoords[3] =
	    _mouseTexCoords[4] = _mouseTexCoords[5] =
	    _mouseTexCoords[6] = _mouseTexCoords[7] = 0;

	// Initialize the OpenGL ES context
	[self createContext];

	return self;
}

- (void)dealloc {
	if (_keyboardView != nil) {
		[_keyboardView dealloc];
	}

	_videoContext.screenTexture.free();
	_videoContext.overlayTexture.free();
	_videoContext.mouseTexture.free();

	[_eventLock dealloc];
	[super dealloc];
}

- (void)drawRect:(CGRect)frame {
#if 0
	if (g_lastTick == 0) {
		g_lastTick = time(0);
	}

	g_frames++;
	if (time(0) > g_lastTick) {
		g_lastTick = time(0);
		printf("FPS: %i\n", g_frames);
		g_frames = 0;
	}
#endif
}

- (void)setFilterModeForTexture:(GLuint)tex {
	if (!tex)
		return;

	glBindTexture(GL_TEXTURE_2D, tex); printOpenGLError();

	GLint filter = GL_LINEAR;

	switch (_videoContext.graphicsMode) {
	case kGraphicsModeLinear:
		filter = GL_LINEAR;
		break;

	case kGraphicsModeNone:
		filter = GL_NEAREST;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); printOpenGLError();
	// We use GL_CLAMP_TO_EDGE here to avoid artifacts when linear filtering
	// is used. If we would not use this for example the cursor in Loom would
	// have a line/border artifact on the right side of the covered rect.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); printOpenGLError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); printOpenGLError();
}

- (void)setGraphicsMode {
	[self setFilterModeForTexture:_screenTexture];
	[self setFilterModeForTexture:_overlayTexture];
	[self setFilterModeForTexture:_mouseCursorTexture];
}

- (void)updateSurface {
	if (!g_needsScreenUpdate) {
		return;
	}
	g_needsScreenUpdate = 0;

	glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();

	[self updateMainSurface];

	if (_videoContext.overlayVisible)
		[self updateOverlaySurface];

	if (_videoContext.mouseIsVisible)
		[self updateMouseSurface];

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();
	[_context presentRenderbuffer:GL_RENDERBUFFER_OES];

}

- (void)notifyMouseMove {
	const GLint mouseX = (GLint)(_videoContext.mouseX * _mouseScaleX) - _mouseHotspotX;
	const GLint mouseY = (GLint)(_videoContext.mouseY * _mouseScaleY) - _mouseHotspotY;

	_mouseVertCoords[0] = _mouseVertCoords[4] = mouseX;
	_mouseVertCoords[1] = _mouseVertCoords[3] = mouseY;
	_mouseVertCoords[2] = _mouseVertCoords[6] = mouseX + _mouseWidth;
	_mouseVertCoords[5] = _mouseVertCoords[7] = mouseY + _mouseHeight;
}

- (void)updateMouseCursorScaling {
	CGRect *rect;
	int maxWidth, maxHeight;

	if (!_videoContext.overlayVisible) {
		rect = &_gameScreenRect;
		maxWidth = _videoContext.screenWidth;
		maxHeight = _videoContext.screenHeight;
	} else {
		rect = &_overlayRect;
		maxWidth = _videoContext.overlayWidth;
		maxHeight = _videoContext.overlayHeight;
	}

	if (!maxWidth || !maxHeight) {
		printf("WARNING: updateMouseCursorScaling called when screen was not ready (%d)!\n", _videoContext.overlayVisible);
		return;
	}

	_mouseScaleX = CGRectGetWidth(*rect) / (GLfloat)maxWidth;
	_mouseScaleY = CGRectGetHeight(*rect) / (GLfloat)maxHeight;

	_mouseWidth = (GLint)(_videoContext.mouseWidth * _mouseScaleX);
	_mouseHeight = (GLint)(_videoContext.mouseHeight * _mouseScaleY);

	_mouseHotspotX = (GLint)(_videoContext.mouseHotspotX * _mouseScaleX);
	_mouseHotspotY = (GLint)(_videoContext.mouseHotspotY * _mouseScaleY);

	// We subtract the screen offset to the hotspot here to simplify the
	// screen offset handling in the mouse code. Note the subtraction here
	// makes sure that the offset actually gets added to the mouse position,
	// since the hotspot offset is substracted from the position.
	_mouseHotspotX -= (GLint)CGRectGetMinX(*rect);
	_mouseHotspotY -= (GLint)CGRectGetMinY(*rect);

	// FIXME: For now we also adapt the mouse position here. In reality we
	// would be better off to also adjust the event position when switching
	// from overlay to game screen or vica versa.
	[self notifyMouseMove];
}

- (void)updateMouseCursor {
	if (_mouseCursorTexture == 0) {
		glGenTextures(1, &_mouseCursorTexture); printOpenGLError();
		[self setFilterModeForTexture:_mouseCursorTexture];
	}

	[self updateMouseCursorScaling];

	_mouseTexCoords[2] = _mouseTexCoords[6] = _videoContext.mouseWidth / (GLfloat)_videoContext.mouseTexture.w;
	_mouseTexCoords[5] = _mouseTexCoords[7] = _videoContext.mouseHeight / (GLfloat)_videoContext.mouseTexture.h;

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _videoContext.mouseTexture.w, _videoContext.mouseTexture.h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _videoContext.mouseTexture.pixels); printOpenGLError();
}

- (void)updateMainSurface {
	glVertexPointer(2, GL_FLOAT, 0, _gameScreenVertCoords); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, _gameScreenTexCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _screenTexture); printOpenGLError();

	// Unfortunately we have to update the whole texture every frame, since glTexSubImage2D is actually slower in all cases
	// due to the iPhone internals having to convert the whole texture back from its internal format when used.
	// In the future we could use several tiled textures instead.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _videoContext.screenTexture.w, _videoContext.screenTexture.h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, _videoContext.screenTexture.pixels); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateOverlaySurface {
	glVertexPointer(2, GL_FLOAT, 0, _overlayVertCoords); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, _overlayTexCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _overlayTexture); printOpenGLError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _videoContext.overlayTexture.w, _videoContext.overlayTexture.h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, _videoContext.overlayTexture.pixels); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)updateMouseSurface {
	glVertexPointer(2, GL_FLOAT, 0, _mouseVertCoords); printOpenGLError();
	glTexCoordPointer(2, GL_FLOAT, 0, _mouseTexCoords); printOpenGLError();

	glBindTexture(GL_TEXTURE_2D, _mouseCursorTexture); printOpenGLError();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); printOpenGLError();
}

- (void)setUpOrientation:(UIDeviceOrientation)orientation width:(int *)width height:(int *)height {
	_orientation = orientation;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// We always force the origin (0,0) to be in the upper left corner.
	switch (_orientation) {
	case UIDeviceOrientationLandscapeRight:
		glRotatef( 90, 0, 0, 1); printOpenGLError();
		glOrthof(0, _renderBufferHeight, _renderBufferWidth, 0, 0, 1); printOpenGLError();

		*width = _renderBufferHeight;
		*height = _renderBufferWidth;
		break;

	case UIDeviceOrientationLandscapeLeft:
		glRotatef(-90, 0, 0, 1); printOpenGLError();
		glOrthof(0, _renderBufferHeight, _renderBufferWidth, 0, 0, 1); printOpenGLError();

		*width = _renderBufferHeight;
		*height = _renderBufferWidth;
		break;

	case UIDeviceOrientationPortrait:
	default:
		// We must force the portrait orientation here, since we might not know
		// the real orientation.
		_orientation = UIDeviceOrientationPortrait;

		glOrthof(0, _renderBufferWidth, _renderBufferHeight, 0, 0, 1); printOpenGLError();

		*width = _renderBufferWidth;
		*height = _renderBufferHeight;
		break;
	}
}

- (void)createScreenTexture {
	const uint screenTexWidth = getSizeNextPOT(_videoContext.screenWidth);
	const uint screenTexHeight = getSizeNextPOT(_videoContext.screenHeight);

	_gameScreenTexCoords[2] = _gameScreenTexCoords[6] = _videoContext.screenWidth / (GLfloat)screenTexWidth;
	_gameScreenTexCoords[5] = _gameScreenTexCoords[7] = _videoContext.screenHeight / (GLfloat)screenTexHeight;

	_videoContext.screenTexture.create(screenTexWidth, screenTexHeight, Graphics::createPixelFormat<565>());
}

- (void)initSurface {
	int screenWidth, screenHeight;
	[self setUpOrientation:[[UIDevice currentDevice] orientation] width:&screenWidth height:&screenHeight];

	if (_screenTexture > 0) {
		glDeleteTextures(1, &_screenTexture); printOpenGLError();
	}

	glGenTextures(1, &_screenTexture); printOpenGLError();
	[self setFilterModeForTexture:_screenTexture];

	if (_overlayTexture > 0) {
		glDeleteTextures(1, &_overlayTexture); printOpenGLError();
	}

	glGenTextures(1, &_overlayTexture); printOpenGLError();
	[self setFilterModeForTexture:_overlayTexture];

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer); printOpenGLError();

	[self clearColorBuffer];

	if (_keyboardView != nil) {
		[_keyboardView removeFromSuperview];
		[[_keyboardView inputView] removeFromSuperview];
	}

	GLfloat adjustedWidth = _videoContext.screenWidth;
	GLfloat adjustedHeight = _videoContext.screenHeight;
	if (_videoContext.asprectRatioCorrection) {
		if (_videoContext.screenWidth == 320 && _videoContext.screenHeight == 200)
			adjustedHeight = 240;
		else if (_videoContext.screenWidth == 640 && _videoContext.screenHeight == 400)
			adjustedHeight = 480;
	}

	float overlayPortraitRatio;

	if (_orientation == UIDeviceOrientationLandscapeLeft || _orientation ==  UIDeviceOrientationLandscapeRight) {
		GLfloat gameScreenRatio = adjustedWidth / adjustedHeight;
		GLfloat screenRatio = (GLfloat)screenWidth / (GLfloat)screenHeight;

		// These are the width/height according to the portrait layout!
		int rectWidth, rectHeight;
		int xOffset, yOffset;

		if (gameScreenRatio < screenRatio) {
			// When the game screen ratio is less than the screen ratio
			// we need to scale the width, since the game screen was higher
			// compared to the width than our output screen is.
			rectWidth = (int)(screenHeight * gameScreenRatio);
			rectHeight = screenHeight;
			xOffset = (screenWidth - rectWidth) / 2;
			yOffset = 0;
		} else {
			// When the game screen ratio is bigger than the screen ratio
			// we need to scale the height, since the game screen was wider
			// compared to the height than our output screen is.
			rectWidth = screenWidth;
			rectHeight = (int)(screenWidth / gameScreenRatio);
			xOffset = 0;
			yOffset = (screenHeight - rectHeight) / 2;
		}

		//printf("Rect: %i, %i, %i, %i\n", xOffset, yOffset, rectWidth, rectHeight);
		_gameScreenRect = CGRectMake(xOffset, yOffset, rectWidth, rectHeight);
		overlayPortraitRatio = 1.0f;
	} else {
		GLfloat ratio = adjustedHeight / adjustedWidth;
		int height = (int)(screenWidth * ratio);
		//printf("Making rect (%u, %u)\n", screenWidth, height);
		_gameScreenRect = CGRectMake(0, 0, screenWidth, height);

		CGRect keyFrame = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
		if (_keyboardView == nil) {
			_keyboardView = [[SoftKeyboard alloc] initWithFrame:keyFrame];
			[_keyboardView setInputDelegate:self];
		}

		[self addSubview:[_keyboardView inputView]];
		[self addSubview: _keyboardView];
		[[_keyboardView inputView] becomeFirstResponder];
		overlayPortraitRatio = (_videoContext.overlayHeight * ratio) / _videoContext.overlayWidth;
	}

	_overlayRect = CGRectMake(0, 0, screenWidth, screenHeight * overlayPortraitRatio);

	_gameScreenVertCoords[0] = _gameScreenVertCoords[4] = CGRectGetMinX(_gameScreenRect);
	_gameScreenVertCoords[1] = _gameScreenVertCoords[3] = CGRectGetMinY(_gameScreenRect);
	_gameScreenVertCoords[2] = _gameScreenVertCoords[6] = CGRectGetMaxX(_gameScreenRect);
	_gameScreenVertCoords[5] = _gameScreenVertCoords[7] = CGRectGetMaxY(_gameScreenRect);

	_overlayVertCoords[2] = _overlayVertCoords[6] = CGRectGetMaxX(_overlayRect);
	_overlayVertCoords[5] = _overlayVertCoords[7] = CGRectGetMaxY(_overlayRect);

	[self setViewTransformation];
	[self updateMouseCursorScaling];
}

- (void)setViewTransformation {
	// Set the modelview matrix. This matrix will be used for the shake offset
	// support.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Scale the shake offset according to the overlay size. We need this to
	// adjust the overlay mouse click coordinates when an offset is set.
	_scaledShakeOffsetY = (int)(_videoContext.shakeOffsetY / (GLfloat)_videoContext.screenHeight * CGRectGetHeight(_overlayRect));

	// Apply the shakeing to the output screen.
	glTranslatef(0, -_scaledShakeOffsetY, 0);
}

- (void)clearColorBuffer {
	// The color buffer is triple-buffered, so we clear it multiple times right away to avid doing any glClears later.
	int clearCount = 5;
	while (clearCount-- > 0) {
		glClear(GL_COLOR_BUFFER_BIT); printOpenGLError();
		[_context presentRenderbuffer:GL_RENDERBUFFER_OES];
	}
}

- (void)addEvent:(InternalEvent)event {
	[_eventLock lock];
	_events.push_back(event);
	[_eventLock unlock];
}

- (bool)fetchEvent:(InternalEvent *)event {
	[_eventLock lock];
	if (_events.empty()) {
		[_eventLock unlock];
		return false;
	}

	*event = *_events.begin();
	_events.pop_front();
	[_eventLock unlock];
	return true;
}

/**
 * Converts portrait mode coordinates into rotated mode coordinates.
 */
- (bool)convertToRotatedCoords:(CGPoint)point result:(CGPoint *)result {
	switch (_orientation) {
	case UIDeviceOrientationLandscapeLeft:
		result->x = point.y;
		result->y = _renderBufferWidth - point.x;
		return true;

	case UIDeviceOrientationLandscapeRight:
		result->x = _renderBufferHeight - point.y;
		result->y = point.x;
		return true;

	case UIDeviceOrientationPortrait:
		result->x = point.x;
		result->y = point.y;
		return true;

	default:
		return false;
	}
}

- (bool)getMouseCoords:(CGPoint)point eventX:(int *)x eventY:(int *)y {
	if (![self convertToRotatedCoords:point result:&point])
		return false;

	CGRect *area;
	int width, height, offsetY;
	if (_videoContext.overlayVisible) {
		area = &_overlayRect;
		width = _videoContext.overlayWidth;
		height = _videoContext.overlayHeight;
		offsetY = _scaledShakeOffsetY;
	} else {
		area = &_gameScreenRect;
		width = _videoContext.screenWidth;
		height = _videoContext.screenHeight;
		offsetY = _videoContext.shakeOffsetY;
	}

	point.x = (point.x - CGRectGetMinX(*area)) / CGRectGetWidth(*area);
	point.y = (point.y - CGRectGetMinY(*area)) / CGRectGetHeight(*area);

	*x = (int)(point.x * width);
	// offsetY describes the translation of the screen in the upward direction,
	// thus we need to add it here.
	*y = (int)(point.y * height + offsetY);

	// Clip coordinates
	if (*x < 0 || *x > width || *y < 0 || *y > height)
		return false;

	return true;
}

- (void)deviceOrientationChanged:(UIDeviceOrientation)orientation {
	switch (orientation) {
	case UIDeviceOrientationLandscapeLeft:
	case UIDeviceOrientationLandscapeRight:
	case UIDeviceOrientationPortrait:
		_orientation = orientation;
		break;

	default:
		return;
	}

	[self addEvent:InternalEvent(kInputOrientationChanged, orientation, 0)];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	int x, y;

	switch ([allTouches count]) {
	case 1: {
		UITouch *touch = [touches anyObject];
		CGPoint point = [touch locationInView:self];
		if (![self getMouseCoords:point eventX:&x eventY:&y])
			return;

		_firstTouch = touch;
		[self addEvent:InternalEvent(kInputMouseDown, x, y)];
		break;
		}

	case 2: {
		UITouch *touch = [touches anyObject];
		CGPoint point = [touch locationInView:self];
		if (![self getMouseCoords:point eventX:&x eventY:&y])
			return;

		_secondTouch = touch;
		[self addEvent:InternalEvent(kInputMouseSecondDown, x, y)];
		break;
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	//NSSet *allTouches = [event allTouches];
	int x, y;

	for (UITouch *touch in touches) {
		if (touch == _firstTouch) {
			CGPoint point = [touch locationInView:self];
			if (![self getMouseCoords:point eventX:&x eventY:&y])
				return;

			[self addEvent:InternalEvent(kInputMouseDragged, x, y)];
		} else if (touch == _secondTouch) {
			CGPoint point = [touch locationInView:self];
			if (![self getMouseCoords:point eventX:&x eventY:&y])
				return;

			[self addEvent:InternalEvent(kInputMouseSecondDragged, x, y)];
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	int x, y;

	switch ([allTouches count]) {
	case 1: {
		UITouch *touch = [[allTouches allObjects] objectAtIndex:0];
		CGPoint point = [touch locationInView:self];
		if (![self getMouseCoords:point eventX:&x eventY:&y])
			return;

		[self addEvent:InternalEvent(kInputMouseUp, x, y)];
		break;
		}

	case 2: {
		UITouch *touch = [[allTouches allObjects] objectAtIndex:1];
		CGPoint point = [touch locationInView:self];
		if (![self getMouseCoords:point eventX:&x eventY:&y])
			return;

		[self addEvent:InternalEvent(kInputMouseSecondUp, x, y)];
		break;
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
}

- (void)handleKeyPress:(unichar)c {
	[self addEvent:InternalEvent(kInputKeyPressed, c, 0)];
}

- (BOOL)canHandleSwipes {
	return TRUE;
}

- (int)swipe:(int)num withEvent:(struct __GSEvent *)event {
	//printf("swipe: %i\n", num);

	[self addEvent:InternalEvent(kInputSwipe, num, 0)];
	return 0;
}

- (void)applicationSuspend {
	[self addEvent:InternalEvent(kInputApplicationSuspended, 0, 0)];
}

- (void)applicationResume {
	[self addEvent:InternalEvent(kInputApplicationResumed, 0, 0)];
}

@end
