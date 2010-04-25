/*
 Copyright (c) 2010 OpenEmu Team
 
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

#import "MupenGameCore.h"
#import "MupenGameController.h"
#import "MupenPreferenceViewController.h"

const NSString *MupenControlNames[] = {
    @"N64_DPadR", @"N64_DPadL", @"N64_DPadD", @"N64_DPadU",
    @"N64_Start", @"N64_Z", @"N64_B", @"N64_A", @"N64_CR",
    @"N64_CL", @"N64_CD", @"N64_CU", @"N64_R", @"N64_L"
};
// FIXME: missing: joypad X, joypad Y, mempak switch, rumble switch

const int kMupenControlNamesCount = sizeof(MupenControlNames) / sizeof(MupenControlNames[0]);

static NSArray *MupenControlNamesArray;

@implementation MupenGameController

+ (void)initialize
{
    [super initialize];
    
    [self registerPreferenceViewControllerClasses:
     [NSDictionary dictionaryWithObject:[MupenPreferenceViewController class]
                                 forKey:OEControlsPreferenceKey]];
    
    MupenControlNamesArray = [[NSArray alloc] initWithObjects:MupenControlNames
                                                        count:kMupenControlNamesCount];
}

- (NSArray *)genericControlNames
{
    return MupenControlNamesArray;
}

- (NSUInteger)playerCount
{
    return 4;
}

- (Class)gameCoreClass
{
    return [MupenGameCore class];
}

@end
