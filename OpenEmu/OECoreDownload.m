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
#import "OECorePlugin.h"

#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>

#import "OESystemPlugin.h"

#import <OpenEmuBase/OpenEmuBase.h>

@interface OECoreDownload () <NSURLDownloadDelegate>
{
    NSString           *_downloadPath;
    NSString           *_fullPluginPath;

    unsigned long long  _expectedLength;
    unsigned long long  _downloadedSize;

    NSURLDownload *fileDownload;
}

@property(readwrite, getter=isDownloading) BOOL downloading;
@property(readwrite) double progress;

@end

@implementation OECoreDownload

#pragma mark -
#pragma mark Memory Management

- (id)init
{
    if((self = [super init]))
    {
        _expectedLength = 1;
    }
    return self;
}

- (id)initWithPlugin:(OECorePlugin *)plugin
{
    if((self = [self init]))
    {
        [self OE_setValuesUsingPlugin:plugin];
    }
    return self;
}

- (void)dealloc
{
    [self setDelegate:nil];
}

- (void)OE_setValuesUsingPlugin:(OECorePlugin *)plugin
{
    self.name = [[plugin displayName] copy];
    self.version = [[plugin version] copy];
    self.hasUpdate = NO;
    self.canBeInstalled = NO;

    NSMutableArray *mutableSystemNames = [NSMutableArray arrayWithCapacity:[[plugin systemIdentifiers] count]];
    [[plugin systemIdentifiers] enumerateObjectsUsingBlock:
     ^(NSString *systemIdentifier, NSUInteger idx, BOOL *stop)
     {
         OESystemPlugin *plugin = [OESystemPlugin systemPluginForIdentifier:systemIdentifier];
         NSString *systemName = [plugin systemName];

         if(systemName != nil) [mutableSystemNames addObject:systemName];
     }];

    self.systemNames = [mutableSystemNames copy];
    self.systemIdentifiers = [[plugin systemIdentifiers] copy];
    self.bundleIdentifier = [[plugin bundleIdentifier] copy];
}

#pragma mark Core Download

- (void)startDownload:(id)sender
{
    if(self.appcastItem == nil) return;

    NSURL *url = [_appcastItem fileURL];

    NSURLRequest  *request = [NSURLRequest requestWithURL:url];

    fileDownload = [[NSURLDownload alloc] initWithRequest:request delegate:self];
    self.downloading = YES;

    [[self delegate] coreDownloadDidStart:self];

    if(fileDownload == nil) NSLog(@"ERROR: Couldn't download %@", self);
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    _downloadPath = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"OEDownload.%@", [NSString stringWithUUID]]];
    [download setDestination:_downloadPath allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    if([[self delegate] respondsToSelector:@selector(coreDownloadDidFail:withError:)])
       [[self delegate] coreDownloadDidFail:self withError:error];
    else // inform the user
       [[NSApplication sharedApplication] presentError:error];
    self.downloading = NO;
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
    DLog(@"%@, %@", @"created dest", path);
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length
{
    _downloadedSize += length;
    [self willChangeValueForKey:@"progress"];
    self.progress = (double)_downloadedSize / _expectedLength;
    [self didChangeValueForKey:@"progress"];

    //NSLog(@"Got data:%f", (double) _downloadedSize /  (double) _expectedLength);
}

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response
{
    _expectedLength = [response expectedContentLength];
    DLog(@"Got response");
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    XADArchive *archive = nil;
    @try
    {
        archive = [XADArchive archiveForFile:_downloadPath];
    }
    @catch (NSException *exception)
    {
        archive = nil;
    }
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
    NSString *appsupportFolder = [basePath stringByAppendingPathComponent:@"OpenEmu"];
    appsupportFolder = [appsupportFolder stringByAppendingPathComponent:@"Cores"];

    _fullPluginPath = [appsupportFolder stringByAppendingPathComponent:[archive nameOfEntry:0]];
    [archive extractTo:appsupportFolder];

    // Delete the temp file
    [[NSFileManager defaultManager] removeItemAtPath:_downloadPath error:nil];

    OECorePlugin *plugin = [OECorePlugin pluginWithFileAtPath:_fullPluginPath type:[OECorePlugin class]];
    if(self.hasUpdate)
    {
        NSString *infoPlistPath = [[[plugin bundle] bundlePath] stringByAppendingPathComponent:@"Contents/Info.plist"];
        NSDictionary *infoPlist = [NSDictionary dictionaryWithContentsOfFile:infoPlistPath];

        self.version = [[infoPlist objectForKey:@"CFBundleVersion"] copy];
        self.canBeInstalled = NO;
        self.hasUpdate = NO;
    }
    else if(self.canBeInstalled)
        [self OE_setValuesUsingPlugin:plugin];

    self.downloading = NO;
    [self.delegate coreDownloadDidFinish:self];
}

- (void)cancelDownload:(id)sender
{
    [fileDownload cancel];
}
@end
