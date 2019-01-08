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

#import "OEDBSaveStateMigrationPolicy.h"
#import "OELibraryDatabase.h"
#import "NSArray+OEAdditions.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

@implementation OEDBSaveStateMigrationPolicy

- (BOOL)createDestinationInstancesForSourceInstance:(NSManagedObject *)oldObject entityMapping:(NSEntityMapping *)mapping manager:(NSMigrationManager *)manager error:(NSError **)error
{
    NSAssert(manager.sourceModel.versionIdentifiers.count == 1, @"Found a source model with various versionIdentifiers!");
    
    NSString *version = manager.sourceModel.versionIdentifiers. anyObject;
    
    if([version isEqualTo:@"1.0 Beta"])
    {
        NSString *path     = [oldObject valueForKey:@"path"];
        NSURL    *url      = [NSURL URLWithString:path];
        NSString *location = url.absoluteString;
        if(location)
        {
            NSArray<NSPropertyMapping *> *attributeMappings = mapping.attributeMappings;
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
                                          ^ BOOL (NSPropertyMapping *obj)
                                          {
                                              return [[obj name] isEqualToString:@"location"];
                                          }];
            mapping.valueExpression = [NSExpression expressionForConstantValue:location];
        }
    }
    else if([version isEqualToString:@"1.2"])
    {
        NSURL *romsFolderURL = self.stateFolderURL;
        NSString *urlString = [oldObject valueForKey:@"location"];
        NSURL *url = nil;
        if(![urlString containsString:@"file://"])
            url = [NSURL URLWithString:urlString relativeToURL:romsFolderURL];
        else
            url = [NSURL URLWithString:urlString];

        NSURL *relativeURL = [url URLRelativeToURL:romsFolderURL];
        NSString *location = relativeURL.relativeString;
        if(location)
        {
            // make sure we don't save trailing '/' for save state bundles
            if([location characterAtIndex:location.length - 1] == '/')
                location = [location substringToIndex:location.length - 1];

            NSArray<NSPropertyMapping *> *attributeMappings = mapping.attributeMappings;
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
                                          ^ BOOL (NSPropertyMapping *obj)
                                          {
                                              return [[obj name] isEqualToString:@"location"];
                                          }];
            mapping.valueExpression = [NSExpression expressionForConstantValue:location];
        }
    }

    return [super createDestinationInstancesForSourceInstance:oldObject entityMapping:mapping manager:manager error:error];
}

- (NSURL *)stateFolderURL
{
    if([[NSUserDefaults standardUserDefaults] objectForKey:OESaveStateFolderURLKey])
        return [[NSUserDefaults standardUserDefaults] URLForKey:OESaveStateFolderURLKey];

    NSString *saveStateFolderName = @"Save States";
    NSURL    *result = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
    result = [result URLByAppendingPathComponent:@"OpenEmu" isDirectory:YES];
    result = [result URLByAppendingPathComponent:saveStateFolderName isDirectory:YES];

    return result.standardizedURL;
}
@end

NS_ASSUME_NONNULL_END
