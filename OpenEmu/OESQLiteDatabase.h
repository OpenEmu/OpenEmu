//
//  OESQLiteDatabase.h
//  LookupDB
//
//  Created by Christoph Leimbrock on 07/12/13.
//  Copyright (c) 2013 Christoph Leimbrock. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface OESQLiteDatabase : NSObject
- (id)initWithURL:(NSURL*)url error:(NSError *__autoreleasing *)error;
- (id)executeQuery:(NSString*)sql error:(NSError *__autoreleasing *)error;
@end
