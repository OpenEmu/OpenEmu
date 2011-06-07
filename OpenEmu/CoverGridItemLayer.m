//
//  CoverGridItemself.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "CoverGridItemLayer.h"
#import "IKSGridItemLayer.h"

#import "CoverGridDataSourceItem.h"
#import "CoverGridNoArtwork.h"

#import "OEUIDrawingUtils.h"
#import "NSColor+IKSAdditions.h"
#import "NSURL+OELibraryAdditions.h"

#import "OEFieldEditor.h"
@interface CoverGridItemLayer (privates)
// Sets rating by click point
- (void)setRatingWithPoint:(NSPoint)p pressed:(BOOL)pressed;

// animates resize of a layer
- (void)resizeLayer:(CALayer*)layer to:(CGSize)size;
// animates movement of layer
- (void)moveLayer:(CALayer*)layer to:(CGPoint)point centered:(BOOL)centered;
// animates opacity of layer
- (void)fadeOpacityOfLayer:(CALayer*)layer to:(float)newOpacity;
// updates values
- (void)_valuesDidChange;

// called to display on drop animation
- (void)_displayOnDrop:(id)sender;

- (void)setUpFieldEditor:(OEFieldEditor*)fieldEditor;

@end
#pragma mark -
@implementation CoverGridItemLayer
@synthesize imageRatio;
@synthesize selectionLayer, glossLayer, indicationLayer, imageLayer, titleLayer, ratingLayer;

- (id)init{
    self = [super init];
    if (self) {
		// Setup image layer
		CALayer* iLayer = [CALayer layer];
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
		CoverGridIndicationLayer* inLayer = [CoverGridIndicationLayer layer];
		inLayer.transform = CATransform3DMakeScale(1, -1, 1);
		inLayer.delegate = self;
		self.indicationLayer = inLayer;
		[self insertSublayer:inLayer above:iLayer];
		
		// setup gloss layer
		CoverGridGlossLayer* gLayer = [CoverGridGlossLayer layer];
		gLayer.delegate = self;
		self.glossLayer = gLayer;
		[self insertSublayer:gLayer above:inLayer];
		
		// setup selection layer
		CoverGridSelectionLayer* sLayer = [CoverGridSelectionLayer layer];
		sLayer.delegate = self;
		self.selectionLayer = sLayer;
		[self insertSublayer:sLayer above:gLayer];
		
		// setup title layer;
		CATextLayer* tLayer = [CATextLayer layer];
		tLayer.delegate = self;
		
		NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];
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
		CoverGridRatingLayer* rLayer = [CoverGridRatingLayer layer];
		rLayer.delegate = self;
		[self addSublayer:rLayer];
		self.ratingLayer = rLayer;
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
	NSRect rect = self.frame;
	
	rect.origin.x += self.imageLayer.frame.origin.x;
	rect.origin.y += self.imageLayer.frame.origin.y;
	rect.size = self.imageLayer.frame.size;
	
	return rect;
}


#pragma mark -
- (void)display{
	[self.glossLayer setNeedsDisplay];
	
	// make sure indication layer displays correctly
	if(!acceptingOnDrop)
		self.indicationLayer.type = [[self representedObject] gridStatus];
}


- (void)layoutSublayers{
	if(acceptingOnDrop) return;
		
	// Border between top of layer and top of image
	float topBorder = 7;
	
	// Height of title string
	float titleHeight = 16;
	// Space between bottom of image and top of title string
	float titleImageSpacing = 17;
	
	
	float subtitleHeight = 11;
	float subtitleWidth = 56;
	
	// Space between left layer border and left side of image
	float imageBorderLeft = 13;
	// Space between right side of image and right border of layer
	float imageBorderRight = 13;

	// calculate area where image is placed
	imageContainerRect = NSMakeRect(0+imageBorderLeft, 0+topBorder, self.bounds.size.width-imageBorderLeft-imageBorderRight, self.bounds.size.height - titleHeight-titleImageSpacing-subtitleHeight-topBorder);

	// calculate rect for title layer
	titleRect = CGRectMake(self.bounds.origin.x, imageContainerRect.origin.y+imageContainerRect.size.height+titleImageSpacing, self.bounds.size.width, titleHeight);
	// calculate rect for rating layer
	ratingRect = RoundCGRect(CGRectMake(self.bounds.origin.x+(self.bounds.size.width-subtitleWidth)/2, titleRect.origin.y+titleRect.size.height, subtitleWidth, subtitleHeight));
	// make sure it has correct size to prevent sub pixel stuff
	ratingRect.size = CGSizeMake(55, 11);
	
	// Calculated size of image in container frame
	float width, height;
	if(self.imageRatio<1){ // width > height
		width = imageContainerRect.size.width;
		height = width*self.imageRatio;
	} else {
		height = imageContainerRect.size.height;
		width = height/self.imageRatio;			
	}
	
	// Determine actual frame for image
	CGRect coverImageRect = RoundCGRect(CGRectMake(imageContainerRect.origin.x+(imageContainerRect.size.width-width)/2, imageContainerRect.origin.y+imageContainerRect.size.height-height, width, height));
	
	// set background colors for debug (to point out the various frames)
	BOOL debug_colors = ([[NSUserDefaults standardUserDefaults] objectForKey:@"debug_showGridFrameBorder"] && [[NSUserDefaults standardUserDefaults] boolForKey:@"debug_showGridFrameBorder"]);
	titleLayer.backgroundColor = debug_colors?[[NSColor greenColor] CGColor]:NULL;
	imageLayer.backgroundColor = debug_colors?[[NSColor blueColor] CGColor]:NULL;
	ratingLayer.backgroundColor = debug_colors?[[NSColor orangeColor] CGColor]:NULL;
	self.backgroundColor = debug_colors?[[NSColor yellowColor] CGColor]:NULL;
	
	// update layer frames
	titleLayer.frame = titleRect;
	ratingLayer.frame = ratingRect;
	self.glossLayer.frame = coverImageRect;
	self.imageLayer.frame = coverImageRect; 
	
	NSWindow* win = [self.gridView window];
	
	// determine states that affect selection display
	BOOL selectionInactive = (![win isMainWindow] && [win firstResponder] == self.gridView);
	BOOL selectionHidden = !self.selected;
	self.selectionLayer.isInactive = selectionInactive;

	CGRect selectionRect = CGRectInset(coverImageRect, -6, -6);
	if((self.selected && !CGRectEqualToRect(self.selectionLayer.frame, selectionRect)) ||
	   (self.selected && self.selectionLayer.isInactive != selectionInactive)){
		[self.selectionLayer setNeedsDisplay];
	}
	
	self.selectionLayer.frame = selectionRect;
	self.selectionLayer.hidden = selectionHidden;
	self.indicationLayer.frame = CGRectInset(coverImageRect, 1, 1);
}

#pragma mark -
#pragma mark IKSGridItemLayerEventProtocol
- (BOOL)mouseDown:(NSEvent*)theEvent{	
	[self _valuesDidChange];
	
	// update self, just in case
	[self setNeedsLayout];
	[self setNeedsDisplay];

	NSPoint p = [self.gridView convertPoint:[theEvent locationInWindow] fromView:nil];
	p.y -= self.frame.origin.y;
	p.x -= self.frame.origin.x;
	
	
	if(NSPointInRect(p, NSRectFromCGRect(ratingRect))){
		isEditingRating = YES;
		
		[self setRatingWithPoint:p pressed:YES];
		
		return YES;
	}
	
	isEditingRating = NO;
	
	if(NSPointInRect(p, self.titleLayer.frame)){
		if ([theEvent clickCount]==2 ){
			CGRect fieldFrame = self.titleLayer.frame;
			
			fieldFrame.size.height += 1;

			OEFieldEditor* fieldEditor = [self.gridView fieldEditorForFrame:fieldFrame ofLayer:self];
			[self setUpFieldEditor:fieldEditor];
			
			return YES;
		} 
	}
	
	return NO;
}

- (BOOL)mouseDragged:(NSEvent*)theEvent{
	NSPoint p = [self.gridView convertPoint:[theEvent locationInWindow] fromView:nil];
	p.y -= self.frame.origin.y;
	p.x -= self.frame.origin.x;
	if(isEditingRating){
		[self setRatingWithPoint:p pressed:YES];
		return YES;
	}
	isEditingRating = NO;
	
	return NO;
}

- (BOOL)mouseEntered:(NSEvent*)theEvent{
	return NO;
}

- (BOOL)mouseExited:(NSEvent*)theEvent{
	return NO;
}

- (BOOL)mouseMoved:(NSEvent*)theEvent{
	return NO;
}

- (BOOL)mouseUp:(NSEvent*)theEvent{
	NSPoint p = [self.gridView convertPoint:[theEvent locationInWindow] fromView:nil];
	p.y -= self.frame.origin.y;
	p.x -= self.frame.origin.x;
	
	if(isEditingRating){
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
- (NSImage*)dragImage{
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
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender{
	// cancel if we are already working on a drop, when another comes in
	// (is that even possible?)
	if(acceptingOnDrop) return NSDragOperationNone;
	
	// Try to get an image directly from pasteboard
	id proposedImageRepresentation;
	NSArray* images = [[sender draggingPasteboard] readObjectsForClasses:[NSArray arrayWithObject:[NSImage class]] options:nil];
	if([images count]==0){
		// Didn't get image, look for files
		images = [[sender draggingPasteboard] readObjectsForClasses:[NSArray arrayWithObject:[NSURL class]] options:nil];
		if(images==0){
			return NSDragOperationNone;	
		}		
		
		// got a file url, check if it has image suffix
		NSURL* url = [images objectAtIndex:0];
		if([url hasImageSuffix]){
			proposedImageRepresentation = url;
		} else {
			return NSDragOperationNone;
		}
	} else {
		proposedImageRepresentation = [images objectAtIndex:0];
	}

	acceptingOnDrop = YES;

	// wait a while to prevent animation when dragging is just dragging by
	float dropAnimatioTimernDelay = [[NSUserDefaults standardUserDefaults] floatForKey:@"debug_drop_animation_delay"];
	dropAnimationDelayTimer = [[NSTimer scheduledTimerWithTimeInterval:dropAnimatioTimernDelay target:self selector:@selector(_displayOnDrop:) userInfo:proposedImageRepresentation repeats:NO] retain];	
	
	return NSDragOperationGeneric;
}

 - (void)_displayOnDrop:(id)sender{
	if(!acceptingOnDrop)
		return;

	 NSImage* proposedCoverImage;
	 
	 id userInfo = [sender userInfo];
	 // check if we need to load image from url
	 if([userInfo isKindOfClass:[NSURL class]]){
		 proposedCoverImage = [[[NSImage alloc] initWithContentsOfURL:userInfo] autorelease];
		 if(!proposedCoverImage){
			// could not load image, cancel
			 acceptingOnDrop = NO;
			 
			 return;
		 }
	 } else {
		 proposedCoverImage = userInfo;
	 }
	 
	 // display drop acceptance
	 self.indicationLayer.type = 3;
	 
	 // calculate aspect ration of new cover image
	 float rat = [proposedCoverImage size].height/[proposedCoverImage size].width;
	 float width, height;
	 if(rat<1){ // width > height
		 width = imageContainerRect.size.width;
		 height = width*rat;
	 } else {
		 height = imageContainerRect.size.height;
		 width = height/rat;			
	 }

	 CALayer* newImageLayer = [CALayer layer];
	 newImageLayer.contents = proposedCoverImage;
	 newImageLayer.geometryFlipped = YES;
	 newImageLayer.transform = CATransform3DMakeScale(1, -1, 1);
	 newImageLayer.contentsGravity = kCAGravityResize;
	 newImageLayer.delegate = self;

	 float startWidth =  self.gridView.itemSize.width*[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_startWidthFac"];
	 float startHeight = startWidth*rat;
	 newImageLayer.opacity = [[NSUserDefaults standardUserDefaults] floatForKey:@"debug_opacityStart"];
	 newImageLayer.frame = CGRectMake((self.frame.size.width-startWidth)/2, (self.frame.size.height-startHeight)/2, startWidth, startHeight);

	 [self insertSublayer:newImageLayer above:self.imageLayer];
	 
	 CGRect newCoverImageRect = RoundCGRect(CGRectMake(imageContainerRect.origin.x+(imageContainerRect.size.width-width)/2, imageContainerRect.origin.y+imageContainerRect.size.height-height, width, height));
	 
	 [self resizeLayer:self.imageLayer to:newCoverImageRect.size];
	 [self resizeLayer:self.glossLayer to:newCoverImageRect.size];
	 [self resizeLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).size];		
	 [self resizeLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).size];
	 
	 [self fadeOpacityOfLayer:newImageLayer to:[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_opacityEnd"]];
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
- (void)draggingExited:(id < NSDraggingInfo >)sender{
	if(!acceptingOnDrop) return;	
	
	if(dropAnimationDelayTimer){ 
		[dropAnimationDelayTimer invalidate]; 
		[dropAnimationDelayTimer release]; 
		dropAnimationDelayTimer = nil; 
	}
	
	float width, height;
	if(self.imageRatio<1){ // width > height
		width = imageContainerRect.size.width;
		height = width*self.imageRatio;
	} else {
		height = imageContainerRect.size.height;
		width = height/self.imageRatio;			
	}

	// Space between left layer border and left side of image
	float imageBorderLeft = 13;
	CGRect newCoverImageRect = RoundCGRect(CGRectMake(imageBorderLeft+(imageContainerRect.size.width-width)/2, imageContainerRect.origin.y+imageContainerRect.size.height-height, width, height));
	
	NSUInteger index = [[self sublayers] indexOfObjectIdenticalTo:self.imageLayer]+1;
	CALayer* newImageLayer = [[self sublayers] objectAtIndex:index];
	if([newImageLayer isKindOfClass:[CALayer class]]){
		[self resizeLayer:newImageLayer to:CGSizeMake(0, 0)];
		[self moveLayer:newImageLayer to:CGPointMake(self.frame.size.width/2, self.frame.size.height/2) centered:YES];
	}
		
	//	[self moveLayer:self.imageLayer to:newCoverImageRect.origin];
	[self resizeLayer:self.imageLayer to:newCoverImageRect.size];	
	[self resizeLayer:self.glossLayer to:newCoverImageRect.size];	
	[self resizeLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).size];		
	[self resizeLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).size];
	
	[self fadeOpacityOfLayer:newImageLayer to:0.0];
	
	[self moveLayer:self.imageLayer to:newCoverImageRect.origin centered:NO];
	[self moveLayer:self.glossLayer to:newCoverImageRect.origin centered:NO];
	[self moveLayer:self.selectionLayer to:CGRectInset(newCoverImageRect, -6, -6).origin centered:NO];
	[self moveLayer:self.indicationLayer to:CGRectInset(newCoverImageRect, 1, 1).origin centered:NO];
	
	
	self.indicationLayer.type = [[self representedObject] gridStatus];
	
	acceptingOnDrop = NO;
}

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender{
	if(acceptingOnDrop) return NSDragOperationGeneric;
	
	return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender{
	self.indicationLayer.type = [[self representedObject] gridStatus];
	
	acceptingOnDrop = NO;
	NSUInteger index = [[self sublayers] indexOfObjectIdenticalTo:self.imageLayer]+1;
	CALayer* newImageLayer = [[self sublayers] objectAtIndex:index];
	if([newImageLayer isKindOfClass:[CALayer class]]){
		
		id <CoverGridDataSourceItem> rObject = self.representedObject;
		[rObject setGridImage:((CALayer*) newImageLayer).contents];
		
		newImageLayer.shadowColor = self.imageLayer.shadowColor;
		newImageLayer.shadowOffset = self.imageLayer.shadowOffset;
		newImageLayer.shadowOpacity = self.imageLayer.shadowOpacity;
		newImageLayer.shadowRadius = self.imageLayer.shadowRadius;
				
		[self.imageLayer removeFromSuperlayer];
		self.imageLayer = (CALayer*)newImageLayer;
		
		[self _valuesDidChange];
	}
	
	return YES;
}
#pragma mark -
#pragma mark "Cell" methods
- (void)setRepresentedObject:(id)robJect{
	if((robJect != self.representedObject)){
		[super setRepresentedObject:robJect];
		[self _valuesDidChange];
	}
}
#pragma mark -
#pragma mark ValueChanging
- (void)setSelected:(BOOL)selectedFlag{
	BOOL redraw = selectedFlag && selectedFlag!=self.selected;
	
	[super setSelected:selectedFlag];

	self.selectionLayer.hidden = !selectedFlag;
	
	NSWindow* win = [self.gridView window];
	BOOL selectionInactive = (![win isMainWindow] && [win firstResponder] == self.gridView);
	redraw |= (self.selectionLayer.isInactive != selectionInactive);
	self.selectionLayer.isInactive = selectionInactive;
	
	if(redraw){
		[self.selectionLayer setNeedsDisplay];
	}
}

- (void)beginValueChange{
	isChangingValues = YES;
	
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	
	// hide layers that might take long to change value, or look weired without those that 
	self.imageLayer.hidden = YES;
	
	self.glossLayer.hidden = YES;
	self.indicationLayer.hidden = YES;
	self.selectionLayer.hidden = YES;
	
	[CATransaction commit];
}

- (void)endValueChange{
	
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	// show layers that have previously been hidden
	self.imageLayer.hidden = NO;
	
	self.glossLayer.hidden = NO;
	self.indicationLayer.hidden = NO;
	
	[CATransaction commit];
	
	isChangingValues = NO;
}
#pragma mark -
#pragma mark Helpers and private methods
- (void)setRatingWithPoint:(NSPoint)p pressed:(BOOL)pressed{
	float pos = p.x-ratingRect.origin.x;
	
	float stepWidth = 55/5;
	int rating = 0;
	if(pos<=0){ rating =  0; }
	else if(pos>=55-stepWidth){ rating = 5; }
	else rating = ((pos/stepWidth)+1);
	
	[ratingLayer setRating:rating pressed:pressed];
	id <CoverGridDataSourceItem> rObject = self.representedObject;
	[rObject setGridRating:rating];
}

- (void)moveLayer:(CALayer*)layer to:(CGPoint)point centered:(BOOL)centered{
    CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"position"];
    animation.fromValue = [layer valueForKey:@"position"];

	if(!centered){
		point.x += layer.frame.size.width/2;
		point.y += layer.frame.size.height/2;
	}

    animation.toValue = [NSValue valueWithPoint:NSPointFromCGPoint(point)];
	animation.delegate = self;

    layer.position = point;

    [layer addAnimation:animation forKey:@"position"];
}

- (void)resizeLayer:(CALayer*)layer to:(CGSize)size{
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

- (void)fadeOpacityOfLayer:(CALayer*)layer to:(float)newOpacity{
	CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"opacity"];
	
	animation.fromValue = [NSNumber numberWithFloat:layer.opacity];
	animation.toValue = [NSNumber numberWithFloat:newOpacity];
	
	layer.opacity = newOpacity;
	animation.delegate = self;
	
	[layer addAnimation:animation forKey:@"opacity"];
}

- (void)_valuesDidChange{
	id <CoverGridDataSourceItem> rObject = self.representedObject;
	
	NSImage* coverImage = [rObject gridImage];
	float r = coverImage?[coverImage size].height/[coverImage size].width:1.365385;
	if(r!=self.imageRatio) [self setNeedsLayout];
	self.imageRatio = r;
	
	self.titleLayer.string = [rObject gridTitle];
	self.imageLayer.contents = coverImage;
	
	if(coverImage==nil){
		CoverGridNoArtwork* noArtworkLayer;
		if([self.imageLayer.sublayers count]==0){
			noArtworkLayer = [CoverGridNoArtwork layer];
						
			noArtworkLayer.frame = CGRectMake(0, 0, self.imageLayer.bounds.size.width, self.imageLayer.bounds.size.height);
			noArtworkLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
			noArtworkLayer.delegate = self;
			
			[self.imageLayer addSublayer:noArtworkLayer];
			
			[noArtworkLayer display];
		}
	} else if([self.imageLayer.sublayers count]!=0){
		[[[self.imageLayer sublayers] objectAtIndex:0] removeFromSuperlayer];
	}
	
	[self.ratingLayer setRating:[rObject gridRating] pressed:NO];
	
	self.indicationLayer.type = [rObject gridStatus];
}

#pragma mark -
#pragma mark Animation stuff
- (id < CAAction >)actionForLayer:(CALayer *)layer forKey:(NSString *)key{
	if(layer == self.ratingLayer)
	if(isEditingRating && layer == self.ratingLayer && [key isEqualTo:@"contents"])
		return nil;
	
	return (id < CAAction >)[NSNull null];
}

- (void)animationDidStop:(CAAnimation *)theAnimation finished:(BOOL)flag{
	if(self.selected)
		[self.selectionLayer setNeedsDisplay];

	[self setNeedsDisplay];
}

#pragma mark -
- (void)setUpFieldEditor:(OEFieldEditor*)fieldEditor{
	id <CoverGridDataSourceItem> rObject = self.representedObject;
	[fieldEditor setString:[rObject gridTitle]];
	
	[fieldEditor setAlignment:NSCenterTextAlignment];
	[fieldEditor setBorderColor:[NSColor blackColor]];
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];
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
- (void)controlTextDidEndEditing:(NSNotification *)aNotification{
	// NSLog(@"control text did end editing");
	
	NSTextField* field = [aNotification object];
	OEFieldEditor* fieldEditor = (OEFieldEditor*)[field superview];
	
	[fieldEditor setFrameSize:NSMakeSize(0, 0)];
	[fieldEditor setHidden:YES];
	
	NSString* newTitle = [fieldEditor string];
	id <CoverGridDataSourceItem> rObject = [self representedObject];
	if([newTitle isNotEqualTo:@""] && [newTitle isNotEqualTo:[rObject gridTitle]]){
		
		
		[rObject setGridTitle:newTitle];
		[self _valuesDidChange];
		
		[NSTimer scheduledTimerWithTimeInterval:0.3 target:self.gridView selector:@selector(reloadData) userInfo:nil repeats:NO];
	}
	
	[fieldEditor setDelegate:nil];
}
#pragma mark -

@end
