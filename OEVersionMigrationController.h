//
//  OEVersionMigrationController.h
//  OpenEmu
//
//  Created by Steve Streza on 12/20/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SUVersionComparisonProtocol.h"

static NSString *OEVersionMigrationErrorDomain = @"OEVersionMigrationErrorDomain";
static NSString *OEVersionMigrationFailureErrorsKey = @"OEVersionMigrationFailureErrorsKey";

@interface OEVersionMigrationController : NSObject {
	NSMutableDictionary *migrators;
	id<SUVersionComparison> versionComparator;
}

@property (assign) id<SUVersionComparison> versionComparator;

+(id)defaultMigrationController;

-(void)runMigrationIfNeeded;
-(NSArray *)allMigrationVersions;

// the selector must return look like this:
// - (BOOL) doSomething: (NSError **) err;
-(void)addMigratorTarget:(id)target 
				selector:(SEL)selector
			  forVersion:(NSString *)version;

@end
