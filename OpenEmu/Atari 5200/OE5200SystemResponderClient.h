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

typedef enum _OE5200Button
{
    OE5200ButtonUp,
    OE5200ButtonDown,
    OE5200ButtonLeft,
    OE5200ButtonRight,
    OE5200ButtonFire1,
    OE5200ButtonFire2,
    OE5200ButtonStart,
    OE5200ButtonPause,
    OE5200ButtonReset,
    OE5200Button1,
    OE5200Button2,
    OE5200Button3,
    OE5200Button4,
    OE5200Button5,
    OE5200Button6,
    OE5200Button7,
    OE5200Button8,
    OE5200Button9,
    OE5200Button0,
    OE5200ButtonAsterisk,
    OE5200ButtonPound,
    OE5200ButtonCount
} OE5200Button;

@protocol OE5200SystemResponderClient <OESystemResponderClient, NSObject>

- (oneway void)didPush5200Button:(OE5200Button)button forPlayer:(NSUInteger)player;
- (oneway void)didRelease5200Button:(OE5200Button)button forPlayer:(NSUInteger)player;

@end
