//
//  OESaveStateController.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/28/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OESaveStateController.h"
#import "GameDocumentController.h"

@implementation OESaveStateController


@synthesize plugins, availablePluginsPredicate, selectedPlugins, docController, selectedRomPredicate;

- (id)init
{
    self = [super initWithWindowNibName:@"SaveStateManager"];
    if(self != nil)
    {
		self.docController = [GameDocumentController sharedDocumentController];
		self.selectedRomPredicate = nil;
	}
    return self;
}

- (void)windowDidLoad
{
	NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:
							 [NSNumber numberWithBool:YES],NSRaisesForNotApplicableKeysBindingOption,nil];
	
	[imageBrowser bind:@"content" toObject:savestateController withKeyPath:@"arrangedObjects" options:options];
		[imageBrowser bind:@"selectionIndexes" toObject:savestateController withKeyPath:@"selectionIndexes" options:options];
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


/*
- (NSUInteger) numberOfItemsInImageBrowser:(IKImageBrowserView *) aBrowser
{		
	return [[savestateController arrangedObjects] count];
}

- (id) imageBrowser:(IKImageBrowserView *) aBrowser itemAtIndex:(NSUInteger)index
{
	return [[savestateController arrangedObjects] objectAtIndex:index];
}*/

- (void) imageBrowser:(IKImageBrowserView *) aBrowser cellWasDoubleClickedAtIndex:(NSUInteger) index
{
	[self.docController loadState:[savestateController selectedObjects]];	
}

@end
