//
//  OEPrefBoxControls.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefBoxControls.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEPrefBoxControls

- (void)drawRect:(NSRect)dirtyRect{
	NSImage* image = [NSImage imageNamed:@"box_controls"];
	[image drawInRect:self.bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:16 rightBorder:16 topBorder:16 bottomBorder:16];
}

- (BOOL)isFlipped{
	return YES;
}

@end
