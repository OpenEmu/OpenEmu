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

#import "OEWSSystemController.h"
#import "OEWSSystemResponder.h"
#import "OEWSSystemResponderClient.h"
#import "OELocalizationHelper.h"

@implementation OEWSSystemController

- (NSUInteger)numberOfPlayers;
{
    return 1;
}

- (Class)responderClass;
{
    return [OEWSSystemResponder class];
}

- (NSDictionary *)defaultControls
{
    return @{
    @"OEWSButtonX1"    : @(kHIDUsage_KeyboardUpArrow)   ,
    @"OEWSButtonX3"    : @(kHIDUsage_KeyboardDownArrow) ,
    @"OEWSButtonX4"    : @(kHIDUsage_KeyboardLeftArrow) ,
    @"OEWSButtonX2"    : @(kHIDUsage_KeyboardRightArrow),
    @"OEWSButtonY1"    : @(kHIDUsage_KeyboardW)         ,
    @"OEWSButtonY3"    : @(kHIDUsage_KeyboardS)         ,
    @"OEWSButtonY4"    : @(kHIDUsage_KeyboardA)         ,
    @"OEWSButtonY2"    : @(kHIDUsage_KeyboardD)         ,
    @"OEWSButtonA"     : @(kHIDUsage_KeyboardZ)         ,
    @"OEWSButtonB"     : @(kHIDUsage_KeyboardX)         ,
    @"OEWSButtonStart" : @(kHIDUsage_KeyboardSpacebar)  ,
    @"OEWSButtonSound" : @(kHIDUsage_KeyboardEscape)    ,
    };
}

@end
