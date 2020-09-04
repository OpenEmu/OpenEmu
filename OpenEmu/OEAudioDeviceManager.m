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

#import "OEAudioDeviceManager.h"

#import <CoreAudio/CoreAudio.h>


#pragma mark - Private functions

static OSStatus _OEAOPropertyListenerProc(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[], void *inClientData);

#pragma mark -

@implementation OEAudioDeviceManager
{
    NSMutableArray *_devices;
}

#pragma mark - Lifecycle

+ (instancetype)sharedAudioDeviceManager
{
    static OEAudioDeviceManager *sharedAudioDeviceManager;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedAudioDeviceManager = [[self alloc] init];

        const AudioObjectPropertySelector propSelectors[] =
        {
            kAudioHardwarePropertyDevices,
            kAudioHardwarePropertyDefaultInputDevice,
            kAudioHardwarePropertyDefaultOutputDevice,
            kAudioHardwarePropertyDefaultSystemOutputDevice,
        };

        const int numSelectors = sizeof(propSelectors) / sizeof(propSelectors[0]);
        AudioObjectPropertyAddress propAddr = {0, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
        for(int i = 0; i < numSelectors; i++)
        {
            propAddr.mSelector = propSelectors[i];
            AudioObjectAddPropertyListener(kAudioObjectSystemObject, &propAddr, _OEAOPropertyListenerProc, NULL);
        }
    });

    return sharedAudioDeviceManager;
}

- (instancetype)init
{
    if((self = [super init]))
    {
        _devices = [NSMutableArray array];
        [self OE_updateDeviceList];
    }
    return self;
}

#pragma mark - Properties

- (NSArray *)audioDevices
{
    NSArray *result;
    @synchronized(_devices)
    {
        result = [_devices copy];
    }
    return result;
}

- (OEAudioDevice *)OE_audioDeviceWithPropertySelector:(AudioObjectPropertySelector)propertySelector
{
    AudioDeviceID              deviceID;
    OSStatus                   err      = 0;
    UInt32                     dataSize = sizeof(AudioDeviceID);
    AudioObjectPropertyAddress propAddr = {propertySelector, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};

    err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propAddr, 0, NULL, &dataSize, &deviceID);
    if(err != noErr)
        return nil;

    return [OEAudioDevice audioDeviceWithID:deviceID];

}

- (OEAudioDevice *)defaultInputDevice
{
    return [self OE_audioDeviceWithPropertySelector:kAudioHardwarePropertyDefaultInputDevice];
}

- (OEAudioDevice *)defaultOutputDevice
{
    return [self OE_audioDeviceWithPropertySelector:kAudioHardwarePropertyDefaultOutputDevice];
}

- (OEAudioDevice *)defaultSystemOutputDevice
{
    return [self OE_audioDeviceWithPropertySelector:kAudioHardwarePropertyDefaultSystemOutputDevice];
}

#pragma mark - Core Audio

- (void)OE_updateDeviceList
{
    AudioDeviceID              *deviceIDs       = NULL;
    NSUInteger                  numberOfDevices = 0;
    OSStatus                    err             = noErr;
    UInt32                      dataSize        = 0;
    static const AudioObjectPropertyAddress propAddr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster };
    static const AudioObjectPropertyAddress aggDevCompositionPropAddr = {
        kAudioAggregateDevicePropertyComposition,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster,
    };

    err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propAddr, 0, NULL, &dataSize);
    if(err != noErr) return;

    numberOfDevices = dataSize / (UInt32)sizeof(AudioDeviceID);
    if(numberOfDevices == 0)
    {
        DLog(@"No audio devices were found");

        NSUInteger previousNumberOfDevices;
        @synchronized(_devices)
        {
            previousNumberOfDevices = [_devices count];
            [_devices removeAllObjects];
        }

        if(previousNumberOfDevices > 0)
            [[NSNotificationCenter defaultCenter] postNotificationName:OEAudioDeviceManagerDidChangeDeviceListNotification object:self];

        return;
    }

    deviceIDs = malloc(dataSize);
    memset(deviceIDs, 0, dataSize);
    err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propAddr, 0, NULL, &dataSize, (void *)deviceIDs);
    if(err != noErr)
    {
        DLog(@"Error retrieving the list of audio devices");
        free(deviceIDs);
        return;
    }

    NSMutableArray *devicesToAdd = [NSMutableArray array];

    @synchronized(_devices)
    {
        for(NSUInteger i = 0; i < numberOfDevices; i++)
        {
            AudioDeviceID deviceID = deviceIDs[i];
            
            CFDictionaryRef composition;
            dataSize = sizeof(CFDictionaryRef);
            AudioObjectGetPropertyData(deviceID, &aggDevCompositionPropAddr, 0, NULL, &dataSize, &composition);
            if (composition) {
                NSNumber *private = [(__bridge NSDictionary*)composition objectForKey:@kAudioAggregateDeviceIsPrivateKey];
                CFRelease(composition);
                if ([private boolValue])
                    continue;
            }
            
            NSUInteger deviceIndex = [_devices indexOfObjectPassingTest:
                                      ^ BOOL (OEAudioDevice *device, NSUInteger idx, BOOL *stop)
                                      {
                                          return [device deviceID] == deviceID;
                                      }];

            if(deviceIndex == NSNotFound)
                [devicesToAdd addObject:[OEAudioDevice audioDeviceWithID:deviceID]];
        }

        NSIndexSet *indexesToRemove =
        [_devices indexesOfObjectsPassingTest:
         ^ BOOL (OEAudioDevice *device, NSUInteger idx, BOOL *stop)
         {
             AudioDeviceID deviceID = [device deviceID];
             for(NSUInteger j = 0; j < numberOfDevices; j++)
                 if(deviceIDs[j] == deviceID)
                     return NO;

             return YES;
         }];

        // TODO: Maybe post individual notifications for each device that was removed/added. Maybe
        //         devicesToRemove = [_devices objectsAtIndexes:indexesToRemove];

        [_devices removeObjectsAtIndexes:indexesToRemove];
        [_devices addObjectsFromArray:devicesToAdd];
    }

    DLog(@"OEAudioDeviceManager added audio devices %@", devicesToAdd);

    free(deviceIDs);
}

OSStatus _OEAOPropertyListenerProc(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[], void *inClientData)
{
    NSMutableSet *notificationNames = [NSMutableSet setWithCapacity:4];

    for(UInt32 i = 0; i < inNumberAddresses; i++)
    {
        switch(inAddresses[i].mSelector)
        {
            case kAudioHardwarePropertyDefaultInputDevice :
                DLog(@"OEAudioDeviceManager new default input device is %@", [[OEAudioDeviceManager sharedAudioDeviceManager] defaultInputDevice]);
                [notificationNames addObject:OEAudioDeviceManagerDidChangeDefaultInputDeviceNotification];
                break;

            case kAudioHardwarePropertyDefaultOutputDevice :
                DLog(@"OEAudioDeviceManager new default output device is %@", [[OEAudioDeviceManager sharedAudioDeviceManager] defaultOutputDevice]);
                [notificationNames addObject:OEAudioDeviceManagerDidChangeDefaultOutputDeviceNotification];
                break;

            case kAudioHardwarePropertyDefaultSystemOutputDevice :
                DLog(@"OEAudioDeviceManager new default system output device is %@", [[OEAudioDeviceManager sharedAudioDeviceManager] defaultSystemOutputDevice]);
                [notificationNames addObject:OEAudioDeviceManagerDidChangeDefaultSystemOutputDeviceNotification];
                break;

            case kAudioHardwarePropertyDevices :
                [notificationNames addObject:OEAudioDeviceManagerDidChangeDeviceListNotification];
                break;

            default :
                DLog(@"OEAOPropertyListener: unknown message");
                break;
        }
    }

    NSNotificationCenter *nc  = [NSNotificationCenter defaultCenter];
    OEAudioDeviceManager *mgr = [OEAudioDeviceManager sharedAudioDeviceManager];

    for(NSNotificationName notificationName in notificationNames)
    {
        if(notificationName == OEAudioDeviceManagerDidChangeDeviceListNotification)
            [[OEAudioDeviceManager sharedAudioDeviceManager] OE_updateDeviceList];

        DLog(@"OEAudioDeviceManager is posting notification %@", notificationName);
        [nc postNotificationName:notificationName object:mgr];
    }

    return noErr;
}

@end

#pragma mark -

@implementation OEAudioDevice

+ (instancetype)audioDeviceWithID:(AudioDeviceID)deviceID
{
    return [[self alloc] initWithID:deviceID];
}

- (instancetype)initWithID:(AudioDeviceID)deviceID
{
    if((self = [super init]) == nil) return nil;

    _deviceID = deviceID;

    [self OE_retrieveDeviceName];
    [self OE_retrieveNumberOfInputChannels];
    [self OE_retrieveNumberOfOutputChannels];

    return self;
}

- (void)OE_retrieveDeviceName
{
    CFStringRef                tempString = NULL;
    OSStatus                   err        = noErr;
    UInt32                     ioSize     = sizeof(CFStringRef);
    AudioObjectPropertyAddress propAddr   =
    {
        kAudioObjectPropertyName,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    err = AudioObjectGetPropertyData(_deviceID, &propAddr, 0, NULL, &ioSize, &tempString);
    if(tempString && err == noErr)
        _deviceName = [(__bridge_transfer NSString *)tempString copy];
}

- (NSUInteger)OE_numberOfChannelsInScope:(AudioObjectPropertyScope)propScope
{
    OSStatus                    err        = noErr;
    UInt32                      ioSize     = 0;
    UInt32                      number     = 0;
    AudioBufferList            *bufferList = NULL;
    AudioObjectPropertyAddress  propAddr   = { kAudioDevicePropertyStreamConfiguration, propScope, 0 };

    err = AudioObjectGetPropertyDataSize(_deviceID, &propAddr, 0, NULL, &ioSize);
    if(err == noErr && ioSize != 0)
    {
        bufferList = malloc(ioSize);
        if(bufferList != NULL)
        {
            // Get stream configuration
            err = AudioObjectGetPropertyData(_deviceID, &propAddr, 0, NULL, &ioSize, bufferList);
            if(err == noErr)
            {
                // Count the total number of channels in the stream
                for(UInt32 i = 0; i < bufferList->mNumberBuffers; i++)
                    number += bufferList->mBuffers[i].mNumberChannels;
            }

            free(bufferList);
            return number;
        }
    }

    return 0;
}

- (void)OE_retrieveNumberOfInputChannels
{
    _numberOfInputChannels = [self OE_numberOfChannelsInScope:kAudioObjectPropertyScopeInput];
}

- (void)OE_retrieveNumberOfOutputChannels
{
    _numberOfOutputChannels = [self OE_numberOfChannelsInScope:kAudioObjectPropertyScopeOutput];
}

- (BOOL)isEqual:(OEAudioDevice *)object
{
    return object == self || ([object isKindOfClass:[OEAudioDevice class]] && _deviceID == object->_deviceID);
}

- (NSUInteger)hash
{
    return _deviceID;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Audio device %lu, name %@, %lu input channels, %lu output channels",
            (unsigned long)_deviceID,
            _deviceName,
            (unsigned long)_numberOfInputChannels,
            (unsigned long)_numberOfOutputChannels];
}

@end


#pragma mark -

NSNotificationName const OEAudioDeviceManagerDidChangeDeviceListNotification                = @"OEAudioDeviceManagerDidChangeDeviceListNotification";
NSNotificationName const OEAudioDeviceManagerDidChangeDefaultInputDeviceNotification        = @"OEAudioDeviceManagerDidChangeDefaultInputDeviceNotification";
NSNotificationName const OEAudioDeviceManagerDidChangeDefaultOutputDeviceNotification       = @"OEAudioDeviceManagerDidChangeDefaultOutputDeviceNotification";
NSNotificationName const OEAudioDeviceManagerDidChangeDefaultSystemOutputDeviceNotification = @"OEAudioDeviceManagerDidChangeDefaultSystemOutputDeviceNotification";
