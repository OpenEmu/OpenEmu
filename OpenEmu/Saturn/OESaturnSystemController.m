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

#import "OESaturnSystemController.h"
#import "OESaturnSystemResponder.h"
#import "OESaturnSystemResponderClient.h"
#import "OELocalizationHelper.h"

@implementation OESaturnSystemController

- (NSUInteger)numberOfPlayers;
{
    return 2;
}

- (Class)responderClass;
{
    return [OESaturnSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OESaturnButtonNameTable count:OESaturnButtonCount];
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]        , @"OESaturnButtonUp[1]"            ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow]      , @"OESaturnButtonDown[1]"          ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow]      , @"OESaturnButtonLeft[1]"          ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow]     , @"OESaturnButtonRight[1]"         ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]              , @"OESaturnButtonA[1]"             ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]              , @"OESaturnButtonB[1]"             ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardD]              , @"OESaturnButtonC[1]"             ,
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardQ]              , @"OESaturnButtonX[1]"             ,
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardW]              , @"OESaturnButtonY[1]"             ,
							  [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardE]              , @"OESaturnButtonZ[1]"             ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardY]              , @"OESaturnButtonTriggerLeft[1]"   ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardX]              , @"OESaturnButtonTriggerRight[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardReturnOrEnter]  , @"OESaturnButtonStart[1]"         ,
                              nil];
							  // Note that we skip Debug since we're not exposing that control through the UI at this time.
    return controls;
}

- (NSString*)systemName
{
    return @"Sega Saturn";
}

- (NSImage*)systemIcon
{
    NSString* imageName;
	if([[OELocalizationHelper sharedHelper] isRegionNA])
		imageName = @"saturn_library";
	else 
		imageName = @"saturn_library"; 
    
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
