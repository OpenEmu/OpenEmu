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

#import "OECoreUpdater.h"
#import "OECoreDownloader.h"
#import <Sparkle/Sparkle.h>
#import "OEDownload.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"

@implementation OECoreUpdater

#pragma mark Lifecycle

- (id)init
{
    return [self initWithWindowTitle:@"Update Cores" downloadAllButtonTitle:@"Update All Cores"];
}

- (void)loadUpdateList
{
    for(OECorePlugin *plugin in [OECorePlugin allPlugins])
    {
        NSString  *appcastURLString = [plugin.infoDictionary objectForKey:@"SUFeedURL"];
        SUUpdater *updater          = [SUUpdater updaterForBundle:plugin.bundle];
        
        if(updater != nil)
        {
            [updater setDelegate:self];
            [updater setFeedURL:[NSURL URLWithString:appcastURLString]];
            [updater resetUpdateCycle];
            [updater checkForUpdateInformation];
        }
    }
}

- (void)showWindow:(id)sender
{
    [downloadArrayController removeObjects:[downloadArrayController arrangedObjects]];
    [self loadUpdateList];
    [downloadTableView setNeedsDisplay];
    [super showWindow:sender];
}

#pragma mark SUUpdater Delegate

- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update
{
    for(OECorePlugin *plugin in [OECorePlugin allPlugins])
    {
        if(updater == [SUUpdater updaterForBundle:plugin.bundle])
        {
            OEDownload *download = [[[OEDownload alloc] init] autorelease];
            download.downloadTitle = plugin.displayName;
            download.downloadIcon  = plugin.icon;
            download.appcastItem   = update;
            [download setDelegate:self];
            
            [downloadArrayController addObject:download];
            [downloadAllCoresButton setEnabled:YES];
            
            break;
        }
    }
}

@end
