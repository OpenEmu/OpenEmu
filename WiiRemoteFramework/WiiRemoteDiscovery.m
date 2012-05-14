//
//  WiiRemoteDiscovery.m
//  DarwiinRemote
//
//  Created by Ian Rickard on 12/9/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import "WiiRemoteDiscovery.h"


@implementation WiiRemoteDiscovery

- (id) init
{
	self = [super init];
	
	if (self != nil) {
		// cam: calling IOBluetoothLocalDeviceAvailable has two advantages:
		// 1. it sets up a event source in the run loop (bug for C version of the bluetooth api )
		// 2. it checks for the availability of the BT hardware
		if (IOBluetoothLocalDeviceAvailable () == FALSE)
		{
			[self release];
			self = nil;
			
			[NSException raise:NSGenericException format:@"Bluetooth hardware not available"];
		}		
	}
	
	return self;
}

+ (WiiRemoteDiscovery*) discoveryWithDelegate:(id) delegate
{
	// cam: when using this convention, we must autorelease the returned object
	return [[[WiiRemoteDiscovery alloc] initWithDelegate:delegate] autorelease];
}

- (void) dealloc
{	
	NSLogDebug (@"Wiimote Discovery released");
	[super dealloc];
}

- (id) delegate
{
	return _delegate;
}

- (void) setDelegate:(id) delegate
{
	// only retain a shallow reference to the delegate, could be problematic if the delegate went to be
	// dealloced, but in the most general case, the delegate 'owns' the WiiRemoteDiscovery
	_delegate = delegate;
}

- (IOReturn) start
{
	// cam: check everytime the presence of the bluetooth hardware,
	// we don't know if the user has not turned it off meanwhile
	if (IOBluetoothLocalDeviceAvailable () == FALSE)
		return kIOReturnNotAttached;
	
	// if we are currently discovering, we can't start a new discovery right now.
	if ([self isDiscovering])
		return kIOReturnSuccess;
	
	if (!_delegate) {
		NSLog (@"Warning: starting WiiRemoteDiscovery without delegate set");
	}
	
	// to start all over again, first stop the discovery
	[self close];

	// setup the inquiry for best performance and results, ie:
	//  limit the serach in time
	//  limit the serach in quality
	//  don't update the name of the devices, this is useless
	// the returned inquiry is autoreleased, we will have to retain it if we decide to keep it
	_inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
	[_inquiry setInquiryLength:20];
	[_inquiry setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
	[_inquiry setUpdateNewDeviceNames:NO];

	IOReturn status = [_inquiry start];
	if (status == kIOReturnSuccess) {
		[_inquiry retain];
	} else {
		// not likely to happen, but we handle it anyway
		NSLog (@"Error: Inquiry did not start, error %d", status);
		[_inquiry setDelegate:nil];
		_inquiry = nil;
	}
	
	return status;
}

- (IOReturn) stop
{
	return [_inquiry stop];
}

- (IOReturn) close
{
	IOReturn ret = kIOReturnSuccess;
	
	ret = [_inquiry stop];
	if (ret != kIOReturnNotPermitted)
		LogIOReturn (ret);
	
	[_inquiry setDelegate:nil];
	[_inquiry release];
	_inquiry = nil;
	
	NSLogDebug (@"Discovery closed");
	return ret;
}

- (BOOL) isDiscovering
{
	return _isDiscovering;
}

- (void) setIsDiscovering:(BOOL) flag
{
	[self willChangeValueForKey:@"isDiscovering"];
	_isDiscovering = flag;
	[self didChangeValueForKey:@"isDiscovering"];
}


#pragma mark -
#pragma mark IOBluetoothDeviceInquiry delegates

- (void) deviceInquiryStarted:(IOBluetoothDeviceInquiry*) sender
{
	// this delegate method is called only when the search actually started
	// it is important not to start another inquiry at the same time, cf apple docs
	[self setIsDiscovering:YES];
}

- (void) deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *) sender 
						   device:(IOBluetoothDevice *) device
{
	// here we limit the search to only one device, we could search for more, but it is not necessary.
	// note: never try to connect to the wiimote while the inquiry is still running! (cf apple docs)
	[_inquiry stop];
}

- (void) deviceInquiryComplete:(IOBluetoothDeviceInquiry*) sender 
						 error:(IOReturn) error
					   aborted:(BOOL) aborted
{	
	// the inquiry has completed, we can now process what we have found
	[self setIsDiscovering:NO];
	
	// unlikely to be called, but handle it anyway
	if ((error != kIOReturnSuccess) && !aborted) {
		[_delegate WiiRemoteDiscoveryError:error];
		return;
	}

	// tell our delegate that we are going to connect to the found devices.
	// as it takes some time to do it, the delegate could for example display a
	// animated hourglass cursor during the connection process.
	if ([[_inquiry foundDevices] count])
		[_delegate willStartWiimoteConnections];
	
	[self connectToFoundDevices];
}

#pragma mark -

- (void) connectToFoundDevices
{
	// use the inquiry foundDevices array to query the found devices
	// as we limited the bluetooth search, we can be sure all the items are actually wiimotes.
	NSEnumerator * en = [[_inquiry foundDevices] objectEnumerator];
	id device = nil;
	while ((device = [en nextObject]) != nil) {
		WiiRemote * wii = [[[WiiRemote alloc] init] autorelease];
		IOReturn ret = [wii connectTo:device];
		
		if (ret == kIOReturnSuccess)
			[_delegate WiiRemoteDiscovered:wii];
		else
			[_delegate WiiRemoteDiscoveryError:ret];
	}
	
	// we passed through all devices, clear the search for now
	[_inquiry clearFoundDevices];
}


@end
