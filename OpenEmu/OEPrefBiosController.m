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

#import "NSString+OEAdditions.h"
#import "NSURL+OELibraryAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"

#import "OEPreferencesController.h"

NSString * const OEPreferencesAlwaysShowBiosKey = @"OEPreferencesAlwaysShowBios";
NSString * const OEBiosUserGuideURLString = @"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-BIOS-files";
@interface OEPrefBiosController () <NSTextViewDelegate>
@property (strong) NSArray *items;
@property (nonatomic, getter=isVisible) BOOL visisble;
@end

@implementation OEPrefBiosController
@synthesize tableView, visisble=_visisble;

static void *const _OEPrefBiosCoreListContext = (void *)&_OEPrefBiosCoreListContext;

#pragma mark - ViewController Overrides
- (id)init
{
    self = [super init];
    if(self)
    {
        [OECorePlugin addObserver:self forKeyPath:@"allPlugins" options:0 context:_OEPrefBiosCoreListContext];
        [self reloadData];
    }
    return self;
}

- (void)awakeFromNib
{
    NSTableView *view = [self tableView];

    [view setDelegate:self];
    [view setDataSource:self];
    [view setUsesAlternatingRowBackgroundColors:NO];
    [view setFloatsGroupRows:YES];

    [view registerForDraggedTypes:@[NSURLPboardType]];

    [self reloadData];
}

- (NSString *)nibName
{
    return @"OEPrefBiosController";
}

- (void)setVisisble:(BOOL)visisble
{
    if(visisble != _visisble)
    {
        _visisble = visisble;
        [[NSNotificationCenter defaultCenter] postNotificationName:OEPreferencePaneDidChangeVisibilityNotificationName object:self];
    }
}

- (BOOL)isVisible
{
    return _visisble || [[NSUserDefaults standardUserDefaults] boolForKey:OEPreferencesAlwaysShowBiosKey];
}

- (void)dealloc
{
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

    [cores enumerateObjectsUsingBlock:^(OECorePlugin *core, NSUInteger idx, BOOL *stop)
    {
        NSArray *requiredFiles = [[core requiredFiles] sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"Description" ascending:YES]]];
        if([requiredFiles count])
        {
            [items addObject:core];
            [items addObjectsFromArray:requiredFiles];
        }
    }];

    [self setItems:items];
    [self setVisisble:[items count]!=0];

    [[self tableView] reloadData];
}

- (NSString*)OE_formatByteNumber:(NSNumber*)number
{
    NSUInteger size = [number integerValue];

    NSArray *units = @[ @"Bytes", @"KB", @"MB", @"GB" ];
    NSUInteger unitIdx = 0;
    while(size > 1000 && unitIdx < [units count]-1)
    {
        unitIdx ++;
        size /= 1000;
    }

    return [NSString stringWithFormat:@"%ld %@", size, [units objectAtIndex:unitIdx]];
}

- (BOOL)isBIOSFileAvailable:(NSDictionary*)file
{
    NSString *fileName = [file objectForKey:@"Name"];

    NSString      *biosPath = [NSString pathWithComponents:@[[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject], @"OpenEmu", @"BIOS"]];
    NSString      *destination = [biosPath stringByAppendingPathComponent:fileName];
    NSURL *url = [NSURL fileURLWithPath:destination];
    return [url checkResourceIsReachableAndReturnError:nil];
}

- (BOOL)importBIOSFile:(NSURL*)url
{
    BOOL success = NO;

    if([url checkResourceIsReachableAndReturnError:nil])
    {
        NSString *md5 = nil;
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSString      *biosPath = [NSString pathWithComponents:@[[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject], @"OpenEmu", @"BIOS"]];

        NSError  *error              = nil;
        NSDictionary *dictionary = [url resourceValuesForKeys:@[NSURLFileSizeKey] error:&error];
        if(!dictionary)
        {
            DLog(@"Something is wrong with this file, could not read its size");
            return NO;
        }

        NSUInteger fileSize = [[dictionary objectForKey:NSURLFileSizeKey] unsignedIntegerValue];

        // Copy known BIOS / System Files to BIOS folder
        for(id validFile in [OECorePlugin requiredFiles])
        {
            NSString  *biosSystemFileName = [validFile valueForKey:@"Name"];
            NSString  *biosSystemFileMD5  = [validFile valueForKey:@"MD5"];
            NSUInteger biosSystemFileSize = (NSUInteger)[[validFile valueForKey:@"Size"] integerValue];

            NSString *destination = [biosPath stringByAppendingPathComponent:biosSystemFileName];
            NSURL    *desitionationURL = [NSURL fileURLWithPath:destination];
            if([desitionationURL checkResourceIsReachableAndReturnError:nil])
            {
                success = YES; // already imported
                continue;
            }

            // compare file size
            if(fileSize != biosSystemFileSize)
            {
                continue;
            }

            // compare md5 hash
            if(!md5 && ![fileManager hashFileAtURL:url md5:&md5 crc32:nil error:&error])
            {
                DLog(@"Could not hash bios file at %@", url);
                DLog(@"%@", error);
                return NO;
            }

            if(![md5 caseInsensitiveCompare:biosSystemFileMD5] == NSOrderedSame)
            {
                continue;
            }

            if(![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:biosPath] withIntermediateDirectories:YES attributes:nil error:&error])
            {
                DLog(@"Could not create directory before copying bios at %@", url);
                NSLog(@"%@", error);
                error = nil;
                continue;
            }

            if(![fileManager copyItemAtURL:url toURL:[NSURL fileURLWithPath:destination] error:&error])
            {
                DLog(@"Could not copy bios file %@ to %@", url, destination);
                DLog(@"%@", error);
                continue;
            }

            success = YES;
        }
    }
    return success;
}

#pragma mark - Table View
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[self items] count]+1;
}

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    if(row == 0)
        return NO;

    id item = [[self items] objectAtIndex:row-1];
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

        NSString *infoText = [NSString stringWithFormat:NSLocalizedString(@"In order to emulate some systems, BIOS files are needed due to increasing complexity of the hardware and software of modern gaming consoles. Please read our %@ for more information.", @"BIOS files preferences introduction text"), OEBiosUserGuideURLString];
        NSString *linkText = NSLocalizedString(@"User guide on BIOS files", @"Bios files introduction text, active region");
        NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:infoText attributes:attributes];

        NSRange linkRange = [infoText rangeOfString:OEBiosUserGuideURLString];
        [attributedString setAttributes:@{NSLinkAttributeName:[NSURL URLWithString:OEBiosUserGuideURLString]} range:linkRange];
        [attributedString replaceCharactersInRange:linkRange withString:linkText];

        [groupCell setObjectValue:attributedString];

        NSTextView *textView = [[[groupCell subviews] lastObject] documentView];
        [textView setDelegate:self];
        [textView setEditable:NO];
        [textView setSelectable:YES];
        [textView setDrawsBackground:YES];
        [textView setTypingAttributes:attributes];
        [textView setSelectedTextAttributes:attributes];
        [textView setLinkTextAttributes:linkAttributes];
        [textView setBackgroundColor:rowBackground];
        [groupCell setBackgroundColor:rowBackground];

        return groupCell;
    } else row = row -1;

    id item = [[self items] objectAtIndex:row];
    if([self tableView:view isGroupRow:row+1])
    {
        OECorePlugin *core = (OECorePlugin*)item;
        NSTableCellView *groupCell = [view makeViewWithIdentifier:@"CoreCell" owner:self];
        [[groupCell textField] setStringValue:[core name]];
        return groupCell;
    }
    else
    {
        OETableCellView *fileCell = [tableView makeViewWithIdentifier:@"FileCell" owner:self];

        NSTextField *descriptionField = [fileCell textField];
        NSTextField *fileNameField = [fileCell viewWithTag:1];
        NSImageView *availabilityIndicator = [fileCell viewWithTag:3];

        NSString *description = [item objectForKey:@"Description"];
        __unused NSString *md5  = [item objectForKey:@"MD5"];
        NSString *name = [item objectForKey:@"Name"];
        NSNumber *size = [item objectForKey:@"Size"];

        BOOL available = [self isBIOSFileAvailable:item];
        NSString *imageName = available ? @"bios_found" : @"bios_missing";
        NSImage  *image     = [NSImage imageNamed:imageName];

        [descriptionField setStringValue:description];
        [fileNameField setStringValue:[NSString stringWithFormat:@"%@ (%@)", name, [self OE_formatByteNumber:size]]];
        [availabilityIndicator setImage:image];

        int rowsFromHeader = 0;
        while(![self tableView:view isGroupRow:row+1-rowsFromHeader++])
        [fileCell setBackgroundColor:rowsFromHeader%2 ? rowBackground : alternateRowBackground];

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
- (NSDragOperation)tableView:(NSTableView *)table validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation
{
    [table setDropRow:-1 dropOperation:NSTableViewDropOn];
    return NSDragOperationCopy;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation
{
    NSArray *files = [[info draggingPasteboard] readObjectsForClasses:@[[NSURL class]] options:nil];

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
    return NSLocalizedString([self title], @"Preferences: Bios Toolbar item");
}

- (NSSize)viewSize
{
    return NSMakeSize(423, 460);
}

@end
