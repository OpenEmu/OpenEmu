/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import <Foundation/Foundation.h>
#import "IKSGridItemLayer.h"

#import <QuartzCore/CoreAnimation.h>

#import "OECoverGridSelectionLayer.h"
#import "OECoverGridGlossLayer.h"
#import "OECoverGridIndicationLayer.h"
#import "OECoverGridRatingLayer.h"
@interface OECoverGridItemLayer : IKSGridItemLayer <NSTextViewDelegate>
{
@private
    CGSize lastSize;
    // often reused rects
    CGRect titleRect, ratingRect;
    CGRect imageContainerRect;
    
    float imageRatio;                             // keeps aspect ratio of artwork image
    NSSize lastImageSize;
    
    OECoverGridSelectionLayer *selectionLayer;    // Layer for selection indicator
    CALayer *glossLayer;                          // Effect overlay for artwork image
    OECoverGridIndicationLayer *indicationLayer;  // Displays status of rom (missing, accepting artwork, ....)
    CALayer *imageLayer;                          // Draws artwork and black stroke around it
    CATextLayer *titleLayer;
    OECoverGridRatingLayer *ratingLayer;          // Displays star rating (interaction is done on item layer)
    
    BOOL acceptingOnDrop;                         // keeps track of "on drop" state
    NSTimer *dropAnimationDelayTimer;
    
    BOOL isEditingRating;
    
    NSImage *image;
    BOOL reloadImage;
    
    NSInteger representedIndex;
}

@property BOOL isReloading;
@property(readwrite, strong, nonatomic) NSImage *image;
@property float imageRatio;

@property(readwrite, strong) OECoverGridSelectionLayer *selectionLayer;
@property(readwrite, strong) CALayer *glossLayer;
@property(readwrite, strong) OECoverGridIndicationLayer *indicationLayer;
@property(readwrite, strong) CALayer *imageLayer;
@property(readwrite, strong) CATextLayer *titleLayer;
@property(readwrite, strong) OECoverGridRatingLayer *ratingLayer;
@end