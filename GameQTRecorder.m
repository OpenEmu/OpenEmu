//
//  GameQTRecorder.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 2/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//


#import "GameQTRecorder.h"


@implementation GameQTRecorder

@synthesize recording;

- (id) initWithGameCore: (GameCore*) core
{
	self = [super init];
	if(self)
	{
		recording = NO;
		gameCore = core;
	}
	return self;
}

- (void) startRecording
{
	
	recording = YES;
	// generate a name for our movie file
	NSString *tempName = [NSString stringWithCString:tmpnam(nil) 
											encoding:[NSString defaultCStringEncoding]];

	
	// Create a QTMovie with a writable data reference
	movie = [[QTMovie alloc] initToWritableFile:tempName error:NULL];
	
	
	[movie setAttribute:[NSNumber numberWithBool:YES] 
				 forKey:QTMovieEditableAttribute];
	
	
	[[[NSThread alloc] initWithTarget: self selector: @selector(timerCallInstallLoop) object: nil] start];
}

-(void) timerCallInstallLoop
{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		//[NSThread setThreadPriority:1.0];
		//add NTSC/PAL timer
		
		timer = [[NSTimer timerWithTimeInterval:1.0/24.0 target:self selector:@selector(addFrame) userInfo:nil repeats:true] retain];
		[[NSRunLoop currentRunLoop] addTimer: timer forMode: NSRunLoopCommonModes];	 	
		
		[[NSRunLoop currentRunLoop] run];
		[pool release];
}

- (void) addFrame
{
	int width = [gameCore width];
	int height = [gameCore height];
	
	NSBitmapImageRep *newBitmap = [[NSBitmapImageRep alloc]
								   initWithBitmapDataPlanes:NULL pixelsWide:width
								   pixelsHigh:height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES
								   isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace
								   bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
								   bytesPerRow:width*4 bitsPerPixel:32];
	
	memcpy([newBitmap bitmapData], [gameCore videoBuffer], width * height * 4 * sizeof(unsigned char));
	
	uint32* buf = (uint32*)[newBitmap bitmapData];
	uint32 temp = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x <  width; x++)
		{
			temp = buf[x + (y * width)];
			unsigned char R = ((temp >> 16) & 0xFF);
			unsigned char G = ((temp >> 8) & 0xFF);
			unsigned char B = ((temp >> 0) & 0xFF);
			unsigned char A = 255;
			
			temp = (A << 24) | ( B << 16 ) | (G << 8 ) | R;
			buf[x + (y * width)] = temp;
		}
	}

	
	
	NSImage *image = [[NSImage alloc] initWithSize:[newBitmap size]];
	[image addRepresentation:newBitmap];

	NSDictionary *myDict = nil;
	myDict = [NSDictionary dictionaryWithObjectsAndKeys:@"tiff",
			  QTAddImageCodecType,
			  [NSNumber numberWithLong:codecHighQuality],
			  QTAddImageCodecQuality,
			  nil];
	
	[movie addImage:image forDuration:QTMakeTime(24,  600) withAttributes:myDict];
}


-(void) finishRecording
{
	[timer invalidate];
	
	NSArray* components = [self availableComponents];
	
	for(NSDictionary* d in components)
	{
		NSLog(@"%@", [d valueForKey:@"name"]);
	}
	
	// setup the proper export attributes in a dictionary
	NSDictionary	*dict = nil;
	dict = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] 
									   forKey:QTMovieFlatten];
	
	[self writeMovieToFile:@"/Users/jweinberg/test.mov" withComponent:[[self availableComponents] objectAtIndex:9] withExportSettings:[self getExportSettings]];
	[movie release];
}

- (NSArray *)availableComponents
{
	NSMutableArray *array = [NSMutableArray array];
	
	ComponentDescription cd;
	Component c = NULL;
	
	cd.componentType = MovieExportType;
	cd.componentSubType = 0;
	cd.componentManufacturer = 0;
	cd.componentFlags = canMovieExportFiles;
	cd.componentFlagsMask = canMovieExportFiles;
	
	while((c = FindNextComponent(c, &cd)))
	{
		Handle name = NewHandle(4);
		ComponentDescription exportCD;
		
		if (GetComponentInfo(c, &exportCD, name, nil, nil) == noErr)
		{
			char *namePStr = *name;
			NSString *nameStr = [[NSString alloc] initWithBytes:&namePStr[1] length:namePStr[0] encoding:NSUTF8StringEncoding];
			
			NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
										nameStr, @"name",
										[NSData dataWithBytes:&c length:sizeof(c)], @"component",
										[NSNumber numberWithLong:exportCD.componentType], @"type",
										[NSNumber numberWithLong:exportCD.componentSubType], @"subtype",
										[NSNumber numberWithLong:exportCD.componentManufacturer], @"manufacturer",
										nil];
			[array addObject:dictionary];
			[nameStr release];
		}
		
		DisposeHandle(name);
	}
	return array;
}

- (NSData *)getExportSettings
{
	Component c;
	memcpy(&c, [[[[self availableComponents] objectAtIndex:9] objectForKey:@"component"] bytes], sizeof(c));
	
	Movie theMovie = [movie quickTimeMovie] ;
	TimeValue duration = GetMovieDuration(theMovie) ;
	
	Boolean canceled;
	MovieExportComponent exporter; 
	OSErr err = noErr;
	
	err = OpenAComponent(c, &exporter);
	if (err != noErr) {
        NSLog(@"error in OpenADefaultComponent");
		//  goto bail;
    }
    
    err = MovieExportDoUserDialog(exporter, theMovie, NULL, 0, duration, &canceled);
    if (canceled || err != noErr) {
        NSLog(@"error or cancel in MovieExportDoUserDialog");
    }

	if(err)
	{
		NSLog(@"Got error %d when calling MovieExportDoUserDialog",err);
		CloseComponent(exporter);
		return nil;
	}
	
	if(canceled)
	{
		CloseComponent(exporter);
		return nil;
	}
	
	QTAtomContainer settings;
	err = MovieExportGetSettingsAsAtomContainer(exporter, &settings);
	
	if(err)
	{
		NSLog(@"Got error %d when calling MovieExportGetSettingsAsAtomContainer",err);
		CloseComponent(exporter);
		return nil;
	}
	
	NSData *data = [NSData dataWithBytes:*settings length:GetHandleSize(settings)];

	DisposeHandle(settings);
	CloseComponent(exporter);
	
	return data;
}

- (BOOL)writeMovieToFile:(NSString *)file withComponent:(NSDictionary *)component withExportSettings:(NSData *)exportSettings
{
	NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								[NSNumber numberWithBool:YES], QTMovieExport,
								[component objectForKey:@"subtype"], QTMovieExportType,
								[component objectForKey:@"manufacturer"], QTMovieExportManufacturer,
								exportSettings, QTMovieExportSettings,
								nil];

	BOOL result = [movie writeToFile:file withAttributes:attributes];

	if(!result)
	{
		NSLog(@"Couldn't write movie to file");
		return NO;
	}
	
	return YES;
}

@end
