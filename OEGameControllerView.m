//
//  OEGameControllerView.m
//  OpenEmu
//
//  Created by Remy Demarest on 24/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGameControllerView.h"


@implementation OEGameControllerView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        [self setControlZone:frame];
        lines = [[NSBezierPath bezierPath] retain];
        [lines setLineWidth:2.0];
    }
    return self;
}

- (void)dealloc
{
    [lines release];
    [gameController release];
    [super dealloc];
}

- (void)drawRect:(NSRect)rect
{
    [gameController drawInRect:drawRect
                      fromRect:NSZeroRect
                     operation:NSCompositeSourceOver
                      fraction:1.0];
    
    [[NSColor redColor] set];
    [lines stroke];
}

- (void)mouseDown:(NSEvent *)anEvent
{
    NSLog(@"%@", NSStringFromPoint([self convertPointFromBase:[anEvent locationInWindow]]));
}

- (NSRect)controlZone
{
    return controlZone;
}

- (void)setControlZone:(NSRect)aZone
{
    controlZone = aZone;
    
    drawRect.origin.x = 150.0 + controlZone.origin.x;
    drawRect.origin.y = 50.0 + controlZone.origin.y;
    drawRect.size.width = controlZone.size.width - 300;
    drawRect.size.height = controlZone.size.height - 100;
}

- (NSImage *)gameController
{
    return [[gameController retain] autorelease];
}

- (void)setGameController:(NSImage *)_value
{
    [gameController autorelease];
    gameController = [_value retain];
}

#define BUTTON_SIZE NSMakeSize(96.0, 32.0)
- (void)addButtonWithName:(NSString *)aName target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end
{
    NSRect bounds = [self bounds];
    NSRect button = NSZeroRect;
    button.size = BUTTON_SIZE;
    NSPoint middle = NSZeroPoint;
    if(start.y <= NSMinY(drawRect))
    {
        button.origin.x = start.x - button.size.width / 2.0;
        button.origin.y = 14.0;
        start.y = button.origin.y + button.size.height / 2.0;
        middle.x = start.x;
        middle.y = NSMinY(drawRect) + button.size.height / 2.0;
    }
    else if(start.x <= NSMinX(drawRect))
    {
        button.origin.x = 14.0;
        start.x = button.origin.x + button.size.width / 2.0;
        button.origin.y = start.y - button.size.height / 2.0;
        middle.x = NSMinX(drawRect);
        middle.y = start.y;
    }
    else if(start.x >= NSMaxX(drawRect))
    {
        button.origin.x = bounds.size.width - (button.size.width + 14.0);
        start.x = button.origin.x + button.size.width / 2.0;
        button.origin.y = start.y - button.size.height / 2.0;
        middle.x = NSMaxX(drawRect);
        middle.y = start.y;
    }
    else if(start.y >= NSMaxY(drawRect))
    {
        button.origin.x = start.x - button.size.width / 2.0;
        button.origin.y = bounds.size.height - (button.size.height + 14.0);
        start.y = button.origin.y + button.size.height / 2.0;
        middle.x = start.x;
        middle.y = NSMaxY(drawRect) - button.size.height / 2.0;
    }
    
    NSButton *added = [[[NSButton alloc] initWithFrame:button] autorelease];
    [added setTarget:aTarget];
    [added setAction:@selector(selectInputControl:)];
    [added bind:@"title" toObject:aTarget withKeyPath:aName options:nil];
    [added setBezelStyle:NSRoundedBezelStyle];
    [added setButtonType:NSPushOnPushOffButton];
    
    [self addSubview:added];
    
    [lines moveToPoint:start];
    [lines lineToPoint:middle];
    [lines lineToPoint:end];
}

@end
