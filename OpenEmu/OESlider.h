//
//  OESlider.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"
@interface OESlider : NSSlider {
@private
	IBOutlet NSImageView* minHint;
	IBOutlet NSImageView* maxHint;
}

- (void)loadImages;
@property (assign) IBOutlet NSImageView* minHint;
@property (assign) IBOutlet NSImageView* maxHint;
@end

@interface OESliderCell : NSSliderCell {
@private
    
}

- (NSRect)trackImageRectForState:(OEUIState)state;
- (NSRect)knobImageRectForState:(OEUIState)state;
@end
