//
//  BackgroundColorView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEBackgroundColorView.h"
#import "NSColor+IKSAdditions.h"
@implementation OEBackgroundColorView
@synthesize backgroundColor;
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		self.wantsLayer = YES;
        self.backgroundColor=[NSColor blackColor];
    }
    return self;
}

- (void)setBackgroundColor:(NSColor *)_backgroundColor{
	[_backgroundColor retain];
	[backgroundColor release];
	
	backgroundColor = _backgroundColor;
	
	self.layer.backgroundColor = [backgroundColor CGColor];
}

@end
