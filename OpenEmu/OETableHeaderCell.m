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

#import "OETableHeaderCell.h"

#import "OEUIDrawingUtils.h"
#import "OETheme.h"
#pragma mark - Private variables

static const NSSize  _OESortIndicatorSize   = {15, 14};
static const CGFloat _OESortIndicatorMargin = 5;

#pragma mark -

@implementation OETableHeaderCell

#pragma mark -

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSTableHeaderView *tableHeaderView = (NSTableHeaderView *)controlView;
	NSTableView *tableView             = [tableHeaderView tableView];
    NSArray *tableColumns              = [tableView tableColumns];
    const NSUInteger columnCount       = [tableColumns count];
    NSInteger visibleColumnIndex       = [tableHeaderView columnAtPoint:cellFrame.origin];
    NSTableColumn *tableColumn         = (visibleColumnIndex == -1 ? nil : [tableColumns objectAtIndex:visibleColumnIndex]);
    NSUInteger visibleColumnCount      = columnCount;

    // Normalise column index & count based on table column visibility
    for(NSInteger i = 0; i < columnCount; i++)
    {
        if([[tableColumns objectAtIndex:i] isHidden])
        {
            visibleColumnCount--;
            if(visibleColumnIndex > i)
                visibleColumnIndex--;
        }
    }

    const BOOL isOutOfBoundsColumn   = (visibleColumnIndex == -1);
    const BOOL isFirstColumn         = (visibleColumnIndex == 0);
	const BOOL isPressed             = [self state] && [self isClickable];

    // Draw background and border
	const BOOL hideLeftHighlight     = isPressed || isFirstColumn || isOutOfBoundsColumn;
    OEThemeState sate = isPressed ? OEThemeInputStatePressed : OEThemeStateDefault;

	NSImage *backgroundImage = [[OETheme sharedTheme] imageForKey:@"table_header_background" forState:sate];
    [backgroundImage setMatchesOnlyOnBestFittingAxis:YES];
    [backgroundImage drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];

    // Draw highlight on left edge
    if(!hideLeftHighlight)
    {
        NSRect leftHighlightRect = cellFrame;
        leftHighlightRect.size.width = 1;

        [[NSColor colorWithDeviceWhite:1.0 alpha:0.04] setFill];
        NSRectFillUsingOperation(leftHighlightRect, NSCompositeSourceOver);
    }

    if(isOutOfBoundsColumn)
        return;

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

    // Draw glow and title
    NSSortDescriptor *sortDescriptor = ([[tableView sortDescriptors] count] > 0 ?
                                        [[tableView sortDescriptors] objectAtIndex:0] :
                                        nil);
    const NSInteger priority         = ([[sortDescriptor key] isEqualToString:[[tableColumn sortDescriptorPrototype] key]] ? 1 : 0);


    NSFont *titleFont = [NSFont boldSystemFontOfSize:11];
	NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
	[paraStyle setLineBreakMode:NSLineBreakByTruncatingTail];
    [paraStyle setAlignment:[self alignment]];

	NSShadow *shadow = [[NSShadow alloc] init];
	[shadow setShadowColor:[NSColor blackColor]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	[shadow setShadowBlurRadius:0];

	NSDictionary *attributes;
	NSAttributedString *header;
	NSRect headerRect;

	NSColor *textColor = [NSColor colorWithDeviceWhite:.85 alpha:1];

    headerRect = NSInsetRect(cellFrame, 8, 0);
    headerRect.origin.y   += 1;
    headerRect.size.width -= (priority * _OESortIndicatorSize.width);

	// Draw glow if header is pressed
	if(isPressed)
    {
		textColor = [NSColor whiteColor];

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


	header = [[NSAttributedString alloc] initWithString:[self title] attributes:attributes];
	[header drawInRect:headerRect];

    // Draw sort indicator
    if(priority != 0)
    {
        const NSRect sortIndicatorRect =
        {
            .origin.x = cellFrame.origin.x + cellFrame.size.width - _OESortIndicatorMargin - _OESortIndicatorSize.width,
            .origin.y = roundf(cellFrame.origin.y + (cellFrame.size.height - _OESortIndicatorSize.height) / 2) - 1,
            .size     = _OESortIndicatorSize
        };
        [self drawSortIndicatorWithFrame:sortIndicatorRect inView:controlView ascending:[sortDescriptor ascending] priority:priority];
    }
}

- (void)drawSortIndicatorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView ascending:(BOOL)ascending priority:(NSInteger)priority
{
	if(priority != 1) return;

    OEThemeState state = [self state] ? OEThemeInputStatePressed : OEThemeStateDefault;

    NSImage *image = [[OETheme sharedTheme] imageForKey:@"sort_arrow" forState:state];
    [image drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:!ascending hints:nil];
}

@end
