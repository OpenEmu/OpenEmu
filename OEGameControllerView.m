/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

#define BUTTON_SIZE NSMakeSize(90.0, 32.0)
- (void)addButtonWithName:(NSString *)aName target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end
{
    [self addButtonWithName:aName toolTip:aName target:aTarget startPosition:start endPosition:end];
}

- (void)addButtonWithName:(NSString *)aName toolTip:(NSString *)aToolTip target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end
{
    NSRect bounds = [self bounds];
    NSRect button = NSZeroRect;
    button.size = BUTTON_SIZE;
    NSPoint middle = NSZeroPoint;
	
	// additional points for bezier curve path
//	NSPoint control1, control2;
	
    if(start.y <= NSMinY(drawRect))
    {
        button.origin.x = start.x - button.size.width / 2.0;
        button.origin.y = 14.0;
        start.y = button.origin.y + button.size.height / 2.0;
        middle.x = start.x;
        middle.y = NSMinY(drawRect) + button.size.height / 2.0;
		
//		control1.x = middle.x;
//		control2.x = middle.x;
//		control1.y = middle.y / 3.0;
//		control2.y = control1.y * 2.0;
    }
    else if(start.x <= NSMinX(drawRect))
    {
        button.origin.x = 14.0;
        start.x = button.origin.x + button.size.width / 2.0;
        button.origin.y = start.y - button.size.height / 2.0;
        middle.x = NSMinX(drawRect);
        middle.y = start.y;
		
//		control1.y = middle.y;
//		control2.y = middle.y;
//		control1.x = middle.x / 3.0;
//		control2.x = control1.x * 2.0;
    }
    else if(start.x >= NSMaxX(drawRect))
    {
        button.origin.x = bounds.size.width - (button.size.width + 14.0);
        start.x = button.origin.x + button.size.width / 2.0;
        button.origin.y = start.y - button.size.height / 2.0;
        middle.x = NSMaxX(drawRect);
        middle.y = start.y;
		
//		control1.y = middle.y;
//		control2.y = middle.y;
//		control1.x = middle.x / 3.0;
//		control2.x = control1.x * 2.0;
    }
    else if(start.y >= NSMaxY(drawRect))
    {
        button.origin.x = start.x - button.size.width / 2.0;
        button.origin.y = bounds.size.height - (button.size.height + 14.0);
        start.y = button.origin.y + button.size.height / 2.0;
        middle.x = start.x;
        middle.y = NSMaxY(drawRect) - button.size.height / 2.0;

//		control1.x = middle.x;
//		control2.x = middle.x;
//		control1.y = middle.y / 3.0;
//		control2.y = control1.y * 2.0;
	}
    
    NSButton *added = [[[NSButton alloc] initWithFrame:button] autorelease];
    [added setTarget:aTarget];
    [added setAction:@selector(selectInputControl:)];
    [added bind:@"title" toObject:aTarget withKeyPath:aName options:nil];
    [added setBezelStyle:NSRoundRectBezelStyle];
//    [added setBezelStyle:NSRoundedBezelStyle];
	[added setButtonType:NSPushOnPushOffButton];
    [added setToolTip:aToolTip];
//	[[added cell]  setControlSize:NSSmallControlSize];  
    
    [self addSubview:added];
    
    [lines moveToPoint:start];
	//	[lines curveToPoint:end controlPoint1:control1 controlPoint2:control2];
	[lines lineToPoint:middle];
	[lines lineToPoint:end];
}

@end
