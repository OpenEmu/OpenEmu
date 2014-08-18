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

#import "OEPSPSystemController.h"
#import "OEPSPSystemResponder.h"
#import "OEPSPSystemResponderClient.h"

#import <zlib.h>

CISO_H ciso;

@implementation OEPSPSystemController

// read header to detect PSP ISO & CSO
- (OECanHandleState)canHandleFile:(NSString *)path
{
    BOOL handleFileExtension = [super canHandleFileExtension:[path pathExtension]];
    OECanHandleState canHandleFile = OECanHandleNo;

    if(handleFileExtension)
    {
        // Handle cso file and return early
        if([[[path pathExtension] lowercaseString] isEqualToString:@"cso"])
            return OECanHandleYes;

        NSFileHandle *dataFile;
        NSData *dataBuffer;

        dataFile = [NSFileHandle fileHandleForReadingAtPath:path];
        [dataFile seekToFileOffset: 0x8008];
        dataBuffer = [dataFile readDataOfLength:8];

        NSString *dataString = [[NSString alloc] initWithData:dataBuffer encoding:NSUTF8StringEncoding];
        NSLog(@"'%@'", dataString);

        if([dataString isEqualToString:@"PSP GAME"])
            canHandleFile = OECanHandleYes;

        [dataFile closeFile];
    }
    return canHandleFile;
}

- (NSString *)serialLookupForFile:(NSString *)path
{
    NSFileHandle *dataFile;
    NSData *mode1DataBuffer, *mode2DataBuffer, *magicBuffer;

    dataFile = [NSFileHandle fileHandleForReadingAtPath: path];

    // Check if it's a CSO file
    magicBuffer = [dataFile readDataOfLength: 4];
    NSString *magicDataString = [[NSString alloc]initWithData:magicBuffer encoding:NSUTF8StringEncoding];

    if([magicDataString isEqualToString:@"CISO"])
        return [self serialLookupForCSOFile:path];

    // ISO 9660 CD001, check for MODE1 or MODE2
    [dataFile seekToFileOffset: 0x8001]; // MODE1
    mode1DataBuffer = [dataFile readDataOfLength: 5];
    [dataFile seekToFileOffset: 0x9319]; // MODE2
    mode2DataBuffer = [dataFile readDataOfLength: 5];

    NSUInteger discSectorSize, discSectorHeader, discOffset;

    NSString *mode1DataString = [[NSString alloc]initWithData:mode1DataBuffer encoding:NSUTF8StringEncoding];
    NSString *mode2DataString = [[NSString alloc]initWithData:mode2DataBuffer encoding:NSUTF8StringEncoding];

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
        [dataFile closeFile];
        return nil;
    }

    // Root Directory Record offset
    NSUInteger rootDirectoryRecordLocationOffset = 158; // 0x9E
    NSData *rootDirectoryRecordBuffer;

    [dataFile seekToFileOffset: discOffset + rootDirectoryRecordLocationOffset];
    rootDirectoryRecordBuffer = [dataFile readDataOfLength: 8];

    int rootDirectoryRecordSector = *((int *)[rootDirectoryRecordBuffer bytes]);

    NSUInteger rootDirectoryRecordOffset = rootDirectoryRecordSector * discSectorSize;

    //NSLog(@"Root Directory Record Offset: 0x%08X", (uint32_t)rootDirectoryRecordOffset);

    [dataFile seekToFileOffset: rootDirectoryRecordOffset];

    // Find PARAM.SFO
    BOOL foundPSPGameDir = NO;
    NSUInteger position = 0;
    NSUInteger pramsfoOffset = 0;
    NSData *pspgameDataBuffer;

    while(position < discSectorSize)
    {
        pspgameDataBuffer = [dataFile readDataOfLength: 8];
        NSString *pspgameString = [[NSString alloc]initWithData:pspgameDataBuffer encoding:NSUTF8StringEncoding];

        if([pspgameString isEqualToString:@"PSP_GAME"])
        {
            // PSP_GAME file record found
            //NSLog(@"PSP_GAME file record found at pos: 0x%03X", (uint32_t)position);
            foundPSPGameDir = true;
            break;
        }

        position++;

        [dataFile seekToFileOffset: rootDirectoryRecordOffset + position];
    }

    if(!foundPSPGameDir)
    {
        // bad dump, couldn't find PSP_GAME entry on the specified sector
        [dataFile closeFile];
        return nil;
    }
    else
    {
        // PSP_GAME Extent Location (Data location)
        NSUInteger pspgameDataOffset = (rootDirectoryRecordOffset + position) - 0x1F;

        NSData *pspgameExtentLocationDataBuffer;
        [dataFile seekToFileOffset: pspgameDataOffset];
        pspgameExtentLocationDataBuffer = [dataFile readDataOfLength: 8];

        int sizeExtentOffset = *((int *)[pspgameExtentLocationDataBuffer bytes]);

        NSUInteger pspgameExtentOffset = sizeExtentOffset * discSectorSize;
        //NSLog(@"PSP_GAME Extent (data) Offset: 0x%08X", (uint32_t)pspgameExtentOffset);

        pramsfoOffset = pspgameExtentOffset;
    }

    BOOL foundTitleIDFile = NO;
    position = 0;
    NSData *pramsfoDataBuffer;

    [dataFile seekToFileOffset: pramsfoOffset + position];

    while(position < discSectorSize)
    {
        pramsfoDataBuffer = [dataFile readDataOfLength: 9];
        NSString *pramsfoString = [[NSString alloc]initWithData:pramsfoDataBuffer encoding:NSUTF8StringEncoding];

        if([pramsfoString isEqualToString:@"PARAM.SFO"])
        {
            // PARAM.SFO file record found
            //NSLog(@"PARAM.SFO file record found at pos: 0x%03X", (uint32_t)position);
            foundTitleIDFile = true;
            break;
        }

        position++;

        [dataFile seekToFileOffset: pramsfoOffset + position];
    }

    if(!foundTitleIDFile)
    {
        // bad dump, couldn't find PARAM.SFO entry on the specified sector
        [dataFile closeFile];
        return nil;
    }
    else
    {
        // PARAM.SFO Extent Location (Data location)
        NSUInteger pramsfoDataOffset = (pramsfoOffset + position) - 0x1F;

        NSData *pramsfoExtentLocationDataBuffer;
        [dataFile seekToFileOffset: pramsfoDataOffset];
        pramsfoExtentLocationDataBuffer = [dataFile readDataOfLength: 8];

        int sizeExtentOffset = *((int *)[pramsfoExtentLocationDataBuffer bytes]);

        NSUInteger pramsfoExtentOffset = sizeExtentOffset * discSectorSize;
        //NSLog(@"PARAM.SFO Extent (data) Offset: 0x%08X", (uint32_t)pramsfoExtentOffset);

        // Data length (size)
        NSUInteger pramsfoDataLengthOffset = (pramsfoOffset + position) - 0x17;

        NSData *pramsfoDataLengthDataBuffer;
        [dataFile seekToFileOffset: pramsfoDataLengthOffset];
        pramsfoDataLengthDataBuffer = [dataFile readDataOfLength: 8];

        NSUInteger pramsfoDataLength = *((int *)[pramsfoDataLengthDataBuffer bytes]);

        //NSLog(@"PARAM.SFO Data Length: 0x%08X", (uint32_t)pramsfoDataLength);

        NSData *pramsfoHeaderDataBuffer;
        [dataFile seekToFileOffset: pramsfoExtentOffset + discSectorHeader];
        pramsfoHeaderDataBuffer = [dataFile readDataOfLength: pramsfoDataLength];

        NSString *unformattedOutput = [[NSString alloc]initWithData:pramsfoHeaderDataBuffer encoding:NSASCIIStringEncoding];

        // Replace null chars
        NSString *output = [unformattedOutput stringByReplacingOccurrencesOfString:@"\0" withString:@" "];

        //NSLog(@"output: %@", output);

        // Scan for UG and end of serial, format string and return
        NSString *serial;
        NSScanner *scanner = [NSScanner scannerWithString:output];
        [scanner scanUpToString:@"UG" intoString:nil];
        [scanner scanUpToString:@"  " intoString:nil];
        [scanner scanUpToString:@" " intoString:&serial];

        NSMutableString *formattedSerial = [NSMutableString stringWithString:serial];
        [formattedSerial insertString:@"-" atIndex:4];

        NSLog(@"Serial: %@", formattedSerial);

        [dataFile closeFile];

        return formattedSerial;
    }
}

- (NSString *)serialLookupForCSOFile:(NSString *)path
{
    unsigned int index , index2;
    unsigned long long read_pos , read_size;
    unsigned int *index_buf;
    unsigned char *block_buf1, *block_buf2;
    int index_size, block, cmp_size, status, plain, ciso_total_block;
    FILE *fin;
    z_stream z;

    // Uncompress the CSO header
    if((fin = fopen([path UTF8String], "rb")) == NULL)
        return nil;

    if(fread(&ciso, 1, sizeof(ciso), fin) != sizeof(ciso))
        return nil;

    if(ciso.magic[0] != 'C' || ciso.magic[1] != 'I' || ciso.magic[2] != 'S' || ciso.magic[3] != 'O' || ciso.block_size ==0  || ciso.total_bytes == 0)
        return nil;

    ciso_total_block = (int)ciso.total_bytes / ciso.block_size;
    index_size = (ciso_total_block + 1 ) * sizeof(unsigned long);
    index_buf  = malloc(index_size);
    block_buf1 = malloc(ciso.block_size);
    block_buf2 = malloc(ciso.block_size*2);

    if(!index_buf || !block_buf1 || !block_buf2)
        return nil;

    memset(index_buf,0,index_size);

    if( fread(index_buf, 1, index_size, fin) != index_size )
        return nil;

    // Init zlib
    z.zalloc = Z_NULL;
    z.zfree  = Z_NULL;
    z.opaque = Z_NULL;

    NSMutableData *header = [[NSMutableData alloc] init];

    // 16 blocks are enough to get the header up to the serial
    for(block = 0;block <= 16 ; block++)
    {
        if (inflateInit2(&z,-15) != Z_OK)
            return nil;

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
            return nil;

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
                return nil;
            cmp_size = (int)ciso.block_size - z.avail_out;
            if(cmp_size != ciso.block_size)
                return nil;
        }
        [header appendData:[NSData dataWithBytes:block_buf1 length:cmp_size]];
    }
    fclose(fin);

    // Get the Serial
    NSString *serial = [[NSString alloc]initWithData:[header subdataWithRange:NSMakeRange(0x8373, 10)] encoding:NSASCIIStringEncoding];
    NSLog(@"Serial: %@", serial);
    
    return serial;
}

@end
