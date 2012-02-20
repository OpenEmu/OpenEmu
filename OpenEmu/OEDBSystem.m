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

@implementation OEDBSystem

+ (id)systemFromPlugin:(OESystemPlugin *)plugin inDatabase:(OELibraryDatabase *)database
{
    NSString *systemIdentifier = [plugin systemIdentifier];
    OEDBSystem *system = [database systemWithIdentifier:systemIdentifier];
    
    if(system) return system;
    
    NSManagedObjectContext *moc = [database managedObjectContext];
    
    system = [[OEDBSystem alloc] initWithEntity:[self entityDescriptionInContext:moc] insertIntoManagedObjectContext:moc];
    // TODO: get archive id(s) from plugin
    [system setValue:systemIdentifier forKey:@"systemIdentifier"];
    [system setValue:[system name] forKey:@"lastLocalizedName"];
    
    return system;
}

+ (id)systemForPluginIdentifier:(NSString *)identifier inDatabase:(OELibraryDatabase *)database
{
    return [database systemWithIdentifier:identifier];
}

+ (id)systemForArchiveID:(NSNumber *)archiveID
{
    return [self systemForArchiveID:archiveID inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForArchiveID:(NSNumber *)archiveID inDatabase:(OELibraryDatabase *)database
{
    return [database systemWithArchiveID:archiveID];
}

+ (id)systemForArchiveName:(NSString *)name
{
    return [self systemForArchiveName:name inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForArchiveName:(NSString *)name inDatabase:(OELibraryDatabase *)database
{
    return [database systemWithArchiveName:name];
}

+ (id)systemForArchiveShortName:(NSString *)shortName
{
    return [self systemForArchiveShortName:shortName inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForArchiveShortName:(NSString *)shortName inDatabase:(OELibraryDatabase *)database
{
    return [database systemWithArchiveShortname:shortName];
}

+ (id)systemForFile:(NSString *)filepath;
{
    return [self systemForFile:filepath inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)systemForFile:(NSString *)filepath inDatabase:(OELibraryDatabase *)database
{
    return [database systemForFile:filepath];
}

#pragma mark -
#pragma mark Core Data utilities

+ (NSString *)entityName
{
    return @"System";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

#pragma mark -

- (OESystemPlugin *)plugin
{
    NSString *systemIdentifier = [self valueForKey:@"systemIdentifier"];
    OESystemPlugin *plugin = [OESystemPlugin gameSystemPluginForIdentifier:systemIdentifier];
    
    return plugin;
}

#pragma mark -

- (NSImage *)icon
{
    return [[self plugin] systemIcon];
}

- (NSString *)name
{
    return [[self plugin] systemName] ? : [self valueForKey:@"lastLocalizedName"];
}

@end
