//
//  OEBackgroundImageView.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface OEBackgroundImageView : NSView {
@private
    NSImage* image;
	NSColor* topLineColor;
}
@property (retain, readwrite) NSColor* topLineColor;
@property (retain, readwrite) NSImage* image;
@end
