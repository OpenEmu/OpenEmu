/*
 Copyright (c) 2012, OpenEmu Team
 
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

#import "OE2600SystemController.h"
#import "OE2600SystemResponder.h"
#import "OE2600SystemResponderClient.h"

@implementation OE2600SystemController

- (NSUInteger)numberOfPlayers;
{
    return 2;
}

- (Class)responderClass;
{
    return [OE2600SystemResponder class];
}

- (NSDictionary *)defaultControls
{
    return @{
    @"OE2600ButtonUp"         : @(kHIDUsage_KeyboardUpArrow)   ,
    @"OE2600ButtonDown"       : @(kHIDUsage_KeyboardDownArrow) ,
    @"OE2600ButtonLeft"       : @(kHIDUsage_KeyboardLeftArrow) ,
    @"OE2600ButtonRight"      : @(kHIDUsage_KeyboardRightArrow),
    @"OE2600ButtonFire1"      : @(kHIDUsage_KeyboardA)         ,
    @"OE2600ButtonLeftDiffA"  : @(kHIDUsage_KeyboardQ)         ,
    @"OE2600ButtonLeftDiffB"  : @(kHIDUsage_KeyboardW)         ,
    @"OE2600ButtonRightDiffA" : @(kHIDUsage_KeyboardE)         ,
    @"OE2600ButtonRightDiffB" : @(kHIDUsage_KeyboardR)         ,
    @"OE2600ButtonReset"      : @(kHIDUsage_KeyboardSpacebar)  ,
    @"OE2600ButtonSelect"     : @(kHIDUsage_KeyboardEscape)    ,
    };
}

@end
