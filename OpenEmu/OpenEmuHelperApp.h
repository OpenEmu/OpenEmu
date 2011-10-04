/*
 Copyright (c) 2010, OpenEmu Team
 
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


#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <IOSurface/IOSurface.h>
#import <CoreVideo/CoreVideo.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLIOSurface.h>

// DO  object
#import "OEGameCoreHelper.h"
#import "OEGameCore.h"

// we are going to be cheap and just use gameCore for now.

@class OEGameCore, OECorePlugin;
@class OEGameCoreController;
@class OEGameAudio;

extern NSString *const OEHelperServerNamePrefix;
extern NSString *const OEHelperProcessErrorDomain;

enum _OEHelperAppErrorCodes 
{
    OEHelperNoError                      =  0,
    OEHelperCouldNotStartConnectionError = -1,
    OEHelperIncorrectFileError           = -2,
};

@interface OpenEmuHelperApp : NSResponder <NSApplicationDelegate, OEGameCoreHelper, OERenderDelegate>
{
    NSRunningApplication *parentApplication; // the process id of the parent app (Open Emu or our debug helper)
    
    // IOSurface requirements
    IOSurfaceRef   surfaceRef;
    IOSurfaceID    surfaceID;
    
    // GL Requirements
    CGLContextObj     glContext;
    GLuint            gameTexture;      // this is the texture that is defined by the gameCores pixelFormat and type
    GLuint            gameFBO;          // this FBO uses the IOSurfaceTexture as an attachment and renders the gameTexture to 'square pixels'
    GLuint            ioSurfaceTexture; // square pixel, bufferSize texture sent off to our Parent App for display. Yay.
        
    // poll parent ID, KVO does not seem to be working with NSRunningApplication
    NSTimer          *pollingTimer;
    
    // we will need a way to do IPC, for now its DO.
    NSString         *doUUID;
    NSConnection     *theConnection;
    
    // OE stuff
    NSArray          *plugins;
    NSArray          *validExtensions;
    OEGameCore       *gameCore;
    OEGameAudio      *gameAudio;
    
    BOOL              loadedRom;
    
    // screen subrect stuff
    id <OEGameCoreHelperDelegate> delegate;
    OEIntSize previousScreenSize, correctedSize;
    float     gameAspectRatio;
    BOOL      drawSquarePixels;
    BOOL      running;
}

@property(retain) NSString *doUUID;
@property(assign) BOOL      loadedRom;
@property(readonly, assign, getter=isRunning) BOOL running;

- (BOOL)launchConnectionWithIdentifierSuffix:(NSString *)aSuffix error:(NSError **)anError;
- (void)setupProcessPollingTimer;
- (void)quitHelperTool;

#pragma mark -
#pragma mark IOSurface and GL Render
- (void)setupOpenGLOnScreen:(NSScreen*)screen;
- (void)setupIOSurface;
- (void)setupFBO;
- (void)setupGameTexture;
- (void)pollParentProcess;
- (void)setupGameCore;
- (void)updateGameTexture;

//- (void)drawIntoIOSurface;
- (void)beginDrawToIOSurface;
- (void)drawGameTexture;
- (void)endDrawToIOSurface;
- (void)destroySurface;
- (void)updateScreenSize;
- (void)stopEmulation;

#pragma mark -
#pragma mark OE DO protocol delegate methods
- (void)setVolume:(float)volume;
- (void)setPauseEmulation:(BOOL)paused;

#pragma mark -
#pragma mark OE Render Delegate protocol methods
- (void)willExecute;
- (void)didExecute;
@end
