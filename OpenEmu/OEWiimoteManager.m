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

#import "OEWiimoteManager.h"
#import "OEHIDManager.h"
#import "OEHIDEvent.h"
#import "OEWiimoteDeviceHandler.h"

#import "NSApplication+OEHIDAdditions.h"

#import <IOBluetooth/IOBluetooth.h>
#define MaximumWiimotes 7
#define SynVibrateDuration 0.35

NSString *const OEWiimoteSupportDisabled = @"wiimoteSupporDisabled";

@interface OEWiimoteManager ()
@property(strong) WiimoteBrowser *browser;
@property(strong) NSMutableArray *wiiRemotes;
@end

@implementation OEWiimoteManager
@synthesize wiiRemotes;
@synthesize browser;

+ (void)search
{
    [[[self sharedHandler] browser] startSearch];
}

+ (id)sharedHandler
{
    static OEWiimoteManager *sharedHandler = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedHandler = [[OEWiimoteManager alloc] init];
        [sharedHandler setWiiRemotes:[NSMutableArray arrayWithCapacity:MaximumWiimotes]];
        
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(wiimoteDidConnect:) name:OEWiimoteDidConnectNotificationName object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(wiimoteDidDisconnect:) name:OEWiimoteDidDisconnectNotificationName object:nil];
        
        WiimoteBrowser *aBrowser =  [[WiimoteBrowser alloc] init];
        [aBrowser setDelegate:sharedHandler];
        [aBrowser setMaxWiimoteCount:1];
        [sharedHandler setBrowser:aBrowser];
        
        [aBrowser startSearch];
        
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(bluetoothDidPowerOn:) name:IOBluetoothHostControllerPoweredOnNotification object:nil];

    });
    
    return sharedHandler;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    [self.browser stopSearch];
    self.browser = nil;
    
    for(Wiimote *aWiimote in [self wiiRemotes])
        [aWiimote disconnect];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)bluetoothDidPowerOn:(NSNotification*)notification
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEWiimoteSupportDisabled])
        [[self browser] startSearch];
}

#pragma mark -

- (NSArray *)connectedWiiRemotes
{
    return [self wiiRemotes];
}

#pragma mark - Wiimote Browser Delegate -
- (void)wiimoteBrowserWillSearch
{
}

- (void)wiimoteBrowserDidStopSearchWithResults:(NSArray*)discoveredDevices
{
    [discoveredDevices enumerateObjectsUsingBlock:
     ^(Wiimote * wiimote, NSUInteger idx, BOOL *stop)
     {
        [[self wiiRemotes] addObject:wiimote];

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
        [[self browser] startSearch];
}

- (void)wiimoteBrowserSearchFailedWithError:(int)code
{
    DLog(@"wiimoteBrowserSearchFailedWithError: %d", code);
}

#pragma mark - Wiimote Notifications -
- (void)wiimoteDidConnect:(NSNotification *)notification
{
    Wiimote *theWiimote = [notification object];
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, SynVibrateDuration * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        [theWiimote setRumbleActivated:NO];
        [theWiimote syncLEDAndRumble];
    });
    
    OEWiimoteDeviceHandler *handler = [OEWiimoteDeviceHandler deviceHandlerWithWiimote:theWiimote];
    OEHIDManager *hidManager = [OEHIDManager sharedHIDManager];
    [theWiimote setHandler:handler];
    [hidManager addDeviceHandler:handler];
}

- (void)wiimoteDidDisconnect:(NSNotification *)notification
{
    Wiimote *theWiimote = [notification object];
    [[self wiiRemotes] removeObject:theWiimote];
    [[self browser] startSearch];
}
@end

