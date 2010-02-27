/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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
-(NSSet *)allEmulatorIDsForSaveStates;

@end

@implementation OEROMFile (SaveStateManagerExtras)

-(NSSet *)allEmulatorIDsForSaveStates{
	NSMutableSet *emulatorIDs = [NSMutableSet set];
	NSSet *saves = [self saveStates];
	for(OESaveState *state in saves){
		[emulatorIDs addObject:[state emulatorID]];
	}
	return [[emulatorIDs copy] autorelease];
}

@end

@implementation OESaveStateController

@synthesize availablePluginsPredicate, selectedPlugins, selectedRomPredicate, browserZoom, 
            sortDescriptors, pathArray, pathRanges, romFileController, savestateController, 
            pluginController, treeController, listView, collectionView, outlineView, imageBrowser,
            imageFlow, contextMenu, segmentButton, holderView, searchField;

- (id)init
{
    self = [super initWithWindowNibName:@"SaveStateManager"];
    if(self != nil)
    {
        NSSortDescriptor *path = [[NSSortDescriptor alloc] initWithKey:@"romFile.path" ascending:NO];
        NSSortDescriptor *time = [[NSSortDescriptor alloc] initWithKey:@"timeStamp" ascending:YES];
        
        [self setSortDescriptors:[NSArray arrayWithObjects:path, time, nil]];
        [self setPathArray:[NSMutableArray array]];
        [self setPathRanges:[NSMutableArray array]];
		
        [path release];
        [time release];
    }
    return self;
}

static void * const OEContentChangedContext   = @"OEContentChangedContext";
static void * const OESelectionChangedContext = @"OESelectionChangedContext";

- (GameDocumentController *)docController{
	return [GameDocumentController sharedDocumentController];
}

- (NSPredicate *)selectedPluginsPredicate{
	return selectedPluginsPredicate;
}

-(void)updateSelectedPlugins{
	NSArray *selection = [pluginController selectedObjects];
	NSMutableArray *subpredicates = [NSMutableArray arrayWithCapacity:selection.count];
	for(OECorePlugin *plugin in selection){
		[subpredicates addObject:[NSPredicate predicateWithFormat:@"%@ IN allEmulatorIDsForSaveStates",[plugin displayName]]];
	}
	
	NSPredicate *pluginsPredicate = [NSCompoundPredicate orPredicateWithSubpredicates:subpredicates];
	
	if(searchPredicate){
		pluginsPredicate = [NSCompoundPredicate andPredicateWithSubpredicates:[NSArray arrayWithObjects:pluginsPredicate, searchPredicate, nil]];
	}
	
	[self willChangeValueForKey:@"selectedPluginsPredicate"];
	[selectedPluginsPredicate autorelease];
	selectedPluginsPredicate = [pluginsPredicate retain];;	
	[self  didChangeValueForKey:@"selectedPluginsPredicate"];
}

- (void)windowDidLoad
{
    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
                             (id)kCFBooleanTrue, NSRaisesForNotApplicableKeysBindingOption, nil];
    
    // This keeps the outline view up to date
    [savestateController addObserver:self forKeyPath:@"arrangedObjects" options:0 context:OEContentChangedContext];
    [savestateController addObserver:self forKeyPath:@"selection"       options:0 context:OESelectionChangedContext];
    [  romFileController addObserver:self forKeyPath:@"arrangedObjects" options:0 context:OEContentChangedContext];
    [  romFileController addObserver:self forKeyPath:@"selection"       options:0 context:OESelectionChangedContext];
	
    //[imageFlow bind:@"content" toObject:savestateController withKeyPath:@"arrangedObjects" options:options];
    [imageFlow setDataSource:self];
    [imageFlow setDelegate:self];
    [imageFlow setAutoresizesSubviews:YES];
    [imageFlow setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    
    //[imageFlow
    //[imageBrowser bind:@"content" toObject:savestateController withKeyPath:@"arrangedObjects" options:options];
    //[imageBrowser bind:@"selectionIndexes" toObject:savestateController withKeyPath:@"selectionIndexes" options:options];
    [imageBrowser bind:@"zoomValue" toObject:self withKeyPath:@"browserZoom" options:options];
    
    [imageBrowser setCellsStyleMask:IKCellsStyleSubtitled];
    [imageBrowser setCellSize:NSMakeSize(150.0f, 150.0f)];
    [imageBrowser setAnimates:NO];
    
    [imageBrowser setDataSource:self];
    [imageBrowser setMenu:contextMenu];
    
    [imageBrowser reloadData];
    
    [holderView addSubview:listView];
    
    [outlineView setTarget:self];
    [outlineView setDoubleAction:@selector(doubleClickedOutlineView:)];
    listView.frame = holderView.bounds;
}

#pragma mark Cover Flow Data Source Methods
- (NSUInteger)numberOfItemsInImageFlow:(id)aBrowser
{
    // we calculated the ranges, so grab the last one and calculate how large it is
    NSRange lastRange = [[[self pathRanges] lastObject] rangeValue];
    return lastRange.location + lastRange.length;
}

-(void)setSearchPredicate:(NSPredicate *)pred{
	[pred retain];
	[searchPredicate release];
	searchPredicate = pred;

	[self updateSelectedPlugins];
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

-(OESaveState *)saveStateAtAbsoluteIndex:(NSUInteger)uIndex{
    NSInteger index = (NSInteger)uIndex;
    NSInteger romIndex = 0;
    NSRange range = NSMakeRange(0, 0);
    
    if(index > 0){
        for(romIndex=0; romIndex < self.pathRanges.count; romIndex++){
            NSValue *value = (NSValue *)[self.pathRanges objectAtIndex:romIndex];
            range = [value rangeValue];
            
            if((range.location + range.length) > index){
                break;
            }
        }
    }
    
    NSArray *allROMs = [self allROMs];
    if(romIndex < 0 || romIndex >= [allROMs count]) return nil;
    
    OEROMFile *romFile = [allROMs objectAtIndex:romIndex];
    index -= range.location;
    
    NSArray *allSaves = [[romFile saveStates] allObjects];
    if(index < 0 || index >= [allSaves count]) return nil;
    
    return [allSaves objectAtIndex:index];    
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
        int selectedIndex = [savestateController selectionIndex];
        
        if( selectedIndex < 0 || selectedIndex > [[savestateController arrangedObjects] count])
            return;
        
        [imageFlow setSelectedIndex:selectedIndex];
        //Find which path this is in
        NSString *selectedPath = [[[self saveStateAtAbsoluteIndex:selectedIndex] romFile] name];
        [outlineView expandItem:selectedPath];
        NSIndexSet* indexSet = [NSIndexSet indexSetWithIndex:[outlineView rowForItem: [[savestateController selectedObjects] objectAtIndex:0]]];
        [outlineView selectRowIndexes:indexSet  byExtendingSelection:NO];
	}
}

- (void)doubleClickedOutlineView:(id)sender
{
    //NSLog(@"Clicked by: %@", sender);
    id selectedObject = [outlineView itemAtRow:[outlineView selectedRow]];
    
    if([selectedObject class] == [OESaveState class])
        [[GameDocumentController sharedDocumentController] loadState:[NSArray arrayWithObject:selectedObject]];
}

- (NSArray *)plugins
{
    return [[GameDocumentController sharedDocumentController] plugins];
}

- (void)dealloc
{
    [selectedPlugins release];
    [super           dealloc];
}

- (void)setSelectedPlugins:(NSIndexSet *)indexes
{
    [selectedPlugins release];
    NSUInteger index = [indexes firstIndex];
    
    if(indexes != nil && index < [[pluginController arrangedObjects] count] && index != NSNotFound)
    {
        currentPlugin = [[pluginController selectedObjects] objectAtIndex:0];
        selectedPlugins = [[NSIndexSet alloc] initWithIndex:index];
//        [self setSelectedRomPredicate:[NSPredicate predicateWithFormat:@"%K matches[c] %@",  @"emulatorID", [currentPlugin displayName]]];
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

- (IBAction)toggleViewType:(NSSegmentedControl *)sender
{
    [outlineView reloadData];
    [imageFlow reloadData];
    
    for(NSView *view in [holderView subviews])
        [view removeFromSuperview];
    
    NSView *addedView = nil;
    
    switch([sender selectedSegment])
    {
        case 0 : addedView = listView;       break;
        case 1 : addedView = collectionView; break;            
        case 2 : addedView = imageFlow;      break;
    }
    
    [holderView addSubview:addedView];
    [addedView setFrame:[holderView bounds]];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasDoubleClickedAtIndex:(NSUInteger) index
{
    [[GameDocumentController sharedDocumentController] loadState:[NSSet setWithObject:[self selectedSaveState]]];
}

- (void)updateRomGroups
{
    NSArray *allROMs  = [romFileController arrangedObjects];
    
    [[self pathArray] removeAllObjects];
    [[self pathRanges] removeAllObjects];
    
    NSRange range = NSMakeRange(0, 0);
    for(OEROMFile *romFile in allROMs)
    {
        range.length = [[romFile saveStates] count];
        
        [[self pathRanges] addObject:[NSValue valueWithRange:range]];
        
        range.location += range.length;
        range.length = 0;
    }
}
    
-(NSArray *)allROMs
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

- (NSUInteger) numberOfItemsInImageBrowser:(IKImageBrowserView *) aBrowser
{
    NSUInteger sum = 0;
    for(OEROMFile *romFile in [self allROMs])
        sum += [[romFile saveStates] count];
    
    return sum;
}

- (NSDictionary *)imageBrowser:(IKImageBrowserView *)aBrowser groupAtIndex:(NSUInteger)index
{
    OEROMFile *romFile = [[romFileController arrangedObjects] objectAtIndex:index];
    
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                          [[self pathRanges] objectAtIndex:index],         IKImageBrowserGroupRangeKey,
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
    if(item == nil) return [[self allROMs] count];    // return the ROMs
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
    else
        return [[[item saveStates] allObjects] objectAtIndex:index];
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
            saveState = [[savestateController arrangedObjects] objectAtIndex:[imageFlow selectedIndex]];
            break;
    }
    
    return ([saveState isKindOfClass:[OESaveState class]] ? saveState : nil);
}

- (IBAction)exportSave:(id)sender
{
    OESaveState *saveState = [self selectedSaveState];
    
    NSData *saveData = [[saveState saveData] copy];
    
    [[NSSavePanel savePanel] beginSheetForDirectory:nil
                                               file:nil 
                                     modalForWindow:[self window]
                                      modalDelegate:self
                                     didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:)
                                        contextInfo:saveData];
}

- (IBAction)deleteState:(id)sender
{
    OESaveState *saveState = [self selectedSaveState];    
    [[NSFileManager defaultManager] removeFileAtPath:[saveState bundlePath] handler:nil];
}

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    NSData *saveData = (NSData *)contextInfo;
    
    if(returnCode == NSOKButton) 
        [saveData writeToFile:[sheet filename] atomically:YES];
    
    [saveData release];
}

@end
