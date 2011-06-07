//
//  OEGradientBackgroundView.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 02.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface OEBackgroundGradientView : NSView {
@private
    NSColor* topColor;
	NSColor* bottomColor;
}
@property (retain, readwrite) NSColor* topColor;
@property (retain, readwrite) NSColor* bottomColor;
@end
