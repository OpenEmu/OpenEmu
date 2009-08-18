/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "OECoreDownloader.h"
#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>
#import "OEDownload.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"

@implementation OECoreDownloader

@synthesize downloads;

- (id) init
{
    self = [super initWithWindowNibName:@"CoreDownloader"];
    if(self != nil)
    {
        
        NSString* string = [[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"];
        NSError *error;

        
        NSString* combinedList = [NSString stringWithContentsOfURL:[NSURL URLWithString:string] encoding:NSUTF8StringEncoding error:&error];
        
        NSArray * list = [combinedList componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
        
        NSMutableArray* tempURLList = [NSMutableArray array];
        
        NSArray* allPlugins = [OECorePlugin allPlugins];
        for( NSString* appcastString in list )
        {
            BOOL pluginExists = NO;
            
            NSURL* appcastURL = [NSURL URLWithString:appcastString];
            
            for( OECorePlugin* plugin in allPlugins )
            {
                @try {
                    SUUpdater* updater =  [SUUpdater updaterForBundle: [plugin bundle] ];
                    if ( [[updater feedURL] isEqual: appcastURL] )
                    {
                        pluginExists = YES;
                        break;
                    }
                }
                @catch (NSException * e) {
                    
                }

            
            }
            
            if( !pluginExists )            
                [tempURLList addObject:appcastURL];
        }
        
        urlList = [NSArray arrayWithArray:tempURLList];
        
        appcasts = [[NSMutableArray alloc] init];
        
        docController = [GameDocumentController sharedDocumentController];
        downloadToPathMap = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)windowDidLoad
{
    [self loadAppcasts];
}

- (void) dealloc
{
    [appcasts release];
    [urlList release];
    [super dealloc];
}

- (void)loadAppcasts
{
    
    for( NSURL* appcastURL in urlList )
    {
        SUAppcast* appcast = [[SUAppcast alloc] init];
        
        [appcast setDelegate:self];
        [appcast fetchAppcastFromURL:appcastURL];
        
        [appcasts addObject:appcast];
        [appcast release];
    }
    
}

- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
    OEDownload* downlad = [[OEDownload alloc] initWithAppcast:appcast];
    
    [downloadArrayController addObject:downlad];
    [downlad release];


    NSLog(@"Finished loading");
    
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
    [appcasts removeObject:appcast];
    //NSLog(@"Failed to load appcast %@", appcast );
}

- (IBAction)downloadSelectedCores:(id)sender
{
    for(OEDownload *download in downloads)
    {
        if( [download enabled] )
        {
            NSURLRequest *request = [NSURLRequest requestWithURL:[[download appcastItem] fileURL]];
            NSURLDownload *fileDownload = [[[NSURLDownload alloc] initWithRequest:request delegate:self] autorelease];
            
            if(fileDownload != nil)
                NSLog(@"Couldn't download!??");
        }
    }
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    NSString *destinationFilename;
    
    destinationFilename=[NSString stringWithCString:tmpnam(nil) 
                                           encoding:[NSString defaultCStringEncoding]];
    
    [downloadToPathMap setValue:download forKey:destinationFilename];
    [download setDestination:destinationFilename allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    // release the connection
    [download release];
    
    // inform the user
    NSLog(@"Download failed! Error - %@ %@",
          [error localizedDescription],
          [[error userInfo] objectForKey:NSErrorFailingURLStringKey]);
}

- (void) download: (NSURLDownload*)download didCreateDestination: (NSString*)path
{
     //  NSLog(@"%@",@"created dest");
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    NSString *path = nil;    
    for(NSString *key in [downloadToPathMap keyEnumerator])
        if([downloadToPathMap objectForKey:key] == download)
            path = key;
    
    XADArchive* archive = [XADArchive archiveForFile:path];
    
    NSString* appsupportFolder = [docController applicationSupportFolder];
    appsupportFolder = [appsupportFolder stringByAppendingPathComponent:@"Cores"];
    [archive extractTo:appsupportFolder];
    
    // release the connection
    [download release];
    
    // do something with the data
   // NSLog(@"downloadDidFinish to path %@",path);
    
    [[NSFileManager defaultManager] removeFileAtPath:path handler:nil];
}

@end
