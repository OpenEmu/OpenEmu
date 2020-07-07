/*
 * XADPlatform.h
 *
 * Copyright (c) 2017-present, MacPaw Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */
#import "XADUnarchiver.h"
#import "CSHandle.h"

@interface XADPlatform:NSObject {}

// Archive entry extraction.
+(XADError)extractResourceForkEntryWithDictionary:(NSDictionary *)dict
unarchiver:(XADUnarchiver *)unarchiver toPath:(NSString *)destpath;
+(XADError)updateFileAttributesAtPath:(NSString *)path
forEntryWithDictionary:(NSDictionary *)dict parser:(XADArchiveParser *)parser
preservePermissions:(BOOL)preservepermissions;
+(XADError)createLinkAtPath:(NSString *)path withDestinationPath:(NSString *)link;

// Archive post-processing.
+(id)readCloneableMetadataFromPath:(NSString *)path;
+(void)writeCloneableMetadata:(id)metadata toPath:(NSString *)path;
+(BOOL)copyDateFromPath:(NSString *)src toPath:(NSString *)dest;
+(BOOL)resetDateAtPath:(NSString *)path;

// Path functions.
+(BOOL)fileExistsAtPath:(NSString *)path;
+(BOOL)fileExistsAtPath:(NSString *)path isDirectory:(BOOL *)isdirptr;
+(NSString *)uniqueDirectoryPathWithParentDirectory:(NSString *)parent;
+(NSString *)sanitizedPathComponent:(NSString *)component;
+(NSArray *)contentsOfDirectoryAtPath:(NSString *)path;
+(BOOL)moveItemAtPath:(NSString *)src toPath:(NSString *)dest;
+(BOOL)removeItemAtPath:(NSString *)path;

// Resource forks
+(CSHandle *)handleForReadingResourceForkAtPath:(NSString *)path;

// Time functions.
+(double)currentTimeInSeconds;

@end
