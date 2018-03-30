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

#import "OEThemeObject.h"

#pragma mark - Input state names

// Input state tokens used by OEThemeStateFromString to parse an NSString into an OEThemeState. The 'Default' token takes precedent over any other token and will automatically set the OEThemeState to OEThemeStateDefault.
static NSString * const OEThemeInputStateDefaultName        = @"Default";
static NSString * const OEThemeInputStateWindowInactiveName = @"Window Inactive";
static NSString * const OEThemeInputStateWindowActiveName   = @"Window Active";
static NSString * const OEThemeInputStateToggleOffName      = @"Toggle Off";
static NSString * const OEThemeInputStateToggleOnName       = @"Toggle On";
static NSString * const OEThemeInputStateToggleMixedName    = @"Toggle Mixed";
static NSString * const OEThemeInputStateUnpressedName      = @"Unpressed";
static NSString * const OEThemeInputStatePressedName        = @"Pressed";
static NSString * const OEThemeInputStateDisabledName       = @"Disabled";
static NSString * const OEThemeInputStateEnabledName        = @"Enabled";
static NSString * const OEThemeInputStateUnfocusedName      = @"Unfocused";
static NSString * const OEThemeInputStateFocusedName        = @"Focused";
static NSString * const OEThemeInputStateMouseOverName      = @"Mouse Over";
static NSString * const OEThemeInputStateMouseOffName       = @"Mouse Off";

static NSString * const OEThemeInputStateModifierNoneName = @"No Modifier";
static NSString * const OEThemeInputStateModifierAlternateName  = @"Alternate";

#pragma mark - Input state wild card names

// These are extended input state tokens to create OEThemeStates with a 'wild card' for the input states specified. If a particular input state is left unspecified, these wild cards are implicitly specified.
static NSString * const OEThemeStateAnyWindowActivityName   = @"Any Window State";
static NSString * const OEThemeStateAnyToggleName           = @"Any Toggle";
static NSString * const OEThemeStateAnySelectionName        = @"Any Selection";
static NSString * const OEThemeStateAnyInteractionName      = @"Any Interaction";
static NSString * const OEThemeStateAnyFocusName            = @"Any Focus";
static NSString * const OEThemeStateAnyMouseName            = @"Any Mouse State";
static NSString * const OEThemeStateAnyModifierName         = @"Any Modifier State";

#pragma mark - Common theme object attributes

NSString * const OEThemeObjectStatesAttributeName = @"States";
NSString * const OEThemeObjectValueAttributeName  = @"Value";


#pragma mark - Implementation

static inline id OEKeyForState(OEThemeState state) {
    // Implicitly define a zero state as the default state
    return @((state == 0 ? OEThemeStateDefault : state));
}

@interface OEThemeObject () {
    NSMutableDictionary <NSNumber *, id> *_objectByState;  // State table
    NSMutableArray <NSNumber *> *_states;              // Used for implicit selection of object for desired state
}

- (void)OE_setValue:(id)value forState:(OEThemeState)state;

@end

@implementation OEThemeObject

- (instancetype)initWithDefinition:(id)definition {
    
    if ((self = [super init])) {
        
        _states = [[NSMutableArray alloc] init];
        _objectByState = [[NSMutableDictionary alloc] init];

        if ([definition isKindOfClass:[NSDictionary class]]) {
            
            // Create a root definition that can be inherited by the states
            NSMutableDictionary *rootDefinition = [definition mutableCopy];
            [rootDefinition removeObjectForKey:OEThemeObjectStatesAttributeName];
            [self OE_setValue:[[self class] parseWithDefinition:rootDefinition] forState:OEThemeStateDefault];

            // Iterate through each of the state descriptions and create a state table
            NSDictionary *states = [definition valueForKey:OEThemeObjectStatesAttributeName];
            if ([states isKindOfClass:[NSDictionary class]]) {
                
                [states enumerateKeysAndObjectsUsingBlock:^(NSString *key, id obj, BOOL *stop) {
                    
                    NSMutableDictionary *newDefinition = [rootDefinition mutableCopy];
                    if ([obj isKindOfClass:[NSDictionary class]]) {
                        [newDefinition setValuesForKeysWithDictionary:obj];
                    } else {
                        newDefinition[OEThemeObjectValueAttributeName] = obj;
                    }
                    
                    NSString *trimmedKey = [key stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceCharacterSet];
                    OEThemeState state = (trimmedKey.length == 0 ? OEThemeStateDefault : OEThemeStateFromString(trimmedKey));
                    
                    if (state == 0) {
                        NSLog(@"Error parsing state: %@", trimmedKey);
                    } else {
                        [self OE_setValue:[[self class] parseWithDefinition:newDefinition] forState:state];
                        
                        // Append the state to the state mask
                        if(state != OEThemeStateDefault) self->_stateMask |= state;
                    }
                }];
            }
            
            if (_stateMask != OEThemeStateDefault) {
                
                // Aggregate the bit-mask that all the state's cover
                if (_stateMask & OEThemeStateAnyWindowActivity) {
                    _stateMask |= OEThemeStateAnyWindowActivity;
                }
                if (_stateMask & OEThemeStateAnyToggle) {
                    _stateMask |= OEThemeStateAnyToggle;
                }
                if (_stateMask & OEThemeStateAnySelection) {
                    _stateMask |= OEThemeStateAnySelection;
                }
                if (_stateMask & OEThemeStateAnyInteraction) {
                    _stateMask |= OEThemeStateAnyInteraction;
                }
                if (_stateMask & OEThemeStateAnyFocus) {
                    _stateMask |= OEThemeStateAnyFocus;
                }
                if (_stateMask & OEThemeStateAnyMouse) {
                    _stateMask |= OEThemeStateAnyMouse;
                }
                if (_stateMask & OEThemeStateAnyModifier) {
                    _stateMask |= OEThemeStateAnyModifier;
                }

                // Iterate through each state to determine if unspecified inputs should be discarded
                BOOL updateStates = FALSE;
                for (NSNumber *obj in _states) {
                    
                    OEThemeState state = obj.unsignedIntegerValue;
                    
                    if (state != OEThemeStateDefault) {
                        
                        // Implicitly set any unspecified input state with it's wild card counter part
                        if (!(state & OEThemeStateAnyWindowActivity)) {
                            state |= OEThemeStateAnyWindowActivity;
                        }
                        if (!(state & OEThemeStateAnyToggle)) {
                            state |= OEThemeStateAnyToggle;
                        }
                        if (!(state & OEThemeStateAnySelection)) {
                            state |= OEThemeStateAnySelection;
                        }
                        if (!(state & OEThemeStateAnyInteraction)) {
                            state |= OEThemeStateAnyInteraction;
                        }
                        if (!(state & OEThemeStateAnyFocus)) {
                            state |= OEThemeStateAnyFocus;
                        }
                        if (!(state & OEThemeStateAnyMouse)) {
                            state |= OEThemeStateAnyMouse;
                        }
                        if (!(state & OEThemeStateAnyModifier)) {
                            state |= OEThemeStateAnyModifier;
                        }
                        
                        // Trim bits not specified in the state mask
                        state &= _stateMask;
                        
                        // Update state table if the state was modified
                        if (state != obj.unsignedIntegerValue) {
                            _objectByState[OEKeyForState(state)] = _objectByState[obj];
                            [_objectByState removeObjectForKey:obj];
                            updateStates = YES;
                        }
                    }
                }

                // If the state table was modified then get a sorted list of states that can be used by -objectForState:
                if (updateStates) {
                    _states = [[_objectByState.allKeys sortedArrayUsingSelector:@selector(compare:)] mutableCopy];
                }
            }
        } else {
            NSDictionary <NSString *, id> *newDefinition = @{ OEThemeObjectValueAttributeName : definition };
            [self OE_setValue:[[self class] parseWithDefinition:newDefinition] forState:OEThemeStateDefault];
        }
    }
    return self;
}

+ (id)parseWithDefinition:(id)definition {
    // It is critical that the subclass implements this method
    [self doesNotRecognizeSelector:_cmd];
    return nil;
}

+ (OEThemeState)themeStateWithWindowActive:(BOOL)windowActive buttonState:(NSCellStateValue)state selected:(BOOL)selected enabled:(BOOL)enabled focused:(BOOL)focused houseHover:(BOOL)hover modifierMask:(NSUInteger)modifierMask {
    return ((windowActive ? OEThemeInputStateWindowActive : OEThemeInputStateWindowInactive) |
            (selected ? OEThemeInputStatePressed : OEThemeInputStateUnpressed) |
            (enabled ? OEThemeInputStateEnabled : OEThemeInputStateDisabled) |
            (focused ? OEThemeInputStateFocused : OEThemeInputStateUnfocused) |
            (hover ? OEThemeInputStateMouseOver : OEThemeInputStateMouseOff) |

            ((modifierMask & NSAlternateKeyMask) != 0 ? OEThemeInputStateModifierAlternate : OEThemeInputStateModifierNone) |
            
            (state == NSOnState ? OEThemeInputStateToggleOn : (state == NSMixedState ? OEThemeInputStateToggleMixed : OEThemeInputStateToggleOff)));
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: states = [%@]>", [self className], [_objectByState.allKeys componentsJoinedByString:@", "]];
}

- (void)OE_setValue:(id)value forState:(OEThemeState)state {
    
    // Assign the state look up table
    _objectByState[OEKeyForState(state)] = (value ?: [NSNull null]);
    
    // Insert the state in the states array (while maintaining a sorted array)
    const NSUInteger  count = _states.count;
    NSNumber *stateValue = @(state);

    if (count > 0) {
        
        NSUInteger index = [_states indexOfObject:stateValue inSortedRange:NSMakeRange(0, [_states count]) options:NSBinarySearchingFirstEqual | NSBinarySearchingInsertionIndex usingComparator:^NSComparisonResult(id obj1, id obj2) {
            return [obj1 compare:obj2];
        }];

        if (![_states[index] isEqualToNumber:stateValue]) {
            [_states insertObject:stateValue atIndex:index];
        }
    } else {
        [_states addObject:stateValue];
    }
}

- (id)objectForState:(OEThemeState)state {
    
    OEThemeState maskedState = state & _stateMask; // Trim unused bits
    id results = nil;

    if (maskedState == 0) {
        results = _objectByState[OEKeyForState(OEThemeStateDefault)];
    } else {
        
        // Return object explicitly defined by state
        results = _objectByState[OEKeyForState(maskedState)];
        if (results == nil) {
            
            // Try to implicitly determine what object represents the supplied state
            for (NSNumber *obj in _states) {
                
                const OEThemeState state = obj.unsignedIntegerValue;
                if ((maskedState & state) == maskedState) {
                    
                    // This state is the best we are going to get for the requested state
                    results = _objectByState[OEKeyForState(state)];
                    
                    // Explicitly set the state to the implicitly discovered object so the next time we are looking for this state we can short circuit this process
                    if (state != 0 && state != OEThemeStateDefault) {
                        [self OE_setValue:results forState:maskedState];
                    }
                    
                    break;
                }
            }

            // If no object was found, then explicitly set it to Null, so the next time we try to obtain an object for the specified state we will quickly return nil
            if (results == nil) {
                [self OE_setValue:[NSNull null] forState:maskedState];
            }
        }
    }

    return (results == [NSNull null] ? nil : results);
}

@end

NSString *NSStringFromThemeState(OEThemeState state) {
    
    NSMutableArray *results = [NSMutableArray array];

    // Empty states implicitly represent the 'Default' state
    if (state == 0 || state == OEThemeStateDefault) {
        [results addObject:OEThemeInputStateDefaultName];
    } else
    {
        if ((state & OEThemeStateAnyWindowActivity) == OEThemeStateAnyWindowActivity) {
            [results addObject:OEThemeStateAnyWindowActivityName];
        } else if (state & OEThemeInputStateWindowActive) {
            [results addObject:OEThemeInputStateWindowActiveName];
        } else if (state & OEThemeInputStateWindowInactive) {
            [results addObject:OEThemeInputStateWindowInactiveName];
        }

        if ((state & OEThemeStateAnyToggle) == OEThemeStateAnyToggle) {
            [results addObject:OEThemeStateAnyToggleName];
        } else if (state & OEThemeInputStateToggleOn) {
            [results addObject:OEThemeInputStateToggleOnName];
        } else if (state & OEThemeInputStateToggleMixed) {
            [results addObject:OEThemeInputStateToggleMixedName];
        } else if (state & OEThemeInputStateToggleOff) {
            [results addObject:OEThemeInputStateToggleOffName];
        }

        if ((state & OEThemeStateAnySelection) == OEThemeStateAnySelection) {
            [results addObject:OEThemeStateAnySelectionName];
        } else if (state & OEThemeInputStatePressed) {
            [results addObject:OEThemeInputStatePressedName];
        } else if (state & OEThemeInputStateUnpressed) {
            [results addObject:OEThemeInputStateUnpressedName];
        }

        if ((state & OEThemeStateAnyInteraction) == OEThemeStateAnyInteraction) {
            [results addObject:OEThemeStateAnyInteractionName];
        } else if (state & OEThemeInputStateEnabled) {
            [results addObject:OEThemeInputStateEnabledName];
        } else if (state & OEThemeInputStateDisabled) {
            [results addObject:OEThemeInputStateDisabledName];
        }

        if ((state & OEThemeStateAnyFocus) == OEThemeStateAnyFocus) {
            [results addObject:OEThemeStateAnyFocusName];
        } else if (state & OEThemeInputStateFocused) {
            [results addObject:OEThemeInputStateFocusedName];
        } else if (state & OEThemeInputStateUnfocused) {
            [results addObject:OEThemeInputStateUnfocusedName];
        }
        
        if ((state & OEThemeStateAnyMouse) == OEThemeStateAnyMouse) {
            [results addObject:OEThemeStateAnyMouseName];
        } else if (state & OEThemeInputStateMouseOver) {
            [results addObject:OEThemeInputStateMouseOverName];
        } else if (state & OEThemeInputStateMouseOff) {
            [results addObject:OEThemeInputStateMouseOffName];
        }
        
        if ((state & OEThemeStateAnyModifier) == OEThemeStateAnyModifier) {
            [results addObject:OEThemeStateAnyModifierName];
        } else if (state & OEThemeInputStateModifierNone) {
            [results addObject:OEThemeInputStateModifierNoneName];
        } else if (state & OEThemeInputStateModifierAlternate) {
            [results addObject:OEThemeInputStateModifierAlternateName];
        }
    }

    return [results componentsJoinedByString:@", "];
}

OEThemeState OEThemeStateFromString(NSString *state) {
    
    OEThemeState result = 0;
    NSCharacterSet *whitespace = NSCharacterSet.whitespaceCharacterSet;
    
    for (NSString *obj in [state componentsSeparatedByString:@","]) {
        
        NSString *component = [obj stringByTrimmingCharactersInSet:whitespace];
        
        if ([component caseInsensitiveCompare:OEThemeInputStateDefaultName] == NSOrderedSame) {
            // The 'Default' input state takes precendent over any other input state
            result = OEThemeStateDefault;
            break;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnyWindowActivityName]       == NSOrderedSame) {
            result |= OEThemeStateAnyWindowActivity;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnyToggleName]               == NSOrderedSame) {
            result |= OEThemeStateAnyToggle;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnySelectionName]            == NSOrderedSame) {
            result |= OEThemeStateAnySelection;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnyInteractionName]          == NSOrderedSame) {
            result |= OEThemeStateAnyInteraction;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnyModifierName]             == NSOrderedSame) {
            result |= OEThemeStateAnyModifier;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnyFocusName]                == NSOrderedSame) {
            result |= OEThemeStateAnyFocus;
        } else if ([component caseInsensitiveCompare:OEThemeStateAnyMouseName]                == NSOrderedSame) {
            result |= OEThemeStateAnyMouse;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateWindowInactiveName]     == NSOrderedSame) {
            result |= OEThemeInputStateWindowInactive;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateWindowActiveName]       == NSOrderedSame) {
            result |= OEThemeInputStateWindowActive;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateToggleOffName]          == NSOrderedSame) {
            result |= OEThemeInputStateToggleOff;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateToggleOnName]           == NSOrderedSame) {
            result |= OEThemeInputStateToggleOn;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateToggleMixedName]        == NSOrderedSame) {
            result |= OEThemeInputStateToggleMixed;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateUnpressedName]          == NSOrderedSame) {
            result |= OEThemeInputStateUnpressed;
        } else if ([component caseInsensitiveCompare:OEThemeInputStatePressedName]            == NSOrderedSame) {
            result |= OEThemeInputStatePressed;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateDisabledName]           == NSOrderedSame) {
            result |= OEThemeInputStateDisabled;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateEnabledName]            == NSOrderedSame) {
            result |= OEThemeInputStateEnabled;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateUnfocusedName]          == NSOrderedSame) {
            result |= OEThemeInputStateUnfocused;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateFocusedName]            == NSOrderedSame) {
            result |= OEThemeInputStateFocused;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateMouseOffName]           == NSOrderedSame) {
            result |= OEThemeInputStateMouseOff;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateMouseOverName]          == NSOrderedSame) {
            result |= OEThemeInputStateMouseOver;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateModifierAlternateName]  == NSOrderedSame) {
            result |= OEThemeInputStateModifierAlternate;
        } else if ([component caseInsensitiveCompare:OEThemeInputStateModifierNoneName]       == NSOrderedSame) {
            result |= OEThemeInputStateModifierNone;
        } else {
            NSLog(@"- Unknown State Input: %@", component);
        }
    }
    
    // Implicitly return the default state, if no input state was specified
    return result;
}
