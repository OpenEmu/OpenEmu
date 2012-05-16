//
//  OEBlackView.m
//  OpenEmu
//
//  Created by Anton Marini on 5/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEBlackView.h"

@implementation OEBlackView

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
    [[NSColor blackColor] set];
    NSRectFill(dirtyRect);
}

@end
