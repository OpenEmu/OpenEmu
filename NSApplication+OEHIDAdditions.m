//
//  NSApplication+OEHIDAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 09/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"

@implementation NSApplication (OEHIDAdditions)

- (void)postHIDEvent:(OEHIDEvent *)anEvent
{
    [[[self mainWindow] firstResponder] handleHIDEvent:anEvent];
}

@end

@implementation NSResponder (OEHIDAdditions)

- (void)handleHIDEvent:(OEHIDEvent *)anEvent
{
    switch ([anEvent type])
    {
        case OEHIDAxis :
            [self axisMoved:anEvent];
            break;
        case OEHIDButton :
            if([anEvent state] == NSOffState)
                [self buttonUp:anEvent];
            else
                [self buttonDown:anEvent];
            break;
        case OEHIDHatSwitch :
            if([anEvent position] == 0)
                [self hatSwitchUp:anEvent];
            else
                [self hatSwitchDown:anEvent];
            break;
        default:
            break;
    }
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisMoved:anEvent];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder buttonDown:anEvent];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder buttonUp:anEvent];
}


- (void)hatSwitchDown:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder hatSwitchDown:anEvent];
}

- (void)hatSwitchUp:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder hatSwitchUp:anEvent];
}

@end

