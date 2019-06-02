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

#import "ALIterativeMigrator.h"

@implementation ALIterativeMigrator

+ (BOOL)iterativeMigrateURL:(NSURL*)sourceStoreURL
                     ofType:(NSString*)sourceStoreType
                    toModel:(NSManagedObjectModel*)finalModel
          orderedModelNames:(NSArray*)modelNames
                      error:(NSError**)error
{
   // If the persistent store does not exist at the given URL,
   // assume that it hasn't yet been created and return success immediately.
   if (NO == [[NSFileManager defaultManager] fileExistsAtPath:[sourceStoreURL path]])
   {
      return YES;
   }

   // Get the persistent store's metadata.  The metadata is used to
   // get information about the store's managed object model.
   NSDictionary* sourceMetadata =
    [self metadataForPersistentStoreOfType:sourceStoreType
                                       URL:sourceStoreURL
                                     error:error];
   if (nil == sourceMetadata)
   {
      return NO;
   }

   // Check whether the final model is already compatible with the store.
   // If it is, no migration is necessary.
   if ([finalModel isConfiguration:nil compatibleWithStoreMetadata:sourceMetadata])
   {
      return YES;
   }

   // Find the current model used by the store.
   NSManagedObjectModel* sourceModel =
    [self modelForStoreMetadata:sourceMetadata error:error];
   if (nil == sourceModel)
   {
      return NO;
   }

   // Get NSManagedObjectModels for each of the model names given.
   NSArray* models = [self modelsNamed:modelNames error:error];
   if (nil == models)
   {
      return NO;
   }

   // Build an inclusive list of models between the source and final models.
   NSMutableArray* relevantModels = [NSMutableArray array];
   BOOL firstFound = NO;
   BOOL lastFound = NO;
   BOOL reverse = NO;
   for (NSManagedObjectModel* model in models)
   {
      if ([model isEqual:sourceModel] || [model isEqual:finalModel])
      {
         if (firstFound)
         {
            lastFound = YES;
            // In case a reverse migration is being performed (descending through the
            // ordered array of models), check whether the source model is found
            // after the final model.
            reverse = [model isEqual:sourceModel];
         }
         else
         {
            firstFound = YES;
         }
      }

      if (firstFound)
      {
         [relevantModels addObject:model];
      }

      if (lastFound)
      {
         break;
      }
   }

   // Ensure that the source model is at the start of the list.
   if (reverse)
   {
      relevantModels =
       [[[relevantModels reverseObjectEnumerator] allObjects] mutableCopy];
   }

   // Migrate through the list
   for (int i = 0; i < ([relevantModels count] - 1); i++)
   {
      NSManagedObjectModel* modelA = [relevantModels objectAtIndex:i];
      NSManagedObjectModel* modelB = [relevantModels objectAtIndex:(i + 1)];

      // Check whether a custom mapping model exists.
      NSMappingModel* mappingModel = [NSMappingModel mappingModelFromBundles:nil
                                                              forSourceModel:modelA
                                                            destinationModel:modelB];

      // If there is no custom mapping model, try to infer one.
      if (nil == mappingModel)
      {
         mappingModel = [NSMappingModel inferredMappingModelForSourceModel:modelA
                                                          destinationModel:modelB
                                                                     error:error];
         if (nil == mappingModel)
         {
            return NO;
         }
      }

      if (![self migrateURL:sourceStoreURL
                     ofType:sourceStoreType
                  fromModel:modelA
                    toModel:modelB
               mappingModel:mappingModel
                      error:error])
      {
         return NO;
      }
   }

   return YES;
}

+ (BOOL)migrateURL:(NSURL*)sourceStoreURL
            ofType:(NSString*)sourceStoreType
         fromModel:(NSManagedObjectModel*)sourceModel
           toModel:(NSManagedObjectModel*)targetModel
      mappingModel:(NSMappingModel*)mappingModel
             error:(NSError**)error
{
   // Build a temporary path to write the migrated store.
   NSURL* tempDestinationStoreURL =
    [NSURL fileURLWithPath:[[sourceStoreURL path] stringByAppendingPathExtension:@"tmp"]];

   // Migrate from the source model to the target model using the mapping,
   // and store the resulting data at the temporary path.
   NSMigrationManager* migrator = [[NSMigrationManager alloc]
                                   initWithSourceModel:sourceModel
                                   destinationModel:targetModel];

   if (![migrator migrateStoreFromURL:sourceStoreURL
                                 type:sourceStoreType
                              options:nil
                     withMappingModel:mappingModel
                     toDestinationURL:tempDestinationStoreURL
                      destinationType:sourceStoreType
                   destinationOptions:nil
                                error:error])
   {
      return NO;
   }

   // Move the original source store to a backup location.
   NSString* backupPath = [[sourceStoreURL path] stringByAppendingPathExtension:@"bak"];
   NSFileManager* fileManager = [NSFileManager defaultManager];
   if (![fileManager moveItemAtPath:[sourceStoreURL path]
                             toPath:backupPath
                              error:error])
   {
      // If the move fails, delete the migrated destination store.
      [fileManager moveItemAtPath:[tempDestinationStoreURL path]
                           toPath:[sourceStoreURL path]
                            error:NULL];
      return NO;
   }

   // Move the destination store to the original source location.
   if ([fileManager moveItemAtPath:[tempDestinationStoreURL path]
                            toPath:[sourceStoreURL path]
                             error:error])
   {
      // If the move succeeds, delete the backup of the original store.
      [fileManager removeItemAtPath:backupPath error:NULL];
   }
   else
   {
      // If the move fails, restore the original store to its original location.
      [fileManager moveItemAtPath:backupPath
                           toPath:[sourceStoreURL path]
                            error:NULL];
      return NO;
   }

   return YES;
}

+ (NSString*)errorDomain
{
   return @"com.artlogic.IterativeMigrator";
}

#pragma mark - Private methods

// Returns an NSError with the give code and localized description,
// and this class' error domain.
+ (NSError*)errorWithCode:(NSInteger)code description:(NSString*)description
{
   NSDictionary* userInfo = @{
      NSLocalizedDescriptionKey: description
   };
   
   return [NSError errorWithDomain:[ALIterativeMigrator errorDomain]
                              code:code
                          userInfo:userInfo];
   
   return nil;
}

// Gets the metadata for the given persistent store.
+ (NSDictionary*)metadataForPersistentStoreOfType:(NSString*)storeType
                                              URL:(NSURL*)url
                                            error:(NSError **)error
{
   NSDictionary* sourceMetadata = [NSPersistentStoreCoordinator metadataForPersistentStoreOfType:storeType URL:url options:nil error:error];
   if (nil == sourceMetadata && NULL != error)
   {
      NSString* errorDesc = [NSString stringWithFormat:
                             @"Failed to find source metadata for store: %@",
                             url];
      *error = [self errorWithCode:102 description:errorDesc];
   }

   return sourceMetadata;
}

// Finds the source model for the store described by the given metadata.
+ (NSManagedObjectModel*)modelForStoreMetadata:(NSDictionary*)metadata
                                         error:(NSError**)error
{
   NSManagedObjectModel* sourceModel = [NSManagedObjectModel
                                        mergedModelFromBundles:nil
                                        forStoreMetadata:metadata];
   if (nil == sourceModel && NULL != error)
   {
      NSString* errorDesc = [NSString stringWithFormat:
                             @"Failed to find source model for metadata: %@",
                             metadata];
      *error = [self errorWithCode:100 description:errorDesc];
   }

   return sourceModel;
}

// Returns an array of NSManagedObjectModels loaded from mom files with the given names.
// Returns nil if any model files could not be found.
+ (NSArray*)modelsNamed:(NSArray*)modelNames
                  error:(NSError**)error
{
   NSMutableArray* models = [NSMutableArray array];
   for (NSString* modelName in modelNames)
   {
      NSURL* modelUrl = [self urlForModelName:modelName inDirectory:nil];
      NSManagedObjectModel* model =
       [[NSManagedObjectModel alloc] initWithContentsOfURL:modelUrl];
      
      if (nil == model)
      {
         if (NULL != error)
         {
            NSString* errorDesc =
            [NSString stringWithFormat:@"No model found for %@ at URL %@", modelName, modelUrl];
            *error = [self errorWithCode:110 description:errorDesc];
         }
         return nil;
      }

      [models addObject:model];
   }
   return models;
}

// Returns an array of paths to .mom model files in the given directory.
// Recurses into .momd directories to look for .mom files.
// @param directory The name of the bundle directory to search.  If nil,
//    searches default paths.
+ (NSArray*)modelPathsInDirectory:(NSString*)directory
{
   NSMutableArray* modelPaths = [NSMutableArray array];
   
   // Get top level mom file paths.
   [modelPaths addObjectsFromArray:
    [[NSBundle mainBundle] pathsForResourcesOfType:@"mom"
                                       inDirectory:directory]];
   
   // Get mom file paths from momd directories.
   NSArray* momdPaths = [[NSBundle mainBundle] pathsForResourcesOfType:@"momd"
                                                           inDirectory:directory];
   for (NSString* momdPath in momdPaths)
   {
      NSString* resourceSubpath = [momdPath lastPathComponent];

      [modelPaths addObjectsFromArray:
       [[NSBundle mainBundle]
        pathsForResourcesOfType:@"mom"
        inDirectory:resourceSubpath]];
   }
   
   return modelPaths;
}

// Returns the URL for a model file with the given name in the given directory.
// @param directory The name of the bundle directory to search.  If nil,
//    searches default paths.
+ (NSURL*)urlForModelName:(NSString*)modelName
              inDirectory:(NSString*)directory
{
   NSBundle* bundle = [NSBundle mainBundle];
   NSURL* url = [bundle URLForResource:modelName
                         withExtension:@"cdm"
                          subdirectory:directory];
    
   if (nil == url)
   {
      // Get mom file paths from momd directories.
      NSArray* momdPaths = [[NSBundle mainBundle] pathsForResourcesOfType:@"momd"
                                                              inDirectory:directory];
      for (NSString* momdPath in momdPaths)
      {
         url = [bundle URLForResource:modelName
                        withExtension:@"mom"
                         subdirectory:[momdPath lastPathComponent]];
      }
   }

   return url;
}

@end
