//
//  GameBuffer.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/15/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class GameCore;
typedef enum
{
	eFilter_None,
	eFilter_Nearest,
	eFilter_Scaler2xGLSL,
	eFilter_Scaler4xGLSL,
	eFilter_HQ2xGLSL,
	eFilter_HQ4xGLSL,
	
} eFilter;

DEPRECATED_ATTRIBUTE
@interface GameBuffer : NSObject {
	unsigned char *buffer;
	
	eFilter filter;
	
	GameCore *gameCore;
	
	int width;
	int height;
	
	int multiplier;
	
	GLenum pixelType;
	GLenum pixelForm;
	GLenum internalForm;
}
- (id)initWithGameCore:(GameCore *)core;

@property(readonly) unsigned char *buffer;
@property eFilter filter;
@property(readonly) GLenum pixelForm;
@property(readonly) GLenum pixelType;
@property(readonly) GLenum internalForm;
@property(readonly) int width;
@property(readonly) int height;
@property(readonly) GameCore *gameCore;

@end
