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

#import "OE3DOSystemController.h"
#import "OE3DOSystemResponder.h"
#import "OE3DOSystemResponderClient.h"
#import "OELocalizationHelper.h"

@implementation OE3DOSystemController

- (NSString*)systemName
{
    return @"3DO";
}

- (OECanHandleState)canHandleFile:(NSString *)path
{
    OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
    NSString *dataTrack = [cueSheet dataTrackPath];
	
    NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];
    NSLog(@"3DO data track path: %@", dataTrackPath);
	
    BOOL handleFileExtension = [super canHandleFileExtension:[path pathExtension]];
    OECanHandleState canHandleFile = OECanHandleNo;
    
    if (handleFileExtension)
    {
        NSFileHandle *dataTrackFile;
        NSData *dataTrackBuffer, *otherDataTrackBuffer;
		
        dataTrackFile = [NSFileHandle fileHandleForReadingAtPath: dataTrackPath];
        [dataTrackFile seekToFileOffset: 0x0];
        dataTrackBuffer = [dataTrackFile readDataOfLength: 8];
        [dataTrackFile seekToFileOffset: 0x28];
        otherDataTrackBuffer = [dataTrackFile readDataOfLength: 6];
		
        NSString *dataTrackString = [[NSString alloc]initWithData:dataTrackBuffer encoding:NSUTF8StringEncoding];
        NSString *otherDataTrackString = [[NSString alloc]initWithData:otherDataTrackBuffer encoding:NSUTF8StringEncoding];
        NSLog(@"%@", dataTrackString);
        NSLog(@"%@", otherDataTrackString);
        if ([dataTrackString isEqualToString:@"\x01\x5a\x5a\x5a\x5a\x5a\x01\x00"] && [otherDataTrackString isEqualToString:@"CD-ROM"])
            canHandleFile = OECanHandleYes;
		
        [dataTrackFile closeFile];
    }
    return canHandleFile;
}

@end
