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

#import "OEStorageDeviceManager.h"

NSString * const OEStorageDeviceDidAppearNotificationName = @"OEStorageDeviceDidAppearNotificationName";
NSString * const OEStorageDeviceDidDisappearNotificationName = @"OEStorageDeviceDidDisappearNotificationName";

@interface OEStorageDeviceManager ()
@end

@implementation OEStorageDeviceManager
static NSMutableArray *_deviceManagers;
static NSMutableArray *_devices;

+ (id)sharedStorageDeviceManager
{
    static id sharedStorageDeviceManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedStorageDeviceManager = [[self alloc] init];
    });
    return sharedStorageDeviceManager;
}

- (id)init
{
    self = [super init];
    if (self) {
        _deviceManagers = [NSMutableArray array];
        _devices = [NSMutableArray array];
    }
    return self;
}

- (void)startDeviceSupport
{
    
}

- (void)stopDeviceSupport
{
    if(![self isMemberOfClass:[OEStorageDeviceManager class]]) return;
    
    // Stop support for all device managers
    [_deviceManagers enumerateObjectsUsingBlock:^(OEStorageDeviceManager *obj, NSUInteger idx, BOOL *stop) {
        [obj stopDeviceSupport];
    }];
}

- (NSArray*)devices
{
    return _devices;
}
#pragma mark - Private Methods
- (void)registerStorageDeviceManager:(OEStorageDeviceManager *)manager
{
    [_deviceManagers addObject:manager];
    [manager startDeviceSupport];
}

- (void)addDevice:(OEStorageDevice*)device
{
    [_devices addObject:device];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEStorageDeviceDidAppearNotificationName object:device userInfo:nil];
}

- (void)removeDevice:(OEStorageDevice*)device
{
    [_devices removeObject:device];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEStorageDeviceDidDisappearNotificationName object:device userInfo:nil];
}
@end
