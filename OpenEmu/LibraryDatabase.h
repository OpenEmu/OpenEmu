//
//  LibraryDatabase.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 31.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface LibraryDatabase : NSObject {
@private
    NSArray* consoles;
	NSArray* collections;
	
	// Simple caching :)
	NSString* lastSystem;
	NSString* lastSearch;
	
	NSArray* lastResult;
	
	NSArrayController* romsController;
}
// not sure where to put this
- (void)loadConsoleImages;

#pragma mark -
#pragma mark For Testing...
- (void)setConsoles:(NSArray*)newConsoles;
- (void)addRoms:(NSArray*)newRoms;
- (void)setCollections:(NSArray*)newCollections;
#pragma mark -
#pragma mark Database queries
- (NSArray*)consoles;
- (NSArray*)collections;

- (NSArray*)romsForPredicate:(NSPredicate*)predicate;
- (NSArray*)romsInCollection:(id)collection;
#pragma mark -
#pragma mark Basic Database rom editing
/*
- (void)addRomsWithFiles:(NSArray*)files;
- (void)addRomWithFile:(NSString*)file;

- (void)deleteRomWithID:(id)romID;
- (void)deleteRomsWithIDs:(NSArray*)romIDs;

- (void)updateRom:(id)newValues;
- (void)updateRoms:(NSArray*)newValues;

#pragma mark -
#pragma mark Basic Collection Handling
- (void)addRom:(id)rom toCollection:(id)collection;
- (void)removeRom:(id)rom fromCollection:(id)collection;
- (void)moveRomAtIndex:(id)index toIndex:(id)newIndex;

#pragma mark -
#pragma mark Smart Collection Handling
- (void)addRule:(id)rule toCollection:(id)collection;
- (void)removeRule:(id)rule fromCollection:(id)collection;
*/

@property (retain) NSString* lastSystem;
@property (retain) NSString* lastSearch;

@property (retain) NSArray* lastResult;

@end
