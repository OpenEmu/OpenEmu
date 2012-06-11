//
//  OEImportViewController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 29.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OESideBarDataSourceItem.h"
#import "OEROMImporter.h"
@interface OEImportViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource, OEROMImporterDelegate>

@property (strong) IBOutlet NSProgressIndicator *progressIndicator;
@property (strong) IBOutlet NSTextField             *statusField;
@property (strong) IBOutlet NSTableView            *tableView;
@end
