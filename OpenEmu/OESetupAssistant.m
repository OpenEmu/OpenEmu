/*
 Copyright (c) 2009-2012, OpenEmu Team
 
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

#import "OESetupAssistant.h"
#import "OESetupAssistantTableView.h"
#import "OEFiniteStateMachine.h"
#import "OEButton.h"
#import <objc/runtime.h>

#import "OECoreUpdater.h"
#import "OECoreDownload.h"

@import QuartzCore;

#pragma mark - Public variables

// Public user default keys
NSString *const OESetupAssistantHasFinishedKey = @"setupAssistantFinished";

#pragma mark - Private variables

static const NSTimeInterval _OEVideoIntroductionDuration = 10;

enum : OEFSMStateLabel
{
    _OEFSMVideoIntroState,
    _OEFSMWelcomeState,
    _OEFSMCoreSelectionState,
    _OEFSMGameScannerSelectionState,
    _OEFSMGameScannerVolumeSelectionState,
    _OEFSMLastScreenState,
    _OEFSMEndState,
};

// We need to keep event values in sync with the OEFSMEventNumber user-defined runtime attribute in the nib file.
// Currently, it is being used by OEButtons (Next, Back, Go) only.
enum : OEFSMEventLabel
{
    _OEFSMBackEvent                 = 1,
    _OEFSMNextEvent                 = 2,
    _OEFSMNextWithCheckmarkEvent    = 3,
    _OEFSMNextWithoutCheckmarkEvent = 4,
};

// The enum above makes Xcode think it needs to indent all lines past this point. The following NOP code
// convinces Xcode that it does not need to indent.
#if 0
}
#endif

#pragma mark - OESetupCoreInfo

@interface OESetupCoreInfo : NSObject
@property(nonatomic, weak) OECoreDownload                       *core;
@property(nonatomic, assign, getter = isSelected) BOOL           selected;
@property(nonatomic, assign, getter = isDownloadRequested) BOOL  downloadRequested;
@property(nonatomic, assign, getter = isDefaultCore) BOOL        defaultCore;
+ (instancetype)setupCoreInfoWithCore:(OECoreDownload *)core;
@end

#pragma mark - OESetupVolumeInfo

@interface OESetupVolumeInfo : NSObject
@property(nonatomic, strong) NSURL                     *URL;
@property(nonatomic, copy) NSString                    *name;
@property(nonatomic, assign, getter = isSelected) BOOL  selected;
+ (instancetype)setupVolumeInfoWithURL:(NSURL *)URL name:(NSString *)name;
@end


#pragma mark - NSButton (OESetupAssistantAdditions)

@interface NSButton (OESetupAssistantAdditions)
@property(nonatomic, strong) NSNumber *OEFSMEventNumber;
@end

#pragma mark - OESetupAssistant

@interface OESetupAssistant ()
{
    NSMutableArray       *_coresToDownload; // contains OESetupCoreInfo objects; it's a table view data source
    NSMutableArray       *_volumesToScan;   // contains OESetupVolumeInfo objects; it's a table view data source
    CATransition         *_viewTransition;
    OEFiniteStateMachine *_fsm;
}

// IB outlets
@property(nonatomic, weak) IBOutlet NSView *replaceView;
@property(nonatomic, weak) IBOutlet NSView *welcomeView;
@property(nonatomic, weak) IBOutlet NSView *coreSelectionView;
@property(nonatomic, weak) IBOutlet NSView *gameScannerAllowView;
@property(nonatomic, weak) IBOutlet NSView *gameScannerVolumeSelectionView;
@property(nonatomic, weak) IBOutlet NSView *lastStepView;
@property(nonatomic, weak) IBOutlet NSButton *allowScanForGames;
@property(nonatomic, weak) IBOutlet OESetupAssistantTableView *installCoreTableView;
@property(nonatomic, weak) IBOutlet OESetupAssistantTableView *mountedVolumesTableView;

- (IBAction)processFSMButtonAction:(id)sender;
- (IBAction)processAllowGameScannerNextButtonAction:(id)sender;

- (void)OE_goForwardToView:(NSView *)view;
- (void)OE_goBackToView:(NSView *)view;
- (void)OE_dissolveToView:(NSView *)view;
- (void)OE_processVolumeNotification:(NSNotification *)notification;

@end

@implementation OESetupAssistant

- (void)dealloc
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
}

- (NSString *)nibName
{
    return @"OESetupAssistant";
}

// For some reason, -viewDidLoad is sent multiple times to the setup assistant.
// Good old -loadView is sent only once, so we prefer it.
- (void)loadView
{
    [super loadView];
    
    // TODO: need to fail gracefully if we have no internet connection.
    [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForNewCores:) withObject:@(NO)];
    [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForUpdates) withObject:nil];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(OE_processVolumeNotification:) name:NSWorkspaceDidMountNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(OE_processVolumeNotification:) name:NSWorkspaceDidUnmountNotification object:nil];

    _coresToDownload = [NSMutableArray array];

    _volumesToScan   = [NSMutableArray array];
    NSArray *volumes = [[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:@[NSURLLocalizedNameKey] options:NSVolumeEnumerationSkipHiddenVolumes];
    for(NSURL *volumeURL in volumes)
    {
        NSString *volumeName = nil;
        if(![volumeURL getResourceValue:&volumeName forKey:NSURLVolumeLocalizedNameKey error:NULL]) volumeName = @"Unnamed Volume";

        [_volumesToScan addObject:[OESetupVolumeInfo setupVolumeInfoWithURL:volumeURL name:volumeName]];
    }


    [[self replaceView] setWantsLayer:YES];

    // setup default transition properties
    _viewTransition = [CATransition animation];
    [_viewTransition setType:kCATransitionFade];
    [_viewTransition setSubtype:kCATransitionFromRight];
    [_viewTransition setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault]];
    [_viewTransition setDuration:1.0];
    [[self replaceView] setAnimations:@{@"subviews" : _viewTransition}];

    [self OE_setupFiniteStateMachine];
    [_fsm start];
}

- (void)viewDidAppear
{
    [super viewDidAppear];

    NSWindow *window = [[self view] window];
    [window setStyleMask:[window styleMask] ^ NSWindowStyleMaskClosable];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    NSWindow *window = [[self view] window];
    [window setStyleMask:[window styleMask] | NSWindowStyleMaskClosable];
}

- (IBAction)processFSMButtonAction:(id)sender
{
    OEFSMEventLabel event = [[(NSButton *)sender OEFSMEventNumber] unsignedIntegerValue];
    [_fsm processEvent:event];
}

- (IBAction)processAllowGameScannerNextButtonAction:(id)sender
{
    OEFSMEventLabel event = ([[self allowScanForGames] state] == NSControlStateValueOn ? _OEFSMNextWithCheckmarkEvent : _OEFSMNextWithoutCheckmarkEvent);
    [_fsm processEvent:event];
}

- (void)OE_processVolumeNotification:(NSNotification *)notification
{
    if([[notification name] isEqualToString:NSWorkspaceDidMountNotification])
    {
        // NSWorkspaceVolumeLocalizedNameKey, NSWorkspaceVolumeURLKey
        NSURL *volumeURL     = [[notification userInfo] objectForKey:NSWorkspaceVolumeURLKey];
        NSString *volumeName = [[notification userInfo] objectForKey:NSWorkspaceVolumeLocalizedNameKey];

        [_volumesToScan addObject:[OESetupVolumeInfo setupVolumeInfoWithURL:volumeURL name:volumeName]];
    }
    else if([[notification name] isEqualToString:NSWorkspaceDidUnmountNotification])
    {
        // @"NSDevicePath"
        NSString *volumePath = [[notification userInfo] objectForKey:@"NSDevicePath"];
        NSUInteger indexForRemoval = [_volumesToScan indexOfObjectPassingTest:^BOOL(OESetupVolumeInfo *volumeInfo, NSUInteger idx, BOOL *stop) {
            if([[[volumeInfo URL] path] isEqualToString:volumePath])
            {
                *stop = YES;
                return YES;
            }
            return NO;
        }];

        if(indexForRemoval != NSNotFound) [_volumesToScan removeObjectAtIndex:indexForRemoval];
    }

    [_mountedVolumesTableView reloadData];
}

#pragma mark - Finite state machine setup

- (void)OE_setupFiniteStateMachine
{
    NSDictionary *stateDescriptions = (@{
                                       @(_OEFSMVideoIntroState)                 : @"Video introduction",
                                       @(_OEFSMWelcomeState)                    : @"Welcome screen",
                                       @(_OEFSMCoreSelectionState)              : @"Core selection",
                                       @(_OEFSMGameScannerSelectionState)       : @"Game scanner allow checkbox",
                                       @(_OEFSMGameScannerVolumeSelectionState) : @"Game scanner volume selection",
                                       @(_OEFSMLastScreenState)                 : @"Last screen",
                                       @(_OEFSMEndState)                        : @"This is the end, beautiful friend",
                                       });
    NSDictionary *eventDescriptions = (@{
                                       @(_OEFSMBackEvent)                 : @"Back",
                                       @(_OEFSMNextEvent)                 : @"Next",
                                       @(_OEFSMNextWithCheckmarkEvent)    : @"Next with checkmark",
                                       @(_OEFSMNextWithoutCheckmarkEvent) : @"Next without checkmark",
                                       });

    _fsm = [OEFiniteStateMachine new];
    [_fsm setStateDescriptions:stateDescriptions];
    [_fsm setEventDescriptions:eventDescriptions];
    [_fsm setActionsQueue:dispatch_get_main_queue()];

    OESetupAssistant __unsafe_unretained *blockSelf = self;

    // Video introduction

    [_fsm addState:_OEFSMVideoIntroState];
    [_fsm setTimerTransitionFrom:_OEFSMVideoIntroState to:_OEFSMWelcomeState delay:_OEVideoIntroductionDuration action:^{
        [blockSelf OE_dissolveToView:[blockSelf welcomeView]];
    }];

    // Welcome screen

    [_fsm addState:_OEFSMWelcomeState entry:^{
        // Note: we are not worrying about a core being removed from the core list
        NSArray *knownCores = [blockSelf->_coresToDownload valueForKey:@"core"];
        for(OECoreDownload *core in [[OECoreUpdater sharedUpdater] coreList])
        {
            if(![knownCores containsObject:core]) [blockSelf->_coresToDownload addObject:[OESetupCoreInfo setupCoreInfoWithCore:core]];
        }
        // Check if a core is set as default in OEApplicationDelegate
        for(OESetupCoreInfo *coreInfo in blockSelf->_coresToDownload)
        {
            NSString *coreID = [[coreInfo core] bundleIdentifier];
            NSArray *systemIDs = [[coreInfo core] systemIdentifiers];

            for(id system in systemIDs)
            {
                NSString *sysID = [NSString stringWithFormat:@"defaultCore.%@", system];
                NSString *userDef = [[NSUserDefaults standardUserDefaults] valueForKey:sysID];
                
                if(userDef != nil && [userDef caseInsensitiveCompare:coreID] == NSOrderedSame)
                    [coreInfo setDefaultCore:YES];
            }
        }
    }];
    [_fsm addTransitionFrom:_OEFSMWelcomeState to:_OEFSMCoreSelectionState event:_OEFSMNextEvent action:^{
        [blockSelf OE_goForwardToView:[blockSelf coreSelectionView]];
        // On Mavericks, if the user has scroll bars set to be alaways visible our scroll views do not update the scrollers correctly
        // so we have to force them
        [[blockSelf installCoreTableView] noteNumberOfRowsChanged];
    }];

    // Core selection screen

    [_fsm addState:_OEFSMCoreSelectionState exit:^{
        // Note: if the user selected a few cores and clicked next, we start downloading them.
        //       if the user goes back to the core selection screen, he shouldn't really deselect because
        //       once the download started, we don't cancel or remove it
        for(OESetupCoreInfo *coreInfo in blockSelf->_coresToDownload)
        {
            if([coreInfo isSelected] && ![coreInfo isDownloadRequested])
            {
                [[coreInfo core] startDownload:blockSelf];
                [coreInfo setDownloadRequested:YES];
            }
        }
     }];
    [_fsm addTransitionFrom:_OEFSMCoreSelectionState to:_OEFSMWelcomeState event:_OEFSMBackEvent action:^{
        [blockSelf OE_goBackToView:[blockSelf welcomeView]];
    }];
    [_fsm addTransitionFrom:_OEFSMCoreSelectionState to:_OEFSMLastScreenState event:_OEFSMNextEvent action:^{
        [blockSelf OE_goForwardToView:[blockSelf lastStepView]];
    }];

    // Game scanner allow checkbox screen
/*
    [_fsm addState:_OEFSMGameScannerSelectionState];
    [[self allowScanForGames] setState:NSOffState];
    [_fsm addTransitionFrom:_OEFSMGameScannerSelectionState to:_OEFSMCoreSelectionState event:_OEFSMBackEvent action:^{
        [blockSelf OE_goBackToView:[blockSelf coreSelectionView]];
    }];
    [_fsm addTransitionFrom:_OEFSMGameScannerSelectionState to:_OEFSMGameScannerVolumeSelectionState event:_OEFSMNextWithCheckmarkEvent action:^{
        [blockSelf OE_goForwardToView:[blockSelf gameScannerVolumeSelectionView]];
        // On Mavericks, if the user has scroll bars set to be alaways visible our scroll views do not update the scrollers correctly
        // so we have to force them
        [[blockSelf mountedVolumesTableView] noteNumberOfRowsChanged];
    }];
    [_fsm addTransitionFrom:_OEFSMGameScannerSelectionState to:_OEFSMLastScreenState event:_OEFSMNextWithoutCheckmarkEvent action:^{
        [blockSelf OE_goForwardToView:[blockSelf lastStepView]];
    }];

    // Game scanner volume selection screen

    [_fsm addState:_OEFSMGameScannerVolumeSelectionState];
    [_fsm addTransitionFrom:_OEFSMGameScannerVolumeSelectionState to:_OEFSMGameScannerSelectionState event:_OEFSMBackEvent action:^{
        [blockSelf OE_goBackToView:[blockSelf gameScannerAllowView]];
    }];
    [_fsm addTransitionFrom:_OEFSMGameScannerVolumeSelectionState to:_OEFSMLastScreenState event:_OEFSMNextEvent action:^{
        [blockSelf OE_goForwardToView:[blockSelf lastStepView]];
    }];
*/
    // Last screen

    [_fsm addState:_OEFSMLastScreenState];
    [_fsm addTransitionFrom:_OEFSMLastScreenState to:_OEFSMCoreSelectionState event:_OEFSMBackEvent action:^{
        [blockSelf OE_goBackToView:[blockSelf coreSelectionView]];
    }];
    [_fsm addTransitionFrom:_OEFSMLastScreenState to:_OEFSMEndState event:_OEFSMNextEvent action:nil];

    // This is the end, beautiful friend

    [_fsm addState:_OEFSMEndState entry:^{
        // Mark setup done
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:OESetupAssistantHasFinishedKey];

        NSMutableArray *selectedVolumes = nil;
        BOOL shouldScan = NO;
/*
        BOOL shouldScan = ([[blockSelf allowScanForGames] state] == NSOnState);
        if(shouldScan)
        {
            selectedVolumes = [NSMutableArray new];
            for(OESetupVolumeInfo *volumeInfo in blockSelf->_volumesToScan)
            {
                if([volumeInfo isSelected]) [selectedVolumes addObject:[volumeInfo URL]];
            }
        }
*/
        if([blockSelf completionBlock] != nil) [blockSelf completionBlock](shouldScan, selectedVolumes);
    }];
}

#pragma mark - View switching

- (void)OE_goBackToView:(NSView *)view
{
    [self OE_switchToView:view transitionType:kCATransitionPush transitionSubtype:kCATransitionFromLeft];
}

- (void)OE_goForwardToView:(NSView *)view
{
    [self OE_switchToView:view transitionType:kCATransitionPush transitionSubtype:kCATransitionFromRight];
}

- (void)OE_dissolveToView:(NSView *)view
{
    [self OE_switchToView:view transitionType:kCATransitionFade transitionSubtype:nil];
}

- (void)OE_switchToView:(NSView *)view transitionType:(NSString *)transitionType transitionSubtype:(NSString *)transitionSubtype
{
    [_viewTransition setType:transitionType];
    [_viewTransition setSubtype:transitionSubtype];

    [view setFrame:[_replaceView frame]];

    if([[_replaceView subviews] count] == 0)
    {
        [[_replaceView animator] addSubview:view];
    }
    else
    {
        NSView *oldView = [[_replaceView subviews] objectAtIndex:0];
        [[_replaceView animator] replaceSubview:oldView with:view];
    }
}

#pragma mark - NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if(tableView == [self installCoreTableView])         return [_coresToDownload count];
    else if(tableView == [self mountedVolumesTableView]) return [_volumesToScan count];

    return 0;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{    
    NSString *identifier = [tableColumn identifier];

    if(tableView == [self installCoreTableView])
    {
        OESetupCoreInfo *coreInfo = [_coresToDownload objectAtIndex:rowIndex];
        
        if([identifier isEqualToString:@"enabled"])             return @([coreInfo isSelected]);
        else if([identifier isEqualToString:@"emulatorName"])   return [[coreInfo core] name];
        else if([identifier isEqualToString:@"emulatorSystem"])
        {
            NSUInteger columnIndex = [tableView columnWithIdentifier:[tableColumn identifier]];
            OESetupAssistantMinorTextCell *cell = (OESetupAssistantMinorTextCell *)[self tableView:tableView dataCellForTableColumn:tableColumn row:rowIndex];
            NSMutableArray *systemNames = [[[coreInfo core] systemNames] mutableCopy];
            NSString *systemNamesString = nil;
            CGFloat columnWidth = NSWidth([tableView rectOfColumn:columnIndex]);
            CGFloat stringWidth = 0.0;
            do
            {
                systemNamesString = [systemNames componentsJoinedByString:@", "];
                stringWidth = [systemNamesString sizeWithAttributes:[cell attributes]].width;
                if([systemNames count] == 0)
                    return [[[coreInfo core] systemNames] componentsJoinedByString:@", "];
                
                [systemNames removeObjectAtIndex:[systemNames count]-1];
            } while(stringWidth > columnWidth);
            
            return systemNamesString;
        }
    }
    else if(tableView == [self mountedVolumesTableView])
    {
        OESetupVolumeInfo *volumeInfo = [_volumesToScan objectAtIndex:rowIndex];

        if([identifier isEqualToString:@"enabled"])        return @([volumeInfo isSelected]);
        else if([identifier isEqualToString:@"mountName"]) return ([volumeInfo name]);
    }

    return nil;
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    NSString *identifier = [tableColumn identifier];

    if(tableView == [self installCoreTableView])
    {
        OESetupCoreInfo *coreInfo = [_coresToDownload objectAtIndex:rowIndex];
        if([identifier isEqualToString:@"enabled"]) [coreInfo setSelected:[(NSNumber *)object boolValue]];
    }
    else if(tableView == [self mountedVolumesTableView])
    {
        OESetupVolumeInfo *volumeInfo = [_volumesToScan objectAtIndex:rowIndex];
        if([identifier isEqualToString:@"enabled"]) [volumeInfo setSelected:[(NSNumber *)object boolValue]];
    }

}

#pragma mark - NSTableViewDelegate

- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    if(tableView == _installCoreTableView)
    {
        if([[tableColumn identifier] isEqualToString:@"enabled"])
        {
            OESetupCoreInfo *coreInfo = [_coresToDownload objectAtIndex:rowIndex];
            return ![coreInfo isDownloadRequested];
        }

        return NO;
    }
    return [[tableColumn identifier] isEqualToString:@"enabled"];
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSCell *cell = [tableColumn dataCellForRow:row];

    if(tableView == _installCoreTableView && [[tableColumn identifier] isEqualToString:@"enabled"])
    {
        OESetupCoreInfo *coreInfo = [_coresToDownload objectAtIndex:row];
        NSButtonCell *buttonCell = (NSButtonCell *)cell;
        [buttonCell setEnabled:(![coreInfo isDownloadRequested])];
        
        if([coreInfo isDefaultCore]) [buttonCell setEnabled:NO];
    }

    return cell;
}

- (NSString*)tableView:(NSTableView *)tableView toolTipForCell:(NSCell *)cell rect:(NSRectPointer)rect tableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row mouseLocation:(NSPoint)mouseLocation
{
    if(tableView == _installCoreTableView && [[tableColumn identifier] isEqualToString:@"emulatorSystem"])
    {
        OESetupCoreInfo *coreInfo = [_coresToDownload objectAtIndex:row];
        return [[[coreInfo core] systemNames] componentsJoinedByString:@", "];
    }
    
    if([cell isKindOfClass:[NSTextFieldCell class]])
        return [cell stringValue];
    return @"";
}
@end

#pragma mark - OESetupCoreInfo

@implementation OESetupCoreInfo
+ (instancetype)setupCoreInfoWithCore:(OECoreDownload *)core
{
    OESetupCoreInfo *newCore = [self new];
    [newCore setCore:core];
    [newCore setSelected:YES];
    return newCore;
}
@end

#pragma mark - OESetupVolumeInfo

@implementation OESetupVolumeInfo
+ (instancetype)setupVolumeInfoWithURL:(NSURL *)URL name:(NSString *)name
{
    OESetupVolumeInfo *newVolume = [self new];
    [newVolume setURL:URL];
    [newVolume setName:name];
    [newVolume setSelected:YES];
    return newVolume;
}
@end

#pragma mark - NSButton (OESetupAssistantAdditions)

@implementation NSButton (OESetupAssistantAdditions)

- (void)setOEFSMEventNumber:(NSNumber *)number
{
    objc_setAssociatedObject(self, @"OEFSMEventNumber", number, OBJC_ASSOCIATION_RETAIN);
}

- (NSNumber *)OEFSMEventNumber
{
    return objc_getAssociatedObject(self, @"OEFSMEventNumber");
}

@end
