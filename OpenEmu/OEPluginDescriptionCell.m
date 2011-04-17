/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OEPluginDescriptionCell.h"
#import "OECorePlugin.h"

@implementation OEPluginDescriptionCell

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    [self setTextColor:[NSColor blackColor]];
    
    OECorePlugin* data = [self objectValue];
    
    // give the delegate a chance to set a different data object
    /*if ([[self dataDelegate] respondsToSelector: @selector(dataElementForCell:)]) {
        data = [[self dataDelegate] dataElementForCell:self];
     }*/
    
    //TODO: Selection with gradient and selection color in white with shadow
    // check out http://www.cocoadev.com/index.pl?NSTableView
    
    BOOL elementDisabled = NO;
    /*if ([[self dataDelegate] respondsToSelector: @selector(disabledForCell:data:)]) {
        elementDisabled = [[self dataDelegate] disabledForCell: self data: data];
    }*/
    
    NSColor *primaryColor = ([self isHighlighted]
                             ? [NSColor alternateSelectedControlTextColor]
                             : (elementDisabled
                                ? [NSColor disabledControlTextColor]
                                : [NSColor textColor]));
    NSString *primaryText = [data displayName];
    
    NSDictionary *primaryTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                           primaryColor,                 NSForegroundColorAttributeName,
                                           [NSFont systemFontOfSize:13], NSFontAttributeName,
                                           nil];
    
    [primaryText drawAtPoint:NSMakePoint(cellFrame.origin.x+cellFrame.size.height+10, cellFrame.origin.y)
              withAttributes:primaryTextAttributes];
    
    NSColor *secondaryColor = ([self isHighlighted]
                               ? [NSColor alternateSelectedControlTextColor]
                               : [NSColor disabledControlTextColor]);
    
    NSString *secondaryText = [data details];
    NSDictionary *secondaryTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                             secondaryColor,               NSForegroundColorAttributeName,
                                             [NSFont systemFontOfSize:10], NSFontAttributeName,
                                             nil];
    
    [secondaryText drawAtPoint:NSMakePoint(cellFrame.origin.x + cellFrame.size.height + 10,
                                           cellFrame.origin.y + cellFrame.size.height / 2)
                withAttributes:secondaryTextAttributes];
    
    
    [[NSGraphicsContext currentContext] saveGraphicsState];
    
    CGFloat yOffset = cellFrame.origin.y;
    if([controlView isFlipped])
    {
        NSAffineTransform *xform = [NSAffineTransform transform];
        [xform translateXBy:0.0 yBy: cellFrame.size.height];
        [xform scaleXBy:1.0 yBy:-1.0];
        [xform concat];
        yOffset = 0 - cellFrame.origin.y;
    }
    NSImage *icon = [data icon];
    
    NSImageInterpolation interpolation = [[NSGraphicsContext currentContext] imageInterpolation];
    [[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationHigh];
    
    [icon drawInRect:NSMakeRect(cellFrame.origin.x + 5,yOffset + 3,cellFrame.size.height - 6, cellFrame.size.height - 6)
            fromRect:NSMakeRect(0, 0, [icon size].width, [icon size].height)
           operation:NSCompositeSourceOver
            fraction:1.0];
    
    [[NSGraphicsContext currentContext] setImageInterpolation:interpolation];
    
    [[NSGraphicsContext currentContext] restoreGraphicsState];
}

@end
