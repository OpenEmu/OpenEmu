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

#import "OEDBImageMigrationPolicy.h"
#import "OELibraryDatabase.h"

NS_ASSUME_NONNULL_BEGIN

NSString * const OEDBImageMigrateImageFormat = @"OEDBImageMigrateImageFormat";

@implementation OEDBImageMigrationPolicy

- (BOOL)createDestinationInstancesForSourceInstance:(NSManagedObject *)oldObject entityMapping:(NSEntityMapping *)mapping manager:(NSMigrationManager *)manager error:(NSError **)error
{
    NSAssert(manager.sourceModel.versionIdentifiers.count == 1, @"Found a source model with various versionIdentifiers!");

    NSString *sourceVersion = manager.sourceModel.versionIdentifiers.anyObject;
    NSDictionary  *entities = manager.sourceModel. entitiesByName;

    // Version 1.1 and 1.0 both share version identifier 1.0 :/
    if([sourceVersion isEqualTo:@"1.0"] && entities[@"ImageThumbnail"] != nil)
    {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:OEDBImageMigrateImageFormat];

        NSSet *versions = [oldObject valueForKey:@"versions"];
        __block CGFloat size = 0;
        __block NSManagedObject *originalVersion = nil;
        [versions enumerateObjectsUsingBlock:^(NSManagedObject *aVersion, BOOL *stop) {
            CGFloat currentSize = [[aVersion valueForKey:@"width"] floatValue];
            if(currentSize > size)
            {
                originalVersion = aVersion;
                size = currentSize;
            }
        }];

        if(originalVersion)
        {
            NSURL *coverFolderURL = self.coverFolderURL;

            id width  = [originalVersion valueForKey:@"width"];
            id height = [originalVersion valueForKey:@"height"];
            id path   = [originalVersion valueForKey:@"relativePath"];
            id source = [oldObject valueForKey:@"sourceURL"];
            id format = @(-1);
            [versions enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
                NSString *aPath = [obj valueForKey:@"relativePath"];
                if([aPath isNotEqualTo:path])
                {
                    NSURL *imageURL = [NSURL URLWithString:aPath relativeToURL:coverFolderURL];
                    [[NSFileManager defaultManager] removeItemAtURL:imageURL error:nil];
                }
            }];

            NSManagedObject *newObject = [NSEntityDescription insertNewObjectForEntityForName:@"Image" inManagedObjectContext:[manager destinationContext]];

            [newObject setValue:width  forKey:@"width"];
            [newObject setValue:height forKey:@"height"];
            [newObject setValue:path   forKey:@"relativePath"];
            [newObject setValue:source forKey:@"source"];
            [newObject setValue:format forKey:@"format"];

            [manager associateSourceInstance:oldObject withDestinationInstance:newObject forEntityMapping:mapping];

            return YES;
        }
    }

    return NO;
}

- (NSURL *)coverFolderURL
{
    NSUserDefaults *standardDefaults  = [NSUserDefaults standardUserDefaults];
    NSString       *libraryFolderPath = [[standardDefaults stringForKey:OEDatabasePathKey] stringByExpandingTildeInPath];
    NSString       *coverFolderPath   = [libraryFolderPath stringByAppendingPathComponent:@"Artwork/"];

    NSURL *baseURL = [NSURL fileURLWithPath:coverFolderPath isDirectory:YES];
    return baseURL;
}

@end

NS_ASSUME_NONNULL_END
