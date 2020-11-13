/*
 Copyright (c) 2020, OpenEmu Team
 
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


#import "OEPrefDebugController.h"
#import "OELibraryDatabase.h"
#import "OESidebarController.h"

#import "OEDBGame.h"
#import "OEDBImage.h"
#import "OEDBRom.h"
#import "OEDBSaveState.h"

#import "OEGameControlsBar.h"

#import "OEGameViewController.h"

#import "OEXPCGameCoreManager.h"

#import "OEGameDocument.h"

#import "OEGameInfoHelper.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "OEAlert.h"
#import "NSFileManager+OEHashingAdditions.h"

#pragma mark Key sources
#import "OESetupAssistant.h"
#import "OECollectionViewController.h"
#import "OEGameViewController.h"
#import "OEGridGameCell.h"
#import "OEControllerImageView.h"
#import "OEControlsButtonSetupView.h"
#import "OEDBDataSourceAdditions.h"
#import "OEImportOperation.h"
#import "OEPrefBiosController.h"
#import "OEMainWindowController.h"
#import "OELibraryGamesViewController.h"
#import "OEDBSavedGamesMedia.h"

#import "OpenEmu-Swift.h"

NSString * const OEAppearancePreferenceKey = @"OEAppearance";
NSString * const OEHUDBarAppearancePreferenceKey = @"OEHUDBarAppearance";
NSString * const OEControlsPrefsAppearancePreferenceKey = @"OEControlsPrefsAppearance";

@interface OELibraryDatabase (Private)
- (void)OE_createInitialItems;
@end

@interface OEPrefDebugController () <NSTableViewDelegate, NSTableViewDataSource>
@property NSArray *keyDescriptions;
@property (nonatomic, readonly) NSWindow *mainWindow;
@end

NSString * const  CheckboxType  = @"Checkbox";
NSString * const  ButtonType    = @"Button";
NSString * const  SeparatorType = @"Separator";
NSString * const  GroupType     = @"Group";
NSString * const  ColorType     = @"Color";
NSString * const  LabelType     = @"Label";
NSString * const  PopoverType   = @"Popover";
NSString * const  NumericTextFieldType = @"NumericTextField";

NSString * const TypeKey  = @"type";
NSString * const LabelKey = @"label";
NSString * const KeyKey   = @"key";
NSString * const ActionKey = @"action";
NSString * const NegatedKey = @"negated";
NSString * const ValueKey = @"value";
NSString * const OptionsKey = @"options";
NSString * const NumberFormatterKey = @"numberFormatter";

#define Separator() \
@{ TypeKey:SeparatorType }
#define FirstGroup(_NAME_) \
@{ TypeKey:GroupType, LabelKey:NSLocalizedStringFromTable(_NAME_, @"Debug", @"Debug Group") }
#define Group(_NAME_) Separator(), \
@{ TypeKey:GroupType, LabelKey:NSLocalizedStringFromTable(_NAME_, @"Debug", @"Debug Group") }
#define Group2(_NAME_) Separator(), \
@{ TypeKey:GroupType, LabelKey:NSLocalizedString(_NAME_, @"Debug Group") }
#define Checkbox(_KEY_, _LABEL_)  \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"Debug Checkbox Label"), TypeKey:CheckboxType }
#define NCheckbox(_KEY_, _LABEL_) \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"Debug Checkbox Label"), TypeKey:CheckboxType, NegatedKey:@YES }
#define Button(_LABEL_, _ACTION_)  \
@{ LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"Debug Button Label"), TypeKey:ButtonType, ActionKey:NSStringFromSelector(_ACTION_) }
#define Label(_LABEL_)  \
@{ LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"Debug Label"), TypeKey:LabelType }
#define Popover(_LABEL_, _ACTION_, ...)  \
@{ LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"Debug Popover Label"), TypeKey:PopoverType, ActionKey:NSStringFromSelector(_ACTION_), OptionsKey:@[__VA_ARGS__] }
#define Option(_OLABEL_, _OVAL_) \
@{ LabelKey:NSLocalizedStringFromTable(_OLABEL_, @"Debug", @"Debug Popover Menu Item Label"), ValueKey:_OVAL_ }
#define ColorWell(_KEY_, _LABEL_) \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"DebugModeLabel"), TypeKey:ColorType }
#define NumberTextBox(_KEY_, _LABEL_, _FORMATTER_) \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedStringFromTable(_LABEL_, @"Debug", @"Debug Label"), NumberFormatterKey:_FORMATTER_, TypeKey:NumericTextFieldType }

@implementation OEPrefDebugController

@synthesize viewSize;

- (void)awakeFromNib
{
    if([self keyDescriptions] != nil) return;
    [self OE_setupKeyDescription];
	[self setupGridView];
}

- (void)OE_setupKeyDescription
{
    NSNumberFormatter *adcSensitivityNF = [[NSNumberFormatter alloc] init];
    [adcSensitivityNF setAllowsFloats:YES];
    [adcSensitivityNF setMinimum:@0.01];
    [adcSensitivityNF setMaximum:@0.99];
    [adcSensitivityNF setNumberStyle:NSNumberFormatterDecimalStyle];
    
    self.keyDescriptions =  @[
                              FirstGroup(@"General"),
                              Checkbox([OEPreferencesWindowController debugModeKey], @"Debug Mode"),
                              Checkbox(OESetupAssistantHasFinishedKey, @"Setup Assistant has finished"),
                              Popover(@"Region:", @selector(changeRegion:),
                                      Option(@"Auto (region)", @(-1)),
                                      Option(@"North America", @0),
                                      Option(@"Japan", @1),
                                      Option(@"Europe", @2),
                                      Option(@"Other", @3),
                                      ),
                              Popover(@"Run games using:", @selector(changeGameMode:),
                                      Option(@"XPC", NSStringFromClass([OEXPCGameCoreManager class])),
                                      Option(@"Background Thread", NSStringFromClass([OEThreadGameCoreManager class])),
                                      ),
                              Popover(@"Appearance:", @selector(changeAppAppearance:),
                                      Option(@"System", @(OEAppearancePreferenceValueSystem)),
                                      Option(@"Dark (default)", @(OEAppearancePreferenceValueDark)),
                                      Option(@"Light", @(OEAppearancePreferenceValueLight))
                                      ),

                              Group(@"Library Window"),
                              Button(@"Reset main window size", @selector(resetMainWindow:)),
                              Checkbox(OECoverGridViewAutoDownloadEnabledKey, @"Download missing artwork on the fly"),
                              Checkbox(OEDisplayGameTitle, @"Show game titles instead of rom names"),
                              Checkbox(OEImportManualSystems, @"Manually choose system on import"),
                              Checkbox(OEDBSavedGamesMediaShowsAutoSaves, @"Show autosave states in save state category"),
                              Checkbox(OEDBSavedGamesMediaShowsQuickSaves, @"Show quicksave states in save state category"),
                              Checkbox(OEGameScannerViewController.OESidebarHideBottomBarKey, @"Hide bottom bar in sidebar"),
                              Checkbox(@"useNewScreenshotsViewController", @"Use new screenshots view"),
                              Button(@"Show game scanner view", @selector(showGameScannerView:)),
                              Button(@"Hide game scanner view", @selector(hideGameScannerView:)),
                              
                              Group(@"Shaders"),
                              Button(@"Clear shader cache", @selector(clearShaderCache:)),
                              Button(@"Reveal user shader folder", @selector(openUserShaderFolder:)),

                              Group(@"HUD Bar / Gameplay"),
                              Checkbox(OEGameControlsBarCanDeleteSaveStatesKey, @"Can delete save states"),
                              NCheckbox(OEGameControlsBarHidesOptionButtonKey, @"Show options button"),
                              Checkbox(OEGameLayerNotificationView.OEShowNotificationsKey, @"Show notifications during gameplay"),
                              Checkbox(OESaveStateUseQuickSaveSlotsKey, @"Use quicksave slots"),
                              Checkbox(OEGameControlsBarShowsQuickSaveStateKey, @"Show quicksave in menu"),
                              Checkbox(OEGameControlsBarShowsAutoSaveStateKey, @"Show autosave in menu"),
                              Checkbox(OEGameControlsBarShowsAudioOutput, @"Show audio output device in menu"),
                              Checkbox(OETakeNativeScreenshots, @"Take screenshots in native size"),
                              Checkbox(OEScreenshotAspectRatioCorrectionDisabled, @"Disable aspect ratio correction in screenshots"),
                              Popover(@"Appearance:", @selector(changeHUDBarAppearance:),
                                      Option(@"Vibrant", @(OEHUDBarAppearancePreferenceValueVibrant)),
                                      Option(@"Dark", @(OEHUDBarAppearancePreferenceValueDark))
                                      ),
                              ColorWell(OEGameViewBackgroundColorKey, @"Game View Background color:"),

                              Group(@"Controls Setup"),
                              Checkbox(OEWiimoteSupportEnabled, @"WiiRemote support (requires relaunch)"),
                              NCheckbox(OEControlsDisableMouseSelection, @"Clicking on image selects button"),
                              NCheckbox(OEControlsDisableMouseDeactivation, @"Clicking outside image deselects button"),
                              Checkbox(OEControlsButtonHighlightRollsOver, @"Select first field after setting the last"),
                              Checkbox(OEDebugDrawControllerMaskKey, @"Draw button mask above image"),
                              Checkbox(@"logsHIDEvents", @"Log HID Events"),
                              Checkbox(@"logsHIDEventsNoKeyboard", @"Log Keyboard Events"),
                              Checkbox(@"OEShowAllGlobalKeys", @"Show all global keys"),
                              Popover(@"Appearance:", @selector(changeControlsPrefsAppearance:),
                                      Option(@"Wood", @(OEControlsPrefsAppearancePreferenceValueWood)),
                                      Option(@"Vibrant", @(OEControlsPrefsAppearancePreferenceValueLumberjack)),
                                      Option(@"Vibrant Wood", @(OEControlsPrefsAppearancePreferenceValueWoodVibrant))
                                      ),
                              ColorWell(OEGameViewBackgroundColorKey, @"Game View Background color:"),
                              NumberTextBox(@"OESystemResponderADCThreshold", @"Threshold for analog controls bound to buttons:", adcSensitivityNF),

                              Group2(@"Save States"),
                              Button(@"Set default save states directory", @selector(restoreSaveStatesDirectory:)),
                              Button(@"Choose save states directory…", @selector(chooseSaveStatesDirectory:)),
                              Button(@"Cleanup autosave state", @selector(cleanupAutoSaveStates:)),
                              Button(@"Cleanup Save States", @selector(cleanupSaveStates:)),

                              Group(@"OpenVGDB"),
                              Button(@"Update OpenVGDB", @selector(updateOpenVGDB:)),
                              Button(@"Cancel OpenVGDB Update", @selector(cancelOpenVGDBUpdate:)),

                              Group(@"Database Actions"),
                              Button(@"Delete useless image objects", @selector(removeUselessImages:)),
                              Button(@"Delete artwork that can be downloaded", @selector(removeArtworkWithRemoteBacking:)),
                              Button(@"Sync games without artwork", @selector(syncGamesWithoutArtwork:)),
                              Button(@"Download missing artwork…", @selector(downloadMissingArtwork:)),
                              Button(@"Remove untracked artwork files", @selector(removeUntrackedImageFiles:)),
                              Button(@"Cleanup rom hashes", @selector(cleanupHashes:)),
                              Button(@"Remove duplicated roms", @selector(removeDuplicatedRoms:)),
                              Button(@"Cancel cover sync for all games", @selector(cancelCoverArtSync:)),
                              Label(@""),
                              Button(@"Perform Sanity Check on Database", @selector(sanityCheck:)),
                              Label(@""),
                              ];
}

#pragma mark - Retrieving The Main Window
- (NSWindow *)mainWindow
{
    for (NSWindow *window in NSApp.windows)
    {
        if([window.windowController isKindOfClass:[OEMainWindowController class]])
        {
            return window;
        }
    }
    
    return nil;
}

#pragma mark - Actions
- (void)changeRegion:(NSPopUpButton*)sender
{
    NSMenuItem *item = [sender selectedItem];
    NSInteger value = [[item representedObject] integerValue];

    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    if(value == -1)
    {
        [standardUserDefaults removeObjectForKey:OERegionKey];
    }
    else
    {
        [standardUserDefaults setObject:@(value) forKey:OERegionKey];
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemAvailabilityDidChangeNotification object:self];
}

- (void)changeGameMode:(NSPopUpButton*)sender
{
    NSMenuItem *selectedItem = [sender selectedItem];
    [[NSUserDefaults standardUserDefaults] setObject:[selectedItem representedObject] forKey:OEGameCoreManagerModePreferenceKey];
}

- (void)changeAppAppearance:(NSPopUpButton *)sender
{
    NSMenuItem *selectedItem = [sender selectedItem];
    [[NSUserDefaults standardUserDefaults] setObject:[selectedItem representedObject] forKey:OEAppearancePreferenceKey];
}

- (void)changeHUDBarAppearance:(NSPopUpButton *)sender
{
    NSMenuItem *selectedItem = [sender selectedItem];
    [[NSUserDefaults standardUserDefaults] setObject:[selectedItem representedObject] forKey:OEHUDBarAppearancePreferenceKey];
}

- (void)changeControlsPrefsAppearance:(NSPopUpButton *)sender
{
    NSMenuItem *selectedItem = [sender selectedItem];
    [[NSUserDefaults standardUserDefaults] setObject:[selectedItem representedObject] forKey:OEControlsPrefsAppearancePreferenceKey];
    OEAlert *alert = [OEAlert new];
    alert.messageText = NSLocalizedString(@"You need to restart the application to commit the change", @"");
    [alert addButtonWithTitle:NSLocalizedString(@"OK", @"")];
    [alert runModal];
}

#pragma mark -
- (void)resetMainWindow:(id)sender
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    [defaults removeObjectForKey:@"NSSplitView Subview Frames mainSplitView"];
    [defaults removeObjectForKey:@"NSWindow Frame LibraryWindow"];
    [defaults removeObjectForKey:@"lastSidebarWidth"];
    [defaults removeObjectForKey:OELastGridSizeKey];

    NSWindow *mainWindow = self.mainWindow;
    
    // Matches the content size specified in MainWindow.xib.
    [mainWindow setFrame:NSMakeRect(0, 0, 845, 555 + 22) display:NO];
    
    [mainWindow center];

    [[NSNotificationCenter defaultCenter] postNotificationName:OELibrarySplitViewResetSidebarNotification object:self];
}

- (void)showGameScannerView:(id)sender {
    
    OEMainWindowController *mainWindowController = self.mainWindow.windowController;
    NSViewController *currentViewController = mainWindowController.libraryController.currentSubviewController;
    
    if([currentViewController isKindOfClass:[OELibraryGamesViewController class]])
    {
        [((OELibraryGamesViewController *)currentViewController).gameScannerController showGameScannerViewAnimated:YES];
    }
}

- (void)hideGameScannerView:(id)sender {
    
    OEMainWindowController *mainWindowController = self.mainWindow.windowController;
    NSViewController *currentViewController = mainWindowController.libraryController.currentSubviewController;
    
    if([currentViewController isKindOfClass:[OELibraryGamesViewController class]])
    {
        [((OELibraryGamesViewController *)currentViewController).gameScannerController hideGameScannerViewAnimated:YES];
    }
}

#pragma mark -
- (void)restoreSaveStatesDirectory:(id)sender
{
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:OESaveStateFolderURLKey];
}

- (void)chooseSaveStatesDirectory:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanCreateDirectories:YES];

    if([openPanel runModal] == NSAlertFirstButtonReturn)
        [[NSUserDefaults standardUserDefaults] setObject:[[openPanel URL] absoluteString] forKey:OESaveStateFolderURLKey];
}

- (void)cleanupAutoSaveStates:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    
    NSArray *allRoms = [OEDBRom allObjectsInContext:context];
    
    for (OEDBRom *rom in allRoms) {
        
        NSSortDescriptor *timeStampSort = [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:NO];
        NSArray *roms = [[rom saveStates] sortedArrayUsingDescriptors:@[timeStampSort]];
        NSPredicate *autosaveFilter = [NSPredicate predicateWithFormat:@"name BEGINSWITH %@", OESaveStateAutosaveName];
        NSArray *autosaves = [roms filteredArrayUsingPredicate:autosaveFilter];
        OEDBSaveState *autosave = nil;
        
        for(int i=0; i < [autosaves count]; i++)
        {
            OEDBSaveState *state = [autosaves objectAtIndex:i];
            if(/* DISABLES CODE */ (YES)) // TODO: fix -checkFilesAvailable
            {
                if(autosave)
                {
                    [state setName:NSLocalizedString(@"Recovered Auto Save", @"Recovered auto save name")];
                    [state moveToDefaultLocation];
                }
                else autosave = state;
            }
            else
            {
                [state delete];
            }
        }
        
        [autosave moveToDefaultLocation];
    }
    [context save:nil];
}

- (void)cleanupSaveStates:(id)sender
{
    OELibraryDatabase     *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSArray          *allSaveStates = [OEDBSaveState allObjectsInContext:context];

    // remove invalid save states
    [allSaveStates makeObjectsPerformSelector:@selector(deleteAndRemoveFilesIfInvalid)];
    [context save:nil];

    // add untracked save states
    NSURL *statesFolder = [database stateFolderURL];
    NSFileManager *fm   = [NSFileManager defaultManager];
    NSDirectoryEnumerator *enumerator = [fm enumeratorAtURL:statesFolder includingPropertiesForKeys:nil options:0 errorHandler:nil];
    for (NSURL *url in enumerator)
    {
        if([[url pathExtension] isEqualToString:OESaveStateSuffix])
            [OEDBSaveState createSaveStateByImportingBundleURL:url intoContext:context];
    }

    // remove invalid save states, again
    allSaveStates = [OEDBSaveState allObjectsInContext:context];
    [allSaveStates makeObjectsPerformSelector:@selector(deleteAndRemoveFilesIfInvalid)];
    [context save:nil];

    // remove duplicates
    allSaveStates = [OEDBSaveState allObjectsInContext:context];
    allSaveStates = [allSaveStates sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"rom.md5" ascending:YES], [NSSortDescriptor sortDescriptorWithKey:@"coreIdentifier" ascending:YES], [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:YES]]];
    OEDBSaveState *lastState = nil;
    for(OEDBSaveState *saveState in allSaveStates)
    {
        if(lastState && [lastState rom] == [saveState rom]
           && [[lastState timestamp] isEqualTo:[saveState timestamp]]
           && [[lastState coreIdentifier] isEqualToString:[saveState coreIdentifier]])
        {
            NSString *currentHash = nil, *previousHash;
            [[NSFileManager defaultManager] hashFileAtURL:[saveState dataFileURL] headerSize:0 md5:&currentHash error:nil];
            [[NSFileManager defaultManager] hashFileAtURL:[lastState dataFileURL] headerSize:0 md5:&previousHash error:nil];

            if([currentHash isEqualToString:previousHash])
            {
                if([[lastState URL] isEqualTo:[saveState URL]])
                {
                    [lastState delete];
                }
                else
                {
                    [lastState deleteAndRemoveFiles];
                }
            }
        }
        lastState = saveState;
    }
    [context save:nil];

    // move to default location
    allSaveStates = [OEDBSaveState allObjectsInContext:context];
    for(OEDBSaveState *saveState in allSaveStates)
    {
        if(![saveState moveToDefaultLocation])
        {
            NSLog(@"SaveState is still corrupt!");
            DLog(@"%@", [saveState URL]);
        }
    }
}
#pragma mark - OpenVGDB Actions
- (void)updateOpenVGDB:(id)sender
{
    DLog(@"Removing OpenVGDB update check date and version from user defaults to force update.");
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults removeObjectForKey:OEOpenVGDBUpdateCheckKey];
    [standardDefaults removeObjectForKey:OEOpenVGDBVersionKey];

    OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
    [helper checkForUpdatesWithHandler:^(NSURL * _Nullable url, NSString * _Nullable version) {
        if (url && version) {
            [helper installVersion:version withDownloadURL:url];
        }
    }];
}

- (void)cancelOpenVGDBUpdate:(id)sender
{
    OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
    [helper cancelUpdate];
}

#pragma mark - Database actions
- (void)removeUselessImages:(id)sender
{
    // removes all image objects that are neither on disc nor have a source
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"relativePath == nil and source == nil"];
    [request setPredicate:predicate];

    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    [result makeObjectsPerformSelector:@selector(delete)];
    [context save:nil];
    NSLog(@"Deleted %ld images!", result.count);
}

- (void)removeArtworkWithRemoteBacking:(id)sender
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"source == nil"];

    [request setPredicate:predicate];

    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    NSUInteger count = 0;
    for(OEDBImage *image in result)
    {
        // make sure we only delete image files that can be downloaded automatically!
        if([image sourceURL])
        {
            NSURL *fileURL = [image imageURL];
            [[NSFileManager defaultManager] removeItemAtURL:fileURL error:nil];
            [image setRelativePath:nil];
            count++;
        }
    }
    [context save:nil];
    NSLog(@"Deleted %ld image files!", count);
}

- (void)syncGamesWithoutArtwork:(id)sender
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"boxImage == nil"];
    [request setPredicate:predicate];
    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    NSLog(@"Found %ld games", [result count]);
    for(OEDBGame *game in result){
        [game requestInfoSync];
    }
}


- (void)downloadMissingArtwork:(id)sender
{
    OEAlert *alert = [[OEAlert alloc] init];
    alert.messageText = NSLocalizedStringFromTable(@"While performing this operation OpenEmu will be unresponsive.", @"Debug", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Continue", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    if([alert runModal] != NSAlertFirstButtonReturn) return;

    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"source != nil"];
    [request setPredicate:predicate];

    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    NSUInteger count = 0;
    for(OEDBImage *image in result)
    {
        // make sure we only delete image files that can be downloaded automatically!
        if(![image localFilesAvailable])
        {
            @autoreleasepool {
                NSDictionary *newInfo = [OEDBImage prepareImageWithURLString:[image source]];
                if(newInfo && [newInfo valueForKey:@"relativePath"])
                {
                    [image setWidth:[[newInfo valueForKey:@"width"] floatValue]];
                    [image setHeight:[[newInfo valueForKey:@"height"] floatValue]];
                    [image setRelativePath:[newInfo valueForKey:@"relativePath"]];
                    [image setFormat:[[newInfo valueForKey:@"format"] shortValue]];
                    count ++;
                }
            }
        }
        if(count % 20 == 0)
            [context save:nil];
    }
    [context save:nil];
    NSLog(@"Downloaded %ld image files!", count);
}

- (void)removeUntrackedImageFiles:(id)sender
{
    OELibraryDatabase *library   = [OELibraryDatabase defaultDatabase];
    NSURL *artworkDirectory      = [library coverFolderURL];

    NSArray *artworkFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:artworkDirectory includingPropertiesForKeys:nil options:0 error:nil];
    NSMutableSet *artwork = [NSMutableSet setWithArray:artworkFiles];

    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"relativePath != nil"];
    [request setPredicate:predicate];

    NSArray *images = [context executeFetchRequest:request error:nil];
    if(images == nil)
    {
        return;
    }

    for(OEDBImage *image in images)
    {
        [artwork removeObject:[image imageURL]];
    }

    for (NSURL *untrackedFile in artwork) {
        [[NSFileManager defaultManager] removeItemAtURL:untrackedFile error:nil];
    }
    NSLog(@"Removed %ld unknown files from artwork directory", [artwork count]);
}

- (void)cleanupHashes:(id)sender
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];

    for (OEDBRom *rom in [OEDBRom allObjectsInContext:context]) {
        rom.md5 = rom.md5.lowercaseString;
    }

    [context save:nil];
}

- (void)removeDuplicatedRoms:(id)sender
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];

    NSArray *objects = [OEDBRom allObjectsInContext:context];
    objects = [objects sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"md5" ascending:YES]]];
    OEDBRom *lastRom = nil;
    NSMutableArray *romsToDelete = [NSMutableArray array];
    for(OEDBRom *rom in objects)
    {
        if(lastRom && [[rom md5] isEqualToString:[lastRom md5]])
        {
            [rom setSaveStates:[[lastRom saveStates] setByAddingObjectsFromSet:[rom saveStates]]];
            [romsToDelete addObject:lastRom];
        }
        lastRom = rom;
    }

    for (OEDBRom *rom in romsToDelete) {
        [rom.game deleteByMovingFile:NO keepSaveStates:YES];
        [rom deleteByMovingFile:NO keepSaveStates:NO];
    }

    NSLog(@"%ld roms deleted", [romsToDelete count]);
    [context save:nil];
}

- (IBAction)cancelCoverArtSync:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
    [request setPredicate:[NSPredicate predicateWithFormat:@"status == %d", OEDBGameStatusProcessing]];
    NSArray *games = [context executeFetchRequest:request error:nil];
    [games makeObjectsPerformSelector:@selector(setStatus:) withObject:@(OEDBGameStatusOK)];
    [context save:nil];

    NSLog(@"Cancelled cover art download for %ld games", [games count]);
}

- (IBAction)sanityCheck:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    __block NSUInteger counts[3] = {0};

    NSLog(@"= START SANITY CHECK =");

    NSArray *allRoms = [OEDBRom allObjectsInContext:context];

    // Look for roms without games
    counts[0] = 0;
    for(OEDBRom *rom in allRoms)
    {
        if([rom game] == nil)
            counts[0] ++;
    }
    if(counts[0]) NSLog(@"Found %ld roms without game!", counts[0]);

    // Look for roms referencing the same file
    allRoms = [allRoms sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"location" ascending:YES]]];
    counts[0] = 0;
    OEDBRom *lastRom = nil;
    for(OEDBRom *rom in allRoms)
        if([[rom location] isEqualToString:[lastRom location]])
            counts[0] ++;
    if(counts[0]) NSLog(@"Found %ld duplicated roms!", counts[0]);

    // Look for roms with same hash
    allRoms = [allRoms sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"md5" ascending:YES]]];
    counts[0] = 0;
    counts[1] = 0;
    lastRom = nil;
    for(OEDBRom *rom in allRoms)
    {
        if([[rom md5] isEqualToString:[lastRom md5]] && [rom md5] != nil && [[rom md5] length] != 0)
            counts[0] ++;
        if([[[rom md5] lowercaseString] isNotEqualTo:[rom md5]])
            counts[1] ++;
    }
    if(counts[0]) NSLog(@"Found %ld duplicated roms!", counts[0]);
    if(counts[1]) NSLog(@"Found %ld roms with wrong case in hash!", counts[1]);

    // Look for games without roms
    NSArray *allGames = [OEDBGame allObjectsInContext:context];
    counts[0] = 0;
    for(OEDBGame *game in allGames)
        if([[game roms] count] == 0)
            counts[0] ++;

    if(counts[0]) NSLog(@"Found %ld games without rom!", counts[0]);


    // Look for save states without rom
    NSArray *allStates = [OEDBSaveState allObjectsInContext:context];
    counts[0] = 0;
    counts[1] = 0;
    for(OEDBSaveState *state in allStates)
    {
        if([state rom] == nil)
            counts[0] ++;
        if(![state isValid])
            counts[1]++;
    }
    if(counts[0]) NSLog(@"Found %ld save states without rom!", counts[0]);
    if(counts[1]) NSLog(@"Found %ld invalid save states!", counts[1]);


    // Look for images without game
    NSArray *allImages = [OEDBImage allObjectsInContext:context];
    counts[0] = 0;
    for(OEDBImage *image in allImages)
    {
        if([image Box] == nil)
            counts[0] ++;
    }
    if(counts[0]) NSLog(@"Found %ld images without game!", counts[0]);
    
    // Look for images without source
    allImages = [OEDBImage allObjectsInContext:context];
    counts[0] = 0;
    counts[1] = 0;
    for(OEDBImage *image in allImages)
    {
        if(image.source == nil || [image.source isEqualToString:@""])
            counts[0] ++;
        if(image.relativePath == nil || [image.relativePath isEqualToString:@""])
            counts[1] ++;
        if(image.image == nil)
            counts[2] ++;
    }
    if(counts[0]) NSLog(@"Found %ld images without source!", counts[0]);
    if(counts[1]) NSLog(@"Found %ld images without local path!", counts[1]);

    NSLog(@"= Done =");
}
#pragma mark -
- (void)changeUDColor:(id)sender
{
    NSInteger index = [sender tag];
    if(index >= 0)
    {
        NSDictionary *colorObject = [[self keyDescriptions] objectAtIndex:index];
        if([colorObject objectForKey:TypeKey] != ColorType)
        {
            // Wrong object, should not happen…
            return;
        }

        NSString *key  = [colorObject objectForKey:KeyKey];
        NSString *value = [[(NSColorWell *)sender color] toString];

        [[NSUserDefaults standardUserDefaults] setObject:value forKey:key];
    }
}

#pragma mark - Shader group actions

- (void)clearShaderCache:(id)sender
{
    [NSFileManager.defaultManager removeItemAtURL:OEShadersModel.shadersCachePath error:nil];
}

- (void)openUserShaderFolder:(id)sender
{
    [NSWorkspace.sharedWorkspace openURL:OEShadersModel.userShadersPath];
}


#pragma mark - Grid View Setup


- (void)setupGridView
{
    NSGridView *gridView = [NSGridView gridViewWithNumberOfColumns:2 rows:0];
    [[gridView columnAtIndex:0] setXPlacement:NSGridCellPlacementTrailing];
    [gridView setRowAlignment:NSGridRowAlignmentFirstBaseline];
    NSInteger i = 0;
    for (NSDictionary *keyDesc in self.keyDescriptions) {
        [self createRowForGridView:gridView index:i fromDescription:keyDesc];
        i++;
    }
    
    [gridView setContentHuggingPriority:NSLayoutPriorityDefaultLow-1 forOrientation:NSLayoutConstraintOrientationHorizontal];
    [gridView setContentHuggingPriority:NSLayoutPriorityDefaultHigh-1 forOrientation:NSLayoutConstraintOrientationVertical];
    gridView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.contentView addSubview:gridView];
    
    CGFloat scrollerWidth = [NSScroller scrollerWidthForControlSize:NSControlSizeRegular scrollerStyle:NSScrollerStyleOverlay];
    NSSize fittingSize = gridView.fittingSize;
    fittingSize.width += 60 - scrollerWidth;
    fittingSize.height += 40;
    [self.contentView setFrameSize:fittingSize];
    [self.contentView.enclosingScrollView.contentView scrollToPoint:NSMakePoint(0, fittingSize.height)];
    
    fittingSize.width += scrollerWidth;
    viewSize = NSMakeSize(fittingSize.width, 500);
    
    [self.contentView addConstraints:@[
        [NSLayoutConstraint constraintWithItem:gridView attribute:NSLayoutAttributeLeft
            relatedBy:NSLayoutRelationEqual
            toItem:self.contentView attribute:NSLayoutAttributeLeft
            multiplier:1.0 constant:30],
        [NSLayoutConstraint constraintWithItem:gridView attribute:NSLayoutAttributeTop
            relatedBy:NSLayoutRelationEqual
            toItem:self.contentView attribute:NSLayoutAttributeTop
            multiplier:1.0 constant:20]]];
}


- (void)createRowForGridView:(NSGridView *)gridView index:(NSInteger)i fromDescription:(NSDictionary *)keyDescription
{
	NSString *type   = [keyDescription objectForKey:@"type"];

    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary<NSString *, id> *defaultDefaults = [userDefaults volatileDomainForName:NSRegistrationDomain];
    
    if(type == CheckboxType) {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSString *udkey = [keyDescription objectForKey:KeyKey];
        BOOL negated    = [[keyDescription objectForKey:NegatedKey] boolValue];

        NSButton *checkbox = [NSButton checkboxWithTitle:label target:nil action:nil];
        [checkbox setTitle:label];

        NSDictionary *options = @{ NSContinuouslyUpdatesValueBindingOption:@YES };
        if(negated)
        {
            options = @{ NSValueTransformerNameBindingOption : NSNegateBooleanTransformerName, NSContinuouslyUpdatesValueBindingOption:@YES };
        }

        NSString *keypath = [NSString stringWithFormat:@"values.%@", udkey];
        [checkbox bind:@"value" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:keypath options:options];
        
        id originalValue = [defaultDefaults objectForKey:udkey] ?: @NO;
        if ([originalValue isKindOfClass:[NSNumber class]])  {
            BOOL origbool = [originalValue boolValue] ^ negated;
            NSString *fmt = NSLocalizedStringFromTable(@"Default Value: %@", @"Debug", @"Default value tooltip format in the Debug Preferences");
            NSString *val = origbool ?
                NSLocalizedStringFromTable(@"Checked", @"Debug", @"Default value tooltip for checkboxes: checked default") :
                NSLocalizedStringFromTable(@"Unchecked", @"Debug", @"Default value tooltip for checkboxes: unchecked default");
            NSString *defaultTooltip = [NSString stringWithFormat:fmt, val];
            [checkbox setToolTip:defaultTooltip];
        }
        
        [gridView addRowWithViews:@[[NSGridCell emptyContentView], checkbox]];
        
    } else if(type == GroupType) {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSTextField *field = [NSTextField labelWithString:label];
        [field setStringValue:label];
        [field setFont:[NSFont boldSystemFontOfSize:0]];
        
        NSGridRow *row = [gridView addRowWithViews:@[[NSGridCell emptyContentView], field]];
        row.bottomPadding = 4;
        
    } else if(type == ButtonType) {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSString *action = [keyDescription objectForKey:ActionKey];

        NSButton *button = [NSButton buttonWithTitle:label target:self action:NSSelectorFromString(action)];
        [gridView addRowWithViews:@[[NSGridCell emptyContentView], button]];
        
    } else if(type == ColorType) {
        NSString *label  = [keyDescription objectForKey:LabelKey];
        NSString *key    = [keyDescription objectForKey:KeyKey];

        NSTextField *labelField = [NSTextField labelWithString:label];
        [labelField setAlignment:NSTextAlignmentRight];

        NSColorWell *colorWell = [[NSColorWell alloc] init];
        NSColor     *color     = [NSColor blackColor];
        if([userDefaults stringForKey:key])
        {
            color = [[NSColor alloc] colorFromHexString:[userDefaults stringForKey:key]];
        }
        [colorWell setColor:color];
        [colorWell setAction:@selector(changeUDColor:)];
        [colorWell setTarget:self];
        colorWell.tag = i;
        colorWell.autoresizingMask = NSViewWidthSizable + NSViewHeightSizable;
        
        NSGridRow *row = [gridView addRowWithViews:@[labelField, colorWell]];
        row.rowAlignment = NSGridRowAlignmentNone;
        row.yPlacement = NSGridCellPlacementCenter;
        [gridView addConstraints:@[
            [NSLayoutConstraint constraintWithItem:colorWell attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:60],
            [NSLayoutConstraint constraintWithItem:colorWell attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:24],
        ]];
        
    } else if(type == LabelType) {
        NSString *label  = [keyDescription objectForKey:LabelKey];
        NSTextField *labelField = [NSTextField labelWithString:label];
        [gridView addRowWithViews:@[[NSGridCell emptyContentView], labelField]];
        
    } else if(type == PopoverType) {
        NSString *label  = [keyDescription objectForKey:LabelKey];
        NSArray *options = [keyDescription objectForKey:OptionsKey];
        NSString *action = [keyDescription objectForKey:ActionKey];
        NSTextField *labelField = [NSTextField labelWithString:label];
        [labelField setAlignment:NSTextAlignmentRight];

        NSPopUpButton *popup = [[NSPopUpButton alloc] init];
        [popup setAction:NSSelectorFromString(action)];
        [popup setTarget:self];

        [options enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            [popup addItemWithTitle:[obj objectForKey:LabelKey]];
            [[popup itemAtIndex:idx] setRepresentedObject:[obj objectForKey:ValueKey]];
        }];

        [self OE_setupSelectedItemForPopupButton:popup withKeyDescription:keyDescription];
        [popup sizeToFit];
        [gridView addRowWithViews:@[labelField, popup]];
        
    } else if (type == NumericTextFieldType) {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSNumberFormatter *nf = [keyDescription objectForKey:NumberFormatterKey];
        NSString *udkey = [keyDescription objectForKey:KeyKey];
        
        NSTextField *labelField = [NSTextField labelWithString:label];
        [labelField setAlignment:NSTextAlignmentRight];
        
        NSTextField *inputField = [NSTextField textFieldWithString:@""];
        [inputField setFormatter:nf];
        NSString *keypath = [NSString stringWithFormat:@"values.%@", udkey];
        [inputField bind:NSValueBinding toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:keypath options:nil];
        
        NSString *validRangeFormat = NSLocalizedStringFromTable(@"Range: %@ to %@", @"Debug", @"Range indicator tooltip for numeric text boxes in the Debug Preferences");
        NSString *min = [nf stringFromNumber:nf.minimum];
        NSString *max = [nf stringFromNumber:nf.maximum];
        NSMutableString *tooltip = [NSMutableString stringWithFormat:validRangeFormat, min, max];
        
        id defaultv = [defaultDefaults objectForKey:udkey] ?: @0;
        if ([defaultv isKindOfClass:[NSNumber class]]) {
            NSString *fmt = NSLocalizedStringFromTable(@"Default Value: %@", @"Debug", @"Default value tooltip format in the Debug Preferences");
            NSString *defaultstr = [nf stringFromNumber:defaultv];
            [tooltip appendString:@"\n"];
            [tooltip appendFormat:fmt, defaultstr];
        }
        [inputField setToolTip:tooltip];
        
        [gridView addRowWithViews:@[labelField, inputField]];
        [gridView addConstraints:@[
            [NSLayoutConstraint constraintWithItem:inputField attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:70]]];
        
    } else if (type == SeparatorType) {
        NSBox *sep = [[NSBox alloc] init];
        sep.boxType = NSBoxSeparator;
        
        NSGridRow *row = [gridView addRowWithViews:@[sep]];
        [row mergeCellsInRange:NSMakeRange(0, 2)];
        row.topPadding = 6;
        row.bottomPadding = 6;
    }
}

- (void)OE_setupSelectedItemForPopupButton:(NSPopUpButton*)button withKeyDescription:(NSDictionary*)keyDescription
{
    NSString      *action    = [keyDescription valueForKey:@"action"];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    NSString   *userDefaultsKey = nil;
    BOOL (^test)(id, id) = NULL;

    if([action isEqualToString:@"changeGameMode:"])
    {
        userDefaultsKey = OEGameCoreManagerModePreferenceKey;
        test = ^BOOL(id obj, id currentValue) {
            return [[obj representedObject] isEqualToString:currentValue];
        };
    }
    else if([action isEqualToString:@"changeRegion:"])
    {
        userDefaultsKey = OERegionKey;
        test = ^BOOL(id obj, id currentValue) {
            return [[obj representedObject] intValue] == [currentValue intValue];
        };
    }
    else if ([action isEqual:NSStringFromSelector(@selector(changeAppAppearance:))])
    {
        userDefaultsKey = OEAppearancePreferenceKey;
        test = ^BOOL(id obj, id currentValue) {
            return [[obj representedObject] intValue] == [currentValue intValue];
        };
    }
    else if ([action isEqual:NSStringFromSelector(@selector(changeHUDBarAppearance:))])
    {
        userDefaultsKey = OEHUDBarAppearancePreferenceKey;
        test = ^BOOL(id obj, id currentValue) {
            return [[obj representedObject] intValue] == [currentValue intValue];
        };
    }
    else if ([action isEqual:NSStringFromSelector(@selector(changeControlsPrefsAppearance:))])
    {
        userDefaultsKey = OEControlsPrefsAppearancePreferenceKey;
        test = ^BOOL(id obj, id currentValue) {
            return [[obj representedObject] intValue] == [currentValue intValue];
        };
    }

    id currentValue;
    __block NSInteger index = 0;
    if(userDefaultsKey && (currentValue = [defaults objectForKey:userDefaultsKey]))
    {
        NSArray *itemArray = [button itemArray];
        [itemArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            if(test(obj, currentValue))
            {
                index = idx;
                *stop = YES;
            }
        }];
    }
    [button selectItemAtIndex:index];
}


#pragma mark - OEPreferencePane Protocol
- (NSImage *)icon
{
    return [NSImage imageNamed:@"debug_tab_icon"];
}

- (NSString *)panelTitle
{
    return @"Secrets";
}

- (NSString *)localizedPanelTitle
{
    return NSLocalizedString([self panelTitle], @"Preferences: Debug Toolbar Item");
}

- (NSString *)nibName
{
    return @"OEPrefDebugController";
}

@end
