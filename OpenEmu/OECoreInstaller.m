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

#import "OECoreInstaller.h"
#import "OECoreDownloader.h"
#import <Sparkle/Sparkle.h>
#import "OEDownload.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"

@implementation OECoreInstaller

static NSString *elementChildAsString(NSXMLElement *element, NSString *name)
{
    NSString *value = nil;
    NSArray *nodes = [element elementsForName:name];
    if([nodes count] > 0)
    {
        NSXMLElement *childNode = [nodes objectAtIndex:0];
        value = [childNode stringValue];
    }
    return value;
}

#pragma mark -
#pragma mark Lifecycle

- (id)init
{
    if((self = [self initWithWindowTitle:@"Install New Cores" downloadAllButtonTitle:@"Install All Cores"]))
    {
        availableDownloads = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)dealloc
{
    [availableDownloads release];
    [super dealloc];
}

- (void)loadCoreList
{
    NSURL         *coreListURL = [NSURL URLWithString:[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"]];
    NSXMLDocument *coreListDoc = [[[NSXMLDocument alloc] initWithContentsOfURL:coreListURL options:0 error:NULL] autorelease];
    NSArray       *coreNodes   = nil;
    
    if(coreListDoc != nil) coreNodes = [coreListDoc nodesForXPath:@"/cores/core" error:NULL];
    
    if(coreNodes != nil)
    {
        NSMutableArray *installedPluginIds = [NSMutableArray array];
        
        for(OECorePlugin *plugin in [OECorePlugin allPlugins])
            [installedPluginIds addObject:[[plugin infoDictionary] objectForKey:@"CFBundleIdentifier"]];
        
        for(NSXMLElement *coreNode in coreNodes)
        {
            NSString *coreId = [[coreNode attributeForName:@"id"] stringValue];
            
            __block BOOL alreadyInstalled = NO;
            [installedPluginIds enumerateObjectsUsingBlock:
             ^(NSString *installedPluginId, NSUInteger idx, BOOL *stop)
             {
                 if([coreId caseInsensitiveCompare:installedPluginId] == NSOrderedSame)
                     alreadyInstalled = *stop = YES;
             }];
            
            if(alreadyInstalled) continue;
            
            OEDownload *download = [[[OEDownload alloc] init] autorelease];
            download.downloadTitle = [[coreNode attributeForName:@"name"] stringValue];
            download.downloadDescription = elementChildAsString(coreNode, @"description");
            
            NSString *iconURLString = elementChildAsString(coreNode, @"iconURL");
            if(iconURLString) [download downloadIconFromURL:[NSURL URLWithString:iconURLString]];
            
            NSURL *appcastURL = [NSURL URLWithString:[[coreNode attributeForName:@"appcastURL"] stringValue]];
            download.appcast = [[[SUAppcast alloc] init] autorelease];
            [download.appcast setDelegate:self];
            [download.appcast fetchAppcastFromURL:appcastURL];
            
            [availableDownloads addObject:download];
        }
    }
}

- (void)showWindow:(id)sender
{
    [downloadArrayController removeObjects:[downloadArrayController arrangedObjects]];
    [self loadCoreList];
    [downloadTableView setNeedsDisplay];
    [super showWindow:sender];
}


#pragma mark Appcast delegate

- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
    [availableDownloads enumerateObjectsUsingBlock:
     ^(OEDownload *download, NSUInteger idx, BOOL *stop)
     {
         if(download.appcast == appcast)
         {
             //Assuming 0 is the best download, may or may not be the best
             NSArray *appcastItems = [appcast items];
             if([appcastItems count] > 0) download.appcastItem = [appcastItems objectAtIndex:0];
             
             [download setDelegate:self];
             [downloadArrayController addObject:download];
             [downloadAllCoresButton setEnabled:YES];
             *stop = YES;
         }
     }];
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
    // Appcast couldn't load, remove it
    [availableDownloads enumerateObjectsUsingBlock:
     ^(OEDownload *download, NSUInteger idx, BOOL *stop)
     {
         if(download.appcast == appcast)
         {
             download.appcast = nil;
             *stop = YES;
         }
     }];
}

#pragma mark IB Actions

- (IBAction)openCoreInstallerWindow:(id)sender
{
    [self close];
}

@end
