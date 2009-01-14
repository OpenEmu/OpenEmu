//
//  GameButton.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/8/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameButton.h"


@implementation KeyboardButton

@synthesize keyCode;
@synthesize button;
@synthesize character;

+ (NSString*) charForKey: (int) key
{
	switch(key)
	{
		/* The alphabet */
		case 0:
			return @"A";
		case 11:
			return @"B";
		case 8:
			return @"C";
		case 2:
			return @"D";
		case 14:
			return @"E";
		case 3:
			return @"F";
		case 5:
			return @"G";
		case 4:
			return @"H";
		case 34:
			return @"I";
		case 38:
			return @"J";
		case 40:
			return @"K";
		case 37:
			return @"L";
		case 46:
			return @"M";
		case 45:
			return @"N";
		case 31:
			return @"O";
		case 35:
			return @"P";
		case 12:
			return @"Q";
		case 15:
			return @"R";
		case 1:
			return @"S";
		case 17:
			return @"T";
		case 32:
			return @"U";
		case 9:
			return @"V";
		case 13:
			return @"W";
		case 7:
			return @"X";
		case 16:
			return @"Y";
		case 6:
			return @"Z";
			
		/* Numbers */
		case 29:
			return @"0";
		case 18:
			return @"1";
		case 19:
			return @"2";
		case 20:
			return @"3";
		case 21:
			return @"4";
		case 23:
			return @"5";
		case 22:
			return @"6";
		case 26:
			return @"7";
		case 28:
			return @"8";
		case 25:
			return @"9";
		
		case 27:
			return @"-";
		case 24:
			return @"=";
		case 10:
			return @"`";
		case 33:
			return @"[";
		case 30:
			return @"]";
		case 41:
			return @";";
		case 39:
			return @"'";
		case 43:
			return @",";
		case 47:
			return @".";
		case 44:
			return @"/";
		case 42:
			return @"\\";
			
		/* "Special" keys */
			
		case 49:
			return @"Space";
		case 36:
			return @"Return";
		case 117:
			return @"Delete";
		case 48:
			return @"Tab";
		case 53:
			return @"Esc";
		case 57:
			return @"Caps Lock";
		case 71:
			return @"Num Lock";
		case 107:
			return @"Scroll Lock";
		case 113:
			return @"Pause";
		case 51:
			return @"Backspace";
		case 114:
			return @"Insert";
		
		/* Cursor movement */
		case 126:
			return @"Up";
		case 125:
			return @"Down";
		case 123:
			return @"Left";
		case 124:
			return @"Right";
		case 116:
			return @"Page Up";
		case 121:
			return @"Page Down";
		case 115:
			return @"Home";
		case 119:
			return @"End";
			
		/* Numeric keypad */
		case 82:
			return @"Num 0";
		case 83:
			return @"Num 1";
		case 84:
			return @"Num 2";
		case 85:
			return @"Num 3";
		case 86:
			return @"Num 4";
		case 87:
			return @"Num 5";
		case 88:
			return @"Num 6";
		case 89:
			return @"Num 7";
		case 91:
			return @"Num 8";
		case 92:
			return @"Num 9";
		case 76:
			return @"Num Enter";
		case 65:
			return @"Num .";
		case 69:
			return @"Num +";
		case 78:
			return @"Num -";
		case 67:
			return @"Num *";
		case 75:
			return @"Num /";
			
		/* Function keys */
		case 122:
			return @"F1";
		case 120:
			return @"F2";
		case 99:
			return @"F3";
		case 118:
			return @"F4";
		case 96:
			return @"F5";
		case 97:
			return @"F6";
		case 98:
			return @"F7";
		case 100:
			return @"F8";
		case 101:
			return @"F9";
		case 109:
			return @"F10";
		case 103:
			return @"F11";
		case 111:
			return @"F12";
			
		/* Modifier keys */
		case 56:
			return @"Shift";
		case 59:
			return @"Ctrl";
		case 58:
			return @"Command";
		case 55:
			return @"Alt";
		default:
			return @"What did you just hit??";
	}

	
}


- (id) initWithKeycode:(int) aKeyCode
{
	
	self = [super init];
	if(self)
	{		
		character = [KeyboardButton charForKey:aKeyCode];
		keyCode = aKeyCode;
	}
	
	return self;
}


- (id) initWithEvent:(NSEvent*) event
{
	return [self initWithKeycode:[event keyCode]];
	
}

- (id)initWithCoder:(NSCoder *)coder

{
	
	character = [[coder decodeObjectForKey:@"Char"] retain];
	keyCode = [coder decodeIntForKey:@"Code"];
    return self;
}


- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeInt:keyCode forKey:@"Code"];
	[character encodeWithCoder:coder];
}

- (NSString*) description
{
	return [character autorelease];
}

- (id)copyWithZone: (NSZone*) zone
{
	KeyboardButton *copy = [[[self class] allocWithZone: zone]
										initWithKeycode:keyCode];
						
	return copy;
}


@end






@implementation GameButton

@synthesize page;
@synthesize usage;
@synthesize player;
@synthesize button;
@synthesize axis;


- (id) initWithPage: (int) aPage usage: (int) aUsage value: (int) aValue forButton: (eButton_Type) aButton player:(int) aPlayer
{
	self = [super init];
	
	if(self)
	{
		page = aPage;
		usage = aUsage;
		
		value = aValue;
		
		axis = eAxis_None;
		
		if(page == kHIDPage_GenericDesktop)
		{
			axis = ( aValue > 0 ? eAxis_Positive : eAxis_Negative );
		}
		
		button = aButton;
		player = aPlayer;
	}
	return self;
}

- (id)copyWithZone: (NSZone*) zone
{
	GameButton *copy = [[[self class] allocWithZone: zone]
						initWithPage:page
						usage:usage 
						value: value
						forButton:button 
						player:player];
	return copy;
}

- (id)initWithCoder:(NSCoder *)coder

{
	value = [coder decodeIntForKey:@"Value"];
	axis = [coder decodeIntForKey:@"Axis"];
    page = [coder decodeIntForKey:@"Page"];
    usage = [coder decodeIntForKey:@"Usage"];
    button = [coder decodeIntForKey:@"Button"];
    player = [coder decodeIntForKey:@"Player"];
    return self;
}


- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeInt:value forKey:@"Value"];
	[coder encodeInt:axis forKey:@"Axis"];
    [coder encodeInt:page forKey:@"Page"];	
    [coder encodeInt:usage forKey:@"Usage"];
    [coder encodeInt:button forKey:@"Button"];
    [coder encodeInt:player forKey:@"Player"];
}

- (NSString*) description
{
	
	//return [NSString stringWithFormat:@"Value: %i\nPage: %i\nUsage: %i", value, page, usage];
	
	if ( axis == eAxis_None ) //its a button
	{
		return [NSString stringWithFormat:@"Button %i", usage];
	}
	else if( axis == eAxis_Positive )
	{
		return [NSString stringWithFormat:@"Positive %c axis", (usage == kHIDUsage_GD_X ? 'X' : (usage == kHIDUsage_GD_Y ? 'Y' : 'Z') )];
	}
	else
	{
		return [NSString stringWithFormat:@"Negative %c axis", (usage == kHIDUsage_GD_X ? 'X' : (usage == kHIDUsage_GD_Y ? 'Y' : 'Z') )];
	}
}


@end
