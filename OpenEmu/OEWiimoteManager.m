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

#define MaximumWiimotes 7
#define SynVibrateDuration 0.35

NSString *const OEWiimoteSupportDisabled = @"wiimoteSupporDisabled";

@interface OEWiimoteManager ()
{
    BOOL _isSearching;
	IOBluetoothDeviceInquiry* _inquiry;
	int _maxWiimotes; // maximum number of wiimotes to discover (-1 for unlimited)

	NSArray* _discoveredDevices;
}
@property(strong) NSMutableArray *wiiRemotes;
@end

@implementation OEWiimoteManager
@synthesize wiiRemotes;

+ (void)search
{
    NSLog(@"Searching for Wiimotes");
    [[self sharedHandler] startSearch];
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
        
        [sharedHandler setMaxWiimoteCount:1];        
        [sharedHandler startSearch];
        
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:sharedHandler selector:@selector(bluetoothDidPowerOn:) name:IOBluetoothHostControllerPoweredOnNotification object:nil];

    });
    
    return sharedHandler;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    [self stopSearch];
    
    for(Wiimote *aWiimote in [self wiiRemotes])
        [aWiimote disconnect];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)bluetoothDidPowerOn:(NSNotification*)notification
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEWiimoteSupportDisabled])
        [self startSearch];
}

- (id)init
{
    self = [super init];
    if (self) {
        _maxWiimotes = -1;
    }
    return self;
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
        [wiimote setRumbleActivated:YES];
        [wiimote setExpansionPortEnabled:YES];

        NSInteger count = [[self wiiRemotes] count];
        [wiimote setLED1:count>0&&count<4 LED2:count>1&&count<5 LED3:count>2&&count<6 LED4:count>3];
        [wiimote connect];
    }];
    
    if([discoveredDevices count] && [[self wiiRemotes] count] < MaximumWiimotes)
        [self startSearch];
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
    [self startSearch];
}

#pragma mark - Former WiimoteBrowser -

- (int)maxWiimoteCount
{
	return _maxWiimotes;
}

- (void)setMaxWiimoteCount:(int)newMax
{
	_maxWiimotes = newMax;
}


- (void)startSearch
{
    
	if(_isSearching) // already searching, just igone startSearch ;)
		return;
	
	_isSearching = YES;
	if([self respondsToSelector:@selector(wiimoteBrowserWillSearch)])
		[self performSelector:@selector(wiimoteBrowserWillSearch)];
	
	_inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
	[_inquiry setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
	[_inquiry setInquiryLength:20];
	[_inquiry setUpdateNewDeviceNames:NO];
    
	IOReturn status = [_inquiry start];
	if (status != kIOReturnSuccess)
    {
        // DLog(@"Error: Inquiry did not start, error %d", status);
		[_inquiry setDelegate:nil];
		_inquiry = nil;
		
		_isSearching = FALSE;
		
		if([self respondsToSelector:@selector(wiimoteBrowserSearchFailedWithError:)])
			[self performSelector:@selector(wiimoteBrowserSearchFailedWithError:)];
	}
}

- (void)stopSearch
{}

- (NSArray*)discoveredDevices
{
	return _discoveredDevices;
}
#pragma mark -
#pragma mark privat methods
- (NSArray*)_convertFoundDevicesToWiimotes:(NSArray*)foundDevices
{
	NSMutableArray* wiimotes = [[NSMutableArray alloc] init];
	
	Wiimote* wiimo = nil;
	for(IOBluetoothDevice* btDev in foundDevices)
    {
		wiimo = [[Wiimote alloc] initWithDevice:btDev];
		[wiimotes addObject:wiimo];
	}
	
	return wiimotes;
}

#pragma mark -
#pragma mark BT Inquiry	Delegates
- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender device:(IOBluetoothDevice *)device
{
	// note: never try to connect to the wiimote while the inquiry is still running! (cf apple docs)
	if([[sender foundDevices] count]==_maxWiimotes)
		[_inquiry stop];
}

- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry*)sender error:(IOReturn)error aborted:(BOOL)aborted
{
	// DLog(@"inquiry stopped, was aborted: %d", aborted);
	// DLog(@"We've found: %lu devices, the maxiumum was set to %d", [[sender foundDevices] count], _maxWiimotes);
	_isSearching = FALSE;
    
	NSArray* results = [self _convertFoundDevicesToWiimotes:[sender foundDevices]];
    
	_discoveredDevices = results;
	
	if([self respondsToSelector:@selector(wiimoteBrowserDidStopSearchWithResults:)])
		[self performSelector:@selector(wiimoteBrowserDidStopSearchWithResults:) withObject:results];
	
}

@end

