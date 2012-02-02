//
//  OEHUDProgressbar.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 21.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDProgressbar.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEHUDProgressbar


- (id)init {
    self = [super init];
    if (self) {
        self.minValue = 0.0;
        self.value = 0.5;
        self.maxValue = 1.0;
        
        [self addObserver:self forKeyPath:@"minValue" options:0 context:nil];
        [self addObserver:self forKeyPath:@"value" options:0 context:nil];
        [self addObserver:self forKeyPath:@"maxValue" options:0 context:nil];
    }
    return self;
}

- (void)dealloc {
    [self removeObserver:self forKeyPath:@"minValue"];
    [self removeObserver:self forKeyPath:@"value"];
    [self removeObserver:self forKeyPath:@"maxValue"];
    
    [super dealloc];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self setNeedsDisplay:YES];
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSImage* trackImage = [NSImage imageNamed:@"hud_progress_bar_track"];
    [trackImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:7 topBorder:2 bottomBorder:2];
    
    if(self.value == 0.0) return;
    
    NSImage* barImage = [NSImage imageNamed:@"hud_progress_bar"];
    NSRect barRect = (NSRect){{0,0},{(roundf([self bounds].size.width)*(self.value-self.minValue)/(self.maxValue-self.minValue)),[self bounds].size.height}};
    [barImage drawInRect:barRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:7 topBorder:1 bottomBorder:1];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if(self.value>=self.maxValue)
    {
        self.value = self.maxValue;
        return;
    }
    self.value = self.maxValue;
    
    if(self.value>self.maxValue)
    {
        self.value = self.minValue;
        return;
    }
}
@synthesize minValue,value,maxValue;
@end
