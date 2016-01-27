/*
 Copyright (c) 2012, OpenEmu Team
 
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

// The extension declared in this file is meant to be used by the scripting
// infrastructure. There are architecturally better ways to achieve this from
// elsewhere. The interface is defined in the implementation file to avoid
// improper use.

@import Cocoa;

#include "OEApplicationDelegate.h"
#include "OEDBCollection.h"
#include "OEDBGame.h"
#include "OELibraryDatabase.h"

NS_ASSUME_NONNULL_BEGIN

@interface NSApplication (OEScripting)

@property(readonly) NSString *scripting_appVersion;
@property(readonly) NSString *scripting_buildVersion;
@property(readonly) NSArray <OEDBCollection *> *scripting_libraryCollections;

@property(readonly) NSArray <OEDBGame *> *scripting_games;
-(OEDBGame *)valueInScripting_gamesWithName:(NSString*)name;

@end

@implementation NSApplication (OEScripting)

-(NSString *)scripting_appVersion
{
	OEApplicationDelegate *appDelegate = (OEApplicationDelegate *)self.delegate;
	return appDelegate.appVersion;
}

-(NSString *)scripting_buildVersion
{
	OEApplicationDelegate *appDelegate = (OEApplicationDelegate *)self.delegate;
	return appDelegate.buildVersion;
}

-(NSArray <OEDBCollection *> *)scripting_libraryCollections
{
	// HACKHACK: OEDBAllGamesCollection isn't a OEDBCollection, and this causes trouble
	// with AppleScript. Scripting users should use the "games" collection instead.
	OELibraryDatabase *library = [OELibraryDatabase defaultDatabase];
	NSArray *list = [library collections];
	return [list filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^(id object, NSDictionary *_) {
		return [object isKindOfClass:[OEDBCollection class]];
	}]];
}

static NSArray *fetchGames(NSFetchRequest *request)
{
	OELibraryDatabase *library = [OELibraryDatabase defaultDatabase];
	NSManagedObjectContext *context = library.mainThreadContext;
	return [context executeFetchRequest:request error:nil];
}

-(NSArray <OEDBGame *> *)scripting_games
{
	NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
	return fetchGames(fetchRequest);
}

-(OEDBGame *)valueInScripting_gamesWithName:(NSString*)name
{
	NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
	fetchRequest.predicate = [NSPredicate predicateWithFormat:@"name like %@", name];
	NSArray* result = fetchGames(fetchRequest);
	return result.count > 0 ? result[0] : nil;
}

@end

NS_ASSUME_NONNULL_END
