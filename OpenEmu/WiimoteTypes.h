//
//  WiimoteTypes.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.11.12.
//
//

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
	WiiClassicControllerPlusButton,
    
	WiiButtonCount
} WiiButtonType;

typedef enum {
	WiiNunchukJoyStick					= 0,
	WiiClassicControllerLeftJoyStick	= 1,
	WiiClassicControllerRightJoyStick	= 2
} WiiJoyStickType;

typedef enum {
	WiiNunchukJoyStickMaximumX					= 255,
    WiiNunchukJoyStickMaximumY					= 255
} WiiAxisMaximums;

typedef enum {
    WiiNunchuckCenterTreshholdX = 30,
    WiiNunchuckCenterTreshholdY = 30
    
} WiiAxisTheshholds;

typedef enum {
    WiiExpansionNunchuck,
    WiiExpansionClassicController,
    
    WiiExpansionUnkown,
    WiiExpansionNotConnected,
    WiiExpansionNotInitialized,
} WiiExpansionType;
