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
#import "OEDownload.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"

@implementation OECoreDownloader

@synthesize downloads, downloadArrayController, downloadTableView;

- (id)init
{
    if(self = [super initWithWindowNibName:@"CoreDownloader"])
    {
        // FIXME: Never used, what's the point ?
        NSError *error = nil;
        
        // Get the URL for the list of available plugins
        NSString *coreURLs = [[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"];
        NSString *rawList = [NSString stringWithContentsOfURL:[NSURL URLWithString:coreURLs] encoding:NSUTF8StringEncoding error:&error];

        NSArray *list = [rawList componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
        
        NSMutableArray *tempURLList = [NSMutableArray array];
        
        NSArray *allPlugins = [OECorePlugin allPlugins];
        
        // Load the appcast for each core
        for(NSString *appcastString in list)
        {
            BOOL pluginExists = NO;
            NSURL *appcastURL = [NSURL URLWithString:appcastString];
            
            // Check if the core is already installed by checking against the appcast URL
            for(OECorePlugin* plugin in allPlugins )
            {
                if([[[plugin bundle] infoDictionary] valueForKey:@"SUFeedURL"])
                {
                    SUUpdater* updater = [SUUpdater updaterForBundle:[plugin bundle]];
                    if ([[updater feedURL] isEqual:appcastURL])
                    {
                        pluginExists = YES;
                        break;
                    }
                }    
            }
            
            if(!pluginExists) [tempURLList addObject:appcastURL];
        }
        
        urlList = [NSArray arrayWithArray:tempURLList];
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
    //Fetch all the appcasts
    for(NSURL *appcastURL in urlList)
    {
        SUAppcast *appcast = [[SUAppcast alloc] init];
        
        [appcast setDelegate:self];
        [appcast fetchAppcastFromURL:appcastURL];
        
        [appcasts addObject:appcast];
        [appcast release];
    }
}

- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
    OEDownload *downlad = [[OEDownload alloc] initWithAppcast:appcast];
    [downlad setDelegate:self];
    [downloadArrayController addObject:downlad];
    [downlad release];
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
    // Appcast couldn't load, remove it
    [appcasts removeObject:appcast];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:@"progress"])
    {
        [downloadTableView setNeedsDisplay];
    }
}

- (void)OEDownloadDidFinish:(OEDownload *)download;
{
    [downloadTableView setNeedsDisplay];
    [[GameDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[NSURL fileURLWithPath:[download fullPluginPath]] display:NO error:nil];
    [downloadArrayController removeObject:download];
}


@end
