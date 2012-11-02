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

#import "OEHIDWiimoteEvent.h"
#import "OEWiimoteHandler.h"

@implementation OEHIDWiimoteEvent
- (NSString*)stringForButtonNumber:(NSUInteger)buttonNumber
{
    WiiButtonType btnType = buttonNumber;
    switch (btnType)
    {
        case WiiRemoteAButton: return @"A";
        case WiiRemoteBButton: return @"B";
        case WiiRemoteOneButton: return @"1";
        case WiiRemoteTwoButton: return @"2";
        case WiiRemoteMinusButton: return @"-";
        case WiiRemoteHomeButton: return @"Home";
        case WiiRemotePlusButton: return @"+";
            
        case WiiRemoteUpButton: return @"Up";
        case WiiRemoteDownButton: return @"Down";
        case WiiRemoteLeftButton: return @"Left";
        case WiiRemoteRightButton: return @"Right";
            
        case WiiNunchukZButton: return @"Nu Z";
        case WiiNunchukCButton: return @"Nu C";
            
        case WiiClassicControllerXButton: return @"CC X";
        case WiiClassicControllerYButton: return @"CC Y";
        case WiiClassicControllerAButton: return @"CC A";
        case WiiClassicControllerBButton: return @"CC B";
        case WiiClassicControllerLButton: return @"CC L";
        case WiiClassicControllerRButton: return @"CC R";
        case WiiClassicControllerZLButton: return @"CC ZL";
        case WiiClassicControllerZRButton: return @"CC ZR";
        case WiiClassicControllerUpButton: return @"CC Up";
        case WiiClassicControllerDownButton: return @"CC Down";
        case WiiClassicControllerLeftButton: return @"CC Left";
        case WiiClassicControllerRightButton: return @"CC Right";
        case WiiClassicControllerMinusButton: return @"CC -";
        case WiiClassicControllerHomeButton: return @"CC Home";
        case WiiClassicControllerPlusButton: return @"CC +";
            
        default:
            break;
    }
    return [NSString stringWithFormat:@"%ld", buttonNumber];
}

- (NSString*)displayDescription
{
    switch ([self type])
    {
        case OEHIDEventTypeButton:
            return [NSString stringWithFormat:@"Wi%ld %@", [self padNumber]-WiimoteBasePadNumber+1, [self stringForButtonNumber:[self buttonNumber]]];
        default:
            return [NSString stringWithFormat:@"Wi%ld %@", [self padNumber]-WiimoteBasePadNumber+1, @"Unknown"];
    }
    
}
@end
