/*
 Copyright (c) 2015, OpenEmu Team

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

#import "OELibraryGamesViewController.h"

#import "OEGameCollectionViewController.h"
#import "OESidebarController.h"
#import "OEGameScannerViewController.h"

@interface OELibraryGamesViewController () <OELibrarySplitViewDelegate>
@end

@implementation OELibraryGamesViewController

- (void)awakeFromNib
{
    [self _assignLibraryController];

    NSNotificationCenter *noc = [NSNotificationCenter defaultCenter];
    [noc addObserver:self selector:@selector(_updateCollectionContentsFromSidebar:) name:OESidebarSelectionDidChangeNotificationName object:[self sidebarController]];

    NSView *collectionView = [[self collectionController] view];
    NSView *collectionViewContainer = [self collectionViewContainer];

    [collectionView setFrame:[collectionViewContainer bounds]];
    [collectionView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [collectionViewContainer addSubview:collectionView];

    [self addChildViewController:[self sidebarController]];
    [self addChildViewController:[self collectionController]];
}

- (void)viewDidAppear
{
    [self _setupToolbar];
    [self _updateCollectionContentsFromSidebar:nil];
}

- (void)_setupToolbar
{
    OELibraryController *libraryController = [self libraryController];
    OELibraryToolbar *toolbar = [libraryController toolbar];

    [[toolbar addButton] setEnabled:YES];

    [[toolbar gridSizeSlider] setEnabled:YES];
    [[toolbar gridViewButton] setEnabled:YES];
    [[toolbar listViewButton] setEnabled:YES];

    NSSearchField *field = [toolbar searchField];
    [field setSearchMenuTemplate:nil];
    [field setEnabled:false];
    [field setStringValue:@""];
}

#pragma mark - OELibrarySubviewController
- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{
    return;
}

- (NSArray*)selectedGames
{
    return [[self collectionController] selectedGames];
}

- (void)setLibraryController:(OELibraryController *)libraryController
{
    _libraryController = libraryController;
    [self _assignLibraryController];
}

- (void)_assignLibraryController
{
    [[self sidebarController] setDatabase:[_libraryController database]];
    [[self collectionController] setLibraryController:_libraryController];
    [[self gameScannerController] setLibraryController:_libraryController];
}

#pragma mark - Toolbar
- (IBAction)addCollectionAction:(id)sender
{
    [[self sidebarController] addCollectionAction:sender];
}

- (IBAction)switchToGridView:(id)sender
{
    [[self collectionController] switchToGridView:sender];
}

- (IBAction)switchToListView:(id)sender
{
    [[self collectionController] switchToListView:sender];
}

- (IBAction)search:(id)sender
{
    [[self collectionController] search:sender];
}

- (IBAction)changeGridSize:(id)sender
{
    [[self collectionController] changeGridSize:sender];
}

#pragma mark - Sidebar handling
- (void)_updateCollectionContentsFromSidebar:(id)sender
{
    id selectedItem = [[self sidebarController] selectedSidebarItem];
    [[self collectionController] setRepresentedObject:selectedItem];
}

@end
