//
//  OEMigrationWindowController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/04/14.
//
//

#import "OEMigrationWindowController.h"

@interface OEMigrationWindowController ()
@property (strong) NSMigrationManager *manager;
@end

@implementation OEMigrationWindowController

- (id)initWithMigrationManager:(NSMigrationManager*)manager
{
    self = [super initWithWindowNibName:[self windowNibName]];
    if (self)
    {
        [self setManager:manager];
        [manager addObserver:self forKeyPath:@"migrationProgress" options:NSKeyValueObservingOptionNew context:NULL];
    }
    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    float progress = [[self manager] migrationProgress];
    [[self indicator] setDoubleValue:progress];
    [[self indicator] setIndeterminate:progress==0.0 || (progress > 1.0-FLT_EPSILON)];
}


- (void)windowDidLoad
{
    [super windowDidLoad];
    [[self indicator] startAnimation:self];
}

- (NSString*)windowNibName
{
    return [self className];
}
#pragma mark -
- (IBAction)cancelMigration:(id)sender
{

}

@end
