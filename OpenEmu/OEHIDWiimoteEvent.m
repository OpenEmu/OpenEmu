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
    switch (buttonNumber) {
        case 0: return @"A";
        case 1: return @"B";
        case 2: return @"1";
        case 3: return @"2";
        case 4: return @"-";
        case 5: return @"Home";
        case 6: return @"+";
            
        case 7: return @"Up";
        case 8: return @"Down";
        case 9: return @"Left";
        case 10: return @"Right";
            
        default:
            break;
    }
    return [NSString stringWithFormat:@"%ld", buttonNumber];
}
- (NSString*)displayDescription
{
    return [NSString stringWithFormat:@"Wiimote %ld %@", [self padNumber]-WiimoteBasePadNumber+1, [self stringForButtonNumber:[self buttonNumber]]];
}
@end
