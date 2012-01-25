//
//  OECoreTableButtonCell.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OECoreTableButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OECoreTableButtonCell
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OECoreTableButtonCell class])
        return;

    NSImage* image = [NSImage imageNamed:@"slim_dark_pill_button"];
    [image setName:@"slim_dark_pill_button_normal" forSubimageInRect:(NSRect){{0,image.size.height/2},{image.size.width, image.size.height/2}}];
    [image setName:@"slim_dark_pill_button_pressed" forSubimageInRect:(NSRect){{0,0},{image.size.width, image.size.height/2}}];
}

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
    OECoreTableButtonCell* copy = [super copyWithZone:zone];
    copy.widthInset = self.widthInset;
    return copy;
}
#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{   
    BOOL highlighted = [self isHighlighted];
    NSImage* image = [NSImage imageNamed:highlighted?@"slim_dark_pill_button_pressed":@"slim_dark_pill_button_normal"];
    
    cellFrame = NSInsetRect(cellFrame, self.widthInset, (cellFrame.size.height-15)/2);
    
    [image drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:9 rightBorder:9 topBorder:0 bottomBorder:0];
    
    cellFrame.origin.y += 1;
    
    NSString* label = [self title];
    
    NSMutableParagraphStyle* paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setAlignment:NSCenterTextAlignment];
    NSDictionary* textAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0.0 size:9.0], NSFontAttributeName,
                                    [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                    paraStyle, NSParagraphStyleAttributeName,
                                    nil];
    [paraStyle release];
    [label drawInRect:cellFrame withAttributes:textAttributes];
}

@synthesize widthInset;
@end
