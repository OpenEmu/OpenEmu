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

#import "GameQTRecorder.h"
#import "OEFrameEncodeOperation.h"

@implementation GameQTRecorder
//#if !__LP64__
@synthesize recording;


static NSTimeInterval currentTime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (t.tv_usec / 1000000.0);
}


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
	
	timer = [[NSTimer timerWithTimeInterval:1.0/30.0f target:self selector:@selector(addFrame) userInfo:nil repeats:true] retain];
	[[NSRunLoop currentRunLoop] addTimer: timer forMode: NSRunLoopCommonModes];	 	

	lastTime = currentTime();
	encodingQueue = [[NSOperationQueue alloc] init];
	[encodingQueue setMaxConcurrentOperationCount:1];
	//[[[NSThread alloc] initWithTarget: self selector: @selector(timerCallInstallLoop) object: nil] start];
}

-(void) timerCallInstallLoop
{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		//[NSThread setThreadPriority:1.0];
		//add NTSC/PAL timer
		
		timer = [[NSTimer timerWithTimeInterval:1.0f/10.0f target:self selector:@selector(addFrame) userInfo:nil repeats:true] retain];
		[[NSRunLoop currentRunLoop] addTimer: timer forMode: NSRunLoopCommonModes];	 	
		
		[[NSRunLoop currentRunLoop] run];
		[pool release];
}

- (void) addFrame
{	
	
	NSTimeInterval time = currentTime();

	OEFrameEncodeOperation* op = [[OEFrameEncodeOperation alloc] initWithImage:[(GameDocument*)[gameCore document] screenShot] forMovie:movie withDuration:time-lastTime ];
	[encodingQueue addOperation:op];
	lastTime = time;
	/*NSDictionary *myDict = nil;
	myDict = [NSDictionary dictionaryWithObjectsAndKeys:@"SVQ3",
			  QTAddImageCodecType,
			  [NSNumber numberWithLong:codecMinQuality],
			  QTAddImageCodecQuality,
			  nil];
	
	[movie addImage:[(GameDocument*)[gameCore document] screenShot] forDuration:QTMakeTime(24,  600) withAttributes:myDict];*/
}


-(void) finishRecording
{
	[timer invalidate];
	
	[encodingQueue waitUntilAllOperationsAreFinished];
	
	QTMovie* audioTrackMovie = [QTMovie movieWithFile:@"/Users/jweinberg/temp.caf" error:nil];
	[audioTrackMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
	NSArray *videoTracks = [movie tracks];//:QTMediaTypeVideo];
	QTTrack *videoTrack = nil;
	if( [videoTracks count] > 0 )
	{
		videoTrack = [videoTracks objectAtIndex:0];
	}
	
	if( videoTrack )
	{
		QTTimeRange videoRange;
		videoRange.time = QTZeroTime;
		videoRange.duration = [[movie attributeForKey:QTMovieDurationAttribute] QTTimeValue];
		
		QTTimeRange audioRange;
		audioRange.time = QTZeroTime;
		audioRange.duration = [[audioTrackMovie attributeForKey:QTMovieDurationAttribute] QTTimeValue];
		
		[audioTrackMovie insertSegmentOfTrack: videoTrack fromRange: videoRange scaledToRange: audioRange ];
	}
	

	
	BOOL result = [audioTrackMovie writeToFile:@"/Users/jweinberg/test.mov" withAttributes:    [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] 
																												 forKey:QTMovieFlatten]];
	if(!result)
	{
		NSLog(@"Couldn't write movie to file");
	}
	
	//[self writeMovieToFile:@"/Users/jweinberg/test.mov" withComponent:[[self availableComponents] objectAtIndex:9] withExportSettings:[self getExportSettings]];
	[movie release];
}
/*
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
*/

- (BOOL)writeMovieToFile:(NSString *)file withComponent:(NSDictionary *)component withExportSettings:(NSData *)exportSettings
{
/*NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								[NSNumber numberWithBool:YES], QTMovieExport,
								[component objectForKey:@"subtype"], QTMovieExportType,
								[component objectForKey:@"manufacturer"], QTMovieExportManufacturer,
								exportSettings, QTMovieExportSettings,
								nil];*/

		
	BOOL result = [movie writeToFile:file withAttributes:[NSDictionary 
														  dictionaryWithObject: [NSNumber numberWithBool: YES] forKey: QTMovieFlatten]];

	if(!result)
	{
		NSLog(@"Couldn't write movie to file");
		return NO;
	}
	
	return YES;
}
//#endif
@end
