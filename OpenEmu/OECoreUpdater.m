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

#import "OECoreUpdater.h"

#import <Sparkle/Sparkle.h>

#import "OECoreDownload.h"
#import "OECorePlugin.h"

#import "OEHUDAlert.h"
#import "OEHUDButtonCell.h"
@interface OECoreUpdater (Private)
- (void)updateCoreList;
@end
@implementation OECoreUpdater

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

@synthesize coreList, coresDict;

static OECoreUpdater *sharedController = nil;

// TODO: remove when feed holds correct ids
- (NSString*)lowerCaseID:(NSString*)mixedCaseID
{
    return [mixedCaseID lowercaseString];
}
#pragma mark -
+ (id)sharedUpdater
{
    if(sharedController==nil)
    {
        sharedController = [[self alloc] init];
    }
    return sharedController;
}

- (id)init {
    self = [super init];
    if (self) {
        isCheckingForAlertDownload = NO;
        
        coreList = [[NSMutableArray alloc] init];
        coresDict = [[NSMutableDictionary alloc] init];
        [[OECorePlugin allPlugins] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            OECoreDownload *aDownload = [[OECoreDownload alloc] initWithPlugin:obj];
            NSString *bundleID = [self lowerCaseID:[obj bundleIdentifier]];
            [coresDict setObject:aDownload forKey:bundleID];
        }];
        [self updateCoreList];
    }
    return self;
}

- (void)updateCoreList
{
    [self willChangeValueForKey:@"coreList"];
    
    coreList = [[[self coresDict] allValues] sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2)
    {
        return [[obj1 name] compare:[obj2 name]];
    }];
    [self didChangeValueForKey:@"coreList"];
}


- (void)checkForUpdates{
    if(![NSThread isMainThread])
    {
        [self performSelectorOnMainThread:@selector(checkForUpdates) withObject:nil waitUntilDone:NO];
        return;
    }
    
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
- (void)checkForNewCores:(NSNumber*)fromModal{
    NSURL         *coreListURL = [NSURL URLWithString:[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"]];
    NSXMLDocument *coreListDoc = [[NSXMLDocument alloc] initWithContentsOfURL:coreListURL options:0 error:NULL];
    NSArray       *coreNodes   = nil;
    
    if(coreListDoc != nil) coreNodes = [coreListDoc nodesForXPath:@"/cores/core" error:NULL];
    
    if(coreNodes != nil)
    {
        for(NSXMLElement *coreNode in coreNodes)
        {
            NSString *coreId = [self lowerCaseID:[[coreNode attributeForName:@"id"] stringValue]];
            if([[self coresDict] valueForKey:coreId] != nil) continue;
            
            OECoreDownload *download = [[OECoreDownload alloc] init];
            [download setName:[[coreNode attributeForName:@"name"] stringValue]];
            [download setDescription:elementChildAsString(coreNode, @"description")];
            [download setCanBeInstalled:YES];
            
            NSURL *appcastURL = [NSURL URLWithString:[[coreNode attributeForName:@"appcastURL"] stringValue]];
            download.appcast = [[SUAppcast alloc] init];
            [download.appcast setDelegate:self];
            
            if([fromModal boolValue])
                [download.appcast performSelectorOnMainThread:@selector(fetchAppcastFromURL:) withObject:appcastURL waitUntilDone:NO modes:[NSArray arrayWithObject:NSModalPanelRunLoopMode]];
            else
                [download.appcast performSelectorOnMainThread:@selector(fetchAppcastFromURL:) withObject:appcastURL waitUntilDone:NO];
                
            [[self coresDict] setObject:download forKey:coreId];
        }
    }
    [self updateCoreList];
}
#pragma mark -
#pragma mark Installing with OEHUDAlert
@synthesize completionHandler, coreIdentifier, alert, coreDownload;

- (void)installCoreWithIdentifier:(NSString*)aCoreIdentifier coreName:(NSString*)coreName systemName:(NSString*)systemName withCompletionHandler:(void (^)())handle
{
    OEHUDAlert *aAlert = [[OEHUDAlert alloc] init];
    aAlert.defaultButtonTitle = NSLocalizedString(@"Install", @"");
    aAlert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    
    aAlert.title = NSLocalizedString(@"Missing Core", @"");
    aAlert.messageText = [NSString stringWithFormat:NSLocalizedString(@"Unfortunately, in order to play %@ games you will need to install the '%@' Core", @""), systemName, coreName];
    
    [aAlert setDefaultButtonAction:@selector(startInstall) andTarget:self];
    
    self.coreIdentifier = aCoreIdentifier;
    self.completionHandler = handle;
    
    self.alert = aAlert;
    
    [self.alert runModal];
    
    self.coreDownload = nil;
    self.coreIdentifier = nil;
    self.completionHandler = nil;
    
    self.alert = nil;
    
}

- (void)cancelInstall
{
    self.completionHandler = nil;
    self.coreDownload = nil;
    [self.alert closeWithResult:NSAlertAlternateReturn];
    self.alert = nil;
    self.coreIdentifier = nil;
}

- (void)startInstall
{
    [self.alert setProgress:0.0];
    self.alert.headlineLabelText = NSLocalizedString(@"Downloading and Installing Core...", @"");
    self.alert.title = NSLocalizedString(@"Installing Core", @"");
    self.alert.showsProgressbar = YES;
    self.alert.defaultButtonTitle = nil;
    self.alert.messageText = nil;
    
    [self.alert setAlternateButtonAction:@selector(cancelInstall) andTarget:self];
    OECoreDownload *pluginDL = [[self coresDict] valueForKey:self.coreIdentifier];
    if(!pluginDL)
    {
        [self checkForNewCores:[NSNumber numberWithBool:YES]];
        pluginDL = [[self coresDict] valueForKey:self.coreIdentifier];
    }
    
    if(!pluginDL)
    {
        self.alert.showsProgressbar = NO;
        self.alert.headlineLabelText = nil;
        self.alert.title = NSLocalizedString(@"Error!", @"");
        
        self.alert.messageText = NSLocalizedString(@"The core could not be downloaded. Try installing it from the Cores preferences.", @"");
        self.alert.defaultButtonTitle = NSLocalizedString(@"OK", @"");
        self.alert.alternateButtonTitle = nil;
        
        [[self.alert.defaultButton cell] setButtonColor:OEHUDButtonColorRed];
        
        [self.alert setDefaultButtonAction:@selector(buttonAction:) andTarget:self.alert];
        return;
    }
    
    self.coreDownload = pluginDL;
    if(self.coreDownload.appcastItem)
    {
        [self.coreDownload startDownload:self];
    }
}

- (void)finishInstall
{
    [self.alert closeWithResult:NSAlertDefaultReturn];
    
    self.completionHandler();
    
    self.alert = nil;
    self.coreIdentifier = nil;
    self.completionHandler = nil;
}
#pragma mark -
#pragma mark OEDownload delegate
- (void)OEDownloadDidStart:(OECoreDownload *)download
{
    [download addObserver:self forKeyPath:@"progress" options:0xF context:nil];
}
- (void)OEDownloadDidFinish:(OECoreDownload *)download
{
    [download removeObserver:self forKeyPath:@"progress"];
    if(download == self.coreDownload)
    {
        [self finishInstall];
    }
}
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{    
    if(object == self.coreDownload)
    {
        self.alert.progress = self.coreDownload.progress;    
    }
    else 
    {
        [self updateCoreList];
    }
}

#pragma mark -
#pragma mark SUUpdater Delegate
- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update
{
    for(OECorePlugin *plugin in [OECorePlugin allPlugins])
    {
        if(updater == [SUUpdater updaterForBundle:plugin.bundle])
        {
            NSString *coreID = [self lowerCaseID:[plugin bundleIdentifier]];
            OECoreDownload *download = [coresDict objectForKey:coreID];
            [download setHasUpdate:YES];
            [download setAppcastItem:update];
            [download setDelegate:self];
            
            break;
        }
    }
    [self updateCoreList];
}
#pragma mark -
#pragma mark Appcast delegate
- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
    [[[self coresDict] allValues] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         if([obj appcast] == appcast)
         {
             //Assuming 0 is the best download, may or may not be the best
             NSArray *appcastItems = [appcast items];
             if([appcastItems count] > 0) [obj setAppcastItem:[appcastItems objectAtIndex:0]];
             
             [obj setDelegate:self];
             *stop = YES;
             
             if(obj == self.coreDownload)
             {
                 [self.coreDownload startDownload:self];
             }
         }
     }];
    [self updateCoreList];
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
    // Appcast couldn't load, remove it
    [[[self coresDict] allValues] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         if([obj appcast] == appcast)
         {
             [obj setAppcast:nil];
             *stop = YES;
         }
     }];
    [self updateCoreList];
}
@end
