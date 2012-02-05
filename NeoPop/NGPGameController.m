/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "NGPGameController.h"
#import "NGPGameEmu.h"

@implementation NGPGameController

+ (void)initialize
{
    if(self == [NGPGameController class])
    {
        /*
        [self registerPreferenceViewControllerClasses:
         [NSDictionary dictionaryWithObject:[SMSPreferenceView class]
                                     forKey:OEControlsPreferenceKey]];
         */
    }
}

- (NSDictionary*)defaultControls
{
    /*
    NSDictionary *controls = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardUpArrow], @"SMS_PAD1_UP",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardRightArrow], @"SMS_PAD1_RIGHT",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardLeftArrow], @"SMS_PAD1_LEFT",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardDownArrow], @"SMS_PAD1_DOWN",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardA], @"SMS_PAD1_A",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardS], @"SMS_PAD1_B",
                              [NSNumber numberWithUnsignedInt:kHIDUsage_KeyboardEscape], @"SMS_RESET",nil];
    return controls;
     */
    return nil;
}

- (NSArray *)genericControlNames
{
    return nil;
    /*
    static NSArray *genericControlNames = nil;
    if(genericControlNames == nil)
    {
        //genericControlNames = [[NSArray alloc] initWithObjects:SMSButtonNameTable count:8];
    }
    return genericControlNames;
     */
}

- (NSUInteger)playerCount
{
    return 2;
}

- (Class)gameCoreClass
{
    return [NGPGameEmu class];
}

@end
