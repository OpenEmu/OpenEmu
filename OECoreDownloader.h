//
//  OECoreDownloader.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 8/2/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GameDocumentController;

@interface OECoreDownloader :  NSWindowController {
	NSMutableArray* appcasts;
	NSMutableArray* downloads;
	NSArray* urlList;
	GameDocumentController* docController;
	
	NSMutableDictionary* downloadToPathMap;
	IBOutlet NSArrayController* downloadArrayController;
}

@property (readonly) NSArray* downloads;

- (IBAction) downloadSelectedCores: (id) sender;

@end
