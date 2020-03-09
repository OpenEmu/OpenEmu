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

@import CoreData;
#import "OEDBItem.h"

@class OEDBRom, OECorePlugin, OELibraryDatabase;

NS_ASSUME_NONNULL_BEGIN

extern NSString *const OESaveStateSuffix;

extern NSString *const OESaveStateInfoVersionKey;
extern NSString *const OESaveStateInfoNameKey;
extern NSString *const OESaveStateInfoDescriptionKey;
extern NSString *const OESaveStateInfoROMMD5Key;
extern NSString *const OESaveStateInfoCoreIdentifierKey;
extern NSString *const OESaveStateInfoCoreVersionKey;

extern NSString *const OESaveStateSpecialNamePrefix;
extern NSString *const OESaveStateAutosaveName;
extern NSString *const OESaveStateQuicksaveName;

extern NSString *const OESaveStateUseQuickSaveSlotsKey;

@interface OEDBSaveState : OEDBItem <NSPasteboardWriting>

+ (nullable instancetype)updateOrCreateStateWithURL:(NSURL *)stateURL inContext:(NSManagedObjectContext *)context;

+ (nullable instancetype)createSaveStateNamed:(NSString *)name forRom:(OEDBRom *)rom core:(OECorePlugin *)core withFile:(NSURL *)stateFileURL inContext:(NSManagedObjectContext *)context;
+ (nullable instancetype)createSaveStateByImportingBundleURL:(NSURL *)bundleURL intoContext:(NSManagedObjectContext *)context;
+ (nullable instancetype)createSaveStateByImportingBundleURL:(NSURL *)bundleURL intoContext:(NSManagedObjectContext *)context copy:(BOOL)flag;

+ (NSString *)nameOfQuickSaveInSlot:(NSInteger)slot;

#pragma mark - Management

- (BOOL)replaceStateFileWithFile:(NSURL *)stateFile;
- (BOOL)moveToDefaultLocation;

- (BOOL)readFromDisk;
- (BOOL)writeToDisk;

@property(readonly, getter=isValid) BOOL valid;

- (void)deleteAndRemoveFiles;
// returns YES if it was invalid
- (BOOL)deleteAndRemoveFilesIfInvalid;

#pragma mark - Data Accessors

@property(readonly, nullable) NSString *displayName; // Should be used instead of -name if the string is to be displayed to the user
@property(readonly) BOOL isSpecialState;

#pragma mark - Data Model Properties

@property (nonatomic, retain, readonly, nullable) NSString *systemIdentifier;
@property (nonatomic, retain, nullable)           NSString *location;
@property (nonatomic, retain)           NSURL    *URL;
@property (nonatomic, retain, readonly) NSURL    *screenshotURL;
@property (nonatomic, retain, readonly) NSURL    *dataFileURL;

@end

NS_ASSUME_NONNULL_END
