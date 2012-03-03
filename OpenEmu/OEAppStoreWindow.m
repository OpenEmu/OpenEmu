//
//  OEAppStoreWindow.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 03.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEAppStoreWindow.h"

@implementation OEAppStoreWindow
- (NSRect)contentRectForFrameRect:(NSRect)frameRect{
    NSRect rect = [super contentRectForFrameRect:frameRect];
    
    float adjustment = self.titleBarHeight - 22;
    rect.size.height -= adjustment;
    
    return rect;
}

- (NSRect)frameRectForContentRect:(NSRect)contentRect
{
    NSRect rect = [super frameRectForContentRect:contentRect];
    
    float adjustment = self.titleBarHeight - 22;
    rect.size.height += adjustment;
    
    return rect;
}

@end
