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
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OEWSButtonX1[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OEWSButtonX3[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OEWSButtonX4[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OEWSButtonX2[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OEWSButtonY1[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OEWSButtonY3[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OEWSButtonY4[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OEWSButtonY2[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardX]         , @"OEWSButtonA[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardZ]         , @"OEWSButtonB[1]"     ,
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardReturnOrEnter]    , @"OEWSButtonStart[1]",
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDeleteOrBackspace]    , @"OEWSButtonSound[1]",
                              nil];
    return controls;
}

@end
