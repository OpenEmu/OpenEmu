//
//  BackgroundColorView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEBackgroundColorView.h"


@implementation OEBackgroundColorView
@synthesize backgroundColor;
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor=[NSColor blackColor];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    if(self.backgroundColor){
        [self.backgroundColor setFill];
        NSRectFill([self bounds]);
    }
}

@end
