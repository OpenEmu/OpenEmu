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

#import "OEBackgroundColorView.h"

#import "OEImportOperation.h"
#import "OECoreTableButtonCell.h"

#import "OEButton.h"
#import "OEMenu.h"
#import "OEDBSystem.h"

#import "OEHUDAlert.h"

#import "OEGameInfoHelper.h"

#define OEGameScannerUpdateDelay 0.2

@interface OEGameScannerViewController ()
@property NSMutableArray *itemsRequiringAttention;
@property BOOL isScanningDirectory;
@end

@implementation OEGameScannerViewController
- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
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

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewBoundsDidChangeNotification object:[self view]];
}

- (void)setView:(NSView *)view
{
    [super setView:view];

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

}

- (void)setGameScannerView:(NSView *)gameScannerView
{
    _gameScannerView = gameScannerView;

    [self setItemsRequiringAttention:[NSMutableArray array]];

    [[self importer] setDelegate:self];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameScannerViewFrameChanged:) name:NSViewFrameDidChangeNotification object:gameScannerView];

    [self OE_createFixButton];

    // Show game scanner if importer is running already
    // or game info is downloading
    if([[self importer] status] == OEImporterStatusRunning || [[OEGameInfoHelper sharedHelper] isUpdating])
    {
        [self OE_updateProgress];
        [self OE_showGameScannerView];
    }
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
        [[self statusLabel] setStringValue:@"Downloading Game DB"];
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
                status = NSLocalizedString(@"Scanning Directory", "");
        }
        else if([importer status] == OEImporterStatusStopped)
        {
            [[self progressIndicator] stopAnimation:self];
            [[self progressIndicator] setIndeterminate:YES];
            status = @"Done";
        }
        else
        {
            [[self progressIndicator] stopAnimation:self];
            [[self progressIndicator] setIndeterminate:YES];
            status = @"Scanner Paused";
        }

        BOOL hideButton = YES;
        if([[self itemsRequiringAttention] count] != 0)
        {
            [[self fixButton] setTitle:[NSString stringWithFormat:NSLocalizedString(@"Resolve %ld Issues", @""), [[self itemsRequiringAttention] count]]];
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
    [menu addItemWithTitle:NSLocalizedString(@"Don't Import Selected", @"") action:NULL keyEquivalent:@""];

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

    [[self actionPopUpButton] setMenu:menu];
}

- (void)OE_createFixButton
{
    OEButton *fixIssuesButton = [[OEButton alloc] initWithFrame:(NSRect){ { 14, 4 }, { [[self view] frame].size.width - 20, 20 } }];
    [fixIssuesButton setAutoresizingMask:NSViewWidthSizable];
    [fixIssuesButton setAlignment:NSLeftTextAlignment];
    [fixIssuesButton setImagePosition:NSImageRight];
    [fixIssuesButton setThemeKey:@"game_scanner_fix_issues"];
    [fixIssuesButton setTarget:self];
    [fixIssuesButton setAction:@selector(showIssuesView:)];
    [fixIssuesButton setTitle:NSLocalizedString(@"Resolve Issues", @"")];
    [fixIssuesButton sizeToFit];

    [fixIssuesButton setHidden:YES];

    [[self gameScannerView] addSubview:fixIssuesButton];
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

#pragma mark - OELibrarySubviewController Protocol Implementation

- (void)setRepresentedObject:(id)representedObject
{
}

- (id)representedObject
{
    return self;
}

- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{
}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    if(libraryController == nil) return;
    _libraryController = libraryController;
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
    return NSLocalizedString(@"Game Scanner", @"");
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
            [self OE_showGameScannerView];
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
            [self OE_hideGameScannerView];
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
        [self OE_showGameScannerView];
    }
    [self OE_updateProgress];
}

#pragma mark - OEGameInfo Helper
- (void)gameInfoHelperWillUpdate:(NSNotification*)notification
{
    [self OE_updateProgress];
    [self OE_showGameScannerView];
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
            [self OE_hideGameScannerView];
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
         }];
    }

    [[self itemsRequiringAttention] removeObjectsAtIndexes:selectedItemIndexes];

    [self OE_setupActionsMenu];
    [self OE_updateProgress];

    [[self issuesView] reloadData];

    [[self importer] start];
    [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self];
}

#pragma mark - UI Actions Scanner

- (IBAction)buttonAction:(id)sender;
{
    if([NSEvent modifierFlags] & NSAlternateKeyMask)
    {
        [[self importer] pause];
        OEHUDAlert *cancelAlert = [[OEHUDAlert alloc] init];

        cancelAlert.headlineText = NSLocalizedString(@"Do you really want to cancel the import process?", @"");
        cancelAlert.messageText  = NSLocalizedString(@"Chose Yes to remove all items from the queue. Items that finished importing will be preserved in your library.", @"");
        cancelAlert.defaultButtonTitle   = NSLocalizedString(@"Yes", @"");
        cancelAlert.alternateButtonTitle = NSLocalizedString(@"No", @"");

        [sender setState:[sender state] == NSOnState ? NSOffState : NSOnState];
        if([cancelAlert runModal] == NSAlertDefaultReturn)
        {
            [[self importer] cancel];
            [self OE_hideGameScannerView];

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

- (IBAction)showIssuesView:(id)sender
{
    [[self issuesView] reloadData];
    [[self libraryController] showViewController:self];
    [[self issuesView] sizeToFit];
}

#pragma mark - Private

- (void)OE_showGameScannerView
{
    NSView *scannerView = [self gameScannerView];
    NSView *sidebarView = [[[scannerView superview] subviews] objectAtIndex:0];
    NSView *superView   = [sidebarView superview];
    NSRect superviewBounds = [superView bounds];
    NSRect scannerViewBounds = [scannerView bounds];

    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setCompletionHandler:
     ^{
         [sidebarView setFrame:(NSRect){ { 0, NSHeight(scannerViewBounds) }, { NSWidth(superviewBounds), NSHeight(superviewBounds) - NSHeight(scannerViewBounds) } }];
     }];
    [[scannerView animator] setFrameOrigin:NSMakePoint(0, 0)];
    [NSAnimationContext endGrouping];
}

- (void)OE_hideGameScannerView
{
    if([[self itemsRequiringAttention] count] != 0) return;

    NSView *scannerView = [self gameScannerView];
    NSView *sidebarView = [[[scannerView superview] subviews] objectAtIndex:0];
    NSView *superView   = [sidebarView superview];

    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setCompletionHandler:
     ^{
         [sidebarView setFrame:[superView bounds]];
     }];
    [[scannerView animator] setFrameOrigin:NSMakePoint(0, -NSHeight([scannerView frame]))];
    [NSAnimationContext endGrouping];
}

@end
