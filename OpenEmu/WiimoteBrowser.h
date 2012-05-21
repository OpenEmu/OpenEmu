//
//  WiimoteBrowser.h
//  WiimoteFramework
//
//  Created by Christoph Leimbrock on 27.07.09.
//  Copyright 2009 Christoph Leimbrock. All rights reserved.
//

//  Based on WiiRemoteFramework by KIMURA Hiroaki on 06/12/04.
//  Copyright 2006 KIMURA Hiroaki. All rights reserved.

#import <Cocoa/Cocoa.h>

#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>

#include "Wiimote.h"
@protocol WiimoteBrowserDelegate;
@interface WiimoteBrowser : NSObject {
	BOOL _isSearching;
	
	IOBluetoothDeviceInquiry* _inquiry;
	
	int _maxWiimotes; // maximum number of wiimotes to discover (-1 for unlimited)
	
	NSArray* _discoveredDevices;
}

- (int)maxWiimoteCount;
- (void)setMaxWiimoteCount:(int)newMax;

@property (strong) id <WiimoteBrowserDelegate> delegate;
- (NSArray*)discoveredDevices;

- (void)startSearch;
- (void)stopSearch;

- (NSArray*)_convertFoundDevicesToWiimotes:(NSArray*)foundDevices;
@end

@protocol WiimoteBrowserDelegate <NSObject>

- (void)wiimoteBrowserWillSearch;
- (void)wiimoteBrowserDidStopSearchWithResults:(NSArray*)discoveredDevices;
- (void)wiimoteBrowserSearchFailedWithError:(int)code;

@end