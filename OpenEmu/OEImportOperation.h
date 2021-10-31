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

NS_ASSUME_NONNULL_BEGIN

@import Foundation;

@class OEROMImporter;

#pragma mark - User Default Keys
extern NSString *const OECopyToLibraryKey;
extern NSString *const OEAutomaticallyGetInfoKey;

#pragma mark - Error Codes
extern NSString *const OEImportErrorDomainFatal;
extern NSString *const OEImportErrorDomainResolvable;
extern NSString *const OEImportErrorDomainSuccess;

typedef NS_ENUM(NSInteger, OEImportErrorCode) {
    OEImportErrorCodeAlreadyInDatabase     = -1,
    OEImportErrorCodeMultipleSystems       = 2,
    OEImportErrorCodeNoSystem              = 3,
    OEImportErrorCodeInvalidFile           = 4,
    OEImportErrorCodeAdditionalFiles       = 5,
    OEImportErrorCodeNoHash                = 6,
    OEImportErrorCodeNoGame                = 7,
    OEImportErrorCodeDisallowArchivedFile  = 8,
    OEImportErrorCodeEmptyFile             = 9,
    OEImportErrorCodeAlreadyInDatabaseFileUnreachable = 10,
};

typedef NS_ENUM(NSInteger, OEImportExitStatus)  {
    OEImportExitNone,
    OEImportExitErrorResolvable,
    OEImportExitErrorFatal,
    OEImportExitSuccess,
};

extern NSString * const OEImportManualSystems;

typedef void (^OEImportItemCompletionBlock)(NSManagedObjectID * _Nullable);

@interface OEImportOperation: NSOperation <NSObject, NSSecureCoding, NSCopying>

+ (nullable instancetype)operationWithURL:(NSURL*)operationURL inImporter:(OEROMImporter *)importer;

@property BOOL exploreArchives;
@property BOOL isDisallowedArchiveWithMultipleFiles;

@property (copy) NSURL *URL;
@property (copy) NSURL *sourceURL;
@property (nullable, copy) NSManagedObjectID *collectionID;
@property (nonatomic, readonly) NSManagedObjectID *romObjectID;
@property (nullable) NSURL *extractedFileURL;

@property NSArray <NSString *> *systemIdentifiers;
@property (nullable) NSString *romLocation;

@property (copy, nullable) NSError *error;
@property OEImportExitStatus exitStatus;

@property (copy, nullable) OEImportItemCompletionBlock completionHandler;
@property OEROMImporter *importer;

@property (getter=isChecked) BOOL checked;

@end

NS_ASSUME_NONNULL_END
