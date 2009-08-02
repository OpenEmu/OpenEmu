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

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@class IKImageBrowserView, IKImageFlowView;

@class OECorePlugin, GameDocumentController;
@interface OESaveStateController : NSWindowController {
	NSPredicate					*availablePluginsPredicate;
    NSIndexSet					*selectedPlugins;
	NSArray						*sortDescriptors;
    OECorePlugin				*currentPlugin;	
	GameDocumentController		*docController;
	
	IBOutlet NSArrayController	*savestateController;
    IBOutlet NSArrayController	*pluginController;
	IBOutlet NSTreeController	*treeController;
	IBOutlet NSPredicate *selectedRomPredicate;
	
	IBOutlet NSView	*listView;
	IBOutlet NSView *collectionView;
	
	IBOutlet NSOutlineView* outlineView;
	IBOutlet IKImageBrowserView* imageBrowser;
	IBOutlet IKImageFlowView* imageFlow;
	
	float browserZoom;
	
	NSMutableArray* pathArray;
	NSMutableArray* pathRanges;
	
	IBOutlet NSView *holderView;
}

@property(assign) float browserZoom;
@property(retain) GameDocumentController *docController;
@property(readonly) NSArray     *plugins;
@property(retain)   NSPredicate *availablePluginsPredicate;
@property(retain)   NSIndexSet  *selectedPlugins;
@property(retain)	NSPredicate *selectedRomPredicate;
@property(retain)	NSArray		*sortDescriptors;
@property(retain)	NSMutableArray	*pathArray;
@property(retain)	NSMutableArray	*pathRanges;

- (IBAction) toggleViewType:(id) sender;

@end
