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

#import "OEUtilities.m"
#import <CommonCrypto/CommonDigest.h>
#import "OEBuildVersion.h"
// Compression support
#import <XADMaster/XADArchive.h>

// output must be at least 2*len+1 bytes
void tohex(const unsigned char *input, size_t len, char *output)
{
    static char table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (int i = 0; i < len; ++i) {
        output[2*i] = table[input[i]>>4];
        output[2*i+1] = table[input[i]&0xF];
    }
    output[2*len] = '\0';
}

void OEPrintFirstResponderChain(void)
{
    NSWindow *keyWindow = [NSApp keyWindow];
    
    if(keyWindow == nil) return;
    
    NSLog(@"responders: %@", OENextRespondersFromResponder([keyWindow firstResponder]));
}

NSArray *OENextRespondersFromResponder(NSResponder *responder)
{
    if(responder == nil) return @[ ];
    
    NSMutableArray *responders = [NSMutableArray array];
    
    while(responder != nil)
    {
        [responders addObject:responder];
        responder = [responder nextResponder];
    }
    
    return responders;
}


NSString *OETemporaryDirectoryForDecompressionOfFileWithHash(NSString *aPath, NSString *fileHash)
{
    NSFileManager *fm = [NSFileManager new];
    NSError *error;
    
    // compute the filename hash
    unsigned char hash[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(aPath.fileSystemRepresentation, (CC_LONG)strlen(aPath.fileSystemRepresentation), hash);

    char hexhash[2*CC_SHA1_DIGEST_LENGTH+1];
    tohex(hash, CC_SHA1_DIGEST_LENGTH, hexhash);
    
    NSString *finalhash;
    if (fileHash)
        finalhash = [NSString stringWithFormat:@"%s_%@", hexhash, fileHash];
    else
        finalhash = [NSString stringWithUTF8String:hexhash];

    // get the bundle identifier of ourself, or our parent app if we have none
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *folder = [bundle bundleIdentifier];
    if (!folder) {
        NSString *path = [bundle bundlePath];
        path = [[path stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
        bundle = [NSBundle bundleWithPath:path];
        folder = [bundle bundleIdentifier];
        if (!folder) {
            DLog(@"Couldn't get bundle identifier of OpenEmu");
            folder = @"OpenEmu";
        }
    }
    folder = [NSTemporaryDirectory() stringByAppendingPathComponent:folder];
    folder = [folder stringByAppendingPathComponent:finalhash];
    if (![fm createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:&error]) {
        DLog(@"Couldn't create temp directory %@, %@", folder, error);
        return nil;
    }

    return folder;
}


NSString *OEDecompressFileInArchiveAtPathWithHash(NSString *aPath, NSString * __nullable hash, BOOL * __nullable outSuccess)
{
    if (outSuccess)
        *outSuccess = NO;
        
    // check path for :entryIndex appendix, extract it and restore original path
    // paths will look like this /path/to/rom/file.zip:2

    int entryIndex = 0;
    NSMutableArray *components = [[aPath componentsSeparatedByString:@":"] mutableCopy];
    NSString *entry = [components lastObject];
    if([[NSString stringWithFormat:@"%ld", [entry integerValue]] isEqualToString:entry])
    {
        entryIndex = [entry intValue];
        [components removeLastObject];
        aPath = [components componentsJoinedByString:@":"];
    }

    // we check for known compression types for the ROM at the path
    // If we detect one, we decompress it and store it in /tmp at a known location
    XADArchive *archive = nil;
    @try {
        archive = [XADArchive archiveForFile:aPath];
    }
    @catch (NSException *exc)
    {
        archive = nil;
    }

    if(archive == nil || [archive numberOfEntries] <= entryIndex)
        return aPath;

    // XADMaster identifies some legit Mega Drive as LZMA archives
    NSString *formatName = [archive formatName];
    if ([formatName isEqualToString:@"MacBinary"] || [formatName isEqualToString:@"LZMA_Alone"])
        return aPath;

    if(![archive entryHasSize:entryIndex] || [archive uncompressedSizeOfEntry:entryIndex]==0 || [archive entryIsEncrypted:entryIndex] || [archive entryIsDirectory:entryIndex] || [archive entryIsArchive:entryIndex])
        return aPath;

    NSFileManager *fm = [NSFileManager new];
    NSString *folder = OETemporaryDirectoryForDecompressionOfFileWithHash(aPath, hash);
    NSString *tmpPath = [folder stringByAppendingPathComponent:[archive nameOfEntry:entryIndex]];
    if([[tmpPath pathExtension] length] == 0 && [[aPath pathExtension] length] > 0)
    {
        // we need an extension
        tmpPath = [tmpPath stringByAppendingPathExtension:[aPath pathExtension]];
    }

    BOOL isdir;
    if([fm fileExistsAtPath:tmpPath isDirectory:&isdir] && !isdir)
    {
        DLog(@"Found existing decompressed ROM for path %@", aPath);
        return tmpPath;
    }

    BOOL success = YES;
    @try
    {
        success = [archive _extractEntry:entryIndex as:tmpPath deferDirectories:NO dataFork:YES resourceFork:NO];
    }
    @catch (NSException *exception)
    {
        success = NO;
    }

    if(!success)
    {
        [fm removeItemAtPath:folder error:nil];
        return aPath;
    }

    if (outSuccess)
        *outSuccess = YES;
    return tmpPath;
}


NSString *temporaryDirectoryForDecompressionOfPath(NSString *aPath)
{
    return OETemporaryDirectoryForDecompressionOfFileWithHash(aPath, nil);
}


NSString *decompressedPathForRomAtPath(NSString *aPath)
{
    return OEDecompressFileInArchiveAtPathWithHash(aPath, nil, NULL);
}


#ifdef DebugLocalization
static NSFileHandle *OELocalizationLog = nil;
static NSMutableDictionary *OELocalizationTableLog = nil;
NSString *OELogLocalizedString(NSString *key, NSString *comment, NSString *fileName, int line, const char* function, NSString *table)
{
    NSFileHandle *handle = nil;
    if(table == nil && OELocalizationLog == nil)
    {
        NSURL *url = [NSURL fileURLWithPath:[@"~/Desktop/OpenEmu Runtime Analysis.strings" stringByExpandingTildeInPath]];
        [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
        [[NSFileManager defaultManager] createFileAtPath:[url path] contents:[NSData data] attributes:nil];
        OELocalizationLog = [NSFileHandle fileHandleForWritingToURL:url error:nil];

        NSString *version = [NSString stringWithFormat:@"/*%@*/\n", LONG_BUILD_VERSION];
        [OELocalizationLog writeData:[version dataUsingEncoding:NSUTF8StringEncoding]];
    }
    else if(table != nil && [OELocalizationTableLog objectForKey:table] == nil)
    {
        if(OELocalizationTableLog == nil) OELocalizationTableLog = [NSMutableDictionary dictionary];
        NSString *path = [NSString stringWithFormat:@"~/Desktop/OpenEmu Runtime Analysis (%@).strings", table];
        NSURL *url = [NSURL fileURLWithPath:[path stringByExpandingTildeInPath]];
        [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
        [[NSFileManager defaultManager] createFileAtPath:[url path] contents:[NSData data] attributes:nil];
        NSFileHandle *h = [NSFileHandle fileHandleForWritingToURL:url error:nil];

        NSString *version = [NSString stringWithFormat:@"/*%@*/\n", LONG_BUILD_VERSION];
        [h writeData:[version dataUsingEncoding:NSUTF8StringEncoding]];

        [OELocalizationTableLog setObject:h forKey:table];
    }

    if(table == nil) handle = OELocalizationLog;
    else handle = [OELocalizationTableLog objectForKey:table];

    {
        NSString *escapedKey = [key stringByReplacingOccurrencesOfString:@"\"" withString:@"\\\""];

        NSString *value = [escapedKey copy];
        NSError *error = nil;
        NSRegularExpression *ex = [NSRegularExpression regularExpressionWithPattern:@"%[0-9]*\\.*[0-9]*[hlqLztj]*[%dDuUxXoOfeEgGcCsSpaAF@]" options:0 error:&error];
        NSArray *matches = [ex matchesInString:value options:0 range:NSMakeRange(0, [key length])];
        if([matches count] > 1)
        {
            for(NSInteger i=[matches count]; i > 0; i--)
            {
                NSTextCheckingResult *match = [matches objectAtIndex:i-1];
                NSRange range = [match range];

                NSMutableString *replacement = [NSMutableString stringWithFormat:@"%%%ld$", i];

                NSRegularExpression *ex2 = [NSRegularExpression regularExpressionWithPattern:@"[hlqLztj]*[%dDuUxXoOfeEgGcCsSpaAF@]" options:0 error:nil];
                NSRange r = [[[ex2 matchesInString:value options:0 range:range] lastObject] range];
                [replacement appendString:[value substringWithRange:r]];

                value = [value stringByReplacingCharactersInRange:range withString:replacement];
            }
        } else {
            value = key;
        }


        NSString *string = [NSString stringWithFormat:@"/*r%@%@%@%d%@%@*/ \"%@\" = \"%@\";\n",
                            OELocalizationSeparationString, fileName,
                            OELocalizationSeparationString, line,
                            OELocalizationSeparationString, comment,
                            escapedKey, value];
        NSData *data = [string dataUsingEncoding:NSUTF8StringEncoding];
        [handle writeData:data];
    }

    return [@"[L]" stringByAppendingString:NSLocalizedString(key, comment)];
}
#endif
