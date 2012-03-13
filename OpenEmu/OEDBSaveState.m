/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OEDBSaveState.h"
#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OECorePlugin.h"
@interface OEDBSaveState ()
+ (id)OE_newSaveStateInContext:(NSManagedObjectContext*)context;
@end

@implementation OEDBSaveState

+ (id)OE_newSaveStateInContext:(NSManagedObjectContext*)context{
	NSEntityDescription *description = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
	OEDBSaveState *result = [[OEDBSaveState alloc] initWithEntity:description insertIntoManagedObjectContext:context];
	
	[result setTimestamp:[NSDate date]];
	
	return result;
}

+ (id)createSaveStateNamed:(NSString*)name forRom:(OEDBRom*)rom core:(OECorePlugin*)core withFile:(NSURL*)stateFileURL screenshot:(NSURL*)screenshotFileURL
{
    return [self createSaveStateNamed:name forRom:rom core:core withFile:stateFileURL screenshot:screenshotFileURL inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)createSaveStateNamed:(NSString*)name forRom:(OEDBRom*)rom core:(OECorePlugin*)core withFile:(NSURL*)stateFileURL screenshot:(NSURL*)screenshotFileURL inDatabase:(OELibraryDatabase *)database
{
    OEDBSaveState *newSaveState = [self OE_newSaveStateInContext:[database managedObjectContext]];
    
    
    return newSaveState;
}



#pragma mark -
#pragma mark Data Model Properties
@dynamic emulatorID, path, screenshot, timestamp, userDescription;

#pragma mark -
#pragma mark Data Model Relationships
@dynamic rom;

@end
