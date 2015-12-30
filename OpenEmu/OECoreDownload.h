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

@class SUAppcast, SUAppcastItem;
@class OECorePlugin;
@class OECoreDownload;
@protocol OECoreDownloadDelegate;

NS_ASSUME_NONNULL_BEGIN

@interface OECoreDownload : NSObject

- (instancetype)initWithPlugin:(OECorePlugin *)plugin;
- (void)startDownload:(nullable id)sender;
- (void)cancelDownload:(nullable id)sender;

@property (weak) id <OECoreDownloadDelegate> delegate;

@property (copy) NSString *name;
@property (copy) NSArray <NSString *> *systemIdentifiers;
@property (copy) NSArray <NSString *> *systemNames;
@property (copy) NSString *version;
@property (copy) NSString *bundleIdentifier;

@property BOOL hasUpdate;
@property BOOL canBeInstalled;

@property (readonly, getter=isDownloading) BOOL downloading;

@property (readonly) CGFloat progress;
@property (nullable) SUAppcast *appcast;
@property (nullable) SUAppcastItem *appcastItem;

@end

@protocol OECoreDownloadDelegate <NSObject>

- (void)coreDownloadDidStart:(OECoreDownload *)download;
- (void)coreDownloadDidFinish:(OECoreDownload *)download;

@optional
- (void)coreDownloadDidFail:(OECoreDownload *)download withError:(NSError *)error;

@end

NS_ASSUME_NONNULL_END
