// ProgressCell.m

#import "OEDownloadCell.h"
#import "OEDownload.h"

@implementation OEDownloadCell

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
	OEDownload* data = [self objectValue];
    BOOL elementDisabled    = NO;    
	NSColor *primaryColor = ([self isHighlighted]
                             ? [NSColor alternateSelectedControlTextColor]
                             : (elementDisabled
                                ? [NSColor disabledControlTextColor]
                                : [NSColor textColor]));
    NSString *primaryText = [data name];

	NSDictionary *primaryTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                           primaryColor,                 NSForegroundColorAttributeName,
                                           [NSFont systemFontOfSize:13], NSFontAttributeName,
                                           nil];
    
    [primaryText drawAtPoint:NSMakePoint(cellFrame.origin.x+32, cellFrame.origin.y+0)
              withAttributes:primaryTextAttributes];
	
	if( !data.downloading )
	{
		NSButton* button = data.button;
		
		[controlView addSubview:button];
		[button setFrame:NSMakeRect(cellFrame.origin.x + 14,
									cellFrame.origin.y + cellFrame.size.height / 2 - 7,
									14, 14)];
	}
	else
	{
		[data.button removeFromSuperview];
		NSProgressIndicator* progressIndicator = data.progressBar;
		
		[controlView addSubview:progressIndicator];
		[progressIndicator setFocusRingType:NSFocusRingTypeNone];
		[progressIndicator setDoubleValue:data.progress];
		[progressIndicator setFrame:NSMakeRect(cellFrame.origin.x + 32,
											   cellFrame.origin.y + cellFrame.size.height / 2,
											   cellFrame.size.width - 48, NSProgressIndicatorPreferredThickness)];
	}
}

@end