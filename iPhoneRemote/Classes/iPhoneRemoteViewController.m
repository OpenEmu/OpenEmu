//
//  iPhoneRemoteViewController.m
//  iPhoneRemote
//
//  Created by Josh Weinberg on 9/8/10.
//  Copyright 2010 OpenEmu. All rights reserved.
//

#import "iPhoneRemoteViewController.h"
#import "AsyncUdpSocket.h"
#import "OEThumbstickControl.h"

@implementation iPhoneRemoteViewController
@synthesize lrDir, thumbstick;

- (id)initWithAddress:(NSString*)address port:(uint32_t)port;
{
	if ((self = [super initWithNibName:nil bundle:nil]))
	{
		socket = [[AsyncUdpSocket alloc] initWithDelegate:self];
		[socket connectToHost:address onPort:port error:nil];
		
		NSString *h = @"HELLO";
		 NSData *data = [NSData dataWithBytes:[h UTF8String] length:[h length]];
		 if(![socket sendData:data withTimeout:-1 tag:1])
		 {
		 NSLog(@"Couldn't send!");
		 }
		
		/*motion = [[CMMotionManager alloc] init];
		[motion startDeviceMotionUpdatesToQueue:[NSOperationQueue mainQueue] withHandler:^(CMDeviceMotion *mData, NSError *error)
		 {
			 CMAttitude *attitude = [mData attitude];

			 float pitch = [attitude pitch];
			 if (pitch > 0.2)
				 self.lrDir = 1;
			 else if (pitch < -0.2f)
				 self.lrDir = -1;
			 else
				 self.lrDir = 0;
			 
			 //NSLog(@"Motion: %f, %f, %f", [attitude roll], [attitude pitch], [attitude yaw]); 
		 }];*/
	}
	return self;
}

- (void)viewWillAppear:(BOOL)animated
{
	[self.navigationController setNavigationBarHidden:YES];
	[[UIApplication sharedApplication] setStatusBarHidden:YES];
	[super viewWillAppear:animated];
}

- (void)pressDown:(id)sender;
{
	[self pressButton:1];
}

- (void)pressLeft:(id)sender;
{
		[self pressButton:0];
}

- (void)pressRight:(id)sender;
{
		[self pressButton:3];
}

- (void)pressUp:(id)sender;
{
		[self pressButton:2];
}

- (void)releaseDown:(id)sender;
{
	[self releaseButton:1];
}

- (void)releaseLeft:(id)sender;
{	
	[self releaseButton:0];
}

- (void)releaseRight:(id)sender;
{
		[self releaseButton:3];
}

- (void)releaseUp:(id)sender;
{
		[self releaseButton:2];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}



- (void)pressButton:(NSUInteger)button
{
	NSLog(@"Press: %u", button);
	uint8_t byte = 0x80;
	byte |= button;
	NSData *d = [NSData dataWithBytes:&byte length:1];
	[socket sendData:d withTimeout:-1 tag:1];
}

- (void)releaseButton:(NSUInteger)button
{
	NSLog(@"Release: %u", button);
	uint8_t byte = 0x00;
	byte |= button;
	NSData *d = [NSData dataWithBytes:&byte length:1];
	[socket sendData:d withTimeout:-1 tag:1];
}

- (IBAction)sendButtonPress:(id)sender;
{
	[self pressButton:[sender tag]];
}
- (IBAction)sendButtonRelease:(id)sender;
{
	[self releaseButton:[sender tag]];
}

- (void) setLrDir:(NSInteger)newDir
{
	if (newDir != lrDir)
	{
		if (lrDir == -1)
			[self releaseButton:0];
		else if (lrDir == 1)
			[self releaseButton:3];
		lrDir = newDir;
		if (lrDir == -1)
			[self pressButton:0];
		else if (lrDir == 1)
			[self pressButton:3];
	}
}

@end
