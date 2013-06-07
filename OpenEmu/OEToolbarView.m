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

#import "OEToolbarView.h"
#import "NSImage+OEDrawingAdditions.h"
#import "NSImage+OEHighlight.h"

@interface OEToolbarView ()
- (void)OE_commonToolbarViewInit;
@end 

@implementation OEToolbarView
@synthesize items;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self OE_commonToolbarViewInit];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
    {
        [self OE_commonToolbarViewInit];
    }
    
    return self;
}

#pragma mark -

- (void)addItem:(OEToolbarItem *)item
{
    [self.items addObject:item];
    
    [self setNeedsDisplay:YES];
}

#pragma mark -
- (OEToolbarItem*)selectedItem{
    return selectedItem;
}
- (NSUInteger)selectedItemIndex
{
    return [self.items indexOfObject:selectedItem];
}
- (void)markItemAsSelected:(OEToolbarItem*)tbItem{
    NSUInteger index = [self.items indexOfObject:tbItem];
    if(index == NSNotFound){
        selectedItem = nil;
    } else {
        selectedItem = [self.items objectAtIndex:index];
    }
}

- (void)markItemIndexAsSelected:(NSUInteger)itemIndex;
{
    selectedItem = (itemIndex < [[self items] count] ?
                    [[self items] objectAtIndex:itemIndex] :
                    nil);
}

#pragma mark -
- (BOOL)isOpaque{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect{
    [super drawRect:dirtyRect];
        
    float itemSpacing = 5;
    float titlePadding = 8;
    float imageSideLength = 36;
    float imageTitleSpacing = 4;
    
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:4.0 size:11.0];
    NSColor *textColor = [NSColor blackColor];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.45]];
    [shadow setShadowBlurRadius:1];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setAlignment:NSCenterTextAlignment];
    
    NSMutableDictionary *textAttributes = [NSMutableDictionary dictionary];
    [textAttributes setObject:font forKey:NSFontAttributeName];
    [textAttributes setObject:shadow forKey:NSShadowAttributeName];
    [textAttributes setObject:textColor forKey:NSForegroundColorAttributeName];
    [textAttributes setObject:ps forKey:NSParagraphStyleAttributeName];
    
    float x = itemSpacing;
    for(OEToolbarItem *anItem in self.items){
        x += titlePadding;
        
        NSAttributedString *title = [[NSAttributedString alloc] initWithString:anItem.title attributes:textAttributes];
        NSRect titleRect = NSMakeRect(x, 6, title.size.width, title.size.height);
        NSRect imageRect = NSMakeRect(x+(title.size.width-imageSideLength)/2, titleRect.origin.y+titleRect.size.height+imageTitleSpacing-3, imageSideLength, imageSideLength);
        imageRect.origin.x = roundf(imageRect.origin.x);

        if(NSEqualRects(anItem.itemRect, NSZeroRect)){
            anItem.itemRect = NSMakeRect(x-titlePadding, 1, title.size.width+2*titlePadding, 54);
        }
        
        if(anItem == selectedItem){
            NSImage *selectionHighlight = [NSImage imageNamed:@"tab_selector"];
            [selectionHighlight drawInRect:anItem.itemRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:12 rightBorder:12 topBorder:0 bottomBorder:0];
        }
        
        NSImage *icon = anItem.icon;

        NSPoint mouseLocationOnScreen = [NSEvent mouseLocation];
        NSPoint mouseLocationOnWindow = [[self window] convertRectFromScreen:(NSRect){mouseLocationOnScreen, {0,0}}].origin;
        NSPoint mouseLocationOnView   = [self convertPoint:mouseLocationOnWindow fromView:nil];

        if(anItem == highlightedItem && NSPointInRect(mouseLocationOnView, anItem.itemRect)){
            icon = [icon imageForHighlight];
        }
        
        [icon drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        [anItem.title drawInRect:titleRect withAttributes:textAttributes];
        
        x += title.size.width;
        x += titlePadding;
        x += itemSpacing;
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    highlightedItem = nil;
    for(OEToolbarItem *anItem in [self items])
        if(NSPointInRect(loc, [anItem itemRect]))
        {
            highlightedItem = anItem;
            break;
        }
    
    [self setNeedsDisplay:YES];
    
    if(highlightedItem == nil) [super mouseDown:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    if(highlightedItem && NSPointInRect(loc, [highlightedItem itemRect]))
    {
        selectedItem = highlightedItem;        
        if([selectedItem action] != NULL)
        {
            [NSApp sendAction:[selectedItem action] to:[selectedItem target] from:selectedItem];
        }
    }
    highlightedItem = nil;

    
    [self setNeedsDisplay:YES];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    [self setNeedsDisplay:YES];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    [self setNeedsDisplay:YES];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    [self setNeedsDisplay:YES];
}

- (void)OE_commonToolbarViewInit
{
    self.items = [[NSMutableArray alloc] init];
}

@end

@implementation OEToolbarItem
@synthesize title, icon, target, action, itemRect;
@end
