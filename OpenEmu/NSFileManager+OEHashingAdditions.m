/*
 Copyright (c) 2020, OpenEmu Team
 
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

#import "NSFileManager+OEHashingAdditions.h"
#import <CommonCrypto/CommonDigest.h>

#define HASH_READ_CHUNK_SIZE (1024 * 32)

@implementation NSFileManager (OEHashingAdditions)

// MARK: - MD5
// TODO: replace with CryptoKit Insecure.MD5 once macOS 10.15+ is minimum supported
- (BOOL)hashFileAtURL:(NSURL*)url md5:(NSString**)outMD5 error:(NSError**)error
{
    return [self hashFileAtURL:url headerSize:0 md5:outMD5 error:error];
}

- (BOOL)hashFileAtURL:(NSURL*)url headerSize:(int)headerSize md5:(NSString**)outMD5 error:(NSError**)error
{
    NSFileHandle *handle = [NSFileHandle fileHandleForReadingFromURL:url error:error];
    if(handle == nil || !outMD5)
        return NO;

    [handle seekToFileOffset:headerSize];

    CC_MD5_CTX md5Context;
    CC_MD5_Init(&md5Context);

    do {
        @autoreleasepool
        {
            NSData *data = [handle readDataOfLength:HASH_READ_CHUNK_SIZE];
            const unsigned char *bytes = data.bytes;
            NSUInteger length = data.length;
            CC_MD5_Update(&md5Context, bytes, (CC_LONG)length);

            if(data == nil || length < HASH_READ_CHUNK_SIZE) break;
        }
    } while(YES);

    [handle closeFile];

    // Finalize MD5
    unsigned char md5Digest[CC_MD5_DIGEST_LENGTH];
    CC_MD5_Final(md5Digest, &md5Context);

	if(outMD5 != NULL)
		*outMD5 = [NSString stringWithFormat:@"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                   md5Digest[0], md5Digest[1],
                   md5Digest[2], md5Digest[3],
                   md5Digest[4], md5Digest[5],
                   md5Digest[6], md5Digest[7],
                   md5Digest[8], md5Digest[9],
                   md5Digest[10], md5Digest[11],
                   md5Digest[12], md5Digest[13],
                   md5Digest[14], md5Digest[15]];

    return YES;
}

@end
