//
//  OEFrameEncodeOperation.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/29/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OEFrameEncodeOperation.h"
#import <QTKit/QTKit.h>

@implementation OEFrameEncodeOperation

@synthesize imageToEncode, movie;

- (id) initWithImage:(NSImage*) image forMovie:(QTMovie*) aMovie  withDuration:(NSTimeInterval) interval
{		
	if( self = [super init] )
	{
		self.imageToEncode = image;
		self.movie = aMovie;
		frameDuration = interval;
	}
	return self;
}

- (void) dealloc
{
	self.imageToEncode = nil;
	self.movie = nil;
	[super dealloc];
}

- (void)main 
{
	NSDictionary *myDict = nil;
	myDict = [NSDictionary dictionaryWithObjectsAndKeys:@"mjpb",
			  QTAddImageCodecType,
			  [NSNumber numberWithLong:codecNormalQuality],
			  QTAddImageCodecQuality,
			  nil];
	
	[movie addImage:imageToEncode forDuration:QTMakeTimeWithTimeInterval(frameDuration) withAttributes:myDict];
}

@end
