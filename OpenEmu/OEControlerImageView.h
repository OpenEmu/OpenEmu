//
//  OEControlerImageView.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface OEControlerImageView : NSView {
	NSImage* image;
	
	NSPoint highlightedButtonPoint;
}
@property (nonatomic, retain) NSImage* image;
@end
