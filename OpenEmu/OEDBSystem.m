//
//  OEDBConsole.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OELibraryDatabase.h"
@implementation OEDBSystem
+ (id)createSystemFromPlugin:(OESystemPlugin*)plugin inDatabase:(OELibraryDatabase*)database{
	NSString* systemIdentifier = [plugin systemIdentifier];
	OEDBSystem* system = [database systemWithIdentifier:systemIdentifier];
	
	if(system) return system;
	
	NSManagedObjectContext* moc = [database managedObjectContext];
	
	system = [[OEDBSystem alloc] initWithEntity:[self entityDescriptionInContext:moc] insertIntoManagedObjectContext:moc];
	// TODO: get archive id(s) from plugin
	[system setValue:systemIdentifier forKey:@"systemIdentifier"];
	
	NSLog(@"new system: %@", system);
	return [system autorelease];		
}
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName{
    return @"System";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}
#pragma mark -
- (OESystemPlugin*)plugin{
	NSString* systemIdentifier = [self valueForKey:@"systemIdentifier"];
	OESystemPlugin* plugin = [OESystemPlugin gameSystemPluginForIdentifier:systemIdentifier];
	
	return plugin;
}
#pragma mark -
- (NSImage*)icon{
    NSString* locName = NSLocalizedString([self valueForKey:@"name"], @"");
    NSImage* image = [NSImage imageNamed:locName];
        
    return image;
}

- (NSString*)name{
	return [[self plugin] systemName];
}

@end
