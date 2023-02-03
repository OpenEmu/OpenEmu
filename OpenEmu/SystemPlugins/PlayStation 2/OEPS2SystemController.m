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

#import "OEPS2SystemController.h"
#import <zlib.h>

CISO_H ciso;

@implementation OEPS2SystemController

static inline char itoh(int i) {
    if (i > 9) return 'A' + (i - 10);
    return '0' + i;
}

- (NSString *)NSDataToHex:(NSData *) data {
    NSUInteger i, len;
    unsigned char *buf, *bytes;
    
    len = data.length;
    bytes = (unsigned char*)data.bytes;
    buf = malloc(len*2);
    
    for (i=0; i<len; i++) {
        buf[i*2] = itoh((bytes[i] >> 4) & 0xF);
        buf[i*2+1] = itoh(bytes[i] & 0xF);
    }
    
    return [[NSString alloc] initWithBytesNoCopy:buf
                                          length:len*2
                                        encoding:NSASCIIStringEncoding
                                    freeWhenDone:YES];
}
- (OEFileSupport)canHandleFile:(__kindof OEFile *)file
{
    
    // Handle cso file and return early
    if ([file.fileExtension isEqualToString:@"cso"])
    {
        unsigned int index , index2;
        unsigned long long read_pos , read_size;
        unsigned int *index_buf;
        unsigned char *block_buf1, *block_buf2;
        int index_size, block, cmp_size, status, plain, ciso_total_block;
        FILE *fin;
        z_stream z;
        
        // Uncompress the CSO header
        if((fin = fopen(file.dataTrackFileURL.path.fileSystemRepresentation, "rb")) == NULL)
            return OEFileSupportNo;
        
        if(fread(&ciso, 1, sizeof(ciso), fin) != sizeof(ciso))
            return OEFileSupportNo;
        
        if(ciso.magic[0] != 'C' || ciso.magic[1] != 'I' || ciso.magic[2] != 'S' || ciso.magic[3] != 'O' || ciso.block_size ==0  || ciso.total_bytes == 0)
            return OEFileSupportNo;
        
        ciso_total_block = (int)ciso.total_bytes / ciso.block_size;
        index_size = (ciso_total_block + 1 ) * sizeof(unsigned long);
        index_buf  = malloc(index_size);
        block_buf1 = malloc(ciso.block_size);
        block_buf2 = malloc(ciso.block_size*2);
        
        if(!index_buf || !block_buf1 || !block_buf2)
            return OEFileSupportNo;
        
        memset(index_buf,0,index_size);
        
        if( fread(index_buf, 1, index_size, fin) != index_size )
            return OEFileSupportNo;
        
        // Init zlib
        z.zalloc = Z_NULL;
        z.zfree  = Z_NULL;
        z.opaque = Z_NULL;
        
        NSMutableData *header = [[NSMutableData alloc] init];
        
        // 16 blocks are enough to get the header up to the serial
        for(block = 0;block <= 16 ; block++)
        {
            if (inflateInit2(&z,-15) != Z_OK)
                return OEFileSupportNo;
            
            index  = index_buf[block];
            plain  = index & 0x80000000;
            index  &= 0x7fffffff;
            read_pos = index << (ciso.align);
            if(plain)
            {
                read_size = ciso.block_size;
            }
            else
            {
                index2 = index_buf[block+1] & 0x7fffffff;
                read_size = (index2-index) << (ciso.align);
            }
            fseek(fin,read_pos,SEEK_SET);
            
            z.avail_in  = (unsigned int)fread(block_buf2, 1, read_size , fin);
            if(z.avail_in != read_size)
                return OEFileSupportNo;
            
            if(plain)
            {
                memcpy(block_buf1,block_buf2,read_size);
                cmp_size = (int)read_size;
            }
            else
            {
                z.next_out  = block_buf1;
                z.avail_out = (unsigned int)ciso.block_size;
                z.next_in   = block_buf2;
                status = inflate(&z, Z_FULL_FLUSH);
                if (status != Z_STREAM_END)
                    return OEFileSupportNo;
                cmp_size = (int)ciso.block_size - z.avail_out;
                if(cmp_size != ciso.block_size)
                    return OEFileSupportNo;
            }
            
             [header appendData:[NSData dataWithBytes:block_buf1 length:cmp_size]];
        }
        fclose(fin);
        
        // Look for a Serial
        NSString *serial = [[[NSString alloc]initWithData:[header subdataWithRange:NSMakeRange(0x8373, 10)] encoding:NSISOLatin1StringEncoding]  stringByTrimmingCharactersInSet:[[NSCharacterSet alphanumericCharacterSet] invertedSet]];
        NSLog(@"Serial: %@", serial);
        NSLog(@"Serial Len: %lu", (unsigned long)[serial length]);
              
        if(![serial  isEqual: @""]) //We found a PSP-Serial so it's not a PS2 game
            return OEFileSupportNo;
        
        return OEFileSupportYes;
    }
    
    if (!([file isKindOfClass:[OEDiscDescriptor class]] || [file.fileExtension caseInsensitiveCompare:@"iso"] == NSOrderedSame))
        return OEFileSupportNo;

    NSString *dataTrackString = [file readASCIIStringInRange:NSMakeRange(0x8008, 11)];
    NSString *dataTrackString2 = [file readASCIIStringInRange:NSMakeRange(0x9320, 11)];

    // Only tested this on one ISO. Might be different for DVDs.
    return ([dataTrackString isEqualToString:@"PLAYSTATION"] || [dataTrackString2 isEqualToString:@"PLAYSTATION"]) ? OEFileSupportYes : OEFileSupportNo;
}

- (NSString *)serialLookupForFile:(__kindof OEFile *)file
{
    if(!([file isKindOfClass:[OEDiscDescriptor class]] || [file.fileExtension caseInsensitiveCompare:@"iso"] == NSOrderedSame))
        return nil;

    // ISO 9660 CD001, check for MODE1 or MODE2

    NSString *mode1DataString = [file readASCIIStringInRange:NSMakeRange(0x8001, 5)];
    NSString *mode2DataString = [file readASCIIStringInRange:NSMakeRange(0x9319, 5)];
    
    NSUInteger discSectorSize, discSectorHeader, discOffset;

    // Find which offset contains CD001
    if([mode1DataString isEqualToString:@"CD001"])
    {
        // ISO9660/MODE1/2048
        discSectorSize	= 2048; // 0x800
        discSectorHeader = 0;
        discOffset	= ((discSectorSize * 16) + discSectorHeader);
        
    }
    else if([mode2DataString isEqualToString:@"CD001"])
    {
        // ISO9660/MODE2/FORM1/2352
        discSectorSize = 2352;  // 0x930
        discSectorHeader = 24; // 0x18
        discOffset = ((discSectorSize * 16) + discSectorHeader);
    }
    else
    {
        // bad dump, not ISO 9660
        return nil;
    }
    
    // Root Directory Record offset
    NSUInteger rootDirectoryRecordLocationOffset = 0x9E;
    NSData *rootDirectoryRecordBuffer = [file readDataInRange:NSMakeRange(discOffset + rootDirectoryRecordLocationOffset, 8)];
    
    int rootDirectoryRecordSector = *((int *)[rootDirectoryRecordBuffer bytes]);
    
    NSUInteger rootDirectoryRecordOffset = rootDirectoryRecordSector * discSectorSize;
    
    //NSLog(@"Root Directory Record Offset: 0x%08X", (uint32_t)rootDirectoryRecordOffset);

    // Find SYSTEM.CNF
    BOOL foundTitleIDFile = NO;
    NSUInteger position = 0;
    while(position < discSectorSize)
    {
        NSString *systemcnfString = [file readASCIIStringInRange:NSMakeRange(rootDirectoryRecordOffset + position, 10)];
        if([systemcnfString isEqualToString:@"SYSTEM.CNF"])
        {
            // SYSTEM.CNF file record found
            //NSLog(@"SYSTEM.CNF file record found at pos: 0x%03X", (uint32_t)position);
            foundTitleIDFile = YES;
            break;
        }
        
        position++;
    }
    
    if(!foundTitleIDFile)
    {
        // bad dump, couldn't find SYSTEM.CNF entry on the specified sector
        return nil;
    }

    // SYSTEM.CNF Extent Location (Data location)
    NSUInteger systemcnfDataOffset = (rootDirectoryRecordOffset + position) - 0x1F;
    NSData *systemcnfExtentLocationDataBuffer = [file readDataInRange:NSMakeRange(systemcnfDataOffset, 8)];

    int sizeExtentOffset = *((int *)[systemcnfExtentLocationDataBuffer bytes]);
    NSUInteger systemcnfExtentOffset = sizeExtentOffset * discSectorSize;
    //NSLog(@"SYSTEM.CNF Extent (data) Offset: 0x%08X", (uint32_t)systemcnfExtentOffset);

    // Data length (size)
    NSUInteger systemcnfDataLengthOffset = (rootDirectoryRecordOffset + position) - 0x17;
    NSData *systemcnfDataLengthDataBuffer = [file readDataInRange:NSMakeRange(systemcnfDataLengthOffset, 8)];
    NSUInteger systemcnfDataLength = *((int *)[systemcnfDataLengthDataBuffer bytes]);

    //NSLog(@"SYSTEM.CNF Data Length: 0x%08X", (uint32_t)systemcnfDataLength);

    NSString *output = [file readASCIIStringInRange:NSMakeRange(systemcnfExtentOffset + discSectorHeader, systemcnfDataLength)];

    //NSLog(@"output: %@", output);

    // RegEx pattern match the disc serial
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"BOOT2\\s*=\\s*?cdrom0:\\\\?(.+\\\\)?(.+?(?=;|\\s))" options:NSRegularExpressionCaseInsensitive error:nil];
    NSTextCheckingResult *match = [regex firstMatchInString:output options:0 range:NSMakeRange(0, [output length])];

    if(match == nil)
    {
        NSLog(@"RegEx pattern could not match serial. Full string:\n%@", output);
        return @"NO MATCH";
    }

    NSString *serial = [output substringWithRange:[match rangeAtIndex:2]];

    // Format serial and return
    serial = [[serial componentsSeparatedByCharactersInSet:[[NSCharacterSet alphanumericCharacterSet] invertedSet]] componentsJoinedByString:@""];

    NSMutableString *formattedSerial = [NSMutableString stringWithString:[serial uppercaseString]];
    [formattedSerial insertString:@"-" atIndex:4];

    NSLog(@"Serial: %@", formattedSerial);

    return formattedSerial;
}

@end
