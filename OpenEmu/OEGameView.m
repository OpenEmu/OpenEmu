/*
 Copyright (c) 2010, OpenEmu Team
 
 
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
#import "OEGameCore.h"
#import "OEGameLayer.h"
#import "OEGameCoreHelper.h"
#import "OESystemResponder.h"

static void OE_bindGameLayer(OEGameLayer *gameLayer)
{
    NSUserDefaultsController *ctrl = [NSUserDefaultsController sharedUserDefaultsController];
    [gameLayer bind:@"filterName"   toObject:ctrl withKeyPath:@"values.filterName" options:nil];
    [gameLayer bind:@"vSyncEnabled" toObject:ctrl withKeyPath:@"values.vsync"      options:nil];
}

@implementation OEGameView

@synthesize gameLayer, gameResponder;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if(self != nil)
    {
        frame.origin = NSZeroPoint;
        gameView = [[[NSView alloc] initWithFrame:frame] autorelease];
        
        [self addSubview:gameView];
        
        CALayer *rootLayer = [CALayer layer];
        
        [rootLayer setLayoutManager:[CAConstraintLayoutManager layoutManager]];
        
        CGColorRef color = CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1.0);
        [rootLayer setBackgroundColor:color];
        CGColorRelease(color);
        
        //Show the layer
        [gameView setLayer:rootLayer];
        [gameView setWantsLayer:YES];
        
        gameLayer = [OEGameLayer layer];
        OE_bindGameLayer(gameLayer);
        [gameLayer setDelegate:self];
        [gameLayer setOwnerView:self];
        [gameLayer setAsynchronous:YES];
        
        // fix to make sure gameLayer resizes its texture attachment and reports the right bounds to QC
        [gameLayer setNeedsDisplayOnBoundsChange:YES];

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

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor blackColor] set];
    NSRectFill([self bounds]);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}
- (BOOL)becomeFirstResponder{
	return YES;
}

- (id<OEGameCoreHelper>)rootProxy                { return [gameLayer rootProxy];   }
- (void)setRootProxy:(id<OEGameCoreHelper>)value { [gameLayer setRootProxy:value]; }

- (OESystemResponder *)gameResponder { return gameResponder; }
- (void)setGameResponder:(OESystemResponder *)value
{
    if(gameResponder != value)
    {
        id next = (gameResponder == nil
                   ? [super nextResponder]
                   : [gameResponder nextResponder]);
        
        gameResponder = value;
        
        [value setNextResponder:next];
        if(value == nil) value = next;
        [super setNextResponder:value];
    }
}

- (void)setNextResponder:(NSResponder *)aResponder
{
    if(gameResponder != nil)
        [gameResponder setNextResponder:aResponder];
    else
        [super setNextResponder:aResponder];
}

#if CGFLOAT_IS_DOUBLE
#define CGFLOAT_EPSILON DBL_EPSILON
#else
#define CGFLOAT_EPSILON FLT_EPSILON
#endif

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
    id<OEGameCoreHelper> rootProxy = [self rootProxy];
    
    NSRect bounds = [self bounds];
    OEIntSize maxScreenSize = rootProxy.screenSize;
    NSRect frame  = NSMakeRect(0.0, 0.0, maxScreenSize.width, maxScreenSize.height);
    
    CGFloat factor     = NSWidth(frame) / NSHeight(frame);
    CGFloat selfFactor = NSWidth(bounds) / NSHeight(bounds);
    
    if(selfFactor - CGFLOAT_EPSILON < factor && factor < selfFactor + CGFLOAT_EPSILON)
        frame = bounds;
    else
    {
        CGFloat scale = MIN(NSWidth(bounds) / NSWidth(frame), NSHeight(bounds) / NSHeight(frame));
        
        frame.size.width  *= scale;
        frame.size.height *= scale;
        frame.origin.x = NSMidX(bounds) - NSWidth(frame)  / 2.0;
        frame.origin.y = NSMidY(bounds) - NSHeight(frame) / 2.0;
    }
    
    [gameView setFrame:frame];
}

- (CGFloat)preferredWindowScale
{
    return [gameLayer preferredWindowScale];
}

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block
{
    [gameLayer setScreenshotHandler:block];
}

@end
