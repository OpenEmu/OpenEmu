/*
 Copyright (c) 2011, OpenEmu Team

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

@import Foundation;
#import "OECoreDownload.h"

NS_ASSUME_NONNULL_BEGIN

extern NSString *const OECoreUpdaterErrorDomain;
typedef NS_ERROR_ENUM(OECoreUpdaterErrorDomain, OECoreUpdaterErrorCode)
{
    OENoDownloadableCoreForIdentifierError = -1,
    OENewCoreCheckAlreadyPendingError = -2
};

@class OEAlert;
@class OEDBGame, OEDBSaveState;

@interface OECoreUpdater : NSObject <OECoreDownloadDelegate>

@property(class, readonly) OECoreUpdater *sharedUpdater;

- (void)checkForUpdates;
- (void)checkForUpdatesAndInstall;

- (void)checkForNewCoresWithCompletionHandler:(nullable void(^)(NSError *error))handler NS_SWIFT_NAME(checkForNewCores(completionHandler:));
- (void)cancelCheckForNewCores;

- (void)installCoreForGame:(OEDBGame *)game withCompletionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler;
- (void)installCoreForSaveState:(OEDBSaveState *)state withCompletionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler;
- (void)installCoreWithDownload:(OECoreDownload *)download message:(NSString *)message completionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler;
- (void)installCoreWithDownload:(OECoreDownload *)download completionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler;

- (void)installCoreInBackgroundUserInitiated:(OECoreDownload *)download NS_SWIFT_NAME(installCoreInBackgroundUserInitiated(_:));

@property(readonly) NSArray<OECoreDownload *> *coreList;

@property(copy, nullable) void(^completionHandler)(OECorePlugin * _Nullable plugin, NSError * _Nullable);
@property(nullable) NSString *coreIdentifier;
@property(nullable) OEAlert *alert;
@property(nullable) OECoreDownload *coreDownload;
@end

NS_ASSUME_NONNULL_END
