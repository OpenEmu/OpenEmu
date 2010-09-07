/*
 Copyright (c) 2009, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"

@implementation NSApplication (OEHIDAdditions)

- (void)postHIDEvent:(OEHIDEvent *)anEvent
{
    [[[self keyWindow] firstResponder] handleHIDEvent:anEvent];
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
		case OEHIDKeypress :
			if([anEvent state] == NSOffState)
                [self hidKeyUp:anEvent];
            else
                [self hidKeyDown:anEvent];
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

- (void)hidKeyDown:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder hidKeyDown:anEvent];
}

- (void)hidKeyUp:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder hidKeyUp:anEvent];
}

@end

