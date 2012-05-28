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

#import "OEHUDButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"

@interface OEHUDButtonCell ()
- (NSImage *)OE_imageForHudButtonColor:(OEHUDButtonColor)color;
@end

@implementation OEHUDButtonCell
@synthesize buttonColor;

- (id)init
{
    if((self = [super init]))
    {
        [self setBordered:YES];
        [self setFocusRingType:NSFocusRingTypeNone];
        [self setBezeled:NO];
        [self setBackgroundColor:nil];
        [self setButtonType:NSMomentaryPushInButton];
        [self setBezelStyle:NSSmallSquareBezelStyle];
        
        [self setButtonColor:OEHUDButtonColorDefault];
    }
    
    return self;
}

- (BOOL)isOpaque
{
    return NO;
}

+ (void)initialize
{
    if(self != [OEHUDButtonCell class]) return;
    
    NSImage *image = [NSImage imageNamed:@"hud_button"];
    CGSize   size  = [image size];
    
    [image setName:@"hud_button_pressed" forSubimageInRect:NSMakeRect(0.0, 2.0 * size.height / 4.0, size.width, size.height / 4.0)];
    [image setName:@"hud_button_blue"    forSubimageInRect:NSMakeRect(0.0, 1.0 * size.height / 4.0, size.width, size.height / 4.0)];
    [image setName:@"hud_button_normal"  forSubimageInRect:NSMakeRect(0.0, 3.0 * size.height / 4.0, size.width, size.height / 4.0)];
    [image setName:@"hud_button_red"     forSubimageInRect:NSMakeRect(0.0, 0.0                    , size.width, size.height / 4.0)];
}

#pragma mark -

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    NSRect cellFrame = frame;
    cellFrame.size.height = 23;
    
    NSImage *img = [self isHighlighted] ? [NSImage imageNamed:@"hud_button_pressed"] : [self OE_imageForHudButtonColor:self.buttonColor];
    
    [img drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:5 rightBorder:5 topBorder:0 bottomBorder:0];
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView
{
    image = [self isHighlighted] ? [self alternateImage] : [self image];
    
    frame.origin.y = round((frame.size.height - image.size.height) / 2.0) + 3;
    
    [super drawImage:image withFrame:frame inView:controlView];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
    NSRect titleRect = frame;
    
    titleRect.origin.y -= 1;
    
    [title drawInRect:titleRect];
    return titleRect;
}

#pragma mark -

- (NSAttributedString *)attributedTitle
{
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:3.0 size:11.0];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowBlurRadius:1.0];
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setAlignment:NSCenterTextAlignment];
    
    if([self isHighlighted])
    {
        [attributes setObject:[NSColor colorWithDeviceWhite:0.03 alpha:1.0] forKey:NSForegroundColorAttributeName];
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.4]];
        [shadow setShadowOffset:NSMakeSize(0, -1)];
    }
    else
    {
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
        [attributes setObject:[NSColor colorWithDeviceWhite:0.91 alpha:1.0] forKey:NSForegroundColorAttributeName];
        [shadow setShadowOffset:NSMakeSize(0, 1)];
    }
    
    [attributes setObject:ps forKey:NSParagraphStyleAttributeName];
    [attributes setObject:font forKey:NSFontAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
    
    return [[NSAttributedString alloc] initWithString:[self title] attributes:attributes];
}

- (NSImage *)OE_imageForHudButtonColor:(OEHUDButtonColor)color
{
    switch(color)
    {
        case OEHUDButtonColorDefault:
            return [NSImage imageNamed:@"hud_button_normal"];
        case OEHUDButtonColorBlue:
            return [NSImage imageNamed:@"hud_button_blue"];
        case OEHUDButtonColorRed:
            return [NSImage imageNamed:@"hud_button_red"];
        default:
            break;
    }
    return nil;
}

@end

