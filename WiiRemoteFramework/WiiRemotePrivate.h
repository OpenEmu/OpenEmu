/*
 *  WiiRemotePrivate.h
 *  WiiRemoteFramework
 *
 *  Created by KIMURA Hiroaki on 07/03/06.
 *  Copyright 2007 KIMURA Hiroaki. All rights reserved.
 *
 */

// this type is used a lot (data array):
typedef unsigned char darr[];

typedef UInt16 WiiButtonBitMask;
enum {
	kWiiRemoteTwoButton					= 0x0001,
	kWiiRemoteOneButton					= 0x0002,
	kWiiRemoteBButton					= 0x0004,
	kWiiRemoteAButton					= 0x0008,
	kWiiRemoteMinusButton				= 0x0010,
	kWiiRemoteHomeButton				= 0x0080,
	kWiiRemoteLeftButton				= 0x0100,
	kWiiRemoteRightButton				= 0x0200,
	kWiiRemoteDownButton				= 0x0400,
	kWiiRemoteUpButton					= 0x0800,
	kWiiRemotePlusButton				= 0x1000,
	
	
	kWiiNunchukZButton					= 0x0001,
	kWiiNunchukCButton					= 0x0002,
	
	
	kWiiClassicControllerUpButton		= 0x0001,
	kWiiClassicControllerLeftButton		= 0x0002,
	kWiiClassicControllerZRButton		= 0x0004,
	kWiiClassicControllerXButton		= 0x0008,
	kWiiClassicControllerAButton		= 0x0010,
	kWiiClassicControllerYButton		= 0x0020,
	kWiiClassicControllerBButton		= 0x0040,
	kWiiClassicControllerZLButton		= 0x0080,
	kWiiClassicControllerUnUsedButton	= 0x0100,
	kWiiClassicControllerRButton		= 0x0200,
	kWiiClassicControllerPlusButton		= 0x0400,
	kWiiClassicControllerHomeButton		= 0x0800,
	kWiiClassicControllerMinusButton	= 0x1000,
	kWiiClassicControllerLButton		= 0x2000,
	kWiiClassicControllerDownButton		= 0x4000,
	kWiiClassicControllerRightButton	= 0x8000
	
};


@interface WiiRemote (Private)
- (IOBluetoothL2CAPChannel *) openL2CAPChannelWithPSM:(BluetoothL2CAPPSM) psm delegate:(id) delegate;

- (IOReturn)writeData:(const unsigned char*)data at:(unsigned long)address length:(size_t)length;
- (IOReturn)readData:(unsigned long)address length:(unsigned short)length;
- (IOReturn)sendCommand:(const unsigned char*)data length:(size_t)length;

- (void)getCurrentStatus:(NSTimer*)timer;
- (void)sendWiiRemoteButtonEvent:(UInt16)data;
- (void)sendWiiNunchukButtonEvent:(UInt16)data;
- (void)sendWiiClassicControllerButtonEvent:(UInt16)data;
- (unsigned char)decrypt:(unsigned char)data;
@end
