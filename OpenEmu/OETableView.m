//
//  OETableView.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OETableView.h"

#import "OETableCornerView.h"
#import "OETableHeaderCell.h"
@implementation OETableView
static NSColor* cellEditingFillColor, *textColor, *cellSelectedTextColor, *strokeColor;
static NSGradient* highlightGradient, *normalGradient;

- (id)initWithCoder:(NSCoder *)aDecoder{
    self = [super initWithCoder:aDecoder];
    if (self) {
		if(!cellEditingFillColor){
			cellEditingFillColor = [[NSColor greenColor] retain];
		}
		
		if(!textColor){
			textColor = [[NSColor whiteColor] retain];
		}
		
		if(!cellSelectedTextColor){
			cellSelectedTextColor = [[NSColor whiteColor] retain];
		}
		
		if(!highlightGradient){
			highlightGradient = [[NSGradient alloc] initWithStartingColor:[NSColor greenColor] endingColor:[NSColor magentaColor]];
		}
		
		if(!strokeColor){
			strokeColor = [[NSColor blackColor] retain];
		}
		
		if(!normalGradient){
			NSColor* c1 = [NSColor colorWithDeviceWhite:0.29 alpha:1.0];
			NSColor* c2 = [NSColor colorWithDeviceWhite:0.18 alpha:1.0];
			normalGradient = [[NSGradient alloc] initWithStartingColor:c1 endingColor:c2];
		}
		
		self.selectionColor = [NSColor colorWithDeviceRed:0.173 green:0.286 blue:0.976 alpha:1.0];
		[self setIntercellSpacing:NSMakeSize(1, 0)];
		
		[self setBackgroundColor:[NSColor blackColor]];
		[self setFocusRingType:NSFocusRingTypeNone];
		
		[self setRowHeight:17];

		for (NSTableColumn* aColumn in [self tableColumns]) {
			OETableHeaderCell *newHeader = [[OETableHeaderCell alloc] initTextCell:[[aColumn headerCell] stringValue]];
			[newHeader setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:11]];
			[aColumn setHeaderCell: newHeader];
			[newHeader release];
		}
	}
    
    return self;
}
- (void)awakeFromNib{
	[self setCornerView:[[[OETableCornerView alloc] init] autorelease]];
}

- (void)drawBackgroundInClipRect:(NSRect)clipRect{
	NSColor* rowBackground = [NSColor colorWithDeviceWhite:0.059 alpha:1.0];
	NSColor* alternateRowBackground = [NSColor colorWithDeviceWhite:0.114 alpha:1.0];
	
	[rowBackground setFill];
	NSRectFill(clipRect);
	
	[alternateRowBackground setFill];
	
	NSRect rect = [self visibleRect];
	for(float i=[self rowHeight]+[self intercellSpacing].height; i<rect.origin.y+rect.size.height; i+=2*([self rowHeight]+[self intercellSpacing].height)){
		NSRect rowRect = NSMakeRect(rect.origin.x, i, rect.size.width, [self rowHeight]+[self intercellSpacing].height);
		NSRectFill(rowRect);
	}	
}
- (void)highlightSelectionInClipRect:(NSRect)theClipRect{
	BOOL isActive = [[self window] isMainWindow] && [[self window] firstResponder] == self;
	
	NSColor* fillColor;
	NSColor* lineColor;
	if(isActive){
		fillColor = self.selectionColor;
		lineColor = [NSColor colorWithDeviceRed:0.114 green:0.188 blue:0.635 alpha:1.0];
	} else {
		fillColor = [NSColor colorWithDeviceWhite:0.55 alpha:1.0];
		lineColor = [NSColor colorWithDeviceWhite:0.35 alpha:1.0];
	}
	
	[fillColor setFill];
	
	NSIndexSet* selectedRows = [self selectedRowIndexes];
	
	NSUInteger nextIndex = [selectedRows firstIndex];
	NSUInteger currentIndex = [selectedRows firstIndex];
	while(currentIndex!=NSNotFound){
		NSRect frame = [self rectOfRow:nextIndex];
		NSRectFill(frame);
		
		nextIndex = [selectedRows indexGreaterThanIndex:currentIndex];
		
		if(nextIndex == currentIndex+1){
			[lineColor setFill];
			frame.origin.y += frame.size.height-1;
			frame.size.height = 1;
			
			NSRectFill(frame);
			[fillColor setFill];
		}
		
		currentIndex = nextIndex;
	}
}

- (void)drawGridInClipRect:(NSRect)aRect{
	NSSize gridSize = NSMakeSize(1, [self bounds].size.height);
	
	[[NSColor colorWithDeviceRed:0.255 green:0.251 blue:0.255 alpha:1.0] setFill];
	
	NSRect fillRect;
	fillRect.size = gridSize;
	fillRect.origin = aRect.origin;
	
	
	for(NSUInteger i=0; i < [[self tableColumns] count]; i++){
		NSRect colRect = [self rectOfColumn:i];
		fillRect.origin.x = colRect.origin.x + colRect.size.width-1;
		NSRectFill(fillRect);
	}
	
	
	NSColor* fillColor;
	
	BOOL isActive = [[self window] isMainWindow] && [[self window] firstResponder] == self;
	if(isActive){
		fillColor = [NSColor colorWithDeviceRed:0.235 green:0.455 blue:0.769 alpha:1.0];
	} else {
		fillColor = [NSColor colorWithDeviceWhite:0.33 alpha:1.0];
	}	
	
	[fillColor setFill];
	
	NSIndexSet* selectedRows = [self selectedRowIndexes];
	
	NSUInteger nextIndex;
	NSUInteger currentIndex = [selectedRows firstIndex];
	while(currentIndex!=NSNotFound){
		NSRect rowfillRect = [self rectOfRow:currentIndex];
		rowfillRect.size.width = 1;
		
		nextIndex = [selectedRows indexGreaterThanIndex:currentIndex];
		
		if(nextIndex == currentIndex+1)
			rowfillRect.size.height -= 1;
		
		for(NSUInteger i=0; i < [[self tableColumns] count]; i++){
			NSRect colRect = [self rectOfColumn:i];
			
			rowfillRect.origin.x = colRect.origin.x + colRect.size.width-1;
			NSRectFill(rowfillRect);			
		}
		
		currentIndex = nextIndex;
	}
}

- (void)dealloc{
    [super dealloc];
}

@synthesize selectionColor;
@end
