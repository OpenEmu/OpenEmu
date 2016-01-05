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
#import "OECorePlugin.h"

#import "OETableView.h"
#import "OETableCellView.h"
#import "OETheme.h"

#import "NSFileManager+OEHashingAdditions.h"
#import "OEBIOSFile.h"

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
    tableView.usesAlternatingRowBackgroundColors = NO;
    tableView.floatsGroupRows = YES;

    [tableView registerForDraggedTypes:@[NSURLPboardType]];

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

- (NSString*)OE_formatByteNumber:(NSNumber*)number
{
    NSUInteger size = number.integerValue;

    NSArray *units = @[ @"Bytes", @"KB", @"MB", @"GB" ];
    NSUInteger unitIdx = 0;
    while(size > 1000 && unitIdx < units.count - 1)
    {
        unitIdx ++;
        size /= 1000;
    }

    return [NSString stringWithFormat:@"%ld %@", size, units[unitIdx]];
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
    NSColor *rowBackground = [NSColor colorWithDeviceWhite:0.059 alpha:1.0];
    NSColor *alternateRowBackground = [NSColor colorWithDeviceWhite:0.114 alpha:1.0];

    if(row == 0)
    {
        OETableCellView *groupCell = [view makeViewWithIdentifier:@"InfoCell" owner:self];

        OETheme *theme = [OETheme sharedTheme];
        NSMutableDictionary *attributes = [[theme textAttributesForKey:@"bios_info" forState:OEThemeStateDefault] mutableCopy];
        NSMutableDictionary *linkAttributes = [[theme textAttributesForKey:@"bios_info_link" forState:OEThemeStateDefault] mutableCopy];

        // Change cursors
        [attributes     setObject:[NSCursor arrowCursor]        forKey:NSCursorAttributeName];
        [linkAttributes setObject:[NSCursor pointingHandCursor] forKey:NSCursorAttributeName];
        [linkAttributes setObject:[NSURL URLWithString:OEBiosUserGuideURLString] forKey:NSLinkAttributeName];
        
        NSString *wildcard = @"\uFFFC";
        NSString *infoText = [NSString stringWithFormat:NSLocalizedString(@"In order to emulate some systems, BIOS files are needed due to increasing complexity of the hardware and software of modern gaming consoles. Please read our %@ for more information.", @"BIOS files preferences introduction text"), wildcard];
        NSString *linkText = NSLocalizedString(@"User guide on BIOS files", @"Bios files introduction text, active region");
        NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:infoText attributes:attributes];

        NSRange linkRange = [infoText rangeOfString:wildcard];
        [attributedString setAttributes:linkAttributes range:linkRange];
        [attributedString replaceCharactersInRange:linkRange withString:linkText];
        
        [groupCell setObjectValue:attributedString];

        NSTextView *textView = [groupCell.subviews.lastObject documentView];
        textView.delegate = self;
        textView.editable = NO;
        textView.selectable = YES;
        textView.drawsBackground = YES;
        textView.typingAttributes = attributes;
        textView.selectedTextAttributes = attributes;
        textView.linkTextAttributes = linkAttributes;
        textView.backgroundColor = rowBackground;
        groupCell.backgroundColor = rowBackground;

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
        OETableCellView *fileCell = [self.tableView makeViewWithIdentifier:@"FileCell" owner:self];

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
        fileNameField.stringValue = [NSString stringWithFormat:@"%@ (%@)", name, [self OE_formatByteNumber:size]];
        availabilityIndicator.image = image;

        int rowsFromHeader = 0;
        while(![self tableView:view isGroupRow:row + 1 - rowsFromHeader++])
        fileCell.backgroundColor = rowsFromHeader % 2 ? rowBackground : alternateRowBackground;

        return fileCell;
    }
    return nil;
}

- (CGFloat)tableView:(NSTableView *)view heightOfRow:(NSInteger)row
{
    if(row == 0)
    {
        // TODO: Localize height of row
        return 80.0;
    }
    else if([self tableView:view isGroupRow:row])
    {
        return 17.0;
    }
    else
    {
        return 56.0;
    }
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

- (NSString *)title
{
    return @"System Files";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString(self.title, @"Preferences: Bios Toolbar item");
}

- (NSSize)viewSize
{
    return NSMakeSize(423, 460);
}

@end
