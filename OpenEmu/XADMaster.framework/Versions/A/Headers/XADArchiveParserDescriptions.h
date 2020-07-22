/*
 * XADArchiveParserDescriptions.h
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
#import "XADArchiveParser.h"

@interface XADArchiveParser (Descriptions)

-(NSString *)descriptionOfValueInDictionary:(NSDictionary *)dict key:(NSString *)key;
-(NSString *)descriptionOfKey:(NSString *)key;
-(NSArray *)descriptiveOrderingOfKeysInDictionary:(NSDictionary *)dict;

@end

NSString *XADHumanReadableFileSize(uint64_t size);
NSString *XADShortHumanReadableFileSize(uint64_t size);
NSString *XADHumanReadableBoolean(uint64_t boolean);
NSString *XADHumanReadablePOSIXPermissions(uint64_t permissions);
NSString *XADHumanReadableAmigaProtectionBits(uint64_t protection);
NSString *XADHumanReadableDOSFileAttributes(uint64_t attributes);
NSString *XADHumanReadableWindowsFileAttributes(uint64_t attributes);
NSString *XADHumanReadableOSType(uint64_t ostype);
NSString *XADHumanReadableEntryWithDictionary(NSDictionary *dict,XADArchiveParser *parser);

NSString *XADHumanReadableObject(id object);
NSString *XADHumanReadableDate(NSDate *date);
NSString *XADHumanReadableData(NSData *data);
NSString *XADHumanReadableArray(NSArray *array);
NSString *XADHumanReadableDictionary(NSDictionary *dict);
NSString *XADHumanReadableList(NSArray *labels,NSArray *values);
NSString *XADIndentTextWithSpaces(NSString *text,int spaces);
