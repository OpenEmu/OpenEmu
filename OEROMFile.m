//
//  OEROMFile.m
//  OpenEmu
//
//  Created by Steve Streza on 8/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OEROMFile.h"
#import "OECorePlugin.h"

#import "OEROMFileSystemTypeTransformer.h"

@implementation OEROMFile

@dynamic path;
@dynamic lastPlayedDate;

+(void)initialize{
	if(self == [OEROMFile class]){
		OEROMFileSystemTypeTransformer *transformer = [OEROMFileSystemTypeTransformer transformer];
		[NSValueTransformer setValueTransformer:transformer forName:@"OEROMFileSystemTypeTransformer"];

		OEROMFileSystemTypeTransformer *reverseTransformer = [OEROMFileSystemTypeReverseTransformer transformer];
		[NSValueTransformer setValueTransformer:reverseTransformer forName:@"OEROMFileSystemTypeReverseTransformer"];
}
}

+(NSString *)entityName{
	return @"ROMFile";
}

+(NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
	return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

+(OEROMFile *)fileWithPath:(NSString *)path
		 createIfNecessary:(BOOL)create
	inManagedObjectContext:(NSManagedObjectContext *)context{

	NSFetchRequest *request = [[NSFetchRequest alloc] init];
	[request setEntity:[self entityDescriptionInContext:context]];
	[request setPredicate:[NSPredicate predicateWithFormat:@"path == %@",path]];
	[request setFetchLimit:1];
	
	NSError *error = nil;
	NSArray *items = [context executeFetchRequest:request error:&error];
	
	OEROMFile *romFile = nil;
	if(items && items.count > 0){
		romFile = [items objectAtIndex:0];
	}else{
		romFile = [self createFileWithPath:path insertedIntoManagedObjectContext:context];
	}
	return romFile;
}

+(OEROMFile *)createFileWithPath:(NSString *)path insertedIntoManagedObjectContext:(NSManagedObjectContext *)context{
	OEROMFile *romFile = [[[self class] alloc] initWithEntity:[self entityDescriptionInContext:context]
							   insertIntoManagedObjectContext:context];
	
	[romFile setPath:path];
	[romFile setValue:[self nameForPath:path] forKey:@"name"];
	[romFile setValue:[NSNumber numberWithShort:[self systemTypeForFileAtPath:path]] forKey:@"systemType"];
	
	return romFile;
}

+(NSString *)nameForPath:(NSString *)path{
	return [[path lastPathComponent] stringByDeletingPathExtension];
}

+(OEROMFileSystemType)systemTypeForFileAtPath:(NSString *)path{
	NSString *extension = [[path pathExtension] lowercaseString];
	NSArray *corePlugins = [OECorePlugin allPlugins];
	for(OECorePlugin *plugin in corePlugins){
		if([[plugin supportedTypeExtensions] containsObject:extension]){
			NSArray *systemTypeNames = [[plugin supportedTypes] allKeys];
			for(NSString *systemTypeName in systemTypeNames){
				NSArray *extensions = [[plugin supportedTypes] objectForKey:systemTypeName];
				if([extensions containsObject:extension]){
					//assume name is in the form of "<<Console>> Game"
					NSString *consoleName = [systemTypeName stringByReplacingOccurrencesOfString:@" Game" withString:@""];
					
					NSNumber *number = [[OEROMFileSystemTypeTransformer transformer] reverseTransformedValue:consoleName];
					OEROMFileSystemType type = [number unsignedIntValue];
					
					return type;
				}
			}
		}
	}
	return OEROMFileSystemTypeUnknown;
}

-(NSURL *)pathURL{
	return [NSURL fileURLWithPath:[self path] isDirectory:NO];
}

-(void)setPath:(NSString *)path{
	[self setPrimitiveValue:[[path copy] autorelease] forKey:@"path"];
	
	AliasHandle handle;
	Boolean isDirectory;
	OSErr err = FSNewAliasFromPath( NULL, [path UTF8String], 0, &handle, &isDirectory );
	if ( err != noErr )
	{
//		[[self managedObjectContext] undo];
		return;
	}
	
	long aliasSize = GetAliasSize(handle);
	DLog(@"Alias is %i bytes",aliasSize);
	NSData *aliasData = [NSData dataWithBytes:*handle length:aliasSize];
	[self setValue:aliasData forKey:@"pathAlias"];
	
	DisposeHandle((Handle)handle);
}

-(id)init{
    if(self = [super init]){
        //STUB initialize your object here
    }
    return self;
}

-(void)dealloc{
    //STUB release your objects here
    
    [super dealloc];
}

@end
