/*
 Copyright (c) 2014, OpenEmu Team
 
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

#import "OEN64SystemController.h"

@implementation OEN64SystemController

- (NSString *)headerLookupForFile:(__kindof OEFile *)file
{
    // Read the full 64 byte header
    NSData *dataBuffer = [file readDataInRange:NSMakeRange(0, 64)];

    unsigned char temp;
    unsigned char *rom = (unsigned char *)dataBuffer.bytes;
    NSUInteger romSize = dataBuffer.length;
    
    // Read the first 4 bytes of the header to get the 'magic word' in hex
    NSMutableString *hexString = [[NSMutableString alloc] initWithCapacity:4];
    for(NSUInteger i = 0; i < 4;)
    {
        [hexString appendFormat:@"%02X", rom[i]];
        i++;
    }
    
    // Detect rom formats using 'magic word' hex and swap byte order to [ABCD] if neccessary
    // .z64 rom is N64 native (big endian) with header 0x80371240 [ABCD], no need to swap
    
    // Byteswapped .v64 rom with header 0x37804012 [BADC]
    if([hexString isEqualToString:@"37804012"])
    {
        for(int i = 0; i < romSize; i+=2)
        {
            temp=rom[i];
            rom[i]=rom[i+1];
            rom[i+1]=temp;
        }
    }
    // Little endian .n64 rom with header 0x40123780 [DCBA]
    else if([hexString isEqualToString:@"40123780"])
    {
        for(NSUInteger i = 0; i < romSize; i+=4)
        {
            temp=rom[i];
            rom[i]=rom[i+3];
            rom[i+3]=temp;
            temp=rom[i+1];
            rom[i+1]=rom[i+2];
            rom[i+2]=temp;
        }
    }
    // Wordswapped .n64 rom with header 0x12408037 [CDAB]
    else if([hexString isEqualToString:@"12408037"])
    {
        for(NSUInteger i = 0; i < romSize; i+=4)
        {
            temp=rom[i];
            rom[i]=rom[i+2];
            rom[i+2]=temp;
            temp=rom[i+1];
            rom[i+1]=rom[i+3];
            rom[i+3]=temp;
        }
    }

    // Final rom header in hex after any swapping that may have occured
    NSData *romBuffer = [NSData dataWithBytes:rom length:romSize];
    
    // Format the hexadecimal representation and return
    NSString *hex = [romBuffer oe_hexStringRepresentation];
    
    return hex;
}

- (CGFloat)coverAspectRatio
{
    return ([[OELocalizationHelper sharedHelper] isRegionJAP]
            ? 1.37
            : 0.7);
}

@end
