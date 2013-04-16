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

#import "OEStorageDevice.h"

#import <IOKit/usb/IOUSBLib.h>

extern const int64_t kOEDisconnectDelay;
extern const int32_t kOERetrodeVendorID;
extern const int32_t kOERetrodeProductIDRevision1;
extern const int32_t kOERetrodeProductIDRevision2;
extern const int32_t kOEVendorIDVersion2DFU;
extern const int32_t kOEProductIDVersion2DFU;

typedef struct OERetrodeDeviceData {
    io_object_t				notification;
    IOUSBDeviceInterface	**deviceInterface;
    io_service_t            ioService;
    UInt32					locationID;
} OERetrodeDeviceData;

@class OERetrode;
@protocol OERetrodeDelegate <NSObject>
@optional
- (void)retrodeDidDisconnect:(OERetrode*)retrode;
- (void)retrodeDidRescanGames:(OERetrode*)retrode;
- (void)retrodeHardwareDidBecomeAvailable:(OERetrode*)retrode;
- (void)retrodeHardwareDidBecomeUnavailable:(OERetrode*)retrode;

- (void)retrodeDidMount:(OERetrode*)retrode;
- (void)retrodeDidUnmount:(OERetrode*)retrode;

- (void)retrodeDidEnterDFUMode:(OERetrode*)retrode;
- (void)retrodeDidLeaveDFUMode:(OERetrode*)retrode;
@end

@interface OERetrode : OEStorageDevice
- (NSString*)mountPath;
- (void)mountFilesystem;
- (void)unmountFilesystem;

@property (readonly, strong) NSString *identifier;
@property (copy)             NSString *deviceVersion;
@property BOOL isMounted;


@property (strong) NSArray *games;
@end
