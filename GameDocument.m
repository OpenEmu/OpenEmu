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

#import "GameDocument.h"
#import "OECorePlugin.h"
#import "GameDocumentController.h"
//#import "GameAudio.h"
#import "OEGameLayer.h"
//#import "GameCore.h"
#import "OEGameCoreController.h"
#import "GameQTRecorder.h"

#import "OpenEmuDOProtocol.h"

#import "NSString+UUID.h"

@implementation GameDocument

//@synthesize gameCore;
@synthesize emulatorName, view, gameWindow;

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"GameDocument";
}

static void OE_bindGameLayer(OEGameLayer *gameLayer)
{
    NSUserDefaultsController *ctrl = [NSUserDefaultsController sharedUserDefaultsController];
    [gameLayer bind:@"filterName"   toObject:ctrl withKeyPath:@"values.filterName" options:nil];
    [gameLayer bind:@"vSyncEnabled" toObject:ctrl withKeyPath:@"values.vsync"      options:nil];
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{            
    [gameWindow makeFirstResponder:view];
    [gameWindow setAcceptsMouseMovedEvents:YES];
    
    //recorder = [[GameQTRecorder alloc] initWithGameCore:gameCore];
    //Setup Layer hierarchy
    rootLayer = [CALayer layer];
        
    rootLayer.layoutManager = [CAConstraintLayoutManager layoutManager];
    rootLayer.backgroundColor = CGColorCreateGenericRGB(0.0f,0.0f, 0.0f, 1.0f);
    
    //Show the layer
    [view setLayer:rootLayer];
    [view setWantsLayer:YES];
        
    gameLayer = [OEGameLayer layer];
    [gameLayer setDocController:[GameDocumentController sharedDocumentController]];
    OE_bindGameLayer(gameLayer);
    
    [gameLayer setOwner:self];

	// make sure our game layer knows about our DO object so it can get Surface IDs
	[gameLayer setRootProxy:rootProxy];
	     
    gameLayer.name = @"game";
    gameLayer.frame = CGRectMake(0,0,1,1);
    [gameLayer addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidX relativeTo:@"superlayer" attribute:kCAConstraintMidX]];
    [gameLayer addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidY relativeTo:@"superlayer" attribute:kCAConstraintMidY]];
        	
	NSSize aspect = NSMakeSize([rootProxy screenWidth], [rootProxy screenHeight]);
	
	NSLog(@"Aspect IS: %@", NSStringFromSize(aspect));
    
	rootLayer.bounds = CGRectMake(0, 0, aspect.width, aspect.height);	//[gameCore screenWidth] 
    gameLayer.bounds = CGRectMake(0, 0, aspect.width, aspect.height);
    //Add the NESLayer to the hierarchy
    [rootLayer addSublayer:gameLayer];
    
	gameLayer.asynchronous = YES;
	
    CGFloat scaleFactor = [gameLayer preferredWindowScale];
    [gameWindow setContentSize:NSMakeSize(aspect.width * scaleFactor, aspect.height * scaleFactor)];

    [gameWindow setAspectRatio:aspect];
	[gameWindow center];
	//[gameWindow setContentResizeIncrements:aspect];
    [rootLayer setNeedsLayout];
    
    //[recorder startRecording];
    [gameWindow makeKeyAndOrderFront:self];
    
    if([self defaultsToFullScreenMode])
        [self toggleFullScreen:self];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    if(outError != NULL)
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    return nil;
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

- (BOOL) startHelperProcessWithPath:(NSString*)romPath
{
	// check to make sure the Rom path is a valid path;
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
		
		DLog(@"launched task with environment: %@", [[helper task] environment]);
		
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
				
		//TODO: check to make sure things really launched and are running, before returning YES
		
		return YES;
	}
	
	return NO;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    DLog(@"%@",self);
    
    GameDocumentController *docControl = [GameDocumentController sharedDocumentController];
	OECorePlugin *plugin = nil;
    
	for(OEPlugin *aPlugin in [docControl plugins])
		if([[aPlugin displayName] isEqualToString:typeName])
		{
			plugin = (OECorePlugin*)aPlugin;
			break;
		}

	if( plugin == nil ) return NO;
	
    emulatorName = [[plugin displayName] retain];
    //gameCore = [[plugin controller] newGameCoreWithDocument:self];
    
   // DLog(@"gameCore class: %@", [gameCore class]);
    //[gameWindow makeFirstResponder:gameCore];
     
    //if([gameCore loadFileAtPath:[absoluteURL path]]) return YES;
    
	if([self startHelperProcessWithPath:[absoluteURL path]]) return YES;
	
	// load our TasWapper friend here:
	
	
    NSLog(@"Incorrect file");
    if(outError != NULL) *outError = [[NSError alloc] initWithDomain:@"Bad file" code:0 userInfo:nil];
    
    return NO;
}

- (void)refresh
{    
   // [gameLayer setNeedsDisplay];
}

- (BOOL)backgroundPauses
{
    return [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.backgroundPause"] boolValue];
}

- (BOOL)defaultsToFullScreenMode
{
    return [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.fullScreen"] boolValue];
}

- (BOOL)isEmulationPaused
{
    return [rootProxy isEmulationPaused];
}

- (void)setPauseEmulation:(BOOL)flag
{	
	[rootProxy setPauseEmulation:flag];

	if (flag)
	{
		[playPauseToolbarItem setImage:[NSImage imageNamed:NSImageNameRightFacingTriangleTemplate]];
        [playPauseToolbarItem setLabel:@"Play"];
	}
	else
	{
		[playPauseToolbarItem setImage:[NSImage imageNamed:NSImageNameStopProgressTemplate]];
        [playPauseToolbarItem setLabel:@"Pause"];
	}
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    [[GameDocumentController sharedDocumentController] setGameLoaded:YES];
    if ([self backgroundPauses])
		[self setPauseEmulation:NO];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if([self backgroundPauses])
    {
        if(![self isFullScreen])
        {
            @try {
                [self setPauseEmulation:YES];
            }
            @catch (NSException * e) {
                NSLog(@"Failed to pause");
            }
        }
    }
}

/*- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)proposedFrameSize
{
    // We want to force aspect ratio with resize increments
    int scale;
    if(proposedFrameSize.width < proposedFrameSize.height)
        scale = proposedFrameSize.width / [gameCore screenWidth];
    else
        scale = proposedFrameSize.height / [gameCore screenHeight];
    scale = MAX(scale, 1);
    
    NSRect newContentRect = NSMakeRect(0,0, [gameCore screenWidth] * scale, [gameCore screenHeight] * scale);
    return [sender frameRectForContentRect:newContentRect].size;
}*/

- (void)windowDidResize:(NSNotification *)notification
{
    //adjust the window to zoom from the center
    if ([gameWindow isZoomed])
        [gameWindow center];
    
    [gameLayer setNeedsDisplay];
}

- (void)windowWillClose:(NSNotification *)notification
{
    if([view isInFullScreenMode]) [self toggleFullScreen:self];
 //   [gameCore stopEmulation];
 //   [audio stopAudio];
 //   [gameCore release];
  //  gameCore = nil;
    
	
	[self endHelperProcess];
	
    //[recorder finishRecording];
    [gameLayer setDocController:nil];
	[gameLayer release];
    [[GameDocumentController sharedDocumentController] setGameLoaded:NO];
}

- (void)performClose:(id)sender
{
    [gameWindow performClose:sender];
}
    
- (BOOL)isFullScreen
{
    return [view isInFullScreenMode];
}

- (IBAction)toggleFullScreen:(id)sender
{
    [self setPauseEmulation:YES];
    if(![view isInFullScreenMode])
    {
        [view enterFullScreenMode:[[view window] screen]
                      withOptions:[NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithBool:NO], NSFullScreenModeAllScreens, nil]];
        [NSCursor hide];
    }
    else
    {
        [view exitFullScreenModeWithOptions:nil];           
        [NSCursor unhide];
    }
    [self setPauseEmulation:NO];
   // [[view window] makeFirstResponder:gameCore];
}

- (IBAction)saveState:(id)sender
{ 
    [[NSSavePanel savePanel] beginSheetForDirectory:nil
                                               file:nil 
                                     modalForWindow:gameWindow
                                      modalDelegate:self
                                     didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:)
                                        contextInfo:NULL];
}

- (void)saveStateToFile:(NSString *)fileName
{
//    if([gameCore respondsToSelector:@selector(saveStateToFileAtPath:)])
//        [gameCore saveStateToFileAtPath: fileName];
}

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if(returnCode == NSOKButton) [self saveStateToFile:[sheet filename]];
}

- (IBAction)loadState:(id)sender
{
    [[NSOpenPanel openPanel] beginSheetForDirectory:nil
                                               file:nil
                                     modalForWindow:gameWindow
                                      modalDelegate:self
                                     didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                                        contextInfo:NULL];
}

- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if(returnCode == NSOKButton) [self loadStateFromFile:[panel filename]];
}

- (void)loadStateFromFile:(NSString *)fileName
{
 //   if([gameCore respondsToSelector:@selector(loadStateFromFileAtPath:)])
//        [gameCore loadStateFromFileAtPath: fileName];
}

- (IBAction)scrambleRam:(id)sender
{
    [self scrambleBytesInRam:100];
}

- (void)scrambleBytesInRam:(NSUInteger)bytes
{
//    for(NSUInteger i = 0; i < bytes; i++)
//        [gameCore setRandomByte];
}

- (IBAction)resetGame:(id)sender
{
//    [gameCore resetEmulation];
}

- (IBAction)playPauseGame:(id)sender
{
	[self setPauseEmulation:![self isEmulationPaused]];
}

- (NSImage *)screenShot
{
    return [gameLayer imageForCurrentFrame];
}

#pragma mark TaskWrapper delegates

- (void)appendOutput:(NSString *)output fromProcess: (TaskWrapper *)aTask
{
    printf("%s", [output UTF8String]);
}	

- (void)processStarted: (TaskWrapper *)aTask
{
}

- (void)processFinished: (TaskWrapper *)aTask withStatus: (int)statusCode
{
}

@end
