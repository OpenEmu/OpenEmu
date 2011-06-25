//
//  OEToolbarView.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 31.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEToolbarView.h"
#import "NSImage+OEDrawingAdditions.h"
#import "NSImage+OEHighlight.h"
@interface OEToolbarView (Private)
- (void)_setup;
@end 
@implementation OEToolbarView
@synthesize items;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
		[self _setup];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
		[self _setup];
    }
    return self;
}

- (id)init {
    self = [super init];
    if (self) {
		[self _setup];
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

#pragma mark -
- (void)addItem:(OEToolbarItem*)item{
	[self.items addObject:item];
	
	[self setNeedsDisplay:YES];
}
#pragma mark -
- (void)setContentSeperatorColor:(NSColor *)contentSeperatorColor{
	[contentSeperatorColor retain];
	[_contentSeperatorColor release];
	
	_contentSeperatorColor = contentSeperatorColor;
	[self setNeedsDisplay:YES];
}
- (NSColor*)contentSeperatorColor{
	return _contentSeperatorColor;
}

#pragma mark -
- (OEToolbarItem*)selectedItem{
	return selectedItem;
}

- (void)markItemAsSelected:(OEToolbarItem*)tbItem{
	@try {
		selectedItem = [self.items objectAtIndex:[self.items indexOfObject:tbItem]];
	}
	@catch (NSException *exception) {
		
	}	
}

#pragma mark -
- (BOOL)isOpaque{
	return NO;
}

- (void)drawRect:(NSRect)dirtyRect{
	[super drawRect:dirtyRect];
	
	if(self.contentSeperatorColor){
		NSRect lineRect = self.bounds;
		lineRect.size.height = 1;
		
		[self.contentSeperatorColor setFill];
		NSRectFill(lineRect);
	}
	
	float itemSpacing = 5;
	float titlePadding = 8;
	float imageSideLength = 36;
	float imageTitleSpacing = 4;
	
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:4.0 size:11.0];
	NSColor* textColor = [NSColor blackColor];
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.45]];
	[shadow setShadowBlurRadius:1];
	[shadow setShadowOffset:NSMakeSize(0, -1)];
	
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setAlignment:NSCenterTextAlignment];
	
	NSMutableDictionary* textAttributes = [NSMutableDictionary dictionary];
	[textAttributes setObject:font forKey:NSFontAttributeName];
	[textAttributes setObject:shadow forKey:NSShadowAttributeName];
	[textAttributes setObject:textColor forKey:NSForegroundColorAttributeName];
	[textAttributes setObject:ps forKey:NSParagraphStyleAttributeName];
	
	float x = itemSpacing;
	for(OEToolbarItem* anItem in self.items){
		x += titlePadding;
		
		NSAttributedString* title = [[[NSAttributedString alloc] initWithString:anItem.title attributes:textAttributes] autorelease];
		NSRect titleRect = NSMakeRect(x, 6, title.size.width, title.size.height);
		NSRect imageRect = NSMakeRect(x+(title.size.width-imageSideLength)/2, titleRect.origin.y+titleRect.size.height+imageTitleSpacing-3, imageSideLength, imageSideLength);
		
		if(NSEqualRects(anItem.itemRect, NSZeroRect)){
			anItem.itemRect = NSMakeRect(x-titlePadding, 1, title.size.width+2*titlePadding, 54);
		}
		
		if(anItem == selectedItem){
			NSImage* selectionHighlight = [NSImage imageNamed:@"tab_selector"];
			[selectionHighlight drawInRect:anItem.itemRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:12 rightBorder:12 topBorder:0 bottomBorder:0];		
		}
		
		NSImage* icon = anItem.icon;
		
		
		
		if(anItem == highlightedItem && NSPointInRect([self convertPointFromBase:[[self window] convertScreenToBase:[NSEvent mouseLocation]]], anItem.itemRect)){
			icon = [icon imageForHighlight];	
		}
				
		[icon drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
		[anItem.title drawInRect:titleRect withAttributes:textAttributes];
		
		x += title.size.width;
		x += titlePadding;
		x += itemSpacing;
	}
}

- (void)mouseDown:(NSEvent *)theEvent{
	NSPoint loc = [self convertPointFromBase:[theEvent locationInWindow]];

	highlightedItem = nil;
	for(OEToolbarItem* anItem in self.items){
		if(NSPointInRect(loc, anItem.itemRect)){
			highlightedItem = anItem;
			break;
		}
	}
	[self setNeedsDisplay:YES];

	
	if(highlightedItem==nil)
		[super mouseDown:theEvent];	
}

- (void)mouseUp:(NSEvent *)theEvent{
	NSPoint loc = [self convertPointFromBase:[theEvent locationInWindow]];
	
	if(highlightedItem && NSPointInRect(loc, highlightedItem.itemRect)){
		selectedItem = highlightedItem;
		highlightedItem = nil;
		
		if(selectedItem.target && [selectedItem.target respondsToSelector:selectedItem.action]){
			[selectedItem.target performSelector:selectedItem.action withObject:selectedItem];
			[self setNeedsDisplay:YES];
			return;
		}
	}
	highlightedItem = nil;
	
	[self setNeedsDisplay:YES];
}

- (void)mouseEntered:(NSEvent *)theEvent{
	[self setNeedsDisplay:YES];
}
- (void)mouseDragged:(NSEvent *)theEvent{
	[self setNeedsDisplay:YES];
}
- (void)mouseExited:(NSEvent *)theEvent{
	[self setNeedsDisplay:YES];
}

- (void)_setup{
	self.items = [[[NSMutableArray alloc] init] autorelease];
}
@end

@implementation OEToolbarItem
@synthesize title, icon, target, action, itemRect;
@end
