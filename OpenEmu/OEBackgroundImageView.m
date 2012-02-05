//
//  OEBackgroundImageView.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEBackgroundImageView.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEBackgroundImageView
@synthesize image, leftBorder,rightBorder, topBorder,bottomBorder;
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {}
    return self;
}

- (void)dealloc{
    [super dealloc];
}

- (BOOL)isOpaque
{
    return NO;
}
- (void)drawRect:(NSRect)dirtyRect
{
    if(self.image)
    {
        NSRect imgRect = [self bounds];
        [self.image drawInRect:imgRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:[self isFlipped] hints:nil leftBorder:[self leftBorder] rightBorder:[self rightBorder] topBorder:[self topBorder] bottomBorder:[self bottomBorder]];
    }
}

@end
