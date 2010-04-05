//
//  OpenEmuDebugHelperAppApp
//  OpenEmu
//
//  Created by vade on 4/4/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "OpenEmuDebugHelperAppApp.h"

// for speedz
#import <OpenGL/CGLMacro.h>

#import "NSString+UUID.h"

@implementation OpenEmuDebugHelperAppApp

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// only run our helper once...
	launchedHelperAlready = NO;
	
	// set up GL and IOSurface shit
	NSOpenGLPixelFormatAttribute attr[] = {NSOpenGLPFAAccelerated, NSOpenGLPFADoubleBuffer , (NSOpenGLPixelFormatAttribute)0 };
		
	NSOpenGLPixelFormat* pFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
	
	if(pFormat != nil)
	{
		glContext = [[NSOpenGLContext alloc] initWithFormat:pFormat shareContext:nil];
		
		if(glContext != nil)
		{
			[glContext setView:glView];
			CGLContextObj cgl_ctx = [glContext CGLContextObj];
			glGenTextures(1, &ioSurfaceTexture);
		}
		else
		{
			NSLog(@"unable to create GL context.. bailing");
			[[NSApplication sharedApplication] terminate:nil];
		}
	}
	else
	{
		NSLog(@"unable to create pixel format.. bailing");
		[[NSApplication sharedApplication] terminate:nil];
	}
	
	// start rendering.
	[self setupTimer];
}

- (void) setupTimer
{
	// CVDisplaylink at some point?
	renderTimer = [NSTimer scheduledTimerWithTimeInterval: (NSTimeInterval) 1/60
											 target: self
										   selector: @selector(render)
										   userInfo: nil
											repeats: YES];
	
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSModalPanelRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode];
}

- (IBAction) launchHelper:(id)sender
{
    if(launchedHelperAlready)
    {
		[self endHelperProcess];
		launchedHelperAlready = NO;
    }
    
    [self startHelperProcess];
    launchedHelperAlready = YES;
}

- (void) startHelperProcess
{
	// check to make sure the Rom path is a valid path;
    NSString *romPath = [romPathField stringValue];
	if([[NSFileManager defaultManager] fileExistsAtPath:romPath])
	{			
		// run our background task. Get our IOSurface ids from its standard out.
		NSString *cliPath = [[NSBundle bundleForClass:[self class]] pathForResource: @"OpenEmuHelperApp" ofType: @""];
		
		// generate a UUID string so we can have multiple screen capture background tasks running.
		taskUUIDForDOServer = [[NSString stringWithUUID] retain];
		// NSLog(@"helper tool UUID should be %@", taskUUIDForDOServer);
		
		NSArray *args = [NSArray arrayWithObjects: cliPath, taskUUIDForDOServer, romPath, nil];
		
		helper = [[TaskWrapper alloc] initWithController:self arguments:args userInfo:nil];
		[helper startProcess];
		
		NSLog(@"launched task with environment: %@", [[helper task] environment]);
		
		// now that we launched the helper, start up our NSConnection for DO object vending and configure it
		// this is however a race condition if our helper process is not fully launched yet. 
		// we hack it out here. Normally this while loop is not noticable, its very fast
		
		taskConnection = nil;
		while(taskConnection == nil)
		{
			taskConnection = [NSConnection connectionWithRegisteredName:[NSString stringWithFormat:@"com.openemu.OpenEmuHelper-%@", taskUUIDForDOServer, nil] host:nil];
			if(taskConnection != nil)
				break;
		}
		
		[taskConnection retain];
		
		// now that we have a valid connection...
		rootProxy = [[taskConnection rootProxy] retain];
		if(rootProxy == nil)
			NSLog(@"nil root proxy object?");
		[rootProxy setProtocolForProxy:@protocol(OpenEmuDOProtocol)];	
	}
}

- (void) endHelperProcess
{
	// kill our background friend
	[helper stopProcess];
	helper = nil;
	
	[rootProxy release];
	rootProxy = nil;
	
	[taskConnection release];
	taskConnection = nil;
}

- (void) render
{
	// probably should do this the right way and register for viewDidChangeSize or whatever notificaiton but.....
	[glContext update];
	
	CGLContextObj cgl_ctx = [glContext CGLContextObj];
	
	// set up our ortho and viewport  
	glViewport(0, 0, [glView frame].size.width ,  [glView frame].size.height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
		
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// according to CGLIOSurface we must rebind our texture every time we want a new stuff from it.
	// since our ID may change every frame we make a new texture each pass. 
	surfaceID = [rootProxy surfaceID];
	
	//	NSLog(@"Surface ID: %u", (NSUInteger) surfaceID);
	
	surfaceRef = IOSurfaceLookup(surfaceID); 
	
	// get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
	if(surfaceRef)
	{			
		// retain our surface incase it gets fucked with elsewhere, it wont go away?
		CFRetain(surfaceRef);
		
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ioSurfaceTexture);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);        

		CGLError err = CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
		if(err != kCGLNoError)
		{
			NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
		}
		
		//TODO: why is our IOSurface upside down?!
		
		glBegin(GL_QUADS);
		glTexCoord2f(IOSurfaceGetWidth(surfaceRef), 0.0);
		glVertex2f(1.0, 1.0);
		
		glTexCoord2f(0.0, 0.0);
		glVertex2f(0.0, 1.0);
		
		glTexCoord2f(0.0, IOSurfaceGetHeight(surfaceRef));
		glVertex2f(0.0, 0.0);
		
		glTexCoord2f(IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef));
		glVertex2f(1.0, 0.0);
		glEnd();
		
		// release the surface 
		CFRelease(surfaceRef);	
	}

	// Restore OpenGL states 
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	[glContext flushBuffer];
}

#pragma mark -
#pragma mark Send to Helper app via DO

- (IBAction) changeVolume:(id)sender
{
	[rootProxy setVolume:[sender floatValue]];
}

- (IBAction) changePlayPause:(id)sender
{
    [rootProxy setPauseEmulation:[sender state] == NSOnState];
}

- (IBAction) pressUp:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_Up];

	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_Up];
}

- (IBAction) pressDown:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_Down];

	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_Down];
}

- (IBAction) pressLeft:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_Left];

	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_Left];
}

- (IBAction) pressRight:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_Right];
	
	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_Right];
}

- (IBAction) pressSelect:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_Select];
	
	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_Select];
}

- (IBAction) pressStart:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_Start];
	
	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_Start];
}

- (IBAction) pressA:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_1];
	
	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_1];
}

- (IBAction) pressB:(id)sender
{
	if([sender state] == NSOnState)
		[rootProxy player:0 didPressButton:OEButton_2];

	else if([sender state] == NSOffState)
		[rootProxy player:0 didReleaseButton:OEButton_2];
}


#pragma mark TaskWrapper delegates

- (void)appendOutput:(NSString *)output fromProcess: (TaskWrapper *)aTask
{
}	

- (void)processStarted: (TaskWrapper *)aTask
{
}

- (void)processFinished: (TaskWrapper *)aTask withStatus: (int)statusCode
{
}


@end

