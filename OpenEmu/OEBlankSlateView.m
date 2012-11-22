//
//  OEBlankSlateView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 21.11.12.
//
//

#import "OEBlankSlateView.h"
#import "OEGridBlankSlateView.h"
#import "OECoverGridForegroundLayer.h"

@interface OEBlankSlateView ()
@property NSDragOperation lastDragOperation;
- (void)OE_commonInit;
- (void)OE_setupSlateView:(NSView*)view;
- (void)OE_layoutSlateView;
@end
@implementation OEBlankSlateView
@synthesize delegate=_delegate;
@synthesize lastDragOperation=_lastDragOperation;
@synthesize representedCollectionName=_representedCollectionName;
@synthesize representedSystemPlugin=_representedSystemPlugin;

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:NSZeroRect];
    if (self) {
        [self OE_commonInit];
    }
    return self;
}

- (void)OE_layoutSlateView
{
    OEGridBlankSlateView *slateView = [[self subviews] lastObject];
    const NSRect  visibleRect = [self bounds];
    const NSSize  viewSize    = [slateView frame].size;
    const NSRect  viewFrame   = NSMakeRect(ceil((NSWidth(visibleRect) - viewSize.width) / 2.0), ceil((NSHeight(visibleRect) - viewSize.height) / 2.0), viewSize.width, viewSize.height);
    [slateView setFrame:viewFrame];
    [slateView setAutoresizingMask:NSViewMaxXMargin|NSViewMinXMargin|NSViewMinYMargin|NSViewMaxYMargin];
}

// Following code inspired by: http://stackoverflow.com/questions/2520978/how-to-tile-the-contents-of-a-calayer
// callback for CreateImagePattern.
static void drawPatternImage(void *info, CGContextRef ctx)
{
    CGImageRef image = (CGImageRef)info;
    CGContextDrawImage(ctx, CGRectMake(0.0, 0.0, CGImageGetWidth(image), CGImageGetHeight(image)), image);
}

// callback for CreateImagePattern.
static void releasePatternImage(void *info)
{
    CGImageRef image = (CGImageRef)info;
    CGImageRelease(image);
}

- (void)OE_commonInit
{
    static CGImageRef      noiseImageRef = nil;
    static CGColorRef      noiseColorRef = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // Create a pattern from the noise image and apply as the background color
        static const CGPatternCallbacks callbacks = {0, &drawPatternImage, &releasePatternImage};
        
        NSURL            *noiseImageURL = [[NSBundle mainBundle] URLForImageResource:@"noise"];
        CGImageSourceRef  source        = CGImageSourceCreateWithURL((__bridge CFURLRef)noiseImageURL, NULL);
        noiseImageRef                   = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        
        CGFloat width  = CGImageGetWidth(noiseImageRef);
        CGFloat height = CGImageGetHeight(noiseImageRef);
        
        CGPatternRef    pattern       = CGPatternCreate(noiseImageRef, CGRectMake(0.0, 0.0, width, height), CGAffineTransformMake(1.0, 0.0, 0.0, 1.0, 0.0, 0.0), width, height, kCGPatternTilingConstantSpacing, YES, &callbacks);
        CGColorSpaceRef space         = CGColorSpaceCreatePattern(NULL);
        CGFloat         components[1] = {1.0};
        
        noiseColorRef = CGColorCreateWithPattern(space, pattern, components);
        
        CGColorSpaceRelease(space);
        CGPatternRelease(pattern);
        CFRelease(source);
    });
    
    [self setWantsLayer:YES];
    
    CALayer *layer = [CALayer layer];
    [self setLayer:layer];
    
    // Disable implicit animations
    [layer setActions:@{ @"onOrderIn" : [NSNull null],
                        @"onOrderOut" : [NSNull null],
                         @"sublayers" : [NSNull null],
                          @"contents" : [NSNull null],
                            @"bounds" : [NSNull null]
     }];
    
    // Set background lighting
    [layer setContentsGravity:kCAGravityResize];
    [layer setContents:[NSImage imageNamed:@"background_lighting"]];
    [layer setFrame:[self bounds]];
    
    
    // Setup noise
    CALayer *noiseLayer = [CALayer layer];
    [noiseLayer setFrame:[self bounds]];
    [noiseLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [noiseLayer setGeometryFlipped:YES];
    [noiseLayer setBackgroundColor:noiseColorRef];
    [layer addSublayer:noiseLayer];
    
    // Setup foreground
    OECoverGridForegroundLayer *foregroundLayer = [[OECoverGridForegroundLayer alloc] init];
    [foregroundLayer setFrame:[self bounds]];
    [foregroundLayer setFrame:[self bounds]];
    [foregroundLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [layer addSublayer:foregroundLayer];
}

#pragma mark -
- (void)setRepresentedCollectionName:(NSString *)representedCollectionName
{
    if(representedCollectionName == _representedCollectionName) return;
    _representedCollectionName = representedCollectionName;
    _representedSystemPlugin = nil;
        
    OEGridBlankSlateView *view = [[OEGridBlankSlateView alloc] initWithCollectionName:_representedCollectionName];
    [self OE_setupSlateView:view];
}

- (void)setRepresentedSystemPlugin:(OESystemPlugin *)representedSystemPlugin
{
    if(representedSystemPlugin == _representedSystemPlugin) return;
    _representedSystemPlugin = representedSystemPlugin;
    _representedCollectionName = nil;
    
    OEGridBlankSlateView *view = [[OEGridBlankSlateView alloc] initWithSystemPlugin:_representedSystemPlugin];
    [self OE_setupSlateView:view];
}

- (void)OE_setupSlateView:(NSView*)view
{
    // Remove current blank slate subview
    [[[self subviews] lastObject] removeFromSuperview];

    [self addSubview:view];
    [self OE_layoutSlateView];
}
#pragma mark -
#pragma mark NSDraggingDestination
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    // The delegate has to be able to validate and accept drops, if it can't do then then there is no need to drag anything around
    if([_delegate respondsToSelector:@selector(blankSlateView:validateDrop:)] && [_delegate respondsToSelector:@selector(blankSlateView:acceptDrop:)])
    {
        _lastDragOperation = [_delegate blankSlateView:self validateDrop:sender];
    }
    
    return _lastDragOperation;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
    if([_delegate respondsToSelector:@selector(blankSlateView:draggingUpdated:)])
    {
        _lastDragOperation = [_delegate blankSlateView:self draggingUpdated:sender];
    }
    return _lastDragOperation;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return [_delegate respondsToSelector:@selector(blankSlateView:acceptDrop:)] && [_delegate blankSlateView:self acceptDrop:sender];
}
@end
