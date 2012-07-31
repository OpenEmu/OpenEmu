//
//  OEImportViewController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 29.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OESidebarItem.h"
#import "OEROMImporter.h"

#import "OELibrarySubviewController.h"

@interface OEImportViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource, OEROMImporterDelegate, OELibrarySubviewController>

@property (strong) IBOutlet NSProgressIndicator *progressIndicator;
@property (strong) IBOutlet NSTextField             *statusField;
@property (strong) IBOutlet NSTableView            *tableView;
@end
