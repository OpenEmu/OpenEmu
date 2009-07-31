//
//  OESaveStateController.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/28/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OESaveStateController.h"
#import "GameDocumentController.h"
#import "SaveState.h"

@implementation OESaveStateController


@synthesize plugins, availablePluginsPredicate, selectedPlugins, docController, 
selectedRomPredicate, browserZoom, sortDescriptors, pathArray, pathRanges;

- (id)init
{
    self = [super initWithWindowNibName:@"SaveStateManager"];
    if(self != nil)
    {
		self.docController = [GameDocumentController sharedDocumentController];
		self.selectedRomPredicate = nil;
		
		NSSortDescriptor *path = [[NSSortDescriptor alloc] initWithKey:@"rompath" ascending:NO];
		NSSortDescriptor *time = [[NSSortDescriptor alloc] initWithKey:@"timeStamp" ascending:YES];
		
		self.sortDescriptors = [NSArray arrayWithObjects:path,time,nil];
		self.pathArray = [NSMutableArray array];
		self.pathRanges = [NSMutableArray array];
		
		[path release];
		[time release];
	}
    return self;
}

- (void)windowDidLoad
{
	NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:
							 [NSNumber numberWithBool:YES],NSRaisesForNotApplicableKeysBindingOption,nil];
	
	[imageBrowser bind:@"content" toObject:savestateController withKeyPath:@"arrangedObjects" options:options];
	[imageBrowser bind:@"selectionIndexes" toObject:savestateController withKeyPath:@"selectionIndexes" options:options];
	[imageBrowser bind:@"zoomValue" toObject:self withKeyPath:@"browserZoom" options:options];
	
	[imageBrowser setCellsStyleMask:IKCellsStyleTitled];
	[imageBrowser setCellSize:NSMakeSize(150.0f, 150.0f)];
	[imageBrowser setAnimates:NO];
	
	[imageBrowser setDataSource:self];
	
	[holderView addSubview:listView];
	listView.frame = holderView.bounds;
}

- (NSArray *)plugins
{
    return [[GameDocumentController sharedDocumentController] plugins];
}

- (void)dealloc
{
	self.docController = nil;
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
		self.selectedRomPredicate = [NSPredicate predicateWithFormat:@"%K matches[c] %@",  @"emulatorID", [currentPlugin displayName] ];
    }
    else
    {
        selectedPlugins = [[NSIndexSet alloc] init];
        currentPlugin = nil;
    }
}

- (IBAction) toggleViewType:(id) sender
{
	NSSegmentedControl* segments = (NSSegmentedControl*) sender;

	switch( [segments selectedSegment] )
	{
		case 0:
			[collectionView removeFromSuperview];
			[holderView addSubview:listView];
			listView.frame = holderView.bounds;
			break;
		case 1:
			[listView removeFromSuperview];
			[holderView addSubview:collectionView];
			collectionView.frame = holderView.bounds;
			break;			
	}
}

- (void) imageBrowser:(IKImageBrowserView *) aBrowser cellWasDoubleClickedAtIndex:(NSUInteger) index
{
	[self.docController loadState:[savestateController selectedObjects]];	
}

- (NSUInteger) numberOfGroupsInImageBrowser:(IKImageBrowserView *) aBrowser
{
	NSArray *allItems = [savestateController arrangedObjects];
	
	[self.pathArray removeAllObjects];
	[self.pathRanges removeAllObjects];
	
	NSRange range;
	for( int i = 0; i < [allItems count]; i++ )
	{
		SaveState* state = [allItems objectAtIndex:i];
		
		if(! [self.pathArray containsObject:[state rompath]] )
		{
			if( [self.pathArray count] != 0 )
				[self.pathRanges addObject:[NSValue valueWithRange:range]];
				
			[self.pathArray addObject:[state rompath]];
			range.location = i;
			range.length = 1;
		}
		else
		{
			range.length++;
		}
		
	}
	if( [self.pathArray count] )
		[self.pathRanges addObject:[NSValue valueWithRange:range]];
	
	return [self.pathArray count];
}

- (NSDictionary *) imageBrowser:(IKImageBrowserView *) aBrowser groupAtIndex:(NSUInteger) index
{
	return [NSDictionary dictionaryWithObjectsAndKeys:[self.pathRanges objectAtIndex:index], IKImageBrowserGroupRangeKey,
			[[self.pathArray objectAtIndex:index] lastPathComponent] ,IKImageBrowserGroupTitleKey,
			[NSNumber numberWithInt:IKGroupDisclosureStyle], IKImageBrowserGroupStyleKey,
			nil];	
}

@end
