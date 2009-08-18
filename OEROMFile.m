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

#import "OEROMFile.h"
#import "OECorePlugin.h"

@implementation OEROMFile

@dynamic path;
@dynamic lastPlayedDate;

+(NSString *)entityName{
    return @"ROMFile";
}

+(NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

+(OEROMFile *)fileWithPath:(NSString *)path
         createIfNecessary:(BOOL)create
    inManagedObjectContext:(NSManagedObjectContext *)context{

    NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
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

+ (OEROMFile *)createFileWithPath:(NSString *)path insertedIntoManagedObjectContext:(NSManagedObjectContext *)context{
    OEROMFile *romFile = [[[self alloc] initWithEntity:[self entityDescriptionInContext:context]
                               insertIntoManagedObjectContext:context] autorelease];
    
    [romFile setPath:path];
    [romFile setValue:[self nameForPath:path] forKey:@"name"];
    
    return romFile;
}

+(NSString *)nameForPath:(NSString *)path{
    return [[path lastPathComponent] stringByDeletingPathExtension];
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
//        [[self managedObjectContext] undo];
        return;
    }
    
    long aliasSize = GetAliasSize(handle);
    DLog(@"Alias is %i bytes",aliasSize);
    NSData *aliasData = [NSData dataWithBytes:*handle length:aliasSize];
    [self setValue:aliasData forKey:@"pathAlias"];
    
    DisposeHandle((Handle)handle);
}

-(NSString *)systemName{
	NSString *path = [self path];
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
					return consoleName;
                }
            }
        }
    }
    return NSLocalizedString(@"Unknown",@"");	
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
