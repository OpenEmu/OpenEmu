//
//  OEVersionMigrationController.m
//  OpenEmu
//
//  Created by Steve Streza on 12/20/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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
	}
	return self;
}

-(void)runMigrationIfNeeded{
	BOOL migrationNeeded = NO;
	
#define MigrationNeededIf(__cond) if(__cond){ migrationNeeded = YES; goto migrate; }
	
	NSString *currentVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey];
	NSString *mostRecentVersion = [[NSUserDefaults standardUserDefaults] objectForKey:@"OEMigrationLastVersion"];
	
	MigrationNeededIf(!mostRecentVersion);
	
	MigrationNeededIf([self.versionComparator compareVersion:mostRecentVersion
											  toVersion:currentVersion] == NSOrderedAscending);
	
migrate:
	if(migrationNeeded){
		if(!mostRecentVersion) mostRecentVersion = @"0.0.1";
		
		NSMutableArray *errors = [NSMutableArray array];
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
					}
				}
			}
		}
		
		if(errors.count > 0){
			NSError *migrationError = [NSError errorWithDomain:OEVersionMigrationErrorDomain
														  code:1 
													  userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
																NSLocalizedString(@"Some migrations failed to complete properly",@""),NSLocalizedDescriptionKey,
																errors,OEVersionMigrationFailureErrorsKey,
																nil]];
			NSLog(@"%i failed migrations: %@",errors.count,migrationError);
		}
		
		[[NSUserDefaults standardUserDefaults] setObject:currentVersion forKey:@"OEMigrationLastVersion"];
	}
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
}

-(void)dealloc{
	[migrators release], migrators = nil;
	
	[super dealloc];
}

@end
