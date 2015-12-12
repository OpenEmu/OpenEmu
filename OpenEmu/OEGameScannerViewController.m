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

#import "OEGameScannerViewController.h"

#import "OELibraryDatabase.h"
#import "OELibraryController.h"
#import "OELibraryGamesViewController.h"

#import "OEButton.h"
#import "OESearchField.h"
#import "OEMenu.h"
#import "OEBackgroundColorView.h"
#import "OECoreTableButtonCell.h"

#import "OEHUDAlert.h"

#import "OEDBSystem.h"
#import "OEImportOperation.h"
#import "OEGameInfoHelper.h"

@import QuartzCore;

#import "OpenEmu-Swift.h"

#define OEGameScannerUpdateDelay 0.2

@interface OEGameScannerViewController ()

@property NSMutableArray *itemsRequiringAttention;
@property BOOL isScanningDirectory;

@property (nonatomic, weak) IBOutlet NSView *bottomBar;
@property (nonatomic, weak) IBOutlet NSScrollView *sourceListScrollView;

@property (nonatomic) BOOL gameScannerIsVisible;

@property (weak) IBOutlet OELibraryGamesViewController *libraryGamesViewController;

@end

@implementation OEGameScannerViewController

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        // The game scanner view is already visible in OELibraryGamesViewController.xib.
        _gameScannerIsVisible = YES;
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameInfoHelperWillUpdate:) name:OEGameInfoHelperWillUpdateNotificationName object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameInfoHelperDidChangeUpdateProgress:) name:OEGameInfoHelperDidChangeUpdateProgressNotificationName object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameInfoHelperDidUpdate:) name:OEGameInfoHelperDidUpdateNotificationName object:nil];
    }
    return self;
}

- (NSString *)nibName
{
    return @"OEGameScanner";
}

- (void)awakeFromNib
{
    [self view]; // load other xib as well

    if(!_scannerView || !_issuesView)
        return;

    [self OE_setupActionsMenu];

    NSMenuItem *item = nil;
    NSMenu     *menu = [[NSMenu alloc] init];

    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Select All", @"") action:@selector(selectAll:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Deselect All", @"") action:@selector(deselectAll:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];

    [[self issuesView] setMenu:menu];

    [self layoutSidebarViewsWithVisibleGameScannerView:NO animated:NO];

    [self setItemsRequiringAttention:[NSMutableArray array]];

    [[self importer] setDelegate:self];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameScannerViewFrameChanged:) name:NSViewFrameDidChangeNotification object:self.scannerView];

    [self OE_createFixButton];


    // Show game scanner if importer is running already
    // or game info is downloading
    if([[self importer] status] == OEImporterStatusRunning || [[OEGameInfoHelper sharedHelper] isUpdating])
    {
        [self OE_updateProgress];
        [self showGameScannerViewAnimated:YES];
    }
}

- (void)viewDidAppear
{
    [[self issuesView] reloadData];
}
#pragma mark -
- (OEROMImporter *)importer
{
    return [[OELibraryDatabase defaultDatabase] importer];
}

#pragma mark -
- (void)OE_updateProgress
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(OE_updateProgress) object:nil];
    [CATransaction begin];
    OEGameInfoHelper *infoHelper = [OEGameInfoHelper sharedHelper];
    if([infoHelper isUpdating])
    {
        [[self progressIndicator] setMinValue:0];
        [[self progressIndicator] setDoubleValue:[infoHelper downloadProgress]];
        [[self progressIndicator] setMaxValue:1.0];
        [[self progressIndicator] setIndeterminate:NO];
        [[self progressIndicator] startAnimation:self];

        [[self fixButton] setHidden:YES];
        [[self statusLabel] setStringValue:NSLocalizedString(@"Downloading Game DB",@"")];
    }
    else
    {
        OEROMImporter *importer = [self importer];

        NSUInteger maxItems = [[self importer] totalNumberOfItems];
        [[self progressIndicator] setMinValue:0];
        [[self progressIndicator] setDoubleValue:[[self importer] numberOfProcessedItems]];
        [[self progressIndicator] setMaxValue:maxItems];

        NSString *status;

        if([importer status] == OEImporterStatusRunning)
        {
            [[self progressIndicator] setIndeterminate:NO];
            [[self progressIndicator] startAnimation:self];

            NSInteger count = MIN([[self importer] numberOfProcessedItems]+1, maxItems);
            status = [NSString stringWithFormat:NSLocalizedString(@"Game %ld of %ld", @""), count, maxItems];

            if([self isScanningDirectory])
                status = NSLocalizedString(@"Scanning Directory", @"");
        }
        else if([importer status] == OEImporterStatusStopped)
        {
            [[self progressIndicator] stopAnimation:self];
            [[self progressIndicator] setIndeterminate:YES];
            status = NSLocalizedString(@"Done", @"");
        }
        else
        {
            [[self progressIndicator] stopAnimation:self];
            [[self progressIndicator] setIndeterminate:YES];
            status = NSLocalizedString(@"Scanner Paused", @"");
        }

        BOOL hideButton = YES;
        if([[self itemsRequiringAttention] count] != 0)
        {
            [[self fixButton] setTitle:[[self itemsRequiringAttention] count] > 1 ? [NSString stringWithFormat:NSLocalizedString(@"Resolve %ld Issues", @""), [[self itemsRequiringAttention] count]] : [NSString stringWithFormat:NSLocalizedString(@"Resolve %ld Issue", @""), [[self itemsRequiringAttention] count]]];
            [[self fixButton] sizeToFit];
            hideButton = NO;
            
            status = @"";
        }
        
        [[self fixButton] setHidden:hideButton];
        [[self statusLabel] setStringValue:status];
    }

    [CATransaction commit];
}

- (void)OE_setupActionsMenu
{
    NSMutableSet *systemIDSet = [NSMutableSet set];
    for(OEImportOperation *item in [self itemsRequiringAttention])
    {
        NSArray *systemIDs = [item systemIdentifiers];
        [systemIDSet addObjectsFromArray:systemIDs];
    }

    NSMenu *menu = [[NSMenu alloc] init];

    NSArray *systemIDs = [systemIDSet allObjects];
    NSManagedObjectContext *context = [[[self libraryController] database] mainThreadContext];
    for(NSString *systemID in systemIDs)
    {
        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemID inContext:context];
        NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:[system name] action:NULL keyEquivalent:@""];
        [menuItem setImage:[system icon]];
        [menuItem setRepresentedObject:systemID];

        [menu addItem:menuItem];
    }

    // sort by system name
    NSArray *items = [[menu itemArray] sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        return [[obj1 title] compare:[obj2 title]];
    }];

    [menu removeAllItems];
    [menu addItemWithTitle:NSLocalizedString(@"Don't Import Selected", @"") action:NULL keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];

    [items enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [menu addItem:obj];
    }];


    [[self actionPopUpButton] setMenu:menu];
}

- (void)OE_createFixButton
{
    OEButton *fixIssuesButton = [[OEButton alloc] initWithFrame:(NSRect){ { 14, 4 }, { [[self scannerView] frame].size.width - 20, 20 } }];
    [fixIssuesButton setAutoresizingMask:NSViewWidthSizable];
    [fixIssuesButton setAlignment:NSLeftTextAlignment];
    [fixIssuesButton setImagePosition:NSImageRight];
    [fixIssuesButton setThemeKey:@"game_scanner_fix_issues"];
    [fixIssuesButton setTarget:[self libraryGamesViewController]];
    [fixIssuesButton setAction:@selector(showIssuesView:)];
    [fixIssuesButton setTitle:NSLocalizedString(@"Resolve Issues", @"")];
    [fixIssuesButton sizeToFit];

    [fixIssuesButton setHidden:YES];

    [[self scannerView] addSubview:fixIssuesButton];
    [self setFixButton:fixIssuesButton];
}

- (void)gameScannerViewFrameChanged:(NSNotification*)notification
{
    NSRect bounds = [[notification object] bounds];
    CGFloat width = NSWidth(bounds);

    NSRect frame = [[self progressIndicator] frame];
    frame.origin.x = 16;
    frame.size.width = width - 16 - 38;
    [[self progressIndicator] setFrame:frame];

    frame = [[self headlineLabel] frame];
    frame.origin.x = 17;
    frame.size.width = width - 17 - 12;
    [[self headlineLabel] setFrame:frame];

    frame = [[self statusLabel] frame];
    frame.origin.x = 17;
    frame.size.width = width - 17 - 12;
    [[self statusLabel] setFrame:frame];

    frame = [[self fixButton] frame];
    frame.origin.x = 14;
    [[self fixButton] setFrame:frame];
    [[self fixButton] sizeToFit];
}

#pragma mark - OESidebarItem Implementation

- (NSImage *)sidebarIcon
{
    return nil;
}
- (NSString *)sidebarName
{
    return NSLocalizedString(@"Game Scanner", @"");
}
- (NSString *)sidebarID
{
    return @"Game Scanner";
}

- (NSString *)viewControllerClassName
{
    return [self className];
}

- (void)setSidebarName:(NSString *)newName
{
}

- (BOOL)isSelectableInSidebar
{
    return YES;
}

- (BOOL)isEditableInSidebar
{
    return NO;
}

- (BOOL)isGroupHeaderInSidebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

#pragma mark - OEROMImporter Delegate

- (void)romImporterDidStart:(OEROMImporter *)importer
{
    DLog();
    self.isScanningDirectory = NO;

    int64_t delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        if([[self importer] totalNumberOfItems] != [[self importer] numberOfProcessedItems])
        {
            [self OE_updateProgress];
            [self showGameScannerViewAnimated:YES];
        }
    });
}

- (void)romImporterDidCancel:(OEROMImporter *)importer
{
    DLog();
    [self OE_updateProgress];
}

- (void)romImporterDidPause:(OEROMImporter *)importer
{
    DLog();
    [self OE_updateProgress];
}

- (void)romImporterDidFinish:(OEROMImporter *)importer
{
    DLog();
    int64_t delayInSeconds = 0.5;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        if([[self importer] totalNumberOfItems] == [[self importer] numberOfProcessedItems] && ![[OEGameInfoHelper sharedHelper] isUpdating])
            [self hideGameScannerViewAnimated:YES];
    });
}

- (void)romImporterChangedItemCount:(OEROMImporter*)importer
{
    [self performSelector:@selector(OE_updateProgress) withObject:nil afterDelay:OEGameScannerUpdateDelay];
}

- (void)romImporter:(OEROMImporter*)importer stoppedProcessingItem:(OEImportOperation*)item
{
    if([[item error] domain] == OEImportErrorDomainResolvable && [[item error] code] == OEImportErrorCodeMultipleSystems)
    {
        [[self itemsRequiringAttention] addObject:item];
        [[self issuesView] reloadData];
        [self OE_setupActionsMenu];
        [self showGameScannerViewAnimated:YES];
    }
    [self OE_updateProgress];
}

#pragma mark - OEGameInfo Helper
- (void)gameInfoHelperWillUpdate:(NSNotification*)notification
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self OE_updateProgress];
        [self showGameScannerViewAnimated:YES];
    });
}
- (void)gameInfoHelperDidChangeUpdateProgress:(NSNotification*)notification
{
    [self OE_updateProgress];
}
- (void)gameInfoHelperDidUpdate:(NSNotification*)notification
{
    [self OE_updateProgress];
    int64_t delayInSeconds = 0.5;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        if([[self importer] totalNumberOfItems] == [[self importer] numberOfProcessedItems])
            [self hideGameScannerViewAnimated:YES];
    });
}

#pragma mark - NSTableViewDataSource Implementation

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[self itemsRequiringAttention] count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *columnIdentifier = [tableColumn identifier];
    OEImportOperation *item = [[self itemsRequiringAttention] objectAtIndex:row];
    if([columnIdentifier isEqualToString:@"path"])
    {
        return [[item URL] lastPathComponent];
    }
    else if([columnIdentifier isEqualToString:@"checked"])
    {
        return @([item checked]);
    }

    return nil;
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *columnIdentifier = [tableColumn identifier];
    if([columnIdentifier isEqualToString:@"checked"])
    {
        OEImportOperation *item = [[self itemsRequiringAttention] objectAtIndex:row];
        [item setChecked:[object boolValue]];
    }
}

#pragma mark - NSTableViewDelegate Implementation

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return YES;
}

- (NSString *)tableView:(NSTableView *)tableView toolTipForCell:(NSCell *)cell rect:(NSRectPointer)rect tableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row mouseLocation:(NSPoint)mouseLocation
{
    if(row >= [[self itemsRequiringAttention] count])
        return nil;

    NSString *columnIdentifier = [tableColumn identifier];
    if([columnIdentifier isEqualToString:@"path"])
    {
        OEImportOperation *item = [[self itemsRequiringAttention] objectAtIndex:row];
        return [[item sourceURL] path];
    }

    return nil;
}

#pragma mark - UI Actions Issues

- (void)selectAll:(id)sender
{
    [[self itemsRequiringAttention] setValue:@YES forKeyPath:@"checked"];
    [[self issuesView] reloadData];
}

- (void)deselectAll:(id)sender
{
    [[self itemsRequiringAttention] setValue:@NO forKeyPath:@"checked"];
    [[self issuesView] reloadData];
}

- (IBAction)resolveIssues:(id)sender
{
    NSIndexSet *selectedItemIndexes =
    [[self itemsRequiringAttention] indexesOfObjectsPassingTest:
     ^ BOOL (id obj, NSUInteger idx, BOOL *stop)
     {
         return [obj checked];
     }];

    [[self issuesView] beginUpdates];
    [[self issuesView] removeRowsAtIndexes:selectedItemIndexes withAnimation:NSTableViewAnimationEffectGap];
    [[self issuesView] endUpdates];

    NSString *selectedSystem = [[[self actionPopUpButton] selectedItem] representedObject];
    if(selectedSystem != nil)
    {
        [[self itemsRequiringAttention] enumerateObjectsAtIndexes:selectedItemIndexes options:0 usingBlock:
         ^(OEImportOperation *item, NSUInteger idx, BOOL *stop)
         {
             [item setSystemIdentifiers:@[selectedSystem]];
             [[item importer] rescheduleOperation:item];
             [item setCompletionBlock:nil];
             [item setCompletionHandler:nil];
         }];
    }
    else
    {
        [[self itemsRequiringAttention] enumerateObjectsAtIndexes:selectedItemIndexes options:0 usingBlock:
         ^(OEImportOperation *item, NSUInteger idx, BOOL *stop)
         {
             NSURL *extractedFile = [item extractedFileURL];
             if(extractedFile)
                 [[NSFileManager defaultManager] removeItemAtURL:extractedFile error:nil];
         }];
    }

    [[self itemsRequiringAttention] removeObjectsAtIndexes:selectedItemIndexes];

    [self OE_setupActionsMenu];
    [self OE_updateProgress];

    [[self issuesView] reloadData];

    [[self importer] start];
    [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self];

    if([[self importer] numberOfProcessedItems] == [[self importer] totalNumberOfItems])
    {
        [self hideGameScannerViewAnimated:YES];
    }

    if([[self itemsRequiringAttention] count] == 0) {
        [[self view] removeFromSuperview];
    }
    
    // Re-enable toolbar controls.
    OELibraryToolbar *toolbar = [[self libraryController] toolbar];
    [[toolbar categorySelector] setEnabled:YES];
    [[toolbar gridViewButton] setEnabled:YES];
    [[toolbar listViewButton] setEnabled:YES];
    [[toolbar gridSizeSlider] setEnabled:YES];
    [[toolbar searchField] setEnabled:YES];
}

#pragma mark - UI Actions Scanner

- (IBAction)buttonAction:(id)sender;
{
    if([NSEvent modifierFlags] & NSAlternateKeyMask)
    {
        [[self importer] pause];
        OEHUDAlert *cancelAlert = [[OEHUDAlert alloc] init];

        cancelAlert.headlineText = NSLocalizedString(@"Do you really want to cancel the import process?", @"");
        cancelAlert.messageText  = NSLocalizedString(@"Choose Yes to remove all items from the queue. Items that finished importing will be preserved in your library.", @"");
        cancelAlert.defaultButtonTitle   = NSLocalizedString(@"Yes", @"");
        cancelAlert.alternateButtonTitle = NSLocalizedString(@"No", @"");

        [sender setState:[sender state] == NSOnState ? NSOffState : NSOnState];
        if([cancelAlert runModal] == NSAlertFirstButtonReturn)
        {
            [[self importer] cancel];
            [self setItemsRequiringAttention:[NSMutableArray array]];
            [self OE_updateProgress];
            if([[self view] superview])
            {
                [[self issuesView] reloadData];
                [[self view] removeFromSuperview];
            }
            [self hideGameScannerViewAnimated:YES];

            [sender setState:NSOffState];
        } else
            [[self importer] start];
    }
    else
    {
        [[self importer] togglePause];
    }
    [self OE_updateProgress];
}

#pragma mark - Sidebar View Layout

- (void)layoutSidebarViewsWithVisibleGameScannerView:(BOOL)visibleGameScannerView animated:(BOOL)animated
{
    NSRect gameScannerFrame = self.scannerView.frame;
    gameScannerFrame.origin.y = visibleGameScannerView ? 0 : -NSHeight(gameScannerFrame);
    
    NSRect bottomBarFrame = self.bottomBar.frame;
    bottomBarFrame.origin.y = NSMaxY(gameScannerFrame);
    
    NSRect sourceListFrame = self.sourceListScrollView.frame;
    sourceListFrame.origin.y = NSMaxY(bottomBarFrame);
    sourceListFrame.size.height = NSHeight(self.sourceListScrollView.superview.frame) - sourceListFrame.origin.y;
    
    if(animated)
    {
        [NSAnimationContext beginGrouping];
        
        // Set frames through animator proxies to implicitly animate changes.
        self.scannerView.animator.frame = gameScannerFrame;
        self.bottomBar.animator.frame = bottomBarFrame;
        self.sourceListScrollView.animator.frame = sourceListFrame;
    
        [NSAnimationContext endGrouping];
    }
    else
    {
        // Set frames directly without implicit animations.
        self.scannerView.frame = gameScannerFrame;
        self.bottomBar.frame = bottomBarFrame;
        self.sourceListScrollView.frame = sourceListFrame;
    }

    self.gameScannerIsVisible = visibleGameScannerView;
}

#pragma mark - Show/Hide Game Scanner View

- (void)showGameScannerViewAnimated:(BOOL)animated
{
    [self layoutSidebarViewsWithVisibleGameScannerView:YES animated:animated];
}

- (void)hideGameScannerViewAnimated:(BOOL)animated
{
    if([[self itemsRequiringAttention] count] > 0)
        return;

    [self layoutSidebarViewsWithVisibleGameScannerView:NO animated:animated];
}

@end
