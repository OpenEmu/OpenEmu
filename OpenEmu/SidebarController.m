//
//  SidebarController.m
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SidebarController.h"
#import "SidebarGroupItem.h"
#import "SideBarDataSourceItem.h"
#import "LibraryDatabase.h"
#import "SidebarCell.h"

#import "SidebarOutlineView.h"
@implementation SidebarController
@synthesize groups, database, editingItem;

- (id)init{
    self = [super init];
    if (self) {}
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}

- (void)awakeFromNib{
	// should localize group names here
	self.groups = [NSArray arrayWithObjects:
				   [SidebarGroupItem groupItemWithName:@"CONSOLES"],
				   [SidebarGroupItem groupItemWithName:@"COLLECTIONS"],
				   nil];
	
	// setup sidebar outline view
	[sidebarView setHeaderView:nil];
	
	SidebarCell* cell = [[[SidebarCell alloc] init] autorelease];
	[cell setEditable:YES];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controlTextDidEndEditing:) name:NSControlTextDidEndEditingNotification object:cell];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controlTextDidBeginEditing:) name:NSControlTextDidBeginEditingNotification object:cell];
	
	[[[sidebarView tableColumns] objectAtIndex:0] setDataCell:cell];
	[sidebarView setIndentationPerLevel:7];
	[sidebarView setAutoresizesOutlineColumn:NO];
	[sidebarView registerForDraggedTypes:[NSArray arrayWithObjects:@"org.openEmu.rom", NSFilenamesPboardType, nil]];
	sidebarView.delegate = self;
	sidebarView.dataSource = self;
	
	[sidebarView selectRowIndexes:[NSIndexSet indexSetWithIndex:1] byExtendingSelection:NO];
	[sidebarView expandItem:[sidebarView itemAtRow:0]];
	
	NSScrollView* enclosingScrollView = [sidebarView enclosingScrollView];
	if(enclosingScrollView){
		[enclosingScrollView setDrawsBackground:YES];
		[enclosingScrollView setBackgroundColor:[NSColor colorWithDeviceWhite:0.19 alpha:1.0]];
		
		[sidebarView setBackgroundColor:[NSColor clearColor]];
	}
}

#pragma mark -
#pragma mark Public
- (void)addCollection:(BOOL)isSmart{

}

- (void)reloadData{
	[sidebarView reloadData];
}
#pragma mark -
#pragma mark NSOutlineView Delegate
- (void)outlineViewSelectionDidChange:(NSNotification *)notification{
	id selectedCollection = [sidebarView itemAtRow:[sidebarView selectedRow]];
	NSDictionary* userInfo = [NSDictionary dictionaryWithObject:selectedCollection forKey:@"selectedCollection"];
	
	[[NSNotificationCenter defaultCenter] postNotificationName:@"SidebarSelectionChanged" object:self userInfo:userInfo];
}

- (void)outlineViewSelectionIsChanging:(NSNotification *)notification{}
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item{
	return ![item isGroupHeaderInSdebar];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item{
	if ( item==[self.groups objectAtIndex:0] ) {
		return NO;
	}
	return YES;
}
#pragma mark -
#pragma mark NSOutlineView DataSource
- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item{
	if( item == nil){
		return [self.groups objectAtIndex:index];
	}
	
	if( item == [self.groups objectAtIndex:0] ){
		return [[self.database consoles] objectAtIndex:index];
	}
	
	
	if( item == [self.groups objectAtIndex:1] ){
		return [[self.database collections] objectAtIndex:index];
	}
	
	return nil;
}
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item{
	return [(id <SidebarDataSourceItem>)item isGroupHeaderInSdebar];
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item{
	if( item == nil ){
		return [self.groups count];
	}
	
	if( item == [self.groups objectAtIndex:0] ){
		return [[self.database consoles] count];
	}
	
	if( item == [self.groups objectAtIndex:1] ){
		return [[self.database collections] count];
	}
	
	return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item{
	return [item sidebarName];
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item{
	self.editingItem = nil;
	
	[item setSidebarName:object];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item {
	BOOL result = [item isEditableInSdebar];
	if(result){
		self.editingItem = item;
	} else {
		self.editingItem = nil;
	}
	
	return result;
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item{
	if([item isGroupHeaderInSdebar]){
		return 26.0;
	}	
	return 20.0;
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell*)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item {
    if([cell isKindOfClass:[SidebarCell class]]){
		[(SidebarCell*)cell setImage:[item sidebarIcon]];
		[(SidebarCell*)cell setIsGroup:[item isGroupHeaderInSdebar]];
		
		if(self.editingItem == nil)
			[(SidebarCell*)cell setIsEditing:NO];
	}
}

- (void)setEditingItem:(id)newEdItem{
	[newEdItem retain];
	[editingItem release];
	
	editingItem = newEdItem;
}

#pragma mark -

- (void)controlTextDidBeginEditing:(NSNotification *)aNotification{
	
}
- (void)controlTextDidEndEditing:(NSNotification *)aNotification{
	self.editingItem = nil;
	
}
@end
