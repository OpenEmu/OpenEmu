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

#import "OESaturnSystemController.h"
#import "OESaturnSystemResponder.h"
#import "OESaturnSystemResponderClient.h"

@implementation OESaturnSystemController

- (NSString*)systemName
{
    return @"Sega Saturn";
}

- (OECanHandleState)canHandleFile:(NSString *)path
{
    OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
    NSString *dataTrack = [cueSheet dataTrackPath];

    NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];
    NSLog(@"Saturn data track path: %@", dataTrackPath);

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
        
        if([dataTrackString isEqualToString:@"SEGA SEGASATURN "] || [otherDataTrackString isEqualToString:@"SEGA SEGASATURN "])
            canHandleFile = OECanHandleYes;

        [dataTrackFile closeFile];
    }
    return canHandleFile;
}

- (NSImage*)systemIcon
{
    NSString *imageName = ([[OELocalizationHelper sharedHelper] isRegionJAP]
                           ? @"saturn_jap_library"
                           : @"saturn_library");

    NSImage *image = [NSImage imageNamed:imageName];
    if(image == nil)
    {
        NSBundle *bundle = [NSBundle bundleForClass:[self class]];
        NSString *path = [bundle pathForImageResource:imageName];
        image = [[NSImage alloc] initWithContentsOfFile:path];
        [image setName:imageName];
    }
    return image;
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
    [dataTrackFile seekToFileOffset: 0x0];
    dataTrackBuffer = [dataTrackFile readDataOfLength: 16];
    [dataTrackFile seekToFileOffset: 0x010];
    otherDataTrackBuffer = [dataTrackFile readDataOfLength: 16];
    
    NSString *dataTrackString = [[NSString alloc]initWithData:dataTrackBuffer encoding:NSUTF8StringEncoding];
    NSString *otherDataTrackString = [[NSString alloc]initWithData:otherDataTrackBuffer encoding:NSUTF8StringEncoding];
    
    unsigned long long offsetFound;
    
    // Find which offset contains the 256-byte header
    if([dataTrackString isEqualToString:@"SEGA SEGASATURN "]) offsetFound = 0x0;
    if([otherDataTrackString isEqualToString:@"SEGA SEGASATURN "]) offsetFound = 0x010;
    
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
