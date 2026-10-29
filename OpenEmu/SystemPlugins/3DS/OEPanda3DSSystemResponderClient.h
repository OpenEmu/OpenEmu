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

#import <Foundation/Foundation.h>

@protocol OESystemResponderClient;

typedef enum _OEPanda3DSButton
{
    OEPanda3DSButtonUp,
    OEPanda3DSButtonDown,
    OEPanda3DSButtonLeft,
    OEPanda3DSButtonRight,
    OEPanda3DSButtonCirclePadUp,
    OEPanda3DSButtonCirclePadDown,
    OEPanda3DSButtonCirclePadLeft,
    OEPanda3DSButtonCirclePadRight,
    OEPanda3DSButtonA,
    OEPanda3DSButtonB,
    OEPanda3DSButtonX,
    OEPanda3DSButtonY,
    OEPanda3DSButtonL,
    OEPanda3DSButtonR,
    OEPanda3DSButtonZL,
    OEPanda3DSButtonZR,
    OEPanda3DSButtonStart,
    OEPanda3DSButtonSelect,
    OEPanda3DSButtonCount
} OEPanda3DSButton;

@protocol OEPanda3DSSystemResponderClient <OESystemResponderClient, NSObject>

- (oneway void)didMovePanda3DSJoystickDirection:(OEPanda3DSButton)button withValue:(CGFloat)value forPlayer:(NSUInteger)player;
- (oneway void)didPushPanda3DSButton:(OEPanda3DSButton)button forPlayer:(NSUInteger)player;
- (oneway void)didReleasePanda3DSButton:(OEPanda3DSButton)button forPlayer:(NSUInteger)player;

// The touch screen coordinates are in 3DS screen space (320 x 240, bottom screen).
- (oneway void)didTouchPanda3DSScreenPoint:(OEIntPoint)point;
- (oneway void)didReleasePanda3DSTouch;

@end
