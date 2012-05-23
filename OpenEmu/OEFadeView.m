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
#import <QuartzCore/QuartzCore.h>

#import "NSColor+OEAdditions.h"
@interface OEFadeView ()
@property (strong) void(^callback)();
@end
@implementation OEFadeView
@synthesize callback;
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {        
        [self setWantsLayer:YES];
        [self setLayer:[[CALayer alloc] init]];
        [[self layer] setFrame:[self bounds]];
        
        CATransition *transition = [CATransition animation];
        [transition setType:kCATransitionFade];
        [transition setDelegate:self];
        
        [[self layer] setActions:[NSDictionary dictionaryWithObject:transition forKey:@"sublayers"]];
    }
    
    return self;
}

- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    if(flag && self.callback){
        self.callback();
        self.callback = nil;
        
        [[self layer] setActions:[NSDictionary dictionary]];
    }
}

- (void)fadeFromImage:(NSBitmapImageRep*)start toImage:(NSBitmapImageRep*)end callback:(void(^)(void))block;
{
    self.callback = block;
    
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    CALayer *layer = [self layer];
    CALayer *startLayer = [CALayer layer];
    [startLayer setFrame:layer.bounds];
    
    NSImage *image = [[NSImage alloc] init];
    [image addRepresentation:start];
    startLayer.contents = image;
    
    [layer addSublayer:startLayer];
    [CATransaction commit];

    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.001 * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        CALayer *endLayer = [CALayer layer];
        if(end)
        {
            NSImage *image = [[NSImage alloc] init];
            [image addRepresentation:end];
            [endLayer setContents:image];
        }
        else 
            [endLayer setBackgroundColor:[[NSColor orangeColor] CGColor]];
        
        [layer replaceSublayer:startLayer with:endLayer];
    });
}

@end
