/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OESaveStateController.h"
#import "GameDocumentController.h"
#import <Quartz/Quartz.h>
#import "IKImageFlowView.h"
#import "OESaveState.h"
#import "OEROMFile.h"

@interface OEROMFile (SaveStateManagerExtras)

// This is necessary to work around an issue with array controllers
// and KVO.
- (NSSet *)allEmulatorIDsForSaveStates;

@end

@implementation OEROMFile (SaveStateManagerExtras)

- (NSSet *)allEmulatorIDsForSaveStates
{
    NSMutableSet *emulatorIDs = [NSMutableSet set];
    NSSet *saves = [self saveStates];
    for(OESaveState *state in saves) [emulatorIDs addObject:[state emulatorID]];
    
    return [[emulatorIDs copy] autorelease];
}

@end

@implementation OESaveStateController

@synthesize availablePluginsPredicate, selectedPlugins, selectedRomPredicate, browserZoom, sortDescriptors, romFileController, savestateController, pluginController, treeController, listView, collectionView, outlineView, imageBrowser, imageFlow, contextMenu, segmentButton, holderView, searchField;

- (id)init
{
    return [self initWithWindowNibName:@"SaveStateManager"];
}

- (id)initWithWindowNibName:(NSString *)windowNibName
{
    self = [super initWithWindowNibName:windowNibName];
    if(self != nil)
    {
        NSSortDescriptor *path = [[NSSortDescriptor alloc] initWithKey:@"romFile.path" ascending:NO];
        NSSortDescriptor *time = [[NSSortDescriptor alloc] initWithKey:@"timeStamp" ascending:YES];
        
        [self setSortDescriptors:[NSArray arrayWithObjects:path, time, nil]];
        
        [path release];
        [time release];

        romFileIndexes = [[NSMutableDictionary alloc] init];
        sortedSaveStates = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [romFileController         release];
    [savestateController       release];
    [pluginController          release];
    [treeController            release];
    [selectedRomPredicate      release];
    [listView                  release];
    [collectionView            release];
    [holderView                release];
    [outlineView               release];
    [imageBrowser              release];
    [imageFlow                 release];
    [searchField               release];
    [contextMenu               release];
    [segmentButton             release];
    [availablePluginsPredicate release];
    [sortDescriptors           release];
    [selectedPlugins           release];
    [romFileIndexes            release];
    [sortedSaveStates          release];
    [super                     dealloc];
}

- (IBAction)openSaveStateWindow:(id)sender
{
    // For this to be called means that the Save State Window is already
    // open.
    [self close];
}

static void *const OEContentChangedContext   = @"OEContentChangedContext";
static void *const OESelectionChangedContext = @"OESelectionChangedContext";

- (GameDocumentController *)docController
{
    return [GameDocumentController sharedDocumentController];
}

- (NSPredicate *)selectedPluginsPredicate
{
    return selectedPluginsPredicate;
}

- (void)updateSelectedPlugins
{
    NSArray        *selection     = [pluginController selectedObjects];
    NSMutableArray *subpredicates = [NSMutableArray arrayWithCapacity:[selection count]];
    
    for(OECorePlugin *plugin in selection)
        [subpredicates addObject:[NSPredicate predicateWithFormat:@"%@ IN allEmulatorIDsForSaveStates", [plugin displayName]]];
    
    NSPredicate *pluginsPredicate = [NSCompoundPredicate orPredicateWithSubpredicates:subpredicates];
    
    if(searchPredicate)
        pluginsPredicate = [NSCompoundPredicate andPredicateWithSubpredicates:[NSArray arrayWithObjects:pluginsPredicate, searchPredicate, nil]];
    
    [self willChangeValueForKey:@"selectedPluginsPredicate"];
    [selectedPluginsPredicate autorelease];
    selectedPluginsPredicate = [pluginsPredicate retain];;
    [self  didChangeValueForKey:@"selectedPluginsPredicate"];
}

- (void)windowDidLoad
{
    NSDictionary *options = [NSDictionary dictionaryWithObject:(id)kCFBooleanTrue
                                                        forKey:NSRaisesForNotApplicableKeysBindingOption];
    
    // This keeps the outline view up to date
    [savestateController addObserver:self forKeyPath:@"arrangedObjects" options:0 context:OEContentChangedContext];
    [savestateController addObserver:self forKeyPath:@"selection"       options:0 context:OESelectionChangedContext];
    [  romFileController addObserver:self forKeyPath:@"arrangedObjects" options:0 context:OEContentChangedContext];
    [  romFileController addObserver:self forKeyPath:@"selection"       options:0 context:OESelectionChangedContext];
    
    [imageFlow setDataSource:self];
    [imageFlow setDelegate:self];
    [imageFlow setAutoresizesSubviews:YES];
    [imageFlow setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    
    [imageBrowser bind:@"zoomValue" toObject:self withKeyPath:@"browserZoom" options:options];
    
    [imageBrowser setCellsStyleMask:IKCellsStyleSubtitled];
    [imageBrowser setCellSize:NSMakeSize(150.0, 150.0)];
    [imageBrowser setAnimates:NO];
    
    [imageBrowser setDataSource:self];
    [imageBrowser setMenu:contextMenu];
    
    [imageBrowser reloadData];
    
    [holderView addSubview:listView];
    
    [outlineView setTarget:self];
    [outlineView setDoubleAction:@selector(doubleClickedOutlineView:)];
    [listView setFrame:[holderView bounds]];
}

#pragma mark Cover Flow Data Source Methods
- (NSUInteger)numberOfItemsInImageFlow:(id)aBrowser
{
    return [sortedSaveStates count];
}

- (void)setSearchPredicate:(NSPredicate *)value
{
    if(searchPredicate != value)
    {
        [searchPredicate release];
        searchPredicate = [value retain];
        
        [self updateSelectedPlugins];
    }
}

- (void)imageFlow:(IKImageFlowView *)sender didSelectItemAtIndex:(NSInteger)index
{
}

- (void)imageFlow:(IKImageFlowView *)sender cellWasDoubleClickedAtIndex:(NSInteger)index
{
    [[GameDocumentController sharedDocumentController] loadState:[NSArray arrayWithObject:[self selectedSaveState]]];
}

- (id)imageFlow:(id)aBrowser itemAtIndex:(NSUInteger)index
{
    return [self saveStateAtAbsoluteIndex:index];
}

- (OESaveState *)saveStateAtAbsoluteIndex:(NSUInteger)uIndex
{
    return [sortedSaveStates objectAtIndex:uIndex];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == OEContentChangedContext)
    {
        [self updateRomGroups];
        [imageFlow reloadData];
        [outlineView reloadData];
        [imageBrowser reloadData];
    }
    else if(context == OESelectionChangedContext)
    {
        NSUInteger selectedIndex = [savestateController selectionIndex];
        
        if(selectedIndex > [[savestateController arrangedObjects] count])
            return;
        
        [imageFlow setSelectedIndex:selectedIndex];
        
        // Find which path this is in
        NSString *selectedPath = [[[self saveStateAtAbsoluteIndex:selectedIndex] romFile] name];
        [outlineView expandItem:selectedPath];
        
        NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:[outlineView rowForItem:[[savestateController selectedObjects] objectAtIndex:0]]];
        
        [outlineView selectRowIndexes:indexSet  byExtendingSelection:NO];
    }
}

- (void)doubleClickedOutlineView:(id)sender
{
    //NSLog(@"Clicked by: %@", sender);
    id selectedObject = [outlineView itemAtRow:[outlineView selectedRow]];
    
    if([selectedObject isKindOfClass:[OESaveState class]])
        [[GameDocumentController sharedDocumentController] loadState:[NSArray arrayWithObject:selectedObject]];
}

- (NSArray *)plugins
{
    return [[GameDocumentController sharedDocumentController] plugins];
}

- (void)setSelectedPlugins:(NSIndexSet *)indexes
{
    if(selectedPlugins != indexes)
    {
        [selectedPlugins release];
        NSUInteger index = [indexes firstIndex];
        
        if(indexes != nil && index < [[pluginController arrangedObjects] count] && index != NSNotFound)
        {
            currentPlugin   = [[pluginController selectedObjects] objectAtIndex:0];
            selectedPlugins = [[NSIndexSet alloc] initWithIndex:index];
            //[self setSelectedRomPredicate:[NSPredicate predicateWithFormat:@"%K matches[c] %@",  @"emulatorID", [currentPlugin displayName]]];
        }
        else
        {
            selectedPlugins = [[NSIndexSet alloc] init];
            currentPlugin = nil;
        }
        
        [self updateSelectedPlugins];
        
        [outlineView reloadData];
        [imageFlow reloadData];
    }
}

- (IBAction)toggleViewType:(NSSegmentedControl *)sender
{
    [outlineView reloadData];
    [imageFlow reloadData];
    
    NSView *addedView = nil;
    
    switch([sender selectedSegment])
    {
        case 0  : addedView = listView;       break;
        case 1  : addedView = collectionView; break;
        case 2  : addedView = imageFlow;      break;
        default : return;
    }
    
    [holderView setSubviews:[NSArray arrayWithObject:addedView]];
    [addedView setFrame:[holderView bounds]];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasDoubleClickedAtIndex:(NSUInteger) index
{
    [[GameDocumentController sharedDocumentController] loadState:[NSSet setWithObject:[self selectedSaveState]]];
}

- (void)updateRomGroups
{
    NSArray *allROMs  = [self allROMs];

    [romFileIndexes removeAllObjects];
    [sortedSaveStates removeAllObjects];

    NSUInteger index = 0;
    for(OEROMFile *romFile in allROMs)
    {
        [sortedSaveStates addObjectsFromArray:[[romFile saveStates] sortedArrayUsingDescriptors:sortDescriptors]];

        [romFileIndexes setObject:[NSNumber numberWithUnsignedInteger:index] forKey:[romFile path]];
        index += [[romFile saveStates] count];
    }
}

- (NSArray *)allROMs
{
    return [romFileController arrangedObjects];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasRightClickedAtIndex:(NSUInteger)index withEvent:(NSEvent *)event
{
    [NSMenu popUpContextMenu:contextMenu withEvent:event forView:aBrowser];
}

- (NSUInteger)numberOfGroupsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    return [[self allROMs] count];
}

- (NSUInteger)numberOfItemsInImageBrowser:(IKImageBrowserView *) aBrowser
{
    return [sortedSaveStates count];
}

- (NSDictionary *)imageBrowser:(IKImageBrowserView *)aBrowser groupAtIndex:(NSUInteger)index
{
    OEROMFile *romFile = [[self allROMs] objectAtIndex:index];

    NSRange range = NSMakeRange([[romFileIndexes objectForKey:[romFile path]] unsignedIntegerValue],
                                [[romFile saveStates] count]);
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSValue valueWithRange:range],                  IKImageBrowserGroupRangeKey,
                          [romFile name],                                  IKImageBrowserGroupTitleKey,
                          [NSNumber numberWithInt:IKGroupDisclosureStyle], IKImageBrowserGroupStyleKey,
                          nil];
    
    return dict;
}

- (id)imageBrowser:(IKImageBrowserView *)aBrowser itemAtIndex:(NSUInteger)index
{
    return [self saveStateAtAbsoluteIndex:index];
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil) return [[self allROMs]    count]; // return the ROMs
    else            return [[item saveStates] count]; // return the save states for the ROM
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [item isKindOfClass:[OEROMFile class]] && [[item saveStates] count] > 0;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    return [item isKindOfClass:[OESaveState class]];
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if(item == nil)
        return [[self allROMs] objectAtIndex:index];
    else {
        NSInteger shiftedIndex = [[romFileIndexes objectForKey:[item path]] integerValue] + index;
        return [sortedSaveStates objectAtIndex:shiftedIndex];
    }
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{ 
    if([item isKindOfClass:[OEROMFile class]])
        return [item name];
    else if([item isKindOfClass:[OESaveState class]])
        return [item imageSubtitle];
    return nil;
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    if([outlineView selectedRow] == -1 || [[outlineView itemAtRow:[outlineView selectedRow]] class] != [OESaveState class])
        [savestateController setSelectedObjects: nil];
    else
        [savestateController setSelectedObjects: [NSArray arrayWithObject:[outlineView itemAtRow:[outlineView selectedRow]]]];
}

- (OESaveState *)selectedSaveState
{
    OESaveState *saveState = nil;
    
    switch([segmentButton selectedSegment])
    {
        case 0:
            saveState = [outlineView itemAtRow:[outlineView clickedRow]];
            break;
        case 1:
            saveState = [self saveStateAtAbsoluteIndex:[[imageBrowser selectionIndexes] firstIndex]];
            break;
        case 2:
            saveState = [self saveStateAtAbsoluteIndex:[imageFlow selectedIndex]];
            break;
    }
    
    return ([saveState isKindOfClass:[OESaveState class]] ? saveState : nil);
}

- (IBAction)exportSave:(id)sender
{
    OESaveState *saveState = [self selectedSaveState];
    
    NSData *saveData = [[[saveState saveData] copy] autorelease];
    
    NSSavePanel *sheet = [NSSavePanel savePanel];
    
    [sheet beginSheetModalForWindow:[self window]
                  completionHandler:
     ^(NSInteger result)
     {
         if(result == NSOKButton)
             [saveData writeToFile:[sheet filename] atomically:YES];
     }];
}

- (IBAction)deleteState:(id)sender
{
    [[NSFileManager defaultManager] removeItemAtPath:[[self selectedSaveState] bundlePath] error:nil];
}

@end
