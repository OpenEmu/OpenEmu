//
//  MyDocument.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/1/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import "GamePreferencesController.h"
#import "GameDocument.h"
#import "GameDocumentController.h"
#import "GameAudio.h"
#import "GameLayer.h"
#import "GameBuffer.h"
#import "GameCore.h"

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
	[gameCore setup];
	
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
	[audio startAudio];
	[audio setVolume:[[[GameDocumentController sharedDocumentController] preferenceController] volume]];
	
	
	NSRect f = [gameWindow frame];
	
	NSSize aspect;
	
	if([gameCore respondsToSelector:@selector(outputSize)])
	   aspect = [gameCore outputSize];
	else
	   aspect = NSMakeSize([gameBuffer width], [gameBuffer height]);
	[gameWindow setFrame: NSMakeRect(NSMinX(f), NSMinY(f), aspect.width, aspect.height + 22) display:NO];
	
	[rootLayer setNeedsLayout];
	
	[gameCore start];	

	//frameTimer = [NSTimer timerWithTimeInterval:1.0/60.0 target:gameCore selector:@selector(tick) userInfo:nil repeats:YES];
	//[[NSRunLoop currentRunLoop] addTimer: frameTimer forMode: NSRunLoopCommonModes];

	[gameWindow makeKeyAndOrderFront:self];
	
	if([[[GameDocumentController sharedDocumentController] preferenceController] fullScreen])
		[view enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
		
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to write your document to data of the specified type. If the given outError != NULL, ensure that you set *outError when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    // For applications targeted for Panther or earlier systems, you should use the deprecated API -dataRepresentationOfType:. In this case you can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.

    if ( outError != NULL ) {
		*outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
	}
	return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	NSLog(@"%@",self);
	
	GameDocumentController* docControl = [GameDocumentController sharedDocumentController];
	gameCore = [[[[docControl bundleForType: typeName] principalClass] alloc] init];
	gameBuffer = [[GameBuffer alloc] initWithGameCore:gameCore];
	//[gameBuffer setFilter:eFilter_HQ2x];
	[self resetFilter];
	
	NSLog(@"%@",[[docControl bundleForType: typeName] principalClass]);
	if ([gameCore load: [absoluteURL path] withParent: self])
	{
		return YES;
	}
	else
	{
		NSLog(@"Incorrect file");
		return NO;
	}
}


- (void) resetFilter
{
	[gameBuffer setFilter:[[[GameDocumentController sharedDocumentController] preferenceController]filter]];
}

- (void) refreshAudio
{
	[audio advanceBuffer];
}

- (void) refresh
{
	//[gameBuffer updateBuffer];
	[gameLayer display];
	[audio advanceBuffer];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
	GameDocumentController* docControl = [GameDocumentController sharedDocumentController];
	[docControl setGameLoaded:YES];
	[gameCore pause:NO];
	[audio startAudio];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
	if(gameCore != nil && [[[GameDocumentController sharedDocumentController] preferenceController] pauseBackground])
	{
		if(![view isInFullScreenMode])
		{
			@try {				
				[gameCore pause:YES];
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
	[gameCore stop];
	[audio stopAudio];
	[gameCore release];
	gameCore = nil;
	
	GameDocumentController* docControl = [GameDocumentController sharedDocumentController];
	[docControl setGameLoaded:NO];
}
	
- (BOOL)isFullScreen
{
	return [view isInFullScreenMode];
}

- (void)switchFullscreen
{
	if([view isInFullScreenMode])
		[view exitFullScreenModeWithOptions:nil];
	else
		[view enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
}

- (void) setVolume: (float) volume
{
	[audio setVolume: volume];
}

- (void) saveState: (NSString *) fileName
{
	[gameCore saveState: fileName];
}

- (void) loadState: (NSString *) fileName
{
	[gameCore loadState: fileName];
}

- (void) scrambleRam:(int) bytes
{
	for(int i = 0; i < bytes; i++)
		[gameCore setRandomByte];
}

#if 0 // Redundant implementation
- (NSBitmapImageRep*) getRawScreenshot
{
	int width = [gameCore width];
	int height = [gameCore height];
#ifdef __LITTLE_ENDIAN__
	//little endian code
	NSBitmapImageRep *newBitmap = [[NSBitmapImageRep alloc]
								   initWithBitmapDataPlanes:NULL pixelsWide:width
								   pixelsHigh:height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES
								   isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace
								   bitmapFormat:0
								   bytesPerRow:width*4 bitsPerPixel:32];
	
	memcpy([newBitmap bitmapData], [gameCore buffer], width*height*4*sizeof(unsigned char));
	
	unsigned char* debut = [newBitmap bitmapData];
	unsigned char temp = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			//swap Red with Blue
			temp = debut[width*4*i + 4*j];
			debut[width*4*i + 4*j] = debut[width*4*i + 4*j +2];
			debut[width*4*i + 4*j +2] = temp;
			
			//alpha full
			debut[width * 4 * i + 4 * j + 3] = 255;
			
		}
	
#else
	//big endian code
	NSBitmapImageRep *newBitmap = [[NSBitmapImageRep alloc]
								   initWithBitmapDataPlanes:NULL pixelsWide:width
								   pixelsHigh:height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES
								   isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace
								   bitmapFormat:NSAlphaFirstBitmapFormat
								   bytesPerRow:width*4 bitsPerPixel:32];
	
	memcpy([newBitmap bitmapData], [gameCore buffer], width*height*4*sizeof(unsigned char));
	
	unsigned char* debut = [newBitmap bitmapData];
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{			
			//alpha full
			debut[width * 4 * i + 4 * j] = 255;
		}
	
#endif	
	return newBitmap;
}

#else // Proposed implementation

- (NSBitmapImageRep*) getRawScreenshot
{
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
										  bitmapFormat:0
										   bytesPerRow:width * 4
										  bitsPerPixel:32];
	
	memcpy([newBitmap bitmapData], [gameCore buffer], width * height * 4 * sizeof(unsigned char));
	
	unsigned char* debut = [newBitmap bitmapData];

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

#endif

@end
