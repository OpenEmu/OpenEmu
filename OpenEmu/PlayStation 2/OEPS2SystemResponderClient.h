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

#import <Cocoa/Cocoa.h>

@protocol OESystemResponderClient;

typedef enum _OEPS2Button
{
    OEPS2ButtonUp,
    OEPS2ButtonDown,
    OEPS2ButtonLeft,
    OEPS2ButtonRight,
    OEPS2ButtonTriangle,
    OEPS2ButtonCircle,
    OEPS2ButtonCross,
    OEPS2ButtonSquare,
    OEPS2ButtonL1,
    OEPS2ButtonL2,
    OEPS2ButtonL3,
    OEPS2ButtonR1,
    OEPS2ButtonR2,
    OEPS2ButtonR3,
    OEPS2ButtonStart,
    OEPS2ButtonSelect,
    OEPS2ButtonAnalogMode,
    OEPS2LeftAnalogUp,
    OEPS2LeftAnalogDown,
    OEPS2LeftAnalogLeft,
    OEPS2LeftAnalogRight,
    OEPS2RightAnalogUp,
    OEPS2RightAnalogDown,
    OEPS2RightAnalogLeft,
    OEPS2RightAnalogRight,
    OEPS2ButtonCount
} OEPS2Button;

@protocol OEPS2SystemResponderClient <OESystemResponderClient, NSObject>

- (oneway void)didMovePS2JoystickDirection:(OEPS2Button)button withValue:(CGFloat)value forPlayer:(NSUInteger)player;
- (oneway void)didPushPS2Button:(OEPS2Button)button forPlayer:(NSUInteger)player;
- (oneway void)didReleasePS2Button:(OEPS2Button)button forPlayer:(NSUInteger)player;

@end
