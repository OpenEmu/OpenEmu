//
//  OEDBConsole.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OELibraryDatabase.h"
@implementation OEDBSystem
+ (id)systemFromPlugin:(OESystemPlugin*)plugin inDatabase:(OELibraryDatabase*)database
{
    NSString* systemIdentifier = [plugin systemIdentifier];
    OEDBSystem* system = [database systemWithIdentifier:systemIdentifier];
    
    if(system) return system;
    
    NSManagedObjectContext* moc = [database managedObjectContext];
    
    system = [[OEDBSystem alloc] initWithEntity:[self entityDescriptionInContext:moc] insertIntoManagedObjectContext:moc];
    // TODO: get archive id(s) from plugin
    [system setValue:systemIdentifier forKey:@"systemIdentifier"];
    [system setValue:[system name] forKey:@"lastLocalizedName"];
    
    return [system autorelease];
}

+ (id)systemForPluginIdentifier:(NSString*)identifier inDatabase:(OELibraryDatabase*)database
{
    return [database systemWithIdentifier:identifier];
}

+ (id)systemForArchiveID:(NSNumber*)archiveID
{
    return [self systemForArchiveID:archiveID inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForArchiveID:(NSNumber*)archiveID inDatabase:(OELibraryDatabase*)database
{
    OEDBSystem* system = [database systemWithArchiveID:archiveID];
    return system;
}

+ (id)systemForArchiveName:(NSString*)name
{
    return [self systemForArchiveName:name inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForArchiveName:(NSString*)name inDatabase:(OELibraryDatabase*)database
{
    OEDBSystem* system = [database systemWithArchiveName:name];
    return system;
}

+ (id)systemForArchiveShortName:(NSString*)shortName
{
    return [self systemForArchiveShortName:shortName inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForArchiveShortName:(NSString*)shortName inDatabase:(OELibraryDatabase*)database
{
    OEDBSystem* system = [database systemWithArchiveShortname:shortName];
    return system;
}

+ (id)systemForFile:(NSString*)filepath;
{
    return [self systemForFile:filepath inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForFile:(NSString*)filepath inDatabase:(OELibraryDatabase*)database
{
    return [database systemForFile:filepath];
}
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName{
    return @"System";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}
#pragma mark -
- (OESystemPlugin*)plugin{
    NSString* systemIdentifier = [self valueForKey:@"systemIdentifier"];
    OESystemPlugin* plugin = [OESystemPlugin gameSystemPluginForIdentifier:systemIdentifier];
    
    return plugin;
}
#pragma mark -
- (NSImage*)icon{
    NSString* locName = [self name];
    NSImage* image = [NSImage imageNamed:locName];
    
    return image;
}

- (NSString*)name{
    if([self plugin])   return [[self plugin] systemName];
    else                return [self valueForKey:@"lastLocalizedName"];
}

@end
