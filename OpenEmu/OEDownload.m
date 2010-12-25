/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OEDownload.h"
#import "GameDocumentController.h"
#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>
#import "OECoreInfo.h"

@implementation OEDownload

@synthesize coreInfo, enabled, appcastItem, progressBar, delegate, fullPluginPath, button, downloading;

- (id)init
{
    if(self = [super init])
    {
        enabled        = YES;
        downloading    = NO;
        downloadedSize = 0;
        expectedLength = 1;
        progressBar    = [[NSProgressIndicator alloc] init];
        
        [progressBar setControlSize:NSMiniControlSize];
        [progressBar setMinValue:0.0];
        [progressBar setMaxValue:1.0];
        [progressBar setStyle: NSProgressIndicatorBarStyle];
        [progressBar setIndeterminate:NO];
        
        button = [[NSButton alloc] init];
        [button setButtonType:NSMomentaryChangeButton];
        [button setImage:[NSImage imageNamed:@"download_arrow_up.png"]];
        [button setAlternateImage:[NSImage imageNamed:@"download_arrow_down.png"]];
        [button setAction:@selector(startDownload:)];
        [button setTarget:self];
        [button setBordered:NO];
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [self retain];
}

- (id)initWithCoreInfo:(OECoreInfo *)theCoreInfo
{
    if(self = [self init])
    {
		coreInfo = [theCoreInfo retain];
        enabled = YES;

        //Assuming 0 is the best download, may or may not be the best
        [self setAppcastItem:[[coreInfo.appcast items] objectAtIndex:0]];
        //NSLog(@"%@", [appcastItem propertiesDictionary]);
    }
    return self;
}

- (void)startDownload:(id)sender
{
    NSURLRequest  *request      = [NSURLRequest requestWithURL:[appcastItem fileURL]];
    NSURLDownload *fileDownload = [[[NSURLDownload alloc] initWithRequest:request delegate:self] autorelease];
    downloading = YES;
    if(fileDownload == nil) NSLog(@"ERROR: Couldn't download!?? %@", self);
}


- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{    
    downloadPath = [[NSString stringWithCString:tmpnam(nil) 
                                       encoding:[NSString defaultCStringEncoding]] retain];
    
    [download setDestination:downloadPath allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    // inform the user
    [[NSApplication sharedApplication] presentError:error];
    //    NSLog(@"Download failed! Error - %@ %@",
    //    [error localizedDescription],
    //    [[error userInfo] objectForKey:NSErrorFailingURLStringKey]);
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
      DLog(@"%@, %@", @"created dest", path);
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length
{
    downloadedSize += length;
    [self willChangeValueForKey:@"progress"];
    double progress = (double)downloadedSize / (double)expectedLength;
    [progressBar setDoubleValue:progress];
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
    
    NSString *appsupportFolder = [[GameDocumentController sharedDocumentController] applicationSupportFolder];
    appsupportFolder = [appsupportFolder stringByAppendingPathComponent:@"Cores"];
    
    fullPluginPath = [[appsupportFolder stringByAppendingPathComponent:[archive nameOfEntry:0]] retain];
    DLog(@"%@", fullPluginPath);
    [archive extractTo:appsupportFolder];
    
    // Delete the temp file
    [[NSFileManager defaultManager] removeItemAtPath:downloadPath error:nil];
    
    [progressBar removeFromSuperview];
    [delegate OEDownloadDidFinish:self];
}

- (NSString *)name
{
    return [NSString stringWithFormat:@"%@%@ %@",
			coreInfo.name,
			(coreInfo.coreDescription ? [NSString stringWithFormat:@" (%@)", coreInfo.coreDescription] : @""),
			[appcastItem title]];
}

- (void) dealloc
{
    [progressBar release];
    [downloadPath release];
    [fullPluginPath release];
    [button release];
	[coreInfo release];
    [self setAppcastItem:nil];
    
    [super dealloc];
}
@end
