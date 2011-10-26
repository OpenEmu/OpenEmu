//
//  OEHUDGameWindow.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 05.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDWindow.h"
@class OEGameViewController;
@interface OEHUDGameWindow : NSPanel
{
	OEGameViewController* _gameViewController;
}
- (id)initWithContentRect:(NSRect)contentRect andGameViewController:(OEGameViewController*)gameViewCtrl;
@property (retain, nonatomic) OEGameViewController* gameViewController;
@end
