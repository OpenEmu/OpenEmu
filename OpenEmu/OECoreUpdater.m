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
@import OpenEmuKit;

#import "OEAlert.h"

#import "OEDBGame+CoreDataProperties.h"
#import "OEDBSystem+CoreDataProperties.h"
#import "OEDBSaveState+CoreDataProperties.h"

NSString *const OECoreUpdaterErrorDomain = @"OECoreUpdaterErrorDomain";

@interface OECoreUpdater () <NSFileManagerDelegate, SUUpdaterDelegate>
{
    NSMutableDictionary<NSString *, OECoreDownload *> *_coresDict;
    BOOL autoInstall;
    NSURLSessionDataTask *_lastCoreListURLTask;
    NSMutableSet<OECoreDownload *> *_pendingUserInitiatedDownloads;
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
        _pendingUserInitiatedDownloads = [[NSMutableSet alloc] init];

        autoInstall = NO;

        [[OECorePlugin allPlugins] enumerateObjectsUsingBlock:
         ^(id obj, NSUInteger idx, BOOL *stop)
         {
             OECoreDownload *aDownload = [[OECoreDownload alloc] initWithPlugin:obj];
             NSString *bundleID = [self lowerCaseID:[obj bundleIdentifier]];
             if(bundleID != nil)
             {
                 [self->_coresDict setObject:aDownload forKey:bundleID];
             }
             else
             {
                 NSLog(@"Warning: There's a CorePlugin without bundle identifier");
             }
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
         return [[obj1 name] localizedCaseInsensitiveCompare:[obj2 name]];
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

            // Core updates are silently installed on launch, so ensure there is no annoying update prompt from Sparkle
            [updater setAutomaticallyChecksForUpdates:YES];
            [updater setAutomaticallyDownloadsUpdates:YES];

            [updater resetUpdateCycle];
            [updater checkForUpdateInformation];
        }
    }
}

- (void)checkForUpdatesAndInstall
{
    autoInstall = YES;
    [self checkForUpdates];
}

- (void)checkForNewCoresWithCompletionHandler:(nullable void(^)(NSError *error))handler
{
    if (_lastCoreListURLTask) {
        if (handler)
            handler([NSError errorWithDomain:OECoreUpdaterErrorDomain code:OENewCoreCheckAlreadyPendingError userInfo:nil]);
        return;
    }

    NSURL *coreListURL = [NSURL URLWithString:[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"]];
    
    _lastCoreListURLTask = [[NSURLSession sharedSession]
            dataTaskWithURL:coreListURL
            completionHandler:^(NSData * _Nullable download, NSURLResponse * _Nullable response, NSError * _Nullable dlError) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if (!download) {
                if (handler)
                    handler(dlError);
                self->_lastCoreListURLTask = nil;
                return;
            }
            
            NSXMLDocument *coreListDoc = [[NSXMLDocument alloc] initWithData:download options:0 error:NULL];
            NSArray       *coreNodes   = nil;
            
            if(coreListDoc != nil) coreNodes = [coreListDoc nodesForXPath:@"/cores/core" error:NULL];
            
            if(coreNodes != nil)
            {
                for(NSXMLElement *coreNode in coreNodes)
                {
                    NSString *coreId = [self lowerCaseID:[[coreNode attributeForName:@"id"] stringValue]];
                    if([self->_coresDict objectForKey:coreId] != nil) continue;
                    
                    OECoreDownload *download = [[OECoreDownload alloc] init];
                    [download setName:[[coreNode attributeForName:@"name"] stringValue]];
                    [download setBundleIdentifier:coreId];
                    
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

                    dispatch_async(dispatch_get_main_queue(), ^{
                        [[download appcast] fetchAppcastFromURL:appcastURL inBackground:YES completionBlock:^(NSError *error) {
                            if (error) {
                                NSLog(@"%@", error);
                            } else {
                                [self appcastDidFinishLoading:[download appcast]];
                            }
                        }];
                    });

                    [self->_coresDict setObject:download forKey:coreId];
                }
            }

            [self OE_updateCoreList];
            
            if (handler)
                handler(nil);
            self->_lastCoreListURLTask = nil;
        });
    }];
    [_lastCoreListURLTask resume];
}

- (void)cancelCheckForNewCores
{
    if (_lastCoreListURLTask) {
        [_lastCoreListURLTask cancel];
    }
    _lastCoreListURLTask = nil;
}

#pragma mark -
#pragma mark Installing with OEAlert

- (void)installCoreForGame:(OEDBGame *)game withCompletionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler
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
        OECoreDownload *download = nil;

        if([validPlugins count] == 1)
            download = [validPlugins lastObject];
        else
        {
            // Sort by core name alphabetically to match our automatic Core Picker behavior
            validPlugins = [validPlugins sortedArrayUsingComparator:
                            ^ NSComparisonResult (id obj1, id obj2)
                            {
                                return [[obj1 name] compare:[obj2 name]];
                            }];

            // Check if a core is set as default in OEApplicationDelegate
            __block BOOL didFindDefaultCore = NO;
            __block NSUInteger foundDefaultCoreIndex = 0;

            [validPlugins enumerateObjectsUsingBlock:^(id plugin, NSUInteger idx, BOOL *stop)
            {
                NSString *sysID = [NSString stringWithFormat:@"defaultCore.%@", systemIdentifier];
                NSString *userDef = [[NSUserDefaults standardUserDefaults] valueForKey:sysID];

                if(userDef != nil && [userDef caseInsensitiveCompare:[plugin bundleIdentifier]] == NSOrderedSame)
                {
                    didFindDefaultCore = YES;
                    foundDefaultCoreIndex = idx;
                    *stop = YES;
                }
            }];

            // Use default core plugin for this system, otherwise just use first found from the sorted list
            if(didFindDefaultCore)
                download = [validPlugins objectAtIndex:foundDefaultCoreIndex];
            else
                download = [validPlugins objectAtIndex:0];
        }

        NSString *coreName = [download name];
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"OpenEmu uses 'Cores' to emulate games. You need the %@ Core to play %@", @""), coreName, [game displayName]];
        [self installCoreWithDownload:download message:message completionHandler:handler];
    }
    else
    {
        NSError *error = [NSError errorWithDomain:OECoreUpdaterErrorDomain code:OENoDownloadableCoreForIdentifierError userInfo:nil];
        handler(nil, error);
    }
}

- (void)installCoreForSaveState:(OEDBSaveState *)state withCompletionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler
{
    NSString *aCoreIdentifier = [state coreIdentifier];
    OECoreDownload *download = [_coresDict objectForKey:[aCoreIdentifier lowercaseString]];
    if(download != nil)
    {
        NSString *coreName = [download name];
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"To launch the save state %@ you will need to install the '%@' Core", @""), [state name], coreName];
        [self installCoreWithDownload:download message:message completionHandler:handler];
    }
    else
    {
        // TODO: create proper error saying that no core is available for the state
        NSError *error = [NSError errorWithDomain:OECoreUpdaterErrorDomain code:OENoDownloadableCoreForIdentifierError userInfo:nil];
        handler(nil, error);
    }
}

- (void)installCoreWithDownload:(OECoreDownload *)download message:(NSString *)message completionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler
{
    OEAlert *aAlert = [[OEAlert alloc] init];
    aAlert.messageText = NSLocalizedString(@"Missing Core", @"");
    aAlert.informativeText = message;
    aAlert.defaultButtonTitle = NSLocalizedString(@"Install", @"");
    aAlert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    [aAlert setDefaultButtonAction:@selector(startInstall) andTarget:self];

    NSString *coreIdentifier = [[_coresDict allKeysForObject:download] lastObject];
    [self setCoreIdentifier:coreIdentifier];
    [self setCompletionHandler:handler];

    [self setAlert:aAlert];

    NSUInteger result = [[self alert] runModal];
    if(result == NSAlertSecondButtonReturn)
    {
        handler(nil, [NSError errorWithDomain:NSCocoaErrorDomain code:NSUserCancelledError userInfo:nil]);
    }

    [self setCompletionHandler:nil];
    [self setCoreDownload:nil];
    [self setCoreIdentifier:nil];

    [self setAlert:nil];
}

- (void)installCoreWithDownload:(OECoreDownload *)download completionHandler:(void(^)(OECorePlugin *plugin, NSError *error))handler
{
    OEAlert *aAlert = [[OEAlert alloc] init];
    
    NSString *coreIdentifier = [[_coresDict allKeysForObject:download] lastObject];
    [self setCoreIdentifier:coreIdentifier];
    [self setCompletionHandler:handler];
    [self setAlert:aAlert];
    
    [aAlert performBlockInModalSession:^{
        [self startInstall];
    }];
    [aAlert runModal];
    
    [self setCompletionHandler:nil];
    [self setCoreDownload:nil];
    [self setCoreIdentifier:nil];

    [self setAlert:nil];
}

#pragma mark -

- (void)cancelInstall
{
    [[self coreDownload] cancelDownload:self];
    [self setCompletionHandler:nil];
    [self setCoreDownload:nil];
    [[self alert] closeWithResult:NSAlertSecondButtonReturn];
    [self setAlert:nil];
    [self setCoreIdentifier:nil];
}

- (void)startInstall
{
    self.alert.messageText = NSLocalizedString(@"Downloading and Installing Core…", @"");
    self.alert.informativeText = nil;
    self.alert.defaultButtonTitle = nil;
    [self.alert setAlternateButtonAction:@selector(cancelInstall) andTarget:self];
    self.alert.showsProgressbar = YES;
    self.alert.progress = 0.0;

    OECoreDownload *pluginDL = [_coresDict objectForKey:[self coreIdentifier]];

    if(pluginDL == nil)
    {
        self.alert.messageText = NSLocalizedString(@"Error!", @"");
        self.alert.informativeText = NSLocalizedString(@"The core could not be downloaded. Try installing it from the Cores preferences.", @"");
        self.alert.defaultButtonTitle = NSLocalizedString(@"OK", @"");
        self.alert.alternateButtonTitle = nil;
        [self.alert setDefaultButtonAction:@selector(buttonAction:) andTarget:self.alert];
        self.alert.showsProgressbar = NO;
        
        return;
    }

    [self setCoreDownload:pluginDL];
    if([[self coreDownload] appcastItem] != nil)
        [[self coreDownload] startDownload:self];
}

- (void)failInstallWithError:(NSError*)error
{
    [[self alert] closeWithResult:NSAlertFirstButtonReturn];

    if([self completionHandler] != nil) [self completionHandler]([OECorePlugin corePluginWithBundleIdentifier:[self coreIdentifier]], error);

    [self setAlert:nil];
    [self setCoreIdentifier:nil];
    [self setCompletionHandler:nil];
}

- (void)finishInstall
{
    [[self alert] closeWithResult:NSAlertFirstButtonReturn];

    if([self completionHandler] != nil)
    {
        OECorePlugin *plugin = [OECorePlugin corePluginWithBundleIdentifier:[self coreIdentifier]];
        [self completionHandler](plugin, nil);
    }

    [self setAlert:nil];
    [self setCoreIdentifier:nil];
    [self setCompletionHandler:nil];
}

#pragma mark - Other user-initiated (= with error reporting) downloads

- (void)installCoreInBackgroundUserInitiated:(OECoreDownload *)download
{
    if (download.delegate != self) {
        NSLog(@"download %@'s delegate is not the singleton OECoreUpdater!?", download);
    } else {
        [_pendingUserInitiatedDownloads addObject:download];
    }
    [download startDownload:self];
}

#pragma mark -
#pragma mark OEDownload delegate

static void *const _OECoreDownloadProgressContext = (void *)&_OECoreDownloadProgressContext;

- (void)coreDownloadDidStart:(OECoreDownload *)download
{
    [self OE_updateCoreList];
    [download addObserver:self forKeyPath:@"progress" options:0xF context:_OECoreDownloadProgressContext];
}

- (void)coreDownloadDidFinish:(OECoreDownload *)download
{
    [download removeObserver:self forKeyPath:@"progress" context:_OECoreDownloadProgressContext];
    [self OE_updateCoreList];
    if(download == [self coreDownload])
        [self finishInstall];
    [_pendingUserInitiatedDownloads removeObject:download];
}

- (void)coreDownloadDidFail:(OECoreDownload*)download withError:(NSError*)error
{
    [self OE_updateCoreList];
    if(download == [self coreDownload])
        [self failInstallWithError:error];
    if ([_pendingUserInitiatedDownloads containsObject:download] && error) {
        [NSApp presentError:error];
    }
    [_pendingUserInitiatedDownloads removeObject:download];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context != _OECoreDownloadProgressContext)
        return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

    if(object == self.coreDownload)
        self.alert.progress = self.coreDownload.progress;
}

#pragma mark -
#pragma mark SUUpdater Delegate

- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update
{
    for(OECorePlugin *plugin in [OECorePlugin allPlugins])
    {
        if(updater != [SUUpdater updaterForBundle:plugin.bundle])
            continue;

        NSString *coreID = [self lowerCaseID:[plugin bundleIdentifier]];
        OECoreDownload *download = [_coresDict objectForKey:coreID];
        [download setHasUpdate:YES];
        [download setAppcastItem:update];
        [download setDelegate:self];

        if(autoInstall)
            [download startDownload:nil];

        break;
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
         // This runs when the core is not installed at all. Since
         // Sparkle's update checking logic is all in the SUUpdater,
         // which is tied to an existing bundle (which doesn't exist,
         // since it hasn't been installed yet), it's necessary to
         // reproduce the operating system version requirement check
         // here. There is no existing version to check against. This
         // code assumes that the newest versions are always ordered
         // first. As a result, maximum system version checks are not
         // implemented.
         if([obj appcast] == appcast)
         {
             NSOperatingSystemVersion OSVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
             NSString *OSVersionString = [NSString stringWithFormat:@"%ld.%ld.%ld", (long)OSVersion.majorVersion, (long)OSVersion.minorVersion, (long)OSVersion.patchVersion];
             for (SUAppcastItem *item in [appcast items]) {
                 if (item.minimumSystemVersion == nil || [item.minimumSystemVersion isEqualToString:@""] ||
                     ([[SUStandardVersionComparator defaultComparator]
                       compareVersion:item.minimumSystemVersion
                       toVersion:OSVersionString] != NSOrderedDescending))
                 {
                     [obj setAppcastItem:item];
                     break;
                 }
             }

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
