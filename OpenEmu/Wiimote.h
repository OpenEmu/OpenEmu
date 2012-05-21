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

#import <IOBluetooth/objc/IOBluetoothL2CAPChannel.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>

# pragma mark Type definitions
typedef unsigned char WiiIRModeType;
enum {
	kWiiIRModeBasic			= 0x01,
	kWiiIRModeExtended		= 0x03,
	kWiiIRModeFull			= 0x05
};

typedef struct {
	int x, y, s;
} IRData;

typedef struct {
	unsigned short accX_zero, accY_zero, accZ_zero, accX_1g, accY_1g, accZ_1g; 
} WiiAccCalibData;

typedef struct {
	unsigned short x_min, x_max, x_center, y_min, y_max, y_center; 
} WiiJoyStickCalibData;

typedef struct {
	unsigned short tr, br, tl, bl;
} WiiBalanceBoardGrid;

typedef struct {
	WiiBalanceBoardGrid kg0, kg17, kg34;
} WiiBalanceBoardCalibData;

typedef enum {
	WiiRemoteAButton,
	WiiRemoteBButton,
	WiiRemoteOneButton,
	WiiRemoteTwoButton,
	WiiRemoteMinusButton,
	WiiRemoteHomeButton,
	WiiRemotePlusButton,
	WiiRemoteUpButton,
	WiiRemoteDownButton,
	WiiRemoteLeftButton,
	WiiRemoteRightButton,
	
	WiiNunchukZButton,
	WiiNunchukCButton,
	
	WiiClassicControllerXButton,
	WiiClassicControllerYButton,
	WiiClassicControllerAButton,
	WiiClassicControllerBButton,
	WiiClassicControllerLButton,
	WiiClassicControllerRButton,
	WiiClassicControllerZLButton,
	WiiClassicControllerZRButton,
	WiiClassicControllerUpButton,
	WiiClassicControllerDownButton,
	WiiClassicControllerLeftButton,
	WiiClassicControllerRightButton,
	WiiClassicControllerMinusButton,
	WiiClassicControllerHomeButton,
	WiiClassicControllerPlusButton
} WiiButtonType;

typedef enum {
	WiiNunchukJoyStick					= 0,
	WiiClassicControllerLeftJoyStick	= 1,
	WiiClassicControllerRightJoyStick	= 2
} WiiJoyStickType;
# pragma mark -
@class Wiimote;
@protocol WiimoteDelegate <NSObject>
- (void)wiimoteDidConnect:(Wiimote*)theWiimote;
- (void)wiimoteDidDisconnect:(Wiimote*)theWiimote;

- (void)wiimote:(Wiimote*)theWiimote didNotConnect:(NSError*)err;
- (void)wiimote:(Wiimote*)theWiimote didNotDisconnect:(NSError*)err;

- (void)wiimoteReportsExpansionPortChanged:(Wiimote*)theWiimote;
- (void)wiimote:(Wiimote*)theWiimote reportsButtonChanged:(WiiButtonType)type isPressed:(BOOL)isPressed;
- (void)wiimote:(Wiimote*)theWiimote reportsIrPointMovedX:(float)px Y:(float)py;
- (void)wiimote:(Wiimote*)theWiimote reportsJoystickChanged:(WiiJoyStickType)type tiltX:(unsigned short)tiltX tiltY:(unsigned short)tiltY;
@end


@interface Wiimote : NSObject {	
	IOBluetoothDevice* _btDevice;
	IOBluetoothL2CAPChannel * _ichan;
	IOBluetoothL2CAPChannel * _cchan;
	
	WiiIRModeType irMode; 
	
	BOOL _connected;
	
	BOOL _motionSensorEnabled;
	BOOL _irSensorEnabled;
	BOOL _rumbleActivated;
	BOOL _expansionPortEnabled;
	BOOL _expansionPortAttached;

	BOOL _LED1Illuminated;
	BOOL _LED2Illuminated;
	BOOL _LED3Illuminated;
	BOOL _LED4Illuminated;
	
	BOOL _speakerEnabled;
	BOOL _speakerMuted;
    
    UInt16 lastButtonReport;
}
# pragma mark -
- (Wiimote*)init;
- (Wiimote*)initWithDelegate:(id <WiimoteDelegate>)newDelegate;
- (Wiimote*)initWithDevice:(IOBluetoothDevice*)newDevice;
- (Wiimote*)initWithDevice:(IOBluetoothDevice*)newDevice andDelegate:(id)newDelegate;

# pragma mark -
# pragma mark usual getters + setters
@property (strong) id <WiimoteDelegate> delegate;

# pragma mark -
# pragma mark Wiimote Configuration
- (void)syncConfig;
- (void)syncLEDAndRumble;

- (BOOL)motionSensorEnabled;
- (BOOL)irSensorEnabled;
- (BOOL)rumbleActivated;
- (BOOL)expansionPortEnabled;
- (BOOL)expansionPortAttached;

- (BOOL)LED1Illuminted;
- (BOOL)LED2Illuminted;
- (BOOL)LED3Illuminted;
- (BOOL)LED4Illuminted;

- (void)setMotionSensorEnabled:(BOOL)flag;
- (void)setIrSensorEnabled:(BOOL)flag;
- (void)setRumbleActivated:(BOOL)flag;
- (void)setMotionSensorEnabled:(BOOL)flag;
- (void)setExpansionPortEnabled:(BOOL)flag;

- (void)setLED1:(BOOL)flag1 LED2:(BOOL)flag2 LED3:(BOOL)flag3 LED4:(BOOL)flag4;

# pragma mark -
# pragma mark Connection
- (void)setDevice:(IOBluetoothDevice*)newDev;
- (void)connect;
- (void)disconnect;

- (BOOL)isConnected;
# pragma mark -
- (void)getStatus;

# pragma mark -
# pragma mark Data Handling
- (void)writeData:(const unsigned char*)data at:(unsigned long)address length:(size_t)length;
- (void)readData:(unsigned long)address length:(unsigned short)length;
- (void)sendCommand:(const unsigned char*)data length:(size_t)length;

# pragma mark -
# pragma mark Sound
- (void)setSpeakerEnabled:(BOOL)flag;
- (BOOL)speakerEnabled;
- (BOOL)speakerMuted;
- (void)playSound:(NSSound*)theSound;
- (void)setSpeakerMute:(BOOL)flag;
- (void)toggleMute;
@end
