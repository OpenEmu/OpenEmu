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

#import "OEMSXSystemResponder.h"
#import "OEMSXSystemResponderClient.h"

@implementation OEMSXSystemResponder

@dynamic client;

static NSDictionary *virtualPhysicalKeyMap;

+ (void)initialize
{
    virtualPhysicalKeyMap = @{ @(kHIDUsage_KeyboardF1): @(OEMSXKeyF1),
                               @(kHIDUsage_KeyboardF2): @(OEMSXKeyF2),
                               @(kHIDUsage_KeyboardF3): @(OEMSXKeyF3),
                               @(kHIDUsage_KeyboardF4): @(OEMSXKeyF4),
                               @(kHIDUsage_KeyboardF5): @(OEMSXKeyF5),
                               
                               @(kHIDUsage_KeyboardF10):           @(OEMSXKeyStop),
                               @(kHIDUsage_KeyboardF11):           @(OEMSXKeyCls),
                               @(kHIDUsage_KeyboardF12):           @(OEMSXKeySelect),
                               @(kHIDUsage_KeyboardInsert):        @(OEMSXKeyInsert),
                               @(kHIDUsage_KeyboardDeleteForward): @(OEMSXKeyDelete),
                               
                               @(kHIDUsage_KeyboardEscape):            @(OEMSXKeyEscape),
                               @(kHIDUsage_Keyboard1):                 @(OEMSXKey1),
                               @(kHIDUsage_Keyboard2):                 @(OEMSXKey2),
                               @(kHIDUsage_Keyboard3):                 @(OEMSXKey3),
                               @(kHIDUsage_Keyboard4):                 @(OEMSXKey4),
                               @(kHIDUsage_Keyboard5):                 @(OEMSXKey5),
                               @(kHIDUsage_Keyboard6):                 @(OEMSXKey6),
                               @(kHIDUsage_Keyboard7):                 @(OEMSXKey7),
                               @(kHIDUsage_Keyboard8):                 @(OEMSXKey8),
                               @(kHIDUsage_Keyboard9):                 @(OEMSXKey9),
                               @(kHIDUsage_Keyboard0):                 @(OEMSXKey0),
                               @(kHIDUsage_KeyboardHyphen):            @(OEMSXKeyMinus),
                               @(kHIDUsage_KeyboardEqualSign):         @(OEMSXKeyEquals),
                               @(kHIDUsage_KeyboardBackslash):         @(OEMSXKeyBackslash),
                               @(kHIDUsage_KeyboardDeleteOrBackspace): @(OEMSXKeyBackspace),

                               @(kHIDUsage_KeyboardTab):          @(OEMSXKeyTab),
                               @(kHIDUsage_KeyboardQ):            @(OEMSXKeyQ),
                               @(kHIDUsage_KeyboardW):            @(OEMSXKeyW),
                               @(kHIDUsage_KeyboardE):            @(OEMSXKeyE),
                               @(kHIDUsage_KeyboardR):            @(OEMSXKeyR),
                               @(kHIDUsage_KeyboardT):            @(OEMSXKeyT),
                               @(kHIDUsage_KeyboardY):            @(OEMSXKeyY),
                               @(kHIDUsage_KeyboardU):            @(OEMSXKeyU),
                               @(kHIDUsage_KeyboardI):            @(OEMSXKeyI),
                               @(kHIDUsage_KeyboardO):            @(OEMSXKeyO),
                               @(kHIDUsage_KeyboardP):            @(OEMSXKeyP),
                               @(kHIDUsage_KeyboardOpenBracket):  @(OEMSXKeyLeftBracket),
                               @(kHIDUsage_KeyboardCloseBracket): @(OEMSXKeyRightBracket),
                               @(kHIDUsage_KeyboardReturnOrEnter): @(OEMSXKeyReturn),
                               
                               @(kHIDUsage_KeyboardLeftControl): @(OEMSXKeyCtrl),
                               @(kHIDUsage_KeyboardA):           @(OEMSXKeyA),
                               @(kHIDUsage_KeyboardS):           @(OEMSXKeyS),
                               @(kHIDUsage_KeyboardD):           @(OEMSXKeyD),
                               @(kHIDUsage_KeyboardF):           @(OEMSXKeyF),
                               @(kHIDUsage_KeyboardG):           @(OEMSXKeyG),
                               @(kHIDUsage_KeyboardH):           @(OEMSXKeyH),
                               @(kHIDUsage_KeyboardJ):           @(OEMSXKeyJ),
                               @(kHIDUsage_KeyboardK):           @(OEMSXKeyK),
                               @(kHIDUsage_KeyboardL):           @(OEMSXKeyL),
                               @(kHIDUsage_KeyboardSemicolon):   @(OEMSXKeySemicolon),
                               @(kHIDUsage_KeyboardQuote):       @(OEMSXKeyQuote),
                               @(kHIDUsage_KeyboardGraveAccentAndTilde): @(OEMSXKeyBacktick),
                               
                               @(kHIDUsage_KeyboardLeftShift):  @(OEMSXKeyLeftShift),
                               @(kHIDUsage_KeyboardZ):          @(OEMSXKeyZ),
                               @(kHIDUsage_KeyboardX):          @(OEMSXKeyX),
                               @(kHIDUsage_KeyboardC):          @(OEMSXKeyC),
                               @(kHIDUsage_KeyboardV):          @(OEMSXKeyV),
                               @(kHIDUsage_KeyboardB):          @(OEMSXKeyB),
                               @(kHIDUsage_KeyboardN):          @(OEMSXKeyN),
                               @(kHIDUsage_KeyboardM):          @(OEMSXKeyM),
                               @(kHIDUsage_KeyboardComma):      @(OEMSXKeyComma),
                               @(kHIDUsage_KeyboardPeriod):     @(OEMSXKeyPeriod),
                               @(kHIDUsage_KeyboardSlash):      @(OEMSXKeySlash),
                               // No mapping for @(OEMSXKeyUnderscore),
                               @(kHIDUsage_KeyboardRightShift): @(OEMSXKeyRightShift),
                               
                               @(kHIDUsage_KeyboardCapsLock): @(OEMSXKeyCapsLock),
                               @(kHIDUsage_KeyboardLeftAlt):  @(OEMSXKeyGraph),
                               // No mapping for @(OEMSXKeyTorikeshi),
                               @(kHIDUsage_KeyboardSpacebar): @(OEMSXKeySpacebar),
                               // No mapping for @(OEMSXKeyJikkou),
                               @(kHIDUsage_KeyboardRightAlt): @(OEMSXKeyCode),
                               @(kHIDUsage_KeyboardPause):    @(OEMSXKeyPause),
                               
                               @(kHIDUsage_KeyboardLeftArrow):  @(OEMSXKeyLeft),
                               @(kHIDUsage_KeyboardUpArrow):    @(OEMSXKeyUp),
                               @(kHIDUsage_KeyboardDownArrow):  @(OEMSXKeyDown),
                               @(kHIDUsage_KeyboardRightArrow): @(OEMSXKeyRight),

                               @(kHIDUsage_Keypad7):        @(OEMSXKeyNumpad7),
                               @(kHIDUsage_Keypad8):        @(OEMSXKeyNumpad8),
                               @(kHIDUsage_Keypad9):        @(OEMSXKeyNumpad9),
                               @(kHIDUsage_KeypadSlash):    @(OEMSXKeyNumpadSlash),
                               @(kHIDUsage_Keypad4):        @(OEMSXKeyNumpad4),
                               @(kHIDUsage_Keypad5):        @(OEMSXKeyNumpad5),
                               @(kHIDUsage_Keypad6):        @(OEMSXKeyNumpad6),
                               @(kHIDUsage_KeypadAsterisk): @(OEMSXKeyNumpadStar),
                               @(kHIDUsage_Keypad1):        @(OEMSXKeyNumpad1),
                               @(kHIDUsage_Keypad2):        @(OEMSXKeyNumpad2),
                               @(kHIDUsage_Keypad3):        @(OEMSXKeyNumpad3),
                               @(kHIDUsage_KeypadHyphen):   @(OEMSXKeyNumpadMinus),
                               @(kHIDUsage_Keypad0):        @(OEMSXKeyNumpad0),
                               @(kHIDUsage_KeypadPeriod):   @(OEMSXKeyNumpadPoint),
                               @(kHIDUsage_KeypadComma):    @(OEMSXKeyNumpadComma),
                               @(kHIDUsage_KeypadPlus):     @(OEMSXKeyNumpadPlus),
                               };
}

+ (Protocol *)gameSystemResponderClientProtocol;
{
    return @protocol(OEMSXSystemResponderClient);
}

- (void)pressEmulatorKey:(OESystemKey *)aKey
{
    [self.client didPushMSXJoystickButton:(OEMSXJoystickButton)aKey.key
                                 controller:aKey.player];
}

- (void)releaseEmulatorKey:(OESystemKey *)aKey
{
    [self.client didReleaseMSXJoystickButton:(OEMSXJoystickButton)aKey.key
                                    controller:aKey.player];
}

- (void)HIDKeyDown:(OEHIDEvent *)anEvent
{
    [super HIDKeyDown:anEvent];

    NSNumber *virtualCode = [virtualPhysicalKeyMap objectForKey:@(anEvent.keycode)];
    if(virtualCode)
        [self.client didPressKey:virtualCode.intValue];
}

- (void)HIDKeyUp:(OEHIDEvent *)anEvent
{
    [super HIDKeyUp:anEvent];

    NSNumber *virtualCode = [virtualPhysicalKeyMap objectForKey:@(anEvent.keycode)];
    if(virtualCode)
        [self.client didReleaseKey:virtualCode.intValue];
}

@end
