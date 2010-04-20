/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OESaveState.h"
#import "NSString+Aliases.h"

@interface OESaveState ()
- (void)_OE_writeDataToPlist;
- (NSDictionary *)_OE_readDataFromPlist;
- (void)_OE_setupBundleContents;
@end


@implementation OESaveState

@dynamic romFile, emulatorID, timeStamp, pathAlias;

- (id)initInsertedIntoManagedObjectContext:(NSManagedObjectContext *)context
{
    NSEntityDescription *description = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
    return [self initWithEntity:description insertIntoManagedObjectContext:context];
}

- (void)setBundlePath:(NSString *)path
{
    [[NSFileManager defaultManager] createDirectoryAtPath:path
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    [self setPathAlias:[path OE_pathAliasData]];
    
    [self _OE_setupBundleContents];
}

- (void)_OE_setupBundleContents
{
    [[NSFileManager defaultManager] createDirectoryAtPath:[self resourcePath]
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
}

- (NSString *)bundlePath
{
    return [NSString OE_stringWithPathOfAliasData:[self valueForKey:@"pathAlias"]];
}

- (NSString *)resourcePath
{
    return [[self bundlePath] stringByAppendingPathComponent:@"Resources"];
}

- (NSString *)userDescription
{
    if(userDescription == nil)
        userDescription = [[[self _OE_readDataFromPlist] objectForKey:@"userDescription"] copy];
    
    return userDescription;
}

- (NSString *)emulatorID
{
    if(emulatorID == nil)
        emulatorID = [[[self _OE_readDataFromPlist] objectForKey:@"emulatorID"] copy];
    
    return emulatorID;
}

- (void)setEmulatorID:(NSString *)newEmulatorID
{
    if([emulatorID isEqual:newEmulatorID]) return;
    
    [emulatorID release];
    emulatorID = [newEmulatorID copy];

    [self _OE_writeDataToPlist];
 }

- (void)setUserDescription:(NSString *)newDescription
{
    if([userDescription isEqual:newDescription]) return;
    
    DLog(@"User description!");
    
    [userDescription release];
    userDescription = [newDescription copy];
    
    [self _OE_writeDataToPlist];
}

- (void)_OE_writeDataToPlist
{
    NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                [self emulatorID],      @"emulatorID",
                                [self userDescription], @"userDescription",
                                nil];
    
    //create the parent directories if needed
    [self _OE_setupBundleContents];
    
    [dictionary writeToFile:[self infoPlistPath] 
                 atomically:YES];
}

- (NSDictionary *)_OE_readDataFromPlist
{
    return [NSDictionary dictionaryWithContentsOfFile:[self infoPlistPath]];
}
     
- (NSString *)infoPlistPath
{
    return [[self bundlePath] stringByAppendingPathComponent:@"Info.plist"];
}

- (NSString *)screenshotPath
{
    return [[self resourcePath] stringByAppendingPathComponent:@"screenshot.tiff"];
}

- (NSString *)saveDataPath
{
    return [[self resourcePath] stringByAppendingPathComponent:@"save.data"];
}

- (NSBundle *)bundle
{
    return [NSBundle bundleWithPath:[self bundlePath]];
}

- (NSData *)saveData
{
    return [NSData dataWithContentsOfFile:[self saveDataPath]];
}

- (void)setSaveData:(NSData *)saveData
{
    [[NSFileManager defaultManager] createFileAtPath:[self saveDataPath]
                                            contents:saveData
                                          attributes:nil];
}

- (NSImage *)screenshot
{
    return [[[NSImage alloc] initWithContentsOfFile:[self screenshotPath]] autorelease];
}

- (void)setScreenshot:(NSImage *)newScreenshot
{
    [[NSFileManager defaultManager] createFileAtPath:[self screenshotPath]
                                            contents:[newScreenshot TIFFRepresentation]
                                          attributes:nil];
}

- (NSDate *)timeStamp
{
    return [[[NSFileManager defaultManager] attributesOfItemAtPath:[self bundlePath] 
                                                             error:nil] objectForKey:NSFileModificationDate];
}

- (id)imageRepresentation
{
    return [self screenshot];
}

- (NSString *)imageRepresentationType
{
    // We use this representation type because we are storing the image as binary data.
    return IKImageBrowserNSImageRepresentationType;
}

- (NSString *)imageUID
{
    // This is uses the NSManagedObjectID for the entity to generate a unique string.
    return [[[self objectID] URIRepresentation] description];
}

- (NSString *)imageTitle
{
    //return [self.timeStamp descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
    return [[self romFile] name];
}

- (NSString *)imageSubtitle
{
    return [[self timeStamp] descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
}

@end
