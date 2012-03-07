//
//  OEAppStoreWindow.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 03.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "INAppStoreWindow.h"

@interface OEAppStoreWindow : INAppStoreWindow
- (NSRect)contentRectForFrameRect:(NSRect)frameRect;
- (NSRect)frameRectForContentRect:(NSRect)contentRect;
@end
