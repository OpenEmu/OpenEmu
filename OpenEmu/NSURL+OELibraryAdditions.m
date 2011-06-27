//
//  NSURL+OELibraryAdditions.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 05.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "NSURL+OELibraryAdditions.h"


@implementation NSURL (NSURL_OELibraryAdditions)
- (BOOL)hasImageSuffix{
	NSArray* imageSuffixes = [NSArray arrayWithObjects:	
							  @"png",												// Portable Network Graphics
							  @"tif", @"tiff",										// Tagged Image File Format
							  @"jpg", @"jpeg", @"jpe", @"jif", @"jfif", @"jfi",		// JPG
							  @"jpf",												// JPG 2000
							  @"bmp", @"dib",										// Windows Bitmap / device-independent bitmap
							  @"gif",												// Graphics Interchange Format
							  @"ico",												
							  @"pdf",												// Portable Document Format
							  @"eps", @"epsf", @"epsi",								// Encapsulated PostScript
							  @"pict", @"pct", @"pic",								// Apple Macintosh QuickDraw
							  @"psd",												// Adobe Photoshop
							  @"tga", @"tpic",										// YESvision TGA
							  nil];
	
	NSString* urlSuffix = [[self pathExtension] lowercaseString];
	return [imageSuffixes containsObject:urlSuffix];
}
@end
