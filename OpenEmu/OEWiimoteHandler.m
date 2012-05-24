/*
 Copyright (c) 2012, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEWiimoteHandler.h"
#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDWiimoteEvent.h"

#import <IOBluetooth/IOBluetooth.h>
#define MaximumWiimotes 7
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
        
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(bluetoothDidPowerOn:) name:IOBluetoothHostControllerPoweredOnNotification object:nil];
	}
	return sharedHandler;
}


- (void)applicationWillTerminate:(NSNotification*)notification
{
	[self.browser stopSearch];
	self.browser = nil;
	
	for (Wiimote *aWiimote in [self wiiRemotes]) {
		[aWiimote disconnect];
	}
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)bluetoothDidPowerOn:(NSNotification*)notification
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:UDWiimoteSupportDisabled])
        [[self browser] startSearch];
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
{}
- (void)wiimoteBrowserDidStopSearchWithResults:(NSArray*)discoveredDevices
{
	[discoveredDevices enumerateObjectsUsingBlock:^(Wiimote* wiimote, NSUInteger idx, BOOL *stop) {
		[[self wiiRemotes] addObject:wiimote];

		[wiimote setDelegate:self];
		[wiimote setIrSensorEnabled:NO];
		[wiimote setMotionSensorEnabled:NO];
		[wiimote setSpeakerEnabled:NO];
		[wiimote setRumbleActivated:YES];
		[wiimote setExpansionPortEnabled:YES];

		NSInteger count = [[self wiiRemotes] count];
		[wiimote setLED1:count>0&&count<4 LED2:count>1&&count<5 LED3:count>2&&count<6 LED4:count>3];
		[wiimote connect];
	}];
	
	if([discoveredDevices count] && [[self wiiRemotes] count] < MaximumWiimotes)
		[self.browser startSearch];
}

- (void)wiimoteBrowserSearchFailedWithError:(int)code
{
	DLog(@"wiimoteBrowserSearchFailedWithError: %d", code);
}
#pragma mark - WiiRemote Handling
- (void)wiimoteDidConnect:(Wiimote*)theWiimote
{
	DLog(@"wiimoteDidConnect: %@", theWiimote);
	dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, SynVibrateDuration * NSEC_PER_SEC);
	dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
		[theWiimote setRumbleActivated:NO];
		[theWiimote syncLEDAndRumble];
	});
}
- (void)wiimoteDidDisconnect:(Wiimote*)theWiimote
{
	DLog(@"wiimoteDidDisconnect: %@", theWiimote);
	[self.wiiRemotes removeObject:theWiimote];
	[self.browser startSearch];
}

- (void)wiimote:(Wiimote*)theWiimote didNotConnect:(NSError*)err
{
	DLog(@"wiimote: %@ didNotConnect: %@", theWiimote, err);
}
- (void)wiimote:(Wiimote*)theWiimote didNotDisconnect:(NSError*)err
{
	DLog(@"wiimote: %@ didNotDisconnect: %@", theWiimote, err);
}

- (void)wiimoteReportsExpansionPortChanged:(Wiimote*)theWiimote
{
	DLog(@"wiimoteReportsExpansionPortChanged: %@", theWiimote);
    switch ([theWiimote expansionType]) {
        case WiiExpansionUnkown:
            NSLog(@"WiiExpansionUnkown");
            break;
        case WiiExpansionClassicController:
            NSLog(@"WiiExpansionClassicController");
            break;
        case WiiExpansionNotInitialized:
            NSLog(@"WiiExpansionNotInitialized");
            break;
        case WiiExpansionNotConnected:
            NSLog(@"WiiExpansionNotConnected");
            break;
        case WiiExpansionNunchuck:
            NSLog(@"WiiExpansionNunchuck");
            break;
            
        default:
            break;
    }
}
- (void)wiimote:(Wiimote*)theWiimote reportsButtonChanged:(WiiButtonType)type isPressed:(BOOL)isPressed
{
	NSInteger padNumber = [[self connectedWiiRemotes] indexOfObject:theWiimote];
	if(padNumber>=0)
	{
		OEHIDWiimoteEvent *event = [OEHIDWiimoteEvent buttonEventWithPadNumber:WiimoteBasePadNumber+padNumber timestamp:[NSDate timeIntervalSinceReferenceDate] buttonNumber:type	state:isPressed cookie:type];
		[NSApp postHIDEvent:event];
	}
}
- (void)wiimote:(Wiimote*)theWiimote reportsIrPointMovedX:(float)px Y:(float)py
{
}
- (void)wiimote:(Wiimote*)theWiimote reportsJoystickChanged:(WiiJoyStickType)type tiltX:(unsigned short)tiltX tiltY:(unsigned short)tiltY
{
}
@end
