//
//  OESetupAssistantScrollView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantTableView.h"
@interface OESetupAssistantTableView (Private) <NSTableViewDataSource>
- (void)_setup;
@end
@implementation OESetupAssistantTableView

- (id)init
{
    self = [super init];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (void)_setup
{
    [self setSelectionColor:[[NSColor blueColor] colorWithAlphaComponent:0.8]];
    [self setRowHeight:23.0];
    
    for(NSTableColumn *column in [self tableColumns])
    {
        [column setEditable:NO];
    }
    
#warning Only for testing:
    [self setDataSource:self];
}

- (void)dealloc 
{
    [self setSelectionColor:nil];
    
    [super dealloc];
}

#pragma mark -
#pragma mark Drawing
- (void)highlightSelectionInClipRect:(NSRect)theClipRect
{
	BOOL isActive = [[self window] isMainWindow] && [[self window] firstResponder] == self;
	
	NSColor *fillColor;
	if(isActive)
	{
		fillColor = [NSColor colorWithDeviceRed:27/255.0 green:49/255.0 blue:139/255.0 alpha:0.5];
	}
	else
	{
		fillColor = [NSColor colorWithDeviceWhite:0.55 alpha:.5];
	}
	
	[fillColor setFill];
	
	NSIndexSet *selectedRows = [self selectedRowIndexes];
	[selectedRows enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
		NSRect frame = [self rectOfRow:idx];
		NSRectFill(frame);
	}];
}

- (void)drawBackgroundInClipRect:(NSRect)clipRect
{
	NSColor *rowBackground = [NSColor colorWithDeviceWhite:0.0 alpha:0.1];
	NSColor *alternateRowBackground = [NSColor colorWithDeviceWhite:1.0 alpha:0.01];
	
	[rowBackground setFill];    
	NSRect rect = [self visibleRect];
	for(float i=0; i<rect.origin.y+rect.size.height; i+=2*([self rowHeight]+[self intercellSpacing].height))
    {
		NSRect rowRect = NSMakeRect(rect.origin.x, i, rect.size.width, [self rowHeight]+[self intercellSpacing].height);
		NSRectFill(rowRect);
	}
    
	[alternateRowBackground setFill];
	rect = [self visibleRect];
	for(float i=[self rowHeight]+[self intercellSpacing].height; i<rect.origin.y+rect.size.height; i+=2*([self rowHeight]+[self intercellSpacing].height))
    {
		NSRect rowRect = NSMakeRect(rect.origin.x, i, rect.size.width, [self rowHeight]+[self intercellSpacing].height);
		NSRectFill(rowRect);
	}
}

#pragma mark -
#pragma mark Dummy Data Source Implementation
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return 6;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    NSString *identifier = [aTableColumn identifier];
    if([identifier isEqualTo:@"enabled"])
    {
        return [NSNumber numberWithBool:YES];
    }
    else if([identifier isEqualToString:@"emulatorName"])
    {
        switch (rowIndex) 
        {
            case 0:
                return @"BSNES";
                break;
            case 1:
                return @"Genesis Plus";
            case 2:
                return @"Nestopia";
            case 3:
                return @"Snes9X";
            case 4:
                return @"Mupen64";
            case 5:
                return @"VisualBoyAdvance";
            default:
                break;
        }
    }
    else if([identifier isEqualToString:@"emulatorSystem"])
    {
        switch (rowIndex) 
        {
            case 0:
                return @"Super Nintendo (SNES) Emulator";
                break;
            case 1:
                return @"Sega Genesis Emulator";
            case 2:
                return @"Nintendo (NES) Emulator";
            case 3:
                return @"Super Nintendo (SNES) Emulator";
            case 4:
                return @"Nintendo 64 Emulator";
            case 5:
                return @"GameBoy Advance Emulator";
            default:
                break;
        }
    }
    return nil;
}
#pragma mark -
@synthesize selectionColor;
@end

@implementation OESetupAssistantMajorTextCell

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSSize contentSize = [self cellSize];
	
    cellFrame.origin.y += (cellFrame.size.height - contentSize.height) / 2.0;
    cellFrame.size.height = contentSize.height;
	
	cellFrame = NSInsetRect(cellFrame, 3, 0);
	cellFrame.origin.y += 2;
    
	NSString *val = [self stringValue];
	NSAttributedString *drawString = [[NSAttributedString alloc] initWithString:val attributes:[self attributes]];
    
	[drawString drawInRect:cellFrame];
    
    [drawString release];
}

- (NSDictionary*)attributes
{
	NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:0 size:11.5];
	
	NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
	[style setAlignment:NSLeftTextAlignment];
	[attributes setObject:style forKey:NSParagraphStyleAttributeName];
	[style release];
    
    return [attributes autorelease];
}

@end

@implementation OESetupAssistantMinorTextCell
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSSize contentSize = [self cellSize];
	
    cellFrame.origin.y += (cellFrame.size.height - contentSize.height) / 2.0;
    cellFrame.size.height = contentSize.height;
	
	cellFrame = NSInsetRect(cellFrame, 3, 0);
	cellFrame.origin.y += 2;
	
	NSString *val = [self stringValue];
	NSAttributedString *drawString = [[NSAttributedString alloc] initWithString:val attributes:[self attributes]];
    
	[drawString drawInRect:cellFrame];
    
    [drawString release];
}

- (NSDictionary*)attributes
{
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
	
	NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0];
	
	NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.86 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
	[style setAlignment:NSLeftTextAlignment];
	[attributes setObject:style forKey:NSParagraphStyleAttributeName];
	[style release];
    
    return [attributes autorelease];
}

@end