//
//  WiiRemoteDiscovery.h
//  DarwiinRemote
//
//  Created by Ian Rickard on 12/9/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h> 
#import "WiiRemote.h"


@interface WiiRemoteDiscovery : NSObject {
	IOBluetoothDeviceInquiry * _inquiry;
	BOOL _isDiscovering;
	
	id _delegate;
}

+ (WiiRemoteDiscovery*) discoveryWithDelegate:(id)delegate;

- (id) delegate;
- (void) setDelegate:(id) delegate;

- (IOReturn) start;
- (IOReturn) stop;
- (IOReturn) close;

- (BOOL) isDiscovering;
- (void) setIsDiscovering:(BOOL) flag;

- (void) connectToFoundDevices;

@end


@interface NSObject (WiiRemoteDiscoveryDelegate)

- (void) willStartWiimoteConnections;
- (void) WiiRemoteDiscovered:(WiiRemote*)wiimote;
- (void) WiiRemoteDiscoveryError:(int)code;

@end;