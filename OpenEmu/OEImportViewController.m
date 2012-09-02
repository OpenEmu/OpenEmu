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
    [[self progressIndicator] setIndeterminate:YES];
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
- (void)romImporter:(OEROMImporter*)importer startedProcessingItem:(OEImportItem*)item
{
    DLog(@"");
}
- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportItem*)item
{
    DLog(@"");
}
- (void)romImporter:(OEROMImporter*)importer finishedProcessingItem:(OEImportItem*)item
{
    DLog(@"");
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
    else if([identifier isEqualToString:@"status"]) return nil;
    
    return nil;
}
#pragma mark - Table View Delegate
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
