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

#import "OEThreeDOSystemController.h"
#import "OEThreeDOSystemResponder.h"
#import "OEThreeDOSystemResponderClient.h"
#import "OELocalizationHelper.h"

@implementation OEThreeDOSystemController

- (NSUInteger)numberOfPlayers;
{
    return 8;
}

- (Class)responderClass;
{
    return [OEThreeDOSystemResponder class];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OEThreeDOButtonUp[1]"          ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OEThreeDOButtonDown[1]"        ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OEThreeDOButtonLeft[1]"        ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OEThreeDOButtonRight[1]"       ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]         , @"OEThreeDOButtonA[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OEThreeDOButtonB[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]         , @"OEThreeDOButtonX[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OEThreeDOButtonY[1]"           ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]         , @"OEThreeDOButtonTriggerLeft[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]         , @"OEThreeDOButtonTriggerRight[1]",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OEThreeDOButtonStart[1]"       ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OEThreeDOButtonSelect[1]"      ,
                              nil];
    return controls;
}

- (NSString*)systemName{

		return @"Panasonic 3DO";
}

- (NSImage*)systemIcon
{
    NSString* imageName;
	if([[OELocalizationHelper sharedHelper] isRegionNA])
		imageName = @"ThreeDO_usa_library";
	else 
		imageName = @"ThreeDO_eujap_library"; 
    
    NSImage* image = [NSImage imageNamed:imageName];
    if(!image)
    {
        NSBundle* bundle = [NSBundle bundleForClass:[self class]];
        NSString* path = [bundle pathForImageResource:imageName];
        image = [[NSImage alloc] initWithContentsOfFile:path];
        [image setName:imageName];
    }
    return image;
}

@end
