//
//  OEPrefCoresController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefCoresController.h"
#import "OECorePlugin.h"

#import "OETableView.h"
#import "OECenteredTextFieldCell.h"
#import "OECoreTableButtonCell.h"
#import "OECoreTableProgressCell.h"
@interface OEPrefCoresController (Private)
- (void)rearrangePlugins;
- (void)updateOrInstallItemAtRow:(NSInteger)rowIndex;
@end
@implementation OEPrefCoresController
@synthesize coresTableView;
@synthesize arrangedPlugins;
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    
    return self;
}

- (void)dealloc
{
    [[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins"];
    [self removeObserver:self forKeyPath:@"arrangedPlugins"];
    
    [self setCoresTableView:nil];
    [super dealloc];
}
#pragma mark -
#pragma mark ViewController Overrides
- (void)awakeFromNib
{    
    [self rearrangePlugins];
    
    [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:NSKeyValueChangeInsertion|NSKeyValueChangeRemoval|NSKeyValueChangeReplacement context:nil];
    [self addObserver:self forKeyPath:@"arrangedPlugins" options:NSKeyValueChangeSetting context:nil];

    [[[self coresTableView] tableColumns] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        OECenteredTextFieldCell* cell = [obj dataCell];
        [cell setWidthInset:8.0];
    }];
    
    [[self coresTableView] setDelegate:self];
    [[self coresTableView] setDataSource:self];
}

- (NSString*)nibName
{
	return @"OEPrefCoresController";
}

#pragma mark -
#pragma mark Private Methods
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:@"arrangedPlugins"])
    {
        NSLog(@"reloading data");
        [[self coresTableView] reloadData];
    }
    else if([keyPath isEqualToString:@"allPlugins"])
    {
        [self rearrangePlugins];
    }
}

- (void)rearrangePlugins
{
    NSArray* installedCorePlugins = [OECorePlugin allPlugins];
    NSArray* availableCorePlugins = [NSArray array];
    NSMutableArray* _arrangedPlugins = [[NSMutableArray alloc] initWithCapacity:[installedCorePlugins count]+[availableCorePlugins count]];
    [_arrangedPlugins addObjectsFromArray:installedCorePlugins];
    for(id aPlugin in availableCorePlugins)
    {
        BOOL alreadyIncluded = NO;
        if(!alreadyIncluded)
        {
            [_arrangedPlugins addObject:aPlugin];
        }
    }

    [_arrangedPlugins sortUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        return [[obj1 valueForKey:@"displayName"] compare:[obj2 valueForKey:@"displayName"] options:NSCaseInsensitiveSearch];
    }];
    
    [self setArrangedPlugins:_arrangedPlugins];
    [_arrangedPlugins release];
}
- (void)updateOrInstallItemAtRow:(NSInteger)rowIndex
{
    id plugin = [self tableView:[self coresTableView] objectValueForTableColumn:nil row:rowIndex];
    if([plugin isKindOfClass:[OECorePlugin class]])
    {
        // We have a real plugin --> launch update
        OECorePlugin* realPlugin = (OECorePlugin*)plugin;
    } 
    else
    {
        // Plugin is only a dummy -> install real plugin
        NSDictionary* pluginDummy = (NSDictionary*)plugin;
    }
}
#pragma mark - 
#pragma mark NSTableViewDatasource Implementaion
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    NSArray* corePlugins = [self arrangedPlugins];
    return [corePlugins count];
}
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSArray* corePlugins = [self arrangedPlugins];
    OECorePlugin* plugin = [corePlugins objectAtIndex:row];
    
    NSString* columnIdentifier = [tableColumn identifier];
    if([columnIdentifier isEqualToString:@"coreColumn"])
    {
        return [plugin displayName];
    } 
    else if([columnIdentifier isEqualToString:@"systemColumn"])
    {
        return nil;
    }
    else if([columnIdentifier isEqualToString:@"versionColumn"])
    {
        BOOL hasProgress = NO;
        if(hasProgress)
        {
            float progress = 0.0;
            return [NSNumber numberWithFloat:progress];
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
        
        if([[aTableView selectedRowIndexes] containsIndex:rowIndex])
        {
            attr = [NSDictionary dictionaryWithObjectsAndKeys:
                    [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:weight+5 size:11.0], NSFontAttributeName, 
                    [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName, nil];
        } else {
            attr = [NSDictionary dictionaryWithObjectsAndKeys:
                    [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:weight size:11.0], NSFontAttributeName, 
                    [NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName, nil];
        }
        [aCell setAttributedStringValue:[[[NSAttributedString alloc] initWithString:[aCell stringValue] attributes:attr] autorelease]];
    }
}

- (NSCell*)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if(!tableColumn) return nil;
    
    if([[tableColumn identifier] isNotEqualTo:@"versionColumn"])
        return [tableColumn dataCellForRow:row];
    
    BOOL progressAvailable = NO;  // progressAvailable(row)
    if(progressAvailable)
    {
        OECoreTableProgressCell* cell = [[OECoreTableProgressCell alloc] init];
        return [cell autorelease];
    }
    BOOL installable = NO;  // installable(row)
    BOOL updateAvailable = NO; // updateAvailable(row)
    NSString* title = nil;
    if(installable)
    {
        title = NSLocalizedString(@"Install", @"Install Core");
    }
    else if(updateAvailable)
    {
        title = NSLocalizedString(@"Update", @"Update Core");
    }
    
    if(!title) return [tableColumn dataCellForRow:row];
    
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
    if([[aTableView identifier] isEqualToString:@"versionColumn"])
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
