//
//  SidebarController.m
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESidebarController.h"
#import "OESidebarGroupItem.h"
#import "OESideBarDataSourceItem.h"
#import "OELibraryDatabase.h"
#import "OESidebarCell.h"

#import "OESidebarOutlineView.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OESidebarController
@synthesize groups, database, editingItem;
@synthesize systems, collections;

+ (void)initialize{
    // Collection Icons for sidebar
    NSImage* image = [NSImage imageNamed:@"collections"];
    
    [image setName:@"collections_simple" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];
    [image setName:@"collections_smart" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
}
- (void)dealloc{
    [super dealloc];
}

- (void)awakeFromNib{
    self.groups = [NSArray arrayWithObjects:
				   [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"CONSOLES", @"")],
				   [OESidebarGroupItem groupItemWithName:NSLocalizedString(@"COLLECTIONS", @"")],
				   nil];
    
    // Setup toolbar button
    [addCollectionBtn setImage:[NSImage imageNamed:@"toolbar_add_button"]];
    
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
	
    // setup sidebar outline view
    [sidebarView setHeaderView:nil];
    
    OESidebarCell* cell = [[[OESidebarCell alloc] init] autorelease];
    [cell setEditable:YES];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controlTextDidEndEditing:) name:NSControlTextDidEndEditingNotification object:cell];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controlTextDidBeginEditing:) name:NSControlTextDidBeginEditingNotification object:cell];
    
    [[[sidebarView tableColumns] objectAtIndex:0] setDataCell:cell];
    [sidebarView setIndentationPerLevel:7];
    [sidebarView setAutosaveName:@"sidebarView"];
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
- (void)setEnabled:(BOOL)enabled{
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
    [sidebarView setEnabled:enabled];
    [addCollectionBtn setEnabled:enabled];
}
- (IBAction)addCollectionAction:(id)sender{
    [self addCollection:([NSEvent modifierFlags] & NSAlternateKeyMask)==0];
}
- (void)addCollection:(BOOL)isSmart{
    id item;
    
    if(isSmart){
		item = [self.database addNewCollection:nil];
    } else {
		item = [self.database addNewSmartCollection:nil];
    }
	
	[self reloadData];    
    [self expandCollections:self];
    [self selectItem:item];
    [self startEditingItem:item];
}

- (void)reloadData{
	self.systems = self.database ? [self.database systems] : [NSArray array];
	self.collections = self.database ? [self.database collections] : [NSArray array];
	
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
    [sidebarView reloadData];
}

- (void)selectItem:(id)item{
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
	
    if(![item isSelectableInSdebar]) return;
    NSInteger index = [sidebarView rowForItem:item];
    if(index == -1) return;
    
    [sidebarView selectRowIndexes:[NSIndexSet indexSetWithIndex:index] byExtendingSelection:NO];
}

- (void)startEditingItem:(id)item{
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
    if(![item isEditableInSdebar]) return;
    
    NSInteger index = [sidebarView rowForItem:item];
    if(index == -1) return;
    
    [sidebarView editColumn:0 row:index withEvent:[[[NSEvent alloc] init] autorelease]  select:YES];
}
- (void)expandCollections:(id)sender{
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
    [sidebarView expandItem:[self.groups objectAtIndex:1]];
}
#pragma mark -
#pragma mark NSOutlineView Delegate
- (void)outlineViewSelectionDidChange:(NSNotification *)notification{
	OESidebarOutlineView* sidebarView = (OESidebarOutlineView*)[self view];
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
	  	return [self.systems objectAtIndex:index];
    }
    
    if( item == [self.groups objectAtIndex:1] ){
		return [self.collections objectAtIndex:index];
    }
    
    return nil;
}
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item{
    return [(id <OESidebarDataSourceItem>)item isGroupHeaderInSdebar];
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item{
    if( item == nil ){
		return [self.groups count];
    }
    
    if(!self.database)
		return 0;
    
    if( item == [self.groups objectAtIndex:0] ){
		int count = [self.systems count];
		return count;
    }
    
    if( item == [self.groups objectAtIndex:1] ){
		return [self.collections count];
    }
    
    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item{
    return [item sidebarName];
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item{
    self.editingItem = nil;
    
    
    if([[object stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@" "]] isNotEqualTo:@""]){
		[item setSidebarName:object];
		[self reloadData];
		
		NSInteger row = [outlineView rowForItem:item];
		[outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
    }
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
    if([cell isKindOfClass:[OESidebarCell class]]){
		[(OESidebarCell*)cell setImage:[item sidebarIcon]];
		[(OESidebarCell*)cell setIsGroup:[item isGroupHeaderInSdebar]];
		
		if(self.editingItem == nil)
			[(OESidebarCell*)cell setIsEditing:NO];
    }
}


- (void)setEditingItem:(id)newEdItem{
    [newEdItem retain];
    [editingItem release];
    
    editingItem = newEdItem;
}


- (void)removeSelectedItemsOfOutlineView:(NSOutlineView*)outlineView{
    NSIndexSet* indexes = [outlineView selectedRowIndexes];
    NSUInteger index = [indexes firstIndex];
    
    id item = [outlineView itemAtRow:index];
    BOOL removeItem = NO;
    if([item isEditableInSdebar]){  
		NSString* AlertSuppressedKey = @"ASRemoveCollection"; 
		BOOL alertSuppressed = [[NSUserDefaults standardUserDefaults] boolForKey:AlertSuppressedKey];
		if(!alertSuppressed){
			NSString* msg = NSLocalizedString(@"Errorrororo", @"");
			NSString* info = NSLocalizedString(@"Blubbbi die blub blub", @"");
			NSString* confirm = NSLocalizedString(@"Remove", @"");
			NSString* cancel = NSLocalizedString(@"Cancel", @"");
			
			NSAlert* alert = [NSAlert alertWithMessageText:msg defaultButton:confirm alternateButton:cancel otherButton:nil informativeTextWithFormat:info];
			[alert setShowsSuppressionButton:YES];
			
			removeItem = [alert runModal]==NSOKButton;
			if(removeItem){
				[[NSUserDefaults standardUserDefaults] setBool:[[alert suppressionButton] state] forKey:AlertSuppressedKey];		    
			}
		} else {
			removeItem = alertSuppressed;
		}
    }
    
    if(removeItem){
		[self.database removeCollection:item];
		
		// keep selection on last object if the one we removed was last
		if(index == [outlineView numberOfRows]-1){
			index --;
		}
		
		NSIndexSet* selIn = [[NSIndexSet alloc] initWithIndex:index];
		[outlineView selectRowIndexes:selIn byExtendingSelection:NO];
		[selIn release];
		[self reloadData];
    }
}
#pragma mark -
- (void)willHide{
	[addCollectionBtn setEnabled:NO];
}
- (void)willShow{
	[addCollectionBtn setEnabled:YES];
}
#pragma mark -
- (void)controlTextDidBeginEditing:(NSNotification *)aNotification{
    
}
- (void)controlTextDidEndEditing:(NSNotification *)aNotification{
    self.editingItem = nil;
}

@end
