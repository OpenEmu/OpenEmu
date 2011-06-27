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
		
		[self updateTrackingAreas];
    }
    return self;
}
- (void)updateTrackingAreas{
	while([[self trackingAreas] count]>0){
		[self removeTrackingArea:[[self trackingAreas] lastObject]];
	}
	
	NSCell* cell = [self cell];
	if(cell && [cell isKindOfClass:[OEImageButtonCell class]] && [(OEImageButtonCell*)cell displaysHover]){
		[self addTrackingArea:[[[NSTrackingArea alloc] initWithRect:self.bounds options:NSTrackingActiveInActiveApp|NSTrackingMouseEnteredAndExited owner:self userInfo:nil] autorelease]];
	}
}

- (void)windowChanged:(id)sender{
	[self setNeedsDisplay:YES];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeMainNotification object:[self window]];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignMainNotification object:[self window]];
	
    [super dealloc];
}

- (void)mouseEntered:(NSEvent *)theEvent{
	[self setNeedsDisplay:TRUE];
}
- (void)mouseExited:(NSEvent *)theEvent{
	[self setNeedsDisplay:TRUE];
}

- (void)setCell:(NSCell *)aCell{
	[super setCell:aCell];
	[self updateTrackingAreas];
}
@end
#pragma mark -
@implementation OEImageButtonCell
- (BOOL)displaysHover{
	return NO;
}
#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
	if(!image) return;
	
	BOOL windowActive = [[controlView window] isMainWindow] || ([[controlView window] parentWindow] && [[[controlView window] parentWindow] isMainWindow]);
	BOOL isPressed = [self isHighlighted];
	BOOL isEnabled = [self isEnabled];
	BOOL isSelected = [self state]==NSOnState;
	
	NSPoint p = [controlView convertPointFromBase:[[controlView window] convertScreenToBase:[NSEvent mouseLocation]]];
	BOOL rollover = NSPointInRect(p, controlView.frame);
	OEButtonState buttonState;
	if(isSelected){
		if(!isEnabled){
			buttonState = OEButtonStateSelectedDisabled;
		} else if(!windowActive){
			buttonState = OEButtonStateSelectedInactive;			
		} else if(isPressed){
			buttonState = OEButtonStateSelectedPressed;
		} else if(rollover){
			buttonState = OEButtonStateSelectedHover;
		} else {
			buttonState = OEButtonStateSelectedNormal;
		}
	} else {
		if(!isEnabled){
			buttonState = OEButtonStateUnselectedDisabled;
		} else if(!windowActive){
			buttonState = OEButtonStateUnselectedInactive;			
		} else if(isPressed){
			buttonState = OEButtonStateUnselectedPressed;
		} else if(rollover){
			buttonState = OEButtonStateUnselectedHover;
		} else {
			buttonState = OEButtonStateUnselectedNormal;
		}
	}
		
	NSRect sourceRect = [self imageRectForButtonState:buttonState];
	NSRect targetRect = cellFrame;
	[self.image drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:0 rightBorder:0 topBorder:0 bottomBorder:0];
}

- (NSRect)imageRectForButtonState:(OEButtonState)state{
	return NSZeroRect;
}
@synthesize image;
@end

@implementation OEToolbarButtonPushCell
- (BOOL)displaysHover{
	return NO;
}

- (NSRect)imageRectForButtonState:(OEButtonState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width/3, [self.image size].height);
	switch (state) {
		case OEButtonStateSelectedInactive:
		case OEButtonStateUnselectedInactive:
		case OEButtonStateSelectedDisabled:
		case OEButtonStateUnselectedDisabled:
			rect.origin.x = 0;
			break;
		case OEButtonStateSelectedHover:
		case OEButtonStateUnselectedHover:
		case OEButtonStateSelectedNormal:
		case OEButtonStateUnselectedNormal:
			rect.origin.x += rect.size.width;
			break;
		case OEButtonStateSelectedPressed:
		case OEButtonStateUnselectedPressed:
			rect.origin.x += 2*rect.size.width;
		default:
			break;
	}
	
	return rect;
}
@end

@implementation OEToolbarButtonSelectableCell
- (BOOL)displaysHover{
	return NO;
}

- (NSRect)imageRectForButtonState:(OEButtonState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width, [self.image size].height/5);
	switch (state) {
		case OEButtonStateSelectedDisabled:
		case OEButtonStateSelectedInactive:
			rect.origin.y = 69;
			break;
		case OEButtonStateUnselectedDisabled:
		case OEButtonStateUnselectedInactive:
			rect.origin.y = 92;
			break;
		case OEButtonStateUnselectedHover:
		case OEButtonStateUnselectedNormal:
			rect.origin.y = 46;
			break;
		case OEButtonStateSelectedHover:
		case OEButtonStateSelectedNormal:
			rect.origin.y = 23;
			break;
		case OEButtonStateSelectedPressed:
		case OEButtonStateUnselectedPressed:
			rect.origin.y = 0;
			break;
		default:
			break;
	}
	return rect;
}

@end

@implementation OEImageButtonHoverPressed
- (BOOL)displaysHover{
	return YES;
}
- (NSRect)imageRectForButtonState:(OEButtonState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width/3, [self.image size].height);
	switch (state) {
		case OEButtonStateSelectedHover:
		case OEButtonStateUnselectedHover:
			rect.origin.x = 28;
			break;
		case OEButtonStateSelectedPressed:
		case OEButtonStateUnselectedPressed:
			rect.origin.x += 2*rect.size.width;
			break;
		default:
			rect.origin.x = 0;
			break;
	}
	return rect;
}
@end

@implementation OEImageButtonHoverSelectable
- (BOOL)displaysHover{
	return YES;
}

- (NSRect)imageRectForButtonState:(OEButtonState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width/3, [self.image size].height/2);
	switch (state) {
		case OEButtonStateSelectedHover:
		case OEButtonStateUnselectedHover:
			rect.origin.x = 28;
			break;
		case OEButtonStateSelectedPressed:
		case OEButtonStateUnselectedPressed:
			rect.origin.x += 2*rect.size.width;
			break;
		default:
			break;
	}
	
	switch (state) {
		case OEButtonStateSelectedInactive:
		case OEButtonStateSelectedDisabled:
		case OEButtonStateSelectedPressed:
		case OEButtonStateSelectedNormal:
		case OEButtonStateSelectedHover:
			rect.origin.y = 0;
			break;
		case OEButtonStateUnselectedHover:
		case OEButtonStateUnselectedNormal:
		case OEButtonStateUnselectedPressed:
		case OEButtonStateUnselectedInactive:
		case OEButtonStateUnselectedDisabled:
			rect.origin.y = rect.size.height;
			break;
		default:
			break;
	}
	return rect;
}
@end

@implementation OEImageButtonPressed

- (BOOL)displaysHover{
	return FALSE;
}

- (NSRect)imageRectForButtonState:(OEButtonState)state{
	NSRect rect = NSMakeRect(0, 0, [self.image size].width, [self.image size].height/2);

	switch (state) {
		case OEButtonStateSelectedInactive:
		case OEButtonStateSelectedDisabled:
		case OEButtonStateSelectedNormal:
		case OEButtonStateSelectedHover:
		case OEButtonStateUnselectedHover:
		case OEButtonStateUnselectedNormal:
		case OEButtonStateUnselectedInactive:
		case OEButtonStateUnselectedDisabled:
			rect.origin.y = 0;
		break;
		case OEButtonStateUnselectedPressed:
		case OEButtonStateSelectedPressed:
			rect.origin.y = rect.size.height;
			break;
		default:
			break;
	}
	return rect;
}

@end