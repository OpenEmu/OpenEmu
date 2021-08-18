/*
 Copyright (c) 2014, OpenEmu Team
 
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

#import "OEHomebrewViewController.h"

#import "OEDownload.h"
#import "OEHomebrewCoverView.h"
#import "OEDBSystem+CoreDataProperties.h"
#import "OELibraryDatabase.h"

#import "OEDBGame.h"
#import "OEDBRom.h"

#import "NSArray+OEAdditions.h"

#import "OpenEmu-Swift.h"

@import Quartz;

NSString * const OEHomebrewGamesURLString = @"https://raw.githubusercontent.com/OpenEmu/OpenEmu-Update/master/games.xml";

@interface OEHomebrewGame : NSObject
- (instancetype)initWithNode:(NSXMLNode*)node;

@property (readonly, copy) NSString *name;
@property (readonly, copy) NSString *developer;
@property (readonly, copy) NSString *website;
@property (readonly, copy) NSString *fileURLString;
@property (readonly, copy) NSString *gameDescription;
@property (readonly, copy) NSDate   *added;
@property (readonly, copy) NSDate   *released;
@property (readonly)       NSInteger fileIndex;
@property (readonly, copy) NSArray  *images;
@property (readonly, copy) NSString  *md5;
@property (readonly, copy) NSString  *systemGroup;

@property (nonatomic, readonly) NSString *systemShortName;

@property (readonly, copy) NSString *systemIdentifier;
@end
@interface OEHomebrewViewController () <NSTableViewDataSource, NSTableViewDelegate>
@property (readonly, nullable, nonatomic) OELibraryToolbar *toolbar;
@property (strong) NSArray *games;
@property (strong) NSArray *headerIndices;
@property (strong) OEDownload *currentDownload;
@property (strong, nonatomic) OEHomebrewBlankSlateView *blankSlate;
@end

@implementation OEHomebrewViewController
@synthesize blankSlate = _blankSlate;

- (NSString*)nibName
{
    return @"OEHomebrewViewController";
}

- (void)loadView
{
    [super loadView];

    NSView *view = self.view;

    [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    NSTableView *tableView = [self tableView];
    [tableView setAllowsColumnReordering:NO];
    [tableView setAllowsEmptySelection:YES];
    [tableView setAllowsMultipleSelection:NO];
    [tableView setAllowsColumnResizing:NO];
    [tableView setAllowsTypeSelect:NO];
    [tableView setDelegate:self];
    [tableView setDataSource:self];
    [tableView sizeLastColumnToFit];
    [tableView setColumnAutoresizingStyle:NSTableViewUniformColumnAutoresizingStyle];
    [[[tableView tableColumns] lastObject] setResizingMask:NSTableColumnAutoresizingMask];
}

- (void)viewDidAppear
{
    [self _validateToolbarItems];
    
    [super viewDidAppear];

    // Fetch games if we haven't already, this allows reloading if an error occured, by switching to a different collection or media view and then back to homebrew
    if([[self games] count] == 0)
    {
        [self updateGames];
    }
}

- (OELibraryToolbar * _Nullable)toolbar
{
    NSToolbar *tb = self.view.window.toolbar;
    return [tb isKindOfClass:OELibraryToolbar.class] ? (OELibraryToolbar *)tb : nil;
}

- (void)_validateToolbarItems
{
    OELibraryToolbar *toolbar = self.toolbar;
    
    toolbar.viewModeSelector.enabled = NO;
    toolbar.viewModeSelector.selectedSegment = -1;
    
    toolbar.gridSizeSlider.enabled = NO;
    toolbar.decreaseGridSizeButton.enabled = NO;
    toolbar.increaseGridSizeButton.enabled = NO;
    
    toolbar.searchField.enabled = NO;
    toolbar.searchField.searchMenuTemplate = nil;
    toolbar.searchField.stringValue = @"";
    
    toolbar.addButton.enabled = NO;
    
    if (@available(macOS 11.0, *)) {
        for (NSToolbarItem *item in toolbar.items) {
            if ([item.itemIdentifier isEqual: @"OEToolbarSearchItem"]) {
                    item.enabled = NO;
            }
        }
    }
}

#pragma mark - Data Handling
- (void)updateGames
{
    // Indicate that we're fetching some remote resources
    [self displayUpdate];

    // Cancel last download
    [[self currentDownload] cancelDownload];

    NSURL *url = [NSURL URLWithString:OEHomebrewGamesURLString];

    OEDownload *download = [[OEDownload alloc] initWithURL:url];
    OEDownload __weak *blockDL = download;
    [download setCompletionHandler:^(NSURL *destination, NSError *error) {
        if([self currentDownload]==blockDL)
            [self setCurrentDownload:nil];
        if([error domain] == NSCocoaErrorDomain && [error code] == NSUserCancelledError)
            return;

        if(error == nil && destination != nil)
        {
            if(![self parseFileAtURL:destination error:&error])
            {
                [self displayError:error];
            }
            else
            {
                [self displayResults];
            }
        }
        else
        {
            [self displayError:error];
        }
    }];
    [self setCurrentDownload:download];
    [download startDownload];
}

- (BOOL)parseFileAtURL:(NSURL*)url error:(NSError**)outError
{
    NSError       *error    = nil;
    NSXMLDocument *document = [[NSXMLDocument alloc] initWithContentsOfURL:url options:0 error:&error];
    if(document == nil)
    {
        DLog(@"%@", error);
        return NO;
    }

    NSArray *allGames = [document nodesForXPath:@"//system | //game" error:&error];
    NSMutableArray *allHeaderIndices = [[NSMutableArray alloc] init];
    self.games = [allGames arrayByEvaluatingBlock:^id(id node, NSUInteger idx, BOOL *stop) {
        // Keep track of system node indices to use for headers
        if([[node nodesForXPath:@"@id" error:nil] lastObject])
            [allHeaderIndices addObject:@(idx)];

        return [[OEHomebrewGame alloc] initWithNode:node];
    }];

    self.headerIndices = [allHeaderIndices mutableCopy];

    return YES;
}

#pragma mark - View Management

- (void)displayUpdate
{
    OEHomebrewBlankSlateView *blankSlate = [[OEHomebrewBlankSlateView alloc] initWithFrame:[[self view] bounds]];
    [blankSlate setRepresentedObject:NSLocalizedString(@"Fetching Games…", @"Homebrew Blank Slate View Updating Info")];
    [self displayBlankSlate:blankSlate];
}

- (void)displayError:(NSError*)error
{
    OEHomebrewBlankSlateView *blankSlate = [[OEHomebrewBlankSlateView alloc] initWithFrame:[[self view] bounds]];
    [blankSlate setRepresentedObject:error];
    [self displayBlankSlate:blankSlate];
}

- (void)displayResults
{
    [self displayBlankSlate:nil];
}

- (void)displayBlankSlate:(OEHomebrewBlankSlateView *)blankSlate
{
    // Determine if we are about to replace the current first responder or one of its superviews
    id firstResponder = self.view.window.firstResponder;
    BOOL makeFirstResponder = [firstResponder isKindOfClass:[NSView class]] && [firstResponder isDescendantOf:self.view];
    
    if(_blankSlate != blankSlate)
    {
        [_blankSlate removeFromSuperview];
    }

    [self setBlankSlate:blankSlate];

    if(_blankSlate)
    {
        NSView *view = [self view];
        NSRect bounds = (NSRect){NSZeroPoint, view.window.contentLayoutRect.size};
        [_blankSlate setFrame:bounds];

        [view addSubview:_blankSlate];
        [_blankSlate setAutoresizingMask:NSViewHeightSizable|NSViewWidthSizable];
        
        self.tableView.hidden = YES;
    }
    else
    {
        [self.tableView reloadData];
        self.tableView.hidden = NO;
    }
    
    // restore first responder if necessary
    if (makeFirstResponder)
    {
        if (_blankSlate)
            [self.view.window makeFirstResponder:_blankSlate];
        else
            [self.view.window makeFirstResponder:self.tableView];
    }
}
#pragma mark - UI Methods
- (IBAction)gotoDeveloperWebsite:(id)sender
{
    NSInteger row = [self rowOfView:sender];

    if(row < 0 || row >= [[self games] count]) return;

    OEHomebrewGame *game = [[self games] objectAtIndex:row];

    NSURL *url = [NSURL URLWithString:[game website]];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

- (IBAction)importGame:(id)sender
{
    NSInteger row = [self rowOfView:sender];

    if(row < 0 || row >= [[self games] count]) return;
}

- (IBAction)launchGame:(id)sender
{
    NSInteger row = [self rowOfView:sender];

    if(row < 0 || row >= [[self games] count]) return;

    OEHomebrewGame *homebrewGame = [[self games] objectAtIndex:row];
    NSURL *url = [NSURL URLWithString:[homebrewGame fileURLString]];
    NSInteger fileIndex = [homebrewGame fileIndex];
    NSString *systemIdentifier = [homebrewGame systemIdentifier];
    NSString *md5  = [homebrewGame md5];
    NSString *name = [homebrewGame name];

    OELibraryDatabase      *db = self.database;
    NSManagedObjectContext *context = [db mainThreadContext];
    OEDBRom *rom = [OEDBRom romWithMD5HashString:md5 inContext:context error:nil];
    if(rom == nil)
    {
        NSEntityDescription *romDescription  = [OEDBRom entityDescriptionInContext:context];
        NSEntityDescription *gameDescription = [OEDBGame entityDescriptionInContext:context];
        rom = [[OEDBRom alloc] initWithEntity:romDescription insertIntoManagedObjectContext:context];
        [rom setSourceURL:url];
        [rom setArchiveFileIndex:@(fileIndex)];
        [rom setMd5:[md5 lowercaseString]];

        OEDBGame *game = [[OEDBGame alloc] initWithEntity:gameDescription insertIntoManagedObjectContext:context];
        [game setRoms:[NSSet setWithObject:rom]];
        [game setName:name];

        [game setBoxImageByURL:[[homebrewGame images] objectAtIndex:0]];

        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
        [game setSystem:system];
    }

    OEDBGame *game = [rom game];
    [game save];
    id target = [NSApp targetForAction:@selector(startGame:)];
    [target startGame:game];
}

- (NSInteger)rowOfView:(NSView*)view
{
    NSRect viewRect = [view frame];
    NSRect viewRectOnView = [[self tableView] convertRect:viewRect fromView:[view superview]];

    NSInteger row = [[self tableView] rowAtPoint:(NSPoint){NSMidX(viewRectOnView), NSMidY(viewRectOnView)}];

    if(row == 1)
    {
        NSView *container = [[view superview] superview];
        return [[container subviews] indexOfObjectIdenticalTo:[view superview]];
    }

    return row;
}

#pragma mark - Table View Datasource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[self games] count] +0; // -3 for featured games which share a row + 2 for headers +1 for the shared row
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(rowIndex == 0) return NSLocalizedString(@"Featured Games", @"");
    if(rowIndex == 2) return nil;

    if(rowIndex == 1) return [[self games] subarrayWithRange:NSMakeRange(0, 3)];

    if([[self headerIndices] containsObject:@(rowIndex)])
    {
        // Use system lastLocalizedName for header
        OEHomebrewGame *game = [[self games] objectAtIndex:rowIndex];
        NSString *identifier = [game systemGroup];

        NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];

        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:identifier inContext:context];

        return [system lastLocalizedName];
    }

    return [[self games] objectAtIndex:rowIndex];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSTableCellView *view = nil;

    if(row == 0 || [[self headerIndices] containsObject:@(row)])
    {
        view = [tableView makeViewWithIdentifier:@"HeaderView" owner:self];
    }
    else if (row == 2)
    {
        view = [tableView makeViewWithIdentifier:@"DummyView" owner:self];
    }
    else if(row == 1)
    {
        view = [tableView makeViewWithIdentifier:@"FeatureView" owner:self];
        NSArray *games = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];

        NSView *subview = [[[[view subviews] lastObject] subviews] lastObject];
        [games enumerateObjectsUsingBlock:^(OEHomebrewGame *game, NSUInteger idx, BOOL *stop) {
            NSView *container = [[subview subviews] objectAtIndex:idx];

            OEHomebrewCoverView *artworkView = [[container subviews] objectAtIndex:0];
            [artworkView setURLs:[game images]];
            [artworkView setTarget:self];
            [artworkView setDoubleAction:@selector(launchGame:)];

            NSTextField *label = [[container subviews] objectAtIndex:1];
            [label setStringValue:[game name]];
        }];
    }
    else
    {
        view = [tableView makeViewWithIdentifier:@"GameView" owner:self];
        NSArray *subviews = [view subviews];

        OEHomebrewGame *game = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];

        NSTextField *titleField = [subviews objectAtIndex:0];
        [titleField setStringValue:[game name]];

        // system / year flags
        NSButton    *system  = [subviews objectAtIndex:1];
        [system setTitle:[game systemShortName]];

        NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
        [formatter setDateFormat:@"Y"];
        NSButton    *year  = [subviews objectAtIndex:2];
        [year setHidden:[[game released] timeIntervalSince1970] == 0];
        [year setTitle:[formatter stringFromDate:[game released]]];

        // description
        NSTextField *description = [subviews objectAtIndex:3];
        description.stringValue = [game gameDescription] ?: @"";

        NSButton    *developer = [subviews objectAtIndex:5];
        [developer setTarget:self];
        [developer setAction:@selector(gotoDeveloperWebsite:)];
        [developer setObjectValue:[game website]];
        [developer setTitle:[game developer]];

        OEHomebrewCoverView *imagesView = [subviews objectAtIndex:4];
        [imagesView setURLs:[game images]];
        [imagesView setTarget:self];
        [imagesView setDoubleAction:@selector(launchGame:)];
    }

    return view;
}

#pragma mark - TableView Delegate

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

@end

@implementation OEHomebrewGame
- (instancetype)initWithNode:(NSXMLNode*)node
{
    self = [super init];
    if(self)
    {
#define StringValue(_XPATH_)  [[[[node nodesForXPath:_XPATH_ error:nil] lastObject] stringValue] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]
#define IntegerValue(_XPATH_) [StringValue(_XPATH_) integerValue]
#define DateValue(_XPATH_)    [NSDate dateWithTimeIntervalSince1970:IntegerValue(_XPATH_)]

        _name            = StringValue(@"@name");
        _developer       = StringValue(@"@developer");
        _website         = StringValue(@"@website");
        _fileURLString   = StringValue(@"@file");
        _fileIndex       = IntegerValue(@"@fileIndex");
        _gameDescription = StringValue(@"description");
        _added           = DateValue(@"@added");
        _released        = DateValue(@"@released");
        _systemIdentifier = StringValue(@"@system");
        _md5             = StringValue(@"@md5");
        _systemGroup     = StringValue(@"@id");

        NSArray *images = [node nodesForXPath:@"images/image" error:nil];
        _images = [images arrayByEvaluatingBlock:^id(NSXMLNode *node, NSUInteger idx, BOOL *stop) {
            return [NSURL URLWithString:StringValue(@"@src")];
        }];

#undef StringValue
#undef IntegerValue
#undef DateValue
    }
    return self;
}

- (NSString*)systemShortName
{
    NSString *identifier = [self systemIdentifier];
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];

    OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:identifier inContext:context];
    if([[system shortname] length] != 0)
        return [system shortname];

    return [[[identifier componentsSeparatedByString:@"."] lastObject] uppercaseString];
}
@end

