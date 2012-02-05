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

#import "OECoreTableButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OECoreTableButtonCell
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OECoreTableButtonCell class])
        return;

    NSImage *image = [NSImage imageNamed:@"slim_dark_pill_button"];
    [image setName:@"slim_dark_pill_button_normal" forSubimageInRect:(NSRect){{0,image.size.height/2},{image.size.width, image.size.height/2}}];
    [image setName:@"slim_dark_pill_button_pressed" forSubimageInRect:(NSRect){{0,0},{image.size.width, image.size.height/2}}];
}

- (id)init 
{
    self = [super init];
    if (self) 
    {
        self.widthInset = 9.0;
    }
    return self;
}
- (id)initImageCell:(NSImage *)image
{
    self = [super initImageCell:image];
    if (self)
    {
        self.widthInset = 9.0;
    }
    return self;
}

- (id)initTextCell:(NSString *)aString
{
    self = [super initTextCell:aString];
    if (self)
    {
        self.widthInset = 9.0;
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        self.widthInset = 9.0;
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    OECoreTableButtonCell *copy = [super copyWithZone:zone];
    copy.widthInset = self.widthInset;
    return copy;
}
#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{   
    BOOL highlighted = [self isHighlighted];
    NSImage *image = [NSImage imageNamed:highlighted?@"slim_dark_pill_button_pressed":@"slim_dark_pill_button_normal"];
    
    cellFrame = NSInsetRect(cellFrame, self.widthInset, (cellFrame.size.height-15)/2);
    
    [image drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:9 rightBorder:9 topBorder:0 bottomBorder:0];
    
    cellFrame.origin.y += 1;
    
    NSString *label = [self title];
    
    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setAlignment:NSCenterTextAlignment];
    NSDictionary *textAttributes;
    
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    if([self isHighlighted])
    {
        textAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0.0 size:9.0], NSFontAttributeName,
                                    [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName,
                                    paraStyle, NSParagraphStyleAttributeName,
                                    shadow, NSShadowAttributeName,
                                    nil];
    }
    else
    {
        textAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                          [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0.0 size:9.0], NSFontAttributeName,
                          [NSColor colorWithDeviceWhite:0.89 alpha:1.0], NSForegroundColorAttributeName,
                          paraStyle, NSParagraphStyleAttributeName,
                          shadow, NSShadowAttributeName,
                          nil];
   
    }
    [shadow release];
    
    [paraStyle release];
    [label drawInRect:cellFrame withAttributes:textAttributes];
}

@synthesize widthInset;
@end
