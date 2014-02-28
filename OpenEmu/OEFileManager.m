//
//  OEFileManager.m
//  OEFileManager
//
//  Created by Christoph Leimbrock on 28/02/14.
//  Copyright (c) 2014 ccl. All rights reserved.
//

#import "OEFileManager.h"
#include <copyfile.h>

typedef struct {
    off_t current;
    off_t done;
    off_t total;
    __unsafe_unretained OEFileManager *manager;
} copyfile_ctx;

@interface OEFileManager ()
@property (strong) NSFileManager *fm;
@property off_t totalBytes;
@property off_t bytesCopied;
@end

@implementation OEFileManager
@synthesize fm=fm;
int copyfile_callback(int what, int stage, copyfile_state_t state, const char * src, const char * dst, void * ctx_ptr);

- (id)init
{
    self = [super init];
    if (self) {
        fm = [NSFileManager defaultManager];
    }
    return self;
}

- (NSUInteger)sizeOfItemAtURL:(NSURL*)url
{
    NSUInteger fileSize = 0;
    id         value    = nil;
    NSError    *error   = nil;

    if([url getResourceValue:&value forKey:NSURLIsDirectoryKey error:&error])
    {
        if([value boolValue])
        {
            NSArray *keys = @[NSURLIsDirectoryKey, NSURLFileSizeKey];
            NSDirectoryEnumerator *directoryEnumerator = [fm enumeratorAtURL:url includingPropertiesForKeys:keys options:0 errorHandler:nil];
            while((url = [directoryEnumerator nextObject]))
            {
                if([url getResourceValue:&value forKey:NSURLIsDirectoryKey error:nil] && ![value boolValue] && [url getResourceValue:&value forKey:NSURLFileSizeKey error:nil])
                    fileSize += [value unsignedIntegerValue];
            }
        }
        else
        {
            if([url getResourceValue:&value forKey:NSURLFileSizeKey error:&error])
            {
                fileSize = [value unsignedIntegerValue];
            }
        }
    }
    return fileSize;
}

int copyfile_callback(int what, int stage, copyfile_state_t state, const char * src, const char * dst, void * ctx_ptr)
{
    return ((__bridge int (^)(copyfile_state_t, int, int, const char *, const char *))ctx_ptr)(state, what, stage, src, dst);
}

- (BOOL)copyItemAtURL:(NSURL*)source toURL:(NSURL *)destination error:(NSError *__autoreleasing *)error
{
    const char *src = [[source path] cStringUsingEncoding:NSUTF8StringEncoding];
    const char *dst = [[destination path] cStringUsingEncoding:NSUTF8StringEncoding];
    
    __block off_t total   = [self sizeOfItemAtURL:source];
    __block off_t done    = 0;

    int (^callback)(copyfile_state_t, int, int, const char *, const char *) = ^ int (copyfile_state_t state, int what, int stage, const char *csrc, const char *cdst){
        off_t current;
        copyfile_state_get(state, COPYFILE_STATE_COPIED, &current);

        off_t tmp_done = done + current;
        
        double progress = (double)tmp_done/total;
        switch (stage) {
            case COPYFILE_PROGRESS:
                if(_progressHandler)
                    return _progressHandler(progress) ? COPYFILE_CONTINUE : COPYFILE_QUIT;
                break ;
            case COPYFILE_FINISH:
                done += current;
                if((what == COPYFILE_RECURSE_DIR_CLEANUP || what == COPYFILE_RECURSE_FILE) && _itemDoneHandler!=nil)
                {
                    NSURL *srcURL = [NSURL fileURLWithFileSystemRepresentation:csrc isDirectory:what!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    NSURL *dstURL = [NSURL fileURLWithFileSystemRepresentation:cdst isDirectory:what!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    return _itemDoneHandler(srcURL, dstURL, nil) ? COPYFILE_CONTINUE : COPYFILE_QUIT;
                }
                break;
            case COPYFILE_ERR:
                if(_errorHandler)
                {
                    // TODO: figure out what error occured
                    NSURL *srcURL = [NSURL fileURLWithFileSystemRepresentation:csrc isDirectory:stage!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    NSURL *dstURL = [NSURL fileURLWithFileSystemRepresentation:cdst isDirectory:stage!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    NSError *error = [NSError errorWithDomain:@"OEFileManagerDomain" code:errno userInfo:nil];
                    return _errorHandler(srcURL, dstURL, error) ? COPYFILE_CONTINUE : COPYFILE_QUIT;
                }
                return COPYFILE_QUIT;
        }
        return COPYFILE_CONTINUE;
    };
    
    copyfile_state_t state = copyfile_state_alloc();
    copyfile_state_set(state, COPYFILE_STATE_STATUS_CB, copyfile_callback);
    copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, (__bridge const void *)(callback));

    int res = copyfile(src, dst, state, COPYFILE_ALL|COPYFILE_RECURSIVE|COPYFILE_EXCL);
    copyfile_state_free(state);
    
    if(res != NO)
        *error = [NSError errorWithDomain:@"OEFileManagerDomain" code:errno userInfo:nil];
    return res == 0;
}

@end
