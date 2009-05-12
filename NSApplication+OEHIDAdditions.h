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

- (void)axisMoved:(OEHIDEvent *)anEvent;

- (void)buttonDown:(OEHIDEvent *)anEvent;
- (void)buttonUp:(OEHIDEvent *)anEvent;

- (void)hatSwitchDown:(OEHIDEvent *)anEvent;
- (void)hatSwitchUp:(OEHIDEvent *)anEvent;

@end

