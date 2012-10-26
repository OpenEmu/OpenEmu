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

#import "NSString+UUID.h"
#import "OESystemPlugin.h"
@implementation OECoreDownload
@synthesize name, systemNames, version;

@synthesize hasUpdate, canBeInstalled, downloading;
@synthesize progress;

@synthesize appcastItem, appcast;
@synthesize delegate;
#pragma mark -
#pragma mark Memory Management
- (id)init {
    self = [super init];
    if (self) {
        downloading = NO;
        downloadedSize = 0;
        expectedLength = 1;
        
        hasUpdate = NO;
        canBeInstalled = NO;
    }
    return self;
}
- (id)initWithPlugin:(OECorePlugin *)plugin
{
    self = [self init];
    if(self)
    {
        [self OE_setValuesUsingPlugin:plugin];
    }
    return self;
}
- (void)dealloc {
    self.delegate = nil;
}

- (void)OE_setValuesUsingPlugin:(OECorePlugin *)plugin
{
    self.name = [plugin displayName];
    self.version = [plugin version];
    self.hasUpdate = NO;
    self.canBeInstalled = NO;
    
    NSMutableString *mutableSystemNames = nil;
    for(NSString *aSystemIdentifier in [plugin systemIdentifiers])
    {
        OESystemPlugin *plugin = [OESystemPlugin gameSystemPluginForIdentifier:aSystemIdentifier];
        NSString *systemName = [plugin systemName];
        if(systemName != nil && mutableSystemNames == nil)
        {
            mutableSystemNames = [NSMutableString stringWithString:systemName];
        }
        else
        {
            [mutableSystemNames appendFormat:@", %@", systemName];
        }
    }
    self.systemNames = mutableSystemNames;
}
#pragma mark Core Download
- (void)startDownload:(id)sender
{
    if(!appcastItem) return;
    
    NSURL *url = [appcastItem fileURL];
    
    NSURLRequest  *request      = [NSURLRequest requestWithURL:url];
    
    id <NSURLDownloadDelegate> urlDLDelegate = (id <NSURLDownloadDelegate>)self;
    NSURLDownload *fileDownload = [[NSURLDownload alloc] initWithRequest:request delegate:urlDLDelegate];
    downloading = YES;
    
    [[self delegate] OEDownloadDidStart:self];
    
    if(fileDownload == nil) NSLog(@"ERROR: Couldn't download %@", self);
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    downloadPath = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"OEDownload.%@", [NSString stringWithUUID]]];
    [download setDestination:downloadPath allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    // inform the user
    [[NSApplication sharedApplication] presentError:error];
    
    downloading = NO;
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
    DLog(@"%@, %@", @"created dest", path);
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length
{
    downloadedSize += length;
    [self willChangeValueForKey:@"progress"];
    progress = (double)downloadedSize / expectedLength;
    [self didChangeValueForKey:@"progress"];
    
    //NSLog(@"Got data:%f", (double) downloadedSize /  (double) expectedLength);
}

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response
{
    expectedLength = [response expectedContentLength];
    DLog(@"Got response");
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    XADArchive *archive = [XADArchive archiveForFile:downloadPath];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
    NSString *appsupportFolder = [basePath stringByAppendingPathComponent:@"OpenEmu"];
    appsupportFolder = [appsupportFolder stringByAppendingPathComponent:@"Cores"];
    
    fullPluginPath = [appsupportFolder stringByAppendingPathComponent:[archive nameOfEntry:0]];
    DLog(@"%@", fullPluginPath);
    [archive extractTo:appsupportFolder];
    
    // Delete the temp file
    [[NSFileManager defaultManager] removeItemAtPath:downloadPath error:nil];
    
    if(self.canBeInstalled)
    {
        OECorePlugin *plugin = [OECorePlugin pluginWithBundleAtPath:fullPluginPath type:[OECorePlugin class]];
        [self OE_setInfoUsingPlugin:plugin];
    }
    downloading = NO;
    
    [delegate OEDownloadDidFinish:self];
}

@end
