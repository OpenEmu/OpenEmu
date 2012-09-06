//
//  OEImportViewController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 29.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEImportViewController.h"

#import "OELibraryDatabase.h"
#import "OEBackgroundColorView.h"

#import "NSViewController+OEAdditions.h"
#import "OEImportItem.h"
#import "OECoreTableButtonCell.h"

#import "OEMenu.h"
#import "OEDBSystem.h"
@interface OEImportViewController ()
@end
@implementation OEImportViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {}
    return self;
}

- (NSString*)nibName
{
    return @"ImportView";
}

- (void)loadView
{
    [super loadView];
    
    [(OEBackgroundColorView*)[self view] setBackgroundColor:[NSColor lightGrayColor]];
    [[self progressIndicator] setIndeterminate:NO];
    [[self progressIndicator] startAnimation:self];
    
    [[self tableView] setDelegate:self];
}
@synthesize progressIndicator, statusField, tableView;
#pragma mark -
- (void)viewWillAppear
{
    [[self importer] setDelegate:self];
    
    [[self tableView] setDataSource:self];
    [[self tableView] reloadData];
}

- (void)viewDidDisappear
{
    [[self tableView] setDataSource:nil];
    [[self tableView] reloadData];
    
    [[self importer] setDelegate:nil];
    
    [[self importer] removeFinished];
}

- (OEROMImporter*)importer
{
    return [[OELibraryDatabase defaultDatabase] importer];
}
#pragma mark - OELibrarySubviewController Protocol Implementation
- (void)setItem:(id)item
{}

- (id)selectedItem
{
    return [self importer];
}

- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{}

#pragma mark - OEROMImporter Delegate
- (void)romImporterChangedItemCount:(OEROMImporter*)importer
{
    [[self tableView] reloadData];
}

- (void)romImporter:(OEROMImporter*)importer startedProcessingItem:(OEImportItem*)item
{
    NSUInteger idx = [[[self importer] queue] indexOfObjectIdenticalTo:item];
    if(idx != NSNotFound)
    {
        NSIndexSet *columnIndexes = [[self tableView] columnIndexesInRect:[[self tableView] visibleRect]];
        [[self tableView] reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:idx] columnIndexes:columnIndexes];
    }
}
- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportItem*)item
{
    NSUInteger idx = [[[self importer] queue] indexOfObjectIdenticalTo:item];
    if(idx != NSNotFound)
    {
        NSIndexSet *columnIndexes = [[self tableView] columnIndexesInRect:[[self tableView] visibleRect]];
        [[self tableView] reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:idx] columnIndexes:columnIndexes];
    }
}
- (void)romImporter:(OEROMImporter*)importer finishedProcessingItem:(OEImportItem*)item
{
    NSUInteger idx = [[[self importer] queue] indexOfObjectIdenticalTo:item];
    if(idx != NSNotFound)
    {
        NSIndexSet *columnIndexes = [[self tableView] columnIndexesInRect:[[self tableView] visibleRect]];
        [[self tableView] reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:idx] columnIndexes:columnIndexes];
    }
}

#pragma mark - UI Methods
- (IBAction)togglePause:(id)sender
{
    [[self importer] pause];
}

- (IBAction)cancel:(id)sender
{
    [[self importer] cancel];
}

- (void)resolveMultipleSystemsError:(NSMenuItem*)menuItem
{
    OEImportItem *importItem = [[[self importer] queue] objectAtIndex:[[self tableView] selectedRow]];
    [[importItem importInfo] setValue:[NSArray arrayWithObject:[menuItem representedObject]] forKey:OEImportInfoSystemID];
    [importItem setError:nil];
    [importItem setImportStep:OEImportStepSyncArchive];
    [importItem setImportState:OEImportItemStatusIdle];
    
    // TODO: TableView reload data for item at selected row
    
    [[self importer] startQueueIfNeeded];
}
#pragma mark - TableView Datasource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[self importer] numberOfItems];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *identifier = [tableColumn identifier];
    if([identifier isEqualToString:@"icon"])
        return nil;
    else if([identifier isEqualToString:@"path"])
        return [[[[[self importer] queue] objectAtIndex:row] URL] path];
    else if([identifier isEqualToString:@"status"])
        return [[[[self importer] queue] objectAtIndex:row] localizedStatusMessage];
    
    return nil;
}

#pragma mark - Table View Delegate
- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *identifier = [tableColumn identifier];
    if([identifier isEqualToString:@"status"])
    {
        OEImportItem *item = [[[self importer] queue] objectAtIndex:row];
        if([[[item error] domain] isEqualToString:OEImportErrorDomainResolvable] && [[item error] code] == OEImportErrorCodeMultipleSystems)
        {
            return [[OECoreTableButtonCell alloc] initTextCell:@"Pick a system!"];
        }
    }
    return [tableColumn dataCellForRow:row];
}

- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
        if([aCell isKindOfClass:[NSTextFieldCell class]])
        {
            NSDictionary *attr;
            
            if([[aTableView selectedRowIndexes] containsIndex:rowIndex])
            {
                attr = [NSDictionary dictionaryWithObjectsAndKeys:
                        [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:9 size:11.0], NSFontAttributeName,
                        [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName, nil];
            } else {
                attr = [NSDictionary dictionaryWithObjectsAndKeys:
                        [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:7 size:11.0], NSFontAttributeName,
                        [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName, nil];
            }
            
                [aCell setAttributedStringValue:[[NSAttributedString alloc] initWithString:[aCell stringValue] attributes:attr]];
        }    
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    NSString *columnIdentifier = [aTableColumn identifier];
    if([columnIdentifier isEqualToString:@"status"] && [anObject boolValue])
    {
        OEImportItem *item = [[[self importer] queue] objectAtIndex:rowIndex];
        NSMenu     *menu = [[NSMenu alloc] init];
        NSArray *systemIds = [[item importInfo] valueForKey:OEImportInfoSystemID];
        [systemIds enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:obj inDatabase:[[self importer] database]];
            NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:[system name] action:@selector(resolveMultipleSystemsError:) keyEquivalent:@""];
            [menuItem setImage:[system icon]];
            [menuItem setTarget:self];
            [menuItem setRepresentedObject:obj];
            [menu addItem:menuItem];
        }];
        OEMenu *oemenu = [menu convertToOEMenu];
        NSRect rowRect = [[self tableView] rectOfRow:rowIndex];
        NSRect columnRect = [[self tableView] rectOfColumn:2];
        NSRect buttonRect = NSIntersectionRect(rowRect, columnRect);
        NSRect rectOnWindow = [[self tableView] convertRect:buttonRect toView:nil];
        [oemenu openOnEdge:OENoEdge ofRect:rectOnWindow ofWindow:[[self view] window]];
    }
}
#pragma mark -
- (NSImage*)sidebarIcon
{
    return nil;
}

- (NSString*)sidebarName
{
    return NSLocalizedString(@"Importing", "");
}
- (void)setSidebarName:(NSString*)newName
{
    return;
}
- (BOOL)isSelectableInSdebar
{
    return YES;
}
- (BOOL)isEditableInSdebar
{
    return NO;
}
- (BOOL)isGroupHeaderInSdebar
{
    return YES;
}

- (BOOL)hasSubCollections
{
    return NO;
}
@end
