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

#import "OECoreDownload.h"
@import OpenEmuKit;

#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>

#import <OpenEmuBase/OpenEmuBase.h>

@interface OECoreDownload () <NSURLSessionDownloadDelegate>

@property (copy) NSString *fullPluginPath;

@property NSURLSession *downloadSession;

@property (readwrite, getter=isDownloading) BOOL downloading;
@property (readwrite) CGFloat progress;

@end

@implementation OECoreDownload

- (instancetype)initWithPlugin:(OECorePlugin *)plugin {
    
    NSParameterAssert(plugin);
    
    if ((self = [super init])) {
        [self OE_updatePropertiesWithPlugin:plugin];
    }
    return self;
}

#pragma mark - Core Downloading

- (void)startDownload:(id)sender {
    
    if (self.appcastItem == nil || self.isDownloading) {
        return;
    }

    NSURL *url = self.appcastItem.fileURL;
    
    NSAssert(self.downloadSession == nil, @"There shouldn't be a previous download session.");
    
    self.downloadSession = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration] delegate:self delegateQueue:[NSOperationQueue mainQueue]];
    self.downloadSession.sessionDescription = self.bundleIdentifier;
    
    NSURLSessionDownloadTask *downloadTask = [self.downloadSession downloadTaskWithURL:url];
        
    DLog(@"Starting core download (%@)", self.downloadSession.sessionDescription);
    
    [downloadTask resume];
    
    self.downloading = YES;
    [self.delegate coreDownloadDidStart:self];
}

- (void)cancelDownload:(id)sender {
    
    DLog(@"Cancelling core download (%@)", self.downloadSession.sessionDescription);
    
    [self.downloadSession invalidateAndCancel];
}

#pragma mark - NSURLSessionDownloadTaskDelegate

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    
    DLog(@"Core download (%@) did complete: %@", self.downloadSession.sessionDescription, error ? error.localizedDescription : @"no errors");
    
    self.downloading = NO;
    self.progress = 0.0;
    
    [self.downloadSession finishTasksAndInvalidate];
    self.downloadSession = nil;

    if (error != nil) {
        if ([self.delegate respondsToSelector:@selector(coreDownloadDidFail:withError:)]) {
            [self.delegate coreDownloadDidFail:self withError:error];
        } else {
            [[NSApplication sharedApplication] presentError:error];
        }
    } else {
        [self.delegate coreDownloadDidFinish:self];
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    
    [self willChangeValueForKey:@"progress"];
    
    self.progress = (double)totalBytesWritten / (double)totalBytesExpectedToWrite;
    
    [self didChangeValueForKey:@"progress"];
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(NSURL *)location {
    
    DLog(@"Core download (%@) did finish downloading temporary data.", self.downloadSession.sessionDescription);
    
    XADArchive *archive = nil;
    @try {
        archive = [XADArchive archiveForFile:location.path];
    } @catch (NSException *exception) {
        archive = nil;
    }
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *basePath = (paths.count > 0) ? paths.firstObject : NSTemporaryDirectory();
    NSString *appsupportFolder = [basePath stringByAppendingPathComponent:@"OpenEmu"];
    appsupportFolder = [appsupportFolder stringByAppendingPathComponent:@"Cores"];
    
    _fullPluginPath = [appsupportFolder stringByAppendingPathComponent:[archive nameOfEntry:0]];
    [archive extractTo:appsupportFolder];
    
    DLog(@"Core (%@) extracted to application support folder.", self.bundleIdentifier);
    
    OECorePlugin *plugin = [OECorePlugin pluginWithFileAtPath:_fullPluginPath type:[OECorePlugin class]];
    
    if (self.hasUpdate) {
        
        NSString *infoPlistPath = [plugin.bundle.bundlePath stringByAppendingPathComponent:@"Contents/Info.plist"];
        NSDictionary *infoPlist = [NSDictionary dictionaryWithContentsOfFile:infoPlistPath];
        
        self.version = [infoPlist[@"CFBundleVersion"] copy];
        self.canBeInstalled = NO;
        self.hasUpdate = NO;
        
    } else if (self.canBeInstalled) {
        
        [self OE_updatePropertiesWithPlugin:plugin];
    }
}

#pragma mark - Private

- (void)OE_updatePropertiesWithPlugin:(OECorePlugin *)plugin {
    
    self.name = [plugin.displayName copy];
    self.version = [plugin.version copy];
    self.hasUpdate = NO;
    self.canBeInstalled = NO;
    
    NSMutableArray <NSString *> *mutableSystemNames = [NSMutableArray arrayWithCapacity:plugin.systemIdentifiers.count];
    
    for (NSString *systemIdentifier in plugin.systemIdentifiers) {
        
        OESystemPlugin *plugin = [OESystemPlugin systemPluginForIdentifier:systemIdentifier];
        NSString *systemName = plugin.systemName;
        
        if (systemName != nil) {
            [mutableSystemNames addObject:systemName];
        }
    }
    
    self.systemNames = [mutableSystemNames copy];
    self.systemIdentifiers = [plugin.systemIdentifiers copy];
    self.bundleIdentifier = [plugin.bundleIdentifier copy];
}

@end
