//
//  OEDBImage.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 08.07.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEDBImage.h"
@interface OEDBImage (Private)
- (void)_putThumbnailsInOrder;


+ (NSData*)_convertImageToData:(NSImage*)image;
+ (NSImage*)_convertDataToImage:(NSData*)data;
- (NSData*)_convertImageToData:(NSImage*)image;
- (NSImage*)_convertDataToImage:(NSData*)data;
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

+ (id)newFromImage:(NSImage*)image inContext:(NSManagedObjectContext*)context
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
	
	[thumbnailDataObj release];
	[original release];
	
	return imageObject;
}

+ (id)newFromPath:(NSString*)path inContext:(NSManagedObjectContext*)context
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
	
		[thumbnailDataObj release];
	[image release];
	[original release];
	
	return imageObject;
}

+ (id)newFromURL:(NSURL*)url inContext:(NSManagedObjectContext*)context
{
	NSEntityDescription *desc = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
	
	
	NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
	if(!image)
		return nil;
	
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
	
		[thumbnailDataObj release];
	[image release];
	[original release];
	
	return imageObject;
}

+ (id)newFromData:(NSData*)data inContext:(NSManagedObjectContext*)context
{
	NSEntityDescription *desc = [NSEntityDescription entityForName:@"Image" inManagedObjectContext:context];
	
	NSImage *image = [[NSImage alloc] initWithData:data];
	if(!image)
		return nil;
	
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
	
		[thumbnailDataObj release];
	[image release];
	[original release];
	
	return imageObject;
}

// returns image with highest resolution
- (NSImage*)image
{
	NSSet *thumbnailsSet = [self valueForKey:@"versions"];
	
	NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
	NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
	
	
	NSManagedObject *image = [thumbnails lastObject];
	NSData *imageData = [image valueForKeyPath:@"data.data"];
	return [self _convertDataToImage:imageData];
}

- (NSImage*)imageForSize:(NSSize)size
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
			usableThumbnail = [obj retain];
			break;
		}
	}
	
	if(!usableThumbnail)
    {
		usableThumbnail = [[thumbnails lastObject] retain];	
	}

	NSImage *image = nil;
	@try {
		NSData *imageData = [usableThumbnail valueForKeyPath:@"data.data"];
		image = [self _convertDataToImage:imageData];
	}
	@catch (NSException *exception) {
		NSLog(@"caught exception!");
		NSLog(@"%@", exception);
	}
	[usableThumbnail release];
	
	return image;
}

// generates thumbnail to fill size
- (void)generateImageForSize:(NSSize)size{
	NSMutableSet *thumbnailsSet = [self mutableSetValueForKey:@"versions"];
	
	NSSortDescriptor *sotDescr = [NSSortDescriptor sortDescriptorWithKey:@"width" ascending:YES];
	NSArray *thumbnails = [thumbnailsSet sortedArrayUsingDescriptors:[NSArray arrayWithObject:sotDescr]];
	
	NSManagedObject *original = [thumbnails lastObject];
	NSSize originalSize = NSMakeSize([[original valueForKey:@"width"] floatValue], [[original valueForKey:@"height"] floatValue]);
	
	// thumbnails only make sense when smaller than original
	if(size.width >= originalSize.width || size.height >= originalSize.height){
		return;
	}
	
	// TODO: check if we already have a thumbnail with specified size
	
	float originalAspect = originalSize.width / originalSize.height;
	
	NSSize thumbnailSize;
	if(originalAspect < 1){ // width < height
		
		float width = size.height * originalAspect;
		thumbnailSize = NSMakeSize(width, size.height);
	} else { // 
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
	
	[thumbnailDataObj release];
	[newThumbnailImage release];
	[newThumbnailObject release];
	 
		
	[self _putThumbnailsInOrder];
}

- (void)_putThumbnailsInOrder{
}

#pragma mark -
+ (NSData*)_convertImageToData:(NSImage*)image{
	// TODO: think about using something other than tiff data
	return [image TIFFRepresentation];
}
+ (NSImage*)_convertDataToImage:(NSData*)data{
	return [[[NSImage alloc] initWithData:data] autorelease];
}


- (NSImage*)_convertDataToImage:(NSData*)data{
	return [self.class _convertDataToImage:data];
}
- (NSData*)_convertImageToData:(NSImage*)image{
	return [self.class _convertImageToData:image];
}
@end
