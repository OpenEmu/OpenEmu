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

#import "SaveState.h"
#import "NSString+Aliases.h"

@implementation SaveState 

@dynamic timeStamp;
@dynamic emulatorID;
@dynamic screenShot;
@dynamic saveData;
@dynamic pathalias;

@dynamic rompath;

- (NSString *)rompath
{
    NSData *aliasData = [self valueForKey:@"pathalias"];
    return [[NSURL URLByResolvingBookmarkData:aliasData options:0 relativeToURL:nil bookmarkDataIsStale:NULL error:NULL] path]; // [NSString OE_stringWithPathOfAliasData:aliasData];
}

- (id) imageRepresentation
{
    NSImage* image = [[NSImage alloc] initWithData:[self.screenShot valueForKey:@"screenShot"]];
    return [image autorelease];
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
    return [self.rompath lastPathComponent];
}

- (NSString *)imageSubtitle
{
    return [self.timeStamp descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
}

@end
