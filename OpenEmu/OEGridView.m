/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEGridView.h"
#import "OECollectionViewController.h"

#import "OEGridCell.h"
#import "OEGridViewFieldEditor.h"

#import "OECoverGridForegroundLayer.h"
#import "OEBackgroundNoisePattern.h"

#import "OEMenu.h"

// TODO: replace OEDBGame with OECoverGridDataSourceItem
@interface OEGridView ()
@property NSInteger editingIndex;
@property NSInteger ratingTracking;
@property OEGridViewFieldEditor *fieldEditor;
@end

@implementation OEGridView

- (void)awakeFromNib
{
    _editingIndex = NSNotFound;
    _ratingTracking = NSNotFound;

    [self setValue:[NSColor clearColor] forKey:IKImageBrowserBackgroundColorKey];

    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSPasteboardTypePNG, NSPasteboardTypeTIFF, nil]];
    [self setAllowsReordering:NO];
    [self setAllowsDroppingOnItems:YES];

    OECoverGridForegroundLayer *foregroundLayer = [OECoverGridForegroundLayer layer];
    [foregroundLayer setActions:@{}];
    [self setForegroundLayer:foregroundLayer];

    // TODO: Replace magic numbers with constants
    // IKImageBrowserView adds 20 pixels vertically for the title
    // TODO: Explain subtraction
    [self setCellsStyleMask:IKCellsStyleNone];

    //[self setAnimates:YES];

    // Set Fonts
    NSMutableParagraphStyle *paraphStyle = [[NSMutableParagraphStyle alloc] init];
	[paraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	[paraphStyle setAlignment:NSCenterTextAlignment];

    NSFont *titleFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];

    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowOffset:NSMakeSize(0.0, -1.0)];

	NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
	[attributes setObject:titleFont forKey:NSFontAttributeName];
	[attributes setObject:paraphStyle forKey:NSParagraphStyleAttributeName];
	[attributes setObject:[NSColor whiteColor] forKey:NSForegroundColorAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
	[self setValue:attributes forKey:IKImageBrowserCellsTitleAttributesKey];
    [self setValue:attributes forKey:IKImageBrowserCellsHighlightedTitleAttributesKey];

    _fieldEditor = [[OEGridViewFieldEditor alloc] initWithFrame:NSMakeRect(50, 50, 50, 50)];
    [self addSubview:_fieldEditor];
}

- (void)viewBoundsChanged:(NSNotification *)aNotification
{
    DLog();
    [[[self subviews] objectAtIndex:0] setFrame:[self visibleRect]];
}
#pragma mark - Cells
- (void)setCellSize:(NSSize)size
{
    [super setCellSize:size];
}

- (NSSize)cellSize
{
    return [super cellSize];
}

- (IKImageBrowserCell *)newCellForRepresentedItem:(id) cell
{
	return [[OEGridCell alloc] init];
}
#pragma mark -
- (NSInteger)indexOfItemWithFrameAtPoint:(NSPoint)point
{
    __block NSInteger result = NSNotFound;
    [[self visibleItemIndexes] enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        NSRect frame = [self itemFrameAtIndex:idx];
        if(NSPointInRect(point, frame))
        {
            result = idx;
            *stop = YES;
        }
    }];
    return result;
}
#pragma mark -
- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

- (BOOL)resignFirstResponder
{
    return YES;
}
#pragma mark -
- (void)mouseDown:(NSEvent *)theEvent
{
    [self OE_cancelFieldEditor];

    NSPoint mouseLocationInWindow = [theEvent locationInWindow];
    NSPoint mouseLocationInView = [self convertPoint:mouseLocationInWindow fromView:nil];
    NSInteger index = [self indexOfItemWithFrameAtPoint:mouseLocationInView];

    if(index != NSNotFound)
    {
        OEGridCell *clickedCell = (OEGridCell*)[self cellForItemAtIndex:index];
        NSRect titleRect  = [clickedCell titleFrame];
        NSRect ratingRect = NSInsetRect([clickedCell ratingFrame], -5, -1);

        // see if user double clicked on title layer
        if([theEvent clickCount] >= 2 && NSPointInRect(mouseLocationInView, titleRect))
        {
            [self renameGameAtIndex:index];
            return;
        }
        // Check for rating layer interaction
        else if(NSPointInRect(mouseLocationInView, ratingRect))
        {
            _ratingTracking = index;
            [self OE_updateRatingForItemAtIndex:index withLocation:mouseLocationInView inRect:ratingRect];
            return;
        }
    }

    [super mouseDown:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint mouseLocationInWindow = [theEvent locationInWindow];
    NSPoint mouseLocationInView = [self convertPoint:mouseLocationInWindow fromView:nil];

    if(_ratingTracking != NSNotFound)
    {
        OEGridCell *clickedCell = (OEGridCell*)[self cellForItemAtIndex:_ratingTracking];
        NSRect ratingRect = NSInsetRect([clickedCell ratingFrame], -5, -1);

        [self OE_updateRatingForItemAtIndex:_ratingTracking withLocation:mouseLocationInView inRect:ratingRect];
        return;
    }

    [super mouseDragged:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    _ratingTracking = NSNotFound;
    [super mouseUp:theEvent];
}
#pragma mark -
- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
    [[self window] makeFirstResponder:self];

    NSPoint mouseLocationInWindow = [theEvent locationInWindow];
    NSPoint mouseLocationInView = [self convertPoint:mouseLocationInWindow fromView:nil];

    NSInteger index = [self indexOfItemAtPoint:mouseLocationInView];
    if(index != NSNotFound && [[self dataSource] respondsToSelector:@selector(gridView:menuForItemsAtIndexes:)])
    {
        BOOL            itemIsSelected      = [[self cellForItemAtIndex:index] isSelected];
        NSIndexSet     *indexes             = itemIsSelected ? [self selectionIndexes] : [NSIndexSet indexSetWithIndex:index];

        if(!itemIsSelected)
            [self setSelectionIndexes:indexes byExtendingSelection:NO];

        NSMenu *contextMenu = [(id <OEGridViewMenuSource>)[self dataSource] gridView:self menuForItemsAtIndexes:indexes];

        OEMenuStyle     style      = ([[NSUserDefaults standardUserDefaults] boolForKey:OEMenuOptionsStyleKey] ? OEMenuStyleLight : OEMenuStyleDark);
        IKImageBrowserCell *itemCell   = [self cellForItemAtIndex:index];

        NSRect          hitRect             = NSInsetRect([itemCell imageFrame], 5, 5);
        //NSRect          hitRectOnView       = [itemCell convertRect:hitRect toLayer:self.layer];
        int major, minor;
        GetSystemVersion(&major, &minor, NULL);
        if (major == 10 && minor < 8) hitRect.origin.y = self.bounds.size.height - hitRect.origin.y - hitRect.size.height;
        NSRect          hitRectOnWindow     = [self convertRect:hitRect toView:nil];
        NSRect          visibleRectOnWindow = [self convertRect:[self visibleRect] toView:nil];
        NSRect          visibleItemRect     = NSIntersectionRect(hitRectOnWindow, visibleRectOnWindow);

        // we enhance the calculated rect to get a visible gap between the item and the menu
        NSRect enhancedVisibleItemRect = NSInsetRect(visibleItemRect, -3, -3);

        const NSRect  targetRect = [[self window] convertRectToScreen:enhancedVisibleItemRect];
        NSDictionary *options    = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithUnsignedInteger:style], OEMenuOptionsStyleKey,
                                    [NSNumber numberWithUnsignedInteger:OEMinXEdge], OEMenuOptionsArrowEdgeKey,
                                    [NSValue valueWithRect:targetRect], OEMenuOptionsScreenRectKey,
                                    nil];

        // Display the menu
        [[self window] makeFirstResponder:self];
        [OEMenu openMenu:contextMenu withEvent:theEvent forView:self options:options];
        return nil;
    }

    return [super menuForEvent:theEvent];
}
#pragma mark - Rating items
- (void)OE_updateRatingForItemAtIndex:(NSInteger)index withLocation:(NSPoint)location inRect:(NSRect)rect
{
    CGFloat percent = (location.x - NSMinX(rect))/NSWidth(rect);
    percent = MAX(MIN(percent, 1.0), 0.0);
    [self setRating:roundf(5*percent) forGameAtIndex:index];
}

- (void)setRating:(NSInteger)rating forGameAtIndex:(NSInteger)index
{
    OEGridCell *selectedCell = (OEGridCell *)[self cellForItemAtIndex:index];
    OEDBGame   *selectedGame = [selectedCell representedItem];

    [selectedGame setRating:@(rating)];
    // TODO: can we only reload one item?
    [self reloadData];
}
#pragma mark - Renaming items
- (void)renameSelectedGame:(id)sender
{
    if([[self selectionIndexes] count] != 1)
    {
        DLog(@"I can only rename a single game, sir.");
        return;
    }

    NSUInteger selectedIndex = [[self selectionIndexes] firstIndex];
    [self renameGameAtIndex:selectedIndex];
}

- (void)renameGameAtIndex:(NSInteger)index
{
    if(![[self fieldEditor] isHidden])
        [self OE_cancelFieldEditor];

    OEGridCell *selectedCell = (OEGridCell *)[self cellForItemAtIndex:index];
    if(!selectedCell) return;
    if(![selectedCell isKindOfClass:[OEGridCell class]]) return;

    [self OE_setupFieldEditorForCellAtIndex:index];
}

#pragma mark -

- (void)OE_setupFieldEditorForCellAtIndex:(NSInteger)index
{
    OEGridCell *cell = (OEGridCell*)[self cellForItemAtIndex:index];
    _editingIndex = index;

    NSRect fieldFrame = [cell titleFrame];
    fieldFrame        = NSOffsetRect(NSInsetRect(fieldFrame, 0.0, -1.0), 0.0, -1.0);
    [_fieldEditor setFrame:fieldFrame];

    //[textLayer setHidden:YES];

    NSString *title = [[cell representedItem] displayName];
    [_fieldEditor setString:title];
    [_fieldEditor setDelegate:self];
    [_fieldEditor setHidden:NO];
    [[self window] makeFirstResponder:[[_fieldEditor subviews] objectAtIndex:0]];
}

- (void)OE_cancelFieldEditor
{
    _editingIndex   = NSNotFound;

    if([_fieldEditor isHidden]) return;

    //OEGridCell *delegate = [_fieldEditor delegate];
    //if([delegate isKindOfClass:[OEGridViewCell class]]) [delegate setEditing:NO];

    [_fieldEditor setHidden:YES];
    [[self window] makeFirstResponder:self];
}

- (void)setDropIndex:(NSInteger)index dropOperation:(IKImageBrowserDropOperation)operation
{
    if(operation != IKImageBrowserDropOn) index = -1;
    [super setDropIndex:index dropOperation:operation];
    _draggingOperation = operation;
}

#pragma mark - NSControlSubclassNotifications
- (void)controlTextDidEndEditing:(NSNotification *)obj
{
    // The _fieldEditor finished editing, so let's save the game with the new name
    if ([[[obj object] superview] isKindOfClass:[OEGridViewFieldEditor class]])
    {
        if(_editingIndex == NSNotFound) return;

        OEGridCell *selectedCell = (OEGridCell *)[self cellForItemAtIndex:_editingIndex];
        OEDBGame   *selectedGame = [selectedCell representedItem];
        
        [selectedGame setDisplayName:[_fieldEditor string]];
        [[selectedGame managedObjectContext] save:nil];
        
        [self OE_cancelFieldEditor];
        [self reloadData];
    }
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
    if ([[control superview] isKindOfClass:[OEGridViewFieldEditor class]])
    {
        // User pressed the 'Esc' key, cancel the editing
        if(commandSelector == @selector(cancelOperation:))
        {
            [self OE_cancelFieldEditor];
            return YES;
        }
    }
    
    return NO;
}
@end

