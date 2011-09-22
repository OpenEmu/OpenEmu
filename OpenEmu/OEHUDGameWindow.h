//
//  OEHUDGameWindow.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 05.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDWindow.h"
@class OENewGameDocument;
@interface OEHUDGameWindow : NSPanel
{
	OENewGameDocument* gameDocument;
}
- (id)initWithContentRect:(NSRect)contentRect andGameDocument:(OENewGameDocument*)gameDoc;
@property (retain, nonatomic) OENewGameDocument* gameDocument;
@end
