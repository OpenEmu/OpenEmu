//
//  OEGridScrollView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.07.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEGridScrollView.h"

@interface OEGridBackgroundLayer : CALayer @end
@interface OEGridScrollView (Private)
- (void)_setup;
@end

@implementation OEGridScrollView
- (id)init {
    self = [super init];
    if (self) {
        [self _setup];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self _setup];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self _setup];
    }
    return self;
}

- (BOOL)isFlipped{
	return NO;
}
- (void)_setup{
	CALayer *layer = [OEGridBackgroundLayer layer];
	self.layer = layer;
	[self setWantsLayer:YES];
	layer.needsDisplayOnBoundsChange = YES;
}

- (void)dealloc {
    [super dealloc];
}
@end

@implementation OEGridBackgroundLayer
static CGImageRef noiseImage = NULL;
static CGPDFDocumentRef lightingImage = NULL;

- (id) init{
	if((self = [super init])){
		// Load noise image if necessary
		if(noiseImage==NULL){
			NSString *path = [[NSBundle mainBundle] pathForResource:@"noise" ofType:@"png"];
			
			CGImageSourceRef imageSource = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:path], NULL);
			if(imageSource){
				noiseImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
				CFRelease(imageSource);
			}
		}
		
		// Load lighting image if necessary
		if(lightingImage==NULL){	
			NSString *path = [[NSBundle mainBundle] pathForResource:@"background_lighting" ofType:@"pdf"];
			lightingImage = CGPDFDocumentCreateWithURL((CFURLRef)[NSURL fileURLWithPath:path]);
		}
	}
	return self;
}

- (id<CAAction>)actionForKey:(NSString *)event{
	return nil;
}

- (void)drawInContext:(CGContextRef)context{
	// retreive background image
	NSRect lightningRect = [self bounds];
	
	CGContextSaveGState(context);
	
	CGPDFPageRef page = CGPDFDocumentGetPage(lightingImage, 1);
	CGRect pdfRect = CGPDFPageGetBoxRect(page, kCGPDFCropBox);
	
	// Stretched lightning
	//	lightningRect.size.height = bounds.size.height;
	// CGContextTranslateCTM(context, 0, -NSMinY(visibleRect));
	
	// Static lightning
	// nothing to do for this
	
	float widthScale = lightningRect.size.width / pdfRect.size.width;
	float heightScale = lightningRect.size.height / pdfRect.size.height;
	
	// Flip
	//	CGContextScaleCTM(context, 1, -1);
	//	CGContextTranslateCTM(context, 0, -[self bounds].size.height);
	
	
	CGContextScaleCTM (context, widthScale, heightScale);
	CGContextDrawPDFPage(context, page);
	
	CGContextRestoreGState(context);
	
	// retrieve noise image size
	float width = (float) CGImageGetWidth(noiseImage);
	float height = (float) CGImageGetHeight(noiseImage);
	
	//compute coordinates to fill visible rect
	float left, top, right, bottom;
	
	/* Scrolling Noise:*
	 top = bounds.size.height - NSMaxY(visibleRect);
	 top = fmod(top, height);
	 top = height - top;
	 
	 right = NSMaxX(visibleRect);
	 bottom = -height;*/
	
	
	/* Static Noise:*/
	// top = bounds.size.height - NSMaxY(visibleRect);
	top = 0;
	top = height - top;
	
	right = NSMaxX(lightningRect);
	bottom = -height;
	
	// tile the image and take in account the offset to 'emulate' a scrolling background
	for (top = lightningRect.size.height-top; top>bottom; top -= height){
		for(left=0; left<right; left+=width){
			CGContextDrawImage(context, CGRectMake(left, top, width, height), noiseImage);
		}
	}
}

@end
