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

static BOOL _logHIDEvents = NO;
static BOOL _logHIDEventsNoKeyboard = NO;

- (void)postHIDEvent:(OEHIDEvent *)anEvent
{
    if(anEvent == nil) return;
    
    if(_logHIDEvents) {
        if (!_logHIDEventsNoKeyboard || anEvent.type != OEHIDEventTypeKeyboard) {
            NSLog(@"%@", anEvent);
        }
    }

    [[[self keyWindow] firstResponder] handleHIDEvent:anEvent];
}

- (BOOL)logHIDEvents
{
    return _logHIDEvents;
}

- (void)setLogHIDEvents:(BOOL)value
{
    _logHIDEvents = value;
}

- (BOOL)logHIDEventsNoKeyboard
{
    return _logHIDEventsNoKeyboard;
}

- (void)setLogHIDEventsNoKeyboard:(BOOL)value
{
    _logHIDEventsNoKeyboard = value;
}

@end

static dispatch_queue_t oehid_queue;

@implementation NSResponder (OEHIDAdditions)

- (void)handleHIDEvent:(OEHIDEvent *)anEvent
{
    if(oehid_queue == NULL) oehid_queue = dispatch_queue_create("OEHIDAdditions HID forwarding", DISPATCH_QUEUE_SERIAL);
    
    dispatch_async(oehid_queue, ^{
        switch([anEvent type])
        {
            case OEHIDEventTypeAxis :
                [self axisMoved:anEvent];
                break;
            case OEHIDEventTypeTrigger :
                if([anEvent hasOffState])
                    [self triggerRelease:anEvent];
                else
                    [self triggerPull:anEvent];
                break;
            case OEHIDEventTypeButton :
                if([anEvent hasOffState])
                    [self buttonUp:anEvent];
                else
                    [self buttonDown:anEvent];
                break;
            case OEHIDEventTypeHatSwitch :
                [self hatSwitchChanged:anEvent];
                break;
            case OEHIDEventTypeKeyboard :
                if([anEvent hasOffState])
                    [self HIDKeyUp:anEvent];
                else
                    [self HIDKeyDown:anEvent];
                break;
            default:
                break;
        }
    });
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder axisMoved:anEvent];
}

- (void)triggerPull:(OEHIDEvent *)anEvent;
{
    if(_nextResponder != nil)
        [_nextResponder triggerPull:anEvent];
}

- (void)triggerRelease:(OEHIDEvent *)anEvent;
{
    if(_nextResponder != nil)
        [_nextResponder triggerRelease:anEvent];
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

- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
{
    if(_nextResponder != nil)
        [_nextResponder hatSwitchChanged:anEvent];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder HIDKeyDown:anEvent];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent
{
    if(_nextResponder != nil)
        [_nextResponder HIDKeyUp:anEvent];
}

@end
