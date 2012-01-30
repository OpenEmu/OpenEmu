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
@interface OEPrefCoresController (Private)
- (void)updateOrInstallItemAtRow:(NSInteger)rowIndex;
- (OECoreDownload*)coreDownloadAtRow:(NSInteger)row;
@end
@implementation OEPrefCoresController
@synthesize coresTableView;
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    
    return self;
}

- (void)dealloc
{
    [[OECoreUpdater sharedUpdater] removeObserver:self forKeyPath:@"coreList"];
    
    [self setCoresTableView:nil];
    [super dealloc];
}
#pragma mark -
#pragma mark ViewController Overrides
- (void)awakeFromNib
{        
    [[OECoreUpdater sharedUpdater] addObserver:self forKeyPath:@"coreList" options:NSKeyValueChangeInsertion|NSKeyValueChangeRemoval|NSKeyValueChangeReplacement context:nil];
    
    [[[self coresTableView] tableColumns] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        OECenteredTextFieldCell* cell = [obj dataCell];
        [cell setWidthInset:8.0];
    }];
    
    [[self coresTableView] setDelegate:self];
    [[self coresTableView] setDataSource:self];
    [(OETableView*)[self coresTableView] setHeaderClickable:NO];
    
    [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForNewCores:) withObject:[NSNumber numberWithBool:NO]];
    [[OECoreUpdater sharedUpdater] performSelectorInBackground:@selector(checkForUpdates) withObject:nil];
}

- (NSString*)nibName
{
    return @"OEPrefCoresController";
}

#pragma mark -
#pragma mark Private Methods
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:@"coreList"])
    {
        [[self coresTableView] reloadData];
    }
}

- (void)updateOrInstallItemAtRow:(NSInteger)rowIndex
{
    OECoreDownload* plugin = [self coreDownloadAtRow:rowIndex];
    [plugin startDownload:self];
}

- (OECoreDownload*)coreDownloadAtRow:(NSInteger)row
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
    OECoreDownload* plugin = [self coreDownloadAtRow:row];
    
    NSString* columnIdentifier = [tableColumn identifier];
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
        NSDictionary* attr;
        
        int weight = [[aTableColumn identifier] isEqualToString:@"coreColumn"]?15:0;
        NSColor* color = [[self coreDownloadAtRow:rowIndex] canBeInstalled]?[NSColor darkGrayColor]:[NSColor whiteColor];
        
        attr = [NSDictionary dictionaryWithObjectsAndKeys:
                [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:weight size:11.0], NSFontAttributeName, 
                color, NSForegroundColorAttributeName, nil];
        [aCell setAttributedStringValue:[[[NSAttributedString alloc] initWithString:[aCell stringValue] attributes:attr] autorelease]];
    }
}

- (NSCell*)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if(!tableColumn) return nil;
    
    if([[tableColumn identifier] isNotEqualTo:@"versionColumn"])
        return [tableColumn dataCellForRow:row];
    
    OECoreDownload* plugin = [self tableView:tableView objectValueForTableColumn:nil row:row];
    if([plugin isDownloading])
    {
        OECoreTableProgressCell* cell = [[OECoreTableProgressCell alloc] init];
        return [cell autorelease];
    }
    
    NSString* title = nil;
    if([plugin canBeInstalled])
    {
        title = NSLocalizedString(@"Install", @"Install Core");
    }
    else if([plugin hasUpdate])
    {
        title = NSLocalizedString(@"Update", @"Update Core");
    }
    
    if(![plugin appcastItem] || !title) return [tableColumn dataCellForRow:row];
    
    OECoreTableButtonCell* buttonCell = [[OECoreTableButtonCell alloc] initTextCell:title];
    return [buttonCell autorelease];
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
    NSString* columnIdentifier = [aTableColumn identifier];
    if([columnIdentifier isEqualTo:@"versionColumn"])
        [self updateOrInstallItemAtRow:rowIndex];
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon
{
    return [NSImage imageNamed:@"cores_tab_icon"];
}

- (NSString*)title
{
    return @"Cores";
}

- (NSSize)viewSize
{
    return NSMakeSize(423, 474);
}

@end
