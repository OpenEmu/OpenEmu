//
//  OEPopUpButtonCell.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPopUpButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEPopUpButtonCell

- (id)init{
    self = [super init];
    if (self) {
    }
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}

+ (void)initialize{
	NSImage* image = [NSImage imageNamed:@"PopUpButton"];
	
	[image setName:@"popupbutton" forSubimageInRect:NSMakeRect(0, image.size.height/2, image.size.width, image.size.height/2)];
	[image setName:@"popupbutton_pressed" forSubimageInRect:NSMakeRect(0, 0, image.size.width, image.size.height/2)];
}

- (NSAttributedString *)attributedTitle{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:4.0 size:11.0];
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	[attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	
	
	return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}

#pragma mark -
- (void)drawTitleWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
	cellFrame.size.height -= 3;
	cellFrame.origin.y += 1;
	
	cellFrame.origin.x -= 3;
	cellFrame.size.width += 3;
	
	[super drawTitleWithFrame:cellFrame inView:controlView];
}

- (void)drawBorderAndBackgroundWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
	cellFrame.size.height = 23;
	
	NSImage* img = [NSImage imageNamed:@"popupbutton"];
	[img drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:9 rightBorder:21 topBorder:0 bottomBorder:0];
}
@end
