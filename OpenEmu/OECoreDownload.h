//
//  OECoreDownload.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
@class SUAppcast, SUAppcastItem;

@class OECorePlugin;
@class OECoreDownload;
@protocol OECoreDownloadDelegate
- (void)OEDownloadDidStart:(OECoreDownload *)download;
- (void)OEDownloadDidFinish:(OECoreDownload *)download;
@end
@interface OECoreDownload : NSObject
{
    id<OECoreDownloadDelegate>  delegate;
    
    NSString               *downloadPath;
    NSString               *fullPluginPath;
    
    unsigned long long      expectedLength;
    unsigned long long      downloadedSize;
    
    BOOL                    downloading;
}
- (id)initWithPlugin:(OECorePlugin*)plugin;
- (void)startDownload:(id)sender;

@property (assign) id <OECoreDownloadDelegate> delegate;

@property (copy) NSString* name;
@property (copy) NSString* description;
@property (copy) NSString* version;

@property BOOL hasUpdate;
@property BOOL canBeInstalled;

@property (readonly, getter = isDownloading) BOOL downloading;

@property (readonly) float progress;
@property (retain)   SUAppcast*appcast;
@property (retain)   SUAppcastItem          *appcastItem;

@end
