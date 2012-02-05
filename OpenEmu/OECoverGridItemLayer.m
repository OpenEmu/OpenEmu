//
//  CoverGridItemself.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoverGridItemLayer.h"
#import "IKSGridItemLayer.h"

#import "OECoverGridDataSourceItem.h"
#import "OECoverGridNoArtworkLayer.h"

#import "OEUIDrawingUtils.h"
#import "NSColor+IKSAdditions.h"
#import "NSURL+OELibraryAdditions.h"

#import "OEGridViewFieldEditor.h"

#import "DelayedBlockExecution.h"
#import <QuickLook/QuickLook.h>
@interface OECoverGridItemLayer (privates)
// Sets rating by click point
- (void)setRatingWithPoint:(NSPoint)p pressed:(BOOL)pressed;

// animates resize of a layer
- (void)resizeLayer:(CALayer*)layer to:(CGSize)size;
// animates movement of layer
- (void)moveLayer:(CALayer*)layer to:(CGPoint)point centered:(BOOL)centered;
// animates opacity of layer
- (void)fadeOpacityOfLayer:(CALayer*)layer to:(float)newOpacity;


// called to display on drop animation
- (void)_displayOnDrop:(id)sender;

- (void)setUpFieldEditor:(OEGridViewFieldEditor*)fieldEditor;

- (id)_datasourceProxy_objectForKey:(NSString*)key;
- (void)_datasourceProxy_setObject:(id)obj forKey:(NSString*)key;

- (void)_layoutStaticElements;
- (void)_layoutImageAndSelection;
@end
#pragma mark -
@implementation OECoverGridItemLayer
@synthesize imageRatio;
@synthesize selectionLayer, glossLayer, indicationLayer, imageLayer, titleLayer, ratingLayer;
@synthesize image;
@synthesize isReloading;
- (id)init{
    self = [super init];
    if (self) {
        lastImageSize = NSZeroSize;
        
        // Setup image layer
        CALayer *iLayer = [CALayer layer];
        iLayer.shadowColor = [[NSColor blackColor] CGColor];
        iLayer.geometryFlipped = YES;
        iLayer.transform = CATransform3DMakeScale(1, -1, 1);
        iLayer.shadowOffset = CGSizeMake(0, 3);
        iLayer.shadowOpacity = 1.0;
        iLayer.shadowRadius = 3.0;
        iLayer.contentsGravity = kCAGravityResize;
        iLayer.delegate = self;
        
        self.imageLayer = iLayer;
        [self addSublayer:iLayer];
        
        // setup layer for status display (missing, processing, ....)
        OECoverGridIndicationLayer *inLayer = [OECoverGridIndicationLayer layer];
        inLayer.transform = CATransform3DMakeScale(1, -1, 1);
        inLayer.delegate = self;
        self.indicationLayer = inLayer;
        [self insertSublayer:inLayer above:iLayer];
        
        // setup gloss layer
        OECoverGridGlossLayer *gLayer = [OECoverGridGlossLayer layer];
        gLayer.delegate = self;
        gLayer.needsDisplayOnBoundsChange = YES;
        self.glossLayer = gLayer;
        [self insertSublayer:gLayer above:inLayer];
        
        // setup selection layer
        OECoverGridSelectionLayer *sLayer = [OECoverGridSelectionLayer layer];
        sLayer.delegate = self;
        self.selectionLayer = sLayer;
        [self insertSublayer:sLayer above:gLayer];
        
        // setup title layer;
        CATextLayer *tLayer = [CATextLayer layer];
        tLayer.delegate = self;
        
        NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];
        tLayer.font=font;
        tLayer.fontSize=12;
        tLayer.foregroundColor=[[NSColor whiteColor] CGColor];
        tLayer.truncationMode = kCATruncationEnd;
        tLayer.alignmentMode = kCAAlignmentCenter;
        
        tLayer.shadowColor = [[NSColor blackColor] CGColor];
        tLayer.shadowOffset = CGSizeMake(0, -1);
        tLayer.shadowRadius = 1.0;
        tLayer.shadowOpacity = 1.0;
        
        tLayer.transform = CATransform3DMakeScale(1, -1, 1);
        [self addSublayer:tLayer];
        self.titleLayer = tLayer;
        
        // setup rating layer
        OECoverGridRatingLayer *rLayer = [OECoverGridRatingLayer layer];
        rLayer.delegate = self;
        [self addSublayer:rLayer];
        self.ratingLayer = rLayer;
        
        lastSize = [self bounds].size;
    }
    
    return self;
}


- (void)dealloc{
    self.selectionLayer = nil;
    self.glossLayer = nil;
    self.indicationLayer = nil;
    self.imageLayer = nil;
    self.titleLayer = nil;
    self.ratingLayer = nil;
    
    [super dealloc];
}

#pragma mark -
- (NSRect)hitRect{
    NSRect rect = [self frame];
    
    rect.origin.x += self.imageLayer.frame.origin.x;
    rect.origin.y += self.imageLayer.frame.origin.y;
    rect.size = self.imageLayer.frame.size;
    
    return rect;
}

#pragma mark -
- (void)reloadImage{
    reloadImage = NO;
    self.image = [self _datasourceProxy_objectForKey:@"image"];
    [self _layoutImageAndSelection];
}

- (void)reloadData
{
    NSString *title = [self _datasourceProxy_objectForKey:@"title"];
    self.titleLayer.string = title;
    
    int rating = [[self _datasourceProxy_objectForKey:@"rating"] intValue];
    [self.ratingLayer setRating:rating pressed:NO];
    
    int status = [[self _datasourceProxy_objectForKey:@"status"] intValue];
    self.indicationLayer.type = status;
    
    if(reloadImage)
    {
        reloadImage = NO;
        [self reloadImage];
    }
    self.isReloading = NO;
}

- (void)layoutSublayers
{
    if(!CGSizeEqualToSize(lastSize, [self bounds].size))
    {
        [self _layoutStaticElements];
        
        reloadImage = NO;
        self.image = [self _datasourceProxy_objectForKey:@"image"];
        [self _layoutImageAndSelection];
    }
    
    lastSize = [self bounds].size;
}

- (void)_layoutStaticElements
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    // Height of title string
    float titleHeight = 16;
    
    // Space between bottom of image and top of title string
    float titleImageSpacing = 17;
    
    float subtitleHeight = 11;
    float subtitleWidth = 56;
    
    // calculate rect for title layer
    titleRect = CGRectMake([self bounds].origin.x, ([self bounds].size.height-titleHeight-titleImageSpacing-subtitleHeight+titleImageSpacing), [self bounds].size.width, titleHeight);
    // calculate rect for rating layer
    ratingRect = CGRectIntegral(CGRectMake([self bounds].origin.x+([self bounds].size.width-subtitleWidth)/2, titleRect.origin.y+titleRect.size.height, subtitleWidth, subtitleHeight));
    // make sure it has correct size to prevent sub pixel stuff
    ratingRect.size = CGSizeMake(55, 11);
    
    // set background colors for debug (to point out the various frames)
    BOOL debug_colors = NO;
    if(debug_colors)
    {
        titleLayer.backgroundColor = [[NSColor greenColor] CGColor];
        imageLayer.backgroundColor = [[NSColor blueColor] CGColor];
        ratingLayer.backgroundColor = [[NSColor orangeColor] CGColor];
        self.backgroundColor = [[NSColor yellowColor] CGColor];
    } 
    else 
    {
        titleLayer.backgroundColor = imageLayer.backgroundColor = ratingLayer.backgroundColor = self.backgroundColor = NULL;
    }
    // update layer frames
    titleLayer.frame = titleRect;
    ratingLayer.frame = ratingRect;
    [CATransaction commit];
}

- (void)_layoutImageAndSelection
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [self retain];
    
    imageRatio = 1.0;
    CALayer *newImageLayer;
    if(self.image==nil)
    {
        imageRatio = 1.365385;
        newImageLayer = [OECoverGridNoArtworkLayer layer];
        newImageLayer.needsDisplayOnBoundsChange = YES;
        newImageLayer.contents = nil;
    } 
    else
    {
        imageRatio = self.image.size.height/self.image.size.width;
        
        newImageLayer = [CALayer layer];
        newImageLayer.contents = self.image;
    }
    
    newImageLayer.shadowColor = [[NSColor blackColor] CGColor];
    newImageLayer.geometryFlipped = YES;
    newImageLayer.transform = CATransform3DMakeScale(1, -1, 1);
    newImageLayer.shadowOffset = CGSizeMake(0, 3);
    newImageLayer.shadowOpacity = 1.0;
    newImageLayer.shadowRadius = 3.0;
    newImageLayer.contentsGravity = kCAGravityResize;
    newImageLayer.delegate = self;
    
#warning fix grid view crash
    // weired issue: sometimes imageLayer is not a sublayer of imageLayer.superlayer
    @try 
    {
        [self.imageLayer.superlayer replaceSublayer:self.imageLayer with:newImageLayer];
        self.imageLayer = newImageLayer;
    }
    @catch (NSException *exception) 
    {
        NSLog(@"%@", exception);
        [CATransaction commit];
        [self release];
        return;
    }
    
    // Height of title string
    float titleHeight = 16;
    
    // Space between bottom of image and top of title string
    float titleImageSpacing = 17;
    float subtitleHeight = 11;
    
    // Border between top of layer and top of image
    float topBorder = 7;
    
    // Space between left layer border and left side of image
    float imageBorderLeft = 13;
    // Space between right side of image and right border of layer
    float imageBorderRight = 13;
    
    
    // calculate area where image is placed
    imageContainerRect = NSMakeRect(0+imageBorderLeft, 0+topBorder, [self bounds].size.width-imageBorderLeft-imageBorderRight, [self bounds].size.height - titleHeight-titleImageSpacing-subtitleHeight-topBorder);
    
    // Calculated size of image in container frame
    float width, height;
    if(self.imageRatio<1)
    { // width > height
        width = imageContainerRect.size.width;
        height = width*self.imageRatio;
    } 
    else 
    {
        height = imageContainerRect.size.height;
        width = height/self.imageRatio;
    }
    
    // Determine actual frame for image
    CGRect coverImageRect = CGRectIntegral(CGRectMake(imageContainerRect.origin.x+(imageContainerRect.size.width-width)/2, imageContainerRect.origin.y+imageContainerRect.size.height-height, width, height));
    
    self.glossLayer.frame = coverImageRect;
    self.imageLayer.frame = coverImageRect;
    
    NSWindow *win = [self.gridView window];
    
    // determine states that affect selection display
    BOOL selectionInactive = (![win isMainWindow] && [win firstResponder] == self.gridView);
    BOOL selectionHidden = !self.selected;
    self.selectionLayer.isInactive = selectionInactive;
    
    CGRect selectionRect = CGRectInset(coverImageRect, -6, -6);
    if((self.selected && !CGRectEqualToRect(self.selectionLayer.frame, selectionRect)) ||
       (self.selected && self.selectionLayer.isInactive != selectionInactive))
    {
        [self.selectionLayer setNeedsDisplay];
    }
    
    self.selectionLayer.frame = selectionRect;
    self.selectionLayer.hidden = selectionHidden;
    self.indicationLayer.frame = CGRectInset(coverImageRect, 1, 1);
    
    [CATransaction commit];
    
    [self release];
}
#pragma mark -
- (void)display
{
    if(!acceptingOnDrop)
    {
        NSNumber *val = [self _datasourceProxy_objectForKey:@"status"];
        self.indicationLayer.type = [val intValue];
    }
}

- (void)setRepresentedIndex:(NSInteger)_representedIndex
{
    if(_representedIndex!=representedIndex)
    {
        reloadImage = YES;
    }
    
    [super setRepresentedIndex:_representedIndex];
}

- (void)setImage:(NSImage *)_image
{
    [_image retain];
    [image release];
    
    image = _image;
    [self _layoutImageAndSelection];
}
#pragma mark -
- (void)beginValueChange
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    
    // hide layers that might take long to change value, or look weired without those that don't
    self.imageLayer.hidden = YES;
    
    self.glossLayer.hidden = YES;
    self.indicationLayer.hidden = YES;
    self.selectionLayer.hidden = YES;
    
    [CATransaction commit];
}

- (void)endValueChange
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    // show layers that have previously been hidden
    self.imageLayer.hidden = NO;
    
    self.glossLayer.hidden = NO;
    self.indicationLayer.hidden = NO;
    
    [CATransaction commit];
}

#pragma mark -
#pragma mark IKSGridItemLayerEventProtocol
- (BOOL)mouseDown:(NSEvent*)theEvent
{
    
    NSPoint p = [self.gridView convertPoint:[theEvent locationInWindow] fromView:nil];
    p.y -= [self frame].origin.y;
    p.x -= [self frame].origin.x;
    
    
    if(NSPointInRect(p, NSRectFromCGRect(ratingRect)))
    {
        isEditingRating = YES;
        
        [self setRatingWithPoint:p pressed:YES];
        
        return YES;
    }
    
    isEditingRating = NO;
    
    if(NSPointInRect(p, self.titleLayer.frame))
    {
        if ([theEvent clickCount]==2 )
        {
            CGRect fieldFrame = self.titleLayer.frame;
            
            fieldFrame.size.height += 1;
            
            OEGridViewFieldEditor *fieldEditor = [self.gridView fieldEditorForFrame:fieldFrame ofLayer:self];
            [self setUpFieldEditor:fieldEditor];
            
            return YES;
        } 
    }
    
    return NO;
}

- (BOOL)mouseDragged:(NSEvent*)theEvent
{
    NSPoint p = [self.gridView convertPoint:[theEvent locationInWindow] fromView:nil];
    p.y -= [self frame].origin.y;
    p.x -= [self frame].origin.x;
    if(isEditingRating)
    {
        [self setRatingWithPoint:p pressed:YES];
        return YES;
    }
    isEditingRating = NO;
    
    return NO;
}

- (BOOL)mouseEntered:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseExited:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseMoved:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseUp:(NSEvent*)theEvent
{
    NSPoint p = [self.gridView convertPoint:[theEvent locationInWindow] fromView:nil];
    p.y -= [self frame].origin.y;
    p.x -= [self frame].origin.x;
    
    if(isEditingRating)
    {
        isEditingRating = NO;
        [self setRatingWithPoint:p pressed:NO];
        return YES; 
    }
    
    isEditingRating = NO;
    
    if(NSPointInRect(p, self.titleLayer.frame))
        return YES;
    
    return NO;
}
#pragma mark -
#pragma mark Dragging
- (NSImage*)dragImage
{
    CGSize imageSize = self.imageLayer.bounds.size;
    
    NSBitmapImageRep *dragImageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:imageSize.width pixelsHigh:imageSize.height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:(NSInteger)imageSize.width * 4 bitsPerPixel:32];
    NSGraphicsContext *bitmapContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:dragImageRep];
    CGContextRef context = (CGContextRef)[bitmapContext graphicsPort];
    
    CGContextClearRect(context, CGRectMake(0.0, 0.0, imageSize.width, imageSize.height));
    CGContextSetAllowsAntialiasing(context, YES);
    
    [self.imageLayer renderInContext:context];
    [self.glossLayer renderInContext:context];        
    CGContextFlush(context);
    
    NSImage *dragImage = [[NSImage alloc] initWithSize:NSSizeFromCGSize(imageSize)];
    [dragImage addRepresentation:dragImageRep];
    [dragImage setFlipped:YES];
    
    
    [dragImageRep release];
    
    return [dragImage autorelease];
}


#pragma mark -
#pragma mark Dropping
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
    // cancel if we are already working on a drop, when another comes in
    // (is that even possible?)
    if(acceptingOnDrop) return NSDragOperationNone;
    
    // Try to get an image directly from pasteboard
    id proposedImageRepresentation;
    NSArray *images = [[sender draggingPasteboard] readObjectsForClasses:[NSArray arrayWithObject:[NSImage class]] options:nil];
    if([images count]==0)
    {
        // Didn't get image, look for files
        images = [[sender draggingPasteboard] readObjectsForClasses:[NSArray arrayWithObject:[NSURL class]] options:nil];
        if(images==0)
        {
            return NSDragOperationNone;
        }
        
        // got a file url, check if it has image suffix
        NSURL *url = [images objectAtIndex:0];
        if([url hasImageSuffix])
        {
            proposedImageRepresentation = url;
        } 
        else 
        {
            return NSDragOperationNone;
        }
    }
    else 
    {
        proposedImageRepresentation = [images objectAtIndex:0];
    }
    
    acceptingOnDrop = YES;
    
    // wait a while to prevent animation when dragging is just dragging by
    float dropAnimatioTimernDelay = [[NSUserDefaults standardUserDefaults] floatForKey:@"debug_drop_animation_delay"];
    dropAnimationDelayTimer = [[NSTimer scheduledTimerWithTimeInterval:dropAnimatioTimernDelay target:self selector:@selector(_displayOnDrop:) userInfo:proposedImageRepresentation repeats:NO] retain];
    
    return NSDragOperationGeneric;
}

- (void)_displayOnDrop:(id)sender
{
    if(!acceptingOnDrop)
        return;
    
    NSImage *proposedCoverImage;
    
    id userInfo = [sender userInfo];
    // check if we need to load image from url
    if([userInfo isKindOfClass:[NSURL class]])
    {
        QLThumbnailRef thumbnailRef = QLThumbnailCreate(NULL, (CFURLRef)userInfo, [self frame].size, NULL);
        CGImageRef thumbnailImageRef = QLThumbnailCopyImage(thumbnailRef);
        proposedCoverImage = [[NSImage alloc] initWithCGImage:thumbnailImageRef size:NSMakeSize(CGImageGetWidth(thumbnailImageRef), CGImageGetHeight(thumbnailImageRef))];
        CGImageRelease(thumbnailImageRef);
        
        [proposedCoverImage autorelease];
    } 
    else 
    {
        proposedCoverImage = userInfo;
    }
   
    if(![proposedCoverImage isValid])
    {
        acceptingOnDrop = NO;
        return;
    }
    
    
    // display drop acceptance
    self.indicationLayer.type = 3;
    
    // calculate aspect ration of new cover image
    float rat = [proposedCoverImage size].height/[proposedCoverImage size].width;
    float width, height;
    if(rat<1)
    { // width > height
        width = imageContainerRect.size.width;
        height = width*rat;
    } 
    else 
    {
        height = imageContainerRect.size.height;
        width = height/rat;
    }
    
    
    CALayer *newImageLayer = [CALayer layer];
    newImageLayer.contents = proposedCoverImage;
    newImageLayer.geometryFlipped = YES;
    newImageLayer.transform = CATransform3DMakeScale(1, -1, 1);
    newImageLayer.contentsGravity = kCAGravityResize;
    newImageLayer.delegate = self;
    
    float startWidth =  self.gridView.itemSize.width*[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_startWidthFac"];
    float startHeight = startWidth*rat;
    newImageLayer.opacity = 1.0; //[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_opacityStart"];
    newImageLayer.frame = CGRectMake(([self frame].size.width-startWidth)/2, ([self frame].size.height-startHeight)/2, startWidth, startHeight);
    
    [self insertSublayer:newImageLayer above:self.imageLayer];
    
    CGRect newCoverImageRect = CGRectIntegral(CGRectMake(imageContainerRect.origin.x+(imageContainerRect.size.width-width)/2, imageContainerRect.origin.y+imageContainerRect.size.height-height, width, height));
    
    [self resizeLayer:self.imageLayer to:newCoverImageRect.size];
    [self resizeLayer:self.glossLayer to:newCoverImageRect.size];
    [self resizeLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).size];
    [self resizeLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).size];
    
    [self fadeOpacityOfLayer:newImageLayer to:1.0f];
    [self resizeLayer:newImageLayer to:newCoverImageRect.size];
    [self moveLayer:newImageLayer to:newCoverImageRect.origin centered:NO];
    
    [self moveLayer:self.imageLayer to:newCoverImageRect.origin centered:NO];
    [self moveLayer:self.glossLayer to:newCoverImageRect.origin centered:NO];
    [self moveLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).origin centered:NO];
    [self moveLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).origin centered:NO];
    
    [dropAnimationDelayTimer invalidate];
    [dropAnimationDelayTimer release];
    dropAnimationDelayTimer = nil;
}
- (void)draggingExited:(id < NSDraggingInfo >)sender
{
    if(!acceptingOnDrop) return;
    
    if(dropAnimationDelayTimer)
    {
        [dropAnimationDelayTimer invalidate]; 
        [dropAnimationDelayTimer release]; 
        dropAnimationDelayTimer = nil; 
    }
    
    float width, height;
    if(self.imageRatio<1)
    { // width > height
        width = imageContainerRect.size.width;
        height = width*self.imageRatio;
    } 
    else 
    {
        height = imageContainerRect.size.height;
        width = height/self.imageRatio;
    }
    
    // Space between left layer border and left side of image
    float imageBorderLeft = 13;
    CGRect newCoverImageRect = CGRectIntegral(CGRectMake(imageBorderLeft+(imageContainerRect.size.width-width)/2, imageContainerRect.origin.y+imageContainerRect.size.height-height, width, height));
    
    NSUInteger index = [[self sublayers] indexOfObjectIdenticalTo:self.imageLayer]+1;
    CALayer *newImageLayer = [[self sublayers] objectAtIndex:index];
    if([newImageLayer isKindOfClass:[CALayer class]])
    {
        [self resizeLayer:newImageLayer to:CGSizeMake(0, 0)];
        [self moveLayer:newImageLayer to:CGPointMake([self frame].size.width/2, [self frame].size.height/2) centered:YES];
        
        [self performAfterDelay:0.2f block:^{
            [newImageLayer removeFromSuperlayer];
        }];
        
    }
    
    //[self moveLayer:self.imageLayer to:newCoverImageRect.origin];
    [self resizeLayer:self.imageLayer to:newCoverImageRect.size];
    [self resizeLayer:self.glossLayer to:newCoverImageRect.size];
    [self resizeLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).size];
    [self resizeLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).size];
    
    [self fadeOpacityOfLayer:newImageLayer to:0.0];
    
    [self moveLayer:self.imageLayer to:newCoverImageRect.origin centered:NO];
    [self moveLayer:self.glossLayer to:newCoverImageRect.origin centered:NO];
    [self moveLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).origin centered:NO];
    [self moveLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).origin centered:NO];
    
    NSNumber *val = [self _datasourceProxy_objectForKey:@"status"];
    self.indicationLayer.type = [val intValue];
    
    acceptingOnDrop = NO;
}

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender
{
    if(acceptingOnDrop) return NSDragOperationGeneric;
    
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender
{
    NSNumber *val = [self _datasourceProxy_objectForKey:@"status"];
    self.indicationLayer.type = [val intValue];
    
    acceptingOnDrop = NO;
    NSUInteger index = [[self sublayers] indexOfObjectIdenticalTo:self.imageLayer]+1;
    CALayer *newImageLayer = [[self sublayers] objectAtIndex:index];
    if([newImageLayer isKindOfClass:[CALayer class]])
    {
        [self _datasourceProxy_setObject:((CALayer*) newImageLayer).contents forKey:@"image"];
        
        newImageLayer.frame = self.imageLayer.frame;
        newImageLayer.autoresizingMask = self.imageLayer.autoresizingMask;
        newImageLayer.contentsGravity = self.imageLayer.contentsGravity;
        newImageLayer.delegate = self.imageLayer.delegate;
        newImageLayer.shadowColor = self.imageLayer.shadowColor;
        newImageLayer.shadowOffset = self.imageLayer.shadowOffset;
        newImageLayer.shadowOpacity = self.imageLayer.shadowOpacity;
        newImageLayer.shadowRadius = self.imageLayer.shadowRadius;
        newImageLayer.borderColor = self.imageLayer.borderColor;
        newImageLayer.borderWidth = self.imageLayer.borderWidth;
        newImageLayer.needsDisplayOnBoundsChange = NO;
        newImageLayer.geometryFlipped = self.imageLayer.geometryFlipped;
        newImageLayer.delegate = self.imageLayer.delegate;
        newImageLayer.transform = self.imageLayer.transform;
        
        [self.imageLayer removeFromSuperlayer];
        self.imageLayer = (CALayer*)newImageLayer;
    }
    
    return YES;
}
#pragma mark -
#pragma mark ValueChanging
- (void)setSelected:(BOOL)selectedFlag
{
    BOOL redraw = selectedFlag && selectedFlag!=self.selected;
    
    [super setSelected:selectedFlag];
    
    self.selectionLayer.hidden = !selectedFlag;
    
    NSWindow *win = [self.gridView window];
    BOOL selectionInactive = (![win isMainWindow] && [win firstResponder] == self.gridView);
    redraw |= (self.selectionLayer.isInactive != selectionInactive);
    self.selectionLayer.isInactive = selectionInactive;
    
    if(redraw)
    {
        [self.selectionLayer setNeedsDisplay];
    }
}

#pragma mark -
#pragma mark Helpers and private methods
- (void)setRatingWithPoint:(NSPoint)p pressed:(BOOL)pressed
{
    float pos = p.x-ratingRect.origin.x;
    
    float stepWidth = 55/5;
    int rating = 0;
    if(pos<=0)
    {
        rating =  0; 
    }
    else if(pos>=55-stepWidth)
    { rating = 5; 
    }
    else 
        rating = ((pos/stepWidth)+1);
    
    [ratingLayer setRating:rating pressed:pressed];
    [self _datasourceProxy_setObject:[NSNumber numberWithInt:rating] forKey:@"rating"];
}

- (void)moveLayer:(CALayer*)layer to:(CGPoint)point centered:(BOOL)centered
{
    CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"position"];
    animation.fromValue = [layer valueForKey:@"position"];
    
    if(!centered)
    {
        point.x += layer.frame.size.width/2;
        point.y += layer.frame.size.height/2;
    }
    
    animation.toValue = [NSValue valueWithPoint:NSPointFromCGPoint(point)];
    animation.delegate = self;
    
    layer.position = point;
    
    [layer addAnimation:animation forKey:@"position"];
}

- (void)resizeLayer:(CALayer*)layer to:(CGSize)size
{
    CGRect oldBounds = layer.bounds;
    CGRect newBounds = oldBounds;
    newBounds.size = size;
    CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"bounds"];
    
    animation.fromValue = [NSValue valueWithRect:NSRectFromCGRect(oldBounds)];
    animation.toValue = [NSValue valueWithRect:NSRectFromCGRect(newBounds)];
    
    layer.bounds = newBounds;
    
    animation.delegate = self;
    
    [layer addAnimation:animation forKey:@"bounds"];
}

- (void)fadeOpacityOfLayer:(CALayer*)layer to:(float)newOpacity
{
    CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"opacity"];
    
    animation.fromValue = [NSNumber numberWithFloat:layer.opacity];
    animation.toValue = [NSNumber numberWithFloat:newOpacity];
    
    layer.opacity = newOpacity;
    animation.delegate = self;
    
    [layer addAnimation:animation forKey:@"opacity"];
}

#pragma mark -
#pragma mark Animation stuff
- (id < CAAction >)actionForLayer:(CALayer *)layer forKey:(NSString *)key
{
    if(layer == self.ratingLayer)
        if(isEditingRating && layer == self.ratingLayer && [key isEqualTo:@"contents"])
            return nil;
    
    return (id < CAAction >)[NSNull null];
}

- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag
{
    if(self.selected)
        [self.selectionLayer setNeedsDisplay];
    
    [self setNeedsDisplay];
}

#pragma mark -
- (void)setUpFieldEditor:(OEGridViewFieldEditor*)fieldEditor
{
    NSString *title = [self _datasourceProxy_objectForKey:@"title"];
    [fieldEditor setString:title];
    
    [fieldEditor setAlignment:NSCenterTextAlignment];
    [fieldEditor setBorderColor:[NSColor blackColor]];
    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];
    [fieldEditor setFont:font];
    
    [fieldEditor setDelegate:self];
    
    fieldEditor.layer.shadowOpacity = 0.45;
    fieldEditor.layer.shadowColor = [[NSColor blackColor] CGColor];
    fieldEditor.layer.shadowOffset = CGSizeMake(0, -6);
    fieldEditor.layer.shadowRadius = 5;
    
    /*
     [fieldEditor setDelegate:self];
     
     [fieldEditor setSelectedRange:NSMakeRange(0, [[fieldEditor string] length])];
     
     fieldEditor.layer.borderColor = [[NSColor blackColor] CGColor];
     fieldEditor.layer.borderWidth = 1.0;
     */
    
    [[[self gridView] window] makeFirstResponder:[[fieldEditor subviews] objectAtIndex:0]];
    [fieldEditor setHidden:NO];
}


#pragma mark -
- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{  
    NSTextField *field = [aNotification object];
    OEGridViewFieldEditor *fieldEditor = (OEGridViewFieldEditor*)[field superview];
    
    [fieldEditor setFrameSize:NSMakeSize(0, 0)];
    [fieldEditor setHidden:YES];
    
    NSString *newTitle = [fieldEditor string];
    NSString *title = [self _datasourceProxy_objectForKey:@"title"];
    if([newTitle isNotEqualTo:@""] && [newTitle isNotEqualTo:title])
    {
        
        [self _datasourceProxy_setObject:newTitle forKey:@"title"];
        [self reloadData];
        
        [NSTimer scheduledTimerWithTimeInterval:0.3 target:self.gridView selector:@selector(reloadData) userInfo:nil repeats:NO];
    }
    
    [fieldEditor setDelegate:nil];
}

#pragma mark -

- (id)_datasourceProxy_objectForKey:(NSString*)key
{
    return [self.gridView.dataSource gridView:self.gridView objectValueForKey:key withRepresentedObject:self.representedObject];
}
- (void)_datasourceProxy_setObject:(id)obj forKey:(NSString*)key
{
    [self.gridView.dataSource gridView:self.gridView setObject:obj forKey:key withRepresentedObject:self.representedObject];
}
@end
