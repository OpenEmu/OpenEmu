//
//  OEBackgroundImageView.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface OEBackgroundImageView : NSView
@property (retain, readwrite) NSImage *image;
@property float leftBorder, rightBorder, topBorder, bottomBorder;
@end
