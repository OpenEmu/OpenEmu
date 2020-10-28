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
#import "OEPrefBiosController.h"

#import "OELibraryDatabase.h"
@import OpenEmuKit;

#import "NSFileManager+OEHashingAdditions.h"

#import "OpenEmu-Swift.h"
#import "OEFSWatcher.h"

NSString * const OEPreferencesAlwaysShowBiosKey = @"OEPreferencesAlwaysShowBios";
NSString * const OEBiosUserGuideURLString = @"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-BIOS-files";

@interface OEPrefBiosController () <NSTextViewDelegate>
@property (strong) NSArray *items;
@property (strong) OEFSWatcher *fileSystemWatcher;
@end

@implementation OEPrefBiosController
@synthesize tableView = _tableView;

static void *const _OEPrefBiosCoreListContext = (void *)&_OEPrefBiosCoreListContext;

#pragma mark - ViewController Overrides

- (instancetype)init
{
    self = [super init];
    if(self)
    {
        [OECorePlugin addObserver:self forKeyPath:@"allPlugins" options:0 context:_OEPrefBiosCoreListContext];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSTableView *tableView = self.tableView;
    
    tableView.delegate = self;
    tableView.dataSource = self;
    tableView.usesAutomaticRowHeights = YES;
    tableView.floatsGroupRows = YES;

    [tableView registerForDraggedTypes:@[NSPasteboardTypeFileURL]];

    NSString *biosPath = [OEBIOSFile biosPath];
    self.fileSystemWatcher = [OEFSWatcher watcherForPath:biosPath withBlock:^(NSString *path, FSEventStreamEventFlags flags) {
        [self reloadData];
    }];
}

- (NSString *)nibName
{
    return @"OEPrefBiosController";
}

- (void)viewWillAppear
{
    [super viewWillAppear];

    [self.fileSystemWatcher startWatching];
    [self reloadData];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [self.fileSystemWatcher stopWatching];
}

- (void)dealloc
{
    if(self.fileSystemWatcher)
        [self.fileSystemWatcher stopWatching];

    [OECorePlugin removeObserver:self forKeyPath:@"allPlugins" context:_OEPrefBiosCoreListContext];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context != _OEPrefBiosCoreListContext)
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    else
        [self reloadData];
}

#pragma mark - Private Methods

- (void)reloadData
{
    NSArray        *cores = [OECorePlugin allPlugins];
    NSMutableArray *items = [NSMutableArray array];

    for (OECorePlugin *core in cores) {
        NSArray *requiredFiles = [core.requiredFiles sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"Description" ascending:YES]]];
        if([requiredFiles count])
        {
            [items addObject:core];
            [items addObjectsFromArray:requiredFiles];
        }
    }

    self.items = items;

    [self.tableView reloadData];
}

- (BOOL)importBIOSFile:(NSURL*)url
{
    OEBIOSFile *biosFile = [[OEBIOSFile alloc] init];
    return [biosFile checkIfBIOSFileAndImportAtURL:url];
}

#pragma mark - Table View

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.items.count + 1;
}

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    if(row == 0)
        return NO;

    id item = self.items[row - 1];
    return [item isKindOfClass:[OECorePlugin class]];
}

- (NSView *)tableView:(NSTableView *)view viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if(row == 0)
    {
        NSTableCellView *groupCell = [view makeViewWithIdentifier:@"InfoCell" owner:self];
        NSTextField *textField = groupCell.textField;
        
        NSMutableParagraphStyle *parStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        parStyle.alignment = NSTextAlignmentJustified;
        NSDictionary *attributes = @{
            NSFontAttributeName: [NSFont systemFontOfSize:NSFont.smallSystemFontSize],
            NSForegroundColorAttributeName: [NSColor labelColor],
            NSParagraphStyleAttributeName: parStyle
        };
        NSDictionary *linkAttributes = @{
            NSFontAttributeName: [NSFont systemFontOfSize:NSFont.smallSystemFontSize],
            NSForegroundColorAttributeName: [NSColor linkColor],
            NSParagraphStyleAttributeName: parStyle,
            NSUnderlineStyleAttributeName: @(NSUnderlineStylePatternSolid),
            NSLinkAttributeName: [NSURL URLWithString:OEBiosUserGuideURLString]
        };
        
        NSString *wildcard = @"\uFFFC";
        NSString *infoText = [NSString stringWithFormat:NSLocalizedString(@"In order to emulate some systems, BIOS files are needed due to increasing complexity of the hardware and software of modern gaming consoles. Please read our %@ for more information.", @"BIOS files preferences introduction text"), wildcard];
        NSString *linkText = NSLocalizedString(@"User guide on BIOS files", @"Bios files introduction text, active region");
        NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:infoText attributes:attributes];

        NSRange linkRange = [infoText rangeOfString:wildcard];
        [attributedString setAttributes:linkAttributes range:linkRange];
        [attributedString replaceCharactersInRange:linkRange withString:linkText];
        
        textField.attributedStringValue = attributedString;

        return groupCell;
    }
    else
        row = row -1;

    id item = self.items[row];
    if([self tableView:view isGroupRow:row + 1])
    {
        OECorePlugin *core = (OECorePlugin *)item;
        NSTableCellView *groupCell = [view makeViewWithIdentifier:@"CoreCell" owner:self];
        groupCell.textField.stringValue = core.name;
        return groupCell;
    }
    else
    {
        NSTableCellView *fileCell = [self.tableView makeViewWithIdentifier:@"FileCell" owner:self];

        NSTextField *descriptionField = fileCell.textField;
        NSTextField *fileNameField = [fileCell viewWithTag:1];
        NSImageView *availabilityIndicator = [fileCell viewWithTag:3];

        NSString *description = item[@"Description"];
        __unused NSString *md5  = item[@"MD5"];
        NSString *name = item[@"Name"];
        NSNumber *size = item[@"Size"];

        OEBIOSFile *biosFile = [[OEBIOSFile alloc] init];
        BOOL available = [biosFile isBIOSFileAvailable:item];
        NSString *imageName = available ? @"bios_found" : @"bios_missing";
        NSImage  *image = [NSImage imageNamed:imageName];

        descriptionField.stringValue = description;
        NSString *sizeString = [NSByteCountFormatter stringFromByteCount:size.longLongValue countStyle:NSByteCountFormatterCountStyleFile];
        fileNameField.stringValue = [NSString stringWithFormat:@"%@ (%@)", name, sizeString];
        availabilityIndicator.image = image;
        
        return fileCell;
    }
    return nil;
}

#pragma mark - NSTextView Delegate
- (BOOL)textView:(NSTextView *)textView clickedOnLink:(id)link atIndex:(NSUInteger)charIndex
{
    [[NSWorkspace sharedWorkspace] openURL:link];
    return YES;
}

- (NSRange)textView:(NSTextView *)aTextView willChangeSelectionFromCharacterRange:(NSRange)oldSelectedCharRange toCharacterRange:(NSRange)newSelectedCharRange
{
    return NSMakeRange(0, 0);
}

#pragma mark - NSTableView Delegate
- (NSDragOperation)tableView:(NSTableView *)table validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation
{
    [table setDropRow:-1 dropOperation:NSTableViewDropOn];
    return NSDragOperationCopy;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation
{
    NSArray *files = [info.draggingPasteboard readObjectsForClasses:@[[NSURL class]] options:nil];

    __block BOOL importedSomething = NO;

    __weak __block void (^recCheckURL)(NSURL *url, NSUInteger idx, BOOL *stop);
    void(^checkURL)(NSURL *, NSUInteger, BOOL*) = ^(NSURL *url, NSUInteger idx, BOOL *stop) {
        if([url isDirectory])
        {
            NSDirectoryEnumerator *direnum = [[NSFileManager defaultManager] enumeratorAtURL:url includingPropertiesForKeys:nil options:NSDirectoryEnumerationSkipsHiddenFiles|NSDirectoryEnumerationSkipsPackageDescendants|NSDirectoryEnumerationSkipsSubdirectoryDescendants errorHandler:NULL];
            NSURL *suburl = nil;
            while(suburl = [direnum nextObject])
            {
                recCheckURL(suburl, idx, stop);
            }
        }
        else if([url isFileURL])
        {
            importedSomething |= [self importBIOSFile:url];
        }
    };
    recCheckURL = checkURL;

    [files enumerateObjectsUsingBlock:checkURL];
    [self reloadData];
    return importedSomething;
}

#pragma mark - OEPreferencePane Protocol

- (NSImage *)icon
{
    return [NSImage imageNamed:@"bios_tab_icon"];
}

- (NSString *)panelTitle
{
    return @"System Files";
}

- (NSString *)localizedPanelTitle
{
    return NSLocalizedString(self.panelTitle, @"Preferences: Bios Toolbar item");
}

- (NSSize)viewSize
{
    return self.view.fittingSize;
}

@end
