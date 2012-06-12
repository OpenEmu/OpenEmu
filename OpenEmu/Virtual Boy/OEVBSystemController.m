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

#import "OEVBSystemController.h"
#import "OEVBSystemResponder.h"
#import "OEVBSystemResponderClient.h"

@implementation OEVBSystemController

- (NSUInteger)numberOfPlayers;
{
    return 1;
}

- (Class)responderClass;
{
    return [OEVBSystemResponder class];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OEVBButtonRightUp[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow], @"OEVBButtonRightDown[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OEVBButtonRightLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow] , @"OEVBButtonRightRight[1]" ,
                              
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW], @"OEVBButtonLeftUp[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS], @"OEVBButtonLeftDown[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA], @"OEVBButtonLeftLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD], @"OEVBButtonLeftRight[1]" ,
                              
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardOpenBracket]         , @"OEVBButtonL[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardCloseBracket]         , @"OEVBButtonR[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OEVBButtonA[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OEVBButtonB[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardReturnOrEnter]    , @"OEVBButtonStart[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardTab]  , @"OEVBButtonSelect[1]"   ,
                              nil];
    return controls;
}

@end
