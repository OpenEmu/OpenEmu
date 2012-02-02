//
//  OESetupAssistantBackgroundView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantBackgroundView.h"

@implementation OESetupAssistantBackgroundView

- (void)drawRect:(NSRect)dirtyRect
{
    NSImage *backgroundImage = [NSImage imageNamed:@"installer_backgroundSample"];
    [backgroundImage drawInRect:dirtyRect fromRect:dirtyRect operation:NSCompositeCopy fraction:1.0];
}

@end
