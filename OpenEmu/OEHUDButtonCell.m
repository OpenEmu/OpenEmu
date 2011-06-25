//
//  GameViewButton.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEHUDButtonCell.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEHUDButtonCell
@synthesize blue;

- (id)init
{
    self = [super init];
    if (self) {
		[self setBordered:YES];
		[self setFocusRingType:NSFocusRingTypeNone];
		[self setBezeled:NO];
		[self setBackgroundColor:nil];
		[self setButtonType:NSMomentaryPushInButton];
		[self setBezelStyle:NSSmallSquareBezelStyle];
    }
    
    return self;
}
- (BOOL)isOpaque{
	return NO;
}

- (void)dealloc
{
    [super dealloc];
}

+ (void)initialize{
	NSImage* image = [NSImage imageNamed:@"hud_button"];
	
	[image setName:@"hud_button_pressed" forSubimageInRect:NSMakeRect(0, 0, image.size.width, image.size.height/3)];
	[image setName:@"hud_button_blue" forSubimageInRect:NSMakeRect(0, image.size.height/3, image.size.width, image.size.height/3)];
	[image setName:@"hud_button_normal" forSubimageInRect:NSMakeRect(0, 2*image.size.height/3, image.size.width, image.size.height/3)];
}
#pragma mark -
- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView{
	NSRect cellFrame = frame;
	cellFrame.size.height = 23;
	
	NSImage* img;
	if([self isHighlighted]){
		img = [NSImage imageNamed:@"hud_button_pressed"];
	} else {
		img = [self isBlue] ? [NSImage imageNamed:@"hud_button_blue"] : [NSImage imageNamed:@"hud_button_normal"];
	}

	[img drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:4 rightBorder:4 topBorder:0 bottomBorder:0];
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView{

	if([self isHighlighted]){
		image = [self alternateImage];
	} else {
		image = [self image];
	}
	
	[super drawImage:image withFrame:frame inView:controlView];
}


- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView{
	NSRect titleRect = frame;
	
	titleRect.origin.y -= 1;
	
	[title drawInRect:titleRect];
	return titleRect;
}
#pragma mark -
- (NSAttributedString*)attributedTitle{
	NSMutableDictionary *attributes = [[[NSMutableDictionary alloc] init] autorelease];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:3.0 size:11.0];
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowBlurRadius:1.0];
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setAlignment:NSCenterTextAlignment];
	
	if([self isHighlighted]){
		[attributes setObject:[NSColor colorWithDeviceWhite:0.03 alpha:1.0] forKey:NSForegroundColorAttributeName];	
		[shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.4]];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
	} else {
		[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:1.0]];
		[attributes setObject:[NSColor colorWithDeviceWhite:0.91 alpha:1.0] forKey:NSForegroundColorAttributeName];
		[shadow setShadowOffset:NSMakeSize(0, 1)];
	}
	
	[attributes setObject:ps forKey:NSParagraphStyleAttributeName];
	[attributes setObject:font forKey:NSFontAttributeName];
	[attributes setObject:shadow forKey:NSShadowAttributeName];
	
	return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
}

@end

