//
//  OEGridScrollView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.07.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEGridScrollView.h"
#import "NSColor+IKSAdditions.h"
@interface OEGridBackgroundView : NSView @end

@implementation OEGridScrollView

#warning Constant redrawing is bad!
- (void)drawRect:(NSRect)dirtyRect{
	NSImage* lightning = [NSImage imageNamed:@"background_lighting"];
	[lightning drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
	
	NSImage* noise = [NSImage imageNamed:@"noise"];
	
	float x, y;
	for(y=0; y < self.bounds.size.height; y+= noise.size.height){
		for(x=0; x < self.bounds.size.width; x+= noise.size.width){
			[noise drawAtPoint:NSMakePoint(x, y) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
		}	
		
	}
}

- (BOOL)isFlipped{
	return NO;
}
/*- (void)_setup{

	/*
	self.wantsLayer = YES;
	CALayer* layer = self.layer;
	
	NSImage* lightningImage = [NSImage imageNamed:@"background_lighting"];
	CALayer* lightningLayer = [CALayer layer];
	lightningLayer.contents = lightningImage;
	
	CALayer* backgroundLayer = [CALayer layer];
	backgroundLayer.backgroundColor = [[NSColor grayColor] CGColor];
	
	[lightningLayer display];
	[backgroundLayer display];
	
	[layer addSublayer:backgroundLayer];
	[layer addSublayer:lightningLayer];*

}

- (id)init {
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
    [super dealloc];
}
*/

@end

@implementation OEGridBackgroundView
- (BOOL)isFlipped{
	return NO;
}

- (void)drawRect:(NSRect)dirtyRect{
	NSImage* lightning = [NSImage imageNamed:@"background_lighting"];
	[lightning drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
	
	NSImage* noise = [NSImage imageNamed:@"noise"];
	
	float x, y;
	for(y=0; y < self.bounds.size.height; y+= noise.size.height){
		for(x=0; x < self.bounds.size.width; x+= noise.size.width){
			[noise drawAtPoint:NSMakePoint(x, y) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
		}	
		
	}
}

- (BOOL)isOpaque{
	return YES;
}
@end
