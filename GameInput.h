//
//  GameInput.h
//  Gambatte
//
//  Created by ben on 8/26/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#warning This file should be deleted.
#if 0
#import <Cocoa/Cocoa.h>
@class GamePreferencesController, GameDocumentController;

@interface GameInput : NSResponder {
	IBOutlet GameDocumentController* docController;
	IBOutlet GamePreferencesController* prefController; 
}

@property(readonly) GamePreferencesController* prefController;

- (BOOL) handlesEvent:(NSEvent *)theEvent;
- (void) keyDown:(NSEvent *)theEvent;
- (void) keyUp:(NSEvent *)theEvent;

- (void) releaseButton: (int) button forPlayer: (int) player;
- (void) pressButton: (int) button forPlayer: (int) player;

@end
#endif