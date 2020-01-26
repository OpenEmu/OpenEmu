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

@import Foundation;

NS_ASSUME_NONNULL_BEGIN

extern NSString * const OEOpenVGDBVersionKey;
extern NSString * const OEOpenVGDBUpdateCheckKey;
extern NSString * const OEOpenVGDBUpdateIntervalKey;

extern NSNotificationName const OEGameInfoHelperWillUpdateNotification;
extern NSNotificationName const OEGameInfoHelperDidChangeUpdateProgressNotification;
extern NSNotificationName const OEGameInfoHelperDidUpdateNotification;

@interface OEGameInfoHelper : NSObject

@property(class, readonly) OEGameInfoHelper *sharedHelper;

@property (readonly) CGFloat downloadProgress;
@property (nullable, copy) NSString *downloadVersion;
@property (readonly, getter=isUpdating) BOOL updating;

- (NSDictionary * _Nullable)gameInfoWithDictionary:(NSDictionary *)gameInfo;

- (BOOL)hashlessROMCheckForSystem:(NSString *)system;
- (BOOL)headerROMCheckForSystem:(NSString *)system;
- (BOOL)serialROMCheckForSystem:(NSString *)system;
- (int)sizeOfROMHeaderForSystem:(NSString *)system;

// Checks for updates and passes URL of new release and version if any newer DB is found.
- (void)checkForUpdatesWithHandler:(void (^)(NSURL * _Nullable newURL, NSString * _Nullable newVersion))handler;
- (void)installVersion:(NSString *)versionTag withDownloadURL:(NSURL *)url;
- (void)cancelUpdate;

- (id)executeQuery:(NSString *)sql error:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
