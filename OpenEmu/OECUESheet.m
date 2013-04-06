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

#import "OECUESheet.h"

@interface OECUESheet ()

@property NSString *sheetPath;
@property NSString *sheetFile;
@property NSString *referencedFilesDirectoryPath;
@property NSArray  *referencedFiles;

- (void)OE_enumerateFilesUsingBlock:(void(^)(NSString *path, BOOL *stop))block;
- (void)OE_refreshReferencedFiles;

@end

@implementation OECUESheet

- (id)initWithPath:(NSString *)path
{
    if((self = [super init]))
    {
        NSString *file = [NSString stringWithContentsOfFile:path usedEncoding:0 error:nil];
        if(file == nil) return nil;

        [self setSheetPath:path];
        [self setSheetFile:file];

        [self setReferencedFilesDirectoryPath:[path stringByDeletingLastPathComponent]];
        [self OE_refreshReferencedFiles];
    }
    return self;
}

- (id)initWithPath:(NSString *)path andReferencedFilesDirectory:(NSString *)referencedFiles
{
    if((self = [self initWithPath:path]))
    {
        [self setReferencedFilesDirectoryPath:referencedFiles];
        [self OE_refreshReferencedFiles];
    }
    return self;
}

#pragma mark - File Handling

- (BOOL)moveReferencedFilesToPath:(NSString *)newDirectory withError:(NSError **)outError
{
    __block BOOL     success = YES;
    __block NSError *error   = nil;

    NSFileManager *fileManger      = [NSFileManager defaultManager];
    NSString      *directory       = [self referencedFilesDirectoryPath];
    NSArray       *referencedFiles = [self referencedFiles];

    [referencedFiles enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         NSString *fullPath = [directory stringByAppendingPathComponent:obj];
         NSString *newPath = [newDirectory stringByAppendingPathComponent:[fullPath lastPathComponent]];
         if(![fileManger moveItemAtPath:fullPath toPath:newPath error:&error])
         {
             *stop   = YES;
             success = NO;
         }
     }];

    if(outError != NULL)
        *outError = error;

    return success;
}

- (BOOL)copyReferencedFilesToPath:(NSString *)newDirectory withError:(NSError **)outError
{
    __block BOOL     success = YES;
    __block NSError *error   = nil;

    NSFileManager *fileManger      = [NSFileManager defaultManager];
    NSString      *directory       = [self referencedFilesDirectoryPath];
    NSArray       *referencedFiles = [self referencedFiles];

    [referencedFiles enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         NSString *fullPath = [directory stringByAppendingPathComponent:obj];
         NSString *newPath = [newDirectory stringByAppendingPathComponent:[fullPath lastPathComponent]];

         if(![fileManger copyItemAtPath:fullPath toPath:newPath error:&error])
         {
             *stop   = YES;
             success = NO;
         }
     }];

    if(outError != NULL)
        *outError = error;

    return success;
}

- (BOOL)allFilesAvailable
{
    NSFileManager *fileManger      = [NSFileManager defaultManager];
    __block BOOL   success         = YES;
    NSString      *directory       = [self referencedFilesDirectoryPath];
    NSArray       *referencedFiles = [self referencedFiles];

    [referencedFiles enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         NSString *fullPath = [directory stringByAppendingPathComponent:obj];
         if(![fileManger fileExistsAtPath:fullPath])
         {
             //DLog(@"Missing File: %@", fullPath);
             *stop   = YES;
             success = NO;
         }
     }];

    return success;
}

- (NSString *)dataTrackPath
{
    return [[self referencedFiles] count] > 0 ? [[self referencedFiles] objectAtIndex:0] : nil;
}

#pragma mark - Private Helpers

- (void)OE_enumerateFilesUsingBlock:(void(^)(NSString *path, BOOL *stop))block
{
    NSString *directory     = [[self sheetPath] stringByDeletingLastPathComponent];
    NSString *sheetFileName = [[self sheetPath] lastPathComponent];
    NSArray  *allFiles      = [[self referencedFiles] arrayByAddingObject:sheetFileName];

    [allFiles enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         block([directory stringByAppendingPathComponent:obj], stop);
     }];
}

- (void)OE_refreshReferencedFiles
{
    NSRegularExpression *pattern   = [NSRegularExpression regularExpressionWithPattern:@"(?<=FILE \")[^\"]*" options:0 error:nil];
    NSRange              fullRange = NSMakeRange(0, [[self sheetFile] length]);
    NSArray             *matches   = [pattern matchesInString:[self sheetFile] options:0 range:fullRange];
    NSMutableArray      *files     = [NSMutableArray arrayWithCapacity:[matches count]];

#if OECUESheetImproveReadingByUsingBinExtension || OECUESheetImproveReadingByUsingSheetBin
    NSString *referencedDirectory = [self referencedFilesDirectoryPath];
#endif

    for(NSTextCheckingResult *match in matches)
    {
        NSString *matchedString = [[self sheetFile] substringWithRange:[match range]];

#if OECUESheetImproveReadingByUsingBinExtension
        if([[matchedString pathExtension] length] == 0)
        {
            NSString *absolutePath   = [referencedDirectory stringByAppendingPathComponent:matchedString];
            if(![[NSFileManager defaultManager] fileExistsAtPath:absolutePath])
            {
                NSString *fileNameWithBinExtension = [[absolutePath lastPathComponent] stringByAppendingPathExtension:@"bin"];
                if([[NSFileManager defaultManager] fileExistsAtPath:[referencedDirectory stringByAppendingPathComponent:fileNameWithBinExtension]])
                    matchedString = fileNameWithBinExtension;
            }
        }
#endif
        [files addObject:matchedString];
    }

#if OECUESheetImproveReadingByUsingSheetBin
    if([files count] == 1)
    {
        NSString *absolutePath = [referencedDirectory stringByAppendingPathComponent:[files lastObject]];
        if(![[NSFileManager defaultManager] fileExistsAtPath:absolutePath])
        {
            NSString *sheetNameWithBinExtension = [[[[self sheetPath] lastPathComponent] stringByDeletingPathExtension] stringByAppendingPathExtension:@"bin"];
            if([[NSFileManager defaultManager] fileExistsAtPath:[referencedDirectory stringByAppendingPathComponent:sheetNameWithBinExtension]])
                [files replaceObjectAtIndex:0 withObject:sheetNameWithBinExtension];
        }
    }
#endif

    [self setReferencedFiles:files];
}

@end
