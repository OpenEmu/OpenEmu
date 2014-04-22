/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OELibraryDatabase.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

NSString * const OEDBSystemsDidChangeNotification = @"OEDBSystemsDidChangeNotification";
@implementation OEDBSystem
+ (NSInteger)systemsCount
{
    return [self systemsCountInDatabase:[OELibraryDatabase defaultDatabase]];
}
+ (NSInteger)systemsCountInDatabase:(OELibraryDatabase *)database
{
    return [self systemsCountInDatabase:[OELibraryDatabase defaultDatabase] error:nil];
}

+ (NSInteger)systemsCountInDatabase:(OELibraryDatabase *)database error:(NSError**)outError
{
    __block NSInteger result = 0;
    NSManagedObjectContext *context = [database safeContext];
    [context performBlockAndWait:^{
        NSEntityDescription    *descr    = [self entityDescriptionInContext:context];
        NSFetchRequest         *req      = [[NSFetchRequest alloc] init];

        [req setEntity:descr];

        NSError *error    = outError != NULL ? *outError : nil;
        result = [database countForFetchRequest:req error:&error];
        if(result == NSNotFound)
        {
            result = 0;
            DLog(@"Error: %@", error);
        }
    }];
    
    return result;
}

+ (NSArray*)allSystems
{
    return [self allSystemsInDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (NSArray*)allSystemsInDatabase:(OELibraryDatabase *)database
{
    return [self allSystemsInDatabase:database error:nil];
}

+ (NSArray*)allSystemsInDatabase:(OELibraryDatabase *)database error:(NSError**)outError
{
    NSError     *error    = outError != NULL ? *outError : nil;
    NSArray     *sortDesc = @[[NSSortDescriptor sortDescriptorWithKey:@"lastLocalizedName" ascending:YES]];
    NSArray     *result   = [self OE_queryDatabase:database predicate:nil sortDescriptors:sortDesc limit:-1 error:&error];
    if(result == nil)
        DLog(@"Error: %@", error);
    
    return result;
}

+ (NSArray*)allSystemIdentifiers
{
    NSArray *allSystems = [self allSystems];
    NSMutableArray *allSystemIdentifiers = [NSMutableArray arrayWithCapacity:[allSystems count]];

    for(OEDBSystem *system in allSystems)
    {
        [allSystemIdentifiers addObject:[system systemIdentifier]];
    }

    return allSystemIdentifiers;
}

+ (NSArray*)enabledSystems
{
    return [self enabledSystemsInDatabase:[OELibraryDatabase defaultDatabase]];
}
+ (NSArray*)enabledSystemsInDatabase:(OELibraryDatabase *)database
{
    return [self enabledSystemsInDatabase:database error:nil];
}
+ (NSArray*)enabledSystemsInDatabase:(OELibraryDatabase *)database error:(NSError**)outError
{
    NSError     *error    = outError != NULL ? *outError : nil;
    NSArray     *sortDesc = @[[NSSortDescriptor sortDescriptorWithKey:@"lastLocalizedName" ascending:YES]];
    NSPredicate *pred     = [NSPredicate predicateWithFormat:@"enabled = YES"];
    NSArray     *result   = [self OE_queryDatabase:database predicate:pred sortDescriptors:sortDesc limit:-1 error:&error];
    if(result == nil)
        DLog(@"Error: %@", error);
    
    return result;
}

+ (NSArray*)systemsForFileWithURL:(NSURL *)url
{
    return [self systemsForFileWithURL:url error:nil];
}
+ (NSArray*)systemsForFileWithURL:(NSURL *)url error:(NSError**)error
{
    return [self systemsForFileWithURL:url inDatabase:[OELibraryDatabase defaultDatabase] error:error];
}
+ (NSArray*)systemsForFileWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database
{
    return [self systemsForFileWithURL:url inDatabase:[OELibraryDatabase defaultDatabase] error:nil];
}
+ (NSArray*)systemsForFileWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError**)error
{
    __block OESystemPlugin *theOneAndOnlySystemThatGetsAChanceToHandleTheFile = nil;
    NSMutableArray         *otherSystemsThatMightBeAbleToHandleTheFile = [NSMutableArray array];
    
    NSString *fileExtension = [url pathExtension];
    
    [[OESystemPlugin allPlugins] enumerateObjectsUsingBlock:^(OESystemPlugin *systemPlugin, NSUInteger idx, BOOL *stop) {
        if([[systemPlugin controller] canHandleFileExtension:fileExtension])
        {
            OECanHandleState handleState = [[systemPlugin controller] canHandleFile:[url path]];

            if (handleState == OECanHandleYes)
            {
                theOneAndOnlySystemThatGetsAChanceToHandleTheFile = systemPlugin;
                *stop = YES;
            }
            else if (handleState != OECanHandleNo)
            {
                [otherSystemsThatMightBeAbleToHandleTheFile addObject:systemPlugin];
            }
        }
    }];
    
    
    if(theOneAndOnlySystemThatGetsAChanceToHandleTheFile != nil)
    {
        NSString *systemIdentifier = [theOneAndOnlySystemThatGetsAChanceToHandleTheFile systemIdentifier];
        OEDBSystem *system = [self systemForPluginIdentifier:systemIdentifier inDatabase:database];
        if([[system enabled] boolValue])
            return [NSArray arrayWithObject:system];
    }
    else
    {
        NSMutableArray *validSystems = [NSMutableArray arrayWithCapacity:[otherSystemsThatMightBeAbleToHandleTheFile count]];
        [otherSystemsThatMightBeAbleToHandleTheFile enumerateObjectsUsingBlock:
         ^(OESystemPlugin *obj, NSUInteger idx, BOOL *stop) {
             NSString *systemIdentifier = [obj systemIdentifier];
             OEDBSystem *system = [self systemForPluginIdentifier:systemIdentifier inDatabase:database];
             if([[system enabled] boolValue])
                 [validSystems addObject:system];
        }];
        return validSystems;
    }
    
    return [NSArray array];
}

+ (NSString*)headerForFileWithURL:(NSURL *)url forSystem:(NSString *)identifier
{
    OESystemPlugin *systemPlugin = [OESystemPlugin systemPluginForIdentifier:identifier];
    NSString *header = [[systemPlugin controller] headerLookupForFile:[url path]];
    
    return header;
}

+ (NSString*)serialForFileWithURL:(NSURL *)url forSystem:(NSString *)identifier
{
    OESystemPlugin *systemPlugin = [OESystemPlugin systemPluginForIdentifier:identifier];
    NSString *serial = [[systemPlugin controller] serialLookupForFile:[url path]];
    
    return serial;
}

+ (id)systemForPlugin:(OESystemPlugin *)plugin inDatabase:(OELibraryDatabase *)database
{
    NSString *systemIdentifier = [plugin systemIdentifier];
    __block OEDBSystem *system = [self systemForPluginIdentifier:systemIdentifier inDatabase:database];

    if(system) return system;

    NSManagedObjectContext *moc = [database safeContext];
    [moc performBlockAndWait:^{

        system = [[OEDBSystem alloc] initWithEntity:[self entityDescriptionInContext:moc] insertIntoManagedObjectContext:moc];
        [system setSystemIdentifier:systemIdentifier];
        [system setLastLocalizedName:[system name]];
    }];

    return system;
}

+ (id)systemForPluginIdentifier:(NSString *)identifier inDatabase:(OELibraryDatabase *)database
{
    NSError *error    = nil;
    
    NSPredicate *pred = [NSPredicate predicateWithFormat:@"systemIdentifier == %@", identifier];
    NSArray *result   = [self OE_queryDatabase:database predicate:pred sortDescriptors:nil limit:1 error:&error];
    if(result == nil)
        DLog(@"Error: %@", error);
    
    return [result lastObject];
}

+ (id)OE_queryDatabase:(OELibraryDatabase *)database predicate:(NSPredicate*)pred sortDescriptors:(NSArray*)sort limit:(NSInteger)limit error:(NSError**)error
{
    NSFetchRequest         *req      = [[NSFetchRequest alloc] initWithEntityName:[self entityName]];
    if(pred)
        [req setPredicate:pred];
    if(sort)
        [req setSortDescriptors:sort];
    if(limit != -1)
        [req setFetchLimit:limit];
    
    return [database executeFetchRequest:req error:error];
}

- (CGFloat)coverAspectRatio
{
    if([self plugin])
    {
        CGFloat aspectRatio = [[self plugin] coverAspectRatio];
        if(aspectRatio != 0)
            return aspectRatio;
    }

    // in case system plugin has been removed return a default value
    return 1.365385;
}
#pragma mark - Core Data utilities

+ (NSString *)entityName
{
    return @"System";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

#pragma mark -
#pragma mark Data Model Properties
@dynamic lastLocalizedName, shortname, systemIdentifier, enabled;

#pragma mark -
#pragma mark Data Model Relationships
@dynamic games;
- (NSMutableSet*)mutableGames
{
    return [self mutableSetValueForKey:@"games"];
}

#pragma mark -
- (OESystemPlugin *)plugin
{
    NSString *systemIdentifier = [self systemIdentifier];
    OESystemPlugin *plugin = [OESystemPlugin systemPluginForIdentifier:systemIdentifier];
    
    return plugin;
}

- (NSImage *)icon
{
    return [[self plugin] systemIcon];
}

- (NSString *)name
{
    return [[self plugin] systemName] ? : [self lastLocalizedName];
}

#pragma mark - Debug

- (void)dump
{
    [self dumpWithPrefix:@"---"];
}

- (void)dumpWithPrefix:(NSString *)prefix
{
    NSString *subPrefix = [prefix stringByAppendingString:@"-----"];
    NSLog(@"%@ Beginning of system dump", prefix);

    NSLog(@"%@ System last localized name is %@", prefix, [self lastLocalizedName]);
    NSLog(@"%@ short name is %@", prefix, [self shortname]);
    NSLog(@"%@ system identifier is %@", prefix, [self systemIdentifier]);
    NSLog(@"%@ enabled? %s", prefix, BOOL_STR([self enabled]));

    NSLog(@"%@ Number of games in this system is %lu", prefix, (unsigned long)[[self games] count]);

    for(id game in [self games])
    {
        if([game respondsToSelector:@selector(dumpWithPrefix:)]) [game dumpWithPrefix:subPrefix];
        else NSLog(@"%@ Game is %@", subPrefix, game);
    }

    NSLog(@"%@ End of system dump\n\n", prefix);
}

@end
