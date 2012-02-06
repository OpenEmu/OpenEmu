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

#import "OEPopupButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEPopupButton.h"
@implementation OEPopupButtonCell

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEPopupButtonCell class])
        return;

	NSImage *image = [NSImage imageNamed:@"dark_popup_button"];
	
	[image setName:@"dark_popup_button_normal" forSubimageInRect:NSMakeRect(0, image.size.height/2, image.size.width, image.size.height/2)];
	[image setName:@"dark_popup_button_pressed" forSubimageInRect:NSMakeRect(0, 0, image.size.width, image.size.height/2)];
}

- (NSAttributedString *)attributedTitle
{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0.0 size:11.0];
	NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
    if([self isHighlighted])
    {
        [attributes setObject:[NSColor colorWithDeviceRed:0.741 green:0.737 blue:0.737 alpha:1.0] forKey:NSForegroundColorAttributeName];
    } 
    else 
    {
        [attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
    }
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}

#pragma mark -
- (void)drawTitleWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	cellFrame.size.height -= 3;
	cellFrame.origin.y += 1;
	
	cellFrame.origin.x -= 3;
	cellFrame.size.width += 3;
	
	[super drawTitleWithFrame:cellFrame inView:controlView];
	
}

- (void)drawBorderAndBackgroundWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	cellFrame.size.height = 23;
	
	NSImage *img = [self isHighlighted] ? [NSImage imageNamed:@"dark_popup_button_pressed"] : [NSImage imageNamed:@"dark_popup_button_normal"];
	[img drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:9 rightBorder:21 topBorder:0 bottomBorder:0];
}
@end
