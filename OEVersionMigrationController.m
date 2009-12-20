/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "OEVersionMigrationController.h"
#import "SUStandardVersionComparator.h"

#pragma mark Storage
#pragma mark -

@interface _OEMigrator : NSObject{
	id target;
	SEL action;

	BOOL hasRun;
}
@property (nonatomic, assign) id target;
@property (nonatomic, assign) SEL action;
@property (nonatomic, assign) BOOL hasRun;
-(BOOL)runWithError:(NSError **)err;
@end

@implementation _OEMigrator

@synthesize target, action, hasRun;

-(BOOL)runWithError:(NSError **)err{
	if (self.hasRun){
		return YES;
	}
	
	NSMethodSignature *methodSignature = [self.target methodSignatureForSelector:self.action];
	NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:methodSignature];

	[invocation setSelector:self.action];
	[invocation setArgument:&err atIndex:2];
	[invocation invokeWithTarget:self.target];
	
	BOOL returnValue = NO;
	[invocation getReturnValue:&returnValue];
	
	self.hasRun = YES;
	
	return returnValue;
}

@end

#pragma mark -
#pragma mark Private Interface
#pragma mark -

@interface OEVersionMigrationController (Private)

-(void)_runMigrationFromVersion:(NSString *)oldVersion toVersion:(NSString *)newVersion;

@end

NSComparisonResult OEVersionMigrationController_CompareVersions(NSString *oldVersion, NSString *newVersion, id<SUVersionComparison> comparator){
	return [comparator compareVersion:oldVersion toVersion:newVersion];
}

#pragma mark -
#pragma mark Implementation
#pragma mark -

@implementation OEVersionMigrationController

@synthesize versionComparator;

static OEVersionMigrationController *sDefaultMigrationController = nil;

+(id)defaultMigrationController{
	if(!sDefaultMigrationController){
		sDefaultMigrationController = [[OEVersionMigrationController alloc] init];
	}
	return sDefaultMigrationController;
}

-(id)init{
	self = [super init];
	if(self){
		self.versionComparator = [SUStandardVersionComparator defaultComparator];
		
		// we'll cheat here and rely on Sparkle's key
		isFirstRun = ! [[NSUserDefaults standardUserDefaults] boolForKey:@"SUHasLaunchedBefore"];
		lastVersion = [[[NSUserDefaults standardUserDefaults] objectForKey:@"OEMigrationLastVersion"] copy];
	}
	return self;
}

-(void)runMigrationIfNeeded{
	BOOL migrationNeeded = NO;
#define MigrationNeededIf(__cond) if(__cond){ migrationNeeded = YES; goto migrate; }
	
	NSString *currentVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey];
	NSString *mostRecentVersion = lastVersion;
	
	// We have to work around the fact that older versions of OpenEmu didn't stick a version key in the plist.
	// If the Sparkle key for an existing launch doesn't exist, then this is a new installation, not an upgrade.
	// Thus, we log our current version and prevent the migration. Subsequent migrations will then have the new version.
	if(isFirstRun){
		if(!mostRecentVersion){
			[[NSUserDefaults standardUserDefaults] setObject:currentVersion forKey:@"OEMigrationLastVersion"];
		}

		return;
	}
	
	// if it's not the first run, and there's no most recent version, then it's a pre-1.0.0b5 upgrade
	MigrationNeededIf(!mostRecentVersion);
	
	// if the current version has a higher version than the most recent version, then it's an upgrade
	MigrationNeededIf([self.versionComparator compareVersion:mostRecentVersion
											  toVersion:currentVersion] == NSOrderedAscending);
	
migrate:
	if(migrationNeeded){
		[self migrateFromVersion:mostRecentVersion
					   toVersion:currentVersion
						   error:nil];
	}
}

-(BOOL)migrateFromVersion:(NSString *)mostRecentVersion
				toVersion:(NSString *)currentVersion
					error:(NSError **)err{
	if(!mostRecentVersion) mostRecentVersion = @"0.0.1";
	
	BOOL hasFailed = NO;
	
	NSMutableArray *errors = (err == nil ? nil : [NSMutableArray array]);
	NSArray *allVersions = [self allMigrationVersions];

	for(NSString *migratorVersion in allVersions){
		BOOL runThisMigrator = YES;
		
		// @"1.0.0b4", @"1.0.0b5", @"1.0.0"
		if([self.versionComparator compareVersion:mostRecentVersion 
										toVersion:migratorVersion] == NSOrderedDescending){
			// don't need to migrate, migrator is too young and has already run
			runThisMigrator = NO;
		}else if([self.versionComparator compareVersion:currentVersion 
											  toVersion:migratorVersion] == NSOrderedAscending){
			// don't need to migrate, 
			runThisMigrator = NO;
		}
		
		if(runThisMigrator){
			NSArray *allMigrators = [migrators objectForKey:migratorVersion];
			NSLog(@"Running migrators from %@ to %@",mostRecentVersion, migratorVersion);
			for(_OEMigrator *migrator in allMigrators){
				NSError *error = nil;
				if(![migrator runWithError:&error]){
					[errors addObject:error];
					hasFailed = YES;
				}
			}
		}
	}
	
	[[NSUserDefaults standardUserDefaults] setObject:currentVersion forKey:@"OEMigrationLastVersion"];	
	
	if(!isRunning) isRunning = YES;
	
	if(errors.count > 0){
		hasFailed = YES;
		if(err && errors.count == 1){
			*err = [errors objectAtIndex:0];
		}else if(err && errors.count > 1){
			*err = [NSError errorWithDomain:OEVersionMigrationErrorDomain
									   code:1 
								   userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
											 NSLocalizedString(@"Some migrations failed to complete properly",@""),NSLocalizedDescriptionKey,
											 errors,OEVersionMigrationFailureErrorsKey,
											 nil]];
		}

		if(err){
			NSLog(@"Error migrating! %@,",*err);
		}
	}
	
	return hasFailed;
}

-(NSArray *)allMigrationVersions{
	NSArray *allVersions = [[migrators allKeys] sortedArrayUsingFunction:OEVersionMigrationController_CompareVersions 
																 context:self.versionComparator];

	if(!allVersions) allVersions = [NSArray array];
	return allVersions;
}

-(void)addMigratorTarget:(id)target 
				selector:(SEL)selector
			  forVersion:(NSString *)version{

	if(!migrators){
		migrators = [[NSMutableDictionary dictionary] retain];
	}
	
	NSArray *allVersions = [self allMigrationVersions];
	
	NSMutableArray *migratorsForVersion = nil;

	//todo: make this not O(n)
	BOOL breakOut = NO;
	for(NSString *migratorArrayKey in allVersions){ 
		switch([self.versionComparator compareVersion:version toVersion:migratorArrayKey]){
			case NSOrderedAscending:
				//we have passed the version, so no migrators exist.
				breakOut = YES;
				break;
			case NSOrderedSame:
				migratorsForVersion = [migrators objectForKey:migratorArrayKey];
				breakOut = YES;
				break;
		}

		if(breakOut) 
			break;
	}
	
	if(!migratorsForVersion){
		migratorsForVersion = [NSMutableArray array];
		[migrators setObject:migratorsForVersion forKey:version];
	}
	
	_OEMigrator *migratorContainer = [[_OEMigrator alloc] init];
	migratorContainer.target = target;
	migratorContainer.action = selector;
	migratorContainer.hasRun = NO;
	
	[migratorsForVersion addObject:migratorContainer];
	[migratorContainer release];
	
	if(isRunning){
		[self runMigrationIfNeeded];
	}
}

-(void)dealloc{
	[migrators release], migrators = nil;
	[lastVersion release], lastVersion = nil;
	
	[super dealloc];
}

@end
