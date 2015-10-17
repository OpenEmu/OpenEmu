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

#import "OESetupAssistantTableView.h"

@interface OESetupAssistantTableView () <NSTableViewDataSource>
- (void)OE_commonSetupAssistantTableViewInit;
@end

@implementation OESetupAssistantTableView
@synthesize selectionColor;

- (id)init
{
    self = [super init];
    if (self) 
    {
        [self OE_commonSetupAssistantTableViewInit];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self OE_commonSetupAssistantTableViewInit];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self OE_commonSetupAssistantTableViewInit];
    }
    return self;
}

- (void)OE_commonSetupAssistantTableViewInit
{
    [self setSelectionColor:[[NSColor blueColor] colorWithAlphaComponent:0.8]];
    [self setRowHeight:23.0];
    for(NSTableColumn *column in [self tableColumns])
    {
        [column setEditable:YES];
        
        //if([[column identifier] isEqualToString:@"enabled"])
        //    [column setEditable:YES];
        //else
        //    [column setEditable:NO];
    }
}

#pragma mark -
#pragma mark Drawing
- (BOOL)isOpaque
{
    return NO;
}

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
    [[NSColor clearColor] setFill];
    NSRectFill(clipRect);
    
	NSColor *rowBackground          = [NSColor colorWithDeviceWhite:0.0 alpha:0.1];
	NSColor *alternateRowBackground = [NSColor colorWithDeviceWhite:1.0 alpha:0.01];
	
	[rowBackground setFill];
    
    CGFloat rowHeight = [self rowHeight] + [self intercellSpacing].height;
    
	NSRect rect = [self bounds];
    
    for(CGFloat i = 0; i < NSMaxY(rect); i += 2 * rowHeight)
		NSRectFill(NSMakeRect(rect.origin.x, i, rect.size.width, rowHeight));
    
	[alternateRowBackground setFill];
	rect = [self bounds];
    
	for(CGFloat i = rowHeight; i < NSMaxY(rect); i += 2 * rowHeight)
		NSRectFill(NSMakeRect(rect.origin.x, i, rect.size.width, rowHeight));
}

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
}

- (NSDictionary *)attributes
{
	NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
	
    NSFont *font = [NSFont boldSystemFontOfSize:11.5];
	
	NSShadow *shadow = [[NSShadow alloc] init];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
	[style setAlignment:NSLeftTextAlignment];
	[attributes setObject:style forKey:NSParagraphStyleAttributeName];
    
    return attributes;
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
}

- (NSDictionary *)attributes
{
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
	
    NSFont *font = [NSFont systemFontOfSize:11];
	
	NSShadow *shadow = [[NSShadow alloc] init];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.86 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
	[style setAlignment:NSLeftTextAlignment];
	[attributes setObject:style forKey:NSParagraphStyleAttributeName];
    
    return attributes;
}

@end
