//
//  OEControlerImageView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEControlerImageView.h"

@protocol OEControlsButtonHighlightProtocol  <NSObject>
	- (NSPoint)highlightPoint;
	- (float)highlightRadius;
@end
@interface OEControlerImageView (Priavte)
- (void)_setup;
@end
@implementation OEControlerImageView
@synthesize image;

- (void)_setup{
	highlightedButtonPoint = NSZeroPoint;
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(selectedButtonChanged:) name:@"OEControlsPreferencesSelectedButtonDidChange" object:nil];
}

- (id)init{
    self = [super init];
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

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self _setup];
    }
    return self;
}
- (void)dealloc {
    self.image = nil;
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
    [super dealloc];
}
#pragma mark -
- (void)drawRect:(NSRect)dirtyRect{
	if(!self.image) return;
	
	NSRect targetRect;
	targetRect.size = self.image.size;
	targetRect.origin = NSMakePoint((self.frame.size.width-image.size.width)/2, 0);
	
	[self.image drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:NO hints:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:NSImageInterpolationNone] forKey:NSImageHintInterpolation]];
	
	if(NSEqualPoints(highlightedButtonPoint, NSZeroPoint))
		return;
	
	[NSGraphicsContext saveGraphicsState];
	NSRect rect = NSMakeRect(targetRect.origin.x+highlightedButtonPoint.x-33, targetRect.origin.y+highlightedButtonPoint.y-33, 66.0, 66.0);
	
	NSBezierPath* path = [NSBezierPath bezierPathWithRect:self.bounds];
	[path setWindingRule:NSEvenOddWindingRule];
	[path appendBezierPathWithOvalInRect:rect];
	[path setClip];

	[[NSColor colorWithDeviceWhite:0.0 alpha:0.3] setFill];
	NSRectFillUsingOperation(self.bounds, NSCompositeSourceAtop);
	[NSGraphicsContext restoreGraphicsState];
	
	
	
	NSPoint highlightP = NSMakePoint(targetRect.origin.x+highlightedButtonPoint.x-38, targetRect.origin.y+highlightedButtonPoint.y-45);
	NSImage* highlightImage = [NSImage imageNamed:@"controls_highlight"]; 
	[highlightImage drawAtPoint:highlightP fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
	
	
}
- (void)setImage:(NSImage*)img{
	highlightedButtonPoint = NSZeroPoint;
	
	[img retain];
	[image release];
	
	image = img;
	
	[self setNeedsDisplay:YES];
}

- (void)selectedButtonChanged:(NSNotification*)notification{
	id <OEControlsButtonHighlightProtocol> obj = (id <OEControlsButtonHighlightProtocol>)[notification object];
	if(!obj || ![obj respondsToSelector:@selector(highlightPoint)]){
		highlightedButtonPoint = NSZeroPoint;
		[self display];		
		return;
	}
	highlightedButtonPoint =  [obj highlightPoint];
	
	[self setNeedsDisplay:YES];
}
@end
