/*
 Copyright (c) 2011-2012, OpenEmu Team

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

#import "OETableView.h"

#import "OETableCornerView.h"
#import "OETableHeaderCell.h"
#import "OETableTextFieldCell.h"

#import "OEMenu.h"
#import "OEGridView.h"


static NSColor *cellEditingFillColor, *textColor, *cellSelectedTextColor, *strokeColor;
static NSGradient *highlightGradient, *normalGradient;


@interface OETableView ()
@property(strong, readwrite) NSColor *selectionColor;
- (BOOL)OE_isActive;
@end


@implementation OETableView

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) 
	{
		if(!cellEditingFillColor)
		{
			cellEditingFillColor = [NSColor greenColor];
		}
		
		if(!textColor)
		{
			textColor = [NSColor whiteColor];
		}
		
		if(!cellSelectedTextColor)
		{
			cellSelectedTextColor = [NSColor whiteColor];
		}
		
		if(!highlightGradient)
		{
			highlightGradient = [[NSGradient alloc] initWithStartingColor:[NSColor greenColor] endingColor:[NSColor magentaColor]];
		}

		if(!strokeColor)
		{
			strokeColor = [NSColor blackColor];
		}

		if(!normalGradient)
		{
			NSColor *c1 = [NSColor colorWithDeviceWhite:0.29 alpha:1.0];
			NSColor *c2 = [NSColor colorWithDeviceWhite:0.18 alpha:1.0];
			normalGradient = [[NSGradient alloc] initWithStartingColor:c1 endingColor:c2];
		}
		
		self.selectionColor = [NSColor colorWithDeviceRed:0.173 green:0.286 blue:0.976 alpha:1.0];
		[self setIntercellSpacing:NSMakeSize(1, 0)];
		
		[self setBackgroundColor:[NSColor blackColor]];
		[self setFocusRingType:NSFocusRingTypeNone];
		
		for (NSTableColumn *aColumn in [self tableColumns]) 
		{
			OETableHeaderCell *newHeader = [[OETableHeaderCell alloc] initTextCell:[[aColumn headerCell] stringValue]];
			[newHeader setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11]];
			[aColumn setHeaderCell: newHeader];
		}
        
        [self setHeaderClickable:YES];
        [self setCornerView:[[OETableCornerView alloc] init]];
	}
    return self;
}

- (void)drawBackgroundInClipRect:(NSRect)clipRect
{
	NSColor *rowBackground = [NSColor colorWithDeviceWhite:0.059 alpha:1.0];
	NSColor *alternateRowBackground = [NSColor colorWithDeviceWhite:0.114 alpha:1.0];
	
	[rowBackground setFill];
	NSRectFill(clipRect);
	
	[alternateRowBackground setFill];
	
	NSRect rect = [self visibleRect];
	for(float i=[self rowHeight]+[self intercellSpacing].height; i<rect.origin.y+rect.size.height; i+=2*([self rowHeight]+[self intercellSpacing].height))
	{
		NSRect rowRect = NSMakeRect(rect.origin.x, i, rect.size.width, [self rowHeight]+[self intercellSpacing].height);
		NSRectFill(rowRect);
	}
}

- (BOOL)OE_isActive
{
    return [[self window] isMainWindow] && ([[self window] firstResponder] == self || [[self window] firstResponder] == [self currentEditor]);
}

- (void)highlightSelectionInClipRect:(NSRect)theClipRect
{
	NSColor *fillColor;
	NSColor *lineColor;
	if([self OE_isActive])
	{
		fillColor = self.selectionColor;
		lineColor = [NSColor colorWithDeviceRed:0.114 green:0.188 blue:0.635 alpha:1.0];
	} 
	else 
	{
		fillColor = [NSColor colorWithDeviceWhite:0.55 alpha:1.0];
		lineColor = [NSColor colorWithDeviceWhite:0.35 alpha:1.0];
	}
	
	[fillColor setFill];
	
	NSIndexSet *selectedRows = [self selectedRowIndexes];
	
	NSUInteger nextIndex = [selectedRows firstIndex];
	NSUInteger currentIndex = [selectedRows firstIndex];
	while(currentIndex!=NSNotFound)
	{
		NSRect frame = [self rectOfRow:nextIndex];
		NSRectFill(frame);
		
		nextIndex = [selectedRows indexGreaterThanIndex:currentIndex];
		
		if(nextIndex == currentIndex+1)
		{
			[lineColor setFill];
			frame.origin.y += frame.size.height-1;
			frame.size.height = 1;
			
			NSRectFill(frame);
			[fillColor setFill];
		}
		
		currentIndex = nextIndex;
	}
}

- (void)drawGridInClipRect:(NSRect)aRect
{
	NSSize gridSize = NSMakeSize(1, [self bounds].size.height);
	
	[[NSColor colorWithDeviceRed:0.255 green:0.251 blue:0.255 alpha:1.0] setFill];
	
	NSRect fillRect;
	fillRect.size = gridSize;
	fillRect.origin = aRect.origin;
		
	for(NSUInteger i=0; i < [[self tableColumns] count]; i++)
	{
        if([[[self tableColumns] objectAtIndex:i] isHidden])
            continue;
        
		NSRect colRect = [self rectOfColumn:i];
		fillRect.origin.x = colRect.origin.x + colRect.size.width-1;
		NSRectFill(fillRect);
	}
	
	NSColor *fillColor;
	if([self OE_isActive])
	{
		fillColor = [NSColor colorWithDeviceRed:0.235 green:0.455 blue:0.769 alpha:1.0];
	}
	else 
	{
		fillColor = [NSColor colorWithDeviceWhite:0.33 alpha:1.0];
	}
	
	[fillColor setFill];
	
	NSIndexSet *selectedRows = [self selectedRowIndexes];
	
	NSUInteger nextIndex;
	NSUInteger currentIndex = [selectedRows firstIndex];
	while(currentIndex!=NSNotFound)
	{
		NSRect rowfillRect = [self rectOfRow:currentIndex];
		rowfillRect.size.width = 1;
		
		nextIndex = [selectedRows indexGreaterThanIndex:currentIndex];
		
		if(nextIndex == currentIndex+1)
			rowfillRect.size.height -= 1;
		
		for(NSUInteger i=0; i < [[self tableColumns] count]; i++)
		{
			NSRect colRect = [self rectOfColumn:i];
			
			rowfillRect.origin.x = colRect.origin.x + colRect.size.width-1;
			NSRectFill(rowfillRect);
		}
		
		currentIndex = nextIndex;
	}
}

- (void)setHeaderClickable:(BOOL)flag{
    for (NSTableColumn *aColumn in [self tableColumns]) 
    {
        OETableHeaderCell *cell = [aColumn headerCell];
        [cell setClickable:flag];
    }
}

- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
    [[self window] makeFirstResponder:self];
    
    NSPoint mouseLocationInWindow = [theEvent locationInWindow];
    NSPoint mouseLocationInView = [self convertPoint:mouseLocationInWindow fromView:nil];
    
    NSUInteger index = [self rowAtPoint:mouseLocationInView];
    if(index != NSNotFound && [[self dataSource] respondsToSelector:@selector(tableView:menuForItemsAtIndexes:)])
    {
        NSRect rowRect = [self rectOfRow:index];
        mouseLocationInView.y = rowRect.origin.y - rowRect.size.height/2;
        
        BOOL itemIsSelected = [[self selectedRowIndexes] containsIndex:index];
        NSIndexSet *indexes = itemIsSelected ? [self selectedRowIndexes] : [NSIndexSet indexSetWithIndex:index];
        if(!itemIsSelected)
            [self selectRowIndexes:indexes byExtendingSelection:NO];
        
        NSMenu *contextMenu = [(id <OETableViewMenuSource>)[self dataSource] tableView:self menuForItemsAtIndexes:indexes];
        
        OEMenuStyle style = OEMenuStyleDark;
        if([[NSUserDefaults standardUserDefaults] boolForKey:OELightStyleGridViewMenu]) style = OEMenuStyleLight;
        
        NSDictionary *options = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInteger:style] forKey:OEMenuOptionsStyleKey];
        [OEMenu openMenu:contextMenu withEvent:theEvent forView:self options:options];
        return nil;
    }

    return [super menuForEvent:theEvent];
}

- (void)keyDown:(NSEvent *)theEvent
{
    if([theEvent keyCode]==51 || [theEvent keyCode]==117)
        [NSApp sendAction:@selector(delete:) to:nil from:self];
    else
        [super keyDown:theEvent];        
}

- (void)mouseDown:(NSEvent *)theEvent
{
    // AppKit posts a control-mouse-down event when the user control-clicks the view and -menuForEvent: returns nil
    // since a nil return normally means there is no contextual menu.
    // However, we do show a menu before returning nil from -menuForEvent:, so we need to ignore control-mouse-down events.
    if(!([theEvent modifierFlags] & NSControlKeyMask))
        [super mouseDown:theEvent];
}

- (void)cancelOperation:(id)sender
{
    NSTextView *fieldEditor = (NSTextView *)[self currentEditor];
    if(fieldEditor)
    {
        [self abortEditing];
        [[self window] makeFirstResponder:self];
    }
}

#pragma mark - Context menu

- (void)renameSelectedGame:(id)sender
{
    if([[self selectedRowIndexes] count] != 1)
    {
        DLog(@"I can only rename a single game, sir.");
        return;
    }

    NSInteger selectedRow = [[self selectedRowIndexes] firstIndex];

    NSInteger titleColumnIndex = [self columnWithIdentifier:@"listViewTitle"];
    NSAssert(titleColumnIndex != -1, @"The list view must have a column identified by listViewTitle");

    [self editColumn:titleColumnIndex row:selectedRow withEvent:nil select:NO];
}

@synthesize selectionColor;
@end
