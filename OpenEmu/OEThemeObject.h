/*
 Copyright (c) 2016, OpenEmu Team

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

@import Foundation;

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Input state masks

/*
 The following are available state inputs that a state mask is composed of:
 - Window Activity
 - Toggle State
 - Pressed State
 - Interaction State
 - Mouse State
 - Modifier State

 When all 5 state inputs are either zero's (0x0000) or ones (0xFFFF), it is considered a default state mask. When a
 looking for the most appropriate object, based a system's state, the default state mask is used if no other state mask
 could be found.
 */
enum
{
    OEThemeInputStateWindowInactive = 1 <<  0,
    OEThemeInputStateWindowActive   = 1 <<  1,
    OEThemeInputStateToggleOff      = 1 <<  2,
    OEThemeInputStateToggleOn       = 1 <<  3,
    OEThemeInputStateToggleMixed    = 1 <<  4,
    OEThemeInputStateUnpressed      = 1 <<  5,
    OEThemeInputStatePressed        = 1 <<  6,
    OEThemeInputStateDisabled       = 1 <<  7,
    OEThemeInputStateEnabled        = 1 <<  8,
    OEThemeInputStateUnfocused      = 1 <<  9,
    OEThemeInputStateFocused        = 1 << 10,
    OEThemeInputStateMouseOff       = 1 << 11,
    OEThemeInputStateMouseOver      = 1 << 12,
    
    OEThemeInputStateModifierNone        = 1 << 13,
    OEThemeInputStateModifierAlternate   = 1 << 14,
};

#pragma mark - Input state wild card masks

/*
 In the Theme.plist you can define an 'Any' input state for input states that should be ignored when determining which
 object to apply.  'Any' input states can be set explicitly, if an input state is unspecified then the 'Any' mask is
 set implicitly.
 */
enum
{
    OEThemeStateAnyWindowActivity = OEThemeInputStateWindowInactive | OEThemeInputStateWindowActive,
    OEThemeStateAnyToggle         = OEThemeInputStateToggleOff      | OEThemeInputStateToggleOn      | OEThemeInputStateToggleMixed,
    OEThemeStateAnySelection      = OEThemeInputStateUnpressed      | OEThemeInputStatePressed,
    OEThemeStateAnyInteraction    = OEThemeInputStateDisabled       | OEThemeInputStateEnabled,
    OEThemeStateAnyFocus          = OEThemeInputStateUnfocused      | OEThemeInputStateFocused,
    OEThemeStateAnyMouse          = OEThemeInputStateMouseOff       | OEThemeInputStateMouseOver,
    OEThemeStateAnyModifier       = OEThemeInputStateModifierNone   | OEThemeInputStateModifierAlternate,
    OEThemeStateDefault           = 0xFFFFF,
};
typedef NSUInteger OEThemeState;

#pragma mark - Common theme object attributes

extern NSString * const OEThemeObjectStatesAttributeName;
extern NSString * const OEThemeObjectValueAttributeName;

#pragma mark - Implementation

/// Retrieves an NSString from an OEThemeState
extern NSString *NSStringFromThemeState(OEThemeState state);

/// Parses an NSString into an OEThemeState, the NSString is a comma separated list of tokens. The order of the token's appearance has no effect on the final value.
extern OEThemeState  OEThemeStateFromString(NSString *state);

@interface OEThemeObject : NSObject

- (instancetype)initWithDefinition:(id)definition;

/// Must be overridden by subclasses to be able to parse customized UI element
+ (id _Nullable)parseWithDefinition:(NSDictionary *)definition;

/// Convenience function for retrieving an OEThemeState based on the supplied inputs
+ (OEThemeState)themeStateWithWindowActive:(BOOL)windowActive buttonState:(NSCellStateValue)state selected:(BOOL)selected enabled:(BOOL)enabled focused:(BOOL)focused houseHover:(BOOL)hover modifierMask:(NSUInteger)modifierMask;

/// Retrieves UI object for state specified
- (id)objectForState:(OEThemeState)state;

/// Aggregate mask that filters out any unspecified state input
@property (nonatomic, readonly) NSUInteger stateMask;

@end

NS_ASSUME_NONNULL_END
