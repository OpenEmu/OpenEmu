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

#import "OESNESSystemController.h"
#import "OESNESSystemResponder.h"
#import "OESNESPreferenceViewController.h"
#import "OESNESSystemResponderClient.h"
#import "OELocalizationHelper.h"
@implementation OESNESSystemController

- (NSUInteger)numberOfPlayers;
{
    return 8;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionaryWithObject:[OESNESPreferenceViewController class] forKey:OEControlsPreferenceKey];
}

- (Class)responderClass;
{
    return [OESNESSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OESNESButtonNameTable count:OESNESButtonCount];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OESNESButtonUp[1]"          ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OESNESButtonDown[1]"        ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OESNESButtonLeft[1]"        ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OESNESButtonRight[1]"       ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OESNESButtonA[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OESNESButtonB[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OESNESButtonX[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OESNESButtonY[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OESNESButtonTriggerLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OESNESButtonTriggerRight[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OESNESButtonStart[1]"       ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OESNESButtonSelect[1]"      ,
                              nil];
    return controls;
}

- (NSString*)systemName{
	if([[OELocalizationHelper sharedHelper] isRegionJAP])
		return @"Super Famicom";
	else 
		return @"Super Nintendo (SNES)";
}

@end
