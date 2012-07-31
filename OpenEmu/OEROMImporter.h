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

enum _OEImportErrorBehavior {
    OEImportErrorAskUser,
    OEImportErrorCancelKeepChanges,
    OEImportErrorCancelDeleteChanges,
    OEImportErrorIgnore
};
typedef enum _OEImportErrorBehavior OEImportErrorBehavior;

@class OELibraryDatabase;
@protocol OEROMImporterDelegate;
@interface OEROMImporter : NSObject
- (id)initWithDatabase:(OELibraryDatabase *)aDatabase;

- (BOOL)importROMsAtPath:(NSString*)path inBackground:(BOOL)bg error:(NSError**)outError;
- (BOOL)importROMsAtPaths:(NSArray*)pathArray inBackground:(BOOL)bg error:(NSError**)outError;

- (BOOL)importROMsAtURL:(NSURL*)url inBackground:(BOOL)bg error:(NSError**)outError;
- (BOOL)importROMsAtURLs:(NSArray*)urlArray inBackground:(BOOL)bg error:(NSError**)outError;

@property OEImportErrorBehavior errorBehaviour;
@property (weak, readonly) OELibraryDatabase *database;
@property (readonly) NSArray *importedRoms;
@property (readonly) BOOL isBusy;

@property (strong) id <OEROMImporterDelegate> delegate;
#pragma mark -
#pragma Handle Spotlight importing
- (void)discoverRoms:(NSArray*)volumes;
- (void)updateSearchResults:(NSNotification*)notification;
- (void)finalizeSearchResults:(NSNotification*)notification;
- (void)importInBackground;
@end

@interface OEROMImporter (Control)
- (void)pause;
- (void)start;
- (void)cancel;
- (void)removeFinished;

- (NSUInteger)items;
- (NSUInteger)finishedItems;
@end
@protocol OEROMImporterDelegate
- (void)romImporter:(OEROMImporter*)importer startedProcessingItem:(id)item;
- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(id)item;
- (void)romImporter:(OEROMImporter*)importer finishedProcessingItem:(id)item;
@end
