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
@dynamic sourceURL, versions, Box;
#pragma mark - Core Data utilities
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
#pragma mark -
// returns image with highest resolution
- (NSImage *)originalImage
{
    NSSet *thumbnailsSet = [self valueForKey:@"versions"];
    
    NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
    
    OEDBImageThumbnail *originalImage = [thumbnails lastObject];
    NSURL *url = [[[self libraryDatabase] coverFolderURL] URLByAppendingPathComponent:[originalImage relativePath]];
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    return image;
}

- (NSImage *)imageForSize:(NSSize)size
{    
    NSSet *thumbnailsSet = [self valueForKey:@"versions"];
    
    NSSortDescriptor *sortDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
    NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sortDescr]];
    
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
    
    NSURL *url = [[[self libraryDatabase] coverFolderURL] URLByAppendingPathComponent:[usableThumbnail relativePath]];
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

@end
