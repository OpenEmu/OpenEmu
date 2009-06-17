/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
