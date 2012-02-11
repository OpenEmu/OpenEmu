/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import <Cocoa/Cocoa.h>
#import "IKSGridView.h"
#import "IKSGridViewProtocols.h"
#import "IKImageFlowView.h"

#import "OECollectionViewItemProtocol.h"

@class OELibraryController;
@class OEHorizontalSplitView;
@interface OECollectionViewController : NSViewController <IKSGridViewDelegate, IKSGridViewDataSource, NSTableViewDelegate, NSTableViewDataSource>
{
@private
    NSArrayController *gamesController;
    
    id<OECollectionViewItemProtocol> collectionItem;
    
    IBOutlet NSView *gridViewContainer;// gridview
    IBOutlet IKSGridView *gridView;// scrollview for gridview
    
    IBOutlet OEHorizontalSplitView *flowlistViewContainer; // cover flow and simple list container
    IBOutlet IKImageFlowView *coverFlowView;
    IBOutlet NSTableView *listView;
    
    NSTimer *reloadTimer;
}

#pragma mark -
- (NSArray *)selectedGames;
#pragma mark -
#pragma mark View Selection
- (IBAction)selectGridView:(id)sender;
- (IBAction)selectFlowView:(id)sender;
- (IBAction)selectListView:(id)sender;

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)search:(id)sender;
- (IBAction)changeGridSize:(id)sender;

#pragma mark -
- (void)willShow;

- (void)setNeedsReload;
#pragma mark -
@property(unsafe_unretained) IBOutlet OELibraryController *libraryController;
@property(nonatomic, strong) id <OECollectionViewItemProtocol> collectionItem;

@property(nonatomic, strong) IBOutlet NSView *emptyCollectionView;
@property(nonatomic, strong) IBOutlet NSView *emptyConsoleView;
@end
