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

#import <Cocoa/Cocoa.h>

enum _OEDBGameStatus
{
    OEDBGameStatusOK,
    OEDBGameStatusProcessing,
    OEDBGameStatusAlert
};
typedef NSUInteger OEDBGameStatus;

extern NSString *const OEPasteboardTypeGame;

@class OELibraryDatabase;
@class OEDBRom;

@interface OEDBGame : NSManagedObject <NSPasteboardWriting, NSPasteboardReading>
#pragma mark -
#pragma mark Creating and Obtaining OEDBGames
// returns the game with objID in the default database
+ (id)gameWithID:(NSManagedObjectID*)objID;
// returns the game with objID in the specified database
+ (id)gameWithID:(NSManagedObjectID*)objID inDatabase:(OELibraryDatabase*)database;

// returns the game with objID in the default database
+ (id)gameWithURIURL:(NSURL*)objIDUrl;
// returns the game with objID in the specified database
+ (id)gameWithURIURL:(NSURL*)objIDUrl inDatabase:(OELibraryDatabase*)database;

// returns the game with objID in the default database
+ (id)gameWithURIString:(NSString*)objIDString;
// returns the game with objID in the specified database
+ (id)gameWithURIString:(NSString*)objIDString inDatabase:(OELibraryDatabase*)database;

// returns the game from the default database that represents the file at filePath
+ (id)gameWithFilePath:(NSString*)filePath createIfNecessary:(BOOL)createFlag error:(NSError**)outError;

// returns the game from the specified database that represents the file at filePath
+ (id)gameWithFilePath:(NSString*)filePath createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;

// returns the game with the archive id archiveID
+ (id)gameWithArchiveID:(id)archiveID error:(NSError**)outError;
+ (id)gameWithArchiveID:(id)archiveID inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;

#pragma mark -
#pragma mark Archive.VG Sync

- (void)setArchiveVGInfo:(NSDictionary*)gameInfoDictionary;
- (BOOL)performSyncWithArchiveVG:(NSError**)outError;
- (id)mergeInfoFromGame:(OEDBGame*)game;

#pragma mark -
#pragma mark Accessors

@property(readonly) OEDBRom *defaultROM;
@property(readonly) NSDate *lastPlayed;

#pragma mark -
@property(strong) OELibraryDatabase *database;

#pragma mark -

+ (id)gameWithArchiveDictionary:(NSDictionary*)gameInfo inDatabase:(OELibraryDatabase*)database;

#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img;
- (void)setBoxImageByURL:(NSURL*)url;

- (void)mergeWithGameInfo:(NSDictionary*)archiveGameDict;

- (void)updateInfoInBackground;

#pragma mark -
#pragma mark Core Data utilities

+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

@end
