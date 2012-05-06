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

#import "OENDSSystemController.h"
#import "OENDSSystemResponder.h"
#import "OENDSSystemResponderClient.h"

@implementation OENDSSystemController

- (NSUInteger)numberOfPlayers;
{
    return 4;
}

- (Class)responderClass;
{
    return [OENDSSystemResponder class];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OENDSButtonUp[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OENDSButtonDown[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OENDSButtonLeft[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OENDSButtonRight[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardX]         , @"OENDSButtonA[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardZ]         , @"OENDSButtonB[1]"     ,
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OENDSButtonX[1]"     ,
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OENDSButtonY[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OENDSButtonL[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OENDSButtonR[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardTab]       , @"OENDSButtonSelect[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardReturnOrEnter]    , @"OENDSButtonStart[1]",
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardGraveAccentAndTilde]    , @"OENDSButtonMicrophone[1]",
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDeleteOrBackspace]    , @"OENDSButtonLid[1]",
                              nil];
							  // Note that we skip Debug since we're not exposing that control through the UI at this time.
    return controls;
}

@end
