//
//  OEFileManager.h
//  OEFileManager
//
//  Created by Christoph Leimbrock on 28/02/14.
//  Copyright (c) 2014 ccl. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef BOOL (^OEFileManagerProgressCallback)(float progress);
typedef BOOL (^OEFileManagerPerItemCallback)(NSURL *src, NSURL *dst, NSError *result);

@interface OEFileManager : NSFileManager
- (NSUInteger)sizeOfItemAtURL:(NSURL*)url;
- (BOOL)copyItemAtURL:(NSURL*)source toURL:(NSURL *)destination error:(NSError *__autoreleasing *)error;

@property (copy) OEFileManagerProgressCallback progressHandler;
@property (copy) OEFileManagerPerItemCallback  itemDoneHandler;
@property (copy) OEFileManagerPerItemCallback  errorHandler;
@end
