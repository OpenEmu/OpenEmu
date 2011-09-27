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


#import "OENewGameDocument.h"

#import "OEPlugin.h"
#import "OECorePlugin.h"
#import "OESystemPlugin.h"
#import "OECompositionPlugin.h"
@interface OELibraryController (Private)
- (void)_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag;
- (void)_launchGameDoc:(id)gameDoc;
@end
@implementation OELibraryController
+ (void)initialize{
    // This can get called many times, don't need to be blowing away the defaults
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSDictionary *initialValues = [[[defaults initialValues] mutableCopy] autorelease];
    if(initialValues == nil)
        initialValues = [NSMutableDictionary dictionary];
    
    [initialValues setValue:@"Linear"                      forKey:@"filterName"];
    [initialValues setValue:[NSNumber numberWithFloat:1.0] forKey:@"volume"];
    [defaults setInitialValues:initialValues];
    
    if([OELibraryController class] == self){
        [OEPlugin registerPluginClass:[OECorePlugin class]];
        [OEPlugin registerPluginClass:[OESystemPlugin class]];
        [OEPlugin registerPluginClass:[OECompositionPlugin class]];
    }
	
	
    // toolbar sidebar button
    NSImage* image = [NSImage imageNamed:@"toolbar_sidebar_button"];
    [image setName:@"toolbar_sidebar_button_close" forSubimageInRect:NSMakeRect(0, 23, 84, 23)];
    [image setName:@"toolbar_sidebar_button_open" forSubimageInRect:NSMakeRect(0, 0, 84, 23)];
}

- (void)dealloc{
	if(gameDocument){
		[gameDocument terminateEmulation];
		[gameDocument release], gameDocument = nil;
	}
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
    
    self.cancelImport = NO;
}

- (void)windowDidLoad{
    [super windowDidLoad];
	
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    NSString* path = [[[[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmu/Database"] path];
    
    NSDictionary* defaults = [NSDictionary dictionaryWithObjectsAndKeys:path, UDDefaultDatabasePathKey, path, UDDatabasePathKey, nil];
    [standardDefaults registerDefaults:defaults];
    
    // load database
    OELibraryDatabase* db = [[OELibraryDatabase alloc] init];
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
    
	[self setSidebarChangesWindowSize:YES];
    
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
	
	
	// OEROMImporter* importer = [[OEROMImporter alloc] initWithDatabase:self.database];
	// [importer importROMsAtPath:@"~/Documents" inBackground:YES error:nil];
}


#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)toggleSidebar:(id)sender{
	NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
		
	BOOL opening = [mainSplitView splitterPosition]==0;
	float widthCorrection = 0;
	if(opening){
		widthCorrection = [standardDefaults floatForKey:UDSidebarWidthKey];
		if(widthCorrection==0)
			widthCorrection = 168;
	} else {
		NSView* sidebar = [[mainSplitView subviews] objectAtIndex:0];
		float lastWidth = [sidebar frame].size.width;
		[standardDefaults setFloat:lastWidth forKey:UDSidebarWidthKey];		
		
		widthCorrection = -1*lastWidth;
	}
    
	if(self.sidebarChangesWindowSize){
		NSWindow* window = [self window];
		NSRect frameRect = [window frame];

		frameRect.origin.x -= widthCorrection;
		frameRect.size.width += widthCorrection;
		
		NSRect splitViewRect = [mainSplitView frame];
		splitViewRect.size.width += widthCorrection;
		
		[mainSplitView setResizesLeftView:YES];
		[window setFrame:frameRect display:YES animate:YES];
		[mainSplitView setResizesLeftView:NO];
	} else {
		widthCorrection = widthCorrection < 0 ? 0 : widthCorrection; 
		[mainSplitView setSplitterPosition:widthCorrection animated:YES];
	}
	
	NSImage* image;
	if(self.sidebarChangesWindowSize){
		image = !opening? [NSImage imageNamed:@"toolbar_sidebar_button_open"]:[NSImage imageNamed:@"toolbar_sidebar_button_close"];
	} else {
		image = !opening? [NSImage imageNamed:@"toolbar_sidebar_button_close"]:[NSImage imageNamed:@"toolbar_sidebar_button_open"];
	}
    [sidebarBtn setImage:image];
	
	[standardDefaults setBool:opening forKey:UDSidebarVisibleKey];
}

#pragma mark -
#pragma mark Menu Item Actions
- (IBAction)toggleWindow:(id)sender{
	// TODO: this needs some fixing
    if([(NSMenuItem*)sender state]){
		[[self window] orderOut:self];
    } else {
		[self showWindow:self];
    }
    
    [sender setState:![(NSMenuItem*)sender state]];	
}

#pragma mark FileMenu Actions
- (IBAction)filemenu_launchGame:(id)sender{
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
	
    NSInteger result = [panel runModal];
    if(result != NSOKButton) return;
    
	NSURL* fileURL = [panel URL];
	NSError* error = nil;
	OENewGameDocument* gameDoc = [OENewGameDocument newDocumentWithRomAtURL:fileURL error:&error];
	if(!gameDoc){
		[NSApp presentError:error];		
		return;
	}
	[self _launchGameDoc:gameDoc];
	[gameDoc release];
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

- (IBAction) switchToGridView:(id)sender
{
    [collectionViewController selectGridView:sender]; 
}

- (IBAction) switchToListView:(id)sender
{
    [collectionViewController selectListView:sender]; 
}

- (IBAction) switchToFlowView:(id)sender
{
    [collectionViewController selectFlowView:sender];
}

#pragma mark -
#pragma mark Import

- (IBAction)filemenu_addToLibrary:(id)sender{
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setAllowsMultipleSelection:YES];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanCreateDirectories:NO];
    [openPanel setCanChooseDirectories:YES];
    
    [openPanel beginSheetModalForWindow:libraryWindow completionHandler:^(NSInteger result){
        if(result == NSFileHandlingPanelOKButton)
        {
            // exit our initial open panels completion handler
            [self performSelector:@selector(startImportSheet:) withObject:[openPanel URLs] afterDelay:0.0];
        }
    }];
}

- (void) startImportSheet:(NSArray*)URLs
{    
    [importProgress setMaxValue:[URLs count]];
    
    [NSApp beginSheet:importSheet
       modalForWindow:libraryWindow
        modalDelegate:self
       didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:)
          contextInfo:nil];
    
    // need to wait a 'tick' for the NSRunloop to finish, so we do this. Not so bad
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    
        for(NSURL* aURL in URLs)
        {
            if(self.cancelImport)
            {
                self.cancelImport = NO;
                break;
            }
                                
            // NSManagedObjectContext wants main queue for Core Data store access
            dispatch_sync(dispatch_get_main_queue(), ^{                
                [self.database addGamesFromPath:[aURL path] toCollection:nil searchSubfolders:YES];
            });
            
            [importCurrentItem setStringValue:[[[aURL path] lastPathComponent] stringByDeletingPathExtension]];
            [importProgress incrementBy:1.0];
        }
        
        [NSApp endSheet:importSheet];
    });    
}

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{    
    [importSheet close];
}

- (IBAction) cancelImport:(id)sender
{
    self.cancelImport = YES;
}

#pragma mark -

- (IBAction)controlsmenu_startGame:(id)sender{
	NSArray* selection = [collectionViewController selectedGames];
	if(!selection) return;
	
	// TODO: if multiple games are selected we could just start them all... if we want to
	OEDBGame* selectedGame = [selection lastObject];
	if(selectedGame){
		NSSet* roms = [selectedGame valueForKey:@"roms"];
		id romToStart = nil;
		if([roms count] > 1){
			// TODO: find out which rom to start			
		} else {
			romToStart = [roms anyObject];	
		}

		NSError* error = nil;
		OENewGameDocument* gameDoc = [OENewGameDocument newDocumentWithROM:romToStart error:&error];
		if(!gameDoc){
			[NSApp presentError:error];
			return;
		}
		[self _launchGameDoc:gameDoc];
		[gameDoc release];
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
- (NSSize)window:(NSWindow *)window willUseFullScreenContentSize:(NSSize)proposedSize {
	return proposedSize;
}
- (void)window:(NSWindow *)window willEncodeRestorableState:(NSCoder *)state{
}
- (void)window:(NSWindow *)window didDecodeRestorableState:(NSCoder *)state{
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification{
	[self _showFullscreen:YES animated:YES];
}

- (void)windowWillExitFullScreen:(NSNotification *)notification{
	[self _showFullscreen:NO animated:YES];
}

- (void)windowDidFailToExitFullScreen:(NSWindow *)window{
	[self _showFullscreen:YES animated:NO];
}
- (void)windowDidFailToEnterFullScreen:(NSWindow *)window{
	[self _showFullscreen:NO animated:NO];
}

#pragma mark -
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
- (void)setDatabase:(OELibraryDatabase *)newDatabase{
    [newDatabase retain];
    [database release];
    
    database = newDatabase;
    [sidebarController setDatabase:database];
    
    [sidebarController reloadData];	
}

- (void)setSidebarChangesWindowSize:(BOOL)flag{
	sidebarChangesWindowSize = flag;
	
	NSImage* image;
	if(flag){
		image = [mainSplitView splitterPosition]==0? [NSImage imageNamed:@"toolbar_sidebar_button_open"]:[NSImage imageNamed:@"toolbar_sidebar_button_close"];
	} else {
		image = [mainSplitView splitterPosition]==0? [NSImage imageNamed:@"toolbar_sidebar_button_close"]:[NSImage imageNamed:@"toolbar_sidebar_button_open"];
	}
   
    [sidebarBtn setImage:image];
}

- (BOOL)sidebarChangesWindowSize{
	return sidebarChangesWindowSize;
}

#pragma mark -
#pragma mark Private
- (void)_showFullscreen:(BOOL)fsFlag animated:(BOOL)animatedFlag{
	[[self window] setMovable:!fsFlag];
	[self setSidebarChangesWindowSize:!fsFlag];
	mainSplitView.drawsWindowResizer = !fsFlag;
	
	if(fsFlag){
		#ifdef NSApplicationPresentationAutoHideToolbar
		[NSApp setPresentationOptions:NSApplicationPresentationAutoHideDock|NSApplicationPresentationAutoHideToolbar];
		#else
		
		#endif
	} else {
		#ifdef NSApplicationPresentationAutoHideToolbar
		[NSApp setPresentationOptions:NSApplicationPresentationDefault];	
		#else	
		
		#endif
	}
}

- (void)_removeGameView{
	[collectionViewController willShow];
	[[collectionViewController view] setHidden:NO];
	
	[sidebarController willShow];
	[[sidebarController view] setHidden:NO];
	[gridViewBtn setEnabled:YES];
	[flowViewBtn setEnabled:YES];
	[listViewBtn setEnabled:YES];
	
	[editSmartCollectionMenuItem setEnabled:YES];
	[sidebarBtn setEnabled:YES];
	
	if(gameDocument){
		[gameDocument terminateEmulation];
		[[gameDocument gameView] removeFromSuperview];
		[gameDocument release], gameDocument = nil;
    }
}

- (void)_showGameView{
	[collectionViewController willHide];
	[[collectionViewController view] setHidden:YES];
	
	[sidebarController willHide];
	[[sidebarController view] setHidden:YES];
	[gridViewBtn setEnabled:NO];
	[flowViewBtn setEnabled:NO];
	[listViewBtn setEnabled:NO];
	
	[editSmartCollectionMenuItem setEnabled:NO];
	[sidebarBtn setEnabled:NO];
	
	NSView* gameView = [gameDocument gameView];
	[[mainSplitView superview] addSubview:gameView];
	
    NSRect frame = [[mainSplitView superview] bounds];
    frame.origin.y += 45; // Toolbar Height+1px black line;
    frame.size.height -= 45;
    [gameView setFrame:frame];
    [[self window] makeFirstResponder:gameView];
}

- (void)_launchGameDoc:(id)gameDoc{
	NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
	BOOL allowsPopout = [sud boolForKey:UDAllowPopoutKey];
	BOOL forcePopout = YES;
	
	if (allowsPopout && (gameDocument || forcePopout)) {
		NSRect rect = NSMakeRect(150, 150, 640, 480);
		[gameDoc openWindow:rect];
	} else {
		[self _removeGameView];
		
		gameDocument = [gameDoc retain];
		[self _showGameView];
	}
}


@synthesize mainSplitView, database;
@synthesize cancelImport;

@end
