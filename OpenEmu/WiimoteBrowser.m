//
//  WiimoteBrowser.m
//  WiimoteFramework
//
//  Created by Christoph Leimbrock on 27.07.09.
//  Copyright 2009 Christoph Leimbrock. All rights reserved.
//

#import "WiimoteBrowser.h"


@implementation WiimoteBrowser
- (id) init{
	self = [super init];
	if (self != nil) {
		_maxWiimotes = -1;
	}
	return self;
}

- (int)maxWiimoteCount{
	return _maxWiimotes;
}

- (void)setMaxWiimoteCount:(int)newMax{
	_maxWiimotes = newMax;
}

@synthesize delegate;

- (void)startSearch{
	if(![self delegate]){ // check if delegate is set
		NSLog(@"Error: Can't run WiimoteBrowser without delegate!");
		return;
	}
	
	if(!IOBluetoothLocalDeviceAvailable()){ // check if Bluetooth hardware is available
		if([[self delegate] respondsToSelector:@selector(wiimoteBrowserSearchFailedWithError:)])
			[[self delegate] wiimoteBrowserSearchFailedWithError:0];

		return;
	}

	if(_isSearching) // already searching, just igone startSearch ;)
		return;

	
	_isSearching = YES;
	if([[self delegate] respondsToSelector:@selector(wiimoteBrowserWillSearch)])
		[[self delegate] performSelector:@selector(wiimoteBrowserWillSearch)];
	
	_inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
	[_inquiry setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
	[_inquiry setInquiryLength:20];
	[_inquiry setUpdateNewDeviceNames:NO];

	IOReturn status = [_inquiry start];
	if (status == kIOReturnSuccess) {

	} else { // not likely to happen, but we handle it anyway
		NSLog(@"Error: Inquiry did not start, error %d", status);
		[_inquiry setDelegate:nil];
		_inquiry = nil;
		
		_isSearching = FALSE;
		
		if([[self delegate] respondsToSelector:@selector(wiimoteBrowserSearchFailedWithError:)])
			[[self delegate] performSelector:@selector(wiimoteBrowserSearchFailedWithError:)];
	}		
}

- (void)stopSearch{}

- (NSArray*)discoveredDevices{
	return _discoveredDevices;
}
#pragma mark -
#pragma mark privat methods
- (NSArray*)_convertFoundDevicesToWiimotes:(NSArray*)foundDevices{
	NSMutableArray* wiimotes = [[NSMutableArray alloc] init];
	
	Wiimote* wiimo = nil;
	for(IOBluetoothDevice* btDev in foundDevices){
		wiimo = [[Wiimote alloc] initWithDevice:btDev];		
		[wiimotes addObject:wiimo];
	}
	
	return wiimotes;
}

#pragma mark -
#pragma mark BT Inquiry	Delegates
- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender device:(IOBluetoothDevice *)device{
	// note: never try to connect to the wiimote while the inquiry is still running! (cf apple docs)
	if([[sender foundDevices] count]==_maxWiimotes)
		[_inquiry stop];
}

- (void) deviceInquiryComplete:(IOBluetoothDeviceInquiry*)sender error:(IOReturn)error aborted:(BOOL)aborted{
	NSLog(@"inquiry stopped, was aborted: %d", aborted);
	NSLog(@"We've found: %lu devices, the maxiumum was set to %d", [[sender foundDevices] count], _maxWiimotes);
	_isSearching = FALSE;

	NSArray* results = [self _convertFoundDevicesToWiimotes:[sender foundDevices]];

	_discoveredDevices = results;
	
	if([[self delegate] respondsToSelector:@selector(wiimoteBrowserDidStopSearchWithResults:)])
		[[self delegate] performSelector:@selector(wiimoteBrowserDidStopSearchWithResults:) withObject:results];
	
}
@end