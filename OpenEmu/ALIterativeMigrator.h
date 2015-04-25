/*
 Copyright (c) 2013, Art & Logic
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
*/

#import <Foundation/Foundation.h>

/**
 * Iteratively migrates a persistent Core Data store along a series of ordered
 * managed object models.  Allows for easily mixing inferred migrations
 * with mapping models and custom migrations.
 */
@interface ALIterativeMigrator : NSObject

/**
 * Iteratively migrates the store at the given URL from its current model
 * to the given finalModel in order through the list of modelNames.
 *
 * Does nothing (returns YES) if the persistent store does not yet exist
 * at the given sourceStoreURL.
 *
 * @param sourceStoreURL The file URL to the persistent store file.
 * @param sourceStoreType The type of store at sourceStoreURL
 *        (see NSPersistentStoreCoordinator for possible values).
 * @param finalModel The final target managed object model for the migration manager.
 *        At the end of the migration, the persistent store should be migrated
 *        to this model.
 * @param modelNames *.mom file names for each of the managed object models
 *        through which the persistent store might need to be migrated.
 *        The model names should be ordered in such a way that migration
 *        from one to the next can occur either using a custom mapping model
 *        or an inferred mapping model.
 *        The *.mom files should be stored in the top level of the main bundle.
 * @param error If an error occurs during the migration, upon return contains
 *        an NSError object that describes the problem.
 *
 * @return YES if the migration proceeds without errors, otherwise NO.
 */
+ (BOOL)iterativeMigrateURL:(NSURL*)sourceStoreURL
                     ofType:(NSString*)sourceStoreType
                    toModel:(NSManagedObjectModel*)finalModel
          orderedModelNames:(NSArray*)modelNames
                      error:(NSError**)error;

/**
 * Migrates the store at the given URL from one object model to another
 * using the given mapping model.  Writes the store to a temporary file
 * during migration so that if migration fails, the original store is left intact.
 *
 * @param sourceStoreURL The file URL to the persistent store file.
 * @param sourceStoreType The type of store at sourceStoreURL
 *        (see NSPersistentStoreCoordinator for possible values).
 * @param sourceModel The source managed object model for the migration manager.
 * @param targetModel The target managed object model for the migration manager.
 * @param mappingModel The mapping model to use to effect the migration.
 * @param error If an error occurs during the migration, upon return contains 
 *        an NSError object that describes the problem.
 *
 * @return YES if the migration proceeds without errors, otherwise NO.
 */
+ (BOOL)migrateURL:(NSURL*)sourceStoreURL
            ofType:(NSString*)sourceStoreType
         fromModel:(NSManagedObjectModel*)sourceModel
           toModel:(NSManagedObjectModel*)targetModel
      mappingModel:(NSMappingModel*)mappingModel
             error:(NSError**)error;

/**
 * Returns the error domain used in NSErrors created by this class.
 */
+ (NSString*)errorDomain;

@end
