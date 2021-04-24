// Copyright (c) 2021, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "OEPErsistentCheats.h"
#import "OEDBRom.h"

@implementation OEPErsistentCheats
+(BOOL)persistROMCheats:(NSURL *) romPath
          withCheatList:(NSMutableArray *) cheatsList
{
    NSURL *plistCheatsForGameURL = [romPath URLByDeletingPathExtension];
    plistCheatsForGameURL = [plistCheatsForGameURL URLByAppendingPathExtension:@"cheatsPlist"];
    NSLog(@"--  Current Cheats URL => %@", plistCheatsForGameURL);
    NSError *error;
    NSData *plistData = [NSPropertyListSerialization dataWithPropertyList:cheatsList
                                                                   format:NSPropertyListXMLFormat_v1_0
                                                                  options:0
                                                                    error:&error];
    BOOL saveResult = [plistData writeToURL:plistCheatsForGameURL atomically:YES];
    NSLog(@"%s -- result of saveResult == %@", __PRETTY_FUNCTION__, @(saveResult));
    return saveResult;
}

+(NSArray *)loadROMCheats:(NSURL *) romPath
{
    NSURL *plistCheatsForGameURL = [romPath URLByDeletingPathExtension];
    plistCheatsForGameURL = [plistCheatsForGameURL URLByAppendingPathExtension:@"cheatsPlist"];
    NSLog(@"--  Current Cheats URL => %@", plistCheatsForGameURL);
    
    if([[NSFileManager defaultManager] fileExistsAtPath:[plistCheatsForGameURL path]])
    {
        NSError *error;
        NSArray *plistCheats = [NSPropertyListSerialization propertyListWithData:[NSData dataWithContentsOfURL:plistCheatsForGameURL]
                                                                   options:NSPropertyListImmutable
                                                                    format:NULL
                                                                      error:&error];
        NSLog(@"%s -- plistCheats = %@", __PRETTY_FUNCTION__, plistCheats);
        return plistCheats;
    }
    return @[];
}

@end

