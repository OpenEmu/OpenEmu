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
@interface OEDBImage ()
- (void)_putThumbnailsInOrder;

+ (NSData *)_convertImageToData:(NSImage *)image;
+ (NSImage *)_convertDataToImage:(NSData *)data;
- (NSData *)_convertImageToData:(NSImage *)image;
- (NSImage *)_convertDataToImage:(NSData *)data;
@end

@implementation OEDBImage

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

#pragma mark -

+ (id)imageWithImage:(NSImage *)image inContext:(NSManagedObjectContext *)context
{
    NSEntityDescription *desc = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
    OEDBImage *imageObject = [[self alloc] initWithEntity:desc insertIntoManagedObjectContext:context];
    
    NSEntityDescription *thumbnailDesc = [NSEntityDescription entityForName:@"ImageThumbnail" inManagedObjectContext:context];
    
    NSManagedObject *original = [[NSManagedObject alloc] initWithEntity:thumbnailDesc insertIntoManagedObjectContext:context];
    
    NSEntityDescription *thumbnailDataDesc = [NSEntityDescription entityForName:@"ImageData" inManagedObjectContext:context];
    NSManagedObject *thumbnailDataObj = [[NSManagedObject alloc] initWithEntity:thumbnailDataDesc insertIntoManagedObjectContext:context];
    
    NSData *imageData = [self _convertImageToData:image];
    [original setValue:thumbnailDataObj forKey:@"data"];
    [original setValue:imageData forKeyPath:@"data.data"];
    [original setValue:[NSNumber numberWithFloat:image.size.width] forKey:@"width"];
    [original setValue:[NSNumber numberWithFloat:image.size.height] forKey:@"height"];
    
    [[imageObject mutableSetValueForKey:@"versions"] addObject:original];
    
    
    return imageObject;
}

+ (id)imageWithPath:(NSString *)path inContext:(NSManagedObjectContext *)context
{
    NSEntityDescription *desc = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
    if(!image)
        return nil;
    
    OEDBImage *imageObject = [[self alloc] initWithEntity:desc insertIntoManagedObjectContext:context];
    
    NSEntityDescription *thumbnailDesc = [NSEntityDescription entityForName:@"ImageThumbnail" inManagedObjectContext:context];
    
    NSManagedObject *original = [[NSManagedObject alloc] initWithEntity:thumbnailDesc insertIntoManagedObjectContext:context];
    
    NSEntityDescription *thumbnailDataDesc = [NSEntityDescription entityForName:@"ImageData" inManagedObjectContext:context];
    NSManagedObject *thumbnailDataObj = [[NSManagedObject alloc] initWithEntity:thumbnailDataDesc insertIntoManagedObjectContext:context];
    
    NSData *imageData = [self _convertImageToData:image];
    [original setValue:thumbnailDataObj forKey:@"data"];
    [original setValue:imageData forKeyPath:@"data.data"];
    [original setValue:[NSNumber numberWithFloat:image.size.width] forKey:@"width"];
    [original setValue:[NSNumber numberWithFloat:image.size.height] forKey:@"width"];
    
    [[imageObject mutableSetValueForKey:@"versions"] addObject:original];
    
    
    return imageObject;
}

+ (id)imageWithURL:(NSURL*)url inContext:(NSManagedObjectContext *)context
{
    NSEntityDescription *desc = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    if(image == nil) return nil;
    
    OEDBImage *imageObject = [[self alloc] initWithEntity:desc insertIntoManagedObjectContext:context];
    [imageObject setValue:[url absoluteURL] forKey:@"url"];
    
    NSEntityDescription *thumbnailDesc = [NSEntityDescription entityForName:@"ImageThumbnail" inManagedObjectContext:context];
    
    NSManagedObject *original = [[NSManagedObject alloc] initWithEntity:thumbnailDesc insertIntoManagedObjectContext:context];
    
    
    NSEntityDescription *thumbnailDataDesc = [NSEntityDescription entityForName:@"ImageData" inManagedObjectContext:context];
    NSManagedObject *thumbnailDataObj = [[NSManagedObject alloc] initWithEntity:thumbnailDataDesc insertIntoManagedObjectContext:context];
    
    NSData *imageData = [self _convertImageToData:image];
    [original setValue:thumbnailDataObj forKey:@"data"];
    [original setValue:imageData forKeyPath:@"data.data"];
    [original setValue:[NSNumber numberWithFloat:image.size.width] forKey:@"width"];
    [original setValue:[NSNumber numberWithFloat:image.size.height] forKey:@"width"];
    
    [[imageObject mutableSetValueForKey:@"versions"] addObject:original];
    
    
    return imageObject;
}

+ (id)imageFromData:(NSData *)data inContext:(NSManagedObjectContext *)context
{
    NSEntityDescription *desc = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
    
    NSImage *image = [[NSImage alloc] initWithData:data];
    if(image == nil) return nil;
    
    OEDBImage *imageObject = [[self alloc] initWithEntity:desc insertIntoManagedObjectContext:context];
    
    NSEntityDescription *thumbnailDesc = [NSEntityDescription entityForName:@"ImageThumbnail" inManagedObjectContext:context];
    
    NSManagedObject *original = [[NSManagedObject alloc] initWithEntity:thumbnailDesc insertIntoManagedObjectContext:context];
    
    // TODO: think about using something other than original data to have same kind of data in all images no matter how they were created
    NSEntityDescription *thumbnailDataDesc = [NSEntityDescription entityForName:@"ImageData" inManagedObjectContext:context];
    NSManagedObject *thumbnailDataObj = [[NSManagedObject alloc] initWithEntity:thumbnailDataDesc insertIntoManagedObjectContext:context];
    
    [original setValue:thumbnailDataObj forKey:@"data"];
    [original setValue:data forKeyPath:@"data.data"];
    [original setValue:[NSNumber numberWithFloat:image.size.width] forKey:@"width"];
    [original setValue:[NSNumber numberWithFloat:image.size.height] forKey:@"width"];
    
    [[imageObject mutableSetValueForKey:@"versions"] addObject:original];
    
    
    return imageObject;
}

// returns image with highest resolution
- (NSImage *)image
{
    NSSet *thumbnailsSet = [self valueForKey:@"versions"];
    
    NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
    
    NSManagedObject *image = [thumbnails lastObject];
    NSData *imageData = [image valueForKeyPath:@"data.data"];
    
    return [self _convertDataToImage:imageData];
}

- (NSImage *)imageForSize:(NSSize)size
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
    
    NSImage *image = nil;
    
    @try
    {
        NSData *imageData = [usableThumbnail valueForKeyPath:@"data.data"];
        image = [self _convertDataToImage:imageData];
    }
    @catch (NSException *exception)
    {
        NSLog(@"caught exception!");
        NSLog(@"%@", exception);
    }
    
    
    return image;
}


- (NSSize)actualImageSizeForSize:(NSSize)size
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
    
    return NSMakeSize([[usableThumbnail valueForKey:@"width"] floatValue], [[usableThumbnail valueForKey:@"height"] floatValue]);
}

// generates thumbnail to fill size
- (void)generateImageForSize:(NSSize)size
{
    NSMutableSet *thumbnailsSet = [self mutableSetValueForKey:@"versions"];
    
    NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
    
    NSManagedObject *original = [thumbnails lastObject];
    NSSize originalSize = NSMakeSize([[original valueForKey:@"width"] floatValue], [[original valueForKey:@"height"] floatValue]);
    
    // thumbnails only make sense when smaller than original
    if(size.width >= originalSize.width || size.height >= originalSize.height) return;
    
    // TODO: check if we already have a thumbnail with specified size
    
    float originalAspect = originalSize.width / originalSize.height;
    
    NSSize thumbnailSize;
    if(originalAspect < 1)
    { // width < height
        float width = size.height * originalAspect;
        thumbnailSize = NSMakeSize(width, size.height);
    }
    else
    {
        float height = size.width / originalAspect;
        thumbnailSize = NSMakeSize(size.width, height);
    }
    
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSImage *newThumbnailImage = [[NSImage alloc] initWithSize:thumbnailSize];
    NSImage *originalImage = [self _convertDataToImage:[original valueForKeyPath:@"data.data"]];
    
    [newThumbnailImage lockFocus];
    [originalImage drawInRect:NSMakeRect(0, 0, thumbnailSize.width, thumbnailSize.height) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    [newThumbnailImage unlockFocus];
    
    NSEntityDescription *thumbnailDesc = [NSEntityDescription entityForName:@"ImageThumbnail" inManagedObjectContext:context];
    NSManagedObject *newThumbnailObject = [[NSManagedObject alloc] initWithEntity:thumbnailDesc insertIntoManagedObjectContext:context];
    
    NSEntityDescription *thumbnailDataDesc = [NSEntityDescription entityForName:@"ImageData" inManagedObjectContext:context];
    NSManagedObject *thumbnailDataObj = [[NSManagedObject alloc] initWithEntity:thumbnailDataDesc insertIntoManagedObjectContext:context];
    
    NSData *newThumbnailData = [self _convertImageToData:newThumbnailImage];
    [newThumbnailObject setValue:thumbnailDataObj forKey:@"data"];
    [newThumbnailObject setValue:newThumbnailData forKeyPath:@"data.data"];
    
    [newThumbnailObject setValue:[NSNumber numberWithFloat:thumbnailSize.width] forKey:@"width"];
    [newThumbnailObject setValue:[NSNumber numberWithFloat:thumbnailSize.height] forKey:@"height"];
    
    [thumbnailsSet addObject:newThumbnailObject];
    
    
    [self _putThumbnailsInOrder];
}

- (void)_putThumbnailsInOrder
{
}

#pragma mark -

+ (NSData *)_convertImageToData:(NSImage *)image
{
    // TODO: think about using something other than tiff data
    return [image TIFFRepresentation];
}

+ (NSImage *)_convertDataToImage:(NSData *)data
{
    return [[NSImage alloc] initWithData:data];
}

- (NSImage *)_convertDataToImage:(NSData *)data
{
    return [self.class _convertDataToImage:data];
}

- (NSData *)_convertImageToData:(NSImage *)image
{
    return [self.class _convertImageToData:image];
}

@end
