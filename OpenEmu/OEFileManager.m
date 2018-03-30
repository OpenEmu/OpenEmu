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

#import "OEFileManager.h"
#include <copyfile.h>

@interface OEFileManager ()
@end

@implementation OEFileManager
int copyfile_callback(int what, int stage, copyfile_state_t state, const char * src, const char * dst, void * ctx_ptr);

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
            NSDirectoryEnumerator *directoryEnumerator = [self enumeratorAtURL:url includingPropertiesForKeys:keys options:0 errorHandler:nil];
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
    const char *src = source.path.fileSystemRepresentation;
    const char *dst = destination.path.fileSystemRepresentation;
    
    __block off_t total   = [self sizeOfItemAtURL:source];
    __block off_t done    = 0;

    int (^callback)(copyfile_state_t, int, int, const char *, const char *) = ^ int (copyfile_state_t state, int what, int stage, const char *csrc, const char *cdst){
        off_t current;
        copyfile_state_get(state, COPYFILE_STATE_COPIED, &current);

        off_t tmp_done = done + current;
        
        double progress = (double)tmp_done/total;
        switch (stage) {
            case COPYFILE_PROGRESS:
                if(self->_progressHandler)
                    return self->_progressHandler(progress) ? COPYFILE_CONTINUE : COPYFILE_QUIT;
                break ;
            case COPYFILE_FINISH:
                done += current;
                if((what == COPYFILE_RECURSE_DIR_CLEANUP || what == COPYFILE_RECURSE_FILE) && self->_itemDoneHandler!=nil)
                {
                    NSURL *srcURL = [NSURL fileURLWithFileSystemRepresentation:csrc isDirectory:what!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    NSURL *dstURL = [NSURL fileURLWithFileSystemRepresentation:cdst isDirectory:what!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    return self->_itemDoneHandler(srcURL, dstURL, nil) ? COPYFILE_CONTINUE : COPYFILE_QUIT;
                }
                break;
            case COPYFILE_ERR:
                if(self->_errorHandler)
                {
                    // TODO: figure out what error occured
                    NSURL *srcURL = [NSURL fileURLWithFileSystemRepresentation:csrc isDirectory:stage!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    NSURL *dstURL = [NSURL fileURLWithFileSystemRepresentation:cdst isDirectory:stage!=COPYFILE_RECURSE_FILE relativeToURL:nil];
                    NSError *error = [NSError errorWithDomain:@"OEFileManagerDomain" code:errno userInfo:nil];
                    return self->_errorHandler(srcURL, dstURL, error) ? COPYFILE_CONTINUE : COPYFILE_QUIT;
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
    
    if(res != NO && error != NULL)
        *error = [NSError errorWithDomain:@"OEFileManagerDomain" code:errno userInfo:nil];
    return res == 0;
}
@end
