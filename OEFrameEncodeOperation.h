//
//  OEFrameEncodeOperation.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/29/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class QTMovie;

@interface OEFrameEncodeOperation : NSOperation {
	NSImage *imageToEncode;
	QTMovie *movie;
	NSTimeInterval frameDuration;
}

@property (retain) NSImage* imageToEncode;
@property (retain) QTMovie*	movie;
- (id) initWithImage:(NSImage*) image forMovie:(QTMovie*) movie withDuration:(NSTimeInterval) interval;

@end
