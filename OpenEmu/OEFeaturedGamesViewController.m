//
//  OEFeaturedGamesViewController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 09/07/14.
//
//

#import "OEFeaturedGamesViewController.h"

#import "OETheme.h"
#import "OEDownload.h"
#import "OEBlankSlateBackgroundView.h"
#import "OEURLImagesView.h"

#import "NSArray+OEAdditions.h"
#import "NS(Attributed)String+Geometrics.h"

NSString * const OEFeaturedGamesViewURLString = @"file:///Users/chris/Desktop/openemu.github.io/index.html";
NSString * const OEFeaturedGamesURLString = @"file:///Users/chris/Desktop/games.xml";

NSString * const OELastFeaturedGamesCheckKey = @"lastFeaturedGamesCheck";

@interface OEFeaturedGame : NSObject
- (instancetype)initWithNode:(NSXMLNode*)node;

@property (readonly, copy) NSString *name;
@property (readonly, copy) NSString *developer;
@property (readonly, copy) NSString *website;
@property (readonly, copy) NSString *fileURLString;
@property (readonly, copy) NSString *gameDescription;
@property (readonly, copy) NSDate   *added;
@property (readonly, copy) NSDate   *released;
@property (readonly) NSInteger fileIndex;
@property (readonly, copy) NSArray  *images;

@property (readonly, copy) NSString *systemIdentifier;
@end
@interface OEFeaturedGamesViewController () <NSTableViewDataSource, NSTableViewDelegate>
@property (strong) NSArray *games;
@end

@implementation OEFeaturedGamesViewController

+ (void)initialize
{
    if(self == [OEFeaturedGamesViewController class])
    {
        NSDictionary *defaults = @{ OELastFeaturedGamesCheckKey:[NSDate dateWithTimeIntervalSince1970:0],
                                    };
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    }
}

- (NSString*)nibName
{
    return @"OEFeaturedGamesViewController";
}

- (void)loadView
{
    [super loadView];

    NSView *view = self.view;

    [view setPostsBoundsChangedNotifications:YES];
    [view setPostsFrameChangedNotifications:YES];
    [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    [[self webView] setDrawsBackground:NO];
    [[self webView] setUIDelegate:self];
    [[self webView] setPolicyDelegate:self];
    [[self webView] setFrameLoadDelegate:self];
    [[self webView] setMainFrameURL:OEFeaturedGamesViewURLString];

    [[self tableView] setAllowsColumnReordering:NO];
    [[self tableView] setAllowsEmptySelection:YES];
    [[self tableView] setAllowsMultipleSelection:NO];
    [[self tableView] setAllowsTypeSelect:NO];
    [[self tableView] setDelegate:self];
    [[self tableView] setDataSource:self];

    [[self tableView] sizeLastColumnToFit];
    [[self tableView] setColumnAutoresizingStyle:NSTableViewUniformColumnAutoresizingStyle];
    [[[[self tableView] tableColumns] lastObject] setResizingMask:NSTableColumnAutoresizingMask];

    [self updateGames];
    [[self tableView] reloadData];
}

#pragma mark - Data Handling
- (void)updateGames
{
    NSURL    *url = [NSURL URLWithString:OEFeaturedGamesURLString];

    OEDownload *download = [[OEDownload alloc] initWithURL:url];
    [download setCompletionHandler:^(NSURL *destination, NSError *error) {
        if(error == nil && destination != nil)
        {
            [self parseFileAtURL:destination];
        }
        else
        {
            [self displayError:error];
        }
    }];

    [download startDownload];
}

- (void)parseFileAtURL:(NSURL*)url
{
    NSError       *error    = nil;
    NSXMLDocument *document = [[NSXMLDocument alloc] initWithContentsOfURL:url options:0 error:&error];
    if(document == nil)
    {
        DLog(@"%@", error);
        return;
    }

    NSArray *dates = [document nodesForXPath:@"//game/@added" error:&error];
    dates = [dates arrayByEvaluatingBlock:^id(id obj, NSUInteger idx, BOOL *stop) {
        return [NSDate dateWithTimeIntervalSince1970:[[obj stringValue] integerValue]];
    }];

    NSDate *lastCheck = [[NSUserDefaults standardUserDefaults] objectForKey:OELastFeaturedGamesCheckKey];
    NSMutableIndexSet *newGameIndices = [NSMutableIndexSet indexSet];
    [dates enumerateObjectsUsingBlock:^(NSDate *obj, NSUInteger idx, BOOL *stop) {
        if([obj compare:lastCheck] == NSOrderedDescending)
            [newGameIndices addIndex:idx];
    }];

    NSArray *allGames = [document nodesForXPath:@"//game" error:&error];
    NSArray *newGames = [allGames objectsAtIndexes:newGameIndices];

    self.games = [newGames arrayByEvaluatingBlock:^id(id node, NSUInteger idx, BOOL *block) {
        return [[OEFeaturedGame alloc] initWithNode:node];
    }];

    [[self tableView] reloadData];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        WebScriptObject *script = [[self webView] windowScriptObject];
        [script callWebScriptMethod:@"reloadData" withArguments:@[]];
    });
}

- (NSDictionary*)OE_gameDictionaryWithNode:(NSXMLNode*)node
{
#define StringValue(_XPATH_)  [[[node nodesForXPath:_XPATH_ error:nil] lastObject] stringValue]
#define IntegerValue(_XPATH_) (id)(StringValue(_XPATH_) ? @([StringValue(_XPATH_) integerValue]) : [NSNull null])
#define DateValue(_XPATH_)    [NSDate dateWithTimeIntervalSince1970:[IntegerValue(_XPATH_) integerValue]]

    id name            = StringValue(@"@name") ?: [NSNull null];
    id developer       = StringValue(@"@developer") ?: [NSNull null];
    id website         = StringValue(@"@website") ?: [NSNull null];
    id fileURLString   = StringValue(@"@file") ?: [NSNull null];
    id fileIndex       = IntegerValue(@"@fileIndex") ?: [NSNull null];
    id gameDescription = StringValue(@"description") ?: [NSNull null];
    id added             = DateValue(@"@added") ?: [NSNull null];
    id released          = DateValue(@"@released") ?: [NSNull null];
    id systemIdentifier = StringValue(@"@systemIdentifier") ?: [NSNull null];

    NSArray *images = [node nodesForXPath:@"images/image" error:nil];
    images = [images arrayByEvaluatingBlock:^id(NSXMLNode *node, NSUInteger idx, BOOL *stop) {
        return StringValue(@"@src") ?: [NSNull null];
    }];

    return @{ @"name":name, @"developer":developer, @"website":website, @"file":fileURLString, @"description":gameDescription, @"systemIdentifier":systemIdentifier};

#undef StringValue
#undef IntegerValue
#undef DateValue
}
- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id < WebPolicyDecisionListener >)listener
{
    NSURL *url = [request URL];
    if([[url scheme] isEqualTo:@"oe"])
    {
        NSArray *games = nil;
        NSString *host = [url host];
        if([host isEqualTo:@"features"])
        {
            games = [self.games subarrayWithRange:NSMakeRange(0, 3)];
        }
        else if([host isEqualTo:@"others"])
        {
            // TODO: apply search filter here
            games = [self.games subarrayWithRange:NSMakeRange(3, [self.games count]-3)];
        }

        if(games)
        {
            NSUInteger options = 0;
#ifdef DEBUG
            options = NSJSONWritingPrettyPrinted;
#endif
            NSData *data = [NSJSONSerialization dataWithJSONObject:games options:options error:nil];

            [listener ignore];
        }
    }

    NSString *host = [[request URL] host];
    if (host)
    {
        [[NSWorkspace sharedWorkspace] openURL:[request URL]];
    }
    else
    {
        [listener use];
    }
}
#pragma mark - View Managing
- (void)displayError:(NSError*)error
{
    NSLog(@"%@", error);
}

- (NSDictionary*)descriptionStringAttributes
{
    OEThemeTextAttributes *attribtues = [[OETheme sharedTheme] themeTextAttributesForKey:@"feature_description"];
    return [attribtues textAttributesForState:OEThemeStateDefault];
}

#pragma mark - Table View Datasource
- (NSInteger)numberOfRowsInTableView:(IKImageBrowserView *)aBrowser
{
    return [[self games] count] +0; // -3 for featured games which share a row + 2 for headers +1 for the shared row
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(rowIndex == 0) return OELocalizedString(@"Featured Games", @"");
    if(rowIndex == 2) return OELocalizedString(@"All Homebrew", @"");

    if(rowIndex == 1) return [[self games] subarrayWithRange:NSMakeRange(0, 3)];

    return [[self games] objectAtIndex:rowIndex];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSTableCellView *view = nil;
    
    if(row == 0 || row == 2)
    {
        view = [tableView makeViewWithIdentifier:@"HeaderView" owner:self];
    }
    else if(row == 1)
    {
        view = [tableView makeViewWithIdentifier:@"FeatureView" owner:self];
    }
    else
    {
        view = [tableView makeViewWithIdentifier:@"GameView" owner:self];
        NSArray *subviews = [view subviews];

        OEFeaturedGame *game = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];

        NSTextField *titleField = [subviews objectAtIndex:0];
        [titleField setStringValue:[game name]];

        NSButton    *system  = [subviews objectAtIndex:2];
        [system setTitle:[game systemIdentifier]];
        [system sizeToFit];

        NSButton    *import  = [subviews objectAtIndex:3];
        [import sizeToFit];
        [import setFrameOrigin:(NSPoint){NSMaxX([system frame])+0.0, NSMinY([system frame])}];

        NSScrollView *descriptionScroll = [subviews objectAtIndex:4];
        NSTextView *description = [descriptionScroll documentView];

        [description setString:[game gameDescription] ?: @""];
        NSInteger length = [[description textStorage] length];
        NSDictionary *attributes = [self descriptionStringAttributes];
        [[description textStorage] setAttributes:attributes range:NSMakeRange(0, length)];
        [description sizeToFit];

        NSTextField *label     = [subviews objectAtIndex:1];
        NSButton    *developer = [subviews objectAtIndex:6];
        [developer setTitle:[game developer]];
        [developer sizeToFit];
        [developer setFrameSize:NSMakeSize([developer frame].size.width, label.frame.size.height)];

        OEURLImagesView *imagesView = [subviews objectAtIndex:5];
        [imagesView setURLs:[game images]];
    }

    return view;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    if(row == 0 || row == 2)
        return 94.0;
    if(row == 1)
        return 60.0;

    CGFloat textHeight = 0.0;
    OEFeaturedGame *game = [self tableView:tableView objectValueForTableColumn:nil row:row];
    NSString *gameDescription = [game gameDescription];
    if(gameDescription)
    {
        NSDictionary *attributes = [self descriptionStringAttributes];
        NSAttributedString *string = [[NSAttributedString alloc] initWithString:gameDescription attributes:attributes];

        textHeight = [string heightForWidth:291.0] + 130.0;
    }

    return MAX(160.0, textHeight);
}
#pragma mark - TableView Delegate
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

#pragma mark - JavaScript Bridge
+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
    return YES;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector
{
    if(selector == @selector(featuredGames))
        return NO;
    if(selector == @selector(otherGames))
        return NO;

    return YES;
}

- (NSArray*)featuredGames
{
    if([[self games] count] < 3)
        return @[];

    return [[self games] subarrayWithRange:NSMakeRange(0, 3)];
}

- (NSArray*)otherGames
{
    if([[self games] count] < 3)
        return @[];

    return [[self games] subarrayWithRange:NSMakeRange(3, [[self games] count]-3)];
}

- (void)webView:(WebView *)sender didCommitLoadForFrame:(WebFrame *)frame
{
    // Inject OpenEmu object
    [[sender windowScriptObject] setValue:self forKey:@"OpenEmu"];
}

#pragma mark - WebKit dragging
- (NSUInteger)webView:(WebView *)webView dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo
{
    return 0;
}
- (NSUInteger)webView:(WebView *)webView dragSourceActionMaskForPoint:(NSPoint)point
{
    return 0;
}

- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems
{
    return defaultMenuItems;
    return nil;
}

#pragma mark - State Handling
- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    _libraryController = libraryController;

    [[libraryController toolbarFlowViewButton] setEnabled:NO];
    [[libraryController toolbarGridViewButton] setEnabled:NO];
    [[libraryController toolbarListViewButton] setEnabled:NO];

    [[libraryController toolbarSearchField] setEnabled:NO];

    [[libraryController toolbarSlider] setEnabled:NO];
}
@end

@implementation OEFeaturedGame
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

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
    return strlen(name) <= 1;
}

+ (NSString*)webScriptNameForKey:(const char *)name
{
    if(strlen(name) <= 1) return @"";
    return [NSString stringWithUTF8String:name+1];
}
@end

