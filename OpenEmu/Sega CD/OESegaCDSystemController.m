/*
 Copyright (c) 2011, OpenEmu Team
 
 
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

#import "OESegaCDSystemController.h"
#import "OESegaCDSystemResponder.h"
#import "OESegaCDSystemResponderClient.h"

@implementation OESegaCDSystemController

- (NSUInteger)numberOfPlayers;
{
    return 2;
}

- (Class)responderClass;
{
    return [OESegaCDSystemResponder class];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OESegaCDButtonUp[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OESegaCDButtonDown[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OESegaCDButtonLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OESegaCDButtonRight[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OESegaCDButtonA[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OESegaCDButtonB[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OESegaCDButtonC[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OESegaCDButtonX[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OESegaCDButtonY[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OESegaCDButtonZ[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OESegaCDButtonStart[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OESegaCDButtonMode[1]" ,
                              nil];
    return controls;
}

@end
