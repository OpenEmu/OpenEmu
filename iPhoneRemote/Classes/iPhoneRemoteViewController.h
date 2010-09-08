//
//  iPhoneRemoteViewController.h
//  iPhoneRemote
//
//  Created by Josh Weinberg on 9/8/10.
//  Copyright 2010 OpenEmu. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>

@class AsyncUdpSocket, OEThumbstickControl;

@interface iPhoneRemoteViewController : UIViewController
{
	AsyncUdpSocket *socket;
	CMMotionManager *motion;
	NSInteger lrDir;
	float lastPitch;
	OEThumbstickControl *thumbstick;
}

@property (nonatomic, assign) NSInteger lrDir;
@property (nonatomic, retain) IBOutlet OEThumbstickControl *thumbstick;
- (id)initWithAddress:(NSString*)address port:(uint32_t)port;
- (IBAction)sendButtonPress:(id)sender;
- (IBAction)sendButtonRelease:(id)sender;
@end

