/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OERetrodeViewController.h"
#import "OECoverGridViewCell.h"
#import "OERetrode.h"
@interface OERetrodeViewController ()
@end

@implementation OERetrodeViewController

- (void)awakeFromNib
{
    [[self gridView] setDataSource:self];
    [[self gridView] setDelegate:self];
}

- (NSString*)nibName
{
    return @"OERetrodeViewController";
}

- (void)setRepresentedObject:(id)representedObject
{
    NSAssert([representedObject isKindOfClass:[OERetrode class]], @"Retrode View Controller can only represent OERetrode objects.");
    [super setRepresentedObject:representedObject];
    [representedObject setDelegate:self];
}
#pragma mark - OELibrarySubviewController Implementation -
- (id)encodeCurrentState
{
    return nil;
}

- (void)restoreState:(id)state
{}

- (NSArray*)selectedGames
{
    return @[];
}

- (void)setLibraryController:(OELibraryController *)controller
{
    [[controller toolbarGridViewButton] setEnabled:FALSE];
    [[controller toolbarFlowViewButton] setEnabled:FALSE];
    [[controller toolbarListViewButton] setEnabled:FALSE];
    
    [[controller toolbarSearchField] setEnabled:NO];
    [[controller toolbarSlider] setEnabled:NO];
}
#pragma mark - Retrode Delegate
- (void)retrodeDidDisconnect:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeDidRescanGames:(OERetrode*)retrode
{
    DLog();
    [[self gridView] reloadData];
}
- (void)retrodeHardwareDidBecomeAvailable:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeHardwareDidBecomeUnavailable:(OERetrode*)retrode
{
    DLog();
}

- (void)retrodeDidMount:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeDidUnmount:(OERetrode*)retrode
{
    DLog();
}

- (void)retrodeDidEnterDFUMode:(OERetrode*)retrode
{
    DLog();
}
- (void)retrodeDidLeaveDFUMode:(OERetrode*)retrode
{
    DLog();
}

#pragma mark - GridView DataSource -
- (OEGridViewCell *)gridView:(OEGridView *)view cellForItemAtIndex:(NSUInteger)index
{
    /*
    OERetrode *retrode = [self representedObject];
    id game = [[retrode games] objectAtIndex:index];
    
    OECoverGridViewCell *item = (OECoverGridViewCell *)[view cellForItemAtIndex:index makeIfNecessary:NO];
    
    if(item == nil) item = (OECoverGridViewCell *)[view dequeueReusableCell];
    if(item == nil) item = [[OECoverGridViewCell alloc] init];
    
    [item setImageSize:[view itemSize]];
    [item setImage:nil];
    
    [item setTitle:[game objectForKey:@"gameTitle"]];

    return item;
     */
    return nil;
}

- (NSUInteger)numberOfItemsInGridView:(OEGridView *)gridView
{
    return [[[self representedObject] games] count];
}
/*
- (void)gridView:(OEGridView *)gridView willBeginEditingCellForItemAtIndex:(NSUInteger)index;
- (void)gridView:(OEGridView *)gridView didEndEditingCellForItemAtIndex:(NSUInteger)index;
- (id<NSPasteboardWriting>)gridView:(OEGridView *)gridView pasteboardWriterForIndex:(NSInteger)index;
- (NSMenu *)gridView:(OEGridView *)gridView menuForItemsAtIndexes:(NSIndexSet *)indexes;
*/
#pragma mark - GridView Delegate -
/*
- (void)selectionChangedInGridView:(OEGridView *)gridView;
- (void)gridView:(OEGridView *)gridView doubleClickedCellForItemAtIndex:(NSUInteger)index;
- (NSDragOperation)gridView:(OEGridView *)gridView validateDrop:(id<NSDraggingInfo>)sender;
- (NSDragOperation)gridView:(OEGridView *)gridView draggingUpdated:(id<NSDraggingInfo>)sender;
- (BOOL)gridView:(OEGridView *)gridView acceptDrop:(id<NSDraggingInfo>)sender;
- (BOOL)gridView:(OEGridView *)gridView shouldTypeSelectForEvent:(NSEvent *)event withCurrentSearchString:(NSString *)searchString;
- (NSString*)gridView:(OEGridView *)gridView typeSelectStringForItemAtIndex:(NSUInteger)idx;
*/
@end
