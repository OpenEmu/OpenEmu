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

#import "OEDBImageThumbnail.h"
#import "OEDBImage.h"
#import "OELibraryDatabase.h"
#import "NSString+UUID.h"

@implementation OEDBImageThumbnail

- (id)initWithImage:(NSImage*)image size:(NSSize)size inLibrary:(OELibraryDatabase*)library{
  
    NSManagedObjectContext  *context     = [library managedObjectContext];
    NSEntityDescription     *description = [OEDBImageThumbnail entityDescriptionInContext:context];
    self = [super initWithEntity:description insertIntoManagedObjectContext:context];
    if(!self) return nil;
    
    BOOL     resize      = !NSEqualSizes(size, NSZeroSize);
    NSString *version    = !resize ? @"original" : [NSString stringWithFormat:@"%d", (int)size.width];
    NSString *uuid       = [NSString stringWithUUID];
    
    NSURL    *coverFolderURL = [library coverFolderURL];
              coverFolderURL = [coverFolderURL URLByAppendingPathComponent:version isDirectory:YES];
    
    NSURL *url          = [coverFolderURL URLByAppendingPathComponent:uuid];
    
    NSBitmapImageRep    *bitmapRep       = nil;
    NSSize              imageSize        = [image size];
    float               aspectRatio      = imageSize.width / imageSize.height;
    NSSize              thumbnailSize;
    if(resize)
    {       
        thumbnailSize = aspectRatio<1 ? (NSSize){size.height * aspectRatio, size.height} : (NSSize){size.width, size.width / aspectRatio};
        
        // thumbnails only make sense if they are smaller than the original
        if(thumbnailSize.width >= imageSize.width || thumbnailSize.height >= imageSize.height){
            [context deleteObject:self];
            return nil;
        }
        
        NSImage *thumbnailImage = [[NSImage alloc] initWithSize:thumbnailSize];
        [thumbnailImage lockFocus];
        [image drawInRect:(NSRect){{0, 0}, {thumbnailSize.width, thumbnailSize.height}} fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
        bitmapRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:(NSRect){{0,0}, thumbnailSize}];
        [thumbnailImage unlockFocus];
    }
    else
    {
        thumbnailSize = imageSize;
        
        // find a bitmap representation
        for(NSImageRep *rep in [image representations])
        {
            if([rep isKindOfClass:[NSBitmapImageRep class]])
            {
                bitmapRep = (NSBitmapImageRep*)rep;
                break;
            }
        }
        
        if(!bitmapRep) // none found, create one
        {
            [image lockFocus];
            bitmapRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:(NSRect){{0,0}, thumbnailSize}];
            [image unlockFocus];
        }
    }
    
    [self setPath:[NSString stringWithFormat:@"%@/%@", version, uuid]];
    [self setWidth:[NSNumber numberWithFloat:thumbnailSize.width]];
    [self setHeight:[NSNumber numberWithFloat:thumbnailSize.height]];
    
    
    // write image file
    NSDictionary *properties = [NSDictionary dictionary];
    NSData       *imageData  = [bitmapRep representationUsingType:NSPNGFileType properties:properties];
    
    [[NSFileManager defaultManager] createDirectoryAtURL:[url URLByDeletingLastPathComponent] withIntermediateDirectories:YES attributes:nil error:nil];
    
    NSError *error = nil;
    if(![imageData writeToURL:url options:0 error:&error]){
       [context deleteObject:self];
        self = nil;
    }
    
    return self;
}

- (NSString*)absolutePath
{
    return [self path] ? [[[[self libraryDatabase] coverFolderURL] URLByAppendingPathComponent:[self path]] path] : nil;
}
@dynamic height;
@dynamic width;
@dynamic path;
@dynamic image;

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName
{
    return @"ImageThumbnail";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

- (void)prepareForDeletion
{
    [[NSFileManager defaultManager] removeItemAtPath:[self absolutePath] error:nil];
}

@end
