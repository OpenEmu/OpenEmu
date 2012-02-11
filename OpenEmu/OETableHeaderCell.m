//
//  OETableViewHeaderCell.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OETableHeaderCell.h"

#import "OEUIDrawingUtils.h"
#import "NSImage+OEDrawingAdditions.h"
#pragma mark -
@implementation OETableHeaderCell
@synthesize clickable;

- (id)init {
    self = [super init];
    if (self) {
    }
    return self;
}


- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
    }
    return self;
}

- (id)initTextCell:(NSString *)aString{
	self = [super initTextCell:aString];
    if (self) {
    }
    return self;
}

+ (void)initialize
{
    if(self != [OETableHeaderCell class])
        return;
    
    NSImage *sourceImage = [NSImage imageNamed:@"table_header"];
    [sourceImage setName:@"table_header_background_active" forSubimageInRect:NSMakeRect(0, 17, 16, 17)];
    [sourceImage setName:@"table_header_background_pressed" forSubimageInRect:NSMakeRect(0, 0, 16, 17)];
    
    sourceImage = [NSImage imageNamed:@"sort_arrow"];
    [sourceImage setName:@"sort_arrow_inactive" forSubimageInRect:NSMakeRect(0, 0, 15, 14)];
    [sourceImage setName:@"sort_arrow_pressed" forSubimageInRect:NSMakeRect(15, 0, 15, 14)];
    [sourceImage setName:@"sort_arrow_rollover" forSubimageInRect:NSMakeRect(30, 0, 15, 14)];
}

#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
	NSTableView *tableView = [(NSTableHeaderView*)controlView tableView];

	BOOL isPressed = [self state] && [self isClickable];
	BOOL isFirst = [(NSTableHeaderView*)controlView columnAtPoint:cellFrame.origin] == 0;

	BOOL hideDarkBorderOnRight = !isFirst && ([(NSTableHeaderView*)controlView columnAtPoint:cellFrame.origin] >= [[tableView tableColumns] count]-1);
	BOOL hideHighlight = hideDarkBorderOnRight && ([(NSTableHeaderView*)controlView columnAtPoint:cellFrame.origin] >= [[tableView tableColumns] count]);
	
	NSRect sourceRect = NSZeroRect;
	NSImage *backgroundImage = [NSImage imageNamed:@"table_header_background_active"];
	if(hideDarkBorderOnRight) sourceRect = NSMakeRect(0, 0, backgroundImage.size.width-1, backgroundImage.size.height);
	
	if(isPressed){
		backgroundImage = [NSImage imageNamed:@"table_header_background_pressed"];
		[backgroundImage drawInRect:cellFrame fromRect:sourceRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:hideDarkBorderOnRight?7:8 topBorder:0 bottomBorder:0];
	} else {
		[backgroundImage drawInRect:cellFrame fromRect:sourceRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:hideDarkBorderOnRight?7:8 topBorder:0 bottomBorder:0];
		
		// draw highlight on left edge
		if(!isFirst && !hideHighlight){
			NSRect leftHighlightRect = cellFrame;
			leftHighlightRect.size.width = 1;
			
			[[NSColor colorWithDeviceWhite:1.0 alpha:0.04] setFill];
			NSRectFillUsingOperation(leftHighlightRect, NSCompositeSourceOver);
		}
	}
	
	/*
	 *	Highlight stuff (included in image)
	 *
	 
	// Draw Dark border on right
	NSRect rightBorderRect = cellFrame;
	rightBorderRect.origin.x += rightBorderRect.size.width-1;
	rightBorderRect.size.width = 1;
	
	[[NSColor colorWithDeviceWhite:0.08 alpha:1.0] setFill];
	NSRectFill(rightBorderRect);
	
	// Draw dark dot in lower right corner
	NSRect lowerRightBorderRect = cellFrame;
	lowerRightBorderRect.origin.x += lowerRightBorderRect.size.width-1;
	lowerRightBorderRect.size.width = 1;
	lowerRightBorderRect.origin.y += lowerRightBorderRect.size.height-1;
	lowerRightBorderRect.size.height = 1;
	
	[[NSColor colorWithDeviceWhite:0.06 alpha:1.0] setFill];
	NSRectFill(lowerRightBorderRect);
	
	// Draw Black Border on bottom
	NSRect borderLineRect = cellFrame;
	borderLineRect.origin.y += borderLineRect.size.height-1;
	borderLineRect.size.height = 1;
	
	[[NSColor blackColor] setFill];
	NSRectFill(borderLineRect); 
	 */
	
	NSFont *titleFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:4 size:11];
	NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
	[paraStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	
	NSShadow *shadow = [[NSShadow alloc] init];
	[shadow setShadowColor:[NSColor blackColor]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	[shadow setShadowBlurRadius:0];
		
	NSDictionary *attributes;
	NSAttributedString *header;
	NSRect headerRect;

	NSColor *textColor = [NSColor colorWithDeviceWhite:.85 alpha:1];
	
	// Draw glow if header is pressed
	if(isPressed){
		textColor = [NSColor whiteColor];
		
		headerRect = NSInsetRect(cellFrame, 8, 0);
		headerRect.origin.y += 1;
		
		NSShadow *glow = [[NSShadow alloc] init];
		
		[glow setShadowColor:[NSColor whiteColor]];
		[glow setShadowOffset:NSMakeSize(0, 0)];
		[glow setShadowBlurRadius:5];
		
		attributes = [NSDictionary dictionaryWithObjectsAndKeys:
					  textColor, NSForegroundColorAttributeName,
					  titleFont, NSFontAttributeName,
					  glow, NSShadowAttributeName,
					  paraStyle, NSParagraphStyleAttributeName,
					  nil];
		
		header = [[NSAttributedString alloc] initWithString:[self title] attributes:attributes];
		[header drawInRect:headerRect];
	}
	
	attributes = [NSDictionary dictionaryWithObjectsAndKeys:
				  textColor, NSForegroundColorAttributeName,
				  titleFont, NSFontAttributeName,
				  shadow, NSShadowAttributeName,
				  paraStyle, NSParagraphStyleAttributeName,
				  nil];
	
	headerRect = NSInsetRect(cellFrame, 8, 0);
	headerRect.origin.y += 1;
	
	header = [[NSAttributedString alloc] initWithString:[self title] attributes:attributes];
	[header drawInRect:headerRect];
		
	int columnIndex = [(NSTableHeaderView*)controlView columnAtPoint:cellFrame.origin];
	if(columnIndex < 0) return;
	
	NSTableColumn *column = [[tableView tableColumns] objectAtIndex:columnIndex];
	
	if([[tableView sortDescriptors] count]==0)
		return;
	
	
	NSSortDescriptor *sortDesc = [[tableView sortDescriptors] objectAtIndex:0];
	
    NSInteger priority = [[sortDesc key] isEqualToString:[column identifier]];
	BOOL ascending = [sortDesc ascending];
	
	
	NSRect sortIndicatorRect = NSMakeRect(cellFrame.origin.x + cellFrame.size.width - 5 - 15, roundf(cellFrame.origin.y + (cellFrame.size.height-14)/2)-1, 15, 14);
	[self drawSortIndicatorWithFrame:sortIndicatorRect inView:controlView ascending:ascending priority:priority];
}

- (void)drawSortIndicatorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView ascending:(BOOL)ascending priority:(NSInteger)priority{
	if(priority != 1) return;
	
	NSImage *sortindicatorImage = [NSImage imageNamed:@"sort_arrow_inactive"];
	BOOL isPressed = [self state];
	if(isPressed)
		sortindicatorImage = [NSImage imageNamed:@"sort_arrow_pressed"];
	
	[sortindicatorImage drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:!ascending hints:nil];
	
	
}

@end