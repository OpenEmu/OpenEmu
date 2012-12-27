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

#import "OEPCFXSystemController.h"
#import "OEPCFXSystemResponder.h"
#import "OEPCFXSystemResponderClient.h"

@implementation OEPCFXSystemController

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
    NSLog(@"PC-FX data track path: %@", dataTrackPath);

    BOOL valid = [super canHandleFileExtension:[path pathExtension]];
    if (valid)
    {
        NSFileHandle *dataTrackFile;
        NSData *dataTrackBuffer;
        
        dataTrackFile = [NSFileHandle fileHandleForReadingAtPath: dataTrackPath];
        //[dataTrackFile seekToEndOfFile];
        //NSUInteger endOfDataTrackFile = [dataTrackFile offsetInFile];

        valid = NO;
        
        // Need a better, faster way to do this as it will slow import of other CD systems
        for (NSUInteger i=0x0; i < 16337024; i+=0x10) // this is a bad guess at the range
        {
            [dataTrackFile seekToFileOffset: i];
            dataTrackBuffer = [dataTrackFile readDataOfLength: 16];
            NSString *dataTrackString = [[NSString alloc]initWithData:dataTrackBuffer encoding:NSUTF8StringEncoding];
            if ([dataTrackString isEqualToString:@"PC-FX:Hu_CD-ROM "])
            {
                NSLog (@"'%@' at offset = 0x%lX", dataTrackString, i);
                valid = YES;
                break;
            }
        }

        [dataTrackFile closeFile];
    }
    //return valid;
    return valid?OECanHandleYes:OECanHandleNo;
}

@end
