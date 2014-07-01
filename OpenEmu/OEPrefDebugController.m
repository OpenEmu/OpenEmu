/*
 Copyright (c) 2012, OpenEmu Team
 
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
#import "OEDBSaveState.h"

#import "NSURL+OELibraryAdditions.h"
#import "NSColor+OEAdditions.h"

#import "OEGameViewController.h"

#import "OEDOGameCoreManager.h"
#import "OEThreadGameCoreManager.h"
#import "OEXPCGameCoreManager.h"

#import "OEGameDocument.h"

#import "OEGameInfoHelper.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "OEHUDAlert.h"
#pragma mark Key sources
#import "OEPreferencesController.h"
#import "OESetupAssistant.h"
#import "OECollectionViewController.h"
#import "OEGameViewController.h"
#import "OECollectionDebugWindowController.h"
#import "OERetrodeDeviceManager.h"
#import "OEGridGameCell.h"
#import "OEGameView.h"
#import "OEControllerImageView.h"
#import "OEControlsButtonSetupView.h"
#import "OEDBDataSourceAdditions.h"
#import "OEImportOperation.h"

@interface OELibraryDatabase (Private)
- (void)OE_createInitialItems;
@end

@interface OEPrefDebugController () <NSTableViewDelegate, NSTableViewDataSource>
@property NSArray *keyDescriptions;
@end

NSString * const  CheckboxType  = @"Checkbox";
NSString * const  ButtonType    = @"Button";
NSString * const  SeparatorType = @"Separator";
NSString * const  GroupType     = @"Group";
NSString * const  ColorType     = @"Color";
NSString * const  LabelType     = @"Label";
NSString * const  PopoverType   = @"Popover";

NSString * const TypeKey  = @"type";
NSString * const LabelKey = @"label";
NSString * const KeyKey   = @"key";
NSString * const ActionKey = @"action";
NSString * const NegatedKey = @"negated";
NSString * const ValueKey = @"value";
NSString * const OptionsKey = @"options";

#define Separator() \
@{ TypeKey:SeparatorType }
#define FirstGroup(_NAME_) \
@{ TypeKey:GroupType, LabelKey:_NAME_ }
#define Group(_NAME_) Separator(), \
@{ TypeKey:GroupType, LabelKey:_NAME_ }
#define Checkbox(_KEY_, _LABEL_)  \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedString(_LABEL_,@"DebugModeLabel"), TypeKey:CheckboxType }
#define NCheckbox(_KEY_, _LABEL_) \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedString(_LABEL_,@"DebugModeLabel"), TypeKey:CheckboxType, NegatedKey:@YES }
#define Button(_LABEL_, _ACTION_)  \
@{ LabelKey:NSLocalizedString(_LABEL_,@"DebugModeLabel"), TypeKey:ButtonType, ActionKey:NSStringFromSelector(_ACTION_) }
#define Label(_LABEL_)  \
@{ LabelKey:NSLocalizedString(_LABEL_,@"DebugModeLabel"), TypeKey:LabelType }
#define Popover(_LABEL_, _ACTION_, ...)  \
@{ LabelKey:NSLocalizedString(_LABEL_,@"DebugModeLabel"), TypeKey:PopoverType, ActionKey:NSStringFromSelector(_ACTION_), OptionsKey:@[__VA_ARGS__] }
#define Option(_OLABEL_, _OVAL_) \
@{ LabelKey:_OLABEL_, ValueKey:_OVAL_ }
#define ColorWell(_KEY_, _LABEL_) \
@{ KeyKey:_KEY_, LabelKey:NSLocalizedString(_LABEL_,@"DebugModeLabel"), TypeKey:ColorType }

@implementation OEPrefDebugController
- (void)awakeFromNib
{
    if([self keyDescriptions] != nil) return;
    [self OE_setupKeyDescription];

	NSTableView *tableView = [self tableView];
	[tableView setDelegate:self];
	[tableView setDataSource:self];

    [tableView setHeaderView:nil];
	[tableView setRowHeight:30.0];
	[tableView setGridStyleMask:0];
	[tableView setAllowsColumnReordering:NO];
	[tableView setAllowsColumnResizing:NO];
	[tableView setAllowsColumnSelection:NO];
	[tableView setAllowsEmptySelection:YES];
	[tableView setAllowsMultipleSelection:NO];
	[tableView setAllowsTypeSelect:NO];
}

- (void)OE_setupKeyDescription
{
    self.keyDescriptions =  @[
                              FirstGroup(@"General"),
                              Checkbox(OEDebugModeKey, @"Debug Mode"),
                              Checkbox(OESetupAssistantHasFinishedKey, @"Setup Assistant has finished"),
                              Popover(@"Region", @selector(changeRegion:),
                                      Option(@"Auto", @(-1)),
                                      Option(@"North America", @0),
                                      Option(@"Japan", @1),
                                      Option(@"Europe", @2),
                                      Option(@"Other", @3),
                                      ),
                              Popover(@"Run games using", @selector(changeGameMode:),
                                      Option(@"XPC", NSStringFromClass([OEXPCGameCoreManager class])),
                                      Option(@"Distributed Objects", NSStringFromClass([OEDOGameCoreManager class])),
                                      Option(@"Background Thread", NSStringFromClass([OEThreadGameCoreManager class])),
                                      ),

                              Group(@"Library Window"),
                              NCheckbox(OEMenuOptionsStyleKey, @"Dark GridView context menu"),
                              Checkbox(OEDebugCollectionView, @"Show collection view debug controller"),
                              Checkbox(OERetrodeSupportEnabledKey, @"Enable Retrode support"),
                              Checkbox(OECoverGridViewGlossDisabledKey, @"Disable grid view gloss overlay"),
                              Checkbox(OECoverGridViewAutoDownloadEnabledKey, @"Download missing artwork on the fly"),
                              Checkbox(OEDisplayGameTitle, @"Show game titles instead of rom names"),
                              Checkbox(OEImportManualSystems, @"Manually choose system on import"),

                              Group(@"HUD Bar / Gameplay"),
                              NCheckbox(OEDontShowGameTitleInWindowKey, @"Use game name as window title"),
                              Checkbox(OEGameControlsBarCanDeleteSaveStatesKey, @"Can delete save states"),
                              NCheckbox(OEGameControlsBarHidesOptionButtonKey, @"Show options button"),
                              Checkbox(OEForceCorePicker, @"Use gamecore picker"),
                              Checkbox(OESaveStateUseQuickSaveSlotsKey, @"Use quicksave slots"),
                              Checkbox(OEGameControlsBarShowsQuickSaveStateKey, @"Show quicksave in menu"),
                              Checkbox(OEGameControlsBarShowsAutoSaveStateKey, @"Show autosave in menu"),
                              Checkbox(OEGameControlsBarShowsAudioOutput, @"Show audio output device in menu"),
                              Checkbox(OETakeNativeScreenshots, @"Take screenshots in native size"),
                              Checkbox(OEScreenshotAspectRationCorrectionDisabled, @"Disable aspect ratio correction"),
                              ColorWell(OEGameViewBackgroundColorKey, @"Game View Background color"),

                              Group(@"Controls Setup"),
                              Checkbox(OEWiimoteSupportEnabled, @"WiiRemote support (requires relaunch)"),
                              NCheckbox(OEControlsDisableMouseSelection, @"Clicking on image selects button"),
                              NCheckbox(OEControlsDisableMouseDeactivation, @"Clicking outside image deselects button"),
                              Checkbox(OEControlsButtonHighlightRollsOver, @"Select first field after setting the last"),
                              Checkbox(OEDebugDrawControllerMaskKey, @"Draw button mask above image"),
                              Checkbox(@"logsHIDEvents", @"Log HID Events"),
                              Checkbox(@"logsHIDEventsNoKeyboard", @"Log Keyboard Events"),
                              Checkbox(@"OEShowAllGlobalKeys", @"Show all global keys"),

                              Group(@"Save States"),
                              Button(@"Set default save states directory", @selector(restoreSaveStatesDirectory:)),
                              Button(@"Choose save states directory", @selector(chooseSaveStatesDirectory:)),
                              Button(@"Add untracked save states", @selector(findUntrackedSaveStates:)),
                              Button(@"Remove missing states", @selector(removeMissingStates:)),
                              Button(@"Remove duplicate states", @selector(removeDuplicateStates:)),
                              Button(@"Cleanup autosave state", @selector(cleanupAutoSaveStates:)),

                              Group(@"OpenVGDB"),
                              Button(@"Update OpenVGDB", @selector(updateOpenVGDB:)),
                              Button(@"Cancel OpenVGDB Update", @selector(cancelOpenVGDBUpdate:)),

                              Group(@"Database Actions"),
                              Label(@"Not implemented right now"),
                              Label(@""),
                              ];
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

    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsDidChangeNotification object:self];
}

- (void)changeGameMode:(NSPopUpButton*)sender
{
    NSMenuItem *selectedItem = [sender selectedItem];
    [[NSUserDefaults standardUserDefaults] setObject:[selectedItem representedObject] forKey:OEGameCoreManagerModePreferenceKey];
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

    if([openPanel runModal] == NSAlertDefaultReturn)
        [[NSUserDefaults standardUserDefaults] setObject:[[openPanel URL] absoluteString] forKey:OESaveStateFolderURLKey];
}

- (void)findUntrackedSaveStates:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSURL *statesFolder = [database stateFolderURL];
    NSFileManager *fm   = [NSFileManager defaultManager];

    NSDirectoryEnumerator *enumerator = [fm enumeratorAtURL:statesFolder includingPropertiesForKeys:nil options:0 errorHandler:nil];
    for (NSURL *url in enumerator)
    {
        if([[url pathExtension] isEqualToString:@"oesavestate"])
            [OEDBSaveState updateOrCreateStateWithURL:url inContext:[database mainThreadContext]];
    }
}

- (void)removeMissingStates:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];

    NSArray *objects = [OEDBSaveState allObjectsInContext:[database mainThreadContext]];
    [objects makeObjectsPerformSelector:@selector(removeIfMissing)];
    [[database mainThreadContext] save:nil];
}

- (void)removeDuplicateStates:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];

    NSArray *objects = [OEDBSaveState allObjectsInContext:[database mainThreadContext]];
    objects = [objects sortedArrayUsingComparator:^NSComparisonResult(OEDBSaveState *obj1, OEDBSaveState *obj2) {
        return [[[[obj1 URL] standardizedURL] absoluteString] compare:[[[obj2 URL] standardizedURL] absoluteString]];
    }];

    for(int i=0; i < [objects count]; i++)
    {
        OEDBSaveState *currentState = [objects objectAtIndex:i];
        [currentState setURL:[currentState URL]];
    }

    OEDBSaveState *lastState = nil;
    for(int i=0; i < [objects count]; i++)
    {
        OEDBSaveState *currentState = [objects objectAtIndex:i];
        if([[[lastState URL] standardizedURL] isEqualTo:[[currentState URL] standardizedURL]]) {
            [currentState delete];
        } else {
            [currentState setURL:[currentState URL]];
            lastState = currentState;
        }
    }

    [[database mainThreadContext] save:nil];

}

- (void)cleanupAutoSaveStates:(id)sender
{
    [self removeDuplicateStates:self];

    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSArray *allRoms = [OEDBRom allObjectsInContext:context];
    [allRoms enumerateObjectsUsingBlock:^(OEDBRom *rom, NSUInteger idx, BOOL *stop) {
        NSSortDescriptor *timeStampSort = [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:NO];
        NSArray *roms = [[rom saveStates] sortedArrayUsingDescriptors:@[timeStampSort]];
        NSPredicate *autosaveFilter = [NSPredicate predicateWithFormat:@"name BEGINSWITH %@", OESaveStateAutosaveName];
        NSArray *autosaves = [roms filteredArrayUsingPredicate:autosaveFilter];
        OEDBSaveState *autosave = nil;
        for(int i=0; i < [autosaves count]; i++)
        {
            OEDBSaveState *state = [autosaves objectAtIndex:i];
            if([state checkFilesAvailable])
            {
                if(autosave)
                {
                    [state setName:NSLocalizedString(@"Recovered Auto Save", @"Recovered auto save name")];
                    [state moveToSaveStateFolder];
                    [state writeInfoPlist];
                }
                else autosave = state;
            }
            else
            {
                [state delete];
            }
        }

        [autosave moveToSaveStateFolder];
        [autosave writeInfoPlist];
    }];
    [context save:nil];
}
#pragma mark -
- (void)updateOpenVGDB:(id)sender
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults removeObjectForKey:OEOpenVGDBUpdateCheckKey];
    [standardDefaults removeObjectForKey:OEOpenVGDBVersionKey];

    OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
    NSString *version = nil;
    NSURL *url = [helper checkForUpdates:&version];
    [helper installVersion:version withDownloadURL:url];
}

- (void)cancelOpenVGDBUpdate:(id)sender
{
    OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
    [helper cancelUpdate];
}
#pragma mark -
- (void)changeUDColor:(id)sender
{
    NSRect    frame = [sender convertRect:[sender bounds] toView:[self tableView]];

    NSInteger index = [[self tableView] rowAtPoint:(NSPoint){NSMidX(frame), NSMidY(frame)}];
    if(index != -1)
    {
        NSDictionary *colorObject = [[self keyDescriptions] objectAtIndex:index];
        if([colorObject objectForKey:TypeKey] != ColorType)
        {
            // Wrong object, should not happen…
            return;
        }

        NSString *key  = [colorObject objectForKey:KeyKey];
        NSColor *color = [sender color];
        NSString *value = OENSStringFromColor(color);

        [[NSUserDefaults standardUserDefaults] setObject:value forKey:key];
    }
}
#pragma mark - NSTableView Delegate
- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	return YES;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	return NO;
}

- (NSView*)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSDictionary *keyDescription = [[self keyDescriptions] objectAtIndex:row];
	NSString *type   = [keyDescription objectForKey:@"type"];
    NSView *cellView = [tableView makeViewWithIdentifier:type owner:self];

    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    if(type == CheckboxType)
    {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSString *udkey = [keyDescription objectForKey:KeyKey];
        BOOL negated    = [[keyDescription objectForKey:NegatedKey] boolValue];

        NSButton *checkbox = [[cellView subviews] lastObject];
        [checkbox setTitle:label];

        NSDictionary *options = @{ NSContinuouslyUpdatesValueBindingOption:@YES };
        if(negated)
        {
            options = @{ NSValueTransformerNameBindingOption : NSNegateBooleanTransformerName, NSContinuouslyUpdatesValueBindingOption:@YES };
        }

        NSString *keypath = [NSString stringWithFormat:@"values.%@", udkey];
        [checkbox bind:@"value" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:keypath options:options];
    }
    else if(type == GroupType)
    {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSTextField *field = [[cellView subviews] lastObject];
        [field setStringValue:label];
    }
    else if(type == ButtonType)
    {
        NSString *label = [keyDescription objectForKey:LabelKey];
        NSString *action = [keyDescription objectForKey:ActionKey];

        NSButton *button = [[cellView subviews] lastObject];
        [button setTitle:label];
        [button setAction:NSSelectorFromString(action)];
        [button setTarget:self];

        [button sizeToFit];
        NSRect frame = [button frame];
        frame.size.height = 23.0;
        frame.size.width += 30;
        [button setFrame:frame];
    }
    else if(type == ColorType)
    {
        NSString *label  = [keyDescription objectForKey:LabelKey];
        NSString *key    = [keyDescription objectForKey:KeyKey];

        NSTextField *labelField = [[cellView subviews] objectAtIndex:0];
        [labelField setStringValue:label];

        NSColorWell *colorWell = [[cellView subviews] lastObject];
        NSColor     *color     = [NSColor blackColor];
        if([userDefaults stringForKey:key])
        {
            color = OENSColorFromString([userDefaults stringForKey:key]);
        }
        [colorWell setColor:color];
        [colorWell setAction:@selector(changeUDColor:)];
        [colorWell setTarget:self];
    }
    else if(type == LabelType)
    {
        NSString *label  = [keyDescription objectForKey:LabelKey];

        NSTextField *labelField = [[cellView subviews] objectAtIndex:0];
        [labelField setStringValue:label];
    }
    else if(type == PopoverType)
    {
        NSString *label  = [keyDescription objectForKey:LabelKey];
        NSArray *options = [keyDescription objectForKey:OptionsKey];
        NSString *action = [keyDescription objectForKey:ActionKey];
        NSTextField *labelField = [[cellView subviews] objectAtIndex:0];
        [labelField setStringValue:label];

        NSPopUpButton *popup = [[cellView subviews] lastObject];
        [popup setAction:NSSelectorFromString(action)];
        [popup setTarget:self];

        [options enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            [popup addItemWithTitle:[obj objectForKey:LabelKey]];
            [[popup itemAtIndex:idx] setRepresentedObject:[obj objectForKey:ValueKey]];
        }];
    }
    return cellView;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    NSDictionary *keyDescription = [[self keyDescriptions] objectAtIndex:row];
	NSString *type   = [keyDescription objectForKey:@"type"];
    if(type == SeparatorType) return 10.0;
    if(type == CheckboxType)  return 20.0;
    return 29.0;
}
#pragma mark - NSTableView DataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [[self keyDescriptions] count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	return [[self keyDescriptions] objectAtIndex:row];
}

#pragma mark - OEPreferencePane Protocol
- (NSImage *)icon
{
    return [NSImage imageNamed:@"debug_tab_icon"];
}

- (NSString *)title
{
    return @"Secrets";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
    return NSMakeSize(320, 400);
}

- (NSString *)nibName
{
    return @"OEPrefDebugController";
}
@end
