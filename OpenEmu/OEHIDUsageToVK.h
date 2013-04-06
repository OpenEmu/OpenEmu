/*
 Copyright (c) 2010, OpenEmu Team
 
 
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

#import <Carbon/Carbon.h>
#import <IOKit/hid/IOHIDUsageTables.h>

typedef struct
{
    NSUInteger hidCode;
    CGKeyCode vkCode;
    __unsafe_unretained NSString *string;
} _OEHIDVirtualKeyCodeNameTriplet;

static const _OEHIDVirtualKeyCodeNameTriplet _OEHIDVirtualKeyCodesTable[] = {
    { kHIDUsage_Keyboard0                  , kVK_ANSI_0             , nil          },
    { kHIDUsage_Keyboard1                  , kVK_ANSI_1             , nil          },
    { kHIDUsage_Keyboard2                  , kVK_ANSI_2             , nil          },
    { kHIDUsage_Keyboard3                  , kVK_ANSI_3             , nil          },
    { kHIDUsage_Keyboard4                  , kVK_ANSI_4             , nil          },
    { kHIDUsage_Keyboard5                  , kVK_ANSI_5             , nil          },
    { kHIDUsage_Keyboard6                  , kVK_ANSI_6             , nil          },
    { kHIDUsage_Keyboard7                  , kVK_ANSI_7             , nil          },
    { kHIDUsage_Keyboard8                  , kVK_ANSI_8             , nil          },
    { kHIDUsage_Keyboard9                  , kVK_ANSI_9             , nil          },
    { kHIDUsage_Keyboard0                  , kVK_ANSI_0             , nil          },
    { kHIDUsage_KeyboardA                  , kVK_ANSI_A             , nil          },
    { kHIDUsage_KeyboardB                  , kVK_ANSI_B             , nil          },
    { kHIDUsage_KeyboardC                  , kVK_ANSI_C             , nil          },
    { kHIDUsage_KeyboardD                  , kVK_ANSI_D             , nil          },
    { kHIDUsage_KeyboardE                  , kVK_ANSI_E             , nil          },
    { kHIDUsage_KeyboardF                  , kVK_ANSI_F             , nil          },
    { kHIDUsage_KeyboardG                  , kVK_ANSI_G             , nil          },
    { kHIDUsage_KeyboardH                  , kVK_ANSI_H             , nil          },
    { kHIDUsage_KeyboardI                  , kVK_ANSI_I             , nil          },
    { kHIDUsage_KeyboardJ                  , kVK_ANSI_J             , nil          },
    { kHIDUsage_KeyboardK                  , kVK_ANSI_K             , nil          },
    { kHIDUsage_KeyboardL                  , kVK_ANSI_L             , nil          },
    { kHIDUsage_KeyboardM                  , kVK_ANSI_M             , nil          },
    { kHIDUsage_KeyboardN                  , kVK_ANSI_N             , nil          },
    { kHIDUsage_KeyboardO                  , kVK_ANSI_O             , nil          },
    { kHIDUsage_KeyboardP                  , kVK_ANSI_P             , nil          },
    { kHIDUsage_KeyboardQ                  , kVK_ANSI_Q             , nil          },
    { kHIDUsage_KeyboardR                  , kVK_ANSI_R             , nil          },
    { kHIDUsage_KeyboardS                  , kVK_ANSI_S             , nil          },
    { kHIDUsage_KeyboardT                  , kVK_ANSI_T             , nil          },
    { kHIDUsage_KeyboardU                  , kVK_ANSI_U             , nil          },
    { kHIDUsage_KeyboardV                  , kVK_ANSI_V             , nil          },
    { kHIDUsage_KeyboardW                  , kVK_ANSI_W             , nil          },
    { kHIDUsage_KeyboardX                  , kVK_ANSI_X             , nil          },
    { kHIDUsage_KeyboardY                  , kVK_ANSI_Y             , nil          },
    { kHIDUsage_KeyboardZ                  , kVK_ANSI_Z             , nil          },
    { kHIDUsage_KeyboardGraveAccentAndTilde, kVK_ANSI_Grave         , nil          }, // Grave Accent and Tilde
    { kHIDUsage_KeyboardHyphen             , kVK_ANSI_Minus         , nil          }, // - or _
    { kHIDUsage_KeyboardEqualSign          , kVK_ANSI_Equal         , nil          }, // = or +
    { kHIDUsage_KeyboardQuote              , kVK_ANSI_Quote         , nil          }, // ' or "
    { kHIDUsage_KeyboardOpenBracket        , kVK_ANSI_LeftBracket   , nil          }, // [ or {
    { kHIDUsage_KeyboardCloseBracket       , kVK_ANSI_RightBracket  , nil          }, // ] or }
    { kHIDUsage_KeyboardBackslash          , kVK_ANSI_Backslash     , nil          }, // \ or |
    { kHIDUsage_KeyboardSemicolon          , kVK_ANSI_Semicolon     , nil          }, // ; or :
    { kHIDUsage_KeyboardComma              , kVK_ANSI_Comma         , nil          }, // , or <
    { kHIDUsage_KeyboardPeriod             , kVK_ANSI_Period        , nil          }, // . or >
    { kHIDUsage_KeyboardSlash              , kVK_ANSI_Slash         , nil          }, // / or ?
    { kHIDUsage_KeyboardReturnOrEnter      , kVK_Return             , @"↩"         },
    { kHIDUsage_KeyboardTab                , kVK_Tab                , @"⇥"         },
    { kHIDUsage_KeyboardSpacebar           , kVK_Space              , @"Space"     },
    { kHIDUsage_KeyboardDeleteOrBackspace  , kVK_Delete             , @"⌫"         },
    { kHIDUsage_KeyboardEscape             , kVK_Escape             , @"⎋"         },
    { kHIDUsage_KeyboardRightGUI           , 0x36                   , @"Right ⌘"   }, // Weirdly enough this value is not explicitly given in the VK enum...
    { kHIDUsage_KeyboardLeftGUI            , kVK_Command            , @"Left ⌘"    },
    { kHIDUsage_KeyboardLeftShift          , kVK_Shift              , @"Left ⇧"    },
    { kHIDUsage_KeyboardCapsLock           , kVK_CapsLock           , @"Caps Lock" },
    { kHIDUsage_KeyboardLeftAlt            , kVK_Option             , @"Left ⌥"    },
    { kHIDUsage_KeyboardLeftControl        , kVK_Control            , @"Left ⌃"    },
    { kHIDUsage_KeyboardRightShift         , kVK_RightShift         , @"Right ⇧"   },
    { kHIDUsage_KeyboardRightAlt           , kVK_RightOption        , @"Right ⌥"   },
    { kHIDUsage_KeyboardRightControl       , kVK_RightControl       , @"Right ⌃"   },
    { kHIDUsage_KeyboardF1                 , kVK_F1                 , @"F1"        },
    { kHIDUsage_KeyboardF2                 , kVK_F2                 , @"F2"        },
    { kHIDUsage_KeyboardF3                 , kVK_F3                 , @"F3"        },
    { kHIDUsage_KeyboardF4                 , kVK_F4                 , @"F4"        },
    { kHIDUsage_KeyboardF5                 , kVK_F5                 , @"F5"        },
    { kHIDUsage_KeyboardF6                 , kVK_F6                 , @"F6"        },
    { kHIDUsage_KeyboardF7                 , kVK_F7                 , @"F7"        },
    { kHIDUsage_KeyboardF8                 , kVK_F8                 , @"F8"        },
    { kHIDUsage_KeyboardF9                 , kVK_F9                 , @"F9"        },
    { kHIDUsage_KeyboardF10                , kVK_F10                , @"F10"       },
    { kHIDUsage_KeyboardF11                , kVK_F11                , @"F11"       },
    { kHIDUsage_KeyboardF12                , kVK_F12                , @"F12"       },
    { kHIDUsage_KeyboardF13                , kVK_F13                , @"F13"       },
    { kHIDUsage_KeyboardF14                , kVK_F14                , @"F14"       },
    { kHIDUsage_KeyboardF15                , kVK_F15                , @"F15"       },
    { kHIDUsage_KeyboardF16                , kVK_F16                , @"F16"       },
    { kHIDUsage_KeyboardF17                , kVK_F17                , @"F17"       },
    { kHIDUsage_KeyboardF18                , kVK_F18                , @"F18"       },
    { kHIDUsage_KeyboardF19                , kVK_F19                , @"F19"       },
    { kHIDUsage_KeyboardF20                , kVK_F20                , @"F20"       },
    { kHIDUsage_KeyboardF21                , 0xFFFF                 , @"F21"       },
    { kHIDUsage_KeyboardF22                , 0xFFFF                 , @"F22"       },
    { kHIDUsage_KeyboardF23                , 0xFFFF                 , @"F23"       },
    { kHIDUsage_KeyboardF24                , 0xFFFF                 , @"F24"       },
    { kHIDUsage_KeyboardHome               , kVK_Home               , @"Home"      },
    { kHIDUsage_KeyboardPageUp             , kVK_PageUp             , @"Page Up"   },
    { kHIDUsage_KeyboardDeleteForward      , kVK_ForwardDelete      , nil          },
    { kHIDUsage_KeyboardEnd                , kVK_End                , @"End"       },
    { kHIDUsage_KeyboardPageDown           , kVK_PageDown           , @"Page Down" },
    { kHIDUsage_KeyboardLeftArrow          , kVK_LeftArrow          , @"←"         },
    { kHIDUsage_KeyboardRightArrow         , kVK_RightArrow         , @"→"         },
    { kHIDUsage_KeyboardDownArrow          , kVK_DownArrow          , @"↓"         },
    { kHIDUsage_KeyboardUpArrow            , kVK_UpArrow            , @"↑"         },
    { kHIDUsage_KeypadSlash                , kVK_ANSI_KeypadDivide  , @"Num /"     },
    { kHIDUsage_KeypadAsterisk             , kVK_ANSI_KeypadMultiply, @"Num *"     },
    { kHIDUsage_KeypadHyphen               , kVK_ANSI_KeypadMinus   , @"Num -"     },
    { kHIDUsage_KeypadPlus                 , kVK_ANSI_KeypadPlus    , @"Num +"     },
    { kHIDUsage_KeypadEnter                , kVK_ANSI_KeypadEnter   , @"Num Enter" },
    { kHIDUsage_KeypadEqualSign            , kVK_ANSI_KeypadEquals  , @"Num ="     },
    { kHIDUsage_Keypad1                    , kVK_ANSI_Keypad1       , @"Num 1"     },
    { kHIDUsage_Keypad2                    , kVK_ANSI_Keypad2       , @"Num 1"     },
    { kHIDUsage_Keypad3                    , kVK_ANSI_Keypad3       , @"Num 1"     },
    { kHIDUsage_Keypad4                    , kVK_ANSI_Keypad4       , @"Num 1"     },
    { kHIDUsage_Keypad5                    , kVK_ANSI_Keypad5       , @"Num 5"     },
    { kHIDUsage_Keypad6                    , kVK_ANSI_Keypad6       , @"Num 6"     },
    { kHIDUsage_Keypad7                    , kVK_ANSI_Keypad7       , @"Num 7"     },
    { kHIDUsage_Keypad8                    , kVK_ANSI_Keypad8       , @"Num 8"     },
    { kHIDUsage_Keypad9                    , kVK_ANSI_Keypad9       , @"Num 9"     },
    { kHIDUsage_Keypad0                    , kVK_ANSI_Keypad0       , @"Num 0"     },
    { kHIDUsage_KeypadPeriod               , kVK_ANSI_KeypadDecimal , @"Num ."     },
    { kHIDUsage_KeypadNumLock              , kVK_ANSI_KeypadClear   , @"Num Lock"  },
    { kHIDUsage_KeyboardPageUp             , kVK_PageUp             , @"Page Up"   },
    { kHIDUsage_KeyboardPageDown           , kVK_PageDown           , @"Page Down" },
    { kHIDUsage_KeyboardNonUSPound         , 0xFFFF                 , @"#"         },
    { kHIDUsage_KeyboardNonUSBackslash     , 0xFFFF                 , @"|"         },
};
