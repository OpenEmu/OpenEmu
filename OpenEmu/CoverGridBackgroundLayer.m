//
//  CoverGridBackgroundLayer.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 04.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "CoverGridBackgroundLayer.h"

// Config for footer image
#define FooterImagePaddingTop 15
#define FooterImagePaddingBottom 15

@implementation CoverGridBackgroundLayer

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
	
	// scale to complete height of gridview

	
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
	
	// retreive noise image size
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