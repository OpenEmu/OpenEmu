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
#import "OETheme.h"

#import "NSString+OEAdditions.h"
#import "NSURL+OELibraryAdditions.h"

NSString * const OEBiosUserGuideURLString = @"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-BIOS-files";

@interface OEPrefBiosController () <NSTextViewDelegate>
@property (strong) NSArray *items;
@end

@implementation OEPrefBiosController
@synthesize tableView;

#pragma mark - ViewController Overrides
- (void)awakeFromNib
{
    NSTableView *view = [self tableView];

    [view setDelegate:self];
    [view setDataSource:self];
    [view setUsesAlternatingRowBackgroundColors:NO];
    [view setBackgroundColor:[NSColor clearColor]];
    [view setFloatsGroupRows:YES];

    [view registerForDraggedTypes:@[NSURLPboardType]];

    [self reloadData];
}

- (NSString *)nibName
{
    return @"OEPrefBiosController";
}

#pragma mark - Private Methods
- (void)reloadData
{
    NSArray        *cores = [OECorePlugin allPlugins];
    NSMutableArray *items = [NSMutableArray array];

    [cores enumerateObjectsUsingBlock:^(OECorePlugin *core, NSUInteger idx, BOOL *stop)
    {
        NSArray *requiredFiles = [core requiredFiles];
        if([requiredFiles count])
        {
            [items addObject:core];
            [items addObjectsFromArray:requiredFiles];
        }
    }];

    [self setItems:items];
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

    if(TRUE) // perform longer check (also tries to import…)
        return [OEImportOperation isBiosFileAtURL:url] && [url checkResourceIsReachableAndReturnError:nil];
    else
        return [url checkResourceIsReachableAndReturnError:nil];
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
    if(row == 0)
    {
        NSTableCellView *groupCell = [view makeViewWithIdentifier:@"InfoCell" owner:self];

        NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:OELocalizedString(@"In order to emulate some systems, BIOS files are needed due to increasing complexity of the hardware and software of modern gaming consoles. Please read our User guide on BIOS files for more information.", @"BIOS files preferences introduction text")];
        NSString *linkText = OELocalizedString(@"User guide on BIOS files", @"Bios files introduction text, active region");

        NSDictionary *attributes = [[OETheme sharedTheme] textAttributesForKey:@"bios_info" forState:OEThemeStateDefault];
        [string setAttributes:attributes range:[[string string] fullRange]];

        NSMutableDictionary *linkAttributes = [[[OETheme sharedTheme] textAttributesForKey:@"bios_info_link" forState:OEThemeStateDefault] mutableCopy];
        [linkAttributes setObject:[NSURL URLWithString:OEBiosUserGuideURLString] forKey:NSLinkAttributeName];

        NSRange linkRange = [[string string] rangeOfString:linkText];
        [string setAttributes:linkAttributes range:linkRange];

        [groupCell setObjectValue:string];

        NSTextView *textView = [[[groupCell subviews] lastObject] documentView];
        [textView setDelegate:self];
        [textView setEditable:NO];
        [textView setSelectable:YES];

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
        NSTableCellView *fileCell = [tableView makeViewWithIdentifier:@"FileCell" owner:self];

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

        return fileCell;
    }
    return nil;
}

- (CGFloat)tableView:(NSTableView *)view heightOfRow:(NSInteger)row
{
    if(row == 0)
    {
        return 90.0;
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

- (NSDragOperation)tableView:(NSTableView *)table validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation
{
    [table setDropRow:-1 dropOperation:NSTableViewDropOn];
    return NSDragOperationCopy;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation
{
    NSArray *files = [[info draggingPasteboard] readObjectsForClasses:@[[NSURL class]] options:nil];

    __block BOOL importedSomething = NO;
    [files enumerateObjectsUsingBlock:^(NSURL *url, NSUInteger idx, BOOL *stop) {
        if([url isDirectory])
        {

        }
        else if([url isFileURL])
        {
            importedSomething |= [OEImportOperation isBiosFileAtURL:url];
        }
    }];

    if(importedSomething)
        dispatch_async(dispatch_get_main_queue(), ^{
            [self reloadData];
        });
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
    return OELocalizedString([self title], @"Preferences: Bios Toolbar item");
}

- (NSSize)viewSize
{
    return NSMakeSize(423, 460);
}

@end
