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

//  Based on WiiRemoteFramework by KIMURA Hiroaki on 06/12/04.
//  Copyright 2006 KIMURA Hiroaki. All rights reserved.

#import <Cocoa/Cocoa.h>
#import "OEWiimoteDeviceHandler.h"
#import "WiimoteTypes.h"

extern NSString *const OEWiimoteDidConnectNotificationName;
extern NSString *const OEWiimoteDidDisconnectNotificationName;

# pragma mark -
@class IOBluetoothDevice, OEWiimoteDeviceHandler;
@interface Wiimote : NSObject
# pragma mark - 
- (Wiimote*)init;
- (Wiimote*)initWithDevice:(IOBluetoothDevice*)newDevice;

#pragma mark - Device Info -
- (NSString*)nameOrAddress;
- (NSString*)address;
- (NSNumber*)productID;
- (NSNumber*)vendorID;
- (NSNumber*)locationID;

# pragma mark - Connection -
- (void)connect;
- (void)disconnect;

@property (readonly, getter=isConnected) BOOL connected;

# pragma mark -
# pragma mark Configuration & Status
- (void)syncConfig;
- (void)syncLEDAndRumble;
- (void)requestStatus;
@property (readonly) float batteryLevel;

#pragma mark - LEDs -
- (void)setLED1:(BOOL)flag1 LED2:(BOOL)flag2 LED3:(BOOL)flag3 LED4:(BOOL)flag4;
@property (readonly) BOOL LED1Illuminated;
@property (readonly) BOOL LED2Illuminated;
@property (readonly) BOOL LED3Illuminated;
@property (readonly) BOOL LED4Illuminated;

#pragma mark - Report Modes -
@property (nonatomic) BOOL rumbleActivated;
@property (nonatomic) BOOL expansionPortEnabled;
@property (nonatomic, readonly) BOOL expansionPortAttached;

# pragma mark -
# pragma mark Delegate
@property (weak) OEWiimoteDeviceHandler *handler;
@end
