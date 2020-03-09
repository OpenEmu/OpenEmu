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

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudio.h>

@class OEAudioDevice;

/*
 * @class    OEAudioDeviceManager
 * @abstract Keeps an up-to-date list of all audio devices connected to the machine.
 */
@interface OEAudioDeviceManager : NSObject

@property(class, readonly) OEAudioDeviceManager *sharedAudioDeviceManager;

@property(readonly) NSArray       *audioDevices;
@property(readonly) OEAudioDevice *defaultInputDevice;
@property(readonly) OEAudioDevice *defaultOutputDevice;
@property(readonly) OEAudioDevice *defaultSystemOutputDevice;
@end


/*
 * @class    OEAudioDevice
 * @abstract Contains information about an audio device.
 */
@interface OEAudioDevice : NSObject

+ (instancetype)audioDeviceWithID:(AudioDeviceID)deviceID;
- (instancetype)initWithID:(AudioDeviceID)deviceID;

@property(readonly) AudioDeviceID  deviceID;
@property(readonly) NSString      *deviceName;
@property(readonly) NSUInteger     numberOfInputChannels;
@property(readonly) NSUInteger     numberOfOutputChannels;

@end


/*
 * @const      OEAudioDeviceManagerDidChangeDeviceListNotification
 * @abstract   Posted whenever an audio device is connected or disconnected.
 * @discussion Corresponds to Core Audio’s kAudioHardwarePropertyDevices.
 */
extern NSNotificationName const OEAudioDeviceManagerDidChangeDeviceListNotification;

/*
 * @const      OEAudioDeviceManagerDidChangeDefaultInputDeviceNotification
 * @abstract   Posted when the user changes the default audio input device.
 * @discussion Corresponds to Core Audio’s kAudioHardwarePropertyDefaultInputDevice.
 */
extern NSNotificationName const OEAudioDeviceManagerDidChangeDefaultInputDeviceNotification;

/*
 * @const      OEAudioDeviceManagerDidChangeDefaultOutputDeviceNotification
 * @abstract   Posted when the user changes the default audio output device.
 * @discussion Corresponds to Core Audio’s kAudioHardwarePropertyDefaultOutputDevice.
 */
extern NSNotificationName const OEAudioDeviceManagerDidChangeDefaultOutputDeviceNotification;

/*
 * @const      OEAudioDeviceManagerDidChangeDefaultSystemOutputDeviceNotification
 * @abstract   Posted when the user changes the default system audio output device.
 * @discussion Corresponds to Core Audio’s kAudioHardwarePropertyDefaultSystemOutputDevice.
 */
extern NSNotificationName const OEAudioDeviceManagerDidChangeDefaultSystemOutputDeviceNotification;
