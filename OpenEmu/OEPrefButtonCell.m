//
//  PrefButtonCell.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEPrefButtonCell

- (id)init
{
    self = [super init];
    if (self) 
    {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEPrefButtonCell class])
        return;

	NSImage* image = [NSImage imageNamed:@"dark_button"];
	
	[image setName:@"dark_button_normal" forSubimageInRect:NSMakeRect(0, image.size.height/2, image.size.width, image.size.height/2)];
	[image setName:@"dark_button_pressed" forSubimageInRect:NSMakeRect(0, 0, image.size.width, image.size.height/2)];
}
#pragma mark -

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
	NSRect cellFrame = frame;
	cellFrame.size.height = 23;
	
	NSImage* img = [self isHighlighted] ? [NSImage imageNamed:@"dark_button_pressed"] : [NSImage imageNamed:@"dark_button_normal"];
	[img drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:10 rightBorder:10 topBorder:0 bottomBorder:0];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
	NSRect titleRect = frame;
	titleRect.origin.x += (titleRect.size.width-title.size.width)/2;
	titleRect.size.width = title.size.width;
    titleRect.origin.y -= 2;
	
	[title drawInRect:titleRect];
	return titleRect;
}
#pragma mark -
- (NSAttributedString*)attributedTitle
{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0.0 size:11.0];
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	if([self isHighlighted])
    {
		[attributes setObject:[NSColor colorWithDeviceWhite:0.74 alpha:1.0] forKey:NSForegroundColorAttributeName];	
	}
    else
    {
		[attributes setObject:[NSColor colorWithDeviceWhite:0.89 alpha:1.0] forKey:NSForegroundColorAttributeName];
        
	}
    
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
    
	return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}
@end
