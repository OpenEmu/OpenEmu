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

typedef enum
{
    OEMSXJoystickUp,
    OEMSXJoystickDown,
    OEMSXJoystickLeft,
    OEMSXJoystickRight,
    OEMSXButtonA,
    OEMSXButtonB,
    OEMSXButtonCount
} OEMSXJoystickButton;

typedef enum
{
    // These names reflect the labels on the European keyboard, except
    // as otherwise indicated
    
    OEMSXKeyF1 = 0x01,
    OEMSXKeyF2 = 0x02,
    OEMSXKeyF3 = 0x03,
    OEMSXKeyF4 = 0x04,
    OEMSXKeyF5 = 0x05,

    OEMSXKeyStop   = 0x06,
    OEMSXKeyCls    = 0x07,
    OEMSXKeySelect = 0x08,
    OEMSXKeyInsert = 0x09,
    OEMSXKeyDelete = 0x0A,
    
    OEMSXKeyEscape    = 0x0B,
    OEMSXKey1         = 0x0C,
    OEMSXKey2         = 0x0D,
    OEMSXKey3         = 0x0E,
    OEMSXKey4         = 0x0F,
    OEMSXKey5         = 0x10,
    OEMSXKey6         = 0x11,
    OEMSXKey7         = 0x12,
    OEMSXKey8         = 0x13,
    OEMSXKey9         = 0x14,
    OEMSXKey0         = 0x15,
    OEMSXKeyMinus     = 0x16,
    OEMSXKeyEquals    = 0x17,
    OEMSXKeyBackslash = 0x18,
    OEMSXKeyBackspace = 0x19,
    
    OEMSXKeyTab          = 0x1A,
    OEMSXKeyQ            = 0x1B,
    OEMSXKeyW            = 0x1C,
    OEMSXKeyE            = 0x1D,
    OEMSXKeyR            = 0x1E,
    OEMSXKeyT            = 0x1F,
    OEMSXKeyY            = 0x20,
    OEMSXKeyU            = 0x21,
    OEMSXKeyI            = 0x22,
    OEMSXKeyO            = 0x23,
    OEMSXKeyP            = 0x24,
    OEMSXKeyLeftBracket  = 0x25,
    OEMSXKeyRightBracket = 0x26,
    OEMSXKeyReturn       = 0x27,
    
    OEMSXKeyCtrl      = 0x28,
    OEMSXKeyA         = 0x29,
    OEMSXKeyS         = 0x2A,
    OEMSXKeyD         = 0x2B,
    OEMSXKeyF         = 0x2C,
    OEMSXKeyG         = 0x2D,
    OEMSXKeyH         = 0x2E,
    OEMSXKeyJ         = 0x2F,
    OEMSXKeyK         = 0x30,
    OEMSXKeyL         = 0x31,
    OEMSXKeySemicolon = 0x32,
    OEMSXKeyQuote     = 0x33,
    OEMSXKeyBacktick  = 0x34,
    
    OEMSXKeyLeftShift  = 0x35,
    OEMSXKeyZ          = 0x36,
    OEMSXKeyX          = 0x37,
    OEMSXKeyC          = 0x38,
    OEMSXKeyV          = 0x39,
    OEMSXKeyB          = 0x3A,
    OEMSXKeyN          = 0x3B,
    OEMSXKeyM          = 0x3C,
    OEMSXKeyComma      = 0x3D,
    OEMSXKeyPeriod     = 0x3E,
    OEMSXKeySlash      = 0x3F,
    OEMSXKeyUnderscore = 0x40, /* Not present on EU keyboards */
    OEMSXKeyRightShift = 0x41,
    
    OEMSXKeyCapsLock   = 0x42,
    OEMSXKeyGraph      = 0x43,
    OEMSXKeyTorikeshi  = 0x44, /* JP only, not present on EU keyboards */
    OEMSXKeySpacebar   = 0x45,
    OEMSXKeyJikkou     = 0x46, /* JP only, not present on EU keyboards */
    OEMSXKeyCode       = 0x47,
    OEMSXKeyPause      = 0x48,
    
    OEMSXKeyLeft  = 0x49,
    OEMSXKeyUp    = 0x4A,
    OEMSXKeyDown  = 0x4B,
    OEMSXKeyRight = 0x4C,
    
    OEMSXKeyNumpad7     = 0x4D,
    OEMSXKeyNumpad8     = 0x4E,
    OEMSXKeyNumpad9     = 0x4F,
    OEMSXKeyNumpadSlash = 0x50,
    OEMSXKeyNumpad4     = 0x51,
    OEMSXKeyNumpad5     = 0x52,
    OEMSXKeyNumpad6     = 0x53,
    OEMSXKeyNumpadStar  = 0x54,
    OEMSXKeyNumpad1     = 0x55,
    OEMSXKeyNumpad2     = 0x56,
    OEMSXKeyNumpad3     = 0x57,
    OEMSXKeyNumpadMinus = 0x58,
    OEMSXKeyNumpad0     = 0x59,
    OEMSXKeyNumpadPoint = 0x5A,
    OEMSXKeyNumpadComma = 0x5B,
    OEMSXKeyNumpadPlus  = 0x5C,
} OEMSXKey;

@protocol OEMSXSystemResponderClient <OESystemResponderClient, NSObject>

- (oneway void)didPushMSXJoystickButton:(OEMSXJoystickButton)button
                             controller:(NSInteger)index;
- (oneway void)didReleaseMSXJoystickButton:(OEMSXJoystickButton)button
                                controller:(NSInteger)index;

- (oneway void)didPressKey:(OEMSXKey)key;
- (oneway void)didReleaseKey:(OEMSXKey)key;

@end
