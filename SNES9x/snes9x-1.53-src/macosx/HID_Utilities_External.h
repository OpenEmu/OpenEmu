/*
*	File:		HID_Utilities_External.h
*
*	Contains:   Definition of the HID Utilities exported functions
*				External interface for HID Utilities, can be used with either library or source
*				Check notes below for usage. Some type casting is required so library is framework and carbon free
*
*	Copyright:	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*
*	Disclaimer:	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
*				( "Apple" ) in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms. If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
*				copyrights in this original Apple software ( the "Apple Software" ), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software. Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple. Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION )
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _HID_Utilities_External_h_
#define _HID_Utilities_External_h_

/*****************************************************/
#pragma mark - includes & imports
/*****************************************************/
#if !TARGET_RT_MAC_CFM
#include <IOKit/hid/IOHIDLib.h>
#endif TARGET_RT_MAC_CFM

//#include <IOKit/hid/IOHIDUsageTables.h>
#if 0   // NOTE: These are now in <IOKit/hid/IOHIDUsageTables.h>
#include "PID.h"
#include "IOHIDPowerUsage.h"
#endif

#include <stdio.h>

/*****************************************************/
#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
#pragma pack( push, 2 )
#elif PRAGMA_STRUCT_PACK
#pragma pack( 2 )
#endif

/*****************************************************/
#pragma mark - typedef's, struct's, enums, defines, etc.
/*****************************************************/
#if TARGET_RT_MAC_CFM
// from IOHIDKeys.h( IOKit )
// this can't be included since the original file has framework includes
// developers may need to add definitions here
typedef enum IOHIDElementType {
	kIOHIDElementTypeInput_Misc    = 1,
	kIOHIDElementTypeInput_Button   = 2,
	kIOHIDElementTypeInput_Axis    = 3,
	kIOHIDElementTypeInput_ScanCodes = 4,
	kIOHIDElementTypeOutput      = 129,
	kIOHIDElementTypeFeature      = 257,
	kIOHIDElementTypeCollection    = 513
}IOHIDElementType;

typedef enum IOHIDReportType {
	kIOHIDReportTypeInput = 0,
	kIOHIDReportTypeOutput,
	kIOHIDReportTypeFeature,
	kIOHIDReportTypeCount
}IOHIDReportType;

// Notes: This is a MachO function pointer. If you're using CFM you have to call MachOFunctionPointerForCFMFunctionPointer.
typedef void( *IOHIDCallbackFunction ) ( void* target, unsigned long result, void* refcon, void* sender );
typedef void* IOHIDEventStruct;
#endif TARGET_RT_MAC_CFM

// Device and Element Interfaces

typedef enum HIDElementTypeMask
{
	kHIDElementTypeInput				= 1 << 1,
	kHIDElementTypeOutput      	= 1 << 2,
	kHIDElementTypeFeature      	= 1 << 3,
	kHIDElementTypeCollection    	= 1 << 4,
	kHIDElementTypeIO					= kHIDElementTypeInput | kHIDElementTypeOutput | kHIDElementTypeFeature,
	kHIDElementTypeAll					= kHIDElementTypeIO | kHIDElementTypeCollection
}HIDElementTypeMask;

struct hu_element_t
{
	unsigned long type;						// the type defined by IOHIDElementType in IOHIDKeys.h
	long usage;								// usage within above page from IOUSBHIDParser.h which defines specific usage
	long usagePage;							// usage page from IOUSBHIDParser.h which defines general usage
	void* cookie;				 			// unique value( within device of specific vendorID and productID ) which identifies element, will NOT change
	long min;								// reported min value possible
	long max;								// reported max value possible
	long scaledMin;							// reported scaled min value possible
	long scaledMax;							// reported scaled max value possible
	long size;								// size in bits of data return from element
	unsigned char relative;					// are reports relative to last report( deltas )
	unsigned char wrapping;					// does element wrap around( one value higher than max is min )
	unsigned char nonLinear;				// are the values reported non-linear relative to element movement
	unsigned char preferredState;			// does element have a preferred state( such as a button )
	unsigned char nullState;				// does element have null state
	long units;								// units value is reported in( not used very often )
	long unitExp;							// exponent for units( also not used very often )
	char name[256];							// name of element( c string )

	// runtime variables
	long initialCenter; 					// center value at start up
	unsigned char hasCenter; 				// whether or not to use center for calibration
	long minReport; 						// min returned value
	long maxReport; 						// max returned value( calibrate call )
	long userMin; 							// user set value to scale to( scale call )
	long userMax;

	struct hu_element_t* pPrevious;			// previous element( NULL at list head )
	struct hu_element_t* pChild;			// next child( only of collections )
	struct hu_element_t* pSibling;			// next sibling( for elements and collections )

	long depth;
};
typedef struct hu_element_t hu_element_t;

struct hu_device_t
{
	void* interface;						// interface to device, NULL = no interface
	void* queue;							// device queue, NULL = no queue
	void* runLoopSource;					// device run loop source, NULL == no source
	void* queueRunLoopSource;				// device queue run loop source, NULL == no source
	void* transaction;						// output transaction interface, NULL == no interface
	void* notification;						// notifications
	char transport[256];					// device transport( c string )
	long vendorID;							// id for device vendor, unique across all devices
	long productID;							// id for particular product, unique across all of a vendors devices
	long version;							// version of product
	char manufacturer[256];					// name of manufacturer
	char product[256];						// name of product
	char serial[256];						// serial number of specific product, can be assumed unique across specific product or specific vendor( not used often )
	long locID;								// long representing location in USB( or other I/O ) chain which device is pluged into, can identify specific device on machine
	long usage;								// usage page from IOUSBHID Parser.h which defines general usage
	long usagePage;							// usage within above page from IOUSBHID Parser.h which defines specific usage
	long totalElements;						// number of total elements( should be total of all elements on device including collections ) ( calculated, not reported by device )
	long features;							// number of elements of type kIOHIDElementTypeFeature
	long inputs;							// number of elements of type kIOHIDElementTypeInput_Misc or kIOHIDElementTypeInput_Button or kIOHIDElementTypeInput_Axis or kIOHIDElementTypeInput_ScanCodes
	long outputs;							// number of elements of type kIOHIDElementTypeOutput
	long collections;						// number of elements of type kIOHIDElementTypeCollection
	long axis;								// number of axis( calculated, not reported by device )
	long buttons;							// number of buttons( calculated, not reported by device )
	long hats;								// number of hat switches( calculated, not reported by device )
	long sliders;							// number of sliders( calculated, not reported by device )
	long dials;								// number of dials( calculated, not reported by device )
	long wheels;							// number of wheels( calculated, not reported by device )
	hu_element_t* pListElements;			// head of linked list of elements
	struct hu_device_t* pNext; 				// next device
};
typedef struct hu_device_t hu_device_t;

// this is the procedure type for a client hot plug callback
typedef OSStatus ( *HotPlugCallbackProcPtr )( io_iterator_t inIODeviceIterator );

/*****************************************************/
#pragma mark - exported globals
/*****************************************************/
#pragma mark - exported function prototypes
/*****************************************************/
#pragma mark HID Utilities interface
/*****************************************************/
// Create and open an interface to device, required prior to extracting values or building queues
// Notes: appliction now owns the device and must close and release it prior to exiting
extern unsigned long HIDCreateOpenDeviceInterface( UInt32 inHIDDevice, hu_device_t* inDevice );

// builds list of device with elements( allocates memory and captures devices )
// list is allocated internally within HID Utilites and can be accessed via accessor functions
// structures within list are considered flat and user accessable, but not user modifiable
// can be called again to rebuild list to account for new devices( will do the right thing in case of disposing existing list )
// inUsagePage, usage are each a inNumDeviceTypes sized array of matching usage and usage pages
// returns TRUE if successful

extern Boolean HIDBuildMultiDeviceList( UInt32 *inUsagePage, UInt32 *inUsage, UInt32 inNumDeviceTypes );

// same as above but this uses a single inUsagePage and usage

extern Boolean HIDBuildDeviceList( UInt32 inUsagePage, UInt32 inUsage );

// updates the current device list for any new/removed devices
// if this is called before HIDBuildDeviceList the it functions like HIDBuildMultiDeviceList
// inUsagePage, usage are each a inNumDeviceTypes sized array of matching usage and usage pages
// returns TRUE if successful which means if any device were added or removed( the device config changed )

extern Boolean HIDUpdateDeviceList( UInt32 *inUsagePage, UInt32 *inUsage, UInt32 inNumDeviceTypes );

// release list built by above function
// MUST be called prior to application exit to properly release devices
// if not called( or app crashes ) devices can be recovered by pluging into different location in USB chain
extern void HIDReleaseDeviceList( void );

// does a device list exist
extern Boolean HIDHaveDeviceList( void );

// how many HID devices have been found
// returns 0 if no device list exist
extern UInt32 HIDCountDevices( void );

// how many elements does a specific device have
// returns 0 if device is invalid or NULL
// uses mask of HIDElementTypeMask to restrict element found
// use kHIDElementTypeIO to get non-collection elements
extern UInt32 HIDCountDeviceElements( const hu_device_t* inDevice, HIDElementTypeMask inTypeMask );

// get the first device in the device list
// returns NULL if no list exists
extern hu_device_t* HIDGetFirstDevice( void );

// get next device in list given current device as parameter
// returns NULL if end of list
extern hu_device_t* HIDGetNextDevice( const hu_device_t* inDevice );

// get the first element of device passed in as parameter
// returns NULL if no list exists or device does not exists or is NULL
// uses mask of HIDElementTypeMask to restrict element found
// use kHIDElementTypeIO to get previous HIDGetFirstDeviceElement functionality
extern hu_element_t* HIDGetFirstDeviceElement( const hu_device_t* inDevice, HIDElementTypeMask inTypeMask );

// get next element of given device in list given current element as parameter
// will walk down each collection then to next element or collection( depthwise traverse )
// returns NULL if end of list
// uses mask of HIDElementTypeMask to restrict element found
// use kHIDElementTypeIO to get previous HIDGetNextDeviceElement functionality
extern hu_element_t* HIDGetNextDeviceElement( hu_element_t* inElement, HIDElementTypeMask inTypeMask );

// get previous element of given device in list given current element as parameter
// this walks directly up the tree to the top element and does not search at each level
// returns NULL if beginning of list
// uses mask of HIDElementTypeMask to restrict element found
// use kHIDElementTypeIO to get non-collection elements
extern hu_element_t* HIDGetPreviousDeviceElement( const hu_element_t* inElement, HIDElementTypeMask inTypeMask );

// Sets the client hot plug callback routine
extern OSStatus HIDSetHotPlugCallback( HotPlugCallbackProcPtr inHotPlugCallbackProcPtr );

/*****************************************************/
#pragma mark Name Lookup Interfaces
/*****************************************************/

// get vendor name from vendor ID
extern Boolean HIDGetVendorNameFromVendorID( long inVendorID, char* outCStrName );

// get product name from vendor/product ID
extern Boolean HIDGetProductNameFromVendorProductID( long inVendorID, long inProductID, char* outCStrName );

// set name from vendor id/product id look up( using cookies )
extern Boolean HIDGetElementNameFromVendorProductCookie( long inVendorID, long inProductID, long inCookie, char* inCStrName );

// set name from vendor id/product id look up( using usage page & usage )
extern Boolean HIDGetElementNameFromVendorProductUsage( long inVendorID, long inProductID, long inUsagePage, long inUsage, char* inCStrName );

// Add device to usage & cookie db's
extern Boolean HIDAddDeviceToXML( const hu_device_t* inDevice );

// Add element to usage & cookie db's
extern Boolean HIDAddDeviceElementToXML( const hu_device_t* inDevice, const hu_element_t* inElement );

// returns C string type name given a type enumeration passed in as parameter( see IOHIDKeys.h )
// returns empty string for invalid types
extern void HIDGetTypeName( IOHIDElementType inIOHIDElementType, char* inCStrName );

// returns C string usage given usage page and usage passed in as parameters( see IOUSBHIDParser.h )
// returns usage page and usage values in string form for unknown values
extern void HIDGetUsageName( long inUsagePage, long inUsage, char* inCStrName );

/*****************************************************/
// convert an element type to a mask
extern HIDElementTypeMask HIDConvertElementTypeToMask( IOHIDElementType inIOHIDElementType );

// find this device
extern Boolean HIDFindDevice( const hu_device_t* inSearchDevice, hu_device_t **outFoundDevice );

// find the device and element for this action
// Device: serial, vendorID, productID, location, inUsagePage, usage
// Element: cookie, inUsagePage, usage,
extern Boolean HIDFindActionDeviceAndElement( const hu_device_t* inSearchDevice, const hu_element_t* inSearchElement, hu_device_t **outFoundDevice, hu_element_t **outFoundElement );

// find the device and element for this action
// Device: serial, vendorID, productID, location, inUsagePage, usage
// Element: cookie, inUsagePage, usage,

extern Boolean HIDFindSubElement( const hu_element_t* inStartElement, const hu_element_t* inSearchElement, hu_element_t **outFoundElement );

// print out all of an elements information
extern int HIDPrintElement( const hu_element_t* inElement );

// return TRUE if this is a valid device pointer
extern Boolean HIDIsValidDevice( const hu_device_t* inDevice );

// return TRUE if this is a valid element pointer for this device
extern Boolean HIDIsValidElement( const hu_device_t* inDevice, const hu_element_t* inElement );

/*****************************************************/
#pragma mark Element Event Queue and Value Interfaces
/*****************************************************/
enum
{
	kDefaultUserMin = 0, 					// default user min and max used for scaling
	kDefaultUserMax = 255
};

enum
{
	kDeviceQueueSize = 50	// this is wired kernel memory so should be set to as small as possible
							// but should account for the maximum possible events in the queue
							// USB updates will likely occur at 100 Hz so one must account for this rate of
							// if states change quickly( updates are only posted on state changes )
};

/*****************************************************/
// queues specific element, performing any device queue set up required
extern unsigned long HIDQueueElement( hu_device_t* inDevice, const hu_element_t* inElement );

// adds all elements to queue, performing any device queue set up required
extern unsigned long HIDQueueDevice( hu_device_t* inDevice );

// removes element for queue, if last element in queue will release queue and device
extern unsigned long HIDDequeueElement( hu_device_t* inDevice, const hu_element_t* inElement );

// completely removes all elements from queue and releases queue and device
extern unsigned long HIDDequeueDevice( hu_device_t* inDevice );

// releases all device queues for quit or rebuild( must be called )
extern unsigned long HIDReleaseAllDeviceQueues( void );

// releases interface to device, should be done prior to exiting application( called from HIDReleaseDeviceList )
extern unsigned long HIDCloseReleaseInterface( hu_device_t* inDevice );

// returns TRUE if an event is avialable for the element and fills out *outHIDEvent structure, returns FALSE otherwise
// outHIDEvent is a poiner to a IOHIDEventStruct, using void here for compatibility, users can cast a required
extern unsigned char HIDGetEvent( const hu_device_t* inDevice, void* outHIDEvent );

// returns current value for element, creating device interface as required, polling element
extern long HIDGetElementEvent( const hu_device_t* inDevice, hu_element_t* inElement, IOHIDEventStruct* outIOHIDEvent );

// returns current value for element, creating device interface as required, polling element
extern long HIDGetElementValue( const hu_device_t* inDevice, hu_element_t* inElement );

// Set an elements value
// NOTE: This should only be used when a single element report needs to be sent.
// If multiple elements reports are to be send then transactions should be used.
// outIOHIDEvent is a poiner to a IOHIDEventStruct, using void here for compatibility, users can cast a required
extern long HIDSetElementValue( const hu_device_t* inDevice, const hu_element_t* inElement, void* outIOHIDEvent );

// Set a callback to be called when a queue goes from empty to non-empty
// Notes: This is a MachO function pointer. If you're using CFM you have to call MachOFunctionPointerForCFMFunctionPointer.
extern long HIDSetQueueCallback( hu_device_t* inDevice, IOHIDCallbackFunction inIOHIDCallback, void* inCallbackTarget, void* inCallbackRefcon );

#if defined( AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER )
// Get a report from a device
extern long HIDGetReport( const hu_device_t* inDevice, IOHIDReportType inIOHIDReportType, unsigned long inReportID, void* inReportBuffer, unsigned long* inReportBufferSize ) AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;

// Send a report to a device
extern long HIDSetReport( const hu_device_t* inDevice, IOHIDReportType inIOHIDReportType, unsigned long inReportID, void* inReportBuffer, unsigned long inReportBufferSize ) AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
#endif // defined( AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER )

#if defined( AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER )
// copy matching elements
extern long HIDCopyMatchingElements( const hu_device_t* inDevice, CFDictionaryRef inMatchingDict, CFArrayRef* inElements ) AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;
// Set a callback to be called when a report comes in the interrupt pipe
// Notes: This is a MachO function pointer. If you're using CFM you have to call MachOFunctionPointerForCFMFunctionPointer.
extern long HIDSetInterruptReportHandlerCallback( const hu_device_t* inDevice, void* inReportBuffer, UInt32 inReportBufferSize, IOHIDReportCallbackFunction inIOHIDReportCallback, void* inCallbackTarget, void* inCallbackRefcon ) AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;
#endif // defined( AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER )

/*****************************************************/
#pragma mark calibration & scale routines
/*****************************************************/
// returns calibrated value given raw value passed in
// calibrated value is equal to min and max values returned by HIDGetElementValue since device list built scaled to element reported min and max values
extern SInt32 HIDCalibrateValue( SInt32 inValue, hu_element_t* inElement );

// returns scaled value given raw value passed in
// scaled value is equal to current value assumed to be in the range of element reported min and max values scaled to user min and max scaled values
extern SInt32 HIDScaleValue( SInt32 inValue, const hu_element_t* inElement );

/*****************************************************/
#pragma mark Conguration and Save Interfaces
/*****************************************************/
#define kPercentMove 10 // precent of overall range a element must move to register

typedef struct hu_SaveRec_t
{
	long actionCookie;
	// device( need to add serial number when I have a test case )
	long vendorID;
	long productID;
	long locID;
	long usage;
	long usagePage;
	// elements
	long usagePageE;
	long usageE;
	void* cookie;
}hu_SaveRec_t;

// polls all devices and elements for a change greater than kPercentMove. Times out after given time
// returns TRUE and pointer to device and element if found
// returns FALSE and NULL for both parameters if not found
extern Boolean HIDConfigureAction( hu_device_t **outDevice, hu_element_t **outElement, float inTimeout );

// -- These are routines to use if the applcation wants HID Utilities to do the file handling --
// Notes: the FILE* is a MachO posix FILE and will not work with the MWCW MSL FILE* type.

// take input records, save required info
// assume file is open and at correct position.
extern void HIDSaveElementConfig( FILE* inFILE, const hu_device_t* inDevice, const hu_element_t* inElement, long inActionCookie );

// take file, read one record( assume file position is correct and file is open )
// search for matching device
// return inDevice, inElement and cookie for action
extern long HIDRestoreElementConfig( FILE* inFILE, hu_device_t **outDevice, hu_element_t **outElement );

// -- These routines use the CFPreferences API's.

// Save the device & element values into the specified key in the specified applications preferences
extern Boolean HIDSaveElementPref( CFStringRef inKeyCFStringRef, CFStringRef inAppCFStringRef, const hu_device_t* inDevice, const hu_element_t* inElement );

// Find the specified preference in the specified application
// search for matching device and element
// return inDevice, inElement that matches

extern Boolean HIDRestoreElementPref( CFStringRef inKeyCFStringRef, CFStringRef inAppCFStringRef, hu_device_t **outDevice, hu_element_t **outElement );

// -- These are routines to use if the client wants to use their own file handling --

// Set up a config record for saving
// takes an input records, returns record user can save as they want
// Notes: the save rec must be pre-allocated by the calling app and will be filled out
extern void HIDSetElementConfig( hu_SaveRec_t* inConfigRec, const hu_device_t* inDevice, const hu_element_t* inElement, long inActionCookie );

// Get matching element from config record
// takes a pre-allocated and filled out config record
// search for matching device
// return inDevice, inElement and cookie for action
extern long HIDGetElementConfig( hu_SaveRec_t* inConfigRec, hu_device_t **outDevice, hu_element_t **outElement );

/*****************************************************/
#pragma mark Output Transaction interface
/*****************************************************/
// Create and open an transaction interface to device, required prior to extracting values or building Transactions
extern unsigned long HIDTransactionAddElement( hu_device_t* inDevice, const hu_element_t* inElement );

// removes an element from a Transaction
extern unsigned long HIDTransactionRemoveElement( hu_device_t* inDevice, const hu_element_t* inElement );

// return TRUE if this transaction contains this element
extern Boolean HIDTransactionHasElement( hu_device_t* inDevice, const hu_element_t* inElement );

/* This changes the default value of an element, when the values of the */
/* elements are cleared, on clear or commit, they are reset to the */
/* default value */
/* This call can be made on elements that are not in the transaction, but */
/* has undefined behavior if made on elements not in the transaction */
/* which are later added to the transaction. */
/* In other words, an element should be added before its default is */
/* set, for well defined behavior. */
// outHIDEvent is a poiner to a IOHIDEventStruct, using void here for compatibility, users can cast a required
extern unsigned long HIDTransactionSetElementDefault( hu_device_t* inDevice, const hu_element_t* inElement, IOHIDEventStruct* inValueEvent );

/* Get the current setting of an element's default value */
// outHIDEvent is a poiner to a IOHIDEventStruct, using void here for compatibility, users can cast a required
extern unsigned long HIDTransactionGetElementDefault( hu_device_t* inDevice, const hu_element_t* inElement, IOHIDEventStruct* inValueEvent );

/* Add a change to the transaction, by setting an element value */
/* The change is not actually made until it is commited */
/* The element must be part of the transaction or this call will fail */
// outHIDEvent is a poiner to a IOHIDEventStruct, using void here for compatibility, users can cast a required
extern unsigned long HIDTransactionSetElementValue( hu_device_t* inDevice, const hu_element_t* inElement, IOHIDEventStruct* inValueEvent );

/* Get the current setting of an element value */
// outHIDEvent is a poiner to a IOHIDEventStruct, using void here for compatibility, users can cast a required
extern unsigned long HIDTransactionGetElementValue( hu_device_t* inDevice, const hu_element_t* inElement, IOHIDEventStruct* inValueEvent );

/* Commit the transaction, or clear all the changes and start over */
extern unsigned long HIDTransactionCommit( hu_device_t* inDevice );

/* Clear all the changes and start over */
extern unsigned long HIDTransactionClear( hu_device_t* inDevice );

/*****************************************************/
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
#pragma pack( pop )
#elif PRAGMA_STRUCT_PACK
#pragma pack( )
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif
/*****************************************************/
#endif // _HID_Utilities_External_h_
