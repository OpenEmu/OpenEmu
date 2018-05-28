/*
 Copyright (c) 2014, OpenEmu Team
 
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

// TODO: Intellivision disc supports 16 directions and this only handles 8
typedef enum _OEIntellivisionButton
{
    OEIntellivisionButtonUp,
    OEIntellivisionButtonDown,
    OEIntellivisionButtonLeft,
    OEIntellivisionButtonRight,
    OEIntellivisionButtonTopAction,
    OEIntellivisionButtonBottomLeftAction,
    OEIntellivisionButtonBottomRightAction,
    OEIntellivisionButton1,
    OEIntellivisionButton2,
    OEIntellivisionButton3,
    OEIntellivisionButton4,
    OEIntellivisionButton5,
    OEIntellivisionButton6,
    OEIntellivisionButton7,
    OEIntellivisionButton8,
    OEIntellivisionButton9,
    OEIntellivisionButton0,
    OEIntellivisionButtonClear,
    OEIntellivisionButtonEnter,
    OEIntellivisionButtonCount
} OEIntellivisionButton;

@protocol OEIntellivisionSystemResponderClient <OESystemResponderClient, NSObject>

- (oneway void)didPushIntellivisionButton:(OEIntellivisionButton)button forPlayer:(NSUInteger)player;
- (oneway void)didReleaseIntellivisionButton:(OEIntellivisionButton)button forPlayer:(NSUInteger)player;
- (oneway void)keyUp:(unsigned short)keyCode;
- (oneway void)keyDown:(unsigned short)keyCode;

@end
