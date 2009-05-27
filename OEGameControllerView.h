//
//  OEGameControllerView.h
//  OpenEmu
//
//  Created by Remy Demarest on 24/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface OEGameControllerView : NSView
{
    NSImage *gameController;
    NSBezierPath *lines;
    NSRect controlZone;
    NSRect drawRect;
}

@property(retain) NSImage *gameController;
@property(assign) NSRect controlZone;
- (void)addButtonWithName:(NSString *)aName target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end;
@end
