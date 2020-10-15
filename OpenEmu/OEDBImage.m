/*
 Copyright (c) 2015, OpenEmu Team
 
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

#import "OEDBImage+CoreDataProperties.h"
#import "OELibraryDatabase.h"
#import "OEDBGame.h"

@interface OEDBImage ()
@end

#pragma mark -

@implementation OEDBImage

+ (nullable NSDictionary <NSString *, id> *)prepareImageWithURLString:(NSString *)urlString;
{
    if(urlString == nil) return nil;

    NSURL *url = [NSURL URLWithString:urlString];
    if(url == nil || [urlString isEqualToString:@""])
        return nil;

    NSMutableDictionary *result = @{ @"URL": url }.mutableCopy;
    
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    NSDictionary *tempInfo = [self OE_prepareImage:image];

    [result addEntriesFromDictionary:tempInfo];

    return result;
}

+ (NSDictionary *)prepareImageWithNSImage:(NSImage*)image
{
    return [self OE_prepareImage:image];
}

+ (NSDictionary *)OE_prepareImage:(NSImage*)image
{
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    if(image == nil) return result;

    __block NSSize imageSize = image.size;
    const NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *properties = [standardUserDefaults dictionaryForKey:OEGameArtworkPropertiesKey];
    NSString *fileName = [NSString stringWithUUID];
    OEBitmapImageFileType type = [standardUserDefaults integerForKey:OEGameArtworkFormatKey];

    NSBitmapImageRep *imageRep = nil;
    NSInteger maxArea = 0;
    for(NSImageRep *rep in image.representations)
    {
        if([rep isKindOfClass:[NSBitmapImageRep class]])
        {
            NSInteger area = rep.pixelsHigh * rep.pixelsWide;
            if(area >= maxArea)
            {
                imageRep = (NSBitmapImageRep*)rep;
                maxArea = area;
                
                imageSize.width  = rep.pixelsWide;
                imageSize.height = rep.pixelsHigh;
            }
        }
    }

    if(imageRep == nil)
    {
        DLog(@"No NSBitmapImageRep found, creating one...");
        NSRect proposedRect = {NSZeroPoint, imageSize};
        CGImageRef cgImage = [image CGImageForProposedRect:&proposedRect context:nil hints:nil];
        imageRep = [[NSBitmapImageRep alloc] initWithCGImage:cgImage];
    }

    if(imageRep == nil)
    {
        DLog(@"Could not draw NSImage in NSBitmapimage rep, exiting…");
        return result;
    }

    NSData *data = [imageRep representationUsingType:(NSBitmapImageFileType)type properties:properties];

    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSURL *coverFolderURL = [database coverFolderURL];
    NSURL *imageURL = [NSURL URLWithString:fileName relativeToURL:coverFolderURL];
    if(![data writeToURL:imageURL atomically:YES])
    {
        [[NSFileManager defaultManager] removeItemAtURL:imageURL error:nil];
        DLog(@"Failed to write image file! Exiting…");
        return result;
    }

    result[@"width"]        = @(imageSize.width);
    result[@"height"]       = @(imageSize.height);
    result[@"relativePath"] = imageURL.relativeString;
    result[@"format"]       = @(type);

    return result;
}

+ (instancetype)createImageWithDictionary:(NSDictionary*)dictionary
{
    OECoreDataMainThreadAssertion();
    return [self createImageWithDictionary:dictionary inContext:[[OELibraryDatabase defaultDatabase] mainThreadContext]];
}

+ (instancetype)createImageWithDictionary:(NSDictionary*)dictionary inContext:(NSManagedObjectContext*)context
{
    if(!dictionary) return nil;

    OEDBImage *image = [OEDBImage createObjectInContext:context];
    image.sourceURL = dictionary[@"URL"];
    image.width = [dictionary[@"width"] floatValue];
    image.height = [dictionary[@"height"] floatValue];

    image.relativePath = dictionary[@"relativePath"];
    image.format = [dictionary[@"format"] shortValue];

    return image;
}

#pragma mark -

- (NSURL*)OE_writeImage:(NSImage*)image withType:(OEBitmapImageFileType)type usedFormat:(NSBitmapImageFileType*)usedFormat inContext:(NSManagedObjectContext*)context
{
    const NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *properties = [standardUserDefaults dictionaryForKey:OEGameArtworkPropertiesKey];

    return [self OE_writeImage:image withType:type usedFormat:usedFormat withProperties:properties inContext:context];
}

- (NSURL*)OE_writeImage:(NSImage*)image withType:(OEBitmapImageFileType)type usedFormat:(NSBitmapImageFileType*)usedFormat withProperties:(NSDictionary*)properties inContext:(NSManagedObjectContext*)context
{
    if(image == nil)
    {
        DLog(@"No image passed in, exiting…");
        return nil;
    }

    const NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    const NSSize         imageSize             = image.size;
    NSString *fileName = [NSString stringWithUUID];

    if(type == OEBitmapImageFileTypeDefault || type==OEBitmapImageFileTypeOriginal)
        type = [standardUserDefaults integerForKey:OEGameArtworkFormatKey];

    NSBitmapImageRep *imageRep = nil;
    NSInteger maxArea = 0;
    for(NSImageRep *rep in image.representations)
    {
        if([rep isKindOfClass:[NSBitmapImageRep class]])
        {
            NSInteger area = rep.pixelsHigh * rep.pixelsWide;
            if(area >= maxArea)
            {
                imageRep = (NSBitmapImageRep *)rep;
                maxArea = area;
            }
        }
    }


    if(imageRep == nil)
    {
        DLog(@"No NSBitmapImageRep found, creating one...");
        NSRect proposedRect = {NSZeroPoint, imageSize};
        CGImageRef cgImage = [image CGImageForProposedRect:&proposedRect context:nil hints:nil];
        imageRep = [[NSBitmapImageRep alloc] initWithCGImage:cgImage];
    }

    if(imageRep == nil)
    {
        DLog(@"Could not draw NSImage in NSBitmapimage rep, exiting…");
        return nil;
    }

    NSData *data = [imageRep representationUsingType:(NSBitmapImageFileType)type properties:properties];

    // TODO: get database from context
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];

    NSURL *coverFolderURL = database.coverFolderURL;
    NSURL *imageURL = [NSURL URLWithString:fileName relativeToURL:coverFolderURL];

    if(![data writeToURL:imageURL atomically:YES])
    {
        [[NSFileManager defaultManager] removeItemAtURL:imageURL error:nil];
        DLog(@"Failed to write image file! Exiting…");
        return nil;
    }

    if(usedFormat != NULL)
        *usedFormat = (NSBitmapImageFileType)type;
    
    return imageURL;
}

- (NSURL*)OE_writeURL:(NSURL*)url withType:(OEBitmapImageFileType)type usedFormat:(NSBitmapImageFileType*)usedFormat outSize:(NSSize*)size inContext:(NSManagedObjectContext*)context
{
    NSData *data = [NSData dataWithContentsOfURL:url];
    return [self OE_writeData:data withType:type usedFormat:usedFormat outSize:size inContext:context];
}

- (NSURL*)OE_writeData:(NSData*)data withType:(OEBitmapImageFileType)type usedFormat:(NSBitmapImageFileType*)usedFormat outSize:(NSSize*)size inContext:(NSManagedObjectContext*)context
{
    NSImage *image = [[NSImage alloc] initWithData:data];
    if(size != NULL)
        *size = image.size;

    return [self OE_writeImage:image withType:type usedFormat:usedFormat inContext:context];
}

#pragma mark -

- (BOOL)convertToFormat:(OEBitmapImageFileType)format withProperties:(NSDictionary*)attributes
{
    NSManagedObjectContext *context = self.managedObjectContext;
    NSURL *newURL = [self OE_writeImage:self.image withType:format usedFormat:NULL withProperties:attributes inContext:context];
    if(newURL == nil)
    {
        DLog(@"converting image failed!");
        return NO;
    }
    else
    {
        NSManagedObjectContext *context = self.managedObjectContext;
        [context performBlockAndWait:^{
            NSURL *url = self.imageURL;
            if(url != nil) [[NSFileManager defaultManager] removeItemAtURL:url error:nil];

            NSString *relativePath = newURL.relativeString;

            self.format = format;
            self.relativePath = relativePath;
            [self save];
        }];
        return YES;
    }
}
- (BOOL)localFilesAvailable
{
    return [self.imageURL checkResourceIsReachableAndReturnError:nil];
}

#pragma mark - Core Data utilities

+ (NSString *)entityName
{
    return @"Image";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

- (void)prepareForDeletion
{
    if(self.managedObjectContext.parentContext == nil)
    {
        NSURL *url = self.imageURL;
        [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
    }
}

#pragma mark -

- (NSString *)UUID
{
    return self.relativePath;
}

- (NSImage *)image
{
    NSURL *imageURL = self.imageURL;
    NSImage  *image = [[NSImage alloc] initWithContentsOfURL:imageURL];
    return image;
}

- (NSURL *)imageURL
{
    const OELibraryDatabase *database = self.libraryDatabase;
    const NSURL *coverFolderURL = database.coverFolderURL;
    NSString    *relativePath   = self.relativePath;

    if(relativePath == nil) return nil;
    return [coverFolderURL URLByAppendingPathComponent:relativePath];
}

- (NSURL *)sourceURL
{
    NSString *source = [self source];
    if(source == nil) return nil;
    return [NSURL URLWithString:source];
}

- (void)setSourceURL:(NSURL *)sourceURL
{
    sourceURL = [sourceURL URLByStandardizingPath];
    [self setSource:[sourceURL absoluteString]];
}

- (BOOL)isLocalImageAvailable
{
    return [[self imageURL] checkResourceIsReachableAndReturnError:nil];
}
@end
