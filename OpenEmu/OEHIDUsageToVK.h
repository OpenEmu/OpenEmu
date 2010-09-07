/*
 *  OEHIDUsageToVK.h
 *  OpenEmu
 *
 *  Created by Josh Weinberg on 9/7/10.
 *  Copyright 2010 Roundarch Inc. All rights reserved.
 *
 */

#import <Carbon/Carbon.h>
#import <IOKit/hid/IOHIDUsageTables.h>

typedef struct
{
	NSUInteger hidCode;
	CGKeyCode vkCode;
}OE_HIDVKCode;

OE_HIDVKCode hidvk_codes[] = {
	{kHIDUsage_Keyboard0, kVK_ANSI_0},
	{kHIDUsage_Keyboard1, kVK_ANSI_1},
	{kHIDUsage_Keyboard2, kVK_ANSI_2},
	{kHIDUsage_Keyboard3, kVK_ANSI_3},
	{kHIDUsage_Keyboard4, kVK_ANSI_4},
	{kHIDUsage_Keyboard5, kVK_ANSI_5},
	{kHIDUsage_Keyboard6, kVK_ANSI_6},
	{kHIDUsage_Keyboard7, kVK_ANSI_7},
	{kHIDUsage_Keyboard8, kVK_ANSI_8},
	{kHIDUsage_Keyboard9, kVK_ANSI_9},
	{kHIDUsage_Keyboard0, kVK_ANSI_0},
	{kHIDUsage_KeyboardA, kVK_ANSI_A},
	{kHIDUsage_KeyboardB, kVK_ANSI_B},
	{kHIDUsage_KeyboardC, kVK_ANSI_C},
	{kHIDUsage_KeyboardD, kVK_ANSI_D},
	{kHIDUsage_KeyboardE, kVK_ANSI_E},
	{kHIDUsage_KeyboardF, kVK_ANSI_F},
	{kHIDUsage_KeyboardG, kVK_ANSI_G},
	{kHIDUsage_KeyboardH, kVK_ANSI_H},
	{kHIDUsage_KeyboardI, kVK_ANSI_I},
	{kHIDUsage_KeyboardJ, kVK_ANSI_J},
	{kHIDUsage_KeyboardK, kVK_ANSI_K},
	{kHIDUsage_KeyboardL, kVK_ANSI_L},
	{kHIDUsage_KeyboardM, kVK_ANSI_M},
	{kHIDUsage_KeyboardN, kVK_ANSI_N},
	{kHIDUsage_KeyboardO, kVK_ANSI_O},
	{kHIDUsage_KeyboardP, kVK_ANSI_P},
	{kHIDUsage_KeyboardQ, kVK_ANSI_Q},
	{kHIDUsage_KeyboardR, kVK_ANSI_R},
	{kHIDUsage_KeyboardS, kVK_ANSI_S},
	{kHIDUsage_KeyboardT, kVK_ANSI_T},
	{kHIDUsage_KeyboardU, kVK_ANSI_U},
	{kHIDUsage_KeyboardV, kVK_ANSI_V},
	{kHIDUsage_KeyboardW, kVK_ANSI_W},
	{kHIDUsage_KeyboardX, kVK_ANSI_X},
	{kHIDUsage_KeyboardY, kVK_ANSI_Y},
	{kHIDUsage_KeyboardZ, kVK_ANSI_Z},
	{kHIDUsage_KeyboardGraveAccentAndTilde, kVK_ANSI_Grave},	/* Grave Accent and Tilde */
	{kHIDUsage_KeyboardHyphen, kVK_ANSI_Minus},	/* - or _ */
	{kHIDUsage_KeyboardEqualSign, kVK_ANSI_Equal},	/* = or + */
	{kHIDUsage_KeyboardQuote, kVK_ANSI_Quote},	/* ' or " */
	{kHIDUsage_KeyboardOpenBracket, kVK_ANSI_LeftBracket},	/* [ or { */
	{kHIDUsage_KeyboardCloseBracket, kVK_ANSI_RightBracket},	/* ] or } */
	{kHIDUsage_KeyboardBackslash, kVK_ANSI_Backslash},	/* \ or | */
	{kHIDUsage_KeyboardSemicolon, kVK_ANSI_Semicolon},	/* ; or : */
	{kHIDUsage_KeyboardComma, kVK_ANSI_Comma},	/* , or < */
	{kHIDUsage_KeyboardPeriod, kVK_ANSI_Period},	/* . or > */
	{kHIDUsage_KeyboardSlash, kVK_ANSI_Slash},	/* / or ? */
	
};

typedef struct
{
	NSUInteger hidCode;
	NSString * string;
}OE_HIDString;

OE_HIDString hidlabels[] = {

	{kHIDUsage_KeyboardReturnOrEnter, @"Enter"}, /* Return (Enter) */
	{kHIDUsage_KeyboardEscape, @"Escape"},/* Escape */
	{kHIDUsage_KeyboardDeleteOrBackspace, @"Delete"}, /* Delete (Backspace) */
	{kHIDUsage_KeyboardTab, @"Tab"},	/* Tab */
	{kHIDUsage_KeyboardSpacebar, @"Space"},	/* Spacebar */
	{kHIDUsage_KeyboardCapsLock, @"Caps Lock"},	/* Caps Lock */
	{kHIDUsage_KeyboardF1, @"F1"},
	{kHIDUsage_KeyboardF2, @"F2"},
	{kHIDUsage_KeyboardF3, @"F3"},
	{kHIDUsage_KeyboardF4, @"F4"},
	{kHIDUsage_KeyboardF5, @"F5"},
	{kHIDUsage_KeyboardF6, @"F6"},
	{kHIDUsage_KeyboardF7, @"F7"},
	{kHIDUsage_KeyboardF8, @"F8"},
	{kHIDUsage_KeyboardF9, @"F9"},
	{kHIDUsage_KeyboardF10, @"F10"},
	{kHIDUsage_KeyboardF11, @"F11"},
	{kHIDUsage_KeyboardF12, @"F12"},
	{kHIDUsage_KeyboardF13, @"F13"},
	{kHIDUsage_KeyboardF14, @"F14"},
	{kHIDUsage_KeyboardF15, @"F15"},
	{kHIDUsage_KeyboardF16, @"F16"},
	{kHIDUsage_KeyboardF17, @"F17"},
	{kHIDUsage_KeyboardF18, @"F18"},
	{kHIDUsage_KeyboardF19, @"F19"},
	{kHIDUsage_KeyboardF20, @"F20"},
	{kHIDUsage_KeyboardF21, @"F21"},
	{kHIDUsage_KeyboardF22, @"F22"},
	{kHIDUsage_KeyboardF23, @"F23"},
	{kHIDUsage_KeyboardF24, @"F24"},

	{kHIDUsage_KeyboardLeftControl, @"Left Control"},	/* Left Control */
	{kHIDUsage_KeyboardLeftShift, @"Left Shift"},	/* Left Shift */
	{kHIDUsage_KeyboardLeftAlt, @"Left Option"},	/* Left Alt */
	{kHIDUsage_KeyboardLeftGUI, @"Left Command"},	/* Left GUI */
	{kHIDUsage_KeyboardRightControl, @"Right Control"},	/* Right Control */
	{kHIDUsage_KeyboardRightShift, @"Right Shift"},	/* Right Shift */
	{kHIDUsage_KeyboardRightAlt, @"Right Option"},	/* Right Alt */
	{kHIDUsage_KeyboardRightGUI, @"Right Command"},	/* Right GUI */

	{kHIDUsage_KeyboardLeftArrow, @"Left"},	/* Right GUI */
	{kHIDUsage_KeyboardRightArrow, @"Right"},	/* Right GUI */
	{kHIDUsage_KeyboardUpArrow, @"Up"},	/* Right GUI */
	{kHIDUsage_KeyboardDownArrow, @"Down"},	/* Right GUI */
};