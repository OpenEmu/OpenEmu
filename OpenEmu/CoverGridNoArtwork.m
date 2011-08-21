//
//  CoverGridNoArtwork.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 15.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "CoverGridNoArtwork.h"


@implementation CoverGridNoArtwork

- (id)init{
    self = [super init];
    if (self){}
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}


- (void)drawInContext:(CGContextRef)ctx{	
	NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
	[NSGraphicsContext saveGraphicsState];
	[NSGraphicsContext setCurrentContext:graphicsContext];
	[[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeCopy];
	
	NSImage* scanLineImage = [NSImage imageNamed:@"missing_artwork"];
	CGFloat width = self.bounds.size.width;
	CGFloat height = scanLineImage.size.height;
        
	CGFloat y=0;
	for (y=0; y<self.bounds.size.height; y+= height) {
		NSRect aScanLineRect = NSMakeRect(0, y, width, height);
		[scanLineImage drawInRect:aScanLineRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];		
	}
		
	[NSGraphicsContext restoreGraphicsState];
}

@end
