/*
 Copyright (c) 2011, OpenEmu Team
 
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
#import "OEPrefCoresController.h"

#import "OETableView.h"
#import "OECoreTableButtonCell.h"
#import "OECoreTableProgressCell.h"
#import "OECenteredTextFieldCell.h"

#import "OECoreUpdater.h"
#import "OECoreDownload.h"

#import "OpenEmu-Swift.h"

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
    [[OECoreUpdater sharedUpdater] addObserver:self
                                    forKeyPath:@"coreList"
                                       options:NSKeyValueChangeInsertion | NSKeyValueChangeRemoval | NSKeyValueChangeReplacement
                                       context:_OEPrefCoresCoreListContext];
    
    for (id obj in self.coresTableView.tableColumns) {
        OECenteredTextFieldCell *cell = [obj dataCell];
        cell.widthInset = 8.0;
    }
    
    self.coresTableView.delegate = self;
    self.coresTableView.dataSource = self;
    //((OETableView *)self.coresTableView).headerClickable = NO;
    
    [[OECoreUpdater sharedUpdater] checkForNewCores:@(NO)];
    [[OECoreUpdater sharedUpdater] checkForUpdates];
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

#pragma mark - NSTableViewDatasource Implementaion

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
        return [[plugin systemNames] componentsJoinedByString:@", "];
    }
    else if([columnIdentifier isEqualToString:@"versionColumn"])
    {
        if([plugin isDownloading])
        {
            return @([plugin progress]);
        }
        
        return [plugin canBeInstalled] || [plugin hasUpdate] ? @"" : [plugin version];
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
        
        NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
        [style setLineBreakMode:NSLineBreakByTruncatingTail];
        
        attr = @{
                 NSFontAttributeName : [NSFont systemFontOfSize:11 weight:weight],
                 NSForegroundColorAttributeName : color,
                 NSParagraphStyleAttributeName : style,
                 };
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
    
    if([plugin appcastItem] == nil || title == nil) return [tableColumn dataCellForRow:row];
    
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
    if([columnIdentifier isEqualToString:@"versionColumn"] && [anObject boolValue])
        [self OE_updateOrInstallItemAtRow:rowIndex];
}

- (NSString*)tableView:(NSTableView *)tableView toolTipForCell:(NSCell *)cell rect:(NSRectPointer)rect tableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row mouseLocation:(NSPoint)mouseLocation
{
    const NSString *columnIdentifier = [tableColumn identifier];
    const OECoreDownload *plugin     = [self OE_coreDownloadAtRow:row];

    if([columnIdentifier isEqualToString:@"systemColumn"])
    {
        return [[plugin systemNames] componentsJoinedByString:@", "];
    }

    if([cell isKindOfClass:[NSTextFieldCell class]])
        return [cell stringValue];
    
    return @"";
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

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], @"Preferences: Cores Toolbar item");
}

- (NSSize)viewSize
{
    return NSMakeSize(423, 460);
}

@end
