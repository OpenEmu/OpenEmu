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

#import "OESidebarController.h"

#import <objc/runtime.h>
#import "OESidebarGroupItem.h"

#import "OEDBSystem+CoreDataProperties.h" 
#import "OELibraryDatabase.h"

@import OpenEmuKit;
#import "OpenEmu-Swift.h"

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
    [self setDropBorderColor:[NSColor controlAccentColor]];
    [self setDropBorderWidth:2.0];
    [self setDropCornerRadius:8.0];
    self.selectionHighlightStyle = NSTableViewSelectionHighlightStyleSourceList;
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
    if([theEvent modifierFlags] & NSEventModifierFlagControl)
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
    [self setNeedsDisplay:YES];

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
                [menuItem setState:[[system enabled] boolValue] ? NSControlStateValueOn : NSControlStateValueOff];
                [menu addItem:menuItem];
            }
        }
    }
    else if([item isKindOfClass:[OEDBSystem class]])
    {
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
                NSInteger state = [coreIdentifier isEqualToString:defaultCoreIdentifier] ? NSControlStateValueOn : NSControlStateValueOff;
                [item setState:state];

                [item setRepresentedObject:@{@"core":coreIdentifier, @"system":systemIdentifier}];
                [submenu addItem:item];
            }
            [coreItem setSubmenu:submenu];
            [menu addItem:coreItem];
        }

        NSString *title = [NSString stringWithFormat:NSLocalizedString(@"Hide \"%@\"", @""), [(OEDBSystem*)item name]];
        menuItem = [[NSMenuItem alloc] initWithTitle:title action:@selector(OE_toggleSystemForMenuItem:) keyEquivalent:@""];
        [menuItem setRepresentedObject:item];
        [menu addItem:menuItem];
    }
    else
    {
        if([item isEditableInSidebar])
        {
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
            
            [menu addItem:[NSMenuItem separatorItem]];
        }
        
        menuItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"New Collection", @"")
                                              action:@selector(newCollection:)
                                       keyEquivalent:@""];
        [menu addItem:menuItem];
    }


    if([[menu itemArray] count]) {
        [NSMenu popUpContextMenu:menu withEvent:event forView:self];
    }

    _highlightedRow = -1;
    [self setNeedsDisplay:YES];

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
    
    [[self dropBorderColor] setStroke];
    [path stroke];
}

- (id)_dropHighlightColor
{
    return [NSColor controlAccentColor];
}

- (void)_flashOutlineCell
{}

#pragma mark - Key Handling
- (void)keyDown:(NSEvent *)event;
{
    if(event.keyCode == 51 || event.keyCode == 117)
        [NSApp sendAction:@selector(removeSelectedItemsOfOutlineView:) to:self.dataSource from:self];
    // ignore left and right arrow keys; pressing them causes flickering
    else if(event.keyCode == 123 || event.keyCode == 124)
        return;
    else
        [super keyDown:event];
}

@end
