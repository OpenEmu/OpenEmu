//
//  GameBuffer.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/15/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GameCore.h"

typedef enum
{
	eFilter_None,
	eFilter_Scaler2x,
	eFilter_Scaler3x,
	eFilter_HQ2x,
	eFilter_HQ3x,
	
	// GLSL filters
	
	eFilter_Scaler2xGLSL,
	eFilter_Scaler4xGLSL,
	eFilter_HQ2xGLSL,
	eFilter_HQ4xGLSL,
	
}eFilter;


@interface GameBuffer : NSObject {
	unsigned char* buffer;
	unsigned char* filterBuffer;
	
	eFilter filter;
	
	id <GameCore> gameCore;
	
	int width;
	int height;
	
	int multiplier;
	
	GLenum pixelType;
	GLenum pixelForm;
	GLenum internalForm;
}
+ (unsigned short) convertPixel: (unsigned int) pixel;
+ (unsigned char*) convertTo16bpp: (int *) buffer width: (int) width height: (int) height;

- (void) updateBuffer;
- (unsigned char*) buffer;
- (void) setFilter: (eFilter) filter;
- (eFilter) filter;
- (id) initWithGameCore: (id <GameCore>) core;

//@property(readonly) unsigned char* buffer;

@property(readonly) GLenum pixelForm;
@property(readonly) GLenum pixelType;
@property(readonly) GLenum internalForm;
@property(readonly) int width;
@property(readonly) int height;
@property(readonly) id <GameCore> gameCore;

@end
