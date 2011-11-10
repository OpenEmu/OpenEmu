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
@implementation OEPrefCoresController
#define rando(x0, x1)  ((int)(x0 + (x1 - x0) * rand() / ((float) RAND_MAX)))
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    
    return self;
}

- (void)dealloc
{
    [self setCoresTableView:nil];
    
    [super dealloc];
}

#pragma mark -
@synthesize coresTableView;
#pragma mark - 
#pragma mark NSTableViewDatasource Implementaion
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    NSArray* corePlugins = [OECorePlugin allPlugins];
    return [corePlugins count];
}
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSArray* corePlugins = [OECorePlugin allPlugins];
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
        if(row == __debug_testing__ProgressIndex)
        {
            return [NSNumber numberWithFloat:__debug_testing__progress];
        }
        
        
        return [plugin version];
    }
    
    return nil;
}
#pragma mark -
#pragma mark NSTableViewDelegate Implementation
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if([aCell isKindOfClass:[NSTextFieldCell class]])
    {
        NSDictionary* attr;
        
        int weight = [[aTableColumn identifier] isEqualToString:@"coreColumn"]?10:0;
        
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
    
    if(row == __debug_testing__ProgressIndex)
    {
        OECoreTableProgressCell* cell = [[OECoreTableProgressCell alloc] init];
        return [cell autorelease];
    }
    
    NSString* title = nil;
    if(row == __debug_testing__InstallIndex)
    {
        title = @"Install";
    }
    else if(row == __debug_testing__UpdateIndex)
    {
        title = @"Update";
    }
    
    if(!title) return [tableColumn dataCellForRow:row];
    
    OECoreTableButtonCell* buttonCell = [[OECoreTableButtonCell alloc] initTextCell:title];
    [buttonCell setAction:@selector(updateOrInstallAction:)];
    [buttonCell setTarget:self];
    
    return [buttonCell autorelease];
}

#pragma mark -
- (id)updateOrInstallAction:(id)sender
{
    __debug_testing__progress += 0.05;

    [[self coresTableView] reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:__debug_testing__ProgressIndex] columnIndexes:[NSIndexSet indexSetWithIndex:[[[self coresTableView] tableColumns] count] -1]];
}
#pragma mark -
#pragma mark ViewController Overrides
- (void)awakeFromNib
{    
    NSArray* corePlugins = [OECorePlugin allPlugins];
    
    __debug_testing__InstallIndex = rand()%(int)[corePlugins count];
    __debug_testing__UpdateIndex = rand()%(int)[corePlugins count];
    __debug_testing__ProgressIndex = rand()%(int)[corePlugins count];
    __debug_testing__progress = 0.0;
    
    [[[self coresTableView] tableColumns] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        OECenteredTextFieldCell* cell = [obj dataCell];
        [cell setWidthInset:8.0];
    }];
}

- (NSString*)nibName
{
	return @"OEPrefCoresController";
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
	return NSMakeSize(423, 480);
}

@end
