/*
 Copyright (c) 2010, OpenEmu Team
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

#import "OEGameView.h"
#import "GameCore.h"
#import "OEGameLayer.h"

static void OE_bindGameLayer(OEGameLayer *gameLayer)
{
    NSUserDefaultsController *ctrl = [NSUserDefaultsController sharedUserDefaultsController];
    [gameLayer bind:@"filterName"   toObject:ctrl withKeyPath:@"values.filterName" options:nil];
    [gameLayer bind:@"vSyncEnabled" toObject:ctrl withKeyPath:@"values.vsync"      options:nil];
}

@implementation OEGameView

@synthesize gameLayer;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if(self != nil)
    {
        CALayer *rootLayer = [CALayer layer];
        
        [rootLayer setLayoutManager:[CAConstraintLayoutManager layoutManager]];
        
        CGColorRef color = CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1.0);
        [rootLayer setBackgroundColor:color];
        CGColorRelease(color);
        
        //Show the layer
        [self setLayer:rootLayer];
        [self setWantsLayer:YES];
        
        gameLayer = [OEGameLayer layer];
        OE_bindGameLayer(gameLayer);
        [gameLayer setDelegate:self];
        [gameLayer setOwnerView:self];
        [gameLayer setAsynchronous:YES];
        
        [gameLayer setName:@"gameLayer"];
        [gameLayer setConstraints:
         [NSArray arrayWithObjects:
          [CAConstraint constraintWithAttribute:kCAConstraintMidX   relativeTo:@"superlayer" attribute:kCAConstraintMidX],
          [CAConstraint constraintWithAttribute:kCAConstraintMidY   relativeTo:@"superlayer" attribute:kCAConstraintMidY],
          [CAConstraint constraintWithAttribute:kCAConstraintWidth  relativeTo:@"superlayer" attribute:kCAConstraintWidth],
          [CAConstraint constraintWithAttribute:kCAConstraintHeight relativeTo:@"superlayer" attribute:kCAConstraintHeight],
          nil]];
        
        [rootLayer addSublayer:gameLayer];
    }
    return self;
}

- (void)dealloc
{
    [gameLayer unbind:@"filterName"];
    [gameLayer unbind:@"vSyncEnabled"];
    [super dealloc];
}

- (id<CAAction>)actionForLayer:(CALayer *)layer forKey:(NSString *)event
{
    return (id<CAAction>) [NSNull null];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (id<OEGameCoreHelper>)rootProxy { return [gameLayer rootProxy]; }
- (void)setRootProxy:(id<OEGameCoreHelper>)value
{
    [gameLayer setRootProxy:value];
    [self setGameCore:[value gameCore]];
}

- (GameCore *)gameCore { return gameCore; }
- (void)setGameCore:(GameCore *)value
{
    if(gameCore != value)
    {
        id next = (gameCore == nil
                   ? [super nextResponder]
                   : [gameCore nextResponder]);
        
        gameCore = value;
        
        [value setNextResponder:next];
        if(value == nil) value = next;
        [super setNextResponder:value];
    }
}

- (void)setNextResponder:(NSResponder *)aResponder
{
    if(gameCore != nil)
        [gameCore setNextResponder:aResponder];
    else
        [super setNextResponder:aResponder];
}

- (CGFloat)preferredWindowScale
{
    return [gameLayer preferredWindowScale];
}

- (NSImage *)imageForCurrentFrame
{
    return [gameLayer imageForCurrentFrame];
}

- (void)awakeFromNib
{
    [self setWantsLayer:YES];
}

@end
