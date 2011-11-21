//
//  OECoreDownload.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OECoreDownload.h"
#import "OECorePlugin.h"

#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>

#import "NSString+UUID.h"
@implementation OECoreDownload
@synthesize name, description, version;

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
        self.name = [plugin displayName];
        self.description = [plugin details];
        self.version = [plugin version];
        
        hasUpdate = NO;
        canBeInstalled = NO;
    }
    return self;
}
- (void)dealloc {
    [downloadPath release];
    
    [name release];
    [description release];
    [version release];
    
    [self setAppcastItem:nil];
    [self setAppcast:nil];
    self.delegate = nil;
    
    [super dealloc];
}

#pragma mark Core Download
- (void)startDownload:(id)sender
{
    NSURL* url = [appcastItem fileURL];
    
    NSURLRequest  *request      = [NSURLRequest requestWithURL:url];
    
    id <NSURLDownloadDelegate> urlDLDelegate = (id <NSURLDownloadDelegate>)self;
    NSURLDownload *fileDownload = [[[NSURLDownload alloc] initWithRequest:request delegate:urlDLDelegate] autorelease];
    downloading = YES;
    
    [[self delegate] OEDownloadDidStart:self];
    
    if(fileDownload == nil) NSLog(@"ERROR: Couldn't download %@", self);
}


- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{    
    downloadPath = [[NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"OEDownload.%@", [NSString stringWithUUID]]] retain];    
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
    
    fullPluginPath = [[appsupportFolder stringByAppendingPathComponent:[archive nameOfEntry:0]] retain];
    DLog(@"%@", fullPluginPath);
    [archive extractTo:appsupportFolder];
    
    // Delete the temp file
    [[NSFileManager defaultManager] removeItemAtPath:downloadPath error:nil];
    
    [delegate OEDownloadDidFinish:self];
}

@end
