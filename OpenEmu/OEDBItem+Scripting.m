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
// infrastructure. There are architecturally better

#import "OEDBCollection.h"
#import "OEDBGame.h"
#import "OELibraryDatabase.h"

@interface OEDBCollection (OEScripting)

@property(readonly) NSArray<OEDBGame *> *scripting_games;

-(NSScriptObjectSpecifier *)objectSpecifier;

@end

@implementation OEDBCollection (OEScripting)

static NSArray *fetchGames(NSFetchRequest *request)
{
	OELibraryDatabase *library = [OELibraryDatabase defaultDatabase];
	NSManagedObjectContext *context = library.mainThreadContext;
	return [context executeFetchRequest:request error:nil];
}

-(NSArray<OEDBGame *> *)scripting_games
{
	// The "games" property appears to be dead
	NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
	fetchRequest.predicate = self.fetchPredicate;
	fetchRequest.sortDescriptors = self.fetchSortDescriptors;
	fetchRequest.fetchLimit = self.fetchLimit;
	return fetchGames(fetchRequest);
}

-(NSScriptObjectSpecifier *)objectSpecifier
{
	NSScriptClassDescription *appDescription = [NSScriptClassDescription classDescriptionForClass:[NSApplication class]];
	return [[NSNameSpecifier alloc] initWithContainerClassDescription:appDescription containerSpecifier:nil key:@"scripting_libraryCollections" name:self.sidebarName];
}

@end

@interface OEDBGame (OEScripting)

-(NSScriptObjectSpecifier *)objectSpecifier;

@end

@implementation OEDBGame (OEScripting)

-(NSScriptObjectSpecifier *)objectSpecifier
{
	NSScriptClassDescription *appDescription = [NSScriptClassDescription classDescriptionForClass:[NSApplication class]];
	return [[NSNameSpecifier alloc] initWithContainerClassDescription:appDescription containerSpecifier:nil key:@"scripting_games" name:self.name];
}

@end
