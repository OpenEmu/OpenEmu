//
//  NSApplication+OEHIDAdditions.h
//  OpenEmu
//
//  Created by Remy Demarest on 09/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class OEHIDEvent;

@interface NSApplication (OEHIDAdditions)
- (void)postHIDEvent:(OEHIDEvent *)anEvent;
@end

@interface NSResponder (OEHIDAdditions)

- (void)axisXMoved:(OEHIDEvent *)theEvent;
- (void)axisYMoved:(OEHIDEvent *)theEvent;
- (void)axisZMoved:(OEHIDEvent *)theEvent;
- (void)axisRxMoved:(OEHIDEvent *)theEvent;
- (void)axisRyMoved:(OEHIDEvent *)theEvent;
- (void)axisRzMoved:(OEHIDEvent *)theEvent;

- (void)buttonDown:(OEHIDEvent *)theEvent;
- (void)buttonUp:(OEHIDEvent *)theEvent;

- (void)hatSwitchDown:(OEHIDEvent *)theEvent;
- (void)hatSwitchUp:(OEHIDEvent *)theEvent;

@end

