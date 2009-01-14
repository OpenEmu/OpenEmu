//
//  GameApp.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/13/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GameInput;

@interface GameApp : NSApplication {
	IBOutlet GameInput* inputController;
	IBOutlet NSMenu* wtfMenu;
}

@end
