/*
 Copyright (c) 2009, OpenEmu Team
 
 
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
#import <CoreData/CoreData.h>
#import "OECorePlugin.h"

@interface OEROMFile : NSManagedObject

// Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

// find-or-create methods for ROM files
+ (OEROMFile *)fileWithPath:(NSString *)path 
     inManagedObjectContext:(NSManagedObjectContext *)context;
+ (OEROMFile *)fileWithPath:(NSString *)path
          createIfNecessary:(BOOL)create
     inManagedObjectContext:(NSManagedObjectContext *)context;

+ (OEROMFile *)createFileWithPath:(NSString *)path 
insertedIntoManagedObjectContext:(NSManagedObjectContext *)context;

// Loading a ROM from a file
+ (NSString *)nameForPath:(NSString *)path;

@property(nonatomic, retain) NSData   *pathAlias;
@property(nonatomic, copy)   NSString *path;
@property(nonatomic, copy)   NSDate   *lastPlayedDate;
@property(nonatomic, retain) NSSet    *saveStates;
@property(nonatomic, retain) NSString *name;
@property(nonatomic, retain) NSString *preferredEmulator;

- (NSString *)systemName;
- (NSURL *)pathURL;

// These methods attempt to find a plugin which will work with the OEROMFile.
typedef OECorePlugin* (^handleChoicesBlock)(NSArray *, BOOL *);
- (OECorePlugin *)suitablePlugin;
- (OECorePlugin *)suitablePluginAndHandleMultiple:(handleChoicesBlock)choiceMethod;

@end
