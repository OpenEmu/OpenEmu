//
//  OECheckBox.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 29.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECheckBox.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OECheckBox

- (id)init
{
    self = [super init];
    if (self) 
    {    
        [self setupCell];
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) 
    {    
        [self setupCell];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) 
    {
        [self setupCell];
    }
    return self;
}
#pragma mark -
- (void)setupCell
{
    
    if(![[self cell] isKindOfClass:[OECheckBoxCell class]])
    {
        OECheckBoxCell *cell = [[OECheckBoxCell alloc] init];
        [self setCell:cell];
        [cell release];
        
        
        [self setBordered:NO];
        [self setButtonType:NSSwitchButton];
        [self setTransparent:NO];
    }
}
#pragma mark -
- (void)dealloc
{
    [super dealloc];
}

- (void)awakeFromNib
{
}


- (NSPoint)badgePosition
{
    float y = [self frame].origin.y -1;
    float x = [self frame].origin.x + [[[self cell] attributedTitle] size].width + 24 + 3;
    return (NSPoint){x, y};
}
@end

@implementation OECheckBoxCell
- (id)init 
{
    self = [super init];
    if (self) 
    {
        [self setBezeled:NO];
        [self setBordered:NO];
        [self setImagePosition:NSImageLeft];
    }
    return self;
}
- (void)awakeFromNib{
}

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OECheckBoxCell class])
        return;

    NSImage *image = [NSImage imageNamed:@"dark_checkbox"];
    
    [image setName:@"dark_checkbox_off" forSubimageInRect:NSMakeRect(0, 16, 16, 16)];
    [image setName:@"dark_checkbox_off_pressed" forSubimageInRect:NSMakeRect(16, 16, 16, 16)];
    [image setName:@"dark_checkbox_on" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];
    [image setName:@"dark_checkbox_on_pressed" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
}

- (NSAttributedString *)attributedTitle
{
    NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0.0 size:11.0];
    NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    [attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
    return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
    CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
    CGContextSaveGState(context);
    CGContextSetShouldSmoothFonts(context, NO);
    
    frame.origin.x += 3;
    frame.size.width -= 3;
    
    frame.origin.y += 2;
    
    NSRect rect = [super drawTitle:title withFrame:frame inView:controlView];
    
    
    CGContextRestoreGState(context);
    
    return rect;
}

- (void)drawImage:(NSImage*)image withFrame:(NSRect)frame inView:(NSView*)controlView
{
    NSImage *checkboximage;
    if ([self isHighlighted] && [self state]==NSOnState)
        checkboximage = [NSImage imageNamed:@"dark_checkbox_on_pressed"];
    else if (![self isHighlighted] && [self state]==NSOnState)
        checkboximage = [NSImage imageNamed:@"dark_checkbox_on"];
    else if (![self isHighlighted] && [self state]==NSOffState)
        checkboximage = [NSImage imageNamed:@"dark_checkbox_off"];
    else 
        checkboximage = [NSImage imageNamed:@"dark_checkbox_off_pressed"];
    
    CGFloat y = NSMaxY(frame) - (frame.size.height - checkboximage.size.height) / 2.0 - 15;
    [checkboximage drawInRect:NSMakeRect(frame.origin.x, roundf(y), checkboximage.size.width, checkboximage.size.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

@end