//
//  OEToolbarButton.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 16.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEImageButton.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEImageButton

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChanged:) name:NSWindowDidBecomeMainNotification object:[self window]];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChanged:) name:NSWindowDidResignMainNotification object:[self window]];
    }
    return self;
}

- (void)windowChanged:(id)sender{
	[self setNeedsDisplay:YES];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeMainNotification object:[self window]];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignMainNotification object:[self window]];
	
    [super dealloc];
}
@end
#pragma mark -
@implementation OEImageButtonCell

#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
	if(!image) return;
	
	BOOL windowActive = [[controlView window] isMainWindow];
	BOOL isPressed = [self isHighlighted];
	BOOL isEnabled = [self isEnabled];
	BOOL isSelected = [self state]==NSOnState;
	
	OEUIState state;
	if(isEnabled && windowActive){
		state = isSelected? OEUIStateFocusedSelected:OEUIStateFocusedUnselected;
		
		if(isPressed)
			state = OEUIStatePressed;

	} else {
		state = isSelected? OEUIStateUnfocusedSelected:OEUIStateUnfocusedUnselected;
	}
	
	NSRect sourceRect = [self imageRectForState:state];
	NSRect targetRect = cellFrame;
	[self.image drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:0 rightBorder:0 topBorder:0 bottomBorder:0];
}

- (NSRect)imageRectForState:(OEUIState)state{
	return NSZeroRect;
}
@synthesize image;
@end

@implementation OEToolbarButtonPushCell
- (NSRect)imageRectForState:(OEUIState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width/3, [self.image size].height);
	switch (state) {
		case OEUIStateUnfocusedSelected:
		case OEUIStateUnfocusedUnselected:
			rect.origin.x = 0;
			break;
		case OEUIStateFocusedSelected:
		case OEUIStateFocusedUnselected:
			rect.origin.x += rect.size.width;
			break;
		case OEUIStatePressed:
			rect.origin.x += 2*rect.size.width;
		default:
			break;
	}
	
	return rect;
}
@end

@implementation OEToolbarButtonSelectableCell
- (NSRect)imageRectForState:(OEUIState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width, [self.image size].height/5);
	switch (state) {
		case OEUIStateUnfocusedSelected:
			rect.origin.y = 69;
			break;
		case OEUIStateUnfocusedUnselected:
			rect.origin.y = 92;
			break;
		case OEUIStateFocusedSelected:
			rect.origin.y = 23;
			break;
		case OEUIStateFocusedUnselected:
			rect.origin.y = 46;
			break;
		case OEUIStatePressed:
			rect.origin.y = 0;
			break;
		default:
			break;
	}
	return rect;
}
@end
