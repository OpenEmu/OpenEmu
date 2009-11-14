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
#import "IKImageFlowView.h"
#import "OESaveState.h"
#import "OEROMFile.h"

@implementation OESaveStateController


@synthesize availablePluginsPredicate, selectedPlugins, docController, 
selectedRomPredicate, browserZoom, sortDescriptors, pathArray, pathRanges;

- (id)init
{
    self = [super initWithWindowNibName:@"SaveStateManager"];
    if(self != nil)
    {
        self.docController = [GameDocumentController sharedDocumentController];
        self.selectedRomPredicate = nil;
        
        NSSortDescriptor *path = [[NSSortDescriptor alloc] initWithKey:@"romFile.path" ascending:NO];
        NSSortDescriptor *time = [[NSSortDescriptor alloc] initWithKey:@"timeStamp" ascending:YES];
        
        self.sortDescriptors = [NSArray arrayWithObjects:path,time,nil];
        self.pathArray = [NSMutableArray array];
        self.pathRanges = [NSMutableArray array];
        
        [path release];
        [time release];
    }
    return self;
}

static void *ContentChangedContext = @"ContentChangedContext";
static void *SelectionChangedContext = @"SelectionChangedContext";
- (void)windowDidLoad
{
    NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES],NSRaisesForNotApplicableKeysBindingOption,nil];
    
    //This keeps the outline view up to date
    [savestateController addObserver:self forKeyPath:@"arrangedObjects" options:0 context:ContentChangedContext];
    [savestateController addObserver:self forKeyPath:@"selection" options:0 context:SelectionChangedContext];
    
    //[imageFlow bind:@"content" toObject:savestateController withKeyPath:@"arrangedObjects" options:options];
    [imageFlow setDataSource:self];
    [imageFlow setDelegate:self];
    [imageFlow setAutoresizesSubviews:YES];
    [imageFlow setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    
    //[imageFlow
    [imageBrowser bind:@"content" toObject:savestateController withKeyPath:@"arrangedObjects" options:options];
    [imageBrowser bind:@"selectionIndexes" toObject:savestateController withKeyPath:@"selectionIndexes" options:options];
    [imageBrowser bind:@"zoomValue" toObject:self withKeyPath:@"browserZoom" options:options];
    
    [imageBrowser setCellsStyleMask:IKCellsStyleSubtitled];
    [imageBrowser setCellSize:NSMakeSize(150.0f, 150.0f)];
    [imageBrowser setAnimates:NO];
    
    
    [imageBrowser setDataSource:self];
    [imageBrowser setMenu:contextMenu];
    
    [holderView addSubview:listView];
    
    [outlineView setTarget:self];
    [outlineView setDoubleAction:@selector(doubleClickedOutlineView:)];
    listView.frame = holderView.bounds;
}

#pragma mark Cover Flow Data Source Methods
- (NSUInteger) numberOfItemsInImageFlow:(id)aBrowser
{
    return [[savestateController arrangedObjects] count];
}

- (void)imageFlow:(IKImageFlowView *)sender didSelectItemAtIndex:(NSInteger)index
{
    [savestateController setSelectionIndex:index];
}

- (void)imageFlow:(IKImageFlowView *)sender cellWasDoubleClickedAtIndex:(NSInteger)index
{
    [self.docController loadState:[NSArray arrayWithObject:[[savestateController arrangedObjects] objectAtIndex:index]]];    
}

- (id)imageFlow:(id)aBrowser itemAtIndex:(NSUInteger)index
{
    return [[savestateController arrangedObjects] objectAtIndex:index];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == ContentChangedContext)
    {
        [self updateRomGroups];
        [imageFlow reloadData];
        [outlineView reloadData];
        [imageBrowser reloadData];
    }
    else if(context == SelectionChangedContext)
    {
        int selectedIndex = [savestateController selectionIndex];
        
        if( selectedIndex < 0 || selectedIndex > [[savestateController arrangedObjects] count])
            return;
        
        [imageFlow setSelectedIndex:selectedIndex];
        //Find which path this is in
        NSString *selectedPath = nil;
        for( int i = 0; i < pathRanges.count; i++ )
        {
            NSRange range = [[pathRanges objectAtIndex:i] rangeValue];
            if( selectedIndex - range.location >= 0 && selectedIndex - range.location < range.length )
                selectedPath = [pathArray objectAtIndex:i];
        }
        
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
        [self.docController loadState:[NSArray arrayWithObject:selectedObject]];
}

- (NSArray *)plugins
{
    return [[GameDocumentController sharedDocumentController] plugins];
}

- (void)dealloc
{
    [docController release];
    [selectedPlugins release];
    [super dealloc];
}

- (void)setSelectedPlugins:(NSIndexSet *)indexes
{
    [selectedPlugins release];
    NSUInteger index = [indexes firstIndex];
    
    if(indexes != nil && index < [[pluginController arrangedObjects] count] && index != NSNotFound)
    {
        currentPlugin = [[pluginController selectedObjects] objectAtIndex:0];
        selectedPlugins = [[NSIndexSet alloc] initWithIndex:index];
        self.selectedRomPredicate = [NSPredicate predicateWithFormat:@"%K matches[c] %@",  @"emulatorID", [currentPlugin displayName]];
    }
    else
    {
        selectedPlugins = [[NSIndexSet alloc] init];
        currentPlugin = nil;
    }
    [outlineView reloadData];
    [imageFlow reloadData];
}

- (IBAction)toggleViewType:(NSSegmentedControl *)sender
{
    [outlineView reloadData];
    [imageFlow reloadData];
    
    for(NSView *view in [holderView subviews])
        [view removeFromSuperview];
    switch([sender selectedSegment])
    {
        case 0:
            [holderView addSubview:listView];
            listView.frame = holderView.bounds;
            break;
        case 1:
            [holderView addSubview:collectionView];
            collectionView.frame = holderView.bounds;
            break;            
        case 2:
            [holderView addSubview:imageFlow];
            [imageFlow setFrame:holderView.bounds];
            break;
    }
}

- (void)imageBrowser:(IKImageBrowserView *) aBrowser cellWasDoubleClickedAtIndex:(NSUInteger) index
{
    [self.docController loadState:[savestateController selectedObjects]];    
}

- (void)updateRomGroups
{
	NSArray *allROMs  = [  romFileController arrangedObjects];
    NSArray *allSaves = [savestateController arrangedObjects];
    
	NSLog(@"ROMs: %@ (%@)",allROMs, romFileController);
	
    [self.pathArray removeAllObjects];
    [self.pathRanges removeAllObjects];
    
    NSRange range = NSMakeRange(0, 0);
    for(NSUInteger i = 0; i < [allSaves count]; i++)
    {
        OESaveState *state = [allSaves objectAtIndex:i];
        
        if(![self.pathArray containsObject:[[state romFile] path]])
        {
//            if([self.pathArray count] != 0)
                [self.pathRanges addObject:[NSValue valueWithRange:range]];
            
            [self.pathArray addObject:[[state romFile] path]];
            range.location = i;
            range.length = 1;
        }
        else
        {
            range.length++;
        }
        
    }
    if([self.pathArray count] == 0) [self.pathRanges addObject:[NSValue valueWithRange:range]];    
}
    
-(NSArray *)allROMs{
	return [romFileController arrangedObjects];
}

- (void)imageBrowser:(IKImageBrowserView *)aBrowser cellWasRightClickedAtIndex:(NSUInteger)index withEvent:(NSEvent *)event
{
    [NSMenu popUpContextMenu:contextMenu withEvent:event forView:aBrowser];
}

- (NSUInteger)numberOfGroupsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    [self updateRomGroups];
    [outlineView reloadData];
    return [self.pathArray count];
}

- (NSDictionary *)imageBrowser:(IKImageBrowserView *)aBrowser groupAtIndex:(NSUInteger)index
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
            [self.pathRanges objectAtIndex:index],                    IKImageBrowserGroupRangeKey,
            [[self.pathArray objectAtIndex:index] lastPathComponent], IKImageBrowserGroupTitleKey,
            [NSNumber numberWithInt:IKGroupDisclosureStyle],          IKImageBrowserGroupStyleKey,
            nil];    
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil){
		// return the ROMs
		return [[self allROMs] count];
	}else{
		// return the save states for the ROM
		OEROMFile *romFile = (OEROMFile *)item;
		return [[romFile saveStates] count];
	}
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	if([item isKindOfClass:[OEROMFile class]]){
		OEROMFile *romFile = (OEROMFile *)item;
		return ([[romFile saveStates] count] > 0);
	}else if([item isKindOfClass:[OESaveState class]]){
		return NO;
	}else{
		// shouldn't get here
		return NO;
	}
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
	if([item isKindOfClass:[OEROMFile class]]){
		return NO;
	}else if([item isKindOfClass:[OESaveState class]]){
		return YES;
	}else{
		return NO;
	}
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if(item == nil){
		return [[self allROMs] objectAtIndex:index];
	}else{
		OEROMFile *romFile = (OEROMFile *)item;
		return [[[romFile saveStates] allObjects] objectAtIndex:index];
	}
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{ 
	if([item isKindOfClass:[OEROMFile class]]){
		OEROMFile *romFile = (OEROMFile *)item;
		return [romFile name];
	}else if([item isKindOfClass:[OESaveState class]]){
		OESaveState *saveState = (OESaveState *)item;
		return [saveState imageSubtitle];
	}else{
		// shouldn't get here
		return NO;
	}
}


- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    if( [outlineView selectedRow] == -1 || [[outlineView itemAtRow:[outlineView selectedRow]] class] != [OESaveState class] )
        [savestateController setSelectedObjects: nil];
    else
        [savestateController setSelectedObjects: [NSArray arrayWithObject:[outlineView itemAtRow:[outlineView selectedRow]]]];
}

- (OESaveState *)selectedSaveState
{
    OESaveState *saveState = nil;
    
    switch ([segmentButton selectedSegment])
    {
        case 0:
            saveState = [outlineView itemAtRow: [outlineView clickedRow]];
            break;
        case 1:
            saveState = [[savestateController arrangedObjects] objectAtIndex:[[imageBrowser selectionIndexes] firstIndex]];
            break;
        case 2:
            saveState = [[savestateController arrangedObjects] objectAtIndex:[imageFlow selectedIndex]];
            break;
        default:
            break;
    }
    
    if( [saveState class] != [OESaveState class] )
        return nil;
    
    return saveState;
}

- (IBAction) exportSave:(id) sender
{
    OESaveState* saveState = [self selectedSaveState];
    
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
