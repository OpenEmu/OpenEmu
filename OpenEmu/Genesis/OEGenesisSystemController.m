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

#import "OEGenesisSystemController.h"
#import "OEGenesisSystemResponder.h"
#import "OEGenesisPreferenceView.h"

@implementation OEGenesisSystemController

- (NSUInteger)numberOfPlayers;
{
    return 2;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionaryWithObject:[OEGenesisPreferenceView class] forKey:OEControlsPreferenceKey];
}

- (Class)responderClass;
{
    return [OEGenesisSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OEGenesisButtonNameTable count:OEGenesisButtonCount];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OEGenesisButtonUp[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OEGenesisButtonDown[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OEGenesisButtonLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OEGenesisButtonRight[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OEGenesisButtonA[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OEGenesisButtonB[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OEGenesisButtonC[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OEGenesisButtonX[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OEGenesisButtonY[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OEGenesisButtonZ[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OEGenesisButtonStart[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OEGenesisButtonMode[1]" ,
                              nil];
    return controls;
}

@end
