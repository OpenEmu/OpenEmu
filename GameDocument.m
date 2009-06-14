//
//  MyDocument.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/1/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import "GameDocument.h"
#import "PluginInfo.h"
#import "GameDocumentController.h"
#import "GameAudio.h"
#import "GameLayer.h"
#import "GameBuffer.h"
#import "GameCore.h"
#import "OEGameCoreController.h"

@implementation GameDocument

@synthesize gameCore;

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"GameDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
	[gameCore setupEmulation];
	
    [view setNextResponder:gameCore];
    
	//Setup Layer hierarchy
	rootLayer = [CALayer layer];
	
	rootLayer.layoutManager = [CAConstraintLayoutManager layoutManager];
	rootLayer.backgroundColor = CGColorCreateGenericRGB(0.0f,0.0f, 0.0f, 1.0f);
	
	gameLayer = [GameLayer layer];
	
	[gameLayer setBuffer:gameBuffer];
	 
	gameLayer.name = @"game";
	gameLayer.frame = CGRectMake(0,0,1,1);
	[gameLayer addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidX relativeTo:@"superlayer" attribute:kCAConstraintMidX]];
	[gameLayer addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidY relativeTo:@"superlayer" attribute:kCAConstraintMidY]];
	
	NSRect viewRect = [view bounds];
	gameLayer.bounds = CGRectMake(0, 0, viewRect.size.width,  viewRect.size.height);
	//Add the NESLayer to the hierarchy
	[rootLayer addSublayer:gameLayer];
	
	gameLayer.asynchronous = NO;
	
	//Show the layer
	[view setLayer:rootLayer];
	[view setWantsLayer:YES];
	
	audio = [[GameAudio alloc] initWithCore: gameCore];
    
    [audio bind:@"volume"
       toObject:[NSUserDefaultsController sharedUserDefaultsController]
    withKeyPath:@"values.volume"
        options:nil];
    
	[audio startAudio];
	//[audio setVolume:[[[GameDocumentController sharedDocumentController] preferenceController] volume]];
	
	
	NSRect f = [gameWindow frame];
	
	NSSize aspect;
	
	if([gameCore respondsToSelector:@selector(outputSize)])
	   aspect = [gameCore outputSize];
	else
	   aspect = NSMakeSize([gameBuffer width], [gameBuffer height]);
	[gameWindow setFrame: NSMakeRect(NSMinX(f), NSMinY(f), aspect.width, aspect.height + 22) display:NO];
	
	[rootLayer setNeedsLayout];
	
	[gameCore startEmulation];	

	[gameWindow makeKeyAndOrderFront:self];
    
    if([[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.fullScreen"] boolValue])
        [self toggleFullScreen:self];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    if ( outError != NULL ) {
		*outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
	}
	return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	NSLog(@"%@",self);
	
	GameDocumentController* docControl = [GameDocumentController sharedDocumentController];
    PluginInfo *plugin = [docControl pluginForType:typeName];
    gameCore = [[plugin controller] newGameCoreWithDocument:self];
    NSLog(@"gameCore class: %@", [gameCore class]);
    [view setNextResponder:gameCore];
    gameBuffer = [[GameBuffer alloc] initWithGameCore:gameCore];
    [self resetFilter];
    
    if ([gameCore loadFileAtPath: [absoluteURL path]] ) return YES;
    NSLog(@"Incorrect file");
	*outError = [[NSError alloc] initWithDomain:@"Bad file" code:0 userInfo:nil];
    return NO;
}

- (void)setVideoFilter:(eFilter)filterID
{
    [gameBuffer setFilter:filterID];
}

- (void) resetFilter
{
	// FIXME: Need to be adapted to the new system
    //[gameBuffer setFilter:[[[GameDocumentController sharedDocumentController] preferenceController]filter]];
}

- (void) refresh
{	
	[gameLayer display];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
	GameDocumentController* docControl = [GameDocumentController sharedDocumentController];
	[docControl setGameLoaded:YES];
	[gameCore setPauseEmulation:NO];
	[audio startAudio];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if(gameCore != nil && [[[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:@"values.backgroundPause"] boolValue])
    {
        if(![self isFullScreen])
		{
			@try {				
				[gameCore setPauseEmulation:YES];
				[audio pauseAudio];
			}
			@catch (NSException * e) {
				NSLog(@"Failed to pause");
			}
		}
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
	if([view isInFullScreenMode])
		[view exitFullScreenModeWithOptions:nil];
//	[recorder finishRecording];
	[gameCore stopEmulation];
	[audio stopAudio];
	[gameCore release];
	gameCore = nil;
	
	GameDocumentController* docControl = [GameDocumentController sharedDocumentController];
	[docControl setGameLoaded:NO];
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
    NSResponder *previous = [view nextResponder];
    
    if(![view isInFullScreenMode])
        [view enterFullScreenMode:[[view window] screen]
                      withOptions:[NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithBool:NO], NSFullScreenModeAllScreens,
                                   [NSNumber numberWithInt:0], NSFullScreenModeWindowLevel, nil]];
    else
        [view exitFullScreenModeWithOptions:nil];
    [[view window] makeFirstResponder:view];
    
    [previous setNextResponder:[view nextResponder]];
    [view setNextResponder:previous];
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

- (void) saveStateToFile: (NSString *) fileName
{
    if([gameCore respondsToSelector:@selector(saveStateToFileAtPath:)])
        [gameCore saveStateToFileAtPath: fileName];
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

- (void)loadStateFromFile: (NSString *) fileName
{
    if([gameCore respondsToSelector:@selector(loadStateFromFileAtPath:)])
        [gameCore loadStateFromFileAtPath: fileName];
}

- (void)scrambleRam:(int)bytes
{
	for(int i = 0; i < bytes; i++)
		[gameCore setRandomByte];
}

- (NSBitmapImageRep *)getRawScreenshot
{
#ifdef __LITTLE_ENDIAN__
#define BITMAP_FORMAT 0
#else
#define BITMAP_FORMAT NSAlphaFirstBitmapFormat
#endif
    
	int width = [gameCore width];
	int height = [gameCore height];
	//little endian code
	NSBitmapImageRep *newBitmap =
	[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
											pixelsWide:width
											pixelsHigh:height
										 bitsPerSample:8
									   samplesPerPixel:4
											  hasAlpha:YES
											  isPlanar:NO
										colorSpaceName:NSCalibratedRGBColorSpace
										  bitmapFormat:BITMAP_FORMAT
										   bytesPerRow:width * 4
										  bitsPerPixel:32];
	
	memcpy([newBitmap bitmapData], [gameCore videoBuffer], width * height * 4 * sizeof(unsigned char));
	
	unsigned char *debut = [newBitmap bitmapData];

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
#ifdef __LITTLE_ENDIAN__
#define ALPHA 3
			//swap Red with Blue
			unsigned char temp = debut[width * 4 * i + 4 * j];
			debut[width * 4 * i + 4 * j] = debut[width * 4 * i + 4 * j + 2];
			debut[width * 4 * i + 4 * j + 2] = temp;
#else
#define ALPHA 0
#endif			
			//alpha full
			debut[width * 4 * i + 4 * j + ALPHA] = 255;
			
		}
	return newBitmap;
}

@end
