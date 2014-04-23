//
//  OEMigrationWindowController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/04/14.
//
//

#import <Cocoa/Cocoa.h>

@interface OEMigrationWindowController : NSWindowController
- (id)initWithMigrationManager:(NSMigrationManager*)manager;
- (IBAction)cancelMigration:(id)sender;

@property (assign) IBOutlet NSProgressIndicator *indicator;
@property (assign) IBOutlet NSButton            *cancelButton;
@end
