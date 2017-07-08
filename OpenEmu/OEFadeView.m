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

#import "OEFadeView.h"
#import "NSColor+OEAdditions.h"
@import QuartzCore;

@interface OEFadeView () <CAAnimationDelegate>
@property(copy) void(^callback)(void);
@end

@implementation OEFadeView

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self setLayer:[[CALayer alloc] init]];
        [self setWantsLayer:YES];
        [[self layer] setContentsGravity:kCAGravityResize];
        [[self layer] setBackgroundColor:[[NSColor blackColor] CGColor]];
    }
    
    return self;
}

- (NSImage *)OE_defaultImage
{
    static NSImage *defaultImage;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultImage = [[NSImage alloc] initWithSize:NSMakeSize(1, 1)];

        [defaultImage lockFocus];
        [[NSColor blackColor] setFill];
        NSRectFill(NSMakeRect(0, 0, 1, 1));
        [defaultImage unlockFocus];
    });

    return defaultImage;
}

- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    if(flag && [self callback])
    {
        [self callback]();
        [self setCallback:nil];
    }
}

- (void)fadeFromImage:(NSImage *)start toImage:(NSImage *)end callback:(void(^)(void))block;
{
    if(start == nil) start = [self OE_defaultImage];
    if(end == nil) end = [self OE_defaultImage];

    self.callback = block;
    
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    [[self layer] setContents:start];
    [[self layer] setContentsGravity:kCAGravityResizeAspectFill];

    [CATransaction commit];
    
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.001 * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        CATransition *transition = [CATransition animation];
        [transition setType:kCATransitionFade];
        [transition setRemovedOnCompletion:YES];
        [transition setDelegate:self];

        [[self layer] addAnimation:transition forKey:@"contents"];
        [[self layer] setContents:end];
    });
}

@end
