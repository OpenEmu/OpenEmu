//
//  OEThumbstickControl.h
//  iPhoneRemote
//
//  Created by Josh Weinberg on 9/8/10.
//  Copyright 2010 OpenEmu. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

enum OEThumbstickEvents
{
	OEThumbstickReleasedUp    = 0x00000000,
	OEThumbstickPressedUp     = 0x01000000,
	OEThumbstickReleasedDown  = 0x02000000,
	OEThumbstickPressedDown   = 0x04000000,
	OEThumbstickReleasedLeft  = 0x08000000,
	OEThumbstickPressedLeft   = 0x10000000,
	OEThumbstickReleasedRight = 0x11000000,
	OEThumbstickPressedRight  = 0x12000000,
};

@protocol OEThumbstickControlDelegate
- (void)pressLeft:(id)sender;
- (void)pressRight:(id)sender;
- (void)pressUp:(id)sender;
- (void)pressDown:(id)sender;
- (void)releaseLeft:(id)sender;
- (void)releaseRight:(id)sender;
- (void)releaseUp:(id)sender;
- (void)releaseDown:(id)sender;
@end

@interface OEThumbstickControl : UIView
{
	CGPoint currentPoint;
	BOOL touchDown;
	
	NSInteger lrDir;
	NSInteger udDir;
	
	id <OEThumbstickControlDelegate> delegate;
	CALayer * nub;
}

@property (nonatomic, assign) id delegate;
@end
