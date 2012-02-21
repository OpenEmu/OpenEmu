/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEGridLayer.h"

@implementation OEGridLayer
@synthesize tracking = _tracking, interactive = _interactive;

- (id)init
{
    if((self = [super init]))
    {
        [self setLayoutManager:[OEGridViewLayoutManager layoutManager]];
        [self setNeedsDisplayOnBoundsChange:YES];
    }
    
    return self;
}

- (CALayer *)hitTest:(CGPoint)p
{
    if(!_interactive) return nil;

    if(CGRectContainsPoint([self frame], p))
        return [super hitTest:p] ? : self;
    
    return nil;
}

- (id<CAAction>)actionForKey:(NSString *)event
{
    return nil;
}

- (void)layoutSublayers
{
    if([[self delegate] respondsToSelector:@selector(layoutSublayers)])
        [[self delegate] layoutSublayers];
}

- (void)mouseDownAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
}

- (void)mouseUpAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
}

- (void)mouseMovedAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationNone;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return NO;
}

- (void)willMoveToSuperlayer:(OEGridLayer *)superlayer
{
}

- (void)didMoveToSuperlayer
{
}

- (void)addSublayer:(CALayer *)layer
{
    if([layer isKindOfClass:[OEGridLayer class]])
    {
        [(OEGridLayer *)layer willMoveToSuperlayer:self];
        [super addSublayer:layer];
        [(OEGridLayer *)layer didMoveToSuperlayer];
    }
    else
        [super addSublayer:layer];
}

- (void)insertSublayer:(CALayer *)layer atIndex:(unsigned int)idx
{
    if([layer isKindOfClass:[OEGridLayer class]])
    {
        [(OEGridLayer *)layer willMoveToSuperlayer:self];
        [super insertSublayer:layer atIndex:idx];
        [(OEGridLayer *)layer didMoveToSuperlayer];
    }
    else
        [super insertSublayer:layer atIndex:idx];
}

- (void)removeFromSuperlayer
{
    [self willMoveToSuperlayer:nil];
    
    [super removeFromSuperlayer];
    
    [self didMoveToSuperlayer];
}

#pragma mark - Properties

- (NSWindow *)window
{
    return [[self view] window];
}

- (NSView *)view
{
    CALayer *superlayer = self;
    while(superlayer)
    {
        NSView *delegate = [superlayer delegate];
        
        if([delegate isKindOfClass:[NSView class]])
            return delegate;

        superlayer = [superlayer superlayer];
    }
    
    return nil;
}

@end
