//
//  OEBackgroundImageView.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEBackgroundImageView.h"


@implementation OEBackgroundImageView
@synthesize image, topLineColor;
- (id)initWithFrame:(NSRect)frame{
    self = [super initWithFrame:frame];
    if (self) {}
	return self;
}

- (void)dealloc{
    [super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect{	
    if(self.image){
		NSRect imgRect = self.bounds;
		[self.image drawInRect:imgRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
	}
}

@end
