//
//  CollectionViewController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECollectionViewController.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OELibraryController.h"
#import "OECoverGridForegroundLayer.h"
#import "OECoverGridItemLayer.h"

#import "OEListViewDataSourceItem.h"
#import "OERatingCell.h"
#import "OEHorizontalSplitView.h"

#import "OECoverGridDataSourceItem.h"
#import "OEGridBlankSlateView.h"

#import "OEDBSystem.h"
#import "OESystemPlugin.h"
@interface OECollectionViewController (Private)
- (void)_reloadData;
- (void)_selectView:(int)view;
@end

@implementation OECollectionViewController
@synthesize libraryController;
@synthesize emptyCollectionView, emptyConsoleView;
+ (void)initialize
{
    // Indicators for list view
    NSImage* image = [NSImage imageNamed:@"list_indicators"];
    
    // unselected states
    [image setName:@"list_indicators_playing" forSubimageInRect:NSMakeRect(0, 24, 12, 12)];
    [image setName:@"list_indicators_missing" forSubimageInRect:NSMakeRect(0, 12, 12, 12)];
    [image setName:@"list_indicators_unplayed" forSubimageInRect:NSMakeRect(0, 0, 12, 12)];
    
    // selected states
    [image setName:@"list_indicators_playing_selected" forSubimageInRect:NSMakeRect(12, 24, 12, 12)];
    [image setName:@"list_indicators_missing_selected" forSubimageInRect:NSMakeRect(12, 12, 12, 12)];
    [image setName:@"list_indicators_unplayed_selected" forSubimageInRect:NSMakeRect(12, 0, 12, 12)];
    
    // selection effects for grid view
    image = [NSImage imageNamed:@"selector_ring"];
    [image setName:@"selector_ring_active" forSubimageInRect:NSMakeRect(0, 0, 29, 29)];
    [image setName:@"selector_ring_inactive" forSubimageInRect:NSMakeRect(29, 0, 29, 29)];
}

- (void)dealloc
{
    [collectionItem release], collectionItem = nil;
    [gamesController release], gamesController = nil;
    
    [super dealloc];
}
#pragma mark -
#pragma mark View Controller Stuff
- (void)awakeFromNib
{
    [gamesController setUsesLazyFetching:YES];
}

- (NSString*)nibName
{
    return @"CollectionView";
}

- (void)finishSetup
{
    if(gamesController!=nil) return;
    
    NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
    
    // Set up games controller
    gamesController = [[NSArrayController alloc] init];
    [gamesController setAutomaticallyRearrangesObjects:YES];
    [gamesController setAutomaticallyPreparesContent:YES];
    
    NSManagedObjectContext* context = [[[self libraryController] database] managedObjectContext];
    //[gamesController bind:@"managedObjectContext" toObject:context withKeyPath:@"" options:nil];
    
    [gamesController setManagedObjectContext:context];
    [gamesController setEntityName:@"Game"];
    [gamesController setSortDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"name" ascending:YES]]];
    [gamesController setFetchPredicate:[NSPredicate predicateWithValue:NO]];
    [gamesController prepareContent];
    
    // Setup View
    [[self view] setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    
    // Set up GridView
    [gridView setCellClass:[OECoverGridItemLayer class]];
    [gridView setItemSize:NSMakeSize(168, 193)];
    [gridView setMinimumSpacing:NSMakeSize(22, 29)];
    [gridView setDelegate:self];
    [gridView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSPasteboardTypePNG, NSPasteboardTypeTIFF, nil]];
    [gridView setDataSource:self];
    
    OECoverGridForegroundLayer *foregroundLayer = [OECoverGridForegroundLayer layer];
    [gridView addForegroundLayer:foregroundLayer];
    
    //set initial zoom value
    NSSlider* sizeSlider = [[[self libraryController] windowController] toolbarSlider];
    if([userDefaults valueForKey:UDLastGridSizeKey])
    {
        [sizeSlider setFloatValue:[userDefaults floatForKey:UDLastGridSizeKey]];
    }
    [sizeSlider setContinuous:YES];
    [self changeGridSize:sizeSlider];
    
    // set up flow view
    [coverFlowView setDelegate:self];
    [coverFlowView setDataSource:self];
    [coverFlowView setCellsAlignOnBaseline:YES];
    [coverFlowView setCellBorderColor:[NSColor blueColor]];
    
    // Set up list view
    [listView setDelegate:self];
    [listView setDataSource:self];
    [listView registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
    
    switch ([userDefaults integerForKey:UDLastCollectionViewKey]) {
        case 0:
            [self selectGridView:self];
            break;
        case 1:
            [self selectFlowView:self];
            break;
        case 2:
            [self selectListView:self];
            break;
        default:
            [self selectGridView:self];
            break;
    }
    
    [self _reloadData];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameAddedToLibrary:) name:@"OEDBGameAdded" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameAddedToLibrary:) name:@"OEDBStatusChanged" object:nil];
}
#pragma mark -
- (NSArray*)selectedGames
{
    return [gamesController selectedObjects];
}

#pragma mark -
#pragma mark View Selection
- (IBAction)selectGridView:(id)sender
{
    [self _selectView:0];
}

- (IBAction)selectFlowView:(id)sender
{
    [self _selectView:1];
}

- (IBAction)selectListView:(id)sender
{ 
    [self _selectView:2];
}


- (void)_selectView:(int)view
{
    NSSlider* sizeSlider = [[[self libraryController] windowController] toolbarSlider];
    
    NSMenu* mainMenu = [NSApp mainMenu];
    NSMenu* viewMenu = [[mainMenu itemAtIndex:3] submenu];
    
    [[[[self libraryController] windowController] toolbarGridViewButton] setState: NSOffState];
    [[viewMenu itemWithTag:MainMenu_View_GridViewTag] setState:NSOffState];
    [[[[self libraryController] windowController] toolbarFlowViewButton] setState: NSOffState];
    [[viewMenu itemWithTag:MainMenu_View_FlowViewTag] setState:NSOffState];
    [[[[self libraryController] windowController] toolbarListViewButton] setState: NSOffState];
    [[viewMenu itemWithTag:MainMenu_View_ListViewTag] setState:NSOffState];
    
    NSView* nextView = nil;
    float splitterPosition =-1;
    switch (view)
    {
        case 0: ;// Grid View
            [[[[self libraryController] windowController] toolbarGridViewButton] setState: NSOnState];
            [[viewMenu itemWithTag:MainMenu_View_GridViewTag] setState:NSOnState];
            nextView = gridViewContainer;
            [sizeSlider setEnabled:YES];
            break;
        case 1: ;// CoverFlow View
            [[[[self libraryController] windowController] toolbarFlowViewButton] setState: NSOnState];
            [[viewMenu itemWithTag:MainMenu_View_FlowViewTag] setState:NSOnState];
            nextView = flowlistViewContainer;
            [sizeSlider setEnabled:NO];
            
            // Set Splitter Position
            splitterPosition = 500;
            break;
        case 2: ;// List View
            [[[[self libraryController] windowController] toolbarListViewButton] setState: NSOnState];
            [[viewMenu itemWithTag:MainMenu_View_ListViewTag] setState:NSOnState];
            nextView = flowlistViewContainer;
            [sizeSlider setEnabled:NO];
            
            // Set Splitter position
            splitterPosition = 0;
            break;
        default: return;
    }
    
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithInt:view] forKey:UDLastCollectionViewKey];
    
    if(splitterPosition!=-1) [flowlistViewContainer setSplitterPosition:splitterPosition animated:NO];
    
    if(!nextView || [nextView superview]!=nil)
        return;
    
    while([[[self view] subviews] count]!=0)
    {
        NSView* currentSubview = [[[self view] subviews] objectAtIndex:0];
        [currentSubview removeFromSuperview];
    }
    
    
    [[self view] addSubview:nextView];
    [nextView setFrame:[[self view] bounds]];
}
#pragma mark -
- (void)willShow
{
    [listView setEnabled:YES];
}

- (void)gameAddedToLibrary:(NSNotification*)notification
{
    if(![NSThread isMainThread])
    {
        [self performSelectorOnMainThread:@selector(gameAddedToLibrary:) withObject:notification waitUntilDone:NO];
        return;
    }
    
    [self setNeedsReload];
}

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)search:(id)sender
{
    NSPredicate* pred = [[sender stringValue] isEqualToString:@""]?nil:[NSPredicate predicateWithFormat:@"name contains[cd] %@", [sender stringValue]];
    [gamesController setFilterPredicate:pred];
    
    [listView reloadData];
    [coverFlowView reloadData];
    [gridView reloadData];
}
- (IBAction)changeGridSize:(id)sender
{
    float zoomValue = [sender floatValue];
    [gridView setItemSize:NSMakeSize(roundf(26+142*zoomValue), roundf(44+7+142*zoomValue))];
    [[[gridView enclosingScrollView] verticalScroller] setNeedsDisplayInRect:[[[gridView enclosingScrollView] verticalScroller] bounds]];
    
    [[NSUserDefaults standardUserDefaults] setValue:[NSNumber numberWithFloat:zoomValue] forKey:UDLastGridSizeKey];
}

#pragma mark -
#pragma mark Property Getters / Setters
- (void)setCollectionItem:(id <NSObject, OECollectionViewItemProtocol>)_collectionItem
{
    [_collectionItem retain];
    [collectionItem release];
    
    collectionItem = _collectionItem;
    
    [self _reloadData];
}

- (id)collectionItem
{
    return collectionItem;
}


#pragma mark -
#pragma mark GridView Delegate
- (void)gridView:(IKSGridView*)aGridView selectionChanged:(NSArray*)selectedItems
{
    [gamesController setSelectionIndexes:[aGridView selectedIndexes]];
}

- (void)gridView:(IKSGridView*)gridView itemsMagnifiedToSize:(NSSize)newSize
{}

#pragma mark -
#pragma mark Grid View DataSource
- (NSUInteger)numberOfItemsInGridView:(IKSGridView*)aView
{
    return [[gamesController arrangedObjects] count];
}

- (id)gridView:(IKSGridView*)aView objectValueOfItemAtIndex:(NSUInteger)index
{    NSManagedObjectID* objid = [(NSManagedObject*)[[gamesController arrangedObjects] objectAtIndex:index] objectID];
    return [[[[self libraryController] database] managedObjectContext] objectWithID:objid];
}

- (void)gridView:(IKSGridView *)aView setObject:(id)val forKey:(NSString*)key withRepresentedObject:(id)obj
{      
    OELibraryDatabase* database = [[self libraryController] database];
    NSManagedObjectContext* moc = [database managedObjectContext];
    
    NSManagedObjectID* objId = (NSManagedObjectID*)obj;
    id <OECoverGridDataSourceItem> object = (id <OECoverGridDataSourceItem>)[moc objectWithID:objId];
    
    if([key isEqualTo:@"rating"])
    {
        [object setGridRating:[val unsignedIntegerValue]];
    } 
    else if([key isEqualTo:@"title"])
    {
        [object setGridTitle:val];
    }
    [moc save:nil];
}

- (id)gridView:(IKSGridView *)aView objectValueForKey:(NSString*)key withRepresentedObject:(id)obj
{
    OELibraryDatabase* database = [[self libraryController] database];
    NSManagedObjectContext* moc = [database managedObjectContext];
    NSManagedObjectID* objId = (NSManagedObjectID*)obj;
#warning What if objID is nil????
    id <OECoverGridDataSourceItem> object = (id <OECoverGridDataSourceItem>)[moc objectWithID:objId];
    if([key isEqualTo:@"status"])
    {
        return [NSNumber numberWithInt:[object gridStatus]];
    }
    else if([key isEqualTo:@"image"])
    {
        return [object gridImageWithSize:[gridView itemSize]];
    } 
    else if([key isEqualTo:@"title"])
    {
        return [object gridTitle];
    } 
    else if([key isEqualTo:@"rating"])
    {
        return [NSNumber numberWithInt:[object gridRating]];
    }
    
    return object;
}
- (NSView*)gridViewNoItemsView:(IKSGridView*)gridView
{
    if([collectionItem isKindOfClass:[OEDBSystem class]])
    {
        return [[[OEGridBlankSlateView alloc] initWithSystemPlugin:[(OEDBSystem*)collectionItem plugin]] autorelease];
    }
    
    if([collectionItem respondsToSelector:@selector(collectionViewName)])
    {
        return [[[OEGridBlankSlateView alloc] initWithCollectionName:[collectionItem collectionViewName]] autorelease];
    }
    return nil;
}
#pragma mark -
#pragma mark NSTableView DataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if( aTableView == listView)
    {
        return [[gamesController arrangedObjects] count];
    }
    
    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    
    if( aTableView == listView)
    {
        /*NSManagedObject* manobj = [[gamesController arrangedObjects] objectAtIndex:rowIndex];
         NSManagedObjectID* objID = [manobj objectID];
         
         NSManagedObjectContext* context = [[NSManagedObjectContext alloc] init];
         [context setPersistentStoreCoordinator:[[manobj managedObjectContext] persistentStoreCoordinator]];
         */
        id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:rowIndex];//(id <ListViewDataSourceItem>)[context objectWithID:objID];
        
        NSString* colIdent = [aTableColumn identifier];
        id result = nil;
        if([colIdent isEqualToString:@"romStatus"])
        {
            result = [obj listViewStatus:([aTableView selectedRow]==rowIndex)];
        } 
        else if([colIdent isEqualToString:@"romName"])
        {
            result = [obj listViewTitle];
        } 
        else if([colIdent isEqualToString:@"romRating"])
        {
            result = [obj listViewRating];
        } 
        else if([colIdent isEqualToString:@"romLastPlayed"])
        {
            result = [obj listViewLastPlayed];
        }
        else if([colIdent isEqualToString:@"romConsole"])
        {
            result = [obj listViewConsoleName];
        }
        
        //[context release];
        return result;
    }
    
    return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    
    if( aTableView == listView)
    {
        id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:rowIndex];
        if([[aTableColumn identifier] isEqualTo:@"romRating"])
        {
            [obj setListViewRating:anObject];
        }
        return;
    }
    
    
    return;
}

- (void)tableView:(NSTableView *)aTableView sortDescriptorsDidChange:(NSArray *)oldDescriptors
{
    if( aTableView==listView )
    {
    }
}
#pragma mark -
#pragma mark TableView Drag and Drop 
- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    
    if( aTableView == listView && operation==NSTableViewDropAbove)
        return YES;
    
    return NO;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    
    if( aTableView == listView && operation==NSTableViewDropAbove)
        return NSDragOperationGeneric;
    
    return NSDragOperationNone;
    
}


- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
    
    if( aTableView == listView )
    {
        [rowIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) 
         {
             id <OEListViewDataSourceItem> obj = [[gamesController arrangedObjects] objectAtIndex:idx];
             [pboard writeObjects:[NSArray arrayWithObject:obj]];
         }];
        
        return YES;
    }
    
    return NO;
}

#pragma mark -
#pragma mark NSTableView Delegate
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(aTableView == listView)
    {
        if([aCell isKindOfClass:[NSTextFieldCell class]])
        {
            NSDictionary* attr;
            
            if([[aTableView selectedRowIndexes] containsIndex:rowIndex])
            {
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

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if( aTableView == listView )
    {
        if([[aTableColumn identifier] isEqualTo:@"romRating"]) return NO;
        
        return YES;
    }
    
    return NO;
}

- (BOOL)selectionShouldChangeInTableView:(NSTableView *)aTableView
{
    
    if( aTableView == listView )
    {
        return YES;
    }
    
    
    return YES;
}

- (CGFloat)tableView:(NSTableView *)aTableView heightOfRow:(NSInteger)row
{
    if( aTableView == listView )
    {
        return 17.0;
    }
    
    return 0.0;
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
    if( aTableView == listView ){
        return YES;
    }
    
    return YES;
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    NSTableView* aTableView = [aNotification object];
    
    if( aTableView == listView )
    {
        NSIndexSet* selectedIndexes = [listView selectedRowIndexes];
        
        [coverFlowView setSelectedIndex:[selectedIndexes firstIndex]];
        
        [selectedIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
            [listView setNeedsDisplayInRect:[listView rectOfRow:idx]];
        }];
        
        return;
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if( tableView == listView && [[tableColumn identifier] isEqualTo:@"romRating"] )
    {
        return YES;
    }
    return NO;
}
#pragma mark -
#pragma mark ImageFlow Data Source
- (NSUInteger)numberOfItemsInImageFlow:(IKImageFlowView *)aBrowser
{
    return [[gamesController arrangedObjects] count];
}

- (id)imageFlow:(id)aFlowLayer itemAtIndex:(int)index
{
    return [[gamesController arrangedObjects] objectAtIndex:index];
}


#pragma mark -
#pragma mark ImageFlow Delegates
- (NSUInteger)imageFlow:(IKImageFlowView *)browser writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard{ return 0; }
- (void)imageFlow:(IKImageFlowView *)sender removeItemsAtIndexes:(NSIndexSet *)indexes
{}
- (void)imageFlow:(IKImageFlowView *)sender cellWasDoubleClickedAtIndex:(NSInteger)index
{
    // seems like this method is no longer called in lion
}

- (void)imageFlow:(IKImageFlowView *)sender didSelectItemAtIndex:(NSInteger)index
{    
    [listView selectRowIndexes:[NSIndexSet indexSetWithIndex:[sender selectedIndex]] byExtendingSelection:NO];
}

#pragma mark -
#pragma mark Private
#define reloadDelay 0.1
- (void)setNeedsReload{
    if(reloadTimer) return;
    
    reloadTimer = [NSTimer scheduledTimerWithTimeInterval:reloadDelay target:self selector:@selector(_reloadData) userInfo:nil repeats:NO];                 
    [reloadTimer retain];
}
- (void)_reloadData
{
    if(reloadTimer){
        [reloadTimer invalidate];
        [reloadTimer release];
        reloadTimer = nil;       
    }    
    if(!gamesController) return;
    
    NSPredicate* pred = self.collectionItem?[self.collectionItem predicate]:[NSPredicate predicateWithValue:NO];
    [gamesController setFetchPredicate:pred];
    
    NSError *error = nil;
    BOOL ok = [gamesController fetchWithRequest:nil merge:NO error:&error];
    if(!ok)
    {
        NSLog(@"Error while fetching: %@", error);
        return;
    }
    
    
    [gridView reloadData];
    [listView reloadData];
    [coverFlowView reloadData];
}

#pragma mark -
#pragma mark Debug Actions
/*
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
 */
@end
