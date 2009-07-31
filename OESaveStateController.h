//
//  OESaveStateController.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/28/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@class IKImageBrowserView;

@class OECorePlugin, GameDocumentController;

@interface OESaveStateController : NSWindowController {
	NSPredicate					*availablePluginsPredicate;
    NSIndexSet					*selectedPlugins;
	NSArray						*sortDescriptors;
    OECorePlugin				*currentPlugin;	
	GameDocumentController		*docController;
	
	IBOutlet NSArrayController	*savestateController;
    IBOutlet NSArrayController	*pluginController;
	IBOutlet NSPredicate *selectedRomPredicate;
	
	IBOutlet NSView	*listView;
	IBOutlet NSView *collectionView;
	
	IBOutlet IKImageBrowserView* imageBrowser;
	
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
