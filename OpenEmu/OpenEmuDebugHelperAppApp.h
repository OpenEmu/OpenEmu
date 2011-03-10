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

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>

// IOSurface
#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>

// Task launching
#import "OETaskWrapper.h"

#import "OEGameCoreHelper.h"

@interface OpenEmuDebugHelperAppApp : NSResponder <NSApplicationDelegate, OETaskWrapperController>
{
    IBOutlet NSTextField *romPathField;
    
    IBOutlet NSView      *glView;
    
    // controls
    IBOutlet NSSlider    *volumeSlider;
    IBOutlet NSButton    *playPauseButton;
    
    // IOSurface requirements
    IOSurfaceRef          surfaceRef;
    IOSurfaceID           surfaceID;
    
    // GL requirements
    NSOpenGLContext      *glContext;
    GLuint                ioSurfaceTexture; // square pixel, bufferSize texture sent off to our Parent App for display. Yay.

    NSTimer              *renderTimer;
    
    // we will need a way to do IPC, for now its this.
    OETaskWrapper        *helper;
    NSString             *taskUUIDForDOServer;
    NSConnection         *taskConnection;
    
    id<OEGameCoreHelper>  rootProxy;
    
    BOOL                  launchedHelperAlready;
}

- (IBAction)launchHelper:(id)sender;
- (void)startHelperProcess;
- (void)endHelperProcess;

- (void)setupTimer;
- (void)render;


// sent to helper app via DO
- (IBAction)changeVolume:(id)sender;
- (IBAction)changePlayPause:(id)sender;

- (IBAction)pressUp:(id)sender;
- (IBAction)pressDown:(id)sender;
- (IBAction)pressLeft:(id)sender;
- (IBAction)pressRight:(id)sender;
- (IBAction)pressSelect:(id)sender;
- (IBAction)pressStart:(id)sender;
- (IBAction)pressA:(id)sender;
- (IBAction)pressB:(id)sender;

@end
