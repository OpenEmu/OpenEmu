//
//  OENSMenuView.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEMenuView.h"

#import "OEMenu.h"
#import "NSImage+OEDrawingAdditions.h"
#define menuItemSpacingTop 8 + (imageIncluded ? 1 : 0)
#define menuItemSpacingBottom 9 + (imageIncluded ? 1 : 0)
#define menuItemSpacingLeft 13 + (imageIncluded ? 13 : 0)
#define menuItemImageWidth 16
#define menuItemImageTitleSpacing 6
#define menuItemSpacingRight 17 - (imageIncluded ? 1 : 0)

#define menuItemHeightNoImage 17
#define menuItemHeightImage 20
#define menuItemHeight (imageIncluded ? menuItemHeightImage : menuItemHeightNoImage)

@interface OEMenuView (Private)
- (void)highlightItemAtPoint:(NSPoint)p;
@end
@implementation OEMenuView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		NSTrackingArea* area = [[NSTrackingArea alloc] initWithRect:self.bounds options:NSTrackingMouseMoved|NSTrackingMouseEnteredAndExited|NSTrackingActiveInActiveApp owner:self userInfo:nil];
        [self addTrackingArea:area];
		[area release];
	}
    return self;
}

- (void)dealloc{
	while([[self trackingAreas] count] != 0){
		[self removeTrackingArea:[[self trackingAreas] lastObject]];		
	}
	
    [super dealloc];
}

#pragma mark -
#pragma mark TextAttributes
- (NSDictionary*)itemTextAttributes{
	NSMutableDictionary* dict = [NSMutableDictionary dictionary];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
	NSColor* textColor = [NSColor whiteColor];
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setLineBreakMode:NSLineBreakByTruncatingTail];
	
	[dict setObject:font forKey:NSFontAttributeName];
	[dict setObject:textColor forKey:NSForegroundColorAttributeName];
	[dict setObject:ps forKey:NSParagraphStyleAttributeName];
	
	return dict;
}

- (NSDictionary*)selectedItemTextAttributes{
	NSMutableDictionary* dict = [NSMutableDictionary dictionary];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
	NSColor* textColor = [NSColor blackColor];
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setLineBreakMode:NSLineBreakByTruncatingTail];
	
	[dict setObject:font forKey:NSFontAttributeName];
	[dict setObject:textColor forKey:NSForegroundColorAttributeName];
	[dict setObject:ps forKey:NSParagraphStyleAttributeName];
	
	return dict;
}

- (NSDictionary*)disabledItemTextAttributes{
	NSMutableDictionary* dict = [NSMutableDictionary dictionary];
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:8.0 size:10.0];
	NSColor* textColor = [NSColor colorWithDeviceRed:0.49 green:0.49 blue:0.49 alpha:1.0];
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setLineBreakMode:NSLineBreakByTruncatingTail];
	
	[dict setObject:font forKey:NSFontAttributeName];
	[dict setObject:textColor forKey:NSForegroundColorAttributeName];
	[dict setObject:ps forKey:NSParagraphStyleAttributeName];
	
	return dict;
}
#pragma mark -
#pragma mark Drawing
- (void)updateView{
	NSArray* items = [self.menu itemArray];
		
	float width = 0;
		
	imageIncluded = NO;
	NSDictionary* attributes = [self itemTextAttributes];
	for(NSMenuItem* menuItem in items){
		NSAttributedString* attributedTitle = [[NSAttributedString alloc] initWithString:menuItem.title attributes:attributes];
		width = width < attributedTitle.size.width ? attributedTitle.size.width : width;
		[attributedTitle release];
		
		if(menuItem.image!=nil){
			imageIncluded = YES;
		}
	}
		
	float height = menuItemHeight*[items count] + menuItemSpacingTop+menuItemSpacingBottom;
	
	width += menuItemSpacingLeft + menuItemSpacingRight;
	width += imageIncluded ? menuItemImageWidth+menuItemImageTitleSpacing : 0 ;
	
	width = width < self.menu.minSize.width ? self.menu.minSize.width : width;
	width = width > self.menu.maxSize.width ? self.menu.maxSize.width : width;
	
	height = height < self.menu.minSize.height ? self.menu.minSize.height : height;
	height = height > self.menu.maxSize.height ? self.menu.maxSize.height : height;
	
	NSRect winFrame = self.menu.frame;
	winFrame.size = NSMakeSize(width, height);
	
	[self.menu setFrame:winFrame display:YES];
	[self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect{

	NSColor* startColor = [NSColor colorWithDeviceWhite:0.91 alpha:0.10];
	NSColor* endColor = [startColor colorWithAlphaComponent:0.0];
	NSGradient* grad = [[[NSGradient alloc] initWithStartingColor:startColor endingColor:endColor] autorelease];
	
	NSRect backgroundRect = NSInsetRect(self.bounds, 4, 4);
	NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect:backgroundRect xRadius:3 yRadius:3];
	
	// Draw Background color
	NSColor* backgroundColor = [NSColor colorWithDeviceWhite:0.0 alpha:0.8];
	[[[[NSGradient alloc] initWithStartingColor:backgroundColor endingColor:backgroundColor] autorelease] drawInBezierPath:path angle:90];
	
	// draw gradient above
	[grad drawInBezierPath:path angle:90];
	
	// draw background border
	NSImage* img = [self.menu supermenu]==nil ? [NSImage imageNamed:@"menu_body"] : [NSImage imageNamed:@"menu_body_submenu"];
	[img drawInRect:self.bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:17 rightBorder:17 topBorder:19 bottomBorder:19];
	
	
	NSArray* items = [self.menu itemArray];
	float y = menuItemSpacingTop;
	for(NSMenuItem* menuItem in items){
		NSRect itemRect = NSMakeRect(menuItemSpacingLeft, y, self.frame.size.width-menuItemSpacingLeft-menuItemSpacingRight, menuItemHeight);
		NSRect menuItemFrame = NSMakeRect(5, y, self.frame.size.width-5-5, menuItemHeight);
	
		
		BOOL isSelected = self.menu.highlightedItem==menuItem;
		BOOL isDisabled = ![menuItem isEnabled];
		BOOL hasImage = [menuItem image]!=nil;
		BOOL hasSubmenu = [menuItem hasSubmenu];
		
		if(!isDisabled && isSelected){
			NSColor* cTop = [NSColor colorWithDeviceWhite:0.91 alpha:1.0];
			NSColor* cBottom = [NSColor colorWithDeviceWhite:0.71 alpha:1.0];
			
			NSGradient* selectionGrad = [[NSGradient alloc] initWithStartingColor:cTop endingColor:cBottom];
			[selectionGrad drawInRect:menuItemFrame angle:90];
			[selectionGrad release];
		}

		// Draw menu item image
		if(hasImage){
			NSRect imageRect = itemRect;
			imageRect.origin.y += 2;
			imageRect.size.width = menuItemImageWidth;
			imageRect.size.height = menuItemImageWidth;
			
			[menuItem.image drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
			
			itemRect.origin.x += menuItemImageWidth+menuItemImageTitleSpacing;
			itemRect.size.width -= (menuItemImageWidth+menuItemImageTitleSpacing);
		}
		
		// Draw submenu arrow
		if(hasSubmenu){
			NSImage* arrow = isSelected ? [NSImage imageNamed:@"submenu_arrow_selected"] : [NSImage imageNamed:@"submenu_arrow"];
			NSRect arrowRect = NSMakeRect(0, 0, 0, 0);
			arrowRect.size = arrow.size;
			arrowRect.origin.x = menuItemFrame.origin.x + menuItemFrame.size.width - 11;
			arrowRect.origin.y = menuItemFrame.origin.y + (menuItemHeight - arrow.size.height)/2;
			[arrow drawInRect:arrowRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
		}
		
		// Draw Item Title
		NSDictionary* textAttributes = isSelected ? [self selectedItemTextAttributes] : [self itemTextAttributes];
		textAttributes = isDisabled ? [self disabledItemTextAttributes] : textAttributes;
		
		NSAttributedString* attrStr = [[NSAttributedString alloc] initWithString:menuItem.title attributes:textAttributes];
		itemRect.origin.y += (menuItemHeight-attrStr.size.height)/2.0;
		[attrStr drawInRect:itemRect];	
		[attrStr release];
		
		y += menuItemHeight;
	}
}
#pragma mark -
#pragma mark Interaction
- (void)updateTrackingAreas{
	NSTrackingArea* area = [[self trackingAreas] objectAtIndex:0];
	[self removeTrackingArea:area];
	area = [[NSTrackingArea alloc] initWithRect:self.bounds options:NSTrackingMouseMoved|NSTrackingMouseEnteredAndExited|NSTrackingActiveInActiveApp owner:self userInfo:nil];
	[self addTrackingArea:area];
	[area release];
}


- (void)mouseUp:(NSEvent *)theEvent{
	// check if on selected item && selected item not disabled
		// perform action, update selected item
	
	if(![[self.menu highlightedItem] hasSubmenu]){
		[self.menu closeMenu];
	}
}

- (void)mouseDown:(NSEvent *)theEvent{
	if(!NSPointInRect([theEvent locationInWindow], self.frame)){
		[self.menu closeMenu];
	} else {
		NSPoint loc = [theEvent locationInWindow];
		[self highlightItemAtPoint:[self convertPointFromBase:loc]];
	}
}

- (void)mouseMoved:(NSEvent *)theEvent{
	NSPoint loc = [theEvent locationInWindow];
	[self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseDragged:(NSEvent *)theEvent{
	NSPoint loc = [theEvent locationInWindow];
	[self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseEntered:(NSEvent *)theEvent{
	NSPoint loc = [theEvent locationInWindow];
	[self highlightItemAtPoint:[self convertPointFromBase:loc]];
}

- (void)mouseExited:(NSEvent *)theEvent{
	// if not mouse on subwindow
	NSPoint loc = [theEvent locationInWindow];
	[self highlightItemAtPoint:[self convertPointFromBase:loc]];
}
#pragma mark -
- (void)highlightItemAtPoint:(NSPoint)p{
	NSMenuItem* highlighItem = [self itemAtPoint:p];
	if(highlighItem != self.menu.highlightedItem){
		self.menu.highlightedItem = highlighItem;
		
		[self setNeedsDisplay:YES];
	}
}

- (NSMenuItem*)itemAtPoint:(NSPoint)p{
	if(p.x <= 5 || p.x >= self.bounds.size.width){
		return nil;
	}
	if(p.y <= menuItemSpacingTop || p.y >= self.bounds.size.height-menuItemSpacingBottom){
		return nil;
	}

	int pos = (p.y-menuItemSpacingTop) / menuItemHeight;
		
	NSArray* itemArray = [self.menu itemArray];
	if([itemArray count] <= pos){
		return nil;
	}

	return [itemArray objectAtIndex:pos];
}

- (NSRect)rectOfItem:(NSMenuItem*)m{
	NSArray* itemArray = [self.menu itemArray];
	NSUInteger pos = [itemArray indexOfObject:m];

	float y = menuItemHeight*pos +menuItemSpacingTop;
	NSRect menuItemFrame = NSMakeRect(5, y, self.frame.size.width-5-5, menuItemHeight);
	return menuItemFrame;
}
#pragma mark -
#pragma mark View Config Overrides
- (BOOL)isFlipped{
	return YES;
}

- (BOOL)isOpaque{
	return NO;
}
#pragma mark -
- (OEMenu*)menu{
	return (OEMenu*)[self window];
}

@end
