//
//  OEControlerImageView.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

@interface OEControlerImageView : NSView {
	NSImage* image;
	
	NSPoint ringPosition;
	float overlayAlpha, ringAlpha;
}
@property (nonatomic) float overlayAlpha, ringAlpha;
@property (nonatomic) NSPoint ringPosition;
@property (nonatomic, retain) NSImage* image;
@end
