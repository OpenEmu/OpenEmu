//
//  LibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//
#import "OELibraryController.h"
#import "OELibraryDatabase.h"
#import "OEDBSmartCollection.h"

#import "OESidebarController.h"
#import "OECollectionViewController.h"
#import "OELibrarySplitView.h"

#import "NSImage+OEDrawingAdditions.h"
#import "OEBackgroundColorView.h"

#import "OEGameView.h"

#import "OEHUDGameWindow.h"
#import "OEROMImporter.h"

#import "OEPlugin.h"
#import "OECorePlugin.h"
#import "OESystemPlugin.h"
#import "OECompositionPlugin.h"

#import "NSControl+OEAdditions.h"

#import "OEGameDocument.h"


#ifndef NSWindowWillEnterFullScreenNotification
NSString * const NSWindowWillEnterFullScreenNotification = @"OEWindowWillEnterFullScreenNotification";
NSString * const NSWindowWillExitFullScreenNotification = @"OEWindowWillExitFullScreenNotification";
#endif

@interface OELibraryController (Private)
- (void)_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;

- (void)_setupMenu;
- (void)_setupToolbarItems;
@end
@implementation OELibraryController
- (id)initWithWindowController:(OEMainWindowController*)windowController andDatabase:(OELibraryDatabase*)database
{
    self = [super initWithWindowController:windowController];
    if (self) {
        NSLog(@"Init OELibraryController");
        [self setDatabase:database];
        [self view];
    }
    return self;
}
- (void)dealloc
{
    NSLog(@"Dealloc OELibraryController");
    
    [[NSNotificationCenter defaultCenter] removeObject:self];
    
    [self setCollectionViewController:nil];
    [self setMainSplitView:nil];
    [self setSidebarController:nil];
    [self setDatabase:nil];
    [self setRomImporter:nil];
    
    [super dealloc];
}

#pragma mark -
#pragma mark NSViewController stuff
- (NSString*)nibName{
    return @"Library";
}

- (void) awakeFromNib
{
    [super awakeFromNib];
    
    self.romImporter = [[[OEROMImporter alloc] initWithDatabase:[self database]] autorelease];
    
    // setup sidebar controller
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sidebarSelectionDidChange:) name:@"SidebarSelectionChanged" object:[self sidebarController]];
    [[self sidebarController] setDatabase:[self database]];
    [self setSidebarChangesWindowSize:YES];
    
    // make sure view has been loaded already
    OECollectionViewController* collectionViewController = [self collectionViewController];
    [collectionViewController view];
    
    // Select first view
    // to do: restore last selected collection item
    [collectionViewController setLibraryController:self];
    [collectionViewController setCollectionItem:nil];
    [collectionViewController finishSetup];
    
    // add collection controller's view to splitview
    NSView* rightContentView = [[self mainSplitView] rightContentView];
    [rightContentView addSubview:[collectionViewController view]];
    [[collectionViewController view] setFrame:[rightContentView bounds]];
    
    [[self sidebarController] setDatabase:[self database]];
    [[self mainSplitView] adjustSubviews];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowFullscreenEnter:) name:NSWindowWillEnterFullScreenNotification object:[[self windowController] window]];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowFullscreenExit:) name:NSWindowWillExitFullScreenNotification object:[[self windowController] window]];
}

- (void)contentWillShow
{
    NSLog(@"OELibraryController contentWillShow");
    OEMainWindowController* windowController = [self windowController];
    NSView* toolbarItemContainer = [[windowController toolbarSearchField] superview]; 
    [toolbarItemContainer setAutoresizingMask:0];
    
    [self _setupMenu];
    [self _setupToolbarItems];
    [self layoutToolbarItems];
    
    [[self collectionViewController] willShow];
    
    [[self sidebarController] reloadData];
    
    OELibrarySplitView* splitView = (OELibrarySplitView*)[self view];
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    float splitterPos = [standardUserDefaults floatForKey:UDSidebarWidthKey];
    NSLog(@"%@ :: %f", splitView, splitterPos);
    [splitView setSplitterPosition:splitterPos animated:NO];
}

- (void)contentWillHide
{
    OEMainWindowController* windowController = [self windowController];
    NSView* toolbarItemContainer = [[windowController toolbarSearchField] superview]; 
    [toolbarItemContainer setAutoresizingMask:NSViewWidthSizable];
}
#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)toggleSidebar:(id)sender
{    
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    OELibrarySplitView* mainSplitView = [self mainSplitView];
    
    BOOL opening = [mainSplitView splitterPosition]==0;
    float widthCorrection = 0;
    if(opening)
    {
        widthCorrection = [standardDefaults floatForKey:UDSidebarWidthKey];
        if(widthCorrection==0)
            widthCorrection = 168;
    }
    else 
    {
        NSView* sidebar = [[mainSplitView subviews] objectAtIndex:0];
        float lastWidth = [sidebar frame].size.width;
        [standardDefaults setFloat:lastWidth forKey:UDSidebarWidthKey];
        
        widthCorrection = -1*lastWidth;
    }
    
    if(self.sidebarChangesWindowSize)
    {
        NSWindow* window = [[self windowController] window];
        NSRect frameRect = [window frame];
        
        frameRect.origin.x -= widthCorrection;
        frameRect.size.width += widthCorrection;
        NSRect splitViewRect = [mainSplitView frame];
        splitViewRect.size.width += widthCorrection;
        
        [mainSplitView setResizesLeftView:YES];
        [window setFrame:frameRect display:YES animate:YES];
        [mainSplitView setResizesLeftView:NO];
    }
    else 
    {
        widthCorrection = widthCorrection < 0 ? 0 : widthCorrection; 
        [mainSplitView setSplitterPosition:widthCorrection animated:YES];
    }
    
    NSImage* image;
    if(self.sidebarChangesWindowSize)
    {
        image = !opening? [NSImage imageNamed:@"toolbar_sidebar_button_open"]:[NSImage imageNamed:@"toolbar_sidebar_button_close"];
    } 
    else 
    {
        image = !opening? [NSImage imageNamed:@"toolbar_sidebar_button_close"]:[NSImage imageNamed:@"toolbar_sidebar_button_open"];
    }
    [[[self windowController] toolbarSidebarButton] setImage:image];
    
    [standardDefaults setBool:opening forKey:UDSidebarVisibleKey];
}
- (IBAction) switchToGridView:(id)sender
{
    [[self collectionViewController] selectGridView:sender]; 
}

- (IBAction) switchToListView:(id)sender
{
    [[self collectionViewController] selectListView:sender]; 
}

- (IBAction) switchToFlowView:(id)sender
{
    [[self collectionViewController] selectFlowView:sender];
}
- (IBAction)search:(id)sender
{
    [[self collectionViewController] search:sender];
}
- (IBAction)changeGridSize:(id)sender
{
    [[self collectionViewController] changeGridSize:sender];
}

#pragma mark FileMenu Actions
- (IBAction)filemenu_newCollection:(id)sender
{
    OELibraryDatabase* database = [self database];
    [database addNewCollection:nil];
    
    [[self sidebarController] reloadData];
}

- (IBAction)filemenu_newSmartCollection:(id)sender
{
    OELibraryDatabase* database = [self database];
    [database addNewSmartCollection:nil];
    
    [[self sidebarController] reloadData];
}

- (IBAction)filemenu_newCollectionFolder:(id)sender
{
    OELibraryDatabase* database = [self database];
    [database addNewCollectionFolder:nil];
    
    [[self sidebarController] reloadData];
}

- (IBAction)filemenu_editSmartCollection:(id)sender
{
    NSLog(@"Edit smart collection: ");
}
#pragma mark -
#pragma mark Menu Items
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if([[self windowController] currentContentController]!=self)
        return NO;
    
    NSUInteger tag = [menuItem tag];
    
    if(tag > 100 && tag < 200) // File Menu
    {
        if((tag == MainMenu_File_NewCollection)
           //|| (tag == MainMenu_File_NewCollectionFolder)
           || (tag == MainMenu_File_NewSmartCollection)
           || (tag == MainMenu_File_AddToLibrary))
            return YES;
        return NO;
    }
    
    if(tag == MainMenu_Controls_StartGame) // Controls Menu
        return (BOOL)[[[self collectionViewController] selectedGames] count];
    
    if(tag > 300 && tag < 400) // View Menu
    {
        return YES;
    }
    return NO;
}
- (void)menuItemAction:(id)sender
{
    switch ([sender tag]) {
        case MainMenu_File_NewCollection:
            [self filemenu_newCollection:sender];
            break;
        case MainMenu_File_NewCollectionFolder:
            [self filemenu_newCollectionFolder:sender];
            break;
        case MainMenu_File_NewSmartCollection:
            [self filemenu_newSmartCollection:sender];
            break;
        case MainMenu_File_EditSmartCollection:
            [self filemenu_editSmartCollection:sender];
            break;
        case MainMenu_File_AddToLibrary:
            [self filemenu_addToLibrary:sender];
            break;
        case MainMenu_File_GetInfo:
            break;
        case MainMenu_File_Rating:
            break;
        case MainMenu_File_ShowInFinder:
            break;
        case MainMenu_File_DisplayDuplicates:
            break;
            
            // Controls Menu
        case MainMenu_Controls_StartGame:
            [self controlsmenu_startGame:sender];
            break;
            
            // View Menu
        case MainMenu_View_GridViewTag:
            [self switchToGridView:sender];
            break;
            
        case MainMenu_View_FlowViewTag:            
            [self switchToFlowView:sender];
            break;
            
        case MainMenu_View_ListViewTag:
            [self switchToListView:sender];            
            break;
        default:
            break;
    }    
}

#pragma mark -
#pragma mark Import

- (IBAction)filemenu_addToLibrary:(id)sender
{
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setAllowsMultipleSelection:YES];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanCreateDirectories:NO];
    [openPanel setCanChooseDirectories:YES];
    
    NSWindow* win = [[self view] window];
    [openPanel beginSheetModalForWindow:win completionHandler:^(NSInteger result){
        if(result == NSFileHandlingPanelOKButton)
        {
            // exit our initial open panels completion handler
            //[self performSelector:@selector(startImportSheet:) withObject:[openPanel URLs] afterDelay:0.0];
            romImporter.errorBehaviour = OEImportErrorAskUser;
            [romImporter importROMsAtURLs:[openPanel URLs] inBackground:YES error:nil];
        }
    }];
}
#pragma mark -

- (IBAction)controlsmenu_startGame:(id)sender
{
    NSArray* selection = [[self collectionViewController] selectedGames];
    if(!selection)
    {
        DLog(@"No game. This should not be possible from UI (item disabled)");
        return;
    }
    OEDBGame* selectedGame = [selection lastObject];
    NSDocumentController* docController = [NSDocumentController sharedDocumentController];
    OEGameDocument* document = [[OEGameDocument alloc] initWithGame:selectedGame];
    [docController addDocument:document];
    [document release];
}
#pragma mark -
#pragma mark Sidebar Helpers
- (void)sidebarSelectionDidChange:(NSNotification*)notification
{
    NSDictionary* userInfo = [notification userInfo];
    if(userInfo)
    {
        id collection = [userInfo objectForKey:@"selectedCollection"];
        
        NSMenu* mainMenu = [NSApp mainMenu];
        NSMenu* fileMenu = [[mainMenu itemAtIndex:1] menu];
        NSMenuItem* item = [fileMenu itemWithTag:MainMenu_File_EditSmartCollection];
        [item setEnabled:[collection isKindOfClass:[OEDBSmartCollection class]]];
        [[self collectionViewController] setCollectionItem:collection];
    } 
    else 
    {
        [[self collectionViewController] setCollectionItem:nil];
    }
}
#pragma mark -
#pragma mark Properties
- (void)setSidebarChangesWindowSize:(BOOL)flag
{
    sidebarChangesWindowSize = flag;
    
    NSImage* image;
    if(flag)
    {
        image = [[self mainSplitView] splitterPosition]==0? [NSImage imageNamed:@"toolbar_sidebar_button_open"]:[NSImage imageNamed:@"toolbar_sidebar_button_close"];
    } 
    else 
    {
        image = [[self mainSplitView] splitterPosition]==0? [NSImage imageNamed:@"toolbar_sidebar_button_close"]:[NSImage imageNamed:@"toolbar_sidebar_button_open"];
    }
    
    [[[self windowController] toolbarSidebarButton] setImage:image];
    [[[self windowController] toolbarSidebarButton] display];
}

- (BOOL)sidebarChangesWindowSize
{
    return sidebarChangesWindowSize;
}

#pragma mark -
#pragma mark Private
- (void)_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag
{
    [self setSidebarChangesWindowSize:!fsFlag];
    [self mainSplitView].drawsWindowResizer = !fsFlag;
    
    if(fsFlag)
    {
        [NSApp setPresentationOptions:NSApplicationPresentationAutoHideDock|NSApplicationPresentationAutoHideToolbar];
        
    } 
    else 
    {
        [NSApp setPresentationOptions:NSApplicationPresentationDefault];
        
    }
}
#pragma mark -
- (void)_setupMenu
{}

- (void)_setupToolbarItems
{
    OEMainWindowController* windowController = [self windowController];
    
    [[windowController toolbarFlowViewButton] setEnabled:YES];
    [[windowController toolbarFlowViewButton] setTarget:self andAction:@selector(switchToFlowView:)];
    
    [[windowController toolbarGridViewButton] setEnabled:YES];
    [[windowController toolbarGridViewButton] setTarget:self andAction:@selector(switchToGridView:)];
    
    [[windowController toolbarListViewButton] setEnabled:YES];
    [[windowController toolbarListViewButton] setTarget:self andAction:@selector(switchToListView:)];
    
    [[windowController toolbarSearchField] setEnabled:YES];
    [[windowController toolbarSearchField] setTarget:self andAction:@selector(search:)];
    
    [[windowController toolbarSlider] setEnabled:[[windowController toolbarGridViewButton] state]];
    [[windowController toolbarSlider] setTarget:self andAction:@selector(changeGridSize:)];
    
    [[windowController toolbarSidebarButton] setEnabled:YES];
    [[windowController toolbarSidebarButton] setTarget:self andAction:@selector(toggleSidebar:)];
    
    [[windowController toolbarAddToSidebarButton] setEnabled:YES];
    [[windowController toolbarAddToSidebarButton] setTarget:[self sidebarController] andAction:@selector(addCollectionAction:)];
}

- (void)layoutToolbarItems
{
    OEMainWindowController* windowController = [self windowController];
    NSView* toolbarItemContainer = [[windowController toolbarSearchField] superview]; 
    
    float splitterPosition =[[self mainSplitView] splitterPosition];
    [[NSUserDefaults standardUserDefaults] setFloat:splitterPosition forKey:UDSidebarWidthKey];
    float width = [[[self mainSplitView] rightContentView] frame].size.width, height = 44.0;
    
    NSRect toolbarFrame = (NSRect){{splitterPosition, 0},{width, height}};
    [toolbarItemContainer setFrame:toolbarFrame];
    
}

- (void)windowFullscreenExit:(NSWindow*)window
{
    [self setSidebarChangesWindowSize:YES];
}

- (void)windowFullscreenEnter:(NSWindow*)window
{
    [self setSidebarChangesWindowSize:NO];
}
@synthesize romImporter;
@synthesize database;
@synthesize sidebarController, collectionViewController, mainSplitView;
@end
