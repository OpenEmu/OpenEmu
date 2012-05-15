//
//  WiimoteHandler.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 15.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEWiimoteHandler.h"
#import "OEHIDEvent.h"
#import "NSApplication+OEHIDAdditions.h"

#define MaximumWiimotes 2
#define SynVibrateDuration 0.35

@interface OEWiimoteHandler ()
@property (strong) WiimoteBrowser		*browser;
@property (strong) NSMutableArray	*wiiRemotes;
@end
@implementation OEWiimoteHandler

+ (void)search
{
	[[[self sharedHandler] browser] startSearch];
}


+ (id)sharedHandler
{
	static OEWiimoteHandler *sharedHandler = NULL;
	if(!sharedHandler)
	{
		sharedHandler = [[OEWiimoteHandler alloc] init];
		[sharedHandler setWiiRemotes:[NSMutableArray arrayWithCapacity:MaximumWiimotes]];

		WiimoteBrowser *aBrowser =  [[WiimoteBrowser alloc] init];
		[aBrowser setDelegate:sharedHandler];
		[aBrowser setMaxWiimoteCount:1];
		sharedHandler.browser = aBrowser;
		
		[aBrowser startSearch];
		[[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
	}
	return sharedHandler;
}


- (void)applicationWillTerminate:(NSNotification*)notification
{
	NSLog(@"applicationWillTerminate: %@", notification);
	
	[self.browser stopSearch];
	self.browser = nil;
	
	for (Wiimote *aWiimote in [self wiiRemotes]) {
		[aWiimote disconnect];
	}
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}
#pragma mark -
@synthesize wiiRemotes;
- (NSArray*)connectedWiiRemotes
{
	return [self wiiRemotes];
}
#pragma mark - Wiimote Browser
@synthesize browser;
- (void)wiimoteBrowserWillSearch
{
	NSLog(@"wiimoteBrowserWillSearch");
}
- (void)wiimoteBrowserDidStopSearchWithResults:(NSArray*)discoveredDevices
{
	NSLog(@"wiimoteBrowserDidStopSearchWithResults: %ld", [discoveredDevices count]);
	[discoveredDevices enumerateObjectsUsingBlock:^(Wiimote* wiimote, NSUInteger idx, BOOL *stop) {
		[[self wiiRemotes] addObject:wiimote];

		[wiimote setDelegate:self];
		[wiimote setIrSensorEnabled:NO];
		[wiimote setMotionSensorEnabled:NO];
		[wiimote setSpeakerEnabled:NO];
		[wiimote setRumbleActivated:YES];
		
		NSInteger count = [[self wiiRemotes] count];
		[wiimote setLED1:count>0&&count<4 LED2:count>1&&count<5 LED3:count>2&&count<6 LED4:count>3];
		[wiimote connect];
	}];
	
	if([discoveredDevices count] && [[self wiiRemotes] count] < MaximumWiimotes)
		[self.browser startSearch];
}

- (void)wiimoteBrowserSearchFailedWithError:(int)code
{
	NSLog(@"wiimoteBrowserSearchFailedWithError: %d", code);
}


#pragma mark - WiiRemote Handling
- (void)wiimoteDidConnect:(Wiimote*)theWiimote
{
	NSLog(@"wiimoteDidConnect: %@", theWiimote);
	dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, SynVibrateDuration * NSEC_PER_SEC);
	dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
		[theWiimote setRumbleActivated:NO];
		[theWiimote syncLEDAndRumble];
	});
}
- (void)wiimoteDidDisconnect:(Wiimote*)theWiimote
{
	NSLog(@"wiimoteDidDisconnect: %@", theWiimote);
	[self.wiiRemotes removeObject:theWiimote];
	[self.browser startSearch];
}

- (void)wiimote:(Wiimote*)theWiimote didNotConnect:(NSError*)err
{
	NSLog(@"wiimote: %@ didNotConnect: %@", theWiimote, err);
}
- (void)wiimote:(Wiimote*)theWiimote didNotDisconnect:(NSError*)err
{
	NSLog(@"wiimote: %@ didNotDisconnect: %@", theWiimote, err);
}

- (void)wiimoteReportsExpansionPortChanged:(Wiimote*)theWiimote
{
	NSLog(@"wiimoteReportsExpansionPortChanged: %@", theWiimote);
}
- (void)wiimote:(Wiimote*)theWiimote reportsButtonChanged:(WiiButtonType)type isPressed:(BOOL)isPressed
{
	NSLog(@"%@%d", isPressed?@"↓":@"↑", type);
	NSInteger padNumber = [[self connectedWiiRemotes] indexOfObject:theWiimote];
	if(padNumber>=0)
	{
		OEHIDEvent *event = [OEHIDEvent buttonEventWithPadNumber:padNumber timestamp:[NSDate timeIntervalSinceReferenceDate] buttonNumber:type	state:isPressed cookie:0];
		[NSApp postHIDEvent:event];
	}
}
- (void)wiimote:(Wiimote*)theWiimote reportsIrPointMovedX:(float)px Y:(float)py
{
		NSLog(@"wiimote: %@ reportsIrPointMovedX:%f Y: %f", theWiimote, px, py);
}
- (void)wiimote:(Wiimote*)theWiimote reportsJoystickChanged:(WiiJoyStickType)type tiltX:(unsigned short)tiltX tiltY:(unsigned short)tiltY
{
	NSLog(@"wiimote: %@ reportsJoystickChanged:%d tiltX:%d  tiltY: %d", theWiimote, type, tiltX, tiltY);
}
@end
