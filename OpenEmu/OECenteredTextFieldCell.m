//
//  OECenteredTextFieldCell.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 09.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OECenteredTextFieldCell.h"


@implementation OECenteredTextFieldCell

- (id)init
{
    self = [super init];
    if (self) {
        self.widthInset = 3;
    }
    
    return self;
}
- (id)initImageCell:(NSImage *)image
{
    self = [super initImageCell:image];
    if (self) {
        self.widthInset = 3;
    }
    return self;
}

- (id)initTextCell:(NSString *)aString{
    self = [super initTextCell:aString];
    if (self) {
        self.widthInset = 3;
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        self.widthInset = 3;
    }
    return self;
}


- (void)dealloc{
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    OECenteredTextFieldCell* copy = [super copyWithZone:zone];
    copy.widthInset = self.widthInset;
    return copy;
}
#pragma mark -
#pragma mark Drawing
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
    NSSize contentSize = [self cellSize];
    
    cellFrame.origin.y += (cellFrame.size.height - contentSize.height) / 2.0;
    cellFrame.size.height = contentSize.height;
    
    cellFrame = NSInsetRect(cellFrame, self.widthInset, 0);
    
    NSAttributedString *drawString = [self attributedStringValue];
    
    [drawString drawInRect:cellFrame];
}

#pragma mark -
@synthesize widthInset;
@end
