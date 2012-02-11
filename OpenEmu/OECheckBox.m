/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OECheckBox.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OECheckBox

+ (Class)cellClass
{
    return [OECheckBoxCell class];
}

- (id)init
{
    if((self = [super init]))
    {
        [self setupCell];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if((self = [super initWithCoder:aDecoder]))
    {
        [self setupCell];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frameRect
{
    if((self = [super initWithFrame:frameRect]))
    {
        [self setupCell];
    }
    
    return self;
}
#pragma mark -

- (void)setupCell
{
    [self setBordered:NO];
    [self setButtonType:NSSwitchButton];
    [self setTransparent:NO];
}

#pragma mark -

- (void)awakeFromNib
{
}

- (NSPoint)badgePosition
{
    float y = [self frame].origin.y - 1;
    float x = [self frame].origin.x + [[[self cell] attributedTitle] size].width + 24 + 3;
    return NSMakePoint(x, y);
}

@end

@implementation OECheckBoxCell

- (id)init
{
    if((self = [super init]))
    {
        [self setBezeled:NO];
        [self setBordered:NO];
        [self setImagePosition:NSImageLeft];
    }
    
    return self;
}

- (void)awakeFromNib
{
}

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OECheckBoxCell class]) return;

    NSImage *image = [NSImage imageNamed:@"dark_checkbox"];
    
    [image setName:@"dark_checkbox_off"         forSubimageInRect:NSMakeRect( 0.0, 16.0, 16.0, 16.0)];
    [image setName:@"dark_checkbox_off_pressed" forSubimageInRect:NSMakeRect(16.0, 16.0, 16.0, 16.0)];
    [image setName:@"dark_checkbox_on"          forSubimageInRect:NSMakeRect( 0.0,  0.0, 16.0, 16.0)];
    [image setName:@"dark_checkbox_on_pressed"  forSubimageInRect:NSMakeRect(16.0,  0.0, 16.0, 16.0)];
}

- (NSAttributedString *)attributedTitle
{
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0.0 size:11.0];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    [attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
    return [[NSAttributedString alloc] initWithString:[self title] attributes:attributes];
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

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView
{
    BOOL              highlighted = [self isHighlighted];
    NSCellStateValue  state       = [self state];
    
    NSString         *imageName   = (state == NSOnState
                                     ? highlighted ? @"dark_checkbox_on_pressed"  : @"dark_checkbox_on"
                                     : highlighted ? @"dark_checkbox_off_pressed" : @"dark_checkbox_off");
    
    NSImage *checkboximage = [NSImage imageNamed:imageName];
    
    CGFloat y = NSMaxY(frame) - (frame.size.height - [checkboximage size].height) / 2.0 - 15;
    [checkboximage drawInRect:NSMakeRect(frame.origin.x, roundf(y), checkboximage.size.width, checkboximage.size.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

@end