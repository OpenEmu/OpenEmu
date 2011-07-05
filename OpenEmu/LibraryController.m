//
//  LibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import "LibraryController.h"
#import "LibraryDatabase.h"
#import "OEDBSmartCollection.h"

#import "OEGameViewController.h"

#import "SidebarController.h"
#import "CollectionViewController.h"
#import "LibrarySplitView.h"

#import "NSImage+OEDrawingAdditions.h"
#import "OEBackgroundColorView.h"

#import "OEGameView.h"
@implementation LibraryController
+ (void)initialize{
    // toolbar sidebar button
    NSImage* image = [NSImage imageNamed:@"toolbar_sidebar_button"];
    [image setName:@"toolbar_sidebar_button_close" forSubimageInRect:NSMakeRect(0, 23, 84, 23)];
    [image setName:@"toolbar_sidebar_button_open" forSubimageInRect:NSMakeRect(0, 0, 84, 23)];
}

- (void)dealloc{
    [gameViewController release], gameViewController = nil;
    [database release], database = nil;
    
    [super dealloc];
}

#pragma mark -
#pragma mark WindowController stuff
- (NSString*)windowNibName{
    return @"Library";
}

- (void)awakeFromNib{
    // load the window
    [self window];
}

- (void)windowDidLoad{
    [super windowDidLoad];
	
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    NSString* path = [[[[[NSFileManager defaultManager] URLsForDirectory:NSLibraryDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmuDatabaseTest"] path];
    
#define UDDatabasePathKey @"databasePath"
#define UDDefaultDatabasePathKey @"defaultDatabasePath"
    
    NSDictionary* defaults = [NSDictionary dictionaryWithObjectsAndKeys:path, UDDefaultDatabasePathKey, path, UDDatabasePathKey, nil];
    [standardDefaults registerDefaults:defaults];
    
    // load database
    LibraryDatabase* db = [[LibraryDatabase alloc] init];
    if(!db){
		[NSApp terminate:self];
		return;
    }
    [self setDatabase:db];
    [db release];
	
    // Set up window
    [[self window] setOpaque:NO];
    [[self window] setBackgroundColor:[NSColor clearColor]];
    [[self window] setExcludedFromWindowsMenu:TRUE];
    [[self window] setShowsResizeIndicator:NO];
    [[self window] setDelegate:self];
    [(OEBackgroundColorView*)[[self window] contentView] setBackgroundColor:[NSColor clearColor]];
    
    NSMenu* menu = [editSmartCollectionMenuItem menu];
    [menu setAutoenablesItems:FALSE];
    [editSmartCollectionMenuItem setEnabled:FALSE];
    
    menu = [gridViewMenuItem menu];
    [menu setAutoenablesItems:FALSE];
    
    [gridViewMenuItem setEnabled:TRUE];
    [listViewMenuItem setEnabled:TRUE];
    [flowViewBtn setEnabled:TRUE];    
    
    // Set up Toggle Sidebar Button
    NSImage* image = [mainSplitView splitterPosition]==0? [NSImage imageNamed:@"toolbar_sidebar_button_open"]:[NSImage imageNamed:@"toolbar_sidebar_button_close"];
    [sidebarBtn setImage:image];
    
    // setup sidebar controller
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sidebarSelectionDidChange:) name:@"SidebarSelectionChanged" object:sidebarController];
    
    // make sure view has been loaded already
    [collectionViewController view];
    
    // Select first view
    // to do: restore the last used view!
	// to do: restore last selected collection item
    [collectionViewController setDatabase:self.database];
    [collectionViewController selectGridView:nil];
    [collectionViewController setCollectionItem:nil];
    [collectionViewController finishSetup];
	
    // add collection controller's view to splitview
    NSView* rightContentView = [mainSplitView rightContentView];
    [rightContentView addSubview:[collectionViewController view]];
    [[collectionViewController view] setFrame:[rightContentView bounds]];
    
    [mainSplitView adjustSubviews];
    
    [[self window] makeKeyAndOrderFront:self];
}

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)toggleSidebar:(id)sender{
    NSWindow* window = [self window];
    NSRect frameRect = [window frame];
    
    float widthCorrection;
    BOOL opening = [mainSplitView splitterPosition]==0;
    if(opening){
		frameRect.size.width += 186;
		frameRect.origin.x -= 186;
		
		widthCorrection = 186;
		// To do: Restore previous position instead of 186px
    } else {
		NSView* sidebar = [[mainSplitView subviews] objectAtIndex:0];
		float sidebarWidth = [sidebar frame].size.width;
		
		widthCorrection = -sidebarWidth;
		
		frameRect.size.width -= sidebarWidth;
		frameRect.origin.x += sidebarWidth;
    }
    
    NSRect splitViewRect = [mainSplitView frame];
    splitViewRect.size.width += widthCorrection;
    
    [mainSplitView setResizesLeftView:YES];
    [window setFrame:frameRect display:YES animate:YES];
    [mainSplitView setResizesLeftView:NO];
    
    NSImage* image = opening? [NSImage imageNamed:@"toolbar_sidebar_button_close"]:[NSImage imageNamed:@"toolbar_sidebar_button_open"];
    [sidebarBtn setImage:image];
    // TODO: remember sidebar width
}

#pragma mark -
#pragma mark Menu Item Actions
- (IBAction)toggleWindow:(id)sender{
    if([(NSMenuItem*)sender state]){
		[[self window] orderOut:self];
    } else {
		[self showWindow:self];
    }
    
    [sender setState:![(NSMenuItem*)sender state]];	
}

#pragma mark FileMenu Actions
- (IBAction)filemenu_launchGame:(id)sender{
    if(gameViewController){
		[gameViewController terminateEmulation];
		[[gameViewController view] removeFromSuperview];
		[gameViewController release], gameViewController = nil;
    }
    
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
	
    NSInteger result = [panel runModal];
    if(result != NSOKButton) return;
    
    NSError* error = nil;
    NSURL* fileURL = [panel URL];
    
    gameViewController = [[OEGameViewController alloc] init];
    
    if(![gameViewController loadFromURL:fileURL error:&error]){
		[gameViewController release], gameViewController = nil;
		[NSApp presentError:error];
		return;
    }
    
    [sidebarBtn setEnabled:NO];
    [sidebarController setEnabled:NO];
    [collectionViewController willHide];
    
    NSView* gameDocView = [gameViewController view];
    [[mainSplitView superview] addSubview:gameDocView];
    
    NSRect frame = [[mainSplitView superview] bounds];
    frame.origin.y += 45; // Toolbar Height+1px black line;
    frame.size.height -= 45;
    [gameDocView setFrame:frame];
    [[self window] makeFirstResponder:[[gameDocView subviews] objectAtIndex:0]];
}


- (IBAction)filemenu_newCollection:(id)sender{
    [self.database addNewCollection:nil];
    
    [sidebarController reloadData];
}

- (IBAction)filemenu_newSmartCollection:(id)sender{
    [self.database addNewSmartCollection:nil];
    
    [sidebarController reloadData];
}

- (IBAction)filemenu_newCollectionFolder:(id)sender{
    [self.database addNewCollectionFolder:nil];
    
    [sidebarController reloadData];
}
- (IBAction)filemenu_editSmartCollection:(id)sender{
    NSLog(@"Edit smart collection: ");
}
#pragma mark -
- (IBAction)filemenu_addToLibrary:(id)sender{
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setAllowsMultipleSelection:YES];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanCreateDirectories:NO];
    [openPanel setCanChooseDirectories:YES];
    
    NSInteger result = [openPanel runModal];
    if(result != NSOKButton){
		return;
    }
    
    for(NSURL* aURL in [openPanel URLs]){
		NSString* aPath = [aURL path];
		[self.database addGamesFromPath:aPath toCollection:nil searchSubfolders:YES];
    }
}
#pragma mark -
#pragma mark Sidebar Helpers
- (void)sidebarSelectionDidChange:(NSNotification*)notification{
    NSDictionary* userInfo = [notification userInfo];
    id collection = [userInfo objectForKey:@"selectedCollection"];
    
    [editSmartCollectionMenuItem setEnabled:[collection isKindOfClass:[OEDBSmartCollection class]]];
    
    [collectionViewController setCollectionItem:collection];
}

#pragma mark -
#pragma mark NSWindow Delegates
- (void)windowDidBecomeKey:(NSNotification *)notification{
    [gridViewMenuItem setEnabled:[gridViewBtn isEnabled]];
    [listViewMenuItem setEnabled:[listViewBtn isEnabled]];
    [flowViewMenuItem setEnabled:[flowViewBtn isEnabled]];
}

- (void)windowDidResignKey:(NSNotification *)notification{
    [gridViewMenuItem setEnabled:FALSE];
    [listViewMenuItem setEnabled:FALSE];
    [flowViewMenuItem setEnabled:FALSE];
}

#pragma mark -
#pragma mark Properties
- (void)setDatabase:(LibraryDatabase *)newDatabase{
    [newDatabase retain];
    [database release];
    
    database = newDatabase;
    [sidebarController setDatabase:database];
    
    [sidebarController reloadData];	
}
@synthesize mainSplitView, database;
@end
