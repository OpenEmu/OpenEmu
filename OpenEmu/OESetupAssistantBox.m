//
//  OESetupAssistantBox.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantBox.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OESetupAssistantBox
- (void)drawRect:(NSRect)dirtyRect
{
    NSImage *image = [NSImage imageNamed:@"installer_dark_inset_box"];
    [image drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:16 rightBorder:16 topBorder:16 bottomBorder:16];
}
@end
