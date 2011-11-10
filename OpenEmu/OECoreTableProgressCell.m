//
//  OECoreTableProgressCell.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OECoreTableProgressCell.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OECoreTableProgressCell

- (id)init {
    self = [super init];
    if (self) {
        self.widthInset = 9.0;
    }
    return self;
}
- (id)initImageCell:(NSImage *)image
{
    self = [super initImageCell:image];
    if (self){
        self.widthInset = 9.0;
    }
    return self;
}

- (id)initTextCell:(NSString *)aString
{
    self = [super initTextCell:aString];
    if (self){
        self.widthInset = 9.0;
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self){
        self.widthInset = 9.0;
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    OECoreTableProgressCell* copy = [super copyWithZone:zone];
    copy.widthInset = self.widthInset;
    return copy;
}
@synthesize widthInset;
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    cellFrame = NSInsetRect(cellFrame, self.widthInset, (cellFrame.size.height-15)/2);
    
    NSRect trackRect = cellFrame;
    NSImage* image = [NSImage imageNamed:@"install_progress_bar_track"];
    [image drawInRect:trackRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:10 rightBorder:10 topBorder:0 bottomBorder:0];
    
    NSRect progressRect = cellFrame;
    float value = [[self objectValue] floatValue];
    if(value==0.0) return;
    if(value>=1.0) value = 1.0;
    progressRect.size.width = value*progressRect.size.width;
    progressRect.size.width = progressRect.size.width < 12 ? 12 : roundf(progressRect.size.width);
    
    image = [NSImage imageNamed:@"install_progress_bar"];
    [image drawInRect:progressRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:6 rightBorder:6 topBorder:0 bottomBorder:0];    
}
@end
