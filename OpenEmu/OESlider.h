//
//  OESlider.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"
@interface OESlider : NSSlider 
{
    BOOL hintImagesShowActive;
}
@property (assign) IBOutlet NSImageView* minHint;
@property (assign) IBOutlet NSImageView* maxHint;
@end

@interface OESliderCell : NSSliderCell
- (NSRect)trackImageRectForState:(OEUIState)state;
- (NSRect)knobImageRectForState:(OEUIState)state;
@end
