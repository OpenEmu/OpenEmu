/*
 Copyright (c) 2015, OpenEmu Team
 
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

#import "OEMigrationWindowController.h"
#import "OELibraryMigrator.h"

NS_ASSUME_NONNULL_BEGIN

@interface OEMigrationWindowController ()
@property (strong) NSMigrationManager *manager;
@end

@implementation OEMigrationWindowController

- (id)initWithMigrationManager:(NSMigrationManager *)manager
{
    self = [super initWithWindowNibName:self.windowNibName];
    if (self)
    {
        _manager = manager;
    }
    return self;
}

- (void)observeValueForKeyPath:(nullable NSString *)keyPath ofObject:(nullable id)object change:(nullable NSDictionary<NSString*, id> *)change context:(nullable void *)context
{
    float progress = self.manager.migrationProgress;
    self.indicator.doubleValue = progress;
    self.indicator.indeterminate = progress == 0.0 || (progress > 1.0 - FLT_EPSILON);
}


- (void)windowDidLoad
{
    [super windowDidLoad];
    
    self.window.title = NSLocalizedString(@"Updating Library", @"Library database migration window title");
    [self.indicator startAnimation:self];
}

- (void)showWindow:(nullable id)sender
{
    [self.manager addObserver:self forKeyPath:@"migrationProgress" options:NSKeyValueObservingOptionNew context:NULL];

    [super showWindow:sender];
}

- (void)close
{
    [self.manager removeObserver:self forKeyPath:@"migrationProgress"];
    [super close];
}

- (nullable NSString *)windowNibName
{
    return self.className;
}

#pragma mark -

- (IBAction)cancelMigration:(nullable id)sender
{
    NSError *error = [NSError errorWithDomain:OEMigrationErrorDomain code:OEMigrationCancelled userInfo:nil];
    [self.manager cancelMigrationWithError:error];
}

@end

NS_ASSUME_NONNULL_END
