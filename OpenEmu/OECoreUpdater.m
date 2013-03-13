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
#import "OEButton.h"

#import "OEDBGame.h"
#import "OEDBSystem.h"
#import "OEDBSaveState.h"

NSString *const OECoreUpdaterErrorDomain = @"OECoreUpdaterErrorDomain";

@interface OECoreUpdater ()
{
    NSMutableDictionary *_coresDict;
}

- (void)OE_updateCoreList;

@end

@implementation OECoreUpdater

// TODO: remove when feed holds correct ids
- (NSString *)lowerCaseID:(NSString*)mixedCaseID
{
    return [mixedCaseID lowercaseString];
}

#pragma mark -

+ (id)sharedUpdater
{
    static OECoreUpdater *sharedController = nil;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedController = [[self alloc] init];
    });

    return sharedController;
}

- (id)init
{
    if((self = [super init]))
    {
        _coresDict = [[NSMutableDictionary alloc] init];

        [[OECorePlugin allPlugins] enumerateObjectsUsingBlock:
         ^(id obj, NSUInteger idx, BOOL *stop)
         {
             OECoreDownload *aDownload = [[OECoreDownload alloc] initWithPlugin:obj];
             NSString *bundleID = [self lowerCaseID:[obj bundleIdentifier]];
             [_coresDict setObject:aDownload forKey:bundleID];
         }];

        [self OE_updateCoreList];
    }
    return self;
}

- (void)OE_updateCoreList
{
    [self willChangeValueForKey:@"coreList"];

    _coreList =
    [[_coresDict allValues] sortedArrayUsingComparator:
     ^ NSComparisonResult (id obj1, id obj2)
     {
         return [[obj1 name] compare:[obj2 name]];
     }];

    [self didChangeValueForKey:@"coreList"];
}


- (void)checkForUpdates
{
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
- (void)checkForNewCores:(NSNumber *)fromModal
{
    NSURL         *coreListURL = [NSURL URLWithString:[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"]];
    NSXMLDocument *coreListDoc = [[NSXMLDocument alloc] initWithContentsOfURL:coreListURL options:0 error:NULL];
    NSArray       *coreNodes   = nil;

    if(coreListDoc != nil) coreNodes = [coreListDoc nodesForXPath:@"/cores/core" error:NULL];

    if(coreNodes != nil)
    {
        for(NSXMLElement *coreNode in coreNodes)
        {
            NSString *coreId = [self lowerCaseID:[[coreNode attributeForName:@"id"] stringValue]];
            if([_coresDict objectForKey:coreId] != nil) continue;

            OECoreDownload *download = [[OECoreDownload alloc] init];
            [download setName:[[coreNode attributeForName:@"name"] stringValue]];

            NSArray *nodes = [coreNode nodesForXPath:@"./systems/system" error:NULL];
            NSMutableArray *systemNames = [NSMutableArray arrayWithCapacity:[nodes count]];
            NSMutableArray *systemIdentifiers = [NSMutableArray arrayWithCapacity:[nodes count]];
            [nodes enumerateObjectsUsingBlock:
             ^(NSXMLElement *systemNode, NSUInteger idx, BOOL *stop)
             {
                 NSString *systemName = [systemNode objectValue];
                 NSString *systemIdentifier = [[systemNode attributeForName:@"id"] objectValue];

                 [systemNames addObject:systemName];
                 [systemIdentifiers addObject:systemIdentifier];
             }];

            [download setSystemNames:systemNames];
            [download setSystemIdentifiers:systemIdentifiers];
            [download setCanBeInstalled:YES];

            NSURL *appcastURL = [NSURL URLWithString:[[coreNode attributeForName:@"appcastURL"] stringValue]];
            download.appcast = [[SUAppcast alloc] init];
            [download.appcast setDelegate:self];

            if([fromModal boolValue])
                [[download appcast] performSelectorOnMainThread:@selector(fetchAppcastFromURL:) withObject:appcastURL waitUntilDone:NO modes:[NSArray arrayWithObject:NSModalPanelRunLoopMode]];
            else
                [[download appcast] performSelectorOnMainThread:@selector(fetchAppcastFromURL:) withObject:appcastURL waitUntilDone:NO];

            [_coresDict setObject:download forKey:coreId];
        }
    }

    [self OE_updateCoreList];
}

#pragma mark -
#pragma mark Installing with OEHUDAlert

- (void)installCoreForGame:(OEDBGame*)game withCompletionHandler:(void(^)(NSError *error))handler
{
    NSString *systemIdentifier = [[game system] systemIdentifier];
    NSArray *validPlugins = [[self coreList] filteredArrayUsingPredicate:
                             [NSPredicate predicateWithBlock:
                              ^ BOOL (id evaluatedObject, NSDictionary *bindings)
                              {
                                  return [[evaluatedObject systemIdentifiers] containsObject:systemIdentifier];
                              }]];

    if([validPlugins count])
    {
        OECoreDownload *download = [validPlugins lastObject];
        NSString *coreName = [download name];
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"Unfortunately, in order to play %@ you will need to install the '%@' Core", @""), [game name], coreName];
        [self installCoreWithDownload:download message:message andCompletionHandler:handler];
    }
    else
    {
        NSError *error = [NSError errorWithDomain:OECoreUpdaterErrorDomain code:OENoDownloadableCoreForIdentifier userInfo:nil];
        handler(error);
    }
}

- (void)installCoreForSaveState:(OEDBSaveState*)state withCompletionHandler:(void(^)(NSError *error))handler
{
    NSString *aCoreIdentifier = [state coreIdentifier];
    OECoreDownload *download = [_coresDict objectForKey:[aCoreIdentifier lowercaseString]];
    if(download != nil)
    {
        NSString *coreName = [download name];
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"To launch the save state %@ you will need to install the '%@' Core", @""), [state name], coreName];
        [self installCoreWithDownload:download message:message andCompletionHandler:handler];
    }
    else
    {
        // TODO: create proper error saying that no core is available for the state
        NSError *error = [NSError errorWithDomain:OECoreUpdaterErrorDomain code:OENoDownloadableCoreForIdentifier userInfo:nil];
        handler(error);
    }
}

- (void)installCoreWithDownload:(OECoreDownload *)download message:(NSString *)message andCompletionHandler:(void(^)(NSError *error))handler
{
    OEHUDAlert *aAlert = [[OEHUDAlert alloc] init];
    [aAlert setDefaultButtonTitle:NSLocalizedString(@"Install", @"")];
    [aAlert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];

    [aAlert setTitle:NSLocalizedString(@"Missing Core", @"")];
    [aAlert setMessageText:message];

    [aAlert setDefaultButtonAction:@selector(startInstall) andTarget:self];

    NSString *aCoreIdentifier = [[_coresDict allKeysForObject:download] lastObject];
    [self setCoreIdentifier:aCoreIdentifier];
    [self setCompletionHandler:handler];

    [self setAlert:aAlert];

    [[self alert] runModal];

    [self setCompletionHandler:nil];
    [self setCoreDownload:nil];
    [self setCoreIdentifier:nil];

    [self setAlert:nil];
}

#pragma mark -

- (void)cancelInstall
{
    [self setCompletionHandler:nil];
    [self setCoreDownload:nil];
    [[self alert] closeWithResult:NSAlertAlternateReturn];
    [self setAlert:nil];
    [self setCoreIdentifier:nil];
}

- (void)startInstall
{
    [[self alert] setProgress:0.0];
    [[self alert] setHeadlineText:NSLocalizedString(@"Downloading and Installing Core…", @"")];
    [[self alert] setTitle:NSLocalizedString(@"Installing Core", @"")];
    [[self alert] setShowsProgressbar:YES];
    [[self alert] setDefaultButtonTitle:nil];
    [[self alert] setMessageText:nil];

    [[self alert] setAlternateButtonAction:@selector(cancelInstall) andTarget:self];

    OECoreDownload *pluginDL = [_coresDict objectForKey:[self coreIdentifier]];

    if(pluginDL == nil)
    {
        [self checkForNewCores:[NSNumber numberWithBool:YES]];
        pluginDL = [_coresDict objectForKey:[self coreIdentifier]];
    }

    if(pluginDL == nil)
    {
        [[self alert] setShowsProgressbar:NO];
        [[self alert] setHeadlineText:nil];
        [[self alert] setTitle:NSLocalizedString(@"Error!", @"")];

        [[self alert] setMessageText:NSLocalizedString(@"The core could not be downloaded. Try installing it from the Cores preferences.", @"")];
        [[self alert] setDefaultButtonTitle:NSLocalizedString(@"OK", @"")];
        [[self alert] setAlternateButtonTitle:nil];

        [[[self alert] defaultButton] setThemeKey:@"hud_button_red"];
        [[self alert] setDefaultButtonAction:@selector(buttonAction:) andTarget:[self alert]];
        return;
    }

    [self setCoreDownload:pluginDL];
    if([[self coreDownload] appcastItem] != nil)
    {
        [[self coreDownload] startDownload:self];
    }
}

- (void)finishInstall
{
    [[self alert] closeWithResult:NSAlertDefaultReturn];

    if([self completionHandler] != nil) [self completionHandler](nil);

    [self setAlert:nil];
    [self setCoreIdentifier:nil];
    [self setCompletionHandler:nil];
}
#pragma mark -
#pragma mark OEDownload delegate

static void *const _OECoreDownloadProgressContext = (void *)&_OECoreDownloadProgressContext;

- (void)coreDownloadDidStart:(OECoreDownload *)download
{
    [download addObserver:self forKeyPath:@"progress" options:0xF context:_OECoreDownloadProgressContext];
}

- (void)coreDownloadDidFinish:(OECoreDownload *)download
{
    [download removeObserver:self forKeyPath:@"progress" context:_OECoreDownloadProgressContext];
    if(download == [self coreDownload]) [self finishInstall];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context != _OECoreDownloadProgressContext)
        return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

    if(object == [self coreDownload])
        [[self alert] setProgress:[[self coreDownload] progress]];
    else
        [self OE_updateCoreList];
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
            OECoreDownload *download = [_coresDict objectForKey:coreID];
            [download setHasUpdate:YES];
            [download setAppcastItem:update];
            [download setDelegate:self];

            break;
        }
    }

    [self OE_updateCoreList];
}

#pragma mark -
#pragma mark Appcast delegate

- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
    [_coresDict enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop)
     {
         if([obj appcast] == appcast)
         {
             //Assuming 0 is the best download, may or may not be the best
             NSArray *appcastItems = [appcast items];
             if([appcastItems count] > 0) [obj setAppcastItem:[appcastItems objectAtIndex:0]];

             [obj setDelegate:self];
             *stop = YES;

             if(obj == [self coreDownload])
                 [[self coreDownload] startDownload:self];
         }
     }];

    [self OE_updateCoreList];
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
    // Appcast couldn't load, remove it
    [_coresDict enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop)
     {
         if([obj appcast] == appcast)
         {
             [obj setAppcast:nil];
             *stop = YES;
         }
     }];

    [self OE_updateCoreList];
}

@end
