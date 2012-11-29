#import "OECUESheet.h"

@interface OECUESheet ()
@property NSString *sheetPath;
@property NSString *sheetFile;
@property NSString *referencedFilesDirectoryPath;
@property NSArray  *referencedFiles;

- (void)OE_enumerateAllFilesUsingBlock:(void(^)(NSString *path, BOOL * stop))blck;
- (void)OE_refreshReferencedFiles;
@end

@implementation OECUESheet
- (id)initWithPath:(NSString*)path
{
    self = [super init];
    if (self)
    {
        NSString *file = [NSString stringWithContentsOfFile:path usedEncoding:0 error:nil];
        if(file == nil)
            return nil;
        
        [self setSheetPath:path];
        [self setSheetFile:file];
        
        [self setReferencedFilesDirectoryPath:[path stringByDeletingLastPathComponent]];
        [self OE_refreshReferencedFiles];
    }
    return self;
}

- (id)initWithPath:(NSString*)path andReferencedFilesDirectory:(NSString*)referencedFiles
{
    self = [self initWithPath:path];
    if (self)
    {
        [self setReferencedFilesDirectoryPath:referencedFiles];
        [self OE_refreshReferencedFiles];        
    }
    return self;
}
#pragma mark - File Handling
- (BOOL)moveReferencedFilesToPath:(NSString*)newDirectory withError:(NSError**)outError
{
    __block BOOL    success = YES;
    __block NSError *error  = nil;
    
    NSFileManager *fileManger      = [NSFileManager defaultManager];
    NSString      *directory       = [self referencedFilesDirectoryPath];
    NSArray       *referencedFiles = [self referencedFiles];
    
    [referencedFiles enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
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

- (BOOL)copyReferencedFilesToPath:(NSString*)newDirectory withError:(NSError**)outError
{
    __block BOOL    success = YES;
    __block NSError *error  = nil;
    
    NSFileManager *fileManger      = [NSFileManager defaultManager];
    NSString      *directory       = [self referencedFilesDirectoryPath];
    NSArray       *referencedFiles = [self referencedFiles];
    
    
    [referencedFiles enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
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
    __block BOOL  success          = YES;
    NSString      *directory       = [self referencedFilesDirectoryPath];
    NSArray       *referencedFiles = [self referencedFiles];
    
    [referencedFiles enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *fullPath = [directory stringByAppendingPathComponent:obj];
        if(![fileManger fileExistsAtPath:fullPath])
        {
            DLog(@"Missing File: %@", fullPath);
            *stop   = YES;
            success = NO;
        }
    }];
    return success;

}
#pragma mark - Private Helpers
- (void)OE_enumerateAllFilesUsingBlock:(void(^)(NSString *path, BOOL * stop))blck
{
    NSString      *directory  = [[self sheetPath] stringByDeletingLastPathComponent];
    NSString  *sheetFileName  = [[self sheetPath] lastPathComponent];
    NSArray    *allFiles      = [[self referencedFiles] arrayByAddingObject:sheetFileName];
    
    [allFiles enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *fullPath = [directory stringByAppendingPathComponent:obj];
        blck(fullPath, stop);
    }];
}

- (void)OE_refreshReferencedFiles
{
    NSRegularExpression *pattern  = [NSRegularExpression regularExpressionWithPattern:@"(?<=FILE \")[^\"]*" options:0 error:nil];
    NSRange             fulLRange = NSMakeRange(0, [[self sheetFile] length]);
    NSArray             *matches  = [pattern matchesInString:[self sheetFile] options:0 range:fulLRange];
    NSMutableArray      *files    = [NSMutableArray arrayWithCapacity:[matches count]];
    
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
                {
                    matchedString = fileNameWithBinExtension;
                }
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
            {
                [files replaceObjectAtIndex:0 withObject:sheetNameWithBinExtension];
            }
        }
    }
#endif

    [self setReferencedFiles:files];
}
@end
