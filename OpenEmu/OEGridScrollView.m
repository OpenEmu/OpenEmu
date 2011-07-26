//
//  OEGridScrollView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.07.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEGridScrollView.h"

@implementation OEGridScrollView
- (id)init {
    self = [super init];
    if (self) {
		[self setPostsBoundsChangedNotifications:YES];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(drawRect:) name:NSViewBoundsDidChangeNotification object:self];
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
		[self setPostsBoundsChangedNotifications:YES];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(drawRect:) name:NSViewBoundsDidChangeNotification object:self];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setPostsBoundsChangedNotifications:YES];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(drawRect:) name:NSViewBoundsDidChangeNotification object:self];
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	
    [super dealloc];
}
- (BOOL)isFlipped{
	return NO;
}
#pragma mark -
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

@end
