//
//  OEUIDrawingUtils.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 19.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEUIDrawingUtils.h"

CGRect RoundCGRect(CGRect imageFrame){
	imageFrame.origin.x = floorf(imageFrame.origin.x);
	imageFrame.origin.y = floorf(imageFrame.origin.y);
	imageFrame.size.width = ceilf(imageFrame.size.width);
	imageFrame.size.height = ceilf(imageFrame.size.height);
	
	return imageFrame;
}

NSRect RoundNSRect(NSRect imageFrame){
	imageFrame.origin.x = floorf(imageFrame.origin.x);
	imageFrame.origin.y = floorf(imageFrame.origin.y);
	imageFrame.size.width = ceilf(imageFrame.size.width);
	imageFrame.size.height = ceilf(imageFrame.size.height);
	
	return imageFrame;
}