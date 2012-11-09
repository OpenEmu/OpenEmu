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
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>

#define SynVibrateDuration 0.35

NSString *const OEWiimoteSupportDisabled = @"wiimoteSupporDisabled";

@interface OEWiimoteManager ()

@property (strong) IOBluetoothDeviceInquiry *inquiry;
@property (strong) NSMutableDictionary      *wiiRemotes;
@property NSLock *searching;
@end

@implementation OEWiimoteManager
@synthesize wiiRemotes, searching, inquiry;

+ (void)startSearch
{
    [[self sharedHandler] startSearch];
}

+ (id)sharedHandler
{
    static OEWiimoteManager *sharedHandler = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedHandler = [[OEWiimoteManager alloc] init];
        [sharedHandler setWiiRemotes:[NSMutableDictionary dictionary]];
        [sharedHandler setSearching:[NSLock new]];
        
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(wiimoteDidConnect:) name:OEWiimoteDidConnectNotificationName object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(wiimoteDidDisconnect:) name:OEWiimoteDidDisconnectNotificationName object:nil];
        
        [sharedHandler startSearch];
        
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(bluetoothDidPowerOn:) name:IOBluetoothHostControllerPoweredOnNotification object:nil];

    });
    
    return sharedHandler;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    [self stopSearch];
    
    for(Wiimote *aWiimote in [[self wiiRemotes] allValues])
        [aWiimote disconnect];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)bluetoothDidPowerOn:(NSNotification*)notification
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEWiimoteSupportDisabled])
        [self startSearch];
}

#pragma mark -
- (NSArray *)connectedWiimotes
{
    return [[self wiiRemotes] allValues];
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

- (void)wiimoteDidNotConnect:(NSNotification *)notification
{
    // TODO: actually send notification Wiimote (Wiimote.m)
    NSLog(@"wiimoteDidNotConnect:");
    Wiimote *theWiimote = [notification object];
    [[self wiiRemotes] removeObjectForKey:[theWiimote address]];
    [self startSearch];
}

- (void)wiimoteDidDisconnect:(NSNotification *)notification
{
    Wiimote *theWiimote = [notification object];
    [[self wiiRemotes] removeObjectForKey:[theWiimote address]];
    [self startSearch];
}

#pragma mark - Former WiimoteBrowser -
- (void)startSearch
{
	if(![[self searching] tryLock])
		return;
    
    NSLog(@"Searching for Wiimotes");
	
	inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
	[inquiry setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
	[inquiry setInquiryLength:60];
	[inquiry setUpdateNewDeviceNames:NO];
    
	IOReturn status = [inquiry start];
	if (status != kIOReturnSuccess)
    {
		[inquiry setDelegate:nil];
		inquiry = nil;
        [[self searching] unlock];
        NSLog(@"Error: Inquiry did not start, error %d", status);
	}
}

- (void)addWiimoteWithDevice:(IOBluetoothDevice*)device
{
    if([[self wiiRemotes] objectForKey:[device addressString]])
        return;
 
    Wiimote *wiimote = [[Wiimote alloc] initWithDevice:device];
    [[self wiiRemotes] setObject:wiimote forKey:[wiimote address]];

    [wiimote setRumbleActivated:YES];
    [wiimote setExpansionPortEnabled:YES];
    
    NSInteger count = [[[self wiiRemotes] allKeys] count];
    [wiimote setLED1:count>0&&count<4 LED2:count>1&&count<5 LED3:count>2&&count<6 LED4:count>3];
    [wiimote connect];
    
}

- (void)stopSearch
{
    if(![[self searching] tryLock])
       [inquiry stop];
    [[self searching] unlock];
}

#pragma mark - IOBluetoothDeviceInquiry Delegates -
- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender device:(IOBluetoothDevice *)device
{
	// note: never try to connect to the wiimote while the inquiry is still running! (cf apple docs)
    [[self inquiry] stop];
}

- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry*)sender error:(IOReturn)error aborted:(BOOL)aborted
{
    [[sender foundDevices] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [self addWiimoteWithDevice:obj];
    }];

    if(aborted && [[sender foundDevices] count])
        [inquiry start];
    else if(!aborted)
        [self stopSearch];
}
@end

