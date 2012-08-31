/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OEDBImage.h"
#import "OEDBImageThumbnail.h"
#import "OELibraryDatabase.h"
@interface OEDBImage ()
@end
#pragma mark -
@implementation OEDBImage
+ (id)imageWithImage:(NSImage *)image inLibrary:(OELibraryDatabase *)library
{
    NSManagedObjectContext  *context        = [library managedObjectContext];
    NSEntityDescription     *desc           = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
    OEDBImage               *imageObject    = [[self alloc] initWithEntity:desc insertIntoManagedObjectContext:context];
    OEDBImageThumbnail      *thumbnailImage = [[OEDBImageThumbnail alloc] initWithImage:image size:NSZeroSize inLibrary:library];
    
    if(!thumbnailImage)
    {
        [context deleteObject:imageObject];
        return nil;
    }
    
    [imageObject addVersion:thumbnailImage];
    return imageObject;
}

+ (id)imageWithImage:(NSImage *)image andSourceURL:(NSURL*)url inLibrary:(OELibraryDatabase *)library
{
    OEDBImage *imageObject = [OEDBImage imageWithImage:image inLibrary:library];
    [imageObject setSourceURL:[url absoluteString]];
    return imageObject;
}

+ (id)imageWithPath:(NSString *)path inLibrary:(OELibraryDatabase *)library
{
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
    if(!image) return nil;
    
    OEDBImage *imageObject = [OEDBImage imageWithImage:image inLibrary:library];
    [imageObject setSourceURL:[[NSURL fileURLWithPath:path] absoluteString]];
    return imageObject;
}

+ (id)imageWithURL:(NSURL*)url inLibrary:(OELibraryDatabase *)library
{
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    if(!image) return nil;
    
    OEDBImage *imageObject = [OEDBImage imageWithImage:image inLibrary:library];
    [imageObject setSourceURL:[url absoluteString]];
    return imageObject;
}

+ (id)imageWithData:(NSData *)data inLibrary:(OELibraryDatabase *)library
{
    NSImage *image = [[NSImage alloc] initWithData:data];
    if(!image) return nil;
    
    OEDBImage *imageObject = [OEDBImage imageWithImage:image inLibrary:library];
    return imageObject;
}
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName
{
    return @"Image";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

- (void)addVersion:(OEDBImageThumbnail*)version
{
    if(version)
        [[self mutableSetValueForKey:@"versions"] addObject:version];
}
- (void)removeVersion:(OEDBImageThumbnail*)version
{
    if(version)
        [[self mutableSetValueForKey:@"versions"] removeObject:version];
}
@dynamic sourceURL;
#pragma mark -
// returns image with highest resolution
- (NSImage *)originalImage
{
    NSSet *thumbnailsSet = [self valueForKey:@"versions"];
    
    NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
    
    OEDBImageThumbnail *originalImage = [thumbnails lastObject];
    NSURL *url = [[[self libraryDatabase] coverFolderURL] URLByAppendingPathComponent:[originalImage path]];
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    return image;
}

- (NSImage *)imageForSize:(NSSize)size
{    
    NSSet *thumbnailsSet = [self valueForKey:@"versions"];
    
    NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
    
    OEDBImageThumbnail *usableThumbnail = nil;
    for(OEDBImageThumbnail *obj in thumbnails)
    {
        if([[obj valueForKey:@"width"] floatValue] >= size.width ||
           [[obj valueForKey:@"height"] floatValue] >= size.height)
        {
            usableThumbnail = obj;
            break;
        }
    }
    
    if(usableThumbnail == nil)
        usableThumbnail = [thumbnails lastObject];
    
    if(!usableThumbnail) return nil;
    
    NSURL *url = [[[self libraryDatabase] coverFolderURL] URLByAppendingPathComponent:[usableThumbnail path]];
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    return image;
}


- (NSSize)sizeOfThumbnailForSize:(NSSize)size
{
    NSSet *thumbnailsSet = [self valueForKey:@"versions"];
    
    NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
    
    NSManagedObject *usableThumbnail = nil;
    for(NSManagedObject *obj in thumbnails)
    {
        if([[obj valueForKey:@"width"] floatValue] >= size.width ||
           [[obj valueForKey:@"height"] floatValue] >= size.height)
        {
            usableThumbnail = obj;
            break;
        }
    }
    
    if(usableThumbnail == nil)
        usableThumbnail = [thumbnails lastObject];
    
    return (NSSize){[[usableThumbnail valueForKey:@"width"] floatValue], [[usableThumbnail valueForKey:@"height"] floatValue]};
}

// generates thumbnail to fill size
- (void)generateThumbnailForSize:(NSSize)size
{
    @autoreleasepool {
        OELibraryDatabase *library = [self libraryDatabase];
        NSURL             *coverFolderURL = [library coverFolderURL];
        
        // Find Original Thumbnail
        NSSet               *thumbnailsSet  = [self valueForKey:@"versions"];
        NSSortDescriptor    *sotDescr       = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
        NSArray             *thumbnails     = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
        
        OEDBImageThumbnail  *originalImageThumb  = [thumbnails lastObject];
        NSURL               *originalURL         = [coverFolderURL URLByAppendingPathComponent:[originalImageThumb path]];
        NSImage             *originalImage       = [[NSImage alloc] initWithContentsOfURL:originalURL];
        
        if(!originalImage){
            return;
        }
        OEDBImageThumbnail  *newThumbnail        = [[OEDBImageThumbnail alloc] initWithImage:originalImage size:size inLibrary:library];
        [self addVersion:newThumbnail];
    }
}
@end
