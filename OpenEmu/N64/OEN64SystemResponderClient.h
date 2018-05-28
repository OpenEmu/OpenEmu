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

#import <Cocoa/Cocoa.h>

@protocol OESystemResponderClient;

typedef enum _OEN64Button
{
    // D-Pad
    OEN64ButtonDPadUp,
    OEN64ButtonDPadDown,
    OEN64ButtonDPadLeft,
    OEN64ButtonDPadRight,

    // C buttons
    OEN64ButtonCUp,
    OEN64ButtonCDown,
    OEN64ButtonCLeft,
    OEN64ButtonCRight,

    OEN64ButtonA,
    OEN64ButtonB,

    // Shoulder buttons
    OEN64ButtonL,
    OEN64ButtonR,
    OEN64ButtonZ,

    OEN64ButtonStart,

    OEN64AnalogUp,
    OEN64AnalogDown,
    OEN64AnalogLeft,
    OEN64AnalogRight,
    OEN64ButtonCount
} OEN64Button;
// FIXME: analog stick (x,y), memory pack, rumble pack

@protocol OEN64SystemResponderClient <OESystemResponderClient, NSObject>

- (oneway void)didMoveN64JoystickDirection:(OEN64Button)button withValue:(CGFloat)value forPlayer:(NSUInteger)player;
- (oneway void)didPushN64Button:(OEN64Button)button forPlayer:(NSUInteger)player;
- (oneway void)didReleaseN64Button:(OEN64Button)button forPlayer:(NSUInteger)player;

@end
