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
#import "OECoreInfo.h"

@implementation OECoreDownloader

@synthesize downloadArrayController, downloadTableView;

static NSString *elementChildAsString(NSXMLElement *element, NSString *name) {
	NSString *value = nil;
	NSArray *nodes = [element elementsForName:name];
	if ([nodes count] > 0) {
		NSXMLElement *childNode = [nodes objectAtIndex:0];
		value = [childNode stringValue];
	}
	return value;
}

- (id)init
{
    return [self initWithWindowNibName:@"CoreDownloader"];
}

- (id)initWithWindowNibName:(NSString *)windowNibName
{
    self = [super initWithWindowNibName:windowNibName];
    if(self != nil)
    {
        NSError *error = nil;
        
        // Get the URL for the list of available plugins
		// <core id="" name="" appcastURL="">
		//   <description>...</description> (optional)
		//   <iconURL>...</iconURL> (optional)
		// </core>
        // NSURL *coreListURL = [NSURL URLWithString:[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"]];
		NSURL *coreListURL = [NSURL URLWithString:@"http://f.cl.ly/items/2G1B0b391I40141o3i2S/oecores.xml"];
		NSXMLDocument *coreListDoc = [[[NSXMLDocument alloc] initWithContentsOfURL:coreListURL options:0 error:&error] autorelease];
		NSArray *coreNodes = nil;
		
		if(coreListDoc) coreNodes = [coreListDoc nodesForXPath:@"/cores/core" error:NULL];
		
		if(coreNodes)
		{
			NSMutableArray *tempAvailableCores = [NSMutableArray arrayWithCapacity:[coreNodes count]];
			NSArray *allPlugins = [OECorePlugin allPlugins];
			
			for(NSXMLElement *coreNode in coreNodes)
			{
				OECoreInfo *core = [[[OECoreInfo alloc] init] autorelease];
				core.coreID = [[coreNode attributeForName:@"id"] stringValue];
				core.name = [[coreNode attributeForName:@"name"] stringValue];
				core.appcastURL = [NSURL URLWithString:[[coreNode attributeForName:@"appcastURL"] stringValue]];
				core.coreDescription = elementChildAsString(coreNode, @"description");
				
				NSString *iconURLString = elementChildAsString(coreNode, @"iconURL");
				if (iconURLString) core.iconURL = [NSURL URLWithString:iconURLString];
				
				NSLog(@"available core: %@", core);

				// Check whether the core is already installed
				BOOL pluginExists = NO;
				for(OECorePlugin *plugin in allPlugins)
				{
					if([[[plugin bundle] infoDictionary] valueForKey:@"SUFeedURL"])
					{
						SUUpdater* updater = [SUUpdater updaterForBundle:[plugin bundle]];
						if ([[updater feedURL] isEqual:core.appcastURL])
						{
							pluginExists = YES;
							break;
						}
					}					
				}
				
				if (! pluginExists) [tempAvailableCores addObject:core];
			}
			
			availableCores = [tempAvailableCores copy];
		}		
    }
	
    return self;
}

- (void)windowDidLoad
{
    [self loadAppcasts];
}

- (void)dealloc
{
	[availableCores          release];
    [downloadArrayController release];
    [downloadTableView       release];
    [super                   dealloc];
}

- (void)loadAppcasts
{
    // Fetch all the appcasts
    for(OECoreInfo *core in availableCores)
    {
		core.appcast = [[[SUAppcast alloc] init] autorelease];
		[core.appcast setDelegate:self];
		[core.appcast fetchAppcastFromURL:core.appcastURL];
    }
}

- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
	for(OECoreInfo *core in availableCores)
	{
		if(core.appcast == appcast)
		{
			OEDownload *download = [[[OEDownload alloc] initWithCoreInfo:core] autorelease];
			[download setDelegate:self];
			[downloadArrayController addObject:download];
			break;
		}
	}
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
    // Appcast couldn't load, remove it
	for(OECoreInfo *core in availableCores)
	{
		if (core.appcast == appcast) {
			core.appcast = nil;
			break;
		}
	}
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:@"progress"])
        [downloadTableView setNeedsDisplay];
}

- (void)OEDownloadDidFinish:(OEDownload *)download;
{
    [downloadTableView setNeedsDisplay];
    [[GameDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[NSURL fileURLWithPath:[download fullPluginPath]] display:NO error:nil];
    [downloadArrayController removeObject:download];
}

- (IBAction)openCoreDownloaderWindow:(id)sender
{
    [self close];
}

@end
