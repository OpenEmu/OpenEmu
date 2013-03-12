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

#import "OESidebarOutlineView.h"

#import "OESidebarFieldEditor.h"
#import "OESidebarController.h"

#import <objc/runtime.h>
#import "OESidebarOutlineButtonCell.h"
#import "OESideBarGroupItem.h"

NSString *const OESidebarConsolesNotCollapsibleKey = @"sidebarConsolesNotCollapsible";

@interface NSOutlineView (ApplePrivateOverrides)
- (id)_highlightColorForCell:(NSCell *)cell;
- (NSRect)_dropHighlightBackgroundRectForRow:(NSInteger)arg1;
- (void)_setNeedsDisplayForDropCandidateRow:(NSInteger)arg1 operation:(NSUInteger)arg2 mask:(NSUInteger)arg3;
- (void)_drawDropHighlightOnRow:(NSInteger)arg1;
- (id)_dropHighlightColor;
- (void)_flashOutlineCell;
@end

@implementation OESidebarOutlineView

- (id)initWithCoder:(NSCoder *)aDecoder
{    
    self = [super initWithCoder:aDecoder];
    if (self) 
    {
        [self setupOutlineCell];
        [self OE_setupDefaultColors];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) 
    {
        [self setupOutlineCell];
        [self OE_setupDefaultColors];
    }
    
    return self;
}

- (void)OE_setupDefaultColors
{
    [self setDropBorderColor:[NSColor colorWithDeviceRed:0.03 green:0.41 blue:0.85 alpha:1.0]];
    [self setDropBackgroundColor:[NSColor colorWithDeviceRed:0.03 green:0.24 blue:0.34 alpha:1.0]];
    [self setDropBorderWidth:2.0];
    [self setDropCornerRadius:8.0];
}

#pragma mark - Calculating rects
- (NSRect)rectOfRow:(NSInteger)row
{
    // We substract 1 here because the view is 1px wider than it should be so it can show a 1px black line on the right side that easily disappears when the sidebar is collapsed
    NSRect rect = [super rectOfRow:row];
    rect.size.width -= 1.0;
    return rect;
}

- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row
{
    if(row==0 && [[NSUserDefaults standardUserDefaults] boolForKey:OESidebarConsolesNotCollapsibleKey])
        return NSZeroRect;
    
    NSRect rect = [super frameOfOutlineCellAtRow:row];
    rect.origin.y += 3;
    return rect;
}

- (NSRect)rectOfGroup:(id)item
{
    if(item == nil)
    {
        NSRect bounds = [self bounds];
        bounds.size.width -= 1;
        bounds.size.height -= 2.0;
        return bounds;
    }
    
    // TODO: this will break when we add collection folders that can have children on their own
    NSUInteger children = [[self dataSource] outlineView:self numberOfChildrenOfItem:item];
    NSRect firstItem = [self rectOfRow:[self rowForItem:item]];
    NSRect lastItem  = [self rectOfRow:[self rowForItem:item] + children];
    
    return NSMakeRect(NSMinX(firstItem), NSMinY(firstItem), NSMaxX(lastItem)-NSMinX(firstItem), NSMaxY(lastItem)-NSMinY(firstItem));
}

#pragma mark - Selection Highlight
- (id)_highlightColorForCell:(NSCell *)cell
{
    // disable default selection
    return nil;
}

- (void)highlightSelectionInClipRect:(NSRect)theClipRect
{
    NSWindow *win = [self window];
    BOOL isActive = ([win isMainWindow] && [win firstResponder]==self) || [win firstResponder]==[OESidebarFieldEditor fieldEditor];
    
    NSColor *bottomLineColor;
    NSColor *topLineColor;
    
    NSColor *gradientTop;
    NSColor *gradientBottom;
    
    if(isActive)
    {
        // Active
        topLineColor = [NSColor colorWithDeviceRed:0.373 green:0.584 blue:0.91 alpha:1];
        bottomLineColor = [NSColor colorWithDeviceRed:0.157 green:0.157 blue:0.157 alpha:1];
        
        gradientTop = [NSColor colorWithDeviceRed:0.263 green:0.51 blue:0.894 alpha:1];
        gradientBottom = [NSColor colorWithDeviceRed:0.137 green:0.243 blue:0.906 alpha:1];
    }
    else 
    {
        // Inactive
        topLineColor = [NSColor colorWithDeviceRed:0.671 green:0.671 blue:0.671 alpha:1];
        bottomLineColor = [NSColor colorWithDeviceRed:0.184 green:0.184 blue:0.184 alpha:1];
        
        gradientTop = [NSColor colorWithDeviceRed:0.612 green:0.612 blue:0.612 alpha:1];
        gradientBottom = [NSColor colorWithDeviceRed:0.443 green:0.443 blue:0.447 alpha:1];
    }
    
    // draw highlight for visible & selected rows
    NSRange visibleRows = [self rowsInRect:theClipRect];
    NSIndexSet *aSelectedRowIndexes = [self selectedRowIndexes];
    for(NSUInteger aRow=visibleRows.location ; aRow < NSMaxRange(visibleRows); aRow++)
    {
        if([aSelectedRowIndexes containsIndex:aRow])
        {
            NSRect rowFrame = [self rectOfRow:aRow];
            NSRect innerTopLine = NSMakeRect(rowFrame.origin.x, rowFrame.origin.y+1, rowFrame.size.width, 1);
            NSRect topLine = NSMakeRect(rowFrame.origin.x, rowFrame.origin.y, rowFrame.size.width, 1);
            NSRect bottomLine = NSMakeRect(rowFrame.origin.x, rowFrame.origin.y+rowFrame.size.height-1, rowFrame.size.width, 1);
            
            [topLineColor setFill];
            NSRectFill(innerTopLine);
            
            [bottomLineColor setFill];
            NSRectFill(topLine);
            NSRectFill(bottomLine);
            
            rowFrame.size.height -= 3;
            rowFrame.origin.y += 2;
            
            NSGradient *selectionGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
            [selectionGradient drawInRect:rowFrame angle:90];
        }
    }
}

#pragma mark - Drop Highlight
- (struct CGRect)_dropHighlightBackgroundRectForRow:(long long)arg1
{
    return NSZeroRect;
}

- (void)_setNeedsDisplayForDropCandidateRow:(long long)arg1 operation:(unsigned long long)arg2 mask:(unsigned long long)arg3
{
    [self setNeedsDisplayInRect:[self bounds]];
}

- (void)_drawDropHighlightOnRow:(long long)arg1
{
    NSRect rect = [self rectOfGroup:[self itemAtRow:arg1]];
    if([[self itemAtRow:arg1] isGroupHeaderInSidebar] || arg1 == -1)
    {
        rect.origin.y += 2.0;
        
        rect.origin.y += 4.0;
        rect.size.height -= 4.0;
    }
    
    rect = NSInsetRect(rect, 3, 1);
    
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:[self dropCornerRadius] yRadius:[self dropCornerRadius]];
    [path setLineWidth:[self dropBorderWidth]];
    
    [[self dropBackgroundColor] setFill];
    [path fill];
    
    [[self dropBorderColor] setStroke];
    [path stroke];
    
    
    self.isDrawingAboveDropHighlight = YES;
    NSRange rowsInRect = [self rowsInRect:rect];
    for(NSInteger row=rowsInRect.location; row<NSMaxRange(rowsInRect); row++) {
        [self drawRow:row clipRect:[self rectOfRow:row]];
    }
    self.isDrawingAboveDropHighlight = NO;
}

- (id)_dropHighlightColor
{
    return [NSColor colorWithDeviceRed:8/255.0 green:105/255.0 blue:216/255.0 alpha:1.0];
}

- (void)_flashOutlineCell
{}
#pragma mark - Key Handling
- (void)keyDown:(NSEvent *)theEvent
{
    if([theEvent keyCode] == 51 || [theEvent keyCode] == 117)
        [NSApp sendAction:@selector(removeSelectedItemsOfOutlineView:) to:[self dataSource] from:self];
    else
        [super keyDown:theEvent];
}

@end
