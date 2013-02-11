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

#import "OEImportItem.h"
#import "OECoreTableButtonCell.h"

#import "OEButton.h"
#import "OEMenu.h"
#import "OEDBSystem.h"

@interface OEGameScannerViewController ()
@property NSMutableArray *itemsRequiringAttention;
@end
@implementation OEGameScannerViewController
- (NSString*)nibName
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
    
    item = [[NSMenuItem alloc] initWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@""];
    [item setTarget:self];
    [menu addItem:item];
    item = [[NSMenuItem alloc] initWithTitle:@"Deselect All" action:@selector(deselectAll:) keyEquivalent:@""];
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
    if([[self importer] status] == OEImporterStatusRunning)
        [self OE_showGameScannerView];
}

#pragma mark -
- (OEROMImporter*)importer
{
    return [[OELibraryDatabase defaultDatabase] importer];
}
#pragma mark -
- (void)OE_updateProgress
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
        status = [NSString stringWithFormat:@"Game %ld of %ld", [[self importer] numberOfProcessedItems], maxItems];
    }
    else if([importer status] == OEImporterStatusStopped || [importer status] == OEImporterStatusStopping)
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
        [[self fixButton] setTitle:[NSString stringWithFormat:@"Resolve %ld Issues", [[self itemsRequiringAttention] count]]];
        [[self fixButton] sizeToFit];
        hideButton = NO;
        
        status = @"";
    }
    
    [[self fixButton] setHidden:hideButton];
    [[self statusLabel] setStringValue:status];
}

- (void)OE_setupActionsMenu
{
    NSMutableSet *systemIDSet = [NSMutableSet set];
    for(OEImportItem *item in [self itemsRequiringAttention])
    {
        id systemIDs = [[item importInfo] objectForKey:OEImportInfoSystemID];
        if([systemIDs isKindOfClass:[NSArray class]])
            [systemIDSet addObjectsFromArray:systemIDs];
    }
    
    NSMenu       *menu = [[NSMenu alloc] init];
    [menu addItemWithTitle:@"Don't Import Selected" action:NULL keyEquivalent:@""];
    
    NSArray *systemIDs = [systemIDSet allObjects];
    for(NSString *systemID in systemIDs)
    {
        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemID inDatabase:[[self importer] database]];
        NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:[system name] action:NULL keyEquivalent:@""];
        [menuItem setImage:[system icon]];
        [menuItem setRepresentedObject:systemID];
        
        [menu addItem:menuItem];
    }
    
    [[self actionPopUpButton] setMenu:menu];
}

- (void)OE_createFixButton
{
    OEButton *fixIssuesButton = [[OEButton alloc] initWithFrame:(NSRect){{ 14, 4}, { [[self view] frame].size.width - 20, 20 }}];
    [fixIssuesButton setAutoresizingMask:NSViewWidthSizable];
    [fixIssuesButton setAlignment:NSLeftTextAlignment];
    [fixIssuesButton setImagePosition:NSImageRight];
    [fixIssuesButton setThemeKey:@"game_scanner_fix_issues"];
    [fixIssuesButton setTarget:self];
    [fixIssuesButton setAction:@selector(showIssuesView:)];
    [fixIssuesButton setTitle:@"Resolve Issues"];
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
    frame.size.width = width-16-38;
    [[self progressIndicator] setFrame:frame];
    
    frame = [[self headlineLabel] frame];
    frame.origin.x = 17;
    frame.size.width = width-17-12;
    [[self headlineLabel] setFrame:frame];
    
    frame = [[self statusLabel] frame];
    frame.origin.x = 17;
    frame.size.width = width-17-12;
    [[self statusLabel] setFrame:frame];
    
    frame = [[self fixButton] frame];
    frame.origin.x = 14;
    [[self fixButton] setFrame:frame];
    [[self fixButton] sizeToFit];
}
#pragma mark - OELibrarySubviewController Protocol Implementation
- (void)setRepresentedObject:(id)representedObject
{}

- (id)representedObject
{
    return self;
}

- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{}

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
- (NSString*)sidebarName
{
    return @"Game Scaner";
}
- (NSString*)sidebarID
{
    return @"Game Scaner";
}

- (NSString*)viewControllerClassName
{
    return [self className];
}

- (void)setSidebarName:(NSString*)newName
{}

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
    int64_t delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if([[self importer] totalNumberOfItems] != [[self importer] numberOfProcessedItems])
        {
            [self OE_updateProgress];
            [self OE_showGameScannerView];
        }
    });
}

- (void)romImporterDidCancel:(OEROMImporter *)importer
{
    [self OE_updateProgress];
}

- (void)romImporterDidPause:(OEROMImporter *)importer
{
    [self OE_updateProgress];
}

- (void)romImporterDidFinish:(OEROMImporter *)importer
{
    int64_t delayInSeconds = 0.5;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if([[self importer] totalNumberOfItems] == [[self importer] numberOfProcessedItems])
            [self OE_hideGameScannerView];
    });
}

- (void)romImporterChangedItemCount:(OEROMImporter*)importer
{
    [self OE_updateProgress];
}

- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportItem*)item
{
}

- (void)romImporter:(OEROMImporter*)importer stoppedProcessingItem:(OEImportItem*)item
{
    if([[item error] domain] == OEImportErrorDomainResolvable && [[item error] code] == OEImportErrorCodeMultipleSystems)
    {
        [[self itemsRequiringAttention] addObject:item];
        [[self issuesView] reloadData];
        [self OE_setupActionsMenu];
    }
    [self OE_updateProgress];
}

#pragma mark - NSTableViewDataSource Implementation
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[self itemsRequiringAttention] count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *columnIdentifier = [tableColumn identifier];
    OEImportItem *item = [[self itemsRequiringAttention] objectAtIndex:row];
    if([columnIdentifier isEqualToString:@"path"])
    {
        return [[item URL] lastPathComponent];
    }

    return [[item importInfo] objectForKey:columnIdentifier];
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *columnIdentifier = [tableColumn identifier];
    if([columnIdentifier isEqualToString:@"path"]) return;
    
    OEImportItem *item = [[self itemsRequiringAttention] objectAtIndex:row];
    [[item importInfo] setObject:object forKey:columnIdentifier];
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

- (NSString*)tableView:(NSTableView *)tableView toolTipForCell:(NSCell *)cell rect:(NSRectPointer)rect tableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row mouseLocation:(NSPoint)mouseLocation
{
    if(row >= [[self itemsRequiringAttention] count])
        return nil;
    
    NSString *columnIdentifier = [tableColumn identifier];
    if([columnIdentifier isEqualToString:@"path"])
    {
        OEImportItem *item = [[self itemsRequiringAttention] objectAtIndex:row];
        return [[item sourceURL] path];
    }
    
    return nil;
}

#pragma mark - UI Actions Issues
- (void)selectAll:(id)sender
{
    [[self itemsRequiringAttention] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [[obj importInfo] setObject:@(YES) forKey:@"checked"];
    }];
    [[self issuesView] reloadData];
}

- (void)deselectAll:(id)sender
{
    [[self itemsRequiringAttention] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [[obj importInfo] setObject:@(NO) forKey:@"checked"];
    }];
    [[self issuesView] reloadData];
}

- (IBAction)resolveIssues:(id)sender
{
    NSIndexSet *allIndexes = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [[self itemsRequiringAttention] count])];
    NSIndexSet *selectedItemIndexes = [[self itemsRequiringAttention] indexesOfObjectsAtIndexes:allIndexes options:NSEnumerationConcurrent passingTest:^BOOL(id obj, NSUInteger idx, BOOL *stop) {
        return [[[obj importInfo] objectForKey:@"checked"] boolValue];
    }];
    
    [[self issuesView] beginUpdates];
    [[self issuesView] removeRowsAtIndexes:selectedItemIndexes withAnimation:NSTableViewAnimationEffectGap];
    [[self issuesView] endUpdates];
    
    NSString *selectedSystem = [[[self actionPopUpButton] selectedItem] representedObject];
    if(selectedSystem != nil)
    {
        [[self itemsRequiringAttention] enumerateObjectsAtIndexes:selectedItemIndexes options:NSEnumerationConcurrent usingBlock:^(OEImportItem *item, NSUInteger idx, BOOL *stop) {
            [[item importInfo] setObject:[NSArray arrayWithObject:selectedSystem] forKey:OEImportInfoSystemID];
            [item setError:nil];
            [item setImportState:OEImportItemStatusIdle];
            item.importStep ++;
        }];
    }
    
    [[self itemsRequiringAttention] removeObjectsAtIndexes:selectedItemIndexes];
    
    [self OE_setupActionsMenu];
    [self OE_updateProgress];

    [[self issuesView] reloadData];
    [[self importer] processNextItemIfNeeded];
    
    if([[self itemsRequiringAttention] count] == 0)
        [[NSNotificationCenter defaultCenter] postNotificationName:OESidebarSelectionDidChangeNotificationName object:self];
}

#pragma mark - UI Actions Scanner
- (IBAction)buttonAction:(id)sender
{
    if([NSEvent modifierFlags] & NSAlternateKeyMask)
    {
        //TODO: Show a proper (dark, rephrased) dialog here
        NSAlert *cancelAlert = [NSAlert alertWithMessageText:@"Do you really want to cancel the import process?" defaultButton:@"Yes" alternateButton:@"No" otherButton:@"" informativeTextWithFormat:@"Chose Yes to remove all items from the queue. Items that finished importing will be preserved in your library."];
        [sender setState:[sender state]==NSOnState?NSOffState:NSOnState];
        if([cancelAlert runModal] == NSAlertDefaultReturn)
        {
            [[self importer] cancel];
            [self OE_hideGameScannerView];
        
            [sender setState:NSOffState];
        }
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
    
    [NSAnimationContext beginGrouping];    
    [[NSAnimationContext currentContext] setCompletionHandler:^{
         [sidebarView setFrame:(NSRect){{0,NSHeight([scannerView bounds])}, {NSWidth([superView bounds]), NSHeight([superView bounds])-NSHeight([scannerView bounds])}}];
    }];
    [[scannerView animator] setFrameOrigin:NSMakePoint(0, 0)];
    [NSAnimationContext endGrouping];
}

- (void)OE_hideGameScannerView
{
    NSView *scannerView = [self gameScannerView];
    NSView *sidebarView = [[[scannerView superview] subviews] objectAtIndex:0];
    NSView *superView   = [sidebarView superview];
    
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setCompletionHandler:^{
        [sidebarView setFrame:[superView bounds]];
    }];
    [[scannerView animator] setFrameOrigin:NSMakePoint(0, -NSHeight([scannerView frame]))];
    [NSAnimationContext endGrouping];
}
@end
