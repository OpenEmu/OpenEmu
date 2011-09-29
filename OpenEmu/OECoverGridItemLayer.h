//
//  CoverGridItemLayer.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "IKSGridItemLayer.h"

#import <QuartzCore/CoreAnimation.h>

#import "OECoverGridSelectionLayer.h"
#import "OECoverGridGlossLayer.h"
#import "OECoverGridIndicationLayer.h"
#import "OECoverGridRatingLayer.h"
@interface OECoverGridItemLayer : IKSGridItemLayer <NSTextViewDelegate> {
@private
	CGSize lastSize;
	// often reused rects
    CGRect titleRect, ratingRect;
	CGRect imageContainerRect;
	
	float imageRatio;	// keeps aspect ratio of artwork image
	NSSize lastImageSize;
	
	OECoverGridSelectionLayer *selectionLayer;	// Layer for selection indicator
	CALayer *glossLayer;						// Effect overlay for artwork image 
	OECoverGridIndicationLayer *indicationLayer;	// Displays status of rom (missing, accepting artwork, ....)
	CALayer *imageLayer;						// Draws artwork and black stroke around it
	CATextLayer* titleLayer;
	OECoverGridRatingLayer* ratingLayer;			// Displays star rating (interaction is done on item layer)
	
	BOOL acceptingOnDrop; // keeps track of "on drop" state
	NSTimer* dropAnimationDelayTimer;
	
	BOOL isEditingRating;
	
	NSImage* image;
	BOOL reloadImage;
}

@property BOOL isReloading;
@property (readwrite, retain, nonatomic) NSImage* image;
@property float imageRatio;

@property (readwrite, retain) OECoverGridSelectionLayer *selectionLayer;
@property (readwrite, retain) CALayer *glossLayer;
@property (readwrite, retain) OECoverGridIndicationLayer *indicationLayer;
@property (readwrite, retain) CALayer *imageLayer;
@property (readwrite, retain) CATextLayer *titleLayer;
@property (readwrite, retain) OECoverGridRatingLayer *ratingLayer;
@end