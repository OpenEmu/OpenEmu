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
#import "OETheme.h"
@interface OEToolbarView ()
- (void)OE_commonToolbarViewInit;
@property(strong) NSMutableArray *items;
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
    [item setAccessibilityParent:self];
    [[self items] addObject:item];
    [self setNeedsDisplay:YES];
}

#pragma mark -
- (OEToolbarItem*)selectedItem{
    return selectedItem;
}

- (NSUInteger)selectedItemIndex
{
    return [[self items] indexOfObject:selectedItem];
}

- (void)markItemAsSelected:(OEToolbarItem*)tbItem{
    NSUInteger index = [self indexOfItem:tbItem];
    if(index == NSNotFound){
        selectedItem = nil;
    } else {
        selectedItem = [self itemAtIndex:index];
    }
}

- (void)markItemIndexAsSelected:(NSUInteger)itemIndex;
{
    selectedItem = (itemIndex < [[self items] count] ?
                    [[self items] objectAtIndex:itemIndex] :
                    nil);
}
#pragma mark -
- (NSInteger)numberOfItems
{
    return [[self items] count];
}
- (OEToolbarItem*)itemAtIndex:(NSInteger)index
{
    return [[self items] objectAtIndex:index];
}
- (NSInteger)indexOfItem:(OEToolbarItem*)item
{
    return [[self items] indexOfObject:item];
}
#pragma mark -
- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

    float itemSpacing = 5;
    float titlePadding = 8;
    float imageSideLength = 36;
    float imageTitleSpacing = 4;

    NSFont *font = [NSFont systemFontOfSize:11];
    NSColor *textColor = [NSColor blackColor];
    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.45]];
    [shadow setShadowBlurRadius:1];
    [shadow setShadowOffset:NSMakeSize(0, -1)];

    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    [ps setAlignment:NSCenterTextAlignment];

    NSDictionary *textAttributes = @{
                                     NSFontAttributeName: font,
                                     NSShadowAttributeName: shadow,
                                     NSForegroundColorAttributeName: textColor,
                                     NSParagraphStyleAttributeName:ps
                                     };

    float x = itemSpacing;
    for(OEToolbarItem *anItem in [self items])
    {
        x += titlePadding;

        NSAttributedString *title = [[NSAttributedString alloc] initWithString:anItem.title attributes:textAttributes];
        NSRect titleRect = NSMakeRect(x, 6, title.size.width, title.size.height);
        NSRect imageRect = NSMakeRect(x+(title.size.width-imageSideLength)/2, titleRect.origin.y+titleRect.size.height+imageTitleSpacing-3, imageSideLength, imageSideLength);
        imageRect.origin.x = roundf(imageRect.origin.x);

        if(NSEqualRects(anItem.itemRect, NSZeroRect))
        {
            anItem.itemRect = NSMakeRect(x-titlePadding, 1, title.size.width+2*titlePadding, 54);
        }

        if(anItem == selectedItem)
        {
            NSImage *selectionHighlight = [[OETheme sharedTheme] imageForKey:@"tab_selector" forState:OEThemeStateDefault];
            [selectionHighlight drawInRect:anItem.itemRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
        }

        NSImage *icon = anItem.icon;

        if([NSEvent pressedMouseButtons] & NSLeftMouseDown)
        {
            NSPoint mouseLocationOnScreen = [NSEvent mouseLocation];
            NSPoint mouseLocationOnWindow = [[self window] convertRectFromScreen:(NSRect){mouseLocationOnScreen, {0,0}}].origin;
            NSPoint mouseLocationOnView   = [self convertPoint:mouseLocationOnWindow fromView:nil];

            if(anItem == highlightedItem && NSPointInRect(mouseLocationOnView, anItem.itemRect)){
                icon = [icon imageForHighlight];
            }
        }
        [icon drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        [anItem.title drawInRect:titleRect withAttributes:textAttributes];

        x += title.size.width;
        x += titlePadding;
        x += itemSpacing;
    }

    // draw bottom separator
    [[self bottomSeparatorColor] setFill];
    NSRect viewRect = [self bounds];
    NSRectFill(NSMakeRect(NSMinX(viewRect), NSMinY(viewRect), NSWidth(viewRect), 1));
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    BOOL highlightSuccess = [self _highlightItemAtPoint:loc];

    if(!highlightSuccess)
    {
        [super mouseDown:theEvent];
    }
}

- (BOOL)_highlightItemAtPoint:(NSPoint)loc
{
    highlightedItem = nil;
    for(OEToolbarItem *anItem in [self items])
        if(NSPointInRect(loc, [anItem itemRect]))
        {
            highlightedItem = anItem;
            break;
        }

    [self setNeedsDisplay:YES];

    return highlightedItem != nil;
}

- (void)mouseUp:(NSEvent *)theEvent
{
    NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    [self _selectItemAtPoint:loc];
}

- (BOOL)_selectItemAtPoint:(NSPoint)loc
{
    if(highlightedItem && NSPointInRect(loc, [highlightedItem itemRect]))
    {
        selectedItem = highlightedItem;
        if([selectedItem action] != NULL)
        {
            [NSApp sendAction:[selectedItem action] to:[selectedItem target] from:selectedItem];
        }
    }
    [self setNeedsDisplay:YES];

    return YES;
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
    [self setItems:[NSMutableArray array]];
}

#pragma mark - Accessiblity
- (NSRect)accessibilityFrame
{
    NSRect windowFrame = [[self window] frame];
    NSRect viewFrame   = [self frame];

    return (NSRect){.origin = { NSMinX(windowFrame), NSMaxY(windowFrame)-NSHeight(viewFrame) },
        .size=viewFrame.size };
}

- (id)accessibilityParent
{
    return [self window];
}

- (NSArray*)accessibilityChildren
{
    return self.items;
}

-  (NSString*)accessibilityRoleDescription
{
    return @"toolbar";
}

- (NSString*)accessibilityRole
{
    return @"toolbar";
}

@end

@implementation OEToolbarItem
@synthesize title, icon, target, action, itemRect;
@synthesize accessibilityParent;
#pragma mark - Accessiblity
- (NSRect)accessibilityFrame
{
    OEToolbarView *view = [self accessibilityParent];
    NSRect    viewFrame = [view accessibilityFrame];

    return (NSRect){ .origin = NSMakePoint(NSMinX(viewFrame)+NSMinX(itemRect), NSMinY(viewFrame)+NSMinY(itemRect)),
        .size   = itemRect.size };
}

- (NSArray*)accessibilityChildren
{
    return nil;
}

-  (NSString*)accessibilityRoleDescription
{
    return @"toolbar button";
}

- (NSString*)accessibilityLabel
{
    return [self title];
}

- (BOOL)accessibilityPerformPress
{
    OEToolbarView *view = [self accessibilityParent];
    NSPoint center = (NSPoint){.x=NSMidX([self itemRect]), .y=NSMidY([self itemRect])};
    [view _highlightItemAtPoint:center];
    return [view _selectItemAtPoint:center];
}

@end
