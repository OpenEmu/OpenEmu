/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OERetrodeDeviceManager.h"

#import <IOKit/IOKitLib.h>
#import <IOKit/IOMessage.h>
#import <IOKit/IOCFPlugIn.h>
#import <IOKit/IOBSD.h>

#import <paths.h>

#import "OERetrode.h"
#import "OERetrode_IOLevel.h"

NSString * const OERetrodeSupportEnabledKey = @"retrodeSupport";

@interface OERetrodeDeviceManager ()
{
    BOOL _retrodeSupportActive;
    NSMutableDictionary     *_retrodes;
    
    CFRunLoopRef			_runLoop;
    IONotificationPortRef	_notificationPort;
    io_iterator_t			_matchedRetrode1ItemsIterator, _matchedRetrode2ItemsIterator, _matchedDFUItemsIterator;
}

BOOL addDevice(void *refCon, io_service_t usbDevice);
void DeviceNotification(void *refCon, io_service_t service, natural_t messageType, void *messageArgument);
void DeviceAdded(void *refCon, io_iterator_t iterator);

- (NSDictionary*)OE_retrode2MatchingCriteria;
- (NSDictionary*)OE_retrode2MatchingCriteriaWithLocationID:(UInt32)locationID;
- (NSDictionary*)OE_dfuMatchingCriteria;
- (NSDictionary*)OE_dfuMatchingCriteriaWithLocationID:(UInt32)locationID;
@end

@implementation OERetrodeDeviceManager

+ (void)initialize
{
    if(self != [OERetrodeDeviceManager class])
        return;
    
    [[OEStorageDeviceManager sharedStorageDeviceManager] registerStorageDeviceManager:[self sharedRetrodeDeviceManager]];
}

+ (id)sharedRetrodeDeviceManager
{
    static OERetrodeDeviceManager *retrodeDeviceManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        retrodeDeviceManager = [[OERetrodeDeviceManager alloc] init];
    });
    return retrodeDeviceManager;
}

- (id)init
{
    self = [super init];
    if (self) {
        _retrodes = [NSMutableDictionary dictionary];
    }
    return self;
}

- (void)startDeviceSupport
{
    if(_retrodeSupportActive) return;
    _retrodeSupportActive = YES;
    
    kern_return_t error;
    
    // Setup dictionary to match USB device
    CFDictionaryRef re1MatchingCriteria = CFBridgingRetain([self OE_retrode1MatchingCriteria]);
    CFDictionaryRef re2MatchingCriteria = CFBridgingRetain([self OE_retrode2MatchingCriteria]);
    CFDictionaryRef dfuMatchingCriteria = CFBridgingRetain([self OE_dfuMatchingCriteria]);
    _notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(_notificationPort);
    
    _runLoop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(_runLoop, runLoopSource, kCFRunLoopDefaultMode);
    
    // Now set up a notification to be called when a device is first matched by I/O Kit.
    error = IOServiceAddMatchingNotification(_notificationPort, kIOMatchedNotification, re1MatchingCriteria, DeviceAdded, NULL, &_matchedRetrode1ItemsIterator);
    NSAssert(error==noErr, @"Could not register main matching notification");
    error = IOServiceAddMatchingNotification(_notificationPort, kIOMatchedNotification, re2MatchingCriteria, DeviceAdded, NULL, &_matchedRetrode2ItemsIterator);
    NSAssert(error==noErr, @"Could not register main matching notification");
    error = IOServiceAddMatchingNotification(_notificationPort, kIOMatchedNotification, dfuMatchingCriteria, DeviceAdded, NULL, &_matchedDFUItemsIterator);
    NSAssert(error==noErr, @"Could not register dfu matching notification");
    
    // Iterate once to get already-present devices and arm the notification
    DeviceAdded(NULL, _matchedRetrode1ItemsIterator);
    DeviceAdded(NULL, _matchedRetrode2ItemsIterator);
    DeviceAdded(NULL, _matchedDFUItemsIterator);
}

- (void)stopDeviceSupport
{
    if(!_retrodeSupportActive) return;
    _retrodeSupportActive = NO;
    
    [_retrodes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        OERetrode    *aRetrode   = obj;
        OERetrodeDeviceData *deviceData = [aRetrode deviceData];
        [aRetrode setupWithDeviceData:NULL];
        if(deviceData != NULL)
        {
            if (deviceData->deviceInterface)
                (*deviceData->deviceInterface)->Release(deviceData->deviceInterface);
            deviceData->deviceInterface = NULL;
            IOObjectRelease(deviceData->notification);
            free(deviceData);
        }
    }];
    [_retrodes removeAllObjects];
    
    IONotificationPortDestroy(_notificationPort);
    _notificationPort = NULL;
    _runLoop = NULL;
    
    IOObjectRelease(_matchedRetrode1ItemsIterator);
    IOObjectRelease(_matchedRetrode2ItemsIterator);
    IOObjectRelease(_matchedDFUItemsIterator);
}
#pragma mark - Device Matching Callbacks
void DeviceNotification(void *refCon, io_service_t service, natural_t messageType, void *messageArgument)
{
    OERetrodeDeviceData *deviceData = (OERetrodeDeviceData *)refCon;
    if (messageType == kIOMessageServiceIsTerminated) {
        OERetrodeDeviceManager    *self              = [OERetrodeDeviceManager sharedRetrodeDeviceManager];
        NSMutableDictionary *retrodes          = self->_retrodes;
        NSString            *retrodeIdentifier = [OERetrode generateIdentifierFromDeviceData:deviceData];
        OERetrode           *retrode           = [retrodes objectForKey:retrodeIdentifier];
        
        [retrode setupWithDeviceData:NULL];
        
        if (deviceData->deviceInterface)
            (*deviceData->deviceInterface)->Release(deviceData->deviceInterface);
        IOObjectRelease(deviceData->notification);
        IOObjectRelease(deviceData->ioService);
        free(deviceData);
        
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, kOEDisconnectDelay * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            OERetrodeDeviceManager *self        = [OERetrodeDeviceManager sharedRetrodeDeviceManager];
            NSMutableDictionary *retrodes = self->_retrodes;
            OERetrode  *retrode           = [retrodes objectForKey:retrodeIdentifier];
            if([retrode deviceData] == NULL)
            {
                // try to recover one last time
                io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, CFBridgingRetain([self OE_retrode2MatchingCriteriaWithLocationID:[retrode locationID]]));
                if(service == 0)
                {
                    service = IOServiceGetMatchingService(kIOMasterPortDefault, CFBridgingRetain([self OE_dfuMatchingCriteriaWithLocationID:[retrode locationID]]));
                }
                if(service == 0)
                {
                    [retrode disconnect];
                    [retrodes removeObjectForKey:retrodeIdentifier];
                    [[OEStorageDeviceManager sharedStorageDeviceManager] removeDevice:retrode];
                }
                else
                    addDevice((__bridge void *)(self), service);
            }
        });
    }
}

void DeviceAdded(void *refCon, io_iterator_t iterator)
{
    OERetrodeDeviceManager *self = [OERetrodeDeviceManager sharedRetrodeDeviceManager];
    io_service_t  usbDevice;
    while ((usbDevice = IOIteratorNext(iterator))) {
        addDevice((__bridge void *)(self), usbDevice);
    }
}

BOOL addDevice(void *refCon, io_service_t usbDevice)
{
    BOOL isNewDevice = NO;
    OERetrodeDeviceManager *self = [OERetrodeDeviceManager sharedRetrodeDeviceManager];
    assert(self!=nil);
    kern_return_t error;
    IOCFPlugInInterface	**plugInInterface = NULL;
    OERetrodeDeviceData *deviceDataRef    = NULL;
    UInt32			    locationID;
    
    // Prepare struct for device specific data
    deviceDataRef = malloc(sizeof(OERetrodeDeviceData));
    memset(deviceDataRef, '\0', sizeof(OERetrodeDeviceData));
    
    // Get Interface Plugin
    SInt32 score; // unused
    error = IOCreatePlugInInterfaceForService(usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
    if(error != noErr)
    {
        free(deviceDataRef);
        DLog(@"Error: %0x (%d) occured.", error, error);
        return NO;
    }
    assert(error == noErr && plugInInterface!=NULL);
    
    // Get USB Device Interface
    error = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*) &deviceDataRef->deviceInterface);
    assert(error == noErr);
    
    IODestroyPlugInInterface(plugInInterface);
    
    // Get USB Location ID
    error = (*deviceDataRef->deviceInterface)->GetLocationID(deviceDataRef->deviceInterface, &locationID);
    assert(error == noErr);
    deviceDataRef->locationID = locationID;
    
    UInt16 productID;
    error = (*deviceDataRef->deviceInterface)->GetDeviceProduct(deviceDataRef->deviceInterface, &productID);
    assert(error == noErr);
    NSString *deviceVersion;
    BOOL isDFUDevice = NO;
    if(productID == kOERetrodeProductIDRevision1)
        deviceVersion = @"1";
    else if(productID == kOERetrodeProductIDRevision2)
        deviceVersion = @"2";
    else if(productID == kOEProductIDVersion2DFU)
        isDFUDevice = YES;
    
    // Register for device removal notification (keeps notification ref in device data)
    error = IOServiceAddInterestNotification(self->_notificationPort, usbDevice, kIOGeneralInterest, DeviceNotification, deviceDataRef, &(deviceDataRef->notification));
    assert(error == noErr);
    
    deviceDataRef->ioService = usbDevice;
    
    // Create Retrode objc object
    NSString  *identifier = [OERetrode generateIdentifierFromDeviceData:deviceDataRef];
    OERetrode *retrode    = [self->_retrodes objectForKey:identifier];
    if(!retrode && !isDFUDevice)
    {
        retrode = [[OERetrode alloc] init];
        [self->_retrodes setObject:retrode forKey:identifier];
        isNewDevice = YES;
    }
    else if(!retrode && isDFUDevice)
    {
        DLog(@"Ignoring DFU device because we can't be sure that it's a retrode");
        if (deviceDataRef->deviceInterface)
            (*deviceDataRef->deviceInterface)->Release(deviceDataRef->deviceInterface);
        IOObjectRelease(deviceDataRef->notification);
        IOObjectRelease(deviceDataRef->ioService);
        free(deviceDataRef);
        return isNewDevice;
    }
    
    if(isDFUDevice)
    {
        [retrode setDFUMode:isDFUDevice];
    }
    else
    {
        [retrode setDeviceVersion:deviceVersion];
    }
    [retrode setupWithDeviceData:deviceDataRef];
    
    if(isNewDevice) [[OEStorageDeviceManager sharedStorageDeviceManager] addDevice:retrode];

    return isNewDevice;
}

#pragma mark - Matching Criteria
- (NSDictionary*)OE_retrode2MatchingCriteria
{
    return [self OE_retrode2MatchingCriteriaWithLocationID:0];
}
- (NSDictionary*)OE_retrode2MatchingCriteriaWithLocationID:(UInt32)locationID
{
    if(locationID == 0)
        return @{ @kIOProviderClassKey : @kIOUSBDeviceClassName, @kUSBVendorID : @(kOERetrodeVendorID), @kUSBProductID : @(kOERetrodeProductIDRevision2) };
    else
        return @{ @kIOProviderClassKey : @kIOUSBDeviceClassName, @kUSBVendorID : @(kOERetrodeVendorID), @kUSBProductID : @(kOERetrodeProductIDRevision2), @kIOLocationMatchKey : @(locationID) };
    
}

- (NSDictionary*)OE_retrode1MatchingCriteria
{
    return [self OE_retrode1MatchingCriteriaWithLocationID:0];
}
- (NSDictionary*)OE_retrode1MatchingCriteriaWithLocationID:(UInt32)locationID
{
    if(locationID == 0)
        return @{ @kIOProviderClassKey : @kIOUSBDeviceClassName, @kUSBVendorID : @(kOERetrodeVendorID), @kUSBProductID : @(kOERetrodeProductIDRevision1) };
    else
        return @{ @kIOProviderClassKey : @kIOUSBDeviceClassName, @kUSBVendorID : @(kOERetrodeVendorID), @kUSBProductID : @(kOERetrodeProductIDRevision1), @kIOLocationMatchKey : @(locationID) };
    
}

- (NSDictionary*)OE_dfuMatchingCriteria
{
    return [self OE_dfuMatchingCriteriaWithLocationID:0];
}

- (NSDictionary*)OE_dfuMatchingCriteriaWithLocationID:(UInt32)locationID
{
    if(locationID == 0)
        return @{ @kIOProviderClassKey : @kIOUSBDeviceClassName, @kUSBVendorID : @(kOEVendorIDVersion2DFU), @kUSBProductID : @(kOEProductIDVersion2DFU) };
    else
        return @{ @kIOProviderClassKey : @kIOUSBDeviceClassName, @kUSBVendorID : @(kOEVendorIDVersion2DFU), @kUSBProductID : @(kOEProductIDVersion2DFU), @kIOLocationMatchKey : @(locationID) };
}
@end
