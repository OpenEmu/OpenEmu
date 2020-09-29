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

@implementation OEPCFXSystemController

- (OEFileSupport)canHandleFile:(__kindof OEFile *)file
{
    if (![file isKindOfClass:[OEDiscDescriptor class]])
        return OEFileSupportNo;

    OEDiscDescriptor *descriptor = file;
    for(NSURL *dataTrack in descriptor.referencedBinaryFileURLs)
    {
        NSError *error = nil;
        NSData *dataTrackBuffer = [NSData dataWithContentsOfURL:dataTrack options:NSDataReadingUncached error:&error];

        NSString *dataTrackString = @"PC-FX:Hu_CD-ROM ";
        NSData *dataSearch = [dataTrackString dataUsingEncoding:NSUTF8StringEncoding];
        // this still slows import down but we need to scan the disc as there's no common offset
        NSRange indexOfData = [dataTrackBuffer rangeOfData: dataSearch options:0 range:NSMakeRange(0, [dataTrackBuffer length])];

        if(indexOfData.length == 0)
            continue;

        NSLog(@"PC-FX data track: %@", dataTrack);
        NSLog (@"'%@' at offset = 0x%lX", dataTrackString, indexOfData.location);
        return OEFileSupportYes;
    }

    return OEFileSupportNo;
}

@end
