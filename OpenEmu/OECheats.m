/*
 Copyright (c) 2013, OpenEmu Team
 
 
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

#import "OECheats.h"

@interface OECheats ()
@property BOOL didFindMd5Hash;
@property NSMutableArray *cheatsFromMd5Hash;
@property NSString *searchHash;
@end

@implementation OECheats
@synthesize didFindMd5Hash, cheatsFromMd5Hash;

- (id)initWithMd5Hash:(NSString*)md5
{
    self = [super init];
    if(self != nil)
    {
        [self setSearchHash:md5];
        [self findCheats];
    }
    return self;
}

- (NSArray *)allCheats;
{
    return [cheatsFromMd5Hash copy];
}

-(void)findCheats
{
    /* XML cheats database format:
     
     <?xml version="1.0" encoding="UTF-8"?>
     <systems>
        <system id="openemu.system.nes">
            <game title="Some Game">
                <hashes>
                    <hash md5="9e107d9d372bb6826bd81d3542a419d6" />
                </hashes>
                <cheats>
                    <cheat code="048E:88" type="Action Replay" description="Invincibility" />
                    <cheat code="010F4ED8+01424FD8" type="GameShark" description="Infinite Money" />
                    <cheat code="69C4-AF6C+62C4-A7DC" type="Game Genie" description="Infinite time" />
                </cheats>
            </game>
        </system>
     ...
     </systems>
    
    */
    
    // TODO: decide proper place to read database from
    NSString *cheatsDatabaseFilename = @"cheats-database.xml";
    NSString *appSupportPath = [NSString pathWithComponents:@[
                                [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject],
                                @"OpenEmu", @"BIOS"]];
    NSString *cheatsDatabasePath = [appSupportPath stringByAppendingPathComponent:cheatsDatabaseFilename];
    
    NSData *xml = [NSData dataWithContentsOfFile:cheatsDatabasePath];
    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:xml];
    
    cheatsFromMd5Hash = [[NSMutableArray alloc] init];
    
    [parser setDelegate:(id)self];
    [parser parse];
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
    // Parse until we find our md5 hash
    if ([elementName isEqualToString:@"hash"] && [[attributeDict valueForKey:@"md5"] isEqualToString:[self searchHash]]) {
        
        [self setDidFindMd5Hash:YES];
        
    // Parse cheats where md5 hash was found
    } else if(didFindMd5Hash && [elementName isEqualToString:@"cheat"]) {
        NSMutableDictionary *cheatsDictionary = [[NSMutableDictionary alloc] init];
        [cheatsDictionary setObject:[attributeDict valueForKey:@"code"] forKey:@"code"];
        [cheatsDictionary setObject:[attributeDict valueForKey:@"type"] forKey:@"type"];
        [cheatsDictionary setObject:[attributeDict valueForKey:@"description"] forKey:@"description"];
        [cheatsDictionary setObject:[NSNumber numberWithBool:NO] forKey:@"enabled"];
        
        [cheatsFromMd5Hash addObject:cheatsDictionary];
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName
{
    // Stop parsing after all cheats have been found for md5
    if(didFindMd5Hash && [elementName isEqualToString:@"cheats"]) {
        [parser abortParsing];
    }
}

@end
