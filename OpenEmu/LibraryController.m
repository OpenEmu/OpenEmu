//
//  LibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import "LibraryController.h"
#import "LibraryDatabase.h"
#import "SidebarController.h"
#import "LibrarySplitView.h"

#import "CoverGridForegroundLayer.h"
#import "CoverGridBackgroundLayer.h"

#import "OEDBConsole.h"
#import "OEDBRom.h"

#import "IKImageFlowView.h"

#import "SidebarGroupItem.h"
#import "SidebarDataSourceItem.h"
#import "ListViewDataSourceItem.h"

#import "CoverGridItemLayer.h"

#import "OERatingCell.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation LibraryController

+ (void)initialize{
	// Collection Icons for sidebar
	NSImage* image = [NSImage imageNamed:@"collections"];
	
	[image setName:@"collections_simple" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];
	[image setName:@"collections_smart" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
	
	// Indicators for list view
	image = [NSImage imageNamed:@"list_indicators"];
	
	// unselected states
	[image setName:@"list_indicators_playing" forSubimageInRect:NSMakeRect(0, 24, 12, 12)];
	[image setName:@"list_indicators_missing" forSubimageInRect:NSMakeRect(0, 12, 12, 12)];
	[image setName:@"list_indicators_unplayed" forSubimageInRect:NSMakeRect(0, 0, 12, 12)];
	
	// selected states
	[image setName:@"list_indicators_playing_selected" forSubimageInRect:NSMakeRect(12, 24, 12, 12)];
	[image setName:@"list_indicators_missing_selected" forSubimageInRect:NSMakeRect(12, 12, 12, 12)];
	[image setName:@"list_indicators_unplayed_selected" forSubimageInRect:NSMakeRect(12, 0, 12, 12)];
	
	// toolbar sidebar button
	image = [NSImage imageNamed:@"toolbar_sidebar_button"];
	[image setName:@"toolbar_sidebar_button_close" forSubimageInRect:NSMakeRect(0, 23, 84, 23)];
	[image setName:@"toolbar_sidebar_button_open" forSubimageInRect:NSMakeRect(0, 0, 84, 23)];
	
	// toolbar view buttons
	image = [NSImage imageNamed:@"toolbar_view_buttons"];
	[image setName:@"toolbar_view_button_grid" forSubimageInRect:NSMakeRect(0, 0, 27, 115)];
	[image setName:@"toolbar_view_button_flow" forSubimageInRect:NSMakeRect(27, 0, 27, 115)];
	[image setName:@"toolbar_view_button_list" forSubimageInRect:NSMakeRect(54, 0, 27, 115)];
	
	
	// selection ring effects for grid view
	image = [NSImage imageNamed:@"selector_ring"];
	
	[image setName:@"selector_ring_active" forSubimageInRect:NSMakeRect(0, 0, 29, 29)];
	[image setName:@"selector_ring_inactive" forSubimageInRect:NSMakeRect(29, 0, 29, 29)];
}
- (void)dealloc{
	[romsController release];
	
    [super dealloc];
}

#pragma mark -
- (void)reloadData{
	[gridView reloadData];
	[sidebarController reloadData];
}
#pragma mark -
#pragma mark WindowController stuff
- (NSString*)windowNibName{
	return @"Library";
}

- (void)windowDidLoad{
    [super windowDidLoad];
    
    // Set up window
    [[self window] setOpaque:NO];
    [[self window] setBackgroundColor:[NSColor clearColor]];
    [[self window] setShowsResizeIndicator:NO];
    [(OEBackgroundColorView*)[[self window] contentView] setBackgroundColor:[NSColor clearColor]];
	
	// setup splitview
//	[mainSplitView setDelegate:self];
	
	// Set up roms controller
	romsController = [[NSArrayController alloc] init];
	[romsController setAutomaticallyRearrangesObjects:YES];
	[romsController setSortDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"title" ascending:YES]]];
	allRomsArranged = NO;
	
	// Set up toolbar buttons
	
	// Set up Add Collection Button
	[addCollectionBtn setImage:[NSImage imageNamed:@"toolbar_add_button"]];
	
	// Set up Toggle Sidebar Button
	NSImage* image = [mainSplitView splitterPosition]==0? [NSImage imageNamed:@"toolbar_sidebar_button_open"]:[NSImage imageNamed:@"toolbar_sidebar_button_close"];
	[sidebarBtn setImage:image];
	
	[gridViewBtn setImage:[NSImage imageNamed:@"toolbar_view_button_grid"]];
	[flowViewBtn setImage:[NSImage imageNamed:@"toolbar_view_button_flow"]];
	[listViewBtn setImage:[NSImage imageNamed:@"toolbar_view_button_list"]];
	
	// Set up GridView	
	// foreground layer
	CoverGridForegroundLayer *foregroundLayer = [CoverGridForegroundLayer layer];
	[gridView addForegroundLayer:foregroundLayer];
	
	// background layer
	CoverGridBackgroundLayer *backgroundLayer = [CoverGridBackgroundLayer layer];
	[gridView addBackgroundLayer:backgroundLayer];
	
	//set initial zoom value
	//	[sizeSlider setFloatValue:0.5];
	[sizeSlider setContinuous:YES];
	
	//[gridView setItemLayerDelegate:[CoverGridPainter sharedPainter]];
	[gridView setCellClass:[CoverGridItemLayer class]];
	[gridView setItemSize:NSMakeSize(168, 193)];
	[gridView setMinimumSpacing:NSMakeSize(22, 29)];
	[gridView setDelegate:self];
	[gridView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSPasteboardTypePNG, NSPasteboardTypeTIFF, nil]];
	[gridView setDataSource:self];
	
	[self changeViewSize:sizeSlider];
	
	// set up flow view
	[coverFlowView setDelegate:self];
	[coverFlowView setDataSource:self];
	
	// Set up list view
	[listView setDelegate:self];	
	[listView setDataSource:self];
	[listView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
	
	// setup sidebar controller
	[sidebarController setDatabase:self.database];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sidebarSelectionDidChange:) name:@"SidebarSelectionChanged" object:sidebarController];
	
	// Select first view
	// to do: restore the last used view!
	[self selectView:gridViewBtn];
	
	
	[self setupDebug];
	
	[self reloadData];
}

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)addCollection:(id)sender{}

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
	// To do: store position
}

- (IBAction)selectView:(id)sender{
	[gridViewBtn setState:0];
	[listViewBtn setState:0];
	[flowViewBtn setState:0];
	
	[sender setState:1]; 
	
	
	NSView* nextView = nil;
	if( sender == gridViewBtn ){
		nextView = gridViewContainer;
		
		// Enable size slider
		[sizeSlider setEnabled:YES];
		
	} else {
		
		nextView = flowlistViewContainer;		
		
		// To do: get slider value for listview invisible
		float sliderPos = 0;
		if( sender == flowViewBtn ) sliderPos = 500;
		
		[flowlistViewContainer setSplitterPosition:sliderPos animated:NO];
		
		// Disable size slider
		[sizeSlider setEnabled:NO];
	}
	
	if(!nextView || [nextView superview]!=nil)
		return;
	
	if([[gameViewContainer subviews] count]!=0){
		NSView* currentSubview = [[gameViewContainer subviews] objectAtIndex:0];
		[currentSubview removeFromSuperview];
	}
	
	[gameViewContainer addSubview:nextView];
	[nextView setFrame:[gameViewContainer bounds]];
}

- (IBAction)search:(id)sender{	
	NSPredicate* pred = [[sender stringValue] isEqualToString:@""]?nil:[NSPredicate predicateWithFormat:@"title contains[cd] %@", [sender stringValue]];
	[romsController setFilterPredicate:pred];
	
	[listView reloadData];
	[coverFlowView reloadData];
	[gridView reloadData];
}

- (IBAction)changeViewSize:(id)sender{
	float zoomValue = [sender floatValue];

	[gridView setItemSize:NSMakeSize(roundf(26+142*zoomValue), roundf(44+7+142*zoomValue))];
	
	[[[gridView enclosingScrollView] verticalScroller] setNeedsDisplayInRect:[[[gridView enclosingScrollView] verticalScroller] bounds]];
}
#pragma mark -
#pragma mark Sidebar Helpers
- (void)sidebarSelectionDidChange:(NSNotification*)notification{
	NSDictionary* userInfo = [notification userInfo];
	id collection = [userInfo objectForKey:@"selectedCollection"];
	
	[romsController setFilterPredicate:nil];
	[romsController removeObjects:[romsController arrangedObjects]];
	[searchField setStringValue:@""];
	
	if([[collection className] isEqualTo:@"OEDBConsole"]){
		NSPredicate* consolePredicate = [NSPredicate predicateWithFormat:@"consoleName == %@", [collection name]];
		[romsController addObjects:[database romsForPredicate:consolePredicate]];
	} else {
		if([collection respondsToSelector:@selector(predicate)]){
			[romsController addObjects:[database romsForPredicate:[collection predicate]]];
		} else {
			[romsController addObjects:[database romsInCollection:collection]];
		}
	}
	
	[gridView reloadData];
	[listView reloadData];
	[coverFlowView reloadData];
}
#pragma mark -
#pragma mark NSSplitView Delegate
- (BOOL)splitView:(NSSplitView *)splitView canCollapseSubview:(NSView *)subview{
	
	if( splitView == mainSplitView ){
		return [[mainSplitView subviews] indexOfObject:subview] == 0;
	}
	
	if( splitView == flowlistViewContainer ){
		return YES;
	}

	return NO;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex{
	if( splitView == mainSplitView ){
		return [mainSplitView isSubviewCollapsed:[[splitView subviews] objectAtIndex:0]];
	}
	
	if( splitView == flowlistViewContainer ){
		return NO;
	}

	return NO;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldCollapseSubview:(NSView *)subview forDoubleClickOnDividerAtIndex:(NSInteger)dividerIndex{
	
	if( splitView == mainSplitView ){
		return subview == [[splitView subviews] objectAtIndex:0];
	}
	
	if( splitView == flowlistViewContainer ){
		return NO;
	}
	
	return NO;
}
#pragma mark -
#pragma mark NSTableView DataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView{
	
	if( aTableView == listView){
		return [[romsController arrangedObjects] count];
	}
	
	
	return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex{
		
	if( aTableView == listView){
		id <ListViewDataSourceItem> obj = [[romsController arrangedObjects] objectAtIndex:rowIndex];
		
		NSString* colIdent = [aTableColumn identifier];
		
		if([colIdent isEqualToString:@"romStatus"]){
			return [obj listViewStatus:([aTableView selectedRow]==rowIndex)];
		}
		
		if([colIdent isEqualToString:@"romName"]){
			return [obj listViewTitle];
		}
		
		if([colIdent isEqualToString:@"romRating"]){
			return [obj listViewRating];
		}
		
		if([colIdent isEqualToString:@"romLastPlayed"]){
			return [obj listViewLastPlayed];			
		}
		
		if([colIdent isEqualToString:@"romConsole"]){
			return [obj listViewConsoleName];
		}
		
		return nil;
	}
	
	
	return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex{
	
	if( aTableView == listView){
		id <ListViewDataSourceItem> obj = [[romsController arrangedObjects] objectAtIndex:rowIndex];
		if([[aTableColumn identifier] isEqualTo:@"romRating"]){
			[obj setListViewRating:anObject];
		}
		
		return;
	}
	
	
	return;
}

- (void)tableView:(NSTableView *)aTableView sortDescriptorsDidChange:(NSArray *)oldDescriptors{
	if( aTableView==listView ){
	}
}
#pragma mark -
#pragma mark TableView Drag and Drop 
- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation{
	
	if( aTableView == listView && operation==NSTableViewDropAbove)
		return YES;
		
	return NO;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation{
	
	if( aTableView == listView && operation==NSTableViewDropAbove)
		return NSDragOperationGeneric;
	
	return NSDragOperationNone;
	
}


- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard{
	
	if( aTableView == listView ){
		[rowIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
			id <ListViewDataSourceItem> obj = [[romsController arrangedObjects] objectAtIndex:idx];
			[pboard writeObjects:[NSArray arrayWithObject:obj]];			
		}];
		
		return YES;
	}
	
	return NO;
}

#pragma mark -
#pragma mark NSTableView Delegate
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex{
	
	if(aTableView == listView){
		
		if([aCell isKindOfClass:[NSTextFieldCell class]]){
			NSDictionary* attr;
			
			if([[aTableView selectedRowIndexes] containsIndex:rowIndex]){
				attr = [NSDictionary dictionaryWithObjectsAndKeys:
						[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:9 size:11.0], NSFontAttributeName, 
						[NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName, nil];		
			} else {
				attr = [NSDictionary dictionaryWithObjectsAndKeys:
						[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:7 size:11.0], NSFontAttributeName, 
						[NSColor colorWithDeviceWhite:1.0 alpha:1.0], NSForegroundColorAttributeName, nil];			
			}
			
			[aCell setAttributedStringValue:[[[NSAttributedString alloc] initWithString:[aCell stringValue] attributes:attr] autorelease]];
		}
		
		if(![aCell isKindOfClass:[OERatingCell class]])
			[aCell setHighlighted:NO];
	}
	
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex{
	if( aTableView == listView){
		if([[aTableColumn identifier] isEqualTo:@"romRating"]) return NO;
		
		return YES;
	}
	
	return NO;	
}

- (BOOL)selectionShouldChangeInTableView:(NSTableView *)aTableView{
	
	if( aTableView == listView){
		return YES;
	}
	
	
	return YES;
}

- (CGFloat)tableView:(NSTableView *)aTableView heightOfRow:(NSInteger)row{	
	if( aTableView == listView){
		return 17.0;
	}
	
	return 0.0;
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex{
	if( aTableView == listView){
		return YES;
	}
	
	return YES;
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification{
	NSTableView* aTableView = [aNotification object];
		
	if( aTableView == listView){
		NSIndexSet* selectedIndexes = [listView selectedRowIndexes];
		
		[coverFlowView setSelectedIndex:[selectedIndexes firstIndex]];
		
		[selectedIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
			[listView setNeedsDisplayInRect:[listView rectOfRow:idx]];
		}];
		
		
		return;
	}
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row{
	if( tableView == listView && [[tableColumn identifier] isEqualTo:@"romRating"]){
		return YES;
	}
	return NO;
}
#pragma mark -
#pragma mark GridView Stuff
- (void)gridView:(IKSGridView*)gridView selectionChanged:(NSArray*)selectedItems{
}

- (void)gridView:(IKSGridView*)gridView itemsMagnifiedToSize:(NSSize)newSize{
}
#pragma mark -
#pragma mark Grid View DataSource
- (NSUInteger)numberOfItemsInGridView:(IKSGridView*)aView{
	return [[romsController arrangedObjects] count];
}

- (id)gridView:(IKSGridView*)aView objectValueOfItemAtIndex:(NSUInteger)index{
	return [[romsController arrangedObjects] objectAtIndex:index];
}

#pragma mark -
#pragma mark ImageFlow Data Source
- (NSUInteger)numberOfItemsInImageFlow:(IKImageFlowView *) aBrowser{
	return [[romsController arrangedObjects] count];
}

- (id)imageFlow:(id)aFlowLayer itemAtIndex:(int)index{
	return [[romsController arrangedObjects] objectAtIndex:index];
}


#pragma mark -
#pragma mark ImageFlow Delegates
- (NSUInteger)imageFlow:(IKImageFlowView *)browser writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard{ return 0; }

- (void)imageFlow:(IKImageFlowView *)sender removeItemsAtIndexes:(NSIndexSet *)indexes{}

- (void)imageFlow:(IKImageFlowView *)sender cellWasDoubleClickedAtIndex:(NSInteger)index{
}

- (void)imageFlow:(IKImageFlowView *)sender didSelectItemAtIndex:(NSInteger)index{
}

#pragma mark -
#pragma mark Debug Actions
- (void)setupDebug{
	NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
	[standardUserDefaults registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
											[NSNumber numberWithFloat:0.15], @"debug_drop_animation_delay",
											[NSNumber numberWithFloat:0.0], @"debug_startWidthFac",
											[NSNumber numberWithFloat:0.0], @"debug_opacityStart",
											[NSNumber numberWithFloat:1.0], @"debug_opacityEnd",
											nil]];
	
	[dbg_btn_colors setState:[standardUserDefaults boolForKey:@"debug_showGridFrameBorder"]];
	
	[dbg_dnd_delay setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_drop_animation_delay"]];
	[dbg_dnd_width setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_startWidthFac"]];
	[dbg_dnd_opend setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_opacityEnd"]];
	[dbg_dnd_opstart setFloatValue:[[NSUserDefaults standardUserDefaults] floatForKey:@"debug_opacityStart"]];
}

- (IBAction)debug_toggleGridFrame:(id)sender{
	[[NSUserDefaults standardUserDefaults] setBool:[(NSButton*)sender state]==NSOnState forKey:@"debug_showGridFrameBorder"];
}

- (IBAction)debug_setArrowsPosition:(id)sender{
	switch ([sender selectedTag]) {
		case 0: 
			[[NSUserDefaults standardUserDefaults] setInteger:NSScrollerArrowsNone forKey:@"debug_scrollbarArrowsPosition"];
			break;
		case 1:
			[[NSUserDefaults standardUserDefaults] setInteger:NSScrollerArrowsMaxEnd forKey:@"debug_scrollbarArrowsPosition"];
			break;
		case 2:
			[[NSUserDefaults standardUserDefaults] setInteger:3 forKey:@"debug_scrollbarArrowsPosition"];
			break;
		default:
			break;
	}
	
}

- (IBAction)debug_applyValues:(id)sender{
	[[NSUserDefaults standardUserDefaults] setFloat:[dbg_dnd_width floatValue] forKey:@"debug_startWidthFac"];
	[[NSUserDefaults standardUserDefaults] setFloat:[dbg_dnd_opstart floatValue] forKey:@"debug_opacityStart"];
	[[NSUserDefaults standardUserDefaults] setFloat:[dbg_dnd_opend floatValue] forKey:@"debug_opacityEnd"];
	[[NSUserDefaults standardUserDefaults] setFloat:[dbg_dnd_delay floatValue] forKey:@"debug_drop_animation_delay"];
}

- (IBAction)debug_layerState:(id)sender{
	int state = [sender selectedTag];
	
	NSIndexSet* selection = [self.gridView selectedIndexes];
	[selection enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
		OEDBRom* obj = [self gridView:self.gridView objectValueOfItemAtIndex:idx];
		
		if(state == 10){
			obj.coverArt = nil;
			obj.coverPath = nil;
		} else {
			obj.fileStatus = state;
		}
		
		[gridView reloadData];
	}];
}
#pragma mark -
#pragma mark Properties
- (void)setDatabase:(LibraryDatabase *)newDatabase{
	[newDatabase retain];
	[database release];
	
	database = newDatabase;
	[sidebarController setDatabase:database];

	[self reloadData];	
}
@synthesize mainSplitView, searchField, sizeSlider, addCollectionBtn, gridViewBtn, gridView, flowViewBtn, listViewBtn, flowlistViewContainer, gridViewContainer, coverFlowView, listView, gameViewContainer, database;
@end
