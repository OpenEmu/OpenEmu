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

#import "OECoreDownloader.h"
#import <Sparkle/Sparkle.h>
#import "OEDownload.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"

@implementation OECoreDownloader

@synthesize downloadAllCoresButton, downloadArrayController, downloadTableView;

#pragma mark Lifecycle

- (id)initWithWindowTitle:(NSString *)wtitle downloadAllButtonTitle:(NSString *)btitle
{
    if((self = [self initWithWindowNibName:@"CoreDownloader"]))
    {
        windowTitle = [wtitle copy];
        downloadAllButtonTitle = [btitle copy];
    }
    return self;
}

- (id)init
{
    return [self initWithWindowTitle:@"" downloadAllButtonTitle:@"Download All Cores"];
}

- (void)dealloc
{
    [downloadAllCoresButton release];
    [downloadArrayController release];
    [downloadTableView release];
    
    [windowTitle release];
    [downloadAllButtonTitle release];
    
    [super dealloc];
}

- (void)windowDidLoad
{
    [[self window] setTitle:windowTitle];
    [downloadAllCoresButton setTitle:downloadAllButtonTitle];
    NSSortDescriptor *nameSortDescriptor = [[[NSSortDescriptor alloc] initWithKey:@"downloadTitle"
                                                                        ascending:YES
                                                                         selector:@selector(localizedCaseInsensitiveCompare:)]
                                            autorelease];
    NSArray *sortDescriptors = [NSArray arrayWithObject:nameSortDescriptor];
    [downloadArrayController setSortDescriptors:sortDescriptors];
}

- (void)OEDownloadDidStart:(OEDownload *)download
{
    [downloadTableView setNeedsDisplay];
    
    BOOL allDownloading = YES;
    for(OEDownload *download in [downloadArrayController arrangedObjects])
    {
        if(![download isDownloading])
        {
            allDownloading = NO;
            break;
        }
    }
    
    [downloadAllCoresButton setEnabled:!allDownloading];
}

- (void)OEDownloadDidFinish:(OEDownload *)download
{
    [[GameDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[NSURL fileURLWithPath:[download fullPluginPath]] display:NO error:nil];
    [downloadArrayController removeObject:download];
    [downloadTableView setNeedsDisplay];
    if([[downloadArrayController arrangedObjects] count] == 0)
        [downloadAllCoresButton setEnabled:NO];
}

- (void)OEIconDownloadDidFinish:(OEDownload *)download
{
    [downloadTableView setNeedsDisplay];
}

- (IBAction)downloadAllCores:(id)sender
{
    for(OEDownload *download in [downloadArrayController arrangedObjects])
        if([download isEnabled] && ![download isDownloading])
            [download startDownload:self];
}

@end
