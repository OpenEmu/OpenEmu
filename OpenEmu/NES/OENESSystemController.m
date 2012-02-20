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

#import "OENESSystemController.h"
#import "OENESSystemResponder.h"
#import "OENESSystemResponderClient.h"
#import "OELocalizationHelper.h"

@implementation OENESSystemController

- (Class)responderClass;
{
    return [OENESSystemResponder class];
}

- (NSArray *)genericSettingNames;
{
    return [super genericSettingNames];
}

- (NSArray *)genericControlNames;
{
    return [NSArray arrayWithObjects:OENESControlNameTable count:OENESButtonCount];
}

- (NSUInteger)numberOfPlayers;
{
    return 4;
}

- (NSDictionary *)defaultControls
{
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow]   , @"OENESButtonUp[1]"    ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"OENESButtonRight[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow] , @"OENESButtonLeft[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow] , @"OENESButtonDown[1]"  ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA]         , @"OENESButtonA[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS]         , @"OENESButtonB[1]"     ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardSpacebar]  , @"OENESButtonStart[1]" ,
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape]    , @"OENESButtonSelect[1]",
                              nil];
    return controls;
}

- (NSString *)systemName
{
	if([[OELocalizationHelper sharedHelper] isRegionJAP])
		return @"Famicom";
	else 
		return @"Nintendo (NES)";
}

- (NSImage*)systemIcon
{
    NSString* imageName;
	if([[OELocalizationHelper sharedHelper] isRegionJAP])
		imageName = @"famicom";
	else 
		imageName = @"nes"; 
    
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
