//
//  OEPrefBoxControls.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPreferencesControlsBox.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEPreferencesControlsBox

- (void)drawRect:(NSRect)dirtyRect{
	NSImage* image = [NSImage imageNamed:@"wood_inset_box"];
	[image drawInRect:self.bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:NO hints:nil leftBorder:16 rightBorder:16 topBorder:16 bottomBorder:16];
}

- (BOOL)isFlipped{
	return NO;
}

@end
