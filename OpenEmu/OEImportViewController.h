//
//  OEImportViewController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 29.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "OEROMImporter.h"
#import "OELibrarySubviewController.h"
@interface OEImportViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource, OEROMImporterDelegate, OELibrarySubviewController>

#pragma mark - UI Methods
- (IBAction)togglePause:(id)sender;
- (IBAction)cancel:(id)sender;

@property (strong) IBOutlet NSProgressIndicator *progressIndicator;
@property (strong) IBOutlet NSTextField         *statusField;
@property (strong) IBOutlet NSTableView         *tableView;
@end
