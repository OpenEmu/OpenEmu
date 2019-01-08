/*
 Copyright (c) 2014, OpenEmu Team
 
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

#import "OEHomebrewCoverView.h"

NSString * const OEURLImagesViewImageDidLoadNotificationName = @"OEURLImagesViewImageDidLoad";

@interface OEHomebrewCoverView ()
@property (assign) NSProgressIndicator *loadingIndicator;
@property (nonatomic) NSInteger currentImageIndex;
@property (nonatomic) NSImage *currentImage;
@end

@implementation OEHomebrewCoverView
static NSCache *cache;
static NSMutableDictionary *loading;

const static CGFloat itemWidth = 10.0;
const static CGFloat itemSpace =  4.0;

const static NSLock *lock;
+ (void)initialize
{
    if([self class] == [OEHomebrewCoverView class])
    {
        lock = [[NSLock alloc] init];

        cache = [[NSCache alloc] init];
        [cache setCountLimit:50];

        loading = [NSMutableDictionary dictionary];
    }
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        [self OE_performSetup];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self)
    {
        [self OE_performSetup];
    }
    return self;
}

- (void)OE_performSetup
{
    [self setURLs:nil];

    NSProgressIndicator *indicator = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
    [indicator setIndeterminate:YES];
    [indicator setStyle:NSProgressIndicatorSpinningStyle];
    [indicator setControlSize:NSRegularControlSize];
    [indicator setHidden:YES];
    [indicator setUsesThreadedAnimation:YES];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(imageDidLoad:) name:OEURLImagesViewImageDidLoadNotificationName object:nil];

    _loadingIndicator = indicator;

    [self addSubview:indicator];

    [self setCurrentImageIndex:0];
}

#pragma mark -
- (void)setURLs:(NSArray *)urls
{
    _URLs = urls;

    if(urls) [self setCurrentImageIndex:0];
}

- (void)setCurrentImageIndex:(NSInteger)currentImage
{
    _currentImageIndex = currentImage;

    NSURL *url = [[self URLs] objectAtIndex:currentImage];

    [lock lock];
    NSImage *cachedImage = [cache objectForKey:url];
    [lock unlock];

    NSProgressIndicator *progress = [self loadingIndicator];
    if(cachedImage)
    {
        self.currentImage = cachedImage;
        if(![progress isHidden])
        {
            [progress stopAnimation:nil];
            [progress setHidden:YES];
        }
    }
    else
    {
        self.currentImage = nil;
        [self OE_fetchImage:_currentImageIndex];

        if([progress isHidden])
        {
            [progress setHidden:NO];
            [progress startAnimation:self];
        }
    }

    [self setNeedsDisplay:YES];
}

- (void)imageDidLoad:(NSNotification*)notification
{
    NSURL *loadedImage = [[notification userInfo] objectForKey:@"URL"];
    NSURL *url = [[self URLs] objectAtIndex:_currentImageIndex];

    if([loadedImage isEqualTo:url])
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setCurrentImageIndex:self->_currentImageIndex];
        });
}

- (void)viewDidMoveToSuperview
{
    NSProgressIndicator *progress = [self loadingIndicator];
    [progress sizeToFit];

    CGFloat x = NSMidX([self bounds]) - NSWidth([progress frame])/2.0;
    CGFloat y = NSMidY([self bounds]) - NSHeight([progress frame])/2.0;
    [progress setFrameOrigin:NSMakePoint(x, y)];
}

#pragma mark - Frames
- (NSRect)pageSelectorRect
{
    const CGFloat midX = NSMidX([self bounds]);
    const NSInteger numberOfImages = [[self URLs] count];

    const CGFloat width = (numberOfImages*itemWidth + (numberOfImages-1)*itemSpace);
    const CGFloat minX = midX - width/2.0;
    return NSMakeRect(minX, 1, width, itemWidth);
}

- (NSRect)rectForPageSelector:(NSInteger)page
{
    const CGFloat midX = NSMidX([self bounds]);
    const NSInteger numberOfImages = [[self URLs] count];

    const CGFloat minX = midX - (numberOfImages*itemWidth + (numberOfImages-1)*itemSpace)/2.0
                              + (page*itemWidth + (page-1)*itemSpace);

    return NSMakeRect(minX, 1, itemWidth, itemWidth);
}

- (NSRect)rectForImage:(NSImage*)image
{
    const NSRect  bounds      = [self bounds];
    const NSSize  imageSize   = [image size];
    const CGFloat aspectRatio = imageSize.width/imageSize.height;

    const CGFloat border = 10.0; // border for drawing image shadows
    const CGFloat pageSelectorHeight = 10.0;

    NSRect rect = NSInsetRect(bounds, border, border);

    // leave some room for page selector
    rect.size.height -= pageSelectorHeight;

    // keep original aspect ratio
    if(aspectRatio < NSWidth(bounds)/NSHeight(bounds))
        rect.size.width  = NSHeight(rect) * aspectRatio;
    else
        rect.size.height = NSWidth(rect) / aspectRatio;

    // center horizontally
    rect.origin.x = ( NSWidth(bounds)  - NSWidth(rect)  ) / 2.0;

    // adjust for page selector, again
    rect.origin.y += pageSelectorHeight;

    return rect;
}

#pragma mark - Drawing
- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    [[NSColor clearColor] setFill];
    NSRectFill([self bounds]);

    // Draw image
    NSImage *image = self.currentImage;
    if(image)
    {
        const NSRect imageRect = [self rectForImage:image];

        [[NSGraphicsContext currentContext] saveGraphicsState];
        NSShadow *shadow = [[NSShadow alloc] init];
        [shadow setShadowBlurRadius:3.0];
        [shadow setShadowOffset:NSMakeSize(0.0, -3.0)];
        [shadow setShadowColor:[NSColor blackColor]];
        [shadow set];

        [image drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
        [[NSGraphicsContext currentContext] restoreGraphicsState];
    }

    // Draw page selector
    NSInteger numberOfImages = [[self URLs] count];
    for(NSInteger i=0; i < numberOfImages; i++)
    {
        const NSRect rect = [self rectForPageSelector:i];
        const NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect:rect];
        if(i == _currentImageIndex)
        {
            [[NSColor clearColor] setFill];
            [[NSColor colorWithRed:0 green:136.0/255.0 blue:204.0/255.0 alpha:1.0] setStroke];
        }
        else
        {
            [[NSColor grayColor] setFill];
            [[NSColor clearColor] setStroke];
        }

        [path fill];
        [path stroke];
    }
}

#pragma mark - Interaction
- (void)mouseDown:(NSEvent *)theEvent
{
    if([theEvent clickCount] == 2 && [self doubleAction])
    {
        [NSApp sendAction:[self doubleAction] to:[self target] from:self];
    }
}

- (void)updateTrackingAreas
{
    [[self trackingAreas] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [self removeTrackingArea:obj];
    }];

    const NSRect trackingRect = [self pageSelectorRect];
    NSTrackingAreaOptions options = NSTrackingActiveInKeyWindow|NSTrackingMouseMoved;
    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:trackingRect options:options owner:self userInfo:nil];
    [self addTrackingArea:area];

    NSTrackingArea *viewArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingActiveInKeyWindow|NSTrackingMouseEnteredAndExited owner:self userInfo:nil];
    [self addTrackingArea:viewArea];

}
- (void)mouseMoved:(NSEvent *)theEvent
{
    const NSRect  trackingRect = [self pageSelectorRect];
    const NSPoint locationInWindow = [theEvent locationInWindow];
    const NSPoint location = [self convertPoint:locationInWindow fromView:nil];

    if(NSPointInRect(location, trackingRect))
    {
        const CGFloat x = location.x-NSMinX(trackingRect);
        NSInteger index = x  / (itemWidth+itemSpace);

        if(x > index * (itemWidth+itemSpace)-itemSpace + itemWidth)
            return;

        [self setCurrentImageIndex:index];
    }
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    [self mouseMoved:theEvent];
}

- (void)mouseEntered:(NSEvent *)theEvent
{}

- (void)mouseExited:(NSEvent *)theEvent
{
    [self setCurrentImageIndex:0];
}

#pragma mark -
- (void)OE_fetchImage:(NSInteger)index
{
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    NSURL *url = [[self URLs] objectAtIndex:index];

    if(url == nil) return;

    [lock lock];
    if([cache objectForKey:url] == nil && [loading objectForKey:url] == nil)
    {
        [loading setObject:@(YES) forKey:url];

        dispatch_async(queue, ^{
            NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
            if(image == nil)
                image = [NSImage imageNamed:NSImageNameCaution];

            [lock lock];
            [cache setObject:image forKey:url];
            [loading removeObjectForKey:url];
            [lock unlock];

            NSDictionary *userInfo = @{ @"URL": url };
            [[NSNotificationCenter defaultCenter] postNotificationName:OEURLImagesViewImageDidLoadNotificationName object:nil userInfo:userInfo];
        });
    }
    [lock unlock];
}
@end
