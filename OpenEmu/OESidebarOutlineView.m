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
#import "OESideBarGroupItem.h"
#import "OEMenu.h"

#import "OEDBSystem.h"
#import "OELibraryDatabase.h"

#import "OECorePlugin.h"

@interface OESidebarOutlineView ()
{
    // This should only be used for drawing the highlight
    NSInteger _highlightedRow;
}
- (void)OE_selectRowForMenuItem:(NSMenuItem *)menuItem;
- (void)OE_renameRowForMenuItem:(NSMenuItem *)menuItem;
- (void)OE_removeRowForMenuItem:(NSMenuItem *)menuItem;
- (void)OE_toggleSystemForMenuItem:(NSMenuItem *)menuItem;
- (void)OE_duplicateCollectionForMenuItem:(NSMenuItem *)menuItem;
- (void)OE_toggleGroupForMenuItem:(NSMenuItem *)menuItem;
@end

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
        [self OE_setupDefaultColors];
        _highlightedRow = -1;
    }
    return self;
}

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) 
    {
        [self OE_setupDefaultColors];
        _highlightedRow = -1;
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

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:[self bounds]];

    if(_highlightedRow != -1)
        [self _drawDropHighlightOnRow:_highlightedRow];
}

- (void)mouseDown:(NSEvent*)theEvent
{
    // ignore control-clicks, those will go directly to -menuForEvent:
    // before eventually arriving here
    if([theEvent modifierFlags] & NSControlKeyMask)
        return;

    [super mouseDown:theEvent];
}

#pragma mark - Menu
- (NSMenu *)menuForEvent:(NSEvent *)event
{
    [[self window] makeFirstResponder:self];

    NSPoint mouseLocationInWindow = [event locationInWindow];
    NSPoint mouseLocationInView = [self convertPoint:mouseLocationInWindow fromView:nil];
    NSInteger index = [self rowAtPoint:mouseLocationInView];

    if(index == -1) return nil;
    id item = [self itemAtRow:index];

    _highlightedRow = index;
    [self setNeedsDisplay];

    NSMenu *menu = [[NSMenu alloc] init];
    NSMenuItem *menuItem;
    NSString *title;

    if([item isGroupHeaderInSidebar])
    {
        if(index == 0)
        {
            // TODO: clean up, menuForEvent should be delegated to the sidebarcontroller
            for(OEDBSystem *system in [OEDBSystem allSystemsInContext:[[OELibraryDatabase defaultDatabase] mainThreadContext]])
            {
                menuItem = [[NSMenuItem alloc] initWithTitle:[system name] action:@selector(OE_toggleSystemForMenuItem:) keyEquivalent:@""];
                [menuItem setRepresentedObject:system];
                [menuItem setState:[[system enabled] boolValue] ? NSOnState : NSOffState];
                [menu addItem:menuItem];
            }
        }
    }
    else if([item isKindOfClass:[OEDBSystem class]])
    {
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Open Library", @"")
                                              action:@selector(OE_selectRowForMenuItem:)
                                       keyEquivalent:@""];
        [menuItem setTag:index];
        [menu addItem:menuItem];

        NSArray *cores = [OECorePlugin corePluginsForSystemIdentifier:[item systemIdentifier]];
        if([cores count] > 1)
        {
            NSString *title = NSLocalizedString(@"Default Core", @"Sidebar context menu item to pick default core for a system");
            NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
            NSString *systemIdentifier = [(OEDBSystem*)item systemIdentifier];
            NSString *defaultCoreKey = [NSString stringWithFormat:@"defaultCore.%@", systemIdentifier];
            NSString *defaultCoreIdentifier = [defaults objectForKey:defaultCoreKey];

            NSMenuItem *coreItem = [[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""];
            NSMenu *submenu = [[NSMenu alloc] initWithTitle:title];
            for(OECorePlugin *core in cores)
            {
                NSString *coreName = [core displayName];
                NSString *systemIdentifier = [(OEDBSystem*)item systemIdentifier];
                NSString *coreIdentifier = [core bundleIdentifier];

                NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:coreName action:@selector(changeDefaultCore:) keyEquivalent:@""];
                NSInteger state = [coreIdentifier isEqualToString:defaultCoreIdentifier] ? NSOnState : NSOffState;
                [item setState:state];

                [item setRepresentedObject:@{@"core":coreIdentifier, @"system":systemIdentifier}];
                [submenu addItem:item];
            }
            [coreItem setSubmenu:submenu];
            [menu addItem:coreItem];
        }

        [menu addItem:[NSMenuItem separatorItem]];

        NSString *title = [NSString stringWithFormat:NSLocalizedString(@"Hide \"%@\"", @""), [item name]];
        menuItem = [[NSMenuItem alloc] initWithTitle:title action:@selector(OE_toggleSystemForMenuItem:) keyEquivalent:@""];
        [menuItem setRepresentedObject:item];
        [menu addItem:menuItem];
    }
    else
    {
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Open Collection", @"")
                                              action:@selector(OE_selectRowForMenuItem:)
                                       keyEquivalent:@""];
        [menuItem setTag:index];
        [menu addItem:menuItem];

        if([item isEditableInSidebar])
        {
            [menu addItem:[NSMenuItem separatorItem]];

            title = [NSString stringWithFormat:NSLocalizedString(@"Rename \"%@\"", @"Rename collection sidebar context menu item"), [item sidebarName]];
            menuItem = [[NSMenuItem alloc] initWithTitle:title action:@selector(OE_renameRowForMenuItem:) keyEquivalent:@""];
            [menuItem setTag:index];
            [menu addItem:menuItem];

            menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Duplicate Collection", @"")
                                                  action:@selector(OE_duplicateCollectionForMenuItem:)
                                           keyEquivalent:@""];
            [menuItem setRepresentedObject:item];
            [menu addItem:menuItem];
            
            menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Delete Collection", @"")
                                                  action:@selector(OE_removeRowForMenuItem:)
                                           keyEquivalent:@""];
            [menuItem setTag:index];
            [menu addItem:menuItem];
        }
    }


    if([[menu itemArray] count]) {
        OEMenuStyle style = OEMenuStyleDark;
        if([[NSUserDefaults standardUserDefaults] boolForKey:OEMenuOptionsStyleKey]) style = OEMenuStyleLight;

        NSDictionary *options = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInteger:style] forKey:OEMenuOptionsStyleKey];
        [OEMenu openMenu:menu withEvent:event forView:self options:options];
    }

    _highlightedRow = -1;
    [self setNeedsDisplay];

    return nil;
}

- (void)OE_selectRowForMenuItem:(NSMenuItem *)menuItem
{
    [self selectRowIndexes:[NSIndexSet indexSetWithIndex:[menuItem tag]] byExtendingSelection:NO];
}

- (void)OE_renameRowForMenuItem:(NSMenuItem *)menuItem
{
    [NSApp sendAction:@selector(renameItemForMenuItem:) to:[self dataSource] from:menuItem];
}

- (void)OE_removeRowForMenuItem:(NSMenuItem *)menuItem
{
    [NSApp sendAction:@selector(removeItemForMenuItem:) to:[self dataSource] from:menuItem];
}

- (void)OE_toggleSystemForMenuItem:(NSMenuItem *)menuItem
{
    OEDBSystem *system = menuItem.representedObject;
    [system toggleEnabledAndPresentError];
}

- (void)OE_duplicateCollectionForMenuItem:(NSMenuItem *)menuItem
{
    [NSApp sendAction:@selector(duplicateCollection:) to:[self dataSource] from:[menuItem representedObject]];
}

- (void)OE_toggleGroupForMenuItem:(NSMenuItem *)menuItem
{
    id item = [menuItem representedObject];

    if([self isItemExpanded:item])
        [self collapseItem:item];
    else
        [self expandItem:item];
}

#pragma mark - Calculating rects

- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row
{
    return NSZeroRect;
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

    if(![self isItemExpanded:item])
        return [self rectOfRow:[self rowForItem:item]];
    
    // TODO: this will break when we add collection folders that can have children of their own
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
    
    NSColor *fillColor;
    if(isActive)
    {
        // Active
        fillColor = [NSColor colorWithDeviceRed:0.243 green:0.502 blue:0.871 alpha:1];
    }
    else 
    {
        // Inactive
        fillColor = [NSColor colorWithDeviceRed:0.612 green:0.612 blue:0.612 alpha:1];
    }
    
    // draw highlight for visible & selected rows
    NSRange visibleRows = [self rowsInRect:theClipRect];
    NSIndexSet *aSelectedRowIndexes = [self selectedRowIndexes];
    for(NSUInteger aRow=visibleRows.location ; aRow < NSMaxRange(visibleRows); aRow++)
    {
        if([aSelectedRowIndexes containsIndex:aRow])
        {
            NSRect rowFrame = [self rectOfRow:aRow];
            [fillColor set];
            [NSBezierPath fillRect:rowFrame];
        }
    }
}

#pragma mark - Drop Highlight
- (struct CGRect)_dropHighlightBackgroundRectForRow:(NSInteger)arg1
{
    return NSZeroRect;
}

- (void)_setNeedsDisplayForDropCandidateRow:(NSInteger)arg1 operation:(NSUInteger)arg2 mask:(NSUInteger)arg3
{
    [self setNeedsDisplayInRect:[self bounds]];
}

- (void)_drawDropHighlightOnRow:(NSInteger)arg1
{
    // do not draw Drop Highlight on currently selected row
    if([self selectedRow] == arg1)
        return;
    
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
