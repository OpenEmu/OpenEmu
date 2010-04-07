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

// for speedz
#import <OpenGL/CGLMacro.h>


#import "OpenEmuHelperApp.h"
#import "NSString+UUID.h"

// Open Emu
#import "GameCore.h"
#import "GameAudio.h"
#import "OECorePlugin.h"


@implementation OpenEmuHelperApp

@synthesize doUUID;
@synthesize romPath;
@synthesize loadedRom, surfaceID;

#pragma mark -

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{	
	parentID = getppid();
		
	// our distributed object to vend
	//doObject = [OpenEmuHelperApp sharedDistributedObject];
	//[doObject setDelegate:self];
	
	// unique server name per plugin instance
	theConnection = [[NSConnection new] retain];
	[theConnection setRootObject:self];
	if ([theConnection registerName:[NSString stringWithFormat:@"com.openemu.OpenEmuHelper-%@", doUUID, nil]] == NO) 
	{
		NSLog(@"Error opening NSConnection - exiting");
	}
	else
		NSLog(@"NSConnection Open");	

	// Cache OE results
	plugins = [[OECorePlugin allPlugins] retain];
	validExtensions = [[OECorePlugin supportedTypeExtensions] retain];

	// load the rom.
	[self loadRom:self.romPath];
	[gameAudio setVolume:1.0];
	
	// init resources
	[self setupOpenGLOnScreen:[NSScreen mainScreen]];
	
	[self setupIOSurface];
	[self setupFBO];
	[self setUpGameTexture];
	
	// being rendering
	[self setupTimer];
}

- (void) quitHelperTool
{
	[[NSApplication sharedApplication] terminate:nil];
}

#pragma mark -
#pragma mark IOSurface and GL Render
- (void) setupOpenGLOnScreen:(NSScreen*) screen
{	
	// init our fullscreen context.
	CGLPixelFormatAttribute attributes[] = {kCGLPFAAllRenderers, 0};
	
	CGLError err = kCGLNoError;
	CGLPixelFormatObj pf;
	GLint numPixelFormats = 0;
	
	NSLog(@"choosing pixel format");
	err = CGLChoosePixelFormat(attributes, &pf, &numPixelFormats);
	
	if(err != kCGLNoError)
	{
		NSLog(@"Error choosing pixel format %s", CGLErrorString(err));
		[[NSApplication sharedApplication] terminate:nil];
	}
	CGLRetainPixelFormat(pf);
	
	NSLog(@"creating context");
	
	err = CGLCreateContext(pf, NULL, &glContext);
	if(err != kCGLNoError)
	{
		NSLog(@"Error creating context %s", CGLErrorString(err));
		[[NSApplication sharedApplication] terminate:nil];
	}
	CGLRetainContext(glContext);
	
}

- (void) setupIOSurface
{
	// init our texture and IOSurface
	NSMutableDictionary* surfaceAttributes = [[NSMutableDictionary alloc] init];
	[surfaceAttributes setObject:[NSNumber numberWithBool:YES] forKey:(NSString*)kIOSurfaceIsGlobal];
	[surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)gameCore.screenWidth] forKey:(NSString*)kIOSurfaceWidth];
	[surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)gameCore.screenHeight] forKey:(NSString*)kIOSurfaceHeight];
	[surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)4] forKey:(NSString*)kIOSurfaceBytesPerElement];
	
	// TODO: do we need to ensure openGL Compatibility and CALayer compatibility? 
	
	surfaceRef =  IOSurfaceCreate((CFDictionaryRef) surfaceAttributes);
	[surfaceAttributes release];
	
	// make a new texture.
	CGLContextObj cgl_ctx = glContext;	
	
	glGenTextures(1, &ioSurfaceTexture);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ioSurfaceTexture);
	
	// TODO: this is probably not right to rely on screenWidth/height. 
	// for example Nestopia's returned values depend on NTSC being enabled or not.
	// we should probably have some sort of gameCore protocol for maxWidth maxHeight possible, and render into a sub-section of that.
	CGLError err = CGLTexImageIOSurface2D(glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, (GLsizei)gameCore.screenWidth, (GLsizei) gameCore.screenHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
	if(err != kCGLNoError)
	{
		NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
	}
	glFlush();
}

// make an FBO and bind out IOSurface backed texture to it
- (void) setupFBO
{
	DLog(@"creating FBO");
		
	GLenum status;

	CGLContextObj cgl_ctx = glContext;
	
	// Create temporary FBO to render in texture 
	glGenFramebuffersEXT(1, &gameFBO);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gameFBO);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, ioSurfaceTexture, 0);
	
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{    
		NSLog(@"Cannot create FBO");
		NSLog(@"OpenGL error %04X", status);
		
		glDeleteFramebuffersEXT(1, &gameFBO);
	}    
}

- (void) setUpGameTexture
{
	GLenum status;

	CGLContextObj cgl_ctx = glContext;
	
	glEnable(GL_TEXTURE_RECTANGLE_EXT);    
	// create our texture 
	glGenTextures(1, &gameTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
	
	status = glGetError();
	if(status)
	{
		NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);
	}
	
	// with storage hints & texture range -- assuming image depth should be 32 (8 bit rgba + 8 bit alpha ?) 
	glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT,  [gameCore bufferWidth] * [gameCore bufferHeight] * 4, [gameCore videoBuffer]); 
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	
	// proper tex params.
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	
	glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameCore internalPixelFormat], [gameCore bufferWidth], [gameCore bufferHeight], 0, [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
	
	status = glGetError();
	if(status)
	{
		NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
		glDeleteTextures(1, &gameTexture);
		gameTexture = 0;
	}
	
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	
}
	 
- (void) setupTimer
{
	// CVDisplaylink at some point?
	timer = [NSTimer scheduledTimerWithTimeInterval: (NSTimeInterval) 1/60
											 target: self
										   selector: @selector(render)
										   userInfo: nil
											repeats: YES];
}

- (void) render
{
	// check to see if our parent process ID is still around... is not, bail.
	if([NSRunningApplication runningApplicationWithProcessIdentifier:parentID] == nil)
		[self quitHelperTool];
		
	if([gameCore frameFinished])
	{
		[self updateGameTexture];
		[self correctPixelAspectRatio];
	}
}

- (void) updateGameTexture
{	
	CGLContextObj cgl_ctx = glContext;
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gameTexture);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, [gameCore bufferWidth], [gameCore bufferHeight], [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]); 
}

- (void) correctPixelAspectRatio
{
    // the size of our output image, we may need/want to put in accessors for texture coord
    // offsets from the game core should the image we want be 'elsewhere' within the main texture. 
    CGRect cropRect = [gameCore sourceRect];

	CGLContextObj cgl_ctx = glContext;
	        
    // bind our FBO / and thus our IOSurface
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gameFBO);
    
    // Assume FBOs JUST WORK, because we checked on startExecution    
    //GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);    
    //if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {    
        // Setup OpenGL states 
        glViewport(0, 0, gameCore.screenWidth,  gameCore.screenHeight);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, gameCore.screenWidth, 0, gameCore.screenHeight, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // dont bother clearing. we dont have any alpha so we just write over the buffer contents. saves us an expensive write.
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);        
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
        
        // do a nearest linear interp.
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);        
		
        glColor4f(1.0, 1.0, 1.0, 1.0);
		
        // why do we need it ?
        glDisable(GL_BLEND);
        
        glBegin(GL_QUADS);    // Draw A Quad
        {
            glMultiTexCoord2f(GL_TEXTURE0, cropRect.origin.x, cropRect.origin.y);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, cropRect.size.width + cropRect.origin.x, cropRect.origin.y);
            // glTexCoord2f(pixelsWide, 0.0f );
            glVertex3f(gameCore.screenWidth, 0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, cropRect.size.width + cropRect.origin.x, cropRect.size.height + cropRect.origin.y);
            // glTexCoord2f(pixelsWide, pixelsHigh);
            glVertex3f(gameCore.screenWidth, gameCore.screenHeight, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, cropRect.origin.x, cropRect.size.height + cropRect.origin.y);
            // glTexCoord2f(0.0f, pixelsHigh);
            glVertex3f(0.0f, gameCore.screenHeight, 0.0f);
        }
        glEnd(); // Done Drawing The Quad
        
        // Restore OpenGL states 
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    // flush to make sure IOSurface updates are seen in parent app.
	glFlushRenderAPPLE();
	
	// get the updated surfaceID to pass to STDOut...
	surfaceID = IOSurfaceGetID(surfaceRef);
	
	// TODO: Do vending
	// let DO handle IPC IO, if we have a valid ID to output.
	//[doObject setSurfaceID:surfaceID];
		
    // Check for OpenGL errors 
    /*    status = glGetError();
	 if(status)
	 {
	 NSLog(@"FrameBuffer OpenGL error %04X", status);
	 }
     */
}

- (void) destroySurface
{
	CFRelease(surfaceRef);
	surfaceRef = nil;	
	
	CGLContextObj cgl_ctx = glContext;
	
	glDeleteTextures(1, &ioSurfaceTexture);
	glDeleteTextures(1, &gameTexture);
	glDeleteFramebuffersEXT(1, &gameFBO);
	
	glFlush();
}

#pragma mark -
#pragma mark Game Core methods

- (OECorePlugin*) pluginForType:(NSString *)extension
{
    for(OECorePlugin *plugin in plugins)
        if([plugin supportsFileType:extension])
            return plugin;
    return nil;
}

- (BOOL) loadRom:(NSString *)path
{
    NSString *theRomPath = [path stringByStandardizingPath];
    BOOL isDir;    
    
    DLog(@"New ROM path is: %@",theRomPath);
    
    if([[NSFileManager defaultManager] fileExistsAtPath:theRomPath isDirectory:&isDir] && !isDir)
    {
        NSString *extension = [theRomPath pathExtension];
        DLog(@"extension is: %@", extension);
        
        // cleanup
        if(self.loadedRom)
        {
            [gameCore stopEmulation];
            [gameAudio stopAudio];
            [gameCore release];
            [gameAudio release];
            
            DLog(@"released/cleaned up for new ROM");            
        }
        self.loadedRom = NO;
        
        OECorePlugin *plugin = [self pluginForType:extension];
        
        gameCoreController = [plugin controller];
        gameCore = [gameCoreController newGameCore];
        
        DLog(@"Loaded bundle. About to load rom...");
        
        if([gameCore loadFileAtPath:theRomPath])
        {
            DLog(@"Loaded new Rom: %@", theRomPath);
            [gameCore setupEmulation];
            
            // audio!
            gameAudio = [[GameAudio alloc] initWithCore:gameCore];
            DLog(@"initialized audio");
            
            // starts the threaded emulator timer
            [gameCore startEmulation];
            
            DLog(@"About to start audio");
            [gameAudio startAudio];
            
            DLog(@"finished loading/starting rom");
            return self.loadedRom = YES;
        }    
        else NSLog(@"ROM did not load.");
    }
    else NSLog(@"bad ROM path or filename");
    return NO;
}

#pragma mark -
#pragma mark OE DO Delegate methods
- (void) setVolume:(float)volume
{
	[gameAudio setVolume:volume];

}

- (void) setPauseEmulation:(BOOL)paused
{
	if(paused)
	{
		[gameAudio pauseAudio];
		[gameCore setPauseEmulation:YES]; 
	}
	else
	{
		[gameAudio startAudio];
		[gameCore setPauseEmulation:NO]; 
	}
}

- (void)player:(NSUInteger)playerNumber didPressButton:(OEButton)button
{
	NSLog(@"did Press Button");
	[gameCore player:playerNumber didPressButton:button];
}

- (void)player:(NSUInteger)playerNumber didReleaseButton:(OEButton)button
{
	NSLog(@"did Release Button");
	[gameCore player:playerNumber didReleaseButton:button];
}

- (void)postEvent:(bycopy NSEvent *)theEvent
{
    switch([theEvent type])
    {
        case NSLeftMouseDown         : [gameCore mouseDown:            theEvent]; break;
        case NSLeftMouseUp           : [gameCore mouseUp:              theEvent]; break;
        case NSRightMouseDown        : [gameCore rightMouseDown:       theEvent]; break;
        case NSRightMouseUp          : [gameCore rightMouseUp:         theEvent]; break;
        case NSMouseMoved            : [gameCore mouseMoved:           theEvent]; break;
        case NSLeftMouseDragged      : [gameCore mouseDragged:         theEvent]; break;
        case NSRightMouseDragged     : [gameCore rightMouseDragged:    theEvent]; break;
        case NSMouseEntered          : [gameCore mouseEntered:         theEvent]; break;
        case NSMouseExited           : [gameCore mouseExited:          theEvent]; break;
        case NSKeyDown               : [gameCore keyDown:              theEvent]; break;
        case NSKeyUp                 : [gameCore keyUp:                theEvent]; break;
        case NSFlagsChanged          : [gameCore flagsChanged:         theEvent]; break;
        case NSCursorUpdate          : [gameCore cursorUpdate:         theEvent]; break;
        case NSScrollWheel           : [gameCore scrollWheel:          theEvent]; break;
        case NSTabletPoint           : [gameCore tabletPoint:          theEvent]; break;
        case NSTabletProximity       : [gameCore tabletProximity:      theEvent]; break;
        case NSOtherMouseDown        : [gameCore otherMouseDown:       theEvent]; break;
        case NSOtherMouseUp          : [gameCore otherMouseUp:         theEvent]; break;
        case NSOtherMouseDragged     : [gameCore otherMouseDragged:    theEvent]; break;
        case NSEventTypeMagnify      : [gameCore magnifyWithEvent:     theEvent]; break;
        case NSEventTypeSwipe        : [gameCore swipeWithEvent:       theEvent]; break;
        case NSEventTypeRotate       : [gameCore rotateWithEvent:      theEvent]; break;
        case NSEventTypeBeginGesture : [gameCore beginGestureWithEvent:theEvent]; break;
        case NSEventTypeEndGesture   : [gameCore endGestureWithEvent:  theEvent]; break;
    }
}

@end

#pragma mark -
#pragma mark main

OpenEmuHelperApp *helper;

int main (int argc, const char * argv[])
{
	NSLog(@"Helper tool UUID is: %s", argv[1]);
	NSLog(@"Helper tool romPath is: %s", argv[2]);
	
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSApplication *app = [NSApplication sharedApplication];
	helper	= [[OpenEmuHelperApp alloc] init];
	
	[app setDelegate:helper];
	[helper setDoUUID:[NSString stringWithCString:argv[1] encoding:NSUTF8StringEncoding]];
	[helper setRomPath:[NSString stringWithCString:argv[2] encoding:NSUTF8StringEncoding]];
	
	[app run];	
	
    [pool release];
	[app release];
	
	return 0;
}
