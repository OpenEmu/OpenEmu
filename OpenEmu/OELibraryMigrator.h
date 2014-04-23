//
//  OELibraryMigrator.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/04/14.
//
//

#import <Foundation/Foundation.h>

@interface OELibraryMigrator : NSObject
- (id)initWithStoreURL:(NSURL*)url;
- (BOOL)runMigration:(NSError*__autoreleasing*)outError;
@end
