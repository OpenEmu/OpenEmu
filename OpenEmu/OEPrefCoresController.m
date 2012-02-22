//
//  OEPrefCoresController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefCoresController.h"

#import "OETableView.h"
#import "OECenteredTextFieldCell.h"
#import "OECoreTableButtonCell.h"
#import "OECoreTableProgressCell.h"

#import "OECoreUpdater.h"
#import "OECoreDownload.h"

@interface OEPrefCoresController ()
- (void)OE_updateOrInstallItemAtRow:(NSInteger)rowIndex;
- (OECoreDownload *)OE_coreDownloadAtRow:(NSInteger)row;
@end

static void *const _OEPrefCoresCoreListContext = (void *)&_OEPrefCoresCoreListContext;

@implementation OEPrefCoresController
@synthesize coresTableView;

- (void)dealloc
{
    [[OECoreUpdater sharedUpdater] removeObserver:self forKeyPath:@"coreList" context:_OEPrefCoresCoreListContext];
}

#pragma mark -
#pragma mark ViewController Overrides

- (void)awakeFromNib
{        
    [[OECoreUpdater sharedUpdater] addObserver:self forKeyPath:@"coreList" options:NSKeyValueChangeInsertion | NSKeyValueChangeRemoval | NSKeyValueChangeReplacement context:_OEPrefCoresCoreListContext];
    
    [[[self coresTableView] tableColumns] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         OECenteredTextFieldCell *cell = [obj dataCell];
         [cell setWidthInset:8.0];
     }];
    
    [[self coresTableView] setDelegate:self];
    [[self coresTableView] setDataSource:self];
    [(OETableView *)[self coresTableView] setHeaderClickable:NO];
    
    [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForNewCores:) withObject:[NSNumber numberWithBool:NO]];
    [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForUpdates) withObject:nil];
}

- (NSString *)nibName
{
    return @"OEPrefCoresController";
}

#pragma mark -
#pragma mark Private Methods

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == _OEPrefCoresCoreListContext)
        [[self coresTableView] reloadData];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

- (void)OE_updateOrInstallItemAtRow:(NSInteger)rowIndex
{
    OECoreDownload *plugin = [self OE_coreDownloadAtRow:rowIndex];
    [plugin startDownload:self];
}

- (OECoreDownload*)OE_coreDownloadAtRow:(NSInteger)row
{
    return [[[OECoreUpdater sharedUpdater] coreList] objectAtIndex:row];
}

#pragma mark - 
#pragma mark NSTableViewDatasource Implementaion

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[[OECoreUpdater sharedUpdater] coreList] count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    OECoreDownload *plugin = [self OE_coreDownloadAtRow:row];
    
    NSString *columnIdentifier = [tableColumn identifier];
    if([columnIdentifier isEqualToString:@"coreColumn"])
    {
        return [plugin name];
    } 
    else if([columnIdentifier isEqualToString:@"systemColumn"])
    {
        return [plugin description];
    }
    else if([columnIdentifier isEqualToString:@"versionColumn"])
    {
        if([plugin isDownloading])
        {
            return [NSNumber numberWithFloat:[plugin progress]];
        }
        
        return [plugin version];
    }
    return plugin;
}

#pragma mark -
#pragma mark NSTableViewDelegate Implementation

- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if([aCell isKindOfClass:[NSTextFieldCell class]])
    {
        NSDictionary *attr;
        
        int weight = 0;
        NSColor *color;
        if([[aTableColumn identifier] isEqualToString:@"coreColumn"])
        {
            weight = 15.0;
            color = [NSColor colorWithDeviceWhite:[[self OE_coreDownloadAtRow:rowIndex] canBeInstalled] ? 0.44 : 0.89 alpha:1.0];
        }
        else
        {
            color = [NSColor colorWithDeviceWhite:[[self OE_coreDownloadAtRow:rowIndex] canBeInstalled] ? 0.44 : 0.86 alpha:1.0];
        }
        
        attr = [NSDictionary dictionaryWithObjectsAndKeys:
                [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:weight size:11.0], NSFontAttributeName, 
                color, NSForegroundColorAttributeName, nil];
        [aCell setAttributedStringValue:[[NSAttributedString alloc] initWithString:[aCell stringValue] attributes:attr]];
    }
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if(tableColumn == nil) return nil;
    
    if([[tableColumn identifier] isNotEqualTo:@"versionColumn"])
        return [tableColumn dataCellForRow:row];
    
    OECoreDownload *plugin = [self tableView:tableView objectValueForTableColumn:nil row:row];
    if([plugin isDownloading])
    {
        OECoreTableProgressCell *cell = [[OECoreTableProgressCell alloc] init];
        return cell;
    }
    
    NSString *title = nil;
    if([plugin canBeInstalled])
    {
        title = NSLocalizedString(@"Install", @"Install Core");
    }
    else if([plugin hasUpdate])
    {
        title = NSLocalizedString(@"Update", @"Update Core");
    }
    
    if([plugin appcastItem] != nil || title != nil) return [tableColumn dataCellForRow:row];
    
    OECoreTableButtonCell *buttonCell = [[OECoreTableButtonCell alloc] initTextCell:title];
    return buttonCell;
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [cell isKindOfClass:[NSButtonCell class]];
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [[self tableView:tableView dataCellForTableColumn:tableColumn row:row] isKindOfClass:[NSButtonCell class]];
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    NSString *columnIdentifier = [aTableColumn identifier];
    if([columnIdentifier isEqualToString:@"versionColumn"])
        [self OE_updateOrInstallItemAtRow:rowIndex];
}

#pragma mark -
#pragma mark OEPreferencePane Protocol

- (NSImage *)icon
{
    return [NSImage imageNamed:@"cores_tab_icon"];
}

- (NSString *)title
{
    return @"Cores";
}

- (NSString*)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
    return NSMakeSize(423, 474);
}

@end
