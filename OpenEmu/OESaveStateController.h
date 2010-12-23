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

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@class OESaveState;
@class IKImageBrowserView, IKImageFlowView;

@class OECorePlugin, GameDocumentController;

@interface OESaveStateController : NSWindowController
{
    NSPredicate         *availablePluginsPredicate;
    NSIndexSet          *selectedPlugins;
    NSArray             *sortDescriptors;
    OECorePlugin        *currentPlugin;    
    
    NSArrayController   *romFileController;
    NSArrayController   *savestateController;
    NSArrayController   *pluginController;
    NSTreeController    *treeController;
    NSPredicate         *selectedPluginsPredicate;
    NSPredicate         *selectedRomPredicate;
    NSPredicate         *searchPredicate;
    
    NSView              *listView;
    NSView              *collectionView;
    NSView              *holderView;
    
    NSSearchField       *searchField;
    
    NSOutlineView       *outlineView;
    IKImageBrowserView  *imageBrowser;
    IKImageFlowView     *imageFlow;
    
    NSMenu              *contextMenu;
    
    NSSegmentedControl  *segmentButton;
    
    NSMutableDictionary *romFileIndexes;
    NSMutableArray      *sortedSaveStates;
    
    float                browserZoom;
}

@property(retain) IBOutlet NSArrayController  *romFileController;
@property(retain) IBOutlet NSArrayController  *savestateController;
@property(retain) IBOutlet NSArrayController  *pluginController;
@property(retain) IBOutlet NSTreeController   *treeController;
@property(retain) IBOutlet NSPredicate        *selectedRomPredicate;

@property(retain) IBOutlet NSView             *listView;
@property(retain) IBOutlet NSView             *collectionView;
@property(retain) IBOutlet NSView             *holderView;

@property(retain) IBOutlet NSOutlineView      *outlineView;
@property(retain) IBOutlet IKImageBrowserView *imageBrowser;
@property(retain) IBOutlet IKImageFlowView    *imageFlow;

@property(retain) IBOutlet NSSearchField      *searchField;

@property(retain) IBOutlet NSMenu             *contextMenu;

@property(retain) IBOutlet NSSegmentedControl *segmentButton;

@property(assign)           float              browserZoom;
@property(readonly)         NSArray           *plugins;
@property(retain)           NSPredicate       *availablePluginsPredicate;
@property(retain,nonatomic) NSIndexSet        *selectedPlugins;
@property(retain)           NSArray           *sortDescriptors;

- (IBAction)exportSave:(id)sender;
- (IBAction)toggleViewType:(NSSegmentedControl *)sender;
- (IBAction)deleteState:(id)sender;
- (void)updateRomGroups;

- (NSArray *)allROMs;

- (OESaveState *)selectedSaveState;
- (OESaveState *)saveStateAtAbsoluteIndex:(NSUInteger)index;
@end
