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


#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

#define TODO(x)
#define FIXME(x)
#define NOTE(x)

#define BOOL_STR(value) ((value) ? "YES" : "NO")

#define OECoreDataMainThreadAssertion() NSAssert([NSThread isMainThread], @"Only call on main thread!")
// output must be at least 2*len+1 bytes
void tohex(const unsigned char *input, size_t len, char *output);
void OEPrintFirstResponderChain(void);
NSArray *OENextRespondersFromResponder(NSResponder *responder);

NSString *temporaryDirectoryForDecompressionOfPath(NSString *aPath);
NSString *decompressedPathForRomAtPath(NSString *aPath);
NSString *OETemporaryDirectoryForDecompressionOfFileWithHash(NSString *aPath, NSString * __nullable hash);
NSString *OEDecompressFileInArchiveAtPathWithHash(NSString *aPath, NSString * __nullable hash, BOOL * __nullable success);

NS_ASSUME_NONNULL_END
