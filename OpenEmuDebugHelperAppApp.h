//
//  OpenEmuDebugHelperAppApp.h
//  OpenEmu
//
//  Created by vade on 4/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>

// IOSurface
#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>

// Task launching
#import "TaskWrapper.h"

#import "OpenEmuDOProtocol.h"

@interface OpenEmuDebugHelperAppApp : NSObject <NSApplicationDelegate, TaskWrapperController>
{
	IBOutlet NSTextField* romPath;
	
	IBOutlet NSView* glView;
	
	// controls
	IBOutlet NSSlider* volumeSlider;
	IBOutlet NSButton* playPauseButton;
	
	// IOSurface requirements
	IOSurfaceRef surfaceRef;
	IOSurfaceID	surfaceID;
	
	// GL requirements
	NSOpenGLContext* glContext;
	GLuint ioSurfaceTexture;	// square pixel, screenWidth/height texture sent off to our Parent App for display. Yay.

	NSTimer* renderTimer;
	
	// we will need a way to do IPC, for now its this.
	TaskWrapper *helper;
	NSString* taskUUIDForDOServer;	
	NSConnection* taskConnection;
	
	id rootProxy;
	
	BOOL launchedHelperAlready;
}

- (IBAction) setRomPath:(id)sender;
- (IBAction) launchHelper:(id)sender;
- (void) startHelperProcess;
- (void) endHelperProcess;

- (void) setupTimer;
- (void) render;


// sent to helper app via DO
- (IBAction) setVolume:(id)sender;
- (IBAction) setPlayPause:(id)sender;

@end
