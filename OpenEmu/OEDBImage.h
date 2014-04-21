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

#import "OEDBItem.h"

typedef enum : NSBitmapImageFileType
{
    OEBitmapImageFileTypeOriginal,
    OEBitmapImageFileTypeDefault,
} OEBitmapImageFileType;

@class OELibraryDatabase, OEDBImageThumbnail, OEDBGame;
@interface OEDBImage : OEDBItem

+ (instancetype)createImageWithNSImage:(NSImage*)image;
+ (instancetype)createImageWithNSImage:(NSImage*)image type:(OEBitmapImageFileType)type;
+ (instancetype)createImageWithNSImage:(NSImage*)image type:(OEBitmapImageFileType)type inLibrary:(OELibraryDatabase*)library;

+ (instancetype)createImageWithURL:(NSURL*)url;
+ (instancetype)createImageWithURL:(NSURL*)url type:(OEBitmapImageFileType)type;
+ (instancetype)createImageWithURL:(NSURL*)url type:(OEBitmapImageFileType)type inLibrary:(OELibraryDatabase*)library;
+ (instancetype)createImageWithData:(NSData*)data;
+ (instancetype)createImageWithData:(NSData*)data type:(OEBitmapImageFileType)type;
+ (instancetype)createImageWithData:(NSData*)data type:(OEBitmapImageFileType)type inLibrary:(OELibraryDatabase*)library;
#pragma mark - Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

- (NSImage *)image;
- (NSURL *)imageURL;
@property (nonatomic) NSURL *sourceURL;

@property (readonly, nonatomic, getter = isLocalImageAvailable) BOOL localImageAvailable;

@property (nonatomic) float width, height;
@property (nonatomic) NSInteger format;
@property (nonatomic, retain) NSString *relativePath;
@property (nonatomic, retain) NSString * source;
@property (nonatomic, retain) OEDBGame *Box;
@end
