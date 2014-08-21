/*
 Copyright (c) 2012, OpenEmu Team
 
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

#import "OESegaCDSystemController.h"
#import "OESegaCDSystemResponder.h"
#import "OESegaCDSystemResponderClient.h"

@implementation OESegaCDSystemController

- (NSString *)systemName
{
    return ([[OELocalizationHelper sharedHelper] isRegionNA]
            ? @"Sega CD"
            : @"Sega Mega-CD");
}

- (OECanHandleState)canHandleFile:(NSString *)path
{
    //if (![[path pathExtension] isEqualToString:@".cue"])
    //{
    //    return OECanHandleUncertain;
    //}

    //BOOL valid = NO;
    OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
    NSString *dataTrack = [cueSheet dataTrackPath];

    NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];
    NSLog(@"SCD data track path: %@", dataTrackPath);

    BOOL handleFileExtension = [super canHandleFileExtension:[path pathExtension]];
    OECanHandleState canHandleFile = OECanHandleNo;

    if(handleFileExtension)
    {
        NSFileHandle *dataTrackFile;
        NSData *dataTrackBuffer, *otherDataTrackBuffer;
        
        dataTrackFile = [NSFileHandle fileHandleForReadingAtPath: dataTrackPath];
        [dataTrackFile seekToFileOffset: 0x0];
        dataTrackBuffer = [dataTrackFile readDataOfLength: 16];
        [dataTrackFile seekToFileOffset: 0x010];
        otherDataTrackBuffer = [dataTrackFile readDataOfLength: 16];
        
        NSString *dataTrackString = [[NSString alloc]initWithData:dataTrackBuffer encoding:NSUTF8StringEncoding];
        NSString *otherDataTrackString = [[NSString alloc]initWithData:otherDataTrackBuffer encoding:NSUTF8StringEncoding];
        NSLog(@"'%@'", dataTrackString);
        NSLog(@"'%@'", otherDataTrackString);
        NSArray *dataTrackList = @[ @"SEGADISCSYSTEM  ", @"SEGABOOTDISC    ", @"SEGADISC        ", @"SEGADATADISC    " ];

        for(NSString *d in dataTrackList)
        {
            if([dataTrackString isEqualToString:d] || [otherDataTrackString isEqualToString:d])
            {
                canHandleFile = OECanHandleYes;
                break;
            }
        }

        [dataTrackFile closeFile];
    }
    return canHandleFile;
}

- (NSString *)headerLookupForFile:(NSString *)path
{
    // Path is a cuesheet so get the first data track from the file for reading
    OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
    NSString *dataTrack = [cueSheet dataTrackPath];
    NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];
    
    NSFileHandle *dataTrackFile;
    NSData *dataTrackBuffer, *otherDataTrackBuffer, *headerDataTrackBuffer;
    
    // Read both offsets because of various dumps
    dataTrackFile = [NSFileHandle fileHandleForReadingAtPath: dataTrackPath];
    [dataTrackFile seekToFileOffset: 0x0100];
    dataTrackBuffer = [dataTrackFile readDataOfLength: 16];
    [dataTrackFile seekToFileOffset: 0x0110];
    otherDataTrackBuffer = [dataTrackFile readDataOfLength: 16];
    
    NSString *dataTrackString = [[NSString alloc]initWithData:dataTrackBuffer encoding:NSUTF8StringEncoding];
    NSString *otherDataTrackString = [[NSString alloc]initWithData:otherDataTrackBuffer encoding:NSUTF8StringEncoding];
    
    unsigned long long offsetFound = 0;
    NSArray *dataTrackList = @[ @"SEGA GENESIS    ", @"SEGA MEGA DRIVE " ];
    
    // Find which offset contains the 256-byte header
    for(NSString *d in dataTrackList)
    {
        if([dataTrackString isEqualToString:d])
        {
            offsetFound = 0x0100;
            break;
        }
        else if ([otherDataTrackString isEqualToString:d])
        {
            offsetFound = 0x0110;
            break;
        }
    }
    // Read the full header at the offset found
    [dataTrackFile seekToFileOffset: offsetFound];
    headerDataTrackBuffer = [dataTrackFile readDataOfLength: 256];
    
    [dataTrackFile closeFile];
    
    // Format the hexadecimal representation and return
    NSString *buffer = [[headerDataTrackBuffer description] uppercaseString];
    NSString *hex = [[buffer componentsSeparatedByCharactersInSet:[[NSCharacterSet alphanumericCharacterSet] invertedSet]] componentsJoinedByString:@""];
    
    return hex;
}

@end
