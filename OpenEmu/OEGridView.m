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

#import "OEBackgroundNoisePattern.h"

#import "OEMenu.h"


@interface OEGridView ()

@property OEGridViewFieldEditor *fieldEditor;

@end

@implementation OEGridView

- (void)awakeFromNib
{
    OEBackgroundNoisePatternCreate();
    CALayer *layer = [CALayer layer];
    
    [layer setContentsGravity:kCAGravityResize];
    [layer setContents:[NSImage imageNamed:@"background_lighting"]];
    [layer setFrame:[self bounds]];
    [layer setDelegate:self];
    [self setBackgroundLayer:layer];

    CALayer *noiseLayer = [CALayer layer];
    [noiseLayer setFrame:[self bounds]];
    [noiseLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [noiseLayer setGeometryFlipped:YES];
    [noiseLayer setBackgroundColor:OEBackgroundNoiseColorRef];
    [layer addSublayer:noiseLayer];

    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSPasteboardTypePNG, NSPasteboardTypeTIFF, nil]];
    [self setAllowsReordering:NO];
    [self setAllowsDroppingOnItems:YES];

    // TODO: Replace magic numbers with constants
    // IKImageBrowserView adds 20 pixels vertically for the title
    //[self setCellSize:NSMakeSize(13 + 142 + 13, 7 + 142 + 16 + 17 + 11)];
    [self setCellSize:NSMakeSize(142 + 26, 142)];
    // TODO: Explain subtraction
    [self setIntercellSpacing:NSMakeSize(22, 29 - 20)];
    [self setCellsStyleMask:IKCellsStyleTitled];

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

- (void)renameSelectedGame:(id)sender
{
    if([[self selectionIndexes] count] != 1)
    {
        DLog(@"I can only rename a single game, sir.");
        return;
    }

    NSUInteger selectedIndex = [[self selectionIndexes] firstIndex];

    OEGridCell *selectedCell = (OEGridCell *)[self cellForItemAtIndex:selectedIndex];
    if(!selectedCell) return;
    if(![selectedCell isKindOfClass:[OEGridCell class]]) return;

    [self OE_setupFieldEditorForCell:selectedCell];
}

- (void)OE_setupFieldEditorForCell:(OEGridCell *)cell
{
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
    if([_fieldEditor isHidden]) return;

    //OEGridCell *delegate = [_fieldEditor delegate];
    //if([delegate isKindOfClass:[OEGridViewCell class]]) [delegate setEditing:NO];

    [_fieldEditor setHidden:YES];
    [[self window] makeFirstResponder:self];
}

#pragma mark - NSControlSubclassNotifications

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
    NSUInteger selectedIndex = [[self selectionIndexes] firstIndex];
    OEGridCell *selectedCell = (OEGridCell *)[self cellForItemAtIndex:selectedIndex];
    
    OEDBGame *selectedGame = [selectedCell representedItem];
    
    [selectedGame setDisplayName:[_fieldEditor string]];
    
    if([selectedGame isKindOfClass:[NSManagedObject class]])
    {
        [[(NSManagedObject*)selectedGame managedObjectContext] save:nil];
    }
    
    [self OE_cancelFieldEditor];
    [self reloadData];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
    BOOL retval = NO;
    
    if ([[control superview] isKindOfClass:[OEGridViewFieldEditor class]])
    {
        if(commandSelector == @selector(complete:)) {
            
            // User pressed the 'Esc' key, cancel the editing
            retval = YES;
            [self OE_cancelFieldEditor];
            [[self window] makeFirstResponder:nil];
        }
    }
    
    // Debug: NSLog(@"Selector = %@", NSStringFromSelector( commandSelector ) );
    return retval;
}


@end

