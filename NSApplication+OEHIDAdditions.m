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
    NSResponder *first = [[self mainWindow] firstResponder];
    switch ([anEvent type])
    {
        case OEHIDAxis :
        {
            switch ([anEvent axis])
            {
                case OEHIDAxisX :
                    [first axisXMoved:anEvent];
                    break;
                case OEHIDAxisY :
                    [first axisYMoved:anEvent];
                    break;
                case OEHIDAxisZ :
                    [first axisZMoved:anEvent];
                    break;
                case OEHIDAxisRx :
                    [first axisRxMoved:anEvent];
                    break;
                case OEHIDAxisRy :
                    [first axisRyMoved:anEvent];
                    break;
                case OEHIDAxisRz :
                    [first axisRzMoved:anEvent];
                    break;
            }
        }
            break;
        case OEHIDButton :
            if([anEvent state] == NSOffState)
                [first buttonUp:anEvent];
            else
                [first buttonDown:anEvent];
            break;
        case OEHIDHatSwitch :
            if([anEvent direction] == 0)
                [first hatSwitchUp:anEvent];
            else
                [first hatSwitchDown:anEvent];
            break;
        default:
            break;
    }
}

@end

@implementation NSResponder (OEHIDAdditions)

- (void)axisXMoved:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisXMoved:theEvent];
}

- (void)axisYMoved:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisYMoved:theEvent];
}

- (void)axisZMoved:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisZMoved:theEvent];
}

- (void)axisRxMoved:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisRxMoved:theEvent];
}

- (void)axisRyMoved:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisRyMoved:theEvent];
}

- (void)axisRzMoved:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisRzMoved:theEvent];
}

- (void)buttonDown:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder buttonDown:theEvent];
}

- (void)buttonUp:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder buttonUp:theEvent];
}


- (void)hatSwitchDown:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder hatSwitchDown:theEvent];
}

- (void)hatSwitchUp:(OEHIDEvent *)theEvent
{
    if(_nextResponder != nil)
        [_nextResponder hatSwitchUp:theEvent];
}

@end

