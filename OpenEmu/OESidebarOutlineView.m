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

@implementation OESidebarOutlineView

- (id)init
{
    self = [super init];
    if (self) 
    {
        [self setupOutlineCell];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{    
    self = [super initWithCoder:aDecoder];
    if (self) 
    {
        [self setupOutlineCell];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) 
    {
        [self setupOutlineCell];
    }
    
    return self;
}

#pragma mark -
#pragma mark Drag and Drop
@synthesize dragDelegate;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate draggingEntered:sender];
}
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate draggingUpdated:sender];
}
- (void)draggingEnded:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate draggingEnded:sender];
}
- (void)draggingExited:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate draggingExited:sender];
}

- (BOOL)prepareForDragOperation:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate prepareForDragOperation:sender];
}
- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate performDragOperation:sender];
}
- (void)concludeDragOperation:(id < NSDraggingInfo >)sender
{
    return [self.dragDelegate concludeDragOperation:sender];
}
#pragma mark -
- (void)reloadData
{
    [super reloadData];
    
    if([[NSUserDefaults standardUserDefaults] boolForKey:UDSidebarCollectionNotCollapsableKey])
        [self expandItem:[self itemAtRow:0]];
}

- (void)reloadItem:(id)item
{
    [super reloadItem:item];
    
    if([[NSUserDefaults standardUserDefaults] boolForKey:UDSidebarCollectionNotCollapsableKey])
        [self expandItem:[self itemAtRow:0]];
}

- (id)_highlightColorForCell:(NSCell *)cell
{
    // we need to override this to return nil
    // or we'll see the default selection rectangle when the app is running 
    // in any OS before leopard
    
    // you can also return a color if you simply want to change the table's default selection color
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
    { // Active
        topLineColor = [NSColor colorWithDeviceRed:0.373 green:0.584 blue:0.91 alpha:1];
        bottomLineColor = [NSColor colorWithDeviceRed:0.157 green:0.157 blue:0.157 alpha:1];
        
        gradientTop = [NSColor colorWithDeviceRed:0.263 green:0.51 blue:0.894 alpha:1];
        gradientBottom = [NSColor colorWithDeviceRed:0.137 green:0.243 blue:0.906 alpha:1];
    }
    else 
    { // Inactive
        topLineColor = [NSColor colorWithDeviceRed:0.671 green:0.671 blue:0.671 alpha:1];
        bottomLineColor = [NSColor colorWithDeviceRed:0.184 green:0.184 blue:0.184 alpha:1];
        
        gradientTop = [NSColor colorWithDeviceRed:0.612 green:0.612 blue:0.612 alpha:1];
        gradientBottom = [NSColor colorWithDeviceRed:0.443 green:0.443 blue:0.447 alpha:1];
    }
    
    NSRange aVisibleRowIndexes = [self rowsInRect:theClipRect];
    NSIndexSet *aSelectedRowIndexes = [self selectedRowIndexes];
    NSUInteger aRow = aVisibleRowIndexes.location;
    NSUInteger anEndRow = aRow + aVisibleRowIndexes.length;
     
    // draw highlight for the visible, selected rows
    for ( ; aRow < anEndRow; aRow++)
    {
        if([aSelectedRowIndexes containsIndex:aRow])
        {
            NSRect rowFrame = [self rectOfRow:aRow];
            
            NSRect innerTopLine = NSMakeRect(rowFrame.origin.x, rowFrame.origin.y+1, rowFrame.size.width, 1);
            NSRect topLine = NSMakeRect(rowFrame.origin.x, rowFrame.origin.y, rowFrame.size.width, 1);
            NSRect bottomLine = NSMakeRect(rowFrame.origin.x, rowFrame.origin.y+rowFrame.size.height-1, rowFrame.size.width+3, 1);
            
            [topLineColor setFill];
            NSRectFill(innerTopLine);
            
            [bottomLineColor setFill];
            NSRectFill(topLine);
            NSRectFill(bottomLine);
            
            NSRect gradientRect = rowFrame;
            gradientRect.size.height -= 3;
            gradientRect.origin.y += 2;
            
            NSGradient *selectionGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
            [selectionGradient drawInRect:gradientRect angle:90];
        }
    }
}
- (NSRect)rectOfRow:(NSInteger)row 
{
    NSRect rect = [super rectOfRow:row];
    rect.size.width -= 1;
    return rect;
}

- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row
{
    if(row==0 && [[NSUserDefaults standardUserDefaults] boolForKey:UDSidebarCollectionNotCollapsableKey])
        return NSZeroRect;
    
    NSRect rect = [super frameOfOutlineCellAtRow:row];
    
    rect.origin.y += 3;
    
    return rect;
}

#pragma mark -
- (void) keyDown:(NSEvent *) theEvent
{
    if([theEvent keyCode]==51 || [theEvent keyCode]==117)
    {
        [NSApp sendAction:@selector(removeSelectedItemsOfOutlineView:) to:[self dataSource] from:self];
    } 
    else
    {
        [super keyDown:theEvent];
    }
}

@end
