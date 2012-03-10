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

#import <Foundation/Foundation.h>

@class OESystemPlugin, OELibraryDatabase;
@interface OEDBSystem : NSManagedObject

+ (id)systemFromPlugin:(OESystemPlugin *)plugin inDatabase:(OELibraryDatabase *)database;
+ (id)systemForPluginIdentifier:(NSString *)identifier inDatabase:(OELibraryDatabase *)database;
+ (id)systemForArchiveID:(NSNumber *)number;
+ (id)systemForArchiveID:(NSNumber *)number inDatabase:(OELibraryDatabase *)database;
+ (id)systemForArchiveName:(NSString *)name;
+ (id)systemForArchiveName:(NSString *)name inDatabase:(OELibraryDatabase *)database;
+ (id)systemForArchiveShortName:(NSString *)shortName;
+ (id)systemForArchiveShortName:(NSString *)shortName inDatabase:(OELibraryDatabase *)database;
+ (id)systemForURL:(NSURL *)url;
+ (id)systemForURL:(NSURL *)url  inDatabase:(OELibraryDatabase *)database;

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark -
#pragma mark Data Model Properties
@property (nonatomic, retain) NSString  *lastLocalizedName;
@property (nonatomic, retain) NSString  *shortname;
@property (nonatomic, retain) NSString  *systemIdentifier;
@property (nonatomic, retain) NSNumber  *archiveID;
@property (nonatomic, retain) NSString  *archiveName;
@property (nonatomic, retain) NSString  *archiveShortname;
@property (nonatomic, retain) NSNumber  *enabled;

#pragma mark -
#pragma mark Data Model Relationships
@property (nonatomic, retain)   NSSet         *games;
@property (nonatomic, readonly) NSMutableSet  *mutableGames;

#pragma mark -
- (OESystemPlugin *)plugin;

@property(readonly) NSImage  *icon;
@property(readonly) NSString *name;
@end
